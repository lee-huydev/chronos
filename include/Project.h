#pragma once

#include <string>
#include <vector>

#include "Task.h"

class Project {
private:
    int projectId;
    std::string projectName;
    std::string description;
    std::vector<Task> tasks;

    static int nextProjectId;

public:
    static constexpr int MAX_TASKS = 9999;

    Project();
    Project(const std::string& name, const std::string& description);

    int getProjectId() const;
    std::string getProjectName() const;
    std::string getDescription() const;
    const std::vector<Task>& getAllTasks() const;
    int getTaskCount() const;

    void setProjectName(const std::string& name);
    void setDescription(const std::string& desc);
    void setProjectIdForLoad(int id);

    void addTask(const Task& t);
    bool removeTask(int taskId);
    Task* getTaskById(int taskId);
    const Task* getTaskById(int taskId) const;
    bool updateTask(int taskId, const Task& newTask);

    void displayProject() const;
    void displayAllTasks() const;

    bool operator==(const Project& other) const;
};
