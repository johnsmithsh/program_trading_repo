/********************************************************************************* 
 *Copyright(C), www.com
 *@file:  // 文件名 
 *@author:  //作者 
 *@version:  //版本 
 *@date:  //完成日期 
 *@brief:  
 *    定义从socket收到的数据包格式; 该数据包用于对外联系;
 *@details 
 *Others:  
 *Function List:  
         1.………… 
         2.………… 
 *@note  
 *@history:  
         1.Date: 
           Author: 
           Modification: 
         2.………… 
 **********************************************************************************/  
#ifndef _MXX_SOCK_FRAME_H_
#define _MXX_SOCK_FRAME_H_

//#define FRAME_END_BIT EB557500
#define MAX_FRAME_SIZE (8192) //!< 报文最大长度

//定义报文头
typedef struct __st_sock_frame_head
{
    unsigned int  frame_len;        //!< 报文长度
    unsigned int  serial_no;        //!< 报文序号
    char          frame_version[8]; //!< 协议版本
    unsigned char bit_flag[4];//
    unsigned char head_end_flag[4];
    unsigned int  crc;
}ST_SOCK_FRAME_HEAD;

//定义报文格式:报文头+数据
typedef struct __st_sock_frame
{
    ST_SOCK_FRAME_HEAD frame_head;
    unsigned char      frame_data[MAX_FRAME_SIZE-sizeof(ST_SOCK_FRAME_HEAD)];//!< 数据指针
}ST_SOCK_FRAME;

//@brief 校验报文头释放有效
inline bool framehead_is_valid(ST_SOCK_FRAME_HEAD *frame_head)
{
    if(NULL==frame_head) 
	    return false;
    return true;
}
bool framehead_setversion(ST_SOCK_FRAME_HEAD *frame_head, char *version);

//@brief 构建socket frame数据包
int build_sock_frame(unsigned char *data_ptr, int data_len, ST_SOCK_FRAME *socket_frame);
//int build_sock_frame_vec(unsigned char *data_ptr, int data_len, std::vector<ST_SOCK_FRAME> *socket_frame_vec);

#endif
