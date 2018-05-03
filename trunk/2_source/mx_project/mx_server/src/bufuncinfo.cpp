
#include <string.h>

#include "bufuncinfo.h"

CBuFuncList::CBuFuncList()
{
}

CBuFuncList::~CBuFuncList()
{
    m_func_vec.clear();
}

void CBuFuncList::clear()
{
    m_func_vec.clear();
}

int CBuFuncList::add_func_info(char *func_id, char *func_desc)
{
    if((NULL==func_id)  || (strlen(func_id)<=0))
        return -1;
    const ST_BUFUNC_ITEM *item_ptr=this->find_func_info(func_id);
    if(NULL!=item_ptr)
        return 0;
    ST_BUFUNC_ITEM func_info;
    memset(&func_info, 0, sizeof(ST_BUFUNC_ITEM));
    strncpy(func_info.func_id,   func_id,   sizeof(func_info.func_id)-1);
    strncpy(func_info.func_desc, func_desc, sizeof(func_info.func_desc)-1);
    m_func_vec.push_back(func_info);
    return 0;
}

int CBuFuncList::remove_func_info(char *func_id)
{
    if((NULL==func_id)||(strlen(func_id)<=0))
        return 0;
    
    std::vector<ST_BUFUNC_ITEM>::iterator iter;
    for(iter=m_func_vec.begin(); iter!=m_func_vec.end(); iter++)
    {
        ST_BUFUNC_ITEM &item_ref=*iter;//!< 为了方便测试查看
        if(0==strcmp(func_id, item_ref.func_id))
        {
            m_func_vec.erase(iter);
            return 0;
        }
    }
    
    return 0;
}

const ST_BUFUNC_ITEM *CBuFuncList::find_func_info(char *func_id)
{
    std::vector<ST_BUFUNC_ITEM>::iterator iter;
    for(iter=m_func_vec.begin(); iter!=m_func_vec.end(); iter++)
    {
        ST_BUFUNC_ITEM &item_ref=*iter;//!< 为了方便测试查看
        if(0==strcmp(func_id, item_ref.func_id))
            return &(*iter);
    }
    return NULL;
}


