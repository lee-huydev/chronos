# Tóm tắt thay đổi: Lớp Quản Lý Dự Án (Multi-Project)

> File này tóm tắt **những gì sẽ được thêm vào Chronos** sau khi áp dụng yêu cầu mới của bạn.
> Mục đích: bạn confirm hoặc sửa đổi trước khi tôi đụng vào code.

## 1. User Flow Mới

```
App khởi động
   │
   ▼
ProjectManager.load("data/projects/index.csv")
   │
   ▼
Index rỗng?
 ├── Yes → WIZARD: hỏi tên + mô tả → tạo dự án đầu tiên → vào Main Menu luôn
 └── No  → OUTER MENU: hiển thị danh sách dự án → user chọn 1 → vào Main Menu

Trong MAIN MENU:
  - Banner LUÔN có dòng: "Project: [#id] Tên dự án"
  - Slot 1: Đổi tên dự án hiện tại (thay cho "Tạo dự án mới" cũ)
  - Slot 12 (MỚI): "Quay về danh sách dự án" → auto-save → quay lại OUTER MENU
  - Slot 0: Thoát chương trình (như cũ; auto-save)

OUTER MENU:
  1. Mở dự án (Select)
  2. Tạo dự án mới (Create)
  3. Đổi tên dự án (Rename)
  4. Xóa dự án (Delete) — confirm Y/N
  0. Thoát chương trình
```

## 2. Use Cases Mới (UC1–UC7)

| UC | Mô tả | Tiền đk | Hậu đk |
|----|-------|---------|--------|
| UC1 | Lần đầu chạy (chưa có dự án) | Index rỗng | Wizard tạo dự án đầu tiên → tự động Main Menu |
| UC2 | Chạy lại với 1+ dự án sẵn có | Index ≥ 1 entry | Hiển thị danh sách → user chọn → Main Menu |
| UC3 | Tạo dự án mới từ Outer Menu | Đang ở Outer | Index có entry mới + file CSV mới |
| UC4 | Đổi tên dự án | ≥ 1 dự án | Tên mới persist trong index |
| UC5 | Xóa dự án | ≥ 1 dự án | Entry biến mất, file CSV bị xóa; nếu xóa hết → quay lại Wizard |
| UC6 | Switch dự án từ Main Menu (Slot 12) | Đang trong 1 dự án | Auto-save → quay về Outer |
| UC7 | Banner project name trong Main Menu | Đã chọn dự án | Mọi lần render menu đều in `[#id] Tên` |

## 3. Class / File Mới

| File | Vai trò |
|------|---------|
| `include/ProjectManager.h` + `src/ProjectManager.cpp` | Quản lý index + per-project files |
| `include/OuterMenu.h` + `src/OuterMenu.cpp` | Menu chọn / tạo / đổi tên / xóa dự án |
| `tests/unit/test_project_manager.cpp` | 10 test case cho ProjectManager |
| `tests/integration/test_project_lifecycle.cpp` | 5 test case end-to-end |

## 4. Class `ProjectManager` — API tóm tắt

```cpp
struct ProjectMeta {
    int id;                  // auto-increment, KHÔNG reuse
    std::string name;
    std::string description;
    std::string savePath;    // "data/projects/<id>.csv"
    std::string createdAt;
    std::string modifiedAt;
    int taskCount = 0;
};

class ProjectManager {
public:
    explicit ProjectManager(const std::string& dataDir = "data/projects");
    void load();                                    // đọc index.csv
    void save() const;                              // ghi index.csv
    const std::vector<ProjectMeta>& list() const;
    bool isEmpty() const;
    const ProjectMeta* findById(int id) const;
    ProjectMeta create(const std::string& name, const std::string& description);
    void rename(int id, const std::string& newName);
    void deleteProject(int id);                     // xóa entry + file CSV
    Project loadProject(int id);
    void saveProject(int id, const Project& project); // cũng update modifiedAt + taskCount
    void migrateLegacySaveIfPresent(const std::string& legacy = "data/chronos_save.csv");
};
```

## 5. Class `OuterMenu` — API tóm tắt

```cpp
class OuterMenu {
public:
    explicit OuterMenu(ProjectManager& pm);
    void run();                  // load → wizard hoặc loop chọn → khi user chọn Thoát thì return
private:
    void firstLaunchWizard();    // UC1
    void handleSelect();         // UC2
    void handleCreate();         // UC3
    void handleRename();         // UC4
    void handleDelete();         // UC5
    void enterMainMenu(int projectId);
};
```

## 6. Thay Đổi `Menu` (per-project) — API mới

```cpp
class Menu {
public:
    Menu(const Project& project, int projectId, ProjectManager& manager);
    void run();
    bool shouldExitProgram() const;  // true nếu user gõ 0 (exit) thay vì 12 (back)
private:
    // BỎ: handleCreateProject (đã chuyển sang OuterMenu)
    void handleRenameCurrentProject();    // MỚI — slot 1
    void handleBackToOuter();             // MỚI — slot 12
    // các handle còn lại giữ nguyên (add, view, sort, extend, undo, save, search, delete, status, stress, exit)
};
```

Slot mapping mới của Main Menu:

| Số | Hành động | Keyword |
|----|-----------|---------|
| 1 | Đổi tên dự án hiện tại | `rename` |
| 2 | Thêm nhiệm vụ | `add` |
| 3 | Xem danh sách | `list`/`view`/`ls` |
| 4 | Sắp xếp (sub-menu) | `sort` (+ `sortu`/`sortdate`/`sortprio`) |
| 5 | Gia hạn deadline | `extend` |
| 6 | Hoàn tác | `undo` |
| 7 | Lưu / Tải | `save`/`load` |
| 8 | Tìm kiếm | `search`/`find` |
| 9 | Xóa nhiệm vụ | `delete`/`rm` |
| 10 | Cập nhật trạng thái | `status`/`update` |
| 11 | Stress test | `stress` |
| **12** | **Quay về danh sách dự án** | `back`/`switch`/`projects` |
| 0 | Thoát chương trình | `exit`/`quit`/`q` |

## 7. Data Model Mới

**Cũ:** 1 file duy nhất `data/chronos_save.csv` chứa 1 project + tasks.

**Mới:** thư mục `data/projects/`:
```
data/projects/
├── index.csv          (danh sách meta của tất cả dự án)
├── 1.csv              (file CSV của dự án id=1, format giữ nguyên hệt cũ)
├── 2.csv
└── …
```

Format `index.csv`:
```
# CHRONOS_INDEX v1
nextId,3
# PROJECTS
id,name,description,savePath,createdAt,modifiedAt,taskCount
1,Daily Tasks,Personal todo,data/projects/1.csv,2026-05-01 09:00:00,2026-05-02 09:30:12,12
2,Q2 Sprint,Work board,data/projects/2.csv,2026-05-02 14:10:00,2026-05-02 18:45:00,7
```

**Migration:** Nếu có file cũ `data/chronos_save.csv` thì sẽ:
1. Tạo `data/projects/`
2. Load file cũ → chuyển thành `data/projects/1.csv`
3. Ghi entry index id=1
4. Rename file cũ thành `chronos_save.csv.bak` (giữ backup)

## 8. Exception Mới

`Exceptions.h` thêm:
```cpp
class ProjectNotFoundException : public std::runtime_error {
public:
    explicit ProjectNotFoundException(int id);
};
```

## 9. Câu Hỏi Cần Bạn Quyết Định

| # | Vấn đề | Default đang đề xuất | Bạn confirm hay đổi? |
|---|--------|----------------------|---------------------|
| Q1 | Lưu trữ: 1 file index + N file project, hay 1 file gộp tất cả? | **Index + N file** (clean, dễ backup) | … |
| Q2 | Project ID có reuse sau khi xóa không? | **KHÔNG reuse** (dùng `nextId` chỉ tăng) | … |
| Q3 | Slot `1` Main Menu cũ là "Tạo dự án mới" — nay làm gì? | **"Đổi tên dự án hiện tại"** (vì tạo đã có ở Outer) | … |
| Q4 | Lệnh "Switch dự án" trong Main Menu | **Slot 12 + keyword `back`/`switch`** | … |
| Q5 | Khi xóa dự án, có xác nhận `Y/N` không? | **CÓ** (mặc định `N`) | … |
| Q6 | Có cho rename project trùng tên không? | **CÓ** (project ID là khóa, không phải name) | … |
| Q7 | Có migration tự động từ `data/chronos_save.csv` không? | **CÓ** (rename `.bak` để giữ an toàn) | … |
| Q8 | Tên dự án có giới hạn ký tự không? | Không cần (chỉ disallow rỗng + `,` được escape) | … |

## 10. Testing Plan

- **Tests cũ (76 tests)**: vẫn phải PASS sau khi refactor (FileHandler API giữ nguyên; Menu chỉ thêm tham số constructor).
- **Tests mới (~15 tests)**:
  - 10 unit test cho `ProjectManager` (xem §7.x SPEC.md)
  - 5 integration test cho lifecycle multi-project (xem §8.x SPEC.md)
- Tổng dự kiến: **~91 tests** sau khi xong.

## 11. Phần KHÔNG đổi

- `Date`, `Task`, `Project`, `Exceptions` (chỉ thêm `ProjectNotFoundException`)
- `UrgencyCalculator`, `TaskFilter`, `UndoManager`
- `FileHandler::saveProject`/`loadProject` cho 1 file CSV (vẫn được `ProjectManager` gọi)
- Format CSV per-project
- Logic Undo, Time Paradox, Buffer Overflow, sort sub-menu, ANSI color

---

## Cần bạn làm gì tiếp?

Confirm 1 trong 3:

- **A. Confirm như trên → bắt tay implement**
- **B. Confirm phần lớn nhưng đổi câu trả lời ở Q1–Q8 (chỉ rõ câu nào)**
- **C. Quay lại sửa đổi thêm trong FEATURE_LIST/SPEC trước**
