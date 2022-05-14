#include <iostream>
#include <fstream>
#include <cerrno>
#include <string>
#include <cstring>
#include <pthread.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <wait.h>
#include <vector>
#include <unordered_set>
#include "utils/utils.hpp"
#include "http/response.hpp"
#include "config/http_config.h"
#include "http/mime_types.h"


void handle_error(const char *func_name, int line, int error_code, int fd, const char *msg);

int init_ipv4(sockaddr_in addr);

int init_ipv6(sockaddr_in6 addr);

long do_session(int fd);

long get_content(int fd, std::string &webpath);

long exec_cgi(int fd, const std::string &webpath, const char *method, const std::string &args);

static bool end_loop = false;

struct event_data;

typedef void (*callback_fun)(epoll_event *, std::unordered_set<event_data *> &);

struct event_data
{
	int ep_fd;
	int sock_fd;
	int family;
	callback_fun callback;
};


void accept_connect(epoll_event *event, std::unordered_set<event_data *> &data_set);

void session_handler(epoll_event *event, std::unordered_set<event_data *> &data_set);

void abort_loop(int sig)
{
	end_loop = true;
	(void)signal(sig, SIG_DFL);
}

HttpConfig http_config;

int main()
{
	signal(SIGINT, abort_loop);
	try
	{
		http_config.parser_json_value("../config.json");
	}
	catch (std::exception &exc)
	{
		std::cout << exc.what() << std::endl;
		return -1;
	}
	int ipv4_fd = -1, ipv6_fd = -1;
	auto var = http_config.get_listen_cfg();
	for (auto cfg:var)
	{
		auto gen_addr = cfg.get_address();
		if ((ipv4_fd == -1) && (cfg.get_address().family == AF_INET))
		{
			sockaddr_in addr{};
			addr.sin_family = gen_addr.family;
			addr.sin_port = gen_addr.port;
			memcpy(&addr.sin_addr, &gen_addr.addr, sizeof(in_addr));
			ipv4_fd = init_ipv4(addr);
		}
		if ((ipv6_fd == -1) && (gen_addr.family == AF_INET6))
		{
			sockaddr_in6 addr{};
			addr.sin6_family = gen_addr.family;
			addr.sin6_port = gen_addr.port;
			memcpy(&addr.sin6_addr, &gen_addr.addr, sizeof(in6_addr));
			ipv6_fd = init_ipv6(addr);
		}
	}

	if ((ipv4_fd == -1) && (ipv6_fd == -1))
	{
		std::cerr << "socket initialization failed." << std::endl;
		return -1;
	}

	int epoll_fd = epoll_create(1);
	if (epoll_fd == -1)
	{
		close(ipv4_fd);
		close(ipv6_fd);
		handle_error(__func__, __LINE__, errno, -1, "epoll create failed!");
		return -1;
	}

	event_data *ipv4_event_data = nullptr;
	event_data *ipv6_event_data = nullptr;
	if (ipv4_fd != -1)
	{
		try
		{
			ipv4_event_data = new event_data();
		}
		catch (std::bad_alloc &e)
		{
			std::cerr << e.what() << std::endl;
			close(ipv4_fd);
			ipv4_fd = -1;
		}
	}
	if (ipv6_fd != -1)
	{
		try
		{
			ipv6_event_data = new event_data();
		}
		catch (std::bad_alloc &e)
		{
			std::cerr << e.what() << std::endl;
			close(ipv6_fd);
			ipv6_fd = -1;
		}
	}
	if ((ipv4_event_data == nullptr) && (ipv6_event_data == nullptr))
	{
		close(epoll_fd);
		return -1;
	}
	if (ipv4_event_data != nullptr)
	{
		ipv4_event_data->ep_fd = epoll_fd;
		ipv4_event_data->sock_fd = ipv4_fd;
		ipv4_event_data->family = AF_INET;
		ipv4_event_data->callback = accept_connect;

		epoll_event ep_event{};
		ep_event.events = EPOLLIN;
		ep_event.data.ptr = ipv4_event_data;
		epoll_ctl(epoll_fd, EPOLL_CTL_ADD, ipv4_fd, &ep_event);
	}
	if (ipv6_event_data != nullptr)
	{
		ipv6_event_data->ep_fd = epoll_fd;
		ipv6_event_data->sock_fd = ipv6_fd;
		ipv6_event_data->family = AF_INET6;
		ipv6_event_data->callback = accept_connect;

		epoll_event ep_event{};
		ep_event.events = EPOLLIN;
		ep_event.data.ptr = ipv6_event_data;
		epoll_ctl(epoll_fd, EPOLL_CTL_ADD, ipv6_fd, &ep_event);
	}
	constexpr int MAX_EVENTS = 10;
	constexpr int WAIT_TIME = 1000;
	epoll_event trig_event[MAX_EVENTS];
	std::unordered_set<event_data *> data_set;
	while (!end_loop)
	{
		memset(trig_event, 0, sizeof(trig_event));
		int ret = epoll_wait(epoll_fd, trig_event, MAX_EVENTS, WAIT_TIME);
		if (ret == -1)
		{
			if (errno != EINTR)
			{
				std::cerr << __func__ << "@" << __LINE__ << ": epoll wait error! " << strerror(errno) << "(" << errno
				          << ")" << std::endl;
			}
		}
		else if (ret > 0)
		{
			for (int i = 0; i < ret; ++i)
			{
				callback_fun callback = (static_cast<event_data *>(trig_event[i].data.ptr))->callback;
				(*callback)(&trig_event[i], data_set);
			}
		}
	}
	while (!data_set.empty())
	{
		auto begin_it = data_set.begin();
		event_data *event_data = *begin_it;
		if (event_data != nullptr)
		{
			shutdown(event_data->sock_fd, SHUT_RDWR);
			close(event_data->sock_fd);
			delete event_data;
		}
		data_set.erase(begin_it);
	}
	close(epoll_fd);
	if (ipv4_fd != -1)
	{
		close(ipv4_fd);
	}
	if (ipv6_fd != -1)
	{
		close(ipv6_fd);
	}

	return 0;
}

void accept_connect(epoll_event *event, std::unordered_set<event_data *> &data_set)
{
	event_data *evt_data = static_cast<event_data *>(event->data.ptr);
	if (event->events & EPOLLIN)
	{
		int fd = evt_data->sock_fd;
		int client_fd;
		if (evt_data->family == AF_INET)
		{
			sockaddr_in client_addr{};
			socklen_t client_name_len = sizeof(client_addr);
			client_fd = accept(fd, reinterpret_cast<sockaddr *>(&client_addr), &client_name_len);
			if (client_fd == -1)
			{
				handle_error(__func__, __LINE__, errno, client_fd, "accept client connect error!");
			}
			else
			{
#ifdef _DEBUG
				char addr_str[INET_ADDRSTRLEN];
				addr_str[0] = 0;
				inet_ntop(client_addr.sin_family, &client_addr.sin_addr, addr_str, sizeof(addr_str));
				std::cout << "new ipv4 connection: " << addr_str << "@" << ntohs(client_addr.sin_port) << std::endl;
#endif
			}
		}
		else
		{
			sockaddr_in6 client_addr{};
			socklen_t client_name_len = sizeof(client_addr);
			client_fd = accept(fd, reinterpret_cast<sockaddr *>(&client_addr), &client_name_len);
			if (client_fd == -1)
			{
				handle_error(__func__, __LINE__, errno, client_fd, "accept client connect error!");
			}
			else
			{
#ifdef _DEBUG
				char addr_str[INET6_ADDRSTRLEN];
				addr_str[0] = 0;
				inet_ntop(client_addr.sin6_family, &client_addr.sin6_addr, addr_str, sizeof(addr_str));
				std::cout << "new ipv6 connection: " << addr_str << "@" << ntohs(client_addr.sin6_port) << std::endl;
#endif
			}
		}
		event_data *new_evt_data = new event_data();
		new_evt_data->ep_fd = evt_data->ep_fd;
		new_evt_data->sock_fd = client_fd;
		new_evt_data->family = evt_data->family;
		new_evt_data->callback = session_handler;
		epoll_event ep_event{};
		ep_event.events = EPOLLIN | EPOLLRDHUP | EPOLLHUP | EPOLLET;
		ep_event.data.ptr = new_evt_data;
		if (-1 == epoll_ctl(evt_data->ep_fd, EPOLL_CTL_ADD, client_fd, &ep_event))
		{
			handle_error(__func__, __LINE__, errno, client_fd, "add new accepted socket fd to epoll failed!");
			delete new_evt_data;
		}
		else
		{
			data_set.insert(new_evt_data);
		}
	}
	else
	{
		int error = 0;
		socklen_t errlen = sizeof(error);
		if (getsockopt(evt_data->sock_fd, SOL_SOCKET, SO_ERROR, &error, &errlen) == 0)
		{
			printf("error = %s\n", strerror(error));
		}
		sockaddr_in client_addr{};
		socklen_t client_name_len = sizeof(client_addr);
		int client_fd = accept(evt_data->sock_fd, reinterpret_cast<sockaddr *>(&client_addr), &client_name_len);
		if (client_fd == -1)
		{
			handle_error(__func__, __LINE__, errno, client_fd, "accept client connect error!");
		}
		std::cerr << "Unrecognized epoll event 0x" << std::hex << event->events << std::endl;
	}
}

void session_handler(epoll_event *event, std::unordered_set<event_data *> &data_set)
{
	event_data *evt_data = static_cast<event_data *>(event->data.ptr);
	uint32_t events = event->events;
	if (events & (EPOLLHUP | EPOLLRDHUP))
	{
		if (-1 == epoll_ctl(evt_data->ep_fd, EPOLL_CTL_DEL, evt_data->sock_fd, nullptr))
		{
			handle_error(__func__, __LINE__, errno, -1, "delete socket fd from epoll failed!");
		}
		else
		{
			shutdown(evt_data->sock_fd, SHUT_RDWR);
			close(evt_data->sock_fd);
			data_set.erase(data_set.find(evt_data));
			delete evt_data;
		}
	}
	else if (events & EPOLLERR)
	{
		handle_error(__func__, __LINE__, errno, -1, "epoll has an error!");
	}
	else if (events & EPOLLIN)
	{
		do_session(evt_data->sock_fd);
	}
	else
	{
		int error = 0;
		socklen_t errlen = sizeof(error);
		if (getsockopt(evt_data->sock_fd, SOL_SOCKET, SO_ERROR, &error, &errlen) == 0)
		{
			printf("error = %s\n", strerror(error));
		}
		std::cerr << "Unrecognized epoll event 0x" << std::hex << events << std::endl;
	}
}

int init_ipv4(const sockaddr_in source_addr)
{
	int fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (fd == -1)
	{
		handle_error(__func__, __LINE__, errno, fd, "create ipv4 socket failed!");
		return -1;
	}

	int reuse_addr_opt = 1;
	if (-1 == setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuse_addr_opt, sizeof(int)))
	{
		handle_error(__func__, __LINE__, errno, fd, "ipv4 setsockopt error!");
	}
	if (-1 == bind(fd, reinterpret_cast<const sockaddr *>(&source_addr), sizeof(source_addr)))
	{
		handle_error(__func__, __LINE__, errno, fd, "ipv4 bind port failed!");
		return -1;
	}

	sockaddr_in tmp_addr{};
	socklen_t name_len = sizeof(tmp_addr);
	if (-1 == getsockname(fd, reinterpret_cast<sockaddr *>(&tmp_addr), &name_len))
	{
		handle_error(__func__, __LINE__, errno, fd, "ipv4 getsockname failed!");
		return -1;
	}
	else
	{
		std::cout << "ipv4 bind port " << ntohs(tmp_addr.sin_port) << std::endl;
	}

	if (-1 == listen(fd, 5))
	{
		handle_error(__func__, __LINE__, errno, fd, "ipv4 listen error!");
		return -1;
	}

	return fd;
}

int init_ipv6(const sockaddr_in6 source_addr)
{
	int fd = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
	if (fd == -1)
	{
		handle_error(__func__, __LINE__, errno, fd, "create ipv6 socket failed!");
		return -1;
	}

	int reuse_addr_opt = 1;
	if (-1 == setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuse_addr_opt, sizeof(reuse_addr_opt)))
	{
		handle_error(__func__, __LINE__, errno, fd, "ipv6 set listenAddr reuse error!");
	}
	int ipv6_only_opt = 1;
	if (-1 == setsockopt(fd, IPPROTO_IPV6, IPV6_V6ONLY, &ipv6_only_opt, sizeof(ipv6_only_opt)))
	{
		handle_error(__func__, __LINE__, errno, fd, "ipv6 setsockopt error!");
	}

	if (-1 == bind(fd, reinterpret_cast<const sockaddr *>(&source_addr), sizeof(source_addr)))
	{
		handle_error(__func__, __LINE__, errno, fd, "ipv6 bind port failed!");
		return -1;
	}

	sockaddr_in6 tmp_addr{};
	socklen_t name_len = sizeof(tmp_addr);
	if (-1 == getsockname(fd, reinterpret_cast<sockaddr *>(&tmp_addr), &name_len))
	{
		handle_error(__func__, __LINE__, errno, fd, "ipv6 getsockname failed!");
		return -1;
	}
	else
	{
		std::cout << "ipv6 bind port " << ntohs(tmp_addr.sin6_port) << std::endl;
	}

	if (-1 == listen(fd, 5))
	{
		handle_error(__func__, __LINE__, errno, fd, "ipv6 listen error!");
		return -1;
	}

	return fd;
}

void handle_error(const char *func_name, int line, int error_code, int fd, const char *msg)
{
	if (fd != -1)
	{
		close(fd);
	}
	std::cerr << func_name << "@" << line << ": " << msg << strerror(error_code) << "(" << error_code << ")"
	          << std::endl;
}

long do_session(int fd)
{
	char buffer[1024];
	char method[32];
	char url_buf[255];
	std::string url;
	std::string query_string;
	bool is_cgi = false;

	unsigned long numchars = readline(fd, buffer, sizeof(buffer));
	if (numchars == 0)
	{
		return -1;
	}
	unsigned int i_buf = 0, index = 0;
	while ((!isspace(buffer[i_buf])) && (i_buf < sizeof(buffer)))
	{
		method[index] = buffer[i_buf];
		++i_buf;
		++index;
	}
	method[index] = '\0';

	if ((0 != strcasecmp(method, "GET")) && (0 != strcasecmp(method, "POST")))
	{
		std::string path = "/";
		return not_implemented(fd, path);
	}

	if (strcasecmp(method, "POST") == 0)
	{
		is_cgi = true;
	}

	//skip space
	while ((isspace(buffer[i_buf])) && (i_buf < sizeof(buffer)))
	{
		++i_buf;
	}
	index = 0;
	while ((!isspace(buffer[i_buf])) && (i_buf < sizeof(buffer)) && (index < sizeof(url_buf)))
	{
		url_buf[index] = buffer[i_buf];
		++index;
		++i_buf;
	}
	url_buf[index] = '\0';
	url.append(url_buf);

	if (strcasecmp(method, "GET") == 0)
	{
		auto pos = url.find('?');
		if (pos != std::string::npos)
		{
			query_string.append(url.substr(pos, std::string::npos));
			is_cgi = true;
			url.erase(pos, std::string::npos);
		}
	}

	std::string webpath = http_config.get_web_root();
	webpath.append(url);
	if (webpath[webpath.length() - 1] == '/')
	{
		webpath.append("index.html");
	}
#ifdef _DEBUG
	std::cout << "method: " << method << ", webpath: " << webpath << ", args: " << query_string << std::endl;
#endif
	struct stat64 webroot_stat{};
	if (-1 == stat64(webpath.c_str(), &webroot_stat))
	{
		while ((numchars > 0) && (strcmp(buffer, "\n") != 0))
		{
			numchars = readline(fd, buffer, sizeof(buffer));
		}
		return not_found(fd, webpath);
	}
	else
	{
		if ((webroot_stat.st_mode & S_IFMT) == S_IFDIR)
		{
			webpath.append("/index.html");
		}
		if ((MIMETypes::isELF(webpath)) && (webroot_stat.st_mode & (S_IXGRP | S_IXUSR | S_IXOTH)))
		{
			is_cgi = true;
		}
		if (is_cgi)
		{
			return exec_cgi(fd, webpath, method, query_string);
		}
		else
		{
			return get_content(fd, webpath);
		}
	}
}

std::string make_headers()
{
	std::string headers;
	headers.append("HTTP/1.0 200 OK\r\n");
	headers.append("Server: minorhttpd/0.0.1\r\n");
	return headers;
}

long exec_cgi(int fd, const std::string &webpath, const char *method, const std::string &args)
{
	char buffer[8192];
	unsigned long content_len = 0;
	int pipe_p2cgi[2]; //parent -> cgi
	int pipe_cgi2p[2]; //cgi -> parent

	if (strcmp(method, "GET") == 0)
	{
		unsigned long numchars;
		do
		{
			numchars = readline(fd, buffer, sizeof(buffer));
		} while ((numchars > 0) && (0 != strcmp("\n", buffer)));
	}
	else
	{
		unsigned long numchars;
		do
		{
			numchars = readline(fd, buffer, sizeof(buffer));
			buffer[15] = '\0';
			if (strncasecmp("Content-Length:", buffer, 15) == 0)
			{
				content_len = strtoul(buffer, nullptr, 10);
			}
		} while ((numchars > 0) && (0 != strcmp("\n", buffer)));
		if (content_len == 0)
		{
			return bad_request(fd, webpath);
		}
	}

	if ((pipe(pipe_p2cgi) == -1) || (pipe(pipe_cgi2p) == -1))
	{
		std::cerr << "create pipe failed!" << strerror(errno) << "(" << errno << ")" << std::endl;
		return internal_server_error(fd, webpath);
	}
	pid_t pid = fork();
	if (pid < 0)
	{
		std::cerr << "cannot fork process!" << strerror(errno) << "(" << errno << ")" << std::endl;
		return internal_server_error(fd, webpath);
	}
	if (pid == 0)
	{
		dup2(pipe_p2cgi[0], 0);
		dup2(pipe_cgi2p[1], 1);
		close(pipe_p2cgi[1]); // write-end used by parent(parent read, cgi write)
		close(pipe_cgi2p[0]); // read-end used by parent(cgi write, parent read)

		char method_env[32];
		scnprintf(method_env, sizeof(method_env), "REQUEST_METHOD=%s", method);
		putenv(method_env);
		if (strcasecmp(method, "GET") == 0)
		{
			char query_str_env[255];
			scnprintf(query_str_env, sizeof(query_str_env), "QUERY_STRING=%s", args.c_str());
			putenv(query_str_env);
		}
		else
		{
			char content_len_env[32];
			scnprintf(content_len_env, sizeof(content_len_env), "CONTENT_LENGTH=%d", content_len);
			putenv(content_len_env);
		}
		execl(webpath.c_str(), webpath.c_str(), nullptr);
		exit(0);
	}
	else
	{
		close(pipe_p2cgi[0]); //read-end used by cgi(parent write, cgi read)
		close(pipe_cgi2p[1]); //write-end used by cgi(cgi write, parent read)

		unsigned int read_size = 0;
		if (strcasecmp(method, "POST") == 0)
		{
			do
			{
				long var = recv(fd, &buffer, sizeof(buffer), 0);
				if (var != -1)
				{
					read_size += var;
				}
				else
				{
					break;
				}
			} while (read_size < content_len);
			write(pipe_p2cgi[1], buffer, read_size);
		}
		read_size = 0;
		long var;
		do
		{
			var = read(pipe_cgi2p[0], buffer, sizeof(buffer));
			if (var > 0)
			{
				read_size += var;
			}
		} while (var > 0);
		std::string header = make_headers();
		send(fd, header.c_str(), header.length(), 0);
		send(fd, buffer, read_size, 0);
		close(pipe_p2cgi[1]);
		close(pipe_cgi2p[0]);
		int cgi_exit_status = 0;
		waitpid(pid, &cgi_exit_status, 0);
		if (cgi_exit_status != 0)
		{
			std::cerr << "cgi exit code: " << cgi_exit_status << std::endl;
		}
		return cgi_exit_status;
	}
}

long get_content(int fd, std::string &webpath)
{
	std::ifstream ifs;
	ifs.open(webpath, std::ios::in | std::ios::binary);
	if (ifs.is_open())
	{
		std::string headers = make_headers();
		MIMETypes &mimeTypes = MIMETypes::getInstance();
		std::string contentType = MIMETypes::getMIMEType(webpath);
		headers.append("Content-Type: " + contentType + "\r\n");

		size_t begin_pos = ifs.tellg();
		ifs.seekg(0, std::ios::end);
		size_t end_pos = ifs.tellg();
		unsigned long file_size = end_pos - begin_pos;
		char *buffer = new char[file_size];
		ifs.seekg(0, std::ios::beg);

		unsigned long read_size = 0;
		while ((ifs.good()) && (read_size < sizeof(buffer)))
		{
			ifs.read(buffer + read_size, std::streamsize(file_size - read_size));
			read_size += ifs.gcount();
		}
		headers.append("Content-Length: " + std::to_string(read_size) + "\r\n\r\n");
		send(fd, headers.c_str(), headers.length(), 0);
		send(fd, buffer, read_size, 0);
		ifs.close();
		return 0;
	}
	else
	{
		return not_found(fd, webpath);
	}
}
