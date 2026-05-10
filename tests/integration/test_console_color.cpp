#include <gtest/gtest.h>

#include <string>

#include "ConsoleColor.h"

TEST(IntegrationConsoleColor, RestCodesArePresent) {
    EXPECT_EQ(std::string(ConsoleColor::RESET), "\033[0m");
    EXPECT_EQ(std::string(ConsoleColor::RED), "\033[31m");
    EXPECT_EQ(std::string(ConsoleColor::GREEN), "\033[32m");
}

TEST(IntegrationConsoleColor, EnableAnsiOnWindowsNoCrashOnLinux) {
    EXPECT_NO_THROW(ConsoleColor::enableAnsiOnWindows());
}
