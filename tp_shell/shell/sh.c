#include "defs.h"
#include "readline.h"
#include "runcmd.h"
#include <bits/types/stack_t.h>
#include <bits/types/struct_sigstack.h>
#include <stdio.h>

char prompt[PRMTLEN] = { 0 };
// runs a shell command

void sigchild_handler(int signal);
void creat_signal_stack();
stack_t stack;
// runs a shell command

void
sigchild_handler(int signal)
{
	int status;
	pid_t pid;
	if ((pid = waitpid(0, &status, WNOHANG)) > 0) {
		printf_debug("==> terminando PID:%i status:%i\n", pid, status);
	}
}

void
creat_signal_stack()
{
	stack.ss_size = SIGSTKSZ;
	stack.ss_flags = 0;
	stack.ss_sp = malloc(SIGSTKSZ);

	if (stack.ss_sp == NULL) {
		perror("Malloc failed at signal stack creation");
		exit(EXIT_FAILURE);
	}

	if (sigaltstack(&stack, 0) < 0) {
		perror("sigaltstack error");
		free(stack.ss_sp);
		exit(EXIT_FAILURE);
	}

	struct sigaction sig_handler = { .sa_handler = sigchild_handler,
		                         .sa_flags = SA_ONSTACK | SA_RESTART };

	sigfillset(&sig_handler.sa_mask);
	if (sigaction(SIGCHLD, &sig_handler, NULL) < 0) {
		perror("Sigaction failed to settel");
		free(stack.ss_sp);
		exit(EXIT_FAILURE);
	};
}


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


int
main(void)
{
	creat_signal_stack();
	init_shell();
	run_shell();
	free(stack.ss_sp);

	return 0;
}
