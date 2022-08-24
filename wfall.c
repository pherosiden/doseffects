/*------------------------------------------------------*/
/* Packet  : Demo & Effect                              */
/* Effect  : Water Fall                                 */
/* Author  : Nguyen Ngoc Van                            */
/* Memory  : Tiny/Small/Compact                         */
/* Address : pherosiden@gmail.com                       */
/* Website : http://www.codedemo.net                    */
/* Created : 23/04/1998                                 */
/* Please sent to me any bugs or suggests.              */
/* You can use freely this code. Have fun :)            */
/* Generate .COM file: wcl -zq -3 -ox -mt wfall.c       */
/*------------------------------------------------------*/

#include <dos.h>
#include <time.h>
#include <stdlib.h>
#include <stdint.h>
#include <conio.h>

void setDAC(uint8_t col, uint8_t r, uint8_t g, uint8_t b)
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

void putPixel(uint16_t x, uint16_t y, uint8_t col)
{
    __asm {
        mov     ax, 0xA000
        mov     es, ax
        xor     di, di
        add     di, x
        mov     ax, y
        shl     ax, 6
        add     ah, byte ptr y
        add     di, ax
        mov     al, col
        stosb
    }
}

inline int16_t random(int16_t x)
{
    if (x == 0) return 0;
    return rand() % x;
}

void makeDrip()
{
    int16_t x, y;
    uint16_t r, s;
    
    for (x = 0; x < 320; x++)
    {
        r = random(255) << 8;
        s = (128 + random(128)) << 3;
        for (y = 0; y < 200; y++)
        {
            if (!(r & 0xFF00)) putPixel(x, y, 1);
            else putPixel (x, y, r >> 8);
            r += s;
        }
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

void rotatePal()
{
    int16_t dac = 0, ofs = 0;

    while (!kbhit())
    {
        retrace();
        for (dac = 0; dac < 256; dac++)
        {
            if ((dac + ofs) & 0xFF) setDAC((dac + ofs) & 0xFF, dac >> 3, dac >> 3, (dac >> 3) + 32);
        }
        ofs++;
    }
}

void main()
{
    __asm {
        mov     ax, 0x13
        int     0x10
    }

    srand(time(NULL));
    makeDrip();
    rotatePal();
    
    __asm {
        mov     ax, 0x03
        int     0x10
    }
}
