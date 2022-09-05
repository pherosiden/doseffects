/*----------------------------------------------------*/
/* Packet  : Demo & Effect                            */
/* Effect  : Fire                                     */
/* Author  : Nguyen Ngoc Van                          */
/* Memory  : Small                                    */
/* Address : pherosiden@gmail.com                     */
/* Website : http://www.codedemo.net                  */
/* Created : 02/02/1998                               */
/* Please sent to me any bugs or suggests.            */
/* You can use freely this code. Have fun :)          */
/*----------------------------------------------------*/

#include <dos.h>
#include <math.h>
#include <mem.h>
#include <time.h>
#include <string.h>
#include <conio.h>
#include <stdlib.h>
#include <stdint.h>

#define ROOTRAND    20
#define DECAY       8
#define YMIN        100
#define SMOOTH 	    1
#define FIREMIN     50
#define XSTART      40
#define XEND        280
#define WIDTH       (XEND - XSTART)
#define MAXCOL      110
#define M_PI        3.141592f

uint8_t rgb[256][3] = {0};
uint8_t flames[WIDTH] = {0};
uint8_t *vmem = (uint8_t*)0xA0000000L;
uint8_t *tmem = (uint8_t*)0xB8000000L;

void setPal()
{
    __asm {
        mov     dx, 0x03C8
        xor     ax, ax
        out     dx, al
        inc     dx
        lea     si, rgb
        mov     cx, 768
        rep     outsb
    }
}

void clearTextMem()
{
    __asm
    {
        les     di, tmem
        xor     ax, ax
        mov     cx, 2000
        rep     stosw
    }
}

void putMsg(int16_t x, int16_t y, uint8_t col, char *msg)
{
    while (*msg)
    {
        char chr = *msg++;
        __asm {
            les     di, tmem
            add     di, x
            shl     di, 1
            mov     ax, y
            shl     ax, 5
            add     di, ax
            shl     ax, 2
            add     di, ax
            mov     ah, col
            mov     al, chr
            stosw
            inc     x
        }
    }
}

void HSI2RGB(float H, float S, float I, uint8_t *pal)
{
    float t, r, g, b;

    r = 1 + S * sin(H - 2 * M_PI / 3.0);
    g = 1 + S * sin(H);
    b = 1 + S * sin(H + 2 * M_PI / 3.0);

    t = 63.999 * I / 2.0;

    pal[0] = r * t;
    pal[1] = g * t;
    pal[2] = b * t;
}

void makePal()
{
    int16_t i;

    for (i = 0; i < MAXCOL; i++) HSI2RGB(4.6 - 1.5 * i / MAXCOL, i * 1.0 / MAXCOL, i * 1.0 / MAXCOL, rgb[i]);

    for (i = MAXCOL; i < 256; i++)
    {
        memcpy(rgb[i], rgb[i - 1], 3);

        if (rgb[i][0] < 63) rgb[i][0]++;
        if (rgb[i][0] < 63) rgb[i][0]++;

        if (!(i % 2) && rgb[i][1] < 53) rgb[i][1]++;
        if (!(i % 2) && rgb[i][2] < 63) rgb[i][2]++;
    }

    setPal();
}

void putPixel(int16_t x, int16_t y, uint8_t c)
{
    __asm {
        les     di, vmem
        mov     bx, y
        shl     bx, 6
        add     bh, byte ptr y
        add     bx, x
        add     di, bx
        mov     al, c
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

inline int16_t random(int16_t x)
{
    if (x == 0) return 0;
    return rand() % x;
}

inline int16_t xrand(int16_t val)
{
    return random((val << 1) + 1) - val;
}

void main()
{
    char key;
    int16_t i, j, x, fires, v;
    
    clearTextMem();
    putMsg(1, 1, 0x0F, "Fire Burning - (c) 1998 by Nguyen Ngoc Van");
    putMsg(1, 2, 0x07, "Control Keys");
    putMsg(1, 3, 0x07, "Space Bar : Throw-in a match");
    putMsg(1, 4, 0x07, "W         : Water effect");
    putMsg(1, 5, 0x07, "+/-       : Increase/Decrease intensity");
    putMsg(1, 6, 0x07, "ESC       : Exit demo");
    putMsg(1, 7, 0x07, "Press any key to start demo...");
    getch();

    __asm {
        mov     ax, 0x13
        int     0x10
    }

    srand(time(NULL));
    fires = 0;
    makePal();

    for (i = 0; i <= 199; i++) for (j = 0; j < 10; j++) putPixel(j, i, i);

    do {
        if (kbhit()) key = getch();
        else key = 32;

        for (i = 0; i < WIDTH; i++) putPixel(i + XSTART, 199, flames[i]);

        for (i = 0; i < WIDTH; i++)
        {
            for (j = YMIN; j <= 199; j++)
            {
                v = getPixel(i + XSTART, j);
                if (!v || v < DECAY || i <= XSTART || i >= XEND) putPixel(i + XSTART, j - 1, 0);
                else
                {
                    x = i - (random(3) - 1) + XSTART;
                    if (x >= XEND) x = XEND - 1;
                    putPixel(x, j - 1, v - random(DECAY));
                }
            }
        }

        if (!random(150) || key == ' ') memset(&flames[random(WIDTH - 10)], 255, 10);

        if (key == '-' && fires > -2) fires--;
        if (key == '+' && fires < 4) fires++;

        if ((key & 0x0DF) == 'W') for (i = 0; i < 10; i++) flames[random(WIDTH)] = 0;

        for (i = 0; i < WIDTH; i++)
        {
            x = flames[i];
            if (x < FIREMIN)
            {
                if (x > 10) x += random(6);
            }
            else
            {
                x += xrand(ROOTRAND) + fires;
                if (x > 255) x = 255;
                flames[i] = x;
            }
        }

        for (i = 0; i < WIDTH >> 3; i++)
        {
            x = random(50);
            flames[x] = 0;
            flames[WIDTH - x - 1] = 0;
        }

        for (i = SMOOTH; i < WIDTH - SMOOTH; i++)
        {
            x = 0;
            for (j = -SMOOTH; j <= SMOOTH; j++) x += flames[i + j];
            flames[i] = x / ((SMOOTH << 1) + 1);
        }
    } while (key != 27);

    __asm {
        mov     ax, 0x03
        int     0x10
    }
}
