//
// Created by Logan Drescher on 3/19/26.
//
#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include <cstring>  // Note the c++ variant, not the .h original
#include <cstdlib>  // Note the c++ variant, not the .h original
#include <vcellhybrid.h>

#include "smoldynfuncs.h"
#include "../build/VCell/tests/testFiles/input/testResourceLocations.h" // provides testResourceLocations

TEST(SmoldynTest, ExerciseSmoldyn) {
	// When running as a CTest, we can get carry over from previous set variables, including hybrid-mode.
	// This call should prevent such bleed-over
	vcellhybrid::resetHybrid();
	std::string first_arg{"ignored"};
	std::string second_arg{testResourceLocations::EXAMPLE_SMOLDYN_INPUT};
	// Allocate zero-initialized memory
	char* first_arg_ptr = static_cast<char*>(std::calloc(first_arg.size() + 1, sizeof(char)));
	char* second_arg_ptr = static_cast<char*>(std::calloc(second_arg.size() + 1, sizeof(char)));
	ASSERT_TRUE(first_arg_ptr && second_arg_ptr) << "Memory allocation failed";

	std::strcpy(first_arg_ptr, first_arg.c_str());
	std::strcpy(second_arg_ptr, second_arg.c_str());
	char* arg_array[2];
	arg_array[0] = first_arg_ptr;
	arg_array[1] = second_arg_ptr;

	int retcode_1 = runSmoldyn(2, arg_array);
	EXPECT_EQ(retcode_1, 0);
	int retcode_2 = runSmoldyn(2, arg_array);
	EXPECT_EQ(retcode_2, 0);

	std::free(first_arg_ptr);
	std::free(second_arg_ptr);
}


