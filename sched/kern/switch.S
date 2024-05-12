/*
 * Your code here for context_switch
 *
 * It should restore the register values in the Trapframe with the 'iret' instruction.
 * This exits the kernel and starts executing some environment's code.
 *
 * This function does not return.
 */

.globl context_switch;
context_switch:
	//Preparamos la cpu para volver al modo usuario
	//obtenemos y alocamos en cpu el struct trapframe
	movl 4(%esp),%esp // Se carga el trapframe ( contenido) al %esp
	popal
	pop %es
	pop %ds
	addl $8, %esp
	iret   
spin:
	jmp spin

