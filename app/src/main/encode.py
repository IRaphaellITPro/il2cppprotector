#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
  JEB PROTECTOR — encode.py
  Telegram @Jebwuax

  Run python3 encode.py and enter the path to an ARM64 libil2cpp.so.

  Outputs:
    <here>/libhack_enc.so
    <here>/jni/key.h
    <here>/jni/lib/arm64-v8a/libhack_so_enc.o
"""

# ─── stdlib only — no pip install needed ────────────────────────────
import os
import sys
import time
import secrets
import hashlib
import struct
import subprocess
import shutil
import tempfile
from pathlib import Path
from datetime import datetime, timezone

# ════════════════════════════════════════════════════════════════════
#  PATHS  (all relative to this script — works from any cwd)
# ════════════════════════════════════════════════════════════════════
HERE    = Path(__file__).resolve().parent
JNI     = HERE / "jni"
LIB_DIR = JNI / "lib" / "arm64-v8a"
KEY_H   = JNI / "key.h"

# ════════════════════════════════════════════════════════════════════
#  ENCRYPTION
#
#  Cipher: XOR + ADD + rolling-feedback  (all byte-level, no deps)
#
#  Three keys, each 16 bytes (128 bits), total 48 secret bytes:
#    K_xor   — XOR mask,  rotated per-byte by index
#    K_add   — ADD mask,  rotated per-byte by index
#    K_fb    — seed for a rolling feedback register
#
#  Encrypt byte at position i:
#    fb[i]  = fb[i-1] ^ plain[i-1]        (0 at start)
#    xk     = K_xor[(i * 7 + 3) % 16]
#    ak     = K_add[(i * 5 + 11) % 16]
#    fk     = K_fb [i % 16]
#    c      = ((plain[i] ^ xk ^ fb[i]) + ak + fk) & 0xFF
#
#  Decrypt:
#    p      = ((c - ak - fk) & 0xFF) ^ xk ^ fb[i]
#
#  Every run: fresh K_xor, K_add, K_fb from os.urandom.
#  The cipher is intentionally simple byte ops — C macro-friendly,
#  matches exactly between Python and C.
# ════════════════════════════════════════════════════════════════════
PART = 16    # bytes per key part
KEY_TOTAL = PART * 3   # 48 bytes total

def _split_key(key48):
    return key48[:PART], key48[PART:PART*2], key48[PART*2:]

def _encrypt(data, key48):
    k_xor, k_add, k_fb = _split_key(key48)
    out  = bytearray(len(data))
    fb   = 0
    for i, b in enumerate(data):
        xk = k_xor[(i * 7 + 3) % PART]
        ak = k_add[(i * 5 + 11) % PART]
        fk = k_fb[i % PART]
        c  = (b ^ xk ^ fb) + ak + fk
        out[i] = c & 0xFF
        fb = (fb ^ b) & 0xFF
    return bytes(out)

def _decrypt(data, key48):
    k_xor, k_add, k_fb = _split_key(key48)
    out  = bytearray(len(data))
    fb   = 0
    for i, c in enumerate(data):
        xk   = k_xor[(i * 7 + 3) % PART]
        ak   = k_add[(i * 5 + 11) % PART]
        fk   = k_fb[i % PART]
        b    = ((c - ak - fk) & 0xFF) ^ xk ^ fb
        out[i] = b
        fb = (fb ^ b) & 0xFF
    return bytes(out)

# ════════════════════════════════════════════════════════════════════
#  CJK DISPLAY  (obfuscation eye-candy, not security)
# ════════════════════════════════════════════════════════════════════
_H2C = {
    '0':'零','1':'一','2':'二','3':'三',
    '4':'四','5':'五','6':'六','7':'七',
    '8':'八','9':'九','a':'甲','b':'乙',
    'c':'丙','d':'丁','e':'戊','f':'己',
}

def _b2cjk(data):
    return ''.join(_H2C[c] for b in data for c in '%02x' % b)

def _fingerprint(key48):
    return _b2cjk(hashlib.sha256(key48).digest()[:8])

# ════════════════════════════════════════════════════════════════════
#  key.h  GENERATOR
#
#  key.h stores NOTHING in plain form.
#  Each of the three 16-byte key parts is split into:
#    STORE[i] = key_part[i] ^ NOISE[i]   (XOR with random noise)
#    NOISE[i] = random bytes, baked in as literals
#
#  The C macro JEB_KX(i) / JEB_KA(i) / JEB_KF(i) XORs them back.
#  A static inline function jeb_decrypt_byte(i, c) does the full
#  cipher in one call — the wrapper just calls that in a loop.
# ════════════════════════════════════════════════════════════════════
def _hex_row(data, cols=8):
    lines = []
    for r in range(0, len(data), cols):
        chunk = data[r:r+cols]
        lines.append('    ' + ', '.join('0x%02X' % b for b in chunk))
    return ',\n'.join(lines)

def _gen_key_h(key48, src_name, enc_name):
    k_xor, k_add, k_fb = _split_key(key48)
    now    = datetime.now(timezone.utc).strftime('%Y-%m-%dT%H:%M:%SZ')
    sha256 = hashlib.sha256(key48).hexdigest()
    fp     = _fingerprint(key48)
    cjk    = _b2cjk(key48)

    # per-part noise
    n_xor = secrets.token_bytes(PART)
    n_add = secrets.token_bytes(PART)
    n_fb  = secrets.token_bytes(PART)

    # STORE = part ^ noise
    s_xor = bytes(k_xor[i] ^ n_xor[i] for i in range(PART))
    s_add = bytes(k_add[i] ^ n_add[i] for i in range(PART))
    s_fb  = bytes(k_fb[i]  ^ n_fb[i]  for i in range(PART))

    return """\
/*
 * ╔══════════════════════════════════════════════════════════════════╗
 * ║         自动生成密钥头文件  ·  AUTO-GENERATED KEY HEADER          ║
 * ║     Join Telegram @Jebwuax   for updates, tools & support       ║
 * ╚══════════════════════════════════════════════════════════════════╝
 *
 *  Generated : {now}
 *  Source    : {src}
 *  Encrypted : {enc}
 *  Key size  : {ksz} bytes ({kbits} bits)  [{parts} x {part}b parts]
 *  SHA-256   : {sha}
 *
 *  CJK fingerprint : {fp}
 *  CJK key (full)  : {cjk}
 *
 *  ⚠  KEEP SECRET — DO NOT COMMIT  ⚠
 *
 *  CIPHER (matches encode.py exactly)
 *    xk    = K_XOR[(i*7+3) % 16]
 *    ak    = K_ADD[(i*5+11) % 16]
 *    fk    = K_FB [i % 16]
 *    plain = ((cipher - ak - fk) & 0xFF) ^ xk ^ fb
 *    fb    = (fb ^ plain) & 0xFF   (rolling, starts 0)
 *
 *  OBFUSCATION: each part stored as STORE = part ^ NOISE
 *    Recovered by macro at call site — no plain key anywhere.
 */

#ifndef JEB_KEY_H
#define JEB_KEY_H

#include <stdint.h>

#define JEB_PART 16u

/* XOR part */
static const uint8_t _JK_XS[JEB_PART] = {{
{sxor}
}};
static const uint8_t _JK_XN[JEB_PART] = {{
{nxor}
}};

/* ADD part */
static const uint8_t _JK_AS[JEB_PART] = {{
{sadd}
}};
static const uint8_t _JK_AN[JEB_PART] = {{
{nadd}
}};

/* Feedback seed part */
static const uint8_t _JK_FS[JEB_PART] = {{
{sfb}
}};
static const uint8_t _JK_FN[JEB_PART] = {{
{nfb}
}};

/* Recover each key part byte by XOR-ing store ^ noise */
#define JEB_KX(i) ((uint8_t)(_JK_XS[(i)] ^ _JK_XN[(i)]))
#define JEB_KA(i) ((uint8_t)(_JK_AS[(i)] ^ _JK_AN[(i)]))
#define JEB_KF(i) ((uint8_t)(_JK_FS[(i)] ^ _JK_FN[(i)]))

/*
 *  jeb_decrypt_byte(i, c, fb)
 *    i  = byte index (size_t)
 *    c  = ciphertext byte (uint8_t)
 *    fb = rolling feedback (uint8_t, caller maintains across loop)
 *  Returns decrypted plain byte.
 *
 *  Usage:
 *    uint8_t fb = 0;
 *    for (size_t i = 0; i < len; i++) {{
 *        uint8_t p = jeb_decrypt_byte(i, enc[i], fb);
 *        out[i] = p;
 *        fb = (fb ^ p) & 0xFF;
 *    }}
 */
static inline uint8_t jeb_decrypt_byte(size_t i, uint8_t c, uint8_t fb) {{
    uint8_t xk = JEB_KX((i * 7u + 3u) % JEB_PART);
    uint8_t ak = JEB_KA((i * 5u + 11u) % JEB_PART);
    uint8_t fk = JEB_KF(i % JEB_PART);
    return (uint8_t)(((c - ak - fk) & 0xFFu) ^ xk ^ fb);
}}

/* SHA-256 of raw 48-byte key (integrity check) */
static const char JEB_KEY_SHA256[] = "{sha}";

#endif /* JEB_KEY_H */
""".format(
        now=now, src=src_name, enc=enc_name,
        ksz=KEY_TOTAL, kbits=KEY_TOTAL*8, parts=3, part=PART,
        sha=sha256, fp=fp, cjk=cjk,
        sxor=_hex_row(s_xor), nxor=_hex_row(n_xor),
        sadd=_hex_row(s_add), nadd=_hex_row(n_add),
        sfb =_hex_row(s_fb),  nfb =_hex_row(n_fb),
    )

# ════════════════════════════════════════════════════════════════════
#  OBJCOPY  →  encrypted .so  to linkable .o
# ════════════════════════════════════════════════════════════════════
def _find_objcopy():
    for cmd in ('llvm-objcopy', 'aarch64-linux-gnu-objcopy', 'objcopy'):
        if shutil.which(cmd):
            return cmd
    return None

def _run_objcopy(tool, enc_so, out_o):
    result = subprocess.run(
        [tool,
         '--input-target',        'binary',
         '--output-target',       'elf64-littleaarch64',
         '--binary-architecture', 'aarch64',
         enc_so.name, str(out_o)],
        cwd=enc_so.parent, capture_output=True, text=True,
    )
    if result.returncode != 0:
        sys.exit('\n  ERROR: objcopy failed: ' + result.stderr.strip())

def _sym_name(enc_so):
    # objcopy rule: replace non-alnum with '_', prefix '_binary_'
    stem = enc_so.name.replace('.', '_').replace('-', '_')
    return '_binary_' + stem

# ════════════════════════════════════════════════════════════════════
#  TERMINAL
# ════════════════════════════════════════════════════════════════════
BANNER = r"""
  ╔══════════════════════════════════════════════════════════════╗
  ║  ██╗███████╗██████╗    ███████╗███╗   ██╗ ██████╗          ║
  ║  ██║██╔════╝██╔══██╗   ██╔════╝████╗  ██║██╔════╝          ║
  ║  ██║█████╗  ██████╔╝   █████╗  ██╔██╗ ██║██║               ║
  ║  ██║██╔══╝  ██╔══██╗   ██╔══╝  ██║╚██╗██║██║               ║
  ║  ██║███████╗██████╔╝   ███████╗██║ ╚████║╚██████╗          ║
  ║  ╚═╝╚══════╝╚═════╝    ╚══════╝╚═╝  ╚═══╝ ╚═════╝          ║
  ║       [ 加密工具 · ENCODER · 暗号化 ]   @Jebwuax            ║
  ╚══════════════════════════════════════════════════════════════╝"""

OK_ART = r"""
  ╔══════════════════════════════════════════════════════════╗
  ║  ███████╗██╗   ██╗ ██████╗ ██████╗███████╗███████╗███╗  ║
  ║  ██╔════╝██║   ██║██╔════╝██╔════╝██╔════╝██╔════╝████╗ ║
  ║  ███████╗██║   ██║██║     ██║     █████╗  ███████╗██╔██╗║
  ║  ╚════██║██║   ██║██║     ██║     ██╔══╝  ╚════██║██║╚██║
  ║  ███████║╚██████╔╝╚██████╗╚██████╗███████╗███████║██║ ╚█║
  ║  ╚══════╝ ╚═════╝  ╚═════╝ ╚═════╝╚══════╝╚══════╝╚═╝  ╚║
  ║              成功  ·  SUCCESS  ·  성공                    ║
  ╚══════════════════════════════════════════════════════════╝"""

def _bar(cur, tot, w=46):
    f   = cur / tot if tot else 1.0
    blk = int(w * f)
    return '[%s%s] %5.1f%%  (%d/%d B)' % ('█'*blk, '░'*(w-blk), f*100, cur, tot)

def _fmtb(n):
    for u in ('B', 'KB', 'MB', 'GB'):
        if n < 1024:
            return '%.1f %s' % (n, u)
        n /= 1024.0
    return '%.1f TB' % n

def _hr(c='─', n=64):
    print('  ' + c * n)

# ════════════════════════════════════════════════════════════════════
#  MAIN
# ════════════════════════════════════════════════════════════════════
def run(src_path):
    enc_path = HERE / 'libhack_enc.so'
    obj_path = LIB_DIR / 'libhack_so_enc.o'
    key_path = KEY_H

    print(BANNER)
    _hr()
    print('  Source    : ' + str(src_path))
    print('  Encrypted : ' + str(enc_path))
    print('  Object    : ' + str(obj_path))
    print('  Key       : ' + str(key_path))
    _hr()

    # ── read ──────────────────────────────────────────────────────
    if not src_path.is_file():
        sys.exit('\n  ERROR: file not found: ' + str(src_path))
    if src_path == enc_path.resolve():
        sys.exit('\n  ERROR: input cannot be the encrypted output')
    tool = _find_objcopy()
    if not tool:
        sys.exit('\n  ERROR: install llvm-objcopy before encoding')
    src = src_path.read_bytes()
    if (len(src) < 64 or src[:6] != b'\x7fELF\x02\x01'
            or struct.unpack_from('<HH', src, 16) != (3, 183)):
        sys.exit('\n  ERROR: input must be a 64-bit ARM Android shared library')
    print('\n  Size      : ' + _fmtb(len(src)))

    # ── key ───────────────────────────────────────────────────────
    key48 = secrets.token_bytes(KEY_TOTAL)
    fp    = _fingerprint(key48)
    cjk16 = _b2cjk(key48)[:32]
    print('  Key       : %d bytes / %d-bit  (fresh random)' % (KEY_TOTAL, KEY_TOTAL*8))
    print('  Cipher    : XOR + ADD + rolling-feedback')
    print('  Fingerprint: ' + fp)
    print('  CJK[0:16] : ' + cjk16 + '...')

    # ── encrypt ───────────────────────────────────────────────────
    print('\n  Encrypting ...')
    t0  = time.time()
    enc = _encrypt(src, key48)
    ms  = (time.time() - t0) * 1000.0

    steps = max(1, len(src) // 30)
    for s in range(0, len(src) + 1, steps):
        pos = min(s, len(src))
        print('\r  ' + _bar(pos, len(src)), end='', **({'flush': True} if sys.version_info >= (3, 3) else {}))
        time.sleep(0.006)
    print('\r  ' + _bar(len(src), len(src)) + '  OK')
    print('  Time      : %.2f ms' % ms)

    # ── verify ────────────────────────────────────────────────────
    dec = _decrypt(enc, key48)
    if dec != src:
        sys.exit('\n  ERROR: round-trip verification FAILED')
    print('  Verify    : PASSED')
    print('  SHA-256   : ' + hashlib.sha256(enc).hexdigest())

    LIB_DIR.mkdir(parents=True, exist_ok=True)
    with tempfile.TemporaryDirectory(prefix='.jeb-encode-', dir=HERE) as tmp:
        tmp = Path(tmp)
        staged_enc = tmp / enc_path.name
        staged_obj = tmp / obj_path.name
        staged_key = tmp / key_path.name
        staged_enc.write_bytes(enc)
        staged_key.write_text(
            _gen_key_h(key48, src_path.name, enc_path.name), encoding='utf-8'
        )
        print('\n  objcopy   : ' + tool)
        _run_objcopy(tool, staged_enc, staged_obj)

        outputs = ((staged_enc, enc_path), (staged_obj, obj_path),
                   (staged_key, key_path))
        backups = []
        installed = []
        try:
            for _, target in outputs:
                if target.exists():
                    backup = tmp / ('previous-' + target.name)
                    os.replace(target, backup)
                    backups.append((backup, target))
            for staged, target in outputs:
                os.replace(staged, target)
                installed.append(target)
        except BaseException:
            for target in installed:
                target.unlink(missing_ok=True)
            for backup, target in backups:
                os.replace(backup, target)
            raise

    print('  Written   : ' + str(enc_path))
    print('  Written   : ' + str(obj_path))
    print('  Written   : ' + str(key_path))
    print('  Symbols   : ' + _sym_name(enc_path) + '_{start,end}')
    _hr('═')
    print(OK_ART)
    _hr('═')
    print('  Build with build-termux.sh -> libil2cpp.so')
    _hr('═')
    print()

# ════════════════════════════════════════════════════════════════════
#  ENTRY
# ════════════════════════════════════════════════════════════════════
if __name__ == '__main__':
    if len(sys.argv) > 2:
        sys.exit('Usage: python3 encode.py [path/to/libil2cpp.so]')
    path = sys.argv[1] if len(sys.argv) == 2 else input('Path to libil2cpp.so: ')
    path = path.strip()
    if len(path) >= 2 and path[0] == path[-1] and path[0] in "\"'":
        path = path[1:-1]
    if not path:
        sys.exit('ERROR: no input path provided')
    run(Path(path).expanduser().resolve())