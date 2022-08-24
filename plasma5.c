/*----------------------------------------------------*/
/* Packet  : Demo & Effect                            */
/* Effect  : Plasma                                   */
/* Author  : Nguyen Ngoc Van                          */ 
/* Memory  : Small                                    */
/* Heaps   : 64K                                      */
/* Address : pherosiden@gmail.com                     */
/* Website : http://www.codedemo.net                  */
/* Created : 22/02/1998                               */ 
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
#define KX (2 * M_PI / 320)
#define KY (2 * M_PI / 200)

uint8_t pal[1536] = {0};
uint8_t *vmem = (uint8_t*)0xA0000000L;
uint8_t *tmem = (uint8_t*)0xB8000000L;

void retrace()
{
    __asm {
        mov     dx, 0x03DA
    waitV:
        in      al, dx
        and     al, 0x08
        jnz     waitV
    waitH:
        in      al, dx
        and     al, 0x08
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

void setPal()
{
    __asm {
        mov     dx, 0x03C8
        xor     al, al
        out     dx, al
        inc     dx
        mov     cx, 768
        lea     si, pal
        rep     outsb
    }
}

void putPixel(int16_t x, int16_t y, uint8_t col)
{
    __asm {
        les     di, vmem
        mov     bx, y
        shl     bx, 6
        add     bh, byte ptr y
        add     bx, x
        add     di, bx
        mov     al, col
        stosb
    }
}

uint8_t getPixel(int16_t x, int16_t y)
{
    __asm {
        lds     si, vmem
        mov     bx, y
        shl     bx, 6
        add     bh, byte ptr y
        add     bx, x
        add     si, bx
        lodsb
    }
}

void moveData(uint8_t *dst, uint8_t *src, uint16_t len)
{
    __asm {
        les     di, dst
        lds     si, src
        mov     cx, len
        test    cx, cx
        jz      quit
        cmp     cx, 3
        jle     step
        shr     cx, 2
        rep     movsd
        and     cx, 3
        test    cx, cx
        jz      quit
    step:
        rep     movsb
    quit:
    }
}

void coolPal()
{
    int16_t i;
    int16_t r = 0, g = 0, b = 0, col = 0;

    for (i = 0; i < 64; i++)
    {
        pal[col++] = r;
        pal[col++] = g;
        pal[col++] = b;

        if (r < 63) r++;
        if (g < 63) g++;
        if (b < 63) b++;
    }

    for (i = 0; i < 64; i++)
    {
        pal[col++] = r;
        pal[col++] = g;
        pal[col++] = b;

        if (b > 0) b--;
    }

    for (i = 0; i < 64; i++)
    {
        pal[col++] = r;
        pal[col++] = g;
        pal[col++] = b;

        if (r > 0) r--;
        if (g > 0) g--;
        if (b < 63 && !(i % 2)) b++;
    }

    for (i = 0; i < 64; i++)
    {
        pal[col++] = r;
        pal[col++] = g;
        pal[col++] = b;

        if (g < 63) g++;
        if (b > 0 && !(i % 2)) b--;
    }

    for (i = 0; i < 64; i++)
    {
        pal[col++] = r;
        pal[col++] = g;
        pal[col++] = b;

        if (r < 63 && !(i % 2)) r++;
        if (g > 0) g--;
    }

    for (i = 0; i < 64; i++)
    {
        pal[col++] = r;
        pal[col++] = g;
        pal[col++] = b;

        if (r < 63 && !(i % 2)) r++;
        if (b < 63 && !(i % 2)) b++;
    }

    for (i = 0; i < 64; i++)
    {
        pal[col++] = r;
        pal[col++] = g;
        pal[col++] = b;

        if (b < 63 && !(i % 2)) b++;
        if (g < 63 && !(i % 2)) g++;
        if (r > 0) r--;
    }

    for (i = 0; i < 64; i++)
    {
        pal[col++] = r;
        pal[col++] = g;
        pal[col++] = b;

        if (b > 0) b--;
        if (g > 0 && !(i % 2)) g--;
    }

    setPal();
}

void rotatePal(int16_t x, int16_t y)
{
    uint8_t rgb[3] = {0};

    moveData(rgb, &pal[x * 3], 3);
    moveData(&pal[x * 3], &pal[(x + 1) * 3], y * 3);
    moveData(&pal[y * 3], rgb, 3);
    setPal();
}

void drawSinCos()
{
    int16_t x, y;
    uint8_t col;

    for (x = 0; x < 320; x++)
    {
        for(y = 0; y < 200; y++)
        {
            col = (sin(x * KX * 0.5) * sin(y * KY * 0.5)) * (127 - 20) + 128;
            putPixel(x, y, col);
        }
    }
}

void filterSinCos()
{
    int16_t x, y;
    uint8_t col;

    for (x = 0; x < 320; x++)
    {
        for(y = 0; y < 200; y++)
        {
            col = getPixel(x, y);
            col += (sin(x * KX * 10) * sin(y * KY * 10)) * 20;
            putPixel(x, y, col);
        }
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

    coolPal();
    drawSinCos();
    getch();
    filterSinCos();
    getch();
    
    while (!kbhit())
    {
        retrace();
        rotatePal(1, 512);
    }

    __asm {
        mov     ax, 0x03
        int     0x10
    }
}
