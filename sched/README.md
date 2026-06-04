# sched — Scheduling y cambio de contexto

**Trabajo Práctico 2 — Sistemas Operativos (7508) - FIUBA**
Cátedra Méndez-Fresia

## Descripción

Este trabajo práctico implementa el mecanismo de **cambio de contexto** y el **planificador (scheduler)** sobre **JOS**, un exokernel educativo del MIT modificado para la materia. JOS corre sobre arquitectura Intel x86 emulada mediante **QEMU**.

El proyecto se divide en tres partes:

1. **Cambio de contexto** — Implementación del pasaje de kernel a usuario (`context_switch` con `iret`) y de usuario a kernel (`_alltraps` vía interrupciones).
2. **Scheduler Round Robin** — Planificador circular que distribuye el CPU equitativamente entre todos los procesos.
3. **Scheduler con Prioridades** — Planificador que asigna y respeta prioridades, con syscalls seguras para consultarlas y modificarlas, más estadísticas de scheduling.

## Estructura del proyecto

```
sched/
├── inc/          # Headers compartidos (Env, Trapframe, syscalls)
├── kern/         # Kernel: scheduling, traps, syscalls, init
│   ├── env.c     # PCB, creación/destrucción de procesos
│   ├── sched.c   # Planificador (round robin y prioridades)
│   ├── switch.S  # context_switch en assembler x86
│   ├── trap.c    # Manejo de interrupciones
│   ├── trapentry.S  # Handlers de interrupciones (_alltraps)
│   └── syscall.c # Syscalls
├── lib/          # Librería de usuario
├── user/         # Programas de usuario (hello, primes, etc.)
├── doc/          # Documentación
├── GNUmakefile   # Build system
└── dock          # Script helper para Docker
```

## Compilación

```bash
make
```

Por defecto se compila el scheduler **round-robin**.

### Compilación condicional

- **Round Robin:**
  ```bash
  make <target> USE_RR=1
  ```
- **Prioridades:**
  ```bash
  make <target> USE_PR=1
  ```

## Ejecución

```bash
make qemu           # Con ventana gráfica
make qemu-nox       # Sin ventana (terminal)
make run-<proceso>  # Ejecutar un proceso específico
```

Ejemplos:

```bash
make run-hello-nox
make run-primes-nox
```

## Depurado con GDB

**Terminal 1:**
```bash
make qemu-gdb
```

**Terminal 2:**
```bash
make gdb
```

### Triple fault

Si QEMU se reinicia constantemente, agregar al `GNUmakefile` en `QEMUOPTS`:

```
-no-reboot -no-shutdown -d cpu_reset
```

## Pruebas

```bash
make grade USE_RR=1   # Pruebas del scheduler round robin
make grade USE_PR=1   # Pruebas del scheduler con prioridades
```

## Partes implementadas

### Parte 1 — Cambio de contexto

- **`context_switch`** en `kern/switch.S`: restaura todos los registros del `Trapframe` y ejecuta `iret` para saltar a modo usuario.
- **`_alltraps`** en `kern/trapentry.S`: completa el `Trapframe` en el stack y llama a `trap()` para manejar interrupciones desde modo usuario.
- **`env_run`** en `kern/env.c`: orquesta el cambio de proceso: actualiza `curenv`, carga la tabla de páginas y llama a `context_switch`.

### Parte 2 — Round Robin

- **`sched_yield`** en `kern/sched.c`: itera circularmente sobre el arreglo de procesos (`envs`) y ejecuta el próximo proceso en estado `ENV_RUNNABLE`.

### Parte 3 — Prioridades

- Prioridad asignada en creación (`env_create`/`env_alloc`)
- Syscalls seguras: un proceso puede **bajar** su prioridad pero no **subirla**
- Las prioridades se heredan/computan en syscalls como `fork`
- Estadísticas de scheduling mostradas al finalizar (`sched_halt`): historial de ejecuciones, cantidad de llamadas al scheduler, conteo por proceso

## Informe teórico

Las decisiones de diseño, explicación del cambio de contexto, y detalles de la implementación con prioridades se encuentran en [`sched.md`](./sched.md).

---

*Proyecto realizado durante el primer cuatrimestre de 2025.*
