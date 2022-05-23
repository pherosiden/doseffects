/*----------------------------------------------------*/
/* Packet  : Demo & Effect                            */
/* Effect  : Pixel Morphing                           */
/* Author  : Nguyen Ngoc Van                          */
/* Memory  : Compact                                  */ 
/* Heaps   : 640K                                     */
/* Address : pherosiden@gmail.com                     */
/* Website : http://www.codedemo.net                  */
/* Created : 13/02/1998                               */  
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

#define JUMP 64
#define SJUMP 6

typedef struct {
    int16_t herex, herey;
    int16_t targetx, targety;
    int16_t dx, dy;
    uint8_t active;
    uint8_t color;
    uint16_t numof;
} Target;

Target nextrow[4096] = {0};

uint8_t pal[768] = {0};
uint8_t vbuff[64000] = {0};

uint8_t *vmem = (uint8_t*)0xA0000000L;
uint8_t *tmem = (uint8_t*)0xB8000000L;

inline int16_t random(int16_t x)
{
    if (x == 0) return 0;
    return rand() % x;
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

void putPixel(int16_t x, int16_t y, uint8_t color, uint8_t *addr)
{
    __asm {
        les     di, addr
        mov     bx, y
        shl     bx, 6
        add     bh, byte ptr y
        add     bx, x
        add     di, bx
        mov     al, color
        stosb
    }
}

uint8_t getPixel(int16_t x, int16_t y, uint8_t *addr)
{
    uint8_t col;

    __asm {
        les     di, addr
        mov     bx, y
        shl     bx, 6
        add     bh, byte ptr y
        add     bx, x
        add     di, bx
        mov     al, es:[di]
        mov     col, al
    }

    return col;
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

void flip(uint8_t *src, uint8_t *dst)
{
    __asm {
        les     di, dst
        lds     si, src
        mov     cx, 16000
        rep     movsd
    }
}

void printStr(int16_t x, int16_t y, uint8_t color, char *msg)
{
    int16_t len = strlen(msg);

    __asm {
        les     di, tmem
        lds     si, msg
        add     di, x
        shl     di, 1
        mov     bx, y
        shl     bx, 5
        add     di, bx
        shl     bx, 2
        add     di, bx
        mov     ah, color
        mov     cx, len
    next:
        lodsb
        stosw
        loop    next
    }
}

void moveEmOut(int16_t num, uint8_t del)
{
    int16_t i;

    if (del) for (i = 0; i < del; i++) retrace();

    for (i = 0; i < num; i++)
    {
        if (nextrow[i].active)
        {
            putPixel(nextrow[i].herex >> SJUMP, nextrow[i].herey >> SJUMP,
            getPixel(nextrow[i].herex >> SJUMP, nextrow[i].herey >> SJUMP, vbuff), vmem);

            nextrow[i].herex -= nextrow[i].dy;
            nextrow[i].herey -= nextrow[i].dx;

            putPixel(nextrow[i].herex >> SJUMP, nextrow[i].herey >> SJUMP, nextrow[i].color, vmem);
            nextrow[i].numof--;

            if (!nextrow[i].numof)
            {
                nextrow[i].active = 0;
                putPixel(nextrow[i].herex >> SJUMP, nextrow[i].herey >> SJUMP, nextrow[i].color, vbuff);
            }
        }
    }
}

void doPic()
{
    FILE *fp;
    int16_t i, j;
    uint8_t data;
    uint16_t k = 0;
    
    memset(nextrow, 0, sizeof(nextrow));

    fp = fopen("boss.cel", "rb");
    if (!fp) return;
    fseek(fp, 800, SEEK_SET);

    for (j = 0; j < 80; j++)
    {
        for (i = 0; i < 80; i++)
        {
            fread(&data, 1, 1, fp);
            if (data)
            {
                nextrow[k].herex = random(320) << SJUMP;
                nextrow[k].herey = random(200) << SJUMP;

                nextrow[k].targetx = (i + 120) << SJUMP;
                nextrow[k].targety = (j + 60) << SJUMP;

                nextrow[k].dy = (nextrow[k].herex - nextrow[k].targetx) / JUMP;
                nextrow[k].dx = (nextrow[k].herey - nextrow[k].targety) / JUMP;

                nextrow[k].color = data;
                nextrow[k].active = 1;
                nextrow[k].numof = JUMP;
                k++;
            }
        }
    }

    fclose(fp);
    for (i = 0; i < JUMP; i++) moveEmOut(k, 2);
    getch();
}

void main()
{
    FILE *fp;

    clearScreen();
    printStr(1, 1, 0x0F, "PIXEL MORPHING - (c) 1998 by Nguyen Ngoc Van");
    printStr(1, 2, 0x07, "Press any key to start demo...");
    getch();

    srand(time(NULL));

    __asm {
        mov     ax, 0x13
        int     0x10
    }

    fp = fopen("duke.cel", "rb");
    if (!fp) return;
    fseek(fp, 32, SEEK_SET);
    fread(pal, 1, 768, fp);
    fread(vbuff, 1, 64000, fp);
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

    flip(vbuff, vmem);
    doPic();

    __asm {
        mov     ax, 0x03
        int     0x10
    }
}
