# Chronos — Personal Task & Deadline Manager

Bài tập lớn môn Lập trình Hướng đối tượng (OOP) — C++17, CLI.

## Tài liệu

| File | Nội dung |
|------|----------|
| [docs/Chronos_Feature_List.pdf](docs/Chronos_Feature_List.pdf) | File **feature list** gốc (PDF) — yêu cầu chức năng. |
| [docs/FEATURE_LIST.md](docs/FEATURE_LIST.md) | Phiên bản markdown của feature list (dễ đọc). |
| [docs/SPEC.pdf](docs/SPEC.pdf) | File **technical spec** gốc (PDF). |
| [docs/SPEC.md](docs/SPEC.md) | Phiên bản markdown của technical spec. |
| [docs/ANALYSIS.md](docs/ANALYSIS.md) | Báo cáo đối chiếu SPEC ↔ Feature List (coverage, gaps, đề xuất). |

## Cấu trúc dự án

```
chronos/
├── CMakeLists.txt          # Build root
├── README.md               # File này
├── docs/                   # Tài liệu (PDF + markdown + analysis)
├── include/                # Header files (.h)
├── src/                    # Source files (.cpp)
├── tests/
│   ├── CMakeLists.txt
│   ├── unit/               # Unit tests (gtest)
│   └── integration/        # Integration tests (gtest)
└── data/
    └── chronos_save.csv    # File save tự sinh
```

## Yêu cầu môi trường
- C++17 compiler (g++ 9.0+ / MSVC 2019+ / clang 10+)
- CMake 3.15+
- Internet (lần build đầu sẽ `FetchContent` GoogleTest)

## Build & Run

```bash
# Build
mkdir -p build && cd build
cmake ..
cmake --build . -j

# Chạy app
./chronos

# Chạy tests
ctest --output-on-failure
```

## Trạng thái hiện tại

Codebase đã được implement đầy đủ theo `docs/SPEC.md`:

| Module | Class | Files cần xem | Trạng thái |
|--------|-------|---------------|-----------|
| Module 1 — Core Domain | `Date`, `Task`, `Project`, Exceptions | `include/Date.h` + `src/Date.cpp`, `include/Task.h` + `src/Task.cpp`, `include/Project.h` + `src/Project.cpp`, `include/Exceptions.h` (header-only) | ✅ |
| Module 2 — Logic | `UrgencyCalculator`, `TaskFilter` | `include/UrgencyCalculator.h` + `src/UrgencyCalculator.cpp`, `include/TaskFilter.h` + `src/TaskFilter.cpp` | ✅ |
| Module 3 — Persistence | `UndoManager` (deque-based, `ProjectState` alias), `FileHandler` (CSV), `ProjectManager` (multi-project + index.csv) | `include/UndoManager.h` + `src/UndoManager.cpp`, `include/FileHandler.h` + `src/FileHandler.cpp`, `include/ProjectManager.h` + `src/ProjectManager.cpp` | ✅ |
| Module 4 — CLI | `ConsoleColor`, `OuterMenu` (chọn/tạo/xóa/đổi tên dự án), `Menu` (thao tác trong 1 dự án), entry-point | `include/ConsoleColor.h` + `src/ConsoleColor.cpp`, `include/OuterMenu.h` + `src/OuterMenu.cpp`, `include/Menu.h` + `src/Menu.cpp`, `src/main.cpp` | ✅ |
| Tests | Unit (gtest) + Integration (gtest) | `tests/unit/*.cpp` (9 file, 73 test), `tests/integration/*.cpp` (7 file, 19 test) | ✅ 92/92 PASS |

**Chi tiết file test (tiện tra khi báo cáo):**

| Loại | File | Số test | Nội dung |
|------|------|--------:|----------|
| Unit | `test_date.cpp` | 13 | Date: validate, năm nhuận, `daysBetween`, `addDays`, operator, parse |
| Unit | `test_task.cpp` | 12 | Task: ctor, TimeParadox, `daysLeft`, `isOverdue`, `operator<`/`++` |
| Unit | `test_project.cpp` | 7 | Project: add/remove/update task, `getTaskById`, `MAX_TASKS` |
| Unit | `test_urgency.cpp` | 7 | UrgencyCalculator: công thức, overdue boost, 3 kiểu sort |
| Unit | `test_filter.cpp` | 6 | TaskFilter: search keyword, filter priority/status/date, overdue |
| Unit | `test_undo.cpp` | 6 | UndoManager: LIFO, giới hạn 50 state, stack rỗng |
| Unit | `test_filehandler.cpp` | 5 | FileHandler: save/load CSV, escape dấu phẩy, file lỗi |
| Unit | `test_project_manager.cpp` | 11 | ProjectManager: create/delete/rename, index.csv, migration |
| Unit | `test_exceptions.cpp` | 6 | Nội dung message của các custom exception |
| Integration | `test_full_workflow.cpp` | 1 | Luồng end-to-end add → sort → save |
| Integration | `test_undo_workflow.cpp` | 2 | Undo qua nhiều thao tác mutate |
| Integration | `test_persistence.cpp` | 2 | Save rồi reload giữ nguyên dữ liệu |
| Integration | `test_error_scenarios.cpp` | 3 | Các tình huống lỗi (TimeParadox, BufferOverflow, file I/O) |
| Integration | `test_menu_features.cpp` | 4 | Sort/search/filter mức Menu |
| Integration | `test_console_color.cpp` | 2 | ConsoleColor / tô màu theo trạng thái task |
| Integration | `test_project_lifecycle.cpp` | 5 | Multi-project: tạo/chuyển/xóa nhiều dự án, reload |

2 điểm mâu thuẫn từng phát hiện trong SPEC đều đã được fix trực tiếp trước khi code:
1. Undo container chốt `std::deque<ProjectState> history;` + `using ProjectState = Project;` (`include/UndoManager.h`).
2. Menu CLI expose đủ 3 chế độ sort (urgency/deadline/priority) qua sub-menu + shortcut `sortu`/`sortdate`/`sortprio` (`src/Menu.cpp::handleSort()`).

Yêu cầu vẫn còn để mở (tùy chọn, ngoài SPEC):
- Tách `InputValidator` thành class riêng (hiện validate nằm rải rác ở `Date` ctor / `Task` setter / `Menu::readDate`).
- Thêm phần "Hướng dẫn quay video demo" vào `docs/SPEC.md` §10 (TV4).
