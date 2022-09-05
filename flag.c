/*----------------------------------------------------*/
/* Packet  : Demo & Effect                            */
/* Effect  : Bitmap Flag                              */
/* Author  : Nguyen Ngoc Van                          */
/* Memory  : Compact                                  */
/* Heaps   : 640K                                     */
/* Address : pherosiden@gmail.com                     */
/* Website : http://www.codedemo.net                  */
/* Created : 12/01/1998                               */
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

#define AMPLI 6
#define SPEED 4
#define M_PI 3.141592f

uint8_t pal[768] = {0};
uint16_t index = 0;
uint16_t costab[256] = {0};
uint8_t vbuff1[100][160] = {0};
uint8_t vbuff2[200][320] = {0};

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

void printStr(int16_t x, int16_t y, uint8_t col, char *msg)
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

void flip(uint16_t dst)
{
    __asm {
        mov     es, dst
        mov     ax, seg vbuff2
        mov     ds, ax
        xor     si, si
        xor     di, di
        mov     cx, 16000
        rep     movsd
    }
}

void putPixel(int16_t x, int16_t y, uint8_t col)
{
    __asm {
        mov     ax, seg vbuff2
        mov     es, ax
        mov     di, x
        mov     bx, y
        shl     bx, 6
        add     bh, byte ptr y
        add     di, bx
        mov     al, col
        stosb
    }
}

void clearSeg()
{
    __asm {
        mov     ax, seg vbuff2
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
        mov     ax, 0xB800
        mov     es, ax
        xor     di, di
        xor     ax, ax
        mov     cx, 2000
        rep     stosw
    }
}

inline int16_t roundf(float x)
{
    if (x > 0.0) return x + 0.5;
    return x - 0.5;
}

void createCosTable()
{
    int16_t i;
    for (i = 0; i < 256; i++) costab[i] = roundf(cos(i * M_PI / 64) * AMPLI);
}

void loadCEL()
{
    FILE *fp = fopen("assets/skull.cel", "rb");
    if (!fp) return;

    fseek(fp, 32, SEEK_SET);
    fread(pal, 1, 768, fp);
    fread(vbuff1[0], 1, 16000, fp);
    fclose(fp);

    __asm {
        mov     dx, 0x03C8
        xor     ax, ax
        out     dx, al
        inc     dx
        lea     si, pal
        mov     cx, 768
        rep     outsb
    }
}

void putImage()
{
    int16_t x, y;

    for (y = 0; y < 100; y++)
    {
        for (x = 0; x < 160; x++)
        {
            if (vbuff1[y][x] != 1) vbuff2[y][x] = vbuff1[y][x];
        }
    }
}

void displayMap()
{
    int16_t x, y;
    int16_t xpos, ypos;

    for (y = 0; y < 100; y++)
    {
        for (x = 0; x < 160; x++)
        {
            xpos = (x << 1) + costab[(index + 125 * (x + y)) & 0xFF];
            ypos = (y << 1) + costab[(index + 3 * x + 125 * y) & 0xFF];
            if (xpos >= 0 && xpos <= 319 && ypos >= 0 && ypos <= 199) vbuff2[ypos][xpos] = vbuff1[y][x];
        }
    }
}

void main()
{
    clearScreen();
    printStr(1, 1, 0x0F, "IMAGE FLAG - (c) 1998 by Nguyen Ngoc Van");
    printStr(1, 2, 0x07, "Press any key to start demo...");
    getch();

    __asm {
        mov     ax, 0x13
        int     0x10
    }

    createCosTable();
    loadCEL();

    while (!kbhit())
    {
        displayMap();
        putImage();
        retrace();
        flip(0xA000);
        clearSeg();
        index += SPEED;
    }

    __asm {
        mov     ax, 0x03
        int     0x10
    }
}
