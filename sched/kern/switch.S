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
    movl 0(%edi), %edi      # reg_edi --> cada campo es de 4 bytes
    movl 4(%edi), %esi      # reg_esi
    movl 8(%edi), %ebp      # reg_ebp
    movl 16(%edi), %ebx     # reg_ebx
    movl 20(%edi), %edx     # reg_edx
    movl 24(%edi), %ecx     # reg_ecx
    movl 28(%edi), %eax     # reg_eax

    # Apilo los valores para iret
    # Están después de los registros de PushRegs:
    # es, ds, trapno, err, eip, cs, eflags, esp, ss
    pushl 32(%edi)          # tf_es
    pushl 36(%edi)          # tf_ds
    pushl 40(%edi)          # tf_trapno
    pushl 44(%edi)          # tf_err
    pushl 48(%edi)          # tf_eip
    pushl 52(%edi)          # tf_cs
    pushl 56(%edi)          # tf_eflags
    pushl 60(%edi)          # tf_esp
    pushl 64(%edi)          # tf_ss

	iret

spin:
	jmp spin
