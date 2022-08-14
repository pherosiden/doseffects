/*----------------------------------------------------*/
/* Packet  : Demo & Effect                            */ 
/* Effect  : Land Mapping                             */ 
/* Author  : Nguyen Ngoc Van                          */
/* Memory  : Compact                                  */
/* Heaps   : 640K                                     */ 
/* Address : pherosiden@gmail.com                     */
/* Website : http://www.codedemo.net                  */
/* Created : 12/02/1998                               */  
/* Please sent to me any bugs or suggests.            */ 
/* You can use freely this code. Have fun :)          */
/*----------------------------------------------------*/

#include <dos.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <conio.h>

#define DENT	3
#define ROUGH	26
#define XMAXS	320
#define YMAXS	200
#define HMAX	128
#define XMAX 	(320 / DENT)
#define YMAX 	(120 / DENT)

uint8_t vbuff[64000] = {0};
uint8_t *vmem = (uint8_t*)0xA0000000L;
uint8_t *tmem = (uint8_t*)0xB8000000L;

void retrace()
{
    __asm {
        mov     dx, 0x03DA
    waitH:
        in      al, dx
        test    al, 0x08
        jnz     waitH
    waitV:
        in      al, dx
        test    al, 0x08
        jz      waitV
    }
}

int16_t mouseDetect()
{
    __asm {
        xor     ax, ax
        int     0x33
        xor     al, al
    }
}

int16_t getMouseX()
{
    __asm {
        mov     ax, 0x03
        int     0x33
        mov     ax, cx
    }
}

int16_t getMouseY()
{
    __asm {
        mov     ax, 0x03
        int     0x33
        mov     ax, dx
    }
}

int16_t leftPressed()
{
    __asm {
        mov     ax, 0x03
        int     0x33
        mov     ax, bx
    }
}

void mouseWindow(int16_t l, int16_t t, int16_t r, int16_t b)
{
    __asm {
        mov     ax, 0x07
        mov     cx, l
        mov     dx, r
        int     0x33
        inc     ax
        mov     cx, t
        mov     dx, b
        int     0x33
    }
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

void flip(uint8_t *src, uint8_t *dst)
{
    __asm {
        les     di, dst
        lds     si, src
        mov     cx, 16000
        rep     movsd
    }
}

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

inline int16_t random(int16_t x)
{
    if (x == 0) return 0;
    return rand() % x;
}

void setPalette()
{
    int16_t i;

    for (i = 0; i <= 15; i++) setRGB(i, 0, 0, i << 2);
    for (i = 0; i <= 31; i++) setRGB(i + 16, 63 - (i >> 1), 63 - i, 0);
    for (i = 0; i <= 31; i++) setRGB(i + 48, 0, 63 - i, 0);
    for (i = 0; i <= 48; i++) setRGB(i + 80, i + 16, i + 16, i + 16);

    setRGB(255, 63, 0, 0);
}

void adjust(int16_t xa, int16_t ya, int16_t x, int16_t y, int16_t xb, int16_t yb)
{
    int16_t d;
    uint8_t c;

    if (getPixel(x, y)) return;

    d = abs(xa - xb) + abs(ya - yb);
    c = (50 * (getPixel(xa, ya) + getPixel(xb, yb)) + (10 * (1 + random(10)) / 11 - 5) * d * ROUGH) / 100;
    if (c > HMAX) c = HMAX;
    putPixel(x, y, c);
}

void subDivide(int16_t l, int16_t t, int16_t r, int16_t b)
{
    int16_t x, y;
    uint8_t c;

    if (r - l < 2  && b - t < 2) return;

    x = (l + r) >> 1;
    y = (t + b) >> 1;

    adjust(l, t, x, t, r, t);
    adjust(r, t, r, y, r, b);
    adjust(l, b, x, b, r, b);
    adjust(l, t, l, y, l, b);

    if (!getPixel(x, y))
    {
        c = (getPixel(l, t) + getPixel(r, t) + getPixel(r, b) + getPixel(l, b)) >> 2;
        if (c > HMAX) c = HMAX;
        putPixel(x, y, c);
    }

    subDivide(l, t, x, y);
    subDivide(x, t, r, y);
    subDivide(l, y, x, b);
    subDivide(x, y, r, b);
}

void generateLandscape()
{
    FILE *fp;
    uint16_t i;

    fp = fopen("assets/land.map", "rb");
    if (!fp)
    {
        srand(time(NULL));
        putPixel(0, 0, random(HMAX));
        putPixel(XMAXS - 1, 0, random(HMAX));
        putPixel(0, YMAXS - 1, random(HMAX));
        putPixel(XMAXS - 1, YMAXS - 1, random(HMAX));
        subDivide(0, 0, XMAXS, YMAXS);
        fp = fopen("assets/land.map", "wb");
        fwrite(vmem, XMAXS * YMAXS, 1, fp);
        fclose(fp);
    }
    else
    {
        fread(vmem, XMAXS * YMAXS, 1, fp);
        fclose(fp);
    }

    flip(vmem, vbuff);

    for (i = 0; i < XMAXS * YMAXS; i++)
    {
        vbuff[i] = 110 + (vbuff[i] >> 1);
        if (vbuff[i] < 115) vbuff[i] = 115;
    }

    clearMem(vmem);
}

void displayScape()
{
    int16_t i = 0, j = 0;
    int16_t pi, pj, n, x;

    do {
        pi = i;
        pj = j;
        i = getMouseX();
        j = getMouseY();
        retrace();
        for (n = 0; n < XMAX * YMAX; n++)
        {
            x = -(DENT * (n % XMAX - (XMAX >> 1) - 1) * 45) / (n / XMAX - 45) - 90;
            if (x >= -250 && x < 60)
            {
                vmem[(320 * (DENT * (n / XMAX) - vbuff[(n % XMAX + pi + (n / XMAX + pj) * XMAXS) % 64000] - 3) + x) % 64000] = 0;
                vmem[(320 * (DENT * (n / XMAX) - vbuff[(n % XMAX +  i + (n / XMAX +  j) * XMAXS) % 64000] - 3) + x) % 64000] = vbuff[(n % XMAX + i + (n / XMAX + j) * XMAXS) % 64000] - 100;
            }
        }
    } while (!kbhit() && leftPressed() != 1);
}

void main()
{
    clearScreen();
    printStr(1, 1, 0x0F, "LAND SCAPE - (c) by Nguyen Ngoc Van");
    printStr(1, 2, 0x07, "Any key : Start demo");
    printStr(1, 3, 0x07, "Use mouse to flash!");
    getch();

    if (!mouseDetect()) return;

    __asm {
        mov     ax, 0x13
        int     0x10
    }

    setPalette();
    generateLandscape();
    mouseWindow(0, 0, XMAXS - XMAX, YMAXS - YMAX);
    displayScape();

    __asm {
        xor     ax, ax
        int     0x33
        mov     ax, 0x03
        int     0x10
    }
}
