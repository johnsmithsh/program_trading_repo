
#include "mxx_epoll.h"
#include "log.h"
#include <string.h>
#include <errno.h>

//功能: socket加入到监听队列;
//参数:
//  [in]fd_epoll监听句柄;
//  [in]fd:socket文件描述符;
//  [in]ev: 监听事件
//返回值: 0-成功; <0-失败;
int mxx_epoll_add(int fd_epoll, int fd, struct epoll_event *ev) {
  if (fd_epoll < 0 || fd < 0 || ev == NULL) {
    return -1;
  }

  if (epoll_ctl(fd_epoll, EPOLL_CTL_ADD, fd, ev) < 0) {
    //ERROR_MSG("epoll_add failed(epoll_ctl)[fd_epoll:%d,fd:%d][%s]", fd_epoll, fd, strerror(errno));
    return -1;
  }

  //INFO_MSG("epoll_add success[fd_epoll:%d,fd:%d]\n", fd_epoll, fd);
  return 0;
}

int mxx_epoll_add(int fd_epoll, int fd, unsigned int events)
{
   struct epoll_event ev;
   ev.events = events | EPOLLET; //ET监听(ET边缘触发,epoll重新触发丢失信号(直到被处理)); LT监听则不会;
   ev.data.fd = fd;
   return mxx_epoll_add(fd_epoll, fd, &ev);
}

int mxx_epoll_mod(int fd_epoll, int fd, unsigned int events)
{
   struct epoll_event ev;
   ev.events = events | EPOLLET; //ET监听(ET边缘触发,epoll重新触发丢失信号(直到被处理)); LT监听则不会;
   ev.data.fd = fd;
   if (epoll_ctl(fd_epoll, EPOLL_CTL_MOD, fd, &ev) < 0) 
   {
     //ERROR_MSG("epoll_mod failed(epoll_ctl)[fd_epoll:%d,fd:%d][%s]", fd_epoll, fd, strerror(errno));
     return -1;
   }
  return 0;
}

/*
 * 功能: 将socket从epool监听队列中删除
 * 参数:
 *    [in]fd_epool: epoll句柄]
 *    [in]fd: socket文件描述符
 * 返回值: 0-成功; <-0失败;
 */
int mxx_epoll_del(int fd_epoll, int fd) {
  if (fd_epoll < 0 || fd < 0) {
    return -1;
  }

  struct epoll_event ev_del;
  if (epoll_ctl(fd_epoll, EPOLL_CTL_DEL, fd, &ev_del) < 0) {
    //ERROR_MSG("epoll_del failed(epoll_ctl)[fd_epoll:%d,fd:%d][%s]\n", fd_epoll, fd, strerror(errno));
    return -1;
  }
  //close(fd);
  //INFO_MSG("epoll_del success[epoll_fd:%d,fd:%d]\n", fd_epoll, fd);
  return 0;
}
