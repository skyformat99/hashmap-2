CXX := g++

ifeq ($(release), y)
    CXXFLAGS := -O2 -DNDEBUG
else
    CXXFLAGS := -g
endif

CXXFLAGS := $(CXXFLAGS) -Wall -Werror -std=c++0x

INCLUDE :=
LIBS := -pthread

OBJS := $(patsubst %.cpp, %.o, $(wildcard *.cpp))

TARGET := test_myhashmap

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)

.cpp.o:
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJS)
