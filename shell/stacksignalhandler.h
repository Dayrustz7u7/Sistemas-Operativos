#ifndef STACKSIGNALHANDLER_H
#define STACKSIGNALHANDLER_H


#include <bits/types/siginfo_t.h>
#include <bits/types/stack_t.h>


stack_t *init_stack();
stack_t *create_stack_signal();
void destroy_stack(stack_t *stack);
void set_sig_action(stack_t *stack);
void sigint_handler(int signum);
void sigchild_handler();

#endif
