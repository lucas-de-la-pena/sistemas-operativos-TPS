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
