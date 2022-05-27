/*----------------------------------------------------*/
/* Packet  : Demo & Effect                            */
/* Effect  : Fire                                     */
/* Author  : Nguyen Ngoc Van                          */
/* Memory  : Compact                                  */ 
/* Heaps   : 640K                                     */
/* Address : pherosiden@gmail.com                     */
/* Website : http://www.codedemo.net                  */
/* Created : 28/01/1998                               */
/* Please sent to me any bugs or suggests.            */
/* You can use freely this code. Have fun :)          */
/*----------------------------------------------------*/

#include <dos.h>
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <conio.h>

#define BLINE 0
#define INTEN 240

uint8_t fptr[5][7680] = {0};
int16_t dbuff[102][160] = {0};
uint8_t vbuff[64000] = {0};

uint8_t *tmem = (uint8_t*)0xB8000000L;
uint8_t *vmem = (uint8_t*)0xA0000000L;

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

void setFirePal()
{
    int16_t i;

    for (i = 0; i < 64; i++)
    {
        setRGB(i, i, 0, 0);
        setRGB(i + 64, 63, i, 0);
        setRGB(i + 128, 63, 63, i);
        setRGB(i + 192, 63, 63, 0);
    }

    setRGB(247, 16, 0, 0);
    setRGB(248, 45, 0, 0);
    setRGB(249, 63, 0, 0);
    setRGB(250, 63, 6, 6);

    setRGB(251, 63, 14, 14);
    setRGB(252, 63, 22, 22);
    setRGB(253, 63, 30, 30);
    setRGB(254, 63, 47, 47);
}

void interpolation()
{
    __asm {
        mov     cx, 16159
        lea     di, dbuff + 320
    lp1:
        mov     ax, ds:[di - 2]
        add     ax, ds:[di]
        add     ax, ds:[di + 2]
        add     ax, ds:[di + 320]
        shr     ax, 2
        jz      lp2
        sub     ax, 1
    lp2:
        mov     word ptr ds:[di - 320], ax
        add     di, 2
        loop    lp1
    }
}

void purgeBuf()
{
    __asm {
        mov     ax, seg vbuff
        mov     es, ax
        xor     di, di
        lea     si, dbuff
        mov     dx, 100
    l3:
        mov     bx, 2
    l2:
        mov     cx, 160
    l1:
        mov     al, [si]
        mov     ah, al
        stosw
        lodsw
        dec     cx
        jnz     l1
        sub     si, 320
        dec     bx
        jnz     l2
        add     si, 320
        dec     dx
        jnz     l3
    }
}

void flip()
{
    __asm {
        les     di, vmem
        mov     ax, seg vbuff
        mov     ds, ax
        xor     si, si
        mov     cx, 16000
        rep     movsd
    }
}

uint8_t getPixel(int16_t x, int16_t y, uint8_t *where)
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

void putPixel(int16_t x, int16_t y, uint8_t c)
{
    __asm {
        mov     ax, seg vbuff
        mov     es, ax
        mov     bx, y
        shl     bx, 6
        add     bh, byte ptr y
        add     bx, x
        mov     di, bx
        mov     al, c
        stosb
    }
}

void blood(uint8_t num)
{
    uint8_t col;
    int16_t i, j;

    for (i = 0; i < 24; i++)
    {
        for (j = 0; j < 320; j++)
        {
            col = getPixel(j, i, fptr[num]);
            if (col != 255) putPixel(j, BLINE + i, col);
        }
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

void clearScreen()
{
    __asm {
        les     di, tmem
        xor     ax, ax
        mov     cx, 2000
        rep     stosw
    }
}

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

void main()
{
    FILE *fp;

    int16_t delta = 0;
    int16_t i = 0, k = 0, kk = 0;
        
    clearScreen();
    printStr(1, 1, 0x0F, "FIRE - (c) 1998 by Nguyen Ngoc Van");
    printStr(1, 2, 0x07, "Any key to start demo...");
    getch();
    
    fp = fopen("assets/bline.dat", "rb");
    if (!fp) exit(1);
    for (i = 0; i < 5; i++) fread(fptr[i], 1, 7680, fp);
    fclose(fp);

    __asm {
        mov     ax, 0x13
        int     0x10
    }

    srand(time(NULL));
    setFirePal();

    do {
        interpolation();
        for (i = 0; i < 160; i++)
        {
            delta = (rand() % 2) * INTEN;
            dbuff[100][i] = delta;
            dbuff[101][i] = delta;
        }

        purgeBuf();
        blood(k);
        flip();
        retrace();

        kk++;
        if (kk == 10)
        {
            k++;
            kk = 0;
        }

        if (k > 4) k = 0;
    } while (!kbhit());

    __asm {
        mov     ax, 0x03
        int     0x10
    }
}
