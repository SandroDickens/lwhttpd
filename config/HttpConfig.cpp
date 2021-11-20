#include <fstream>
#include <iostream>
#include <arpa/inet.h>
#include "HttpConfig.h"
#include "rapidjson/document.h"

int HttpConfig::parsingConfigJSON(const std::string &json_file_name)
{
	std::ifstream json_ifs;
	json_ifs.open(json_file_name, std::ios::in);
	if (!json_ifs.is_open())
	{
		std::cerr << "Unable to open JSON configuration file!" << std::endl;
		return -1;
	}
	std::string json_str;
	char json_file_content[8192];
	while (json_ifs.good())
	{
		json_file_content[0] = '\0';
		json_ifs.read(json_file_content, sizeof(json_file_content) - 1);
		long read_size = json_ifs.gcount();
		if (read_size > 0)
		{
			json_file_content[read_size] = '\0';
			json_str.append(json_file_content);
		}
	}
	json_ifs.close();
	std::cout << "JSON configuration:\n" << json_str << std::endl;

	rapidjson::Document json_doc;
	json_doc.Parse(json_str.c_str());
	if (json_doc.HasParseError())
	{
		int error_code = json_doc.GetParseError();
		unsigned long error_offset = json_doc.GetErrorOffset();
		std::cerr << "JSON configuration file parsing failed!";
		std::cerr << error_code << ": error at " << error_offset << ", " << json_str.at(error_offset) << std::endl;
		return -1;
	}

	std::string key_name = "serverName";
	if ((json_doc.HasMember(key_name.c_str())) && (json_doc[key_name.c_str()].IsString()))
	{
		this->serverName = json_doc[key_name.c_str()].GetString();
	}
	else
	{
		this->serverName = "localhost";
	}

	key_name = "listen";
	if ((json_doc.HasMember(key_name.c_str())) && (json_doc[key_name.c_str()].IsArray()))
	{
		auto var = json_doc[key_name.c_str()].GetArray();
		for (rapidjson::Value::ConstValueIterator it = var.Begin(); it != var.end(); it++)
		{
			auto obj = it->GetObject();
			std::string addr_str = "0.0.0.0";
			int port = 80;
			if (obj.HasMember("address"))
			{
				addr_str = obj["address"].GetString();
			}
			if (obj.HasMember("port"))
			{
				port = obj["port"].GetInt();
			}
			int family = AF_INET;
			if (std::string::npos != addr_str.find(':'))
			{
				family = AF_INET6;
			}
			if (family == AF_INET)
			{
				listenAddr.config_af |= LISTEN_FAMILY_4;
				listenAddr.ipv4_addr.sin_family = AF_INET;
				listenAddr.ipv4_addr.sin_port = htons(port);
				if (1 != inet_pton(family, addr_str.c_str(), &listenAddr.ipv4_addr.sin_addr))
				{
					listenAddr.ipv4_addr.sin_addr.s_addr = htonl(INADDR_ANY);
				}
			}
			else
			{
				if (1 == inet_pton(family, addr_str.c_str(), &listenAddr.ipv6_addr.sin6_addr))
				{
					listenAddr.config_af |= LISTEN_FAMILY_6;
					listenAddr.ipv6_addr.sin6_family + AF_INET6;
					listenAddr.ipv6_addr.sin6_port = htons(port);
				}
			}
		}
	}
	else
	{
		listenAddr.config_af |= LISTEN_FAMILY_4;
		listenAddr.ipv4_addr.sin_family = AF_INET;
		inet_pton(AF_INET, "0.0.0.0", &listenAddr.ipv4_addr.sin_addr);
		listenAddr.ipv4_addr.sin_port = htons(80);
	}

	key_name = "webRoot";
	if ((json_doc.HasMember(key_name.c_str())) && (json_doc[key_name.c_str()].IsString()))
	{
		this->webRoot = json_doc[key_name.c_str()].GetString();
	}
	else
	{
		this->webRoot = "/var/www-data";
	}

	key_name = "tlsConfig";
	if ((json_doc.HasMember(key_name.c_str())) && (json_doc[key_name.c_str()].IsObject()))
	{
		auto var = json_doc[key_name.c_str()].GetObject();
		if (var.HasMember("tlsCertFile"))
		{
			this->tlsConfig.set_cert_file(var["tlsCertFile"].GetString());
		}
		if (var.HasMember("tlsKeyFile"))
		{
			this->tlsConfig.set_key_file(var["tlsKeyFile"].GetString());
		}
	}

	return 0;
}
