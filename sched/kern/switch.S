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
	//iret
	

spin:
	jmp spin

