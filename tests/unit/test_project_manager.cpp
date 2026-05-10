#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <string>
#include <thread>

#include "Exceptions.h"
#include "FileHandler.h"
#include "ProjectManager.h"

namespace fs = std::filesystem;

namespace {
std::atomic<int> tmpCounter{0};

// Tạo thư mục tạm độc lập cho mỗi test, tránh xung đột state giữa các test.
std::string makeTmpDir(const std::string& tag) {
    auto base = fs::temp_directory_path() / "chronos_pm_test";
    fs::create_directories(base);
    auto ts = std::chrono::steady_clock::now().time_since_epoch().count();
    std::string dir = (base / (tag + "_" + std::to_string(ts) + "_" +
                              std::to_string(++tmpCounter))).string();
    fs::create_directories(dir);
    return dir;
}
}  // namespace

TEST(ProjectManager, EmptyDirectoryIsEmpty) {
    auto dir = makeTmpDir("empty");
    ProjectManager pm(dir);
    pm.load();
    EXPECT_TRUE(pm.isEmpty());
    EXPECT_TRUE(pm.list().empty());
    EXPECT_EQ(pm.getNextId(), 1);
}

TEST(ProjectManager, CreateAssignsNewIdAndPersistsFile) {
    auto dir = makeTmpDir("create");
    ProjectManager pm(dir);
    pm.load();

    auto a = pm.create("Alpha", "first");
    EXPECT_EQ(a.id, 1);
    EXPECT_EQ(a.name, "Alpha");
    EXPECT_TRUE(fs::exists(a.savePath));

    auto b = pm.create("Bravo", "");
    EXPECT_EQ(b.id, 2);
    EXPECT_EQ(pm.list().size(), 2u);
    EXPECT_EQ(pm.getNextId(), 3);
}

TEST(ProjectManager, IdIsNotReusedAfterDelete) {
    auto dir = makeTmpDir("noreuse");
    ProjectManager pm(dir);
    pm.load();

    auto a = pm.create("A", "");
    auto b = pm.create("B", "");
    auto c = pm.create("C", "");
    EXPECT_EQ(a.id, 1);
    EXPECT_EQ(b.id, 2);
    EXPECT_EQ(c.id, 3);

    pm.deleteProject(b.id);
    EXPECT_EQ(pm.list().size(), 2u);

    auto d = pm.create("D", "");
    EXPECT_EQ(d.id, 4);
    EXPECT_EQ(pm.findById(2), nullptr);
}

TEST(ProjectManager, DeleteRemovesEntryAndFile) {
    auto dir = makeTmpDir("delete");
    ProjectManager pm(dir);
    pm.load();
    auto a = pm.create("Solo", "x");
    std::string path = a.savePath;
    EXPECT_TRUE(fs::exists(path));

    pm.deleteProject(a.id);
    EXPECT_TRUE(pm.isEmpty());
    EXPECT_FALSE(fs::exists(path));
}

TEST(ProjectManager, DeleteUnknownIdThrows) {
    auto dir = makeTmpDir("delthrow");
    ProjectManager pm(dir);
    pm.load();
    EXPECT_THROW(pm.deleteProject(999), ProjectNotFoundException);
}

TEST(ProjectManager, LoadProjectRoundTripPreservesTasks) {
    auto dir = makeTmpDir("rtrip");
    ProjectManager pm(dir);
    pm.load();
    auto a = pm.create("A", "");

    Project p = pm.loadProject(a.id);
    Task t("Buy milk", "fresh", Date(1, 5, 2026), Date(2, 5, 2026), Priority::HIGH);
    p.addTask(t);
    pm.saveProject(a.id, p);

    Project reloaded = pm.loadProject(a.id);
    ASSERT_EQ(reloaded.getTaskCount(), 1);
    EXPECT_EQ(reloaded.getAllTasks()[0].getTitle(), "Buy milk");
}

TEST(ProjectManager, SaveProjectUpdatesIndexMetadata) {
    auto dir = makeTmpDir("meta");
    ProjectManager pm(dir);
    pm.load();
    auto a = pm.create("A", "");
    auto modBefore = pm.findById(a.id)->modifiedAt;

    // Đảm bảo timestamp đổi (timestamp giây).
    std::this_thread::sleep_for(std::chrono::seconds(1));

    Project p = pm.loadProject(a.id);
    p.addTask(Task("T1", "", Date(1, 5, 2026), Date(2, 5, 2026), Priority::LOW));
    p.addTask(Task("T2", "", Date(1, 5, 2026), Date(3, 5, 2026), Priority::LOW));
    pm.saveProject(a.id, p);

    const ProjectMeta* m = pm.findById(a.id);
    ASSERT_NE(m, nullptr);
    EXPECT_EQ(m->taskCount, 2);
    EXPECT_NE(m->modifiedAt, modBefore);
}

TEST(ProjectManager, IndexPersistsAcrossInstances) {
    auto dir = makeTmpDir("persist");
    {
        ProjectManager pm1(dir);
        pm1.load();
        pm1.create("Alpha", "");
        pm1.create("Bravo", "");
    }
    ProjectManager pm2(dir);
    pm2.load();
    ASSERT_EQ(pm2.list().size(), 2u);
    EXPECT_EQ(pm2.list()[0].name, "Alpha");
    EXPECT_EQ(pm2.list()[1].name, "Bravo");
    EXPECT_GE(pm2.getNextId(), 3);
}

TEST(ProjectManager, MigrationFromLegacyFile) {
    auto dir = makeTmpDir("migrate");
    auto legacy = (fs::path(dir) / "legacy_save.csv").string();

    // Ghi 1 file legacy hợp lệ.
    Project p("Legacy", "imported");
    p.addTask(Task("OldTask", "", Date(1, 5, 2026), Date(5, 5, 2026), Priority::MEDIUM));
    FileHandler::saveProject(p, legacy);
    ASSERT_TRUE(fs::exists(legacy));

    auto pmDir = makeTmpDir("migrate_dir");
    ProjectManager pm(pmDir);
    pm.load();
    pm.migrateLegacySaveIfPresent(legacy);

    ASSERT_EQ(pm.list().size(), 1u);
    EXPECT_EQ(pm.list()[0].id, 1);
    EXPECT_EQ(pm.list()[0].name, "Legacy");
    EXPECT_EQ(pm.list()[0].taskCount, 1);
    EXPECT_TRUE(fs::exists(pm.list()[0].savePath));
    EXPECT_FALSE(fs::exists(legacy));
    EXPECT_TRUE(fs::exists(legacy + ".bak"));
}

TEST(ProjectManager, RenameUpdatesNameAndModifiedAt) {
    auto dir = makeTmpDir("rename");
    ProjectManager pm(dir);
    pm.load();
    auto a = pm.create("OldName", "");
    auto modBefore = pm.findById(a.id)->modifiedAt;
    std::this_thread::sleep_for(std::chrono::seconds(1));

    pm.rename(a.id, "NewName");
    const ProjectMeta* m = pm.findById(a.id);
    ASSERT_NE(m, nullptr);
    EXPECT_EQ(m->name, "NewName");
    EXPECT_NE(m->modifiedAt, modBefore);

    // Tên cũng được lưu vào file CSV của project.
    Project p = pm.loadProject(a.id);
    EXPECT_EQ(p.getProjectName(), "NewName");
}

TEST(ProjectManager, RenameUnknownIdThrows) {
    auto dir = makeTmpDir("renamebad");
    ProjectManager pm(dir);
    pm.load();
    EXPECT_THROW(pm.rename(42, "x"), ProjectNotFoundException);
}
