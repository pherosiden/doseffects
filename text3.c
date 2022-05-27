/*----------------------------------------------------*/
/* Packet  : Demo & Effect                            */
/* Effect  : String text                              */
/* Author  : Nguyen Ngoc Van                          */
/* Memory  : Small                                    */
/* Address : pherosiden@gmail.com                     */
/* Website : http://www.codedemo.net                  */
/* Created : 26/04/1998                               */
/* Please sent to me any bugs or suggests.            */
/* You can use freely this code. Have fun :)          */
/*----------------------------------------------------*/

#include <dos.h>
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <conio.h>

#define M_PI 3.141592f

char *chars[64] = {0};
uint8_t pal[768] = {0};
uint8_t chrinfo[64][3] = {0};
uint8_t bitmap[30][320] = {0};
uint8_t sintab[256] = {0};
uint8_t row, newrow, index;
uint8_t *vmem = (uint8_t*)0xA0000000L;
const char *text = "ILOVE...DEMO!WATCOMC!GRAPHIC!BITMAP!SUPER VESA!";

inline uint8_t roundf(float x)
{
    if (x >= 0.0) return x + 0.5;
    return x - 0.5;
}

void calcSin()
{
    int16_t i;
    for (i = 0; i < 256; i++) sintab[i] = roundf(sin(15 * i * M_PI / 755) * 24 + 70);
}

void loadFont(char *fname)
{
    FILE *fp;
    char chr;
    uint8_t width, height;
    
    fp = fopen(fname, "rb");
    if (!fp) return;

    fread(pal, 1, 768, fp);

    while (!feof(fp))
    {
        fread(&chr, 1, 1, fp);
        fread(&width, 1, 1, fp);
        fread(&height, 1, 1, fp);

        chars[chr - 32] = (char*)calloc(width * height, 1);
        if (!chars[chr - 32]) return;

        chrinfo[chr - 32][0] = width;
        chrinfo[chr - 32][1] = height;
        chrinfo[chr - 32][2] = 1;
        fread(chars[chr - 32], 1, width * height, fp);
    }

    fclose(fp);

    __asm {
        mov     dx, 0x03C8
        xor     ax, ax
        out     dx, al
        inc     dx
        lea     si, pal
        mov     cx, 768
        rep     outsb
    }
}

void waitRetrace()
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

void scrollBitmap(uint8_t *map)
{
    __asm {
        lds     si, map
        les     di, map
        add     si, 2
        mov     cx, 2400
        rep     movsd
    }
}

void drawBitmap(uint8_t *mem, uint16_t ofs)
{
    __asm {
        lds     si, mem
        les     di, vmem
        add     di, ofs
        mov     cx, 2400
        rep     movsd
        pop     ds
    }
}

void deAllocMem()
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

void newRows(char chr, uint8_t row, int16_t pos)
{
    int16_t width, height, i;
    if (chrinfo[chr - 32][2] != 1) return;

    i = (30 - chrinfo[chr - 32][1]) >> 1;

    for (height = 0; height < 30; height++) bitmap[height][pos] = 0;

    if (row == chrinfo[chr - 32][0] + 1)
    {
        for (height = 0; height < chrinfo[chr - 32][1]; height++) bitmap[height + i][pos - 1] = 0;
    }
    else
    {
        for (height = 0; height < chrinfo[chr - 32][1]; height++) bitmap[height + i][pos] = *(chars[chr - 32] + height * chrinfo[chr - 32][0] + row);
    }
}

void update()
{
    row++;

    if (row >= chrinfo[text[newrow] - 32][0])
    {
        row = 0;
        newrow++;
        if (newrow >= strlen(text)) newrow = 0;
    }
}

void main()
{
    memset(bitmap[0], 0, sizeof(bitmap));

    calcSin();

    __asm {
        mov     ax, 0x13
        int     0x10
    }

    loadFont("assets/font002.fnt");

    row = 0;
    newrow = 0;
    index = 0;

    do {
        scrollBitmap(bitmap[0]);
        update();
        newRows(text[newrow], row, 318);
        update();
        newRows(text[newrow], row, 319);
        waitRetrace();
        drawBitmap(bitmap[0], sintab[index] * 320);
        index = (index + 2) % 100;
    } while(!kbhit());

    deAllocMem();

    __asm {
        mov     ax, 0x03
        int     0x10
    }
}
