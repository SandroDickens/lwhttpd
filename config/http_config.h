#ifndef LWHTTPD_CONFIG_H
#define LWHTTPD_CONFIG_H


#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <utility>
#include <boost/json.hpp>

struct sockaddr_generic
{
	unsigned char family;
	unsigned short port;
	union
	{
		in_addr addr4;
		in6_addr addr6;
	}addr;

	friend void tag_invoke(boost::json::value_from_tag, boost::json::value &json_value, sockaddr_generic const &config);

	friend sockaddr_generic
	tag_invoke(boost::json::value_to_tag<sockaddr_generic>, boost::json::value const &json_value);
};

class listen_cfg
{
public:
	void set_server_address(const sockaddr_generic &addr_vec);

	[[nodiscard]] sockaddr_generic get_server_address() const;

	friend void tag_invoke(boost::json::value_from_tag, boost::json::value &json_value, listen_cfg const &config);

	friend listen_cfg tag_invoke(boost::json::value_to_tag<listen_cfg>, boost::json::value const &json_value);

private:
	sockaddr_generic server_address;
};


class tls_cfg
{
public:
	void set_cert(std::string cert);

	[[nodiscard]] std::string get_cert() const;

	void set_key(std::string key);

	[[nodiscard]] std::string get_key() const;

	friend void tag_invoke(boost::json::value_from_tag, boost::json::value &json_value, tls_cfg const &config);

	friend tls_cfg tag_invoke(boost::json::value_to_tag<tls_cfg>, boost::json::value const &json_value);

private:
	std::string cert_file{};
	std::string key_file{};
};

class httpd_cfg
{
public:
	static httpd_cfg parser_config(const std::string &json_file);

	void set_server_name(std::string &name);

	[[nodiscard]] std::string get_server_name() const;

	void set_web_root(std::string &root);

	[[nodiscard]] std::string get_web_root() const;

	void set_work_thread(unsigned long count);

	[[nodiscard]] unsigned long get_work_thread() const;

	void add_listen(const listen_cfg &cfg);

	void clear_listen();

	[[nodiscard]] std::vector<listen_cfg> get_listen() const;

	void set_tls_cfg(const tls_cfg &cfg);

	[[nodiscard]] tls_cfg get_tls_cfg() const;

	friend void tag_invoke(boost::json::value_from_tag, boost::json::value &json_value, httpd_cfg const &config);

	friend httpd_cfg tag_invoke(boost::json::value_to_tag<httpd_cfg>, boost::json::value const &json_value);

private:
	std::string server_name{};
	std::string web_root{};
	tls_cfg tls_config;
	unsigned long work_thread;
	std::vector<listen_cfg> listen{};
};

#endif //LWHTTPD_CONFIG_H
