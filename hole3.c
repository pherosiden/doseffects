/*----------------------------------------------------*/
/* Packet  : Demo & Effect                            */
/* Effect  : The Hole                                 */
/* Author  : Nguyen Ngoc Van                          */
/* Memory  : Compact                                  */
/* Heaps   : 640K                                     */
/* Address : pherosiden@gmail.com                     */
/* Website : http://www.codedemo.net                  */
/* Created : 10/02/1998                               */
/* Please sent to me any bugs or suggests.            */
/* You can use freely this code. Have fun :)          */
/*----------------------------------------------------*/

#include <dos.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <conio.h>

#define SWAP(a, b) {int16_t t = a; a = b; b = t;}
#define MIN(a, b) ((a < b) ? a : b)
#define MAX(a, b) ((a > b) ? a : b)
#define RANGE(a, b, c) ((a >= b && a <= c) ? a : (a <= b) ? b : c)

#define MPOS	200
#define DIVD 	128
#define STEP 	15
#define XST 	2
#define YST 	1
#define M_PI 	3.141592f

int16_t minx, miny, maxx, maxy;
int16_t sintab[450] = {0};
int16_t sinx[256] = {0};
int16_t cosx[256] = {0};
int16_t px[27][24] = {0};
int16_t py[27][24] = {0};
uint8_t vbuff[64000] = {0};

uint16_t x, y, i, j, key;
uint8_t step, ring, point, art;

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

void putPixel(uint8_t *where, uint16_t ofs, uint8_t col)
{
    __asm {
        les     di, where
        add     di, ofs
        mov     al, col
        stosb
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

void clearTextMem()
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

void putMsg(int16_t x, int16_t y, uint8_t col, char *msg)
{
    uint16_t len = strlen(msg);

    __asm {
        mov     cx, len
        test    cx, cx
        jz      quit
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
    next:
        lodsb
        stosw
        loop    next
    quit:
    }
}

void drawLine(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint8_t col, uint8_t *where)
{
    int16_t d, sign;
    int16_t x, y, dx, dy, dt, ds;

    if (x1 < 1 || y1 < 1 || x1 > 318 || y1 > 198) return;
    if (x2 < 1 || y2 < 1 || x2 > 318 || y2 > 198) return;

    if (abs(x2 - x1) < abs(y2 - y1))
    {

        if (y1 > y2)
        {
            SWAP(x1, x2);
            SWAP(y1, y2);
        }

        x = x1;
        y = y1;

        dx = abs(x2 - x1);
        dy = y2 - y1;

        dt = (dx - dy) << 1;
        ds = dx << 1;

        d = (dx << 1) - dy;
        sign = (x2 > x1) ? 1 : -1;

        putPixel(where, (y << 8) + (y << 6) + x, col);

        for (y = y1 + 1; y <= y2; y++)
        {
            if (d >= 0)
            {
                x += sign;
                d += dt;
            } else d += ds;

            putPixel(where, (y << 8) + (y << 6) + x, col);
        }
    }
    else
    {
        if (x1 > x2)
        {
            SWAP(x1, x2);
            SWAP(y1, y2);
        }

        x = x1;
        y = y1;

        dx = x2 - x1;
        dy = abs(y2 - y1);

        dt = (dy - dx) << 1;
        ds = dy << 1;

        d = (dy << 1) - dx;
        sign = (y2 > y1) ? 1 : -1;

        putPixel(where, (y << 8) + (y << 6) + x, col);

        for (x = x1 + 1; x <= x2; x++)
        {
            if (d >= 0)
            {
                y += sign;
                d += dt;
            } else d += ds;

            putPixel(where, (y << 8) + (y << 6) + x, col);
        }
    }
}

void horzLine(int16_t xb, int16_t xe, int16_t y, uint8_t c, uint8_t *where)
{
    __asm {
        mov     bx, xb
        mov     cx, xe
        cmp     bx, cx
        jb      skip
        xchg    bx, cx
    skip:
        inc     cx
        sub     cx, bx
        les     di, where
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
    }
}

void polygon1(int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t x3, int16_t y3, int16_t x4, int16_t y4, int16_t c, uint8_t *where)
{
    int16_t pos[MPOS][2] = {0};
    int16_t xdiv1, xdiv2, xdiv3, xdiv4;
    int16_t ydiv1, ydiv2, ydiv3, ydiv4;
    int16_t ly, gy, y, tmp, step;
    uint8_t dir1, dir2, dir3, dir4;

    ly = MAX(MIN(MIN(MIN(y1, y2), y3), y4), miny);
    gy = MIN(MAX(MAX(MAX(y1, y2), y3), y4), maxy);

    if (ly > maxy) return;
    if (gy < miny) return;

    dir1 = y1 < y2; xdiv1 = x2 - x1; ydiv1 = y2 - y1;
    dir2 = y2 < y3; xdiv2 = x3 - x2; ydiv2 = y3 - y2;
    dir3 = y3 < y4; xdiv3 = x4 - x3; ydiv3 = y4 - y3;
    dir4 = y4 < y1; xdiv4 = x1 - x4; ydiv4 = y1 - y4;

    y = y1;
    step = (dir1 << 1) - 1;

    if (y1 != y2)
    {
        do {
            if (RANGE(y, ly, gy) == y)
            {
                tmp = xdiv1 * (y - y1) / ydiv1 + x1;
                pos[y][dir1] = RANGE(tmp, minx, maxx);
            }
            y += step;
        } while (y != y2 + step);
    }
    else if (y >= ly && y <= gy) pos[y][dir1] = RANGE(x1, minx, maxx);

    y = y2;
    step = (dir2 << 1) - 1;

    if (y2 != y3)
    {
        do {
            if (RANGE(y, ly, gy) == y)
            {
                tmp = xdiv2 * (y - y2) / ydiv2 + x2;
                pos[y][dir2] = RANGE(tmp, minx, maxx);
            }
            y += step;
        } while (y != y3 + step);
    }
    else if (y >= ly && y <= gy) pos[y][dir2] = RANGE(x2, minx, maxx);

    y = y3;
    step = (dir3 << 1) - 1;

    if (y3 != y4)
    {
        do {
            if (RANGE(y, ly, gy) == y)
            {
                tmp = xdiv3 * (y - y3) / ydiv3 + x3;
                pos[y][dir3] = RANGE(tmp, minx, maxx);
            }
            y += step;
        } while (y != y4 + step);
    }
    else if (y >= ly && y <= gy) pos[y][dir3] = RANGE(x3, minx, maxx);

    y = y4;
    step = (dir4 << 1) - 1;

    if (y4 != y1)
    {
        do
        {
            if (RANGE(y, ly, gy) == y)
            {
                tmp = xdiv4 * (y - y4) / ydiv4 + x4;
                pos[y][dir4] = RANGE(tmp, minx, maxx);
            }
            y += step;
        } while (y != y1 + step);
    }
    else if (y >= ly && y <= gy) pos[y][dir4] = RANGE(x4, minx, maxx);

    for (y = ly; y <= gy; y++) horzLine(pos[y][0], pos[y][1], y, c, where);
}

void polygon2(int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t x3, int16_t y3, int16_t x4, int16_t y4, int16_t c, uint8_t *where)
{
    int16_t pos[MPOS][2] = {0};
    int16_t xdiv1, xdiv2, xdiv3, xdiv4;
    int16_t ydiv1, ydiv2, ydiv3, ydiv4;
    int16_t ly, gy, y, tmp, step;
    uint8_t dir1, dir2, dir3, dir4;

    ly = MAX(MIN(MIN(MIN(y1, y2), y3), y4), miny);
    gy = MIN(MAX(MAX(MAX(y1, y2), y3), y4), maxy);

    if (ly > maxy) return;
    if (gy < miny) return;

    dir1 = y1 < y2; xdiv1 = x2 - x1; ydiv1 = y2 - y1;
    dir2 = y2 < y3; xdiv2 = x3 - x2; ydiv2 = y3 - y2;
    dir3 = y3 < y4; xdiv3 = x4 - x3; ydiv3 = y4 - y3;
    dir4 = y4 < y1; xdiv4 = x1 - x4; ydiv4 = y1 - y4;

    y = y1;
    step = (dir1 << 1) - 1;

    if (y1 != y2)
    {
        do
        {
            if (RANGE(y, ly, gy) == y)
            {
                tmp = xdiv1 * (y - y1) / ydiv1 + x1;
                pos[y][dir1] = RANGE(tmp, minx, maxx);
            }
            y += step;
        } while (y != y2 + step);
    }
    else if (y >= ly && y <= gy) pos[y][dir1] = RANGE(x1, minx, maxx);

    y = y2;
    step = (dir2 << 1) - 1;

    if (y2 != y3)
    {
        do
        {
            if (RANGE(y, ly, gy) == y)
            {
                tmp = xdiv2 * (y - y2) / ydiv2 + x2;
                pos[y][dir2] = RANGE(tmp, minx, maxx);
            }
            y += step;
        } while (y != y3 + step);
    }
    else if (y >= ly && y <= gy) pos[y][dir2] = RANGE(x2, minx, maxx);

    y = y3;
    step = (dir3 << 1) - 1;

    if (y3 != y4)
    {
        do
        {
            if (RANGE(y, ly, gy) == y)
            {
                tmp = xdiv3 * (y - y3) / ydiv3 + x3;
                pos[y][dir3] = RANGE(tmp, minx, maxx);
            }
            y += step;
        } while (y != y4 + step);
    }
    else if (y >= ly && y <= gy) pos[y][dir3] = RANGE(x3, minx, maxx);

    y = y4;
    step = (dir4 << 1) - 1;

    if (y4 != y1)
    {
        do
        {
            if (RANGE(y, ly, gy) == y)
            {
                tmp = xdiv4 * (y - y4) / ydiv4 + x4;
                pos[y][dir4] = RANGE(tmp, minx, maxx);
            }
            y += step;
        } while (y != y1 + step);
    }
    else if (y >= ly && y <= gy) pos[y][dir4] = RANGE(x4, minx, maxx);

    for (y = ly; y <= gy; y++)
    {
        horzLine(pos[y][0], pos[y][1], y, c >> 1, where);
        c++;
    }
}

void calcSintab(uint8_t art, int16_t xo, int16_t yo, int16_t r, int16_t a)
{
    uint16_t x, y;

    x = 160 + xo + (r * sintab[90 + a]) / (DIVD - 20);
    y = 100 + yo + (r * sintab[a]) / DIVD;

    switch (art)
    {
    case 0:
        if (x < 320 && y < 200)
        {
            px[ring][point] = x;
            py[ring][point] = (y << 8) + (y << 6);
        }
        else
        {
            px[ring][point] = 0;
            py[ring][point] = 0;
        }
        break;

    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
        px[ring][point] = x;
        py[ring][point] = y;
    }
}

void drawHole(uint8_t art, uint8_t *where)
{
    int16_t ri, po;

    switch (art)
    {
    case 0:
        for (ri = 0; ri <= 26; ri++)
        {
            for (po = 0; po <= 23; po++) putPixel(where, py[ri][po] + px[ri][po], ri + 1);
        }
    
        putPixel(where, 0, 0);
        break;

    case 1:
        for (ri = 0; ri <= 26; ri++)
        {
            for (po = 1; po <= 23; po++) drawLine(px[ri][po - 1], py[ri][po - 1], px[ri][po], py[ri][po], ri + 1, where);
            drawLine(px[ri][23], py[ri][23], px[ri][0], py[ri][0], ri + 1, where);
        }    
        break;

    case 2:
        for (po = 0; po <= 23; po++)
        {
            for (ri = 1; ri <= 26; ri++) drawLine(px[ri - 1][po], py[ri - 1][po], px[ri][po], py[ri][po], ri + 1, where);
            drawLine(px[26][po], py[26][po], px[0][po], py[0][po], ri + 1, where);
        }
        break;

    case 3:
        for (ri = 0; ri <= 26; ri++)
        {
            for (po = 1; po <= 23; po++) drawLine(px[ri][po - 1], py[ri][po - 1], px[ri][po], py[ri][po], ri+1, where);
            drawLine(px[ri][23], py[ri][23], px[ri][0], py[ri][0], ri + 1, where);
        }

        for(po = 0; po <= 23; po++)
        {
            for (ri = 1; ri <= 26; ri++) drawLine(px[ri - 1][po], py[ri - 1][po], px[ri][po], py[ri][po], ri + 1, where);
            drawLine(px[26][po], py[26][po], px[0][po], py[0][po], ri + 1, where);
        }
        break;

    case 4:
        for (ri = 0; ri <= 25; ri++)
        {
            for (po = 1; po <= 23; po++) polygon1(px[ri][po - 1], py[ri][po - 1], px[ri][po], py[ri][po], px[ri + 1][po], py[ri + 1][po], px[ri + 1][po - 1], py[ri + 1][po - 1], ri + 1, where);
            polygon1(px[ri][23], py[ri][23], px[ri][0], py[ri][0], px[ri + 1][0], py[ri + 1][0], px[ri + 1][23], py[ri + 1][23], ri + 1, where);
        }

        for (ri = 0; ri <= 199; ri++) putPixel(where, (ri << 6) + (ri << 8), 0);
        for (ri = 0; ri <= 199; ri++) putPixel(where, (ri << 6) + (ri << 8) + 319, 0);
        break;

    case 5:
        for (ri = 0; ri <= 25; ri++)
        { 
            for (po = 1; po <= 23; po++) polygon2(px[ri][po - 1], py[ri][po - 1], px[ri][po], py[ri][po], px[ri + 1][po], py[ri + 1][po], px[ri + 1][po - 1], py[ri + 1][po - 1], ri + 1, where);
            polygon2(px[ri][23], py[ri][23], px[ri][0], py[ri][0], px[ri + 1][0], py[ri + 1][0], px[ri + 1][23], py[ri + 1][23], ri + 1, where);
        }

        for (ri = 0; ri <= 199; ri++) putPixel(where, (ri << 6) + (ri << 8), 0);
        for (ri = 0; ri <= 199; ri++) putPixel(where, (ri << 6) + (ri << 8) + 319, 0);
        break;
    }
}

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

inline int16_t roundf(float x)
{
    if (x > 0.0) return x + 0.5;
    return x - 0.5;
}

void main()
{
    clearTextMem();
    putMsg(1, 1, 0x0F, "The Hole - (c) 1998 by Nguyen Ngoc Van");
    putMsg(1, 2, 0x07, "Use : 1 -> 6 to switch effects");
    putMsg(1, 3, 0x07, "Use : ESC to exit demo");
    putMsg(1, 4, 0x07, "Press any key to start demo...");
    getch();

    for (i = 0; i < 256; i++)
    {
        cosx[i] = roundf(cos(M_PI * i / 128) * 60);
        sinx[i] = roundf(sin(M_PI * i / 128) * 45);
    }

    __asm {
        mov     ax, 0x13
        int     0x10
    }

    minx = 0;
    miny = 0;
    maxx = 319;
    maxy = 199;
    x = 30;
    y = 90;
    art = 0;

    for (i = 0; i < 450; i++) sintab[i] = sin(2 * M_PI * i / 360) * DIVD;

    for (i = 1; i < 28; i++)
    {
        setRGB(i, i, i * 2, i + 30);
        setRGB(2 * 27 - i, i, i << 1, i + 30);
        setRGB(2 * 27 + i, i, i << 1, i + 30);
        setRGB(3 * 27 - i, i, i << 1, i + 30);
        setRGB(3 * 27 + i, i, i << 1, i + 30);
    }

    do {
        step = 2;
        j = 10;
        ring = 0;

        while (j < 220)
        {
            i = 0;
            point = 0;

            while (i < 360)
            {
                calcSintab(art, cosx[(x + (200 - j)) & 0xFF], sinx[(y + (200 - j)) & 0xFF], j, i);
                i += STEP;
                point++;
            }

            j += step;
            if (!(j % 3)) step++;

            ring++;
        }

        x = XST + (x & 0xFF);
        y = YST + (y & 0xFF);

        clearMem(vbuff);
        drawHole(art, vbuff);
        retrace();
        flip(vbuff, 0xA000);
        key = inp(0x60);

        switch (key)
        {
            case 2: art = 0; break; case 3: art = 1; break;
            case 4: art = 2; break; case 5: art = 3; break;
            case 6: art = 4; break; case 7: art = 5; break;
        }
    } while (key != 1);

    __asm {
        mov     ax, 0x03
        int     0x10
    }
}
