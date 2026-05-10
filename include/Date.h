#pragma once

#include <iosfwd>
#include <string>

class Date {
private:
    int day;
    int month;
    int year;

public:
    Date();
    Date(int day, int month, int year);

    int getDay() const;
    int getMonth() const;
    int getYear() const;

    void setDay(int d);
    void setMonth(int m);
    void setYear(int y);

    bool isValid() const;
    int daysBetween(const Date& other) const;
    Date addDays(int n) const;
    std::string toString() const;
    static Date today();
    static Date fromString(const std::string& s);

    bool operator<(const Date& other) const;
    bool operator>(const Date& other) const;
    bool operator==(const Date& other) const;
    bool operator!=(const Date& other) const;
    bool operator<=(const Date& other) const;
    bool operator>=(const Date& other) const;
    friend std::ostream& operator<<(std::ostream& os, const Date& d);

    static bool isLeapYear(int year);
    static int daysInMonth(int month, int year);

private:
    long toJulian() const;
    static Date fromJulian(long jdn);
};
