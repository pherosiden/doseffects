#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <conio.h>
#include <math.h>

#define swap(a, b)  {int16_t t = a; a = b; b = t;}

uint8_t vbuff[64000] = {0};
float idx = 0, sinus[1000] = {0};

int16_t roundf(float x)
{
    if (x > 0.0) return x + 0.5;
    return x - 0.5;
}

void blur()
{
    __asm {
        mov     ax, seg vbuff
        mov     es, ax
        mov     di, 320
        mov     cx, 63360
        xor     ax, ax
        xor     bx, bx
    lp:
        mov     al, es:[di - 1]
        mov     bl, es:[di + 1]
        add     ax, bx
        mov     bl, es:[di - 320]
        add     ax, bx
        mov     bl, es:[di + 320]
        add     ax, bx
        shr     ax, 2
        stosb
        loop    lp
    }
}

void putPixel(int16_t x, int16_t y, uint8_t c)
{
    __asm {
        mov     ax, seg vbuff
        mov     es, ax
        xor     di, di
        mov     bx, y
        shl     bx, 6
        add     bh, byte ptr y
        add     bx, x
        add     di, bx
        mov     al, c
        stosb
    }
}

void flip()
{
    __asm {
        mov     ax, seg vbuff
        mov     ds, ax
        xor     si, si
        mov     ax, 0xA000
        mov     es, ax
        xor     di, di
        mov     cx, 16000
        rep     movsd
    }
}

void setPalette(uint8_t no, uint8_t r, uint8_t g, uint8_t b)
{
    __asm {
        mov     dx, 0x03C8
        mov     al, no
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

void retrace()
{
    __asm {
        mov     dx, 0x03DA
    wait1:
        in      al, dx
        and     al, 0x08
        jnz     wait1
    wait2:
        in      al, dx
        and     al, 0x08
        jz      wait2
    }
}

void drawLine(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint8_t color)
{
    int16_t d, sign;
    int16_t x, y, dx, dy, dt, ds;
    
    if (abs(x2 - x1) < abs(y2 - y1))
    {
        if (y1 > y2)
        {
            swap(x1, x2);
            swap(y1, y2);
        }

        x = x1;
        y = y1;

        dx = abs(x2 - x1);
        dy = y2 - y1;

        dt = (dx - dy) << 1;
        ds = dx << 1;

        d = (dx << 1) - dy;
        sign = (x2 > x1) ? 1 : -1;

        putPixel(x, y, color);

        for (y = y1 + 1; y <= y2; y++)
        {
            if (d >= 0)
            {
                x += sign;
                d += dt;
            }
            else
            {
                d += ds;
            }

            putPixel(x, y, color);
        }
    }
    else
    {
        if (x1 > x2)
        {
            swap(x1, x2);
            swap(y1, y2);
        }

        x = x1;
        y = y1;

        dx = x2 - x1;
        dy = abs(y2 - y1);

        dt = (dy - dx) << 1;
        ds = dy << 1;

        d = (dy << 1) - dx;
        sign = (y2 > y1) ? 1 : -1;

        putPixel(x, y, color);

        for (x = x1 + 1; x <= x2; x++)
        {
            if (d >= 0)
            {
                y += sign;
                d += dt;
            }
            else
            {
                d += ds;
            }

            putPixel(x, y, color);
        }
    }
}

void pierra(int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t num, int16_t ko)
{
    int16_t i, j, h, k, m;
    float x, y, xx , yy, dx, dy;

    x = x1;
    y = y1;

    dx = (float)(x2 - x1) / num;
    dy = (float)(y2 - y1) / num;

    for (m = 1; m <= num && !kbhit(); m++)
    {
        for (h = roundf(y - ko); h <= roundf(y + ko); h++)
        {
            for (k = roundf(x - ko); k <= roundf(x + ko); k++) putPixel(k, h, 255);
        }

        x += dx;
        y += dy;
        idx += 0.02;

        xx = sinus[roundf(idx * 159) % 1000] * sinus[roundf(idx * 83 + 130) % 1000] * 140;
        yy = sinus[roundf(idx * 97 + 153) % 1000] * sinus[roundf(idx * 107) % 1000] * 80;

        for (i = 158 + roundf(-xx); i <= 162 + roundf(-xx); i++)
        {
            for (j = 98 + roundf(-yy); j <= 102 + roundf(-yy); j++) putPixel(i, j, 155);
        }

        for (i = 158 + roundf(xx); i <= 162 + roundf(xx); i++)
        {
            for (j = 98 + roundf(yy); j <= 102 + roundf(yy); j++) putPixel(i, j, 155);
        }

        drawLine(30, 30, 110, 30, 55);
        drawLine(70, 30, 70, 140, 55);
        drawLine(130, 110, 160, 90, 55);
        drawLine(160, 90, 140, 70, 55);
        drawLine(140, 70, 120, 90, 55);
        drawLine(120, 90, 140, 140, 55);
        drawLine(140, 140, 170, 130, 55);
        drawLine(200, 140, 200, 70, 55);
        drawLine(200, 70, 230, 80, 55);
        drawLine(230, 80, 210, 100, 55);
        drawLine(210, 100, 230, 140, 55);
        drawLine(270, 70, 290, 100, 55);
        drawLine(290, 100, 270, 140, 55);
        drawLine(270, 140, 250, 100, 55);
        drawLine(250, 100, 270, 70, 55);
        drawLine(20, 170, 300, 170, 55);
        blur();
        flip();
    }
}

void main()
{
    int16_t i;

    idx = 0.0;

    for (i = 0; i < 1000; i++)
    {
        sinus[i] = sin(i * (float)0.00628318530718);
    }

    __asm {
        mov     ax, 0x13
        int     0x10
    }

    for (i = 0; i < 64; i++) setPalette(i, i, 0, 0);
    for (i = 64; i < 128; i++) setPalette(i, 63, i, 0);
    for (i = 128; i < 192; i++) setPalette(i, 63, 63, i);
    for (i = 192; i < 256; i++) setPalette(i, 63, 63, 63);
    
    do {
        pierra(30, 30, 110, 30, 40, 5);
        pierra(70, 30, 70, 140, 30, 5);
        pierra(130, 110, 160, 90, 20, 5);
        pierra(160, 90, 140, 70, 20, 5);
        pierra(140, 70, 120, 90, 20, 5);
        pierra(120, 90, 140, 140, 20, 5);
        pierra(140, 140, 170, 130, 20, 5);
        pierra(200, 140, 200, 70, 20, 5);
        pierra(200, 70, 230, 80, 20, 5);
        pierra(230, 80, 210, 100, 20, 5);
        pierra(210, 100, 230, 140, 20, 5);
        pierra(270, 70, 290, 100, 20, 5);
        pierra(290, 100, 270, 140, 20, 5);
        pierra(270, 140, 250, 100, 20, 5);
        pierra(250, 100, 270, 70, 20, 5);
        pierra(20, 170, 300, 170, 25, 5);
        pierra(300, 170, 20, 170, 25, 5);
    } while (!kbhit());

    __asm {
        mov     ax, 0x03
        int     0x10
    }
}
