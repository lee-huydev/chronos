#include "TaskFilter.h"

#include <algorithm>
#include <cctype>
#include <string>

namespace {
std::string toLower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return s;
}

bool containsCI(const std::string& haystack, const std::string& needle) {
    if (needle.empty()) return true;
    return toLower(haystack).find(toLower(needle)) != std::string::npos;
}
}  // namespace

std::vector<Task> TaskFilter::searchByKeyword(const std::vector<Task>& tasks,
                                              const std::string& keyword) {
    std::vector<Task> out;
    for (const auto& t : tasks) {
        if (containsCI(t.getTitle(), keyword) || containsCI(t.getDescription(), keyword)) {
            out.push_back(t);
        }
    }
    return out;
}

std::vector<Task> TaskFilter::filterByPriority(const std::vector<Task>& tasks, Priority p) {
    std::vector<Task> out;
    for (const auto& t : tasks) {
        if (t.getPriority() == p) out.push_back(t);
    }
    return out;
}

std::vector<Task> TaskFilter::filterByStatus(const std::vector<Task>& tasks, Status s) {
    std::vector<Task> out;
    for (const auto& t : tasks) {
        if (t.getStatus() == s) out.push_back(t);
    }
    return out;
}

std::vector<Task> TaskFilter::filterByDateRange(const std::vector<Task>& tasks,
                                                const Date& from, const Date& to) {
    std::vector<Task> out;
    for (const auto& t : tasks) {
        if (t.getDueDate() >= from && t.getDueDate() <= to) out.push_back(t);
    }
    return out;
}

std::vector<Task> TaskFilter::getOverdueTasks(const std::vector<Task>& tasks) {
    std::vector<Task> out;
    for (const auto& t : tasks) {
        if (t.isOverdue()) out.push_back(t);
    }
    return out;
}
