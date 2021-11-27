#ifndef TINYHTTPD_CONFIG_HPP
#define TINYHTTPD_CONFIG_HPP

#include <string>

const std::string CGI_ENV_LIST[] ={
	"HOME",
	"HTTP_ACCEPT_LANGUAGE",
	"HTTP_ACCEPT_ENCODING",
	"HTTP_ACCEPT",
	"HTTP_USER_AGENT",
	"HTTP_CONNECTION",
	"HTTP_HOST",
	"SERVER_NAME",
	"SERVER_PORT",
	"SERVER_ADDR",
	"REMOTE_PORT",
	"REMOTE_ADDR",
	"SERVER_SOFTWARE",
	"GATEWAY_INTERFACE",
	"REQUEST_SCHEME",
	"SERVER_PROTOCOL",
	"DOCUMENT_ROOT",
	"DOCUMENT_URI",
	"REQUEST_URI",
	"CONTENT_LENGTH",
	"CONTENT_TYPE",
	"REQUEST_METHOD",
	"QUERY_STRING",
	"REQUEST_TIME_FLOAT",
	"REQUEST_TIME"
};

class Config
{
public:
	int parseConfig(const std::string &conf_path);

private:
	std::string webroot;
	std::string http_host;
};


#endif //TINYHTTPD_CONFIG_HPP
