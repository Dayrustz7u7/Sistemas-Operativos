#include <inc/lib.h>

void
umain(int argc, char **argv)
{
	int tickets_iniciales = thisenv->tickets;
	cprintf("\n---Arranco test---\n");
	cprintf("Obtengo mi cantidad incial de tickets que es %d == %d\n",
	        tickets_iniciales,
	        sys_get_priority(thisenv->env_id));
	cprintf("Tengo misma cantidad de tickets que mi padre %d == %d\n",
	        sys_get_priority(thisenv->env_parent_id),
	        sys_get_priority(thisenv->env_id));
	cprintf("No puedo aumentar mi prioridad mas alla de la capacidad "
	        "inicial 100000 \n");
	cprintf("Aumento teniendo los tickets en 1000 teniendo aun la cantidad "
	        "inicial: %d == -1 \n",
	        sys_increase_priority(thisenv->env_id, 1000));
	cprintf("Disminuyo la cantidad inicial de tickets en 90000: %d \n",
	        sys_decrease_priority(thisenv->env_id, 90000));
	cprintf("No puedo disminuir la cantidad de tickets a 0: %d == -1\n",
	        sys_decrease_priority(thisenv->env_id, 10000));
	cprintf("No puedo disminuir la cantidad de tickets a 0: %d\n",
	        sys_decrease_priority(thisenv->env_id, 9998));

	sys_increase_priority(thisenv->env_id, 99999);

	int tickets = thisenv->tickets;
	if (fork() == 0) {
		cprintf("Los tickets del hijo deben ser mayor o igual a la del "
		        "padre %d >= %d \n",
		        tickets,
		        tickets_iniciales);
		return;
	}
	for (int i = 0; i < 100000; i++)
		cprintf("");
}
