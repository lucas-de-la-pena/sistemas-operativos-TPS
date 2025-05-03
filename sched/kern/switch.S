/*
 * Your code here for context_switch
 *
 * It should restore the register values in the Trapframe with the 'iret' instruction.
 * This exits the kernel and starts executing some environment's code.
 *
 * This function does not return.
 */

.globl context_switch;
context_switch:        # Recibe un puntero a struct Trapframe en la pila 
    # Ajusto el stack, saco el return address falso del call y dejo ESP apuntando al Trapframe
	add	$4, %esp	   # Salteo la direccion de retorno, no se usa de todas maneras  		
	mov	(%esp), %esp   # Cargo en ESP el puntero al Trapframe 
		
	popal              # Resturo los 16 registros		
	pop %es            # Restauro valor del reg %es
	pop	%ds	           # Restauro valor del reg %ds
	add	$8, %esp	   # Ajusto el stack 8 bytes hacia adelante para remover valores innecesarios		

	iret

spin:
	jmp spin
