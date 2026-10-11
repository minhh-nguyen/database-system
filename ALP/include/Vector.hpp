#ifndef ALP_VECTOR_HPP
#define ALP_VECTOR_HPP

// ALP Vector header — you design this.
//
// §3.1: the exponent, factor and bit-width are stored ONCE per vector (1024
// values), not per value. That amortization is the entire point: it is what
// makes per-vector adaptivity free compared to PDE's per-value exponent.
//
// What it must carry:
//   - the ALP integers (VNOOP output), int64_t
//   - exception positions, 16-bit (§3.1: 80 bits overhead per exception =
//     64 for the value + 16 for the position)
//   - the original doubles for those positions, uncompressed
//   - exponent, factor, bitwidth
//
// Not decided for you: layout, field names, fixed vs. std::vector.

#endif
