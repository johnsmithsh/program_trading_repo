#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "servermanage.h"

CServerManage g_server_manage;
int main(int argc, char **argv)
{

    int rc=0;
    rc=g_server_manage.init();
    if(rc<0)
    {
        printf("Error: g_server_manage初始化失败");
        return -1;
    }
    
    rc=g_server_manage.start_service();
    if(rc<0)
    {
        printf("Error: g_server_manage启动失败");
        return -2;
    }
    
    while(EXITCODE_TRUE!=global_ctrl_ptr->exit_code)
    {
        sleep(2);
    }
    
    g_server_manage.stop_service();
    
    return 0;
}
