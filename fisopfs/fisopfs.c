#include "filesys.h"
#define MAX_CONTENIDO 100

char *filedisk = DEFAULT_FILE_DISK;
char nombre_archivo_disco[MAX_PATH] = "fs.vfsimg";

// ----------------------------------
//       Get Attributes
// ----------------------------------
static int
fisopfs_getattr(const char *path, struct stat *st)
{
	printf("[debug] fisopfs_getattr - path: %s\n", path);
	int i = get_index_inodo(path);
	if (i == -1) {
		fprintf(stderr, "[Debug] getattr: %s\n", strerror(errno));
		errno = ENOENT;
		return -ENOENT;
	}

	struct inode *in = &super_b.inodes[i];
	memset(st, 0, sizeof(struct stat));  // Limpio la estructura de stat
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

// ----------------------------------
//       Read Directory
// ----------------------------------
static int
fisopfs_readdir(const char *path,
                void *buffer,
                fuse_fill_dir_t filler,
                off_t offset,
                struct fuse_file_info *fi)
{
	printf("[debug] fisopfs_readdir - path: %s\n", path);

	// Los directorios '.' y '..'
	filler(buffer, ".", NULL, 0);
	filler(buffer, "..", NULL, 0);

	// Si nos preguntan por el directorio raiz, solo tenemos un archivo
	if (strcmp(path, "/") == 0) {
		filler(buffer, "fisop", NULL, 0);
		return 0;
	}

	return -ENOENT;
}


// ----------------------------------
//         File Read/Write
// ----------------------------------

static int
fisopfs_read(const char *path,
             char *buffer,
             size_t size,
             off_t offset,
             struct fuse_file_info *fi)
{
	printf("[debug] fisopfs_read - path: %s, offset: %lu, size: %lu\n",
	       path,
	       offset,
	       size);

	if (offset < 0 || size < 0) {
		fprintf(stderr, "[Debug] Error read: %s\n", strerror(errno));
		errno = EINVAL;
		return -EINVAL;
	}

	int i = get_index_inodo(path);
	if (i == -1) {
		fprintf(stderr, "[Debug] Error read: %s\n", strerror(errno));
		errno = ENOENT;
		return -ENOENT;
	}

	struct inode *in = &super_b.inodes[i];
	if (in->type != FS_FILE) {
		fprintf(stderr, "[Debug] Error read: %s\n", strerror(errno));
		errno = EISDIR;
		return -EISDIR;
	}

	if (offset == in->size)
		return 0;  // No hay más datos para leer
	else if (offset > in->size) {
		fprintf(stderr, "[Debug] Error read: %s\n", strerror(errno));
		errno = EINVAL;
		return -EINVAL;
	}

	size_t to_read = (in->size - offset > size) ? size : in->size - offset;

	memcpy(buffer, in->content + offset, to_read);
	in->stats_info.last_acc = time(NULL);

	return to_read;
}

static int
fisopfs_write(const char *path,
              const char *buf,
              size_t size,
              off_t offset,
              struct fuse_file_info *fi)
{
	printf("[Debug] fisopfs_write: %s, offset: %lu, size: %lu\n",
	       path,
	       offset,
	       size);
	return write_file(path, buf, size, offset);
}

// ----------------------------------
//     File/Directory Creation
// ----------------------------------

static int
fisopfs_create(const char *path, mode_t mode, struct fuse_file_info *fi)
{
	printf("[Debug] fisop_touch : %s\n", path);
	return create_file(path, mode, FS_FILE);
}

static int
fisopfs_mkdir(const char *path, mode_t mode)
{
	printf("mkdir %s\n", path);
	return create_file(path, mode, FS_DIR);
}


// ----------------------------------
//         File Deletion
// ----------------------------------

static int
fisopfs_unlink(const char *path)
{
	printf("unlink file %s\n", path);
	return unlink(path);
}

static int
fisopfs_rmdir(const char *path)
{
	// Elimino directorio jej ((idem que el anterior pero llamando a la funcion de delete_dir))
}


// ----------------------------------
//        Metadata Updates
// ----------------------------------

static int
fisopfs_truncate(const char *path, off_t size)
{
	printf("truncate %s %ld\n", path, size);

	if (size > MAX_CONTENT)
		return -EFBIG;

	int i = get_index_inodo(path);
	if (i == -1)
		return -ENOENT;

	struct inode *in = &super_b.inodes[i];
	in->size = size;
	in->stats_info.last_mod = time(NULL);

	return 0;
}

static int
fisopfs_updatetime(const char *path, const struct timespec ts[2])
{
	int i = get_index_inodo(path);
	if (i == -1)
		return -ENOENT;

	struct inode *in = &super_b.inodes[i];
	in->stats_info.last_acc = ts[0].tv_sec;
	in->stats_info.last_mod = ts[1].tv_sec;

	return 0;
}


// ----------------------------------
//        FUSE Init/Destroy
// ----------------------------------

void *
fisopfs_init(struct fuse_conn_info *conn, struct fuse_config *cfg)
{
	(void) conn;
	(void) cfg;

	printf("init\n");

	FILE *archivo_disco = fopen(nombre_archivo_disco, "r");
	if (!archivo_disco) {
		initialize_fs();
	} else {
		fread(&super_b, sizeof(super_b), 1, archivo_disco);
		fclose(archivo_disco);
	}

	return NULL;
}

void
fisopfs_destroy()
{
	printf("shutdown\n");

	FILE *archivo_disco = fopen(nombre_archivo_disco, "w");
	if (!archivo_disco) {
		fprintf(stderr, "[Debug] ERROR save fisop: %s\n", strerror(errno));
	}

	if (fwrite(&super_b, sizeof(super_b), 1, archivo_disco) != 1) {
		fprintf(stderr,
		        "[Debug] ERROR writing fisop: %s\n",
		        strerror(errno));
	}

	if (fflush(archivo_disco) != 0) {
		fprintf(stderr,
		        "[Debug] ERROR flushing file: %s\n",
		        strerror(errno));
	}

	if (fsync(fileno(archivo_disco)) != 0) {
		fprintf(stderr,
		        "[Debug] ERROR sync with disk: %s\n",
		        strerror(errno));
	}

	fclose(archivo_disco);
}


// ----------------------------------
//          FUSE Bindings
// ----------------------------------
static struct fuse_operations operations = { .getattr = fisopfs_getattr, 
	                                     .getdir = fisopfs_readdir,
	                                     .read = fisopfs_read,
	                                     .mknod = fisopfs_create,
	                                     .mkdir = fisopfs_mkdir,
	                                     .write = fisopfs_write,
	                                     .rmdir = fisopfs_rmdir,
	                                     .unlink = fisopfs_unlink,
	                                     .utime = fisopfs_updatetime,
	                                     .truncate = fisopfs_truncate };

int
main(int argc, char *argv[])
{
	for (int i = 1; i < argc - 1; i++) {
		if (strcmp(argv[i], "--filedisk") == 0) {
			filedisk = argv[i + 1];

			// We remove the argument so that fuse doesn't use our
			// argument or name as folder.
			// Equivalent to a pop.
			for (int j = i; j < argc - 1; j++) {
				argv[j] = argv[j + 2];
			}

			argc = argc - 2;
			break;
		}
	}
	initialize_fs();
	if (strlen(filedisk) > MAX_PATH) {
		fprintf(stderr, "Error: Disk file name too long.\n");
		return -1;
	}

	// El cuarto parámetro en versiones superior a Fuse 2.9.9 es obsoleto
	// y puede marcar un warning, pero es necesario para la version del TP
	int ret = fuse_main(argc, argv, &operations, NULL);
	fisopfs_destroy(NULL);
	return ret;
}
