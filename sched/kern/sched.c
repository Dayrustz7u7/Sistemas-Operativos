#include <inc/assert.h>
#include <inc/x86.h>
#include <kern/spinlock.h>
#include <kern/env.h>
#include <kern/pmap.h>
#include <kern/monitor.h>

void sched_halt(void); 

// Choose a user environment to run and run it.
void
sched_yield(void)
{
	struct Env *other; 

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
	
	int index = 0;  /// i que uso tomas 
	
	if (curenv) {  // Primero me fijo si hay un env corriendo actualmente
		index = ENVX(curenv->env_id);  /// Esto obtiene el indice del actual (parece)
	}

	for( int i = index; index < NENV; index ++ ) {  // Mientras que el proximo no sea nulo.
		other = &envs[index];
		if (other->env_status == ENV_RUNNABLE) {
			// env_run(other);  // Si el proximo se puede correr, lo corro.
			curenv = other; 
			break;
		}
		cprintf("indice: %d\n", index); 
		index ++; 
	}

	// while (&envs[index] !=
	// 		curenv) {  // Ya vi todos los elementos proximos al actual, ahora veo los previos.
	// 	e = &envs[index];
	// 	if (e->env_status == ENV_RUNNABLE) {
	// 		env_run(e);  // Si el proximo se puede correr, lo corro.
	// 		return;
	// 	}
	// 	index++;
	// }  // Si no encontro ningun elemento para correr antes ni despues, volvemos a ejecutar el actual.
	// if (curenv->env_status == ENV_RUNNING) {
	// 	env_run(curenv);  // Solo lo corro si sigue estando para running.
	// 	return;
	// }


	// } else {  // Caso mas sencillo, simplemente corro el primero que encuentre runnable
	// 	int i;
	// 	for (i = 0; i < NENV; i++) {
	// 		if (envs[i].env_status == ENV_RUNNABLE) {
	// 			env_run(&envs[i]);
	// 			return;
	// 		}
	// 	}
	// }
	
#endif /// Comento un toque esto para poder ver el codigo bien en el ide

#ifdef SCHED_PRIORITIES
	// Implement simple priorities scheduling.
	//
	// Environments now have a "priority" so it must be consider
	// when the selection is performed.
	//
	// Be careful to not fall in "starvation" such that only one
	// environment is selected and run every time.

	// Your code here - Priorities
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
