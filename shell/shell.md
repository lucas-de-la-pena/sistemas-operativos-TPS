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

#### ¿Cuál es el significado de 2>&1?

Indica que va a redireccionar el file descriptor de STDERR hacia el file descriptor de STDOUT, entonces todo lo que se escriba en el FD de STDERR, será redireccionado hacia STDOUT


#### ¿Que sucede con la sálida de `cat out.txt` en el ejemplo?

En el ejemplo lo que sucede es que se va a rediccionar la sálida de STDERR hacía el archivo `out.txt` por lo que en ese archivo
no solo veríamos el listado de los archivos del directorio, sino que también veríamos el error de que el directorio `/noexiste` no existe como tal.

Esta es la sálida de nuestra shell
```
cat out.txt
ls: no se puede acceder a '/noexiste': No existe el archivo o el directorio
/home:
linuxbrew  tommy  tommy-facultad
```

#### ¿Qué pasa si invertimos el orden de las redirecciones?

Si invertimos el orden de las redirecciones, es decir, `ls -C /home /noexiste 2>&1 >out.txt`, lo que sucede es primero vamos a redireccionar STDERR
hacia el primer file descriptor de STDOUT, es decir, no será redigirido hacía `out.txt`, sino hacia STDOUT que sería la sálida de la terminal.

#### 


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
