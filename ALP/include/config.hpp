#ifndef ALP_CONFIG_HPP
#define ALP_CONFIG_HPP

#include <cstddef>

/*
 * ALP Configs
 */
namespace alp {
namespace config {
/// ALP Vector size (We recommend against changing this; it should be constant)
inline constexpr size_t VECTOR_SIZE = 1024;
/// number of vectors per rowgroup
inline constexpr size_t N_VECTORS_PER_ROWGROUP = 100UL;
/// Rowgroup size
inline constexpr size_t ROWGROUP_SIZE = N_VECTORS_PER_ROWGROUP * VECTOR_SIZE;
/// Vectors from the rowgroup from which to take samples; this will be used to then
/// calculate the jumps
inline constexpr size_t ROWGROUP_VECTOR_SAMPLES = 8;
/// We calculate how many equidistant vector we must jump within a rowgroup
inline constexpr size_t ROWGROUP_SAMPLES_JUMP =
    (ROWGROUP_SIZE / ROWGROUP_VECTOR_SAMPLES) / VECTOR_SIZE;
/// Values to sample per vector (rho in the paper)
inline constexpr size_t SAMPLES_PER_VECTOR = 32;
/// Maximum number of combinations (K) obtained from row group sampling
inline constexpr size_t MAX_K_COMBINATIONS = 5;
/// FFOR will not use a bit-width wider than this; ALP falls back to raw if a vector needs
/// more.
inline constexpr size_t CUTTING_LIMIT = 16;
/// Greedy early exit for level-2 sampling: if candidates i+1 and i+2 are both
/// no better than candidate i, stop searching (paper, §3.2).
inline constexpr size_t SAMPLING_EARLY_EXIT_THRESHOLD = 2;
/// fast_double_round is only valid while |d| < 2^52. Outside this range the
/// add/subtract trick stops rounding to the nearest integer.
inline constexpr double ENCODING_UPPER_LIMIT = 9223372036854774784.0;
inline constexpr double ENCODING_LOWER_LIMIT = -9223372036854774784.0;
} // namespace config
} // namespace alp

#endif
