#pragma once
// ALP: Adaptive Lossless floating-Point Compression
// Exercise scaffold. Nothing here is implemented.
//
// The invariant that governs every stage:
//
//   decoded == d * F10[factor] * i_F10[exponent]   exactly, bitwise
//
// for values that are not exceptions. Not approximately. Same 64 bits.
//
// Stages:
//   1. fast_double_round            (§3.1 "Fast Rounding")
//   2. bitpack / bitunpack          (widths 0..64)
//   3. FFOR                         (§3.1 "Fused Frame-Of-Reference")
//   4. encode / decode              (Algorithms 1 & 2)
//   5. adaptive sampling            (§3.2)
//   6. benchmark                    (target: ~21.7 bits/value, Table 4)
//
// Doubles only. ALP_RD (§3.4) is out of scope.

#include "Vector.h"
#include "config.hpp"
#include "constants.hpp"
#include <cstdint>

namespace alp {

template <class T> class ALP {
public:
    // TODO Stage 4: encode one vector of config::VECTOR_SIZE values.
    //   Algorithm 1. Returns total bits written.
    // TODO Stage 4: decode. Algorithm 2. Must be exact.
};

} // namespace alp
