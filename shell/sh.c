#include "defs.h"
#include "readline.h"
#include "runcmd.h"
#include <bits/types/stack_t.h>
#include <bits/types/struct_sigstack.h>
#include <stdio.h>
#include "stacksignalhandler.h"

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


int
main(void)
{
	stack_t *stack = init_stack();

	if (!stack) {
		return 0;
	}

	set_sig_action(stack);

	init_shell();
	run_shell();
	destroy_stack(stack);

	return 0;
}
