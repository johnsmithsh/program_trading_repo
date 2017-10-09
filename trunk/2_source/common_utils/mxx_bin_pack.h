#ifndef __MXX_BIN_PACK_H_
#define __MXX_BIN_PACK_H_
/*
 * 说明: 
 *    为了便于通过socket发送和接收,专门定义此协议包;
 *    该协议包纯粹是为了接收方便,故报文头只包含一个数据包长度,不包含任何业务信息和控制信息;
 *    一个报文中可以包含多个业务数据包; 业务数据包的组装与解析全部由业务层负责,不在此处描述;
 *    由于报文中全是二进制数据,暂时就把这个协议包叫做binary pack吧,缩写BIN_PACK;
 * 报文格式: BIN_HEAD+BIN_BODY=长度+数据;
 * 使用方式:
 *     
 * 注: 该数据包有过多大,不宜申请过多,每个socket连接申请一个到两个即可;
 *     每个ST_BIN_PACK可能只包含一个业务数据包,也可能包含多个;需要业务单元去拆分和组装;
 *     由于socket传输数据比较频繁,考虑到性能,建议后期使用内存池分配;
 * */

//二进制数据包最大长度,即每次socket连接传输的数据不能超过该数值
// 如果业务传输数据超过该数字,则业务层必须自己拆分;
#define BIN_PACK_MAX_SIZE  (10240) //10KB, BIN_PACK最大值
#define BIN_PACK_MIN_SIZE  (1)  //考虑到业务包肯定有报文头,所以数据包肯定至少存在一个报文头,否则没有意义; 暂时设置为32,具体数据以后修改;

typedef unsigned char* BIN_PACK_Ptr; //定义二进制协议包 

//定义二进制报文协议: 报文头;
typedef struct __st_bin_pack_len
{
  unsigned int bin_data_len;//数据长度
  //unsigned char version[4];//版本号
}ST_BIN_DATA_HEAD;

//定义二进制报文协议: 报文: 格式:报文头+变长报文体; 且报文头与报文体保存在连续的内存空间;
//typedef struct __st_bin_pack
//{
//   unsigned char   *bin_pack;//顺序不能颠倒;bin_pack指向的是bin_head的地址;
//   ST_BIN_DATA_HEAD bin_head;//
//}ST_BIN_PACK;

//注:如果该结构上有数据, 绝对不能在该结构体上执行memset或类似的操作;
//
typedef struct __st_socket_pack
{
   int size;//bin_pack缓存大小=BIN_HEAD大小+数据区域大小;(即包括报文头,数据区域大小; 不包含ST_BIN_BUFF的size、len等其他字段)
   int len;//bin_pack已使用内存数=报文头大小+数据区域数据长度;  //(含报文头),用于socket接收校验包完整性
   unsigned char *bin_pack_ptr;//BIN_PACK指针,指向bin_head地址; bin_head与bin_body是连续的, 这使用了一个c语言的小技巧;
   
   //以下才是BIN_PACK真正的缓存区域
   ST_BIN_DATA_HEAD bin_head;//报文头,从此处开始才是真正的二进制协议报文数据;
   //bin_body区域 变长报文数据缓存...
}ST_BIN_BUFF;

//功能: 分配一个ST_BIN_BUFF缓存
//     分配后已经清空缓存;
//参数:
//   [in]data_len: >0;数据长度(仅包括报文长度,不包括bin_head和ST_BIN_BUFF其他字段的长度)
// 注: 返回指针一定不能使用memset清空;
ST_BIN_BUFF *mxx_alloc_bin_pack(int data_len);

//功能: 释放mxx_alloc_bin_pack分配的数据包指针
void mxx_free_bin_pack(ST_BIN_BUFF *bin_buff);

//功能: 清空BIN_PACK;
void mxx_bin_pack_clear(ST_BIN_BUFF *bin_buff);

//功能: 向缓存添加数据
int mxx_bin_pack_append(ST_BIN_BUFF *bin_buff, unsigned char *data_buff, int data_len);

//功能:  获取缓冲区可用长度,不包括报文头
int mxx_bin_pack_get_avail_size(ST_BIN_BUFF *bin_buff);

//功能: 判断缓存区是否已经满了;
bool mxx_bin_pack_is_full(ST_BIN_BUFF *bin_buff);


/////////////////////////////////////////////////////////////////////////////////////////////////

//获取BIN_PACK可存储的数据量,单位:字节; 报文头+报文头体;
int mxx_bin_pack_size(ST_BIN_BUFF *bin_buff);

//从缓存中获取BIN_PACK的数据指针(包含报文头)
BIN_PACK_Ptr mxx_bin_pack_ptr(ST_BIN_BUFF *bin_buff);
//获取BIN_PACK协议包大小,包括报文头
int mxx_bin_pack_len(ST_BIN_BUFF *bin_buff);

//功能: 获取数据指针(即跳过报文头)
unsigned char * mxx_bin_pack_data_ptr(ST_BIN_BUFF *bin_buff);
//获取BIN_PACK协议包中数据长度,不包括报文头
int mxx_bin_pack_datalen(ST_BIN_BUFF *bin_buff);

//////////////////////////////////////////////////////////////////////////////////////////////////
//缓存封装成一个类
//class CBinPackBuffer
//{
//  private:
//    int m_data_len;
//    unsigned char *m_lpBuff;//缓存指针
//  public:
//    CBinPackBuffer(int buff_size);
//    virtual ~CBinPackBuffer();
//
//};

#endif
