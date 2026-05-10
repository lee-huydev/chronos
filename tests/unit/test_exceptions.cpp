#include <gtest/gtest.h>

#include <stdexcept>
#include <string>

#include "Exceptions.h"

TEST(ExceptionTest, TimeParadoxMessage) {
    try {
        throw TimeParadoxException();
    } catch (const std::exception& e) {
        std::string msg(e.what());
        EXPECT_NE(msg.find("Nghịch lý thời gian"), std::string::npos);
    }
}

TEST(ExceptionTest, BufferOverflowMessage) {
    try {
        throw BufferOverflowException();
    } catch (const std::exception& e) {
        std::string msg(e.what());
        EXPECT_NE(msg.find("9999"), std::string::npos);
    }
}

TEST(ExceptionTest, InvalidDateMessageContainsContext) {
    try {
        throw InvalidDateException("32/13/9999");
    } catch (const std::exception& e) {
        std::string msg(e.what());
        EXPECT_NE(msg.find("32/13/9999"), std::string::npos);
    }
}

TEST(ExceptionTest, TaskNotFoundIncludesId) {
    try {
        throw TaskNotFoundException(42);
    } catch (const std::exception& e) {
        std::string msg(e.what());
        EXPECT_NE(msg.find("42"), std::string::npos);
    }
}

TEST(ExceptionTest, FileIOMessage) {
    try {
        throw FileIOException("Khong mo duoc /tmp/xyz");
    } catch (const std::exception& e) {
        std::string msg(e.what());
        EXPECT_NE(msg.find("Khong mo duoc"), std::string::npos);
    }
}

TEST(ExceptionTest, UndoStackEmpty) {
    try {
        throw UndoStackEmptyException();
    } catch (const std::exception& e) {
        std::string msg(e.what());
        EXPECT_FALSE(msg.empty());
    }
}
