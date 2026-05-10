#include <gtest/gtest.h>

#include <vector>

#include "Task.h"
#include "TaskFilter.h"

namespace {
std::vector<Task> makeTasks() {
    return {
        Task("Buy milk", "from store", Date::today(), Date::today().addDays(2), Priority::LOW),
        Task("Write report", "monthly REPORT", Date::today(), Date::today().addDays(7),
             Priority::HIGH),
        Task("Pay bills", "", Date::today(), Date::today().addDays(3), Priority::MEDIUM),
    };
}
}  // namespace

TEST(FilterTest, SearchByKeywordCaseInsensitive) {
    auto tasks = makeTasks();
    auto r = TaskFilter::searchByKeyword(tasks, "report");
    ASSERT_EQ(r.size(), 1u);
    EXPECT_EQ(r[0].getTitle(), "Write report");
}

TEST(FilterTest, SearchByKeywordEmptyReturnsAll) {
    auto tasks = makeTasks();
    auto r = TaskFilter::searchByKeyword(tasks, "");
    EXPECT_EQ(r.size(), tasks.size());
}

TEST(FilterTest, FilterByPriority) {
    auto tasks = makeTasks();
    auto r = TaskFilter::filterByPriority(tasks, Priority::HIGH);
    ASSERT_EQ(r.size(), 1u);
    EXPECT_EQ(r[0].getTitle(), "Write report");
}

TEST(FilterTest, FilterByStatus) {
    auto tasks = makeTasks();
    tasks[0].setStatus(Status::DONE);
    auto r = TaskFilter::filterByStatus(tasks, Status::DONE);
    ASSERT_EQ(r.size(), 1u);
}

TEST(FilterTest, FilterByDateRange) {
    auto tasks = makeTasks();
    auto r = TaskFilter::filterByDateRange(tasks, Date::today(),
                                           Date::today().addDays(3));
    EXPECT_EQ(r.size(), 2u);  // milk(2), bills(3)
}

TEST(FilterTest, GetOverdueTasks) {
    std::vector<Task> tasks;
    Date past = Date::today().addDays(-5);
    tasks.emplace_back("Old", "", past.addDays(-1), past, Priority::LOW);
    tasks.emplace_back("New", "", Date::today(), Date::today().addDays(5), Priority::LOW);
    auto r = TaskFilter::getOverdueTasks(tasks);
    ASSERT_EQ(r.size(), 1u);
    EXPECT_EQ(r[0].getTitle(), "Old");
}
