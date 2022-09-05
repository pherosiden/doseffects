/*----------------------------------------------------*/
/* Packet  : Demo & Effect                            */
/* Effect  : Bump Mapping                             */
/* Author  : Nguyen Ngoc Van                          */
/* Memory  : Compact                                  */
/* Heaps   : 640K                                     */
/* Address : siden@codedemo.net                       */
/* Website : http://www.codedemo.net                  */
/* Created : 12/01/1998                               */
/* Please sent to me any bugs or suggests.            */
/* You can use freely this code. Have fun :)          */
/*----------------------------------------------------*/

#include <dos.h>
#include <mem.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <conio.h>

uint8_t vbuff1[200][320] = {0};
uint8_t vbuff2[200][320] = {0};
uint8_t dbuff[256][256] = {0};

int16_t mouseDetect()
{
    __asm {
        xor     ax, ax
        int     0x33
        xor     al, al
    }
}

int16_t getMouseX()
{
    __asm {
        mov     ax, 0x03
        int     0x33
        mov     ax, cx
    }
}

int16_t getMouseY()
{
    __asm {
        mov     ax, 0x03
        int     0x33
        mov     ax, dx
    }
}

int16_t leftPressed()
{
    __asm {
        mov     ax, 0x03
        int     0x33
        mov     ax, bx
    }
}

void moveMouse(uint16_t x, uint16_t y)
{
    __asm {
        mov     ax, 0x04
        mov     cx, x
        mov     dx, y
        int     0x33
    }
}

void flip()
{
    __asm {
        mov     ax, 0xA000
        mov     es, ax
        xor     di, di
        mov     ax, seg vbuff1
        mov     ds, ax
        xor     si, si
        mov     cx, 16000
        rep     movsd
    }
}

void waitRetrace()
{
    __asm {
        mov     dx, 0x03DA
    waith:
        in      al, dx
        and     al, 0x08
        jnz     waith
    waitv:
        in      al, dx
        and     al, 0x08
        jz      waitv
    }
}

void clearTextMem()
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

void clearBuff(void *buff)
{
    __asm {
        les     di, buff
        xor     eax, eax
        mov     cx, 16000
        rep     stosd
    }
}

void printStr(int16_t x, int16_t y, uint8_t col, const char *msg)
{
    uint16_t len = strlen(msg);

    __asm {
        mov     cx, len
        test    cx, cx
        jz      quit
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
    next:
        lodsb
        stosw
        loop    next
    quit:
    }
}

void createEnvirMap()
{
    int16_t y, x;
    float nx, ny, nz;

    for (y = 0; y < 256; y++)
    {
        for (x = 0; x < 256; x++)
        {
            nx = (x - 128) / 128.0;
            ny = (y - 128) / 128.0;
            nz = 1 - sqrt(nx * nx + ny * ny);
            if (nz < 0) nz = 0;
            dbuff[y][x] = nz * 128;
        }
    }
}

void setFadePalette(uint8_t r1, uint8_t g1, uint8_t b1, uint8_t r2, uint8_t g2, uint8_t b2, uint8_t start, uint8_t end)
{
    int16_t rval = r1 << 8;
    int16_t gval = g1 << 8;
    int16_t bval = b1 << 8;

    int16_t rstep = ((r2 - r1 + 1) << 8) / (end - start + 1);
    int16_t gstep = ((g2 - g1 + 1) << 8) / (end - start + 1);
    int16_t bstep = ((b2 - b1 + 1) << 8) / (end - start + 1);

    __asm {
        mov     dx, 0x03C8
        mov     al, start
        out     dx, al
        xor     cx, cx
        mov     cl, end
        sub     cl, start
        inc     cx
        inc     dx
    next:
        mov     ax, rval
        shr     ax, 8
        out     dx, al
        mov     ax, gval
        shr     ax, 8
        out     dx, al
        mov     ax, bval
        shr     ax, 8
        out     dx, al
        mov     bx, rstep
        add     rval, bx
        mov     bx, gstep
        add     gval, bx
        mov     bx, bstep
        add     bval, bx
        loop    next
    }
}

void bump(int16_t lx, int16_t ly)
{
    int16_t x, y, nx, ny;

    for (y = 1; y < 199; y++)
    {
        for (x = 0; x < 320; x++)
        {
            nx = vbuff2[y][x - 1] - vbuff2[y][x + 1];
            ny = vbuff2[y - 1][x] - vbuff2[y + 1][x];
            nx = nx - x + lx + 128;
            ny = ny - y + ly + 128;
            if (nx < 0 || nx > 255) nx = 0;
            if (ny < 0 || ny > 255) ny = 0;
            vbuff1[y][x] = dbuff[ny][nx] + 120;
        }
    }
}

void main()
{
    FILE *fp;

    clearTextMem();
    printStr(1, 1, 0x0F, "Bump Mapping - (c) 1998 by Nguyen Ngoc Van");
    printStr(1, 2, 0x07, "Move the mouse      : flash effect");
    printStr(1, 3, 0x07, "Any key/Left button : goodbye!!!");
    printStr(1, 4, 0x07, "Press any key to start demo");
    getch();

    if (!mouseDetect()) return;
    createEnvirMap();

    __asm {
        mov     ax, 0x13
        int     0x10
        mov     cx, 4
        mov     dx, 8
        mov     ax, 0x0F
        int     0x33
    }

    moveMouse(160, 100);

    fp = fopen("assets/intro.cel", "rb");
    if (!fp) return;

    fseek(fp, 800, SEEK_SET);
    fread(vbuff2, 1, 64000, fp);
    fclose(fp);

    setFadePalette(0, 0, 0, 0, 0, 20, 0, 127);
    setFadePalette(0, 0, 20, 63, 63, 63, 128, 255);

    do {
        bump(getMouseX(), getMouseY());
        waitRetrace();
        flip();
    } while (!kbhit() && leftPressed() != 1);
    
    __asm {
        xor     ax, ax
        int     0x33
        mov     ax, 0x03
        int     0x10
    }
}
