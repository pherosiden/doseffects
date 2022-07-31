/*----------------------------------------------------*/
/* Packet  : Demo & Effect                            */
/* Effect  : Len                                      */
/* Author  : Nguyen Ngoc Van                          */ 
/* Memory  : Compact                                  */
/* Heaps   : 640K                                     */
/* Address : pherosiden@gmail.com                     */
/* Website : http://www.codedemo.net                  */
/* Created : 11/02/1998                               */
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

#define M_PI 	3.141592f
#define M_PI_2 	(M_PI / 2)

int16_t matrix[64][64] = {0};
int16_t xtab[320] = {0};
int16_t ztab[64] = {0};
int16_t xpostab[256] = {0};
int16_t ypostab[256] = {0};

int16_t x, y, oldx, oldy, stackseg;
uint16_t xpos, ypos, xadd, yadd;

uint8_t pal[768] = {0};
uint8_t vbuff[64000] = {0};

void retrace()
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

void flip(uint16_t fseg)
{
    __asm {
        mov     ax, seg vbuff
        mov     es, ax
        xor     di, di
        mov     ds, fseg
        xor     si, si
        mov     cx, 16000
        rep     movsd
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
        mov     ax, 0xB800
        mov     es, ax
        xor     di, di
        lds     si, msg
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

inline int16_t roundf(float x)
{
    if (x >= 0.0) return x + 0.5;
    return x - 0.5;
}

void calcMatrix()
{
    int16_t x, y;
    int32_t tx, ty, tz;
    
    for (y = -32; y < 32; y++)
    {
        for (x = -32; x < 32; x++)
        {
            tz = sqrt(x * x * 4 + y * y * 4);
            tz = ztab[tz / 2];
            tx = (x * tz) / 2300;
            ty = (y * tz) / 2300;
            matrix[y + 32][x + 32] = ty * 320 + tx;
        }
    }
}

void preCalc()
{
    int16_t i;
    float v, vadd;
    FILE *fp;

    for (i = 0; i < 320; i++) xtab[i] = (i - 160) * (i - 160);

    v = 0.0;
    vadd = 2 * M_PI / 256;

    for (i = 0; i < 256; i++)
    {
        xpostab[i] = roundf(sin(v) * 110 + 160);
        ypostab[i] = roundf(sin(v) * 50 + 100);
        v += vadd;
    }

    v = M_PI_2;
    vadd = M_PI_2 / 64;

    for (i = 0; i < 64; i++) ztab[i] = roundf(sin(v + i * vadd) * 2500);
    calcMatrix();

    if (!(fp = fopen("assets/drunken.cel", "rb"))) return;
    fseek(fp, 32, SEEK_SET);
    fread(pal, 768, 1, fp);
    fread(MK_FP(0xA000, 0), 64000, 1, fp);
    fclose(fp);

    __asm {
        mov     dx, 0x03C8
        xor     al, al
        out     dx, al
        inc     dx
        mov     cx, 768
        lea     si, pal
        rep     outsb
    }

    flip(0xA000);
}

void copyFromBuffer(int16_t x, int16_t y)
{
    __asm {
        mov     ax, y
        mov     dx, 320
        mul     dx
        add     ax, x
        mov     di, ax
        mov     si, ax
        mov     ax, seg vbuff
        mov     ds, ax
        mov     ax, 0xA000
        mov     es, ax
        sub     si, 10272
        sub     di, 10272
        mov     dx, 64
    copy:
        mov     cx, 16
        rep     movsd
        add     si, 256
        add     di, 256
        dec     dx
        jnz     copy
    }
}

void printGlass(int16_t x, int16_t y)
{
    __asm {
        mov     ax, y
        mov     dx, 320
        mul     dx
        add     ax, x
        mov     dx, ax
        mov     di, ax
        sub     di, 20544
        mov     ax, 0xA000
        mov     es, ax
        mov     ax, seg matrix
        mov     ds, ax
        mov     si, offset matrix
        mov     ax, seg vbuff
        mov     fs, ax
        mov     ah, 64
    lp1:
        mov     cx, 64
    lp2:
        mov     bx, [si]
        add     bx, dx
        mov     al, fs:[di]
        mov     es:[bx], al
        add     di, 2
        add     si, 2
        loop    lp2
        add     di, 512
        dec     ah
        jnz     lp1
    }
}

void main()
{
    clearScreen();
    printStr(1, 1, 0x0F, "LEN ZOOM - (c) 1998 by Nguyen Ngoc Van");
    printStr(1, 2, 0x07, "Press any key to start demo...");
    getch();

    __asm {
        mov     ax, 0x13
        int     0x10
    }

    preCalc();

    xpos = 40;
    ypos = 60;

    xadd = 2;
    yadd = 1;

    do {
        retrace();
        copyFromBuffer(oldx, oldy);
        x = xpostab[xpos & 0xFF];
        y = ypostab[ypos & 0xFF];
        printGlass(x, y);
        oldx = x;
        oldy = y;
        xpos += xadd;
        ypos += yadd;
    } while(!kbhit());

    __asm {
        mov     ax, 0x03
        int     0x10
    }
}
