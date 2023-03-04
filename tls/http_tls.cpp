#include <tls.h>
#include "log4cpp.hpp"

#include "http_tls.h"

logger *tls_logger = nullptr;

static void init_logger(const std::string &log_file_name)
{
#ifdef _DEBUG
	log_level level = log_level::TRACE;
#else
	log_level level = log_level::WARN;
#endif
	tls_logger = new logger(log_file_name, level);
	tls_logger->set_log_prefix("http_tls");
}

http_tls::http_tls()
{
	tlsConfig = nullptr;
	tlsContext = nullptr;
}

int http_tls::http_tls_init(const std::string &cert, const std::string &key)
{
	init_logger("/var/log//var/log/lwhttpd.log");
	int var = tls_init();
	if (var == -1)
	{
		tls_logger->log_fatal("TLS initial failed!");
		return -1;
	}
	tlsConfig = tls_config_new();
	if (tlsConfig == nullptr)
	{
		tls_logger->log_fatal("TLS config failed!");
		return -1;
	}
	tls_config_set_protocols(tlsConfig, TLS_PROTOCOL_TLSv1_2|TLS_PROTOCOL_TLSv1_3);
	var = tls_config_set_ca_file(tlsConfig, cert.c_str());
	if (var == -1)
	{
		tls_logger->log_fatal("tls set ca file failed! %s", tls_config_error(tlsConfig));
		tls_config_free(tlsConfig);
		tlsConfig = nullptr;
		return -1;
	}
	tlsContext = tls_server();
	if (tlsContext == nullptr)
	{
		tls_logger->log_fatal("TLS context create failed!");
		tls_config_free(tlsConfig);
		tlsConfig = nullptr;
		return -1;
	}
	var = tls_configure(tlsContext, tlsConfig);
	if (var == -1)
	{
		tls_logger->log_fatal("tls configure failed! %s", tls_config_error(tlsConfig));
		tls_close(tlsContext);
		tls_free(tlsContext);
		tlsContext = nullptr;
		tls_config_free(tlsConfig);
		tlsConfig = nullptr;
		return -1;
	}
	return 0;
}

http_tls::~http_tls()
{
	if (tlsContext != nullptr)
	{
		tls_close(tlsContext);
		tls_free(tlsContext);
		tlsContext = nullptr;
	}
	if (tlsConfig != nullptr)
	{
		tls_config_free(tlsConfig);
		tlsConfig = nullptr;
	}
}

int http_tls::http_tls_write(const char *buf, size_t len)
{
	int var = tls_accept_fds(tlsContext,)
	return 0;
}
