/*---------------------------------------------------*/
/* Packet  : Demo & Effect                           */
/* Effect  : Fire                                    */
/* Author  : Nguyen Ngoc Van                         */
/* Memory  : Small                                   */
/* Address : siden@codedemo.net                      */
/* Website : http://www.codedemo.net                 */
/* Created : 20/01/1998                              */
/* Please sent to me any bugs or suggests.           */
/* You can use freely this code. Have fun :)         */
/*---------------------------------------------------*/

#include <dos.h>
#include <time.h>
#include <math.h>
#include <stdlib.h>
#include <stdint.h>
#include <conio.h>

#define M_PI	3.14159265f

float sint[100] = {0};
uint8_t vbuff[30][100] = {0};
uint8_t *vmem = (uint8_t *)0xA0000000L;

inline int16_t random(int16_t x)
{
    if (x == 0) return 0;
    return rand() % x;
}

inline int16_t roundf(float x)
{
    if (x > 0.0) return x + 0.5;
    return x - 0.5;
}

void main()
{
    int16_t x, y, i = 0, j = 0;
    
    srand(time(NULL));

    __asm {
        mov     ax, 0x13
        int     0x10
    }

    outp(0x03C8, 0);
    for (i = 0; i < 16; i++)
    {
        outp(0x3C9, i << 2);
        outp(0x3C9, i << 2);
        outp(0x3C9, 0);
    }

    for (i = 0; i < 32; i++)
    {
        outp(0x3C9, 63);
        outp(0x3C9, 63 - i << 1);
        outp(0x3C9, 0);
    }

    for (i = 0; i < 64; i++)
    {
        outp(0x3C9, 63);
        outp(0x3C9, 0);
        outp(0x3C9, 0);
    }

    for (i = 0; i < 100; i++) sint[i] = sin(i * M_PI / 24);

    while (!kbhit())
    {
        for (x = 5; x < 24; x++)
        {
            if (random(2) < 1) vbuff[x][99] = 200;
            else vbuff[x][99] = 0;
        }

        for (y = 0; y < 98; y++)
        {
            for (x = 1; x < 28; x++)
            {
                vbuff[x][y] = (vbuff[x - 1][y] + vbuff[x][y] + vbuff[x + 1][y] + vbuff[x - 1][y + 1] + vbuff[x + 1][y + 1] + vbuff[x - 1][y + 2] + vbuff[x][y + 2] + vbuff[x + 1][y + 2]) >> 3;
            }
        }

        j++;

        if (j > 99) j -= 99;

        for (y = 0; y < 100; y++)
        {
            for (x = 0; x < 30; x++)
            {
                vmem[(y << 6) + (y << 8) + x + 145] = vbuff[x][y];	
                i = y + j;
                if (i > 99) i -= 99;
                vmem[145 + x + roundf(sint[i] * ((99 - y) >> 2)) + (131 - (y - 2) / 3) * 320] = vbuff[x][y];
            }
        }
    }

    __asm {
        mov     ax, 0x03
        int     0x10
    }
}
