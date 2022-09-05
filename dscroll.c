/*---------------------------------------------------*/
/* Packet  : Demo & Effect                           */
/* Effect  : Scroll Text                             */
/* Author  : Nguyen Ngoc Van                         */
/* Memory  : Compact                                 */
/* Heaps   : 640K                                    */
/* Address : siden@codedemo.net                      */
/* Website : http://www.codedemo.net                 */
/* Created : 26/02/1998                              */
/* Please sent to me any bugs or suggests.           */
/* You can use freely this code. Have fun :)         */
/*---------------------------------------------------*/

#include <dos.h>
#include <math.h>
#include <stdio.h>
#include <conio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#define OFS 	80
#define AMP 	8
#define LEN 	256
#define SIZE 	2
#define CURVE 	2
#define XMAX 	(309 / SIZE)
#define YMAX 	8
#define SPEED 	-1
#define M_PI 	3.14159265

const uint8_t mask[] = {128, 64, 32, 16, 8, 4, 2, 1};
const char *text = "FONT SCROLLING - (c) 1998 by Nguyen Ngoc Van ";

uint8_t *tbuff = NULL;
uint8_t *vmem = (uint8_t*)0xA0000000L;

uint16_t fseg, fofs;
uint16_t sintab[LEN] = {0};
uint8_t vbuff[64000] = {0};
uint16_t postab[XMAX][YMAX] = {0};
uint8_t bitmap[XMAX][YMAX] = {0};

void moveData(void *dst, void *src, uint16_t len)
{
    __asm {
        lds     si, src
        les     di, dst
        mov     cx, len
        mov     ax, cx
        shr     cx, 2
        rep     movsd
        mov     cl, al
        and     cl, 0x03
        rep     movsb
    }
}

void initFont8x8()
{
    __asm {
        push    es
        push    bp
        mov     ax, 0x1130
        mov     bx, 0x0300
        int     0x10
        mov     fseg, es
        mov     fofs, bp
        pop     bp
        pop     es
    }

    tbuff = (uint8_t*)MK_FP(fseg, fofs);
}

void scrollText()
{
    int16_t x, y, i;
    uint16_t j, len;
    uint8_t pos, c;

    pos = j = 0;
    len = strlen(text);

    while (!kbhit())
    {
        c = text[pos];

        for (i = 0; i < YMAX; i++)
        {
            moveData(bitmap[0], bitmap[1], YMAX * XMAX);

            for (y = 0; y < YMAX; y++)
            {
                if (mask[i] & tbuff[YMAX * c + y]) bitmap[XMAX - 1][y] = 15;
                else bitmap[XMAX - 1][y] = 0;
            }

            while (inp(0x03DA) & 8);
            while (!(inp(0x03DA) & 8));

            for (x = 0; x < XMAX; x++)
            {
                for (y = 0; y < YMAX; y++)
                {
                    vmem[postab[x][y]] = vbuff[postab[x][y]];
                    postab[x][y] = (SIZE * y + sintab[(j + x + CURVE * y) % LEN]) * 320 + SIZE * x + sintab[(x + y) % LEN] - OFS;
                    if (bitmap[x][y]) vmem[postab[x][y]] = bitmap[x][y];
                    else vmem[postab[x][y]] = vbuff[postab[x][y]];
                }
            }

            j = (j + SPEED) % LEN;
        }

        pos++;
        if (pos >= len) pos = 0;

    }
}

inline int16_t roundf(float x)
{
    if (x >= 0.0) return x + 0.5;
    return x - 0.5;
}

void main()
{
    FILE *fp;
    int16_t i;
    uint8_t pal[768] = {0};

    __asm {
        mov     ax, 0x13
        int     0x10
    }
    
    fp = fopen("assets/friend.cel", "rb");
    if (!fp) return;

    fseek(fp, 32, SEEK_SET);
    fread(pal, 1, 768, fp);
    fread(vbuff, 1, 64000, fp);
    fclose(fp);

    __asm {
        mov     dx, 0x03C8
        xor     al, al
        out     dx, al
        inc     dx
        lea     si, pal
        mov     cx, 768
        rep     outsb
    }

    moveData(vmem, vbuff, 64000);
    for (i = 0; i < LEN; i++) sintab[i] = roundf(sin(i * 4 * M_PI / LEN) * AMP + OFS);
    
    initFont8x8();
    scrollText();

    __asm {
        mov     ax, 0x03
        int     0x10
    }
}
