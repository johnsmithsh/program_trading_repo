#ifndef __MXX_SELECT_H_
#define __MXX_SELECT_H_

typedef fd_set mxx_fd_set;

//清空所有句柄
int mxx_fd_zero(mxx_fd_set *fdset);
//建立fd与fdset联系
int mxx_fd_set(int sockfd, mxx_fd_set *fdset);
//清除fd与fdset联系
int mxx_fd_clr(int sockfd, mxx_fd_set *fdset);
//检查fdset的文件句柄是否可读写
int mxx_fd_issset(int sockfd, mxx_fd_set *fdset);
#endif
