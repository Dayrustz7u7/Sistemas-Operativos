#ifndef STACKSIGNAL_H
#define STACKSIGNAL_H


#include <bits/types/stack_t.h>

stack_t *create_stack_signal();


void destroy_stack(stack_t *stack);


void sigint_handler(int signum);

// void sigchild_handler();

#endif
