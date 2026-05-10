#include "Task.h"

#include <algorithm>
#include <cctype>
#include <iostream>
#include <ostream>
#include <stdexcept>
#include <string>

#include "Exceptions.h"

int Task::nextId = 1;

namespace {
std::string toUpper(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c) { return std::toupper(c); });
    return s;
}
}  // namespace

std::string priorityToString(Priority p) {
    switch (p) {
        case Priority::LOW: return "LOW";
        case Priority::MEDIUM: return "MEDIUM";
        case Priority::HIGH: return "HIGH";
        case Priority::CRITICAL: return "CRITICAL";
    }
    return "LOW";
}

Priority priorityFromString(const std::string& s) {
    std::string up = toUpper(s);
    if (up == "LOW") return Priority::LOW;
    if (up == "MEDIUM") return Priority::MEDIUM;
    if (up == "HIGH") return Priority::HIGH;
    if (up == "CRITICAL") return Priority::CRITICAL;
    throw std::invalid_argument("Priority khong hop le: " + s);
}

std::string statusToString(Status s) {
    switch (s) {
        case Status::PENDING: return "PENDING";
        case Status::IN_PROGRESS: return "IN_PROGRESS";
        case Status::DONE: return "DONE";
    }
    return "PENDING";
}

Status statusFromString(const std::string& s) {
    std::string up = toUpper(s);
    if (up == "PENDING") return Status::PENDING;
    if (up == "IN_PROGRESS") return Status::IN_PROGRESS;
    if (up == "DONE") return Status::DONE;
    throw std::invalid_argument("Status khong hop le: " + s);
}

Task::Task()
    : id(nextId++),
      title(""),
      description(""),
      startDate(),
      dueDate(),
      priority(Priority::LOW),
      status(Status::PENDING) {}

Task::Task(const std::string& title_,
           const std::string& description_,
           const Date& startDate_,
           const Date& dueDate_,
           Priority priority_)
    : id(nextId++),
      title(title_),
      description(description_),
      startDate(startDate_),
      dueDate(dueDate_),
      priority(priority_),
      status(Status::PENDING) {
    if (dueDate_ < startDate_) {
        throw TimeParadoxException();
    }
}

Task::Task(const Task& other) = default;
Task& Task::operator=(const Task& other) = default;
Task::~Task() = default;

int Task::getId() const { return id; }
std::string Task::getTitle() const { return title; }
std::string Task::getDescription() const { return description; }
Date Task::getStartDate() const { return startDate; }
Date Task::getDueDate() const { return dueDate; }
Priority Task::getPriority() const { return priority; }
Status Task::getStatus() const { return status; }

void Task::setTitle(const std::string& t) { title = t; }
void Task::setDescription(const std::string& d) { description = d; }

void Task::setStartDate(const Date& d) {
    if (dueDate < d) {
        throw TimeParadoxException();
    }
    startDate = d;
}

void Task::setDueDate(const Date& d) {
    if (d < startDate) {
        throw TimeParadoxException();
    }
    dueDate = d;
}

void Task::setPriority(Priority p) { priority = p; }
void Task::setStatus(Status s) { status = s; }

int Task::daysLeft() const { return Date::today().daysBetween(dueDate); }

bool Task::isOverdue() const {
    if (status == Status::DONE) return false;
    return dueDate < Date::today();
}

void Task::display() const {
    std::cout << "[#" << id << "] " << title
              << " | " << priorityToString(priority)
              << " | " << statusToString(status)
              << " | " << startDate.toString() << " -> " << dueDate.toString();
    if (!description.empty()) {
        std::cout << "\n    " << description;
    }
    std::cout << std::endl;
}

bool Task::operator<(const Task& other) const {
    if (priority != other.priority) {
        return static_cast<int>(priority) > static_cast<int>(other.priority);
    }
    return dueDate < other.dueDate;
}

bool Task::operator==(const Task& other) const { return id == other.id; }

Task& Task::operator++() {
    dueDate = dueDate.addDays(1);
    return *this;
}

Task Task::operator++(int) {
    Task copy = *this;
    dueDate = dueDate.addDays(1);
    return copy;
}

std::ostream& operator<<(std::ostream& os, const Task& t) {
    os << "[#" << t.id << "] " << t.title
       << " (" << priorityToString(t.priority) << "/"
       << statusToString(t.status) << ") "
       << t.startDate << " -> " << t.dueDate;
    return os;
}

void Task::resetIdCounter() { nextId = 1; }

void Task::setIdForLoad(int newId) {
    id = newId;
    if (newId >= nextId) nextId = newId + 1;
}
