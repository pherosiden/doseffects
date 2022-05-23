/*----------------------------------------------------*/
/* Packet  : Demo & Effect                            */  
/* Effect  : Voxel                                    */
/* Author  : Nguyen Ngoc Van                          */ 
/* Memory  : Compact                                  */ 
/* Heaps   : 640K                                     */ 
/* Address : pherosiden@gmail.com                     */
/* Website : http://www.codedemo.net                  */
/* Created : 10/02/1998                               */
/* Please sent to me any bugs or suggests.            */ 
/* You can use freely this code. Have fun :)          */
/*----------------------------------------------------*/

#include <dos.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <conio.h>

#define M_PI 3.141592f

uint8_t height = 0;
uint8_t tmap[256][256] = {0};
uint8_t hmap[256][256] = {0};
uint8_t pal[768] = {0};

int16_t x, y, angle, dst;
int16_t costab[2048] = {0};
int16_t sintab[2048] = {0};
int16_t dcomp[127] = {0};

char kbarr[128] = {0};

uint8_t sky[64000] = {0};
uint8_t vbuff[64000] = {0};

uint8_t *vmem = (uint8_t*)0xA0000000L;
uint8_t *tmem = (uint8_t*)0xB8000000L;

void (interrupt *old09)();

void interrupt new09()
{
    uint8_t chr = inp(0x60);
    if (chr > 128) kbarr[chr - 128] = 0;
    else kbarr[chr] = 1;

    __asm {
        mov     al, 0x20
        out     0x20, al
    }
}

void flip(uint8_t *src, uint8_t *dst)
{
    __asm {
        les     di, dst
        lds     si, src
        mov     cx, 16000
        rep     movsd
    }
}

void clearBuffer(uint8_t *mem)
{
    __asm {
        les     di, mem
        xor     ax, ax
        mov     cx, 16000
        rep     stosd
    }
}

void verticalLine(uint16_t x1, uint16_t y, int16_t len, uint8_t col)
{
    if (len <= 0) return;
    
    __asm {
        mov     ax, seg vbuff
        mov     es, ax
        mov     bx, y
        mov     cx, bx
        shl     bx, 8
        shl     cx, 6
        add     bx, cx
        add     bx, x1
        mov     di, bx
        mov     cx, len
        mov     al, col
    next:
        stosb
        add     di, 319
        loop    next
    }
}

void ray(uint16_t a, uint16_t dx, uint16_t dy, uint16_t sx)
{
    int16_t miny;
    int32_t h, y;
    uint16_t delx, dely, p, dt;

    delx = costab[a];
    dely = sintab[a];

    dt = 0;
    miny = 200;
    
    do {
        dx += delx;
        dy += dely;
        dt++;

        h = hmap[dy >> 8][dx >> 8] - height;
        y = dcomp[dt - 1] - (h << 5) / dt + dst;
    
        if (y < miny && y >= 0)
        {
            verticalLine(sx, y, miny - y, tmap[dy >> 8][dx >> 8]);
            miny = y;
        }
    } while (dt < 127);
}

void drawView()
{
    int16_t a;
    int16_t i;

    for (i = 0; i < 320; i++)
    {
        a = (angle + i + 1888) & 2047;
        ray(a, x, y, i);
    }
}

void initMap()
{
    FILE *fp;

    fp = fopen("ground.cel", "rb");
    if (!fp) exit(1);

    fseek(fp, 32, SEEK_SET);
    fread(pal, 1, 768, fp);
    fread(tmap, 1, 65535, fp);
    fclose(fp);

    fp = fopen("height.cel", "rb");
    if (!fp) exit(1);

    fseek(fp, 800, SEEK_SET);
    fread(hmap, 1, 65535, fp);
    fclose(fp);

    fp = fopen("sky.cel", "rb");
    if (!fp) exit(1);

    fseek(fp, 800, SEEK_SET);
    fread(sky, 1, 64000, fp);
    fclose(fp);
}

inline int16_t roundf(float x)
{
    if (x >= 0.0) return x + 0.5;
    return x - 0.5;
}

void initTables()
{
    int16_t i;
    for (i = 0; i < 2048; i++)
    {
        costab[i] = roundf(cos(i * M_PI / 1024) * 256);
        sintab[i] = roundf(sin(i * M_PI / 1024) * 256);
    }

    for (i = 1; i <= 127; i++) dcomp[i - 1] = 2000 / i + 100;
}

void printStr(int16_t x, int16_t y, uint8_t col, char *msg)
{
    int16_t len = strlen(msg);

    __asm {
        les     di, tmem
        lds     si, msg
        add     di, x
        shl     di, 1
        mov     ax, y
        shl     ax, 5
        add     di, ax
        shl     ax, 2
        add     di, ax
        mov     ah, col
        mov     cx, len
    next:
        lodsb
        stosw
        loop    next
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

void main()
{
    int16_t i;

    clearScreen();
    printStr(1, 1, 0x0F, "VOXEL - (c) 1998 by Nguyen Ngoc Van");
    printStr(1, 2, 0x07, "Use arrow, A, Z to control - ESC to quit.");
    printStr(1, 3, 0x07, "Press any key to start...");
    getch();

    old09 = _dos_getvect(0x09);
    _dos_setvect(0x09, new09);

    initTables();

    x = 32767;
    y = 32767;

    dst = 0;
    angle = 600;
    
    initMap();

    __asm {
        mov     ax, 0x13
        int     0x10
    }

    outp(0x03C8, 0);
    for (i = 0; i < 768; i++) outp(0x3C9, pal[i]);

    do {
        height = hmap[y >> 8][x >> 8];
        flip(sky, vbuff);
        drawView();
        flip(vbuff, vmem);

        if (kbarr[72])
        {
            x += costab[angle] << 1;
            y += sintab[angle] << 1;
        }

        if (kbarr[80])
        {
            x -= costab[angle] << 1;
            y -= 2 * sintab[angle] << 1;
        }

        if (kbarr[75]) angle = (angle + 4064) & 2047;
        if (kbarr[77]) angle = (angle + 32) & 2047;
        if (kbarr[30]) dst = 80;
        else if (kbarr[44]) dst = -100;
        else dst = 0;
    } while(!kbarr[1]);

    __asm {
        mov     ax, 0x03
        int     0x10
    }

    _dos_setvect(0x09, old09);
}
