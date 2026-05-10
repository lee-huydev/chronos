#include "UndoManager.h"

#include "Exceptions.h"

UndoManager::UndoManager() = default;

void UndoManager::saveState(const ProjectState& currentState) {
    if (static_cast<int>(history.size()) >= MAX_UNDO_DEPTH) {
        history.pop_front();
    }
    history.push_back(currentState);
}

ProjectState UndoManager::undo() {
    if (history.empty()) {
        throw UndoStackEmptyException();
    }
    ProjectState top = history.back();
    history.pop_back();
    return top;
}

void UndoManager::clear() { history.clear(); }
std::size_t UndoManager::size() const { return history.size(); }
bool UndoManager::isEmpty() const { return history.empty(); }
