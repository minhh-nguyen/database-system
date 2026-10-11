#include "../include/constants.hpp"

// Empty. Stage 1 goes here: fast_double_round.
static inline int64_t fast_double_round(double d) {
    return (d + alp::Constants<double>::MAGIC_NUMBER) -
           alp::Constants<double>::MAGIC_NUMBER;
}

int main() {
    return 0;
}
