/*---------------------------------------------------*/
/* Packet  : Demo & Effect                           */
/* Effect  : Fire                                    */
/* Author  : Nguyen Ngoc Van                         */
/* Memory  : Compact                                 */
/* Heaps   : 640K                                    */  
/* Address : pherosiden@gmail.com                    */
/* Website : http://www.codedemo.net                 */
/* Created : 15/01/1998                              */
/* Please sent to me any bugs or suggests.           */
/* You can use freely this code. Have fun :)         */ 
/*---------------------------------------------------*/

#include <dos.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <conio.h>

uint8_t ned = 0;
uint8_t vbuff[204][320] = {0};
uint8_t *tmem = (uint8_t*)0xB8000000L;

inline int16_t random(int16_t x)
{
    if (x == 0) return 0;
    return rand() % x;
}

void putPixel(int16_t x, int16_t y, uint8_t col)
{
    __asm {
        lea     di, vbuff
        mov     bx, y
        shl     bx, 6
        add     bh, byte ptr y
        add     bx, x
        add     di, bx
        mov     al, col
        stosb
    }
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
        mov     cx, 16320
        rep     stosd
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

void initFire()
{
    int16_t j;

    __asm {
        mov     ax, 0x13
        int     0x10
    }

    for (j = 180; j >= 64; j--) setRGB(j, 63, 63 - ((j - 64.0) / (180 - 64) * 63), 0);
    for (j = 0; j <= 53; j++) setRGB(63 - j, 63 - j, 63 - j, j);
    for (j = 0; j <= 10; j++) setRGB(10 - j, 10 - j, 10 - j, 50 - 5 * j);
}

void doFire()
{
    int16_t j, i;
    uint8_t col;

    if (ned)
    {
        for (j = 0; j < 20; j++)
        {
            ned = 0;
            col = random(91) + 90;

            for (i = 0; i < 16; i++)
            {
                vbuff[202][(j << 4) + i] = col;
                vbuff[203][(j << 4) + i] = col;
            }
        }
    }
    else
    {
        for (j = 0; j < 40; j++)
        {
            ned = 1;
            col = random(91) + 90;

            for (i = 0; i < 8; i++)
            {
                vbuff[202][(j << 3) + i] = col;
                vbuff[203][(j << 3) + i] = col;
            }
        }
    }

    __asm {
        lea     di, vbuff
        add     di, 16000
        mov     cx, 48640
    start:
        xor     ax, ax
        xor     dx, dx
        mov     bx, di
        add     bx, 319
        mov     al, [bx]
        inc     bx
        mov     dl, [bx]
        add     ax, dx
        inc     bx
        mov     dl, [bx]
        add     ax, dx
        add     bx, 319
        mov     dl, [bx]
        add     ax, dx
        sub     ax, 4
        mov     bx, 4
        xor     dx, dx
        div     bx
        cmp     al, 180
        ja    	fin
        mov     [di], al
    fin:
        inc     di
        loop    start
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

void main()
{
    clearScreen();
    printStr(1, 1, 0x0F, "FIRE - (c) 1998 by Nguyen Ngoc Van");
    printStr(1, 2, 0x07, "Any key to start demo...");
    getch();

    srand(time(NULL));
    initFire();

    do {
        doFire();
        retrace();
        flip(0xA000);
    } while (!kbhit());

    __asm {
        mov     ax, 0x03
        int     0x10
    }
}
