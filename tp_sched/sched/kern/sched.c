#include <inc/assert.h>
#include <inc/x86.h>
#include <kern/spinlock.h>
#include <kern/env.h>
#include <kern/pmap.h>
#include <kern/monitor.h>

void sched_halt(void);
int stadistics[NENV];  // Cantidad de veces que corrio cada proceso.
int scheduler_calls;   // Cantidad de llamadas al scheduler.

// VARIABLE ESTATICA PARA LA SEMILLA - PRUEBA---------
static unsigned long next = 1;
//---------------------------------------------------

// Choose a user environment to run and run it.

/// Agrego funcion auxiliar para hacer legible el codigo
void
check_and_run(int j)
{
	struct Env *env = &envs[j];
	if (env->env_status == ENV_RUNNABLE) {
		stadistics[j]++;
		env_run(env);
	}
}

/// Funcion auxiliar para dejar el codigo mas prolijo.
bool
can_run(struct Env *env)
{
	return (env->env_status == ENV_RUNNABLE);
}

/// Funcion auxiliar que retorna un int, indicando cantidad total de tickets.
uint32_t
get_tot_tickets()
{
	uint32_t tot_tickets = 0;
	for (int i = 0; i < NENV; i++) {
		if (envs[i].env_status == ENV_RUNNABLE) {
			tot_tickets += envs[i].tickets;
		}
	}
	return tot_tickets;
}

// Función srand para establecer la semilla
void
srand(unsigned int seed)
{
	next = seed;
}

// Función para leer el contador de tiempo de CPU
unsigned int
read_cpu_timestamp()
{
	unsigned int low, high;
	// Ensamblador en línea para leer el registro de contador de tiempo de CPU
	__asm__ volatile("rdtsc" : "=a"(low), "=d"(high));
	return low;
}


// Funcion auxiliar Random()
unsigned
get_random(unsigned int total_ticks)
{
	// Initialize seed using CPU timestamp
	static unsigned long seed = 0;
	if (seed == 0) {
		seed = read_cpu_timestamp();
	}

	// Constants for the LCG
	const unsigned long a = 1103515245;
	const unsigned long c = 12345;
	const unsigned long m = 1UL << 31;  // 2^31

	// Update the seed and generate the next random number
	seed = (a * seed + c) % m;
	return (unsigned int) (seed) % (total_ticks);
}

void
sched_yield(void)
{
	scheduler_calls++;
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

	int act_pos = 0;  /// i que uso tomas

	if (curenv) {  // Primero me fijo si hay un env corriendo actualmente
		act_pos = ENVX(curenv->env_id) +
		          1;  /// Obtengo el indice del siguente proceso
	}

	/// Busco todos los procesos que pueden correr a partir del proceso actual si es que hay uno actual
	for (int i = 0; i < NENV; i++) {
		int j = act_pos + i;
		check_and_run(j);
	}

	/// Busco los procesos anteriores a ver cuales pueden correr
	for (int j = 0; j < act_pos; j++) {
		check_and_run(j);
	}

	if (curenv && curenv->env_status == ENV_RUNNING) {
		stadistics[ENVX(curenv->env_id)]++;
		env_run(curenv);
	}

#endif  /// Comento un toque esto para poder ver el codigo bien en el ide

#ifdef SCHED_PRIORITIES
	// Implement simple priorities scheduling.
	//
	// Environments now have a "priority" so it must be consider
	// when the selection is performed.
	//
	// Be careful to not fall in "starvation" such that only one
	// environment is selected and run every time.

	// Your code here - Priorities

	uint32_t tot_tickets = get_tot_tickets();

	struct Env *posible_winners[NENV];
	unsigned int cant_cand = 0;

	if (!tot_tickets) {
		if (curenv && curenv->env_status == ENV_RUNNING) {
			if (curenv->tickets) {
				curenv->tickets--;
				stadistics[ENVX(curenv->env_id)]++;
				env_run(curenv);
			} else {
				sched_halt();
			}
		}
	}

	// Obtenemos el proceso a correr.
	int counter = 0;
	unsigned int winner =
	        get_random(tot_tickets + 1);  // Tenemos que hacer funcion random.

	for (int i = 0; i < NENV; i++) {
		counter += envs[i].tickets;
		if (envs[i].env_status != ENV_RUNNABLE) {
			continue;
		}

		if (counter > winner) {
			struct Env *winner_cand = &envs[i];
			posible_winners[cant_cand] = winner_cand;
			cant_cand++;
		}
	}


	if (cant_cand > 0) {
		unsigned int real_winner = get_random(cant_cand);
		struct Env *goat = posible_winners[real_winner];
		if (goat->tickets > 0) {
			goat->tickets--;
		}
		env_run(goat);
		stadistics[real_winner]++;
		env_run(goat);
	}

#endif
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
		cprintf("STATS: There are %d processes\n", NENV);
		for (int j = 0; j < NENV; j++) {
			cprintf("Process: %d, total times ran: %d\n",
			        j,
			        stadistics[j]);
		}
		cprintf("Scheduler's been called %d times\n", scheduler_calls);
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
