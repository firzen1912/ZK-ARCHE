/*
 * auth_transport_addr.c — parse and format host:port addresses.
 */

#define _POSIX_C_SOURCE 200112L
#define _DEFAULT_SOURCE

#include "auth/auth_transport.h"

#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>

auth_err_t auth_addr_parse(auth_addr_t *out, const char *hostport)
{
    if (!out || !hostport) return AUTH_ERR_INVALID_ARGUMENT;
    memset(out, 0, sizeof *out);

    /* Split into host and port. Supports "host:port" and "[v6]:port". */
    char host[256];
    char port[32];

    const char *p = hostport;
    if (*p == '[') {
        const char *end = strchr(p, ']');
        if (!end) return AUTH_ERR_INVALID_ARGUMENT;
        size_t n = (size_t)(end - p - 1);
        if (n >= sizeof host) return AUTH_ERR_INVALID_ARGUMENT;
        memcpy(host, p + 1, n);
        host[n] = 0;
        if (end[1] != ':') return AUTH_ERR_INVALID_ARGUMENT;
        snprintf(port, sizeof port, "%s", end + 2);
    } else {
        const char *colon = strrchr(p, ':');
        if (!colon) return AUTH_ERR_INVALID_ARGUMENT;
        size_t n = (size_t)(colon - p);
        if (n >= sizeof host) return AUTH_ERR_INVALID_ARGUMENT;
        memcpy(host, p, n);
        host[n] = 0;
        snprintf(port, sizeof port, "%s", colon + 1);
    }

    struct addrinfo hints;
    memset(&hints, 0, sizeof hints);
    hints.ai_family   = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM; /* flag-only; we copy the sockaddr */
    hints.ai_flags    = AI_NUMERICSERV | AI_PASSIVE;

    struct addrinfo *res = NULL;
    if (getaddrinfo(host, port, &hints, &res) != 0 || !res) {
        return AUTH_ERR_INVALID_ARGUMENT;
    }
    if (res->ai_addrlen > sizeof out->sa) {
        freeaddrinfo(res);
        return AUTH_ERR_INVALID_ARGUMENT;
    }
    memcpy(&out->sa, res->ai_addr, res->ai_addrlen);
    out->sa_len = (socklen_t)res->ai_addrlen;
    freeaddrinfo(res);
    return AUTH_OK;
}

void auth_addr_format(const auth_addr_t *a, char *buf, size_t cap)
{
    if (!buf || cap == 0) return;
    buf[0] = 0;
    if (!a || a->sa_len == 0) return;
    char host[NI_MAXHOST] = {0};
    char port[NI_MAXSERV] = {0};
    if (getnameinfo((const struct sockaddr *)&a->sa, a->sa_len,
                    host, sizeof host, port, sizeof port,
                    NI_NUMERICHOST | NI_NUMERICSERV) != 0) {
        snprintf(buf, cap, "<unknown>");
        return;
    }
    if (a->sa.ss_family == AF_INET6) {
        snprintf(buf, cap, "[%s]:%s", host, port);
    } else {
        snprintf(buf, cap, "%s:%s", host, port);
    }
}
