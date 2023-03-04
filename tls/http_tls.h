#ifndef LWHTTPD_HTTP_TLS_H
#define LWHTTPD_HTTP_TLS_H


class http_tls
{
public:
	http_tls();

	int http_tls_init(const std::string &cert, const std::string &key);

	virtual ~http_tls();

	int http_tls_write(const char *buf, size_t len);

	int http_tls_read(char *buf, size_t len);

private:
	tls_config *tlsConfig;
	tls *tlsContext;
};

#endif //LWHTTPD_HTTP_TLS_H
