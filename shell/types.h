#ifndef TYPES_H
#define TYPES_H

#include "defs.h"

/* This file contains definition types for commands representation */

/*
 * cmd
 * ---
 *
 *Interfaz genérica que representa un solo comando.
 * Todas las otras estructuras * CMD se pueden lanzar,
 * Y no pierden información (por ejemplo, el campo 'Tipo').
 *
 *  - type: { EXEC, REDIR, BACK, PIPE }
 *  - pid: the process id
 *  - scmd: a string representing the command before being parsed
 */
struct cmd {
	int type;
	pid_t pid;
	char scmd[BUFLEN];
};

/*
 * execcmd
 * -------
 *
 * Contiene toda la información relevante para ejecutar un comando.
 *
 * - Tipo: podría ser ejecutivo o redir
 * - argc: cantidad de argumentos después de analizar
 * - argv: matriz de cuerdas representan los argumentos
 * de la forma: {"binario/comando", "arg0", "arg1", ..., (char*) null}
 * - EARGC: la cantidad Vars ambiental después de analizar
 * - EARGV: matriz de cadenas de la forma: "clave = valor"
 * R        epresentación de los Vars Environ
 * - * _file: cadena que contiene el nombre del archivo a redirigir a
 *
*  IMPORTANTE
 * ---------
 * Una estructura execcmd puede tener un tipo exec o redir dependiendo de si el comando
 * Ser ejecutado tiene al menos un símbolo de redirección (<,>, >>,> &)
 */
struct execcmd {
	int type;
	pid_t pid;
	char scmd[BUFLEN];
	int argc;
	int eargc;
	char *argv[MAXARGS];
	char *eargv[MAXARGS];
	char out_file[FNAMESIZE];
	char in_file[FNAMESIZE];
	char err_file[FNAMESIZE];
};

/*
 * pipecmd
 * --------
 *
 * It contains the same information as 'cmd' plus two fields representing the left
 * and right part of a command of the form: "command1 arg1 arg2 | command2 arg3"
 */
struct pipecmd {
	int type;
	pid_t pid;
	char scmd[BUFLEN];
	struct cmd *leftcmd;
	struct cmd *rightcmd;
};

/*
 * backcmd
 * -------
 *
 * It contains the same information as 'cmd' plus one more field containing the
 * command to be executed. Take a look to the parsing.c file for better understandig.
 *
 * Again, this extra field, can have type either EXEC or REDIR depending on
 * if the process to be executed in the background contains redirection symbols.
 */
struct backcmd {
	int type;
	pid_t pid;
	char scmd[BUFLEN];
	struct cmd *c;
};

#endif  // TYPES_H
