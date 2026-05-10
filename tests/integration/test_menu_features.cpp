#include <gtest/gtest.h>

#include "Menu.h"

TEST(IntegrationMenu, ParseCommandNumeric) {
    Menu m;
    EXPECT_EQ(m.parseCommand("1"), 1);
    EXPECT_EQ(m.parseCommand("11"), 11);
    EXPECT_EQ(m.parseCommand("12"), 12);
    EXPECT_EQ(m.parseCommand("0"), 0);
    EXPECT_EQ(m.parseCommand("13"), -1);
}

TEST(IntegrationMenu, ParseCommandKeywordCaseInsensitive) {
    Menu m;
    EXPECT_EQ(m.parseCommand("Rename"),   1);
    EXPECT_EQ(m.parseCommand("ADD"),      2);
    EXPECT_EQ(m.parseCommand("ls"),       3);
    EXPECT_EQ(m.parseCommand("sort"),     4);
    EXPECT_EQ(m.parseCommand("undo"),     6);
    EXPECT_EQ(m.parseCommand("rm"),       9);
    EXPECT_EQ(m.parseCommand("update"),   10);
    EXPECT_EQ(m.parseCommand("stress"),   11);
    EXPECT_EQ(m.parseCommand("Back"),     12);
    EXPECT_EQ(m.parseCommand("switch"),   12);
    EXPECT_EQ(m.parseCommand("Projects"), 12);
    EXPECT_EQ(m.parseCommand("q"),        0);
    EXPECT_EQ(m.parseCommand("exit"),     0);
}

TEST(IntegrationMenu, ParseCommandUnknown) {
    Menu m;
    EXPECT_EQ(m.parseCommand("nonsense"), -1);
    EXPECT_EQ(m.parseCommand(""),         -1);
    EXPECT_EQ(m.parseCommand("   "),      -1);
    // 'new' / 'create' đã chuyển sang Outer Menu — không còn map slot main menu.
    EXPECT_EQ(m.parseCommand("new"),      -1);
    EXPECT_EQ(m.parseCommand("create"),   -1);
}

TEST(IntegrationMenu, SortShortcutsSetPendingMode) {
    Menu m;
    EXPECT_EQ(m.parseCommand("sortu"), 4);
    EXPECT_EQ(m.getPendingSortMode(), 1);

    Menu m2;
    EXPECT_EQ(m2.parseCommand("sortdate"), 4);
    EXPECT_EQ(m2.getPendingSortMode(), 2);

    Menu m3;
    EXPECT_EQ(m3.parseCommand("sortprio"), 4);
    EXPECT_EQ(m3.getPendingSortMode(), 3);
}
