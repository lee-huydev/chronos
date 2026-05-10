#include <gtest/gtest.h>

#include "Date.h"
#include "Exceptions.h"

TEST(DateTest, DefaultConstructor) {
    Date d;
    EXPECT_EQ(d.getDay(), 1);
    EXPECT_EQ(d.getMonth(), 1);
    EXPECT_EQ(d.getYear(), 2000);
}

TEST(DateTest, ValidDate) {
    Date d(15, 6, 2026);
    EXPECT_TRUE(d.isValid());
}

TEST(DateTest, InvalidDayThrows) {
    EXPECT_THROW(Date(32, 1, 2026), InvalidDateException);
    EXPECT_THROW(Date(0, 1, 2026), InvalidDateException);
}

TEST(DateTest, InvalidMonthThrows) {
    EXPECT_THROW(Date(1, 13, 2026), InvalidDateException);
    EXPECT_THROW(Date(1, 0, 2026), InvalidDateException);
}

TEST(DateTest, LeapYearFebruary) {
    EXPECT_NO_THROW(Date(29, 2, 2024));
    EXPECT_THROW(Date(29, 2, 2023), InvalidDateException);
    EXPECT_NO_THROW(Date(29, 2, 2000));
    EXPECT_THROW(Date(29, 2, 1900), InvalidDateException);
}

TEST(DateTest, DaysBetween) {
    Date d1(1, 1, 2026);
    Date d2(11, 1, 2026);
    EXPECT_EQ(d1.daysBetween(d2), 10);
    EXPECT_EQ(d2.daysBetween(d1), -10);
}

TEST(DateTest, AddDays) {
    Date d(28, 2, 2024);
    Date result = d.addDays(2);
    EXPECT_EQ(result.getDay(), 1);
    EXPECT_EQ(result.getMonth(), 3);
    EXPECT_EQ(result.getYear(), 2024);
}

TEST(DateTest, AddDaysAcrossYear) {
    Date d(31, 12, 2025);
    Date result = d.addDays(1);
    EXPECT_EQ(result.getDay(), 1);
    EXPECT_EQ(result.getMonth(), 1);
    EXPECT_EQ(result.getYear(), 2026);
}

TEST(DateTest, ToStringFormat) {
    Date d(5, 7, 2026);
    EXPECT_EQ(d.toString(), "05/07/2026");
}

TEST(DateTest, FromString) {
    Date d = Date::fromString("15/06/2026");
    EXPECT_EQ(d.getDay(), 15);
    EXPECT_EQ(d.getMonth(), 6);
    EXPECT_EQ(d.getYear(), 2026);
}

TEST(DateTest, FromStringInvalid) {
    EXPECT_THROW(Date::fromString("not-a-date"), InvalidDateException);
    EXPECT_THROW(Date::fromString("32/01/2026"), InvalidDateException);
}

TEST(DateTest, ComparisonOperators) {
    Date d1(1, 1, 2026);
    Date d2(2, 1, 2026);
    EXPECT_TRUE(d1 < d2);
    EXPECT_TRUE(d2 > d1);
    EXPECT_TRUE(d1 != d2);
    EXPECT_FALSE(d1 == d2);
    EXPECT_TRUE(d1 <= d2);
    EXPECT_TRUE(d2 >= d1);
    EXPECT_TRUE(d1 <= d1);
    EXPECT_TRUE(d1 >= d1);
}

TEST(DateTest, TodayReturnsValid) {
    Date t = Date::today();
    EXPECT_TRUE(t.isValid());
}
