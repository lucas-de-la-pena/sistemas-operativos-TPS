# shell — Intérprete de comandos

**Trabajo Práctico 1 — Sistemas Operativos (7508) - FIUBA**
Cátedra Méndez-Fresia

## Descripción

Implementación de un intérprete de comandos **shell** en C11 / POSIX.1-2008, similar a bash, zsh o fish. Soportando ejecución de binarios con búsqueda en `$PATH`, redirecciones, pipes, variables de entorno, comandos built-in y procesos en segundo plano.

## Funcionalidades implementadas

### Parte 1 — Invocación de comandos
- Ejecución de binarios con búsqueda en `$PATH` vía `execvp(3)`
- Soportes argumentos
- La shell espera correctamente a que los programas terminen

### Parte 2 — Redirecciones y pipes
- Redirección de entrada (`< archivo`)
- Redirección de salida (`> archivo`, trunc)
- Redirección de error (`2> archivo`)
- Combinación de salida y error (`2>&1`)
- Tuberías simples (`|`) entre dos comandos
- Tuberías múltiples (n comandos concatenados con `|`)
- Los pipes se lanzan en simultáneo, sin leaks de file descriptors

### Parte 3 — Variables de entorno
- Expansión de variables (`$PATH`, `$HOME`, etc.)
- Variables no definidas → cadena vacía
- Variables de entorno temporales (`USER=nadie cmd`)
- Pseudo-variable `$?` (código de salida del último comando)

### Parte 4 — Comandos built-in
- `cd` — cambiar directorio actual
- `exit` — finalizar la shell
- `pwd` — mostrar directorio actual

### Parte 5 — Procesos en segundo plano
- Ejecución en background con `&`
- Notificación asíncrona al terminar mediante `SIGCHLD`
- Manejo de grupos de procesos para distinguir foreground de background

## Estructura del proyecto

```
shell/
├── sh.c            # Punto de entrada, loop principal
├── defs.h          # Constantes y tipos base
├── types.h         # Structs: cmd, execcmd, pipecmd, backcmd
├── parsing.c/h     # Parsing, expansión de variables
├── exec.c/h        # Ejecución de comandos, redirecciones, pipes
├── runcmd.c/h      # Orchestrador de comandos
├── builtin.c/h     # Comandos built-in (cd, exit, pwd)
├── createcmd.c/h   # Factory de estructuras cmd
├── freecmd.c/h     # Liberación de memoria
├── readline.c/h    # Lectura de línea
├── printstatus.c/h # Reporte de estado de procesos
├── utils.c/h       # Utilidades (printf_debug, etc.)
├── tests/          # 30 pruebas YAML automatizadas
├── Makefile
└── Dockerfile
```

## Compilación

```bash
make
```

## Ejecución

```bash
./sh                    # Modo interactivo
make run                # ídem
make valgrind           # Ejecutar con valgrind
```

## Pruebas

```bash
make test               # Todas las pruebas
make test-NOMBRE        # Una prueba específica
```

Ejemplo:
```bash
make test-pipes_generate_correct_output
```

Las pruebas cubren: ejecución simple, `cd`, `pwd`, variables de entorno, redirecciones (stdout, stdin, stderr, `2>&1`), pipes, y detección de leaks de file descriptors.

## Informe teórico

Las respuestas a las preguntas teóricas de cada parte se encuentran en [`shell.md`](./shell.md), incluyendo:
- Diferencias entre `execve(2)` y `exec(3)`
- Comportamiento ante fallos de `exec`
- Redirecciones y orden de evaluación
- Variables de entorno temporales vs `execve`
- Variables mágicas (`$?`, `$0`, `$$`, `$#`)
- Built-ins: por qué `cd` debe ser interno
- Manejo de señales y procesos en background

---

*Proyecto realizado durante el primer cuatrimestre de 2025.*
