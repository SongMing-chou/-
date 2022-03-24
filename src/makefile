CXX ?= g++

DEBUG ?= 1
ifeq ($(DEBUG), 1)
    CXXFLAGS += -g
else
    CXXFLAGS += -O2

endif

server0: main.cpp   ./threadpool/threadpool.cpp ./threadpooldata/mythreadpooldata.cpp \
					./server/server.cpp ./ipc/msg.cpp ./filecntl/filecntl.cpp
	$(CXX) -o server0  $^ $(CXXFLAGS) -lpthread 

clean:
	rm  -r server0