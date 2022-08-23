/*----------------------------------------------------*/
/* Packet  : Demo & Effect                            */
/* Effect  : Rotate Bitmap                            */ 
/* Author  : Nguyen Ngoc Van                          */
/* Memory  : Compact                                  */
/* Heaps   : 640K                                     */ 
/* Address : pherosiden@gmail.com                     */ 
/* Website : http://www.codedemo.net                  */
/* Created : 24/02/1998                               */
/* Please sent to me any bugs or suggests.            */
/* You can use freely this code. Have fun :)          */
/*----------------------------------------------------*/

#include <dos.h>
#include <math.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <conio.h>

#define XC 160
#define YC 100
#define M_PI 3.141592f
#define M_PI_2 (M_PI / 2)

int16_t sintab[256] = {0}, costab[256] = {0};
int16_t sin2tab[256] = {0}, cos2tab[256] = {0};

uint16_t rotate, dir;
uint16_t x, y, dist, inc;

uint8_t texture[256][256] = {0};
uint8_t *vmem = (uint8_t*)0xA0000000L;

inline int16_t random(int16_t x)
{
    if (x == 0) return 0;
    return rand() % x;
}

inline int16_t roundf(float x)
{
    if (x >= 0.0) return x + 0.5;
    return x - 0.5;
}

void makeTables()
{
    int16_t i;
    float angle;

    for (i = 0; i < 256; i++)
    {
        angle = i;
        angle = angle * M_PI / 128;
        sintab[i] = roundf(sin(angle) * 256);
        costab[i] = roundf(cos(angle) * 256);
        sin2tab[i] = roundf(sin(angle + M_PI_2) * 256 * 1.2);
        cos2tab[i] = roundf(cos(angle + M_PI_2) * 256 * 1.2);
    }
}

void drawScreen(uint16_t x, uint16_t y, long scale, uint8_t rotate, uint16_t where)
{
    int16_t d1x, d1y;
    int16_t d2x, d2y;
    uint16_t i, j, w, h, ax, dx;

    d1x = costab[rotate] * scale >> 8;
    d1y = sintab[rotate] * scale >> 8;

    d2x = cos2tab[rotate] * scale >> 8;
    d2y = sin2tab[rotate] * scale >> 8;

    i = x - d1x * XC - d2x * YC;
    j = y - d1y * XC - d2y * YC;

    for (h = 0; h < 200; h++)
    {
        ax = i;
        dx = j;
        for (w = 0; w < 320; w++)
        {
            ax += d1x;
            dx += d1y;
            vmem[(h << 8) + (h << 6) + w] = texture[dx >> 8][ax >> 8];
        }
        ax = d2x;
        dx = d2y;
        i += ax;
        j += dx;
    }
}

void loadImage()
{
    FILE *fp;
    int16_t i;
    uint8_t pal[768] = {0};

    fp = fopen("assets/crew.cel", "rb");
    if (!fp) exit(1);

    fseek(fp, 32, SEEK_SET);
    fread(pal, 1, 768, fp);
    fread(texture[0], 1, 65535, fp);
    fclose(fp);

    __asm {
        push    ds
        mov     dx, 0x03C8
        xor     ax, ax
        out     dx, al
        inc     dx
        mov     cx, 768
        lea     si, pal
        rep     outsb
        pop     ds
    }
}

uint16_t allocDOSMem(uint16_t pages)
{
    __asm {
        mov     ah, 0x48
        mov     bx, pages
        int     0x21
    }
}

void freeDOSMem(uint16_t segment)
{
    __asm {
        mov     ah, 0x49
        mov     es, segment
        int     0x21
    }
}

void retrace()
{
    __asm {
        mov     dx, 0x03DA
    waitV:
        in      al, dx
        and     al, 0x08
        jnz     waitV
    waitH:
        in      al, dx
        and     al, 0x08
        jz      waitH
    }
}

void clearMem(uint8_t *mem)
{
    __asm {
        les     di, mem
        xor     eax, eax
        mov     cx, 16384
        rep     stosd
    }
}

void clearScreen()
{
    __asm {
        mov     ax, 0xB800
        mov     es, ax
        xor     di, di
        xor     ax, ax
        mov     cx, 2000
        rep     stosw
    }
}

void printStr(int16_t x, int16_t y, uint8_t col, char *msg)
{
    int16_t len = strlen(msg);

    __asm {
        lds     si, msg
        mov     ax, 0xB800
        mov     es, ax
        xor     di, di
        add     di, x
        shl     di, 1
        mov     bx, y
        shl     bx, 5
        add     di, bx
        shl     bx, 2
        add     di, bx
        mov     ah, col
        mov     cx, len
    next:
        lodsb
        stosw
        loop    next
    }
}

void main()
{
    clearScreen();
    printStr(1, 1, 0x0F, "ROTATE BITMAP - (c) 1998 by Nguyen Ngoc Van");
    printStr(1, 2, 0x07, "Press any key to start demo...");
    getch();

    srand(time(NULL));
    makeTables();

    __asm {
        mov     ax, 0x13
        int     0x10
    }

    loadImage();

    x = 32767;
    y = 0;
    rotate = 0;
    dir = 1;
    dist = 800;
    inc = 65535;

    do {
        retrace();
        drawScreen(x, y, dist, rotate & 0x00FF, 0xA000);
    
        rotate += dir;
        y += 128;
        dist += inc;

        if (dist == 2000 || dist == 2) inc = -inc;
        if (random(150) == 1) dir = random(7) - 3;

    } while(!kbhit());

    __asm {
        mov     ax, 0x03
        int     0x10
    }
}
