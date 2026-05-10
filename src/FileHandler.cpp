#include "FileHandler.h"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "Exceptions.h"

namespace {

// Quote nếu cần để ghi vào CSV: bao trong dấu " nếu chứa ',' hoặc '"' hoặc xuống dòng;
// dấu " bên trong được escape thành "".
std::string csvEscape(const std::string& s) {
    bool needQuote = false;
    for (char c : s) {
        if (c == ',' || c == '"' || c == '\n' || c == '\r') {
            needQuote = true;
            break;
        }
    }
    if (!needQuote) return s;
    std::string out;
    out.reserve(s.size() + 2);
    out.push_back('"');
    for (char c : s) {
        if (c == '"') out.push_back('"');  // escape " -> ""
        out.push_back(c);
    }
    out.push_back('"');
    return out;
}

// Parse 1 dòng CSV thành các field, hỗ trợ quoted strings với "" escape.
std::vector<std::string> csvParseLine(const std::string& line) {
    std::vector<std::string> fields;
    std::string cur;
    bool inQuotes = false;
    for (std::size_t i = 0; i < line.size(); ++i) {
        char c = line[i];
        if (inQuotes) {
            if (c == '"') {
                if (i + 1 < line.size() && line[i + 1] == '"') {
                    cur.push_back('"');
                    ++i;
                } else {
                    inQuotes = false;
                }
            } else {
                cur.push_back(c);
            }
        } else {
            if (c == ',') {
                fields.push_back(cur);
                cur.clear();
            } else if (c == '"' && cur.empty()) {
                inQuotes = true;
            } else {
                cur.push_back(c);
            }
        }
    }
    fields.push_back(cur);
    return fields;
}

}  // namespace

bool FileHandler::fileExists(const std::string& filepath) {
    std::error_code ec;
    return std::filesystem::exists(filepath, ec);
}

void FileHandler::saveProject(const Project& project, const std::string& filepath) {
    std::filesystem::path p(filepath);
    if (p.has_parent_path()) {
        std::error_code ec;
        std::filesystem::create_directories(p.parent_path(), ec);
        // Bỏ qua lỗi tạo thư mục — ofstream sẽ báo lỗi nếu không mở được file.
    }

    std::ofstream out(filepath);
    if (!out.is_open()) {
        throw FileIOException("Khong mo duoc file de ghi: " + filepath);
    }

    out << "# PROJECT\n";
    out << "projectId,projectName,description\n";
    out << project.getProjectId() << ","
        << csvEscape(project.getProjectName()) << ","
        << csvEscape(project.getDescription()) << "\n\n";

    out << "# TASKS\n";
    out << "taskId,title,description,startDate,dueDate,priority,status\n";
    for (const auto& t : project.getAllTasks()) {
        out << t.getId() << ","
            << csvEscape(t.getTitle()) << ","
            << csvEscape(t.getDescription()) << ","
            << t.getStartDate().toString() << ","
            << t.getDueDate().toString() << ","
            << priorityToString(t.getPriority()) << ","
            << statusToString(t.getStatus()) << "\n";
    }

    if (!out.good()) {
        throw FileIOException("Loi khi ghi file: " + filepath);
    }
}

Project FileHandler::loadProject(const std::string& filepath) {
    if (!fileExists(filepath)) {
        throw FileIOException("File khong ton tai: " + filepath);
    }

    std::ifstream in(filepath);
    if (!in.is_open()) {
        throw FileIOException("Khong mo duoc file de doc: " + filepath);
    }

    enum class Section { NONE, PROJECT_HEADER, PROJECT_DATA, TASK_HEADER, TASK_DATA };
    Section sec = Section::NONE;

    Project project("", "");
    bool projectLoaded = false;
    int projectIdRead = -1;

    std::string line;
    while (std::getline(in, line)) {
        // Trim trailing \r
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line.empty()) continue;

        if (line == "# PROJECT") {
            sec = Section::PROJECT_HEADER;
            continue;
        }
        if (line == "# TASKS") {
            sec = Section::TASK_HEADER;
            continue;
        }

        if (sec == Section::PROJECT_HEADER) {
            // Bỏ qua dòng tên cột: projectId,projectName,description
            sec = Section::PROJECT_DATA;
            continue;
        }
        if (sec == Section::TASK_HEADER) {
            sec = Section::TASK_DATA;
            continue;
        }

        if (sec == Section::PROJECT_DATA) {
            auto fields = csvParseLine(line);
            if (fields.size() < 3) {
                throw FileIOException("Dong PROJECT khong du field");
            }
            try {
                projectIdRead = std::stoi(fields[0]);
            } catch (...) {
                throw FileIOException("projectId khong phai so: " + fields[0]);
            }
            project.setProjectName(fields[1]);
            project.setDescription(fields[2]);
            project.setProjectIdForLoad(projectIdRead);
            projectLoaded = true;
            sec = Section::NONE;
            continue;
        }

        if (sec == Section::TASK_DATA) {
            auto fields = csvParseLine(line);
            if (fields.size() < 7) {
                throw FileIOException("Dong TASK khong du field (can 7)");
            }
            int taskId = 0;
            try {
                taskId = std::stoi(fields[0]);
            } catch (...) {
                throw FileIOException("taskId khong phai so: " + fields[0]);
            }
            try {
                Date sd = Date::fromString(fields[3]);
                Date dd = Date::fromString(fields[4]);
                Priority pr = priorityFromString(fields[5]);
                Task t(fields[1], fields[2], sd, dd, pr);
                t.setIdForLoad(taskId);
                t.setStatus(statusFromString(fields[6]));
                project.addTask(t);
            } catch (const std::exception& e) {
                throw FileIOException(std::string("Loi parse task: ") + e.what());
            }
        }
    }

    if (!projectLoaded) {
        throw FileIOException("File khong co section # PROJECT hop le");
    }

    return project;
}
