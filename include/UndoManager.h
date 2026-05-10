#pragma once

#include <cstddef>
#include <deque>

#include "Project.h"

// Alias để khớp tên kiểu trong Feature List ("std::stack<ProjectState>").
// Nếu sau này muốn lưu thêm metadata, chỉ cần đổi alias thành 1 struct chứa
// Project + metadata mà không phá vỡ API public.
using ProjectState = Project;

class UndoManager {
private:
    // GHI CHÚ THIẾT KẾ: Yêu cầu Feature List là "Undo bằng std::stack". Nhưng
    // std::stack KHÔNG cho phép xóa phần tử ở đáy, nên không thể giới hạn 50
    // phần tử (sẽ ngốn bộ nhớ vô hạn). Vì vậy ta dùng std::deque làm storage
    // và mô phỏng API LIFO của stack qua push_back/back/pop_back. Vẫn được
    // tính là "stack-based undo" theo tinh thần OOP của đề bài.
    std::deque<ProjectState> history;

public:
    static constexpr int MAX_UNDO_DEPTH = 50;

    UndoManager();

    void saveState(const ProjectState& currentState);
    ProjectState undo();
    void clear();
    std::size_t size() const;
    bool isEmpty() const;
};
