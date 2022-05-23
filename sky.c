/*----------------------------------------------------*/
/* Packet  : Demo & Effect                            */
/* Effect  : Sky                                      */
/* Author  : Nguyen Ngoc Van                          */
/* Memory  : Compact                                  */
/* Heaps   : 640K                                     */
/* Address : pherosiden@gmail.com                     */
/* Website : http://www.codedemo.net                  */
/* Created : 12/01/1998                               */       
/* Please sent to me any bugs or suggests.            */
/* You can use freely this code. Have fun :)          */
/*----------------------------------------------------*/

#include <dos.h>
#include <mem.h>
#include <math.h>
#include <time.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <conio.h>
#include <stdio.h>

typedef struct {
    uint16_t segment;
    uint16_t posx, posy;
    uint16_t incx, incy;
} TSkyLayer;

uint16_t segbuff = 0;
uint16_t fromcol, tocol, curcolor;

TSkyLayer layer[2] = {0};
uint8_t __far dist[100][320] = {0};
uint8_t __far angle[100][320] = {0};
uint8_t vbuff[200][320] = {0};

uint16_t allocDOSMem(uint16_t pages)
{
    __asm {
        mov     ah, 0x48
        mov     bx, pages
        int     0x21
    }
}

void freeDOSMem(uint16_t segment)
{
    __asm {
        mov     ah, 0x49
        mov     es, segment
        int     0x21
    }
}

void segAlloc(uint16_t *segment)
{
    segbuff = allocDOSMem(4096);
    *segment = segbuff;
}

void segFree(uint16_t segment)
{
    freeDOSMem(segment);
}

void segPutPixel(uint8_t x, uint8_t y, uint8_t color)
{
    *(uint8_t*)MK_FP(segbuff, (y << 8) + x) = color;
}

uint8_t segGetPixel(uint8_t x, uint8_t y)
{
    return *(uint8_t*)MK_FP(segbuff, (y << 8) + x);
}

inline int16_t random(int16_t x)
{
    if (x == 0) return 0;
    return rand() % x;
}

void swapWord(uint16_t *a, uint16_t *b)
{
    *a ^= *b;
    *b ^= *a;
    *a ^= *b;
}

void newColor(uint16_t x0, uint16_t y0, uint16_t x, uint16_t y, uint16_t x1, uint16_t y1)
{
    if (segGetPixel(x, y)) return;

    curcolor = (x1 - x0) + (y1 - y0);
    curcolor = random(curcolor << 1) - curcolor;
    curcolor += ((segGetPixel(x0, y0) + segGetPixel(x1, y1) + 1) >> 1);

    if (curcolor < fromcol) curcolor = fromcol;
    else if (curcolor > tocol) curcolor = tocol;

    segPutPixel(x, y, curcolor);
}

void subDivide(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
    uint16_t x, y;

    if ((x1 - x0) < 2 && (y1 - y0) < 2) return;

    x = (x0 + x1 + 1) >> 1;
    y = (y0 + y1 + 1) >> 1;

    newColor(x0, y0, x, y0, x1, y0);
    newColor(x1, y0, x1, y, x1, y1);
    newColor(x0, y1, x, y1, x1, y1);
    newColor(x0, y0, x0, y, x0, y1);

    curcolor = (segGetPixel(x0, y0) + segGetPixel(x1, y0) + segGetPixel(x1, y1) + segGetPixel(x0, y1) + 2) >> 2;
    segPutPixel(x, y, curcolor);

    subDivide(x0, y0, x, y);
    subDivide(x, y0, x1, y);
    subDivide(x, y, x1, y1);
    subDivide(x0, y, x, y1);
}

void segPlasma()
{
    uint16_t range;

    for (curcolor = 0; curcolor < 256; curcolor++) memset(MK_FP(segbuff, curcolor << 8), 0, 256);
    if (fromcol > tocol) swapWord(&fromcol, &tocol);

    range = tocol - fromcol + 1;
    segPutPixel(0, 0, fromcol + random(range));
    subDivide(0, 0, 256, 256);
}

void flip(uint8_t *src, uint16_t dst)
{
    __asm {
        mov     es, dst
        lds     si, src
        xor     di, di
        mov     cx, 16000
        rep     movsd
    }
}

void setColor(uint8_t c, uint8_t r, uint8_t g, uint8_t b)
{
    __asm {
        mov     dx, 0x03C8
        mov     al, c
        out     dx, al
        inc     dx
        mov     al, r
        out     dx, al
        mov     al, g
        out     dx, al
        mov     al, b
        out     dx, al
    }
}

void calcSky(uint16_t factor)
{
    uint16_t x, y;

    for (y = 0; y < 100; y++) memset(dist[y], log(100 - y) * (factor >> 1), 320);

    for (y = 0; y < 100; y++)
    {
        for (x = 0; x < 160; x++)
        {
            curcolor = (x * factor) / (159 - y);
            angle[y][x + 160] = curcolor + factor;
            curcolor = ((159 - x) * factor) / (159 - y);
            angle[y][x] = factor - curcolor - 1;
        }
    }
}

void showSkyPixel(uint16_t i, uint16_t x, uint16_t y)
{
    uint16_t xm, ym;

    segbuff = layer[i].segment;

    xm = angle[y][x] + layer[i].posx;
    ym = dist[y][x] + layer[i].posy;

    if (i > 0 && segGetPixel(xm, ym) < 150) showSkyPixel(i - 1, x, y);
    else vbuff[y][x] = segGetPixel(xm, ym);
}

void retrace()
{
    __asm {
        mov     dx, 0x03DA
    waitH:
        in      al, dx
        test    al, 0x08
        jnz     waitH
    waitV:
        in      al, dx
        test    al, 0x08
        jz      waitV
    }
}

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

void main()
{
    uint16_t x, y;

    srand(time(NULL));	

    clearScreen();
    printStr(1, 1, 0x0F, "PLASMA SKY - (c) 1998 by Nguyen Ngoc Van");
    printStr(1, 2, 0x07, "Press any key to start demo...");
    getch();

    __asm {
        mov     ax, 0x13
        int     0x10
    }

    for (x = 1; x < 128; x++) setColor(x, 11 + x / 6, 11 + x / 6, 48 + x / 8);
    for (x = 128; x < 256; x++) setColor(x, x >> 2, x >> 2, 63);
    for (x = 100; x < 200; x++) memset(vbuff[x], 200 - x, 320);

    fromcol = 1;
    tocol = 255;
    calcSky(128);

    for (x = 0; x < 2; x++)
    {
        segAlloc(&layer[x].segment);
        layer[x].posx = 0;
        layer[x].posy = 0;
        layer[x].incx = x + 5;
        layer[x].incy = x + 5;
        segPlasma();
    }

    do {
        for (y = 0; y < 100; y++) for (x = 0; x < 320; x++) showSkyPixel(1, x, y);

        retrace();
        flip(vbuff[0], 0xA000);

        for (x = 0; x < 2; x++)
        {
            layer[x].posx += layer[x].incx;
            layer[x].posy += layer[x].incy;
        }
    } while(!kbhit());

    for (x = 0; x < 2; x++) segFree(layer[x].segment);

    __asm {
        mov     ax, 0x03
        int     0x10
    }
}
