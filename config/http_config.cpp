#include <fstream>
#include <iostream>
#include <boost/json.hpp>
#include <utility>
#include <arpa/inet.h>

#include "http_config.h"

//socket监听配置
void tag_invoke(boost::json::value_from_tag, boost::json::value &json_value, const ListenConfig &config)
{
	int family = config._address.family;
	char addr_str[INET6_ADDRSTRLEN];
	inet_ntop(family, &config._address.addr, addr_str, sizeof(addr_str));
	json_value = {{"address", addr_str},
	              {"port",    ntohs(config._address.port)}};
}

sockaddr_generic ListenConfig::get_address() const
{
	return _address;
}

void ListenConfig::set_address(const std::string &addr)
{
	int family = AF_INET;
	if (std::string::npos != addr.find(':'))
	{
		family = AF_INET6;
	}
	inet_pton(family, addr.c_str(), &_address.addr);
	_address.family = family;
}

unsigned short ListenConfig::get_port() const
{
	return ntohs(_address.port);
}

void ListenConfig::set_port(unsigned short port)
{
	_address.port = htons(port);
}

//TLS证书配置
void tag_invoke(boost::json::value_from_tag, boost::json::value &json_value, const TLSConfig &config)
{
	json_value = {{"tls_cert_file", config.tls_cert_file},
	              {"tls_key_file",  config.tls_key_file}};
}

std::string TLSConfig::get_cert_file() const
{
	return tls_cert_file;
}

void TLSConfig::set_cert_file(std::string file_name)
{
	tls_cert_file = std::move(file_name);
}

std::string TLSConfig::get_key_file() const
{
	return tls_key_file;
}

void TLSConfig::set_key_file(std::string file_name)
{
	tls_key_file = std::move(file_name);
}

//HTTP整体配置
void tag_invoke(boost::json::value_from_tag, boost::json::value &json_value, const HttpConfig &config)
{
	json_value = {{"server_name", config.server_name},
	              {"listen",      config.listen},
	              {"web_root",    config.web_root},
	              {"tls",         config.tls_config}};
}

std::string HttpConfig::get_server_name()
{
	return server_name;
}

void HttpConfig::set_server_name(std::string name)
{
	server_name = std::move(name);
}

std::vector<ListenConfig> HttpConfig::get_listen_cfg()
{
	return listen;
}

void HttpConfig::set_listen_cfg(std::vector<ListenConfig> cfg)
{
	listen = std::move(cfg);
}

std::string HttpConfig::get_web_root()
{
	return web_root;
}

void HttpConfig::set_web_root(std::string root)
{
	web_root = std::move(root);
}

TLSConfig HttpConfig::get_tls_cfg()
{
	return tls_config;
}

void HttpConfig::set_tls_cfg(TLSConfig cfg)
{
	tls_config = std::move(cfg);
}

void HttpConfig::print_http_config(const HttpConfig &config)
{
	char addr_str[INET6_ADDRSTRLEN];
	std::cout << "server name:" << config.server_name << std::endl;
	std::cout << "listen: \n";
	for (const ListenConfig &tmp:config.listen)
	{
		auto addr = tmp.get_address();
		inet_ntop(addr.family, &addr.addr, addr_str, sizeof(addr_str));
		std::cout << "address: " << addr_str << "@" << tmp.get_port() << std::endl;
	}
	std::cout << "web root" << config.web_root << std::endl;
	std::cout << "tls cert file: " << config.tls_config.get_cert_file();
	std::cout << "tls key file" << config.tls_config.get_key_file() << std::endl;
}

//解析配置
void HttpConfig::parser_json_value(const std::string &json_file)
{
	//读取JSON配置文件
	std::ifstream ifs;
	ifs.open(json_file, std::ios::in);
	if (!ifs.is_open())
	{
		throw std::invalid_argument("JSON file opening failed");
	}
	std::string json_str;
	while (ifs.good())
	{
		char buffer[4096];
		ifs.read(buffer, sizeof(buffer) - 1);
		size_t read_size = ifs.gcount();
		if (read_size > 0)
		{
			buffer[read_size] = '\0';
			json_str.append(buffer);
		}
	}
	ifs.close();

	boost::json::error_code error_code;
	boost::json::parse_options parse_options{};
	parse_options.allow_comments = true;
	boost::json::storage_ptr sp{};
	boost::json::value json_value = boost::json::parse(json_str, error_code, sp, parse_options);
	if (error_code)
	{
		throw std::invalid_argument("Invalid JSON content!" + error_code.message());
	}

	boost::json::object config_obj = boost::json::value_to<boost::json::object>(json_value);
	auto it = config_obj.find("server_name");
	if (it != config_obj.end())
	{
		server_name = boost::json::value_to<std::string>(it->value());
	}

	//listen
	it = config_obj.find("listen");
	if (it != config_obj.end())
	{
		std::vector<boost::json::object> listen_vec = boost::json::value_to<std::vector<boost::json::object>>(
				it->value());
		std::cout << "listen: [";
		for (boost::json::object tmp_obj:listen_vec)
		{
			ListenConfig listen_config{};
			listen_config.set_address(boost::json::value_to<std::string>(tmp_obj.at("address")));
			listen_config.set_port(boost::json::value_to<int>(tmp_obj.at("port")));
			listen.push_back(listen_config);
		}
		std::cout << "]" << std::endl;
	}
	//web root
	it = config_obj.find("web_root");
	if (it != config_obj.end())
	{
		web_root = boost::json::value_to<std::string>(it->value());
	}
	//TLS
	it = config_obj.find("tls");
	if (it != config_obj.end())
	{
		boost::json::object tls_config_obj = boost::json::value_to<boost::json::object>(it->value());
		auto it2 = tls_config_obj.find("cert_file");
		if (it2 != tls_config_obj.end())
		{
			tls_config.set_cert_file(boost::json::value_to<std::string>(it2->value()));
		}
		it2 = tls_config_obj.find("key_file");
		if (it2 != tls_config_obj.end())
		{
			tls_config.set_key_file(boost::json::value_to<std::string>(it2->value()));
		}
	}
}
