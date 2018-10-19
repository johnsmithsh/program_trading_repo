
#include <string.h>
#include <stdio.h>

//#include "msg_link_define.h"
#include "msg_link_function.h"


///** 
// * @brief 构建建立连接报文
// * @param
// *    [in]group_no: 业务组号
// *    [in]group_desc: 业务组描述
// *    [in]group_version: 业务组版本
// *
// *    [out]buff:缓存指针
// *    [in]buffsize:缓存大小
// * @retval
// *    0-成功; <0-失败;
// */
//int lmasm_req_conn(const char *group_no, const char *group_desc, const char *group_version, int pid, char *buff, size_t buffsize)
//{
//    if( (NULL==buff)||(buffsize<sizeof(ST_MSG_HEAD)+sizeof(MSG_REQ_CONN)) )
//        return -1;
//    memset(buff, 0, sizeof(ST_MSG_HEAD)+sizeof(MSG_REQ_CONN));
//    ST_MSG_HEAD *head_ptr=(ST_MSG_HEAD*)buff;
//    //设置报文头
//    head_ptr->msgid=MSGTYPE_CONN;
//    head_ptr->data_len = sizeof(MSG_REQ_CONN);
//    
//    MSG_REQ_CONN *body_ptr=(MSG_REQ_CONN*)(buff+sizeof(ST_MSG_HEAD));
//    strncpy(body_ptr->group_no,   group_no, sizeof(body_ptr->group_no)-1);//!< 设置业务组号
//    strncpy(body_ptr->group_desc, group_desc, sizeof(body_ptr->group_desc)-1);//!< 设置业务组描述信息
//    strncpy(body_ptr->version, group_version, sizeof(body_ptr->version));//!< 设置版本信息
//    return 0;    
//}
//
///** 
// * @brief 构建建立连接应答报文
// * @param
// *    [in]bu_no: 控制中心为业务服务分配的id;每个服务进程都不相同;
// *    [in]bcc_id: 本业务中心id
// *    [in]if_succ/szmsg: 成功标记及说明信息
// *
// *    [out]buff:缓存指针
// *    [in]buffsize:缓存大小
// * @retval
// *    0-成功; <0-失败;
// */
//int lmasm_ans_conn(int bu_no, int bcc_id, char if_succ, const char *szmsg, char *buff, size_t buffsize)
//{
//    if( (NULL==buff)||(buffsize<sizeof(ST_MSG_HEAD)+sizeof(MSG_ANS_CONN)) )
//        return -1;
//    memset(buff, 0, sizeof(ST_MSG_HEAD)+sizeof(MSG_ANS_CONN));
//    ST_MSG_HEAD *head_ptr=(ST_MSG_HEAD*)buff;
//    //设置报文头
//    head_ptr->msgid=MSGTYPE_CONN;
//    head_ptr->data_len = sizeof(MSG_ANS_CONN);
//    
//    MSG_ANS_CONN *body_ptr=(MSG_ANS_CONN*)(buff+sizeof(ST_MSG_HEAD));
//    body_ptr->bu_no=bu_no;//!< 分配业务服务id
//    body_ptr->bcc_id=bcc_id;//!< 设置本业务中心id
//    body_ptr->if_succ=if_succ;//!< 设置成功标记
//    strncpy(body_ptr->szmsg, szmsg, sizeof(body_ptr->szmsg)-1);
//    return 0;    
//}
//
////---------------------------------------------------------------------------------------
///** 
// * @brief 构建注册业务功能报文初始化
// * @param.
// *    [out]buff:缓存指针
// *    [in]buffsize:缓存大小
// * @retval
// *    0-成功; <0-失败;
// */
//int lmasm_req_regfunc_init(char *buff, size_t buffsize)
//{
//    if( (NULL==buff)||(buffsize<sizeof(ST_MSG_HEAD)) )
//        return -1;
//    memset(buff, 0, sizeof(ST_MSG_HEAD));
//    ST_MSG_HEAD *head_ptr=(ST_MSG_HEAD*)buff;
//    //设置报文头
//    head_ptr->msgid=MSGTYPE_REG_FUNC;
//    head_ptr->data_len = 0;
//    return 0;
//}
//
///** 
// * @brief 注册业务功能报文 增加业务功能信息
// * @param
// *    [in]func_id,func_desc: 业务id与业务描述;
// *    [in]func_type:业务类型
// *    [out]buff:缓存指针
// *    [in]buffsize:缓存大小
// * @retval
// *    0-成功; <0-失败;
// * @note
// *    调用该函数前必须先初始化报文头
// */
//int lmasm_req_regfunc_funcinfo_append(const char *func_id, const char *func_desc, char func_type, char *buff, size_t buffsize)
//{
//    if( (NULL==buff) )
//        return -1;
//    ST_MSG_HEAD *head_ptr=(ST_MSG_HEAD*)buff;
//    if( buffsize<(head_ptr->data_len+sizeof(MSG_REQ_REGFUNC)) )
//        return -2;
//    
//    //设置报文头
//    MSG_REQ_REGFUNC *func_info_ptr=(MSG_REQ_REGFUNC*)(buff+sizeof(ST_MSG_HEAD)+head_ptr->data_len);
//    strncpy(func_info_ptr->bu_func_id, func_id, sizeof(func_info_ptr->bu_func_id)-1);
//    strncpy(func_info_ptr->bu_func_desc, func_desc, sizeof(func_info_ptr->bu_func_desc)-1);
//    func_info_ptr->bu_func_type=func_type;
//
//    head_ptr->data_len += sizeof(MSG_REQ_REGFUNC);
//    return 0;
//}
//
///** 
// * @brief 构建注册函数应答报文
// * @param
// *    [in]if_succ/szmsg: 成功标记及说明信息
// *
// *    [out]buff:缓存指针
// *    [in]buffsize:缓存大小
// * @retval
// *    0-成功; <0-失败;
// */
//int lmasm_ans_regfunc(char if_succ, const char *szmsg, char *buff, size_t buffsize)
//{
//    if( (NULL==buff)||(buffsize<sizeof(ST_MSG_HEAD)+sizeof(MSG_ANS_REGFUNC)) )
//        return -1;
//    memset(buff, 0, sizeof(ST_MSG_HEAD)+sizeof(MSG_ANS_REGFUNC));
//    ST_MSG_HEAD *head_ptr=(ST_MSG_HEAD*)buff;
//    //设置报文头
//    head_ptr->msgid=MSGTYPE_REG_FUNC;
//    head_ptr->data_len = sizeof(MSG_ANS_REGFUNC);
//    //设置报文数据
//    MSG_ANS_REGFUNC *body_ptr=(MSG_ANS_REGFUNC*)(buff+sizeof(ST_MSG_HEAD));
//    body_ptr->if_succ=if_succ;//!< 分配业务服务id
//    strncpy(body_ptr->szmsg, szmsg, sizeof(body_ptr->szmsg)-1);
//    return 0;    
//}
////----------------------------------------------------------------------------------------
///** 
// * @brief 业务请求报文初始化
// * @param
// *    [out]buff:缓存指针
// *    [in]buffsize:缓存大小
// * @retval
// *    0-成功; <0-失败;
// */
//int lmasm_req_request_init(char *buff, size_t buffsize)
//{
//    if( (NULL==buff)||(buffsize<sizeof(ST_MSG_HEAD)+sizeof(MSG_REQ_REQUEST)) )
//        return -1;
//    memset(buff, 0, sizeof(ST_MSG_HEAD));
//    ST_MSG_HEAD *head_ptr=(ST_MSG_HEAD)buff;
//    //设置报文头
//    head_ptr->msgid=MSGTYPE_REQ_REQUEST;
//    head_ptr->data_len = 0;
//    
//    //设置请求数据说明
//    MSG_REQ_REQUEST *req_request_info=(MSG_REQ_REQUEST*)(buff+sizeof(ST_MSG_HEAD));
//    memset(req_request_info, 0, sizeof(MSG_REQ_REQUEST);
//    return 0;
//}
//
///** 
// * @brief 业务请求报文:设置连接信息
// * @param
// *    [out]buff:缓存指针
// *    [in]buffsize:缓存大小
// * @retval
// *    0-成功; <0-失败;
// */
//int lmasm_req_request_set_linkinfo(const char *group_no, int bu_no, int bcc_id, char *buff, size_t buffsize)
//{
//    if( (NULL==buff)||(buffsize<sizeof(ST_MSG_HEAD)+sizeof(MSG_REQ_REQUEST)) )
//        return -1;
//    //ST_MSG_HEAD *head_ptr=(ST_MSG_HEAD)buff;
//    MSG_REQ_REQUEST *req_request_info=(MSG_REQ_REQUEST*)(buff+sizeof(ST_MSG_HEAD));
//    strncpy(req_request_info->group_no, group_no, sizeof(req_request_info->group_no)-1);
//    req_request_info->bu_no = bu_no;
//    req_request_info->bcc_id = bcc_id;
//    
//    return 0;
//}
//
///** 
// * @brief 业务请求报文:设置控制标记
// * @param
// *    [out]buff:缓存指针
// *    [in]buffsize:缓存大小
// * @retval
// *    0-成功; <0-失败;
// */
//int lmasm_req_request_set_ctrlinfo(unsigned int request_id, unsigned char mode, unsigned char first_flag=0, unsigned char next_flag=0, unsigned char push_flag=0, char *buff, size_t buffsize)
//{
//    if( (NULL==buff)||(buffsize<sizeof(ST_MSG_HEAD)+sizeof(MSG_REQ_REQUEST)) )
//        return -1;
//    //ST_MSG_HEAD *head_ptr=(ST_MSG_HEAD)buff;
//    MSG_REQ_REQUEST *req_request_info=(MSG_REQ_REQUEST*)(buff+sizeof(ST_MSG_HEAD));
//    strncpy(req_request_info->group_no, group_no, sizeof(req_request_info->group_no)-1);
//    req_request_info->request_id = request_id;
//    req_request_info->req_mode = mode;
//    
//    unsigned char mask=0x00;
//    if('1'==first_flag) mask |=(1<<MSK_FIRST_BIT);
//    if('1'==next_flag)  mask |=(1<<MSK_NEXT_BIT);
//    if('1'==push_flag)  mask |=(1<<MSK_PUSH_BIT);
//    
//    req_request_info->mask=mask;
//    return 0;
//}
//
///** 
// * @brief 业务请求报文头: 添加业务数据
// * @param
// *    [out]buff:缓存指针
// *    [in]buffsize:缓存大小
// * @retval
// *    0-成功; <0-失败;
// */
//int lmasm_req_request_append_data(char *data_ptr, size_t data_len, char *buff, size_t buffsize)
//{
//    if( (NULL==buff) ||(buffsize<sizeof(ST_MSG_HEAD)+sizeof(MSG_REQ_REQUEST)) )
//        return -1;
//
//    ST_MSG_HEAD *head_ptr=(ST_MSG_HEAD)buff;
//    MSG_REQ_REQUEST *req_request_info=(MSG_REQ_REQUEST*)(buff+sizeof(ST_MSG_HEAD));
//    if(buffsize<sizeof(ST_MSG_HEAD)+head_ptr->data_len+data_len)//缓存长度不足
//        return -2;
//        
//    char *ptr=buff+sizeof(ST_MSG_HEAD)+head_ptr->data_len;
//    memcpy(ptr, data_ptr, data_len);
//    head_ptr->data_len += data_len;
//
//    return 0;
//}
////---------------------------------------------------------------------------------------------------------------
///** 
// * @brief 业务请求确认包: 初始化
// * @param
// *    [out]buff:缓存指针
// *    [in]buffsize:缓存大小
// * @retval
// *    0-成功; <0-失败;
// */
//int lmasm_ack_request_init(char *buff, size_t buffsize)
//{
//    if( (NULL==buff)||(buffsize<sizeof(ST_MSG_HEAD)+sizeof(MSG_ACK_REQUEST)) )
//        return -1;
//    memset(buff, 0, sizeof(ST_MSG_HEAD));
//    ST_MSG_HEAD *head_ptr=(ST_MSG_HEAD)buff;
//    //设置报文头
//    head_ptr->msgid=MSGTYPE_ACK_REQUEST;
//    head_ptr->data_len = 0;
//    
//    //设置请求数据说明
//    MSG_ACK_REQUEST *ack_request_info=(MSG_ACK_REQUEST*)(buff+sizeof(ST_MSG_HEAD));
//    memset(ack_request_info, 0, sizeof(MSG_ACK_REQUEST);
//    return 0;
//}
//
///** 
// * @brief 业务请求确认报: 设置连接信息
// * @param
// *    [out]buff:缓存指针
// *    [in]buffsize:缓存大小
// * @retval
// *    0-成功; <0-失败;
// */
//int lmasm_ack_request_set_linkinfo(const char *group_no, int bu_no, int bcc_id,  char if_succ, const char *szmsg, char *buff, size_t buffsize)
//{
//    if( (NULL==buff)||(buffsize<sizeof(ST_MSG_HEAD)+sizeof(MSG_ACK_REQUEST)) )
//        return -1;
//    //ST_MSG_HEAD *head_ptr=(ST_MSG_HEAD)buff;
//    MSG_ACK_REQUEST *ack_request_info=(MSG_ACK_REQUEST*)(buff+sizeof(ST_MSG_HEAD));
//    strncpy(ack_request_info->group_no, group_no, sizeof(ack_request_info->group_no)-1);
//    ack_request_info->bu_no = bu_no;
//    ack_request_info->bcc_id = bcc_id;
//	
//	ack_request_info->if_succ=if_succ;
//	strncpy(ack_request_info->szmsg, szmsg, sizeof(ack_request_info->szmsg)-1);
//    
//    return 0;
//}
//
////---------------------------------------------------------------------------------------------------------------
///** 
// * @brief 业务请求应答报: 初始化
// * @param
// *    [out]buff:缓存指针
// *    [in]buffsize:缓存大小
// * @retval
// *    0-成功; <0-失败;
// */
//int lmasm_req_rsp_init(char *buff, size_t buffsize)
//{
//    if( (NULL==buff)||(buffsize<sizeof(ST_MSG_HEAD)+sizeof(MSG_REQ_RSP)) )
//        return -1;
//    memset(buff, 0, sizeof(ST_MSG_HEAD));
//    ST_MSG_HEAD *head_ptr=(ST_MSG_HEAD)buff;
//    //设置报文头
//    head_ptr->msgid=MSGTYPE_REQ_ESP;
//    head_ptr->data_len = sizeof(MSG_REQ_RSP);
//    
//    //设置请求数据说明
//    MSG_REQ_RSP *ack_request_info=(MSG_REQ_RSP*)(buff+sizeof(ST_MSG_HEAD));
//    memset(ack_request_info, 0, sizeof(MSG_REQ_RSP);
//    return 0;
//}
//
///** 
// * @brief 业务请求应答报: 设置连接信息
// * @param
// *    [out]buff:缓存指针
// *    [in]buffsize:缓存大小
// * @retval
// *    0-成功; <0-失败;
// */
//int lmasm_req_rsp_set_linkinfo(const char *group_no, int bu_no, int bcc_id,  char if_succ, const char *szmsg, char *buff, size_t buffsize)
//{
//    if( (NULL==buff)||(buffsize<sizeof(ST_MSG_HEAD)+sizeof(MSG_REQ_RSP)) )
//        return -1;
//    //ST_MSG_HEAD *head_ptr=(ST_MSG_HEAD)buff;
//    MSG_REQ_RSP *rsp_request_info=(MSG_REQ_RSP*)(buff+sizeof(ST_MSG_HEAD));
//    strncpy(rsp_request_info->group_no, group_no, sizeof(rsp_request_info->group_no)-1);
//    rsp_request_info->bu_no = bu_no;
//    rsp_request_info->bcc_id = bcc_id;
//	
//	rsp_request_info->if_succ=if_succ;
//	strncpy(rsp_request_info->szmsg, szmsg, sizeof(rsp_request_info->szmsg)-1);
//    
//    return 0;
//}
//
///** 
// * @brief 业务请求应答报: 制标记
// * @param
// *    [out]buff:缓存指针
// *    [in]buffsize:缓存大小
// * @retval
// *    0-成功; <0-失败;
// */
//int lmasm_req_rsp_set_linkinfo(const char *group_no, int bu_no, int bcc_id,  char if_succ, const char *szmsg, char *buff, size_t buffsize)
//{
//    if( (NULL==buff)||(buffsize<sizeof(ST_MSG_HEAD)+sizeof(MSG_REQ_RSP)) )
//        return -1;
//    //ST_MSG_HEAD *head_ptr=(ST_MSG_HEAD)buff;
//    MSG_REQ_RSP *rsp_request_info=(MSG_REQ_RSP*)(buff+sizeof(ST_MSG_HEAD));
//    strncpy(rsp_request_info->group_no, group_no, sizeof(rsp_request_info->group_no)-1);
//    rsp_request_info->request_id = request_id;
//    rsp_request_info->req_mode = mode;
//    
//    unsigned char mask=0x00;
//    if('1'==first_flag) mask |=(1<<MSK_FIRST_BIT);
//    if('1'==next_flag)  mask |=(1<<MSK_NEXT_BIT);
//    if('1'==push_flag)  mask |=(1<<MSK_PUSH_BIT);
//    
//    rsp_request_info->mask=mask;
//    return 0;
//}
//
///** 
// * @brief 业务请求应答报文: 添加业务数据
// * @param
// *    [out]buff:缓存指针
// *    [in]buffsize:缓存大小
// * @retval
// *    0-成功; <0-失败;
// */
//int lmasm_req_rsp_append_data(char *data_ptr, size_t data_len, char *buff, size_t buffsize)
//{
//    if( (NULL==buff) ||(buffsize<sizeof(ST_MSG_HEAD)+sizeof(MSG_REQ_RSP)) )
//        return -1;
//
//    ST_MSG_HEAD *head_ptr=(ST_MSG_HEAD)buff;
//
//    if(buffsize<sizeof(ST_MSG_HEAD)+head_ptr->data_len+data_len)//缓存长度不足
//        return -2;
//        
//    char *ptr=buff+sizeof(ST_MSG_HEAD)+head_ptr->data_len;
//    memcpy(ptr, data_ptr, data_len);
//    head_ptr->data_len += data_len;
//
//    return 0;
//}
////----------------------------------------------------------------------------------------------------------------
///** 
// * @brief 业务请求-应答确认报文: 初始化
// * @param
// *    [out]buff:缓存指针
// *    [in]buffsize:缓存大小
// * @retval
// *    0-成功; <0-失败;
// */
//int lmasm_ack_rsp_init(char *buff, size_t buffsize)
//{
//    if( (NULL==buff)||(buffsize<sizeof(ST_MSG_HEAD)+sizeof(MSG_ACK_RSP)) )
//        return -1;
//    memset(buff, 0, sizeof(ST_MSG_HEAD));
//    ST_MSG_HEAD *head_ptr=(ST_MSG_HEAD)buff;
//    //设置报文头
//    head_ptr->msgid=MSGTYPE_REQ_ESP;
//    head_ptr->data_len = sizeof(MSG_ACK_RSP);
//    
//    //设置请求数据说明
//    MSG_ACK_RSP *ack_request_info=(MSG_ACK_RSP*)(buff+sizeof(MSG_ACK_RSP));
//    memset(ack_request_info, 0, sizeof(MSG_ACK_RSP);
//    return 0;
//}
//
///** 
// * @brief 业务请求-应答确认报文: 连接信息
// * @param
// *    [out]buff:缓存指针
// *    [in]buffsize:缓存大小
// * @retval
// *    0-成功; <0-失败;
// */
//int lmasm_ack_rsp_set_linkinfo(const char *group_no, int bu_no, int bcc_id,  char if_succ, const char *szmsg, char *buff, size_t buffsize)
//{
//    if( (NULL==buff)||(buffsize<sizeof(ST_MSG_HEAD)+sizeof(MSG_ACK_RSP)) )
//        return -1;
//    //ST_MSG_HEAD *head_ptr=(ST_MSG_HEAD)buff;
//    MSG_ACK_RSP *ack_info=(MSG_ACK_RSP*)(buff+sizeof(ST_MSG_HEAD));
//    strncpy(ack_info->group_no, group_no, sizeof(ack_info->group_no)-1);
//    ack_info->bu_no = bu_no;
//    ack_info->bcc_id = bcc_id;
//	
//	ack_info->if_succ=if_succ;
//	strncpy(ack_info->szmsg, szmsg, sizeof(ack_info->szmsg)-1);
//    
//    return 0;
//}
////---------------------------------------------------------------------------------------------------------------
///** 
// * @brief 数据传送报文: 初始化
// * @param
// *    [out]buff:缓存指针
// *    [in]buffsize:缓存大小
// * @retval
// *    0-成功; <0-失败;
// */
//int lmasm_req_transfer_init(char *buff, size_t buffsize)
//{
//    if( (NULL==buff)||(buffsize<sizeof(ST_MSG_HEAD)+sizeof(MSG_REQ_TRANSFER)) )
//        return -1;
//    memset(buff, 0, sizeof(ST_MSG_HEAD));
//    ST_MSG_HEAD *head_ptr=(ST_MSG_HEAD)buff;
//    //设置报文头
//    head_ptr->msgid=MSGTYPE_REQ_TRANSFER;
//    head_ptr->data_len = sizeof(MSG_REQ_TRANSFER);
//    
//    //设置请求数据说明
//    MSG_REQ_TRANSFER *req_info=(MSG_REQ_TRANSFER*)(buff+sizeof(ST_MSG_HEAD));
//    memset(req_info, 0, sizeof(MSG_REQ_TRANSFER);
//    return 0;
//}
//
///** 
// * @brief 数据传送报文: 设置连接信息
// * @param
// *    [out]buff:缓存指针
// *    [in]buffsize:缓存大小
// * @retval
// *    0-成功; <0-失败;
// */
//int lmasm_req_transfer_set_linkinfo(const char *group_no, int bu_no, int bcc_id,  char *buff, size_t buffsize)
//{
//    if( (NULL==buff)||(buffsize<sizeof(ST_MSG_HEAD)+sizeof(MSG_REQ_TRANSFER)) )
//        return -1;
//    //ST_MSG_HEAD *head_ptr=(ST_MSG_HEAD)buff;
//    MSG_REQ_TRANSFER *req_info=(MSG_REQ_TRANSFER*)(buff+sizeof(ST_MSG_HEAD));
//    strncpy(req_info->group_no, group_no, sizeof(rsp_request_info->group_no)-1);
//    req_info->bu_no = bu_no;
//    req_info->bcc_id = bcc_id;
//    
//    return 0;
//}
//
///** 
// * @brief 数据传送报文: 设置控制标记
// * @param
// *    [out]buff:缓存指针
// *    [in]buffsize:缓存大小
// * @retval
// *    0-成功; <0-失败;
// */
//int lmasm_req_transfer_set_ctrlinfo(unsigned int request_id, unsigned char mode, unsigned char first_flag=0, unsigned char next_flag=0, unsigned char push_flag=0, char *buff, size_t buffsize)
//{
//    if( (NULL==buff)||(buffsize<sizeof(ST_MSG_HEAD)+sizeof(MSG_REQ_TRANSFER)) )
//        return -1;
//    //ST_MSG_HEAD *head_ptr=(ST_MSG_HEAD)buff;
//    MSG_REQ_TRANSFER *req_info=(MSG_REQ_TRANSFER*)(buff+sizeof(ST_MSG_HEAD));
//    strncpy(req_info->group_no, group_no, sizeof(req_info->group_no)-1);
//    req_info->request_id = request_id;
//    req_info->req_mode = mode;
//    
//    unsigned char mask=0x00;
//    if('1'==first_flag) mask |=(1<<MSK_FIRST_BIT);
//    if('1'==next_flag)  mask |=(1<<MSK_NEXT_BIT);
//    if('1'==push_flag)  mask |=(1<<MSK_PUSH_BIT);
//    
//    req_info->mask=mask;
//    return 0;
//}
//
///** 
// * @brief 数据传送报文: 增加业务数据
// * @param
// *    [out]buff:缓存指针
// *    [in]buffsize:缓存大小
// * @retval
// *    0-成功; <0-失败;
// */
//int lmasm_req_transfer_append_data(char *data_ptr, size_t data_len, char *buff, size_t buffsize)
//{
//    if( (NULL==buff) ||(buffsize<sizeof(ST_MSG_HEAD)+sizeof(MSG_REQ_RSP)) )
//        return -1;
//
//    ST_MSG_HEAD *head_ptr=(ST_MSG_HEAD)buff;
//
//    if(buffsize<sizeof(ST_MSG_HEAD)+head_ptr->data_len+data_len)//缓存长度不足
//        return -2;
//        
//    char *ptr=buff+sizeof(ST_MSG_HEAD)+head_ptr->data_len;
//    memcpy(ptr, data_ptr, data_len);
//    head_ptr->data_len += data_len;
//
//    return 0;
//}
//
////---------------------------------------------------------------------------------------------------------
///** 
// * @brief 数据传送报文确认:初始化
// * @param
// *    [out]buff:缓存指针
// *    [in]buffsize:缓存大小
// * @retval
// *    0-成功; <0-失败;
// */
//int lmasm_ack_transfer_init(char *buff, size_t buffsize)
//{
//    if( (NULL==buff)||(buffsize<sizeof(ST_MSG_HEAD)+sizeof(MSG_ACK_TRANSFER)) )
//        return -1;
//    memset(buff, 0, sizeof(ST_MSG_HEAD));
//    ST_MSG_HEAD *head_ptr=(ST_MSG_HEAD)buff;
//    //设置报文头
//    head_ptr->msgid=MSGTYPE_REQ_ESP;
//    head_ptr->data_len = sizeof(MSG_ACK_TRANSFER);
//    
//    //设置请求数据说明
//    MSG_ACK_TRANSFER *ack_info=(MSG_ACK_TRANSFER*)(buff+sizeof(MSG_ACK_TRANSFER));
//    memset(ack_info, 0, sizeof(MSG_ACK_TRANSFER);
//    return 0;
//}
//
///** 
// * @brief 数据传送报文确认: 设置连接信息
// * @param
// *    [out]buff:缓存指针
// *    [in]buffsize:缓存大小
// * @retval
// *    0-成功; <0-失败;
// */
//int lmasm_ack_transfer_set_linkinfo(const char *group_no, int bu_no, int bcc_id,  char if_succ, const char *szmsg, char *buff, size_t buffsize)
//{
//    if( (NULL==buff)||(buffsize<sizeof(ST_MSG_HEAD)+sizeof(MSG_ACK_TRANSFER)) )
//        return -1;
//    //ST_MSG_HEAD *head_ptr=(ST_MSG_HEAD)buff;
//    MSG_ACK_TRANSFER *ack_info=(MSG_ACK_TRANSFER*)(buff+sizeof(ST_MSG_HEAD));
//    strncpy(ack_info->group_no, group_no, sizeof(ack_info->group_no)-1);
//    ack_info->bu_no = bu_no;
//    ack_info->bcc_id = bcc_id;
//	
//	ack_info->if_succ=if_succ;
//	strncpy(ack_info->szmsg, szmsg, sizeof(ack_info->szmsg)-1);
//    
//    return 0;
//}
////----------------------------------------------------------------------------------------------------------------------------------------------
///** 
// * @brief 推送报文:初始化
// * @param
// *    [out]buff:缓存指针
// *    [in]buffsize:缓存大小
// * @retval
// *    0-成功; <0-失败;
// */
//int lmasm_req_push_init(char *buff, size_t buffsize)
//{
//    if( (NULL==buff)||(buffsize<sizeof(ST_MSG_HEAD)+sizeof(MSG_REQ_PUSH)) )
//        return -1;
//    memset(buff, 0, sizeof(ST_MSG_HEAD));
//    ST_MSG_HEAD *head_ptr=(ST_MSG_HEAD)buff;
//    //设置报文头
//    head_ptr->msgid=MSGTYPE_req_push;
//    head_ptr->data_len = sizeof(MSG_REQ_PUSH);
//    
//    //设置请求数据说明
//    MSG_REQ_PUSH *req_info=(MSG_REQ_PUSH*)(buff+sizeof(ST_MSG_HEAD));
//    memset(req_info, 0, sizeof(MSG_REQ_PUSH);
//    return 0;
//}
//
///** 
// * @brief 推送报文:设置连接信息
// * @param
// *    [out]buff:缓存指针
// *    [in]buffsize:缓存大小
// * @retval
// *    0-成功; <0-失败;
// */
//int lmasm_req_push_set_linkinfo(const char *group_no, int bu_no, int bcc_id,  char *buff, size_t buffsize)
//{
//    if( (NULL==buff)||(buffsize<sizeof(ST_MSG_HEAD)+sizeof(MSG_REQ_PUSH)) )
//        return -1;
//    //ST_MSG_HEAD *head_ptr=(ST_MSG_HEAD)buff;
//    MSG_REQ_PUSH *req_info=(MSG_REQ_PUSH*)(buff+sizeof(ST_MSG_HEAD));
//    strncpy(req_info->group_no, group_no, sizeof(rsp_request_info->group_no)-1);
//    req_info->bu_no = bu_no;
//    req_info->bcc_id = bcc_id;
//    
//    return 0;
//}
//
///** 
// * @brief 推送报文:设置控制信息
// * @param
// *    [out]buff:缓存指针
// *    [in]buffsize:缓存大小
// * @retval
// *    0-成功; <0-失败;
// */
//int lmasm_req_push_set_ctrlinfo(unsigned int request_id, unsigned char mode, unsigned char first_flag=0, unsigned char next_flag=0, unsigned char push_flag=0, char *buff, size_t buffsize)
//{
//    if( (NULL==buff)||(buffsize<sizeof(ST_MSG_HEAD)+sizeof(MSG_REQ_PUSH)) )
//        return -1;
//    //ST_MSG_HEAD *head_ptr=(ST_MSG_HEAD)buff;
//    MSG_REQ_PUSH *req_info=(MSG_REQ_PUSH*)(buff+sizeof(ST_MSG_HEAD));
//    strncpy(req_info->group_no, group_no, sizeof(req_info->group_no)-1);
//    req_info->request_id = request_id;
//    req_info->req_mode = mode;
//    
//    unsigned char mask=0x00;
//    if('1'==first_flag) mask |=(1<<MSK_FIRST_BIT);
//    if('1'==next_flag)  mask |=(1<<MSK_NEXT_BIT);
//    if('1'==push_flag)  mask |=(1<<MSK_PUSH_BIT);
//    
//    req_info->mask=mask;
//    return 0;
//}
//
///** 
// * @brief 推送报文: 增加业务数据
// * @param
// *    [out]buff:缓存指针
// *    [in]buffsize:缓存大小
// * @retval
// *    0-成功; <0-失败;
// */
//int lmasm_req_push_append_data(char *data_ptr, size_t data_len, char *buff, size_t buffsize)
//{
//    if( (NULL==buff) ||(buffsize<sizeof(ST_MSG_HEAD)+sizeof(MSG_REQ_RSP)) )
//        return -1;
//
//    ST_MSG_HEAD *head_ptr=(ST_MSG_HEAD)buff;
//
//    if(buffsize<sizeof(ST_MSG_HEAD)+head_ptr->data_len+data_len)//缓存长度不足
//        return -2;
//        
//    char *ptr=buff+sizeof(ST_MSG_HEAD)+head_ptr->data_len;
//    memcpy(ptr, data_ptr, data_len);
//    head_ptr->data_len += data_len;
//
//    return 0;
//}
//
////---------------------------------------------------------------------------------------------------------
///** 
// * @brief 推送确认包: 初始化
// * @param
// *    [out]buff:缓存指针
// *    [in]buffsize:缓存大小
// * @retval
// *    0-成功; <0-失败;
// */
//int lmasm_ack_push_init(char *buff, size_t buffsize)
//{
//    if( (NULL==buff)||(buffsize<sizeof(ST_MSG_HEAD)+sizeof(MSG_ACK_PUSH)) )
//        return -1;
//    memset(buff, 0, sizeof(ST_MSG_HEAD));
//    ST_MSG_HEAD *head_ptr=(ST_MSG_HEAD)buff;
//    //设置报文头
//    head_ptr->msgid=MSGTYPE_REQ_ESP;
//    head_ptr->data_len = sizeof(MSG_ACK_PUSH);
//    
//    //设置请求数据说明
//    MSG_ACK_PUSH *ack_info=(MSG_ACK_PUSH*)(buff+sizeof(MSG_ACK_PUSH));
//    memset(ack_info, 0, sizeof(MSG_ACK_PUSH);
//    return 0;
//}
//
///** 
// * @brief 推送确认包: 设置连接信息
// * @param
// *    [out]buff:缓存指针
// *    [in]buffsize:缓存大小
// * @retval
// *    0-成功; <0-失败;
// */
//int lmasm_ack_push_set_linkinfo(const char *group_no, int bu_no, int bcc_id,  char if_succ, const char *szmsg, char *buff, size_t buffsize)
//{
//    if( (NULL==buff)||(buffsize<sizeof(ST_MSG_HEAD)+sizeof(MSG_ACK_PUSH)) )
//        return -1;
//    //ST_MSG_HEAD *head_ptr=(ST_MSG_HEAD)buff;
//    MSG_ACK_PUSH *ack_info=(MSG_ACK_PUSH*)(buff+sizeof(ST_MSG_HEAD));
//    strncpy(ack_info->group_no, group_no, sizeof(ack_info->group_no)-1);
//    ack_info->bu_no = bu_no;
//    ack_info->bcc_id = bcc_id;
//	
//	ack_info->if_succ=if_succ;
//	strncpy(ack_info->szmsg, szmsg, sizeof(ack_info->szmsg)-1);
//    
//    return 0;
//}
////-------------------------------------------------------------------------------------------------------------------------------------------
///** 
// * @brief 转发请求: 初始化
// * @param
// *    [out]buff:缓存指针
// *    [in]buffsize:缓存大小
// * @retval
// *    0-成功; <0-失败;
// */
//int lmasm_req_forward_init(char *buff, size_t buffsize)
//{
//    if( (NULL==buff)||(buffsize<sizeof(ST_MSG_HEAD)+sizeof(MSG_REQ_FORWARD)) )
//        return -1;
//    memset(buff, 0, sizeof(ST_MSG_HEAD));
//    ST_MSG_HEAD *head_ptr=(ST_MSG_HEAD)buff;
//    //设置报文头
//    head_ptr->msgid=MSGTYPE_REQ_FORWARD;
//    head_ptr->data_len = sizeof(MSG_REQ_FORWARD);
//    
//    //设置请求数据说明
//    MSG_REQ_FORWARD *req_info=(MSG_REQ_FORWARD*)(buff+sizeof(ST_MSG_HEAD));
//    memset(req_info, 0, sizeof(MSG_REQ_FORWARD);
//    return 0;
//}
//
///** 
// * @brief 转发请求: 设置连接信息
// * @param
// *    [out]buff:缓存指针
// *    [in]buffsize:缓存大小
// * @retval
// *    0-成功; <0-失败;
// */
//int lmasm_req_forward_set_linkinfo(const char *group_no, int bu_no, int bcc_id,  char *buff, size_t buffsize)
//{
//    if( (NULL==buff)||(buffsize<sizeof(ST_MSG_HEAD)+sizeof(MSG_REQ_FORWARD)) )
//        return -1;
//    //ST_MSG_HEAD *head_ptr=(ST_MSG_HEAD)buff;
//    MSG_REQ_FORWARD *req_info=(MSG_REQ_FORWARD*)(buff+sizeof(ST_MSG_HEAD));
//    strncpy(req_info->group_no, group_no, sizeof(rsp_request_info->group_no)-1);
//    req_info->bu_no = bu_no;
//    req_info->bcc_id = bcc_id;
//    
//    return 0;
//}
//
///** 
// * @brief 转发请求: 设置控制信息
// * @param
// *    [out]buff:缓存指针
// *    [in]buffsize:缓存大小
// * @retval
// *    0-成功; <0-失败;
// */
//int lmasm_req_forward_set_ctrlinfo(unsigned int request_id, unsigned char mode, unsigned char first_flag=0, unsigned char next_flag=0, unsigned char push_flag=0, char *buff, size_t buffsize)
//{
//    if( (NULL==buff)||(buffsize<sizeof(ST_MSG_HEAD)+sizeof(MSG_REQ_FORWARD)) )
//        return -1;
//    //ST_MSG_HEAD *head_ptr=(ST_MSG_HEAD)buff;
//    MSG_REQ_FORWARD *req_info=(MSG_REQ_FORWARD*)(buff+sizeof(ST_MSG_HEAD));
//    strncpy(req_info->group_no, group_no, sizeof(req_info->group_no)-1);
//    req_info->request_id = request_id;
//    req_info->req_mode = mode;
//    
//    unsigned char mask=0x00;
//    if('1'==first_flag) mask |=(1<<MSK_FIRST_BIT);
//    if('1'==next_flag)  mask |=(1<<MSK_NEXT_BIT);
//    if('1'==push_flag)  mask |=(1<<MSK_PUSH_BIT);
//    
//    req_info->mask=mask;
//    return 0;
//}
//
///** 
// * @brief 转发请求: 增加业务数据
// * @param
// *    [out]buff:缓存指针
// *    [in]buffsize:缓存大小
// * @retval
// *    0-成功; <0-失败;
// */
//int lmasm_req_forward_append_data(char *data_ptr, size_t data_len, char *buff, size_t buffsize)
//{
//    if( (NULL==buff) ||(buffsize<sizeof(ST_MSG_HEAD)+sizeof(MSG_REQ_RSP)) )
//        return -1;
//
//    ST_MSG_HEAD *head_ptr=(ST_MSG_HEAD)buff;
//
//    if(buffsize<sizeof(ST_MSG_HEAD)+head_ptr->data_len+data_len)//缓存长度不足
//        return -2;
//        
//    char *ptr=buff+sizeof(ST_MSG_HEAD)+head_ptr->data_len;
//    memcpy(ptr, data_ptr, data_len);
//    head_ptr->data_len += data_len;
//
//    return 0;
//}
//
////---------------------------------------------------------------------------------------------------------
///** 
// * @brief 转发确认包: 初始化
// * @param
// *    [out]buff:缓存指针
// *    [in]buffsize:缓存大小
// * @retval
// *    0-成功; <0-失败;
// */
//int lmasm_ack_forward_init(char *buff, size_t buffsize)
//{
//    if( (NULL==buff)||(buffsize<sizeof(ST_MSG_HEAD)+sizeof(MSG_ACK_PUSH)) )
//        return -1;
//    memset(buff, 0, sizeof(ST_MSG_HEAD));
//    ST_MSG_HEAD *head_ptr=(ST_MSG_HEAD)buff;
//    //设置报文头
//    head_ptr->msgid=MSGTYPE_REQ_FORWARD;
//    head_ptr->data_len = sizeof(MSG_ACK_PUSH);
//    
//    //设置请求数据说明
//    MSG_ACK_PUSH *ack_info=(MSG_ACK_PUSH*)(buff+sizeof(MSG_ACK_PUSH));
//    memset(ack_info, 0, sizeof(MSG_ACK_PUSH);
//    return 0;
//}
//
///** 
// * @brief 转发确认包: 设置连接信息
// * @param
// *    [out]buff:缓存指针
// *    [in]buffsize:缓存大小
// * @retval
// *    0-成功; <0-失败;
// */
//int lmasm_ack_forward_set_linkinfo(const char *group_no, int bu_no, int bcc_id,  char if_succ, const char *szmsg, char *buff, size_t buffsize)
//{
//    if( (NULL==buff)||(buffsize<sizeof(ST_MSG_HEAD)+sizeof(MSG_ACK_PUSH)) )
//        return -1;
//    //ST_MSG_HEAD *head_ptr=(ST_MSG_HEAD)buff;
//    MSG_ACK_PUSH *ack_info=(MSG_ACK_PUSH*)(buff+sizeof(ST_MSG_HEAD));
//    strncpy(ack_info->group_no, group_no, sizeof(ack_info->group_no)-1);
//    ack_info->bu_no = bu_no;
//    ack_info->bcc_id = bcc_id;
//	
//	ack_info->if_succ=if_succ;
//	strncpy(ack_info->szmsg, szmsg, sizeof(ack_info->szmsg)-1);
//    
//    return 0;
//}
//
////------------------------------------------------------------------------------------------------------------------------------------
//
int msglink_checkhead(unsigned char *buff, size_t buff_data_len)
{
    return -1;
}

//@brief 校验控制标记
int msglink_check_ccflag(unsigned char *buff, char ccflag_type)
{
    ST_MSG_HEAD *head_ptr=(ST_MSG_HEAD*)buff;
    switch(ccflag_type)
	{
	    case CC_ACK_FLAG: //ack应答标记
		    return (head_ptr->ccflag&(1<<ACK_MASK)) ? 1 : 0; 
		default:
		    return 0;
	    
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//@brief 设置报文: 初始化报文头
int msglink_head_init(ST_MSG_HEAD *msg_head_ptr)
{
	memset(msg_head_ptr, 0, sizeof(ST_MSG_HEAD));
	return 0;
}

int msglink_head_set_msgtypeinfo(ST_MSG_HEAD *msg_head_ptr, unsigned int msgtype, unsigned int mask)
{
	msg_head_ptr->msgid = msgtype;
	msg_head_ptr->mask  = mask;

	return 0;
}

//@brief 设置流水信息
int msglink_head_set_serialno(ST_MSG_HEAD *msg_head_ptr, unsigned int serial_no)
{
	return 0;
}

//@brief 设置报文头crc,crc包含报文长度; 必须在报文数据添加完成后才能创建crc;
int msglink_head_set_crc(ST_MSG_HEAD *msg_head_ptr, unsigned int crc)
{
	msg_head_ptr->head_crc = crc;
	return 0;
}

//@brief 设置链接信息
int msglink_common_set_conninfo(ST_MSG_COMMON *msg_common_ptr, int bcc_id, int bu_no, char *group_no)
{
	msg_common_ptr->bcc_id = bcc_id;
	msg_common_ptr->bu_no  = bu_no;
	strncpy(msg_common_ptr->group_no, group_no, sizeof(msg_common_ptr->group_no));
	return 0;
}

//@biref 设置控制信息
int msglink_common_set_ctrlinfo(ST_MSG_COMMON *msg_common_ptr, bool first_flag/*=true*/, bool next_flag/*=false*/, bool ack_flag/*=false*/, bool push_flag/*=false*/)
{
	unsigned int mask=0x00;
	if(first_flag)
		mask |= (1<<MSK_FIRST_BIT);
	if(next_flag)
		mask |= (1<<MSK_NEXT_BIT);
	if(ack_flag)
		mask |= (1<<MSK_ACK_BIT);
	if(push_flag)
		mask |= (1<<MSK_PUSH_BIT);
	msg_common_ptr->mask = mask;
	return 0;
}

//@brief 设置请求序号 和报文个数
int msglink_common_set_requestinfo(ST_MSG_COMMON *msg_common_ptr, unsigned int request_id, unsigned int num)
{
	msg_common_ptr->request_id = request_id;
	msg_common_ptr->msg_num    = num;
	return 0;
}

//@brief 设置报文: 增加数据
int msglink_pkg_data_append(unsigned char *buff, size_t buffsize, unsigned char *data_ptr, size_t data_len, char *errmsg)
{
    if( (NULL==buff)||(buffsize<sizeof(ST_MSG_HEAD)+sizeof(ST_MSG_COMMON)) )
	{
	    if(NULL!=errmsg) sprintf(errmsg, "msg消息缓存不足");
        return -1;
	}
	//不需要增加数据,直接返回成功即可
	if((NULL==data_ptr) || (data_len<=0))
	    return 0;

    ST_MSG_HEAD *head_ptr=(ST_MSG_HEAD*)buff;

    if(buffsize<sizeof(ST_MSG_HEAD)+head_ptr->data_len+data_len)//缓存长度不足
	{
	    if(NULL!=errmsg) sprintf(errmsg, "msg消息数据区缓存不足");
        return -2;
	}
        
    unsigned char *ptr=buff+sizeof(ST_MSG_HEAD)+head_ptr->data_len;
    memcpy(ptr, data_ptr, data_len);
    head_ptr->data_len += data_len;

    return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//@brief 设置报文: 初始化报文头
int msglink_pkg_head_init(unsigned char *buff, size_t buffsize, unsigned short msgid, char *errmsg)
{
    if( (NULL==buff)||(buffsize<sizeof(ST_MSG_HEAD)+sizeof(ST_MSG_COMMON)) )
	{
	    if(NULL!=errmsg) sprintf(errmsg, "msg消息缓存不足");
        return -1;
	}
    memset(buff, 0, sizeof(ST_MSG_HEAD));
    ST_MSG_HEAD *head_ptr=(ST_MSG_HEAD*)buff;
	//ST_MSG_COMMON *comminfo_ptr=(ST_MSG_COMMON*)(buff+sizeof(ST_MSG_HEAD));
    //设置报文头
    head_ptr->msgid=msgid;
    head_ptr->data_len = sizeof(ST_MSG_COMMON);

    return 0;
}

//@brief 设置报文: 连接信息
int msglink_pkg_conninfo(unsigned char *buff, size_t buffsize, int bcc_id, int bu_no, char *group_no, char *errmsg)
{
    if( (NULL==buff)||(buffsize<sizeof(ST_MSG_HEAD)+sizeof(ST_MSG_COMMON)) )
	{
	    if(NULL!=errmsg) sprintf(errmsg, "msg消息缓存不足");
        return -1;
	}
    memset(buff, 0, sizeof(ST_MSG_HEAD));
    //ST_MSG_HEAD *head_ptr=(ST_MSG_HEAD*)buff;
    ST_MSG_COMMON *comminfo_ptr=(ST_MSG_COMMON*)(buff+sizeof(ST_MSG_HEAD));

    strncpy(comminfo_ptr->group_no, group_no, sizeof(comminfo_ptr->group_no));
    comminfo_ptr->bcc_id=bcc_id;
    comminfo_ptr->bu_no=bu_no;

    return 0;
}

//@brief 设置报文: 控制信息
int msglink_pkg_ctrlinfo(unsigned char *buff, size_t buffsize, bool first_flag, bool next_flag, bool ack_flag, bool push_flag, char *errmsg)
{
    return 0;
}

