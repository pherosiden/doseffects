/*----------------------------------------------------*/
/* Packet  : Demo & Effect                            */
/* Effect  : Text Scrolling                           */ 
/* Author  : Nguyen Ngoc Van                          */ 
/* Memory  : Compact                                  */ 
/* Heaps   : 640K                                     */
/* Address : pherosiden@gmail.com                     */
/* Website : http://www.codedemo.net                  */
/* Created : 28/02/1998                               */
/* Please sent to me any bugs or suggests.            */
/* You can use freely this code. Have fun :)          */
/*----------------------------------------------------*/

#include <dos.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <conio.h>

#define M_PI 3.141592f

const uint8_t bitmask[] = {128, 64, 32, 16, 8, 4, 2, 1};
const char *scrolledtext = "I LOVE DEMO, GRAPHICS, BITMAP, SUPER VESA ! ";

uint8_t *fbuff = NULL;
uint16_t fseg = 0, fofs = 0;

uint8_t pal[768] = {0};
uint16_t sintab[320] = {0};
uint8_t chrtab[320] = {0};
uint8_t coltab[320] = {0};
uint8_t vbuff[64000] = {0};
uint8_t *vmem = (uint8_t*)0xA0000000L;

inline uint8_t peekb(uint16_t seg, uint16_t ofs)
{
    return *(uint8_t*)MK_FP(seg, ofs);
}

inline void pokeb(uint16_t seg, uint16_t ofs, uint8_t col)
{
    *(uint8_t*)MK_FP(seg, ofs) = col;
}

void initFont8x16()
{
    __asm {
        push    bp
        mov     ax, 0x1130
        mov     bx, 0x0600
        int     0x10
        mov     fseg, es
        mov     fofs, bp
        pop     bp
    }

    fbuff = (uint8_t*)MK_FP(fseg, fofs);
}

void calcSin()
{
    int16_t x;

    for (x = 0; x < 320; x++)
    {
        sintab[x] = sin(x * 5 * M_PI / 320) * 22 + 22;
        if (cos(x * 5 * M_PI / 320) <= 0) coltab[x] = 15;
        else coltab[x] = 14;
    }
}

void scrollText()
{
    int16_t x, i, k;
    uint16_t ofs, pos = 0;
    uint8_t chr;

    while (!kbhit())
    {
        chr = scrolledtext[pos];
        for (i = 0; i < 16; i++)
        {
            for (x = 0; x < 319; x++) chrtab[x] = chrtab[x + 1];
            chrtab[319] = fbuff[(16 * chr) + i];

            while(inp(0x03DA) & 8);
            while(!(inp(0x03DA) & 8));

            for (x = 0; x < 312; x++)
            {
                for (k = 0; k < 16; k++)
                {
                    ofs = (80 - sintab[x]) * 320 + (x + k);
                    if (chrtab[x] & bitmask[k]) vmem[ofs] = coltab[x];
                    else vmem[ofs] = vbuff[ofs];
                }
            }
        }

        pos++;
        if (pos >= strlen(scrolledtext)) pos = 0;
    }
}

void main()
{
    FILE *fp;
    int16_t i;

    __asm {
        mov     ax, 0x13
        int     0x10
    }

    fp = fopen("assets/friend.cel", "rb");
    if(!fp) return;

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
        les     di, vmem
        mov     ax, seg vbuff
        mov     ds, ax
        xor     si, si
        mov     cx, 16000
        rep     movsd
    }

    calcSin();
    initFont8x16();
    scrollText();

    __asm {
        mov     ax, 0x03
        int	    0x10
    }
}
