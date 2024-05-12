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
	if (curenv) {  // Primero me fijo si hay un env corriendo actualmente
		struct Env *e = curenv;
		while (e->env_link !=
		       NULL) {  // Mientras que el proximo no sea nulo.
			e = e->env_link;
			if (e->env_status == ENV_RUNNABLE) {
				env_run(e);  // Si el proximo se puede correr, lo corro.
				return;
			}
		}
		int i = 0;
		while (envs[i] !=
		       curenv) {  // Ya vi todos los elementos proximos al actual, ahora veo los previos.
			e = &envs[i];
			if (e->env_status == ENV_RUNNABLE) {
				env_run(e);  // Si el proximo se puede correr, lo corro.
				return;
			}
			i++;
		}  // Si no encontro ningun elemento para correr antes ni despues, volvemos a ejecutar el actual.
		if (curenv->env_status == ENV_RUNNING) {
			env_run(curenv);  // Solo lo corro si sigue estando para running.
			return;
		}


	} else {  // Caso mas sencillo, simplemente corro el primero que encuentre runnable
		int i;
		for (i = 0; i < NENV; i++) {
			if (envs[i].env_status == ENV_RUNNABLE) {
				env_run(&envs[i]);
				return;
			}
		}
	}
	// Si no que hago? Un shutdown? eso no entendi.
	sched_halt();
#endif

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
