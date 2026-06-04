# fisopfs — File System FUSE

**Trabajo Práctico 3 — Sistemas Operativos (7508) - FIUBA**
Cátedra Méndez-Fresia

## Descripción

**fisopfs** es un sistema de archivos en espacio de usuario implementado sobre **FUSE (Filesystem in Userspace)**. Su funcionamiento es enteramente en memoria RAM durante la operación, y persiste su estado a disco en un archivo `.fisopfs` al desmontarse, permitiendo recuperar los datos en ejecuciones posteriores.

Está diseñado como un filesystem liviano tipo **VSFS (Very Simple File System)**, con una estructura plana basada en inodos, ideal para propósitos educativos.

## Funcionalidades implementadas

### Archivos
| Operación | Comando de ejemplo |
|-----------|-------------------|
| Creación | `touch archivo.txt` |
| Lectura | `cat archivo.txt` |
| Escritura (trunc) | `echo "hola" > archivo.txt` |
| Escritura (append) | `echo "mundo" >> archivo.txt` |
| Borrado | `rm archivo.txt` |

### Directorios
| Operación | Comando de ejemplo |
|-----------|-------------------|
| Creación | `mkdir directorio` |
| Lectura | `ls -al` (incluye `.` y `..`) |
| Borrado (vacío) | `rmdir directorio` |

### Metadatos
- Consulta de atributos con `stat`
- Fechas de último acceso y última modificación (`atime` / `mtime`)

## Estructura interna

El sistema de archivos está organizado en torno a un **superbloque** que contiene:

- **Hasta 64 inodos**, cada uno representando un archivo o directorio
- Un **bitmap de inodos** de 64 bits para tracking de espacio libre
- Cada inodo almacena tipo (archivo/directorio), tamaño (máx. 1024 bytes), modo, timestamps, path y directorio padre

### Persistencia

- Al montar: carga el estado desde el archivo especificado (default: `persistence_file.fisopfs`)
- En operación: todo vive en RAM
- Al desmontar (`.destroy`): escribe el superbloque completo a disco
- Soporta background con `-f` y paths absolutos para el archivo de persistencia

## Compilación

```bash
$ make
```

## Ejecución

```bash
$ mkdir prueba
$ ./fisopfs prueba/
```

Se puede especificar un archivo de persistencia custom:

```bash
$ ./fisopfs prueba/ --filedisk mi_disco.fisopfs
```

### Modo debug (primer plano)

```bash
$ ./fisopfs -f prueba/
```

### Limpieza

```bash
$ sudo umount prueba
```

## Pruebas

El proyecto incluye un script de pruebas automatizadas:

```bash
$ ./pruebas.sh
```

Las pruebas cubren: creación de archivos, lectura/escritura, appending, creación de directorios (incluyendo anidados), borrado de archivos y directorios vacíos, y verificación de persistencia entre montado y desmontado.

## Docker

```bash
$ make docker-build   # Construye imagen Ubuntu 20.04 con FUSE
$ make docker-run     # Inicia contenedor con bash
$ make docker-exec    # Conecta al contenedor en ejecución
```

## Formato de código

```bash
$ make format
```

## Informe teórico

Las decisiones de diseño, estructura de datos, formato de serialización y cuestiones teóricas se encuentran documentadas en [`fisopfs.md`](./fisopfs.md).

---

*Proyecto realizado durante el primer cuatrimestre de 2025.*
