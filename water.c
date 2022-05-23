/*----------------------------------------------------*/
/* Packet  : Demo & Effect                            */
/* Effect  : Water2                                   */
/* Author  : Nguyen Ngoc Van                          */
/* Memory  : Compact                                  */
/* Heaps   : 640K                                     */
/* Address : pherosiden@gmail.com                     */
/* Website : http://www.codedemo.net                  */
/* Created : 20/10/2000                               */
/* Please sent to me any bugs or suggests.            */
/* You can use freely this code. Have fun :)          */
/*----------------------------------------------------*/

#include <dos.h>
#include <mem.h>
#include <math.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <conio.h>

#define WIDTH       320
#define HEIGHT      200
#define DENSITY     4
#define GetTicks()  *((uint32_t*)0x046C)

uint16_t sintab[256] = {0};
uint16_t costab[256] = {0};

uint8_t vbuff[200][320] = {0};
uint8_t dbuff[200][320] = {0};
int16_t water[2][200][320] = {0};

uint8_t *vmem = (uint8_t*)0xA0000000L;
uint8_t *tmem = (uint8_t*)0xB8000000L;

void loadCEL(char *file)
{
    uint8_t pal[768] = {0};

    FILE *fp = fopen(file, "rb");
    if (!fp) return;

    fseek(fp, 32, SEEK_SET);
    fread(pal, 1, 768, fp);
    fread(dbuff[0], 1, 64000, fp);
    fclose(fp);

    __asm {
        mov    dx, 0x03C8
        xor    al, al
        out    dx, al
        inc    dx
        lea    si, pal
        mov    cx, 768
        rep    outsb
    }
}

void waitRetrace()
{
    __asm {
        mov     dx, 0x03DA
    hwait:
        in      al, dx
        test    al, 0x08
        jnz     hwait
    vwait:
        in      al, dx
        test    al, 0x08
        jz      vwait
    }
}

void flipScreen(uint8_t *src, uint8_t *dst)
{
    __asm {
        lds    si, src
        les    di, dst
        mov    cx, 16000
        rep    movsd
    }
}

void initSinCos()
{
    int16_t i;

    for (i = 0; i < 256; i++)
    {
        sintab[i] = sin(i / 20.0) * 80 + 100;
        costab[i] = cos(i / 40.0) * 110 + 160;
    }
}

void showWater(uint8_t page)
{
    /*--------------------------------------------------------------*/
    /* C version: this is slower than asm version below             */
    /*--------------------------------------------------------------*/

    /*int16_t nw, x, y, dx, dy, sx, sy;
    int16_t cpage = page;
    int16_t spage = page ^ 1;

    for (y = 1; y <= HEIGHT - 2; y++)
    {
        for (x = 1; x <= WIDTH - 2; x++)
        {
            nw = ((
                water[cpage][y    ][x - 1] +
                water[cpage][y    ][x + 1] +
                water[cpage][y - 1][x    ] +
                water[cpage][y - 1][x - 1] +
                water[cpage][y - 1][x + 1] +
                water[cpage][y + 1][x    ] +
                water[cpage][y + 1][x - 1] +
                water[cpage][y + 1][x + 1] ) >> 3) - water[spage][y][x];
            water[spage][y][x] = nw - (nw >> DENSITY);
            dx = water[spage][y][x] - water[spage][y + 1][x];
            dy = water[spage][y][x] - water[spage][y][x + 1];
            sx = x + (dx >> 3);
            sy = y + (dy >> 3);
            vbuff[y][x] = dbuff[sy][sx];
        }
    }*/

    /*----------------------------------------------------------------*/
    /* Optimized version: very fast!!!                                */
    /*----------------------------------------------------------------*/
    
    uint32_t cpage, spage;
    
    __asm {
        pusha
        mov     al, page
        and     al, 0x01
        jz      setp1
        mov     cpage, 128000
        mov     spage, 0
        jmp     setp2
    setp1:
        mov     cpage, 0
        mov     spage, 128000
    setp2:
        mov     ax, seg water
        mov     es, ax
        mov     ds, ax
        mov     edi, 640
        mov     esi, edi
        add     edi, cpage
        add     esi, spage
        mov     ax, seg dbuff
        mov     fs, ax
        mov     ax, seg vbuff
        mov     gs, ax
        mov     bp, WIDTH
        mov     dx, 1
        mov     cx, 1
    next:
        inc     bp
        add     esi, 2
        add     edi, 2
        mov     ax, [edi - 2]
        add     ax, [edi - 638]
        add     ax, [edi - 640]
        add     ax, [edi - 642]
        add     ax, [edi + 2]
        add     ax, [edi + 638]
        add     ax, [edi + 640]
        add     ax, [edi + 642]
        sar     ax, 2
        mov     bx, [esi]
        sub     ax, bx
        mov     bx, ax
        sar     bx, DENSITY
        sub     ax, bx
        mov     [esi], ax
        mov     ax, [esi]
        sub     ax, [esi + 2]
        mov     bx, [esi]
        sub     bx, [esi + 640]
        sar     bx, 3
        add     bx, cx
        sar     ax, 3
        add     ax, dx
        shl     ax, 6
        add     bx, ax
        shl     ax, 2
        add     bx, ax
        mov     al, fs:[bx]
        mov     bx, bp
        mov     gs:[bx], al
        inc     cx
        cmp     cx, WIDTH - 2
        jbe     next
        add     esi, 4
        add     edi, 4
        add     bp, 2
        mov     cx, 1
        inc     dx
        cmp     dx, HEIGHT - 2
        jbe     next
        popa
    }
}

void printStr(int16_t x, int16_t y, uint8_t col, char *msg)
{
    uint16_t len = strlen(msg);

    __asm {
        les	   di, tmem
        lds	   si, msg
        add	   di, x
        shl	   di, 1
        mov	   ax, y
        shl	   ax, 5
        add	   di, ax
        shl	   ax, 2
        add	   di, ax
        mov	   ah, col
        mov    cx, len
    next:
        lodsb
        stosw
        loop   next
    }
}

void clearScreen()
{
    __asm {
        les	   di, tmem
        xor	   ax, ax
        mov	   cx, 2000
        rep	   stosw
    }
}

void makeWater(uint16_t idx)
{
    __asm {
        xor     edi, edi
        mov     ax, seg water
        mov     es, ax
        mov     si, idx
        and     si, 0FFh
        shl     si, 1
        mov     di, [costab + si]
        mov     bx, [sintab + si]
        shl     bx, 6
        add     di, bx
        shl     bx, 2
        add     di, bx
        shl     edi, 1
        mov     es:[edi], 500
    }
}

void initWater()
{
    __asm {
        mov     ax, seg water
        mov     es, ax
        mov     ax, 10000
        mov     di, 100 +  60 * 320
        shl     di, 1
        mov     es:[di], ax
        mov     di, 120 +  80 * 320
        shl     di, 1
        mov     es:[di], ax
        mov     di, 160 + 100 * 320
        shl     di, 1
        mov     es:[di], ax
        mov     di, 220 +  60 * 320
        shl     di, 1
        mov     es:[di], ax
    }
}

void main()
{
    uint16_t page;
    uint32_t frames, ticks;
    
    clearScreen();
    printStr(1, 1, 0x0F, "WATER 2 EFFECT - (c) 2000 by Nguyen Ngoc Van");
    printStr(1, 2, 0x07, "Press any key to start...");
    getch();

    __asm {
        mov	   ax, 0x13
        int	   0x10
    }

    page = 0;
    frames = 0;	
    ticks = GetTicks();

    loadCEL("sea.cel");
    flipScreen(dbuff[0], vmem);
    flipScreen(dbuff[0], vbuff[0]);
    
    initSinCos();
    initWater();

    do {
        waitRetrace();
        makeWater(frames);
        showWater(page);
        flipScreen(vbuff[0], vmem);
        page ^= 1;
        frames++;
    } while(!kbhit());

    ticks = GetTicks() - ticks;

    __asm {
        mov	   ax, 0x03
        int	   0x10
    }

    printf("%.2f frames per second.\n", frames * 18.2f / ticks);
}
