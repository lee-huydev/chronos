#include "UrgencyCalculator.h"

#include <algorithm>

double UrgencyCalculator::calculate(const Task& task) {
    if (task.getStatus() == Status::DONE) return 0.0;
    int daysLeft = task.daysLeft();
    double base = static_cast<double>(static_cast<int>(task.getPriority())) * 10.0;
    if (daysLeft < 0) {
        return base + 1000.0;
    }
    return base + (100.0 / static_cast<double>(daysLeft + 1));
}

void UrgencyCalculator::sortByUrgency(std::vector<Task>& tasks) {
    std::sort(tasks.begin(), tasks.end(), [](const Task& a, const Task& b) {
        return calculate(a) > calculate(b);
    });
}

void UrgencyCalculator::sortByDeadline(std::vector<Task>& tasks) {
    std::sort(tasks.begin(), tasks.end(), [](const Task& a, const Task& b) {
        return a.getDueDate() < b.getDueDate();
    });
}

void UrgencyCalculator::sortByPriority(std::vector<Task>& tasks) {
    std::sort(tasks.begin(), tasks.end());
}
