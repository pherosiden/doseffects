/*----------------------------------------------------*/
/* Packet  : Demo & Effect                            */ 
/* Effect  : Plasma                                   */
/* Author  : Nguyen Ngoc Van                          */
/* Memory  : Small                                    */ 
/* Heaps   : 64K                                      */
/* Address : pherosiden@gmail.com                     */
/* Website : http://www.codedemo.net                  */ 
/* Created : 20/02/1998                               */ 
/* Please sent to me any bugs or suggests.            */
/* You can use freely this code. Have fun :)          */
/*----------------------------------------------------*/

#include <dos.h>
#include <math.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <conio.h>

#define M_PI 3.141592f

uint8_t pal[768] = {0};
uint8_t *vmem = (uint8_t*)0xA0000000L;
uint8_t *tmem = (uint8_t*)0xB8000000L;

void waitRetrace()
{
    __asm {
        mov     dx, 0x03DA
    waitV:
        in      al, dx
        test    al, 0x08
        jnz     waitV
    waitH:
        in      al, dx
        test    al, 0x08
        jz      waitH
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

void plasmaPalette()
{
    int16_t i;
    uint8_t r, g, b;
    
    for (i = 1; i <= 255; i++)
    {
        r = 32 + 30 * sin(i * M_PI * 2 / 255);
        pal[3 * i - 3] = r;
        g = 32 + 30 * cos(i * M_PI * 2 / 255);
        pal[3 * i - 2] = g;
        b = 32 + 30 * sin(-i * M_PI * 2 / 255);
        pal[3 * i - 1] = b;
    }

    __asm {
        mov 	dx, 0x03C8
        mov     al, 1
        out     dx, al
        inc     dx
        lea     si, pal
        mov     cx, 768
        rep     outsb
    }
}

inline void plotPixel(uint16_t x, uint16_t y, uint8_t color)
{
    if (!color) color = 255;
    vmem[(y << 8) + (y << 6) + x] = color;
}

inline float FX(float x, float y)
{
    return 128.0 + 128.0 * (cos(x * x * y / 1000000.0) + sin(y * y * x / 1000000.0));
}

void moveData(uint8_t *dst, uint8_t *src, uint16_t len)
{
    __asm {
        lds     si, src
        les     di, dst
        mov     cx, len
        mov     ax, cx
        shr     cx, 2
        rep     movsd
        mov     cl, al
        and     cl, 0x03
        rep     movsb
    }
}

void cyclepallete()
{
    uint8_t tmp[3] = {0};

    moveData(tmp, pal, 3);
    moveData(pal, &pal[3], 762);
    moveData(&pal[762], tmp, 3);
    waitRetrace();

    __asm {
        mov     dx, 0x03C8
        mov     al, 1
        out     dx, al
        inc     dx
        lea     si, pal
        mov     cx, 768
        rep     outsb
    }
}

void main()
{
    int16_t x, y;

    clearScreen();
    printStr(1, 1, 0x0F, "PLASMA DEMO - (c) 1998 by Nguyen Ngoc Van");
    printStr(1, 2, 0x07, "Press any key to start demo...");
    getch();

    __asm {
        mov     ax, 0x13
        int     0x10
    }

    plasmaPalette();

    for (y = 0; y < 200; y++)
    {
        for (x = 0; x < 320; x++) plotPixel(x, y, FX(x, y));
    }

    do cyclepallete(); while(!kbhit());
    
    __asm {
        mov     ax, 0x03
        int     0x10
    }
}
