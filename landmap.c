/*----------------------------------------------------*/
/* Packet  : Demo & Effect                            */
/* Effect  : Land Mapping                             */
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
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <conio.h>

uint8_t map[200][320] = {0};
uint8_t *vmem = (uint8_t*)0xA0000000L;
uint8_t *tmem = (uint8_t*)0xB8000000L;

void setRGB(uint8_t col, uint8_t r, uint8_t g, uint8_t b)
{
    __asm {
        mov     dx, 0x03C8
        mov     al, col
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

void clearMap(uint8_t *mem)
{
    __asm {
        les     di, mem
        xor     eax, eax
        mov     cx, 16000
        rep     stosd
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
    int16_t len = strlen(msg);

    __asm {
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
        mov     cx, len
    next:
        lodsb
        stosw
        loop    next
    }
}

void drawMap(uint8_t *where, uint8_t *buff)
{
    __asm {
        les     di, buff
        lds     si, where
        mov     dx, 200
    next:
        mov     cx, 80
        rep     movsd
        dec     dx
        jnz     next
    }
}

inline int16_t random(int16_t x)
{
    if (x == 0) return 0;
    return rand() % x;
}

inline uint8_t genColor(int16_t mc, int16_t n, int16_t dvd)
{
    if (dvd == 0) return 0;
    return (mc + n - random(2 * n)) / dvd;
}

void makePlasma(int16_t x1, int16_t y1, int16_t x2, int16_t y2)
{
    int16_t xn, yn, dxy, p1, p2, p3, p4;

    if (x2 - x1 < 2 && y2 - y1 < 2) return;

    p1 = map[y1][x1];
    p2 = map[y2][x1];
    p3 = map[y1][x2];
    p4 = map[y2][x2];

    xn = (x2 + x1) >> 1;
    yn = (y2 + y1) >> 1;
    dxy = (x2 - x1 + y2 - y1) * 5 / 3;

    if (!map[y1][xn]) map[y1][xn] = genColor(p1 + p3, dxy, 2);
    if (!map[yn][x1]) map[yn][x1] = genColor(p1 + p2, dxy, 2);
    if (!map[yn][x2]) map[yn][x2] = genColor(p3 + p4, dxy, 2);
    if (!map[y2][xn]) map[y2][xn] = genColor(p2 + p4, dxy, 2);

    map[yn][xn] = genColor(p1 + p2 + p3 + p4, dxy, 4);
    if (x2 - x1 > 10) drawMap(map[0], vmem);

    makePlasma(x1, y1, xn, yn);
    makePlasma(xn, y1, x2, yn);
    makePlasma(x1, yn, xn, y2);
    makePlasma(xn, yn, x2, y2);
}

void main()
{
    int16_t i;

    clearScreen();
    printStr(1, 1, 0x0F, "LAND MAP - (c) 1998 by Nguyen Ngoc Van");
    printStr(1, 2, 0x07, "Press any key to start demo...");
    getch();

    __asm {
        mov     ax, 0x13
        int     0x10
    }

    clearMap(map[0]);
    srand(time(NULL));

    for (i = 0; i < 64; i++)
    {
        setRGB(i, 0, 0, i);
        setRGB(i + 63, 63 - (i >> 2), 63 - (i >> 1), 0);
        setRGB(i + 127, 0, 63 - (i >> 1), 0);
        setRGB(i + 192, i, i, i);
    }

    makePlasma(0, 0, 319, 199);
    drawMap(map[0], vmem);
    getch();

    __asm {
        mov     ax, 0x03
        int     0x10
    }
}
