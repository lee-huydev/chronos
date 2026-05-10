#pragma once

#include <string>
#include <vector>

#include "Date.h"
#include "Task.h"

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
