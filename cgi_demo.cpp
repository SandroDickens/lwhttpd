#include <iostream>
#include <string>


constexpr int CGI_ENV_COUNT = 24;

const std::string CGI_ENV[CGI_ENV_COUNT] = {
		"COMSPEC", "DOCUMENT_ROOT", "GATEWAY_INTERFACE",
		"HTTP_ACCEPT", "HTTP_ACCEPT_ENCODING",
		"HTTP_ACCEPT_LANGUAGE", "HTTP_CONNECTION",
		"HTTP_HOST", "HTTP_USER_AGENT", "PATH",
		"QUERY_STRING", "REMOTE_ADDR", "REMOTE_PORT",
		"REQUEST_METHOD", "REQUEST_URI", "SCRIPT_FILENAME",
		"SCRIPT_NAME", "SERVER_ADDR", "SERVER_ADMIN",
		"SERVER_NAME", "SERVER_PORT", "SERVER_PROTOCOL",
		"SERVER_SIGNATURE", "SERVER_SOFTWARE"
};

int main()
{
	std::string header;
	header.append("Content-type:text/html\r\n");

	std::string http_body;
	http_body.append(
			"<html>\r\n<head>\r\n<title>CGI Environment Variables</title>\r\n</head>\r\n<body>\r\n<table border = \"0\" cellspacing = \"2\">\r\n");

	for (const auto &cgi_env : CGI_ENV)
	{
		http_body.append("<tr><td>" + cgi_env + "</td><td>");

		// attempt to retrieve value of environment variable
		char *value = getenv(cgi_env.c_str());
		if (value != nullptr)
		{
			http_body.append(value);
		}
		else
		{
			http_body.append("N/A");
		}
		http_body.append("</td></tr>\r\n");
	}

	http_body.append("</table>\r\n");
	http_body.append("</body>\r\n");
	http_body.append("</html>\r\n");
	header.append("Content-Length: " + std::to_string(http_body.length()) + "\r\n\r\n");

	std::cout << header << http_body;

	return 0;
}
