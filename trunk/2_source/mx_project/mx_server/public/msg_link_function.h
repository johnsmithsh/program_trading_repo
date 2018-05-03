#ifndef _MXX_MSG_LINK_FUNCTION_H_
#define _MXX_MSG_LINK_FUNCTION_H_

#define MSG_LNK_ERR_INVALID_BUFF -1 //!< 无效缓存

// lmasm: link msg assemble
/** 
 * @brief 构建建立连接报文
 * @param
 *    [in]group_no: 业务组号
 *    [in]group_desc: 业务组描述
 *    [in]group_version: 业务组版本
 *
 *    [out]buff:缓存指针
 *    [in]buffsize:缓存大小
 * @retval
 *    0-成功; <0-失败;
 */
int lmasm_req_conn(const char *group_no, const char *group_desc, const char *group_version, int pid, char *buff, size_t buffsize);

/** 
 * @brief 构建建立连接应答报文
 * @param
 *    [in]bu_no: 控制中心为业务服务分配的id;每个服务进程都不相同;
 *    [in]bcc_id: 本业务中心id
 *    [in]if_succ/szmsg: 成功标记及说明信息
 *
 *    [out]buff:缓存指针
 *    [in]buffsize:缓存大小
 * @retval
 *    0-成功; <0-失败;
 */
int lmasm_ans_conn(int bu_no, int bcc_id, char if_succ, const char *szmsg, char *buff, size_t buffsize);

/** 
 * @brief 构建注册业务功能报文初始化
 * @param
 *    [out]buff:缓存指针
 *    [in]buffsize:缓存大小
 * @retval
 *    0-成功; <0-失败;
 */
int lmasm_req_regfunc_init(char *buff, size_t buffsize);

/** 
 * @brief 注册业务功能报文 增加业务功能信息
 * @param
 *    [in]func_id,func_desc: 业务id与业务描述;
 *    [in]func_type:业务类型
 *    [out]buff:缓存指针
 *    [in]buffsize:缓存大小
 * @retval
 *    0-成功; <0-失败;
 * @note
 *    调用该函数前必须先初始化报文头
 */
int lmasm_req_regfunc_funcinfo_append(const char *func_id, const char *func_desc, char func_type, char *buff, size_t buffsize);

/** 
 * @brief 注册业务功能应答报文
 * @param
 *    [in]if_succ/szmsg: 成功标记及说明信息
 *
 *    [out]buff:缓存指针
 *    [in]buffsize:缓存大小
 * @retval
 *    0-成功; <0-失败;
 */
int lmasm_ans_regfunc(char if_succ, const char *szmsg, char *buff, size_t buffsize);
//-----------------------------------------------
/** 
 * @brief 业务请求报文初始化
 * @param
 *    [out]buff:缓存指针
 *    [in]buffsize:缓存大小
 * @retval
 *    0-成功; <0-失败;
 */
int lmasm_req_request_init(char *buff, size_t buffsize);
int lmasm_req_request_set_linkinfo(const char *group_no, int bu_no, int bcc_id, char *buff, size_t buffsize);
int lmasm_req_request_set_ctrlinfo(unsigned int request_id, unsigned char mode, unsigned char first_flag=0, unsigned char next_flag=0, unsigned char push_flag=0, char *buff, size_t buffsize);
int lmasm_req_request_append_data(char *data_ptr, size_t data_len, char *buff, size_t buffsize);
int lmasm_ack_request_init(char *buff, size_t buffsize);
int lmasm_ack_request_set_linkinfo(const char *group_no, int bu_no, int bcc_id,  char if_succ, const char *szmsg, char *buff, size_t buffsize);

/** 
 * @brief 业务请求应答报: 初始化
 * @param
 *    [out]buff:缓存指针
 *    [in]buffsize:缓存大小
 * @retval
 *    0-成功; <0-失败;
 */
int lmasm_req_rsp_init(char *buff, size_t buffsize);
int lmasm_req_rsp_set_linkinfo(const char *group_no, int bu_no, int bcc_id,  char if_succ, const char *szmsg, char *buff, size_t buffsize);
int lmasm_req_rsp_append_data(char *data_ptr, size_t data_len, char *buff, size_t buffsize);
int lmasm_ack_rsp_init(char *buff, size_t buffsize);
int lmasm_ack_rsp_set_linkinfo(const char *group_no, int bu_no, int bcc_id,  char if_succ, const char *szmsg, char *buff, size_t buffsize);

/** 
 * @brief 数据传送报文: 初始化
 * @param
 *    [out]buff:缓存指针
 *    [in]buffsize:缓存大小
 * @retval
 *    0-成功; <0-失败;
 */
int lmasm_req_transfer_init(char *buff, size_t buffsize);
int lmasm_req_transfer_set_linkinfo(const char *group_no, int bu_no, int bcc_id,  char *buff, size_t buffsize);
int lmasm_req_transfer_set_ctrlinfo(unsigned int request_id, unsigned char mode, unsigned char first_flag=0, unsigned char next_flag=0, unsigned char push_flag=0, char *buff, size_t buffsize);
int lmasm_req_transfer_append_data(char *data_ptr, size_t data_len, char *buff, size_t buffsize);
int lmasm_ack_transfer_init(char *buff, size_t buffsize);
int lmasm_ack_transfer_set_linkinfo(const char *group_no, int bu_no, int bcc_id,  char if_succ, const char *szmsg, char *buff, size_t buffsize);

/** 
 * @brief 推送报文:初始化
 * @param
 *    [out]buff:缓存指针
 *    [in]buffsize:缓存大小
 * @retval
 *    0-成功; <0-失败;
 */
int lmasm_req_push_init(char *buff, size_t buffsize);
int lmasm_req_push_set_linkinfo(const char *group_no, int bu_no, int bcc_id,  char *buff, size_t buffsize);
int lmasm_req_push_set_ctrlinfo(unsigned int request_id, unsigned char mode, unsigned char first_flag=0, unsigned char next_flag=0, unsigned char push_flag=0, char *buff, size_t buffsize);
int lmasm_req_push_append_data(char *data_ptr, size_t data_len, char *buff, size_t buffsize);
int lmasm_ack_push_init(char *buff, size_t buffsize);
int lmasm_ack_push_set_linkinfo(const char *group_no, int bu_no, int bcc_id,  char if_succ, const char *szmsg, char *buff, size_t buffsize);


/** 
 * @brief 转发请求: 初始化
 * @param
 *    [out]buff:缓存指针
 *    [in]buffsize:缓存大小
 * @retval
 *    0-成功; <0-失败;
 */
int lmasm_req_forward_init(char *buff, size_t buffsize);
int lmasm_req_forward_set_linkinfo(const char *group_no, int bu_no, int bcc_id,  char *buff, size_t buffsize);
int lmasm_req_forward_set_ctrlinfo(unsigned int request_id, unsigned char mode, unsigned char first_flag=0, unsigned char next_flag=0, unsigned char push_flag=0, char *buff, size_t buffsize);
int lmasm_req_forward_append_data(char *data_ptr, size_t data_len, char *buff, size_t buffsize);
int lmasm_ack_forward_init(char *buff, size_t buffsize);
int lmasm_ack_forward_set_linkinfo(const char *group_no, int bu_no, int bcc_id,  char if_succ, const char *szmsg, char *buff, size_t buffsize);

//@brief 构建建立连接应答报文
int msglink_checkhead(char *buff, size_t buff_data_len);
int msglink_data_append(char *data_ptr, size_t data_len, char *buf, size_t buffsize);
