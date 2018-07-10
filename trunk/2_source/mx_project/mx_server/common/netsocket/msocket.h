#ifndef __MXX_MSOCKET_H_
#define __MXX_MSOCKET_H_

#define MS_SUCCESS    0 

#define MS_INVSOCK -1

typedef int SOCKET;

class MSocket
{
  public:
    MSocket():m_so(MS_INVSOCK),m_ownflag(true)
	{  }
	virtual ~MSocket()
	{
	    this->close();
	}
  public:
    int create(int domain=AF_INET, int type=SOCK_STREAM);
	int bind(char *ip, int port);
	int listen(int max_conn=50);
	int accept();
	int accept(struct sockaddr *addr, int &addrlen);
	 
	int connect(const char *ip, int port);
	 
  public:
    int send(unsigned char *buff, int bufflen);
	int recv(unsigned char *buff, int buffsize);
	int send_ex(unsigned char *buff, int bufflen,  int millisec);
	int recv_ex(unsigned char *buff, int buffsize, int millisec);
	int peer(unsigned char *buff,    int buffsize);
  public://绑定socket与解除绑定
    int attach(int socket, int ownflag=false);
    int detach();
  public: //设置socket参数
    //@brief 接收超时时间设置
    int sockopt_set_recvtimeout(int millisecond);
    //@brief 发送超时时间设置
    int sockopt_set_sendtimeout(int millisecond);
    //@brief 通过SO_SNDTIMEO让conn超时
    int sockopt_set_conntimeout(int millisecond);
    
    /**
     * @brief 设置阻塞模式(默认情况下,是阻塞模式);
     * @param
     *     blockmode: true-阻塞模式; false-非阻塞模式;
     **/
    int sockopt_set_block(bool blockmode);
    
    /**
     * @brief
     * @note
     *      SO_REUSEADDR  ???
     *      SO_DONTLINGER ???
     **/
    int sockopt_set_timewaitmode(int timewait_millisecond);
    
    /**
     * @biref 用于设置socket的close行为
     * @param
     *    [in]sockfd:socket描述符
     *    [in]enable: true-启用linger; false-停用linger;
     *    [in]second: 延迟时间,单位:秒; 范围[0, 10]
     * @note
     *  注意：
     *    这个选项需要谨慎使用，尤其是强制式关闭，会丢失服务器发给客户端的最后一部分数据。
     *    UNP中:
     *       The TIME_WAIT state is our friend and is there to help us(i.e., to let the old duplicate segments expire in the network).
     *    使用该选项,程序一定要检查close返回值; close超时返回， 则close返回-1 && errno=EWOULDBLOCK;
     **/
    int sockopt_set_linger(bool enable, int second);
    
    /**
     * @brief keepalive属性
     * @param
     *    [in]sockfd
     *    [in]enable: true-启用keepalive; false-禁用keepalive;
     *    [in]keepIdle:     单位:秒; <0-使用默认值; 如果在指定秒内没有任何数据交互,则进行探测. 缺省值:7200(s)
     *    [in]keepInterval: 单位:秒; <0-使用默认值; 探测时发探测包的时间间隔为5秒. 缺省值:75(s)
     *    [in]keepCount:    单位:次; <0-使用默认值; 探测重试的次数. 全部超时则认定连接失效..缺省值:9(次)
     * @note
     *    仅适用于TCP;
     *    大多数内核是基于整个内核维护该时间参数的,而不是基于每个socket维护;
     *    常用于服务器,用于检测半开连接并关闭;
     **/
    int sockopt_set_keepalive_info(bool enable, int keepIdle=-1, int keepInterval=-1, int keepCount=-1);
  private:
    SOCKET m_so;
	bool m_ownflag;
};
#endif
