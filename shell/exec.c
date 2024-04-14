#include "exec.h"

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
open_redir_fd(char *file, int flags)
{
	// Your code here

	return -1;
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
		// genera un comando
        //
        // Tu código aquí
		printf("Commands are not yet implemented\n");
		_exit(-1);
		break;

	case BACK: {
		// ejecuta un comando en segundo plano
		//
		// Your code here
		printf("Background process are not yet implemented\n");
		_exit(-1);
		break;
	}

	case REDIR: {
		// changes the input/output/stderr flow
		//
        // para verificar si se debe realizar una redirección 
        // Verifique si la longitud del nombre del archivo (en la estructura CMD EXEC) 
        // es mayor que cero
		//
		// Your code here
		printf("Redirections are not yet implemented\n");
		_exit(-1);
		break;
	}

	case PIPE: {
		// tuberías dos comandos
		//
		// Your code here
		printf("Pipes are not yet implemented\n");

		// free the memory allocated
		// for the pipe tree structure
		free_command(parsed_pipe);

		break;
	}
	}
}
