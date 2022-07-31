/*----------------------------------------------------*/
/* Packet  : Demo & Effect                            */
/* Effect  : Motion Blur                              */ 
/* Author  : Nguyen Ngoc Van                          */
/* memory  : Compact                                  */
/* Heaps   : 640K                                     */
/* Address : pherosiden@gmail.com                     */
/* Website : http://www.codedemo.net                  */
/* Created : 15/02/1998                               */
/* Please sent to me any bugs or suggests.            */
/* You can use freely this code. Have fun :)          */ 
/*----------------------------------------------------*/

#include <dos.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <conio.h>

#define MPOS 200
#define DIVD 128
#define DIST 400
#define XSTR 1
#define YSTR 2
#define ZSTR -1
#define M_PI 3.141592f

#define Sinus(i)    (stab[i & 0xff])
#define Cosus(i)    (stab[(i + 192) & 0xff])
#define Swap(a, b)  {int16_t t = a; a = b; b = t;}

const int16_t points[][3] = {
    {-25, -25, -25}, {-25, -25, 25}, {25, -25, 25}, {25, -25, -25},
    {-25,  25, -25}, {-25,  25, 25}, {25,  25, 25}, {25,  25, -25}
};

const uint8_t planes[][4] = {
    {0, 4, 5, 1}, {0, 3, 7, 4}, {0, 1, 2, 3},
    {4, 5, 6, 7}, {7, 6, 2, 3}, {1, 2, 6, 5}
};

int16_t polyz[6] = {0};
int16_t stab[256] = {0};
uint8_t pind[6] = {0};

uint8_t vbuff[64000] = {0};
uint8_t *tmem = (uint8_t*)0xB8000000L;
uint8_t *vmem = (uint8_t*)0xA0000000L;

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

void clearMem()
{
    __asm {
        mov     ax, seg vbuff
        mov     es, ax
        xor     di, di
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

void flip()
{
    __asm {
        les     di, vmem
        mov     ax, seg vbuff
        mov     ds, ax
        xor     si, si
        mov     cx, 16000
        rep     movsd
    }
}

void horzLine(int16_t x1, int16_t x2, int16_t y, uint8_t c)
{
    __asm {
        mov     bx, x1
        and     bx, bx
        jz      quit
        mov     cx, x2
        jcxz    quit
        cmp     bx, cx
        jb      skip
        xchg    bx, cx
    skip:
        dec     bx
        inc     cx
        sub     cx, bx
        mov     ax, seg vbuff
        mov     es, ax
        xor     di, di
        mov     ax, y
        shl     ax, 6
        add     di, ax
        shl     ax, 2
        add     di, ax
        add     di, bx
        mov     al, c
        shr     cx, 1
        jnc     skip2
        stosb
    skip2:
        mov     ah, al
        rep     stosw
    quit:
    }
}

void polygon(int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t x3, int16_t y3, int16_t x4, int16_t y4, uint8_t c)
{
    int16_t mx, my, y;
    int16_t s1, s2, s3, s4;
    int16_t xpos[MPOS][2] = {0};

    mx = y1;
    if (y2 < mx) mx = y2;
    if (y3 < mx) mx = y3;
    if (y4 < mx) mx = y4;

    my = y1;
    if (y2 > my) my = y2;
    if (y3 > my) my = y3;
    if (y4 > my) my = y4;

    s1 = (y1 < y2) * 2 - 1;
    s2 = (y2 < y3) * 2 - 1;
    s3 = (y3 < y4) * 2 - 1;
    s4 = (y4 < y1) * 2 - 1;

    y = y1;
    if (y1 != y2)
    {
        do
        {
            xpos[y][y1 < y2] = (x2 - x1) * (y - y1) / (y2 - y1) + x1;
            y += s1;
        } while (y != (y2 + s1));
    }
    else xpos[y][y1 < y2] = x1;

    y = y2;
    if (y2 != y3)
    {
        do
        {
            xpos[y][y2 < y3] = (x3 - x2) * (y - y2) / (y3 - y2) + x2;
            y += s2;
        } while (y != (y3 + s2));
    }
    else xpos[y][y2 < y3] = x2;

    y = y3;
    if (y3 != y4)
    {
        do
        {
            xpos[y][y3 < y4] = (x4 - x3) * (y - y3) / (y4 - y3) + x3;
            y += s3;
        } while (y != (y4 + s3));
    }
    else xpos[y][y3 < y4] = x3;

    y = y4;
    if (y4 != y1)
    {
        do
        {
            xpos[y][y4 < y1] = (x1 - x4) * (y - y4) / (y1 - y4) + x4;
            y += s4;
        } while (y != (y1 + s4));
    }
    else xpos[y][y1 < y4] = x4;

    for (y = mx; y < my; y++) horzLine(xpos[y][0], xpos[y][1], y, c);
}

void motionBlur()
{
    __asm {
        mov     ax, seg vbuff
        mov     es, ax
        xor     di, di
    blur:
        xor     ah, ah
        mov     al, es:[di + 1]
        mov     dx, ax
        mov     al, es:[di - 1]
        add     dx, ax
        mov     al, es:[di + 320]
        add     dx, ax
        mov     al, es:[di - 320]
        add     dx, ax
        shr     dx, 2
        or      dl, dl
        jz      nodec
        dec     dl
    nodec:
        mov     es:[di - 320], dl
        inc     di
        cmp     di, 0xF8C0
        jnz     blur
    }
}

void sort(int16_t left, int16_t right)
{
    int16_t lo = left;
    int16_t hi = right;
    int16_t mid = polyz[(lo + hi) >> 1];

    do {
        while (polyz[lo] < mid) lo++;
        while (mid < polyz[hi]) hi--;

        if (lo <= hi)
        {
            Swap(polyz[lo], polyz[hi]);
            Swap(pind[lo], pind[hi]);

            lo++;
            hi--;
        }

    } while (lo <= hi);

    if (left < hi) sort(left, hi);
    if (lo < right) sort(lo, right);
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

void rotateCube()
{
    int16_t px[8] = {0};
    int16_t py[8] = {0};
    int16_t pz[8] = {0};

    int16_t x, y, n;
    int16_t i, j, k;

    uint8_t ax = 0;
    uint8_t ay = 0;
    uint8_t az = 0;

    do {
        for (n = 0; n <= 7; n++)
        {
            i = (Cosus(ay) * points[n][0] - Sinus(ay) * points[n][2]) / DIVD;
            j = (Cosus(az) * points[n][1] - Sinus(az) * i) / DIVD;
            k = (Cosus(ay) * points[n][2] + Sinus(ay) * points[n][0]) / DIVD;

            x = (Cosus(az) * i + Sinus(az) * points[n][1]) / DIVD;
            y = (Cosus(ax) * j + Sinus(ax) * k) / DIVD;

            pz[n] = (Cosus(ax) * k - Sinus(ax) * j) / DIVD + Cosus(ax) / 3;
            px[n] = 160 + (Sinus(ax) >> 1) + (-x * DIST) / (pz[n] - DIST);
            py[n] = 100 + (-y * DIST) / (pz[n] - DIST);
        }

        for (n = 0; n <= 5; n++)
        {
            polyz[n] = (pz[planes[n][0]] + pz[planes[n][1]] + pz[planes[n][2]] + pz[planes[n][3]]) >> 2;
            pind[n] = n;
        }

        sort(0, 5);

        for (n = 0; n <= 5; n++)
        polygon(px[planes[pind[n]][0]], py[planes[pind[n]][0]],
                px[planes[pind[n]][1]], py[planes[pind[n]][1]],
                px[planes[pind[n]][2]], py[planes[pind[n]][2]],
                px[planes[pind[n]][3]], py[planes[pind[n]][3]], polyz[n] + 75);

        ax += XSTR;
        ay += YSTR;
        az += ZSTR;
        
        motionBlur();
        retrace();
        flip();
    } while (!kbhit());
}

int main()
{
    int16_t i;

    clearScreen();
    printStr(1, 1, 0x0F, "MOTION BLUR - (c) 1998 by Nguyen Ngoc Van");
    printStr(1, 2, 0x07, "Press any to start demo...");
    getch();

    __asm {
        mov     ax, 0x13
        int     0x10
    }

    clearMem();

    for (i = 0; i <= 255; i++) stab[i] = sin(i * M_PI / 128) * DIVD;

    for (i = 0; i <= 31; i++)
    {
        setRGB(i, 0, 0, i);
        setRGB(i + 32, i, i >> 1, 31 - (i >> 1));
        setRGB(i + 64, 31 + i, (31 + i) >> 1, 31 - ((31 + i) >> 1));
        setRGB(i + 96, 63, 31 + i, 0);
        setRGB(i + 128, 63, 63, i);
    }

    rotateCube();

    __asm {
        mov     ax, 0x03
        int     0x10
    }

    return 1;
}
