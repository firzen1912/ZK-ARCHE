/*
 * auth_wire.c — 24-byte header, TLV codec, Hello, negotiation.
 */

#include "auth/auth_wire.h"

#include <string.h>

static uint16_t load_u16_le(const uint8_t *p)
{
    return (uint16_t)((uint16_t)p[0] | (uint16_t)((uint16_t)p[1] << 8));
}

/* ---- Header ---- */

auth_err_t auth_header_encode(
    const auth_header_t *hdr,
    uint8_t *buf, size_t buf_len, size_t *written)
{
    if (!hdr || !buf) return AUTH_ERR_INVALID_ARGUMENT;
    if (buf_len < AUTH_HEADER_LEN) return AUTH_ERR_BUFFER_TOO_SMALL;
    buf[0] = hdr->version;
    buf[1] = hdr->pkt_type;
    buf[2] = (uint8_t)(hdr->flags       & 0xff);
    buf[3] = (uint8_t)((hdr->flags >> 8) & 0xff);
    memcpy(buf + 4, hdr->session_id, AUTH_SESSION_ID_LEN);
    buf[20] = (uint8_t)(hdr->seq        & 0xff);
    buf[21] = (uint8_t)((hdr->seq >>  8) & 0xff);
    buf[22] = (uint8_t)((hdr->seq >> 16) & 0xff);
    buf[23] = (uint8_t)((hdr->seq >> 24) & 0xff);
    if (written) *written = AUTH_HEADER_LEN;
    return AUTH_OK;
}

auth_err_t auth_header_decode(
    auth_header_t *hdr,
    const uint8_t *buf, size_t buf_len,
    const uint8_t **payload_out, size_t *payload_len_out)
{
    if (!hdr || !buf) return AUTH_ERR_INVALID_ARGUMENT;
    if (buf_len < AUTH_HEADER_LEN) return AUTH_ERR_PAYLOAD_TOO_SHORT;

    hdr->version  = buf[0];
    hdr->pkt_type = buf[1];
    hdr->flags    = load_u16_le(buf + 2);
    memcpy(hdr->session_id, buf + 4, AUTH_SESSION_ID_LEN);
    hdr->seq = (uint32_t)buf[20]
             | ((uint32_t)buf[21] <<  8)
             | ((uint32_t)buf[22] << 16)
             | ((uint32_t)buf[23] << 24);

    if (hdr->version < AUTH_MIN_SUPPORTED_VERSION) {
        return AUTH_ERR_UNSUPPORTED_VERSION;
    }
    if (payload_out)     *payload_out     = buf + AUTH_HEADER_LEN;
    if (payload_len_out) *payload_len_out = buf_len - AUTH_HEADER_LEN;
    return AUTH_OK;
}

/* ---- Packets ---- */

auth_err_t auth_packet_build(
    uint8_t pkt_type,
    const uint8_t session_id[AUTH_SESSION_ID_LEN],
    uint32_t seq,
    const uint8_t *payload, size_t payload_len,
    uint8_t *buf, size_t buf_len, size_t *packet_len_out)
{
    if (!buf || (!payload && payload_len > 0) || !session_id)
        return AUTH_ERR_INVALID_ARGUMENT;
    if (payload_len > AUTH_MAX_PAYLOAD)
        return AUTH_ERR_PAYLOAD_TOO_LARGE;
    size_t total = AUTH_HEADER_LEN + payload_len;
    if (buf_len < total) return AUTH_ERR_BUFFER_TOO_SMALL;

    auth_header_t h = {
        .version    = AUTH_PROTOCOL_VERSION,
        .pkt_type   = pkt_type,
        .flags      = AUTH_FLAG_NONE,
        .seq        = seq,
    };
    memcpy(h.session_id, session_id, AUTH_SESSION_ID_LEN);
    auth_err_t err = auth_header_encode(&h, buf, buf_len, NULL);
    if (err) return err;
    if (payload_len) memcpy(buf + AUTH_HEADER_LEN, payload, payload_len);
    if (packet_len_out) *packet_len_out = total;
    return AUTH_OK;
}

auth_err_t auth_packet_build_error(
    const uint8_t session_id[AUTH_SESSION_ID_LEN],
    uint32_t seq,
    auth_err_t code,
    const char *msg,
    uint8_t *buf, size_t buf_len, size_t *packet_len_out)
{
    size_t msg_len = msg ? strlen(msg) : 0;
    if (msg_len > AUTH_MAX_PAYLOAD - 2)
        return AUTH_ERR_PAYLOAD_TOO_LARGE;
    uint8_t payload[AUTH_MAX_PAYLOAD];
    uint16_t code_u16 = (uint16_t)code;
    payload[0] = (uint8_t)(code_u16       & 0xff);
    payload[1] = (uint8_t)((code_u16 >> 8) & 0xff);
    if (msg_len) memcpy(payload + 2, msg, msg_len);
    return auth_packet_build(
        AUTH_PKT_ERROR, session_id, seq,
        payload, 2 + msg_len,
        buf, buf_len, packet_len_out);
}

auth_err_t auth_packet_parse_error(
    const uint8_t *payload, size_t payload_len,
    auth_err_t *code_out,
    const char **msg_out, size_t *msg_len_out)
{
    if (!payload || payload_len < 2) return AUTH_ERR_MALFORMED_PACKET;
    uint16_t code = load_u16_le(payload);
    if (code_out)    *code_out    = (auth_err_t)code;
    if (msg_out)     *msg_out     = (const char *)(payload + 2);
    if (msg_len_out) *msg_len_out = payload_len - 2;
    return AUTH_OK;
}

/* ---- TLV ---- */

auth_err_t auth_tlv_write(
    uint8_t *buf, size_t buf_len, size_t *offset,
    uint16_t tag, const uint8_t *value, size_t value_len)
{
    if (!buf || !offset) return AUTH_ERR_INVALID_ARGUMENT;
    if (value_len > 0xFFFFu) return AUTH_ERR_PAYLOAD_TOO_LARGE;
    size_t need = *offset + 4 + value_len;
    if (need > buf_len) return AUTH_ERR_BUFFER_TOO_SMALL;

    buf[*offset + 0] = (uint8_t)(tag        & 0xff);
    buf[*offset + 1] = (uint8_t)((tag >> 8) & 0xff);
    uint16_t len16 = (uint16_t)value_len;
    buf[*offset + 2] = (uint8_t)(len16        & 0xff);
    buf[*offset + 3] = (uint8_t)((len16 >> 8) & 0xff);
    if (value_len) memcpy(buf + *offset + 4, value, value_len);
    *offset += 4 + value_len;
    return AUTH_OK;
}

auth_err_t auth_tlv_read(
    const uint8_t *buf, size_t buf_len, size_t *offset,
    uint16_t *tag_out,
    const uint8_t **value_out, size_t *value_len_out)
{
    if (!buf || !offset) return AUTH_ERR_INVALID_ARGUMENT;
    if (*offset >= buf_len) return AUTH_ERR_PAYLOAD_TOO_SHORT;
    if (buf_len - *offset < 4) return AUTH_ERR_MALFORMED_PACKET;

    uint16_t tag = load_u16_le(buf + *offset);
    uint16_t len = load_u16_le(buf + *offset + 2);
    if (4 + (size_t)len > buf_len - *offset) return AUTH_ERR_MALFORMED_PACKET;
    if (tag_out)      *tag_out      = tag;
    if (value_out)    *value_out    = buf + *offset + 4;
    if (value_len_out) *value_len_out = len;
    *offset += 4 + len;
    return AUTH_OK;
}

/* ---- Hello ---- */

auth_err_t auth_hello_encode(
    const auth_hello_t *h,
    uint8_t *buf, size_t buf_len, size_t *written)
{
    if (!h || !buf) return AUTH_ERR_INVALID_ARGUMENT;
    if (h->n_suites > sizeof h->suites / sizeof h->suites[0])
        return AUTH_ERR_INVALID_ARGUMENT;

    size_t need = 1 + 2 + 2 * h->n_suites + 8;
    if (buf_len < need) return AUTH_ERR_BUFFER_TOO_SMALL;
    size_t off = 0;
    buf[off++] = h->version;
    uint16_t nsu = (uint16_t)h->n_suites;
    buf[off++] = (uint8_t)(nsu       & 0xff);
    buf[off++] = (uint8_t)((nsu >> 8) & 0xff);
    for (size_t i = 0; i < h->n_suites; ++i) {
        auth_suite_t s = h->suites[i];
        buf[off++] = (uint8_t)(s       & 0xff);
        buf[off++] = (uint8_t)((s >> 8) & 0xff);
    }
    for (size_t i = 0; i < 8; ++i) {
        buf[off++] = (uint8_t)((h->caps >> (8 * i)) & 0xff);
    }

    /* Extensions: MIN_VERSION always; MTU_HINT, VENDOR_ID, DEVICE_MODEL if set. */
    auth_err_t err;
    uint8_t mvb = h->min_version;
    err = auth_tlv_write(buf, buf_len, &off,
                             AUTH_TLV_MIN_VERSION, &mvb, 1);
    if (err) return err;
    if (h->has_mtu_hint) {
        uint8_t mtu_le[2];
        mtu_le[0] = (uint8_t)(h->mtu_hint       & 0xff);
        mtu_le[1] = (uint8_t)((h->mtu_hint >> 8) & 0xff);
        err = auth_tlv_write(buf, buf_len, &off,
                                 AUTH_TLV_MTU_HINT, mtu_le, 2);
        if (err) return err;
    }
    if (h->vendor_id_len) {
        err = auth_tlv_write(buf, buf_len, &off,
                                 AUTH_TLV_VENDOR_ID,
                                 h->vendor_id, h->vendor_id_len);
        if (err) return err;
    }
    if (h->device_model_len) {
        err = auth_tlv_write(buf, buf_len, &off,
                                 AUTH_TLV_DEVICE_MODEL,
                                 h->device_model, h->device_model_len);
        if (err) return err;
    }
    if (written) *written = off;
    return AUTH_OK;
}

auth_err_t auth_hello_decode(
    auth_hello_t *h,
    const uint8_t *buf, size_t buf_len)
{
    if (!h || !buf) return AUTH_ERR_INVALID_ARGUMENT;
    memset(h, 0, sizeof *h);

    if (buf_len < 3) return AUTH_ERR_PAYLOAD_TOO_SHORT;
    size_t off = 0;
    h->version = buf[off++];
    uint16_t nsu = load_u16_le(buf + off);
    off += 2;
    if (nsu > sizeof h->suites / sizeof h->suites[0])
        return AUTH_ERR_MALFORMED_PACKET;
    if (buf_len - off < (size_t)nsu * 2 + 8)
        return AUTH_ERR_PAYLOAD_TOO_SHORT;

    h->n_suites = nsu;
    for (size_t i = 0; i < nsu; ++i) {
        h->suites[i] = load_u16_le(buf + off);
        off += 2;
    }
    h->caps = 0;
    for (size_t i = 0; i < 8; ++i) {
        h->caps |= (uint64_t)buf[off + i] << (8 * i);
    }
    off += 8;

    /* Default min_version = version unless MIN_VERSION TLV says otherwise. */
    h->min_version = h->version;

    /* Walk TLVs; ignore unknown tags per spec. */
    while (off < buf_len) {
        uint16_t tag;
        const uint8_t *value;
        size_t value_len;
        auth_err_t err = auth_tlv_read(buf, buf_len, &off,
                                               &tag, &value, &value_len);
        if (err == AUTH_ERR_PAYLOAD_TOO_SHORT) break;
        if (err) return err;
        switch (tag) {
        case AUTH_TLV_MIN_VERSION:
            if (value_len != 1) return AUTH_ERR_MALFORMED_PACKET;
            h->min_version = value[0];
            break;
        case AUTH_TLV_MTU_HINT:
            if (value_len != 2) return AUTH_ERR_MALFORMED_PACKET;
            h->has_mtu_hint = 1;
            h->mtu_hint = load_u16_le(value);
            break;
        case AUTH_TLV_VENDOR_ID:
            if (value_len > sizeof h->vendor_id) break;  /* silently truncate */
            memcpy(h->vendor_id, value, value_len);
            h->vendor_id_len = value_len;
            break;
        case AUTH_TLV_DEVICE_MODEL:
            if (value_len > sizeof h->device_model) break;
            memcpy(h->device_model, value, value_len);
            h->device_model_len = value_len;
            break;
        default:
            /* Unknown: ignore. */
            break;
        }
    }
    return AUTH_OK;
}

/* ---- Negotiation ---- */

auth_err_t auth_negotiate(
    auth_negotiated_t *out,
    uint8_t local_version, uint8_t local_min,
    const auth_suite_t *local_suites, size_t n_local,
    auth_caps_t local_caps,
    uint8_t peer_version, uint8_t peer_min,
    const auth_suite_t *peer_suites, size_t n_peer,
    auth_caps_t peer_caps)
{
    if (!out) return AUTH_ERR_INVALID_ARGUMENT;
    uint8_t v = local_version < peer_version ? local_version : peer_version;
    uint8_t floor = local_min > peer_min ? local_min : peer_min;
    if (v < floor) return AUTH_ERR_UNSUPPORTED_VERSION;

    /* Pick first local suite the peer also lists. */
    auth_suite_t chosen = 0;
    int found = 0;
    for (size_t i = 0; i < n_local && !found; ++i) {
        for (size_t j = 0; j < n_peer; ++j) {
            if (local_suites[i] == peer_suites[j]) {
                chosen = local_suites[i];
                found = 1;
                break;
            }
        }
    }
    if (!found) return AUTH_ERR_UNSUPPORTED_SUITE;

    auth_caps_t caps = local_caps & peer_caps;
    if ((caps & AUTH_CAPS_BASELINE) != AUTH_CAPS_BASELINE) {
        return AUTH_ERR_CAP_MISMATCH;
    }

    out->version = v;
    out->suite   = chosen;
    out->caps    = caps;
    return AUTH_OK;
}
