#ifndef TINYHTTPD_HTTPCONFIG_H
#define TINYHTTPD_HTTPCONFIG_H


#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <utility>

constexpr int LISTEN_FAMILY_4 = 0x01;
constexpr int LISTEN_FAMILY_6 = 0x10;

struct ListenAddr
{
	int config_af;
	sockaddr_in ipv4_addr;
	sockaddr_in6 ipv6_addr;
};

class TLSConfig
{
public:
	void set_cert_file(std::string file)
	{
		this->cert_file = std::move(file);
	}

	std::string get_cert_file()
	{
		return this->cert_file;
	}

	void set_key_file(std::string file)
	{
		this->key_file = std::move(file);
	}

	std::string get_key_file()
	{
		return this->key_file;
	}

private:
	std::string cert_file;
	std::string key_file;
};

class HttpConfig
{
public:
	int parsingConfigJSON(const std::string &json_file_name);

	[[nodiscard]] std::string getServerName() const
	{
		return this->serverName;
	}

	[[nodiscard]] ListenAddr getListenAddr() const
	{
		return this->listenAddr;
	}

	[[nodiscard]] std::string getWebRoot() const
	{
		return this->webRoot;
	}

	[[nodiscard]] TLSConfig getTLSConfig() const
	{
		return this->tlsConfig;
	}

private:
	std::string serverName;
	ListenAddr listenAddr;
	std::string webRoot;
	TLSConfig tlsConfig;
};


#endif //TINYHTTPD_HTTPCONFIG_H
