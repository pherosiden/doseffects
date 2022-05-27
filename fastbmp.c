/*---------------------------------------*/
/*  FASTEST BMP LOADER (320x200x256)     */
/* Copyright (c) 1998 by Nguyen Ngoc Van */
/* E-mail  : pherosiden@gmail.net        */
/* Website : http://www.codedemo.net     */
/*---------------------------------------*/

#include <dos.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <conio.h>

uint8_t pal[1024] = {0};
uint8_t vbuff[64000] = {0};

void setRGB()
{
    __asm {
        lea     di, pal
        mov     cx, 256
        mov     dx, 0x03C8
        xor     al, al
        out     dx, al
        inc     dx
    step:
        mov     al, [di + 2]
        shr     al, 2
        out     dx, al
        mov     al, [di + 1]
        shr     al, 2
        out     dx, al
        mov     al, [di]
        shr     al, 2
        out     dx, al
        add     di, 4
        loop    step
    }
}

void loadBMP(char *fname)
{
    FILE *fp = fopen(fname, "rb");
    if (!fp) return;

    fseek(fp, 54, SEEK_SET);
    fread(pal, 1, 1024, fp);
    fread(vbuff, 1, 64000, fp);
    fclose(fp);
    setRGB(pal);
}

void flipScreen()
{
    __asm {
        mov     ax, seg vbuff
        mov     ds, ax
        mov     ax, 0xA000
        mov     es, ax
        xor     di, di
        mov     si, 63680
        mov     bx, 200
    next:
        mov     cx, 80
        rep	    movsd
        sub     si, 640
        dec     bx
        jnz     next
    }
}

void main()
{
    __asm {
        mov     ax, 0x13
        int     0x10
    }

    loadBMP("assets/face.bmp");
    flipScreen();
    while(!kbhit());

    __asm {
        mov     ax, 0x03
        int     0x10
    }
}
