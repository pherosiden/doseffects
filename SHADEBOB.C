/*------------------------------------------------*/
/* Packet  : Demo & Effect                        */
/* Effect  : Shade Bob                            */
/* Author  : Nguyen Ngoc Van                      */
/* Memory  : Compact                              */
/* Heaps   : 640K                                 */
/* Address : pherosiden@gmail.com                 */ 
/* Website : http://www.codedemo.net              */
/* Created : 01/03/1998                           */
/* Please sent to me any bugs or suggests.        */
/* You can use freely this code. Have fun :)      */
/*------------------------------------------------*/

#include <dos.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <conio.h>

int16_t xp, xr;
int16_t yp, yr;

int16_t i, j, k, v;
int16_t gw, gr, an;

uint8_t pal[256][3] = {0};
uint8_t vbuff[200][320] = {0};

uint8_t *vmem = (uint8_t*)0xA0000000L;
uint8_t *tmem = (uint8_t*)0xB8000000L;

inline int16_t random(int16_t x)
{
    if (x == 0) return 0;
    return rand() % x;
}

void shadeBob(int16_t x, int16_t y)
{
    __asm {
        les     di, vmem
        mov     ax, y
        shl     ax, 6
        add     ah, byte ptr y
        add     ax, x
        add     di, ax
        mov     ax, v
        mov     bx, gr
    s1:
        mov     cx, gw
    s2:
        add     es:[di], ax
        add     di, 2
        loop    s2
        add     di, j
        dec     bx
        jnz     s1
    }
}

void waitRetrace()
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

void clearScreen()
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

void flip(uint8_t *src, uint8_t *dst)
{
    __asm {
        les     di, dst
        lds     si, src
        mov     cx, 16000
        rep     movsd
    }
}

void printStr(int16_t x, int16_t y, uint8_t col, char *msg)
{
    int16_t len = strlen(msg);

    __asm {
        les     di, tmem
        lds     si, msg
        add     di, x
        shl     di, 1
        mov     ax, y
        shl     ax, 5
        add     di, ax
        shl     ax, 2
        add     di, ax
        mov     ah, col
        mov     cx, len
    next:
        lodsb
        stosw
        loop    next
    }
}

void makePal()
{
    int16_t i;

    for (i = 0; i < 64; i++)
    {
        pal[i	+ 0][0] = i;
        pal[32  + i][1] = i;
        pal[64  + i][0] = 63 - i; 
        pal[64  + i][2] = i;
        pal[128 + i][0] = i;
        pal[128 + i][2] = 63 - i;
        pal[96  + i][1] = 63 - i;
        pal[192 + i][0] = 63 - i;
    }

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

void main()
{
    FILE *fp;

    clearScreen();
    printStr(1, 1, 0x0F, "SHADE BOB - (c) 1998 by Nguyen Ngoc Van");
    printStr(1, 2, 0x07, "Press any key to start demo...");
    getch();

    __asm {
        mov     ax, 0x13
        int     0x10
    }

    srand(time(NULL));

    an = 500;
    gr = random(120) + 40;
    gw = gr >> 1;
    j = 320 - gr;
    k = 200 - gr;

    fp = fopen("killer.cel", "rb");
    if (!fp) return;

    fseek(fp, 800, SEEK_SET);
    fread(vbuff[0], 1, 64000, fp);
    fclose(fp);
    
    makePal();

    do {
        waitRetrace();
        flip(vbuff[0], vmem);

        xp = random(j - 4) + 2;
        yp = random(k - 4) + 2;

        do
        {
            xr = random(5) - 2;
            yr = random(5) - 2;
            v = random(5) - 2;
            if (inp(0x60) == 1) break;
        } while (!xr && !yr && !v);

        v += (v << 8);

        for (i = 0; i < an; i++)
        { 
            xp += xr;
            yp += yr;

            if (xp < 2 || xp >= j - 2) xr = -xr;
            if (yp < 2 || yp >= k - 2) yr = -yr;

            shadeBob(xp, yp);
            shadeBob(j - xp, yp);

            shadeBob(xp, k - yp);
            shadeBob(j - xp, k - yp);
            waitRetrace();

            if (kbhit()) break;
        }
    } while (!kbhit());

    __asm {
        mov     ax, 0x03
        int     0x10
    }
}
