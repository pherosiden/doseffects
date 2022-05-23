/*---------------------------------------------------*/
/* Packet  : Demo & Effect                           */ 
/* Effect  : String text                             */
/* Author  : Nguyen Ngoc Van                         */
/* Memory  : Small                                   */
/* Address : pherosiden@gmail.com                    */
/* Website : http://www.codedemo.net                 */ 
/* Created : 24/04/1998                              */
/* Please sent to me any bugs or suggests.           */ 
/* You can use freely this code. Have fun :)         */
/*---------------------------------------------------*/

#include <dos.h>
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <conio.h>

#define M_PI 3.141592f

uint16_t cpos[320] = {0};
uint8_t *chars[64] = {0};
uint8_t pal[768] = {0};
uint8_t chrinfo[64][3] = {0};
uint8_t bitmap[60][320] = {0};
uint8_t sint1[230] = {0};
uint8_t sint2[115] = {0};
uint8_t row, newrow, index;

uint8_t *vmem = (uint8_t*)0xA0000000L;
const char *text = "ILOVE...DEMO!WATCOMC!GRAPHICS!BITMAP!SUPER VESA!";

inline uint8_t roundf(float x)
{
    if (x >= 0.0) return x + 0.5;
    return x - 0.5;
}

void calcPos()
{
    int16_t x;
    for (x = 0; x < 320; x++) cpos[x] = 320 * (sint1[(x + index) % 230] + sint2[x % 115]) + x;
}

void calcSin()
{
    int16_t i;
    for (i = 0; i < 230; i++) sint1[i] = roundf(sin(2 * i * M_PI / 230) * 20 + 20);
    for (i = 0; i < 115; i++) sint2[i] = roundf(sin(2 * i * M_PI / 115) * 3 + 3);
}

void loadFont(char *fname)
{
    FILE *fp;
    char chr;

    uint8_t w, h;
    
    fp = fopen(fname, "rb");
    if (!fp) exit(1);

    fread(pal, 1, 768, fp);

    while (!feof(fp))
    {
        fread(&chr, 1, 1, fp);
        fread(&w, 1, 1, fp);
        fread(&h, 1, 1, fp);

        chars[chr - 32] = (uint8_t*)calloc(w * h, 1);
        if (!chars[chr - 32]) exit(1);

        chrinfo[chr - 32][0] = w;
        chrinfo[chr - 32][1] = h;
        chrinfo[chr - 32][2] = 1;
        fread(chars[chr - 32], 1, w * h, fp);
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
        mov     cx, 4800
        rep     movsd
    }
}

void drawBitmap(uint8_t *map)
{
    int16_t i, j;
    uint16_t x, y;

    for (i = 0; i < 320; i++)
    {
        y = 0;
        x = cpos[i];
        
        for (j = 0; j < 60; j++)
        {
            vmem[x] = map[y + i];
            x += 320;
            y += 320;
        }
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

void newRows(char chr, uint8_t row, int pos)
{
    int16_t x, y;

    if (chrinfo[chr - 32][2] != 1) return;

    for (y = 0; y < 60; y++) bitmap[y][pos] = 0;

    x = (60 - chrinfo[chr - 32][1]) >> 1;

    if (row == chrinfo[chr - 32][0] + 1)
    {
        for (y = 0; y < chrinfo[chr - 32][1]; y++) bitmap[y + x][pos - 1] = 0;
    }
    else
    {
        for (y = 0; y < chrinfo[chr - 32][1]; y++) bitmap[y + x][pos] = *(chars[chr - 32] + y * chrinfo[chr - 32][0] + row);
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
    memset(bitmap[0], 0, sizeof(bitmap[0]));

    calcSin();

    __asm {
        mov     ax, 0x13
        int     0x10
    }

    loadFont("font003.fnt");
    
    row = 0;
    newrow = 0;
    index = 0;

    do {
        scrollBitmap(bitmap[0]);
        update();
        newRows(text[newrow], row, 318);
        update();
        newRows(text[newrow], row, 319);
        calcPos();
        waitRetrace();
        drawBitmap(bitmap[0]);
        index = (index + 4) % 230;
    } while(!kbhit());

    deAllocMem();

    __asm {
        mov     ax, 0x03
        int     0x10
    }
}
