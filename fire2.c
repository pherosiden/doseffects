/*---------------------------------------------------*/
/* Packet  : Demo & Effect                           */
/* Effect  : Fire                                    */ 
/* Author  : Nguyen Ngoc Van                         */
/* Memory  : Compact                                 */
/* Heaps   : 640K                                    */
/* Address : pherosiden@gmail.com                    */
/* Website : http://www.codedemo.net                 */
/* Created : 13/01/1998                              */
/* Please sent to me any bugs or suggests.           */
/* You can use freely this code. Have fun :)         */
/*---------------------------------------------------*/

#include <dos.h>
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <conio.h>

uint16_t index = 0;
uint8_t seed[32768] = {0};
uint8_t pal[768] = {0};
uint8_t vbuff[64000] = {0};
uint8_t *tmem = (uint8_t*)0xB8000000L;

void initRand()
{
    uint16_t i, j;

    for (i = 0; i < 32768; i++)
    {
        j = rand() % 10;

        if (j > 4) seed[i] = 150;
        if (j > 6) seed[i] = 100;
        if (j > 8) seed[i] = 50;
        else seed[i] = 255;
    }
}

void drawSeed()
{
    int16_t i;
    for (i = 0; i < 320; i++) vbuff[i + 63680] = seed[index++];
}

void updateSeed()
{
    uint16_t x;

    for (x = 63680; x < 64000; x++)
    {
        vbuff[x] = seed[index++];
        if (index >= 32768) index = 0;
    }
}

void extendFlame()
{
    int16_t color;
    uint16_t i = 63680;
    
    while (i >= 320)
    {
        color = ((2 * vbuff[i + 319] + vbuff[i + 318] + vbuff[i - 1]) >> 2) - 2;
        if (color > 240) color = 255;
        if (color < 0) color = 0;

        vbuff[i - 1] = color;
        i--;
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

void clearScreen()
{
    __asm {
        les     di, tmem
        xor     ax, ax
        mov     cx, 2000
        rep     stosw
    }
}

void flip(uint16_t dst)
{
    __asm {
        mov     es, dst
        mov     ax, seg vbuff
        mov     ds, ax
        xor     si, si
        xor     di, di
        mov     cx, 16000
        rep     movsd
    }
}

void loadPal(char *fname)
{
    FILE *fp;
    int16_t x, r, g, b;
    
    fp = fopen(fname, "rb");
    if (!fp) return;
    fread(pal, 1, 768, fp);
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

void main()
{
    clearScreen();
    printStr(1, 1, 0x0F, "FIRE - (c) 1998 by Nguyen Ngoc Van");
    printStr(1, 2, 0x07, "Any key to start demo...");
    getch();

    __asm {
        mov     ax, 0x13
        int     0x10
    }
    
    srand(time(NULL));
    loadPal("assets/fire.pal");
    initRand();
    drawSeed();

    while (!kbhit())
    {
        updateSeed();
        extendFlame();
        retrace();
        flip(0xA000);
    }

    __asm {
        mov     ax, 0x03
        int     0x10
    }
}
