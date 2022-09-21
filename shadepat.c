/*----------------------------------------------------*/
/* Packet  : Demo & Effect                            */
/* Effect  : Shade Bob                                */
/* Author  : Nguyen Ngoc Van                          */
/* Memory  : Compact                                  */
/* Heaps   : 640K                                     */
/* Address : pherosiden@gmail.com                     */
/* Website : http://www.codedemo.net                  */
/* Created : 02/03/1998                               */
/* Please sent to me any bugs or suggests.            */
/* You can use freely this code. Have fun :)          */
/*----------------------------------------------------*/

#include <dos.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <conio.h>

typedef struct {
    uint16_t x, y;
} Point;

Point points[16000] = {0};

uint8_t vbuff[200][320] = {0};
uint8_t *vmem = (uint8_t *)0xA0000000L;
uint8_t *tmem = (uint8_t *)0xB8000000L;

uint8_t brush[30][30] = {0};
uint16_t arrpos, arrmax;
uint16_t newx, newy, oldx, oldy;

void setRGB(uint8_t n, uint8_t r, uint8_t g, uint8_t b)
{
    __asm {
        mov     dx, 0x03C8
        mov     al, n
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

uint8_t getPixel(uint16_t x, uint16_t y, uint8_t *mem)
{
    __asm {
        lds     si, mem
        mov     bx, y
        shl     bx, 6
        add     bh, byte ptr y
        add     bx, x
        add     si, bx
        lodsb
    }
}

void setPixel(uint16_t x, uint16_t y, uint8_t c, uint8_t *mem)
{
    __asm {
        les     di, mem
        mov     bx, y
        shl     bx, 6
        add     bh, byte ptr y
        add     bx, x
        add     di, bx
        mov     al, c
        stosb
    }
}

void clearMem(uint8_t *mem)
{
    __asm {
        les     di, mem
        xor     eax, eax
        mov     cx, 16000
        rep     stosd
    }
}

int16_t initMouse()
{
    __asm {
        xor     ax, ax
        int     0x33
        xor     al, al
    }
}

void getMousePos(uint16_t *x, uint16_t *y)
{
    __asm {
        mov     ax, 0x03
        int     0x33
        les     di, x
        mov     es:[di], cx
        les     di, y
        mov     es:[di], dx
    }
}

void clearScreen(uint8_t *mem)
{
    __asm {
        les     di, mem
        xor     ax, ax
        mov     cx, 2000
        rep     stosw
    }
}

void printStr(int16_t x, int16_t y, uint8_t col, char *msg, uint8_t *mem)
{
    uint16_t len = strlen(msg);

    __asm {
        mov     cx, len
        test    cx, cx
        jz      quit
        lds     si, msg
        les     di, mem
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

void loadImage()
{
    FILE *fp = fopen("assets/killer.cel", "rb");
    if (!fp) return;

    fseek(fp, 800, SEEK_SET);
    fread(vbuff[0], 1, 64000, fp);
    fclose(fp);
}

void initBrush()
{
    int16_t a, b, c;

    for (c = 0; c < 30; c++)
        for (b = c; b < 30 - c; b++)
            for (a = c; a < 30 - c; a++) brush[a][b] = c << 2;
}

void generatePal()
{
    int16_t i;

    for (i = 0; i < 64; i++)
    {
        setRGB(i, i, 0, 0);
        setRGB(i + 64, 63, i, 0);
        setRGB(i + 128, 63, 63, 0);
        setRGB(i + 192, 63, 63, i);
    }
}

void drawShadeBob(uint16_t xp, uint16_t yp)
{
    int16_t a, b, c;

    for (b = 0; b < 30; b++)
    {
        for (a = 0; a < 30; a++)
        {
            c = getPixel(xp + a - 10, yp + b - 10, vbuff[0]) + brush[a][b] + (getPixel(xp + a - 10, yp + b - 10, vmem) >> 2);
            if (c > 255) c = 255;
            setPixel(xp + a - 10, yp + b - 10, c, vmem);
        }
    }
}

void initParameter()
{
    initBrush();
    initMouse();
    loadImage();
    generatePal();
    arrpos = 0;
}

void editShadeBob()
{
    FILE *fp;

    getMousePos(&oldx, &oldy);

    while (!kbhit())
    {
        getMousePos(&newx, &newy);
        
        if (newx != oldx || newy != oldy)
        {
            points[arrpos].x = newx;
            points[arrpos].y = newy;
            drawShadeBob(newx, newy);
            oldx = newx;
            oldy = newy;
            arrpos++;
            if (arrpos >= 16000) break;
        }
    }

    fp = fopen("assets/path.dat", "wb");
    fwrite(&arrpos, sizeof(arrpos), 1, fp);
    fwrite(points, sizeof(Point), arrpos, fp);
    fclose(fp);
}

void playShadeBob()
{
    FILE *fp = fopen("assets/path.dat", "rb");

    if (!fp) return;

    fread(&arrmax, sizeof(arrmax), 1, fp);
    if (arrmax >= 16000)
    {
        fclose(fp);
        return;
    }

    fread(points, sizeof(Point), arrmax, fp);
    fclose(fp);

    while (!kbhit())
     {
        drawShadeBob(points[arrpos].x, points[arrpos].y);
        arrpos++;
        if (arrpos >= arrmax) arrpos = 0;
    }
}

void main(int argc)
{
    clearScreen(tmem);
    printStr(1, 1, 0x0F, "SHADE PATTERN - (c) 1998 by Nguyen Ngoc Van", tmem);
    printStr(1, 2, 0x07, "Use parameter path.dat to generate data file.", tmem);
    printStr(1, 3, 0x07, "Press any key to start demo...", tmem);
    getch();

    __asm {
        mov     ax, 0x13
        int     0x10
    }
   
    initParameter();

    if (argc != 1) editShadeBob();
    else playShadeBob();

    __asm {
        mov     ax, 0x03
        int     0x10
    }

    free(vbuff);
}
