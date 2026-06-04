# Sistemas Operativos — FIUBA

**Cátedra Méndez-Fresia**

Este repositorio contiene los trabajos prácticos realizados en la materia **Sistemas Operativos (7508)** de la **Facultad de Ingeniería de la Universidad de Buenos Aires**.

Cada rama del repositorio alberga uno de los tres trabajos prácticos grupales:

| Rama | Proyecto | Descripción |
|------|----------|-------------|
| [`entrega_shell`](https://github.com/fiubatps/sisop_2025a_g59/tree/entrega_shell) | **Shell** | Implementación de una shell Unix en C con soporte para pipes, redirecciones, variables de entorno, ejecución en background y comandos built-in. |
| [`entrega_sched`](https://github.com/fiubatps/sisop_2025a_g59/tree/entrega_sched) | **Scheduler** | Planificador de procesos estilo xv6, implementando distintas políticas de scheduling (Round Robin, prioridades, etc.) con mediciones de rendimiento. |
| [`entrega_fs`](https://github.com/fiubatps/sisop_2025a_g59/tree/entrega_fs) | **File System** | Sistema de archivos en espacio de usuario implementado con FUSE, basado en inodos con persistencia a disco y soporte para operaciones básicas del sistema de archivos. |

---

## Estructura del repositorio

- La rama `main` contiene este archivo y la configuración compartida del repositorio.
- Cada trabajo práctico se desarrolla y entrega en su propia rama independiente, con su propio código fuente, pruebas automatizadas y documentación.
- Cada rama incluye un `README.md` detallado con instrucciones de compilación, ejecución y testing.

---

## Tecnologías utilizadas

- **Lenguaje:** C (estándares GNU11 y C11)
- **Herramientas:** GCC, Make, Docker, FUSE, xv6
- **Testing:** Pruebas automatizadas con YAML, Bash scripts y test runners custom

---

## Autores

Trabajos prácticos realizados en grupo durante el primer cuatrimestre de 2025.
