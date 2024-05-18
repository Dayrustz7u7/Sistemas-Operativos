#include <inc/lib.h>

void
umain(int argc, char **argv)
{
	cprintf("hello, world\n");
	cprintf("i am environment %08x\n", thisenv->env_id);
    cprintf("My priority (Amount of tickets) is: %08x\n", sys_get_priority(sys_getenvid()));
    sys_increase_priority(sys_getenvid());
    cprintf("I've increased my priority in 10 tickets: %08x\n", sys_get_priority(sys_getenvid()));
    sys_decrease_priority(sys_getenvid());
    cprintf("I've decreased my priority in 10 tickets: %08x\n", sys_get_priority(sys_getenvid()));
}
