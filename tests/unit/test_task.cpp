#include <gtest/gtest.h>

#include "Exceptions.h"
#include "Task.h"

TEST(TaskTest, ValidConstruction) {
    Task t("Test", "Desc", Date(1, 5, 2026), Date(5, 5, 2026), Priority::HIGH);
    EXPECT_EQ(t.getTitle(), "Test");
    EXPECT_EQ(t.getPriority(), Priority::HIGH);
    EXPECT_EQ(t.getStatus(), Status::PENDING);
}

TEST(TaskTest, TimeParadoxOnConstruction) {
    EXPECT_THROW(
        Task("Bad", "Desc", Date(5, 5, 2026), Date(1, 5, 2026), Priority::LOW),
        TimeParadoxException);
}

TEST(TaskTest, TimeParadoxOnSetDueDate) {
    Task t("Test", "Desc", Date(1, 5, 2026), Date(5, 5, 2026), Priority::HIGH);
    EXPECT_THROW(t.setDueDate(Date(31, 12, 2025)), TimeParadoxException);
}

TEST(TaskTest, AutoIncrementId) {
    Task::resetIdCounter();
    Task t1("A", "", Date(1, 1, 2026), Date(2, 1, 2026), Priority::LOW);
    Task t2("B", "", Date(1, 1, 2026), Date(2, 1, 2026), Priority::LOW);
    EXPECT_EQ(t2.getId(), t1.getId() + 1);
}

TEST(TaskTest, OperatorPrefixIncrement) {
    Task t("Test", "", Date(1, 5, 2026), Date(5, 5, 2026), Priority::HIGH);
    ++t;
    EXPECT_EQ(t.getDueDate(), Date(6, 5, 2026));
}

TEST(TaskTest, OperatorPostfixIncrement) {
    Task t("Test", "", Date(1, 5, 2026), Date(5, 5, 2026), Priority::HIGH);
    Task old = t++;
    EXPECT_EQ(old.getDueDate(), Date(5, 5, 2026));
    EXPECT_EQ(t.getDueDate(), Date(6, 5, 2026));
}

TEST(TaskTest, LessOperatorByPriority) {
    Task t1("A", "", Date(1, 5, 2026), Date(10, 5, 2026), Priority::CRITICAL);
    Task t2("B", "", Date(1, 5, 2026), Date(10, 5, 2026), Priority::LOW);
    EXPECT_TRUE(t1 < t2);
}

TEST(TaskTest, LessOperatorByDeadlineWhenSamePriority) {
    Task t1("A", "", Date(1, 5, 2026), Date(5, 5, 2026), Priority::HIGH);
    Task t2("B", "", Date(1, 5, 2026), Date(10, 5, 2026), Priority::HIGH);
    EXPECT_TRUE(t1 < t2);
}

TEST(TaskTest, EqualityById) {
    Task::resetIdCounter();
    Task t1("A", "", Date(1, 5, 2026), Date(2, 5, 2026), Priority::LOW);
    Task t2 = t1;  // copy
    EXPECT_TRUE(t1 == t2);
    Task t3("B", "", Date(1, 5, 2026), Date(2, 5, 2026), Priority::LOW);
    EXPECT_FALSE(t1 == t3);
}

TEST(TaskTest, IsOverdue) {
    Task t("Old", "", Date(1, 1, 2020), Date(2, 1, 2020), Priority::LOW);
    EXPECT_TRUE(t.isOverdue());
    t.setStatus(Status::DONE);
    EXPECT_FALSE(t.isOverdue());
}

TEST(TaskTest, PriorityToFromString) {
    EXPECT_EQ(priorityToString(Priority::HIGH), "HIGH");
    EXPECT_EQ(priorityFromString("CRITICAL"), Priority::CRITICAL);
    EXPECT_EQ(priorityFromString("low"), Priority::LOW);
}

TEST(TaskTest, StatusToFromString) {
    EXPECT_EQ(statusToString(Status::IN_PROGRESS), "IN_PROGRESS");
    EXPECT_EQ(statusFromString("DONE"), Status::DONE);
}
