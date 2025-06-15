#ifndef FILESYS_H
#define FILESYS_H

#include <fuse.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <stdbool.h>

#define DEFAULT_FILE_DISK "persistence_file.fisopfs"
#define MAX_INODES 64
#define SAVE_FILE "fs.fisopsfs"
#define MAX_DIR_SIZE 1024
#define MAX_PATH 200
#define MAX_CONTENT 1024
#define ROOT_PATH "/"
#define MODE_FILE (__S_IFREG | 0644)
#define MODE_DIR (__S_IFDIR | 0755)

typedef enum inode_type {
	FS_DIR,
	FS_FILE
} inode_type;

typedef struct stats {
	time_t last_acc;
	time_t last_mod;
	time_t creation;
} stats_t;

struct inode {
	inode_type type;
	size_t size;
	uid_t id_user;
	gid_t id_grup;
	mode_t mode;
	stats_t stats_info;
	nlink_t link_num;
	char path[MAX_PATH];
	char content[MAX_CONTENT];
	char directory_path[MAX_PATH];
};

struct super_block {
	struct inode inodes[MAX_INODES];
	int bitmap_inodes[MAX_INODES];
};

extern struct super_block super_b;

int get_index_inode(const char *path);
char *remove_slash(const char *path);
void get_path_parent(char *path_parent);
int next_free_inode(const char *path);
int initialize_fs();
int save_fs(char *save_file);
int read_file(struct inode *in, char *buffer, size_t size, off_t offset);
int create_file(const char *path, mode_t mode);
int delete_file(char *path);
int write_file(const char *path, const char *buffer, size_t size, off_t offset);
stats_t get_stats(char *path);
int create_dir(const char *path, mode_t mode);
char *get_dir(char *path, mode_t mode);
int delete_dir(const char *path);
int list_dir(char *path);
int fs_unlink_file(const char *path);

#endif