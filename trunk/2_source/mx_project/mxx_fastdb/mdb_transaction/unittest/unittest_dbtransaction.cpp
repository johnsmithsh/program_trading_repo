#include <stdio.h>
#include <string.h>

#include "gtest/gtest.h"


#include "mdb_transaction.h"
TEST(CMdbTransaction, CMdbTransaction)
{
    CMdbTransaction mdb_transaction;
}

int main(int argc, char **argv)
{
    
    testing::InitGoogleTest(&argc, argv);

   // Runs all tests using Google Test.
   return RUN_ALL_TESTS();
    
    return 0;
}