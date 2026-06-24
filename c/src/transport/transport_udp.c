/*
 * auth_transport_udp.c — UDP bindings over POSIX sockets.
 */

#define _POSIX_C_SOURCE 200112L
#define _DEFAULT_SOURCE

#include "auth/auth_transport.h"

#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <poll.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

auth_err_t auth_udp_bind(auth_udp_t *u, const char *bind_addr)
{
    if (!u || !bind_addr) return AUTH_ERR_INVALID_ARGUMENT;
    auth_addr_t a;
    auth_err_t err = auth_addr_parse(&a, bind_addr);
    if (err) return err;

    int fd = socket(a.sa.ss_family, SOCK_DGRAM, IPPROTO_UDP);
    if (fd < 0) return AUTH_ERR_IO;

    int one = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof one);

    if (bind(fd, (struct sockaddr *)&a.sa, a.sa_len) != 0) {
        close(fd);
        return AUTH_ERR_IO;
    }
    u->fd = fd;
    return AUTH_OK;
}

auth_err_t auth_udp_connect(auth_udp_t *u, const char *server_addr)
{
    if (!u || !server_addr) return AUTH_ERR_INVALID_ARGUMENT;
    auth_addr_t a;
    auth_err_t err = auth_addr_parse(&a, server_addr);
    if (err) return err;

    int fd = socket(a.sa.ss_family, SOCK_DGRAM, IPPROTO_UDP);
    if (fd < 0) return AUTH_ERR_IO;

    if (connect(fd, (struct sockaddr *)&a.sa, a.sa_len) != 0) {
        close(fd);
        return AUTH_ERR_IO;
    }
    u->fd = fd;
    return AUTH_OK;
}

void auth_udp_close(auth_udp_t *u)
{
    if (u && u->fd >= 0) { close(u->fd); u->fd = -1; }
}

auth_err_t auth_udp_local_addr(
    const auth_udp_t *u, auth_addr_t *out)
{
    if (!u || u->fd < 0 || !out) return AUTH_ERR_INVALID_ARGUMENT;
    memset(out, 0, sizeof *out);
    out->sa_len = sizeof out->sa;
    if (getsockname(u->fd, (struct sockaddr *)&out->sa, &out->sa_len) != 0)
        return AUTH_ERR_IO;
    return AUTH_OK;
}

auth_err_t auth_udp_send(
    auth_udp_t *u,
    const auth_addr_t *peer,
    const uint8_t *buf, size_t buf_len)
{
    if (!u || u->fd < 0 || !buf) return AUTH_ERR_INVALID_ARGUMENT;
    if (buf_len > AUTH_MAX_DATAGRAM) return AUTH_ERR_PAYLOAD_TOO_LARGE;
    ssize_t n;
    if (peer && peer->sa_len) {
        n = sendto(u->fd, buf, buf_len, 0,
                   (const struct sockaddr *)&peer->sa, peer->sa_len);
    } else {
        n = send(u->fd, buf, buf_len, 0);
    }
    if (n < 0 || (size_t)n != buf_len) return AUTH_ERR_IO;
    return AUTH_OK;
}

auth_err_t auth_udp_recv(
    auth_udp_t *u,
    uint8_t *buf, size_t buf_cap, size_t *received_len,
    auth_addr_t *peer_out,
    int timeout_ms)
{
    if (!u || u->fd < 0 || !buf || !received_len)
        return AUTH_ERR_INVALID_ARGUMENT;

    if (timeout_ms >= 0) {
        struct pollfd pfd = { .fd = u->fd, .events = POLLIN };
        int pr;
        do { pr = poll(&pfd, 1, timeout_ms); } while (pr < 0 && errno == EINTR);
        if (pr == 0) return AUTH_ERR_TIMEOUT;
        if (pr < 0)  return AUTH_ERR_IO;
    }

    auth_addr_t tmp;
    memset(&tmp, 0, sizeof tmp);
    tmp.sa_len = sizeof tmp.sa;

    ssize_t n = recvfrom(u->fd, buf, buf_cap, 0,
                         (struct sockaddr *)&tmp.sa, &tmp.sa_len);
    if (n < 0) return AUTH_ERR_IO;
    *received_len = (size_t)n;
    if (peer_out) *peer_out = tmp;
    return AUTH_OK;
}
