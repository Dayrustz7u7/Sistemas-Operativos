#include "defs.h"
#include "readline.h"
#include "runcmd.h"
#include "stacksignal.h"
#include <bits/types/stack_t.h>
#include <bits/types/struct_sigstack.h>
#include <signal.h>
#include <stdio.h>

char prompt[PRMTLEN] = { 0 };
// runs a shell command
static void
run_shell()
{
	char *cmd;

	while ((cmd = read_line(prompt)) != NULL)
		if (run_cmd(cmd) == EXIT_SHELL)
			return;
}

// initializes the shell
// with the "HOME" directory
static void
init_shell()
{
	char buf[BUFLEN] = { 0 };
	char *home = getenv("HOME");

	if (chdir(home) < 0) {
		snprintf(buf, sizeof buf, "cannot cd to %s ", home);
		perror(buf);
	} else {
		snprintf(prompt, sizeof prompt, "(%s)", home);
	}
}


stack_t *
init_stack()
{
	stack_t *new_stack = create_stack_signal();

	if (!new_stack) {
		return NULL;
	}

	if (sigaltstack(new_stack, NULL) == -1) {
		perror("sigaltstack");  // Corrección aquí
		destroy_stack(new_stack);
		return NULL;
	}

	return new_stack;
}

void
set_sig_action(stack_t *stack)
{
	// struct sigaction sig_handler;
	// sig_handler.sa_sigaction = sigchild_handler;
	// sig_handler.sa_flags = SA_SIGINFO;
	// if (sigaction(SIGCHLD, &sig_handler, NULL) == -1) {
	// 	perror("sigaction");
	// 	sigaltstack(NULL, NULL);
	// 	destroy_stack(stack);
	// 	exit(EXIT_FAILURE);
	// }
}

int
main(void)
{
	stack_t *stack = init_stack();
	init_shell();

	if (!stack) {
		return 0;
	}

	set_sig_action(stack);

	run_shell();

	sigaltstack(NULL, NULL);
	destroy_stack(stack);

	return 0;
}
