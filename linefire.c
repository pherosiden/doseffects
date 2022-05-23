/*---------------------------------------------------*/
/* Packet  : Demo & Effect                           */
/* Effect  : Fire                                    */
/* Author  : Nguyen Ngoc Van                         */
/* Memory  : Compact                                 */
/* Heaps   : 640K                                    */
/* Address : pherosiden@gmail.com                    */
/* Website : http://www.codedemo.net                 */
/* Created : 15/01/1998                              */
/* Please sent to me any bugs or suggests.           */
/* You can use freely this code. Have fun :)         */
/*---------------------------------------------------*/
#include <dos.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <conio.h>

#define SPEED 	2
#define PLUS 	1
#define MINUS 	0
#define MAXX 	320
#define MAXY 	200
#define Swap(a, b) {int16_t t = a; a = b; b = t;}

typedef struct {
    uint8_t r, g, b;
} RGB;

typedef struct {
    int16_t x0, y0, x1, y1;
    int16_t dirx0, diry0;
    int16_t dirx1, diry1;
} POINT;

POINT point = {0};
RGB curpal[1024] = {0};
RGB oldpal[6][1024] = {0};
uint8_t vbuff[200][320] = {0};

int16_t flag = 1;

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

void clearMem(uint8_t *mem)
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
        mov     ax, 0xB800
        mov     es, ax
        xor     di, di
        xor     ax, ax
        mov     cx, 2000
        rep     stosw
    }
}

void printStr(int16_t x, int16_t y, uint8_t col, char *msg)
{
    int16_t len = strlen(msg);

    __asm {
        mov     ax, 0xB800
        mov     es, ax
        xor     di, di
        lds     si, msg
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

void flip(uint8_t *src, uint16_t dst)
{
    __asm {
        mov     es, dst
        lds     si, src
        xor     di, di
        mov     cx, 16000
        rep     movsd
    }
}

void setColors()
{
    int16_t i, j;

    for (i = 0, j = 1; j < 256; i += 4, j++)
    {
        curpal[i + 0].r = j;
        curpal[i + 1].r = j;
        curpal[i + 2].r = j;
        curpal[i + 3].r = j;

        curpal[i + 0].g = 0;
        curpal[i + 1].g = 0;
        curpal[i + 2].g = 0;
        curpal[i + 3].g = 0;

        curpal[i + 0].b = 0;
        curpal[i + 1].b = 0;
        curpal[i + 2].b = 0;
        curpal[i + 3].b = 0;
    }

    for (i = 0, j = 1; j < 256; i += 4, j++)
    {
        oldpal[0][i + 0].r = 0;
        oldpal[0][i + 1].r = 0;
        oldpal[0][i + 2].r = 0;
        oldpal[0][i + 3].r = 0;

        oldpal[0][i + 0].g = j;
        oldpal[0][i + 1].g = j;
        oldpal[0][i + 2].g = j;
        oldpal[0][i + 3].g = j;

        oldpal[0][i + 0].b = 0;
        oldpal[0][i + 1].b = 0;
        oldpal[0][i + 2].b = 0;
        oldpal[0][i + 3].b = 0;
    }

    for (i = 0, j = 1; j < 256; i += 4, j++)
    {
        oldpal[1][i + 0].r = 0;
        oldpal[1][i + 1].r = 0;
        oldpal[1][i + 2].r = 0;
        oldpal[1][i + 3].r = 0;

        oldpal[1][i + 0].g = 0;
        oldpal[1][i + 1].g = 0;
        oldpal[1][i + 2].g = 0;
        oldpal[1][i + 3].g = 0;

        oldpal[1][i + 0].b = j;
        oldpal[1][i + 1].b = j;
        oldpal[1][i + 2].b = j;
        oldpal[1][i + 3].b = j;
    }

    for (i = 0, j = 1; j < 256; i += 4, j++)
    {
        oldpal[2][i + 0].r = j;
        oldpal[2][i + 1].r = j;
        oldpal[2][i + 2].r = j;
        oldpal[2][i + 3].r = j;

        oldpal[2][i + 0].g = j;
        oldpal[2][i + 1].g = j;
        oldpal[2][i + 2].g = j;
        oldpal[2][i + 3].g = j;

        oldpal[2][i + 0].b = j;
        oldpal[2][i + 1].b = j;
        oldpal[2][i + 2].b = j;
        oldpal[2][i + 3].b = j;
    }

    for (i = 0, j = 1; j < 256; i += 4, j++)
    {
        oldpal[3][i + 0].r = j;
        oldpal[3][i + 1].r = j;
        oldpal[3][i + 2].r = j;
        oldpal[3][i + 3].r = j;

        oldpal[3][i + 0].g = j;
        oldpal[3][i + 1].g = j;
        oldpal[3][i + 2].g = j;
        oldpal[3][i + 3].g = j;

        oldpal[3][i + 0].b = 0;
        oldpal[3][i + 1].b = 0;
        oldpal[3][i + 2].b = 0;
        oldpal[3][i + 3].b = 0;
    }

    for (i = 0, j = 1; j < 64; i += 4, j++)
    {
        oldpal[4][i + 0].r = j;
        oldpal[4][i + 1].r = j;
        oldpal[4][i + 2].r = j;
        oldpal[4][i + 3].r = j;

        oldpal[4][i + 0].g = 0;
        oldpal[4][i + 1].g = 0;
        oldpal[4][i + 2].g = 0;
        oldpal[4][i + 3].g = 0;

        oldpal[4][i + 0].b = j;
        oldpal[4][i + 1].b = j;
        oldpal[4][i + 2].b = j;
        oldpal[4][i + 3].b = j;
    }

    for (i = 0, j = 1; j < 64; i += 4, j++)
    {
        oldpal[5][i + 0].r = 0;
        oldpal[5][i + 1].r = 0;
        oldpal[5][i + 2].r = 0;
        oldpal[5][i + 3].r = 0;

        oldpal[5][i + 0].g = j;
        oldpal[5][i + 1].g = j;
        oldpal[5][i + 2].g = j;
        oldpal[5][i + 3].g = j;

        oldpal[5][i + 0].b = j;
        oldpal[5][i + 1].b = j;
        oldpal[5][i + 2].b = j;
        oldpal[5][i + 3].b = j;
    }
}

void changePals(int16_t k)
{
    int16_t i;

    for (i = 0; i < 256; i++)
    {
        if (curpal[i].r < oldpal[k][i].r) setRGB(i, curpal[i].r++, curpal[i].g, curpal[i].b);
        if (curpal[i].r > oldpal[k][i].r) setRGB(i, curpal[i].r--, curpal[i].g, curpal[i].b);
        if (curpal[i].g < oldpal[k][i].g) setRGB(i, curpal[i].r, curpal[i].g++, curpal[i].b);
        if (curpal[i].g > oldpal[k][i].g) setRGB(i, curpal[i].r, curpal[i].g--, curpal[i].b);
        if (curpal[i].b < oldpal[k][i].b) setRGB(i, curpal[i].r, curpal[i].g, curpal[i].b++);
        if (curpal[i].b > oldpal[k][i].b) setRGB(i, curpal[i].r, curpal[i].g, curpal[i].b--);
        flag = (curpal[i].r == oldpal[k][i].r) && (curpal[i].g == oldpal[k][i].g) && (curpal[i].b == oldpal[k][i].b) ? 0 : 1;
    }
}

void drawLine(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint8_t col)
{
    int16_t x, y, d, dx, dy, sign, dt, ds;

    if (abs(x2 - x1) < abs(y2 - y1))
    {
        if (y1 > y2)
        {
            Swap(x1, x2);
            Swap(y1, y2);
        }

        x = x1;
        y = y1;

        dx = abs(x2 - x1);
        dy = y2 - y1;

        dt = (dx - dy) << 1;
        ds = dx << 1;

        d = (dx << 1) - dy;
        sign = (x2 > x1) ? 1 : -1;

        vbuff[y][x] = col;

        for (y = y1 + 1; y <= y2; y++)
        {
            if (d >= 0)
            {
                x += sign;
                d += dt;
            } else d += ds;

            vbuff[y][x] = col;
        }
    }
    else
    {
        if (x1 > x2)
        {
            Swap(x1, x2);
            Swap(y1, y2);
        }

        x = x1;
        y = y1;

        dx = x2 - x1;
        dy = abs(y2 - y1);

        dt = (dy - dx) << 1;
        ds = dy << 1;

        d = (dy << 1) - dx;
        sign = (y2 > y1) ? 1 : -1;

        vbuff[y][x] = col;

        for (x = x1 + 1; x <= x2; x++)
        {
            if (d >= 0)
            {
                y += sign;
                d += dt;
            } else d += ds;

            vbuff[y][x] = col;
        }
    }
}

void main()
{
    int16_t x, y, var = 0;
    int16_t rad1, rad2, rad3, rad4, rad5, rad6, rad7, rad8;

    clearScreen();
    printStr(1, 1, 0x0F, "LINE BLUR - (c) 1998 by Nguyen Ngoc Van");
    printStr(1, 2, 0x07, "Press any key to start demo...");
    getch();

    memset(vbuff[0], 0, 64000);
    memset(&point, 0, sizeof(POINT));
    srand(time(NULL));

    point.x0 = (rand() % 158) + 1;
    point.y0 = (rand() % 98) + 1;
    point.x1 = (rand() % 158) + 1;
    point.y1 = (rand() % 98) + 1;

    rad1 = (rand() % 4) + 1;
    rad2 = (rand() % 1) + 1;
    rad3 = (rand() % 2) + 1;
    rad4 = (rand() % 3) + 1;
    rad5 = (rand() % 3) + 1;
    rad6 = (rand() % 4) + 1;
    rad7 = (rand() % 2) + 1;
    rad8 = (rand() % 3) + 1;

    __asm {
        mov     ax, 0x13
        int     0x10
    }

    setColors();

    do {
        if (!flag) var = rand() % 6;
        changePals(var);

        if (point.x0 > 319)
        {
            point.dirx0 = MINUS;
            rad1 = (rand() % SPEED) + 1;
        }

        if (point.x0 < 1)
        {
            point.dirx0 = PLUS;
            rad2 = (rand() % SPEED) + 1;
        }

        if (point.y0 > 199)
        {
            point.diry0 = MINUS;
            rad3 = (rand() % SPEED) + 1;
        }

        if (point.y0 < 1)
        {
            point.diry0 = PLUS;
            rad4 = (rand() % SPEED) + 1;
        }

        if (point.x1 > 319)
        {
            point.dirx1 = MINUS;
            rad5 = (rand() % SPEED) + 1;
        }

        if (point.x1 < 1)
        {
            point.dirx1 = PLUS;
            rad6 = (rand() % SPEED) + 1;
        }

        if (point.y1 > 199)
        {
            point.diry1 = MINUS;
            rad7 = (rand() % SPEED) + 1;
        }

        if (point.y1 < 1)
        {
            point.diry1 = PLUS;
            rad8 = (rand() % SPEED) + 1;
        }

        if (point.dirx0 == PLUS) point.x0 += rad1;
        if (point.dirx0 == MINUS) point.x0 -= rad2;
        if (point.diry0 == PLUS) point.y0 += rad3;
        if (point.diry0 == MINUS) point.y0 -= rad4;
        if (point.dirx1 == PLUS) point.x1 += rad5;
        if (point.dirx1 == MINUS) point.x1 -= rad6;
        if (point.diry1 == PLUS) point.y1 += rad7;
        if (point.diry1 == MINUS) point.y1 -= rad8;

        if (point.x0 < 318 && point.x0 > 1 && point.y0 > 1 && point.y0 < 198 && point.x1 < 318 && point.x1 > 1 && point.y1 > 1 && point.y1 < 198)
        {
            drawLine(point.x0, point.y0, point.x1, point.y1, 245);
        }

        for (y = 1; y < MAXY - 1; y++)
        {
            for (x = 0; x < MAXX; x++) vbuff[y][x] = (vbuff[y][x - 1] + vbuff[y][x + 1] + vbuff[y - 1][x] + vbuff[y + 1][x]) >> 2;
        }

        retrace();
        flip(vbuff[0], 0xA000);
    } while(!kbhit());

    __asm {
        mov     ax, 0x03
        int     0x10
    }
}
