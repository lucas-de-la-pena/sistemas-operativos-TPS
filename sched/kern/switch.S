/*
 * Your code here for context_switch
 *
 * It should restore the register values in the Trapframe with the 'iret' instruction.
 * This exits the kernel and starts executing some environment's code.
 *
 * This function does not return.
 */

.globl context_switch;
context_switch:  ;Recibe structTrapframe *tf por EDI --> JOS x86-32. Sintaxis INTEL
	mov edi, [edi + 0]      ; reg_edi --> cada campo es de 4 bytes
	mov esi, [edi + 4]      ; reg_esi
	mov ebp, [edi + 8]      ; reg_ebp
	mov ebx, [edi + 16]     ; reg_ebx
	mov edx, [edi + 20]     ; reg_edx
	mov ecx, [edi + 24]     ; reg_ecx
	mov eax, [edi + 28]     ; reg_eax

	;Apilo los valores para iret
    ;Estan despuas de los registros de PushRegs:
    ;es, ds, trapno, err, eip, cs, eflags, esp, ss
    push dword [edi + 32]    ; tf_es
    push dword [edi + 36]    ; tf_ds
    push dword [edi + 40]    ; tf_trapno
    push dword [edi + 44]    ; tf_err
    push dword [edi + 48]    ; tf_eip
    push dword [edi + 52]    ; tf_cs
    push dword [edi + 56]    ; tf_eflags
    push dword [edi + 60]    ; tf_esp
    push dword [edi + 64]    ; tf_ss

	iret

spin:
	jmp spin
