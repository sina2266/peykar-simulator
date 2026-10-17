//
// Created by cbontempi on 4/16/2026.
//
#include <iostream>

// Forward declarations of test functions
extern bool test_statstest_test1();
extern bool test_multitrialstats_test1();
extern bool test_multitrialstats_testGaussian();

int main(int argc, char** argv) {
    int failed = 0;

    if (!test_statstest_test1()) failed++;
    if (!test_multitrialstats_test1()) failed++;
    if (!test_multitrialstats_testGaussian()) failed++;

    if (failed == 0) {
        std::cout << "\nAll tests passed!" << std::endl;
        return 0;
    } else {
        std::cerr << "\n" << failed << " test(s) failed!" << std::endl;
        return 1;
    }
}