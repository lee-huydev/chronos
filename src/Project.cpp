#include "Project.h"

#include <algorithm>
#include <iomanip>
#include <iostream>

#include "Exceptions.h"

int Project::nextProjectId = 1;

Project::Project()
    : projectId(nextProjectId++), projectName("Untitled"), description("") {}

Project::Project(const std::string& name, const std::string& desc)
    : projectId(nextProjectId++), projectName(name), description(desc) {}

int Project::getProjectId() const { return projectId; }
std::string Project::getProjectName() const { return projectName; }
std::string Project::getDescription() const { return description; }
const std::vector<Task>& Project::getAllTasks() const { return tasks; }
int Project::getTaskCount() const { return static_cast<int>(tasks.size()); }

void Project::setProjectName(const std::string& name) { projectName = name; }
void Project::setDescription(const std::string& desc) { description = desc; }

void Project::setProjectIdForLoad(int id) {
    projectId = id;
    if (id >= nextProjectId) nextProjectId = id + 1;
}

void Project::addTask(const Task& t) {
    if (static_cast<int>(tasks.size()) >= MAX_TASKS) {
        throw BufferOverflowException();
    }
    tasks.push_back(t);
}

bool Project::removeTask(int taskId) {
    auto it = std::find_if(tasks.begin(), tasks.end(),
                           [taskId](const Task& t) { return t.getId() == taskId; });
    if (it == tasks.end()) return false;
    tasks.erase(it);
    return true;
}

Task* Project::getTaskById(int taskId) {
    for (auto& t : tasks) {
        if (t.getId() == taskId) return &t;
    }
    return nullptr;
}

const Task* Project::getTaskById(int taskId) const {
    for (const auto& t : tasks) {
        if (t.getId() == taskId) return &t;
    }
    return nullptr;
}

bool Project::updateTask(int taskId, const Task& newTask) {
    for (auto& t : tasks) {
        if (t.getId() == taskId) {
            // Preserve original taskId so subsequent lookups still work.
            Task copy = newTask;
            copy.setIdForLoad(taskId);
            t = copy;
            return true;
        }
    }
    return false;
}

void Project::displayProject() const {
    std::cout << "=== Project [#" << projectId << "] " << projectName << " ===\n";
    if (!description.empty()) std::cout << "  " << description << "\n";
    std::cout << "  Tong so nhiem vu: " << tasks.size() << "\n";
}

void Project::displayAllTasks() const {
    displayProject();
    if (tasks.empty()) {
        std::cout << "  (Khong co nhiem vu)\n";
        return;
    }
    for (const auto& t : tasks) {
        std::cout << "  - " << t << "\n";
    }
}

bool Project::operator==(const Project& other) const {
    if (projectId != other.projectId) return false;
    if (projectName != other.projectName) return false;
    if (description != other.description) return false;
    if (tasks.size() != other.tasks.size()) return false;
    for (size_t i = 0; i < tasks.size(); ++i) {
        const Task& a = tasks[i];
        const Task& b = other.tasks[i];
        if (a.getId() != b.getId()) return false;
        if (a.getTitle() != b.getTitle()) return false;
        if (a.getDescription() != b.getDescription()) return false;
        if (a.getStartDate() != b.getStartDate()) return false;
        if (a.getDueDate() != b.getDueDate()) return false;
        if (a.getPriority() != b.getPriority()) return false;
        if (a.getStatus() != b.getStatus()) return false;
    }
    return true;
}
