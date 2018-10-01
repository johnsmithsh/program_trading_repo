#include "os_time.h"

#include "gtest/gtest.h"

#include <stdio.h>

#include <errno.h>
#include <string.h>

//测试POSIX信号量: 内存信号量
TEST(os_time, tm_to_os_date_time)
{

   struct tm tm;
   time_t now;

   time(&now);
   localtime_r(&now, &tm);
   

   ST_OS_DATE os_date;
   ST_OS_DATE *pDate;

   memset(&os_date, 0, sizeof(os_date));
   pDate=tm_to_os_date(&tm, &os_date);
   printf("tm_to_os_date:  %04d-%02d-%2d, 周%d, 年天%d\n", os_date.year, os_date.month, os_date.day, os_date.weekday, os_date.yearday);
   //EXPECT_EQ(pDate, &os_date);
   //EXPECT_EQ(os_date.year, 2017);
   //EXPECT_EQ(os_date.month, 9);
   //EXPECT_EQ(os_date.day,  21);
   //EXPECT_EQ(os_date.weekday, 4);
   //EXPECT_EQ(os_date.yearday, 23);

   ST_OS_TIME os_time;
   memset(&os_time, 0, sizeof(os_time));
   tm_to_os_time(&tm, &os_time);
   printf("tm_to_os_time: %02d:%02d:%02d.%03d_%d\n", os_time.hour, os_time.minute, os_time.second, os_time.millisecond, os_time.macroseconds);

   printf("\n");
   memset(&os_date, 0, sizeof(os_date));
   os_get_date(&os_date);
   printf("os_get_date: %04d-%02d-%2d, 周%d, 年天%d\n", os_date.year, os_date.month, os_date.day, os_date.weekday, os_date.yearday);

   memset(&os_time, 0, sizeof(os_time));
   os_get_time(&os_time);
   printf("os_get_time: %02d:%02d:%02d.%03d_%d\n", os_time.hour, os_time.minute, os_time.second, os_time.millisecond, os_time.macroseconds);
   
}

int main(int argc, char **argv)
{
   testing::InitGoogleTest(&argc, argv);

   // Runs all tests using Google Test.
   return RUN_ALL_TESTS();
}
