CXX := g++
CXXFLAGS := -O2 -Wall -Werror -std=c++0x

INCLUDE := -I../../mystring
LIBS := -lpthread

OBJS := $(patsubst %.cpp, %.o, $(wildcard *.cpp)) \
    $(patsubst %.cpp, %.o, $(wildcard ../../mystring/mystring.cpp))

TARGET := test-hash

.PHONY: all clean

all: $(TARGET)

test-hash: $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)

.cpp.o:
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJS)
