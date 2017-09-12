#include "itc_semaphore.h"

int ItcSem::init(int shared, int init_value)
{
    int rc;
    rc=sem_init(&m_sem, (0==shared)?0:1, init_value);
    b_init_succ_flag=(0==rc);
}

ItcSem::ItcSem()
{
   init(0, 1);
}

ItcSem::ItcSem(int shared, int init_value)
{
   init(shared, init_value);
}

ItcSem::~ItcSem()
{
   if(is_init_succ())
   {
      int rc;
      rc=sem_destroy(&m_sem);
      b_init_succ_flag=false;
   }
}

int ItcSem::wait()
{
   if(!is_init_succ()) return -1;
   int rc;
   rc=sem_wait(&m_sem);
   if(0==rc)//获取到所有权
      return 0;
   else //没有获取到
      return -1;
}


