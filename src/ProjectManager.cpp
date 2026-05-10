#include "ProjectManager.h"

#include <algorithm>
#include <chrono>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <string>
#include <system_error>
#include <vector>

#include "Exceptions.h"
#include "FileHandler.h"

namespace fs = std::filesystem;

namespace {

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
        if (c == '"') out.push_back('"');
        out.push_back(c);
    }
    out.push_back('"');
    return out;
}

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

ProjectManager::ProjectManager(const std::string& dir)
    : dataDir(dir), indexPath(dir + "/index.csv") {}

std::string ProjectManager::nowTimestamp() {
    using namespace std::chrono;
    auto now = system_clock::to_time_t(system_clock::now());
    std::tm tmv{};
#if defined(_WIN32)
    localtime_s(&tmv, &now);
#else
    localtime_r(&now, &tmv);
#endif
    std::ostringstream os;
    os << std::put_time(&tmv, "%Y-%m-%d %H:%M:%S");
    return os.str();
}

void ProjectManager::load() {
    metas.clear();
    nextId = 1;

    std::error_code ec;
    fs::create_directories(dataDir, ec);
    // Ignore ec — open() below will fail if the dir is unusable.

    if (!FileHandler::fileExists(indexPath)) {
        // No index yet — first time. Persist an empty index so future runs
        // can detect there's nothing instead of creating the dir again.
        save();
        return;
    }

    std::ifstream in(indexPath);
    if (!in.is_open()) {
        throw FileIOException("Khong mo duoc index: " + indexPath);
    }

    enum class Section { NONE, NEXTID, PROJECT_HEADER, PROJECT_DATA };
    Section sec = Section::NONE;

    std::string line;
    while (std::getline(in, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line.empty()) continue;

        if (line.rfind("# CHRONOS_INDEX", 0) == 0) {
            sec = Section::NEXTID;
            continue;
        }
        if (line == "# PROJECTS") {
            sec = Section::PROJECT_HEADER;
            continue;
        }

        if (sec == Section::NEXTID) {
            auto fields = csvParseLine(line);
            if (fields.size() >= 2 && fields[0] == "nextId") {
                try {
                    nextId = std::stoi(fields[1]);
                } catch (...) {
                    nextId = 1;
                }
            }
            sec = Section::NONE;
            continue;
        }

        if (sec == Section::PROJECT_HEADER) {
            sec = Section::PROJECT_DATA;
            continue;
        }

        if (sec == Section::PROJECT_DATA) {
            auto f = csvParseLine(line);
            if (f.size() < 7) continue;  // skip malformed
            ProjectMeta m;
            try {
                m.id = std::stoi(f[0]);
            } catch (...) {
                continue;
            }
            m.name = f[1];
            m.description = f[2];
            m.savePath = f[3];
            m.createdAt = f[4];
            m.modifiedAt = f[5];
            try {
                m.taskCount = std::stoi(f[6]);
            } catch (...) {
                m.taskCount = 0;
            }
            metas.push_back(m);
            if (m.id >= nextId) nextId = m.id + 1;
        }
    }
}

void ProjectManager::save() const {
    std::error_code ec;
    fs::create_directories(dataDir, ec);

    std::ofstream out(indexPath);
    if (!out.is_open()) {
        throw FileIOException("Khong mo duoc index de ghi: " + indexPath);
    }

    out << "# CHRONOS_INDEX v1\n";
    out << "nextId," << nextId << "\n";
    out << "# PROJECTS\n";
    out << "id,name,description,savePath,createdAt,modifiedAt,taskCount\n";
    for (const auto& m : metas) {
        out << m.id << ","
            << csvEscape(m.name) << ","
            << csvEscape(m.description) << ","
            << csvEscape(m.savePath) << ","
            << csvEscape(m.createdAt) << ","
            << csvEscape(m.modifiedAt) << ","
            << m.taskCount << "\n";
    }
    if (!out.good()) {
        throw FileIOException("Loi khi ghi index: " + indexPath);
    }
}

const ProjectMeta* ProjectManager::findById(int id) const {
    for (const auto& m : metas) {
        if (m.id == id) return &m;
    }
    return nullptr;
}

ProjectMeta ProjectManager::create(const std::string& name,
                                   const std::string& description) {
    ProjectMeta m;
    m.id = nextId++;
    m.name = name;
    m.description = description;
    m.savePath = dataDir + "/" + std::to_string(m.id) + ".csv";
    m.createdAt = nowTimestamp();
    m.modifiedAt = m.createdAt;
    m.taskCount = 0;

    // Tạo file CSV rỗng cho project (chỉ có header + project info).
    Project p(name, description);
    p.setProjectIdForLoad(m.id);
    FileHandler::saveProject(p, m.savePath);

    metas.push_back(m);
    save();
    return m;
}

void ProjectManager::rename(int id, const std::string& newName) {
    for (auto& m : metas) {
        if (m.id == id) {
            m.name = newName;
            m.modifiedAt = nowTimestamp();
            save();
            // Cũng update file CSV của project để projectName trong file đồng bộ.
            try {
                Project p = FileHandler::loadProject(m.savePath);
                p.setProjectName(newName);
                FileHandler::saveProject(p, m.savePath);
            } catch (...) {
                // best-effort: index đã được cập nhật
            }
            return;
        }
    }
    throw ProjectNotFoundException(id);
}

void ProjectManager::deleteProject(int id) {
    auto it = std::find_if(metas.begin(), metas.end(),
                           [id](const ProjectMeta& m) { return m.id == id; });
    if (it == metas.end()) {
        throw ProjectNotFoundException(id);
    }

    std::error_code ec;
    fs::remove(it->savePath, ec);  // bỏ qua lỗi nếu file không tồn tại

    metas.erase(it);
    save();
}

Project ProjectManager::loadProject(int id) {
    const ProjectMeta* m = findById(id);
    if (!m) throw ProjectNotFoundException(id);
    return FileHandler::loadProject(m->savePath);
}

void ProjectManager::saveProject(int id, const Project& project) {
    for (auto& m : metas) {
        if (m.id == id) {
            FileHandler::saveProject(project, m.savePath);
            m.modifiedAt = nowTimestamp();
            m.taskCount = project.getTaskCount();
            m.name = project.getProjectName();  // sync nếu rename trong main menu
            m.description = project.getDescription();
            save();
            return;
        }
    }
    throw ProjectNotFoundException(id);
}

void ProjectManager::migrateLegacySaveIfPresent(const std::string& legacyPath) {
    // Chỉ migrate nếu chưa có index entry nào (avoid double-import).
    if (!metas.empty()) return;
    if (!FileHandler::fileExists(legacyPath)) return;

    Project legacy("", "");
    try {
        legacy = FileHandler::loadProject(legacyPath);
    } catch (...) {
        return;  // best-effort
    }

    ProjectMeta m;
    m.id = nextId++;
    m.name = legacy.getProjectName().empty() ? "Imported" : legacy.getProjectName();
    m.description = legacy.getDescription();
    m.savePath = dataDir + "/" + std::to_string(m.id) + ".csv";
    m.createdAt = nowTimestamp();
    m.modifiedAt = m.createdAt;
    m.taskCount = legacy.getTaskCount();

    // Re-id project và save vào nơi mới.
    legacy.setProjectIdForLoad(m.id);
    try {
        FileHandler::saveProject(legacy, m.savePath);
    } catch (...) {
        return;
    }

    metas.push_back(m);
    save();

    // Rename legacy file -> .bak để giữ backup, không xóa.
    std::error_code ec;
    fs::rename(legacyPath, legacyPath + ".bak", ec);
}
