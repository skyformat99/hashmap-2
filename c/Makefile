CC := gcc
CFLAGS := -O2 -Wall -Werror
OBJS := $(patsubst %.c, %.o, $(wildcard *.c))

TARGET := test-hash-func

.PHONY: all clean

all: $(TARGET)

test-hash-func: $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJS)
