/*----------------------------------------------------*/
/* Packet  : Demo & Effect                            */
/* Effect  : Shade Bob                                */
/* Author  : Nguyen Ngoc Van                          */ 
/* Memory  : Compact                                  */ 
/* Heaps   : 640K                                     */
/* Address : pherosiden@gmail.com                     */ 
/* Website : http://www.codedemo.net                  */
/* Created : 04/03/1998                               */
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

float alpha, rx, ry;

int16_t clear, reg;
int16_t orgx, orgy;

int16_t ux[1000] = {0};
int16_t uy[1000] = {0};

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

void setRGB(uint8_t n, uint8_t r, uint8_t g, uint8_t b)
{
    __asm {
        mov     dx, 0x03C8
        mov     al, n
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

inline uint8_t test(int16_t x, int16_t y)
{
    if (x < 0 || y < 0 || x > 319 || y > 199) return 0;
    return 1;
}

inline int16_t roundf(float x)
{
    if (x >= 0.0) return x + 0.5;
    return x - 0.5;
}

void outDraw()
{
    int16_t x, y, a, b;
    uint8_t p;
    float sinx, cosx;

    sinx = sin(alpha); 
    cosx = cos(alpha);
    
    x = roundf(orgx + rx * cosx * cosx * sinx * sinx * cosx);
    y = roundf(orgy + ry * sinx * sinx * sinx * cosx * cosx);
    
    ux[reg] = x; 
    uy[reg] = y;
    
    for (a = x - 5; a <= x + 5; a++)
    {
        for (b = y - 5; b <= y + 5; b++)
        {
            if (test(a, b))
            {
                p = getPixel(a, b);
                if (p < 250) putPixel(a, b, p + 4);
            }
        }
    }
}

void inDraw()
{
    int16_t a, b, x, y;
    uint8_t p;

    if (reg > 1000)
    {
        x = ux[0];
        y = uy[0];
    }
    else
    {
        x = ux[reg + 1];
        y = uy[reg + 1];
    }

    if (!x && !y) return;

    for (a = x - 2; a <= x + 2; a++)
    {
        for (b = y - 2; b <= y + 2; b++)
        {
            if (test(a, b))
            {
                p = getPixel(a, b);
                if (p > 10 && p < 254) putPixel(a, b, p - 10);
            }
        }
    }
}

void main()
{
    int16_t x, y;
    FILE *fp;

    clearScreen();
    printStr(1, 1, 0x0F, "SHADE SIN - (c) 1998 by Nguyen Ngoc Van");
    printStr(1, 2, 0x07, "Press any key to start demo...");
    getch();

    __asm {
        mov     ax, 0x13
        int     0x10
    }

    for (x = 0; x < 64; x++)
    {
        setRGB(x, x, 0, 0);
        setRGB(x + 64, 63, x, 0);
        setRGB(x + 128, 63, 63, 0);
        setRGB(x + 192, 63, 63, x);
    }

    fp = fopen("assets/killer.cel", "rb");
    if (!fp) return;
    fseek(fp, 800, SEEK_SET);
    fread(vmem, 64000, 1, fp);
    fclose(fp);

    alpha = 0;
    rx = 150;
    ry = 90;
    orgx = 160;
    orgy = 100;
    clear = 17400;
    reg = 0;

    while (!kbhit() && clear)
    {
        outDraw();
        inDraw();
        
        reg++;
        if (reg > 999) reg = 0;

        alpha += 0.001;
        rx += 0.05;

        ry = ry + rx * 0.0001;
        clear--;
        
    }

    for (y = 0; y < 100; y++)
    {
        retrace();
        for (x = 0; x < 320; x++) putPixel(x, y, 0);
        for (x = 0; x < 320; x++) putPixel(x, 199 - y, 0);
    }

    __asm {
        mov     ax, 0x03
        int     0x10
    }
}
