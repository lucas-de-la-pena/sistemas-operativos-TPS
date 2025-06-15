#include "filesys.h"
#include <stdlib.h>

#define MAX_CONTENIDO 100

char *filedisk = DEFAULT_FILE_DISK;
char nombre_archivo_disco[MAX_PATH] = "fs.vfsimg";

static void
fisopfs_save_fs_on_exit()
{
	printf("apagando - guardando el estado del sistema de archivos\n");

	FILE *archivo_disco = fopen(nombre_archivo_disco, "w");
	if (!archivo_disco) {
		fprintf(stderr,
		        "[Debug] ERROR al guardar fisop: %s\n",
		        strerror(errno));
		return;
	}

	if (fwrite(&super_b, sizeof(super_b), 1, archivo_disco) != 1) {
		fprintf(stderr,
		        "[Debug] ERROR escribiendo fisop: %s\n",
		        strerror(errno));
	}
	if (fflush(archivo_disco) != 0) {
		fprintf(stderr,
		        "[Debug] ERROR vaciando archivo: %s\n",
		        strerror(errno));
	}

	if (fsync(fileno(archivo_disco)) != 0) {
		fprintf(stderr,
		        "[Debug] ERROR sincronizando con disco: %s\n",
		        strerror(errno));
	}

	fclose(archivo_disco);
}

int
get_and_validate_inode(const char *path, int expected_type, struct inode **inode_out)
{
	int idx = get_index_inode(path);
	if (idx < 0) {
		fprintf(stderr, "[Debug] Error: %s no encontrado.\n", path);
		return -ENOENT;
	}

	struct inode *node = &super_b.inodes[idx];
	if (node->type != expected_type) {
		fprintf(stderr, "[Debug] Error: %s tiene tipo incorrecto.\n", path);
		return (expected_type == FS_FILE) ? -EISDIR : -ENOTDIR;
	}

	node->stats_info.last_acc = time(NULL);
	*inode_out = node;
	return 0;
}

static int
fisopfs_getattr(const char *path, struct stat *st)
{
	printf("[debug] fisopfs_getattr - ruta: %s\n", path);
	int i = get_index_inode(path);
	if (i == -1) {
		return -ENOENT;
	}

	struct inode *in = &super_b.inodes[i];
	memset(st, 0, sizeof(struct stat));
	st->st_uid = in->id_user;
	st->st_gid = in->id_grup;
	st->st_size = in->size;
	st->st_atime = in->stats_info.last_acc;
	st->st_mtime = in->stats_info.last_mod;
	st->st_ctime = in->stats_info.creation;
	st->st_ino = i;

	if (in->type == FS_DIR) {
		st->st_mode = MODE_DIR;
		st->st_nlink = 2;
	} else {
		st->st_mode = MODE_FILE;
		st->st_nlink = 1;
	}

	return 0;
}

static int
fisopfs_getdir(const char *path, fuse_dirh_t h, fuse_dirfil_t filler)
{
	printf("[debug] fisopfs_getdir - ruta: %s\n", path);

	filler(h, ".", 0);
	filler(h, "..", 0);

	struct inode *inodo_dir;
	int err = get_and_validate_inode(path, FS_DIR, &inodo_dir);
	if (err < 0)
		return err;
	for (int i = 0; i < MAX_INODES; i++) {
		if (super_b.bitmap_inodes[i] &&
		    strcmp(super_b.inodes[i].directory_path, inodo_dir->path) ==
		            0) {
			const char *child_full_path = super_b.inodes[i].path;
			const char *child_name = strrchr(child_full_path, '/');

			if (child_name == NULL) {
				filler(h, child_full_path, 0);
			} else {
				filler(h, child_name + 1, 0);
			}
		}
	}

	return 0;
}

static int
fisopfs_read(const char *path, char *buffer, size_t size, off_t offset)
{
	printf("[debug] fisopfs_read - ruta: %s, offset: %lu, tamaño: %lu\n",
	       path,
	       offset,
	       size);

	if (offset < 0 || size < 0)
		return -EINVAL;

	struct inode *in;
	int err = get_and_validate_inode(path, FS_FILE, &in);
	if (err < 0)
		return err;

	return read_file(in, buffer, size, offset);
}

static int
fisopfs_write(const char *path, const char *buf, size_t size, off_t offset)
{
	printf("[Debug] fisopfs_write: %s, offset: %lu, tamaño: %lu\n",
	       path,
	       offset,
	       size);
	return write_file(path, buf, size, offset);
}

static int
fisopfs_create(const char *path, mode_t mode, dev_t dev)
{
	(void) dev;
	printf("[Debug] fisop_touch (crear/mknod): %s\n", path);
	return create_file(path, mode);
}

static int
fisopfs_mkdir(const char *path, mode_t mode)
{
	printf("mkdir %s\n", path);
	return create_dir(path, mode);
}

static int
fisopfs_unlink(const char *path)
{
	printf("eliminar archivo %s\n", path);
	return fs_unlink_file(path);
}

static int
fisopfs_rmdir(const char *path)
{
	printf("[Debug] fisopfs_rmdir: %s\n", path);
	return delete_dir(path);
}

static int
fisopfs_truncate(const char *path, off_t size)
{
	printf("truncar %s %ld\n", path, size);

	if (size > MAX_CONTENIDO)
		return -EFBIG;

	int i = get_index_inode(path);
	if (i == -1)
		return -ENOENT;

	struct inode *in = &super_b.inodes[i];
	in->size = size;
	in->stats_info.last_mod = time(NULL);

	return 0;
}

static int
fisopfs_utime(const char *path, struct utimbuf *ub)
{
	int i = get_index_inode(path);
	if (i == -1)
		return -ENOENT;

	struct inode *in = &super_b.inodes[i];
	in->stats_info.last_acc = ub->actime;
	in->stats_info.last_mod = ub->modtime;

	return 0;
}

static struct fuse_operations operations = {
	.getattr = fisopfs_getattr,
	.getdir = fisopfs_getdir,
	.read = fisopfs_read,
	.write = fisopfs_write,
	.mknod = fisopfs_create,
	.mkdir = fisopfs_mkdir,
	.unlink = fisopfs_unlink,
	.rmdir = fisopfs_rmdir,
	.truncate = fisopfs_truncate,
	.utime = fisopfs_utime,
};

int
main(int argc, char *argv[])
{
	char temp_disk_path[MAX_PATH] = DEFAULT_FILE_DISK;
	for (int i = 1; i < argc; i++) {
		if (i + 1 < argc && strcmp(argv[i], "--filedisk") == 0) {
			strncpy(temp_disk_path, argv[i + 1], MAX_PATH - 1);

			for (int j = i; j < argc - 2; j++) {
				argv[j] = argv[j + 2];
			}
			argc -= 2;
			break;
		}
	}

	if (temp_disk_path[0] != '/') {
		char current_dir[MAX_PATH];
		if (getcwd(current_dir, sizeof(current_dir)) != NULL) {
			if (strlen(current_dir) + 1 + strlen(temp_disk_path) + 1 >
			    MAX_PATH) {
				fprintf(stderr, "Error: La ruta absoluta al archivo de disco es demasiado larga.\n");
				return 1;
			}
			strcpy(nombre_archivo_disco, current_dir);
			strcat(nombre_archivo_disco, "/");
			strcat(nombre_archivo_disco, temp_disk_path);
		} else {
			perror("Error obteniendo el directorio actual con "
			       "getcwd");
			return 1;
		}
	} else {
		strcpy(nombre_archivo_disco, temp_disk_path);
	}

	printf("init - cargando el estado del sistema de archivos desde la "
	       "ruta absoluta: %s\n",
	       nombre_archivo_disco);

	FILE *archivo_disco = fopen(nombre_archivo_disco, "r");
	if (!archivo_disco) {
		initialize_fs();
	} else {
		if (fread(&super_b, sizeof(super_b), 1, archivo_disco) != 1) {
			fprintf(stderr, "Error al leer el superbloque. Se inicializa un nuevo FS.\n");
			initialize_fs();
		}
		fclose(archivo_disco);
	}

	atexit(fisopfs_save_fs_on_exit);

	return fuse_main(argc, argv, &operations);
}
