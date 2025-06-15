#include "filesys.h"
#include <stdlib.h>

struct super_block super_b = {};

int
initialize_fs()
{
	memset(super_b.inodes, 0, sizeof(super_b.inodes));
	memset(super_b.bitmap_inodes, 0, sizeof(super_b.bitmap_inodes));

	struct inode *root = &super_b.inodes[0];
	root->type = FS_DIR;
	root->mode = MODE_DIR;
	root->size = MAX_DIR_SIZE;
	root->id_user = getuid();
	root->id_grup = getgid();
	time_t now = time(NULL);
	root->stats_info.creation = now;
	root->stats_info.last_acc = now;
	root->stats_info.last_mod = now;
	strcpy(root->path, ROOT_PATH);
	memset(root->content, 0, sizeof(root->content));
	strcpy(root->directory_path, "");

	super_b.bitmap_inodes[0] = 1;
	return 0;
}

int
save_fs(char *save_file)
{
	return 0;
}

int
create_node(const char *path, mode_t mode, int type)
{
	if (strlen(path) >= MAX_PATH) {
		return -ENAMETOOLONG;
	}

	int inode_index = next_free_inode(path);
	if (inode_index < 0) {
		return inode_index;
	}

	char parent_path[MAX_PATH];
	strcpy(parent_path, path);
	get_path_parent(parent_path);

	int parent_idx = get_index_inode(parent_path);
	if (parent_idx < 0) {
		return -ENOENT;
	}

	struct inode *node = &super_b.inodes[inode_index];
	memset(node, 0, sizeof(struct inode));

	node->type = type;
	node->mode = mode;
	node->size = 0;
	node->id_user = getuid();
	node->id_grup = getgid();
	node->stats_info.creation = time(NULL);
	node->stats_info.last_acc = node->stats_info.creation;
	node->stats_info.last_mod = node->stats_info.creation;
	strcpy(node->path, path);
	strcpy(node->directory_path, parent_path);

	super_b.bitmap_inodes[inode_index] = 1;

	return 0;
}

int
delete_inode(const char *path, int expected_type)
{
	int idx = get_index_inode(path);
	if (idx < 0) {
		fprintf(stderr, "[Debug] Error: %s not found.\n", path);
		return -ENOENT;
	}

	struct inode *node = &super_b.inodes[idx];
	if (node->type != expected_type) {
		if (expected_type == FS_DIR)
			return -ENOTDIR;
		else {
			fprintf(stderr, "[Debug] Error: %s is not a file.\n", path);
			return -1;
		}
	}

	if (expected_type == FS_DIR) {
		for (int i = 0; i < MAX_INODES; i++) {
			if (super_b.bitmap_inodes[i] &&
			    strcmp(super_b.inodes[i].directory_path, path) == 0) {
				return -ENOTEMPTY;
			}
		}
	}

	super_b.bitmap_inodes[idx] = 0;
	memset(&super_b.inodes[idx], 0, sizeof(struct inode));
	return 0;
}

int
read_file(struct inode *in, char *buffer, size_t size, off_t offset)
{
	if (offset < 0 || size < 0)
		return -EINVAL;

	if (offset >= in->size)
		return 0;

	size_t to_read = (in->size - offset > size) ? size : in->size - offset;

	memcpy(buffer, in->content + offset, to_read);

	in->stats_info.last_acc = time(NULL);

	return to_read;
}

char *
remove_slash(const char *path)
{
	size_t len = strlen(path);
	char *temp = malloc(len);
	if (!temp)
		return NULL;

	memcpy(temp, path + 1, len - 1);
	temp[len - 1] = '\0';

	const char *last = strrchr(path, '/');
	if (!last)
		return temp;

	size_t final_len = strlen(last + 1);
	char *final_path = malloc(final_len + 1);
	if (!final_path) {
		free(temp);
		return NULL;
	}

	memcpy(final_path, last + 1, final_len);
	final_path[final_len] = '\0';

	free(temp);
	return final_path;
}

int
get_index_inode(const char *path)
{
	for (int i = 0; i < MAX_INODES; i++) {
		if (super_b.bitmap_inodes[i] &&
		    strcmp(path, super_b.inodes[i].path) == 0) {
			return i;
		}
	}
	return -1;
}

void
get_path_parent(char *path_parent)
{
	char *last = strrchr(path_parent, '/');
	if (last) {
		if (last == path_parent) {
			*(last + 1) = '\0';
		} else {
			*last = '\0';
		}
	} else {
		strcpy(path_parent, "/");
	}
}

int
next_free_inode(const char *path)
{
	int free_index = -1;

	for (int i = 0; i < MAX_INODES; i++) {
		if (super_b.bitmap_inodes[i] &&
		    strcmp(super_b.inodes[i].path, path) == 0) {
			return -EEXIST;
		}
		if (super_b.bitmap_inodes[i] == 0 && free_index == -1) {
			free_index = i;
		}
	}

	if (free_index == -1) {
		return -ENOSPC;
	}

	return free_index;
}

int
create_file(const char *path, mode_t mode)
{
	return create_node(path, mode, FS_FILE);
}

int
delete_file(char *path)
{
	return delete_inode(path, FS_FILE);
}

int
write_file(const char *path, const char *buffer, size_t size, off_t offset)
{
	if (offset + size > MAX_CONTENT) {
		fprintf(stderr, "Error: Write exceeds max size.\n");
		return -EFBIG;
	}

	int idx = get_index_inode(path);
	if (idx < 0) {
		if ((idx = create_file(path, 0644)) < 0)
			return idx;
		idx = get_index_inode(path);
	}

	if (idx == -1) {
		fprintf(stderr, "Error: File not found after creation attempt.\n");
		return -ENOENT;
	}

	struct inode *node = &super_b.inodes[idx];

	if (node->type == FS_DIR) {
		fprintf(stderr, "Error: Cannot write in a Directory.\n");
		return -EISDIR;
	}

	if (offset < 0 || offset > node->size) {
		fprintf(stderr, "[Debug] Error write: offset is invalid.\n");
		return -EINVAL;
	}

	memcpy(node->content + offset, buffer, size);

	if (offset + size > node->size) {
		node->size = offset + size;
	}

	node->stats_info.last_mod = time(NULL);
	node->stats_info.last_acc = time(NULL);

	return (int) size;
}

stats_t
get_stats(char *path)
{
	stats_t stats = {};
	return stats;
}

int
create_dir(const char *path, mode_t mode)
{
	return create_node(path, mode, FS_DIR);
}

int
fs_unlink_file(const char *path)
{
	int idx = get_index_inode(path);
	if (idx < 0)
		return -1;

	if (super_b.inodes[idx].type == FS_DIR)
		return -EISDIR;

	super_b.bitmap_inodes[idx] = 0;
	memset(super_b.inodes[idx].content, 0, sizeof(super_b.inodes[idx].content));
	memset(super_b.inodes[idx].path, 0, sizeof(super_b.inodes[idx].path));

	return 0;
}

int
delete_dir(const char *path)
{
	return delete_inode(path, FS_DIR);
}

int
list_dir(char *path)
{
	return 0;
}

char *
get_dir(char *path, mode_t mode)
{
	char *res = "";
	return res;
}