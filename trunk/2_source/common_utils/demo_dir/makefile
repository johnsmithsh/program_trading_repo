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
INCLUDES:= -I./ -I../
#动态库路径
LIB_DIR:=
SO_LIBS:=-lstdc++ -lpthread -lrt

#常用文件
COMMON_SOURCES:=
COMMON_OBJS:= $(COMMON_SOURCES:.cpp=.o)


SOURCES:=$(COMMON_SOURCES) $(AUTO_SOURCES)
OBJS:= $(AUTO_OBJS) $(COMMON_OBJS)

#单元测试
unittest_FLAG:=1
unittest_TARGETS=unittest_os_path 
unittest_SOURCES:=./unittest_os_path.cpp
unittest_OBJS=$(unittest_SOURCES:.cpp=.o)

ifdef unittest_FLAG
unittest_INCLUDES:=-I./  -I./gtest/include
unittest_LIB_DIR:= 
unittest_LIBS:= 

gtest_libs=-L./gtest/linux64/ -lgtest

CFLAGS += -DSINGLE_UNITTEST  
CPPFLAGS += -DSINGLE_UNITTEST 
#使用googletest
LINK_FLAGS =  
endif

#汇总所有的源文件
TARGET:=$(unittest_TARGETS)
#输出路径
OUTPUT_DIR:=./bin
all:$(TARGET)

#单元测试构建规则
unittest_os_path:./unittest_os_path.o
	$(GCC) -o $@ $(LIB_DIR) $(SO_LIBS)  $^   $(gtest_libs)

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



#单元测试构建规则
#unittest:$(OBJS) $(unittest_OBJS)
unittest:$(OBJS) $(unittest_TARGETS)
#	$(GCC) -o $@ $(LIB_DIR) $(SO_LIBS) $(unittest_LIB_DIR) $(unit_test_LIB_DIR) $(LINK_FLAGS)  $(OBJS) $(unittest_OBJS) unit_test/pub_lib/gtest/linux64/libgtest.a
	@echo "build unit test finished!"

./unit_test/unittest_mem_buff:./unit_test/unittest_mem_buff.o 
	$(GCC) -o $@ $(LIB_DIR) $(SO_LIBS) $(unittest_LIB_DIR) $(unit_test_LIB_DIR) $(unittest_LIBS) $(LINK_FLAGS) $< unit_test/pub_lib/gtest/linux64/libgtest.a  
#unit_test/pub_lib/gtest/linux64/libgtest.a

./unit_test/unittest_dataMemberDescriber: ./unit_test/unittest_dataMemberDescriber.o dataMemberDescriber.o
	$(GCC) -o $@ $(LIB_DIR) $(SO_LIBS) $(unittest_LIB_DIR) $(unit_test_LIB_DIR) $(unittest_LIBS) $(LINK_FLAGS) $^ unit_test/pub_lib/gtest/linux64/libgtest.a 

$(unittest_OBJS): %.o:%.cpp
	$(GCC) -o $@ $(DCPPFLAGS) $(CFLAGS) $(INCLUDES) $(unittest_INCLUDES) $<


clean:
	rm -f $(OBJS)  *.o
	rm -f $(unittest_OBJS) $(TARGET) #清理单元测试相关内容

test:
	@echo "AutoSources=[${AUTO_SOURCES}]"
	@echo "AutoObjs=[${AUTO_OBJS}]"
	@echo "LDCXX=[${LDCXX}]"
	@echo "OBJS=[${OBJS}]"
