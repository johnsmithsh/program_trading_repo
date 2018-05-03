#include "sock_frame.h"
bool check_is_framehead(ST_SOCK_FRAME_HEAD *frame_head)
{
    return false;
}

int build_sock_frame(unsigned char *data_ptr, int data_len, ST_SOCK_FRAME *socket_frame)
{
    if(NULL==data_ptr) return -1;
	if(NULL==socket_frame) return -2;
	//数据多长
	if(data_len>MAX_FRAME_SIZE) return -3;
	
	if(data_len<0) data_len=0;
	
	
	memset(&(socket_frame->frame_head), 0, sizeof(socket_frame->frame_head));
	socket_frame->frame_head.frame_len=data_len;
	memcpy(socket_frame->frame_data, data_ptr, data_len);
	return 0;
}
