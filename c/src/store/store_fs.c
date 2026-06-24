/*
 * auth_store_fs.c — filesystem-backed credential, registry, and
 * server-key stores. Writes are atomic (write to .tmp, fsync, rename).
 */

#define _POSIX_C_SOURCE 200809L
#define _DEFAULT_SOURCE

#include "auth/auth_store.h"
#include "auth/auth_crypto.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

/* ---- Little-endian helpers ---- */

static void w32le(uint8_t *p, uint32_t v) {
    p[0] = (uint8_t)(v       & 0xff);
    p[1] = (uint8_t)((v >>  8) & 0xff);
    p[2] = (uint8_t)((v >> 16) & 0xff);
    p[3] = (uint8_t)((v >> 24) & 0xff);
}
static uint32_t r32le(const uint8_t *p) {
    return (uint32_t)p[0]
         | ((uint32_t)p[1] <<  8)
         | ((uint32_t)p[2] << 16)
         | ((uint32_t)p[3] << 24);
}
static void w64le(uint8_t *p, uint64_t v) {
    for (int i = 0; i < 8; ++i) p[i] = (uint8_t)(v >> (8 * i));
}
static uint64_t r64le(const uint8_t *p) {
    uint64_t v = 0;
    for (int i = 0; i < 8; ++i) v |= (uint64_t)p[i] << (8 * i);
    return v;
}

/* ---- Atomic write helper ---- */

static auth_err_t atomic_write(
    const char *path, const uint8_t *data, size_t len, mode_t mode)
{
    char tmp[512];
    int n = snprintf(tmp, sizeof tmp, "%s.tmp", path);
    if (n < 0 || (size_t)n >= sizeof tmp) return AUTH_ERR_INVALID_ARGUMENT;

    int fd = open(tmp, O_CREAT | O_TRUNC | O_WRONLY, mode);
    if (fd < 0) return AUTH_ERR_STORAGE_FAILURE;

    size_t off = 0;
    while (off < len) {
        ssize_t w = write(fd, data + off, len - off);
        if (w < 0) {
            if (errno == EINTR) continue;
            close(fd); unlink(tmp);
            return AUTH_ERR_STORAGE_FAILURE;
        }
        off += (size_t)w;
    }
    if (fsync(fd) != 0) { close(fd); unlink(tmp); return AUTH_ERR_STORAGE_FAILURE; }
    if (close(fd)  != 0) { unlink(tmp); return AUTH_ERR_STORAGE_FAILURE; }
    if (rename(tmp, path) != 0) { unlink(tmp); return AUTH_ERR_STORAGE_FAILURE; }
    return AUTH_OK;
}

static auth_err_t read_whole_file(
    const char *path, uint8_t *buf, size_t buf_cap, size_t *n_read_out)
{
    int fd = open(path, O_RDONLY);
    if (fd < 0) {
        if (errno == ENOENT) return AUTH_ERR_CREDENTIAL_MISSING;
        return AUTH_ERR_STORAGE_FAILURE;
    }
    size_t off = 0;
    while (off < buf_cap) {
        ssize_t r = read(fd, buf + off, buf_cap - off);
        if (r < 0) {
            if (errno == EINTR) continue;
            close(fd); return AUTH_ERR_STORAGE_FAILURE;
        }
        if (r == 0) break;
        off += (size_t)r;
    }
    close(fd);
    *n_read_out = off;
    return AUTH_OK;
}

/* ---- CredentialStore ----
 *
 * Byte layout (154 bytes total):
 *   [0..8]    magic = "IACRED\0\1"
 *   [8]       version = 1
 *   [9]       flags  bit0=pinned, bit1=has_role
 *   [10..42]  device_root
 *   [42..74]  server_pub (or zeros)
 *   [74..106] role_commitment (or zeros)
 *   [106..138] role_blind (or zeros)
 *   [138..146] role_code (u64 LE)
 *   [146..154] reserved zeros
 */

#define CRED_FILE_LEN 154
static const uint8_t CRED_MAGIC[8] = { 'I','A','C','R','E','D', 0x00, 0x01 };

void auth_creds_init_fresh(auth_credentials_t *out)
{
    if (!out) return;
    memset(out, 0, sizeof *out);
    auth_random_bytes32(out->device_root);
}

auth_err_t auth_creds_load(
    auth_credentials_t *out, const char *path)
{
    if (!out || !path) return AUTH_ERR_INVALID_ARGUMENT;
    memset(out, 0, sizeof *out);

    uint8_t buf[CRED_FILE_LEN];
    size_t n = 0;
    auth_err_t err = read_whole_file(path, buf, sizeof buf, &n);
    if (err == AUTH_ERR_CREDENTIAL_MISSING) return err;
    if (err) return err;
    if (n != CRED_FILE_LEN) return AUTH_ERR_REGISTRY_CORRUPT;
    if (memcmp(buf, CRED_MAGIC, 8) != 0) return AUTH_ERR_REGISTRY_CORRUPT;
    if (buf[8] != 1) return AUTH_ERR_REGISTRY_CORRUPT;

    uint8_t flags = buf[9];
    memcpy(out->device_root,       buf + 10, 32);
    out->has_pinned_server = (flags & 1) != 0;
    memcpy(out->server_pub_pinned, buf + 42, 32);
    out->has_role = (flags & 2) != 0;
    memcpy(out->role_commitment,   buf + 74,  32);
    memcpy(out->role_blind,        buf + 106, 32);
    out->role_code = r64le(buf + 138);

    return AUTH_OK;
}

auth_err_t auth_creds_save(
    const auth_credentials_t *in, const char *path)
{
    if (!in || !path) return AUTH_ERR_INVALID_ARGUMENT;
    uint8_t buf[CRED_FILE_LEN] = {0};
    memcpy(buf, CRED_MAGIC, 8);
    buf[8] = 1;
    buf[9] = (uint8_t)((in->has_pinned_server ? 1 : 0)
                    | (in->has_role ? 2 : 0));
    memcpy(buf + 10, in->device_root, 32);
    if (in->has_pinned_server) memcpy(buf + 42, in->server_pub_pinned, 32);
    if (in->has_role) {
        memcpy(buf + 74,  in->role_commitment, 32);
        memcpy(buf + 106, in->role_blind,      32);
        w64le(buf + 138, in->role_code);
    }
    return atomic_write(path, buf, sizeof buf, 0600);
}

/* ---- RegistryStore ----
 *
 * Byte layout:
 *   [0..8]   magic = "IAREG\0\0\1"
 *   [8]      version = 1
 *   [9..13]  count (u32 LE)
 *   [13..]   count * 96 bytes (device_id || device_pub || role_c)
 */

#define REG_ENTRY_LEN  96
#define REG_HEADER_LEN 13
static const uint8_t REG_MAGIC[8] = { 'I','A','R','E','G', 0x00, 0x00, 0x01 };

auth_err_t auth_registry_load(auth_registry_t *reg, const char *path)
{
    if (!reg || !path) return AUTH_ERR_INVALID_ARGUMENT;
    memset(reg, 0, sizeof *reg);
    if (strlen(path) + 1 > sizeof reg->path) return AUTH_ERR_INVALID_ARGUMENT;
    memcpy(reg->path, path, strlen(path) + 1);

    uint8_t buf[REG_HEADER_LEN + AUTH_REGISTRY_MAX_ENTRIES * REG_ENTRY_LEN];
    size_t n = 0;
    auth_err_t err = read_whole_file(path, buf, sizeof buf, &n);
    if (err == AUTH_ERR_CREDENTIAL_MISSING) {
        /* Empty registry is OK for a fresh server. */
        return AUTH_OK;
    }
    if (err) return err;
    if (n < REG_HEADER_LEN) return AUTH_ERR_REGISTRY_CORRUPT;
    if (memcmp(buf, REG_MAGIC, 8) != 0) return AUTH_ERR_REGISTRY_CORRUPT;
    if (buf[8] != 1) return AUTH_ERR_REGISTRY_CORRUPT;
    uint32_t count = r32le(buf + 9);
    if (count > AUTH_REGISTRY_MAX_ENTRIES) return AUTH_ERR_REGISTRY_CORRUPT;
    if (n < REG_HEADER_LEN + (size_t)count * REG_ENTRY_LEN) {
        return AUTH_ERR_REGISTRY_CORRUPT;
    }

    for (size_t i = 0; i < count; ++i) {
        const uint8_t *p = buf + REG_HEADER_LEN + i * REG_ENTRY_LEN;
        memcpy(reg->entries[i].device_id,       p,      32);
        memcpy(reg->entries[i].device_pub,      p + 32, 32);
        memcpy(reg->entries[i].role_commitment, p + 64, 32);
    }
    reg->n = count;
    return AUTH_OK;
}

auth_err_t auth_registry_save(const auth_registry_t *reg)
{
    if (!reg) return AUTH_ERR_INVALID_ARGUMENT;
    if (reg->path[0] == 0) return AUTH_ERR_INVALID_ARGUMENT;

    size_t total = REG_HEADER_LEN + reg->n * REG_ENTRY_LEN;
    uint8_t buf[REG_HEADER_LEN + AUTH_REGISTRY_MAX_ENTRIES * REG_ENTRY_LEN];
    memcpy(buf, REG_MAGIC, 8);
    buf[8] = 1;
    w32le(buf + 9, (uint32_t)reg->n);
    for (size_t i = 0; i < reg->n; ++i) {
        uint8_t *p = buf + REG_HEADER_LEN + i * REG_ENTRY_LEN;
        memcpy(p,      reg->entries[i].device_id,       32);
        memcpy(p + 32, reg->entries[i].device_pub,      32);
        memcpy(p + 64, reg->entries[i].role_commitment, 32);
    }
    return atomic_write(reg->path, buf, total, 0600);
}

auth_err_t auth_registry_put(
    auth_registry_t *reg,
    const uint8_t device_id[AUTH_DEVICE_ID_LEN],
    const uint8_t device_pub[AUTH_POINT_LEN],
    const uint8_t role_commitment[AUTH_POINT_LEN])
{
    if (!reg || !device_id || !device_pub || !role_commitment)
        return AUTH_ERR_INVALID_ARGUMENT;

    /* Update if exists. */
    for (size_t i = 0; i < reg->n; ++i) {
        if (memcmp(reg->entries[i].device_id, device_id, 32) == 0) {
            memcpy(reg->entries[i].device_pub,      device_pub,      32);
            memcpy(reg->entries[i].role_commitment, role_commitment, 32);
            return AUTH_OK;
        }
    }
    if (reg->n >= AUTH_REGISTRY_MAX_ENTRIES) return AUTH_ERR_TOO_MANY_ACTIVE;
    memcpy(reg->entries[reg->n].device_id,       device_id,       32);
    memcpy(reg->entries[reg->n].device_pub,      device_pub,      32);
    memcpy(reg->entries[reg->n].role_commitment, role_commitment, 32);
    reg->n++;
    return AUTH_OK;
}

auth_err_t auth_registry_get(
    const auth_registry_t *reg,
    const uint8_t device_id[AUTH_DEVICE_ID_LEN],
    uint8_t device_pub[AUTH_POINT_LEN],
    uint8_t role_commitment[AUTH_POINT_LEN])
{
    if (!reg || !device_id) return AUTH_ERR_INVALID_ARGUMENT;
    for (size_t i = 0; i < reg->n; ++i) {
        if (memcmp(reg->entries[i].device_id, device_id, 32) == 0) {
            if (device_pub)      memcpy(device_pub,      reg->entries[i].device_pub,      32);
            if (role_commitment) memcpy(role_commitment, reg->entries[i].role_commitment, 32);
            return AUTH_OK;
        }
    }
    return AUTH_ERR_UNKNOWN_DEVICE;
}

int auth_registry_scan(
    const auth_registry_t *reg,
    auth_registry_scan_fn fn, void *ctx)
{
    if (!reg || !fn) return 0;
    for (size_t i = 0; i < reg->n; ++i) {
        int r = fn(ctx,
                   reg->entries[i].device_id,
                   reg->entries[i].device_pub,
                   reg->entries[i].role_commitment);
        if (r) return 1;
    }
    return 0;
}

/* ---- ServerKeyStore ---- */

#define SKEY_FILE_LEN 41
static const uint8_t SKEY_MAGIC[8] = { 'I','A','S','K','E','Y', 0x00, 0x01 };

auth_err_t auth_server_key_load_or_create(
    uint8_t server_sk[AUTH_SCALAR_LEN], const char *path)
{
    if (!server_sk || !path) return AUTH_ERR_INVALID_ARGUMENT;

    uint8_t buf[SKEY_FILE_LEN];
    size_t n = 0;
    auth_err_t err = read_whole_file(path, buf, sizeof buf, &n);
    if (err == AUTH_OK) {
        if (n != SKEY_FILE_LEN) return AUTH_ERR_REGISTRY_CORRUPT;
        if (memcmp(buf, SKEY_MAGIC, 8) != 0) return AUTH_ERR_REGISTRY_CORRUPT;
        if (buf[8] != 1) return AUTH_ERR_REGISTRY_CORRUPT;
        memcpy(server_sk, buf + 9, AUTH_SCALAR_LEN);
        return AUTH_OK;
    }
    if (err != AUTH_ERR_CREDENTIAL_MISSING) return err;

    /* Create fresh key. */
    auth_random_scalar(server_sk);
    uint8_t fresh[SKEY_FILE_LEN] = {0};
    memcpy(fresh, SKEY_MAGIC, 8);
    fresh[8] = 1;
    memcpy(fresh + 9, server_sk, AUTH_SCALAR_LEN);
    return atomic_write(path, fresh, sizeof fresh, 0600);
}
