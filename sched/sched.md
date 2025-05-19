# sched

## Cambio de Contexto
Utilizar GDB para visualizar el cambio de contexto. Realizar una captura donde se muestre claramente:
- el cambio de contexto
- el estado del stack al inicio de la llamada de context_switch
- cómo cambia el stack instrucción a instrucción
- cómo se modifican los registros luego de ejecutar iret

### Llamado a context_switch()

![Registros al entrar](./doc/1-RegsAlEntrar.png)

Registros al entrar.

![Alineacion de stack](./doc/2-AlineoStack.png)

En este punto, dado que la función `context_switch` no retorna, no es necesario restaurar la dirección de retorno dejada por la llamada a `env_run`. Por lo que se puede acceder directamente al puntero del trapframe, cargado en el stack. 

![Accedo al struct](./doc/3-AccedoStruct.png)

Accedo al Struct **trapframe**.

![Restauro regs](./doc/4-RestauroLosRegistros.png)

Restauro registros generales.

![Restauro es](./doc/5-ResES.png)

Restauro registro `%es`.

![Restauro ds](./doc/6-ResDS.png)

Restauro registro `%ds`.

![Droppeo valores innecesarios](./doc/7-DropValoresInnecesarios.png)

Droppeo los registros innecesarios del stack (**trapno**, **err**).

![Llamo a iret](./doc/8-LlamoIret.png)

Se invoca `Iret` con el stack teniendo los registros (`eip`, `cs`, `eflags`, `esp` y `ss`)-

![Registros despues de Iret](./doc/9-RegsDpsIret.png)

Registros luego de la llamada a Iret.

## Informe: Política de Planificación por Prioridades

### Objetivo del Planificador

El sistema implementa un planificador de procesos basado en prioridades. Su objetivo es seleccionar el proceso más prioritario que se encuentre listo para su ejecución.

### Esquema de Prioridades

- Prioridad Inicial: Todos los procesos comienzan con un valor numérico de prioridad de 10 al ser creados.
- Ajuste Dinámico de Prioridad: Cada vez que un proceso se ejecuta, su valor numérico de prioridad se incrementa en 1.
- Significado de la Prioridad: Un valor numérico de prioridad menor implica una mayor urgencia de ejecución. Por lo tanto, a medida que un proceso consume tiempo de CPU y su valor numérico de prioridad aumenta, su precedencia en la planificación disminuye gradualmente
- Este mecanismo introduce un factor de disminución de la prioridad relativa con el uso.

### Algoritmo de Selección (sched_yield)

La función sched_yield implementa la lógica de selección de la siguiente manera:

- Identificación de la Mayor Prioridad: El planificador examina todos los procesos en estado ENV_RUNNABLE para determinar el valor numérico de prioridad más bajo (es decir, la prioridad efectiva más alta) presente en el sistema.
- Selección del Proceso: Una vez identificada la mejor prioridad, el planificador elige el primer proceso ENV_RUNNABLE que posea dicho valor de prioridad. La búsqueda se realiza de manera circular a través de la lista de procesos, comenzando después del último proceso que estuvo en ejecución.
- Manejo de Empates: Si varios procesos comparten la misma prioridad más alta, el criterio de selección circular (escoger el primero encontrado) resulta en una política de reparto equitativo (similar a Round Robin) entre estos procesos de igual prioridad.
- Despacho: El proceso seleccionado es luego ejecutado mediante env_run(). Si no se encuentra ningún proceso listo para ejecutar, se invoca a sched_halt().


