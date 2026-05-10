#include "Date.h"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>

#include "Exceptions.h"

namespace {
constexpr int MIN_YEAR = 1900;
constexpr int MAX_YEAR = 2100;
}  // namespace

Date::Date() : day(1), month(1), year(2000) {}

Date::Date(int day_, int month_, int year_) : day(day_), month(month_), year(year_) {
    if (!isValid()) {
        std::ostringstream os;
        os << day_ << "/" << month_ << "/" << year_;
        throw InvalidDateException(os.str());
    }
}

int Date::getDay() const { return day; }
int Date::getMonth() const { return month; }
int Date::getYear() const { return year; }

void Date::setDay(int d) {
    Date tmp(d, month, year);
    *this = tmp;
}
void Date::setMonth(int m) {
    Date tmp(day, m, year);
    *this = tmp;
}
void Date::setYear(int y) {
    Date tmp(day, month, y);
    *this = tmp;
}

bool Date::isLeapYear(int y) { return (y % 4 == 0 && (y % 100 != 0 || y % 400 == 0)); }

int Date::daysInMonth(int m, int y) {
    static const int dim[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (m < 1 || m > 12) return 0;
    if (m == 2 && isLeapYear(y)) return 29;
    return dim[m - 1];
}

bool Date::isValid() const {
    if (year < MIN_YEAR || year > MAX_YEAR) return false;
    if (month < 1 || month > 12) return false;
    if (day < 1 || day > daysInMonth(month, year)) return false;
    return true;
}

long Date::toJulian() const {
    int a = (14 - month) / 12;
    int y = year + 4800 - a;
    int m = month + 12 * a - 3;
    return day + (153L * m + 2) / 5 + 365L * y + y / 4 - y / 100 + y / 400 - 32045;
}

Date Date::fromJulian(long jdn) {
    long a = jdn + 32044;
    long b = (4 * a + 3) / 146097;
    long c = a - (146097 * b) / 4;
    long d = (4 * c + 3) / 1461;
    long e = c - (1461 * d) / 4;
    long m = (5 * e + 2) / 153;
    int day_ = static_cast<int>(e - (153 * m + 2) / 5 + 1);
    int month_ = static_cast<int>(m + 3 - 12 * (m / 10));
    int year_ = static_cast<int>(100 * b + d - 4800 + m / 10);
    return Date(day_, month_, year_);
}

int Date::daysBetween(const Date& other) const {
    return static_cast<int>(other.toJulian() - toJulian());
}

Date Date::addDays(int n) const { return fromJulian(toJulian() + n); }

std::string Date::toString() const {
    std::ostringstream os;
    os << std::setfill('0') << std::setw(2) << day << "/" << std::setw(2) << month << "/"
       << std::setw(4) << year;
    return os.str();
}

Date Date::today() {
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::tm tm{};
#if defined(_WIN32)
    localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);
#endif
    return Date(tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900);
}

Date Date::fromString(const std::string& s) {
    int d = 0, m = 0, y = 0;
    char sep1 = 0, sep2 = 0;
    std::istringstream is(s);
    is >> d >> sep1 >> m >> sep2 >> y;
    if (is.fail() || sep1 != '/' || sep2 != '/') {
        throw InvalidDateException("Khong parse duoc \"" + s + "\" (mong format DD/MM/YYYY)");
    }
    return Date(d, m, y);
}

bool Date::operator<(const Date& other) const { return toJulian() < other.toJulian(); }
bool Date::operator>(const Date& other) const { return toJulian() > other.toJulian(); }
bool Date::operator==(const Date& other) const { return toJulian() == other.toJulian(); }
bool Date::operator!=(const Date& other) const { return !(*this == other); }
bool Date::operator<=(const Date& other) const { return !(other < *this); }
bool Date::operator>=(const Date& other) const { return !(*this < other); }

std::ostream& operator<<(std::ostream& os, const Date& d) {
    os << d.toString();
    return os;
}
