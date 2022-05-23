/*----------------------------------------------------*/
/* Packet  : Demo & Effect                            */
/* Effect  : The Hole                                 */
/* Author  : Nguyen Ngoc Van                          */
/* Memory  : Small                                    */
/* Address : pherosiden@gmail.com                     */
/* Website : http://www.codedemo.net                  */
/* Created : 07/02/1998                               */
/* Please sent to me any bugs or suggests.            */
/* You can use freely this code. Have fun :)          */
/*----------------------------------------------------*/

#include <dos.h>
#include <math.h>
#include <stdint.h>
#include <conio.h>
#include <string.h>

uint8_t *vmem = (uint8_t*)0xA0000000L;
uint8_t *tmem = (uint8_t*)0xB8000000L;

uint16_t circles[360][30] = {0};
uint16_t sinx[720] = {0};
uint16_t siny[720] = {0};

int16_t ypts[90][30] = {0};
int16_t xpts[90][30] = {0};

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
        push    ds
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

inline int16_t roundf(float x)
{
    if (x > 0.0) return x + 0.5;
    return x - 0.5;
}

void main()
{
    float r;
    int16_t xx, yy, i, x, y;

    clearScreen();
    printStr(1, 1, 0x0F, "HOLE - (c) 1998 by Nguyen Ngoc Van");
    printStr(1, 2, 0x07, "Press any key to start demo...");
    getch();

    for (i = 0; i < 30; i++) 
    {
        r = 0.0;
        for (x = 0; x < 360; x++)
        {
            r = r + 0.0175 * 4;
            circles[x][i] = roundf(sin(r) * (5 + (i << 2)) + (5 + (i << 2)));
        }
    }

    r = 0.0;
    for (x = 0; x < 720; x++)
    {
         r += 0.0175;
         sinx[x] = roundf(sin(r) * 140 + 140);
         siny[x] = roundf(cos(r) * 90 + 90);
    }

    __asm {
        mov     ax, 0x13
        int     0x10
    }

    for (i = 0; i < 256; i++) setRGB(i, 0, i, i << 1);

    i = 0;

    do {
        retrace();

        if (i > 358) i = 0;
        i += 2;

        for (y = 0; y < 30; y++)
        {
            for (x = 0; x < 90; x++)
            {
                xx = xpts[x][y];
                yy = ypts[x][y];
                vmem[(yy << 8) + (yy << 6) + xx] = 0;

                xx = (circles[x][y] + sinx[(y << 3) + i]) - (y << 2);
                yy = (circles[x + 22][y] + siny[i + 89 + (y << 2)]) - (y << 2);

                if (xx >= 0 && xx <= 319 && yy >= 0 && yy <= 199)
                {
                    vmem[(yy << 8) + (yy << 6) + xx] = y;
                    xpts[x][y] = xx;
                    ypts[x][y] = yy;
                }
            }
        }
    } while (!kbhit());

    __asm {
        mov     ax, 0x03
        int     0x10
    }
}
