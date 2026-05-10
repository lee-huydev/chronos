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

| Module | Files | Trạng thái |
|--------|-------|-----------|
| Module 1 — Core Domain | `Date`, `Task`, `Project`, `Exceptions` | ✅ |
| Module 2 — Logic | `UrgencyCalculator`, `TaskFilter` | ✅ |
| Module 3 — Persistence | `UndoManager` (deque-based, ProjectState alias), `FileHandler` (CSV) | ✅ |
| Module 4 — CLI | `ConsoleColor`, `Menu`, `main` | ✅ |
| Tests | 8 file unit (62 test) + 6 file integration (14 test) | ✅ 76/76 PASS |

2 điểm mâu thuẫn từng phát hiện trong SPEC đều đã được fix trực tiếp trước khi code:
1. Undo container chốt `std::deque<ProjectState> history;` + `using ProjectState = Project;` (`include/UndoManager.h`).
2. Menu CLI expose đủ 3 chế độ sort (urgency/deadline/priority) qua sub-menu + shortcut `sortu`/`sortdate`/`sortprio` (`src/Menu.cpp::handleSort()`).

Yêu cầu vẫn còn để mở (tùy chọn, ngoài SPEC):
- Tách `InputValidator` thành class riêng (hiện validate nằm rải rác ở `Date` ctor / `Task` setter / `Menu::readDate`).
- Thêm phần "Hướng dẫn quay video demo" vào `docs/SPEC.md` §10 (TV4).
