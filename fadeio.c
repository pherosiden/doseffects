/*---------------------------------------------------*/
/* Packet  : Demo & Effect                           */
/* Effect  : Fade (In/Out/Min/Max)                   */ 
/* Author  : Nguyen Ngoc Van                         */
/* Memory  : Compact                                 */
/* Heaps   : 640K                                    */
/* Address : siden@codedemo.net                      */
/* Website : http://www.codedemo.net                 */
/* Created : 16/01/1998                              */
/* Please sent to me any bugs or suggests.           */
/* You can use freely this code. Have fun :)         */
/*---------------------------------------------------*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

uint8_t src[768] = {0};
uint8_t dst[768] = {0};
uint8_t vbuff[64000] = {0};

void flip()
{
    __asm {
        mov     ax, 0xA000
        mov     es, ax
        xor     di, di
        mov     ax, seg vbuff
        mov     ds, ax
        xor     si, si
        mov     cx, 16000
        rep     movsd
    }
}

void clearTextMem()
{
    __asm {
        mov     ax, 0xB800
        mov     es, ax
        xor     di, di
        xor     ax, ax
        mov     cx, 2000
        rep     stosw
    }
}

void printStr(int16_t x, int16_t y, uint8_t col, char *msg)
{
    int16_t len = strlen(msg);

    __asm {
        mov     ax, 0xB800
        mov     es, ax
        xor     di, di
        lds     si, msg
        add     di, x
        shl     di, 1
        mov     bx, y
        shl     bx, 5
        add     di, bx
        shl     bx, 2
        add     di, bx
        mov     ah, col
        mov     cx, len
    next:
        lodsb
        stosw
        loop    next
    }
}

void fadeIn()
{
    __asm {
        mov     cx, 64
        mov     dx, 64
        mov     ax, seg dst
        mov     es, ax
        mov     ax, seg src
        mov     ds, ax
        mov     si, offset src
        mov     di, offset dst
    next:
        push    cx
        push    si
        push    di
        dec     dx
        mov     cx, 768
        xor     bx, bx
        xor     ax, ax
    cont:
        lodsb
        inc     bx
        cmp     al, dl
        jbe     quit
        inc     byte ptr es:[di + bx - 1]
    quit:
        loop    cont
        push    dx
        mov     dx, 0x03DA
    waitH:
        in      al, dx
        test    al, 0x08
        jnz     waitH
    waitV:
        in      al, dx
        test    al, 0x08
        jz      waitV
        mov     dx, 0x03C8
        xor     al, al
        out     dx, al
        inc     dx
        lea     si, dst
        mov     cx, 768
        rep     outsb
        pop     dx
        pop     di
        pop     si
        pop     cx
        loop    next
    }
}

void fadeOut()
{
    __asm {
        mov     cx, 64
        mov     dx, 64
        mov     ax, seg dst
        mov     es, ax
        mov     ax, seg src
        mov     ds, ax
        mov     si, offset src
        mov     di, offset dst
    next:
        push    cx
        push    si
        push    di
        dec     dx
        mov     cx, 768
        xor     bx, bx
    cont:
        lodsb
        inc     bx
        cmp     al, dl
        jae     quit
        dec     byte ptr es:[di + bx - 1]
    quit:
        loop    cont
        push    dx
        mov     dx, 0x03DA
    waitH:
        in      al, dx
        test    al, 0x08
        jnz     waitH
    waitV:
        in      al, dx
        test    al, 0x08
        jz      waitV
        mov     dx, 0x03C8
        xor     al, al
        out     dx, al
        inc     dx
        lea     si, dst
        mov     cx, 768
        rep     outsb
        pop     dx
        pop     di
        pop     si
        pop     cx
        loop    next
    }
}

void fadeMax()
{
    __asm {
        mov     ax, seg dst
        mov     ds, ax
        mov     si, offset dst
        mov     cx, 64
    next:
        push    cx
        push    si
        mov     cx, 768
    cont:
        lodsb
        cmp     al, 63
        jae     quit
        inc     byte ptr ds:[si - 1]
    quit:
        loop    cont
        mov     dx, 0x03DA
    waitH:
        in      al, dx
        test    al, 0x08
        jnz     waitH
    waitV:
        in      al, dx
        test    al, 0x08
        jz      waitV
        mov     dx, 0x03C8
        xor     al, al
        out     dx, al
        inc     dx
        lea     si, dst
        mov     cx, 768
        rep     outsb
        pop     si
        pop     cx
        loop    next
    }
}

void fadeMin()
{
    __asm {  
        mov     ax, seg dst
        mov     ds, ax
        mov     si, offset dst
        mov     cx, 64
    next:
        push    cx
        push    si
        mov     cx, 768
    cont:
        lodsb
        test    al, al
        jbe     quit
        dec     byte ptr ds:[si - 1]
    quit:
        loop    cont
        mov     dx, 0x03DA
    waitH:
        in      al, dx
        test    al, 0x08
        jnz     waitH
    waitV:
        in      al, dx
        test    al, 0x08
        jz      waitV
        mov     dx, 0x03C8
        xor     al, al
        out     dx, al
        inc     dx
        lea     si, dst
        mov     cx, 768
        rep     outsb
        pop     si
        pop     cx
        loop    next
    }
}

void main()
{
    FILE *fp;
    
    clearTextMem();
    printStr(1, 1, 0x0F, "FadeOI - (c) 1998 by Nguyen Ngoc Van");
    printStr(1, 2, 0x07, "Press any key to continue...");

    __asm {
    next:
        xor     ah, ah
        int     0x16
        and     al, al
        jz      next
    }

    fp = fopen("assets/arnold.cel", "rb");
    if (!fp) return;

    fseek(fp, 32, SEEK_SET);
    fread(src, 1, 768, fp);
    fread(vbuff, 1, 64000, fp);
    fclose(fp);

    __asm {
        mov     ax, 0x13
        int     0x10
        mov     dx, 0x03C8
        xor     al, al
        out     dx, al
        inc     dx
        mov     cx, 768
        rep     outsb
    }

    flip();
    fadeIn();
    fadeMax();
    fadeOut();
    fadeMin();

    __asm {
        mov     ax, 0x03
        int     0x10
    }
}
