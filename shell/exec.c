#include "exec.h"
#include "defs.h"
#include "types.h"
#include "utils.h"
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "parsing.h"
#include "pipehandler.h"

#define ERROR_EXECV "FALLO EL LLAMADO DE EXCEV"
#define ERROR_MALLOC_K "FALLO AL RESERVAR MEMORIA PARA CLAVE"
#define ERROR_MALLOC_V "FALLO AL RESERVAR MEMORIA PARA VALOR"
// establece "clave" con la parte clave de "arg"
// y nulo termina
//
// Ejemplo:
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

// establece "valor" con la parte de valor de "arg"
// y nulo termina
// "IDX" debería ser el índice en "arg" donde "=" char
// Reside
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

// establece las variables de entorno recibidas
// en la línea de comando
//
// Hints:
// - usar'block_contains()' a
// Obtener el índice donde el '=' es
// - 'get_environ_*()' puede ser útil aquí
static void
set_environ_vars(char **eargv, int eargc)
{
	// Your code here
	int status = 0;
	for (int i = 0; i < eargc; i++) {
		char *key = NULL;
		char *value = NULL;
		int idx = block_contains(eargv[i], '=');

		if (idx < 0) {
			status = -1;
			break;
		}

		// TO-DO: Agregar evaluacion del funcionamiento de los mallocs

		key = malloc(idx + 1);
		if (!key) {
			status = -1;
			perror(ERROR_MALLOC_K);
			break;
		}
		value = malloc(strlen(eargv[i]) - idx);
		if (!value) {
			status = -1;
			free(key);
			perror(ERROR_MALLOC_V);
			break;
		}
		get_environ_key(eargv[i], key);
		get_environ_value(eargv[i], value, idx);

		setenv(key, value, 0);

		free(key);
		free(value);
	}
}

// abre el archivo en el que el stdin/stdout/stderr
// El flujo será redirigido y devuelve
// El descriptor del archivo
//
// Descubre qué permisos necesita.
// ¿Tiene que cerrarse después de la llamada Ejecve (2)?
//
// Hints:
// - if O_CREAT is used, add S_IWUSR and S_IRUSR
// 	to make it a readable normal file
static int
open_redir_fd(char *file, int flags, int O_CREATE)
{
	
	if (O_CREATE) {
		return open(file, flags, S_IWUSR | S_IRUSR);
	}
	return open(file, flags);
}

// ejecuta un comando - no regresa
//
// Pista:
// - Verifique cómo se definen las estructuras 'CMD'
// en tipos.h
// - El casting podría ser una buena opción
void
exec_cmd(struct cmd *cmd)
{
	// Para ser utilizado en los diferentes casos
	struct execcmd *e;
	struct backcmd *b;
	struct execcmd *r;
	struct pipecmd *p;

	switch (cmd->type) {
	case EXEC:
		e = (struct execcmd *) cmd;  //
		set_environ_vars(e->eargv, e->eargc);
		if (execvp(e->argv[0], e->argv) < 0) {
			perror(ERROR_EXECV);
		}
		_exit(-1);
		break;

	case BACK: {
		printf("Pase por aca");
		b = (struct backcmd *) cmd;
		exec_cmd(b->c);
		_exit(-1);
		break;
	}

	case REDIR: {
		// changes the input/output/stderr flow
		//
		// para verificar si se debe realizar una redirección
		// Verifique si la longitud del nombre del archivo (en la
		// estructura CMD EXEC) es mayor que cero
		//
		// Your code here

		r = (struct execcmd *) cmd;

		// Nos aseguramos que el nombre del archivo con el que se va a trabajar sea mayor que 0.
		if (!(strlen(r->in_file) > 0) && !(strlen(r->out_file) > 0) &&
		    !(strlen(r->err_file) > 0)) {
			_exit(-1);
		}


		pid_t pid = fork();

		if (pid < 0) {
			printf("Entro al error del fork\n");
			perror("fork");
			_exit(-1);
		}

		int flags;

		if (pid == 0) {
			// Proceso hijo.
			// O_CREAT es un flag que crea el archivo si no existe
			// O_CLOEXEC es un flag que cierra el archivo al momento de hacer execvp

			if (strlen(r->in_file) > 0) {
				flags = (O_CREAT | O_CLOEXEC);
				int fd_in = open(r->in_file, flags, false);

				if (fd_in == -1) {
					perror("open");
				}
				int err_in = dup2(fd_in, STDIN_FILENO);
				if (err_in == -1) {
					perror("dup2");
				}
			}
			if (strlen(r->out_file) > 0) {
				int fd_out = open(r->out_file,
				                  O_CREAT | O_CLOEXEC,
				                  true);
				if (fd_out == -1) {
					perror("open");
				}
				int err_out = dup2(fd_out, STDOUT_FILENO);
				if (err_out == -1) {
					perror("dup2");
				}
			}
			if (strlen(r->err_file) > 0) {
				int fd_err = open(r->err_file,
				                  O_CREAT | O_CLOEXEC,
				                  false);
				if (fd_err == -1) {
					perror("open");
				}
				int err_err = dup2(fd_err, STDERR_FILENO);
				if (err_err == -1) {
					perror("dup2");
				}
			}
			execvp(r->argv[1], r->argv);
			perror("execvp");

		} else {
			// Proceso padre.
			wait((int *) 0);
		}
		break;
	}

	case PIPE: {
		// tuberías dos comandos
		//
		// Your code here
		p = (struct pipecmd *) cmd;

		handle_pipe(p);

		free_command(parsed_pipe);

		break;
	}
	}
}
