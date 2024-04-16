#include "parsing.h"

// parses an argument of the command stream input
static char *
get_token(char *buf, int idx)
{
	char *tok;
	int i;

	tok = (char *) calloc(ARGSIZE, sizeof(char));
	i = 0;

	while (buf[idx] != SPACE && buf[idx] != END_STRING) {
		tok[i] = buf[idx];
		i++;
		idx++;
	}

	return tok;
}

// analiza y cambia stdin/out/err si es necesario
static bool
parse_redir_flow(struct execcmd *c, char *arg)
{
	int inIdx, outIdx;

	// flow redirection for output
	if ((outIdx = block_contains(arg, '>')) >= 0) {
		switch (outIdx) {
		// stdout redir
		case 0: {
			strcpy(c->out_file, arg + 1);
			break;
		}
		// stderr redir
		case 1: {
			strcpy(c->err_file, &arg[outIdx + 1]);
			break;
		}
		}

		free(arg);
		c->type = REDIR;

		return true;
	}

	// flow redirection for input
	if ((inIdx = block_contains(arg, '<')) >= 0) {
		// stdin redir
		strcpy(c->in_file, arg + 1);

		c->type = REDIR;
		free(arg);

		return true;
	}

	return false;
}

// parses and sets a pair KEY=VALUE
// environment variable
static bool
parse_environ_var(struct execcmd *c, char *arg)
{
	// establece variables de entorno aparte de
	// los definidos en la variable global "Environ"
	if (block_contains(arg, '=') > 0) {
		// verifica si la parte clave del par
		// no contiene un char '-' que significa
		// que no es un ambiente var, sino también
		// Un argumento del programa a ejecutar
		// (Por ejemplo:
		// ./prog -arg = valor
		// ./prog --arg = valor
		//)
		if (block_contains(arg, '-') < 0) {
			c->eargv[c->eargc++] = arg;
			return true;
		}
	}

	return false;
}

// Esta función será llamada para cada token, y debería
// expandir variables de entorno.En otras palabras, si el token
// comienza con '$', la sustitución correcta con el
// El valor del entorno debe realizarse.De lo contrario lo mismo
// se devuelve el token.Si la variable no existe, una cadena vacía debe ser
// regresó dentro del token
//
// Sugerencias:
// - Verifique si el primer byte del argumento contiene el '$'
// - Expanda y copie el valor en 'Arg'
// - Recuerde verificar el tamaño del valor de la variable
// podría ser mayor que el tamaño actual de 'arg'
// Si ese es el caso, debe reasignar 'arg' al nuevo tamaño.
static char *
expand_environ_var(char *arg)
{
	// Your code here

	return arg;
}

// analiza un solo comando que tiene en cuenta:
// - Los argumentos pasados al programa
// - STDIN/STDOUT/STDERR FLOW CAMBIOS
// - Variables de entorno (expandir y establecer)
static struct cmd *
parse_exec(char *buf_cmd)
{
	struct execcmd *c;
	char *tok;
	int idx = 0, argc = 0;

	c = (struct execcmd *) exec_cmd_create(buf_cmd);

	while (buf_cmd[idx] != END_STRING) {
		tok = get_token(buf_cmd, idx);
		idx = idx + strlen(tok);

		if (buf_cmd[idx] != END_STRING)
			idx++;

		if (parse_redir_flow(c, tok))
			continue;

		if (parse_environ_var(c, tok))
			continue;

		tok = expand_environ_var(tok);

		c->argv[argc++] = tok;
	}

	c->argv[argc] = (char *) NULL;
	c->argc = argc;

	return (struct cmd *) c;
}

// analiza un comando sabiendo que contiene el '&' char
static struct cmd *
parse_back(char *buf_cmd)
{
	int i = 0;
	struct cmd *e;

	while (buf_cmd[i] != '&')
		i++;

	buf_cmd[i] = END_STRING;

	e = parse_exec(buf_cmd);

	return back_cmd_create(e);
}

// analiza un comando y verifica si contiene el '&'
// (background process) character
static struct cmd *
parse_cmd(char *buf_cmd)
{
	if (strlen(buf_cmd) == 0)
		return NULL;

	int idx;

	// verifica si el símbolo de fondo es después
	// un símbolo de redir, en cuyo caso
	// No tiene que correr en el'back'
	if ((idx = block_contains(buf_cmd, '&')) >= 0 && buf_cmd[idx - 1] != '>')
		return parse_back(buf_cmd);

	return parse_exec(buf_cmd);
}

// analiza la línea de comando
// Buscando el carácter de la tubería '|'
struct cmd *
parse_line(char *buf)
{
	struct cmd *r, *l;

	char *right = split_line(buf, '|');

	l = parse_cmd(buf);
	r = parse_cmd(right);

	return pipe_cmd_create(l, r);
}
