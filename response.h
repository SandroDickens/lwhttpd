#ifndef TINYHTTPD_RESPONSE_H
#define TINYHTTPD_RESPONSE_H

enum HTTP_STATUS_CODE
{
	HTTP_STATUS_400,
	HTTP_STATUS_401,
	HTTP_STATUS_403,
	HTTP_STATUS_404,
	HTTP_STATUS_500,
	HTTP_STATUS_501,
	HTTP_STATUS_502,
	HTTP_STATUS_503,
	HTTP_STATUS_MAX
};

/* 400 Bad request */
long bad_request(int fd);

/* 401 Access denied */
long access_denied(int fd);

/* 403 Forbidden */
long forbidden(int fd);

/* 404 Not found */
long not_found(int fd);

/* 500 Internal server error */
long internal_server_error(int fd);

/* 501 not implemented */
long not_implemented(int fd);

/* 502 Bad Gateway */
long bad_gateway(int fd);

/* 503 Service unavailable */
long service_unavailable(int fd);

#endif //TINYHTTPD_RESPONSE_H
