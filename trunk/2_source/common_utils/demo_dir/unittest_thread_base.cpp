#include "thread_base.h"

#include "gtest/gtest.h"

#include <unistd.h>
#include <stdio.h>

#include <errno.h>
#include <string.h>

class thread_test: public Thread_Base
{
   public:
     thread_test(const char *name): Thread_Base(name)
     {
     }
     virtual ~thread_test() {};

   public:
     virtual int init() { return 0; }
     virtual void run()
     {
        int i=0; 
        for (;;)
        {
           ++i;
           printf("[%s] is running... count=[%d]\n", m_thread_name, i);
           if(i>10000) i=0;
           sleep(3);
        }
     }

     virtual int clear() { return 0; }
};


TEST(os_math, test)
{
   
  thread_test thread1("thread_test1");
  thread1.start();
  sleep(30);
}

int main(int argc, char **argv)
{
   testing::InitGoogleTest(&argc, argv);

   // Runs all tests using Google Test.
   return RUN_ALL_TESTS();
}
