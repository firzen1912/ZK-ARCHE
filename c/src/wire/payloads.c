/*
 * auth_payloads.c — encoder/decoder for each packet payload.
 *
 * Every function has a `written_out` that receives the exact number
 * of bytes emitted, allowing the caller to wrap the payload in a
 * framed packet with auth_packet_build().
 */

#include "auth/auth_payloads.h"

#include <string.h>

/* ---- Tiny read/write helpers ---- */

static int room_for(size_t off, size_t n, size_t cap) {
    return off + n <= cap;
}

static auth_err_t take_fixed(const uint8_t *buf, size_t buf_len,
                                 size_t *off, uint8_t *out, size_t n)
{
    if (!room_for(*off, n, buf_len)) return AUTH_ERR_PAYLOAD_TOO_SHORT;
    memcpy(out, buf + *off, n);
    *off += n;
    return AUTH_OK;
}

static auth_err_t put_fixed(uint8_t *buf, size_t buf_len,
                                size_t *off, const uint8_t *src, size_t n)
{
    if (!room_for(*off, n, buf_len)) return AUTH_ERR_BUFFER_TOO_SMALL;
    memcpy(buf + *off, src, n);
    *off += n;
    return AUTH_OK;
}

static auth_err_t reject_trailing(size_t off, size_t buf_len)
{
    return off == buf_len ? AUTH_OK : AUTH_ERR_MALFORMED_PACKET;
}

static uint16_t load_u16_le(const uint8_t *p)
{
    return (uint16_t)((uint16_t)p[0] | (uint16_t)((uint16_t)p[1] << 8));
}

/* ---- SETUP_1 ---- */

auth_err_t auth_setup1_encode(
    const auth_setup1_t *m,
    uint8_t *buf, size_t buf_len, size_t *written)
{
    if (!m || !buf) return AUTH_ERR_INVALID_ARGUMENT;
    if (m->pairing_token_len > AUTH_MAX_PAIRING_TOKEN)
        return AUTH_ERR_INVALID_ARGUMENT;
    size_t need = 1 + m->pairing_token_len + 32 + 32 + 32 + 32;
    if (buf_len < need) return AUTH_ERR_BUFFER_TOO_SMALL;
    size_t off = 0;
    buf[off++] = (uint8_t)m->pairing_token_len;
    if (m->pairing_token_len) {
        memcpy(buf + off, m->pairing_token, m->pairing_token_len);
        off += m->pairing_token_len;
    }
    memcpy(buf + off, m->device_id,       32); off += 32;
    memcpy(buf + off, m->device_pub,      32); off += 32;
    memcpy(buf + off, m->client_nonce,    32); off += 32;
    memcpy(buf + off, m->role_commitment, 32); off += 32;
    if (written) *written = off;
    return AUTH_OK;
}

auth_err_t auth_setup1_decode(
    auth_setup1_t *m,
    const uint8_t *buf, size_t buf_len)
{
    if (!m || !buf || buf_len < 1) return AUTH_ERR_PAYLOAD_TOO_SHORT;
    memset(m, 0, sizeof *m);
    size_t off = 0;
    uint8_t tlen = buf[off++];
    if (tlen > AUTH_MAX_PAIRING_TOKEN) return AUTH_ERR_MALFORMED_PACKET;
    if (buf_len - off < (size_t)tlen + 32 + 32 + 32 + 32) return AUTH_ERR_PAYLOAD_TOO_SHORT;
    if (tlen) {
        memcpy(m->pairing_token, buf + off, tlen);
        off += tlen;
    }
    m->pairing_token_len = tlen;
    auth_err_t e;
    if ((e = take_fixed(buf, buf_len, &off, m->device_id,       32))) return e;
    if ((e = take_fixed(buf, buf_len, &off, m->device_pub,      32))) return e;
    if ((e = take_fixed(buf, buf_len, &off, m->client_nonce,    32))) return e;
    if ((e = take_fixed(buf, buf_len, &off, m->role_commitment, 32))) return e;
    return reject_trailing(off, buf_len);
}

/* ---- SETUP_2 ---- */

auth_err_t auth_setup2_encode(
    const auth_setup2_t *m,
    uint8_t *buf, size_t buf_len, size_t *written)
{
    if (!m || !buf) return AUTH_ERR_INVALID_ARGUMENT;
    size_t need = 32 + 16 + 32 + 32 + 32;
    if (buf_len < need) return AUTH_ERR_BUFFER_TOO_SMALL;
    size_t off = 0;
    auth_err_t e;
    if ((e = put_fixed(buf, buf_len, &off, m->server_nonce,    32))) return e;
    if ((e = put_fixed(buf, buf_len, &off, m->setup_challenge, AUTH_SETUP_CHALLENGE_LEN))) return e;
    if ((e = put_fixed(buf, buf_len, &off, m->server_pub,      32))) return e;
    if ((e = put_fixed(buf, buf_len, &off, m->server_proof.a,  32))) return e;
    if ((e = put_fixed(buf, buf_len, &off, m->server_proof.s,  32))) return e;
    if (written) *written = off;
    return AUTH_OK;
}

auth_err_t auth_setup2_decode(
    auth_setup2_t *m,
    const uint8_t *buf, size_t buf_len)
{
    if (!m || !buf) return AUTH_ERR_INVALID_ARGUMENT;
    memset(m, 0, sizeof *m);
    size_t off = 0;
    auth_err_t e;
    if ((e = take_fixed(buf, buf_len, &off, m->server_nonce,    32))) return e;
    if ((e = take_fixed(buf, buf_len, &off, m->setup_challenge, AUTH_SETUP_CHALLENGE_LEN))) return e;
    if ((e = take_fixed(buf, buf_len, &off, m->server_pub,      32))) return e;
    if ((e = take_fixed(buf, buf_len, &off, m->server_proof.a,  32))) return e;
    if ((e = take_fixed(buf, buf_len, &off, m->server_proof.s,  32))) return e;
    return reject_trailing(off, buf_len);
}

/* ---- SETUP_3 ---- */

auth_err_t auth_setup3_encode(
    const auth_setup3_t *m,
    uint8_t *buf, size_t buf_len, size_t *written)
{
    if (!m || !buf) return AUTH_ERR_INVALID_ARGUMENT;
    if (buf_len < 64) return AUTH_ERR_BUFFER_TOO_SMALL;
    memcpy(buf,      m->client_proof.a, 32);
    memcpy(buf + 32, m->client_proof.s, 32);
    if (written) *written = 64;
    return AUTH_OK;
}

auth_err_t auth_setup3_decode(
    auth_setup3_t *m,
    const uint8_t *buf, size_t buf_len)
{
    if (!m || !buf) return AUTH_ERR_INVALID_ARGUMENT;
    if (buf_len < 64) return AUTH_ERR_PAYLOAD_TOO_SHORT;
    memcpy(m->client_proof.a, buf,      32);
    memcpy(m->client_proof.s, buf + 32, 32);
    return reject_trailing(64, buf_len);
}

/* ---- AUTH_1 ---- */

auth_err_t auth_auth1_encode(
    const auth_auth1_t *m,
    uint8_t *buf, size_t buf_len, size_t *written)
{
    if (!m || !buf) return AUTH_ERR_INVALID_ARGUMENT;
    if (m->n_branches > AUTH_MAX_ROLES) return AUTH_ERR_INVALID_ARGUMENT;
    size_t need = 32 * 8 + 2 + 96 * m->n_branches;
    if (buf_len < need) return AUTH_ERR_BUFFER_TOO_SMALL;
    size_t off = 0;
    auth_err_t e;
    if ((e = put_fixed(buf, buf_len, &off, m->pid,                32))) return e;
    if ((e = put_fixed(buf, buf_len, &off, m->client_proof.a,     32))) return e;
    if ((e = put_fixed(buf, buf_len, &off, m->client_proof.s,     32))) return e;
    if ((e = put_fixed(buf, buf_len, &off, m->nonce_c,            32))) return e;
    if ((e = put_fixed(buf, buf_len, &off, m->eph_c,              32))) return e;
    if ((e = put_fixed(buf, buf_len, &off, m->c_prime,            32))) return e;
    if ((e = put_fixed(buf, buf_len, &off, m->rerand_proof.a,     32))) return e;
    if ((e = put_fixed(buf, buf_len, &off, m->rerand_proof.s,     32))) return e;
    uint16_t nb = (uint16_t)m->n_branches;
    buf[off++] = (uint8_t)(nb       & 0xff);
    buf[off++] = (uint8_t)((nb >> 8) & 0xff);
    for (size_t i = 0; i < m->n_branches; ++i) {
        if ((e = put_fixed(buf, buf_len, &off, m->branches[i].a, 32))) return e;
        if ((e = put_fixed(buf, buf_len, &off, m->branches[i].c, 32))) return e;
        if ((e = put_fixed(buf, buf_len, &off, m->branches[i].s, 32))) return e;
    }
    if (written) *written = off;
    return AUTH_OK;
}

auth_err_t auth_auth1_decode(
    auth_auth1_t *m,
    const uint8_t *buf, size_t buf_len)
{
    if (!m || !buf) return AUTH_ERR_INVALID_ARGUMENT;
    memset(m, 0, sizeof *m);
    size_t off = 0;
    auth_err_t e;
    if ((e = take_fixed(buf, buf_len, &off, m->pid,            32))) return e;
    if ((e = take_fixed(buf, buf_len, &off, m->client_proof.a, 32))) return e;
    if ((e = take_fixed(buf, buf_len, &off, m->client_proof.s, 32))) return e;
    if ((e = take_fixed(buf, buf_len, &off, m->nonce_c,        32))) return e;
    if ((e = take_fixed(buf, buf_len, &off, m->eph_c,          32))) return e;
    if ((e = take_fixed(buf, buf_len, &off, m->c_prime,        32))) return e;
    if ((e = take_fixed(buf, buf_len, &off, m->rerand_proof.a, 32))) return e;
    if ((e = take_fixed(buf, buf_len, &off, m->rerand_proof.s, 32))) return e;
    if (buf_len - off < 2) return AUTH_ERR_PAYLOAD_TOO_SHORT;
    uint16_t nb = load_u16_le(buf + off);
    off += 2;
    if (nb > AUTH_MAX_ROLES) return AUTH_ERR_MALFORMED_PACKET;
    if (buf_len - off < (size_t)nb * 96) return AUTH_ERR_PAYLOAD_TOO_SHORT;
    m->n_branches = nb;
    for (size_t i = 0; i < nb; ++i) {
        if ((e = take_fixed(buf, buf_len, &off, m->branches[i].a, 32))) return e;
        if ((e = take_fixed(buf, buf_len, &off, m->branches[i].c, 32))) return e;
        if ((e = take_fixed(buf, buf_len, &off, m->branches[i].s, 32))) return e;
    }
    return reject_trailing(off, buf_len);
}

/* ---- AUTH_2 ---- */

auth_err_t auth_auth2_encode(
    const auth_auth2_t *m,
    uint8_t *buf, size_t buf_len, size_t *written)
{
    if (!m || !buf) return AUTH_ERR_INVALID_ARGUMENT;
    size_t need = 32 * 6;
    if (buf_len < need) return AUTH_ERR_BUFFER_TOO_SMALL;
    size_t off = 0;
    auth_err_t e;
    if ((e = put_fixed(buf, buf_len, &off, m->server_pub,       32))) return e;
    if ((e = put_fixed(buf, buf_len, &off, m->server_proof.a,   32))) return e;
    if ((e = put_fixed(buf, buf_len, &off, m->server_proof.s,   32))) return e;
    if ((e = put_fixed(buf, buf_len, &off, m->nonce_s,          32))) return e;
    if ((e = put_fixed(buf, buf_len, &off, m->eph_s,            32))) return e;
    if ((e = put_fixed(buf, buf_len, &off, m->tag_s,            32))) return e;
    if (written) *written = off;
    return AUTH_OK;
}

auth_err_t auth_auth2_decode(
    auth_auth2_t *m,
    const uint8_t *buf, size_t buf_len)
{
    if (!m || !buf) return AUTH_ERR_INVALID_ARGUMENT;
    memset(m, 0, sizeof *m);
    size_t off = 0;
    auth_err_t e;
    if ((e = take_fixed(buf, buf_len, &off, m->server_pub,       32))) return e;
    if ((e = take_fixed(buf, buf_len, &off, m->server_proof.a,   32))) return e;
    if ((e = take_fixed(buf, buf_len, &off, m->server_proof.s,   32))) return e;
    if ((e = take_fixed(buf, buf_len, &off, m->nonce_s,          32))) return e;
    if ((e = take_fixed(buf, buf_len, &off, m->eph_s,            32))) return e;
    if ((e = take_fixed(buf, buf_len, &off, m->tag_s,            32))) return e;
    return reject_trailing(off, buf_len);
}

/* ---- AUTH_3 ---- */

auth_err_t auth_auth3_encode(
    const auth_auth3_t *m,
    uint8_t *buf, size_t buf_len, size_t *written)
{
    if (!m || !buf) return AUTH_ERR_INVALID_ARGUMENT;
    if (buf_len < 32) return AUTH_ERR_BUFFER_TOO_SMALL;
    memcpy(buf, m->tag_c, 32);
    if (written) *written = 32;
    return AUTH_OK;
}

auth_err_t auth_auth3_decode(
    auth_auth3_t *m,
    const uint8_t *buf, size_t buf_len)
{
    if (!m || !buf) return AUTH_ERR_INVALID_ARGUMENT;
    if (buf_len < 32) return AUTH_ERR_PAYLOAD_TOO_SHORT;
    memcpy(m->tag_c, buf, 32);
    return reject_trailing(32, buf_len);
}

/* ---- ACK ---- */

auth_err_t auth_ack_encode(
    uint8_t *buf, size_t buf_len, size_t *written)
{
    if (!buf) return AUTH_ERR_INVALID_ARGUMENT;
    if (buf_len < 1) return AUTH_ERR_BUFFER_TOO_SMALL;
    buf[0] = AUTH_ACK_BYTE;
    if (written) *written = 1;
    return AUTH_OK;
}

auth_err_t auth_ack_decode(const uint8_t *buf, size_t buf_len)
{
    if (!buf || buf_len != 1 || buf[0] != AUTH_ACK_BYTE)
        return AUTH_ERR_MALFORMED_PACKET;
    return AUTH_OK;
}
