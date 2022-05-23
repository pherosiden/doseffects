/*---------------------------------------------------*/
/* Packet  : Demo & Effect                           */
/* Effect  : The Hole                                */
/* Author  : Nguyen Ngoc Van                         */
/* Memory  : Small                                   */
/* Address : pherosiden@gmail.com                    */
/* Website : http://www.codedemo.net                 */
/* Created : 05/02/1998                              */
/* Please sent to me any bugs or suggests.           */
/* You can use freely this code. Have fun :)         */ 
/*---------------------------------------------------*/

#include <dos.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <conio.h>

#define INCANG	4
#define XMOV	2
#define YMOV	4
#define M_PI 	3.141592f

uint16_t inc = 0;
int16_t sintab[450] = {0};
int16_t sinx[256] = {0};
int16_t cosx[256] = {0};

uint8_t *vmem = (uint8_t*)0x80000000L;
uint8_t *tmem = (uint8_t*)0xB8000000L;

void setColor(uint8_t col, uint8_t r, uint8_t g, uint8_t b)
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

inline void myDec(uint8_t *val, uint8_t dec)
{
    if (*val > dec) *val -= dec;
    else *val = 0;
}

inline int16_t roundf(float x)
{
    if (x > 0.0) return x + 0.5;
    return x - 0.5;
}

void makeDegradated(uint8_t r, uint8_t g, uint8_t b)
{
    int16_t i;

    for (i = 32; i >= 16; i--)
    {
        setColor(i, r, g, b);
        myDec(&r, 4);
        myDec(&g, 4);
        myDec(&b, 4);
    }
}

void clearBuffer()
{
    __asm {
        les     di, vmem
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

void putBuffer(uint8_t *src, uint16_t dst)
{
    __asm {
        push    ds
        mov     es, dst
        lds     si, src
        xor     di, di
        mov     cx, 16000
        rep     movsd
        pop     ds
    }
}

void calcTables()
{
    int16_t i;

    for (i = 0; i < 256; i++)
    {
        sinx[i] = roundf(sin(M_PI * i / 128) * 20);
        cosx[i] = roundf(cos(M_PI * i / 128) * 80);
    }

    for (i = 0; i < 450; i++) sintab[i] = roundf(sin(2 * M_PI * i / 360) * 128);
}

void drawPoint(int16_t xc, int16_t yc, int16_t rad, int16_t i, uint8_t col)
{
    uint16_t x, y;

    x = (rad * sintab[90 + i]) >> 7;
    x = 160 + xc + x;

    y = (rad * sintab[i]) >> 7;
    y = 100 + yc + y;

    if (x < 320 && y < 200)
    {
        __asm {
            les     di, vmem
            mov     al, col
            mov     bx, x
            mov     dx, y
            xchg  	dh, dl
            add     di, dx
            shr     di, 2
            add     di, dx
            add     di, bx
            stosb
        }
    }
}

void drawHole()
{
    int16_t x = 0, y = 0;
    int16_t i = 0, j = 0;
    uint8_t col;

    do {
        col = 19;
        inc = 2;
        j = 10;

        do {
            i = 0;
            do {
                drawPoint(cosx[(x + (200 - j)) & 0xff], sinx[(y + (200 - j)) & 0xff], j, i, col);
                i += INCANG;
            } while (i < 360);

            j += inc;

            if (!(j % 3))
            {
                inc++;
                col++;
                if (col > 31) col = 31;
            }
        } while (j < 220);

        x = XMOV + (x & 0xff);
        y = YMOV + (y & 0xff);

        while (inp(0x03DA) & 8);
        while (!(inp(0x03DA) & 8));

        putBuffer(vmem, 0xA000);
        clearBuffer();
    } while(!kbhit());
}

void main()
{
    clearScreen();
    printStr(1, 1, 0x0F, "THE HOLE - (c) 1998 by Nguyen Ngoc Van");
    printStr(1, 2, 0x07, "Press any key to start demo...");
    getch();

    __asm {
        mov     ax, 0x13
        int     0x10
    }

    clearBuffer();
    calcTables();
    makeDegradated(50, 50, 64);
    drawHole();

    __asm {
        mov     ax, 0x03
        int     0x10
    }
}
