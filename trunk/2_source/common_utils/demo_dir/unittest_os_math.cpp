#include "os_math.h"

#include "gtest/gtest.h"

#include <stdio.h>

#include <errno.h>
#include <string.h>

TEST(os_math, test)
{


   double d=12.123456789;
   double t=double_round(d, 0.000001);
   EXPECT_DOUBLE_EQ(t, 12.123457);

   t=double_round(12.12345642354);
   EXPECT_DOUBLE_EQ(t, 12.123454);

   int i,f;
   i=double_intpart(12.23435);
   EXPECT_EQ(i, 12);
   f=double_fractpart(12.23435);
   EXPECT_DOUBLE_EQ(f, 0.23435);

   i=double_intpart(-12.23435);
   EXPECT_EQ(i, -12);
   f=double_fractpart(-12.23435);
   EXPECT_DOUBLE_EQ(f, -0.23435);

   int rc=double_compare(12.3455, 12.3456, 0.001);
   EXPECT_EQ(rc, 0);
   rc=double_compare(12.34557, 12.34567, 0.0001);
   EXPECT_EQ(rc, -1);
   rc=double_compare(12.3465, 12.3456, 0.001);
   EXPECT_EQ(rc, 1);
   
   
}

int main(int argc, char **argv)
{
   testing::InitGoogleTest(&argc, argv);

   // Runs all tests using Google Test.
   return RUN_ALL_TESTS();
}
