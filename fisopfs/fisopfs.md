# fisop-fs

### Representación del Sistema de Archivos

Para el desarrollo del trabajo práctico se estructura un sistema de archivos con inodos, representando así los archivos y directorios con los mismos. Basándonos en VSFS (very simple file system) también se implementa un super bloque almacenando información general del File System: Inodos y Bitmap de Inodos.

```c
struct super_block {
	struct inode inodes[MAX_INODES];
	int bitmap_inodes[MAX_INODES];
};
```

![Estructura del File System](doc/estructura.png)

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

#### Busqueda de archivo dado un path
Para esta busqueda se implementa la funcion `get_index_inode(const char *path)` la cual se ocupa de buscar, a partir del path pasado por
parametro, el indice de ese inodo particular. Lo primero que hace es remover el "slash" que recibe en el path para luego compararlo con los
inodos que estan en la lista del super_block. De esta manera, se va recorriendo todos los inodos en memoria hasta encontrar el que
coincida con el path pasado por parametro. Si es que lo encuentra, devuelve el indice en el super bloque, de lo contrario devuelve -1.

#### Persistencia en disco
El sistema de archivos propuesto se mantiene en memoria gracias al archivo de nombre "fs.vfsimg" y al funcionamiento del comando `.destroy`, el cual se encarga de escribir el estado actual del super bloque en el archivo del disco. De esta manera se asegura de mantener los cambios entre ejecuciones y de que, al momento de inicializarse el sistema de archivos, se lea el archivo "fs.vfsimg" para poder volver a cargar el super bloque a memoria.

#### Pruebas

Para las pruebas proveemos el script `pruebas.sh` en el cual se verifica que nuestro filesystem funciona mediante las siguientes pruebas. 

A continuación dejamos la sálida de cada uno de los comandos en detalle pero se pueden verificar mirando la salida del script

- *Creación de archivos dentro del directorio de pruebas*
    ```
    touch "$MOUNT_DIR/hola.txt"
    ls -l "$MOUNT_DIR"
    total 0
    >>> -rw-r--r-- 1 tommy-facultad tommy-facultad 0 jun 15 11:53 hola.txt
    ```

- *Escribir y leer un archivo dentro del directorio de pruebas*
    ```
    echo -n "Hola Sistemas Operativos" > "$MOUNT_DIR/saludo.txt"
    cat "$MOUNT_DIR/saludo.txt"
    >>> Hola Sistemas Operativos
    ```
- *Agregar texto alfinal de un archivo que ya existe*
    ```
    echo -n " y FUSE!" >> "$MOUNT_DIR/saludo.txt"
    cat "$MOUNT_DIR/saludo.txt
    >>> Hola Sistemas Operativos y FUSE!
    ```

- *Creación de un directorio*
    ```
    mkdir "$MOUNT_DIR/docs"
    ls -l "$MOUNT_DIR"
    >>> drwxr-xr-x 2 tommy-facultad tommy-facultad  0 jun 15 11:59 docs
        -rw-r--r-- 1 tommy-facultad tommy-facultad  0 jun 15 11:53 hola.txt
        -rw-r--r-- 1 tommy-facultad tommy-facultad 32 jun 15 11:58 saludo.txt
    ```

- *Creación de un directorio anidado*
    ```
    mkdir "$MOUNT_DIR/docs/personales"
    touch "$MOUNT_DIR/docs/personales/cv.pdf"
    ls -l "$MOUNT_DIR/docs"
    >>> drwxr-xr-x 2 tommy-facultad tommy-facultad 0 jun 15 12:03 personales
    ls -l "$MOUNT_DIR/docs/personales"
    >>> -rw-r--r-- 1 tommy-facultad tommy-facultad 0 jun 15 12:03 cv.pdf
    ```

- *Eliminar un archivo*
   ```
   rm "$MOUNT_DIR/hola.txt"
   ls -l "$MOUNT_DIR"
   >>> drwxr-xr-x 2 tommy-facultad tommy-facultad  0 jun 15 11:59 docs
       -rw-r--r-- 1 tommy-facultad tommy-facultad 32 jun 15 11:58 saludo.txt
   ```

- *Eliminar un directorio vacío*
   ```
    mkdir "$MOUNT_DIR/vacio"
    rmdir "$MOUNT_DIR/vacio"
    ls -l "$MOUNT_DIR"
    >>> drwxr-xr-x 2 tommy-facultad tommy-facultad  0 jun 15 11:59 docs
        -rw-r--r-- 1 tommy-facultad tommy-facultad 32 jun 15 11:58 saludo.txt
   ```

- *Eliminar un directorio con contenido no se debería poder*
    ```
    rmdir "$MOUNT_DIR/docs
    >>> rmdir: fallo al borrar './prueba/docs': El directorio no está vacío
    ```

- *Persistencia*
    ```
    umount "$MOUNT_DIR" || true
    rm -rf "$MOUNT_DIR"
    mkdir -p "$MOUNT_DIR"
    ./fisopfs "$MOUNT_DIR" --filedisk "$DISK_FILE"
    ls $MOUNT_DIR
    >>> drwxr-xr-x 2 tommy-facultad tommy-facultad  0 jun 15 11:59 docs
        -rw-r--r-- 1 tommy-facultad tommy-facultad 32 jun 15 11:58 saludo.txt
    cat "$MOUNT_DIR/saludo.txt"
    >>> Hola Sistemas Operativos y FUSE!
    ```

También se puede verificar el contenido del archivo `test_output.log` generado al ejecutar el script mencionado anteriormente.
