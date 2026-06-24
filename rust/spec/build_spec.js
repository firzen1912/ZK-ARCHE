// IoT-Auth ZK-ARCHE v2 — Wire Format & Interoperability Specification
// Generates spec/iot-auth-wire-spec.docx

const fs = require("fs");
const {
  Document, Packer, Paragraph, TextRun, Table, TableRow, TableCell,
  AlignmentType, PageOrientation, LevelFormat, HeadingLevel,
  BorderStyle, WidthType, ShadingType, PageBreak, TabStopType, TabStopPosition,
} = require("docx");

// ---------- Design tokens ----------

const COLOR_HEADER_BG = "1F4E79";   // deep blue
const COLOR_HEADER_FG = "FFFFFF";
const COLOR_SUBHEADER = "D9E2F3";   // light blue
const COLOR_ZEBRA = "F2F2F2";       // zebra row
const COLOR_NOTE_BG = "FFF2CC";     // callout yellow
const COLOR_RULE = "2E75B6";

const BORDER = { style: BorderStyle.SINGLE, size: 4, color: "BFBFBF" };
const BORDERS = { top: BORDER, bottom: BORDER, left: BORDER, right: BORDER };
const CELL_MARGINS = { top: 80, bottom: 80, left: 120, right: 120 };

// Table widths based on US Letter @ 1" margins = 9360 DXA content width
const CONTENT_WIDTH = 9360;

// ---------- Helpers ----------

function p(text, opts = {}) {
  return new Paragraph({
    alignment: opts.align,
    spacing: { before: opts.before ?? 60, after: opts.after ?? 60, line: 300 },
    indent: opts.indent,
    children: Array.isArray(text) ? text : [new TextRun({ text, ...opts })],
  });
}

function h1(text) {
  return new Paragraph({
    heading: HeadingLevel.HEADING_1,
    spacing: { before: 360, after: 120 },
    children: [new TextRun({ text })],
  });
}
function h2(text) {
  return new Paragraph({
    heading: HeadingLevel.HEADING_2,
    spacing: { before: 280, after: 100 },
    children: [new TextRun({ text })],
  });
}
function h3(text) {
  return new Paragraph({
    heading: HeadingLevel.HEADING_3,
    spacing: { before: 220, after: 80 },
    children: [new TextRun({ text })],
  });
}

function code(text) {
  return new Paragraph({
    spacing: { before: 40, after: 40, line: 260 },
    children: [new TextRun({ text, font: "Consolas", size: 20 })],
    shading: { type: ShadingType.CLEAR, fill: "F5F5F5" },
  });
}

function codeBlock(lines) {
  return lines.map(l => code(l));
}

function bullet(text) {
  return new Paragraph({
    numbering: { reference: "bullets", level: 0 },
    spacing: { before: 40, after: 40, line: 280 },
    children: Array.isArray(text) ? text : [new TextRun({ text })],
  });
}

function num(text) {
  return new Paragraph({
    numbering: { reference: "numbers", level: 0 },
    spacing: { before: 40, after: 40, line: 280 },
    children: Array.isArray(text) ? text : [new TextRun({ text })],
  });
}

function hr() {
  return new Paragraph({
    spacing: { before: 100, after: 100 },
    border: { bottom: { style: BorderStyle.SINGLE, size: 6, color: COLOR_RULE, space: 1 } },
    children: [new TextRun("")],
  });
}

function noteBox(text) {
  return new Paragraph({
    spacing: { before: 120, after: 120, line: 280 },
    shading: { type: ShadingType.CLEAR, fill: COLOR_NOTE_BG },
    indent: { left: 360 },
    children: [
      new TextRun({ text: "Note. ", bold: true, color: "7F6000" }),
      new TextRun({ text, color: "7F6000" }),
    ],
  });
}

function headerCell(text, width) {
  return new TableCell({
    borders: BORDERS,
    margins: CELL_MARGINS,
    width: { size: width, type: WidthType.DXA },
    shading: { type: ShadingType.CLEAR, fill: COLOR_HEADER_BG },
    children: [new Paragraph({
      children: [new TextRun({ text, bold: true, color: COLOR_HEADER_FG, size: 20 })],
      spacing: { before: 40, after: 40 },
    })],
  });
}

function dataCell(text, width, opts = {}) {
  return new TableCell({
    borders: BORDERS,
    margins: CELL_MARGINS,
    width: { size: width, type: WidthType.DXA },
    shading: opts.zebra ? { type: ShadingType.CLEAR, fill: COLOR_ZEBRA } : undefined,
    children: [new Paragraph({
      children: [new TextRun({
        text,
        font: opts.mono ? "Consolas" : undefined,
        size: 20,
        bold: opts.bold,
      })],
      spacing: { before: 30, after: 30 },
    })],
  });
}

function makeTable(headers, rows, widths) {
  const rowsOut = [
    new TableRow({
      tableHeader: true,
      children: headers.map((h, i) => headerCell(h, widths[i])),
    }),
    ...rows.map((r, ri) => new TableRow({
      children: r.map((cell, ci) => {
        const opts = {};
        if (ri % 2 === 1) opts.zebra = true;
        if (typeof cell === "object") {
          if (cell.mono) opts.mono = true;
          if (cell.bold) opts.bold = true;
          return dataCell(cell.text, widths[ci], opts);
        }
        return dataCell(cell, widths[ci], opts);
      }),
    })),
  ];
  return new Table({
    width: { size: widths.reduce((a, b) => a + b, 0), type: WidthType.DXA },
    columnWidths: widths,
    rows: rowsOut,
  });
}

// ---------- Document content ----------

const children = [];

// ---- Title page ----
children.push(
  new Paragraph({
    alignment: AlignmentType.CENTER,
    spacing: { before: 2400, after: 240 },
    children: [new TextRun({
      text: "IoT-Auth ZK-ARCHE v2",
      bold: true, size: 56, color: COLOR_HEADER_BG,
    })],
  }),
  new Paragraph({
    alignment: AlignmentType.CENTER,
    spacing: { after: 480 },
    children: [new TextRun({
      text: "Wire-Format & Interoperability Specification",
      size: 36, color: "404040",
    })],
  }),
  new Paragraph({
    alignment: AlignmentType.CENTER,
    spacing: { after: 160 },
    children: [new TextRun({ text: "Version 0.2.0 — April 2026", size: 24, italics: true })],
  }),
  new Paragraph({
    alignment: AlignmentType.CENTER,
    spacing: { after: 960 },
    children: [new TextRun({
      text: "Transport-agnostic authentication with pseudonymous session binding",
      size: 22, color: "595959", italics: true,
    })],
  }),
  new Paragraph({ children: [new PageBreak()] }),
);

// ---- Abstract ----
children.push(
  h1("Abstract"),
  p("This document specifies the wire format, message flows, capability negotiation, cryptographic bindings, error codes, and conformance requirements for the IoT-Auth ZK-ARCHE v2 protocol. The goal of this revision is heterogeneous interoperability: any two implementations that conform to this specification must be able to interoperate regardless of programming language, hardware platform, or transport binding (UDP, TCP, CoAP, BLE-L2CAP, serial gateway, or future transports)."),
  p("The protocol provides mutual authentication between a resource-constrained device and a server, with pseudonymous per-session identifiers, zero-knowledge role-set proofs, and authenticated key confirmation. Cryptographic primitives are defined over ristretto255 and are identified by a stable cipher-suite registry, allowing future agility without breaking existing deployments."),
);

// ---- Table of contents surrogate ----
children.push(
  h1("Contents"),
  bullet("1. Introduction and scope"),
  bullet("2. Terminology"),
  bullet("3. Protocol overview"),
  bullet("4. Wire format"),
  bullet("5. Capability negotiation"),
  bullet("6. Transcript canonicalization"),
  bullet("7. Cipher-suite registry and algorithm agility"),
  bullet("8. Error code registry"),
  bullet("9. Timing and retry policy; profiles"),
  bullet("10. Test vectors and conformance"),
  bullet("11. Conformance checklist"),
  bullet("Appendix A. Domain separators"),
  bullet("Appendix B. Change log"),
  new Paragraph({ children: [new PageBreak()] }),
);

// ===================================================================
// 1. Introduction
// ===================================================================
children.push(
  h1("1. Introduction and scope"),

  h2("1.1 Goals"),
  p("The ZK-ARCHE v2 protocol targets IoT deployments where devices are constrained (flash, RAM, power), networks are unreliable (UDP, lossy links, intermittent connectivity), and observability tolerates little operator intervention. This specification has four concrete goals:"),
  num("Transport independence. The wire format travels unchanged across UDP datagrams, TCP length-prefix streams, CoAP requests, BLE attribute writes, or any future binding. A single interoperability test matrix covers all transports."),
  num("Cryptographic agility. Every message is stamped with a cipher-suite identifier from a stable registry. Deployments may adopt new curves, hashes, or post-quantum KEMs by registering new suite IDs without renumbering existing messages."),
  num("Deterministic canonicalization. Transcripts, point encodings, scalar encodings, and session pseudonyms have byte-for-byte stable definitions. A reference test-vector corpus permits cross-language validation with zero ambiguity."),
  num("Clear failure surfaces. Every protocol error has a stable 16-bit code. Peers that implement the protocol in different languages must report the same error on the same inputs, which is essential for fleet-scale diagnostics."),

  h2("1.2 Non-goals"),
  bullet("Confidentiality of message contents: the protocol authenticates, but transport-layer confidentiality (if required) is expected to be provided by DTLS, QUIC, or an equivalent tunnel."),
  bullet("Forward secrecy guarantees beyond the ephemeral ECDHE exchange documented in §3.3."),
  bullet("Device attestation of hardware identity (TPM, secure-element remote attestation). These are complementary and MAY be carried in a future TLV extension."),

  h2("1.3 Document conventions"),
  p("Key words MUST, MUST NOT, REQUIRED, SHALL, SHALL NOT, SHOULD, SHOULD NOT, RECOMMENDED, MAY, and OPTIONAL are to be interpreted as described in RFC 2119 / RFC 8174."),
  p("All multi-byte integers on the wire are little-endian. All elliptic-curve points are 32-byte compressed ristretto255. All scalars are 32-byte canonical ristretto255 scalar encodings."),
);

// ===================================================================
// 2. Terminology
// ===================================================================
children.push(
  h1("2. Terminology"),
  makeTable(
    ["Term", "Definition"],
    [
      ["device", "The constrained party initiating authentication. Holds a long-term secret and a pinned server public key."],
      ["server", "The verifying party. Holds a long-term static secret and a registry of enrolled devices."],
      ["device_id", "32-byte public identifier derived from the device root. Used only during enrollment, never during online authentication."],
      ["pid", "32-byte per-session pseudonym H(device_pub || nonce_c || eph_c || server_pub). Binds every transcript for the session. See §6.3."],
      ["setup", "Enrollment flow. Issues long-term credentials to the device and pins the server raw public key. Three round-trips."],
      ["auth", "Online authentication flow. Produces a confirmed session key. Three round-trips."],
      ["suite", "Cipher-suite identifier (u16) drawn from the registry in §7."],
      ["profile", "Named bundle of timing, resource, and feature limits (§9.3)."],
      ["role_commitment", "Pedersen-style commitment C = g^role · h^blind carried in the registry, used in CDS role-set proofs."],
      ["session_id", "16-byte peer-scoped transport correlation identifier. Carried in every packet header."],
    ],
    [1800, 7560],
  ),
);

// ===================================================================
// 3. Protocol overview
// ===================================================================
children.push(
  h1("3. Protocol overview"),

  h2("3.1 Layering"),
  p("This specification is layered so heterogeneous implementations can map it onto their environment without interpreting parts that do not concern them:"),
  makeTable(
    ["Layer", "Responsibility", "Neutral ?"],
    [
      ["A. Semantics", "Setup and auth state machines; Schnorr, rerand, CDS-OR; KDF; KC.", "Yes"],
      ["B. Framing",   "24-byte header, packet types, TLV codec, payload encoders.",       "Yes"],
      ["C. Transport", "UDP datagrams, TCP length-prefix streams, CoAP, BLE, serial.",     "Binding"],
      ["D. Storage",   "Credential, registry, replay-cache backends.",                     "Binding"],
    ],
    [1500, 6360, 1500],
  ),
  p("Layers A and B are fully specified here. Layers C and D are defined via abstract traits; a reference UDP binding and a reference filesystem store are provided, but conforming implementations may substitute any binding that preserves the per-message byte boundary and the byte-for-byte storage layout described in §4 and §5."),

  h2("3.2 Flows"),
  p("The protocol defines two flows. Setup is run once, at enrollment time. Auth is run for every authenticated session."),

  h3("3.2.1 Setup"),
  ...codeBlock([
    "device                                                    server",
    "  |                                                          |",
    "  |---- SETUP_1 ---------------------------------------------->|",
    "  |     (pairing_token?, device_id, device_pub,               |",
    "  |      client_nonce, role_commitment)                       |",
    "  |                                                           |",
    "  |<--- SETUP_2 ----------------------------------------------|",
    "  |     (server_nonce, setup_challenge, server_pub, proof_s)  |",
    "  |                                                           |",
    "  |---- SETUP_3 --------------------------------------------->|",
    "  |     (proof_c)                                             |",
    "  |                                                           |",
    "  |<--- SETUP_ACK --------------------------------------------|",
    "  |     (0x01)                                                |",
  ]),

  h3("3.2.2 Auth"),
  ...codeBlock([
    "device                                                    server",
    "  |---- AUTH_1 ---------------------------------------------->|",
    "  |     (pid, proof_c, nonce_c, eph_c,                        |",
    "  |      c_prime, rerand_proof, role_set_branches*)           |",
    "  |                                                           |",
    "  |<--- AUTH_2 ----------------------------------------------|",
    "  |     (server_pub, proof_s, nonce_s, eph_s, tag_s)          |",
    "  |                                                           |",
    "  |---- AUTH_3 --------------------------------------------->|",
    "  |     (tag_c)                                               |",
    "  |                                                           |",
    "  |<--- AUTH_ACK --------------------------------------------|",
    "  |     (0x01)                                                |",
  ]),

  h2("3.3 Cryptographic summary"),
  bullet("Identity proof. Schnorr over ristretto255. Device proves knowledge of x such that device_pub = g·x, bound in auth to pid rather than device_id."),
  bullet("Server proof. Schnorr over ristretto255. Server proves knowledge of its static secret bound to nonce_s and eph_s."),
  bullet("Role privacy. The stored role commitment C = g·role + h·blind is rerandomized per session to C′ = C + h·delta. The device proves (a) C′ is a rerandomization of a registered C and (b) C′ commits to some role in the allowed-roles set, via CDS-OR composition of Schnorr proofs in base h."),
  bullet("Session key. Derived via HKDF-SHA256 from an ECDHE secret over ristretto255, with salt = nonce_c || nonce_s and info binding pid, eph_c, and eph_s."),
  bullet("Key confirmation. HMAC-SHA256 tags over the transcript hash in both directions."),

  noteBox("The v2 change from v1 is that online-auth transcripts bind to pid (per-session pseudonym), not the stable device_id. On-wire observers cannot link sessions from the same device without knowledge of device_pub."),
);

// ===================================================================
// 4. Wire format
// ===================================================================
children.push(
  new Paragraph({ children: [new PageBreak()] }),
  h1("4. Wire format"),

  h2("4.1 Packet header (24 bytes)"),
  p("Every packet — over any transport — begins with the following fixed header. Transports MUST preserve the byte boundary of each packet."),
  makeTable(
    ["Offset", "Size", "Field", "Encoding", "Description"],
    [
      [{ text: "0", mono: true }, "1", "version", "u8", "Protocol version. MUST be 0x02 in this revision."],
      [{ text: "1", mono: true }, "1", "pkt_type", "u8", "Packet type (§4.2)."],
      [{ text: "2", mono: true }, "2", "flags",  "u16 LE", "Flag bits; reserved=0 (§4.3)."],
      [{ text: "4", mono: true }, "16", "session_id", "bytes", "16-byte peer-scoped correlation id."],
      [{ text: "20", mono: true }, "4", "seq", "u32 LE", "Per-session monotonic sequence number."],
      [{ text: "24", mono: true }, "var", "payload", "bytes", "Packet-type-specific payload (§4.4)."],
    ],
    [900, 700, 1700, 1300, 4760],
  ),

  h2("4.2 Packet types"),
  makeTable(
    ["Code", "Name", "Direction", "Description"],
    [
      [{ text: "0x01", mono: true }, "HELLO",       "C→S",    "Capability / version probe (§5)."],
      [{ text: "0x02", mono: true }, "HELLO_REPLY", "S→C",    "Negotiated version / suite / caps (§5)."],
      [{ text: "0x11", mono: true }, "SETUP_1",     "C→S",    "Setup init. device_id, device_pub, nonce, role commitment."],
      [{ text: "0x12", mono: true }, "SETUP_2",     "S→C",    "Server nonce, challenge, server_pub, server proof."],
      [{ text: "0x13", mono: true }, "SETUP_3",     "C→S",    "Client setup proof."],
      [{ text: "0x14", mono: true }, "SETUP_ACK",   "S→C",    "Enrollment OK (single byte 0x01)."],
      [{ text: "0x21", mono: true }, "AUTH_1",      "C→S",    "Auth: pid, client proof, role proofs."],
      [{ text: "0x22", mono: true }, "AUTH_2",      "S→C",    "Server proof, nonce_s, eph_s, tag_s."],
      [{ text: "0x23", mono: true }, "AUTH_3",      "C→S",    "Client finished tag."],
      [{ text: "0x24", mono: true }, "AUTH_ACK",    "S→C",    "Auth OK (single byte 0x01)."],
      [{ text: "0x7f", mono: true }, "ERROR",       "either", "u16 code + UTF-8 message (§8)."],
    ],
    [1100, 1700, 1400, 5160],
  ),
  noteBox("Packet-type values MUST NOT be reassigned. A new message MUST be appended at an unused code; implementations that do not understand a new code MUST reply with ERROR/UnknownPacketType."),

  h2("4.3 Flag bits"),
  makeTable(
    ["Bit", "Name", "Description"],
    [
      [{ text: "0x0001", mono: true }, "FLAG_RETRANSMIT", "Set by the sender when retransmitting a previously-sent packet. Receivers idempotently resend the cached response for the same (session_id, seq)."],
      [{ text: "0x0002…0x8000", mono: true }, "(reserved)", "MUST be zero in this revision."],
    ],
    [1600, 2100, 5660],
  ),

  h2("4.4 Payload layouts"),
  p("All payload layouts below sit immediately after the 24-byte header. All points are 32 bytes, all scalars are 32 bytes."),

  h3("4.4.1 HELLO / HELLO_REPLY"),
  ...codeBlock([
    "u8   version",
    "u16  suite_count",
    "suite_count * u16  suite_ids (LE)",
    "u64  caps (LE)",
    "TLV* extensions (MIN_VERSION, MTU_HINT, VENDOR_ID, DEVICE_MODEL, …)",
  ]),

  h3("4.4.2 SETUP_1"),
  ...codeBlock([
    "u8    token_len              // 0..=128",
    "token_len bytes pairing_token",
    "32    device_id",
    "32    device_pub",
    "32    client_nonce",
    "32    role_commitment",
  ]),

  h3("4.4.3 SETUP_2"),
  ...codeBlock([
    "32    server_nonce",
    "16    setup_challenge",
    "32    server_pub",
    "32    a_s",
    "32    s_s",
  ]),

  h3("4.4.4 SETUP_3"),
  ...codeBlock([
    "32    a_c",
    "32    s_c",
  ]),

  h3("4.4.5 AUTH_1"),
  ...codeBlock([
    "32    pid",
    "32    a_c                    // client Schnorr proof commitment",
    "32    s_c                    // client Schnorr proof response",
    "32    nonce_c",
    "32    eph_c",
    "32    c_prime                // rerandomized role commitment",
    "32    rerand_a",
    "32    rerand_s",
    "u16   branches_len (LE)",
    "branches_len * (32 A_i | 32 c_i | 32 s_i)",
  ]),

  h3("4.4.6 AUTH_2"),
  ...codeBlock([
    "32    server_pub",
    "32    a_s",
    "32    s_s",
    "32    nonce_s",
    "32    eph_s",
    "32    tag_s                  // HMAC-SHA256(k_s2c, \"server finished\" || th)",
  ]),

  h3("4.4.7 AUTH_3"),
  ...codeBlock([
    "32    tag_c                  // HMAC-SHA256(k_c2s, \"client finished\" || th)",
  ]),

  h3("4.4.8 ERROR"),
  ...codeBlock([
    "u16   code (LE)             // see §8",
    "var   utf8_message",
  ]),

  h2("4.5 TLV codec"),
  p("Extensible messages (HELLO today; more in future) use trailing TLV lists:"),
  ...codeBlock([
    "tlv  ::= u16 tag (LE) | u16 len (LE) | len bytes value",
  ]),
  p("Reserved tags:"),
  makeTable(
    ["Tag", "Name", "Value"],
    [
      [{ text: "0x0001", mono: true }, "MIN_VERSION",  "1 byte: lowest acceptable protocol version."],
      [{ text: "0x0002", mono: true }, "SUITE_LIST",   "(reserved; suite list currently in fixed field)."],
      [{ text: "0x0003", mono: true }, "CAPS",         "(reserved; caps currently in fixed field)."],
      [{ text: "0x0004", mono: true }, "MTU_HINT",     "u16 LE: path MTU hint from the sender."],
      [{ text: "0x0100", mono: true }, "VENDOR_ID",    "Opaque vendor identifier (OUI or similar)."],
      [{ text: "0x0101", mono: true }, "DEVICE_MODEL", "UTF-8 device model string."],
    ],
    [1200, 1900, 6260],
  ),
  noteBox("Unknown TLV tags MUST be ignored (skipped) so the format remains additive. Peers MUST NOT reject a packet solely because it contains an unknown tag."),

  h2("4.6 Size limits"),
  bullet("MAX_DATAGRAM is 2048 bytes (header + payload). UDP senders SHOULD also respect path MTU hints carried in HELLO if smaller."),
  bullet("Pairing token ≤ 128 bytes."),
  bullet("TLV value ≤ 65535 bytes."),
);

// ===================================================================
// 5. Capability negotiation
// ===================================================================
children.push(
  new Paragraph({ children: [new PageBreak()] }),
  h1("5. Capability negotiation"),

  h2("5.1 Purpose"),
  p("HELLO / HELLO_REPLY lets peers agree on the protocol version, cipher suite, and optional feature set before starting a setup or auth flow. Deployments that fix the negotiated parameters out-of-band (e.g., fleets with a single software version) MAY skip HELLO and proceed directly to SETUP_1 or AUTH_1; the server then enforces v0x02 / suite 0x0001 / BASELINE capabilities implicitly."),

  h2("5.2 Algorithm"),
  p("Given each peer's advertised version, min_version, suite list, and caps bitmap, the negotiated parameters are:"),
  ...codeBlock([
    "version = min(local_version, peer_version)",
    "require  version >= max(local_min, peer_min)   else UnsupportedVersion",
    "",
    "suite   = first in local_suites that is also in peer_suites",
    "require  suite exists                          else UnsupportedSuite",
    "",
    "caps    = local_caps & peer_caps",
    "require  (caps & BASELINE) == BASELINE          else CapabilityMismatch",
  ]),

  h2("5.3 Capability bits"),
  makeTable(
    ["Bit", "Name", "Meaning"],
    [
      [{ text: "1 << 0", mono: true }, "AUTH_V2",              "Device supports online auth flow AUTH_1…AUTH_ACK. Baseline."],
      [{ text: "1 << 1", mono: true }, "ROLE_RERAND",          "Supports role-commitment rerandomization proofs. Baseline."],
      [{ text: "1 << 2", mono: true }, "ROLE_SET_MEMBERSHIP",  "Supports CDS-OR set-membership proof. Baseline."],
      [{ text: "1 << 3", mono: true }, "PAIRING_TOKEN",        "Accepts pairing-token-gated setup."],
      [{ text: "1 << 4", mono: true }, "TOFU_SETUP",           "Lab-mode TOFU pinning at first setup."],
      [{ text: "1 << 8", mono: true }, "PROFILE_MINIMAL",      "Minimal profile (auth-only; setup OOB)."],
      [{ text: "1 << 9", mono: true }, "PROFILE_STANDARD",     "Standard profile. Baseline."],
      [{ text: "1 << 10", mono: true }, "PROFILE_GATEWAY",     "Gateway / proxy profile."],
      [{ text: "1 << 16", mono: true }, "CBOR_FRAMING",        "Optional CBOR-based framing variant (reserved)."],
    ],
    [1300, 2400, 5660],
  ),
  p("BASELINE = AUTH_V2 | ROLE_RERAND | ROLE_SET_MEMBERSHIP | PROFILE_STANDARD. Every conforming implementation MUST advertise BASELINE."),

  h2("5.4 Negotiation errors"),
  p("Negotiation failures MUST return PKT_ERROR with one of the following codes so that the initiator can localize the mismatch:"),
  bullet("UnsupportedVersion (0x0101): version floor could not be satisfied."),
  bullet("UnsupportedSuite (0x0102): no mutually-supported cipher suite."),
  bullet("CapabilityMismatch (0x0103): mutual intersection missed a BASELINE bit."),
);

// ===================================================================
// 6. Transcript canonicalization
// ===================================================================
children.push(
  new Paragraph({ children: [new PageBreak()] }),
  h1("6. Transcript canonicalization"),

  h2("6.1 Encoding rules"),
  p("Transcripts are length-prefixed, typed byte buffers. An implementation that produces byte-identical transcripts for the test-vector inputs in §10 is conformant at the transcript layer."),
  makeTable(
    ["Element", "Encoding"],
    [
      ["Domain",  "u8 len | bytes"],
      ["Label",   "u8 len | bytes"],
      ["Message", "u32 LE len | bytes"],
      ["Point",   "32-byte compressed ristretto255"],
      ["Scalar",  "32-byte canonical scalar encoding"],
      ["u8",      "1 byte"],
      ["u64",     "8 bytes LE"],
    ],
    [2000, 7360],
  ),

  h2("6.2 Challenge and hash outputs"),
  bullet("Schnorr challenge scalars are produced by SHA-512 of the transcript followed by Scalar::from_bytes_mod_order_wide."),
  bullet("Key-confirmation transcript hashes are produced by SHA-256 of the transcript."),

  h2("6.3 Session pseudonym pid"),
  p("The session pseudonym pid is computed as:"),
  ...codeBlock([
    "pid = SHA256( u32_LE(len(T_PID)) || T_PID ||",
    "              device_pub || nonce_c || eph_c || server_pub )",
    "",
    "T_PID = b\"iot-auth/pid/v1\"",
  ]),
  p("Every online-auth transcript binds to pid. This is the one byte-layout detail most likely to diverge between implementations; see the pid test vector in §10."),

  h2("6.4 Domain separators"),
  p("See Appendix A for the full list and their exact byte contents."),
);

// ===================================================================
// 7. Suite registry
// ===================================================================
children.push(
  h1("7. Cipher-suite registry and algorithm agility"),

  h2("7.1 Registered suites"),
  makeTable(
    ["Suite ID", "Curve", "Hash", "KDF", "MAC", "Status"],
    [
      [{ text: "0x0001", mono: true }, "ristretto255", "SHA-256", "HKDF-SHA256", "HMAC-SHA256", "Mandatory"],
      [{ text: "0x0002", mono: true }, "ristretto255", "SHA-512", "HKDF-SHA512", "HMAC-SHA256", "Reserved"],
      [{ text: "0x0003–0x00FF", mono: true }, "—", "—", "—", "—", "Reserved for future ECDH suites"],
      [{ text: "0x0100–0x01FF", mono: true }, "—", "—", "—", "—", "Reserved for post-quantum suites"],
      [{ text: "0xFF00–0xFFFF", mono: true }, "—", "—", "—", "—", "Private / experimental use"],
    ],
    [1600, 1700, 1200, 1400, 1400, 2060],
  ),

  h2("7.2 Agility rules"),
  num("Suite IDs MUST NOT be reassigned. New suites are added by appending."),
  num("The transcript encoding is independent of the suite. Adding a new suite does not change §6 rules."),
  num("Test-vector files are published per suite (see §10). A single implementation MAY support multiple suites; the one it advertises first in HELLO is preferred."),
);

// ===================================================================
// 8. Error code registry
// ===================================================================
children.push(
  new Paragraph({ children: [new PageBreak()] }),
  h1("8. Error code registry"),

  h2("8.1 Categories"),
  makeTable(
    ["Range", "Category"],
    [
      [{ text: "0x0100–0x01FF", mono: true }, "Version / capability"],
      [{ text: "0x0200–0x02FF", mono: true }, "Packet framing / parsing"],
      [{ text: "0x0300–0x03FF", mono: true }, "Cryptographic validation"],
      [{ text: "0x0400–0x04FF", mono: true }, "Session / replay"],
      [{ text: "0x0500–0x05FF", mono: true }, "Authorization"],
      [{ text: "0x0600–0x06FF", mono: true }, "Rate limiting / resource"],
      [{ text: "0x0700–0x07FF", mono: true }, "Storage / backend"],
      [{ text: "0x7FFF",         mono: true }, "Unspecified / internal"],
    ],
    [2400, 6960],
  ),

  h2("8.2 Full registry"),
  makeTable(
    ["Code", "Name", "Meaning"],
    [
      [{ text: "0x0101", mono: true }, "UnsupportedVersion",   "No mutually-supported protocol version."],
      [{ text: "0x0102", mono: true }, "UnsupportedSuite",     "No mutually-supported cipher suite."],
      [{ text: "0x0103", mono: true }, "CapabilityMismatch",   "Baseline capabilities not mutually supported."],
      [{ text: "0x0201", mono: true }, "MalformedPacket",      "Framing or TLV corruption."],
      [{ text: "0x0202", mono: true }, "UnknownPacketType",    "pkt_type is not in the registry for this version."],
      [{ text: "0x0203", mono: true }, "PayloadTooLarge",      "Exceeds MAX_DATAGRAM or declared MTU."],
      [{ text: "0x0204", mono: true }, "PayloadTooShort",      "Field count / length implies more bytes than present."],
      [{ text: "0x0205", mono: true }, "InvalidEncoding",      "Reserved bits set, bad UTF-8, etc."],
      [{ text: "0x0301", mono: true }, "InvalidPoint",         "32-byte ristretto decoding failed."],
      [{ text: "0x0302", mono: true }, "NonCanonicalScalar",   "Scalar is not in canonical form."],
      [{ text: "0x0303", mono: true }, "IdentityPoint",        "Received point is the identity."],
      [{ text: "0x0304", mono: true }, "ProofVerifyFailed",    "Schnorr, rerand, or set-membership proof rejected."],
      [{ text: "0x0305", mono: true }, "KeyConfirmFailed",     "HMAC key-confirmation tag mismatch."],
      [{ text: "0x0306", mono: true }, "PeerKeyMismatch",      "Server public key differs from pinned value."],
      [{ text: "0x0401", mono: true }, "UnknownSession",       "session_id not in the active session cache."],
      [{ text: "0x0402", mono: true }, "SessionExpired",       "Session exists but its TTL has elapsed."],
      [{ text: "0x0403", mono: true }, "ReplayDetected",       "(pid, nonce_c, eph_c) hash already accepted."],
      [{ text: "0x0404", mono: true }, "SequenceOutOfOrder",   "seq is not the expected next value for this session."],
      [{ text: "0x0501", mono: true }, "UnknownDevice",        "No enrolled device matches the presented pid."],
      [{ text: "0x0502", mono: true }, "DeviceNotEnrolled",    "device_id not in registry (setup flow)."],
      [{ text: "0x0503", mono: true }, "RoleNotPermitted",     "Role set proof verified but role not allowed."],
      [{ text: "0x0504", mono: true }, "PairingTokenInvalid",  "Pairing token missing or incorrect."],
      [{ text: "0x0601", mono: true }, "RateLimited",          "Peer has exceeded the per-source failure budget."],
      [{ text: "0x0602", mono: true }, "ServerBusy",           "Server declines due to transient load."],
      [{ text: "0x0603", mono: true }, "TooManyActive",        "Session or response cache at capacity."],
      [{ text: "0x0701", mono: true }, "StorageFailure",       "Backend I/O error."],
      [{ text: "0x0702", mono: true }, "CredentialMissing",    "Expected credential (device root, server pin) absent."],
      [{ text: "0x0703", mono: true }, "RegistryCorrupt",      "On-disk registry failed integrity or shape checks."],
      [{ text: "0x7FFF", mono: true }, "Unspecified",          "Fallback; peers SHOULD avoid emitting."],
    ],
    [1200, 2400, 5760],
  ),

  h2("8.3 Error payload"),
  ...codeBlock([
    "u16  code (LE)",
    "var  utf8_message         // human-readable, not machine-actionable",
  ]),
);

// ===================================================================
// 9. Timing and profiles
// ===================================================================
children.push(
  new Paragraph({ children: [new PageBreak()] }),
  h1("9. Timing, retry policy, and profiles"),

  h2("9.1 Retry policy (unreliable transports)"),
  p("Over unreliable transports (UDP, CoAP/UDP, BLE with unacknowledged writes), the initiator retransmits the request with exponential backoff until it receives the expected response with matching (session_id, seq) or exceeds the retry budget."),
  ...codeBlock([
    "timeout(attempt) = retransmit_timeout << min(attempt, max_backoff_shift)",
    "attempts = 0 .. max_retries",
  ]),

  h2("9.2 Reliable transports"),
  p("Over reliable transports (TCP, QUIC, BLE-ATT with confirmations), the state machines send once and block for the response up to io_timeout. A stray packet with mismatched (session_id, seq) is discarded and the receiver keeps waiting for the expected one."),

  h2("9.3 Profiles"),
  p("A profile bundles timing and resource limits. Peers announce their profile via capability bits (§5.3); the server SHOULD apply the smaller of the two profiles' limits when responding to a given device."),
  makeTable(
    ["Parameter", "Minimal", "Standard", "Gateway"],
    [
      ["retransmit_timeout",   "800 ms",  "800 ms",  "800 ms"],
      ["max_retries",          "2",       "4",       "4"],
      ["max_backoff_shift",    "3",       "3",       "3"],
      ["io_timeout",           "5 s",     "5 s",     "5 s"],
      ["session_ttl",          "15 s",    "15 s",    "15 s"],
      ["max_active_sessions",  "8",       "1024",    "8192"],
      ["max_cached_responses", "16",      "2048",    "16384"],
      ["Setup supported",      "no (OOB)", "yes",     "yes"],
    ],
    [2500, 1620, 2540, 2700],
  ),

  h2("9.4 Idempotency on unreliable transports"),
  p("Servers MUST maintain a short-TTL cache of the last response produced for each (session_id, seq). A received packet with FLAG_RETRANSMIT set, matching (session_id, seq), MUST cause the cached response to be re-emitted verbatim without re-executing the protocol step. This makes retries safe under arbitrary datagram reordering."),
);

// ===================================================================
// 10. Test vectors
// ===================================================================
children.push(
  h1("10. Test vectors and conformance"),

  h2("10.1 Deterministic harness"),
  p("The reference implementation publishes a JSON test-vector file for each suite under test-vectors/<suite-id>/. Vectors are generated from a seeded RNG (deterministic) covering:"),
  bullet("transcripts.json — domain + fields → transcript bytes → challenge scalar."),
  bullet("pid.json — (device_pub, nonce_c, eph_c, server_pub) → 32-byte pid."),
  bullet("schnorr_setup.json — inputs, proof (a, s), and verification result."),
  bullet("role_set.json — allowed roles, commitment, proof branches, verification result."),
  bullet("kdf.json — HKDF inputs → 32-byte session key."),
  bullet("kc.json — KC transcript inputs → 32-byte hash and HMAC tags."),

  h2("10.2 JSON schema"),
  p("Each vector is a JSON object with these keys:"),
  ...codeBlock([
    "{",
    "  \"suite\": \"0x0001\",",
    "  \"name\": \"pid_basic\",",
    "  \"inputs\": { … fields as hex strings … },",
    "  \"expected\": { … outputs as hex strings … },",
    "  \"notes\": \"optional prose\"",
    "}",
  ]),

  h2("10.3 Required conformance outputs"),
  p("An implementation is conformant at the cryptographic layer if, for every vector in the published suite-0x0001 corpus, it reproduces the expected outputs exactly and accepts / rejects the expected-verification cases."),
);

// ===================================================================
// 11. Conformance checklist
// ===================================================================
children.push(
  new Paragraph({ children: [new PageBreak()] }),
  h1("11. Conformance checklist"),
  p("An implementation claiming conformance to ZK-ARCHE v2 MUST satisfy every item below."),

  h2("11.1 Wire format"),
  bullet("24-byte header with exact field offsets from §4.1."),
  bullet("Every packet type (§4.2) is recognized; unknown codes elicit ERROR/UnknownPacketType."),
  bullet("All flag bits except FLAG_RETRANSMIT are zero."),
  bullet("TLV parser ignores unknown tags (§4.5)."),

  h2("11.2 Cryptography"),
  bullet("Rejects identity point in every received point (Schnorr A, eph_c, eph_s, C, C′, server_pub, device_pub)."),
  bullet("Rejects non-canonical scalars."),
  bullet("Produces byte-identical transcripts on the published test vectors (§10)."),
  bullet("Verifies all six proof types: client-setup, server-setup, client-auth, server-auth, rerand, role-set."),

  h2("11.3 Negotiation"),
  bullet("Advertises BASELINE in HELLO caps (§5.3)."),
  bullet("Replies with the correct specific error code on version / suite / capability mismatch."),

  h2("11.4 Session handling"),
  bullet("Maintains a replay cache keyed on H(pid || nonce_c || eph_c)."),
  bullet("Caches per-(session_id, seq) responses for at least session_ttl and replays them verbatim on retransmit."),
  bullet("Zeroes the device scalar and ephemeral secret after use."),

  h2("11.5 Errors"),
  bullet("Emits PKT_ERROR with a specific code from §8 for every defined failure condition."),
  bullet("Does not emit error payloads containing secret state or user-supplied data."),
);

// ===================================================================
// Appendix A
// ===================================================================
children.push(
  new Paragraph({ children: [new PageBreak()] }),
  h1("Appendix A. Domain separators"),
  p("All domain separators are ASCII bytes without trailing nul. They are length-prefixed in transcripts per §6.1."),
  makeTable(
    ["Constant", "Value (ASCII)", "Scope"],
    [
      ["T_SETUP",        "setup_client_schnorr_v1", "Client Schnorr proof during enrollment."],
      ["T_SETUP_SERVER", "setup_server_schnorr_v1", "Server Schnorr proof during enrollment."],
      ["T_SERVER",       "server_schnorr_v1",       "Server Schnorr proof during online auth."],
      ["T_PID",          "iot-auth/pid/v1",         "PID derivation (§6.3)."],
      ["T_CLIENT_V2",    "client_schnorr_v2",       "Client Schnorr proof during online auth."],
      ["T_KC_V2",        "kc_v2",                   "Key-confirmation transcript hash."],
      ["T_ROLE_SET",     "client_role_set_v1",      "CDS-OR role set-membership proof."],
      ["T_ROLE_RERAND",  "client_role_rerand_v1",   "Role-commitment rerandomization proof."],
    ],
    [1900, 3400, 4060],
  ),
);

// ===================================================================
// Appendix B
// ===================================================================
children.push(
  h1("Appendix B. Change log"),
  bullet("v0.2.0 (this revision): introduced transport abstraction, stable 24-byte packet header (previously 22 bytes, session_id 16B + pkt_type + version + seq), structured 16-bit error codes, cipher-suite registry, HELLO negotiation, TLV extension codec, and conformance checklist."),
  bullet("v0.1.0: original ZK-ARCHE v2 reference implementation. Auth transcripts bound to pid; role commitment rerandomization; CDS-OR role-set proof; HMAC-SHA256 key confirmation."),
);

// ---------- Document assembly ----------

const doc = new Document({
  creator: "Khang (IoT-Auth project)",
  title: "IoT-Auth ZK-ARCHE v2 Wire-Format Specification",
  description: "Transport-agnostic wire format, negotiation, error codes, and conformance for ZK-ARCHE v2.",
  styles: {
    default: {
      document: { run: { font: "Arial", size: 22 } }, // 11 pt body
    },
    paragraphStyles: [
      { id: "Heading1", name: "Heading 1", basedOn: "Normal", next: "Normal", quickFormat: true,
        run: { size: 36, bold: true, font: "Arial", color: COLOR_HEADER_BG },
        paragraph: { spacing: { before: 360, after: 160 }, outlineLevel: 0 } },
      { id: "Heading2", name: "Heading 2", basedOn: "Normal", next: "Normal", quickFormat: true,
        run: { size: 28, bold: true, font: "Arial", color: "2E5984" },
        paragraph: { spacing: { before: 240, after: 120 }, outlineLevel: 1 } },
      { id: "Heading3", name: "Heading 3", basedOn: "Normal", next: "Normal", quickFormat: true,
        run: { size: 24, bold: true, font: "Arial", color: "404040" },
        paragraph: { spacing: { before: 180, after: 80 }, outlineLevel: 2 } },
    ],
  },
  numbering: {
    config: [
      { reference: "bullets",
        levels: [{ level: 0, format: LevelFormat.BULLET, text: "•", alignment: AlignmentType.LEFT,
          style: { paragraph: { indent: { left: 720, hanging: 360 } } } }] },
      { reference: "numbers",
        levels: [{ level: 0, format: LevelFormat.DECIMAL, text: "%1.", alignment: AlignmentType.LEFT,
          style: { paragraph: { indent: { left: 720, hanging: 360 } } } }] },
    ],
  },
  sections: [{
    properties: {
      page: {
        size: { width: 12240, height: 15840 },  // US Letter
        margin: { top: 1440, right: 1440, bottom: 1440, left: 1440 },
      },
    },
    children,
  }],
});

Packer.toBuffer(doc).then(buf => {
  fs.writeFileSync("/home/claude/iot-auth-refactor/spec/iot-auth-wire-spec.docx", buf);
  console.log("wrote spec/iot-auth-wire-spec.docx", buf.length, "bytes");
});
