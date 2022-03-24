CXX ?= gcc

DEBUG ?= 1
ifeq ($(DEBUG), 1)
    CXXFLAGS += -g
else
    CXXFLAGS += -O2

endif

worker: main.cpp   ./executant/executant.c ./monitor/monitor.c \
					./source/source.c ../../src/ipc/msg.cpp
	$(CXX) -o worker  $^ $(CXXFLAGS) -lseccomp -lpthread

clean:
	rm  -r worker