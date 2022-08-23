/*----------------------------------------------------*/
/* Packet  : Demo & Effect                            */
/* Effect  : Turnel                                   */
/* Author  : Nguyen Ngoc Van                          */
/* Memory  : Compact                                  */
/* Heaps   : 640K                                     */
/* Address : pherosiden@gmail.com                     */
/* Website : http://www.codedemo.net                  */
/* Created : 27/04/1998                               */
/* Please sent to me any bugs or suggests.            */
/* You can use freely this code. Have fun :)          */
/*----------------------------------------------------*/

#include <dos.h>
#include <math.h>
#include <time.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <conio.h>

uint8_t br = 120;
uint8_t pal[256][3] = {0};
int16_t tbuff[100][320] = {0};
uint8_t vbuff[200][320] = {0};

uint8_t *vmem = (uint8_t*)0xA0000000L;
uint8_t *tmem = (uint8_t*)0xB8000000L;

inline int16_t random(int16_t x)
{
    if (x == 0) return 0;
    return rand() % x;
}

void setPalette()
{
    __asm {
        xor     ax, ax
        mov     dx, 0x03C8
        out     dx, al
        inc     dx
        mov     cx, 765
        lea     si, pal
    next:
        mov     ah, 127
        sub     ah, [si]
        xor     al, al
        mul     ah
        shl     ax, 2
        lodsb
        add     al, ah
        mul     br
        mov     al, ah
        out     dx, al
        loop    next
    }
}

void waitRetrace()
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
    next:
        lodsb
        test    al, al
        jz      quit
        stosw
        jmp     next
    quit:
    }
}

void flipScreen()
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

void tunnelBlur()
{
    __asm {
        mov     ax, seg vbuff
        mov     es, ax
        mov     di, 320
        mov     cx, 64000 - 640
        xor     bx, bx
    next:
        xor     ax, ax
        mov     bl, es:[di - 1]
        add     ax, bx
        mov     bl, es:[di + 1]
        add     ax, bx
        mov     bl, es:[di - 320]
        add     ax, bx
        mov     bl, es:[di + 320]
        add     ax, bx
        shr     ax, 2
        stosb
        loop    next
    }
}

inline int16_t pike(int16_t x, int16_t arg, int16_t a, int16_t b, int16_t c)
{
    if (x < arg) return a + (b - a) * x / arg;
    return b + (c - b) * (x - arg) / (256 - arg);
}

void makeTunnel()
{
    __asm {
        mov     ax, seg vbuff
        mov     ds, ax
        mov     ax, seg tbuff
        mov     es, ax
        xor     si, si
        xor     di, di
        mov     cx, 32000
    next:
        mov     al, [si]
        shr     al, 1
        mov     bx, es:[di]
        mov     ah, [si + bx]
        shr     ah, 1
        add     al, ah
        mov   	[si], al
        not     si
        not     bx
        mov     al, [64000 + si]
        shr     al, 1       
        mov     ah, [64000 + si + bx]
        shr     ah, 1
        add     al, ah
        mov     [64000 + si], al
        not     si
        inc     si
        add     di, 2
        loop    next
    }
}

void showTunnel()
{
    int16_t i, j;
    int16_t x, y, lx, ly;

    for (i = 0; i < 255; i++)
    {
        pal[i][0] = pike(i, 128, 0, 64, 64);
        pal[i][1] = pike(i, 128, 0, 127, 64);
        pal[i][2] = pike(i, 192, 0, 127, 32);
    }

    pal[255][0] = 120;
    pal[255][1] = 120;
    pal[255][2] = 0;
    setPalette();

    for (i = 0; i < 100; i++)
    {
        for (j = 0; j < 320; j++)
        {
            lx = 160 - j;
            ly = 100 - i;
            x = (lx >> 4) + random(3) + (ly >> 5) - 1;
            y = (ly >> 4) + random(3) - (lx >> 5) - 1;
            tbuff[i][j] = (y << 8) + (y << 6) + x;
        }
    }
    
    do {
        for (i = 0; i < 100; i++)
        {
            x = rand() % 319;
            y = rand() % 199;
            vbuff[y][x] = 240;
            vbuff[y][x + 1] = 240;
            vbuff[y + 1][x] = 240;
            vbuff[y + 1][x + 1] = 240;
        }

        makeTunnel();
        tunnelBlur();
        waitRetrace();
        flipScreen();
    } while (!kbhit());
}

void main()
{
    clearScreen();
    printStr(1, 1, 0x0F, "TUNNEL - (c) 1998 by Nguyen Ngoc Van");
    printStr(1, 2, 0x07, "Press any key to start demo...");
    getch();

    srand(time(NULL));

    __asm {
        mov     ax, 0x13
        int     0x10
        call    showTunnel
        mov     ax, 0x03
        int     0x10
    }
}
