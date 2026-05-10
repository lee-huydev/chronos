#include <gtest/gtest.h>

#include <string>

#include "Exceptions.h"
#include "Project.h"

TEST(ProjectTest, AddTaskIncreasesCount) {
    Project p("Test", "Desc");
    Task t("A", "", Date(1, 5, 2026), Date(5, 5, 2026), Priority::HIGH);
    p.addTask(t);
    EXPECT_EQ(p.getTaskCount(), 1);
}

TEST(ProjectTest, RemoveExistingTask) {
    Project p("Test", "Desc");
    Task t("A", "", Date(1, 5, 2026), Date(5, 5, 2026), Priority::HIGH);
    p.addTask(t);
    int id = t.getId();
    EXPECT_TRUE(p.removeTask(id));
    EXPECT_EQ(p.getTaskCount(), 0);
}

TEST(ProjectTest, RemoveNonExistentTaskReturnsFalse) {
    Project p("Test", "Desc");
    EXPECT_FALSE(p.removeTask(99999));
}

TEST(ProjectTest, BufferOverflowAt9999) {
    Project p("Test", "Desc");
    for (int i = 0; i < 9999; ++i) {
        Task t("T" + std::to_string(i), "", Date(1, 5, 2026), Date(5, 5, 2026),
               Priority::LOW);
        p.addTask(t);
    }
    EXPECT_EQ(p.getTaskCount(), 9999);
    Task overflow("Overflow", "", Date(1, 5, 2026), Date(5, 5, 2026), Priority::LOW);
    EXPECT_THROW(p.addTask(overflow), BufferOverflowException);
}

TEST(ProjectTest, GetTaskById) {
    Project p("Test", "Desc");
    Task t("Find me", "", Date(1, 5, 2026), Date(5, 5, 2026), Priority::HIGH);
    p.addTask(t);
    Task* found = p.getTaskById(t.getId());
    ASSERT_NE(found, nullptr);
    EXPECT_EQ(found->getTitle(), "Find me");
}

TEST(ProjectTest, UpdateTask) {
    Project p("Test", "Desc");
    Task t("Old", "", Date(1, 5, 2026), Date(5, 5, 2026), Priority::HIGH);
    p.addTask(t);
    Task replacement("New", "Updated", Date(1, 5, 2026), Date(10, 5, 2026),
                     Priority::CRITICAL);
    EXPECT_TRUE(p.updateTask(t.getId(), replacement));
    EXPECT_EQ(p.getTaskById(t.getId())->getTitle(), "New");
}

TEST(ProjectTest, EqualityMatchesContents) {
    Project a("X", "");
    Project b = a;
    EXPECT_TRUE(a == b);
}
