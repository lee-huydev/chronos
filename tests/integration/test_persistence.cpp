#include <gtest/gtest.h>

#include <cstdio>
#include <filesystem>
#include <string>

#include "FileHandler.h"
#include "Project.h"
#include "Task.h"

namespace {
std::string tempPath(const std::string& name) {
    return (std::filesystem::temp_directory_path() / name).string();
}
}  // namespace

TEST(IntegrationPersistence, MultipleSaveLoadCycles) {
    std::string path = tempPath("chronos_persistence_cycles.csv");
    std::remove(path.c_str());

    Project p("Cycle", "Test multiple cycles");
    p.addTask(Task("T1", "first", Date(1, 5, 2026), Date(5, 5, 2026), Priority::HIGH));
    FileHandler::saveProject(p, path);

    Project l1 = FileHandler::loadProject(path);
    EXPECT_EQ(l1.getAllTasks().size(), 1u);

    l1.addTask(Task("T2", "second", Date(1, 5, 2026), Date(6, 5, 2026), Priority::LOW));
    FileHandler::saveProject(l1, path);

    Project l2 = FileHandler::loadProject(path);
    ASSERT_EQ(l2.getAllTasks().size(), 2u);
    EXPECT_EQ(l2.getAllTasks()[1].getTitle(), "T2");

    std::remove(path.c_str());
}

TEST(IntegrationPersistence, PreservesAcrossLoadAfterModification) {
    std::string path = tempPath("chronos_persistence_modify.csv");
    std::remove(path.c_str());

    Project p("X", "");
    p.addTask(Task("A", "", Date(1, 5, 2026), Date(5, 5, 2026), Priority::LOW));
    FileHandler::saveProject(p, path);

    Project loaded = FileHandler::loadProject(path);
    int aId = loaded.getAllTasks()[0].getId();
    Task* a = loaded.getTaskById(aId);
    ASSERT_NE(a, nullptr);
    a->setStatus(Status::DONE);
    FileHandler::saveProject(loaded, path);

    Project reloaded = FileHandler::loadProject(path);
    EXPECT_EQ(reloaded.getTaskById(aId)->getStatus(), Status::DONE);

    std::remove(path.c_str());
}
