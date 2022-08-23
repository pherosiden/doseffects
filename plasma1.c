/*----------------------------------------------------*/
/* Packet  : Demo & Effect                            */ 
/* Effect  : Plasma                                   */
/* Author  : Nguyen Ngoc Van                          */
/* Memory  : Small                                    */
/* Heaps   : 64K                                      */
/* Address : pherosiden@gmail.com                     */ 
/* Website : http://www.codedemo.net                  */ 
/* Created : 18/02/1998                               */
/* Please sent to me any bugs or suggests.            */
/* You can use freely this code. Have fun :)          */
/*----------------------------------------------------*/

#include <dos.h>
#include <math.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <conio.h>

#define 	RAD 0.017453293

uint8_t angle = 0;
uint8_t wave[256] = {0};
uint8_t sintab[256] = {0};

uint8_t *vmem = (uint8_t*)0xA0000000L;
uint8_t *tmem = (uint8_t*)0xb8000000L;

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

void initPlasma()
{
    int16_t i;

    for (i = 0; i <= 63; i++)
    {
        setRGB(i, i, 0, 0);
        setRGB(i +  64, 63, i, 0);
        setRGB(i + 128, 63, 63 - i, 0);
        setRGB(i + 192, 63 - i, 0, 0);
    }

    for (i = 0; i <= 255; i++) sintab[i] = sin(i * RAD * 360 / 256) * 32;
}

void drawPlasma()
{
    int16_t x, y;

    for (x = 0; x <= 255; x++)
    {
        wave[x] = sintab[(angle + x) & 0xFF] + sintab[((angle + x) << 1) & 0xFF] + sintab[((angle - x) << 2) & 0xFF];
    }

    for (y = 0; y <= 199; y++)
    {
        for(x = 0; x <= 319; x++) vmem[(y << 8) + (y << 6) + x] = wave[x & 0xFF] + wave[y] + 64;
    }
}

void retrace()
{
    __asm {
        mov     dx, 0x03DA
    wait1:
        in      al, dx
        and     al, 0x08
        jnz     wait1
    wait2:
        in      al, dx
        and     al, 0x08
        jz      wait2
    }
}

void main()
{
    clearScreen();
    printStr(1, 1, 0x0F, "PLASMA DEMO - (c) 1998 by Nguyen Ngoc Van");
    printStr(1, 2, 0x07, "Press any key to start demo...");
    getch();

    __asm {
        mov     ax, 0x13
        int     0x10
    }

    initPlasma();
    angle = 0;

    do {
        retrace();
        drawPlasma();
        angle = (angle + 1) & 0xFF;
    } while(!kbhit());

    __asm {
        mov     ax, 0x03
        int     0x10
    }
}
