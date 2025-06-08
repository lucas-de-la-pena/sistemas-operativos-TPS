#ifndef FS_H
#define FS_H

#include <fuse.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#define FUSE_USE_VERSION 30
#define DEFAULT_FILE_DISK "persistence_file.fisopfs"

#endif