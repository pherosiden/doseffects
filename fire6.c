/*---------------------------------------------------*/
/* Packet  : Demo & Effect                           */
/* Effect  : fire                                    */
/* Author  : Nguyen Ngoc Van                         */
/* Memory  : Tiny                                    */
/* Address : pherosiden@gmail.com                    */
/* Website : http://www.codedemo.net                 */
/* Created : 30/01/1998                              */
/* Please sent to me any bugs or suggests.           */
/* You can use freely this code. Have fun :)         */
/* Generate .com file: buildcom.bat fire6.c          */
/*---------------------------------------------------*/

#include <dos.h>
#include <time.h>
#include <stdlib.h>
#include <stdint.h>
#include <conio.h>

uint8_t flames[330][80] = {0};

void putPixel(int16_t x, int16_t y, uint8_t col)
{
    __asm {
        mov     ax, 0xA000
        mov     es, ax
        mov     bx, y
        shl     bx, 6
        add     bh, byte ptr y
        add     bx, x
        cmp     bx, 64000
        jae     quit
        mov     di, bx
        mov     al, col
        stosb
    quit:
    }
}

void setRGB(uint8_t c, uint8_t r, uint8_t g, uint8_t b)
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

void setFirePalette()
{
    int16_t i;

    for (i = 0; i < 50; i++)
    {
        setRGB(i, 0, 0, 0);
        setRGB(i + 50, i, 0, 0);
        setRGB(i + 100, 49, i, 0);
        setRGB(i + 150, 49, 49, i);
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

void fire()
{
    int16_t x, y, col;

    for (x = 1; x < 330 - 1; x++)
    {
        for (y = 0; y < 80 - 2; y++)
        {
            col = (flames[x - 1][y + 1] + flames[x][y + 1] + flames[x + 1][y + 1] + flames[x][y + 2]) >> 2;
            if (y == 77) col = (rand() % 80) + 120;
            if (col > 0) col--;
            flames[x][y] = col;
            if (x < 320 && y < 200) putPixel(x + 64, y - 80, col);
        }
    }
}

void main()
{
    __asm {
        mov     ax, 0x13
        int     0x10
    }
    
    srand(time(NULL));
    setFirePalette();

    while (!kbhit())
    {
        retrace();
        fire();
    }

    __asm {
        mov     ax, 0x03
        int     0x10
    }
}
