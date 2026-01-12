#include <gtest/gtest.h>

#include "../includes/general/subfuncs.hpp"

int add(int a, int b) { return a + b; }

TEST(AdditionTest, PositiveNumbers) { EXPECT_EQ(add(2, 3), 5); }