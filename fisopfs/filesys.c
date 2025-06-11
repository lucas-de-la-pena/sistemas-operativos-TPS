#include "filesys.h"
#include <stdlib.h>

struct super_block super_b = {};

// FS
// inicializar FILE SYS
// definir tamanio maximo
//  y cantidad max. de directorios anidados
int
initialize_fs()
{
	// inicializo el bloque de memoria en 0
	memset(super_b.inodes, 0, sizeof(super_b.inodes));
	memset(super_b.bitmap_inodes, 0, sizeof(super_b.bitmap_inodes));

	struct inode *root = &super_b.inodes[0];
	root->type = FS_DIR;
	root->mode = MODE_DIR;
	root->size = MAX_DIR_SIZE;
	root->id_user = getuid();
	root->id_grup = getgid();
	root->stats_info.creation = time(NULL);
	root->stats_info.last_acc = time(NULL);
	root->stats_info.last_mod = time(NULL);
	strcpy(root->path, ROOT_PATH);
	memset(root->content, 0, sizeof(root->content));
	strcpy(root->directory_path, "");
	super_b.bitmap_inodes[0] = 1;


	return 0;
}

// guardar
int
save_fs(char *save_file)
{
	return 0;
}

// FILE OPERTATIONS
// read file
int
read_file(char *path)
{
	return 0;
}

// funcion para eliminar el slash del path y devolver solo el nombre del
// archivo o del directorio
char *
remove_slash(const char *path)
{
	size_t len = strlen(path);
	char *new_path = malloc(len);
	if (!new_path) {
		return NULL;
	}

	memcpy(new_path, path + 1, len - 1);
	new_path[len - 1] = '\0';

	const char *ultimo = strrchr(path, '/');
	if (ultimo == NULL) {
		return new_path;
	}

	size_t final_len = strlen(ultimo + 1);
	char *final_path = malloc(final_len + 1);
	if (!final_path) {
		free(new_path);
		return NULL;
	}

	memcpy(final_path, ultimo + 1, final_len);
	final_path[final_len] = '\0';

	free(new_path);
	return final_path;
}

// funcion para obtener el indice del inodo
int
get_index_inode(const char *path)
{
	if (strcmp(path, ROOT_PATH) == 0) {
		return 0;
	}
	char *new_path = remove_slash(path);
	if (!new_path) {
		return -1;
	}
	for (int i = 0; i < MAX_INODES; i++) {
		if (strcmp(new_path, super_b.inodes[i].path) == 0) {
			return i;
		}
	}
	free(new_path);
	return -1;
}

// funcion para modificar el path del inodo y cambio el '/' por '\0'
void
get_path_father(char *path_father)
{
	char *ultimo = strrchr(path_father, '/');
	if (ultimo != NULL) {
		*ultimo = '\0';
	} else {
		path_father[0] = '\0';
	}
}

// Funcion para buscar el proximo inodo libre
//  ENOSPC si no hay espacio
//  EEXIST si ya existe nodo en el path
int
next_free_inode(const char *path)
{
	bool existe = false;
	int next_free = -ENOSPC;
	for (int i = 0; i < MAX_INODES && !existe; i++) {
		if (super_b.bitmap_inodes[i] == 0 && next_free < 0) {
			// me quedo con el primer indice libre
			next_free = i;
		}
		if (strcmp(super_b.inodes[i].path, path) == 0) {
			existe = true;
		}
	}
	if (existe) {
		fprintf(stderr,
		        "[Debug] Error next_free_inode: %s\n",
		        strerror(errno));
		errno = EEXIST;
		return -EEXIST;
	} else {
		return next_free;
	}
}


// create file
/*crea un nuevo inodo con el path, mode y tipo que se le pasa.
lo guarda en el superbloque.-
En caso de error devuelve: ENAMETOOLONG, si el path es muy largo
        ENOSPC, si no hay más espacio
        EEXIST, si ya existe un inodo a ese path*/
int
create_file(const char *path, mode_t mode, int type)
{
	if (strlen(path) - 1 > MAX_CONTENT) {
		fprintf(stderr, "[Debug] Error create_file: %s\n", strerror(errno));
		errno = ENAMETOOLONG;
		return -ENAMETOOLONG;
	}

	char *final_path = remove_slash(path);
	if (!final_path) {
		return -1;
	}
	int i = next_free_inodo(final_path);
	if (i < 0) {
		return i;
	}

	struct inode new_inodo;
	new_inodo.type = type;
	new_inodo.mode = mode;
	new_inodo.size = 0;  // debería arranca vacío
	new_inodo.id_user = getuid();
	new_inodo.id_grup = getgid();
	new_inodo.stats_info.last_acc = time(NULL);
	new_inodo.stats_info.last_mod = time(NULL);
	new_inodo.stats_info.creation = time(NULL);
	strcpy(new_inodo.path, final_path);

	if (type == FS_FILE) {
		char path_padre[MAX_PATH];
		memcpy(path_padre, path + 1, strlen(path) - 1);
		path_padre[strlen(path) - 1] = '\0';
		get_path_padre(path_padre);

		if (strlen(path_padre) == 0) {
			strcpy(path_padre, ROOT_PATH);
		}

		strcpy(new_inodo.directory_path, path_padre);
	} else {
		strcpy(new_inodo.directory_path, ROOT_PATH);
	}

	memset(new_inodo.content, 0, sizeof(new_inodo.content));
	super_b.inodes[i] = new_inodo;
	super_b.bitmap_inodes[i] = 1;
	free(final_path);

	return 0;
}
// delete file
int
delete_file(char *path)
{
	for (int i = 0; i < MAX_INODES; i++) {
		if (super_b.bitmap_inodes[i] &&
		    strcmp(super_b.inodes[i].path, path) == 0) {
			struct inode *file = &super_b.inodes[i];
			if (file->type != FS_FILE) {
				fprintf(stderr,
				        "[Debug] Error: %s is not a file.\n",
				        path);
				return -1;
			}
			memset(file, 0, sizeof(struct inode));
			super_b.bitmap_inodes[i] = 0;
			return 0;
		}
	}
	fprintf(stderr, "[Debug] Error: File %s not found.\n", path);
	return -1;
}

// write file
int
write_file(const char *path, const char *buffer, size_t size, off_t offset)
{
	if (offset + size > MAX_CONTENT) {
		fprintf(stderr, "Error: File write exceeds max content size.\n");
		return -EFBIG;
	}
	int inode_index = get_index_inodo(path);
	if (inode_index < 0) {
		int new_file = create_file(path, 0644, FS_FILE);
		if (new_file < 0) {
			return new_file;
		}
		inode_index = get_index_inodo(path);
	}

	if (inode_index == -1) {
		fprintf(stderr, "Error: File not found.\n");
		return -ENOENT;
	}
	struct inode *file_inode = &super_b.inodes[inode_index];
	if (file_inode->size < offset) {
		fprintf(stderr, "[Debug] Error write: %s\n", strerror(errno));
		errno = EINVAL;
		return -EINVAL;
	}

	if (file_inode->type == FS_DIR) {
		fprintf(stderr, "Error: Cannot write to a directory.\n");
		return -EACCES;
	}

	strncpy(file_inode->content + offset, buffer, size);
	file_inode->size = strlen(file_inode->content);
	file_inode->stats_info.last_mod = time(NULL);
	file_inode->stats_info.last_acc = time(NULL);
	file_inode->content[file_inode->size] = '\0';

	return (int) size;
}

// stats
stats_t
get_stats(char *path)
{
	stats_t stats = {};
	return stats;
}

// DIR OPERATIONS
// create dir
int
create_dir(const char *path, mode_t mode)
{
	return 0;
}

// get dir
char *
get_dir(char *path, mode_t mode)
{
	char *re = "";
	return re;
}

// delete dir
int
unlink(const char *path)
{
	return 0;
}


int
delete_dir(const char *path)
{
	return 0;
}


int
list_dir(char *path)
{
	return 0;
}