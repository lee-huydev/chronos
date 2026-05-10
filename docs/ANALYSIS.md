# BÁO CÁO ĐỐI CHIẾU SPEC ↔ FEATURE LIST

> Mục đích: kiểm tra xem **`SPEC.md`** đã cover đầy đủ và đúng các chức năng được mô tả trong **`Chronos Feature List`** chưa.
> Hai nguồn so sánh:
> - `docs/Chronos_Feature_List.pdf` (= `docs/FEATURE_LIST.md`)
> - `docs/SPEC.pdf` (= `docs/SPEC.md`)
>
> **Kết luận tổng quát:** SPEC là một SUPERSET của Feature List — toàn bộ chức năng bắt buộc đều được đặc tả; SPEC còn bổ sung thêm chi tiết kỹ thuật (kiểm tra ngày hợp lệ, custom exceptions phụ, validation cấp setter, mở rộng menu, format CSV, GoogleTest, CMake, hỗ trợ Windows ANSI, v.v.).
>
> **Cập nhật:** 2 điểm mâu thuẫn phát hiện lần review trước ("Undo container" và "Sort menu") đã được **fix trực tiếp trong `SPEC.md`**. Xem chi tiết ở §3 và §6 dưới đây.

---

## 1. Bảng Coverage Matrix theo Module

Quy ước:
- ✅ **Đầy đủ** — Spec có đặc tả khớp hoặc chi tiết hơn yêu cầu.
- ➕ **Vượt yêu cầu** — Spec có thêm tính năng/chi tiết ngoài Feature List.
- ⚠️ **Cần lưu ý** — Có khác biệt nhỏ hoặc cần làm rõ.
- ❌ **Thiếu** — Spec chưa đề cập (KHÔNG có hạng mục nào rơi vào trạng thái này).

### Module 1 — Core Domain

| # | Yêu cầu (Feature List §) | Mục SPEC tương ứng | Trạng thái | Ghi chú |
|---|--------------------------|--------------------|------------|---------|
| 1.1.a | `Task.id` (int) | §3.3 `int id` + `static int nextId` | ✅ ➕ | Spec thêm auto-increment ID + `resetIdCounter()` để tiện cho test. |
| 1.1.b | `Task.title`, `description` (string) | §3.3 `std::string title`, `description` | ✅ | |
| 1.1.c | `Task.startDate`, `dueDate` (Date) | §3.3 `Date startDate`, `dueDate` | ✅ | |
| 1.1.d | `Task.priority` (enum LOW/MEDIUM/HIGH/CRITICAL) | §3.2 `enum class Priority` (LOW=1…CRITICAL=4) | ✅ | Có gán giá trị số rõ ràng để dùng cho công thức Urgency. |
| 1.1.e | `Task.status` (enum PENDING/IN_PROGRESS/DONE) | §3.2 `enum class Status` | ✅ | |
| 1.1.f | Constructor mặc định / tham số / copy | §3.3 `Task()`, `Task(title,…,priority)`, `Task(const Task&)` | ✅ | |
| 1.1.g | Destructor | §3.3 `~Task()` | ✅ | |
| 1.1.h | Getters/Setters | §3.3 đầy đủ getter `const` + setter có validate | ✅ ➕ | Spec yêu cầu setter validate (ví dụ `setStartDate` đảm bảo `<= dueDate`). |
| 1.1.i | `display()` | §3.3 `void display() const` | ✅ | |
| 1.2.a | `Project.projectId`, `projectName`, `description` | §3.4 đủ thuộc tính | ✅ | |
| 1.2.b | `vector<Task> tasks` | §3.4 `std::vector<Task> tasks` | ✅ | |
| 1.2.c | `addTask(Task)` | §3.4 `void addTask(const Task& t)` (throw `BufferOverflowException`) | ✅ | |
| 1.2.d | `removeTask(int id)` | §3.4 `bool removeTask(int taskId)` | ✅ ➕ | Trả về `bool` báo có xóa được hay không. |
| 1.2.e | `getTaskById(int id)` | §3.4 `Task* getTaskById(int taskId)` | ✅ | Trả `nullptr` khi không tìm thấy. |
| 1.2.f | `getAllTasks()` | §3.4 `const std::vector<Task>& getAllTasks() const` | ✅ | |
| 1.2.g | `displayProject()` | §3.4 `displayProject()` + `displayAllTasks()` | ✅ ➕ | Thêm `displayAllTasks()` in bảng `iomanip`. |
| 1.3.a | Class `Date(day, month, year)` | §3.1 đầy đủ + `static Date today()`, `fromString`, `toString` | ✅ ➕ | Thêm parse/format chuỗi, kiểm tra năm nhuận, `daysBetween`, `addDays`. |
| 1.3.b | So sánh & khoảng cách 2 ngày | §3.1 toàn bộ operator so sánh + `daysBetween` | ✅ | |
| 1.4.a | `operator<` cho Task | §3.3 `bool operator<(const Task&) const` (priority → deadline) | ✅ | |
| 1.4.b | `operator++` (prefix & postfix) | §3.3 `Task& operator++()`, `Task operator++(int)` | ✅ | |
| 1.4.c | `operator==` | §3.3 `bool operator==(const Task&) const` (so sánh `id`) | ✅ | |
| 1.4.d | `operator<<` ra `ostream` | §3.3 `friend std::ostream& operator<<(std::ostream&, const Task&)` | ✅ | |

### Module 2 — Logic & Tính toán

| # | Yêu cầu (Feature List §) | Mục SPEC tương ứng | Trạng thái | Ghi chú |
|---|--------------------------|--------------------|------------|---------|
| 2.1 | Công thức Urgency `priority*10 + 100/(daysLeft+1)` | §4.1 `UrgencyCalculator::calculate` | ✅ ➕ | Bổ sung 2 quy tắc đặc biệt: `DONE → 0`, `overdue → +1000` (boost). |
| 2.2.a | `std::sort` + `operator<` | §4.1 `sortByUrgency` (dùng `std::sort` + comparator urgency) | ✅ | |
| 2.2.b | Sắp xếp theo ưu tiên / deadline / urgency | §4.1 `sortByPriority`, `sortByDeadline`, `sortByUrgency` + §6.1 `handleSort()` với sub-menu 3 chế độ + shortcut `sortu`/`sortdate`/`sortprio` | ✅ | Đã fix: menu đã expose đủ 3 chế độ. |
| 2.3.a | `TimeParadoxException` khi `dueDate < startDate` | §3.3 (constructor & setters) + §4.3 | ✅ | |
| 2.3.b | `BufferOverflowException` khi vượt `MAX_TASKS = 9999` | §3.4 (`MAX_TASKS`) + §4.3 | ✅ | |
| 2.3.c | Validate input chuỗi/số/ngày | Date validate ở §3.1 + setter validate ở §3.3 + `readInt/readDate/readString` ở §6 | ✅ | Validate được phân tán; chưa có một class `InputValidator` tập trung — xem "Đề xuất cải thiện". |
| 2.4.a | Tìm theo tên/từ khóa | §4.2 `searchByKeyword` (case-insensitive) | ✅ ➕ | Spec yêu cầu match cả `title` lẫn `description`, không phân biệt hoa thường. |
| 2.4.b | Lọc theo priority / status / deadline | §4.2 `filterByPriority`, `filterByStatus`, `filterByDateRange` | ✅ ➕ | Có thêm `getOverdueTasks` để liệt kê task quá hạn. |

### Module 3 — Persistence & Undo

| # | Yêu cầu (Feature List §) | Mục SPEC tương ứng | Trạng thái | Ghi chú |
|---|--------------------------|--------------------|------------|---------|
| 3.1.a | Undo bằng `std::stack<ProjectState>` | §5.1 `std::deque<ProjectState> history` + `using ProjectState = Project;` (LIFO API giống stack) | ✅ | Đã fix: tên kiểu `ProjectState` khớp Feature List, container nội bộ dùng `std::deque` để có thể giới hạn 50 phần tử (xem ghi chú thiết kế trong SPEC). |
| 3.1.b | Push state trước mỗi mutate | §5.1 quy tắc "Trước MỖI thao tác mutate → `saveState()`" | ✅ | |
| 3.1.c | Lệnh `undo` lấy state đỉnh | §5.1 `Project undo()` (throw `UndoStackEmptyException` nếu rỗng) | ✅ ➕ | Có exception riêng cho stack rỗng. |
| 3.1.d | Giới hạn 50 lần undo | §5.1 `MAX_UNDO_DEPTH = 50` | ✅ | |
| 3.2.a | `saveToFile` / `loadFromFile` (`.txt` / `.csv` / `.json`) | §5.2 `saveProject` / `loadProject` (CSV) | ✅ | Spec chốt format CSV (1 trong 3 lựa chọn được phép). |
| 3.2.b | Tải lại khi mở chương trình | §6.5 quy tắc "khi khởi động → auto load" | ✅ | |
| 3.3.a | Auto-save khi thoát | §6.5 `handleExit()` auto-save khi `!isDirty` hoặc khi user chọn `Y` | ✅ ➕ | Có cả `SIGINT` handler best-effort. |
| 3.3.b | Cảnh báo khi thoát mà chưa lưu | §6.5 prompt `Y/N/Cancel` | ✅ ➕ | Cho phép `Cancel` để quay lại menu — UX tốt hơn yêu cầu tối thiểu. |

### Module 4 — CLI & Trải nghiệm

| # | Yêu cầu (Feature List §) | Mục SPEC tương ứng | Trạng thái | Ghi chú |
|---|--------------------------|--------------------|------------|---------|
| 4.1.a | Menu 8 mục: tạo project, add task, view list, sort khẩn cấp, gia hạn (++), undo, save/load, thoát | §6.1 + §6.2 menu 11 mục (bao gồm 8 mục bắt buộc + 3 mục mở rộng) | ✅ ➕ | Spec có thêm: `8. Tìm kiếm`, `9. Xóa nhiệm vụ`, `10. Cập nhật trạng thái`, `11. Stress test`. |
| 4.1.b | Nhập bằng số HOẶC từ khóa (`add`, `list`, `undo`…) | §6.1 bảng mapping số ↔ keyword case-insensitive (`parseCommand`) | ✅ ➕ | Bảng mapping rất chi tiết, có alias (`ls`, `q`, `rm`…). |
| 4.2.a | In bảng dùng `iomanip`, cột rõ ràng | §6.3 ASCII table đầy đủ 7 cột | ✅ | |
| 4.2.b | Tô màu: khẩn cấp = đỏ, hoàn thành = xanh | §6.4 `ConsoleColor` namespace + bảng quy tắc tô màu | ✅ ➕ | Có thêm rule cho HIGH (vàng), CRITICAL (đỏ + bold), urgency ≥ 50 (vàng). |
| 4.3.a | Demo Time Paradox | §8.4 `TimeParadoxDemo` test + lệnh menu thực tế (gia hạn / nhập tay) | ✅ | |
| 4.3.b | Demo Buffer Overflow (vòng for 10.000) | §6.1 menu option **11** `handleStressTest()` + §8.4 `BufferOverflowDemo` test | ✅ ➕ | Có hẳn 1 lệnh menu riêng để chạy demo cho video. |
| 4.3.c | Demo Undo sau khi xóa | §8.2 `UndoAfterRemoveTask` + lệnh menu `9` rồi `6` | ✅ | |
| 4.4.a | `README.md` build & sử dụng | §10 hướng dẫn build/run + `README.md` ở root (sẽ tạo từ skeleton) | ✅ | |
| 4.4.b | Test case: thêm/xóa/sửa, sắp xếp, undo, lỗi | §7 (8 unit) + §8 (≥4 integration) | ✅ ➕ | Spec dùng GoogleTest qua `FetchContent`; có cả test `LeapYearFebruary`, `EscapeCommas`, `MaxDepthLimit`… |
| 4.4.c | Báo cáo & video demo | Spec tập trung kỹ thuật, không nhắc video — đây là phần ngoài kỹ thuật của nhóm | ⚠️ | Không phải lỗi: Spec là tài liệu kỹ thuật, video/báo cáo do Member 4 chuẩn bị (Feature List §4). |

---

## 2. Yêu Cầu Tổng Quát Khác

| Yêu cầu Feature List | SPEC § | Trạng thái |
|----------------------|--------|------------|
| Sử dụng STL (`std::stack`, `std::vector`, `std::string`) | §0 (chỉ STL chuẩn), §3.4 (`vector<Task>`), §5.1 (stack/deque) | ✅ |
| Hình thức CLI | §6 toàn bộ | ✅ |
| Tính năng Undo hoạt động đúng với `std::stack` | §5.1 + §7.6 + §8.2 | ✅ ⚠️ (xem #2 dưới) |
| Demo được 2 lỗi đặc biệt | §8.4, §6.1 (option 11) | ✅ |
| Code sạch, comment, chia file rõ ràng | §1 cấu trúc thư mục + §2 convention naming/header guard | ✅ |
| Phân công 4 thành viên | (ngoài phạm vi spec kỹ thuật) | n/a |
| Lộ trình 4 tuần | (ngoài phạm vi spec kỹ thuật) | n/a |

---

## 3. Điểm cần lưu ý (KHÔNG phải lỗi, nhưng nên thống nhất với nhóm)

### #1 — Menu chỉ expose `sortByUrgency` — ĐÃ FIX ✅
- Feature List §2.2 yêu cầu hỗ trợ sắp xếp theo **3 chiều**: ưu tiên, deadline, điểm khẩn cấp.
- SPEC mới (§6.1) thay `handleSortByUrgency()` bằng `handleSort()` với sub-menu cho người dùng chọn 1 trong 3 chế độ, kèm shortcut `sortu` / `sortdate` / `sortprio` ngay ở menu chính.
- Tên lệnh menu đổi từ "Sắp xếp theo độ khẩn cấp" → "Sắp xếp nhiệm vụ".

### #2 — `std::stack` vs `std::deque` cho Undo & tên kiểu `ProjectState` — ĐÃ FIX ✅
- Feature List §3.1 viết: "Sử dụng `std::stack<ProjectState>`".
- SPEC mới (§5.1):
  - Khai báo `std::deque<ProjectState> history;` — dứt khoát một loại container, không còn mâu thuẫn.
  - Thêm `using ProjectState = Project;` — tên kiểu khớp với Feature List.
  - Ghi chú thiết kế giải thích vì sao dùng `std::deque` thực thi cho "stack-based undo" (do `std::stack` không cho xóa đáy → không thể enforce giới hạn 50). API public vẫn LIFO, vẫn thỏa tinh thần đề bài.
  - Cập nhật bảng §11 "Tóm Tắt Yêu Cầu Đã Cover" để phản ánh điều này.

### #3 — Validate input chưa có module riêng (không fix — ngoài scope lần này)
- Feature List §2.3 nhắc "Validate input chuỗi, số, ngày tháng".
- SPEC làm việc này tản mạn ở Date constructor / Task setter / `Menu::readInt`,`readDate`,`readString`.
- Chấp nhận được, nhưng nếu muốn tách riêng để dễ test, có thể tạo `include/InputValidator.h` (không bắt buộc, có thể bổ sung sau).

### #4 — Format file lưu (không fix — đã chốt CSV)
- Feature List §3.2 cho phép `.txt` / `.csv` / `.json`.
- SPEC chốt CSV — đây là MỘT LỰA CHỌN HỢP LỆ, chỉ cần thông báo rõ trong báo cáo.

### #5 — Các tính năng SPEC mở rộng (không bị Feature List cấm)
SPEC cố tình đi quá Feature List ở các điểm sau (ÉẾp như "spec hơn"):
- Auto-increment task ID + `resetIdCounter()`.
- Custom exceptions phụ: `InvalidDateException`, `TaskNotFoundException`, `FileIOException`, `UndoStackEmptyException`.
- Menu mở rộng: search, delete, update status, stress test.
- Cross-platform Windows ANSI virtual terminal.
- `SIGINT` handler best-effort khi Ctrl+C.
- Bộ test rất lớn (8 unit + ≥4 integration) dùng GoogleTest qua `FetchContent`.

Tất cả đều **không mâu thuẫn** Feature List, chỉ là lựa chọn implement chi tiết hơn.

---

## 4. Tóm tắt thay đổi đã áp dụng vào SPEC.md

| # | Thay đổi | Vị trí |
|---|----------|--------|
| 1 | Thêm `using ProjectState = Project;` ở đầu khai báo `UndoManager` | §5.1 |
| 2 | Đổi `std::stack<Project> history;` → `std::deque<ProjectState> history;` và cập nhật comment thiết kế giải thích lý do | §5.1 |
| 3 | Đổi kiểu tham số `saveState(...)` và kiểu trả về `undo()` thành `ProjectState` (alias của `Project`) | §5.1 |
| 4 | Cập nhật quy tắc undo: dùng `pop_front()` khi vượt MAX_UNDO_DEPTH, `push_back()` khi save, `back()`/`pop_back()` khi undo | §5.1 |
| 5 | Đổi `void handleSortByUrgency()` → `void handleSort()` (sub-menu 3 chế độ) | §6.1 |
| 6 | Cập nhật menu chính: mục 4 đổi thành "Sắp xếp nhiệm vụ" | §6.1, §6.2 |
| 7 | Thêm shortcut keyword `sortu`, `sortdate`, `sortprio` vào bảng mapping | §6.1 |
| 8 | Thêm khung sub-menu sort sau menu chính | §6.2 |
| 9 | Cập nhật hàng "`std::stack` cho Undo" trong bảng §11 để phản ánh thiết kế mới | §11 |

## 5. Đề xuất khác (không áp dụng lần này)

- Thêm class `InputValidator` tập trung validate số/chuỗi/ngày — hiện đang validate tản mạn.
- Thêm phần "Hướng dẫn quay video demo" vào §10 để hỗ trợ TV4.

---

## 6. Kết luận

✅ **SPEC đã cover đầy đủ 100% chức năng bắt buộc trong Feature List** và còn vượt yêu cầu ở nhiều mặt (UX, test coverage, cross-platform).

✅ **2 điểm mâu thuẫn phát hiện lần review trước đã được fix trực tiếp trong `SPEC.md`**:
1. Container Undo đã chốt `std::deque<ProjectState>` + `using ProjectState = Project;` (§5.1).
2. Menu CLI đã expose đủ 3 chế độ sort qua sub-menu + shortcut `sortu`/`sortdate`/`sortprio` (§6).

📌 Sẵn sàng cho bước implement — có thể đưa `SPEC.md` cho Claude Code / Cursor / Copilot để sinh codebase theo đúng signature.
