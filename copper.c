/*--------------------------------------------------------------------------*/
/* COPPER3 - 3 simultaneous copper bars, 79 bytes! - final version by       */
/* Nguyen Ngoc Van 1998. Mail : siden@codedemo.net, Site : www.codedemo.net */
/*--------------------------------------------------------------------------*/

#include <dos.h>
#include <time.h>
#include <stdlib.h>
#include <stdint.h>
#include <conio.h>

uint8_t pal[1024] = {0};

void main()
{
    __asm {
        xor     bx, bx
    m1:
        push    bx
        inc     bx
        push    bx
        jpo     m1
    m2:
        mov     ah, 8
        mov     cx, 390
    m3:
        mov     dx, 03DAh
    m4:
        in      al, dx
        and     al, ah
        jnz     m4
    m5:
        in      al, dx
        and     al, ah
        jz      m5

        mov     dl, 0C8h
        xchg    ax, bx
        out     dx, al
        inc     dx

        mov     si, sp
        mov     bl, 3
    m6:
        lodsw
        loop    m7

        sub     [si], ax
        cmp     word ptr pal[si], 1
        ja      m7
        neg     pal[si - 2]
    m7:
        inc     cx
        lodsw
        add     ax, cx
        cmp     ax, 127
        jbe     m8
        xor     al, al
    m8:
        cmp     al, 64
        jb      m9
        not     al
    m9:
        out     dx, al
        dec     bx
        jpo     m6
        mov     ah, 1
        loop    m3
        int     16h
        jz      m2
    }
}
