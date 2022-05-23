.model compact, c
.386
.data
	VBE_STACK_SIZE	equ	2000h	; VBE caller stack size
	KERNEL_DATA_SEG	equ	10h		; kernel data segment

	extern call_addr	: dword	; VBE far call address
	extern stack_sel	: dword	; VBE caller stack selector
	stack_offset	dd	0		; current stack offset

.code
	public VesaCall

; 16 bit protected mode stack switching
; struct VBE_CALL_STACK {
;	unsigned int *esp;
;	unsigned int *ss
;}
StackSwitch proc
	mov		eax, dr3	; get current thread
	mov		edx, [esp]	; get top of stack pointer
	pushf
	pop		ecx
	add		eax, stack_offset
	cli
	push	ss
	cmp		dword ptr [esp], KERNEL_DATA_SEG
	je		kernel_stack
	pop		eax
	jmp		switch_stack
kernel_stack:
	pop		[eax + 4]
	mov		[eax], esp
switch_stack:
	lss		esp, [esp + 4]
	push	ecx
	popf
	jmp		fword ptr [edx]
StackSwitch endp

; call VESA BIOS function
; VesaCall(REGS16 *regs)
VesaCall proc
		pushf
		cli
		push	es
		pusha
		mov		esi, esp
		push	stack_sel
		push	VBE_STACK_SIZE
		call	StackSwitch
		push	KERNEL_DATA_SEG
		push	esi
		push	edi
		push	ds
		mov		dx, [edi + 12]
		mov		es, dx
		mov		ax, [edi]
		mov		bx, [edi + 2]
		mov		cx, [edi + 4]
		mov		dx, [edi + 6]
		mov     si, [edi + 8]
		mov     di, [edi + 10]
		mov     esi, call_addr
		call    fword ptr [esi]
		pop     ds
		pop		ebp
		mov		ds:[ebp], ax
		mov		eax, ebp
		mov		[eax + 2], bx
		mov		[eax + 4], cx
		mov		[eax + 6], dx
		mov		[eax + 8], si
		mov		[eax + 10], di
		mov     bx, es
		mov     [eax + 12], bx
		call    StackSwitch
		popa
		pop		es
		popf
VesaCall endp

end
