#pragma once

#include <string>
#include <vector>

#include "Project.h"

struct ProjectMeta {
    int id = 0;
    std::string name;
    std::string description;
    std::string savePath;
    std::string createdAt;
    std::string modifiedAt;
    int taskCount = 0;
};

class ProjectManager {
private:
    std::string dataDir;
    std::string indexPath;
    std::vector<ProjectMeta> metas;
    int nextId = 1;

public:
    explicit ProjectManager(const std::string& dataDir = "data/projects");

    const std::string& getDataDir() const { return dataDir; }
    const std::string& getIndexPath() const { return indexPath; }
    int getNextId() const { return nextId; }

    void load();
    void save() const;

    const std::vector<ProjectMeta>& list() const { return metas; }
    bool isEmpty() const { return metas.empty(); }
    const ProjectMeta* findById(int id) const;

    ProjectMeta create(const std::string& name, const std::string& description);
    void rename(int id, const std::string& newName);
    void deleteProject(int id);

    Project loadProject(int id);
    void saveProject(int id, const Project& project);

    void migrateLegacySaveIfPresent(
        const std::string& legacyPath = "data/chronos_save.csv");

    static std::string nowTimestamp();
};
