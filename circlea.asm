.model compact, c
.386
.code

	extrn	tmem	: dword
	extrn	vmem	: dword
	extrn	vbuff1	: dword
	extrn	vbuff2	: dword

	public mouseDetect, getMousePos, setMouseWindow, setMousePos
	public flipScreen, putPixel, getPixel, clearTextMem, clearBuffer
	public writeString, waitPressKey, setVideoMode, closeMouse, setPalette

setVideoMode proc
	arg mode : word
	mov	ax, mode
	int	10h
	ret
setVideoMode endp

mouseDetect proc
	xor	ax, ax
	int	33h
	xor	al, al
	ret
mouseDetect endp

closeMouse proc
	xor	ax, ax
	int	33h
	ret
closeMouse endp

getMousePos proc 
	arg x : dword, y : dword, b : dword
	mov ax, 03h
	int	33h
	les	di, b
	mov	word ptr es:[di], bx
	les	di, x
	mov	word ptr es:[di], cx
	les	di, y
	mov	word ptr es:[di], dx
	ret
getMousePos endp

setMouseWindow proc
	arg x1 : word, y1 : word, x2 : word, y2 : word

	mov	ax, 07h
	mov	cx, x1
	mov	dx, x2
	int	33h
	inc	ax
	mov	cx, y1
	mov	dx, y2
	int	33h
	ret
setMouseWindow endp

setMousePos proc
	arg x : word, y : word

	mov	ax, 04h
	mov	cx, x
	mov	dx, y
	int	33h
	ret
setMousePos endp

flipScreen proc
	arg src : dword, dst : dword

	push	ds
	les	di, dst
	lds	si, src
	mov	cx, 16000
	rep	movsd
	pop	ds
	ret
flipScreen endp

getPixel proc
	arg x : word, y : word

	push ds
	push si
	lds	si, vbuff1
	add	si, x
	mov	bx, y
	shl	bx, 6
	add	bh, byte ptr y
	add	si, bx
	lodsb
	pop	si
	pop	ds
	ret
getPixel endp

putPixel proc
	arg x : word, y : word, col : byte

	les	di, vbuff2
	add	di, x
	mov	bx, y
	shl	bx, 6
	add	bh, byte ptr y
	add	di, bx
	mov	al, col
	stosb
	ret
putPixel endp

setPalette proc
	arg pal : dword

	push ds
	mov	dx, 03C8h
	xor	al, al
	out	dx, al
	inc	dx
	lds	si, pal
	mov	cx, 768
	rep	outsb
	pop	ds
	ret
setPalette endp

clearTextMem proc
	les	di, tmem
	xor	ax, ax
	mov	cx, 2000
	rep	stosw
	ret
clearTextMem endp

clearBuffer proc
	arg buff : dword

	les	di, buff
	xor	ax, ax
	mov	cx, 16000
	rep	stosd
	ret
clearBuffer endp

writeString proc
	arg x : word, y : word, col : byte, msg : dword

	push ds
	lds	si, msg
	les	di, tmem
	add	di, x
	shl	di, 1
	mov	bx, y
	shl	bx, 5
	add	di, bx
	shl	bx, 2
	add	di, bx
	mov	ah, col
next:
	lodsb
	test al, al
	jz quit
	stosw
	jmp	next
quit:
	pop	ds
	ret
writeString endp

waitPressKey proc
	arg key : word

	mov	dx, 60h
	xor	ah, ah
wt:
	in	al, dx
	cmp	ax, key
	jne	wt
	ret
waitPressKey endp

end
