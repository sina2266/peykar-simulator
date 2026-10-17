#include <stdexcept>
#include <random>
#include <cmath>
#include <iostream>
#include <cstdlib>
#include "MultiTrialStats.h"

#define TEST_ASSERT(condition, message) \
    if (!(condition)) { \
        std::cerr << "FAIL: " << (message) << " at " << __FILE__ << ":" << __LINE__ << std::endl; \
        return false; \
    }

#define TEST_NEAR(actual, expected, tolerance, message) \
    if (std::abs((actual) - (expected)) > (tolerance)) { \
        std::cerr << "FAIL: " << (message) << " - expected " << (expected) << " got " << (actual) \
                  << " at " << __FILE__ << ":" << __LINE__ << std::endl; \
        return false; \
    }

bool test_multitrialstats_test1() {
    std::cout << "Running multitrialstats_test::test1..." << std::endl;

    MultiTrialStats stats(2, 2);
    {
        double varVals_1[2] = {1, 2};
        double varVals_2[2] = {3, 4};
        stats.startNewTrial();
        stats.addSample(0, 1.0, varVals_1);
        stats.addSample(1, 2.0, varVals_2);
    }
    {
        double varVals_1[2] = {10, 20};
        double varVals_2[2] = {40, 30};
        stats.startNewTrial();
        stats.addSample(0, 1.0, varVals_1);
        stats.addSample(1, 2.0, varVals_2);
    }

    TEST_NEAR(stats.getTimePoint(0), 1.0, 1.0E-12, "TimePoint(0)");
    TEST_NEAR(stats.getTimePoint(1), 2.0, 1.0E-12, "TimePoint(1)");

    int n = 2;
    double epsilon = 1.0E-12;
    double mu_0_0 = (1.0 + 10.0) / 2;
    double mu_1_0 = (2.0 + 20.0) / 2;
    double mu_0_1 = (3.0 + 40.0) / 2;
    double mu_1_1 = (4.0 + 30.0) / 2;

    TEST_NEAR(stats.getMean(0, 0), mu_0_0, epsilon, "Mean(0,0)");
    TEST_NEAR(stats.getMean(1, 0), mu_1_0, epsilon, "Mean(1,0)");
    TEST_NEAR(stats.getMean(0, 1), mu_0_1, epsilon, "Mean(0,1)");
    TEST_NEAR(stats.getMean(1, 1), mu_1_1, epsilon, "Mean(1,1)");

    double var_0_0 = (pow(1 - mu_0_0, 2) + pow(10 - mu_0_0, 2)) / (n - 1);
    double var_1_0 = (pow(2 - mu_1_0, 2) + pow(20 - mu_1_0, 2)) / (n - 1);
    double var_0_1 = (pow(3 - mu_0_1, 2) + pow(40 - mu_0_1, 2)) / (n - 1);
    double var_1_1 = (pow(4 - mu_1_1, 2) + pow(30 - mu_1_1, 2)) / (n - 1);

    TEST_NEAR(stats.getVariance(0, 0), var_0_0, epsilon, "Variance(0,0)");
    TEST_NEAR(stats.getVariance(1, 0), var_1_0, epsilon, "Variance(1,0)");
    TEST_NEAR(stats.getVariance(0, 1), var_0_1, epsilon, "Variance(0,1)");
    TEST_NEAR(stats.getVariance(1, 1), var_1_1, epsilon, "Variance(1,1)");

    std::cout << "PASS: multitrialstats_test::test1" << std::endl;
    return true;
}

bool test_multitrialstats_testGaussian() {
    std::cout << "Running multitrialstats_test::testGaussian..." << std::endl;

    double expected_mean = 100.0;
    double expected_stddev = 1.77;
    std::default_random_engine generator(1634997497);
    std::normal_distribution<double> distribution(expected_mean, expected_stddev);

    MultiTrialStats stats(1, 1);
    int NUM_TRIALS = 100000;
    double sample_min = 1.0E99;
    double sample_max = -1.0E99;

    for (int i = 0; i < NUM_TRIALS; i++) {
        double sample = distribution(generator);
        sample_min = std::min(sample_min, sample);
        sample_max = std::max(sample_max, sample);
        double varVals_1[1] = {sample};
        stats.startNewTrial();
        stats.addSample(0, 0.0, varVals_1);
    }

    double z_score_99999 = 4.417173; // 99.999% confidence
    double epsilon_mean = z_score_99999 * expected_stddev / sqrt(NUM_TRIALS);
    TEST_NEAR(stats.getMean(0, 0), expected_mean, epsilon_mean, "Gaussian Mean");

    double epsilon_variance = 0.05;
    TEST_NEAR(stats.getVariance(0, 0), expected_stddev * expected_stddev, epsilon_variance, "Gaussian Variance");

    TEST_NEAR(stats.getMin(0, 0), sample_min, 1e-12, "Gaussian Min");
    TEST_NEAR(stats.getMax(0, 0), sample_max, 1e-12, "Gaussian Max");

    std::cout << "PASS: multitrialstats_test::testGaussian" << std::endl;
    return true;
}