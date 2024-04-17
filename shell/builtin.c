#include "builtin.h"
#include "defs.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


#define SALIDA_SHELL "exit"
#define HOME "HOME"
#define PWD "pwd"
// Devuelve verdadero si la llamada 'Salir'
// debería de ser realizado
//
// (no debe llamarse desde aquí)
int
exit_shell(char *cmd)
{
	return strcmp(cmd, SALIDA_SHELL) == 0 ? 1 : 0;
}

// returns true if "chdir" was performed
//  this means that if 'cmd' contains:
// 	1. $ cd directory (change to 'directory')
// 	2. $ cd (change to $HOME)
//  it has to be executed and then return true
//
//  Remember to update the 'prompt' with the
//  	new directory.
//
// Examples:
//  1. cmd = ['c','d', ' ', '/', 'b', 'i', 'n', '\0']
//  2. cmd = ['c','d', '\0']
int
cd(char *cmd)
{
	if (cmd[0] != 'c' && cmd[1] != 'd') {
		return 0;
	}

	char *dir = split_line(cmd, ' ');
	if (strlen(dir) == 0) {
		char *home = getenv(HOME);
		if (chdir(home) < 0) {
			perror("error");
			return 0;
		} else {
			snprintf(prompt, sizeof(prompt), "(%s)", home);
			return 1;
		}
	}

	int result = chdir(dir);
	if (result != -1) {
		snprintf(prompt, sizeof(prompt), "(%s)", dir);
		return 1;
	}

	return 0;
}

// returns true if 'pwd' was invoked
// in the command line
//
// (It has to be executed here and then
// 	return true)
int
pwd(char *cmd)
{
	if (strcmp(cmd, PWD) != 0) {
		return 0;
	}

	char buf[BUFLEN];
	char *pwd = getcwd(buf, BUFLEN);
	if (!pwd) {
		perror("error");
		return 0;
	}
	printf("%s\n", pwd);
	return 1;
}

// returns true if `history` was invoked
// in the command line
//
// (It has to be executed here and then
// 	return true)
int
history(char *cmd)
{
	// Your code here

	return 0;
}
