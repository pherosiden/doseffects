/*-----------------------------------------------------*/
/* Packet  : Demo & Effect                             */
/* Effect  : Len                                       */
/* Author  : Nguyen Ngoc Van                           */
/* Memory  : Compact                                   */
/* Heaps   : 640K                                      */
/* Address : pherosiden@gmail.com                      */
/* Website : http://www.codedemo.net                   */
/* Created : 11/02/1998                                */
/* Please sent to me any bugs or suggests.             */
/* You can use freely this code. Have fun :)           */
/*-----------------------------------------------------*/

#include <dos.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <conio.h>

#define RAD 30
#define Sign(x) (((x) > 0) ? 1 : -1)

uint8_t *vmem = (uint8_t*)0xA0000000L;
uint8_t *tmem = (uint8_t*)0xB8000000L;

uint8_t pal[768] = {0};
uint8_t vbuff1[64000] = {0};
uint8_t vbuff2[64000] = {0};
int16_t x, y, xadd, yadd;

void clearMem(uint8_t *mem)
{
    __asm {
        les     di, mem
        xor     eax, eax
        mov     cx, 16000
        rep     stosd
    }
}

void flip(uint8_t *src, uint8_t *dst)
{
    __asm {
        les     di, dst
        lds     si, src
        mov     cx, 16000
        rep     movsd
    }
}

uint8_t getPixel(int16_t x, int16_t y)
{
    __asm {
        mov     ax, seg vbuff2
        mov     ds, ax
        mov     bx, y
        shl     bx, 6
        add     bh, byte ptr y
        add     bx, x
        mov     si, bx
        lodsb
    }
}

void putPixel(int16_t x, int16_t y, uint8_t col)
{
    __asm {
        mov     ax, seg vbuff1
        mov     es, ax
        mov     bx, y
        shl     bx, 6
        add     bh, byte ptr y
        add     bx, x
        mov     di, bx
        mov     al, col
        stosb
    }
}

void retrace()
{
    __asm {
        mov     dx, 0x03DA
    waitH:
        in      al, dx
        and     al, 0x08
        jnz     waitH
    waitV:
        in      al, dx
        and     al, 0x08
        jz      waitV
    }
}

void clearScreen()
{
    __asm {
        les     di, tmem
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
        lds     si, msg
        les     di, tmem
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

void stretch2Lines(int16_t x1, int16_t x2, int16_t y1, int16_t y2, int16_t yr1, int16_t yw1, int16_t yr2, int16_t yw2)
{
    int16_t dx, dy, e, d;
    int16_t dx2, sx, sy;

    dx = abs(x2 - x1);
    dy = abs(y2 - y1);

    sx = Sign(x2 - x1);
    sy = Sign(y2 - y1);

    e = (dy << 1) - dx;

    dx2 = dx << 1;
    dy = dy << 1;

    for (d = 0; d <= dx; d++)
    {
        putPixel(x1, yw1, getPixel(y1, yr1));
        putPixel(x1, yw2, getPixel(y1, yr2));

        while (e >= 0)
        {
            y1 += sy;
            e -= dx2;
        }

        x1 += sx;
        e += dy;
    }
}

void circleStretch(int16_t x0, int16_t y0, int16_t xc, int16_t yc, int16_t r)
{
    int16_t p, x, y, sx, sy;

    p = 3 - (r << 1);
    x = 0;
    y = r;

    while (x < y)
    {
        stretch2Lines(xc - y, xc + y, x0, x0 + (r << 1), r - x + y0, yc - x, r + x + y0, yc + x);
        if (p < 0) p = p + (x << 2) + 6;
        else
        {
            stretch2Lines(xc - x, xc + x, x0, x0 + (r << 1), r - y + y0, yc - y, r + y + y0, yc + y);
            p = p + ((x - y) << 2) + 10;
            y--;
        }
        x++;
    }

    if (x == y) stretch2Lines(xc - x, xc + x, x0, x0 + (r << 1), r - y + y0, yc - y, r + y + y0, yc + y);
}

void main()
{
    int16_t i;
    FILE *fp;

    clearScreen();
    printStr(1, 1, 0x0F, "SIMULATE LEN - (c) 1998 by Nguyen Ngoc Van");
    printStr(1, 2, 0x07, "Press any key to start demo...");
    getch();

    __asm {
        mov     ax, 0x13
        int     0x10
    }

    fp = fopen("assets/sunflow.cel", "rb");
    if (!fp) return;
    fseek(fp, 32, SEEK_SET);
    fread(pal, 1, 768, fp);
    fread(vbuff2, 1, 64000, fp);
    fclose(fp);

    __asm {
        mov     dx, 0x03C8
        xor     al, al
        out     dx, al
        inc     dx
        mov     cx, 768
        lea     si, pal
        rep     outsb
    }

    x = 160;
    y = 100;

    xadd = 1;
    yadd = 1;

    while (!kbhit())
    {
        flip(vbuff2, vbuff1);
        circleStretch(x - RAD, y - RAD, x, y, RAD);
        retrace();
        flip(vbuff1, vmem);

        x += xadd;
        y += yadd;

        if (x < 32) xadd = -xadd;
        if (x > 319 - 32) xadd = -xadd;
        if (y < 32) yadd = -yadd;
        if (y > 199 - 32) yadd = -yadd;
    }

    __asm {
        mov     ax, 0x03
        int     0x10
    }
}
