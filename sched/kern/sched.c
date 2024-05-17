#include <inc/assert.h>
#include <inc/x86.h>
#include <kern/spinlock.h>
#include <kern/env.h>
#include <kern/pmap.h>
#include <kern/monitor.h>

void sched_halt(void);

//VARIABLE ESTATICA PARA LA SEMILLA - PRUEBA---------
static unsigned long next = 1;
//---------------------------------------------------

// Choose a user environment to run and run it.

/// Agrego funcion auxiliar para hacer legible el codigo
void
check_and_run(int j)
{
	struct Env *env = &envs[j];
	if (env->env_status == ENV_RUNNABLE) {
		env_run(env);
	}
}

/// Funcion auxiliar que retorna un int, indicando cantidad total de tickets.
int
get_tot_tickets()
{
	int tot_tickets = 0;
	for (int i = 0; i < NENV; i++){
		if (envs[i].env_status == ENV_FREE){
			continue;
		}
		tot_tickets += envs[i].tickets;
	}
	return tot_tickets;
}

// Función srand para establecer la semilla
 void srand(unsigned int seed) {
     next = seed;
 }

// Función para leer el contador de tiempo de CPU
unsigned int read_cpu_timestamp() {
    unsigned int low, high;
    // Ensamblador en línea para leer el registro de contador de tiempo de CPU
    __asm__ volatile ("rdtsc" : "=a" (low), "=d" (high));
    return low;
}


// Funcion auxiliar Random()
int
get_random(){
	unsigned int seed = read_cpu_timestamp();
	srand(seed);

	next = next * 1103515245 + 12345;
    return (unsigned int)(next / 65536) % 32768;
}

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

	//Obtener la cantidad de tickets.
	int tot_tickets = get_tot_tickets(); //DEBERIAMOS VER QUE PASA SI NO HAY TICKETS??

	//Obtenemos el proceso a correr.
	int counter = 0;
	int winner = get_random(0, tot_tickets); //Tenemos que hacer funcion random.

	for (int i = 0; i < NENV; i++){
		if (envs[i].env_status == ENV_FREE){
			continue;
		}
		counter += envs[i].tickets;
		if (counter > winner) {
			curenv = &envs[i]; //Este es el proceso ganador y consecuentemente el que se correra.
			env_run(curenv);
			break;
		}
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
