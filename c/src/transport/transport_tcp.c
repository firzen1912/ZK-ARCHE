/*
 * auth_transport_tcp.c — TCP bindings with 4-byte little-endian
 * length-prefix framing.
 *
 * Matches the Rust server framing: each application packet is
 * preceded by its length as a u32 in little-endian byte order. On the
 * wire:
 *
 *   [u32 little-endian length] [length bytes of application packet]
 *
 * That way one TCP connection can carry multiple packets back-to-back
 * (which SETUP and AUTH each require) without relying on read boundaries.
 */

#define _POSIX_C_SOURCE 200112L
#define _DEFAULT_SOURCE

#include "auth/auth_transport.h"

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/tcp.h>
#include <poll.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

/* ---- Shared read/write helpers ---- */

static auth_err_t write_all(int fd, const uint8_t *buf, size_t len)
{
    size_t off = 0;
    while (off < len) {
        ssize_t n;
        do {
            n = send(fd, buf + off, len - off, MSG_NOSIGNAL);
        } while (n < 0 && errno == EINTR);
        if (n <= 0) return AUTH_ERR_IO;
        off += (size_t)n;
    }
    return AUTH_OK;
}

/* Read exactly `len` bytes into `buf`. Returns:
 *   AUTH_OK on success
 *   AUTH_ERR_IO on clean EOF before any byte was read
 *   AUTH_ERR_MALFORMED_PACKET on EOF mid-frame
 *   AUTH_ERR_TIMEOUT on poll timeout
 */
static auth_err_t read_all(int fd, uint8_t *buf, size_t len,
                               int timeout_ms, int *got_any_out)
{
    size_t off = 0;
    int got_any = 0;
    while (off < len) {
        if (timeout_ms >= 0) {
            struct pollfd pfd = { .fd = fd, .events = POLLIN };
            int pr;
            do { pr = poll(&pfd, 1, timeout_ms); } while (pr < 0 && errno == EINTR);
            if (pr == 0) {
                if (got_any_out) *got_any_out = got_any;
                return AUTH_ERR_TIMEOUT;
            }
            if (pr < 0) return AUTH_ERR_IO;
        }
        ssize_t n;
        do { n = recv(fd, buf + off, len - off, 0); } while (n < 0 && errno == EINTR);
        if (n == 0) {
            if (got_any_out) *got_any_out = got_any;
            return got_any ? AUTH_ERR_MALFORMED_PACKET : AUTH_ERR_IO;
        }
        if (n < 0) return AUTH_ERR_IO;
        got_any = 1;
        off += (size_t)n;
    }
    if (got_any_out) *got_any_out = got_any;
    return AUTH_OK;
}

/* ---- TCP listener ---- */

auth_err_t auth_tcp_listener_bind(
    auth_tcp_listener_t *l, const char *bind_addr, int backlog)
{
    if (!l || !bind_addr) return AUTH_ERR_INVALID_ARGUMENT;
    auth_addr_t a;
    auth_err_t err = auth_addr_parse(&a, bind_addr);
    if (err) return err;

    int fd = socket(a.sa.ss_family, SOCK_STREAM, IPPROTO_TCP);
    if (fd < 0) return AUTH_ERR_IO;

    int one = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof one);

    if (bind(fd, (struct sockaddr *)&a.sa, a.sa_len) != 0) {
        close(fd); return AUTH_ERR_IO;
    }
    if (listen(fd, backlog > 0 ? backlog : 64) != 0) {
        close(fd); return AUTH_ERR_IO;
    }
    l->fd = fd;
    return AUTH_OK;
}

void auth_tcp_listener_close(auth_tcp_listener_t *l)
{
    if (l && l->fd >= 0) { close(l->fd); l->fd = -1; }
}

auth_err_t auth_tcp_listener_local_addr(
    const auth_tcp_listener_t *l, auth_addr_t *out)
{
    if (!l || l->fd < 0 || !out) return AUTH_ERR_INVALID_ARGUMENT;
    memset(out, 0, sizeof *out);
    out->sa_len = sizeof out->sa;
    if (getsockname(l->fd, (struct sockaddr *)&out->sa, &out->sa_len) != 0)
        return AUTH_ERR_IO;
    return AUTH_OK;
}

auth_err_t auth_tcp_listener_accept(
    auth_tcp_listener_t *l, int *peer_fd, auth_addr_t *peer_addr)
{
    if (!l || l->fd < 0 || !peer_fd) return AUTH_ERR_INVALID_ARGUMENT;
    auth_addr_t a;
    memset(&a, 0, sizeof a);
    a.sa_len = sizeof a.sa;

    int cfd;
    do {
        cfd = accept(l->fd, (struct sockaddr *)&a.sa, &a.sa_len);
    } while (cfd < 0 && errno == EINTR);
    if (cfd < 0) return AUTH_ERR_IO;

    /* Disable Nagle; our packets are small and we always send in one shot. */
    int one = 1;
    setsockopt(cfd, IPPROTO_TCP, TCP_NODELAY, &one, sizeof one);

    *peer_fd = cfd;
    if (peer_addr) *peer_addr = a;
    return AUTH_OK;
}

/* ---- TCP connection ---- */

auth_err_t auth_tcp_connect(
    auth_tcp_conn_t *c, const char *server_addr)
{
    if (!c || !server_addr) return AUTH_ERR_INVALID_ARGUMENT;
    auth_addr_t a;
    auth_err_t err = auth_addr_parse(&a, server_addr);
    if (err) return err;

    int fd = socket(a.sa.ss_family, SOCK_STREAM, IPPROTO_TCP);
    if (fd < 0) return AUTH_ERR_IO;

    if (connect(fd, (struct sockaddr *)&a.sa, a.sa_len) != 0) {
        close(fd); return AUTH_ERR_IO;
    }
    int one = 1;
    setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &one, sizeof one);
    c->fd = fd;
    return AUTH_OK;
}

void auth_tcp_conn_wrap(auth_tcp_conn_t *c, int fd)
{
    if (c) c->fd = fd;
}

void auth_tcp_conn_close(auth_tcp_conn_t *c)
{
    if (c && c->fd >= 0) { close(c->fd); c->fd = -1; }
}

/* ---- Length-prefix framing ---- */

auth_err_t auth_tcp_send(
    auth_tcp_conn_t *c, const uint8_t *buf, size_t buf_len)
{
    if (!c || c->fd < 0 || !buf) return AUTH_ERR_INVALID_ARGUMENT;
    if (buf_len > AUTH_MAX_DATAGRAM) return AUTH_ERR_PAYLOAD_TOO_LARGE;

    uint8_t hdr[4];
    uint32_t n = (uint32_t)buf_len;
    hdr[0] = (uint8_t)( n        & 0xff);
    hdr[1] = (uint8_t)((n >>  8) & 0xff);
    hdr[2] = (uint8_t)((n >> 16) & 0xff);
    hdr[3] = (uint8_t)((n >> 24) & 0xff);

    auth_err_t err = write_all(c->fd, hdr, 4);
    if (err) return err;
    return write_all(c->fd, buf, buf_len);
}

auth_err_t auth_tcp_recv(
    auth_tcp_conn_t *c, uint8_t *buf, size_t buf_cap, size_t *received_len,
    int timeout_ms)
{
    if (!c || c->fd < 0 || !buf || !received_len)
        return AUTH_ERR_INVALID_ARGUMENT;

    uint8_t hdr[4];
    int got_any = 0;
    auth_err_t err = read_all(c->fd, hdr, 4, timeout_ms, &got_any);
    if (err) return err;

    uint32_t len = (uint32_t)hdr[0]
                 | ((uint32_t)hdr[1] <<  8)
                 | ((uint32_t)hdr[2] << 16)
                 | ((uint32_t)hdr[3] << 24);
    if (len > AUTH_MAX_DATAGRAM) return AUTH_ERR_PAYLOAD_TOO_LARGE;
    if (len > buf_cap) return AUTH_ERR_BUFFER_TOO_SMALL;

    err = read_all(c->fd, buf, len, timeout_ms, NULL);
    if (err) return err;
    *received_len = len;
    return AUTH_OK;
}
