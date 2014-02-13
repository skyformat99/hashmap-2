CC := gcc
CXX := g++
CFLAGS := -O2 -Wall -Werror
CXXFLAGS := $(CFLAGS) -std=c++0x
OBJS := $(patsubst %.cpp, %.o, $(wildcard *.cpp)) \
    $(patsubst %.cpp, %.o, $(wildcard ../mystring/mystring.cpp)) \
    $(patsubst %.c, %.o, $(wildcard *.c))

TARGET := test-hash

LIBS := -lpthread

.PHONY: all clean

all: $(TARGET)

test-hash: $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

.cpp.o:
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJS)
