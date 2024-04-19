#include "exec.h"
#include "parsing.h"
#include "pipehandler.h"

// sets "key" with the key part of "arg"
// and null-terminates it
//
// Example:
//  - KEY=value
//  arg = ['K', 'E', 'Y', '=', 'v', 'a', 'l', 'u', 'e', '\0']
//  key = "KEY"
//
static void
get_environ_key(char *arg, char *key)
{
	int i;
	for (i = 0; arg[i] != '='; i++)
		key[i] = arg[i];

	key[i] = END_STRING;
}

// sets "value" with the value part of "arg"
// and null-terminates it
// "idx" should be the index in "arg" where "=" char
// resides
//
// Example:
//  - KEY=value
//  arg = ['K', 'E', 'Y', '=', 'v', 'a', 'l', 'u', 'e', '\0']
//  value = "value"
//
static void
get_environ_value(char *arg, char *value, int idx)
{
	size_t i, j;
	for (i = (idx + 1), j = 0; i < strlen(arg); i++, j++)
		value[j] = arg[i];

	value[j] = END_STRING;
}

// sets the environment variables received
// in the command line
//
// Hints:
// - use 'block_contains()' to
// 	get the index where the '=' is
// - 'get_environ_*()' can be useful here
static void
set_environ_vars(char **eargv, int eargc)
{
	// Your code here
}

// opens the file in which the stdin/stdout/stderr
// flow will be redirected, and returns
// the file descriptor
//
// Find out what permissions it needs.
// Does it have to be closed after the execve(2) call?
//
// Hints:
// - if O_CREAT is used, add S_IWUSR and S_IRUSR
// 	to make it a readable normal file
static int
open_redir_fd(char *file, int flags)
{
	// Your code here

	return -1;
}

// executes a command - does not return
//
// Hint:
// - check how the 'cmd' structs are defined
// 	in types.h
// - casting could be a good option
void
exec_cmd(struct cmd *cmd)
{
	// To be used in the different cases
	struct execcmd *e;
	struct backcmd *b;
	struct execcmd *r;
	struct pipecmd *p;

	switch (cmd->type) {
	case EXEC:
		// spawns a command
		//
		// Your code here
		printf("Commands are not yet implemented\n");
		_exit(-1);
		break;

	case BACK: {
		// runs a command in background
		//
		// Your code here
		printf("Background process are not yet implemented\n");
		_exit(-1);
		break;
	}

	case REDIR: {
		// changes the input/output/stderr flow
		//
		// To check if a redirection has to be performed
		// verify if file name's length (in the execcmd struct)
		// is greater than zero
		//
		// Your code here
		r = (struct execcmd *) cmd;
		
		/*
		printf("linea ingresada: %s\n",r->scmd); //En scmd se guarda la linea tal cual se ingreso
		printf("Parametros previo a redireccion: %d\n",r->argc); //En argc se guarda la cantidad de parametros previo al < o > o 2>&1
		printf("NI idea: %d\n",r->eargc); //NI puta idea que se guarda en earg v
		printf("%s\n",r->argv[0]); //argv es una lista con los parametros ingresados previo al < o > o 2>&1
		
		printf("Ni idea: %s\n",r->eargv); //Ni idea que se guarda en eargv
		printf("Nombre stdin: %s\n",r->in_file); //Guarda el nombre del archivo a el cual se redirige la entrada estandar
		printf("Nombre stdout: %s\n",r->out_file); //Guarda el nombre del archivo a el cual se redirige la salida estandar
		printf("Nombre stderr: %s\n",r->err_file);
		if (r->argv[r->argc] == NULL){
			printf("Con esto comprobe que el buffer tenia un null al final\n");
		}
		*/


		//Nos aseguramos que el nombre del archivo con el que se va a trabajar sea mayor que 0.
		if ( !(strlen(r->in_file)>0) && !(strlen(r->out_file)>0) && !(strlen(r->err_file)>0)){
			_exit(-1);
		}

		pid_t pid = fork();

		if (pid < 0){
			printf("Entro al error del fork\n");
			perror("fork");
			_exit(-1);
		} else if (pid == 0){
			//Proceso hijo.
			if (strlen(r->in_file)>0){
				printf("Entro a input");
				int fd_in = open(r->in_file, O_CREAT | O_CLOEXEC);
				if (fd_in == -1){
					printf("Entro al error del archivo\n");
					perror("open");
				}
				int err_in = dup2(fd_in, STDIN_FILENO);
				if (err_in == -1){
					printf("Entro al error del dup2\n");
					perror("dup2");
				}
			}
			if (strlen(r->out_file)>0){
				printf("Entro a output");
				int fd_out = open(r->out_file, O_CREAT | O_CLOEXEC);
				if (fd_out == -1){
					printf("Entro al error del archivo\n");
					perror("open");
				}
				int err_out = dup2(fd_out, STDOUT_FILENO);
				if (err_out == -1){
					printf("Entro al error del dup2\n");
					perror("dup2");
				}
			}
			if (strlen(r->err_file)>0){
				printf("Entro a error");
				int fd_err = open(r->err_file, O_CREAT | O_CLOEXEC);
				if (fd_err == -1){
					printf("Entro al error del archivo\n");
					perror("open");
				}
				int err_err = dup2(fd_err, STDERR_FILENO);
				if (err_err == -1){
					printf("Entro al error del dup2\n");
					perror("dup2");
				}
			}
			execvp(r->argv[1], r->argv);
			perror("execvp");

		} else{
			//Proceso padre.
			wait((int *) 0);
		}
		break;
	}

	case PIPE: {
		// pipes two commands
		//
		// Your code here
		p = (struct pipecmd *) cmd;

	    handle_pipe(p);

		free_command(parsed_pipe);

		break;
	}
	}
}
