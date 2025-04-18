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
#### Breve explicación del mecanismo utilizado
Primero se crea un **stack alternativo**, el cual permite que el **handler** se ejecute en una pila distinta de la principal. Útil en caso de fallos, ya que evita interferencias con los datos del stack original.
Luego, se configura un `sigaction` que asigna el **handler** a la señal `SIGCHLD`. Este handler se ejecutará en el **stack alternativo** y, gracias a la flag `SA_RESTART`, reiniciará automáticamente cualquier **syscall** que haya sido interrumpida por la señal.

Finalmente, se implementa la función `handler_bp`, asociada a la `sigaction`, con el objetivo de manejar los recursos de los procesos en segundo plano. Esta función es llamada cuando un proceso hijo de tipo **background** finaliza, notificando al usuario de manera inmediata junto con su **PID**. 

La obtención del PID se realiza mediante `waitpid` con la flag `WNOHANG`, lo que permite un llamado a esta función **no bloqueante**. Además, en esta implementación, los procesos que no son de tipo **background** tienen un **group ID distinto al del padre**, lo que evita que el handler se active innecesariamente.


#### ¿Por qué es necesario el uso de señales?
Cuando un proceso hijo finaliza, el sistema operativo envía una señal `SIGCHLD` al proceso padre para notificarlo de dicho evento. La utilización de la misma es esencial para que el padre se entere cuándo termina uno de estos procesos sin necesidad de esperarlos de forma bloqueante, especialmente útil para una **shell** que permite ejecutar **procesos en segundo plano**.
Por lo tanto, utilizar señales permite que la **shell** siga interactuando con el usuario mientras los procesos **background** se ejecutan. Al capturar `SIGCHLD` con un **handler** mediante `sigaction`, la **shell** puede ejecutar `waitpid` con la flag `WNOHANG`, evitando así  que el proceso padre se bloquee esperando (y seguir así su hilo de ejecución).

En el caso de este trabajo práctico, se utiliza el **handler** para mostrarle al usuario apenas un background process finaliza un mensaje del mismo junto a a su  **PID**.
Asimismo, se usan las flags `SA_ONSTACK` y `SA_RESTART` que permiten que el **handler** se ejecute en una pila alternativa y que si la señal interrumpe una **syscall**, esta sea reiniciada automáticamente.
Esto mejora la experiencia del usuario, permitiendo que la **shell** sea reactivada sin perder información sobre los **procesos en segundo plano**, como también el manejo de recursos e información de manera **instantánea** una vez que estos finalicen.

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
