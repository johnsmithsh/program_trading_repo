
#include <string.h>
#include <stdio.h>

#include "servercontext.h"

CServerContext *CServerContext::m_instance=NULL;

CServerContext::CServerContext():m_bcc_id(0),m_bu_no(0)
{
    memset(m_group_no,   0, sizeof(m_group_no));
    memset(m_group_desc, 0, sizeof(m_group_desc));
}

CServerContext::~CServerContext()
{

}


CServerContext::load_ini(const char *cfg_file)
{
    ConfigFile cfg;
	int rc=cfg.load_cfg_file(cfg_file);
	if(rc < 0)
	{
	    ERROR_MSG("打开配置文件[%s]失败, rc=[%d]", cfg_file, rc);
	    return -1;
	}

    memset(group_no,     0, sizeof(group_no));
    memset(m_group_desc, 0, sizeof(m_group_desc));
    
	char serve_section[]="unit";
    cfg.read_string(serve_section,  "group_no",   group_no,    sizeof(group_no),   "");
    cfg.read_string(serve_section,  "group_desc", m_group_desc,sizeof(group_desc), "");
    
	if('\0'==group_no[0])
	{
		ERROR_MSG("read cfg option [%s]group_no failed! rc=[%d]", serve_section, rc);
		return -2;
	}
    
    if('\0'==group_desc[0])
    {
        strcpy(group_desc, group_no);
    }
    
    return 0;
}

