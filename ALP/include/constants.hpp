#ifndef ALP_CONSTANTS_HPP
#define ALP_CONSTANTS_HPP

#include <cstdint>

namespace alp {

template <class T> struct Constants {};

template <> struct Constants<double> {
    /// 10^e for e in [0, MAX_EXPONENT]; ALP never divides by 10, so no FACT_ARR
    /// is needed. The paper derives a search space of 0 <= e <= 21 with f <= e
    /// (253 combinations); the reference implementation caps at 18. Raise this to
    /// 21 to follow the paper exactly.
    static inline constexpr uint8_t MAX_EXPONENT = 18;

    /// 2^51 + 2^52 -- used by fast_double_round. Adding then subtracting this
    /// pushes a double into the [2^52, 2^53) range where it has no fractional
    /// part, so the subtraction rounds it to the nearest integer.
    static inline constexpr double MAGIC_NUMBER = 6755399441055744.0;

    /// An exception costs 64 bits of value + 16 bits of position (paper, §3.1).
    static inline constexpr uint8_t EXCEPTION_POSITION_SIZE = 16;
    static inline constexpr uint8_t EXCEPTION_POSITION_SIZE_B =
        EXCEPTION_POSITION_SIZE / 8;

    /// F10[e] = 10^e
    static inline constexpr double F10[] = {1.0,
                                            10.0,
                                            100.0,
                                            1000.0,
                                            10000.0,
                                            100000.0,
                                            1000000.0,
                                            10000000.0,
                                            100000000.0,
                                            1000000000.0,
                                            10000000000.0,
                                            100000000000.0,
                                            1000000000000.0,
                                            10000000000000.0,
                                            100000000000000.0,
                                            1000000000000000.0,
                                            10000000000000000.0,
                                            100000000000000000.0,
                                            1000000000000000000.0};

    /// i_F10[f] = 10^-f
    static inline constexpr double i_F10[] = {1.0,
                                              0.1,
                                              0.01,
                                              0.001,
                                              0.0001,
                                              0.00001,
                                              0.000001,
                                              0.0000001,
                                              0.00000001,
                                              0.000000001,
                                              0.0000000001,
                                              0.00000000001,
                                              0.000000000001,
                                              0.0000000000001,
                                              0.00000000000001,
                                              0.000000000000001,
                                              0.0000000000000001,
                                              0.00000000000000001,
                                              0.000000000000000001};
};

} // namespace alp

#endif
