#include <exception>
#include <iostream>

#include "ConsoleColor.h"
#include "OuterMenu.h"
#include "ProjectManager.h"

int main() {
    try {
        ProjectManager pm("data/projects");
        OuterMenu outer(pm);
        outer.run();
    } catch (const std::exception& e) {
        std::cerr << ConsoleColor::RED
                  << "[FATAL] " << e.what() << ConsoleColor::RESET << "\n";
        return 1;
    }
    return 0;
}
