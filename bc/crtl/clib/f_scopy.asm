;[]-----------------------------------------------------------------[]
;|      F_SCOPY.ASM -- far struct copy routine                       |
;[]-----------------------------------------------------------------[]

;
;       C/C++ Run Time Library - Version 5.0
; 
;       Copyright (c) 1987, 1992 by Borland International
;       All Rights Reserved.
; 

                INCLUDE RULES.ASI

; calls to this routine are generated by the compiler to copy
; one "struct" value to another
;
; On entry:
;
;       CX      = Number of bytes to copy


_TEXT           SEGMENT
                ASSUME  CS:_TEXT

                public  SCOPY@
                public  F_SCOPY@

SCOPY@:
F_SCOPY@:
                push    bp
                mov     bp,sp
                push    si
                push    di
                push    ds
                lds     si,dword ptr 6[bp]
                les     di,dword ptr 10[bp]
                cld
                shr     cx, 1
                rep     movsw
                adc     cx, cx
                rep     movsb
                pop     ds
                pop     di
                pop     si
                pop     bp
                retf    8

_TEXT           ENDS
                END
