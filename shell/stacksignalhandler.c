#include <bits/types/siginfo_t.h>
#include <bits/types/stack_t.h>
#include <bits/types/struct_sigstack.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "utils.h"
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include "stacksignalhandler.h"

stack_t *
create_stack_signal()
{
	stack_t *stack = malloc(sizeof(stack_t));

	if (!stack) {
		perror("malloc");
		return NULL;
	}

	stack->ss_size = SIGSTKSZ;
	stack->ss_flags = 0;
	stack->ss_sp = malloc(SIGSTKSZ);

	if (!stack->ss_sp) {
		free(stack);
		perror("malloc");
		return NULL;
	}

	return stack;
}


stack_t *
init_stack()
{
	stack_t *new_stack = create_stack_signal();

	if (!new_stack) {
		return NULL;
	}

	if (sigaltstack(new_stack, NULL) == -1) {
		perror("sigaltstack");
		destroy_stack(new_stack);
		return NULL;
	}

	return new_stack;
}

void
set_sig_action(stack_t *stack)
{
	struct sigaction sig_handler;
	sig_handler.sa_sigaction = sigchild_handler;
	sig_handler.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sig_handler, NULL) == -1) {
		perror("sigaction");
		sigaltstack(NULL, NULL);
		destroy_stack(stack);
		exit(EXIT_FAILURE);
	}
}


void
destroy_stack(stack_t *stack)
{
	if (stack) {
		if (stack->ss_sp) {
			free(stack->ss_sp);
		}
		free(stack);
	}
}


void
sigchild_handler()
{
	int status;
	pid_t pid;
	if ((pid = waitpid(0, &status, WNOHANG)) > 0) {
		printf_debug("==> terminando PID:%i status:%i\n", pid, status);
	}
}
