/*------------------------------------------------------------*/
/* Packet  : Demo & Effect                                    */
/* Effect  : Image Processing (Zoom In/Out/Scale/stretch)     */ 
/* Author  : Nguyen Ngoc Van                                  */
/* Memory  : Compact                                          */ 
/* Heaps   : 640K                                             */
/* Address : pherosiden@gmail.com                             */ 
/* Website : http://www.codedemo.net                          */
/* Created : 12/01/1998                                       */
/* Please sent to me any bugs or suggests.                    */
/* You can use freely this code. Have fun :)                  */
/*------------------------------------------------------------*/

#include <dos.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <conio.h>

#define Sign(x) (((x) >= 0) ? 1 : -1)
#define M_PI 3.141592f

uint8_t pal[768] = {0};

uint8_t vbuff1[64000] = {0};
uint8_t vbuff2[64000] = {0};

uint8_t *vmem = (uint8_t*)0xA0000000L;
uint8_t *tmem = (uint8_t*)0xB8000000L;

int16_t costab[256] = {0};
int16_t sintab[256] = {0};

void putPixel(uint16_t x, uint16_t y, uint8_t col, uint8_t *where)
{
    __asm {
        les     di, where
        mov     bx, y
        shl     bx, 6
        add     bh, byte ptr y
        add     bx, x
        add     di, bx
        mov     al, col
        stosb
    }
}

uint8_t getPixel(uint16_t x, uint16_t y, uint8_t *where)
{
    __asm {
        lds     si, where
        mov     bx, y
        shl     bx, 6
        add     bh, byte ptr y
        add     bx, x
        add     si, bx
        lodsb
    }
}

void putMsg(int16_t x, int16_t y, uint8_t col, char *msg)
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

void stretch(int32_t x1, int32_t x2, int32_t y1, int32_t y2, int32_t yr, int32_t yw)
{
    int32_t dx, dy;
    int32_t e, d, dx2;
    int16_t sx, sy;

    dx = labs(x2 - x1);
    dy = labs(y2 - y1);

    sx = Sign(x2 - x1);
    sy = Sign(y2 - y1);

    e = (dy << 1) - dx;
    dx2 = dx << 1;
    dy = dy << 1;

    for (d = 0; d < dx; d++)
    {
        putPixel(x1, yw, getPixel(y1, yr, vbuff2), vbuff1);

        while (e >= 0)
        {
            y1 += sy;
            e -= dx2;
        }

        x1 += sx;
        e += dy;
    }
}

void rectStretch(int32_t xs1, int32_t ys1, int32_t xs2, int32_t ys2, int32_t xd1, int32_t yd1, int32_t xd2, int32_t yd2)
{
    int32_t dx, dy;
    int32_t e, d, dx2;
    int16_t sx, sy;

    dx = labs(yd2 - yd1);
    dy = labs(ys2 - ys1);

    sx = Sign(yd2 - yd1);
    sy = Sign(ys2 - ys1);

    e = (dy << 1) - dx;
    dx2 = dx << 1;
    dy = dy << 1;

    for (d = 0; d < dx; d++)
    {
        stretch(xd1, xd2, xs1, xs2, ys1, yd1);

        while (e >= 0)
        {
            ys1 += sy;
            e -= dx2;
        }

        yd1 += sx;
        e += dy;
    }
}

void flip(uint8_t *src, uint8_t *dst)
{
    __asm {
        les     di, dst
        lds     si, src
        mov     cx, 16000
        rep     movsd
    }
}

void loadCEL(char *fname)
{
    FILE *fp;

    fp = fopen(fname, "rb");
    if (!fp) return;

    fseek(fp, 32, SEEK_SET);
    fread(pal, 768, 1, fp);
    fread(vbuff2, 64000, 1, fp);
    fclose(fp);

    __asm {
        mov     dx, 0x03C8
        xor     al, al
        out     dx, al
        inc     dx
        lea     si, pal
        mov     cx, 768
        rep     outsb
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

void clearTextMem()
{
    __asm {
        les     di, tmem
        xor     ax, ax
        mov     cx, 2000
        rep     stosw
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

void run()
{
    int16_t x, y;
    int16_t z = 114;
    int16_t zadd = -2;

    do {
        z += zadd;
        x = (16 << 8) / z;
        y = (10 << 8) / z;

        flip(vbuff2, vbuff1);
        rectStretch(0, 0, 319, 199, 160 - (x >> 1), 100 - (y >> 1), 160 - (x >> 1) + x, 100 - (y >> 1) + y);
        retrace();
        flip(vbuff1, vmem);
        if (z < 16 || z > 114) zadd = -zadd;
    } while(inp(0x60) != 1 && inp(0x60) != 3 && inp(0x60) != 4);
}

void run1()
{
    int16_t i = 128;
    int16_t dir = 3;
    int16_t vx1 = 10;

    int16_t vy1, vx2, vy2;
    int16_t x1, y1, x2, y2;

    do {
        flip(vbuff2, vbuff1);
        if (vx1 < -50  || vx1 > 260) dir = -dir;

        vy1 = 25 + sintab[i];
        vy2 = vy1 + 100;
        i = (i + 4) % 255;
        if (vy2 < 200) vx1 += dir;

        vx2 = vx1 + 100;
        x1 = vx1;
        if (x1 < 0) x1 = 0;

        y1 = vy1;
        if (y1 < 0) y1 = 0;

        x2 = vx2;
        if (x2 > 319) x2 = 319;

        y2 = vy2;
        if (y2 > 199) y2 = 199;

        rectStretch(0, 0, 319, 199, x1, y1, x2, y2);
        retrace();
        flip(vbuff1, vmem);
    } while(inp(0x60) != 1 && inp(0x60) != 2 && inp(0x60) != 4);
}

void run2()
{
    int16_t i = 0;

    do {
        flip(vbuff2, vbuff1);
        rectStretch(0, 0, 319, 199, 40, 100 - costab[i], 140, 100 + costab[i]);
        rectStretch(0, 0, 319, 199, 230 - costab[i], 50, 230 + costab[i], 150);
        retrace();
        flip(vbuff1, vmem);     
        i = (i + 6) % 256;
    } while(inp(0x60) != 1 && inp(0x60) != 2 && inp(0x60) != 3);
}

inline int16_t roundf(float x)
{
    if (x >= 0.0) return x + 0.5;
    return x - 0.5;
}

void main()
{
    int16_t i;

    clearTextMem();
    putMsg(1, 1, 0x0F, "Scale - (c) 1998 by Nguyen Ngoc Van");
    putMsg(1, 2, 0x07, "Use : 1 -> 3 to switch effect");
    putMsg(1, 3, 0x07, "ESC to exit program");
    putMsg(1, 4, 0x07, "Press any key to start demo...");
    getch();

    for (i = 0; i < 256; i++)
    {
        costab[i] = roundf(cos(2 * M_PI * i / 255) * 50);
        sintab[i] = roundf(sin(2 * M_PI * i / 255) * 60 + 80);
    }

    __asm {
        mov     ax, 0x13
        int     0x10
    }

    loadCEL("assets/face.cel");
    run1();

    do {
        if (inp(0x60) == 2) run();
        if (inp(0x60) == 3) run1();
        if (inp(0x60) == 4) run2();
    } while(inp(0x60) != 1);

    __asm {
        mov     ax, 0x03
        int     0x10
    }
}
