#include <inc/lib.h>

void
umain(int argc, char **argv)
{
	
	int tickets = thisenv->tickets;
	cprintf("My priority (Amount of tickets) is: %d\n", tickets);
	sys_decrease_priority(thisenv->env_id);
	cprintf("My expected priority (after decrease) is: %d\n", 99500);
	cprintf("My actual priority (after decrease) is: %d\n", thisenv->tickets);
	sys_increase_priority(thisenv->env_id);
	cprintf("My expected priority (after increase) is: %d\n", 100500);
	cprintf("My actual priority (after increase) is: %d\n", thisenv->tickets);
}
