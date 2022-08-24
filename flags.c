/*----------------------------------------------------*/
/* Packet  : Demo & Effect                            */
/* Effect  : Sin Flag                                 */ 
/* Author  : Nguyen Ngoc Van                          */ 
/* Memory  : Compact                                  */
/* Heaps   : 640K                                     */   
/* Address : pherosiden@gmail.com                     */
/* Website : http://www.codedemo.net                  */
/* Created : 12/01/1998                               */
/* Please sent to me any bugs or suggests.            */
/* You can use freely this code. Have fun :)          */
/*----------------------------------------------------*/
#include <dos.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <conio.h>

#define FX 3
#define FY 3
#define CE 191
#define M_PI 3.141592f

int16_t sintab[256] = {0};

uint8_t vbuff[64000] = {0};
uint8_t *vmem = (uint8_t*)0xA0000000L;
uint8_t *tmem = (uint8_t*)0xB8000000L;

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

void putPixel(int16_t x, int16_t y, uint8_t col)
{
    __asm {
        mov     ax, seg vbuff
        mov     es, ax
        mov     di, x
        mov     bx, y
        shl     bx, 6
        add     bh, byte ptr y
        add     di, bx
        mov     al, col
        stosb
    }
}

void printStr(int16_t x, int16_t y, uint8_t col, char *msg)
{
    uint16_t len = strlen(msg);

    __asm {
        mov     cx, len
        test    cx, cx
        jz      quit
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
    next:
        lodsb
        stosw
        loop    next
    quit:
    }
}

void clearMem()
{
    __asm {
        mov     ax, seg vbuff
        mov     es, ax
        xor     di, di
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

void main()
{
    int16_t i, j;
    uint16_t wave = 0, col = 0;

    clearScreen();
    printStr(1, 1, 0x0F, "SIN FLAG - (c) 1998 by Nguyen Ngoc Van");
    printStr(1, 2, 0x07, "Press any key to start demo...");
    getch();

    __asm {
        mov     ax, 0x13
        int     0x10
    }

    for (i = 0; i < 256; i++) sintab[i] = sin(i * 8 * M_PI / 255) * 10;

    while (!kbhit())
    {
        wave++;
        clearMem();

        for (i = 0; i <= 280 / FX; i++)
        {
            for (j = 0; j <= 160 / FY; j++)
            {
                //Deutschland
                //if (j * FY < 53) col = 8;
                //else if (j * FY < 106) col = 4; else col = 14;

                //French
                //if (j * FY < 53) col = 4;
                //else if (j * FY < 106) col = 15; else col = 9;

                //Schweden
                //if (i * FX > 80 && i * FX < 100 && j * FY >= 0 &&
                //	j * FY < 160 || j * FY > 70 && j * FY < 90 &&
                //	i * FX >= 0 && i * FX < 280) col = 15; else col = 4;

                //Schweiz
                //if (i * FX > 120 && i * FX < 160 && j * FY > 20 &&
                //	j * FY < 140 || j * FY > 60 && j * FY < 100 &&
                //	i * FX > 60 && i * FX < 220) col = 15; else col = 4;

                //USA
                if (i * FX < 120 && j * FY < 85)
                {
                    if ((j % 3) == 2 && (i % 7) == 2 && (j % 6) == 2 || ((3 + i) % 7) == 2 && ((j + 3) % 6) == 2) col = 15;
                    else col = 1;
                }
                else
                {
                    if ((j * FY) % 25 < 12) col = 4;
                    else col = 15;
                }
                
                putPixel(20 + sintab[(wave + (j + i) * CE) % 256] + i * FX, 20 + sintab[(wave + j + i * CE) % 256] + j * FY, col);
            }
        }
        retrace();
        flip();
    }

    __asm {
        mov     ax, 0x03
        int     0x10
    }
}
