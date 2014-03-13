CC := gcc

ifeq ($(release), y)
    CFLAGS := -O2 -DNDEBUG
else
    CFLAGS := -g
endif

CFLAGS := $(CFLAGS) -Wall -Werror
OBJS := $(patsubst %.c, %.o, $(wildcard *.c))

TARGET := test_hash_func

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJS)
