/*----------------------------------------------------*/
/* Packet  : Demo & Effect                            */ 
/* Effect  : Len                                      */
/* Author  : Nguyen Ngoc Van                          */  
/* Memory  : Compact                                  */ 
/* Heaps   : 640K                                     */
/* Address : pherosiden@gmail.com                     */
/* Website : http://www.codedemo.net                  */
/* Created : 11/02/1998                               */
/* Please sent to me any bugs or suggests.            */ 
/* You can use freely this code. Have fun :)          */
/*----------------------------------------------------*/

#include <dos.h>
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <conio.h>

#define R 40
#define H 18
#define D 80

const uint8_t hand0[17][16] = {
    {0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 1, 1, 0, 1, 0, 0, 1, 1, 1, 0, 0, 0, 0},
    {0, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0},
    {0, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 0, 1, 0, 1, 0},
    {0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1, 0, 1},
    {0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1},
    {0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1},
    {1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0},
    {0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0},
    {0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0},
    {0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0},
    {0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0},
    {0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0},
    {0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0},
    {0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0},
    {0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0}
};

const uint8_t hand1[17][16] = {
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 0, 0, 0, 0},
    {0, 0, 0, 1, 0, 1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0},
    {0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1, 1, 0},
    {0, 0, 1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1, 1},
    {0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1},
    {0, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0},
    {0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0},
    {0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0},
    {0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0},
    {0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0},
    {0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0},
    {0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0},
    {0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0},
    {0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0}
};

int16_t lens[D][D] = {0};
uint16_t x, y, n, but;

uint8_t vbuff1[200][320] = {0};
uint8_t vbuff2[200][320] = {0};

uint8_t *tmem = (uint8_t*)0xB8000000L;
uint8_t *vmem = (uint8_t*)0xA0000000L;

uint8_t m = 0;
uint8_t pal[768] = {0};

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

int16_t initMouse()
{
    int16_t val;

    __asm {	
        xor     ax, ax
        int     0x33
        xor     al, al
        mov     val, ax
    }

    return val;
}

void getMousePos(uint16_t *x, uint16_t *y, uint16_t *but)
{
    __asm {
        mov     ax, 0x03
        int     0x33
        les     di, but
        mov     es:[di], bx
        les     di, x
        mov     es:[di], cx
        les     di, y
        mov     es:[di], dx
    }
}

void setMouseWindow(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
    __asm {
        mov     ax, 0x07
        mov     cx, x1
        mov     dx, x2
        int     0x33
        inc     ax
        mov     cx, y1
        mov     dx, y2
        int     0x33
    }
}

void setMousePos(uint16_t x, uint16_t y)
{
    __asm {
        mov     ax, 0x04
        mov     cx, x
        mov     dx, y
        int     0x33
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

void clearMem(uint8_t *where)
{
    __asm {
        les     di, where
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

void calcMask()
{
    float z;
    int16_t x, y;
    int16_t ux, uy, sx, sy;

    for (y = 0; y < D; y++)
    {
        for (x = 0; x < D; x++)
        {
            ux = x - (D >> 1);
            uy = y - (D >> 1);

            if (ux * ux + uy * uy < R * R)
            {
                z = sqrt(R * R - ux * ux - uy * uy);
                sx = (H - z) * (ux / z);
                sy = (H - z) * (uy / z);
                lens[y][x] = (sy << 8) + (sy << 6) + sx;
            }
            else lens[y][x] = 0;
        }
    }
}

void putHand(uint8_t n, uint16_t x, uint16_t y)
{
    int16_t i, j;
    for (i = 0; i < 17; i++)
    {
        for (j = 0; j < 16; j++)
        {
            switch (n)
            {
                case 0: if (hand0[i][j]) vbuff2[y + i + R - 8][x + j + R - 8] = 255; break;
                case 1: if (hand1[i][j]) vbuff2[y + i + R - 8][x + j + R - 8] = 255; break;
            }
        }
    }
}

void construct(uint8_t n, uint16_t xp, uint16_t yp)
{
    uint16_t ux, uy;
    uint16_t x, y, vp, hp;

    flip(vbuff1[0], vbuff2[0]);

    for (y = 0; y < D; y++)
    {
        for (x = 0; x < D; x++)
        {
            ux = x - (D >> 1);
            uy = y - (D >> 1);

            vp = y + yp + lens[y][x] / 320;
            hp = x + xp + lens[y][x] % 320;

            if (vp < 200 && vp > 0 && xp < 320 && xp > 0 && (R - 1) * (R - 1) > ux * ux + uy * uy)
            vbuff2[y + yp][x + xp] = vbuff1[vp][hp];
        }
    }

    putHand(n, xp, yp);
    retrace();
    flip(vbuff2[0], vmem);
}

void main()
{
    int16_t i;
    FILE *fp;

    clearScreen();
    printStr(1, 1, 0x0F, "LEN ZOOM - (c) 1998 by Nguyen Ngoc Van");
    printStr(1, 2, 0x07, "Any key : Start demo...");
    printStr(1, 3, 0x07, "Use mouse to move!");
    getch();

    if (!initMouse()) return;

    __asm {
        mov     ax, 0x13
        int     0x10
    }

    fp = fopen("assets/insect.cel", "rb");
    if (!fp) return;
    fseek(fp, 32, SEEK_SET);
    fread(pal, 1, 768, fp);
    fread(vbuff1, 1, 64000, fp);
    fclose(fp);

    __asm {
        mov     dx, 0x03C8
        xor     al, al
        out     dx, al
        inc     dx
        mov     cx, 768
        lea     si, pal
        rep     outsb
    }

    calcMask();
    setMouseWindow(5, 5, 315 - D, 200 - D);
    setMousePos(56, 32);

    n = 0;

    do {
        n++;
        getMousePos(&x, &y, &but);

        if (!(n % 20))
        {
            n = 0;
            m = (!m) ? 1 : 0;
        }

        construct(m, x, y);
    } while(!kbhit() && but != 1);

    __asm {
        xor     ax, ax
        int     0x33
        mov     ax, 0x03
        int     0x10
    }
}
