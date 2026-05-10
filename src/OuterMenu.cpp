#include "OuterMenu.h"

#include <algorithm>
#include <cctype>
#include <exception>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

#include "ConsoleColor.h"
#include "Exceptions.h"
#include "Menu.h"

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

OuterMenu::OuterMenu(ProjectManager& manager) : pm(manager) {}

int OuterMenu::parseCommand(const std::string& input) const {
    std::string s = toLower(trim(input));
    if (s.empty()) return -1;
    bool numeric = std::all_of(s.begin(), s.end(),
                               [](unsigned char c) { return std::isdigit(c); });
    if (numeric) {
        try {
            int n = std::stoi(s);
            if (n >= 0 && n <= 4) return n;
            return -1;
        } catch (...) {
            return -1;
        }
    }
    if (s == "open" || s == "select") return 1;
    if (s == "new" || s == "create") return 2;
    if (s == "rename") return 3;
    if (s == "delete" || s == "del" || s == "rm") return 4;
    if (s == "exit" || s == "quit" || s == "q") return 0;
    return -1;
}

void OuterMenu::run() {
    ConsoleColor::enableAnsiOnWindows();
    pm.load();
    pm.migrateLegacySaveIfPresent();

    if (pm.isEmpty()) {
        firstLaunchWizard();
        // Sau wizard: nếu vẫn trống (user EOF) thì thoát.
        if (pm.isEmpty()) return;
    }

    while (true) {
        displayHeader();
        displayProjectList();
        displayMenuOptions();
        std::string raw;
        if (!std::getline(std::cin, raw)) return;
        int cmd = parseCommand(raw);
        try {
            switch (cmd) {
                case 1: handleSelect(); break;
                case 2: handleCreate(); break;
                case 3: handleRename(); break;
                case 4: handleDelete(); break;
                case 0:
                    std::cout << ConsoleColor::CYAN << "Tam biet!\n"
                              << ConsoleColor::RESET;
                    return;
                default:
                    std::cout << ConsoleColor::RED
                              << "Lenh khong hop le.\n"
                              << ConsoleColor::RESET;
                    break;
            }
        } catch (const std::exception& e) {
            std::cout << ConsoleColor::RED << "[Exception] " << e.what()
                      << ConsoleColor::RESET << "\n";
        }
        // Nếu xóa hết → quay lại wizard (UC5 cuối).
        if (pm.isEmpty()) {
            std::cout << ConsoleColor::YELLOW
                      << "Khong con du an nao. Hay tao du an dau tien.\n"
                      << ConsoleColor::RESET;
            firstLaunchWizard();
            if (pm.isEmpty()) return;
        }
    }
}

void OuterMenu::displayHeader() const {
    std::cout << "\n"
              << ConsoleColor::BOLD << ConsoleColor::CYAN
              << "+====================================+\n"
              << "|    CHRONOS - Quan Ly Du An         |\n"
              << "+====================================+\n"
              << ConsoleColor::RESET;
}

void OuterMenu::displayProjectList() const {
    const auto& list = pm.list();
    if (list.empty()) {
        std::cout << "(Chua co du an nao)\n";
        return;
    }
    std::cout << "+----+-----------------------+-------+---------------------+\n";
    std::cout << "| ID | Ten du an             | #Task | Cap nhat            |\n";
    std::cout << "+----+-----------------------+-------+---------------------+\n";
    for (const auto& m : list) {
        std::string name = m.name;
        if (name.size() > 21) name = name.substr(0, 20) + ">";
        std::ostringstream idstr;
        idstr << m.id;
        std::cout << "| " << std::left << std::setw(2) << idstr.str()
                  << " | " << std::setw(21) << name
                  << " | " << std::right << std::setw(5) << m.taskCount
                  << " | " << std::left << std::setw(19) << m.modifiedAt
                  << " |\n";
    }
    std::cout << "+----+-----------------------+-------+---------------------+\n";
}

void OuterMenu::displayMenuOptions() const {
    std::cout << "1. Mo du an (Select)\n"
              << "2. Tao du an moi (Create)\n"
              << "3. Doi ten du an (Rename)\n"
              << "4. Xoa du an (Delete)\n"
              << "0. Thoat chuong trinh\n"
              << "-------------------------------------\n"
              << "Lua chon: ";
}

void OuterMenu::firstLaunchWizard() {
    std::cout << "\n"
              << ConsoleColor::BOLD << ConsoleColor::CYAN
              << "+====================================+\n"
              << "| Chao mung toi CHRONOS!             |\n"
              << "| Ban chua co du an nao.             |\n"
              << "| Hay tao du an dau tien.            |\n"
              << "+====================================+\n"
              << ConsoleColor::RESET;
    std::string name;
    while (true) {
        name = trim(readString("Ten du an: "));
        if (!name.empty()) break;
        std::cout << ConsoleColor::RED << "Ten khong duoc rong.\n"
                  << ConsoleColor::RESET;
        // Nếu user gửi EOF, readString trả về "" và sẽ vòng lặp lại — break để tránh kẹt.
        if (!std::cin.good()) return;
    }
    std::string desc = readString("Mo ta (co the bo trong): ");
    auto m = pm.create(name, desc);
    std::cout << ConsoleColor::GREEN
              << "Da tao du an [#" << m.id << "] " << m.name << "\n"
              << ConsoleColor::RESET;
    enterMainMenu(m.id);
}

void OuterMenu::handleSelect() {
    if (pm.list().empty()) {
        std::cout << ConsoleColor::YELLOW << "Chua co du an nao.\n"
                  << ConsoleColor::RESET;
        return;
    }
    int id = readInt("Nhap ID du an muon mo: ");
    const ProjectMeta* m = pm.findById(id);
    if (!m) {
        std::cout << ConsoleColor::RED << "Khong tim thay du an #" << id
                  << "\n" << ConsoleColor::RESET;
        return;
    }
    enterMainMenu(id);
}

void OuterMenu::handleCreate() {
    std::string name;
    while (true) {
        name = trim(readString("Ten du an: "));
        if (!name.empty()) break;
        std::cout << ConsoleColor::RED << "Ten khong duoc rong.\n"
                  << ConsoleColor::RESET;
        if (!std::cin.good()) return;
    }
    std::string desc = readString("Mo ta (co the bo trong): ");
    auto m = pm.create(name, desc);
    std::cout << ConsoleColor::GREEN
              << "Da tao du an [#" << m.id << "] " << m.name << "\n"
              << ConsoleColor::RESET;
    if (readYesNo("Mo du an ngay? (Y/N): ")) {
        enterMainMenu(m.id);
    }
}

void OuterMenu::handleRename() {
    if (pm.list().empty()) {
        std::cout << ConsoleColor::YELLOW << "Chua co du an nao.\n"
                  << ConsoleColor::RESET;
        return;
    }
    int id = readInt("Nhap ID du an: ");
    const ProjectMeta* m = pm.findById(id);
    if (!m) {
        std::cout << ConsoleColor::RED << "Khong tim thay du an #" << id
                  << "\n" << ConsoleColor::RESET;
        return;
    }
    std::string newName = trim(readString("Ten moi: "));
    if (newName.empty()) {
        std::cout << ConsoleColor::YELLOW << "Ten rong, bo qua.\n"
                  << ConsoleColor::RESET;
        return;
    }
    pm.rename(id, newName);
    std::cout << ConsoleColor::GREEN << "Da doi ten thanh: " << newName << "\n"
              << ConsoleColor::RESET;
}

void OuterMenu::handleDelete() {
    if (pm.list().empty()) {
        std::cout << ConsoleColor::YELLOW << "Chua co du an nao.\n"
                  << ConsoleColor::RESET;
        return;
    }
    int id = readInt("Nhap ID du an muon xoa: ");
    const ProjectMeta* m = pm.findById(id);
    if (!m) {
        std::cout << ConsoleColor::RED << "Khong tim thay du an #" << id
                  << "\n" << ConsoleColor::RESET;
        return;
    }
    std::cout << ConsoleColor::YELLOW
              << "CANH BAO: Se xoa du an [#" << id << "] " << m->name
              << " (" << m->taskCount << " task) VINH VIEN.\n"
              << ConsoleColor::RESET;
    if (!readYesNo("Xac nhan xoa? (y/N): ")) {
        std::cout << "Da huy.\n";
        return;
    }
    pm.deleteProject(id);
    std::cout << ConsoleColor::GREEN << "Da xoa du an #" << id << "\n"
              << ConsoleColor::RESET;
}

void OuterMenu::enterMainMenu(int id) {
    Project p;
    try {
        p = pm.loadProject(id);
    } catch (const std::exception& e) {
        std::cout << ConsoleColor::RED << "[Loi load] " << e.what()
                  << ConsoleColor::RESET << "\n";
        return;
    }
    Menu inner(p, id, pm);
    inner.run();
    if (inner.shouldExitProgram()) {
        // Bubble up via std::exit so main loop terminates cleanly.
        std::exit(0);
    }
    // Otherwise: backToOuter → return về vòng lặp run() outer.
}

int OuterMenu::readInt(const std::string& prompt) const {
    while (true) {
        std::cout << prompt;
        std::string s;
        if (!std::getline(std::cin, s)) return 0;
        try {
            return std::stoi(trim(s));
        } catch (...) {
            std::cout << ConsoleColor::RED << "Khong phai so.\n"
                      << ConsoleColor::RESET;
        }
    }
}

std::string OuterMenu::readString(const std::string& prompt) const {
    std::cout << prompt;
    std::string s;
    if (!std::getline(std::cin, s)) return "";
    return s;
}

bool OuterMenu::readYesNo(const std::string& prompt) const {
    std::cout << prompt;
    std::string s;
    if (!std::getline(std::cin, s)) return false;
    s = toLower(trim(s));
    return (s == "y" || s == "yes");
}
