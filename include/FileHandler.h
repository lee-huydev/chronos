#pragma once

#include <string>

#include "Project.h"

class FileHandler {
public:
    static void saveProject(const Project& project, const std::string& filepath);
    static Project loadProject(const std::string& filepath);
    static bool fileExists(const std::string& filepath);
};
