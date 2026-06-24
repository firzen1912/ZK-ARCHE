/*
 * auth_transport.h — UDP and TCP bindings.
 *
 * The transport layer is a thin POSIX sockets wrapper. Each binding
 * exposes a fixed interface: bind/connect, send, recv, close. The
 * higher-level state machines (proto layer) are fully transport-agnostic
 * and just hand byte buffers to these functions.
 *
 * TCP framing: each application packet is prefixed with a 4-byte
 * big-endian length so multiple packets can share a connection and
 * the receiver can reassemble them without relying on UDP-style
 * datagram boundaries.
 */

#ifndef AUTH_TRANSPORT_H
#define AUTH_TRANSPORT_H

#include "iot_auth.h"

#include <netinet/in.h>
#include <sys/socket.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ---- Peer address (opaque wrapper around sockaddr_storage) ---- */

typedef struct auth_addr {
    struct sockaddr_storage sa;
    socklen_t               sa_len;
} auth_addr_t;

/* Parse "host:port" (IPv4 or [IPv6]:port). */
auth_err_t auth_addr_parse(auth_addr_t *out, const char *hostport);

/* Format a peer address as "host:port" into buf (no allocation). */
void auth_addr_format(const auth_addr_t *a, char *buf, size_t cap);

/* ---- UDP ---- */

typedef struct auth_udp {
    int fd;
} auth_udp_t;

/*
 * Bind a UDP socket to `bind_addr`. For a client, pass a host:port of
 * 0 (any) to let the kernel choose, or use auth_udp_connect() to
 * associate with a server for simpler send/recv.
 */
auth_err_t auth_udp_bind(auth_udp_t *u, const char *bind_addr);

/*
 * Create a connected UDP socket (can use send/recv without addr). Any
 * datagrams from other sources will be filtered by the kernel.
 */
auth_err_t auth_udp_connect(auth_udp_t *u, const char *server_addr);

void auth_udp_close(auth_udp_t *u);

/*
 * Get the local address the socket is bound to (useful after
 * bind-to-ephemeral-port).
 */
auth_err_t auth_udp_local_addr(const auth_udp_t *u,
                                       auth_addr_t *out);

/*
 * Send `buf` to `peer`. `peer` may be NULL if the socket is connected.
 */
auth_err_t auth_udp_send(
    auth_udp_t *u,
    const auth_addr_t *peer,
    const uint8_t *buf, size_t buf_len);

/*
 * Receive one datagram. Blocks up to `timeout_ms` (0 = no wait, -1 = block).
 * On success, *received_len is set and *peer_out is filled (if non-NULL).
 * Returns AUTH_ERR_TIMEOUT on timeout.
 */
auth_err_t auth_udp_recv(
    auth_udp_t *u,
    uint8_t *buf, size_t buf_cap, size_t *received_len,
    auth_addr_t *peer_out,
    int timeout_ms);

/* ---- TCP server (listener) ---- */

typedef struct auth_tcp_listener {
    int fd;
} auth_tcp_listener_t;

auth_err_t auth_tcp_listener_bind(
    auth_tcp_listener_t *l,
    const char *bind_addr,
    int backlog);

void auth_tcp_listener_close(auth_tcp_listener_t *l);

auth_err_t auth_tcp_listener_local_addr(
    const auth_tcp_listener_t *l, auth_addr_t *out);

/*
 * Block until a new connection arrives. On success:
 *   *peer_fd   -- accepted socket
 *   *peer_addr -- remote address
 * Caller owns peer_fd and must close it.
 */
auth_err_t auth_tcp_listener_accept(
    auth_tcp_listener_t *l,
    int *peer_fd, auth_addr_t *peer_addr);

/* ---- TCP client connection ---- */

typedef struct auth_tcp_conn {
    int fd;
} auth_tcp_conn_t;

auth_err_t auth_tcp_connect(
    auth_tcp_conn_t *c, const char *server_addr);

/* Wrap an already-accepted fd (e.g. from listener_accept). */
void auth_tcp_conn_wrap(auth_tcp_conn_t *c, int fd);

void auth_tcp_conn_close(auth_tcp_conn_t *c);

/*
 * Send one packet, length-prefixed (big-endian u32 | payload).
 * Uses MSG_NOSIGNAL to avoid SIGPIPE on peer close.
 */
auth_err_t auth_tcp_send(
    auth_tcp_conn_t *c,
    const uint8_t *buf, size_t buf_len);

/*
 * Receive one length-prefixed packet. Blocks until the full frame is
 * read, a timeout elapses, or the peer closes.
 *
 * Returns AUTH_ERR_IO on peer close (EOF) during the length prefix
 * (a clean end-of-stream). Returns AUTH_ERR_MALFORMED_PACKET on a
 * premature EOF in the middle of a frame.
 */
auth_err_t auth_tcp_recv(
    auth_tcp_conn_t *c,
    uint8_t *buf, size_t buf_cap, size_t *received_len,
    int timeout_ms);

#ifdef __cplusplus
}
#endif
#endif /* AUTH_TRANSPORT_H */
