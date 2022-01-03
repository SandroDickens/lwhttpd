#ifndef TINYHTTPD_HTTP_CONFIG_H
#define TINYHTTPD_HTTP_CONFIG_H


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
	} addr;
};

class ListenConfig
{
public:
	[[nodiscard]] sockaddr_generic get_address() const;

	void set_address(const std::string &addr);

	[[nodiscard]] unsigned short get_port() const;

	void set_port(unsigned short port);

	friend void tag_invoke(boost::json::value_from_tag, boost::json::value &json_value, ListenConfig const &config);

private:
	sockaddr_generic _address;
};


class TLSConfig
{
public:
	[[nodiscard]] std::string get_cert_file() const;

	void set_cert_file(std::string file_name);

	[[nodiscard]] std::string get_key_file() const;

	void set_key_file(std::string file_name);

	friend void tag_invoke(boost::json::value_from_tag, boost::json::value &json_value, TLSConfig const &config);

private:
	std::string tls_cert_file{};
	std::string tls_key_file{};
};

class HttpConfig
{
public:
	std::string get_server_name();

	void set_server_name(std::string name);

	std::vector<ListenConfig> get_listen_cfg();

	void set_listen_cfg(std::vector<ListenConfig> cfg);

	std::string get_web_root();

	void set_web_root(std::string root);

	TLSConfig get_tls_cfg();

	void set_tls_cfg(TLSConfig cfg);

	void parser_json_value(const boost::json::value &json_value);

	static void print_http_config(const HttpConfig &config);

	friend void tag_invoke(boost::json::value_from_tag, boost::json::value &json_value, HttpConfig const &config);

private:
	std::string server_name{};
	std::vector<ListenConfig> listen{};
	std::string web_root{};
	TLSConfig tls_config;
};

#endif //TINYHTTPD_HTTP_CONFIG_H
