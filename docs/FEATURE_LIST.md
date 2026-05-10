# CHRONOS — Trình Quản Lý Nhiệm Vụ & Thời Hạn Cá Nhân

> Bài tập lớn môn Lập trình Hướng đối tượng (OOP) — C++.
> Ngôn ngữ: C++ (sử dụng STL: `std::stack`, `std::vector`, `std::string` …).
> Hình thức: Ứng dụng CLI (Command Line Interface).
>
> Nguồn: trích nội dung từ `docs/Chronos_Feature_List.pdf`.

---

## 1. Tổng Quan Dự Án
Chronos là một công cụ dòng lệnh (CLI) giúp người dùng quản lý nhiệm vụ, theo dõi thời hạn và mức độ ưu tiên. Hệ thống tính toán điểm "Khẩn cấp" (Urgency Score) dựa trên độ ưu tiên và thời gian còn lại đến deadline, đồng thời cung cấp tính năng Hoàn tác (Undo) thông qua cấu trúc `std::stack`.

### Mục tiêu chính
- Theo dõi nhiệm vụ, thời hạn và mức độ ưu tiên.
- Tính toán điểm Khẩn cấp một cách thông minh.
- Quản lý **nhiều Dự án (Project) độc lập**: liệt kê, tạo, đổi tên, xóa, chuyển đổi qua lại.
- Quản lý nhiều nhiệm vụ trong từng dự án.
- Hỗ trợ Hoàn tác thao tác bằng `std::stack`.
- Xử lý các lỗi đặc biệt: Nghịch lý thời gian & Tràn bộ đệm.

## 2. Phân Chia Module Lớn

| STT | Module | Mô tả ngắn |
|-----|--------|------------|
| M1 | Core Domain | Lớp `Task`, `Project`, `Date`, `Exceptions`, nạp chồng toán tử |
| M2 | Logic & Tính toán | Tính điểm Khẩn cấp, sắp xếp, search/filter, validate, xử lý ngoại lệ |
| M3 | Persistence & Undo | Lưu/đọc file CSV, hệ thống Hoàn tác bằng `std::stack`, **`ProjectManager`** quản lý nhiều dự án (index + per-project files) |
| M4 | CLI & Trải nghiệm người dùng | **`OuterMenu`** (chọn / tạo / đổi tên / xóa dự án) + `MainMenu` (thao tác trong dự án), tô màu, demo, tài liệu, kiểm thử |

## 3. Chi Tiết Tính Năng Theo Module

### MODULE 1: CORE DOMAIN (Lớp & OOP cơ bản)

#### 1.1. Lớp `Task` (Nhiệm vụ)
- Thuộc tính:
  - `id` (int) — Mã định danh duy nhất
  - `title` (string) — Tên nhiệm vụ
  - `description` (string) — Mô tả chi tiết
  - `startDate` (Date) — Ngày bắt đầu
  - `dueDate` (Date) — Ngày hết hạn
  - `priority` (enum: LOW, MEDIUM, HIGH, CRITICAL) — Mức độ ưu tiên
  - `status` (enum: PENDING, IN_PROGRESS, DONE) — Trạng thái
- Phương thức:
  - Constructor (mặc định, tham số, copy)
  - Destructor
  - Getters / Setters
  - `display()` — In thông tin nhiệm vụ ra màn hình

#### 1.2. Lớp `Project` (Dự án)
- Thuộc tính:
  - `projectId`, `projectName`, `description`
  - `vector<Task> tasks` — Danh sách nhiệm vụ
- Phương thức:
  - `addTask(Task)`, `removeTask(int id)`
  - `getTaskById(int id)`
  - `getAllTasks()` — Trả về danh sách
  - `displayProject()` — Hiển thị toàn bộ dự án

#### 1.3. Lớp `Date` (Hỗ trợ)
- Lớp riêng để biểu diễn ngày tháng (`day`, `month`, `year`).
- So sánh hai ngày, tính khoảng cách giữa hai ngày.

#### 1.4. Nạp chồng toán tử (Operator Overloading)
- `operator<` — So sánh 2 nhiệm vụ theo độ ưu tiên rồi đến ngày deadline.
- `operator++` — Gia hạn thời hạn thêm 1 ngày (cả tiền tố `++task` và hậu tố `task++`).
- `operator==` — So sánh 2 nhiệm vụ có giống nhau không.
- `operator<<` — In thông tin task ra `ostream`.

### MODULE 2: LOGIC & TÍNH TOÁN

#### 2.1. Tính điểm Khẩn cấp (Urgency Score)
Công thức gợi ý:
```
Urgency = (Priority * 10) + (100 / (daysLeft + 1))
```
- Càng gần deadline & độ ưu tiên càng cao → điểm khẩn cấp càng lớn.
- Sắp xếp danh sách task theo điểm khẩn cấp giảm dần.

#### 2.2. Sắp xếp nhiệm vụ
- Dùng `std::sort` kết hợp `operator<` đã nạp chồng.
- Hỗ trợ sắp xếp theo: ưu tiên, deadline, điểm khẩn cấp.

#### 2.3. Validate dữ liệu & Xử lý ngoại lệ
- **Nghịch lý thời gian (Time Paradox):** Khi `dueDate < startDate` → ném `std::invalid_argument` hoặc class lỗi tự định nghĩa `TimeParadoxException`.
  - Thông báo: "Lỗi: Nghịch lý thời gian! Ngày hết hạn không thể trước ngày bắt đầu."
- **Tràn bộ đệm (Buffer Overflow Logic):**
  - Giới hạn số task trong project (VD: `MAX_TASKS = 9999`).
  - Khi user cố thêm 10.000 task trong vòng lặp → ném `BufferOverflowException`.
  - Thông báo: "Lỗi: Vượt quá giới hạn lưu trữ! Số nhiệm vụ tối đa là 9999."
- Validate input chuỗi, số, ngày tháng.

#### 2.4. Tìm kiếm & Lọc
- Tìm kiếm nhiệm vụ theo tên/từ khóa.
- Lọc theo priority, status, ngày deadline.

### MODULE 3: PERSISTENCE & UNDO

#### 3.1. Hệ thống Hoàn tác (Undo Stack)
- Sử dụng `std::stack<ProjectState>` để lưu các trạng thái trước đó.
- Trước mỗi thao tác thay đổi (add/delete/update/extend) → push trạng thái hiện tại vào stack.
- Lệnh `undo`: Lấy trạng thái trên đỉnh stack ra → khôi phục.
- Giới hạn số lần undo (VD: 50 thao tác gần nhất).

#### 3.2. Lưu trữ dữ liệu (File I/O)
- Lưu danh sách project & task vào file CSV (định dạng tự định nghĩa, có escape `,`).
- Tải lại khi mở chương trình.
- Hàm `saveToFile()`, `loadFromFile()`.

#### 3.3. Quản lý phiên làm việc
- Tự động lưu mỗi khi thoát chương trình hoặc chuyển dự án.
- Cảnh báo khi thoát mà chưa lưu.

#### 3.4. Quản Lý Nhiều Dự Án (`ProjectManager`)

> **Yêu cầu mới:** Chronos phải hỗ trợ **nhiều dự án độc lập** trên cùng một máy. Khi chạy chương trình, người dùng phải **chọn (hoặc tạo) một dự án** trước khi vào menu thao tác chính.

**Phạm vi:**
- Lưu trữ thông tin tổng quan của tất cả dự án trong một **file index** (`data/projects/index.csv`).
- Mỗi dự án có file CSV riêng (`data/projects/<projectId>.csv`) chứa toàn bộ task của dự án đó.
- Cho phép tạo, đổi tên, xóa dự án; mỗi `projectId` là duy nhất và auto-increment.
- Khi xóa dự án → xóa cả entry trong index và file CSV của dự án.
- Khi mở chương trình lần đầu (chưa có dự án nào) → wizard hỏi tạo dự án đầu tiên.
- Khi đã có dự án → bắt buộc chọn 1 dự án trước khi vào main menu.

**API gợi ý (`ProjectManager`):**
- `list()` — Trả về danh sách metadata của tất cả dự án.
- `create(name, description)` — Tạo project mới (sinh `projectId` mới, ghi index + tạo file CSV trống).
- `rename(id, newName)` — Đổi tên dự án.
- `deleteProject(id)` — Xóa entry khỏi index + xóa file CSV.
- `loadProject(id)` — Trả về `Project` đầy đủ (đọc từ file CSV của dự án đó).
- `saveProject(id, project)` — Ghi `Project` ra file CSV + cập nhật `modifiedAt` trong index.

**Use cases chính:**

| UC | Mô tả | Tiền điều kiện | Hậu điều kiện |
|----|-------|----------------|---------------|
| UC1 | Lần đầu chạy (chưa có dự án) | Chưa có `index.csv` hoặc index rỗng | Wizard tạo dự án đầu tiên → tự động chọn → vào main menu |
| UC2 | Chạy lại với 1+ dự án sẵn có | `index.csv` có ≥ 1 entry | Hiển thị danh sách → user chọn 1 → vào main menu |
| UC3 | Tạo dự án mới từ outer menu | Đang ở outer menu | Index có thêm entry mới, file CSV mới được tạo |
| UC4 | Đổi tên dự án | Có ≥ 1 dự án | Tên trong index thay đổi, file CSV không bị di dời |
| UC5 | Xóa dự án | Có ≥ 1 dự án | Entry biến mất khỏi index, file CSV bị xóa; nếu xóa hết → quay lại UC1 wizard |
| UC6 | Chuyển dự án giữa main menu | Đang trong 1 dự án (main menu) | Auto-save dự án hiện tại → quay về outer menu để chọn dự án khác |
| UC7 | Hiển thị tên dự án trong main menu | Đã chọn 1 dự án | Banner main menu luôn hiển thị `[#id] Tên dự án` |

**Edge cases / quy tắc:**
- Không cho xóa dự án mà không có confirm `Y/N`.
- Nếu xóa dự án **đang được chọn** → tự động quay về outer menu sau khi xóa.
- Tên dự án có thể trùng nhau (định danh bằng `projectId`).
- File CSV của dự án bị hỏng → outer menu vẫn liệt kê được nhưng khi mở sẽ báo lỗi và cho phép xóa.
- Migration: nếu trong `data/` còn file `chronos_save.csv` cũ (single-project format) → tự động import thành dự án đầu tiên trong index, sau đó di chuyển vào `data/projects/`.

### MODULE 4: CLI & TRẢI NGHIỆM NGƯỜI DÙNG

#### 4.1. User Flow Tổng Quát

```
          +------------------+
          |  App khởi động    |
          +---------+--------+
                    |
                    v
     ProjectManager.load("data/projects/index.csv")
                    |
             index rỗng?
             /         \
           Yes          No
            |            |
            v            v
   +----------------+   +-----------------+
   | Wizard tạo DA  |   |   OUTER MENU    |
   | đầu tiên (UC1) |   | (Project list)  |
   +-------+--------+   +--------+--------+
           |                     |
           |  user create/select |
           +--------+------------+
                    |
                    v
         +------------------------+
         |       MAIN MENU        |
         | Banner: [#id] Tên DA   |
         | Mọi thao tác task ở đây|
         +-----------+------------+
                     |
          back/switch | exit
                     v
             quay về OUTER MENU
```

#### 4.2. Outer Menu — Quản lý Dự Án

```
+========================================+
| CHRONOS - Quan ly Du an                |
+========================================+
  ID  | Ten Du An          | #Task | Cap nhat
  ----+--------------------+-------+-----------
  1   | Daily Tasks        | 12    | 2026-05-02 09:00
  2   | Q2 Sprint          | 7     | 2026-05-01 18:30
  ----+--------------------+-------+-----------
1. Mo du an (Select)
2. Tao du an moi (Create)
3. Doi ten du an (Rename)
4. Xoa du an (Delete)
0. Thoat chuong trinh
----------------------------------------
Lua chon:
```
Keyword: `select` (1), `new`/`create` (2), `rename` (3), `delete`/`rm` (4), `exit`/`q` (0).

#### 4.3. Main Menu — Thao tác trong dự án
```
+==========================================+
|  CHRONOS - Task Manager v1.0             |
|  Project: [#1] Daily Tasks               |   <-- LUÔN hiển thị tên dự án
+==========================================+
1. Doi ten du an hien tai          (rename)
2. Them nhiem vu                   (add)
3. Xem danh sach nhiem vu          (list/view/ls)
4. Sap xep nhiem vu                (sort)
5. Gia han deadline (+1 ngay)      (extend)
6. Hoan tac (Undo)                 (undo)
7. Luu / Tai du lieu               (save/load)
8. Tim kiem nhiem vu               (search/find)
9. Xoa nhiem vu                    (delete/rm)
10. Cap nhat trang thai            (status/update)
11. [DEMO] Tran bo dem 10000 task  (stress)
12. Quay ve danh sach du an        (back/switch)   <-- MỚI
0. Thoat chuong trinh              (exit/quit/q)
------------------------------------------
Lua chon cua ban:
```
- Slot `1` thay đổi từ "Tạo dự án mới" → **"Đổi tên dự án hiện tại"** (việc tạo project đã chuyển ra outer menu).
- Slot `12` mới: **"Quay về danh sách dự án"** — auto-save dự án hiện tại, quay về outer menu (KHÔNG thoát chương trình).
- Banner phải in cả `projectId` và `projectName` trên mọi lần render menu.
- Khi user chọn `0` (thoát) → auto-save dự án hiện tại + ghi index → kết thúc chương trình.

#### 4.4. CLI Input

Nhập lệnh bằng số hoặc từ khóa (case-insensitive). Mọi mode (outer/main) đều hỗ trợ cả 2 cách nhập. Có shortcut `sortu`/`sortdate`/`sortprio` cho main menu.

#### 4.5. Hiển thị đẹp mắt
- In bảng nhiệm vụ với cột rõ ràng (dùng `iomanip`).
- Tô màu (nếu hỗ trợ): khẩn cấp = đỏ, hoàn thành = xanh.
- Outer menu: tô màu dòng "Cập nhật gần nhất" (vd. màu xám cho dự án ≥ 30 ngày không đụng tới).

#### 4.6. Demo các trường hợp lỗi (cho video báo cáo)
- Minh họa Nghịch lý thời gian: nhập ngày hết hạn < ngày bắt đầu.
- Minh họa Tràn bộ đệm: vòng lặp `for` thêm 10.000 task.
- Minh họa Undo sau khi xóa nhiệm vụ.
- **Mới:** Minh họa multi-project flow: tạo 2 dự án → switch qua lại → xóa → quay về wizard.

#### 4.7. Tài liệu & Kiểm thử
- File `README.md` hướng dẫn build & sử dụng.
- Test case: thêm/xóa/sửa task, sắp xếp, undo, lỗi.
- **Mới:** Test case ProjectManager: tạo/đổi tên/xóa dự án, save/load index, migration từ format cũ.
- **Mới:** Test case OuterMenu: parse command, lifecycle (create → switch → delete).
- Báo cáo & video demo.

## 4. Phân Công 4 Thành Viên

### Thành viên 1 — Trưởng nhóm (Module 1: Core Domain)
- Vai trò: Kiến trúc sư hệ thống, chịu trách nhiệm phần "xương sống" của chương trình.
- Nhiệm vụ: xây dựng `Date`, `Task`, `Project`; nạp chồng `<`, `++`, `==`, `<<`; viết unit test cơ bản; thiết kế interface cho TV2/TV3 dùng.
- Sản phẩm bàn giao: `Date.h/cpp`, `Task.h/cpp`, `Project.h/cpp`.

### Thành viên 2 (Module 2: Logic & Tính toán)
- Vai trò: Chuyên gia thuật toán & xử lý lỗi.
- Nhiệm vụ: Urgency Score, sort, search/filter, custom exceptions (`TimeParadoxException`, `BufferOverflowException`), validate input, test case.
- Sản phẩm bàn giao: `UrgencyCalculator.h/cpp`, `Exceptions.h`, `TaskFilter.h/cpp`.

### Thành viên 3 (Module 3: Persistence & Undo)
- Vai trò: Chuyên gia lưu trữ & quản lý trạng thái.
- Nhiệm vụ: hệ thống Undo (`std::stack<ProjectState>`), `ProjectState`, `saveToFile()`/`loadFromFile()`, auto-save khi thoát, giới hạn 50 undo. **Mới:** thiết kế & implement `ProjectManager` (index + per-project files), migration từ format cũ.
- Sản phẩm bàn giao: `UndoManager.h/cpp`, `FileHandler.h/cpp`, `ProjectManager.h/cpp`.

### Thành viên 4 (Module 4: CLI & Trải nghiệm)
- Vai trò: Frontend CLI, Tester & người làm tài liệu/video.
- Nhiệm vụ: CLI menu, tích hợp module vào `main()`, in bảng `iomanip`, viết README, soạn kịch bản & quay video demo. **Mới:** tách 2 mode `OuterMenu` (project list) + `MainMenu` (per-project), banner project name trong main menu, lệnh `back`/`switch` để quay về outer menu.
- Sản phẩm bàn giao: `main.cpp`, `OuterMenu.h/cpp`, `Menu.h/cpp`, `README.md`, video demo, slide báo cáo.

## 5. Lộ Trình Đề Xuất (4 tuần)

| Tuần | Công việc | Người thực hiện |
|------|-----------|------------------|
| Tuần 1 | Thiết kế class diagram, chia interface | Cả nhóm họp; TV1 chốt thiết kế |
| Tuần 2 | TV1 hoàn thành Module 1; TV2, TV3 bắt đầu | TV1, TV2, TV3 |
| Tuần 3 | TV2 + TV3 hoàn thiện; TV4 bắt đầu CLI | TV2, TV3, TV4 |
| Tuần 4 | Tích hợp, debug, viết báo cáo, quay video | Cả nhóm |

## 6. Công Cụ Đề Xuất
- IDE: Visual Studio 2022 / VS Code + g++ / CLion
- Quản lý mã nguồn: Git + GitHub (mỗi thành viên 1 branch riêng)
- Quay video: OBS Studio
- Vẽ class diagram: draw.io / PlantUML

## 7. Tiêu Chí Đánh Giá
- Đầy đủ các tính năng OOP yêu cầu (class, operator overloading).
- Tính năng Undo hoạt động đúng với `std::stack`.
- Demo được 2 trường hợp lỗi đặc biệt.
- Code sạch, có comment, chia file rõ ràng.
- Báo cáo + video demo đầy đủ.
- Phân công nhóm công bằng & rõ ràng.
