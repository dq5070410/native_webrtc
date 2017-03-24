# 指令编译器和选项  
CC = gcc  
CPP = g++
AR = ar rc
DEFS = -DWEBRTC_POSIX -DWEBRTC_LINUX
CFLAGS = -Wall -std=gnu99  
CPPFLAGS = -Wall -g -std=c++11 -pthread
CPPFLAGS += $(DEFS)
  
# 目标文件  
TARGET = video_receiver 
# 头文件查找路径
INC = -I./ -I../../ -I./include/ -I./include/ffmpeg -I/usr/X11R6/include -L/usr/X11R6/lib
# C文件
SRCS = $(shell ls *.cc)  
#库文件及路径
DLIBS += -lwebrtc_base -lvideo_processing -lvideo_render -lvideo_engine -lvideo_coding -lsystem_wrappers -lrtp_rtcp -lutility
DLIBS += -lremote_bitreate_estimator -lpacing -lchannel_transport -lbitrate_controller -lcommon_video -lyuv -lwebrtc_yuv -lvideo_capture
DLIBS += -lx264 -lavformat -lavcodec -lavutil -lswscale -lswresample -lpostproc -lX11 -lSDL2
LDFLAGS = -L../lib
#指定运行时的库文件路径
RPATH = 
# 目标文件  
OBJS = $(SRCS:.cc=.o)  
  
# 链接为可执行文件  
$(TARGET):$(OBJS)  
	$(CPP) -o $@ $^ $(LDFLAGS) $(DLIBS)

  
clean:  
	rm -rf $(TARGET) $(OBJS)  
  
# 编译规则 $@代表目标文件 $< 代表第一个依赖文件 $^ 去除重复的所有依赖文件 
%.o:%.cc  
	$(CPP) $(CPPFLAGS) $(INC) -o $@ -fPIC -c $< 
