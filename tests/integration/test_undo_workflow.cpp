#include <gtest/gtest.h>

#include "Project.h"
#include "Task.h"
#include "UndoManager.h"

TEST(IntegrationUndoWorkflow, AddRemoveUndo) {
    Project p("X", "");
    UndoManager um;

    um.saveState(p);
    p.addTask(Task("A", "", Date::today(), Date::today().addDays(1), Priority::LOW));
    EXPECT_EQ(p.getTaskCount(), 1);

    um.saveState(p);
    p.addTask(Task("B", "", Date::today(), Date::today().addDays(1), Priority::LOW));
    EXPECT_EQ(p.getTaskCount(), 2);

    p = um.undo();
    EXPECT_EQ(p.getTaskCount(), 1);
    EXPECT_EQ(p.getAllTasks()[0].getTitle(), "A");

    p = um.undo();
    EXPECT_EQ(p.getTaskCount(), 0);
}

TEST(IntegrationUndoWorkflow, UndoExtendDeadline) {
    Project p("X", "");
    UndoManager um;
    Task t("A", "", Date(1, 5, 2026), Date(5, 5, 2026), Priority::LOW);
    p.addTask(t);
    int id = t.getId();

    um.saveState(p);
    Task* live = p.getTaskById(id);
    ASSERT_NE(live, nullptr);
    ++(*live);
    EXPECT_EQ(p.getTaskById(id)->getDueDate(), Date(6, 5, 2026));

    p = um.undo();
    EXPECT_EQ(p.getTaskById(id)->getDueDate(), Date(5, 5, 2026));
}
