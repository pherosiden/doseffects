/*----------------------------------------------------*/
/* Packet  : Demo & Effect                            */
/* Effect  : Hardware Scrolling                       */
/* Author  : Nguyen Ngoc Van                          */
/* Memory  : Compact                                  */
/* Heaps   : 640K                                     */
/* Address : pherosiden@gmail.com                     */
/* Website : http://www.codedemo.net                  */
/* Created : 27/02/1998                               */
/* Please sent to me any bugs or suggests.            */
/* You can use freely this code. Have fun :)          */
/*----------------------------------------------------*/

#include <dos.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <conio.h>

uint8_t pal[768] = {0};

void clearScreen()
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
    uint16_t len = strlen(msg);

    __asm {
        mov     cx, len
        test    cx, cx
        jz      quit
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
    next:
        lodsb
        stosw
        loop    next
    quit:
    }
}

void setAddress(uint16_t addr)
{
    __asm {
        mov     bx, addr
        mov     dx, 0x03DA
    lp1:
        in      al, dx
        and     al, 8
        jnz     lp1
    lp2:
        in      al, dx
        and     al, 8
        jz      lp2
        mov     dx, 0x3D4
        mov     al, 0x0D
        cli
        out     dx, al
        inc     dx
        mov     al, bl
        out     dx, al
        dec     dx
        mov     al, 0x0C
        out     dx, al
        inc     dx
        mov     al, bh
        out     dx, al
        sti
    }
}

void main()
{
    FILE *fp;
    int16_t i, j, incaddr;
    
    clearScreen();
    printStr(1, 1, 0x0F, "HARDWARE SCROLLING - (c) 1998 by Nguyen Ngoc Van");
    printStr(1, 2, 0x07, "Press any key to start demo...");
    getch();

    __asm {
        mov     ax, 0x13
        int     0x10
    }

    fp = fopen("assets/fear.cel", "rb");
    if (!fp) return;

    fseek(fp, 32, SEEK_SET);
    fread(pal, 1, 768, fp);
    fread(MK_FP(0xA000, 0), 1, 64000, fp);
    fclose(fp);

    __asm {
        mov     dx, 0x03C8
        xor     al, al
        out     dx, al
        inc     dx
        lea     si, pal
        mov     cx, 768
        rep     outsb
    }

    i = 1;
    incaddr = 1;

    while (!kbhit())
    {
        setAddress(i);
        i += incaddr;
        if (i > 100 || i < 0) incaddr = -incaddr;
    }

    for (j = 0; j <= 9; j++)
    {
        for (i = 1; i <= 4; i++) setAddress(i * 80);
        for (i = 4; i >= 1; i--) setAddress(i * 80);
    }

    __asm {
        mov     ax, 0x03
        int     0x10
    }
}
