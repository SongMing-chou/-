CXX ?= g++

DEBUG ?= 1
ifeq ($(DEBUG), 1)
    CXXFLAGS += -g
else
    CXXFLAGS += -O2

endif

main: exchanger.cpp msg.cpp 
	$(CXX) -o main  $^ $(CXXFLAGS) -lpthread 

clean:
	rm  -r main