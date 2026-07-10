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

    // Unified search:
    //   - "abc"            -> searchByKeyword
    //   - "p:HIGH"         -> filterByPriority (case-insensitive value)
    //   - "s:DONE"         -> filterByStatus    (case-insensitive value)
    //   - "dd:DD/MM/YYYY-DD/MM/YYYY" -> filterByDateRange
    //   - "od:true"        -> getOverdueTasks (only exact "true")
    // Throw std::invalid_argument on malformed value.
    static std::vector<Task> applyFilter(const std::vector<Task>& tasks,
                                         const std::string& query);
};
