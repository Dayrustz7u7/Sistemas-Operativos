#include <bits/types/stack_t.h>
#include <bits/types/struct_sigstack.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "stacksignal.h"

#include <stdlib.h>
#include <stdio.h>
#include <signal.h>

stack_t *
create_stack_signal()
{
	stack_t *stack = malloc(sizeof(stack_t));

	if (!stack) {
		perror("malloc");
		return NULL;
	}

	stack->ss_size = SIGSTKSZ;
	stack->ss_flags = 0;  // Establecer a 0 para activar la pila de señales
	stack->ss_sp = malloc(SIGSTKSZ);

	if (!stack->ss_sp) {
		free(stack);
		perror("malloc");
		return NULL;
	}

	return stack;
}


void
destroy_stack(stack_t *stack)
{
	if (stack) {
		if (stack->ss_sp) {
			free(stack->ss_sp);  // Liberar la pila
		}
		free(stack);  // Liberar la estructura stack
	}
}


// void
// sigint_handler(int signum)
// {
// 	printf("Señal SIGINT recibida. Saliendo...\n");
// }


// void sigchild_handler() {
//     pid_t pid;
//     int status;
//     pid = waitpid(NULL, &status, NULL);
//     printf("==> terminado: PID=%i", pid);
// }


// sigaction(SIGCHILD, &sa, NULL) == -1;