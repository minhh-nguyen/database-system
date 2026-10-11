#include <cstdio>
#include <cstdlib>
#include <vector>

#include "../tests/harness.hpp"
#include "../tests/stage1.hpp"
#include "../tests/stage2.hpp"
#include "../tests/stage3.hpp"

#ifndef ALP_CURRENT_STAGE
#define ALP_CURRENT_STAGE 3
#endif

static constexpr int STAGES[] = {1, 2, 3};
static constexpr int N_STAGES = sizeof(STAGES) / sizeof(STAGES[0]);

static void run_stage(int stage) {
    alp::test::reset();
    switch (stage) {
    case 1:
        alp::test::stage1_tests();
        break;
    case 2:
        alp::test::stage2_tests();
        break;
    case 3:
        alp::test::stage3_tests();
        break;
    default:
        std::fprintf(stderr, "stage %d has no tests yet (highest is %d)\n", stage,
                     ALP_CURRENT_STAGE);
        std::exit(2);
    }
}

int main(int argc, char **argv) {
    // No arguments -> run every implemented stage.
    std::vector<int> stages;
    for (int i = 1; i < argc; i++)
        stages.push_back(std::atoi(argv[i]));
    if (stages.empty()) {
        for (int i = 0; i < N_STAGES; i++)
            stages.push_back(STAGES[i]);
    }

    int total_failed = 0;
    for (int s : stages) {
        run_stage(s);
        alp::test::summary(s);
        total_failed += alp::test::result().failed;
    }

    if (total_failed) {
        std::fprintf(stderr, "\n==> %d assertion(s) failed\n", total_failed);
        return 1;
    }
    return 0;
}
