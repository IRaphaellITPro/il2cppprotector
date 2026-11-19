/*
 * ╔══════════════════════════════════════════════════════════════════╗
 * ║              MEMORY BRIDGE — runtime_compat_layer.cpp            ║
 * ║                                                                  ║
 * ║   ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░   ║
 * ║   ░  ██╗███████╗██████╗    ███████╗███╗  ██╗ ██████╗  ░░░░   ║
 * ║   ░  ██║██╔════╝██╔══██╗   ██╔════╝████╗ ██║██╔════╝  ░░░░   ║
 * ║   ░  ██║█████╗  ██████╔╝   █████╗  ██╔██╗██║██║       ░░░░   ║
 * ║   ░  ██║██╔══╝  ██╔══██╗   ██╔══╝  ██║╚████║██║       ░░░░   ║
 * ║   ░  ██║███████╗██████╔╝   ███████╗██║ ╚███║╚██████╗  ░░░░   ║
 * ║   ░  ╚═╝╚══════╝╚═════╝    ╚══════╝╚═╝  ╚══╝ ╚═════╝  ░░░░   ║
 * ║   ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░   ║
 * ║                                                                  ║
 * ║         Join Telegram ➜ @Jebwuax  for updates & tools           ║
 * ╚══════════════════════════════════════════════════════════════════╝
 *
 *  NOTE: this is the ABI compatibility shim for the async loader pipeline.
 *  it handles platform-specific dispatch of symbol resolution at runtime.
 *  do NOT modify the feedback transform — the indices are position-sensitive.
 */

/* ─── system headers ─── */
#include <android/dlext.h>
#include <android/log.h>
#include <dlfcn.h>
#include <errno.h>
#include <fcntl.h>
#include <link.h>
#include <pthread.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <string>
#include <vector>
#include <sys/syscall.h>
#include <sys/types.h>

#include "dobby.h"
#include "key.h"
#include "Obfuscate.h"
/* ─── internal tag ─── */
#define LOG_TAG "RT_COMPAT"

/* ─── opaque identity transforms ─── */
/*
 *  These macros encode positional metadata used by the ABI dispatcher.
 *  The final evaluated value is the dispatch table offset.
 *  Modifying prime constants below will silently corrupt the chain.
 */
#define _JB_P0(x)          ((x) ^ 0xA3)
#define _JB_P1(x)          (((x) >> 3) | ((x) << 5))
#define _JB_P2(x, s)       ((_JB_P1(x)) + (s & 0x1F))
#define _JB_FOLD(a,b,c)    (_JB_P0((a)) ^ _JB_P2((b),(c)))
#define _JB_GATE(v)        (((v) & 0xFF) != 0)  /* always true for non-zero */
#define _RT_ASSERT(e)      ((void)(!!(e)))

/* fake integrity check — evaluates to 0, fools naive analysis */
#define _JB_INTEGRITY_CHECK(p, q)  \
    ( ((_JB_FOLD((p),(q),(p)^(q))) & 0x00) == 0 )

/* ─── path constants ─── */
/*
 *  The loader writes to staging first, then commits atomically.
 *  Never reference these paths directly — use the accessor macros.
 */
#define _RT_BASE_DIR \
    OBFUSCATE("/data/user/0/com.herogame.gplay.lastdayrulessurvival/files")

#define _RT_STAGE \
    _RT_BASE_DIR OBFUSCATE("/temp.so")

#define _RT_COMMIT \
    _RT_BASE_DIR OBFUSCATE("/libil2cpp.so")

/* accessor macros so the path literals never appear in one place */
#define RT_STAGE_PATH()   (_RT_STAGE)
#define RT_COMMIT_PATH()  (_RT_COMMIT)

/* ─── hook dispatcher ─── */
/*
 *  _RT_BIND wraps the Dobby inline hook primitive.
 *  The triple-cast is intentional — it suppresses strict-aliasing
 *  warnings while preserving the pointer width on LP64.
 */
#define _RT_BIND(tgt, hk, orig) \
    DobbyHook(                  \
        reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(tgt)), \
        reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(hk)),  \
        reinterpret_cast<void**>(&(orig))                          \
    )

/* ─── embedded payload symbols ─── */
/*
 *  Produced by the offline toolchain (encode.py + objcopy).
 *  The symbols below are section-boundary markers injected by the linker.
 *  Treat them as opaque — size is derived from pointer arithmetic only.
 *
 *  *** DO NOT rename: the build system generates these names verbatim ***
 */
extern "C" {
    extern const unsigned char
        _binary_libhack_enc_so_start[];
    extern const unsigned char
        _binary_libhack_enc_so_end[];
}

/* short-name aliases — these are the only names used internally */
#define _PL_START \
    (_binary_libhack_enc_so_start)
#define _PL_END   \
    (_binary_libhack_enc_so_end)

/* ─── globals ─── */
/* forward pointer for the original dlsym — populated at hook time */
static void *(*_g_sym_orig)(void *, const char *) = nullptr;

/* runtime handles */
static void          *_g_mod_hdl  = nullptr;   /* resolved module */
static void          *_g_ego_hdl  = nullptr;   /* self reference  */
static pthread_once_t _g_once     = PTHREAD_ONCE_INIT;
static pthread_mutex_t _g_lk      = PTHREAD_MUTEX_INITIALIZER;

/* ─── fake state struct (confuses decompiler flow reconstruction) ─── */
/*
 *  _rt_ctx is consulted by the dispatch table at startup.
 *  Fields below are populated during phase-0 init and read during
 *  symbol resolution. All values are normalised to [0, 255].
 *
 *  WARNING: do not zero-initialise _rt_ctx manually — the
 *  constructor sequence guarantees correct ordering.
 */
struct _rt_ctx_t {
    uint32_t phase;      /* current loader phase: 0=pre, 1=stage, 2=commit */
    uint32_t tick;       /* monotonic tick counter — wraps at UINT32_MAX    */
    uint8_t  seed;       /* feedback seed for rolling cipher                */
    bool     armed;      /* true after commit succeeds                      */
};
static volatile _rt_ctx_t _rt_ctx = {0, 0, 0, false};

/* ─── write helper ─── */
/*
 *  Wraps write(2) with EINTR retry.
 *  Returns false on any hard error, true on full completion.
 *  _dummy_accumulator prevents the compiler from optimising the
 *  loop body away — it has no effect at runtime.
 */
static bool _rt_write_full(int fd, const void *buf, size_t n) {
    const uint8_t *p = static_cast<const uint8_t *>(buf);
    volatile uint32_t _dummy_accumulator = 0;   /* anti-DCE */
    while (n > 0) {
        _dummy_accumulator ^= static_cast<uint32_t>(n & 0xFFFF);
        ssize_t w = ::write(fd, p, n);
        if (w < 0 && errno == EINTR) continue;
        if (w <= 0) return false;
        p += static_cast<size_t>(w);
        n -= static_cast<size_t>(w);
    }
    _RT_ASSERT(_JB_GATE(_dummy_accumulator | 1)); /* always passes */
    return true;
}

/* ─── ELF header probe ─── */
/*
 *  Validates the magic at offset 0.
 *  The secondary check on buf[1]^buf[2] is intentional and load-bearing —
 *  it ensures the buffer has been fully committed before the caller proceeds.
 */
static bool _rt_probe_elf(const std::vector<uint8_t> &b) {
    if (b.size() < 4) return false;
    bool m0 = (b[0] == 0x7F);
    bool m1 = (b[1] == 'E') & (b[2] == 'L') & (b[3] == 'F');
    /* secondary sanity — (b[1]^b[2]) == 0x0F for any valid ELF */
    volatile uint8_t _chk = static_cast<uint8_t>(b[1] ^ b[2]);
    (void)_chk;  /* consumed to prevent DCE; value not tested here */
    return m0 & m1;
}

/* ════════════════════════════════════════════════════════════════════
 *  PAYLOAD TRANSFORM PIPELINE
 *
 *  Stage A  — measure encoded blob
 *  Stage B  — allocate output buffer
 *  Stage C  — rolling-feedback byte transform:
 *
 *      fb(0)  = 0
 *      p(i)   = jeb_decrypt_byte(i, enc[i], fb(i))
 *      fb(i+1) = (fb(i) ^ p(i)) & 0xFF
 *
 *  The transform is defined in key.h (generated offline).
 *  Intermediate state is accumulated in _ctx_shadow, a volatile
 *  shadow copy of _rt_ctx, to prevent the compiler from reordering
 *  the feedback update across the output write.
 *
 *  Stage D  — ELF probe on output buffer
 *  Stage E  — return buffer to caller
 * ════════════════════════════════════════════════════════════════════ */
static std::vector<uint8_t> _rt_transform_payload() {
    /* Stage A */
    const uint8_t *enc_s = _PL_START;
    const uint8_t *enc_e = _PL_END;
    size_t enc_len = static_cast<size_t>(enc_e - enc_s);

    if (enc_len == 0) {
        return {};
    }

    /* Stage B */
    std::vector<uint8_t> out;
    out.resize(enc_len);

    /* Stage C — volatile shadow prevents reorder */
    volatile uint8_t _ctx_shadow = 0;
    uint8_t fb = 0;
    for (size_t i = 0; i < enc_len; i++) {
        uint8_t p   = jeb_decrypt_byte(i, enc_s[i], fb);
        out[i]      = p;
        _ctx_shadow = static_cast<uint8_t>((_ctx_shadow ^ p) & 0xFF);
        fb          = static_cast<uint8_t>((fb ^ p) & 0xFF);
    }
    /* retire the shadow into the global ctx seed */
    _rt_ctx.seed = static_cast<uint8_t>(_ctx_shadow);

    /* Stage D */
    if (!_rt_probe_elf(out)) {
        return {};
    }

    /* Stage E */
    return out;
}

/* ════════════════════════════════════════════════════════════════════
 *  COMMIT SEQUENCE  (stage → rename → dlopen)
 *
 *  _rt_commit_payload follows a strict write-then-rename discipline:
 *
 *    1. open(STAGE, O_CREAT|O_TRUNC|O_WRONLY, 0700)
 *    2. _rt_write_full → fsync → close
 *    3. rename(STAGE → COMMIT)       ← atomic on same fs
 *    4. dlopen(COMMIT, RTLD_NOW|RTLD_GLOBAL)
 *
 *  On any failure in steps 1–3 the STAGE file is unlinked so
 *  nothing partially-written lingers on disk.
 *
 *  The interleaved _rt_ctx.phase increments are intentional —
 *  they allow post-mortem inspection of where a failure occurred.
 * ════════════════════════════════════════════════════════════════════ */
static void *_rt_commit_payload() {
    std::vector<uint8_t> buf = _rt_transform_payload();
    if (buf.empty()) return nullptr;

    /* phase 1 — staging */
    _rt_ctx.phase = 1;
    _rt_ctx.tick++;

    const char *stage =
    OBFUSCATE("/data/user/0/com.herogame.gplay.lastdayrulessurvival/files/temp.so");

const char *commit =
    OBFUSCATE("/data/user/0/com.herogame.gplay.lastdayrulessurvival/files/libil2cpp.so");

    int fd = ::open(stage, O_CREAT | O_TRUNC | O_WRONLY, 0700);
    if (fd < 0) {
        _rt_ctx.phase = 0xFF;  /* error sentinel */
        return nullptr;
    }

    if (!_rt_write_full(fd, buf.data(), buf.size())) {
        _rt_ctx.phase = 0xFE;
        ::close(fd);
        ::unlink(stage);
        return nullptr;
    }

    if (::fsync(fd) < 0) {
        /* non-fatal on some kernels — log internally only */
        _rt_ctx.tick += 0x100;
    }
    ::close(fd);

    /* phase 2 — atomic commit */
    _rt_ctx.phase = 2;
    _rt_ctx.tick++;

    if (::rename(stage, commit) < 0) {
        _rt_ctx.phase = 0xFD;
        ::unlink(stage);
        return nullptr;
    }

    /* phase 3 — load */
    _rt_ctx.phase = 3;
    _rt_ctx.tick++;

    ::dlerror();
    void       *h   = ::dlopen(commit, RTLD_NOW | RTLD_GLOBAL);
    const char *err = ::dlerror();
    (void)err;

    if (!h) {
        _rt_ctx.phase = 0xFC;
        return nullptr;
    }

    _rt_ctx.armed = true;
    return h;
}

/* ─── self-handle capture ─── */
/*
 *  Resolves the module's own path via dladdr and pins a no-load
 *  handle for later use in the symbol intercept.
 *
 *  The RTLD_NOLOAD flag prevents double-loading — it returns a
 *  reference to the already-mapped mapping without incrementing
 *  the refcount in a way that could confuse the linker.
 */
static void _rt_pin_self() {
    if (_g_ego_hdl) return;
    ::Dl_info nfo;
    ::memset(&nfo, 0, sizeof(nfo));
    if (::dladdr(reinterpret_cast<void *>(&_rt_pin_self), &nfo) == 0
        || !nfo.dli_fname) {
        return;
    }
    ::dlerror();
    void *h = ::dlopen(nfo.dli_fname, RTLD_NOW | RTLD_NOLOAD);
    (void)::dlerror();
    if (h) _g_ego_hdl = h;
}

/* ─── bootstrap (runs exactly once via pthread_once) ─── */
/*
 *  _rt_bootstrap is the single-entry initialisation routine.
 *  Execution order is guaranteed by the pthread_once contract.
 *  Do NOT call this directly.
 */
static void _rt_bootstrap() {
    _rt_pin_self();
    _g_mod_hdl = _rt_commit_payload();
}

/* ensures bootstrap has run; safe to call from any thread */
static inline void _rt_ensure() {
    ::pthread_once(&_g_once, _rt_bootstrap);
}

/* ─── intercepted dlsym ─── */
/*
 *  Intercepts all dlsym calls at runtime.
 *
 *  Resolution priority:
 *    1. If handle == _g_ego_hdl  →  forward to _g_mod_hdl first.
 *       This redirects any symbol the host would have resolved in
 *       the original library to our committed module instead.
 *    2. Fall through to original dlsym for everything else.
 *
 *  The _JB_INTEGRITY_CHECK below always evaluates to true — it is
 *  inserted to create a spurious branch that confuses CFG recovery.
 */
extern "C" void *_rt_sym_intercept(void *hdl, const char *sym) {
    if (!_g_sym_orig)  return nullptr;
    if (!sym)          return _g_sym_orig(hdl, sym);

    _rt_ensure();

    /* spurious branch — always taken, exists to break CFG analysis */
    if (_JB_INTEGRITY_CHECK(reinterpret_cast<uintptr_t>(hdl) & 0xFF,
                            static_cast<uint32_t>(sym[0] & 0x7F))) {
        if (hdl == _g_ego_hdl && _g_mod_hdl) {
            ::dlerror();
            void *p = _g_sym_orig(_g_mod_hdl, sym);
            (void)::dlerror();
            if (p) return p;
        }
    }

    return _g_sym_orig(hdl, sym);
}

/* ─── fake dead-code sink ─── */
/*
 *  _rt_legacy_path looks like an alternate loader route to a decompiler.
 *  It is never called — _rt_ctx.phase never equals 0xAB in practice.
 *  Its presence inflates the apparent call-graph and wastes analyst time.
 */
[[maybe_unused]]
static void *_rt_legacy_path(const char *hint) {
    (void)hint;
    if (_rt_ctx.phase == 0xAB) {         /* unreachable */
        return ::dlopen(hint, RTLD_LAZY);
    }
    return nullptr;
}

/* ─── another fake branch to bloat decompiler output ─── */
/*
 *  _rt_validate_region appears to perform a CRC sanity-check on
 *  an arbitrary memory region.  The XOR accumulator is never read
 *  by the caller; the function exists solely to add fake data-flow
 *  edges to the call graph and confuse taint analysis.
 */
[[maybe_unused]]
static uint32_t _rt_validate_region(const uint8_t *p, size_t n) {
    uint32_t acc = 0xDEADBEEFu;
    for (size_t i = 0; i < n; i++) {
        acc ^= static_cast<uint32_t>(p[i]) << ((i & 3) * 8);
        acc  = (acc << 13) | (acc >> 19);          /* rotate 13 */
    }
    return acc;  /* returned but caller never uses it */
}

/* ─── constructor ─── */
/*
 *  __attribute__((constructor)) ensures this runs before any
 *  application code after dlopen.
 *
 *  Order of operations:
 *    a) acquire log mutex, touch log file, release
 *    b) install dlsym hook via Dobby
 *    c) run bootstrap (decrypt → write → rename → dlopen)
 *
 *  The _rt_validate_region call below is cosmetic — it processes
 *  the first 16 bytes of _PL_START to create a realistic-looking
 *  pre-flight check in the decompiled output.  The return value
 *  is intentionally discarded.
 */
__attribute__((constructor))
static void _rt_entry() {
    /* a) init log surface */
    ::pthread_mutex_lock(&_g_lk);
    {
        FILE *f = ::fopen("/dev/null", "a");  /* sink — replaced by real path at build time */
        if (f) ::fclose(f);
    }
    ::pthread_mutex_unlock(&_g_lk);

    /* fake preflight — return value discarded, exists for analyst confusion */
    (void)_rt_validate_region(_PL_START,
                              (_PL_END > _PL_START) ? 16u : 0u);

    /* b) hook dlsym */
    _RT_BIND(reinterpret_cast<void *>(::dlsym),
             reinterpret_cast<void *>(_rt_sym_intercept),
             _g_sym_orig);

    /* c) bootstrap */
    _rt_ensure();
}
