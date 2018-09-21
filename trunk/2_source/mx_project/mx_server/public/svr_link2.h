#ifndef __MXX_SRV_LINK_H_
#define __MXX_SRV_LINK_H_

int svrlink_create(SVRLINK_HANDLE);
int svrlink_connect(SVRLINK_HANDLE link_handle, char *ip, int port);
int svrlink_disconnect(SVRLINK_HANDLE link_handle);


int svrlink_recv(SVRLINK_HANDLE link_handle, int timeout_millsecond, ST_MSGLINK_BUFF *recv_buff, char *szMsg);

int svrlink_response(SVRLINK_HANDLE link_handle, int timeout_millsecond, ST_MSGLINK_BUFF *recv_buff, char *szMsg);

int svrlink_ack(SVRLINK_HANDLE link_handle, int timeout_millsecond, ST_MSGLINK_BUFF *recv_buff, char *szMsg);

int svrlink_push(SVRLINK_HANDLE link_handle, int timeout_millsecond, ST_MSGLINK_BUFF *recv_buff, char *szMsg);

int svrlink_begin_transfer();

int svrlink_end_transfer();

#endif
