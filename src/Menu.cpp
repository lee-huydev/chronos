#include "Menu.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <exception>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "ConsoleColor.h"
#include "Exceptions.h"
#include "FileHandler.h"
#include "TaskFilter.h"
#include "UrgencyCalculator.h"
#include <thread> 
#include <chrono> 

namespace {
std::string trim(std::string s) {
    auto notSpace = [](unsigned char c) { return !std::isspace(c); };
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), notSpace));
    s.erase(std::find_if(s.rbegin(), s.rend(), notSpace).base(), s.end());
    return s;
}
std::string toLower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return s;
}
}  // namespace

Menu::Menu() = default;
Menu::Menu(const std::string& saveFile) : saveFilePath(saveFile) {}

Menu::Menu(const Project& project, int projId, ProjectManager& manager)
    : currentProject(project), projectId(projId), pm(&manager) {}

int Menu::parseCommand(const std::string& input) {
    std::string s = toLower(trim(input));
    if (s.empty()) return -1;

    // Pure numeric?
    bool numeric = !s.empty() && std::all_of(s.begin(), s.end(),
                                             [](unsigned char c) { return std::isdigit(c); });
    if (numeric) {
        try {
            int n = std::stoi(s);
            if ((n >= 0 && n <= 12)) return n;
            return -1;
        } catch (...) {
            return -1;
        }
    }

    if (s == "rename") return 1;
    if (s == "add") return 2;
    if (s == "list" || s == "view" || s == "ls") return 3;
    if (s == "sort") return 4;
    if (s == "sortu") {
        pendingSortMode = 1;
        return 4;
    }
    if (s == "sortdate") {
        pendingSortMode = 2;
        return 4;
    }
    if (s == "sortprio") {
        pendingSortMode = 3;
        return 4;
    }
    if (s == "extend") return 5;
    if (s == "undo") return 6;
    if (s == "save" || s == "load") return 7;
    if (s == "search" || s == "find") return 8;
    if (s == "delete" || s == "remove" || s == "rm") return 9;
    if (s == "status" || s == "update") return 10;
    if (s == "stress") return 11;
    if (s == "back" || s == "switch" || s == "projects") return 12;
    if (s == "exit" || s == "quit" || s == "q") return 0;
    return -1;
}

void Menu::run() {
    ConsoleColor::enableAnsiOnWindows();
    if (!pm) tryAutoLoad();

    while (true) {
        if (exitProgram || backToOuter) return;
        displayMainMenu();
        std::string raw;
        if (!std::getline(std::cin, raw)) {
            // EOF: thoát êm.
            handleExit();
            if (exitProgram) return;
            return;
        }
        int cmd = parseCommand(raw);
        try {
            switch (cmd) {
                case 1: handleRenameCurrentProject(); break;
                case 2: handleAddTask(); break;
                case 3: handleViewTasks(); break;
                case 4: handleSort(); break;
                case 5: handleExtendDeadline(); break;
                case 6: handleUndo(); break;
                case 7: handleSaveLoad(); break;
                case 8: handleSearch(); break;
                case 9: handleDeleteTask(); break;
                case 10: handleUpdateStatus(); break;
                case 11: handleStressTest(); break;
                case 12: handleBackToOuter(); break;
                case 0: handleExit(); break;
                default:
                    std::cout << ConsoleColor::RED
                              << "Lenh khong hop le. Hay nhap lai.\n"
                              << ConsoleColor::RESET;
                    break;
            }
        } catch (const std::exception& e) {
            std::cout << ConsoleColor::RED << "[Exception] " << e.what()
                      << ConsoleColor::RESET << "\n";
        }
    }
}

void Menu::tryAutoLoad() {
    if (!FileHandler::fileExists(saveFilePath)) return;
    try {
        currentProject = FileHandler::loadProject(saveFilePath);
        markClean();
        std::cout << ConsoleColor::CYAN
                  << "Da tu dong tai du an tu " << saveFilePath << "\n"
                  << ConsoleColor::RESET;
    } catch (const std::exception& e) {
        std::cout << ConsoleColor::YELLOW
                  << "Khong tai duoc file luu (" << e.what() << "), khoi tao project rong.\n"
                  << ConsoleColor::RESET;
        currentProject = Project("Untitled", "");
    }
}

void Menu::persistProject() {
    if (pm) {
        pm->saveProject(projectId, currentProject);
    } else {
        FileHandler::saveProject(currentProject, saveFilePath);
    }
}

void Menu::displayMainMenu() {
    std::cout << "\n"
              << ConsoleColor::BOLD << ConsoleColor::CYAN
              << "+==========================================+\n"
              << "|  CHRONOS - Task Manager v1.0             |\n"
              << "|  Project: [#" << currentProject.getProjectId() << "] "
              << currentProject.getProjectName() << "\n"
              << "+==========================================+\n"
              << ConsoleColor::RESET
              << "1.  Doi ten du an hien tai\n"
              << "2.  Them nhiem vu\n"
              << "3.  Xem danh sach nhiem vu\n"
              << "4.  Sap xep nhiem vu\n"
              << "5.  Gia han deadline (+1 ngay)\n"
              << "6.  Hoan tac (Undo)\n"
              << "7.  Luu / Tai du lieu\n"
              << "8.  Tim kiem nhiem vu\n"
              << "9.  Xoa nhiem vu\n"
              << "10. Cap nhat trang thai\n"
              << "11. [DEMO] Tran bo dem 10000 task\n"
              << "12. <- Quay ve danh sach du an (back/switch)\n"
              << "0.  Thoat chuong trinh\n"
              << "------------------------------------------\n"
              << "Lua chon cua ban: ";
}

void Menu::displayTaskTable(const std::vector<Task>& tasks) {
    if (tasks.empty()) {
        std::cout << "(Khong co nhiem vu)\n";
        return;
    }
    std::cout << "+-----+----------------------+-------------+-------------+----------"
                 "+-------------+---------+\n";
    std::cout << "| " << std::left << std::setw(3) << "ID" << " | "
              << std::setw(20) << "Tieu de" << " | "
              << std::setw(11) << "Bat dau" << " | "
              << std::setw(11) << "Het han" << " | "
              << std::setw(8) << "Uu tien" << " | "
              << std::setw(11) << "Trang thai" << " | "
              << std::setw(7) << "Khan cap" << " |\n";
    std::cout << "+-----+----------------------+-------------+-------------+----------"
                 "+-------------+---------+\n";
    for (const auto& t : tasks) {
        double u = UrgencyCalculator::calculate(t);
        const char* color = "";
        if (t.getStatus() == Status::DONE) {
            color = ConsoleColor::GREEN;
        } else if (t.isOverdue()) {
            color = ConsoleColor::RED;
        } else if (t.getPriority() == Priority::CRITICAL) {
            color = ConsoleColor::RED;
        } else if (t.getPriority() == Priority::HIGH || u >= 50.0) {
            color = ConsoleColor::YELLOW;
        }
        std::ostringstream urg;
        urg << std::fixed << std::setprecision(2) << u;
        std::string title = t.getTitle();
        if (title.size() > 20) title = title.substr(0, 19) + ">";
        std::cout << color << "| " << std::left << std::setw(3) << t.getId() << " | "
                  << std::setw(20) << title << " | "
                  << std::setw(11) << t.getStartDate().toString() << " | "
                  << std::setw(11) << t.getDueDate().toString() << " | "
                  << std::setw(8) << priorityToString(t.getPriority()) << " | "
                  << std::setw(11) << statusToString(t.getStatus()) << " | "
                  << std::setw(7) << urg.str() << " |"
                  << ConsoleColor::RESET << "\n";
    }
    std::cout << "+-----+----------------------+-------------+-------------+----------"
                 "+-------------+---------+\n";
}

void Menu::handleRenameCurrentProject() {
    std::string newName = readString("Ten du an moi: ");
    newName = trim(newName);
    if (newName.empty()) {
        std::cout << ConsoleColor::YELLOW
                  << "Ten du an khong duoc rong. Bo qua.\n"
                  << ConsoleColor::RESET;
        return;
    }
    undoManager.saveState(currentProject);
    currentProject.setProjectName(newName);
    if (pm) {
        try {
            pm->rename(projectId, newName);
        } catch (const std::exception& e) {
            std::cout << ConsoleColor::RED << "[Loi] " << e.what()
                      << ConsoleColor::RESET << "\n";
        }
    }
    markDirty();
    std::cout << ConsoleColor::GREEN << "Da doi ten du an thanh: " << newName << "\n"
              << ConsoleColor::RESET;
}

void Menu::handleAddTask() {
    std::string title = readString("Tieu de nhiem vu: ");
    std::string desc = readString("Mo ta: ");
    Date sd = readDate("Ngay bat dau (DD/MM/YYYY): ");
    Date dd = readDate("Ngay het han (DD/MM/YYYY): ");
    Priority p = readPriority();
    undoManager.saveState(currentProject);
    Task t(title, desc, sd, dd, p);
    currentProject.addTask(t);
    markDirty();
    std::cout << ConsoleColor::GREEN << "Da them nhiem vu #" << t.getId() << "\n"
              << ConsoleColor::RESET;
}

void Menu::handleViewTasks() {
    currentProject.displayProject();
    displayTaskTable(currentProject.getAllTasks());
}

void Menu::handleSort() {
    int mode = pendingSortMode;
    pendingSortMode = 0;
    if (mode == 0) {
        std::cout << "+- Sap xep nhiem vu ----------------+\n"
                  << "| 1. Theo do khan cap (Urgency)     |\n"
                  << "| 2. Theo deadline (gan nhat truoc) |\n"
                  << "| 3. Theo do uu tien (CRITICAL truoc)|\n"
                  << "| 0. Huy                            |\n"
                  << "+-----------------------------------+\n"
                  << "Lua chon: ";
        std::string raw;
        if (!std::getline(std::cin, raw)) return;
        try {
            mode = std::stoi(trim(raw));
        } catch (...) {
            mode = -1;
        }
    }
    std::vector<Task> sorted = currentProject.getAllTasks();
    if (mode == 1) {
        UrgencyCalculator::sortByUrgency(sorted);
    } else if (mode == 2) {
        UrgencyCalculator::sortByDeadline(sorted);
    } else if (mode == 3) {
        UrgencyCalculator::sortByPriority(sorted);
    } else {
        std::cout << "Khong sap xep.\n";
        return;
    }
    displayTaskTable(sorted);
}

void Menu::handleExtendDeadline() {
    int id = readInt("Nhap ID nhiem vu can gia han: ");
    Task* t = currentProject.getTaskById(id);
    if (!t) {
        std::cout << ConsoleColor::RED << "Khong tim thay nhiem vu #" << id << "\n"
                  << ConsoleColor::RESET;
        return;
    }
    undoManager.saveState(currentProject);
    ++(*t);
    markDirty();
    std::cout << ConsoleColor::GREEN
              << "Da gia han deadline cua nhiem vu #" << id
              << " den " << t->getDueDate().toString() << "\n"
              << ConsoleColor::RESET;
}

void Menu::handleUndo() {
    if (undoManager.isEmpty()) {
        std::cout << ConsoleColor::YELLOW
                  << "Khong co thao tac nao de hoan tac.\n"
                  << ConsoleColor::RESET;
        return;
    }
    currentProject = undoManager.undo();
    markDirty();
    std::cout << ConsoleColor::GREEN << "Da hoan tac thao tac gan nhat.\n"
              << ConsoleColor::RESET;
}

void Menu::handleSaveLoad() {
    std::cout << "1. Luu vao file\n2. Tai tu file\nLua chon: ";
    std::string raw;
    if (!std::getline(std::cin, raw)) return;
    int op = 0;
    try {
        op = std::stoi(trim(raw));
    } catch (...) {
        op = 0;
    }
    if (op == 1) {
        persistProject();
        markClean();
        std::cout << ConsoleColor::GREEN << "Da luu du an hien tai.\n"
                  << ConsoleColor::RESET;
    } else if (op == 2) {
        undoManager.saveState(currentProject);
        if (pm) {
            currentProject = pm->loadProject(projectId);
        } else {
            currentProject = FileHandler::loadProject(saveFilePath);
        }
        markClean();
        std::cout << ConsoleColor::GREEN << "Da tai lai du an tu file.\n"
                  << ConsoleColor::RESET;
    } else {
        std::cout << "Khong co thao tac nao.\n";
    }
}

void Menu::handleSearch() {
    std::string q = readString(
        "Tim kiem (text | p:HIGH | s:DONE | dd:DD/MM/YYYY-DD/MM/YYYY | od:true): ");
    auto results = TaskFilter::applyFilter(currentProject.getAllTasks(), q);
    std::cout << "Ket qua: " << results.size() << " task\n";
    displayTaskTable(results);
}

void Menu::handleDeleteTask() {
    int id = readInt("Nhap ID nhiem vu can xoa: ");
    undoManager.saveState(currentProject);
    if (currentProject.removeTask(id)) {
        markDirty();
        std::cout << ConsoleColor::GREEN << "Da xoa nhiem vu #" << id << "\n"
                  << ConsoleColor::RESET;
    } else {
        std::cout << ConsoleColor::RED << "Khong tim thay nhiem vu #" << id << "\n"
                  << ConsoleColor::RESET;
    }
}

void Menu::handleUpdateStatus() {
    int id = readInt("Nhap ID nhiem vu: ");
    Task* t = currentProject.getTaskById(id);
    if (!t) {
        std::cout << ConsoleColor::RED << "Khong tim thay nhiem vu #" << id << "\n"
                  << ConsoleColor::RESET;
        return;
    }
    std::cout << "Trang thai moi (1=PENDING, 2=IN_PROGRESS, 3=DONE): ";
    std::string raw;
    if (!std::getline(std::cin, raw)) return;
    int v = 0;
    try {
        v = std::stoi(trim(raw));
    } catch (...) {
        v = 0;
    }
    Status s = Status::PENDING;
    if (v == 1) s = Status::PENDING;
    else if (v == 2) s = Status::IN_PROGRESS;
    else if (v == 3) s = Status::DONE;
    else {
        std::cout << "Trang thai khong hop le.\n";
        return;
    }
    undoManager.saveState(currentProject);
    t->setStatus(s);
    markDirty();
    std::cout << ConsoleColor::GREEN
              << "Da cap nhat trang thai #" << id << " -> " << statusToString(s)
              << "\n" << ConsoleColor::RESET;
}

void Menu::handleStressTest() {
    std::cout << ConsoleColor::YELLOW
              << "[STRESS] Bat dau vong lap them 10000 task...\n"
              << ConsoleColor::RESET;
    Project demo("Stress Demo", "Demo Buffer Overflow Exception");
    int added = 0;
    bool caught = false;
    try {
        for (int i = 0; i < 10000; ++i) {
            Task t("Task#" + std::to_string(i), "stress",
                   Date(1, 1, 2026), Date(31, 12, 2026), Priority::LOW);
            demo.addTask(t);
            ++added;

            // In thông báo và Sleep 500ms
            if (added % 1000 == 0 || added == 9999) {
                std::cout << "[INFO] Da them thanh cong " << added << " tasks.\n";
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
            }
        }
    } catch (const BufferOverflowException& e) {
        caught = true;
        std::cout << ConsoleColor::RED << "[CAUGHT] " << e.what() << "\n"
                  << ConsoleColor::RESET;
    }
    std::cout << "Da them: " << added << " task\n";
    std::cout << "Catch overflow: " << (caught ? "YES" : "NO") << "\n";
}

void Menu::handleBackToOuter() {
    if (isDirty) {
        std::cout << ConsoleColor::YELLOW
                  << "Ban co thay doi chua luu. Luu truoc khi quay ra? (Y/N/C): "
                  << ConsoleColor::RESET;
        std::string ans;
        if (!std::getline(std::cin, ans)) ans = "y";
        ans = toLower(trim(ans));
        if (ans == "c" || ans == "cancel") return;
        if (ans == "y" || ans == "yes") {
            try {
                persistProject();
            } catch (const std::exception& e) {
                std::cout << ConsoleColor::RED << "[Loi luu] " << e.what()
                          << ConsoleColor::RESET << "\n";
            }
        }
    } else {
        try {
            persistProject();
        } catch (...) {
            // best-effort
        }
    }
    backToOuter = true;
    std::cout << ConsoleColor::CYAN << "Quay ve danh sach du an...\n"
              << ConsoleColor::RESET;
}

void Menu::handleExit() {
    if (isDirty) {
        std::cout << ConsoleColor::YELLOW
                  << "Ban co thay doi chua luu. Luu truoc khi thoat? (Y/N/C): "
                  << ConsoleColor::RESET;
        std::string ans;
        if (!std::getline(std::cin, ans)) ans = "y";
        ans = toLower(trim(ans));
        if (ans == "c" || ans == "cancel") return;
        if (ans == "y" || ans == "yes") {
            try {
                persistProject();
                std::cout << "Da luu. Tam biet!\n";
            } catch (const std::exception& e) {
                std::cout << ConsoleColor::RED << "[Loi luu] " << e.what()
                          << ConsoleColor::RESET << "\n";
            }
        } else {
            std::cout << "Thoat khong luu. Tam biet!\n";
        }
    } else {
        try {
            persistProject();
        } catch (...) {
            // best-effort
        }
        std::cout << "Tam biet!\n";
    }
    exitProgram = true;
    if (!pm) {
        // Legacy mode: không có outer menu để quay lại → exit luôn cho gọn.
        std::exit(0);
    }
}

int Menu::readInt(const std::string& prompt) {
    while (true) {
        std::cout << prompt;
        std::string s;
        if (!std::getline(std::cin, s)) return 0;
        try {
            return std::stoi(trim(s));
        } catch (...) {
            std::cout << ConsoleColor::RED << "Khong phai so. Nhap lai.\n"
                      << ConsoleColor::RESET;
        }
    }
}

std::string Menu::readString(const std::string& prompt) {
    std::cout << prompt;
    std::string s;
    if (!std::getline(std::cin, s)) return "";
    return s;
}

Date Menu::readDate(const std::string& prompt) {
    while (true) {
        std::cout << prompt;
        std::string s;
        if (!std::getline(std::cin, s)) return Date();
        try {
            return Date::fromString(trim(s));
        } catch (const std::exception& e) {
            std::cout << ConsoleColor::RED << e.what() << "\n" << ConsoleColor::RESET;
        }
    }
}

Priority Menu::readPriority() {
    while (true) {
        std::cout << "Uu tien (1=LOW, 2=MEDIUM, 3=HIGH, 4=CRITICAL): ";
        std::string s;
        if (!std::getline(std::cin, s)) return Priority::LOW;
        try {
            int v = std::stoi(trim(s));
            if (v == 1) return Priority::LOW;
            if (v == 2) return Priority::MEDIUM;
            if (v == 3) return Priority::HIGH;
            if (v == 4) return Priority::CRITICAL;
        } catch (...) {
        }
        std::cout << ConsoleColor::RED << "Khong hop le.\n" << ConsoleColor::RESET;
    }
}

bool Menu::readYesNo(const std::string& prompt) {
    std::cout << prompt;
    std::string s;
    if (!std::getline(std::cin, s)) return false;
    s = toLower(trim(s));
    return (s == "y" || s == "yes");
}

void Menu::pressEnterToContinue() {
    std::cout << "Nhan Enter de tiep tuc...";
    std::string s;
    std::getline(std::cin, s);
}
