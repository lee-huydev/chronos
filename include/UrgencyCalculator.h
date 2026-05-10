#pragma once

#include <vector>

#include "Task.h"

class UrgencyCalculator {
public:
    static double calculate(const Task& task);

    static void sortByUrgency(std::vector<Task>& tasks);
    static void sortByDeadline(std::vector<Task>& tasks);
    static void sortByPriority(std::vector<Task>& tasks);
};
