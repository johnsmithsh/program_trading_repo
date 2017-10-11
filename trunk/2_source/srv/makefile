GCC:=gcc
#c++编译器
CXX:=gcc
#c编译器
CC:=gcc

#取得操作系统名称#OS_NAME="Linux:SunOS:HP-UX:AIX"
OS_NAME:= $(shell uname -s)

#######c编译选项###############
#c编译器参数,建议-W -Wall必须使用
CFLAGS:= -c -g -W -Wall
#CPP c预编译器名字,默认$(CC) -E
CPP:=
#CPPFLAGS C预编译器选项
CPPFLAGS:=
######c++编译选项#############
#c++编译选项 c++编译器参数
CXXFLAGS:= -c -g -W -Wall
#c++预编译器参数
CXXPPFLAGS:=

#c链接器选项
CLD_FLAGS:= 
#c++链接器选项
CXXLD_FLAGS:= 

SFIX:=.c .C .cpp .cc .CPP .c++ .cp .cxx 
#从指定的路径目录下获取源文件


#头文件路径
INCLUDES:= -I./ -I../    \
 -I../common_utils/      \
 -I../common_utils/log/  \
 -I../common_utils/netsocket \
 -I../common_utils/concurrency \
 -I../net_transfer_protocol 
#动态库路径
LIB_DIR:=-L./
#动态库
SO_LIBS:=-lstdc++ -lpthread -lrt
#静态库
STATIC_LIBS:=

#使用log4cpp记录日志
OpenSrc_log4cpp_flag=1
ifdef OpenSrc_log4cpp_flag
 INCLUDES+= -I../common_utils/log/include
 LIB_DIR += -L../common_utils/log/linux
 SO_LIBS+= -llog4cpp
 #STATIC_LIBS += #没有静态库
endif

#常用文件
COMMON_SOURCES:= ../common_utils/concurrency/itc_mutex.cpp \
                 ../net_transfer_protocol/SocketConnPool.cpp \
                 ../common_utils/mxx_bin_pack.cpp ../common_utils/mxx_socket_event.cpp \
                 ../common_utils/mxx_epoll.cpp \
  ../common_utils/thread_recv.cpp ../common_utils/thread_base.cpp ../common_utils/log/LogFileManage.cpp ../common_utils/netsocket/mxx_net_socket.cpp
COMMON_OBJS:= $(COMMON_SOURCES:.cpp=.o)

#工程文件
PRJ_SOURCES=./src/ctrl_center_thread.cpp ./src/main.cpp
PRJ_OBJS=$(PRJ_SOURCES:.cpp=.o)

SOURCES:=$(COMMON_SOURCES) $(AUTO_SOURCES) $(PRJ_SOURCES)
OBJS:= $(PRJ_OBJS) $(AUTO_OBJS) $(COMMON_OBJS)

#-rpath指定运行时动态库搜索路径,多个目录使用分号分割，如-Wl,-rpath,dir1:dir2:...:dirN
#      $ORIGING表示程序所在目录,如-wl,-rpath $ORIGING
LINK_FLAG=-Wl,-rpath,./:./libs


release_flag=  #release模式/debug模式
ifdef release_flag
 #-O0编译选项不进行优化; -g编译时产生调试信息
 CFLAGS+= -O0 -g
 CXXFLAG+= -O0 -g
 LINK_FLAG+= 
else
 CFLAGS+= -g
 CXXFLAGS+= -O0 -g
 LINK_FLAG+=
endif

TARGET:=bu_demo
#输出路径
OUTPUT_DIR:=./bin
all:$(TARGET)

#构建规则
bu_demo:$(OBJS)
	$(GCC) -o $@ $(LINK_FLAG) $(LIB_DIR) $(SO_LIBS)  $^  
	rm -f ./bin/$@ 
	cp $@ bin/

#$@表示目标; $^表示所有依赖文件; $<表示第一个依赖文件
#模式规则: 
# 模式规则中，至少在规则的目标定义中要包含"%"，否则，就是一般的规则。
# 目标中的"%"定义表示对文件名的匹配，"%"表示长度任意的非空字符串。
# 例如："%.c"表示以".c"结尾的文件名（文件名的长度至少为3），而"s.%.c"则表示以"s."开头，".c"结尾的文件名（文件名的长度至少为5）
# 模式规则语法:  %.o: %.c ; COMMAND...
# 老式模式规则语法: .c.o: <=> %.o: %.c
#静态模式:
#  规则存在多个目标;
#  并且不同的目标可以根据目标文件的名字来自动构造出依赖文件
#  静态模式规则比多目标规则更通用
#静态规则:
#  静态规则语法: 
#     TARGETS ...: TARGET-PATTERN: PREREQ-PATTERNS ...
#        
$(COMMON_OBJS): %.o:%.cpp
	$(GCC) -o $@ $(DCPPFLAGS) $(CFLAGS) $(INCLUDES) $<
$(PRJ_OBJS): %.o:%.cpp
	$(GCC) -o $@ $(DCPPFLAGS) $(CFLAGS) $(INCLUDES) $<

%.o: %.c
	$(CC) ${DCPPFLAGS}  ${CFLAGS}   ${INCLUDES} $<
%.o: %.C
	$(CXX) ${DCPPFLAGS} ${CXXFLAGS} ${INCLUDES) $<
%.o: %.cc
	${CXX} ${DCPPFLAGS} ${CXXFLAGS} ${INCLUDES}   $<
%.o: %.cpp
	${CXX}     ${DCPPFLAGS}    ${CXXFLAGS}    ${INCLUDES}   $<
%.o : %.CPP
	${CXX}     ${DCPPFLAGS}    ${CXXFLAGS}    ${INCLUDES}   $<
%.o: %.c++
	${CXX}     ${DCPPFLAGS}    ${CXXFLAGS}    ${INCLUDES}   $<
%.o: %.cp
	${CXX}     ${DCPPFLAGS}    ${CXXFLAGS}    ${INCLUDES}   $<
%.o: %.cxx
	${CXX}     ${DCPPFLAGS}    ${CXXFLAGS}    ${INCLUDES}   $<

clean:
	rm -f $(OBJS)  *.o
	rm -f $(TARGET)

test:
	@echo "AutoSources=[${AUTO_SOURCES}]"
	@echo "AutoObjs=[${AUTO_OBJS}]"
	@echo "LDCXX=[${LDCXX}]"
	@echo "OBJS=[${OBJS}]"
