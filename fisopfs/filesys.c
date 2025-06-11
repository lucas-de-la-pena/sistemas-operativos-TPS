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
// OPERACIONES GENERICAS
// ============================

// Crea un nodo genérico (archivo o directorio)
int
create_node(const char *path, mode_t mode, int type)
{
	if (strlen(path) - 1 > MAX_CONTENT) {
		fprintf(stderr,
		        "[Debug] Error create_node: %s\n",
		        strerror(ENAMETOOLONG));
		errno = ENAMETOOLONG;
		return -ENAMETOOLONG;
	}

	// Normalizar el path
	char *clean_path = remove_slash(path);
	if (!clean_path)
		return -1;

	// Buscar índice de inodo libre
	int inode_index = next_free_inodo(clean_path);
	if (inode_index < 0) {
		free(clean_path);
		return inode_index;
	}

	// Inicializar nuevo inodo
	struct inode node;
	node.type = type;
	node.mode = mode;
	node.size = 0;
	node.id_user = getuid();
	node.id_grup = getgid();
	node.stats_info.creation = time(NULL);
	node.stats_info.last_acc = node.stats_info.creation;
	node.stats_info.last_mod = node.stats_info.creation;
	strcpy(node.path, clean_path);

	// Obtener directorio padre
	char parent_path[MAX_PATH];
	size_t len = strlen(path) - 1;
	memcpy(parent_path, path + 1, len);
	parent_path[len] = '\0';
	get_path_parent(parent_path);

	strcpy(node.directory_path, parent_path);

	// Inicializar contenido del inodo
	memset(node.content, 0, sizeof(node.content));

	// Guardar en el sistema de archivos
	super_b.inodes[inode_index] = node;
	super_b.bitmap_inodes[inode_index] = 1;

	free(clean_path);
	return 0;
}

int
delete_inode(const char *path, int expected_type)
{
	int idx = get_index_inodo(path);
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

	// Si es un directorio, verifico que esté vacío
	if (expected_type == FS_DIR) {
		char *normalized_path = remove_slash(path);
		if (!normalized_path)
			return -ENOMEM;

		for (int i = 0; i < MAX_INODES; i++) {
			if (strcmp(super_b.inodes[i].directory_path,
			           normalized_path) == 0) {
				free(normalized_path);
				return -ENOTEMPTY;
			}
		}
		free(normalized_path);
	}

	// Limpio el inodo
	super_b.bitmap_inodes[idx] = 0;
	memset(&super_b.inodes[idx], 0, sizeof(struct inode));
	return 0;
}


// ============================
// OPERACIONES SOBRE ARCHIVOS
// ============================

int
read_file(struct inode *in, char *buffer, size_t size, off_t offset)
{
	if (offset < 0 || size < 0)
		return -EINVAL;

	// Si el offset es igual o mayor al tamaño, no hay más datos que leer
	if (offset >= in->size)
		return 0;

	// Calculo cuantos bytes se pueden leer sin pasarse del tamaño del archivo
	size_t to_read = (in->size - offset > size) ? size : in->size - offset;

	memcpy(buffer, in->content + offset, to_read);

	// Actualizo la última fecha de acceso
	in->stats_info.last_acc = time(NULL);

	return to_read;
}

// Saca la primera barra del path y devuelve solo el nombre del archivo/directorio.
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
	if (last) {
		if (last == path_parent) {
			// Caso especial: el padre es la raíz "/"
			*(last + 1) = '\0';  // Dejo solo "/"
		} else {
			*last = '\0';  // Corta después del último '/'
		}
	} else {
		// No hay '/' en el path, entonces el padre es root
		strcpy(path_parent, "/");
	}
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

// Crea un archivo o directorio nuevo y lo guarda como inodo.
// Errores posibles:
//- ENAMETOOLONG: nombre demasiado largo
//- ENOSPC: sin espacio
//- EEXIST: ya existe
int
create_file(const char *path, mode_t mode)
{
	return create_node(path, mode, FS_FILE);
}

// Elimina un archivo (si existe y no es un directorio).
int
delete_file(char *path)
{
	return delete_inode(path, FS_FILE);
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
		if ((idx = create_file(path, 0644)) < 0)
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
	return create_node(path, mode, FS_DIR);
}

// Elimina directorio dado su path
int
unlink(const char *path)
{
	int idx = get_index_inodo(path);
	if (idx < 0)
		return -1;

	// Si es un directorio, no se puede eliminar con unlink
	if (super_b.inodes[idx].type == FS_DIR)
		return -EISDIR;

	// Limpio contenido del inodo
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