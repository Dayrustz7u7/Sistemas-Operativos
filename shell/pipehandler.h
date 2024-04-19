#ifndef PIPEHANDLER_H
#define PIPEHANDLER_H

#include "types.h"

//Se fija si el hijo derecho tiene dentro otro pipe
bool has_other_pipe(const char* buf);

//Parsea el buffer para poder pasarlo al execvp de forma correcta
void parse_arguments(char* buf, const char* delim, const char** args);

//De haber mas de un | en el comando ejecuta los multiples pipes.
void handle_multiple_pipes(struct pipecmd *cmd, int readingPipe[2]);

//Ejecuta el unico pipe, de haber mas, ejecuta handle_multiple_pipes.
void handle_pipe(struct pipecmd *cmd);

#endif  // PIPEHANDLER_H