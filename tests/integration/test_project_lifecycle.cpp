#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <filesystem>
#include <string>

#include "Exceptions.h"
#include "ProjectManager.h"
#include "Task.h"

namespace fs = std::filesystem;

namespace {
std::atomic<int> tmpCounter{0};
std::string makeTmpDir(const std::string& tag) {
    auto base = fs::temp_directory_path() / "chronos_lifecycle_test";
    fs::create_directories(base);
    auto ts = std::chrono::steady_clock::now().time_since_epoch().count();
    std::string dir = (base / (tag + "_" + std::to_string(ts) + "_" +
                              std::to_string(++tmpCounter))).string();
    fs::create_directories(dir);
    return dir;
}
}  // namespace

TEST(ProjectLifecycle, FirstLaunchEmptyDirCanCreateFirstProject) {
    auto dir = makeTmpDir("first");
    ProjectManager pm(dir);
    pm.load();
    ASSERT_TRUE(pm.isEmpty());

    auto m = pm.create("Day1", "first ever");
    EXPECT_EQ(m.id, 1);
    EXPECT_FALSE(pm.isEmpty());

    Project p = pm.loadProject(m.id);
    EXPECT_EQ(p.getProjectName(), "Day1");
    EXPECT_EQ(p.getTaskCount(), 0);
}

TEST(ProjectLifecycle, MultipleProjectsSwitchPreservesData) {
    auto dir = makeTmpDir("multi");
    ProjectManager pm(dir);
    pm.load();
    auto a = pm.create("ProjA", "");
    auto b = pm.create("ProjB", "");

    {
        Project pa = pm.loadProject(a.id);
        pa.addTask(Task("A-T1", "", Date(1, 5, 2026), Date(2, 5, 2026), Priority::LOW));
        pa.addTask(Task("A-T2", "", Date(1, 5, 2026), Date(3, 5, 2026), Priority::HIGH));
        pm.saveProject(a.id, pa);
    }
    {
        Project pb = pm.loadProject(b.id);
        pb.addTask(Task("B-T1", "", Date(1, 5, 2026), Date(4, 5, 2026), Priority::MEDIUM));
        pm.saveProject(b.id, pb);
    }

    // Reload PM mới — đảm bảo dữ liệu persist độc lập.
    ProjectManager pm2(dir);
    pm2.load();
    ASSERT_EQ(pm2.list().size(), 2u);

    Project pa2 = pm2.loadProject(a.id);
    Project pb2 = pm2.loadProject(b.id);
    EXPECT_EQ(pa2.getTaskCount(), 2);
    EXPECT_EQ(pb2.getTaskCount(), 1);
    EXPECT_EQ(pa2.getAllTasks()[0].getTitle(), "A-T1");
    EXPECT_EQ(pb2.getAllTasks()[0].getTitle(), "B-T1");
}

TEST(ProjectLifecycle, DeleteOneProjectKeepsOthers) {
    auto dir = makeTmpDir("delone");
    ProjectManager pm(dir);
    pm.load();
    auto a = pm.create("A", "");
    auto b = pm.create("B", "");
    pm.deleteProject(a.id);

    ProjectManager pm2(dir);
    pm2.load();
    ASSERT_EQ(pm2.list().size(), 1u);
    EXPECT_EQ(pm2.list()[0].id, b.id);
    EXPECT_EQ(pm2.findById(a.id), nullptr);
}

TEST(ProjectLifecycle, DeleteAllReturnsToEmptyState) {
    auto dir = makeTmpDir("delall");
    ProjectManager pm(dir);
    pm.load();
    auto a = pm.create("Solo", "");
    pm.deleteProject(a.id);
    EXPECT_TRUE(pm.isEmpty());

    // Reload xác nhận trên đĩa cũng rỗng.
    ProjectManager pm2(dir);
    pm2.load();
    EXPECT_TRUE(pm2.isEmpty());
}

TEST(ProjectLifecycle, RenameProjectPersistsAcrossReload) {
    auto dir = makeTmpDir("rename_lc");
    ProjectManager pm(dir);
    pm.load();
    auto a = pm.create("Old", "");
    pm.rename(a.id, "Brand New");

    ProjectManager pm2(dir);
    pm2.load();
    ASSERT_EQ(pm2.list().size(), 1u);
    EXPECT_EQ(pm2.list()[0].name, "Brand New");

    Project p = pm2.loadProject(a.id);
    EXPECT_EQ(p.getProjectName(), "Brand New");
}
