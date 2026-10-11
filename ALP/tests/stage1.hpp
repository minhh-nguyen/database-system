#pragma once
// =====================================================================
// Stage 1 — fast_double_round   (paper §3.1 "Fast Rounding")
//
// The invariant under test:
//     decoded == d * F10[factor] * i_F10[exponent]   exactly, bitwise
// fast_double_round exists so the encoder can produce that integer d
// using only an add and a subtract — no round() instruction exists in
// AVX / NEON / SVE.
// =====================================================================

#include <cmath>
#include <cstdint>
#include <random>

#include "../include/ALP.h"
#include "harness.hpp"

namespace alp::test {

// T1 — behaviour on values where the trick must work.
inline void stage1_t1_basics() {
    section("T1. Basics (|d| well inside the valid range)");
    struct {
        double      in;
        double      want;
        const char *note;
    } cases[] = {
        {0.0, 0.0, "zero"},
        {-0.0, 0.0, "sign of negative zero is LOST"},
        {0.5, 0.0, "half-to-even -> 0"},
        {1.5, 2.0, "half-to-even -> 2"},
        {2.5, 2.0, "half-to-even -> 2 (not 3)"},
        {3.5, 4.0, "half-to-even -> 4"},
        {-0.5, -0.0, "half-to-even -> 0"},
        {-1.5, -2.0, "half-to-even -> -2"},
        {-2.5, -2.0, "half-to-even -> -2 (not -3)"},
        {1.0, 1.0, "already integral"},
        {-1.0, -1.0, "already integral"},
        {8.0605, 8.0, "the paper's §2.5 example"},
        {123456789.25, 123456789.0, "large-ish"},
    };
    for (auto &c : cases) {
        const double got = (double)alp::fast_double_round(c.in);
        char        detail[256];
        snprintf(detail, sizeof detail, "fdr(%s) = %-22s // %s",
                 f64(c.in), f64(got), c.note);
        check("rounds correctly", got == c.want && got == oracle(c.in), detail);
    }
}

// T2 — random sweep against the oracle, inside the safe range.
inline void stage1_t2_random_sweep() {
    section("T2. Random sweep in [-1e6, 1e6] vs std::nearbyint");
    std::mt19937_64                    rng(12345);
    std::uniform_real_distribution<double> dist(-1e6, 1e6);
    int                                bad = 0, total = 100000;
    double                             first_in = 0, first_got = 0;
    for (int i = 0; i < total; i++) {
        const double d   = dist(rng);
        const double got = (double)alp::fast_double_round(d);
        if (got != oracle(d) || got != std::floor(got)) {
            if (bad == 0) {
                first_in  = d;
                first_got = got;
            }
            bad++;
        }
    }
    char detail[256];
    if (bad == 0) {
        snprintf(detail, sizeof detail, "%d/%d values, 0 mismatches", total, total);
    } else {
        snprintf(detail, sizeof detail, "%d/%d mismatches; first in=%s got=%s want=%s",
                 bad, total, f64(first_in), f64(first_got), f64(oracle(first_in)));
    }
    check("every value rounds correctly", bad == 0, detail);
}

// T3 — THE PRECONDITION. Where does the trick stop working?
//     Answers: valid while |d| < 2^51, NOT 2^52.
inline void stage1_t3_precondition() {
    section("T3. Precondition: scan exponents, count mismatches vs oracle");
    note("%-4s %-15s %-10s %s", "k", "range", "mismatches", "verdict");

    std::mt19937_64 rng(999);
    int             last_contiguous_ok = -1, broke = 0;

    for (int k = 45; k <= 62; k++) {
        const double lo = std::ldexp(1.0, k);
        const double hi = std::ldexp(1.0, k + 1);
        std::uniform_real_distribution<double> dist(lo, hi);
        int    bad = 0, n = 2000;
        double worst_in = 0, worst_got = 0;
        for (int i = 0; i < n; i++) {
            const double d   = dist(rng);
            const double got = (double)alp::fast_double_round(d);
            if (got != oracle(d)) {
                if (bad == 0) {
                    worst_in  = d;
                    worst_got = got;
                }
                bad++;
            }
        }
        char row[300];
        if (bad) {
            snprintf(row, sizeof row,
                     "%-4d [2^%d,2^%d)   %-10d BREAKS  in=%s got=%s want=%s off=%g",
                     k, k, k + 1, bad, f64(worst_in), f64(worst_got),
                     f64(oracle(worst_in)), worst_got - oracle(worst_in));
        } else {
            snprintf(row, sizeof row, "%-4d [2^%d,2^%d)   %-10d ok", k, k, k + 1, bad);
        }
        printf("  %s\n", row);
        if (bad == 0 && !broke) last_contiguous_ok = k;
        if (bad != 0 && !broke)  broke = 1;
    }

    note("");
    note("Last exponent with ZERO mismatches (contiguous): 2^%d = %s",
         last_contiguous_ok, f64(std::ldexp(1.0, last_contiguous_ok)));
    note("The 'off by' value always equals that binade's spacing 2^(k-52):");
    note("once d + MAGIC exceeds 2^53 the add rounds to a coarser grid.");

    // A concrete, reproducible value that is NOT correctly rounded.
    const double broken = 11258999068426242.0; // 2^53 + 2^51 + 2
    const double got    = (double)alp::fast_double_round(broken);
    char        detail[300];
    snprintf(detail, sizeof detail, "fdr(%s) = %s, want %s, off by %g",
             f64(broken), f64(got), f64(oracle(broken)), got - oracle(broken));
    check("produces a real, reproducible failure", got != oracle(broken), detail);
}

// T4 — WHY this function exists: the paper's §2.6 worked example.
inline void stage1_t4_paper_example() {
    section("T4. Paper §2.6 worked example (v = 8.0605, e = 14, f = 10)");
    static const double F10[]   = {1e0,  1e1,  1e2,  1e3,  1e4,  1e5,  1e6,  1e7,  1e8,
                                   1e9,  1e10, 1e11, 1e12, 1e13, 1e14, 1e15, 1e16, 1e17, 1e18};
    static const double i_F10[] = {1e0,  1e-1, 1e-2, 1e-3, 1e-4, 1e-5, 1e-6, 1e-7, 1e-8,
                                   1e-9, 1e-10, 1e-11, 1e-12, 1e-13, 1e-14, 1e-15, 1e-16,
                                   1e-17, 1e-18};
    const int    e = 14, f = 10;
    const double v = 8.0605;

    const int64_t d    = alp::fast_double_round(v * F10[e] * i_F10[f]);
    const double  back = (double)d * F10[f] * i_F10[e];

    note("d = (int64)round(v * F10[e] * i_F10[f])  = %s", i64(d));
    note("decoded = d * F10[f] * i_F10[e]          = %s", f64(back));
    note("original v                               = %s", f64(v));

    // The paper's point: 8.0605 is not a real double. It must round-trip to
    // the stored double 8.06049999999999933209, not to the decimal literal.
    check("VNOOP / VNOOP_R round-trip is bit-exact", back == v);
}

inline void stage1_tests() {
    banner("Stage 1 — fast_double_round   (paper §3.1)");
    note("MAGIC_NUMBER in constants.hpp = %s", f64(alp::Constants<double>::MAGIC_NUMBER));
    note("2^51 + 2^52                     = %s", f64(6755399441055744.0));
    stage1_t1_basics();
    stage1_t2_random_sweep();
    stage1_t3_precondition();
    stage1_t4_paper_example();
}

} // namespace alp::test