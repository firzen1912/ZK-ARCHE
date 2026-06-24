/*
 * auth_wire.h — wire format: 24-byte header, packet types, TLV codec.
 *
 * This is the transport-neutral framing layer. The same bytes travel
 * unchanged across UDP, TCP, CoAP, or any other transport.
 *
 * Header (24 bytes):
 *   0    version       u8
 *   1    pkt_type      u8
 *   2    flags         u16 LE
 *   4    session_id    16 random bytes
 *  20    seq           u32 LE
 *
 * TLV: u16_LE tag | u16_LE len | `len` bytes value. Unknown tags MUST
 * be ignored (skip).
 */

#ifndef AUTH_WIRE_H
#define AUTH_WIRE_H

#include "iot_auth.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ---- Packet type codes ---- */

#define AUTH_PKT_HELLO        0x01u
#define AUTH_PKT_HELLO_REPLY  0x02u
#define AUTH_PKT_SETUP_1      0x11u
#define AUTH_PKT_SETUP_2      0x12u
#define AUTH_PKT_SETUP_3      0x13u
#define AUTH_PKT_SETUP_ACK    0x14u
#define AUTH_PKT_AUTH_1       0x21u
#define AUTH_PKT_AUTH_2       0x22u
#define AUTH_PKT_AUTH_3       0x23u
#define AUTH_PKT_AUTH_ACK     0x24u
#define AUTH_PKT_ERROR        0x7fu

/* ---- Flag bits ---- */

#define AUTH_FLAG_NONE        0x0000u
#define AUTH_FLAG_RETRANSMIT  0x0001u

/* ---- TLV tags ---- */

#define AUTH_TLV_MIN_VERSION   0x0001u
#define AUTH_TLV_SUITE_LIST    0x0002u
#define AUTH_TLV_CAPS          0x0003u
#define AUTH_TLV_MTU_HINT      0x0004u
#define AUTH_TLV_VENDOR_ID     0x0100u
#define AUTH_TLV_DEVICE_MODEL  0x0101u

/* ---- Header ---- */

typedef struct auth_header {
    uint8_t  version;
    uint8_t  pkt_type;
    uint16_t flags;
    uint8_t  session_id[AUTH_SESSION_ID_LEN];
    uint32_t seq;
} auth_header_t;

/* Encode the 24-byte header into `buf`. `buf_len` must be >= 24. */
auth_err_t auth_header_encode(
    const auth_header_t *hdr,
    uint8_t *buf, size_t buf_len, size_t *written_out);

/*
 * Decode the 24-byte header from `buf`. On success:
 *   - `hdr` receives the parsed fields
 *   - `payload_out` receives a pointer into `buf` at offset 24
 *   - `payload_len_out` receives (buf_len - 24)
 *
 * Returns PayloadTooShort if buf_len < 24, UnsupportedVersion if the
 * version byte is below AUTH_MIN_SUPPORTED_VERSION.
 */
auth_err_t auth_header_decode(
    auth_header_t *hdr,
    const uint8_t *buf, size_t buf_len,
    const uint8_t **payload_out, size_t *payload_len_out);

/* ---- Build / parse complete packets ---- */

/*
 * Build a complete packet: `header || payload` into `buf`. Returns
 * BufferTooSmall if `buf_len` is less than (24 + payload_len).
 * Returns PayloadTooLarge if payload_len exceeds AUTH_MAX_PAYLOAD.
 */
auth_err_t auth_packet_build(
    uint8_t pkt_type,
    const uint8_t session_id[AUTH_SESSION_ID_LEN],
    uint32_t seq,
    const uint8_t *payload, size_t payload_len,
    uint8_t *buf, size_t buf_len, size_t *packet_len_out);

/*
 * Build an ERROR packet. The payload is u16_LE(code) || utf8 msg.
 */
auth_err_t auth_packet_build_error(
    const uint8_t session_id[AUTH_SESSION_ID_LEN],
    uint32_t seq,
    auth_err_t code,
    const char *msg,
    uint8_t *buf, size_t buf_len, size_t *packet_len_out);

/*
 * Parse a raw error payload (PKT_ERROR body, after the 24-byte header).
 * Writes the decoded code to `*code_out`. `msg_out` receives a pointer
 * into the input buffer (NOT NUL-terminated); `msg_len_out` gives its
 * length.
 */
auth_err_t auth_packet_parse_error(
    const uint8_t *payload, size_t payload_len,
    auth_err_t *code_out,
    const char **msg_out, size_t *msg_len_out);

/* ---- TLV codec ---- */

/*
 * Write a TLV (tag, len, value) into `buf`. Returns BufferTooSmall if
 * not enough space, PayloadTooLarge if value_len > UINT16_MAX.
 * On success, advances `*offset` by (4 + value_len).
 */
auth_err_t auth_tlv_write(
    uint8_t *buf, size_t buf_len, size_t *offset,
    uint16_t tag, const uint8_t *value, size_t value_len);

/*
 * Read one TLV from `buf` at `*offset`. On success:
 *   - `*tag_out`  = TLV tag
 *   - `*value_out` = pointer into `buf`
 *   - `*value_len_out` = TLV value length
 *   - `*offset` advanced past this TLV
 *
 * Returns PayloadTooShort when no more bytes remain (end-of-stream).
 * Returns MalformedPacket on truncated TLV.
 */
auth_err_t auth_tlv_read(
    const uint8_t *buf, size_t buf_len, size_t *offset,
    uint16_t *tag_out,
    const uint8_t **value_out, size_t *value_len_out);

/* ---- HELLO / HELLO_REPLY ---- */

typedef struct auth_hello {
    uint8_t            version;
    uint8_t            min_version;
    auth_suite_t   suites[8];  /* bounded; matches spec's reserved range */
    size_t             n_suites;
    auth_caps_t    caps;

    /* Optional extensions */
    int                has_mtu_hint;
    uint16_t           mtu_hint;
    uint8_t            vendor_id[16];
    size_t             vendor_id_len;  /* 0 if absent */
    uint8_t            device_model[64];
    size_t             device_model_len;  /* 0 if absent */
} auth_hello_t;

/*
 * Encode a Hello into `buf`. `written_out` receives total bytes written.
 */
auth_err_t auth_hello_encode(
    const auth_hello_t *hello,
    uint8_t *buf, size_t buf_len, size_t *written_out);

/*
 * Decode a Hello from `buf`. Unknown TLVs are silently ignored.
 */
auth_err_t auth_hello_decode(
    auth_hello_t *hello_out,
    const uint8_t *buf, size_t buf_len);

/* ---- Negotiation ---- */

typedef struct auth_negotiated {
    uint8_t            version;
    auth_suite_t   suite;
    auth_caps_t    caps;
} auth_negotiated_t;

auth_err_t auth_negotiate(
    auth_negotiated_t *out,
    uint8_t local_version, uint8_t local_min_version,
    const auth_suite_t *local_suites, size_t n_local_suites,
    auth_caps_t local_caps,
    uint8_t peer_version, uint8_t peer_min_version,
    const auth_suite_t *peer_suites, size_t n_peer_suites,
    auth_caps_t peer_caps);

#ifdef __cplusplus
}
#endif
#endif /* AUTH_WIRE_H */
