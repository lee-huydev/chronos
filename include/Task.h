#pragma once

#include <iosfwd>
#include <string>

#include "Date.h"

enum class Priority {
    LOW = 1,
    MEDIUM = 2,
    HIGH = 3,
    CRITICAL = 4
};

enum class Status {
    PENDING,
    IN_PROGRESS,
    DONE
};

std::string priorityToString(Priority p);
Priority priorityFromString(const std::string& s);
std::string statusToString(Status s);
Status statusFromString(const std::string& s);

class Task {
private:
    int id;
    std::string title;
    std::string description;
    Date startDate;
    Date dueDate;
    Priority priority;
    Status status;

    static int nextId;

public:
    Task();
    Task(const std::string& title,
         const std::string& description,
         const Date& startDate,
         const Date& dueDate,
         Priority priority);

    Task(const Task& other);
    Task& operator=(const Task& other);
    ~Task();

    int getId() const;
    std::string getTitle() const;
    std::string getDescription() const;
    Date getStartDate() const;
    Date getDueDate() const;
    Priority getPriority() const;
    Status getStatus() const;

    void setTitle(const std::string& t);
    void setDescription(const std::string& d);
    void setStartDate(const Date& d);
    void setDueDate(const Date& d);
    void setPriority(Priority p);
    void setStatus(Status s);

    int daysLeft() const;
    bool isOverdue() const;
    void display() const;

    bool operator<(const Task& other) const;
    bool operator==(const Task& other) const;
    Task& operator++();
    Task operator++(int);
    friend std::ostream& operator<<(std::ostream& os, const Task& t);

    static void resetIdCounter();

    // Internal helper: assign id without incrementing nextId (for FileHandler load).
    void setIdForLoad(int newId);
};
