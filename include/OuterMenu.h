#pragma once

#include <string>

#include "ProjectManager.h"

class OuterMenu {
private:
    ProjectManager& pm;

public:
    explicit OuterMenu(ProjectManager& pm);
    void run();

    // Public for testing
    int parseCommand(const std::string& input) const;

private:
    void displayHeader() const;
    void displayProjectList() const;
    void displayMenuOptions() const;

    void firstLaunchWizard();
    void handleSelect();
    void handleCreate();
    void handleRename();
    void handleDelete();
    void enterMainMenu(int projectId);

    int readInt(const std::string& prompt) const;
    std::string readString(const std::string& prompt) const;
    bool readYesNo(const std::string& prompt) const;
};
