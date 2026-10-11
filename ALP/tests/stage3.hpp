// =====================================================================
// Stage 3 — FFOR, Fused Frame-Of-Reference   (paper §3.1)
//
//   "The FOR encoding subtracts the minimum value of the integers in a
//    vector; this will pick up on localized doubles (inside a tight range)
//    and reduce bits needed in the subsequent bit-packing.  Fusing saves a
//    SIMD store and load instruction in between the subtraction and the
//    bit-packing loop."
//
// One kernel does: find base = min, find bitwidth, subtract, bitpack.
// One kernel undoes it: bitunpack, add base.
//
// §3.1 caps the width at config::CUTTING_LIMIT = 16.  A vector whose values
// span more than 16 bits after subtraction cannot be represented by FFOR.
// =====================================================================

#include <algorithm>
#include <cstdint>
#include <random>

#include "../include/ALP.hpp"
#include "harness.hpp"

namespace alp::test {

// T1 — roundtrip on realistic ALP-integer vectors.
inline void stage3_t1_roundtrip() {
    section("T1. FFOR roundtrip on ALP-shaped vectors");
    struct Case { const char *name; int64_t lo; int64_t span; };
    const Case cases[] = {
        {"money-like (span 2^14)", 0, 1 << 14},
        {"tight (span 2^10)", 0, 1 << 10},
        {"negative base", -500000, 1 << 12},
        {"small ints", 0, 1024},
    };
    std::mt19937_64                         rng(31337);
    std::uniform_int_distribution<int64_t>  dist;
    int                                     bad = 0;
    char                                    detail[256] = {0};

    for (const auto &c : cases) {
        std::vector<int64_t> src(V), out(V);
        for (size_t i = 0; i < V; i++) src[i] = c.lo + (dist(rng) % c.span);

        int64_t  base     = 0;
        uint8_t  bitwidth = 0;
        std::vector<uint8_t> packed(V * 64 / 8 + 16, 0);
        const size_t nbytes = alp::FFOR(src.data(), V, base, bitwidth, packed.data());
        if (nbytes == 0) { bad++; continue; }
        alp::UNFFOR(packed.data(), V, base, bitwidth, out.data());

        for (size_t i = 0; i < V; i++) {
            if (src[i] != out[i]) {
                if (!bad) {
                    snprintf(detail, sizeof detail,
                             "%s: [%zu] %lld != %lld", c.name, i,
                             (long long)src[i], (long long)out[i]);
                }
                bad++;
                break;
            }
        }
    }
    check("FFOR roundtrips every case", bad == 0,
          bad ? detail : "4 vector shapes clean");
}

// T2 — base must be the minimum, so every (v - base) is non-negative.
inline void stage3_t2_base_is_min() {
    section("T2. base == min(vector)");
    std::mt19937_64 rng(555);
    std::uniform_int_distribution<int64_t> dist(0, 1 << 20);
    std::vector<int64_t> src(V);
    for (size_t i = 0; i < V; i++) src[i] = dist(rng);
    const int64_t true_min = *std::min_element(src.begin(), src.end());

    int64_t  base = 0;
    uint8_t  bw   = 0;
    std::vector<uint8_t> packed(V * 8, 0);
    alp::FFOR(src.data(), V, base, bw, packed.data());

    char detail[160];
    snprintf(detail, sizeof detail, "base=%lld, true min=%lld",
             (long long)base, (long long)true_min);
    check("base is exactly the minimum", base == true_min, detail);
}

// T3 — degenerate vectors need zero bits.
inline void stage3_t3_zero_bits() {
    section("T3. All-identical vector compresses to zero bits");
    std::vector<int64_t> src(V, 7);
    std::vector<int64_t> zeros(V, 0);
    std::vector<uint8_t> packed(64, 0xAA);
    int64_t base = -1;
    uint8_t bw   = 255;

    const size_t n = alp::FFOR(src.data(), V, base, bw, packed.data());
    check("constant vector -> bitwidth 0", bw == 0, "");
    check("constant vector -> 0 bytes", n == 0, "");

    int64_t b2 = -1;
    uint8_t w2 = 255;
    alp::FFOR(zeros.data(), V, b2, w2, packed.data());
    check("all-zero vector -> bitwidth 0", w2 == 0, "");
}

// T4 — bitwidth must be the tightest that fits, and never exceed CUTTING_LIMIT.
inline void stage3_t4_bitwidth_selection() {
    section("T4. Bitwidth selection");
    std::vector<uint8_t> packed(V * 8, 0);

    struct Case { const char *name; int64_t value; uint8_t want; };
    const Case cases[] = {
        {"all 0",      0,      0},
        {"all 1",      1,      1},
        {"all 255",    255,    8},
        {"all 256",    256,    9},
        {"all 65535",  65535,  16},
    };
    int bad = 0;
    char detail[256] = {0};
    for (const auto &c : cases) {
        std::vector<int64_t> src(V, c.value);
        int64_t base = 0; uint8_t bw = 255;
        alp::FFOR(src.data(), V, base, bw, packed.data());
        if (bw != c.want) {
            if (!bad) snprintf(detail, sizeof detail,
                               "%s: bitwidth %d, want %d", c.name, bw, c.want);
            bad++;
        }
    }
    check("bitwidth is the tightest that fits", bad == 0,
          bad ? detail : "0/1/8/9/16 as expected");
}

// T5 — vectors wider than CUTTING_LIMIT cannot be represented.
inline void stage3_t5_cutting_limit() {
    section("T5. Values spanning more than CUTTING_LIMIT bits");
    std::vector<int64_t> src(V, 1 << 20);
    src[0] = -(1LL << 40); // huge span, far beyond 16 bits
    int64_t base = 0;
    uint8_t bw   = 0;
    std::vector<uint8_t> packed(V * 8, 0);

    const size_t n = alp::FFOR(src.data(), V, base, bw, packed.data());
    char detail[200];
    snprintf(detail, sizeof detail,
             "FFOR returned %zu bytes (must be 0 = failure signal)", n);
    check("returns 0 bytes when the vector is too wide", n == 0, detail);
}

// T6 — the ALP payoff: a realistic money column.
inline void stage3_t6_realistic() {
    section("T6. Realistic: 1024 prices in [0, 100000)");
    std::mt19937_64                        rng(2024);
    std::uniform_int_distribution<int64_t> dist(0, 99999);
    std::vector<int64_t> src(V), out(V);
    for (size_t i = 0; i < V; i++) src[i] = dist(rng);

    int64_t base = 0;
    uint8_t bw   = 0;
    std::vector<uint8_t> packed(V * 8, 0);
    const size_t nbytes = alp::FFOR(src.data(), V, base, bw, packed.data());
    alp::UNFFOR(packed.data(), V, base, bw, out.data());

    bool ok = true;
    for (size_t i = 0; i < V; i++) if (src[i] != out[i]) ok = false;

    const double bpv = (double)nbytes * 8 / (double)V;
    note("base=%lld  bitwidth=%d  bytes=%zu  ->  %.2f bits/value (from 64)",
         (long long)base, bw, nbytes, bpv);
    check("roundtrip is exact", ok, "");
    check("bitwidth <= CUTTING_LIMIT (16)", bw <= alp::config::CUTTING_LIMIT, "");
}

inline void stage3_tests() {
    banner("Stage 3 — FFOR, Fused Frame-Of-Reference   (paper §3.1)");
    note("FFOR subtracts the min, then bit-packs; CUTTING_LIMIT = %d.",
         alp::config::CUTTING_LIMIT);
    stage3_t1_roundtrip();
    stage3_t2_base_is_min();
    stage3_t3_zero_bits();
    stage3_t4_bitwidth_selection();
    stage3_t5_cutting_limit();
    stage3_t6_realistic();
}

} // namespace alp::test