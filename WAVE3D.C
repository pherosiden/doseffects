/*----------------------------------------------------*/
/* Packet  : Demo & Effect                            */   
/* Effect  : Wave 3D                                  */
/* Author  : Nguyen Ngoc Van                          */ 
/* Memory  : Small                                    */
/* Address : pherosiden@gmail.com                     */ 
/* Website : http://www.codedemo.net                  */
/* Created : 21/04/1998                               */
/* Please sent to me any bugs or suggests.            */ 
/* You can use freely this code. Have fun :)          */ 
/*----------------------------------------------------*/

#include <dos.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <conio.h>

#define S159    25281.0
#define S31	    961.0
#define M_PI    3.14159265f

double r, d, v, a;

uint8_t wb[64][128] = {0};
uint8_t lb[32][160] = {0};
uint8_t heighest[160] = {0};
uint16_t c, lasth, limit;

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

void clearScreen()
{
    __asm {
        mov    ax, 0xB800
        mov    es, ax
        xor    di, di
        xor    ax, ax
        mov    cx, 2000
        rep    stosw
    }
}

void printStr(int16_t x, int16_t y, uint8_t col, char *msg)
{
    int16_t len = strlen(msg);

    __asm {
        mov    ax, 0xB800
        mov    es, ax
        xor    di, di
        lds    si, msg
        add    di, x
        shl    di, 1
        mov    ax, y
        shl    ax, 5
        add    di, ax
        shl    ax, 2
        add    di, ax
        mov    ah, col
        mov    cx, len
    next:
        lodsb
        stosw
        loop   next
    }
}

void generateLand()
{
    int16_t i, j;
    for (i = 0; i <= 31; i++)
    {
        for (j = 0; j <= 159; j++)
        {
            lb[i][j] = sqrt((i * i / S31) + (j * j / S159)) * 127;
            if (lb[i][j] > 127) lb[i][j] = 127;
        }
    }
}

void generateWav()
{
    int16_t i, j;
    for (j = 0; j <= 63; j++)
    {
        for (i = 0; i <= 127; i++)
        {
            r = (i + 1) * 5 * M_PI / 320;
            d = j * M_PI / 32;
            v = ((sin(r - d) - sin(-d)) / r) - (cos(r - d) / 2);
            a = cos(i * M_PI / 256);
            wb[j][i] = 100.0 + 100 * a * a * v;
        }
    }
}

void putPixel(uint16_t x, uint16_t y, uint8_t c)
{
    __asm {
        mov    ax, 0xA000
        mov    es, ax
        xor    di, di
        mov    bx, y
        shl    bx, 6
        add    bh, byte ptr y
        add    bx, x
        add    di, bx
        mov    al, c
        stosb
    }
}

void main()
{
    int16_t i, j;
    uint16_t n = 0;

    clearScreen();
    printStr(1, 1, 0x0F, "RIPPLE - (c) 1998 by Nguyen Ngoc Van");
    printStr(1, 2, 0x07, "Press any key to start demo...");
    getch();

    __asm {
        mov    ax, 0x13
        int    0x10
    }

    generateLand();
    generateWav();

    outp(0x03C8, 0);
    for (i = 0; i <= 31; i++)
    {
        outp(0x3C9, i << 1);
        outp(0x3C9, 0);
        outp(0x3C9, 0);
    }

    for (i = 0; i <= 31; i++)
    {
        outp(0x3C9, 63);
        outp(0x3C9, i << 1);
        outp(0x3C9, 0);
    }

    do {
        n = (n + 1) % 64;
        for (j = 0; j <= 159; j++)
        {
            c = 63;
            lasth = 199;
            for (i = 31; i >=  0; i--)
            {
                limit = (56 + i) + wb[n][lb[i][j]];
                while (lasth > limit)
                {
                    putPixel(j + 160, lasth, c);
                    putPixel(159 - j, lasth, c);
                    lasth--;
                }
                c--;
            }
            for (i = 0; i <= 31; i++)
            {
                limit = (56 - i) + wb[n][lb[i][j]];
                while (lasth > limit)
                {
                    putPixel(j + 160, lasth, c);
                    putPixel(159 - j, lasth, c);
                    lasth--;
                }
                c--;
            }
            for (i = lasth; i >= heighest[j]; i--)
            {
                putPixel(j + 160, i, 0);
                putPixel(159 - j, i, 0);
            }
            heighest[j] = lasth;
        }
        retrace();
    } while (!kbhit());

    __asm {
        mov    ax, 0x03
        int    0x10
    }
}
