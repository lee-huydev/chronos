#include <gtest/gtest.h>

#include <vector>

#include "Task.h"
#include "UrgencyCalculator.h"

TEST(UrgencyTest, DoneTaskHasZeroUrgency) {
    Task t("Done", "", Date::today(), Date::today().addDays(5), Priority::CRITICAL);
    t.setStatus(Status::DONE);
    EXPECT_DOUBLE_EQ(UrgencyCalculator::calculate(t), 0.0);
}

TEST(UrgencyTest, OverdueTaskGetsBoost) {
    Date past = Date::today().addDays(-5);
    Task t("Old", "", past.addDays(-1), past, Priority::LOW);
    double u = UrgencyCalculator::calculate(t);
    EXPECT_GT(u, 1000.0);
}

TEST(UrgencyTest, HigherPriorityHigherUrgency) {
    Date today = Date::today();
    Task low("L", "", today, today.addDays(5), Priority::LOW);
    Task crit("C", "", today, today.addDays(5), Priority::CRITICAL);
    EXPECT_GT(UrgencyCalculator::calculate(crit), UrgencyCalculator::calculate(low));
}

TEST(UrgencyTest, FormulaMatchesSpec) {
    // priority=2 (MEDIUM), daysLeft=4 -> base=20, bonus=100/5=20 -> 40
    Date today = Date::today();
    Task t("M", "", today, today.addDays(4), Priority::MEDIUM);
    EXPECT_DOUBLE_EQ(UrgencyCalculator::calculate(t), 40.0);
}

TEST(UrgencyTest, SortByUrgencyDescending) {
    std::vector<Task> tasks = {
        Task("Low",  "", Date::today(), Date::today().addDays(10), Priority::LOW),
        Task("Crit", "", Date::today(), Date::today().addDays(1),  Priority::CRITICAL),
        Task("Med",  "", Date::today(), Date::today().addDays(5),  Priority::MEDIUM)};
    UrgencyCalculator::sortByUrgency(tasks);
    EXPECT_EQ(tasks[0].getTitle(), "Crit");
}

TEST(UrgencyTest, SortByDeadlineAscending) {
    std::vector<Task> tasks = {
        Task("Far",   "", Date::today(), Date::today().addDays(10), Priority::LOW),
        Task("Near",  "", Date::today(), Date::today().addDays(1),  Priority::LOW),
        Task("Mid",   "", Date::today(), Date::today().addDays(5),  Priority::LOW)};
    UrgencyCalculator::sortByDeadline(tasks);
    EXPECT_EQ(tasks[0].getTitle(), "Near");
    EXPECT_EQ(tasks[2].getTitle(), "Far");
}

TEST(UrgencyTest, SortByPriorityDescending) {
    std::vector<Task> tasks = {
        Task("Low",  "", Date::today(), Date::today().addDays(5), Priority::LOW),
        Task("Crit", "", Date::today(), Date::today().addDays(5), Priority::CRITICAL),
        Task("Med",  "", Date::today(), Date::today().addDays(5), Priority::MEDIUM)};
    UrgencyCalculator::sortByPriority(tasks);
    EXPECT_EQ(tasks[0].getTitle(), "Crit");
    EXPECT_EQ(tasks[2].getTitle(), "Low");
}
