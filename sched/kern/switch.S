/*
 * Your code here for context_switch
 *
 * It should restore the register values in the Trapframe with the 'iret' instruction.
 * This exits the kernel and starts executing some environment's code.
 *
 * This function does not return.
 */

.globl context_switch;
context_switch:  # Recibe structTrapframe *tf por %EDI --> JOS x86-32. 
	add	$4, %esp	   # Alineo stack 		
	mov	(%esp), %esp   # Accedo al struct apuntado por el puntero del trapframe
		
	popal              # Resturo los 16 registros		
	pop %es            # Restauro valor del reg %es
	pop	%ds	           # Restauro valor del reg %ds
	add	$8, %esp	   # Ajusto el stack 8 bytes hacia adelante para remover valores innecesarios		

	iret

spin:
	jmp spin
