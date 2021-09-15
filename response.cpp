#include <string>
#include <sys/socket.h>
#include "response.h"

#ifdef _DEBUG

#include <iostream>

#endif

const char *HTTP_STATUS[HTTP_STATUS_MAX] = {[HTTP_STATUS_400] = "400 Bad request", [HTTP_STATUS_401] = "401 Access denied", [HTTP_STATUS_403] = "403 Forbidden", [HTTP_STATUS_404] = "404 Not found", [HTTP_STATUS_500] = "500 Internal server error", [HTTP_STATUS_501] = "501 not implemented", [HTTP_STATUS_502] = "502 Bad Gateway", [HTTP_STATUS_503] = "503 Service unavailable"};

inline std::string assemble(HTTP_STATUS_CODE status_code)
{
	std::string response;
	response.append("HTTP/1.0 ");
	response.append(HTTP_STATUS[status_code]);
	response.append("\r\n");
	response.append("Content-type: text/html\r\n");
	response.append("\r\n");
	response.append("<html><head><title>");
	response.append(HTTP_STATUS[status_code]);
	response.append("</title></head><body><center><h1>");
	response.append(HTTP_STATUS[status_code]);
	response.append("</h1></center><hr><center>tinyhttpd/0.1.0</center></body></html>");
#ifdef _DEBUG
	std::cout << "http header:\n" << response << std::endl;
#endif
	return response;
}

/* 400 Bad request */
long bad_request(int fd)
{
	std::string response = assemble(HTTP_STATUS_400);
	return send(fd, response.c_str(), response.length(), 0);
}

/* 401 Access denied */
long access_denied(int fd)
{
	std::string response = assemble(HTTP_STATUS_401);
	return send(fd, response.c_str(), response.length(), 0);
}

/* 403 Forbidden */
long forbidden(int fd)
{
	std::string response = assemble(HTTP_STATUS_403);
	return send(fd, response.c_str(), response.length(), 0);
}

/* 404 Not found */
long not_found(int fd)
{
	std::string response = assemble(HTTP_STATUS_404);
	return send(fd, response.c_str(), response.length(), 0);
}

/* 500 Internal server error */
long internal_server_error(int fd)
{
	std::string response = assemble(HTTP_STATUS_500);
	return send(fd, response.c_str(), response.length(), 0);
}

/* 501 not implemented */
long not_implemented(int fd)
{
	std::string response = assemble(HTTP_STATUS_501);
	return send(fd, response.c_str(), response.length(), 0);
}

/* 502 Bad Gateway */
long bad_gateway(int fd)
{
	std::string response = assemble(HTTP_STATUS_502);
	return send(fd, response.c_str(), response.length(), 0);
}

/* 503 Service unavailable */
long service_unavailable(int fd)
{
	std::string response = assemble(HTTP_STATUS_503);
	return send(fd, response.c_str(), response.length(), 0);
}
