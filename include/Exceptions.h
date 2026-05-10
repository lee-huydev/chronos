#pragma once

#include <stdexcept>
#include <string>

class TimeParadoxException : public std::runtime_error {
public:
    TimeParadoxException()
        : std::runtime_error(
              u8"Lỗi: Nghịch lý thời gian! Ngày hết hạn không thể trước ngày bắt đầu.") {}
};

class BufferOverflowException : public std::runtime_error {
public:
    BufferOverflowException()
        : std::runtime_error(
              u8"Lỗi: Vượt quá giới hạn lưu trữ! Số nhiệm vụ tối đa là 9999.") {}
};

class InvalidDateException : public std::runtime_error {
public:
    explicit InvalidDateException(const std::string& msg)
        : std::runtime_error(std::string(u8"Lỗi ngày không hợp lệ: ") + msg) {}
};

class TaskNotFoundException : public std::runtime_error {
public:
    explicit TaskNotFoundException(int id)
        : std::runtime_error(std::string(u8"Không tìm thấy nhiệm vụ với ID = ") +
                             std::to_string(id)) {}
};

class FileIOException : public std::runtime_error {
public:
    explicit FileIOException(const std::string& msg)
        : std::runtime_error(std::string(u8"Lỗi I/O file: ") + msg) {}
};

class UndoStackEmptyException : public std::runtime_error {
public:
    UndoStackEmptyException()
        : std::runtime_error(u8"Không còn thao tác nào để hoàn tác.") {}
};

class ProjectNotFoundException : public std::runtime_error {
public:
    explicit ProjectNotFoundException(int id)
        : std::runtime_error(std::string(u8"Không tìm thấy dự án có ID = ") +
                             std::to_string(id)) {}
};
