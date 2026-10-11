// =====================================================================
// Stage 2 — bitpack / bitunpack   (paper §3.1, uses FastLanes' BP)
//
// ALP's VNOOP output is a vector of int64 ALP-integers.  FFOR (Stage 3)
// turns those into tightly packed bits; these two primitives are what does
// the packing.  They must be exact inverses for every width 0..64.
// =====================================================================

#include <cstdint>
#include <random>

#include "../include/ALP.h"
#include "harness.hpp"

namespace alp::test {

// T1 — roundtrip across every width 0..64, random values.
inline void stage2_t1_roundtrip_all_widths() {
    section("T1. Random roundtrip for every width 0..64");
    std::mt19937_64                              rng(4242);
    std::uniform_int_distribution<uint64_t>      dist;
    int                                         failed_widths = 0;
    char                                        first_bad[256] = {0};

    for (uint8_t w = 0; w <= 64; w++) {
        const uint64_t mask = (w == 64) ? ~0ULL : ((1ULL << w) - 1);
        std::vector<uint64_t> src(V), rt(V);
        for (size_t i = 0; i < V; i++) src[i] = dist(rng) & mask;

        std::vector<uint8_t> packed(V * 64 / 8 + 16, 0xAA);
        const size_t nbytes = alp::bitpack(src.data(), w, V, packed.data());
        alp::bitunpack(packed.data(), w, V, rt.data());

        for (size_t i = 0; i < V; i++) {
            if (src[i] != rt[i]) {
                if (!first_bad[0]) {
                    snprintf(first_bad, sizeof first_bad,
                             "width %d: [%zu] packed=%llu unpacked=%llu",
                             w, i, (unsigned long long)src[i],
                             (unsigned long long)rt[i]);
                }
                failed_widths++;
                break;
            }
        }
        (void)nbytes;
    }
    if (failed_widths == 0) {
        check("all 65 widths roundtrip exactly", true, "widths 0..64 clean");
    } else {
        check("all 65 widths roundtrip exactly", false, first_bad);
    }
}

// T2 — exact byte accounting. 1024 values at width w occupy exactly 128*w bytes.
inline void stage2_t2_byte_count() {
    section("T2. Byte accounting: bytes(1024, w) must equal 1024*w/8");
    int  bad = 0;
    char detail[256] = {0};
    std::vector<uint64_t> src(V, 0xFFFFFFFFFFFFFFFFULL);
    std::vector<uint8_t>  packed(V * 64 / 8 + 16, 0);

    for (uint8_t w = 0; w <= 64; w++) {
        const size_t want = V * w / 8;
        const size_t got  = alp::bitpack(src.data(), w, V, packed.data());
        if (got != want) {
            if (!bad) {
                snprintf(detail, sizeof detail,
                         "width %d: got %zu bytes, want %zu", w, got, want);
            }
            bad++;
        }
    }
    check("every width writes exactly 1024*w/8 bytes", bad == 0,
          bad ? detail : "widths 0..64 all exact");
}

// T3 — adversarial patterns.  A correct packer handles these without
//      depending on neighbouring values.
inline void stage2_t3_patterns() {
    section("T3. Adversarial value patterns");
    struct Pattern { const char *name; size_t kind; };
    const Pattern pats[] = {
        {"all zeros", 0}, {"all ones", 1}, {"ascending", 2},
        {"descending", 3}, {"alternating 0/max", 4}, {"one hot low", 5},
        {"one hot high", 6}, {"single value", 7},
    };
    int bad = 0;
    char detail[256] = {0};

    for (const auto &p : pats) {
        for (uint8_t w : {1, 3, 7, 8, 9, 15, 16, 17, 32, 63, 64}) {
            const uint64_t mask = (w == 64) ? ~0ULL : ((1ULL << w) - 1);
            std::vector<uint64_t> src(V);
            for (size_t i = 0; i < V; i++) {
                switch (p.kind) {
                case 0: src[i] = 0; break;
                case 1: src[i] = mask; break;
                case 2: src[i] = (i & mask); break;
                case 3: src[i] = ((V - i) & mask); break;
                case 4: src[i] = (i & 1) ? mask : 0; break;
                case 5: src[i] = (i == 0) ? 1 : 0; break;
                case 6: src[i] = (i == 0) ? (1ULL << (w - 1)) : 0; break;
                case 7: src[i] = 42; break;
                }
            }
            std::vector<uint64_t> rt(V);
            std::vector<uint8_t>  packed(V * 8 + 16, 0);
            alp::bitpack(src.data(), w, V, packed.data());
            alp::bitunpack(packed.data(), w, V, rt.data());
            for (size_t i = 0; i < V; i++) {
                if (src[i] != rt[i]) {
                    if (!bad) {
                        snprintf(detail, sizeof detail,
                                 "%s @ width %d: [%zu] %llu != %llu", p.name, w, i,
                                 (unsigned long long)src[i], (unsigned long long)rt[i]);
                    }
                    bad++;
                    break;
                }
            }
        }
    }
    check("all adversarial patterns roundtrip", bad == 0,
          bad ? detail : "8 patterns x 11 widths clean");
}

// T4 — width 0 is the degenerate case: everything is zero, nothing is written.
inline void stage2_t4_zero_width() {
    section("T4. Edge case: width 0");
    std::vector<uint64_t> src(V, 0xDEADBEEF), rt(V, 1);
    std::vector<uint8_t>  packed(64, 0xAA);
    const size_t nbytes  = alp::bitpack(src.data(), 0, V, packed.data());
    alp::bitunpack(packed.data(), 0, V, rt.data());
    bool all_zero = true;
    for (size_t i = 0; i < V; i++) if (rt[i] != 0) all_zero = false;
    check("width 0 writes 0 bytes", nbytes == 0, "");
    check("width 0 unpacks to all zeros", all_zero, "");
}

// T5 — width 64 must not lose the top bit.
inline void stage2_t5_full_width() {
    section("T5. Edge case: width 64 (MSB must survive)");
    std::vector<uint64_t> src = {0x8000000000000000ULL, ~0ULL, 0ULL, 1ULL};
    std::vector<uint64_t> rt(4, 0);
    std::vector<uint8_t>  packed(64, 0);
    alp::bitpack(src.data(), 64, 4, packed.data());
    alp::bitunpack(packed.data(), 64, 4, rt.data());
    bool ok = rt[0] == src[0] && rt[1] == src[1] && rt[2] == src[2] && rt[3] == src[3];
    check("top bit survives width 64", ok, ok ? "0x8000...0000 roundtrips" : "MSB lost");
}

// T6 — consecutive vectors must stay byte-aligned and independent.
inline void stage2_t6_vector_boundaries() {
    section("T6. Back-to-back vectors stay byte-aligned and independent");
    std::mt19937_64                         rng(7);
    std::uniform_int_distribution<uint64_t> dist;
    const uint8_t w1 = 5, w2 = 13, w3 = 3;
    std::vector<uint64_t> a(V), b(V), c(V);
    for (size_t i = 0; i < V; i++) {
        a[i] = dist(rng) & 31; b[i] = dist(rng) & 8191; c[i] = dist(rng) & 7;
    }
    const size_t b1 = V * w1 / 8, b2 = V * w2 / 8, b3 = V * w3 / 8;
    std::vector<uint8_t> buf(b1 + b2 + b3 + 32, 0);
    alp::bitpack(a.data(), w1, V, buf.data());
    alp::bitpack(b.data(), w2, V, buf.data() + b1);
    alp::bitpack(c.data(), w3, V, buf.data() + b1 + b2);

    std::vector<uint64_t> ra(V), rb(V), rc(V);
    alp::bitunpack(buf.data(), w1, V, ra.data());
    alp::bitunpack(buf.data() + b1, w2, V, rb.data());
    alp::bitunpack(buf.data() + b1 + b2, w3, V, rc.data());

    bool ok = true;
    for (size_t i = 0; i < V; i++) {
        if (ra[i] != a[i] || rb[i] != b[i] || rc[i] != c[i]) ok = false;
    }
    check("three vectors of different widths unpack independently", ok,
          ok ? "5/13/3 bits, all clean" : "cross-contamination between vectors");
}

inline void stage2_tests() {
    banner("Stage 2 — bitpack / bitunpack   (widths 0..64)");
    note("1024 values at width w must occupy exactly 1024*w/8 bytes.");
    stage2_t1_roundtrip_all_widths();
    stage2_t2_byte_count();
    stage2_t3_patterns();
    stage2_t4_zero_width();
    stage2_t5_full_width();
    stage2_t6_vector_boundaries();
}

} // namespace alp::test