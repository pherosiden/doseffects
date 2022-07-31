/*---------------------------------------------------*/
/* Packet  : Demo & Effect                           */
/* Effect  : Fire                                    */
/* Author  : Nguyen Ngoc Van                         */
/* Memory  : Small                                   */ 
/* Address : pherosiden@gmail.com                    */
/* Website : http://www.codedemo.net                 */
/* Created : 02/02/1998                              */
/* Please sent to me any bugs or suggests.           */
/* You can use freely this code. Have fun :)         */
/*---------------------------------------------------*/

#include <dos.h>
#include <time.h>
#include <stdlib.h>
#include <stdint.h>
#include <conio.h>

typedef struct tagRGB {
    uint8_t r, g, b;
} RGB;

RGB pal[256] = {0};
uint8_t dbuff[320][80] = {0};
uint8_t *tmem = (uint8_t*)0xA0000000L;

int16_t random(int16_t x)
{
    if (x == 0) return 0;
    return rand() % x;
}

void putPixel(int16_t x, int16_t y, uint8_t col)
{
    __asm {
        les     di, tmem
        mov     bx, y
        shl     bx, 6
        add     bh, byte ptr y
        add     bx, x
        mov     di, bx
        mov     al, col
        stosb
    }
}

void interpolation()
{
    int16_t x, y;
    for (x = 1; x < 319; x++)
    {
        for (y = 1; y < 79; y++) dbuff[x][y] = (2 * dbuff[x][y + 1] + dbuff[x + 1][y + 1] + dbuff[x - 1][y - 1]) >> 2;
    }
}

void putFire()
{
    int16_t x, y;
    for (x = 0; x <= 319; x++)
    {
        for (y = 0; y <= 79; y++) putPixel(x, y + 120, dbuff[x][y]);
    }
}

void makePalette()
{
    int16_t i;

    for (i = 0; i <= 63; i++)
    {
        pal[i +   0].r = i;  pal[i +   0].g = 0; pal[i +   0].b = 0;
        pal[i +  64].r = 63; pal[i +  64].g = i; pal[i +  64].b = 0;
        pal[i + 128].r = 63; pal[i + 128].g = i; pal[i + 128].b = 0;
    }

    __asm {
        mov     dx, 0x03C8
        xor     al, al
        out     dx, al
        inc     dx
        lea     si, pal
        mov     cx, 384
        rep     outsb
    }
}

void retrace()
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

void main()
{
    int16_t x;

    __asm {
        mov     ax, 0x13
        int     0x10
    }

    srand(time(NULL));
    makePalette();

    do {
        for (x = 0; x < 320; x++) dbuff[x][79] = random(100) + 40;
        interpolation();
        retrace();
        putFire();
    } while(!kbhit());

    __asm {
        mov     ax, 0x03
        int     0x10
    }
}
