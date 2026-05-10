#include <gtest/gtest.h>

#include <cstdio>
#include <filesystem>
#include <string>

#include "Exceptions.h"
#include "FileHandler.h"
#include "Project.h"
#include "Task.h"

namespace {
std::string tempPath(const std::string& name) {
    return (std::filesystem::temp_directory_path() / name).string();
}

void cleanup(const std::string& p) { std::remove(p.c_str()); }
}  // namespace

TEST(FileHandlerTest, FileExists) {
    EXPECT_FALSE(FileHandler::fileExists("/this/does/not/exist.csv"));
}

TEST(FileHandlerTest, SaveAndLoadRoundTrip) {
    std::string path = tempPath("chronos_filehandler_roundtrip.csv");
    cleanup(path);

    Project p("Test Project", "Description with spaces");
    p.addTask(Task("First", "Hello", Date(1, 5, 2026), Date(5, 5, 2026), Priority::HIGH));
    p.addTask(Task("Second", "World", Date(2, 5, 2026), Date(6, 5, 2026), Priority::LOW));

    FileHandler::saveProject(p, path);
    Project loaded = FileHandler::loadProject(path);

    EXPECT_EQ(loaded.getProjectName(), "Test Project");
    ASSERT_EQ(loaded.getAllTasks().size(), 2u);
    EXPECT_EQ(loaded.getAllTasks()[0].getTitle(), "First");
    EXPECT_EQ(loaded.getAllTasks()[1].getPriority(), Priority::LOW);

    cleanup(path);
}

TEST(FileHandlerTest, EscapeCommasInTitles) {
    std::string path = tempPath("chronos_filehandler_comma.csv");
    cleanup(path);

    Project p("X", "");
    p.addTask(Task("A, B, C", "field, with, commas", Date(1, 1, 2026), Date(2, 1, 2026),
                   Priority::LOW));
    FileHandler::saveProject(p, path);

    Project loaded = FileHandler::loadProject(path);
    ASSERT_EQ(loaded.getAllTasks().size(), 1u);
    EXPECT_EQ(loaded.getAllTasks()[0].getTitle(), "A, B, C");
    EXPECT_EQ(loaded.getAllTasks()[0].getDescription(), "field, with, commas");

    cleanup(path);
}

TEST(FileHandlerTest, LoadNonExistentThrows) {
    EXPECT_THROW(FileHandler::loadProject("/no/such/file.csv"), FileIOException);
}

TEST(FileHandlerTest, PreservesStatusAndDates) {
    std::string path = tempPath("chronos_filehandler_status.csv");
    cleanup(path);

    Project p("X", "");
    Task t("T1", "", Date(1, 5, 2026), Date(5, 5, 2026), Priority::CRITICAL);
    t.setStatus(Status::IN_PROGRESS);
    p.addTask(t);
    FileHandler::saveProject(p, path);

    Project loaded = FileHandler::loadProject(path);
    EXPECT_EQ(loaded.getAllTasks()[0].getStatus(), Status::IN_PROGRESS);
    EXPECT_EQ(loaded.getAllTasks()[0].getDueDate(), Date(5, 5, 2026));

    cleanup(path);
}
