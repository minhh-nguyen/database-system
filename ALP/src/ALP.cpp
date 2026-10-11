#include "../include/ALP.hpp"
#include <cstdio>

namespace alp {

// Round d to the nearest integer using only an add and a subtract.
// Adding then subtracting MAGIC = 2^51 + 2^52 moves d into [2^52, 2^53),
// where doubles have no fractional part, so the subtraction rounds implicitly.
// SIMD-friendly: no round() instruction exists in AVX / NEON / SVE.
//
// Precondition: |d| < 2^51.  Beyond that, d + MAGIC exceeds 2^53 and the add
// rounds to a coarser grid than "nearest integer" (off by that binade's spacing).
int64_t fast_double_round(double d) {
    return (d + Constants<double>::MAGIC_NUMBER) - Constants<double>::MAGIC_NUMBER;
}

// =====================================================================
// Stage 2 / Stage 3 — NOT IMPLEMENTED YET.
//
// These are no-op stubs so the test binary links.  Every call prints a
// one-time warning and returns a harmless value, so the tests fail with
// readable assertions instead of crashing the whole run.
// =====================================================================
namespace {

void todo(const char *what) {
    static const char *last = nullptr;
    if (last != what) {
        std::fprintf(stderr, "  [TODO] %s is not implemented yet (Stage %s)\n", what,
                     what);
        last = what;
    }
}

} // namespace

size_t bitpack(const uint64_t *, uint8_t bitwidth, size_t count, uint8_t *) {
    todo("bitpack");
    return count * bitwidth / 8;
}

size_t bitunpack(const uint8_t *, uint8_t, size_t count, uint64_t *out) {
    todo("bitunpack");
    for (size_t i = 0; i < count; i++)
        out[i] = 0;
    return 0;
}

size_t FFOR(const int64_t *, size_t, int64_t &base, uint8_t &bitwidth, uint8_t *) {
    todo("FFOR");
    base = 0;
    bitwidth = 0;
    return 0;
}

size_t UNFFOR(const uint8_t *, size_t count, int64_t, uint8_t, int64_t *out) {
    todo("UNFFOR");
    for (size_t i = 0; i < count; i++)
        out[i] = 0;
    return 0;
}

} // namespace alp
