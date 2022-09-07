/*---------------------------------------------------*/
/* Packet  : Demo & Effect                           */
/* Effect  : Fire                                    */
/* Author  : Nguyen Ngoc Van                         */
/* Memory  : Small                                   */
/* Address : pherosiden@gmail.com                    */
/* Website : http://www.codedemo.net                 */
/* Created : 02/06/1999                              */
/* Please sent to me any bugs or suggests.           */
/* You can use freely this code. Have fun :)         */
/*---------------------------------------------------*/

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <conio.h>
#include <math.h>
#include <time.h>
#include <string.h>

uint16_t rdx = 0;
uint8_t vbuff[64000] = {0};
uint8_t randomize[19200] = {0};

float idx = 0;
float sinus[1000] = {0.0};

inline int16_t roundf(float x)
{
    if (x > 0.0) return x + 0.5;
    return x - 0.5;
}

inline int16_t random(int16_t x)
{
    if (x == 0) return x;
    return rand() % x;
}

void blur()
{
    __asm {
        lea     di, vbuff
        add     di, 320
        mov     cx, 63360
        xor     ax, ax
        xor     bx, bx
    lp:
        mov     al, es:[di - 1]
        mov     bl, es:[di + 1]
        add     ax, bx
        mov     bl, es:[di - 320]
        add     ax, bx
        mov     bl, es:[di + 320]
        add     ax, bx
        shr     ax, 2
        stosb
        loop    lp
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

void flip()
{
    __asm {
        xor     di, di
        mov     ax, seg vbuff
        mov     ds, ax
        xor     si, si
        push    0xA000
        pop     es
        mov     cx, 16000
        rep     movsd
    }
}

void setPalette(uint8_t no, uint8_t r, uint8_t g, uint8_t b)
{
    __asm {
        mov     dx, 0x03C8
        mov     al, no
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

void retrace()
{
    __asm {
        mov     dx, 0x03DA
    wait1:
        in      al, dx
        and     al, 0x08
        jnz     wait1
    wait2:
        in      al, dx
        and     al, 0x08
        jz      wait2
    }
}

void pierra()
{
    int16_t i, j;
    float x, y;

    idx += 0.1;
    x = sinus[roundf(idx * 159) % 1000] * sinus[roundf(idx * 83 + 130) % 1000] * 140;
    y = sinus[roundf(idx * 97 + 153) % 1000] * sinus[roundf(idx * 107) % 1000] * 80;

    for (i = 1; i <= 319; i++)
    {
        for (j = 1; j <= 199; j++)
        {
            if (rdx >= 19200) rdx = 0;
            if (randomize[rdx] == 1) putPixel(i, j, 0);
            rdx++;
        }
    }

    for (i = 150 + roundf(-x); i <= 170 + roundf(-x); i++)
    {
        for (j = 90 + roundf(-y); j <= 110 + roundf(-y); j++) putPixel(i, j, 255);
    }

    for (i = 150 + roundf(x); i <= 170 + roundf(x); i++)
    {
        for (j = 90 + roundf(y); j <= 110 + roundf(y); j++) putPixel(i, j, 255);
    }

    for (i = 0; i < 8; i++) blur();
}

void main()
{
    int16_t i, j;

    __asm {
        mov     ax, 0x13
        int     0x10
    }

    srand(time(NULL));
    //memset(randomize, 0, 19200);
    //memset(sinus, 0, 1000);

    for (i = 0; i < 1000; i++) sinus[i] = sin(i * 0.00628318530718);

    for (i = 0; i < 19200; i++)
    {
        if (random(60) == 1) randomize[i] = 1;
        else randomize[i] = 0;
    }

    for (i = 0; i < 64; i++) setPalette(i, i, 0, 0);
    for (i = 64; i < 128; i++) setPalette(i, 63, i, 0);
    for (i = 128; i < 192; i++) setPalette(i, 63, 63, i);
    for (i = 192; i < 256; i++) setPalette(i, 63, 63, 63);
    
    for (j = 1; j <= 199; j++)
    {
        for (i = 1; i <= 319; i++) putPixel(i, j, 0);
    }

    while (!kbhit())
    {
        pierra();
        retrace();
        flip();
    }

    __asm {
        mov     ax, 0x03
        int     0x10
    }
}
