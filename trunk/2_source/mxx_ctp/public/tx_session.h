#ifndef _MXX_TX_SESSION_H_
#define _MXX_TX_SESSION_H_
/*
 * 由于交易接口采用api+spi的方式,为了在在api中确定请求方、在程序中跟踪每次请求处理情况;
 * 一次会话包括请求+应答;
 */
#include <deque>

typedef struct __st_tx_data
{
    unsigned char  *data_ptr;//数据指针
    size_t size;    //存储空间大小
    size_t data_len;//数据长度
}ST_TX_DATA;

//定义会话状态
#define TX_SS_INIT   '0'  //初始状态
#define TX_SS_REQING '1'  //正在请求
#define TX_SS_ACKING '2'  //确认: 收到对方的ack信号
#define TX_SS_RSPING '3'  //应答: 收到对方的第一个应答数据
#define TX_SS_FINISH '4'  //完结: 收到对方的最后应答数据
#define TX_SS_ERROR  '5'  //错误: 如超时,发送失败等

//会话模式
//#define SESSN_MODE_REP_RPLY         //请求-应答模式
//#define SESSN_MODE_REP_ACK_RPLY     //请求-确认-应答模式
//#define SESSN_MODE_RELIABLE_PUSH    //(可靠)推送模式
//#define SESSN_MODE_UNRELIABLE_PUSH  //(不可靠)推送模式

//英文中XFE=transfer; session=SESSN; 可命名CXfeSessn
//tx有交易的意思,故此处使用txSession表示一次请求和应答
class CTxSession
{
public:
    CTxSession();
    //CTxSession(void *data_ptr, size_t data_len);
    virtual ~CTxSession();
private:
    CTxSession(CTxSession &obj);
    CTxSession & operator=(CTxSession &obj);
public:
    int set_request_id(int request_id) { m_request_id = request_id; }
    int get_request_id()               { return m_request_id; }
    
    //@brief 增加输入数据
    int add_tx_in(void *data_ptr, size_t data_len);
    void get_tx_in(void *data_ptr, size_t data_size);
    
    //@brief 增加交易输出数据
    int add_tx_out(void *data_ptr, size_t data_len);
    void get_tx_out(void *data_ptr, size_t data_size);
    
    //@brief 设置交易状态
    void set_tx_status(char tx_status) { m_tx_status=tx_status; }
    char get_tx_status()               { return m_tx_status; }
    
    
    //@brief 判断流水号是否匹配
    bool check_request_id(int request_id) { return (m_request_id>0) && (request_id==m_request_id); }
    //@brief 判断状态是否匹配
    bool check_tx_status(char tx_status)  { return (tx_status==m_tx_status); }
    
    //@brief 设置错误码和错误信息
    void set_code_msg(int retCode, char *szMsg);
    int get_retcode()          { return m_retCode; }
    const char * get_szmsg()   { return m_szMsg;   }
private:
    ST_TX_DATA *m_tx_in;//请求数据
    std::deque<ST_TX_DATA *> m_tx_out;//输出数据
    int m_request_id;//请求流水号,就是交易接口API中的nRequestID
    char m_tx_status;//状态
    int m_retCode;//返回码
    char m_szMsg[255];//返回消息
    
    //本来还打算增加回调函数,想想就算了,暂时用不到
};

#endif
