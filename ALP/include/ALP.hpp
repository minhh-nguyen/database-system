#pragma once
// ALP: Adaptive Lossless floating-Point Compression
// Exercise scaffold.
//
// The invariant that governs every stage:
//
//   decoded == d * F10[factor] * i_F10[exponent]   exactly, bitwise
//
// for values that are not exceptions. Not approximately. Same 64 bits.
//
// Stages:
//   1. fast_double_round            (§3.1 "Fast Rounding")        [DONE]
//   2. bitpack / bitunpack          (widths 0..64)
//   3. FFOR                         (§3.1 "Fused Frame-Of-Reference")
//   4. encode / decode              (Algorithms 1 & 2)
//   5. adaptive sampling            (§3.2)
//   6. benchmark                    (target: ~21.7 bits/value, Table 4)
//
// Doubles only. ALP_RD (§3.4) is out of scope.

#include "Vector.hpp"
#include "config.hpp"
#include "constants.hpp"
#include <cstddef>
#include <cstdint>

namespace alp {

/// Paper §3.1 "Fast Rounding". See src/ALP.cpp for the precondition.
int64_t fast_double_round(double d);

// ---------------------------------------------------------------------
// Stage 2 — bit packing
//
// Pack `count` values, each `bitwidth` bits wide, into `out` (a byte
// buffer).  Returns the number of bytes written, which is always exactly
// count*bitwidth/8.
//
// Contract:
//   * width 0  -> writes nothing, unpacks to all zeros
//   * width 64 -> must preserve the top bit
//   * values must already fit in `bitwidth` bits
// ---------------------------------------------------------------------
size_t bitpack(const uint64_t *in, uint8_t bitwidth, size_t count, uint8_t *out);
size_t bitunpack(const uint8_t *in, uint8_t bitwidth, size_t count, uint64_t *out);

// ---------------------------------------------------------------------
// Stage 3 — FFOR (Fused Frame-Of-Reference)
//
// Subtract the minimum, then bit-pack, in one kernel.
// Returns bytes written, or 0 if the values span more than
// config::CUTTING_LIMIT bits after subtraction (FFOR cannot represent it;
// Stage 4 decides what to do instead).
// ---------------------------------------------------------------------
size_t FFOR(const int64_t *in, size_t count, int64_t &base, uint8_t &bitwidth,
            uint8_t *out);
size_t UNFFOR(const uint8_t *in, size_t count, int64_t base, uint8_t bitwidth,
              int64_t *out);

template <class T> class ALP {
public:
    // TODO Stage 4: encode one vector (Algorithm 1); decode (Algorithm 2).
    // TODO Stage 5: adaptive sampling (§3.2).
    // TODO Stage 6: benchmark (target ~21.7 bits/value, Table 4).
};

} // namespace alp
