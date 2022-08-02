/*----------------------------------------------------*/
/* Packet  : Demo & Effect                            */
/* Effect  : Motion Blur                              */ 
/* Author  : Nguyen Ngoc Van                          */
/* Memory  : Compact                                  */
/* Heaps   : 640K                                     */
/* Address : pherosiden@gmail.com                     */
/* Website : http://www.codedemo.net                  */
/* Created : 17/02/1998                               */
/* Please sent to me any bugs or suggests.            */
/* You can use freely this code. Have fun :)          */
/*----------------------------------------------------*/

#include <dos.h>
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <conio.h>

#define MPOS 200
#define DIVD 128
#define DIST 150
#define XSTR 1
#define YSTR 2
#define ZSTR -1
#define M_PI 3.141592f

#define Swap(a, b)  {int16_t t = a; a = b; b = t;}
#define Sinus(i)    (stab[i & 0xff])
#define Cosnus(i)   (stab[(i + 192) & 0xff])

const int16_t points[][3] = {
    {-32, -32, -32}, {-32, -32, 32}, {32, -32, 32}, {32, -32, -32},
    {-32,  32, -32}, {-32,  32, 32}, {32,  32, 32}, {32,  32, -32}
};

const uint8_t planes[][4] = {
    {0, 4, 5, 1}, {0, 3, 7, 4}, {0, 1, 2, 3},
    {4, 5, 6, 7}, {7, 6, 2, 3}, {1, 2, 6, 5}
};

int16_t stab[256] = {0};
int16_t polyz[6] = {0};
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

void horzLine(int16_t xb, int16_t xe, int16_t y, uint8_t c)
{
    __asm {
        mov     bx, xb
        and     bx, bx
        jz      quit
        mov     cx, xe
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
        mov     ax, y
        shl     ax, 6
        mov     di, ax
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
    int16_t mny, mxy, y;
    int16_t s1, s2, s3, s4;
    int16_t xpos[MPOS][2] = {0};	

    mny = y1;
    if (y2 < mny) mny = y2;
    if (y3 < mny) mny = y3;
    if (y4 < mny) mny = y4;

    mxy = y1;
    if (y2 > mxy) mxy = y2;
    if (y3 > mxy) mxy = y3;
    if (y4 > mxy) mxy = y4;

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

    for (y = mny; y < mxy; y++) horzLine(xpos[y][0], xpos[y][1], y, c);
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
        or    	dl, dl
        jz    	nodec
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
    int16_t x, y, z;
    int16_t i, j, k, n;

    int16_t px[8] = {0};
    int16_t py[8] = {0};
    int16_t pz[8] = {0};

    uint8_t ax = 1;
    uint8_t ay = 1;
    uint8_t az = 1;

    do {
        for (n = 0; n < 8; n++)
        {
            i = (Cosnus(ay) * points[n][0] - Sinus(ay) * points[n][2]) / DIVD;
            j = (Cosnus(az) * points[n][1] - Sinus(az) * i) / DIVD;
            k = (Cosnus(ay) * points[n][2] + Sinus(ay) * points[n][0]) / DIVD;

            x = (Cosnus(az) * i + Sinus(az) * points[n][1]) / DIVD;
            y = (Cosnus(ax) * j + Sinus(ax) * k) / DIVD;
            z = (Cosnus(ax) * k - Sinus(ax) * j) / DIVD;

            px[n] = 160 + (-x * DIST) / (z - DIST);
            py[n] = 100 + (-y * DIST) / (z - DIST);
            pz[n] = z;
        }

        for (n = 0; n < 6; n++)
        {
            polyz[n] = (pz[planes[n][0]] + pz[planes[n][1]] + pz[planes[n][2]] + pz[planes[n][3]]) >> 2;
            pind[n] = n;
        }

        sort(0, 5);

        for (n = 0; n < 6; n++)
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

void main()
{
    int16_t i;

    clearScreen();
    printStr(1, 1, 0x0F, "MOTION BLUR - (c) 1998 by Nguyen Ngoc Van");
    printStr(1, 2, 0x07, "Press any key to start demo...");
    getch();

    __asm {
        mov     ax, 0x13
        int     0x10
    }

    for (i = 0; i < 256; i++) stab[i] = sin(i * M_PI / 128) * DIVD;

    for (i = 0; i < 64; i++)
    {
        setRGB(i,        i,  0, 0);
        setRGB(i +  64, 63,  i, 0);
        setRGB(i + 128, 63, 63, i);
        setRGB(i + 192, 63, 63, 63);
    }

    rotateCube();

    __asm {
        mov     ax, 0x03
        int     0x10
    }
}
