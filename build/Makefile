CXX = g++
# CFLAGS = -std=c++14 -O2 -Wall -g 

TARGET = server
OBJS = ../code/http/*.cpp ../code/server/*.cpp ../code/main.cpp

all: $(OBJS)
	$(CXX) $(OBJS) -o ../bin/$(TARGET)  -pthread


clean:
	rm -rf ../bin/$(OBJS) $(TARGET)

