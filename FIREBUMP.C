/*----------------------------------------------------*/
/* Packet  : Demo & Effect                            */
/* Effect  : Bump Mapping                             */
/* Author  : Nguyen Ngoc Van                          */
/* Memory  : Compact                                  */ 
/* Heaps   : 640K                                     */
/* Address : pherosiden@gmail.com                     */
/* Website : http://www.codedemo.net                  */ 
/* Created : 13/01/1998                               */ 
/* Please sent to me any bugs or suggests.            */ 
/* You can use freely this code. Have fun :)          */
/*----------------------------------------------------*/

#include <dos.h>
#include <mem.h>
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <conio.h>

uint16_t num = 0;
uint16_t limit[8] = {0};
uint8_t vbuff1[200][320] = {0};
uint8_t vbuff2[200][320] = {0};

int16_t roundf(float x)
{
    if (x > 0) return x + 0.5;
    return x - 0.5;
}

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

void flip()
{
    __asm {
        mov     ax, 0xA000
        mov     es, ax
        xor     di, di
        mov     ax, seg vbuff2
        mov     ds, ax
        xor     si, si
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

void writeMsg(int16_t x, int16_t y, uint8_t col, char *msg)
{
    int16_t len = strlen(msg);

    __asm {
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
        mov     cx, len
    next:
        lodsb
        stosw
        loop    next
    }
}

void bumpMap()
{
    int16_t x, y;
    int16_t vx, vy;
    int16_t lx, ly;
    int16_t nx, ny, col;

    num++;

    lx = roundf(80 * cos(num / 20.0) + 160);
    ly = roundf(80 * sin(num / 20.0) + 100);

    for (y = 1; y < 199; y++)
    {
        vy = y - ly;
        for (x = 1; x < 319; x++)
        {
            vx = x - lx;
            nx = vbuff1[y][x + 1] - vbuff1[y][x - 1];
            ny = vbuff1[y + 1][x] - vbuff1[y - 1][x];
            col = (abs(vx - nx) + abs(vy - ny) + vbuff1[y][x]) >> 1;

            if (col > 127) vbuff2[y][x] = 0;
            else vbuff2[y][x] = limit[vbuff1[y][x] >> 8] - col;
        }
    }
}

void main()
{
    int16_t i;
    FILE *fp;

    clearTextMem();
    writeMsg(1, 1, 0x0F, "Bump Mapping - (c) 1998 by Nguyen Ngoc Van");
    writeMsg(1, 2, 0x07, "Press any key to start demo...");
    getch();

    __asm {
        mov     ax, 0x13
        int     0x10
    }

    fp = fopen("author.cel", "rb");
    if (!fp) return;
    fseek(fp, 800, SEEK_SET);
    fread(vbuff1[0], 1, 64000, fp);
    fclose(fp);

    for (i = 0; i < 8; i++) limit[i] = 127 * (i + 1);
    
    for (i = 0; i < 64; i++)
    {
        setRGB(i     ,  i, 0, 0);
        setRGB(i + 64, 63, i, 0);
    }

    do {
        bumpMap();
        flip();
        retrace();
    } while(!kbhit());

    __asm {
        mov     ax, 0x03
        int     0x10
    }
}
