/*---------------------------------------------------*/
/* Packet  : Demo & Effect                           */
/* Effect  : fire                                    */
/* Author  : Nguyen Ngoc Van                         */
/* Memory  : Small                                   */
/* Address : pherosiden@gmail.com                    */
/* Website : http://www.codedemo.net                 */
/* Created : 30/01/1998                              */
/* Please sent to me any bugs or suggests.           */
/* You can use freely this code. Have fun :)         */
/*---------------------------------------------------*/

#include <dos.h>
#include <time.h>
#include <stdlib.h>
#include <stdint.h>
#include <conio.h>

uint8_t flames[330][80] = {0};
uint8_t *vmem = (uint8_t *)0xA0000000L;

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
        test    al, 0x08
        jnz     waitH
    waitV:
        in      al, dx
        test    al, 0x08
        jz      waitV
    }
}

void fire()
{
    int16_t x, y;
    uint8_t col;
    uint16_t ofs;

    for (x = 1; x < 330 - 1; x++)
    {
        for (y = 0; y < 80 - 2; y++)
        {
            col = (flames[x - 1][y + 1] + flames[x][y + 1] + flames[x + 1][y + 1] + flames[x][y + 2]) >> 2;
            if (y == 77) col = (rand() % 80) + 120;
            if (col > 0) col--;
            flames[x][y] = col;
            if (x <= 319 && y <= 199)
            {
				ofs = ((y - 80) << 6) + ((y - 80) << 8) + x + 64;
				if (ofs > 63999) ofs = 63999;
				vmem[ofs] = col;
            }
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

    do {
        fire();
        retrace();
    } while (!kbhit());

    __asm {
        mov     ax, 0x03
        int     0x10
    }
}
