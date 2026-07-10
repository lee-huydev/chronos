#include "TaskFilter.h"

#include <algorithm>
#include <cctype>
#include <stdexcept>
#include <string>
#include <utility>
#include <Exceptions.h>

namespace
{
    std::string toLower(std::string s)
    {
        std::transform(s.begin(), s.end(), s.begin(),
                       [](unsigned char c)
                       { return std::tolower(c); });
        return s;
    }

    std::string trim(std::string s)
    {
        auto notSpace = [](unsigned char c)
        { return !std::isspace(c); };
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), notSpace));
        s.erase(std::find_if(s.rbegin(), s.rend(), notSpace).base(), s.end());
        return s;
    }

    bool containsCI(const std::string &haystack, const std::string &needle)
    {
        if (needle.empty())
            return true;
        return toLower(haystack).find(toLower(needle)) != std::string::npos;
    }

    // Phat hien prefix loc da biet (case-insensitive): p:, s:, dd:, od:
    bool isFilterPrefix(const std::string &q)
    {
        std::string lower = toLower(q);
        if (lower.size() >= 2 && lower[1] == ':')
        {
            char c = lower[0];
            return c == 'p' || c == 's';
        }
        if (lower.size() >= 3 && lower[2] == ':')
        {
            std::string p2 = lower.substr(0, 2);
            return p2 == "dd" || p2 == "od";
        }
        return false;
    }

    // Tra ve (prefix_lower, value_trimmed)
    std::pair<std::string, std::string> splitPrefix(const std::string &q)
    {
        size_t colon = q.find(':');
        std::string prefix = toLower(q.substr(0, colon + 1));
        std::string value = trim(q.substr(colon + 1));
        return {prefix, value};
    }
} // namespace

std::vector<Task> TaskFilter::searchByKeyword(const std::vector<Task> &tasks,
                                              const std::string &keyword)
{
    std::vector<Task> out;
    for (const auto &t : tasks)
    {
        if (containsCI(t.getTitle(), keyword) || containsCI(t.getDescription(), keyword))
        {
            out.push_back(t);
        }
    }
    return out;
}

std::vector<Task> TaskFilter::filterByPriority(const std::vector<Task> &tasks, Priority p)
{
    std::vector<Task> out;
    for (const auto &t : tasks)
    {
        if (t.getPriority() == p)
            out.push_back(t);
    }
    return out;
}

std::vector<Task> TaskFilter::filterByStatus(const std::vector<Task> &tasks, Status s)
{
    std::vector<Task> out;
    for (const auto &t : tasks)
    {
        if (t.getStatus() == s)
            out.push_back(t);
    }
    return out;
}

std::vector<Task> TaskFilter::filterByDateRange(const std::vector<Task> &tasks,
                                                const Date &from, const Date &to)
{
    std::vector<Task> out;
    for (const auto &t : tasks)
    {
        if (t.getDueDate() >= from && t.getDueDate() <= to)
            out.push_back(t);
    }
    return out;
}

std::vector<Task> TaskFilter::getOverdueTasks(const std::vector<Task> &tasks)
{
    std::vector<Task> out;
    for (const auto &t : tasks)
    {
        if (t.isOverdue())
            out.push_back(t);
    }
    return out;
}

std::vector<Task> TaskFilter::applyFilter(const std::vector<Task> &tasks,
                                          const std::string &query)
{
    std::string q = trim(query);
    if (q.empty())
        return tasks;

    if (!isFilterPrefix(q))
    {
        return searchByKeyword(tasks, q);
    }

    auto [prefix, value] = splitPrefix(q);

    if (prefix == "p:")
    {
        if (value.empty())
        {
            throw std::invalid_argument(
                "Thieu gia tri priority sau 'p:'. Vi du: p:HIGH");
        }
        // priorityFromString tu Task.cpp da throw std::invalid_argument neu sai.
        Priority p = priorityFromString(value);
        return filterByPriority(tasks, p);
    }

    if (prefix == "s:")
    {
        if (value.empty())
        {
            throw std::invalid_argument(
                "Thieu gia tri status sau 's:'. Vi du: s:DONE");
        }
        Status s = statusFromString(value);
        return filterByStatus(tasks, s);
    }

    if (prefix == "dd:")
    {
        if (value.empty())
        {
            throw std::invalid_argument(
                "Thieu khoang deadline sau 'dd:'. Vi du: dd:01/01/2026-31/12/2026");
        }
        size_t dash = value.find('-');
        if (dash == std::string::npos)
        {
            throw std::invalid_argument(
                "Dinh dang dd: phai la DD/MM/YYYY-DD/MM/YYYY, nhan duoc: " + value);
        }
        // Date::fromString tu Date.cpp nem InvalidDateException (runtime_error);
        // ep ve std::invalid_argument de giu contract: input sai -> invalid_argument.
        try
        {
            Date from = Date::fromString(value.substr(0, dash));
            Date to = Date::fromString(value.substr(dash + 1));
            if (from > to)
                throw TimeParadoxException();

            return filterByDateRange(tasks, from, to);
        }
        catch (const TimeParadoxException &)
        {
            throw std::invalid_argument(
                "Khoang thoi gian khong hop le: ngay bat dau phai truoc/sau ngay ket thuc");
        }
        catch (const std::exception &e)
        {
            throw std::invalid_argument(std::string("Deadline khong hop le: ") + e.what());
        }
    }

    if (prefix == "od:")
    {
        if (value != "true")
        {
            throw std::invalid_argument(
                "od: chi chap nhan gia tri chinh xac 'true', nhan duoc: '" + value + "'");
        }
        return getOverdueTasks(tasks);
    }

    // Khong xay ra (isFilterPrefix da chan), nhung de fallback an toan:
    return searchByKeyword(tasks, q);
}