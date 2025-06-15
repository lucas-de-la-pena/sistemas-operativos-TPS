CC = gcc

CFLAGS := -ggdb3 -O2 -Wall -std=c11 -Wno-unused-function -Wvla

LDLIBS := $(shell pkg-config fuse --cflags --libs) -pthread

FS_NAME := fisopfs

SRCS := fisopfs.c filesys.c

all: $(FS_NAME)

$(FS_NAME): $(SRCS)
	$(CC) $(CFLAGS) -o $(FS_NAME) $(SRCS) $(LDLIBS)

clean:
	rm -f $(FS_NAME) *.o core vgcore.*

format: .clang-files .clang-format
	xargs -r clang-format -i <$<

docker-build:
	./dock build

docker-run:
	./dock run

docker-exec:
	./dock exec

.PHONY: all clean format docker-build docker-run docker-exec