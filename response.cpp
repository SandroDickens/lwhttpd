#include <string>
#include <sys/socket.h>
#include "response.hpp"

#ifdef _DEBUG

#include <iostream>

#endif

const char *HTTP_STATUS[HTTP_STATUS_MAX] = {
		[HTTP_STATUS_400] = "400 Bad request",
		[HTTP_STATUS_401] = "401 Access denied",
		[HTTP_STATUS_403] = "403 Forbidden",
		[HTTP_STATUS_404] = "404 Not found",
		[HTTP_STATUS_500] = "500 Internal server error",
		[HTTP_STATUS_501] = "501 not implemented",
		[HTTP_STATUS_502] = "502 Bad Gateway",
		[HTTP_STATUS_503] = "503 Service unavailable"
};

inline std::string assemble(HTTP_STATUS_CODE status_code, const std::string &path)
{
	std::string http_headers;
	http_headers.append("HTTP/1.0 ");
	http_headers.append(HTTP_STATUS[status_code]);
	http_headers.append("\r\n");
	http_headers.append("Server: minorhttpd/0.0.1\r\n");
	http_headers.append("Content-Type: text/html; charset=utf8\r\n");

	std::string http_body;
	http_body.append("<html><head><title>");
	http_body.append(HTTP_STATUS[status_code]);
	http_body.append("</title></head><body><center><h1>");
	http_body.append(HTTP_STATUS[status_code]);
	http_body.append("</h1><h2>Path: " + path + "</h2>");
	http_body.append("</center><hr><center>minorhttpd/0.1.0</center></body></html>\r\n");
	unsigned long content_length = http_body.length();
	http_headers.append("Content-Length: " + std::to_string(content_length) + "\r\n\r\n");
#ifdef _DEBUG
	std::cout << "HTTP Header:\n" << http_headers;
	std::cout << "HTTP Body:\n" + http_body;
#endif
	return http_headers + http_body;
}

/* 400 Bad request */
long bad_request(int fd, const std::string &path)
{
	std::string response = assemble(HTTP_STATUS_400, path);
	return send(fd, response.c_str(), response.length(), 0);
}

/* 401 Access denied */
long access_denied(int fd, const std::string &path)
{
	std::string response = assemble(HTTP_STATUS_401, path);
	return send(fd, response.c_str(), response.length(), 0);
}

/* 403 Forbidden */
long forbidden(int fd, const std::string &path)
{
	std::string response = assemble(HTTP_STATUS_403, path);
	return send(fd, response.c_str(), response.length(), 0);
}

/* 404 Not found */
long not_found(int fd, const std::string &path)
{
	std::string response = assemble(HTTP_STATUS_404, path);
	return send(fd, response.c_str(), response.length(), 0);
}

/* 500 Internal server error */
long internal_server_error(int fd, const std::string &path)
{
	std::string response = assemble(HTTP_STATUS_500, path);
	return send(fd, response.c_str(), response.length(), 0);
}

/* 501 not implemented */
long not_implemented(int fd, const std::string &path)
{
	std::string response = assemble(HTTP_STATUS_501, path);
	return send(fd, response.c_str(), response.length(), 0);
}

/* 502 Bad Gateway */
long bad_gateway(int fd, const std::string &path)
{
	std::string response = assemble(HTTP_STATUS_502, path);
	return send(fd, response.c_str(), response.length(), 0);
}

/* 503 Service unavailable */
long service_unavailable(int fd, const std::string &path)
{
	std::string response = assemble(HTTP_STATUS_503, path);
	return send(fd, response.c_str(), response.length(), 0);
}
