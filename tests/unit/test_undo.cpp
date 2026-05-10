#include <gtest/gtest.h>

#include "Exceptions.h"
#include "Project.h"
#include "Task.h"
#include "UndoManager.h"

TEST(UndoTest, EmptyAtStart) {
    UndoManager um;
    EXPECT_TRUE(um.isEmpty());
    EXPECT_EQ(um.size(), 0u);
}

TEST(UndoTest, SaveAndRestore) {
    UndoManager um;
    Project p1("V1", "");
    um.saveState(p1);
    Project p2("V2", "");
    ProjectState restored = um.undo();
    EXPECT_EQ(restored.getProjectName(), "V1");
    (void)p2;
}

TEST(UndoTest, UndoEmptyThrows) {
    UndoManager um;
    EXPECT_THROW(um.undo(), UndoStackEmptyException);
}

TEST(UndoTest, LIFOOrder) {
    UndoManager um;
    um.saveState(Project("V1", ""));
    um.saveState(Project("V2", ""));
    um.saveState(Project("V3", ""));
    EXPECT_EQ(um.undo().getProjectName(), "V3");
    EXPECT_EQ(um.undo().getProjectName(), "V2");
    EXPECT_EQ(um.undo().getProjectName(), "V1");
}

TEST(UndoTest, MaxDepthDropsOldest) {
    UndoManager um;
    for (int i = 0; i < UndoManager::MAX_UNDO_DEPTH + 5; ++i) {
        um.saveState(Project("V" + std::to_string(i), ""));
    }
    EXPECT_EQ(um.size(),
              static_cast<size_t>(UndoManager::MAX_UNDO_DEPTH));
    // The most-recent-saved is V<MAX+4>
    EXPECT_EQ(um.undo().getProjectName(),
              "V" + std::to_string(UndoManager::MAX_UNDO_DEPTH + 4));
}

TEST(UndoTest, ClearEmptiesHistory) {
    UndoManager um;
    um.saveState(Project("V1", ""));
    um.saveState(Project("V2", ""));
    um.clear();
    EXPECT_TRUE(um.isEmpty());
}
