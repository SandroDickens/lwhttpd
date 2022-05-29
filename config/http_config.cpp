#include <fstream>
#include <iostream>
#include <boost/json.hpp>
#include <arpa/inet.h>
#include <filesystem>
#include <utility>

#include "http_config.h"

//解析配置
httpd_cfg httpd_cfg::parser_config(const std::string &json_file)
{
	std::ifstream ifs;
	ifs.open(json_file, std::ios::in);
	if (!ifs.is_open())
	{
		std::cerr << "JSON file opening failed" << std::endl;
		throw std::filesystem::filesystem_error("JSON file " + json_file + "opening failed",
		                                        std::make_error_code(std::io_errc::stream));
	}

	boost::json::error_code error_code;
	boost::json::parse_options parse_options{};
	parse_options.allow_comments = true;
	boost::json::storage_ptr sp{};
	boost::json::stream_parser sparser(sp, parse_options);
	while (ifs.good())
	{
		char buffer[4096];
		ifs.read(buffer, sizeof(buffer) - 1);
		size_t read_size = ifs.gcount();
		if (read_size > 0)
		{
			buffer[read_size] = '\0';
			sparser.write(buffer, error_code);
			if (error_code)
			{
				break;
			}
		}
	}
	ifs.close();
	if (error_code)
	{
		throw std::invalid_argument("JSON parse failed! " + error_code.message());
	}
	sparser.finish();
	boost::json::value json_value = sparser.release();
	return boost::json::value_to<httpd_cfg>(json_value);
}

/* sockaddr_generic */
void tag_invoke(boost::json::value_from_tag, boost::json::value &json_value, const sockaddr_generic &config)
{
	char buff[INET6_ADDRSTRLEN];
	buff[0] = '\0';
	inet_ntop(config.family, &config.addr, buff, sizeof(buff));
	json_value = {{"address", buff},
	              {"port",    ntohs(config.port)}};
}

sockaddr_generic tag_invoke(boost::json::value_to_tag<sockaddr_generic>, const boost::json::value &json_value)
{
	sockaddr_generic addr{};
	addr.family = AF_INET;
	auto addr_str = json_value.at("address").as_string();
	if (std::string::npos != addr_str.find(":"))
	{
		addr.family = AF_INET6;
	}
	inet_pton(addr.family, addr_str.c_str(), &addr.addr);
	auto type = json_value.at("port").kind();
	addr.port = htons(json_value.at("port").as_int64());
	return addr;
}

/* listen_cfg */
void listen_cfg::set_server_address(const sockaddr_generic &address)
{
	server_address = address;
}

sockaddr_generic listen_cfg::get_server_address() const
{
	return server_address;
}

void tag_invoke(boost::json::value_from_tag, boost::json::value &json_value, const listen_cfg &config)
{
	json_value = boost::json::object{{"listen",
	                                  boost::json::value_to<std::vector<sockaddr_generic>>(json_value.at("listen"))}};
}

listen_cfg tag_invoke(boost::json::value_to_tag<listen_cfg>, const boost::json::value &json_value)
{
	listen_cfg cfg;
	sockaddr_generic address = boost::json::value_to<sockaddr_generic>(json_value);
	cfg.set_server_address(address);
	return cfg;
}

/* tls_cfg */
void tls_cfg::set_cert(std::string cert)
{
	cert_file = std::move(cert);
}

std::string tls_cfg::get_cert() const
{
	return cert_file;
}

void tls_cfg::set_key(std::string key)
{
	key_file = std::move(key);
}

std::string tls_cfg::get_key() const
{
	return key_file;
}

void tag_invoke(boost::json::value_from_tag, boost::json::value &json_value, const tls_cfg &config)
{
	json_value = boost::json::object{{"cert_file", config.cert_file},
	                                 {"key_file",  config.key_file}};
}

tls_cfg tag_invoke(boost::json::value_to_tag<tls_cfg>, const boost::json::value &json_value)
{
	const std::string cert = boost::json::value_to<std::string>(json_value.at("cert_file"));
	const std::string key = boost::json::value_to<std::string>(json_value.at("key_file"));
	tls_cfg cfg;
	cfg.set_cert(cert);
	cfg.set_key(key);
	return cfg;
}

/* httpd_cfg */
void httpd_cfg::set_server_name(std::string &name)
{
	server_name = name;
}

std::string httpd_cfg::get_server_name() const
{
	return server_name;
}

void httpd_cfg::add_listen(const listen_cfg &cfg)
{
	listen.push_back(cfg);
}

void httpd_cfg::clear_listen()
{
	listen.clear();
}

std::vector<listen_cfg> httpd_cfg::get_listen() const
{
	return listen;
}


void httpd_cfg::set_web_root(std::string &root)
{
	web_root = root;
}

std::string httpd_cfg::get_web_root() const
{
	return web_root;
}

void httpd_cfg::set_tls_cfg(const tls_cfg &cfg)
{
	tls_config = cfg;
}

tls_cfg httpd_cfg::get_tls_cfg() const
{
	return tls_config;
}

void httpd_cfg::set_work_thread(unsigned long count)
{
	work_thread = count;
}

unsigned long httpd_cfg::get_work_thread() const
{
	return work_thread;
}

void tag_invoke(boost::json::value_from_tag, boost::json::value &json_value, const httpd_cfg &config)
{
	json_value = boost::json::object{{"server_name", config.server_name},
	                                 {"listen",      config.listen},
	                                 {"web_root",    config.web_root},
	                                 {"tls",         config.tls_config}};
}

httpd_cfg tag_invoke(boost::json::value_to_tag<httpd_cfg>, const boost::json::value &json_value)
{
	httpd_cfg cfg;
	std::string name = boost::json::value_to<std::string>(json_value.at("server_name"));
	cfg.set_server_name(name);
	std::vector<listen_cfg> listen_vec = boost::json::value_to<std::vector<listen_cfg>>(json_value.at("listen"));
	for (const auto &x:listen_vec)
	{
		cfg.add_listen(x);
	}
	std::string root = boost::json::value_to<std::string>(json_value.at("web_root"));
	cfg.set_web_root(root);
	cfg.set_work_thread(json_value.at("work_thread").as_int64());
	tls_cfg tls = boost::json::value_to<tls_cfg>(json_value.at("tls"));
	cfg.set_tls_cfg(tls);
	return cfg;
}
