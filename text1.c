/*---------------------------------------------------*/
/* Packet  : Demo & Effect                           */
/* Effect  : String Text                             */
/* Author  : Nguyen Ngoc Van                         */ 
/* Memory  : Small                                   */
/* Address : pherosiden@gmail.com                    */
/* Website : http://www.codedemo.net                 */
/* Created : 23/04/1998                              */
/* Please sent to me any bugs or suggests.           */
/* You can use freely this code. Have fun :)         */ 
/*---------------------------------------------------*/

#include <dos.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <conio.h>

uint8_t *chars[64] = {0};
uint8_t pal[768] = {0};
uint8_t chrinfo[64][3] = {0};
uint8_t *vmem = (uint8_t*)0xA0000000L;

void putPixel(uint16_t x, uint16_t y, uint8_t col, uint8_t *mem)
{
    __asm {
        les     di, mem
        mov     bx, y
        shl     bx, 6
        add     bh, byte ptr y
        add     bx, x
        add     di, bx
        mov     al, col
        stosb
    }
}

void loadFont(char *fname)
{
    char ch;
    FILE *fp;
    int16_t i;
    uint8_t tx, ty;

    fp = fopen(fname, "rb");
    if (!fp) exit(1);
    
    fread(pal, 1, 768, fp);

    while (!feof(fp))
    {
        fread(&ch, 1, 1, fp);
        fread(&tx, 1, 1, fp);
        fread(&ty, 1, 1, fp);

        chars[ch - 32] = (uint8_t*)calloc(tx * ty, 1);
        if (!chars[ch - 32]) exit(1);

        chrinfo[ch - 32][0] = tx;
        chrinfo[ch - 32][1] = ty;
        chrinfo[ch - 32][2] = 1;
        fread(chars[ch - 32], 1, tx * ty, fp);
    }

    fclose(fp);

    __asm {
        push    ds
        mov     dx, 0x03C8
        xor     ax, ax
        out     dx, al
        inc     dx
        lea     si, pal
        mov     cx, 768
        rep     outsb
        pop     ds
    }
}

void putChar(int16_t dx, int16_t dy, char ch)
{
    uint8_t c, x, y, i, j;

    x = chrinfo[ch - 32][0];
    y = chrinfo[ch - 32][1];

    for (j = 0; j < y; j++)
    {
        for (i = 0; i < x; i++)
        {
            c = *(chars[ch - 32] + j * x + i);
            if (c > 0) putPixel(dx + i, dy + j, c, vmem);
        }
    }
}

void writeXY(int16_t x, int16_t y, char *str)
{
    int16_t i;
    for (i = 0; i < strlen(str); i++)
    {
        if (str[i] >= 32 && str[i] <= 93)
        {
            if (str[i] == 32) x += chrinfo['H' - 32][0] / 2;
            else
            {
                putChar(x, y, str[i]);
                x += chrinfo[str[i] - 32][0] + 1;
            }
        }
    }
}

void deAllocFMem()
{
    int16_t i;
    for (i = 0; i < 64; i++)
    {
        if (chrinfo[i][2])
        {
            free(chars[i]);
            chrinfo[i][2] = 0;
        }
    }
}

void main()
{
    __asm {
        mov     ax, 0x13
        int     0x10
    }

    loadFont("font001.fnt");
    writeXY(1,  2, "I LOVE ...");
    writeXY(1, 38, "DEMOS!");
    writeXY(1, 74, "GRAPHICS!");
    writeXY(1, 110, "BITMAP!");
    writeXY(1, 146, "SUPER VESA!");
    
    getch();
    deAllocFMem();

    __asm {
        mov     ax, 0x03
        int     0x10
    }
}
