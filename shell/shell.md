# shell

### Búsqueda en $PATH
#### ¿cuáles son las diferencias entre la syscall execve(2) y la familia de wrappers proporcionados por la librería estándar de C (libc) exec(3)?
La familia de wrappers `exec(3)` incluye funciones (**execl**, **execp**, **execv**, etc.) que internamente terminan llamando a la Syscall `execve(2)`. Estas versiones que proporciona libc son más amigables ya que algunas buscan el ejecutable en las rutas definidas por `$PATH` (como **execvp**), y la mayoría heredan de manera automática el environment del proceso que las invoca.

Por otra parte, `execve(2)` es la interfaz de "bajo nivel" dado que requiere le pasen explícitamente tanto la ruta del ejecutable como los argumentos y el entorno. No busca en `$PATH` y tampoco asume el entorno actual si no se lo pasa manualmente.

#### ¿Puede la llamada a exec(3) fallar? ¿Cómo se comporta la implementación de la shell en ese caso?
Sí, la llamada a `exec(3)` puede fallar en caso de error, ya sea por los parámetros inválidos o por fallo de ejecución del nuevo programa solicitado. 
Dentro de nuestra implementación, en caso de error, se muestra un mensaje con `perror()`. Luego se libera los recursos utilizados por comando y se finaliza la ejecución con una salida indicando un **error** (distinta de 0).
```c
		int op_result = execvp(e->argv[0], e->argv);
		if (op_result < 0) {
			perror("ERROR: Execvp failed on Exec call");
			free_command(cmd);
			exit(-1);
		}
```

---

### Procesos en segundo plano

---

### Flujo estándar

---

### Tuberías múltiples

---

### Variables de entorno temporarias

---

### Pseudo-variables

---

### Comandos built-in

---

### Historial

---
