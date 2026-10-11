#pragma once
// =====================================================================
// Shared test harness. Header-only, no linkage needed.
// =====================================================================

#include <cinttypes>
#include <cmath>
#include <cstdarg>
#include <cstdint>
#include <cstdio>

#include "../include/config.hpp"

namespace alp::test {

/// ALP vector size, shared by every stage's tests.
static constexpr size_t V = alp::config::VECTOR_SIZE; // 1024

struct Result {
    int passed = 0;
    int failed = 0;
};

inline Result& result() {
    static Result r;
    return r;
}

inline void reset() { result() = Result{}; }

inline void banner(const char* title) {
    printf("\n============================================================\n");
    printf("  %s\n", title);
    printf("============================================================\n");
}

inline void check(const char* name, bool ok, const char* detail = "") {
    printf("  [%s] %-44s %s\n", ok ? "PASS" : "FAIL", name, detail);
    ok ? ++result().passed : ++result().failed;
}

inline void section(const char* title) { printf("\n-- %s\n", title); }

inline void note(const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    printf("     ");
    vprintf(fmt, ap);
    printf("\n");
    va_end(ap);
}

/// Correctly-rounded-to-nearest-even. This is the ground truth that
/// fast_double_round reproduces using only one add and one subtract.
inline double oracle(double d) { return std::nearbyint(d); }

/// Rotating buffers, so several calls can appear inside one printf.
inline const char *f64(double v) {
    static char bufs[8][48];
    static int  n = 0;
    char       *b = bufs[n = (n + 1) & 7];
    snprintf(b, 48, "%.17g", v);
    return b;
}

inline const char *i64(int64_t v) {
    static char bufs[8][32];
    static int  n = 0;
    char       *b = bufs[n = (n + 1) & 7];
    snprintf(b, 32, "%" PRId64, v);
    return b;
}

inline int summary(int stage) {
    printf("\n------------------------------------------------------------\n");
    printf("  Stage %d:   passed %d   failed %d\n", stage,
           result().passed, result().failed);
    printf("------------------------------------------------------------\n");
    return result().failed == 0 ? 0 : 1;
}

} // namespace alp::test