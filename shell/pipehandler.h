#ifndef PIPEHANDLER_H
#define PIPEHANDLER_H

#include "types.h"

bool has_delim(char buf, const char* delim);

void parse_arguments(char* buf, const char* delim, const char** args);

void handle_pipe(struct pipecmd *cmd);

#endif  // PIPEHANDLER_H