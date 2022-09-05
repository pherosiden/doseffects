/*---------------------------------------------------*/
/* Packet  : Demo & Effect                           */
/* Effect  : Fire                                    */
/* Author  : Nguyen Ngoc Van                         */
/* Memory  : Small                                   */
/* Address : pherosiden@gmail.com                    */
/* Website : http://www.codedemo.net                 */
/* Created : 20/01/1998                              */
/* Please sent to me any bugs or suggests.           */
/* You can use freely this code. Have fun :)         */
/*---------------------------------------------------*/

#include <dos.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <conio.h>
#include <string.h>

#define YSTART          100
#define XSTART          150
#define XEND            170
#define FIRESTRENGTH    15

uint8_t vbuff[201][320] = {0};
uint8_t *vmem = (uint8_t*)0xA0000000L;

void waitRetrace()
{
    while (inp(0x03DA) & 8);
    while (!(inp(0x03DA) & 8));
}

void flip()
{
    memcpy(vmem, vbuff, 64000);
}

void doFire()
{
    int16_t x, y;

    for (x = XSTART; x < XEND; x++)
    {
        if (rand() % FIRESTRENGTH > 1) vbuff[198][x] = 255;
        else vbuff[198][x] = 0;
    }

    for (y = 198; y > YSTART; y--)
    {
        for (x = XSTART; x < XEND; x++)
        {
            vbuff[y - 1][x] = (
                vbuff[y][x - 1] +
                vbuff[y][x + 1] +
                vbuff[y + 1][x] +
                vbuff[y - 1][x] +
                vbuff[y - 1][x - 1] +
                vbuff[y - 1][x + 1] +
                vbuff[y + 1][x - 1] +
                vbuff[y + 1][x + 1]) >> 3;

            if (vbuff[y][x] > 200) vbuff[y][x] -= 1;
            else if (vbuff[y][x] > 148) vbuff[y][x] -= 1;
            else if (vbuff[y][x] > 4) vbuff[y][x] -= 2;
        }
    }
}

void doQuit()
{
    int16_t x, y;

    for (y = 199; y > YSTART; y--)
    {
        for (x = XSTART; x < XEND; x++)
        {
            vbuff[y - 1][x] = (
                vbuff[y][x - 1] +
                vbuff[y][x + 1] +
                vbuff[y + 1][x] +
                vbuff[y - 1][x] +
                vbuff[y - 1][x - 1] +
                vbuff[y - 1][x + 1] +
                vbuff[y + 1][x - 1] +
                vbuff[y + 1][x + 1]) >> 3;

            if (vbuff[y][x] > 200) vbuff[y][x] -= 2;
            else if (vbuff[y][x] > 4) vbuff[y][x] -= 1;
         }
     }
}

void setRGB(uint8_t col, uint8_t r, uint8_t g, uint8_t b)
{
    outp(0x03C8, col);
    outp(0x03C9, r);
    outp(0x03C9, g);
    outp(0x03C9, b);
}

void main()
{
    int16_t i;

    __asm {
        mov     ax, 0x13
        int     0x10
    }

    for (i = 0; i < 64; i++) setRGB(i, 0, 0, 0);
    for (i = 64; i < 128; i++) setRGB(i, i - 64, 0, 0);
    for (i = 128; i < 192; i++) setRGB(i, 63, i - 128, 0);
    for (i = 192; i < 255; i++) setRGB(i, 63, 63, i - 192);

    setRGB(255, 0, 0, 0);
    memset(vbuff[199], 255, 320);

    while (!kbhit())
    {
        doFire();
        waitRetrace();
        flip();
    }

    memset(vbuff[199], 0, 320);

    for (i = 0; i < 35; i++)
    {
        doQuit();
        waitRetrace();
        flip();
    }
    
    __asm {
        mov     ax, 0x03
        int     0x10
    }
}
