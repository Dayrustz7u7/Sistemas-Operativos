#include "runcmd.h"
#include "defs.h"
#include "printstatus.h"
#include <unistd.h>

int status = 0;
struct cmd *parsed_pipe;

// runs the command in 'cmd'
int
run_cmd(char *cmd)
{
	pid_t p;
	struct cmd *parsed;

	// if the "enter" key is pressed
	// just print the prompt again
	if (cmd[0] == END_STRING)
		return 0;

	// "history" built-in call
	if (history(cmd))  //----------------> Esto si quiern implementar el desafio
		return 0;

	// "cd" built-in call
	if (cd(cmd))
		return 0;

	// "exit" built-in call
	if (exit_shell(cmd))
		return EXIT_SHELL;

	// "pwd" built-in call
	if (pwd(cmd))
		return 0;

	// parses the command line
	parsed = parse_line(cmd);

	// forks and run the command
	if ((p = fork()) == 0) {
		// Mantenga una referencia
		// A la tubería analizada CMD
		// para que se pueda liberar más tarde
		if (parsed->type == PIPE)
			parsed_pipe = parsed;

		exec_cmd(parsed);
	}

	// almacenaElPidDelProceso
	parsed->pid = p;

	// Proceso de fondo Tratamiento especial
	// Pista:
	// - Compruebe si el proceso es
	// se ejecutará en el 'regreso'
	// - Imprimir información al respecto con
	// 'print_back_info ()'
	//
	// Tu código aquí

	// espera a que termine el proceso

	if (parsed->type == BACK) {
		waitpid(p, &status, WNOHANG);
		print_back_info(parsed);
	} else {
		waitpid(p, &status, 0);
		print_status_info(parsed);
	}
	free_command(parsed);
	return 0;
}
