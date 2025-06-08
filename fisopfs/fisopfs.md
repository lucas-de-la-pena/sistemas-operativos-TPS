# fisop-fs

### Representación del Sistema de Archivos

Para el desarrollo del trabajo práctico se estructura un sistema de archivos con inodos, representando así los archivos y directorios con los mismos. Basándonos en VSFS (very simple file system) también se implementa un super bloque almacenando información general del File System: Inodos y Bitmap de Inodos.

```c
struct super_block {
	struct inode inodes[MAX_INODES];
	int bitmap_inodes[MAX_INODES];
};
```

####  Estructuración de  los Inodos

La metadata de los archivos y directorios se guardan en los mismos. Cada inodo posee:
    - tipo (archivo o directorio) 
    - tamaño
    - el modo (permisos del inodo)
    - id del propietario y del grupo
    - tiempos de de acceso y de modificación
    - el path actual y del padre

La cantidad máximo de inodos son **64** su tamaño máximo es **1024**.

#### Bitmap Inodos
Es una estructura que permite saber qué inodos se encuentran libres y cuáles están ocupados. Nuestra implementacion utiliza cada bit como representación del inodo, en donde el 0 significa que está siendo utilizado y el 1 que está libre. El bitmap posee 64 bits de representación, siendo acorde a la cantidad máxima de nodos utlizada.
