#include <inc/lib.h>

void
umain(int argc, char **argv)
{
	cprintf("\n");
	cprintf("\n");
	cprintf("\n");
	cprintf("\n");
	cprintf("hello, world\n");
	cprintf("i am environment %d\n", thisenv->env_id);
	cprintf("My priority (Amount of tickets) is: %d\n", sys_get_priority(sys_getenvid()));
	
	sys_increase_priority(sys_getenvid());
	cprintf("I've increased my priority in 10 tickets: %d\n", sys_get_priority(sys_getenvid()));
	
	sys_decrease_priority(sys_getenvid());
	cprintf("I've decreased my priority in 10 tickets: %d\n", sys_get_priority(sys_getenvid()));
	cprintf("\n");
	cprintf("\n");
	cprintf("\n");
	cprintf("\n");
}
