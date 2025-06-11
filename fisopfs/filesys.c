#include "filesys.h"
#include <stdlib.h>

struct super_block super_b = {};

// ============================
// SISTEMA DE ARCHIVOS (FS)
// ============================

// Inicializa el sistema de archivos con una estructura base.
// Se establece el directorio raíz y se limpian los mapas de inodos.
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

// ============================
// OPERACIONES SOBRE ARCHIVOS
// ============================

int
read_file(char *path)
{
	return 0;
}

// Quita la primera barra del path y devuelve solo el nombre del archivo/directorio.
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

// Busca el índice de un inodo por su path
int
get_index_inode(const char *path)
{
	if (strcmp(path, ROOT_PATH) == 0)
		return 0;

	char *clean_path = remove_slash(path);
	if (!clean_path)
		return -1;

	for (int i = 0; i < MAX_INODES; i++) {
		if (strcmp(clean_path, super_b.inodes[i].path) == 0) {
			free(clean_path);
			return i;
		}
	}

	free(clean_path);
	return -1;
}

// Extrae el path del directorio padre de un path dado
void
get_path_parent(char *path_parent)
{
	char *last = strrchr(path_parent, '/');
	if (last)
		*last = '\0';
	else
		path_parent[0] = '\0';
}

// Busca el siguiente índice libre de inodo disponible.
// Retorna ENOSPC si no hay espacio o EEXIST si ya existe el path.
int
next_free_inode(const char *path)
{
	bool found = false;
	int free_index = -ENOSPC;

	for (int i = 0; i < MAX_INODES && !found; i++) {
		if (!super_b.bitmap_inodes[i] && free_index < 0)
			free_index = i;

		if (strcmp(super_b.inodes[i].path, path) == 0)
			found = true;
	}

	if (found) {
		fprintf(stderr,
		        "[Debug] Error next_free_inode: %s\n",
		        strerror(errno));
		errno = EEXIST;
		return -EEXIST;
	}

	return free_index;
}

//Crea un archivo o directorio nuevo y lo guarda como inodo.
//Errores posibles:
//- ENAMETOOLONG: nombre demasiado largo
//- ENOSPC: sin espacio
//- EEXIST: ya existe
int
create_file(const char *path, mode_t mode, int type)
{
	if (strlen(path) - 1 > MAX_CONTENT) {
		fprintf(stderr, "[Debug] Error create_file: %s\n", strerror(errno));
		errno = ENAMETOOLONG;
		return -ENAMETOOLONG;
	}

	char *name = remove_slash(path);
	if (!name)
		return -1;

	int idx = next_free_inode(name);
	if (idx < 0) {
		free(name);
		return idx;
	}

	struct inode node = { .type = type,
		              .mode = mode,
		              .size = 0,
		              .id_user = getuid(),
		              .id_grup = getgid(),
		              .stats_info = { .creation = time(NULL),
		                              .last_acc = time(NULL),
		                              .last_mod = time(NULL) } };
	strcpy(node.path, name);

	if (type == FS_FILE) {
		char parent[MAX_PATH];
		strncpy(parent, path + 1, strlen(path) - 1);
		parent[strlen(path) - 1] = '\0';
		get_path_parent(parent);

		if (strlen(parent) == 0)
			strcpy(parent, ROOT_PATH);

		strcpy(node.directory_path, parent);
	} else {
		strcpy(node.directory_path, ROOT_PATH);
	}

	memset(node.content, 0, sizeof(node.content));

	super_b.inodes[idx] = node;
	super_b.bitmap_inodes[idx] = 1;
	free(name);

	return 0;
}

// Elimina un archivo (si existe y no es un directorio).
int
delete_file(char *path)
{
	for (int i = 0; i < MAX_INODES; i++) {
		if (super_b.bitmap_inodes[i] &&
		    strcmp(super_b.inodes[i].path, path) == 0) {
			struct inode *f = &super_b.inodes[i];
			if (f->type != FS_FILE) {
				fprintf(stderr,
				        "[Debug] Error: %s is not a file.\n",
				        path);
				return -1;
			}
			memset(f, 0, sizeof(struct inode));
			super_b.bitmap_inodes[i] = 0;
			return 0;
		}
	}
	fprintf(stderr, "[Debug] Error: File %s not found.\n", path);
	return -1;
}

// Escribe datos en un archivo desde una posición dada.
// Si no existe, lo crea. Maneja errores como exceso de tamaño o intento de escritura en directorio.
int
write_file(const char *path, const char *buffer, size_t size, off_t offset)
{
	if (offset + size > MAX_CONTENT) {
		fprintf(stderr, "Error: Write exceeds max size.\n");
		return -EFBIG;
	}

	int idx = get_index_inode(path);
	if (idx < 0) {
		if ((idx = create_file(path, 0644, FS_FILE)) < 0)
			return idx;
		idx = get_index_inode(path);
	}

	if (idx == -1) {
		fprintf(stderr, "Error: File not found.\n");
		return -ENOENT;
	}

	struct inode *node = &super_b.inodes[idx];
	if (node->size < offset) {
		fprintf(stderr, "[Debug] Error write: %s\n", strerror(errno));
		errno = EINVAL;
		return -EINVAL;
	}

	if (node->type == FS_DIR) {
		fprintf(stderr, "Error: Cannot write in a Directory.\n");
		return -EACCES;
	}

	strncpy(node->content + offset, buffer, size);
	node->size = strlen(node->content);
	node->stats_info.last_mod = time(NULL);
	node->stats_info.last_acc = time(NULL);
	node->content[node->size] = '\0';

	return (int) size;
}

// Devuelve estadísticas del archivo/directorio (placeholder por ahora)
stats_t
get_stats(char *path)
{
	stats_t stats = {};
	return stats;
}

// ============================
// OPERACIONES SOBRE DIRECTORIOS
// ============================

int
create_dir(const char *path, mode_t mode)
{
	return 0;
}

char *
get_dir(char *path, mode_t mode)
{
	char *re = "";
	return re;
}

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
