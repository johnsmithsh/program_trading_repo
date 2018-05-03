#ifndef __MXX_EPOLL_H_
#define __MXX_EPOLL_H_

#include <sys/epoll.h>


//功能: socket加入到监听队列;
//参数:
///  [in]fd_epoll监听句柄;
//  [in]fd:socket文件描述符;
//  [in]ev: 监听事件
//返回值: 0-成功; <0-失败;
int mxx_epoll_add(int fd_epoll, int fd, struct epoll_event *ev);
//功能: socket加入到监听队列; events-epoll监听事件
int mxx_epoll_add(int fd_epoll, int fd, unsigned int events);

//功能: socket加入到监听队列; events-epoll监听事件
int mxx_epoll_mod(int fd_epoll, int fd, struct epoll_event *ev);
//功能: socket加入到监听队列; events-epoll监听事件
int mxx_epoll_mod(int fd_epoll, int fd, unsigned int events);

/*
 * 功能: 将socket从epool监听队列中删除
 * 参数:
 *    [in]fd_epool: epoll句柄]
 *    [in]fd: socket文件描述符
 * 返回值: 0-成功; <-0失败;
 **/
int mxx_epoll_del(int fd_epoll, int fd);

#endif
