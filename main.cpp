#include <iostream>
#include <fstream>
#include <cerrno>
#include <string>
#include <cstring>
#include <pthread.h>
#include <csignal>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <unistd.h>
#include <wait.h>
#include "misce.h"
#include "response.h"


void handle_error(const char *func_name, int line, int error_code, int fd, const char *msg);

int init_ipv4(unsigned short port);

int init_ipv6(unsigned short port);

void *accept_thread(void *arg);

long do_session(int fd);

long get_content(int fd, std::string &webpath);

long exec_cgi(int fd, const std::string &webpath, const char *method, const std::string &args);

static bool end_loop = false;

void abort_loop(int sig)
{
	end_loop = true;
	(void)signal(sig, SIG_DFL);
}

int main()
{
	signal(SIGINT, abort_loop);
	int ipv4_fd = init_ipv4(8086);
	int ipv6_fd = init_ipv6(8086);
	if ((ipv4_fd == -1) && (ipv6_fd == -1))
	{
		return -1;
	}

	pthread_t accept_v6_thread;
	if (ipv6_fd != -1)
	{
		pthread_create(&accept_v6_thread, nullptr, accept_thread, reinterpret_cast<void *>(static_cast<long>(ipv6_fd)));
		pthread_setname_np(accept_v6_thread, "ipv6_accept_thread");
	}
	if (ipv4_fd != -1)
	{
		sockaddr_in client_addr{};
		socklen_t client_name_len = sizeof(client_addr);

		while (!end_loop)
		{
			int client_fd = accept(ipv4_fd, reinterpret_cast<sockaddr *>(&client_addr), &client_name_len);
			if (client_fd == -1)
			{
				handle_error(__func__, __LINE__, errno, client_fd, "ipv4 accept connect error!");
			}
			else
			{
#ifdef _DEBUG
				char addr_str[INET6_ADDRSTRLEN];
				addr_str[0] = 0;
				inet_ntop(client_addr.sin_family, &client_addr.sin_addr, addr_str, sizeof(addr_str));
				std::cout << "new ipv4 connection: " << addr_str << "@" << ntohs(client_addr.sin_port) << std::endl;
#endif
				do_session(client_fd);
				close(client_fd);
			}
		}
	}
	if (ipv6_fd != -1)
	{
		pthread_kill(accept_v6_thread, SIGINT);
		pthread_join(accept_v6_thread, nullptr);
	}
	return 0;
}

void *accept_thread(void *arg)
{
	int fd = static_cast<int>(reinterpret_cast<long>(arg));
	sockaddr_in6 client_addr6{};
	socklen_t client_name_len = sizeof(client_addr6);

	while (!end_loop)
	{
		int client_fd = accept(fd, reinterpret_cast<sockaddr *>(&client_addr6), &client_name_len);
		if (client_fd == -1)
		{
			handle_error(__func__, __LINE__, errno, fd, "accept failed!");
		}
		else
		{
#ifdef _DEBUG
			char addr_str[INET6_ADDRSTRLEN];
			addr_str[0] = 0;
			inet_ntop(client_addr6.sin6_family, &client_addr6.sin6_addr, addr_str, sizeof(addr_str));
			std::cout << "new ipv6 connection: " << addr_str << "@" << ntohs(client_addr6.sin6_port) << std::endl;
#endif
			do_session(client_fd);
			close(client_fd);
		}
	}
	return nullptr;
}

//int parseConfig(std::string string)
//{
//	return 0;
//}

int init_ipv4(unsigned short port)
{
	int fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (fd == -1)
	{
		handle_error(__func__, __LINE__, errno, fd, "create ipv4 socket failed!");
		return -1;
	}

	sockaddr_in source_addr{};
	source_addr.sin_family = AF_INET;
	source_addr.sin_port = htons(port);
	source_addr.sin_addr.s_addr = htonl(INADDR_ANY);

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

	if (port == 0)
	{
		socklen_t name_len = sizeof(source_addr);
		if (-1 == getsockname(fd, reinterpret_cast<sockaddr *>(&source_addr), &name_len))
		{
			handle_error(__func__, __LINE__, errno, fd, "ipv4 getsockname failed!");
			return -1;
		}
		else
		{
			std::cout << "ipv4 bind port " << htons(source_addr.sin_port) << std::endl;
		}
	}
	if (-1 == listen(fd, 5))
	{
		handle_error(__func__, __LINE__, errno, fd, "ipv4 listen error!");
		return -1;
	}

	return fd;
}

int init_ipv6(unsigned short port)
{
	int fd = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
	if (fd == -1)
	{
		handle_error(__func__, __LINE__, errno, fd, "create ipv6 socket failed!");
		return -1;
	}

	sockaddr_in6 source_addr{};
	source_addr.sin6_family = AF_INET6;
	source_addr.sin6_port = htons(port);
	source_addr.sin6_addr = IN6ADDR_ANY_INIT;

	int reuse_addr_opt = 1;
	if (-1 == setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuse_addr_opt, sizeof(reuse_addr_opt)))
	{
		handle_error(__func__, __LINE__, errno, fd, "ipv6 set addr reuse error!");
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

	if (port == 0)
	{
		socklen_t name_len = sizeof(source_addr);
		if (-1 == getsockname(fd, reinterpret_cast<sockaddr *>(&source_addr), &name_len))
		{
			handle_error(__func__, __LINE__, errno, fd, "ipv6 getsockname failed!");
			return -1;
		}
		else
		{
			std::cout << "ipv6 bind port " << htons(source_addr.sin6_port) << std::endl;
		}
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
		return not_implemented(fd);
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

	std::string webpath = "./htdocs";
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
		return not_found(fd);
	}
	else
	{
		if ((webroot_stat.st_mode & S_IFMT) == S_IFDIR)
		{
			webpath.append("/index.html");
		}
		if (webroot_stat.st_mode & (S_IXGRP | S_IXUSR | S_IXOTH))
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
	headers.append("Server: tinyhttpd/0.1.0\r\n");
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
			return bad_request(fd);
		}
	}

	if ((pipe(pipe_p2cgi) == -1) || (pipe(pipe_cgi2p) == -1))
	{
		std::cerr << "create pipe failed!" << strerror(errno) << "(" << errno << ")" << std::endl;
		return internal_server_error(fd);
	}
	pid_t pid = fork();
	if (pid < 0)
	{
		std::cerr << "cannot fork process!" << strerror(errno) << "(" << errno << ")" << std::endl;
		return internal_server_error(fd);
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
				int var = recv(fd, &buffer, sizeof(buffer), 0);
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
		int var;
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
	char buffer[8192];
	std::ifstream ifs;
	ifs.open(webpath, std::ios::in | std::ios::binary);
	if (ifs.is_open())
	{
		std::string headers = make_headers();
		headers.append("Content-Type: text/html\r\n\r\n");
		send(fd, headers.c_str(), headers.length(), 0);
		while (ifs.good())
		{
			ifs.read(buffer, sizeof(buffer));
			send(fd, buffer, ifs.gcount(), 0);
		}
		ifs.close();
		return 0;
	}
	else
	{
		return not_found(fd);
	}
}