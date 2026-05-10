# CHRONOS — Technical Specification (SPEC.md)

> Mục đích: File này là spec kỹ thuật đầy đủ để AI (Claude Code / Cursor / Copilot) có thể sinh ra toàn bộ codebase mà không cần đoán. Mọi class, hàm, format, edge case đều được định nghĩa rõ ràng.
>
> Nguồn: trích nội dung từ `docs/SPEC.pdf`.

## 0. Thông Tin Dự Án
- Tên: Chronos — Personal Task & Deadline Manager
- Ngôn ngữ: C++17
- Compiler: g++ 9.0+ / MSVC 2019+
- Build system: CMake 3.15+
- Platform: Cross-platform (Windows / Linux / macOS)
- Dependencies: Chỉ STL chuẩn, không dùng thư viện ngoài.
- Test framework: GoogleTest (gtest) — fetch qua CMake `FetchContent`.

## 1. Cấu Trúc Thư Mục

```
chronos/
├── CMakeLists.txt
├── README.md
├── SPEC.md
├── include/
│   ├── Date.h
│   ├── Task.h
│   ├── Project.h
│   ├── Exceptions.h
│   ├── UrgencyCalculator.h
│   ├── TaskFilter.h
│   ├── UndoManager.h
│   ├── FileHandler.h
│   ├── ProjectManager.h     # MỚI — quản lý nhiều dự án
│   ├── ConsoleColor.h
│   ├── OuterMenu.h          # MỚI — menu chọn dự án
│   └── Menu.h               # menu thao tác trong dự án (per-project)
├── src/
│   ├── Date.cpp
│   ├── Task.cpp
│   ├── Project.cpp
│   ├── UrgencyCalculator.cpp
│   ├── TaskFilter.cpp
│   ├── UndoManager.cpp
│   ├── FileHandler.cpp
│   ├── ProjectManager.cpp   # MỚI
│   ├── ConsoleColor.cpp
│   ├── OuterMenu.cpp        # MỚI
│   ├── Menu.cpp
│   └── main.cpp             # entry-point: ProjectManager + OuterMenu loop
├── tests/
│   ├── CMakeLists.txt
│   ├── unit/
│   │   ├── test_date.cpp
│   │   ├── test_task.cpp
│   │   ├── test_project.cpp
│   │   ├── test_urgency.cpp
│   │   ├── test_filter.cpp
│   │   ├── test_undo.cpp
│   │   ├── test_filehandler.cpp
│   │   ├── test_project_manager.cpp   # MỚI
│   │   └── test_exceptions.cpp
│   └── integration/
│       ├── test_full_workflow.cpp
│       ├── test_undo_workflow.cpp
│       ├── test_persistence.cpp
│       ├── test_error_scenarios.cpp
│       ├── test_menu_features.cpp
│       ├── test_console_color.cpp
│       └── test_project_lifecycle.cpp # MỚI — multi-project end-to-end
└── data/
    └── projects/
        ├── index.csv         (auto-generated)
        ├── 1.csv             (per-project file, auto-generated)
        └── 2.csv
```

> **Lưu ý migration:** Nếu còn file cũ `data/chronos_save.csv` từ phiên bản single-project, app sẽ tự động import thành `data/projects/1.csv` và ghi entry tương ứng vào `index.csv` (xem §5.3.4).

## 2. Convention & Quy Ước Chung
- Naming:
  - Class: `PascalCase` (vd: `Task`, `UndoManager`)
  - Method/variable: `camelCase` (vd: `addTask`, `dueDate`)
  - Constant/Enum value: `UPPER_SNAKE_CASE` (vd: `MAX_TASKS`, `HIGH`)
  - File: trùng tên class (vd: `Task.h`, `Task.cpp`)
- Memory: ưu tiên giá trị (value semantics). Không dùng raw pointer; nếu cần dùng smart pointer (`std::unique_ptr` / `std::shared_ptr`).
- Error handling: dùng exception (custom class kế thừa `std::exception`), KHÔNG dùng error code.
- Const-correctness: mọi getter và hàm không mutate đều phải `const`.
- Header guard: dùng `#pragma once`.
- Encoding: UTF-8.

## 3. Module 1: Core Domain

### 3.1. Class `Date`
File: `include/Date.h`, `src/Date.cpp`

```cpp
class Date {
private:
    int day;     // 1-31
    int month;   // 1-12
    int year;    // 1900-2100

public:
    // Constructors
    Date();                                   // Mặc định: 01/01/2000
    Date(int day, int month, int year);       // Throw InvalidDateException nếu sai

    // Getters
    int getDay() const;
    int getMonth() const;
    int getYear() const;

    // Setters (có validate)
    void setDay(int d);
    void setMonth(int m);
    void setYear(int y);

    // Methods
    bool isValid() const;                     // Kiểm tra ngày hợp lệ (kể cả năm nhuận)
    int daysBetween(const Date& other) const; // Số ngày giữa 2 date (có thể âm)
    Date addDays(int n) const;                // Trả về Date mới = this + n ngày
    std::string toString() const;             // Format "DD/MM/YYYY"
    static Date today();                      // Lấy ngày hôm nay từ system clock
    static Date fromString(const std::string& s); // Parse "DD/MM/YYYY"

    // Operator overload
    bool operator<(const Date& other) const;
    bool operator>(const Date& other) const;
    bool operator==(const Date& other) const;
    bool operator!=(const Date& other) const;
    bool operator<=(const Date& other) const;
    bool operator>=(const Date& other) const;
    friend std::ostream& operator<<(std::ostream& os, const Date& d);
};
```

Quy tắc năm nhuận: chia hết cho 4 VÀ (không chia hết cho 100 HOẶC chia hết cho 400).

### 3.2. Enum `Priority` & `Status`
File: `include/Task.h`

```cpp
enum class Priority {
    LOW = 1,
    MEDIUM = 2,
    HIGH = 3,
    CRITICAL = 4
};

enum class Status {
    PENDING,
    IN_PROGRESS,
    DONE
};

// Helper functions
std::string priorityToString(Priority p);
Priority priorityFromString(const std::string& s);
std::string statusToString(Status s);
Status statusFromString(const std::string& s);
```

### 3.3. Class `Task`
File: `include/Task.h`, `src/Task.cpp`

```cpp
class Task {
private:
    int id;
    std::string title;
    std::string description;
    Date startDate;
    Date dueDate;
    Priority priority;
    Status status;

    static int nextId; // Auto-increment ID

public:
    // Constructors
    Task();
    Task(const std::string& title,
         const std::string& description,
         const Date& startDate,
         const Date& dueDate,
         Priority priority);
    // Throw TimeParadoxException nếu dueDate < startDate

    Task(const Task& other);                // Copy constructor
    Task& operator=(const Task& other);     // Copy assignment
    ~Task();

    // Getters
    int getId() const;
    std::string getTitle() const;
    std::string getDescription() const;
    Date getStartDate() const;
    Date getDueDate() const;
    Priority getPriority() const;
    Status getStatus() const;

    // Setters (có validate)
    void setTitle(const std::string& t);
    void setDescription(const std::string& d);
    void setStartDate(const Date& d);       // Validate: startDate <= dueDate
    void setDueDate(const Date& d);         // Validate: dueDate >= startDate
    void setPriority(Priority p);
    void setStatus(Status s);

    // Business methods
    int daysLeft() const;                   // Số ngày còn lại đến deadline (có thể âm)
    bool isOverdue() const;                 // dueDate < today && status != DONE
    void display() const;                   // In ra console

    // Operator overload
    bool operator<(const Task& other) const;     // So sánh urgency, ưu tiên cao -> trước
    bool operator==(const Task& other) const;   // So sánh theo id
    Task& operator++();                         // Prefix: gia hạn dueDate +1 ngày
    Task operator++(int);                       // Postfix: gia hạn dueDate +1 ngày
    friend std::ostream& operator<<(std::ostream& os, const Task& t);

    // Static
    static void resetIdCounter();          // Dùng cho test
};
```

Quy tắc `operator<`:
1. So sánh `Priority` (CRITICAL > HIGH > MEDIUM > LOW).
2. Nếu bằng, so sánh `dueDate` (gần hơn → "nhỏ hơn" → đứng trước).
3. Trả về `true` nếu `this` "khẩn cấp hơn" `other`.

LƯU Ý: `operator<` ngược với so sánh số học thông thường — task khẩn cấp hơn được coi là "nhỏ hơn" để `std::sort` đưa lên đầu.

### 3.4. Class `Project`
File: `include/Project.h`, `src/Project.cpp`

```cpp
class Project {
private:
    int projectId;
    std::string projectName;
    std::string description;
    std::vector<Task> tasks;

    static const int MAX_TASKS = 9999;
    static int nextProjectId;

public:
    // Constructors
    Project();
    Project(const std::string& name, const std::string& description);

    // Getters
    int getProjectId() const;
    std::string getProjectName() const;
    std::string getDescription() const;
    const std::vector<Task>& getAllTasks() const;
    int getTaskCount() const;

    // Setters
    void setProjectName(const std::string& name);
    void setDescription(const std::string& desc);

    // Task management
    void addTask(const Task& t);           // Throw BufferOverflowException nếu >= MAX_TASKS
    bool removeTask(int taskId);           // Trả về true nếu xóa thành công
    Task* getTaskById(int taskId);         // Trả về nullptr nếu không tìm thấy
    bool updateTask(int taskId, const Task& newTask);

    // Display
    void displayProject() const;
    void displayAllTasks() const;          // In bảng đẹp với iomanip

    // Comparison (cho UndoManager so sánh state)
    bool operator==(const Project& other) const;
};
```

## 4. Module 2: Logic & Tính Toán

### 4.1. Class `UrgencyCalculator`
File: `include/UrgencyCalculator.h`, `src/UrgencyCalculator.cpp`

```cpp
class UrgencyCalculator {
public:
    // Công thức: urgency = (priority * 10) + (100 / (daysLeft + 1))
    // Nếu daysLeft < 0 (overdue) -> urgency += 1000 (ưu tiên cực cao)
    // Nếu status == DONE -> urgency = 0
    static double calculate(const Task& task);

    static void sortByUrgency(std::vector<Task>& tasks);
    static void sortByDeadline(std::vector<Task>& tasks);
    static void sortByPriority(std::vector<Task>& tasks);
};
```

Công thức chi tiết:
```
if (status == DONE):
    return 0.0
daysLeft = task.daysLeft()
base = (int)priority * 10
if (daysLeft < 0):
    return base + 1000.0    // overdue boost
else:
    return base + (100.0 / (daysLeft + 1))
```

### 4.2. Class `TaskFilter`
File: `include/TaskFilter.h`, `src/TaskFilter.cpp`

```cpp
class TaskFilter {
public:
    static std::vector<Task> searchByKeyword(const std::vector<Task>& tasks,
                                             const std::string& keyword);
    static std::vector<Task> filterByPriority(const std::vector<Task>& tasks, Priority p);
    static std::vector<Task> filterByStatus(const std::vector<Task>& tasks, Status s);
    static std::vector<Task> filterByDateRange(const std::vector<Task>& tasks,
                                               const Date& from, const Date& to);
    static std::vector<Task> getOverdueTasks(const std::vector<Task>& tasks);
};
```

### 4.3. Custom Exceptions
File: `include/Exceptions.h`

```cpp
#include <stdexcept>

class TimeParadoxException : public std::runtime_error {
public:
    TimeParadoxException()
        : std::runtime_error(
            "Lỗi: Nghịch lý thời gian! Ngày hết hạn không thể trước ngày bắt đầu.") {}
};

class BufferOverflowException : public std::runtime_error {
public:
    BufferOverflowException()
        : std::runtime_error(
            "Lỗi: Vượt quá giới hạn lưu trữ! Số nhiệm vụ tối đa là 9999.") {}
};

class InvalidDateException : public std::runtime_error {
public:
    InvalidDateException(const std::string& msg)
        : std::runtime_error("Lỗi ngày không hợp lệ: " + msg) {}
};

class TaskNotFoundException : public std::runtime_error {
public:
    TaskNotFoundException(int id)
        : std::runtime_error("Không tìm thấy nhiệm vụ với ID = " + std::to_string(id)) {}
};

class FileIOException : public std::runtime_error {
public:
    FileIOException(const std::string& msg)
        : std::runtime_error("Lỗi I/O file: " + msg) {}
};

class UndoStackEmptyException : public std::runtime_error {
public:
    UndoStackEmptyException()
        : std::runtime_error("Không còn thao tác nào để hoàn tác.") {}
};
```

## 5. Module 3: Persistence & Undo

### 5.1. Class `UndoManager`
File: `include/UndoManager.h`, `src/UndoManager.cpp`

```cpp
// Alias để khớp tên kiểu trong Feature List ("std::stack<ProjectState>").
// Snapshot của Project chính là một ProjectState. Nếu sau này muốn lưu thêm
// metadata (vd: tên thao tác đã thực hiện), chỉ cần đổi alias thành 1 struct
// chứa Project + metadata mà không phá vỡ API public.
using ProjectState = Project;

class UndoManager {
private:
    // GHI CHÚ THIẾT KẾ: Yêu cầu Feature List là "Undo bằng std::stack". std::stack
    // KHÔNG cho phép xóa phần tử ở đáy, nên không thể giới hạn 50 phần tử (sẽ
    // ngốn bộ nhớ vô hạn). Vì vậy ta dùng std::deque làm storage và mô phỏng
    // API LIFO của stack qua push_back/back/pop_back. Vẫn được tính là
    // "stack-based undo" theo tinh thần OOP của đề bài.
    std::deque<ProjectState> history;
    static const int MAX_UNDO_DEPTH = 50;

public:
    UndoManager();

    // Lưu snapshot trước khi thay đổi.
    // Nếu size() đã đạt MAX_UNDO_DEPTH, snapshot cũ nhất (history.front())
    // sẽ bị pop_front trước khi push state mới.
    void saveState(const ProjectState& currentState);

    // Khôi phục state gần nhất (LIFO). Throw UndoStackEmptyException nếu stack rỗng.
    ProjectState undo();

    // Xóa toàn bộ history
    void clear();

    // Số state đang lưu
    size_t size() const;
    bool isEmpty() const;
};
```

Quy tắc:
- Trước MỖI thao tác mutate (`addTask`, `removeTask`, `updateTask`, gia hạn deadline, đổi status, đổi tên project…) → gọi `saveState()`.
- Nếu `size() == MAX_UNDO_DEPTH` khi `saveState()`, gọi `history.pop_front()` để xóa state cũ nhất rồi mới `push_back()` state mới.
- `undo()` đọc `history.back()` rồi `pop_back()` (LIFO). Nếu rỗng → throw `UndoStackEmptyException`.
- **Vẫn thỏa yêu cầu "Undo Stack" của Feature List §3.1**: API public hành xử như stack (LIFO), `ProjectState` đúng tên Feature List. `std::deque` chỉ là chi tiết implement bên trong (giải thích trong báo cáo).

### 5.2. Class `FileHandler`
File: `include/FileHandler.h`, `src/FileHandler.cpp`

```cpp
class FileHandler {
public:
    static void saveProject(const Project& project, const std::string& filepath);
    static Project loadProject(const std::string& filepath);
    static bool fileExists(const std::string& filepath);
};
```

Format file CSV per-project (`data/projects/<id>.csv`):
```
# PROJECT
projectId,projectName,description
1,My Project,Description here

# TASKS
taskId,title,description,startDate,dueDate,priority,status
1,Buy groceries,Milk and eggs,01/05/2026,03/05/2026,HIGH,PENDING
2,Write report,Q2 financial report,02/05/2026,10/05/2026,CRITICAL,IN_PROGRESS
```

Quy tắc:
- Dấu `,` trong text được escape bằng cách bao trong dấu `"`.
- Khi đọc file lỗi → throw `FileIOException`.
- Khi file không tồn tại → throw `FileIOException`.
- `FileHandler` vẫn chịu trách nhiệm `saveProject()`/`loadProject()` cho 1 file CSV; `ProjectManager` (§5.3) chịu trách nhiệm biết `filepath` nào ngứng với `projectId` nào.

### 5.3. Class `ProjectManager` (MỚI)
File: `include/ProjectManager.h`, `src/ProjectManager.cpp`

> Mục tiêu: quản lý **danh sách nhiều dự án** với index file riêng. Mỗi dự án có một file CSV riêng. Tách hẳn khỏi `Menu` để có thể unit-test độc lập.

```cpp
struct ProjectMeta {
    int id;                          // duy nhất, auto-increment
    std::string name;                // có thể trùng
    std::string description;
    std::string savePath;            // "data/projects/<id>.csv"
    std::string createdAt;           // "YYYY-MM-DD HH:MM:SS" (UTC hoặc local, cố định local cho đơn giản)
    std::string modifiedAt;          // cập nhật mỗi lần saveProject()
    int taskCount = 0;               // cache số task hiện tại (để hiển thị ở outer menu mà không load cả file)
};

class ProjectManager {
private:
    std::string dataDir;             // "data/projects"
    std::string indexPath;           // "data/projects/index.csv"
    std::vector<ProjectMeta> metas;  // load từ index khi khởi tạo
    int nextId = 1;                  // lưu trong index, không reuse id đã xóa

public:
    explicit ProjectManager(const std::string& dataDir = "data/projects");

    // Persistence của chính index
    void load();                                // Đọc index.csv (nếu không tồn tại → tạo file rỗng + thư mục)
    void save() const;                          // Ghi index.csv

    // Truy vấn
    const std::vector<ProjectMeta>& list() const;
    bool isEmpty() const;
    const ProjectMeta* findById(int id) const;

    // CRUD mức project
    ProjectMeta create(const std::string& name, const std::string& description);
    void rename(int id, const std::string& newName);
    void deleteProject(int id);                 // Xóa entry + file CSV

    // Truy cập dữ liệu thực của project
    Project loadProject(int id);                // FileHandler::loadProject(meta.savePath)
    void saveProject(int id, const Project& project); // FileHandler::saveProject + update modifiedAt + taskCount + save()

    // Migration từ format cũ
    void migrateLegacySaveIfPresent(const std::string& legacyPath = "data/chronos_save.csv");
};
```

#### 5.3.1. Format `data/projects/index.csv`
```
# CHRONOS_INDEX v1
nextId,3
# PROJECTS
id,name,description,savePath,createdAt,modifiedAt,taskCount
1,Daily Tasks,Personal todo,data/projects/1.csv,2026-05-01 09:00:00,2026-05-02 09:30:12,12
2,Q2 Sprint,Work board,data/projects/2.csv,2026-05-02 14:10:00,2026-05-02 18:45:00,7
```
- Dòng `nextId,<n>` luôn nằm sau header `# CHRONOS_INDEX v1`. Tăng mỗi lần `create()`. KHÔNG reuse khi `delete()`.
- Escape `,` trong text giống §5.2.
- Index rỗng hợp lệ: chỉ có 2 dòng header + `nextId,1`, không có dòng project nào.

#### 5.3.2. Quy tắc tạo & xóa
- `create(name, desc)`:
  1. Cấp `id = nextId`, `nextId += 1`.
  2. Sinh `savePath = dataDir + "/" + id + ".csv"`.
  3. Tạo `Project` với `(id, name, desc)`, `FileHandler::saveProject(...)` ra `savePath`.
  4. Đặt `createdAt = modifiedAt = now()`, `taskCount = 0`, push vào `metas`.
  5. Gọi `save()` để ghi index.
  6. Trả về `ProjectMeta` vừa tạo.
- `deleteProject(id)`:
  1. Tìm meta theo id; nếu không tìm được → throw `ProjectNotFoundException`.
  2. Xóa file CSV tại `savePath` (nếu tồn tại).
  3. Erase entry khỏi `metas`.
  4. Gọi `save()` để ghi index.
- `rename(id, newName)`:
  1. Update `name` trong meta.
  2. Cập nhật `modifiedAt = now()`.
  3. Gọi `save()`. (KHÔNG cần sửa file CSV của project; nếu muốn đồng bộ triệt để, có thể load → đổi `projectName` → save lại — OPTIONAL.)

#### 5.3.3. Exceptions mới (thêm vào `Exceptions.h`)
```cpp
class ProjectNotFoundException : public std::runtime_error {
public:
    explicit ProjectNotFoundException(int id)
        : std::runtime_error("Không tìm thấy dự án có ID " + std::to_string(id)) {}
};
```
`FileIOException` vẫn phục vụ cho lỗi I/O phân cấp index/per-project.

#### 5.3.4. Migration từ format cũ
- Nếu `data/projects/index.csv` KHÔNG tồn tại và `data/chronos_save.csv` tồn tại:
  1. Tạo thư mục `data/projects/`.
  2. Load file cũ bằng `FileHandler::loadProject("data/chronos_save.csv")` → `Project legacy`.
  3. Sinh id = 1, savePath = `data/projects/1.csv`, cập nhật `legacy.projectId = 1`.
  4. Ghi `legacy` vào `data/projects/1.csv`; ghi entry index và `nextId=2`.
  5. Rename file cũ thành `data/chronos_save.csv.bak` (giữ lại phòng trường hợp), hoặc xóa — tùy. SPEC chốt: rename `.bak` để an toàn.
- Lỗi trong quá trình migration → in cảnh báo, bỏ qua, app vẫn khởi động với index rỗng (→ wizard tạo mới).

## 6. Module 4: CLI & Menu

> **Kiến trúc 2 tầng:** `OuterMenu` (mới) chịu trách nhiệm chọn / tạo / xóa / đổi tên dự án. Sau khi user chọn 1 dự án, `OuterMenu` instantiates `Menu` và chạy vòng lặp main menu cho đúng dự án đó. `Menu::run()` kết thúc khi user chọn `0` (exit) hoặc `12` (back to outer).

### 6.1. Class `OuterMenu` (MỚI)
File: `include/OuterMenu.h`, `src/OuterMenu.cpp`

```cpp
class OuterMenu {
private:
    ProjectManager& pm;

public:
    explicit OuterMenu(ProjectManager& pm);

    // Vòng lặp outer; trở về khi user chọn Thoát.
    void run();

private:
    void displayHeader() const;          // Banner + bảng project list
    void displayProjectList() const;     // Bảng id/name/taskCount/modifiedAt
    void displayMenuOptions() const;     // Các lệnh select/create/rename/delete/exit
    int  parseCommand(const std::string& input) const; // số hoặc keyword

    // Use cases
    void firstLaunchWizard();            // UC1: chưa có dự án nào
    void handleSelect();                 // UC2/UC6: chọn 1 dự án -> Menu
    void handleCreate();                 // UC3
    void handleRename();                 // UC4
    void handleDelete();                 // UC5
    void enterMainMenu(int projectId);   // load Project, tạo Menu, run, save khi return

    // Helpers (có thể lặp lại với Menu — cân nhắc tách ra IOUtil sau)
    int readInt(const std::string& prompt) const;
    std::string readString(const std::string& prompt) const;
    bool readYesNo(const std::string& prompt) const;
    void pressEnterToContinue() const;
};
```

Quy tắc lệnh outer (số / keyword, case-insensitive):

| Số | Keyword | Hành động |
|----|---------|-----------|
| 1 | `select`, `open` | Mở dự án: prompt "Nhập ID:" → `enterMainMenu(id)` |
| 2 | `new`, `create` | Tạo dự án mới (hỏi name, description) |
| 3 | `rename` | Đổi tên (hỏi id + name mới) |
| 4 | `delete`, `rm`, `del` | Xóa dự án (hỏi id, confirm Y/N) |
| 0 | `exit`, `quit`, `q` | Thoát chương trình |

Flow `OuterMenu::run()` (pseudo-code):
```cpp
void OuterMenu::run() {
    pm.load();
    pm.migrateLegacySaveIfPresent();
    if (pm.isEmpty()) {
        firstLaunchWizard();   // sau wizard, tự động enterMainMenu(idMỚI)
    }
    while (true) {
        displayHeader();
        std::string in = readString("Lựa chọn: ");
        switch (parseCommand(in)) {
            case 1: handleSelect();   break;
            case 2: handleCreate();   break;
            case 3: handleRename();   break;
            case 4: handleDelete();   break;
            case 0: return;
            default:
                std::cout << "Lệnh không hợp lệ.\n";
        }
    }
}
```

`enterMainMenu(int projectId)`:
```cpp
void OuterMenu::enterMainMenu(int projectId) {
    Project p = pm.loadProject(projectId);
    Menu menu(p, projectId, pm);  // truyền project + id + tham chiếu pm
    menu.run();                    // trở về khi user chọn 0 hoặc 12
    if (menu.shouldExitProgram()) {
        std::exit(0);              // user gõ 0 trong main menu → exit chương trình
    }
    // nếu user chọn 12 (back) → quay về outer loop bình thường
}
```

### 6.2. Class `Menu` (SỬA)
File: `include/Menu.h`, `src/Menu.cpp`

```cpp
class Menu {
private:
    Project currentProject;          // load từ ProjectManager
    int     projectId;               // id của project hiện tại (để ghi về đúng file)
    ProjectManager* pm;              // tham chiếu để save lại + rename + check tồn tại
    UndoManager undoManager;
    bool isDirty = false;
    bool exitProgram = false;        // true nếu user chọn `0`
    bool backToOuter = false;        // true nếu user chọn `12`

public:
    Menu(const Project& project, int projectId, ProjectManager& manager);

    // Vòng lặp main menu; kết thúc khi exitProgram || backToOuter.
    void run();

    bool shouldExitProgram() const { return exitProgram; }

private:
    // Hiển thị
    void displayMainMenu();          // banner LUÔN in `[#id] Tên dự án`
    void displayTaskTable(const std::vector<Task>& tasks);

    // Parse command (số HOẶC từ khóa)
    int parseCommand(const std::string& input);

    // Xử lý lựa chọn (note: đã bỏ handleCreateProject — chuyển ra OuterMenu)
    void handleRenameCurrentProject();    // MỚI — thay slot 1
    void handleAddTask();
    void handleViewTasks();
    void handleSort();                    // Sub-menu chọn 1 trong 3
    void handleExtendDeadline();          // operator++
    void handleUndo();
    void handleSaveLoad();                // phải dùng pm->saveProject(projectId, currentProject)
    void handleSearch();
    void handleDeleteTask();
    void handleUpdateStatus();
    void handleStressTest();              // Demo Buffer Overflow
    void handleBackToOuter();             // MỚI — slot 12: auto-save → quay về outer
    void handleExit();                    // slot 0: auto-save + warn isDirty + set exitProgram

    // Helpers (giữ nguyên)
    int readInt(const std::string& prompt);
    std::string readString(const std::string& prompt);
    Date readDate(const std::string& prompt);
    Priority readPriority();
    bool readYesNo(const std::string& prompt);
    void pressEnterToContinue();

    void markDirty() { isDirty = true; }
    void markClean() { isDirty = false; }
};
```

Quy tắc nhập command (số HOẶC từ khóa, case-insensitive):

| Số | Từ khóa hỗ trợ |
|----|----------------|
| 1 | `rename` (đổi tên dự án hiện tại) |
| 2 | `add` |
| 3 | `list`, `view`, `ls` |
| 4 | `sort` (sau đó hỏi sub-menu); shortcut: `sortu` (urgency), `sortdate` (deadline), `sortprio` (priority) |
| 5 | `extend` |
| 6 | `undo` |
| 7 | `save`, `load` |
| 8 | `search`, `find` |
| 9 | `delete`, `remove`, `rm` |
| 10 | `status`, `update` |
| 11 | `stress` |
| 12 | `back`, `switch`, `projects` (quay về outer menu) |
| 0 | `exit`, `quit`, `q` |

Hàm `parseCommand()` chuyển input về số tương ứng. Nếu không nhận diện được → trả về `-1` (in lỗi và prompt lại). Các shortcut `sortu`, `sortdate`, `sortprio` cũng map về `4` nhưng đồng thời lưu chế độ sort tương ứng để bỏ qua sub-menu.

> Lưu ý: lệnh cũ `1 = new/create` (tạo dự án mới) đã chuyển ra `OuterMenu`. Slot 1 trong main menu nay là "Đổi tên dự án hiện tại". User muốn tạo dự án mới → gõ `12` hoặc `back` để quay ra outer menu rồi chọn `2` tại đó.

### 6.3. Format Outer Menu (output mẫu)
```
╔════════════════════════════════════╗
║    CHRONOS - Quản lý Dự án         ║
╚════════════════════════════════════╝
┌────┬───────────────────────┬───────┬───────────────────┐
│ ID │ Tên dự án             │ #Task │ Cập nhật          │
├────┼───────────────────────┼───────┼───────────────────┤
│ 1  │ Daily Tasks           │  12   │ 2026-05-02 09:00  │
│ 2  │ Q2 Sprint             │   7   │ 2026-05-01 18:30  │
└────┴───────────────────────┴───────┴───────────────────┘
1. Mở dự án (Select)
2. Tạo dự án mới (Create)
3. Đổi tên dự án (Rename)
4. Xóa dự án (Delete)
0. Thoát chương trình
─────────────────────────────────────
Lựa chọn:
```

Wizard lần đầu (UC1 — khi `pm.isEmpty()`):
```
╔════════════════════════════════════╗
║ Chào mừng tới CHRONOS!             ║
║ Bạn chưa có dự án nào.             ║
║ Hãy tạo dự án đầu tiên.            ║
╚════════════════════════════════════╝
Tên dự án: ____
Mô tả:    ____
```
Sau khi tạo, tự động `enterMainMenu(newId)` luôn.

### 6.4. Format Main Menu (output mẫu)
```
╔══════════════════════════════════════════╗
║  CHRONOS — Task Manager v1.0             ║
║  Project: [#1] Daily Tasks               ║   <-- LUÔN hiển thị
╚══════════════════════════════════════════╝
1.  Đổi tên dự án hiện tại
2.  Thêm nhiệm vụ
3.  Xem danh sách nhiệm vụ
4.  Sắp xếp nhiệm vụ
5.  Gia hạn deadline (+1 ngày)
6.  Hoàn tác (Undo)
7.  Lưu / Tải dữ liệu
8.  Tìm kiếm nhiệm vụ
9.  Xóa nhiệm vụ
10. Cập nhật trạng thái
11. [DEMO] Tràn bộ đệm 10000 task
12. ← Quay về danh sách dự án          (back / switch)
0.  Thoát chương trình
──────────────────────────────────────────
Lựa chọn của bạn:
```

Quy tắc hành động cuối:
- `12` (back) → nếu `isDirty`, hỏi save? Y/N/Cancel; Cancel → ở lại. Y/N → set `backToOuter = true`, return khỏi `run()`. Khi đó `OuterMenu` tiếp tục vòng lặp outer.
- `0` (exit) → logic giống `12` nhưng cuối đặt `exitProgram = true` thay vì `backToOuter`.
- Bất kỳ thao tác `saveProject()` nào phải đi qua `pm->saveProject(projectId, currentProject)` để cập nhật cả file CSV lẫn `index.csv` (modifiedAt + taskCount).

### 6.5. Format bảng task (dùng `iomanip` + tô màu ANSI)
```
┌─────┬──────────────────────┬─────────────┬─────────────┬──────────┬─────────────┬─────────┐
│ ID  │ Tiêu đề              │ Bắt đầu     │ Hết hạn     │ Ưu tiên  │ Trạng thái  │ Khẩn cấp│
├─────┼──────────────────────┼─────────────┼─────────────┼──────────┼─────────────┼─────────┤
│ 1   │ Buy groceries        │ 01/05/2026  │ 03/05/2026  │ HIGH     │ PENDING     │ 80.00   │
│ 2   │ Write report         │ 02/05/2026  │ 10/05/2026  │ CRITICAL │ IN_PROGRESS │ 51.11   │
└─────┴──────────────────────┴─────────────┴─────────────┴──────────┴─────────────┴─────────┘
```

### 6.4. Tô màu console (ANSI escape codes)
File: `include/ConsoleColor.h`

```cpp
namespace ConsoleColor {
    constexpr const char* RESET   = "\033[0m";
    constexpr const char* RED     = "\033[31m";
    constexpr const char* GREEN   = "\033[32m";
    constexpr const char* YELLOW  = "\033[33m";
    constexpr const char* BLUE    = "\033[34m";
    constexpr const char* MAGENTA = "\033[35m";
    constexpr const char* CYAN    = "\033[36m";
    constexpr const char* BOLD    = "\033[1m";

    // Bật ANSI trên Windows 10+ (gọi 1 lần khi khởi động)
    void enableAnsiOnWindows();
}
```

Quy tắc tô màu task khi hiển thị:

| Điều kiện | Màu cả dòng |
|-----------|-------------|
| `status == DONE` | GREEN |
| `isOverdue() == true` | RED |
| `priority == CRITICAL && chưa done` | RED + BOLD |
| `priority == HIGH && chưa done` | YELLOW |
| `urgency >= 50 (gần deadline)` | YELLOW |
| Còn lại | (không màu) |

Lưu ý cross-platform:
- Linux/macOS: ANSI hoạt động mặc định.
- Windows 10+: cần gọi `enableAnsiOnWindows()` trong `main()` để bật virtual terminal.
- Windows cũ: code vẫn chạy nhưng màu không hiển thị (in escape sequence raw) — chấp nhận được.

### 6.5. Auto-save & quản lý phiên làm việc
Quy tắc:
- Khi khởi động: nếu `data/chronos_save.csv` tồn tại → tự động `loadProject()` vào `currentProject`. Lỗi đọc file → in cảnh báo, khởi tạo project rỗng.
- Mọi thao tác mutate (`addTask`, `removeTask`, `updateTask`, gia hạn, đổi status, đổi tên project) → `markDirty()`.
- Mọi thao tác `saveProject()` thành công → `markClean()`.
- Khi chọn lệnh Thoát (option 0):
  1. Nếu `isDirty == true`:
     - Hỏi: "Bạn có thay đổi chưa lưu. Lưu trước khi thoát? (Y/N/Cancel)"
     - `Y` → gọi `saveProject()` rồi exit.
     - `N` → exit không lưu.
     - `Cancel` → quay lại menu chính.
  2. Nếu `isDirty == false`:
     - Auto-save (ghi đè file để cập nhật timestamp/format) rồi exit.
- Khi nhận `SIGINT` (Ctrl+C): cố gắng auto-save bằng signal handler trước khi exit (best-effort, có thể bỏ qua nếu phức tạp).

Pseudo-code `handleExit()`:
```cpp
void Menu::handleExit() {
    if (isDirty) {
        std::cout << "Bạn có thay đổi chưa lưu. Lưu trước khi thoát? (Y/N/C): ";
        std::string ans = readString("");
        std::transform(ans.begin(), ans.end(), ans.begin(), ::tolower);
        if (ans == "y" || ans == "yes") {
            FileHandler::saveProject(currentProject, saveFilePath);
            std::cout << "Đã lưu. Tạm biệt!\n";
            std::exit(0);
        } else if (ans == "n" || ans == "no") {
            std::cout << "Thoát không lưu. Tạm biệt!\n";
            std::exit(0);
        } else {
            // Cancel: quay lại menu
            return;
        }
    } else {
        // Không có thay đổi: auto-save im lặng
        try {
            FileHandler::saveProject(currentProject, saveFilePath);
        } catch (...) { /* bỏ qua */ }
        std::cout << "Tạm biệt!\n";
        std::exit(0);
    }
}
```

## 7. Unit Tests
> Framework: GoogleTest. Mỗi test file độc lập, có `main()` từ `gtest_main`.

Xem file `docs/SPEC.pdf` (mục §7) để tham khảo trọn bộ test cases mẫu cho:
`test_date.cpp`, `test_task.cpp`, `test_project.cpp`, `test_urgency.cpp`,
`test_filter.cpp`, `test_undo.cpp`, `test_filehandler.cpp`, `test_exceptions.cpp`.

### 7.x. `test_project_manager.cpp` (MỚI)
Các test bắt buộc (sử dụng thư mục tạm, vd. `gtest`'s `testing::TempDir()`):
- `Empty_NoIndex_Returns_IsEmptyTrue`: khởi tạo `ProjectManager` với thư mục rỗng → `isEmpty()==true`, `list().empty()`.
- `Create_AssignsNewIdAndPersists`: `create("A","")` → trong list có 1 entry, `id=1`, file `1.csv` tồn tại; sau khi tạo `B` → `id=2`.
- `Create_DoesNotReuseIdAfterDelete`: tạo A,B,C → xóa B → tạo D → `D.id==4` (không reuse 2).
- `Delete_RemovesEntryAndFile`: tạo A → `deleteProject(A.id)` → list rỗng và file CSV đã bị xóa.
- `Delete_NotFound_Throws_ProjectNotFoundException`.
- `LoadProject_RoundTrip`: tạo A → `loadProject(A.id)` → thêm task → `saveProject(A.id, p)` → reload → task vẫn còn.
- `SaveProject_UpdatesModifiedAtAndTaskCount`: sau save, meta `modifiedAt` thay đổi và `taskCount` khớp số task.
- `LoadAndSavePersistsAcrossInstances`: PM1.create A → PM2 mới `load()` thấy A.
- `MigrationFromLegacyFile`: đặt sẵn `data/chronos_save.csv` → `migrateLegacySaveIfPresent()` → có entry id=1, file `1.csv` tồn tại, file cũ được rename `.bak`.
- `Rename_UpdatesNameAndModifiedAt`.

## 8. Integration Tests
Xem file `docs/SPEC.pdf` (mục §8) để tham khảo:
`test_full_workflow.cpp`, `test_undo_workflow.cpp`, `test_persistence.cpp`,
`test_error_scenarios.cpp`, `test_menu_features.cpp`, `test_console_color.cpp`.

### 8.x. `test_project_lifecycle.cpp` (MỚI)
Kiểm thử luồng end-to-end ProjectManager + lớp Menu (không test bàn phím; gọi trực tiếp API):
- `FirstLaunch_EmptyDir_WizardCreatesFirstProject`: PM trống → gọi `pm.create(...)` → `pm.loadProject(id)` thành công → tồn tại đúng 1 dự án.
- `MultipleProjects_SwitchPreservesData`: tạo A & B, thêm 2 task vào A và 1 task vào B → reload PM → A vẫn còn 2 task, B vẫn còn 1 task.
- `DeleteCurrentProject_RemovesEntry`: tạo A & B, xóa A → list chỉ còn B → reload xem index.
- `DeleteAllProjects_GoesBackToWizard`: tạo A, xóa A → `pm.isEmpty()==true`.
- `RenameProject_PersistsAcrossReload`.

## 9. CMakeLists.txt

### 9.1. Root `CMakeLists.txt`
```cmake
cmake_minimum_required(VERSION 3.15)
project(Chronos CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compile options
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Main library
add_library(chronos_lib
    src/Date.cpp
    src/Task.cpp
    src/Project.cpp
    src/UrgencyCalculator.cpp
    src/TaskFilter.cpp
    src/UndoManager.cpp
    src/FileHandler.cpp
    src/ProjectManager.cpp   # MỚI
    src/ConsoleColor.cpp
    src/OuterMenu.cpp        # MỚI
    src/Menu.cpp
)
target_include_directories(chronos_lib PUBLIC include)

# Main executable
add_executable(chronos src/main.cpp)
target_link_libraries(chronos PRIVATE chronos_lib)

# Tests
enable_testing()
add_subdirectory(tests)
```

### 9.2. `tests/CMakeLists.txt`
```cmake
include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG release-1.12.1
)
FetchContent_MakeAvailable(googletest)

# Unit tests
file(GLOB UNIT_TEST_SOURCES unit/*.cpp)
add_executable(unit_tests ${UNIT_TEST_SOURCES})
target_link_libraries(unit_tests PRIVATE chronos_lib gtest_main)
add_test(NAME unit_tests COMMAND unit_tests)

# Integration tests
file(GLOB INTEGRATION_TEST_SOURCES integration/*.cpp)
add_executable(integration_tests ${INTEGRATION_TEST_SOURCES})
target_link_libraries(integration_tests PRIVATE chronos_lib gtest_main)
add_test(NAME integration_tests COMMAND integration_tests)
```

## 10. Hướng Dẫn Build & Run

```bash
# Build
mkdir build && cd build
cmake ..
cmake --build .

# Run main
./chronos

# Run tests
ctest --output-on-failure

# Hoặc chạy trực tiếp
./tests/unit_tests
./tests/integration_tests
```

## 11. Tóm Tắt Yêu Cầu Đã Cover

| Yêu cầu đề bài | Đã cover ở mục |
|----------------|----------------|
| Lớp `Task` & `Project` | §3.3, §3.4 |
| `operator<` (sort) | §3.3 (operator<), §4.1 (sortByUrgency) |
| `operator++` (gia hạn) | §3.3 (prefix & postfix) |
| `std::stack`-style Undo (Feature List §3.1: `std::stack<ProjectState>`) | §5.1 (LIFO API qua `std::deque<ProjectState>` + `using ProjectState = Project;`) |
| Time Paradox | §3.3 (constructor), §4.3, §7.2, §8.4 |
| Buffer Overflow (10000 task) | §3.4 (`MAX_TASKS`), §4.3, §7.3, §8.4 |
| Điểm Khẩn cấp | §4.1 |
| Unit test | §7 (9 file gồm `test_project_manager.cpp`) |
| Integration test | §8 (7 file gồm `test_project_lifecycle.cpp`) |
| File save/load | §5.2 |
| Multi-project (chọn / tạo / đổi tên / xóa, banner project name) | §5.3, §6.1 (`OuterMenu`), §6.3, §6.4 |
| CLI menu | §6 |

> **Cách dùng file này để vibecode:**
> 1. Đưa toàn bộ file này cho Claude Code / Cursor.
> 2. Yêu cầu: "Implement toàn bộ codebase theo SPEC.md, tuân thủ chính xác signature, format, và pass mọi test trong §7-§8."
> 3. Build → chạy `ctest` → mọi test phải PASS.
