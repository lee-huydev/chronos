#include <gtest/gtest.h>

#include <string>

#include "Exceptions.h"
#include "Project.h"
#include "Task.h"

TEST(IntegrationErrorScenarios, TimeParadoxBlocksAddTask) {
    Project p("X", "");
    EXPECT_THROW(
        Task("Bad", "", Date(10, 5, 2026), Date(1, 5, 2026), Priority::LOW),
        TimeParadoxException);
    EXPECT_EQ(p.getTaskCount(), 0);
}

TEST(IntegrationErrorScenarios, BufferOverflowFromOOPLoop) {
    Project p("X", "");
    int i = 0;
    bool caught = false;
    try {
        for (; i < 10000; ++i) {
            p.addTask(Task("T" + std::to_string(i), "", Date(1, 1, 2026),
                           Date(2, 1, 2026), Priority::LOW));
        }
    } catch (const BufferOverflowException&) {
        caught = true;
    }
    EXPECT_TRUE(caught);
    EXPECT_EQ(i, Project::MAX_TASKS);
}

TEST(IntegrationErrorScenarios, InvalidDateThenContinue) {
    EXPECT_THROW(Date(31, 2, 2026), InvalidDateException);
    Project p("X", "");
    p.addTask(Task("OK", "", Date(1, 1, 2026), Date(2, 1, 2026), Priority::LOW));
    EXPECT_EQ(p.getTaskCount(), 1);
}
