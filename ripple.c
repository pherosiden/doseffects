/*----------------------------------------------------*/
/* Packet  : Demo & Effect                            */
/* Effect  : Ripple                                   */
/* Author  : Nguyen Ngoc Van                          */
/* Memory  : Compact                                  */
/* Heaps   : 640K                                     */
/* Address : pherosiden@gmail.com                     */
/* Website : http://www.codedemo.net                  */
/* Created : 24/02/1998                               */
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

#define AMPL 8
#define FREQ 15
#define M_PI 3.141592f

uint8_t img[64000] = {0};
uint8_t vbuff[64000] = {0};
uint8_t sqrtab[32321] = {0};

uint16_t angle = 0;
uint8_t pal[768] = {0};
int16_t wave[200] = {0};

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

void loadImage()
{
    FILE *fp;

    if (!(fp = fopen("assets/pig.cel", "rb"))) return;
    fseek(fp, 32, SEEK_SET);
    fread(pal, 1, 768, fp);
    fread(img, 1, 64000, fp);
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
    uint16_t len = strlen(msg);

    __asm {
        mov     cx, len
        test    cx, cx
        jz      quit
        mov     ax, 0xB800
        mov     es, ax
        lds     si, msg
        xor     di, di
        add     di, x
        shl     di, 1
        mov     ax, y
        shl     ax, 5
        add     di, ax
        shl     ax, 2
        add     di, ax
        mov     ah, col
    next:
        lodsb
        stosw
        loop    next
    quit:
    }
}


void preCalc()
{
    int16_t x, y;
    for (y = 0; y < 100; y++)
    {
        for (x = 0; x < 160; x++)        
        {
            sqrtab[(y * 160 + x) << 1] = sqrt(x * x + y * y);       
        }
    }
}

void updateWave()
{
    int16_t k;
    
    angle++;

    for (k = 0; k < 200; k++)
    {
        wave[k] = AMPL * sin(FREQ * (k - angle) * M_PI / 180);
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

void drawRipples()
{
    int16_t x, y;
    int16_t dist, alt, xx, yy;

    for (y = 0; y < 200; y++)
    {
        for (x = 0; x < 320; x++)
        {
            xx = abs(x - 160);
            yy = abs(y - 100);

            dist = sqrtab[(yy * 160 + xx) << 1];
            alt = wave[dist];

            xx = x + alt;
            yy = y + alt;

            if (yy > 199) yy -= 200;
            if (yy < 0) yy += 200;

            putPixel(x, y, getPixel(xx, yy, img), vbuff);
        }
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

void main()
{
    clearScreen();
    printStr(1, 1, 0x0F, "RIPPLE - (c) 1998 by Nguyen Ngoc Van");
    printStr(1, 2, 0x07, "Press any key to start demo...");
    getch();

    __asm {
        mov     ax, 0x13
        int     0x10
    }

    loadImage();
    preCalc();

    while (!kbhit())
    {
        updateWave();
        drawRipples();
        retrace();
        flip(vbuff, 0xA000);
    }

    __asm {
        mov     ax, 0x03
        int     0x10
    }
}
