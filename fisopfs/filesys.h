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

#define FUSE_USE_VERSION 30
#define DEFAULT_FILE_DISK "persistence_file.fisopfs"

// Definiciones de la estructura de datos del sistema de archivos
#define MAX_INODES 64
#define SAVE_FILE "fs.fisopsfs"
#define MAX_DIR_SIZE 1024
#define MAX_PATH 200
#define MAX_CONTENT 1024
#define ROOT_PATH "/"

// Modos de cada Inodo
#define MODE_FILE (__S_IFREG | 0644)  // Archivo regular
#define MODE_DIR (__S_IFDIR | 0755)   // Directorio


//-----------------------------------
// Enumeraciones y tipos de datos
//-----------------------------------

typedef enum inode_type {
	FS_DIR,  // Directorio
	FS_FILE  // Archivo regular
} inode_type;

// Información temporal del archivo/directorio
typedef struct stats {
	time_t last_acc;  // Ultimo acceso
	time_t last_mod;  // Ultima modificación
	time_t creation;  // Fecha de creación
} stats_t;

//-----------------------------------
// Estructuras principales
//-----------------------------------
struct inode {
	inode_type type;            // Tipo de inodo (archivo o directorio)
	size_t size;                // Tamaño en bytes
	uid_t id_user;              // ID del usuario propietario
	gid_t id_grup;              // ID del grupo propietario
	mode_t mode;                // Permisos (lectura/escritura/ejecución)
	stats_t stats_info;         // Información temporal (timestamps)
	nlink_t link_num;           // Cantidad de enlaces duros
	char path[MAX_PATH];        // Ruta absoluta
	char content[MAX_CONTENT];  // Contenido (solo para archivos)
	char directory_path[MAX_PATH];  // Ruta al directorio padre (si tiene)
};

// Bloque de control del sistema de archivos
struct super_block {
	struct inode inodes[MAX_INODES];  // Tabla de inodos
	int bitmap_inodes[MAX_INODES];    // Bitmap de inodos usados
};

extern struct super_block super_b;


//-----------------------------------
// Utilidades
//-----------------------------------

// Devuelve el índice del inodo asociado a la ruta, o -1 si no existe
int get_index_inode(const char *path);

// Elimina un '/' final si lo hay
char *remove_slash(const char *path);

// Obtiene el path del directorio padre a partir de un path
void get_path_parent(char *path_parent);

// Busca el siguiente inodo libre para una nueva entrada
int next_free_inode(const char *path);


//-----------------------------------
// Inicializacion y persistencia
//-----------------------------------

// Inicializa el sistema de archivos
int initialize_fs();

// Guarda el estado actual del sistema en un archivo
int save_fs(char *save_file);


//-----------------------------------
// Operaciones sobre archivos
//-----------------------------------

// Lee el contenido de un archivo
int read_file(char *path);

// Crea un archivo nuevo
int create_file(const char *path, mode_t mode, int type);

// Elimina un archivo existente
int delete_file(char *path);

// Escribe datos en un archivo con desplazamiento
int write_file(const char *path, const char *buffer, size_t size, off_t offset);

// Obtiene los metadatos (stats) de un archivo
stats_t get_stats(char *path);


//-----------------------------------
// Operaciones sobre directorios
//-----------------------------------

// Crea un directorio
int create_dir(const char *path, mode_t mode);

// Devuelve el contenido de un directorio
char *get_dir(char *path, mode_t mode);

// Elimina un directorio
int delete_dir(const char *path);

// Lista el contenido de un directorio
int list_dir(char *path);


//-----------------------------------
// Otras operaciones
//-----------------------------------

// Elimina un enlace a un archivo/directorio
int unlink(const char *path);

#endif  // FILESYSTEM_H