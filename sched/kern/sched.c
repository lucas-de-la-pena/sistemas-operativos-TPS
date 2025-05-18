#include <inc/assert.h>
#include <inc/x86.h>
#include <kern/spinlock.h>
#include <kern/env.h>
#include <kern/pmap.h>
#include <kern/monitor.h>

void sched_halt(void);

uint32_t total_sched_yield_calls = 0;

void stats_init(void) {
    total_sched_yield_calls = 0;
    // Limpiamos el historial por si las moscas
    cprintf("Sistema de estadisticas del scheduler listo!\n");
}

// stats_display: Muestra las estadisticas recolectadas.
void stats_display(void) {
    cprintf("\n-------------------------------------------------------\n");
    cprintf("--- Estadisticas del Scheduler ---\n");
    cprintf("-------------------------------------------------------\n");

    // 1. Total de llamadas a sched_yield
    cprintf("Total de veces que se llamo al scheduler (sched_yield): %u\n", total_sched_yield_calls);

    // 2. Numero de ejecuciones por proceso (y su prioridad actual)
    cprintf("\n--- Resumen por Proceso (estado actual) ---\n");
    cprintf("ID Proceso | Prioridad | Veces Ejecutado\n");
    cprintf("-----------|-----------|----------------\n");
    int procesos_activos = 0;
    for (int i = 0; i < NENV; i++) {
        if (envs[i].env_status != ENV_FREE) { // Solo si el proceso "existe"
            cprintf("0x%08x | %-9d | %-15u\n",
                    envs[i].env_id,
                    envs[i].env_priority, // Tomamos la prioridad actual
                    envs[i].env_runs);    // El contador de ejecuciones que ya tiene JOS
            procesos_activos++;
        }
    }
    if (procesos_activos == 0) {
        cprintf(" (No hay procesos activos o creados para mostrar stats)\n");
    }
    cprintf("-------------------------------------------------------\n");
}

// Choose a user environment to run and run it.
void
sched_yield(void)
{
#ifdef SCHED_ROUND_ROBIN
	// Implement simple round-robin scheduling.
	//
	// Search through 'envs' for an ENV_RUNNABLE environment in
	// circular fashion starting just after the env this CPU was
	// last running. Switch to the first such environment found.
	//
	// If no envs are runnable, but the environment previously
	// running on this CPU is still ENV_RUNNING, it's okay to
	// choose that environment.
	//
	// Never choose an environment that's currently running on
	// another CPU (env_status == ENV_RUNNING). If there are
	// no runnable environments, simply drop through to the code
	// below to halt the cpu.

	// Your code here - Round robin

	int i, start;
	start = curenv ? ENVX(curenv->env_id) + 1 : 0;

	for (i = 0; i < NENV; i++) {
    	int idx = (start + i) % NENV;
    	if (envs[idx].env_status == ENV_RUNNABLE) {
        	env_run(&envs[idx]);
    	}
	}

	if (curenv && curenv->env_status == ENV_RUNNING &&
    	curenv->env_cpunum == thiscpu->cpu_id) {
    	env_run(curenv);
	}

	// No hay entornos para correr, así que detenemos la CPU
	sched_halt();

#endif

#ifdef SCHED_PRIORITIES
	total_sched_yield_calls++;
    struct Env *chosen_env = NULL;
    int best_priority = 0x7FFFFFFF;
    int start_idx = 0;

    if (thiscpu->cpu_env) // buscamos el siguiente entorno a ejecutar a partir del actual, si no existe proceso corriendo se empieza desde 0
        start_idx = (ENVX(thiscpu->cpu_env->env_id) + 1) % NENV; //arranco desde el siguiente al proceso actual, pongo el modulo para que no se pase de NENV

    // encontramos la mejor prioridad
    for (int i = 0; i < NENV; i++) {
        int idx = (start_idx + i) % NENV;
        struct Env *e = &envs[idx];
        if (e->env_status == ENV_RUNNABLE && e->env_priority < best_priority)
            best_priority = e->env_priority;
    }

    // elegimos el primer runnable con esa prioridad
    for (int i = 0; i < NENV; i++) {
        int idx = (start_idx + i) % NENV;
        struct Env *e = &envs[idx];
        if (e->env_status == ENV_RUNNABLE && e->env_priority == best_priority) {
            chosen_env = e;
            break;
        }
    }

    if (chosen_env) {
		chosen_env->env_priority++;
		env_run(chosen_env);
	}

    if (!chosen_env && curenv && curenv->env_status == ENV_RUNNING &&
        curenv->env_cpunum == thiscpu->cpu_id) {
        env_run(curenv); 
    }

    sched_halt();
#endif

	// Without scheduler, keep runing the last environment while it exists
	if (curenv) {
		env_run(curenv);
	}

	// sched_halt never returns
	sched_halt();
}

// Halt this CPU when there is nothing to do. Wait until the
// timer interrupt wakes it up. This function never returns.
//
void
sched_halt(void)
{
	int i;

	// For debugging and testing purposes, if there are no runnable
	// environments in the system, then drop into the kernel monitor.
	for (i = 0; i < NENV; i++) {
		if ((envs[i].env_status == ENV_RUNNABLE ||
		     envs[i].env_status == ENV_RUNNING ||
		     envs[i].env_status == ENV_DYING))
			break;
	}
	if (i == NENV) {
		cprintf("No runnable environments in the system!\n");
		while (1)
			monitor(NULL);
	}

	// Mark that no environment is running on this CPU
	curenv = NULL;
	lcr3(PADDR(kern_pgdir));

	// Mark that this CPU is in the HALT state, so that when
	// timer interupts come in, we know we should re-acquire the
	// big kernel lock
	xchg(&thiscpu->cpu_status, CPU_HALTED);

	// Release the big kernel lock as if we were "leaving" the kernel
	unlock_kernel();

	// Once the scheduler has finishied it's work, print statistics on
	// performance. Your code here

	// Reset stack pointer, enable interrupts and then halt.
	asm volatile("movl $0, %%ebp\n"
	             "movl %0, %%esp\n"
	             "pushl $0\n"
	             "pushl $0\n"
	             "sti\n"
	             "1:\n"
	             "hlt\n"
	             "jmp 1b\n"
	             :
	             : "a"(thiscpu->cpu_ts.ts_esp0));
}
