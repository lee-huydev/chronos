#pragma once

#include <string>
#include <vector>

#include "Date.h"
#include "Project.h"
#include "ProjectManager.h"
#include "Task.h"
#include "UndoManager.h"

class Menu {
private:
    Project currentProject;
    int projectId = 0;
    ProjectManager* pm = nullptr;          // nullptr → legacy single-file mode
    UndoManager undoManager;
    std::string saveFilePath = "data/chronos_save.csv";
    bool isDirty = false;
    bool exitProgram = false;
    bool backToOuter = false;

    // Sort mode hint cho shortcut: 0 = unset (hỏi sub-menu), 1=urgency, 2=deadline, 3=priority.
    int pendingSortMode = 0;

public:
    // Legacy: tự auto-load từ file CSV duy nhất (giữ cho test parseCommand).
    Menu();
    explicit Menu(const std::string& saveFile);

    // Mode mới: dùng kèm ProjectManager + project hiện tại.
    Menu(const Project& project, int projectId, ProjectManager& manager);

    void run();

    // Public for testing
    int parseCommand(const std::string& input);

    // Test accessors
    const Project& getCurrentProject() const { return currentProject; }
    bool getIsDirty() const { return isDirty; }
    int getPendingSortMode() const { return pendingSortMode; }
    bool shouldExitProgram() const { return exitProgram; }
    bool shouldBackToOuter() const { return backToOuter; }
    int getProjectId() const { return projectId; }

private:
    void displayMainMenu();
    void displayTaskTable(const std::vector<Task>& tasks);

    void handleRenameCurrentProject();
    void handleAddTask();
    void handleViewTasks();
    void handleSort();
    void handleExtendDeadline();
    void handleUndo();
    void handleSaveLoad();
    void handleSearch();
    void handleDeleteTask();
    void handleUpdateStatus();
    void handleStressTest();
    void handleBackToOuter();
    void handleExit();

    int readInt(const std::string& prompt);
    std::string readString(const std::string& prompt);
    Date readDate(const std::string& prompt);
    Priority readPriority();
    bool readYesNo(const std::string& prompt);
    void pressEnterToContinue();

    void markDirty() { isDirty = true; }
    void markClean() { isDirty = false; }

    // Save về đúng nơi: ProjectManager nếu có, ngược lại dùng saveFilePath.
    void persistProject();
    void tryAutoLoad();
};
