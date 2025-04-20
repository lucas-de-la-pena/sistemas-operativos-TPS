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

#### ¿Qué ocurre con el exit code reportado por la shell si se ejecuta un pipe?

- ¿Cambia en algo?

Sí, por lo general cuando ejecutamos un comando de manera individual, vamos a recibir un exit code en base a lo que suceda durante la ejecución del programa.
Lo que cambia cuando se utilizan pipes, es que el exit code reportado pro la shell es el exit code el ÚLTIMO comando ejecutado.

- ¿Qué ocurre si, en un pipe, alguno de los comandos falla?

Si alguno de los comandos falla, el exit code del pipe va a ser el exit code del ÚLTIMO comando ejecutado en el pipe, porque dentro del pipeline, no importa si alguna ejecución de los comandos falla, la cadena de comandos se sigue ejecutando, con el problema de que van a haber comandos que no reciban entrada de datos por STDIN. Podemos ver el exit code del pipeline utilizando `echo $?`.
Por ejemplo:

```bash
> ls | grep | wc
Modo de empleo: grep [OPCIÓN]... PATRONES [FICHERO]...
Pruebe 'grep --help' para más información.
      0       0       0
> echo "Exit code: $?"
> Exit code: 0
```

Si bien el comando grep falló por falta de argumentos, el exit code fue 0.

Ahora, si quisieramos quedarnos con el exit code del último comando que falló, podemos hacerlo levantando una flag llamada `pipefail`

```bash
> set -o pipefail
> ls | grep | wc
Modo de empleo: grep [OPCIÓN]... PATRONES [FICHERO]...
Pruebe 'grep --help' para más información.
      0       0       0
> echo "Exit code: $?"
> Exit code: 2
```

De esa forma podemos obtener el exit code del último comando que falló. Y si ninguno falla, el exit code será 0.


---

### Tuberías múltiples

---

### Variables de entorno temporarias

#### ¿Por qué es necesario hacerlo luego de la llamada a fork(2)?

Es necesario hacerlo despues del fork ya que la intencion que tenemos es que las variables de entorno temporales existan solo en
el proceso hijo. De esta forma las separamos de las variables entorno que viven en la shell (el proceso padre), las cuales tienen
otras caracteristicas

#### ¿El comportamiento resultante es el mismo que en el primer caso? Explicar qué sucede y por qué.

El comportamiento resultante no es el mismo. La diferencia principal es que en el primer caso, al usar setenv y despues exec, el entorno se hereda automaticamente, mientras que con un execve por ejemplo, se pasa por alto el proceso actual y lo reemplaza, solamente usando las variables de entorno pasadas por parametro e ignorando todas las demas.

#### Describir brevemente (sin implementar) una posible implementación para que el comportamiento sea el mismo.

Una posible implementacion podria copiar las variables de todo el entorno en un arreglo para despues pasarla por parametro a la syscall exec(3) que termina con "e". De esta manera se incluirian en el nuevo entorno todas las variables de entorno previamente planteadas y las nuevas que se quieran agregar.


### Pseudo-variables

#### Investigar al menos otras tres variables mágicas estándar, y describir su propósito.

Existe la variable magica $0 que contiene el nombre del script que se esta ejecutando actualmente. Por ejemplo en bash un programa que se llama "nombre.sh" :
```bash
#!/bin/bash
echo "El nombre de este script es $0"
```
Este codigo va a imprimir por terminal "El nombre de este script es nombre.sh"

Existe la variable magica $# que contiene la cantidad de argumentos pasados al script. Por ejemplo, un programa en bash como el siguiente:
 ```bash
#!/bin/bash
echo "La cantidad de argumentos que se pasaron fueron $#"
```
Va a imprimir "La cantidad de argumentos que se pasaron fueron 2"

Existe la variable magica $$ que contiene el PID del proceso actual. Por ejemplo, un programa en bash como el siguiente:
 ```bash
#!/bin/bash
echo "El PID de este proceso es $$"
```
Va a imprimir "El PID de este proceso es 12345" (considerando que 12345 siendo el PID del Shell actual).
### Comandos built-in

En la práctica, **`pwd`** podría vivir perfectamente como un programa externo (por ejemplo `/bin/pwd`), porque solo lee y muestra el directorio actual sin tocar nada del proceso de la shell. En cambio, **`cd`** no puede, porque cambiar de directorio tiene que afectar al proceso de la shell, y eso solo se puede hacer si el código se ejecuta en el mismo proceso.

El comando **`pwd`** se implementa como built‐in para que esté siempre disponible, sea más rápido al evitar el fork() + execve() cada vez que se invoca y no dependa de versiones o permisos en el $PATH.

### Historial

---
