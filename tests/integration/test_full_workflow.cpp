#include <gtest/gtest.h>

#include "Project.h"
#include "Task.h"
#include "UrgencyCalculator.h"

TEST(IntegrationFullWorkflow, CreateAddSortDisplay) {
    Project p("Q2 Plan", "Q2 OOP project");
    p.addTask(Task("Plan",   "", Date::today(), Date::today().addDays(30), Priority::LOW));
    p.addTask(Task("Code",   "", Date::today(), Date::today().addDays(7),  Priority::HIGH));
    p.addTask(Task("Submit", "", Date::today(), Date::today().addDays(1),  Priority::CRITICAL));

    auto tasks = p.getAllTasks();
    UrgencyCalculator::sortByUrgency(tasks);
    EXPECT_EQ(tasks[0].getTitle(), "Submit");
    EXPECT_EQ(tasks[2].getTitle(), "Plan");

    UrgencyCalculator::sortByDeadline(tasks);
    EXPECT_EQ(tasks[0].getTitle(), "Submit");
    EXPECT_EQ(tasks[2].getTitle(), "Plan");

    UrgencyCalculator::sortByPriority(tasks);
    EXPECT_EQ(tasks[0].getPriority(), Priority::CRITICAL);
}
