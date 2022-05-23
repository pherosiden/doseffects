/*---------------------------------------------------*/
/* Packet  : Demo & Effect                           */
/* Effect  : Circle                                  */
/* Author  : Nguyen Ngoc Van                         */
/* Memory  : Compact                                 */
/* Heaps   : 640K                                    */
/* Address : siden@codedemo.net                      */
/* Website : http://www.codedemo.net                 */ 
/* Created : 15/01/1998                              */
/* Please sent to me any bugs or suggests.           */ 
/* You can use freely this code. Have fun :)         */
/*---------------------------------------------------*/

#include <dos.h>
#include <mem.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <conio.h>

uint8_t pal[768] = {0};
uint8_t vbuff1[64000] = {0};
uint8_t vbuff2[64000] = {0};
uint8_t vbuff3[64000] = {0};

uint8_t *tmem = (uint8_t*)0xB8000000L;
uint8_t *vmem = (uint8_t*)0xA0000000L;

float costab[400] = {0};
float sintab[400] = {0};

int16_t initMouse()
{
    __asm {
        xor     ax, ax
        int     0x33
        xor     al, al
    }
}

void getMousePos(int16_t *x, int16_t *y, int16_t *but)
{
    __asm {
        mov     ax, 0x03
        int     0x33
        les     di, but
        mov     es:[di], bx
        les     di, x
        mov     es:[di], cx
        les     di, y
        mov     es:[di], dx
    }
}

void setMouseWindow(int16_t x1, int16_t y1, int16_t x2, int16_t y2)
{
    __asm {
        mov     ax, 0x07
        mov     cx, x1
        mov     dx, x2
        int     0x33
        inc     ax
        mov     cx, y1
        mov     dx, y2
        int     0x33
    }
}

void setMousePos(int16_t x, int16_t y)
{
    __asm {
        mov     ax, 0x04
        mov     cx, x
        mov     dx, y
        int     0x33
    }
}

void flip(void *src, void *dst)
{
    __asm {
        les     di, dst
        lds     si, src
        mov     cx, 16000
        rep     movsd
    }
}

uint8_t getPixel(int16_t x, int16_t y)
{
    __asm {
        mov     ax, seg vbuff1
        mov     ds, ax
        mov     bx, y
        shl     bx, 6
        add     bh, byte ptr y
        add     bx, x
        mov     si, bx
        lodsb
    }
}

void putPixel(int16_t x, int16_t y, uint8_t col)
{
    __asm {
        mov     ax, seg vbuff2
        mov     es, ax
        mov     bx, y
        shl     bx, 6
        add     bh, byte ptr y
        add     bx, x
        mov     di, bx
        mov     al, col
        stosb
    }
}

void clearTextMem()
{
    __asm {
        les     di, tmem
        xor     ax, ax
        mov     cx, 2000
        rep     stosw
    }
}

void clearBuffer(uint8_t *buff)
{
    __asm {
        les     di, buff
        xor     eax, eax
        mov     cx, 16000
        rep     stosd
    }
}

void writeMsg(int16_t x, int16_t y, uint8_t col, char *msg)
{
    __asm {
        push    ds
        lds     si, msg
        les     di, tmem
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
        test    al, al
        jz      quit
        stosw
        jmp     next
    quit:
        pop     ds
    }
}

void preCalc()
{
    int16_t i;
    float deg = 0.0;
    
    for (i = 0; i < 400; i++)
    {
        costab[i] = cos(deg);
        sintab[i] = sin(deg);
        deg += 0.05;
    }
}

int16_t roundf(float x)
{
    if (x > 0.0) return x + 0.5;
    return x - 0.5;
}

void drawCircle(int16_t x, int16_t y, int16_t rad)
{
    int16_t i;
    int16_t ox, oy;

    for (i = 0; i < 400; i++)
    {
        ox = roundf(rad * costab[i]);
        oy = roundf(rad * sintab[i]);
        putPixel(x + ox, y + oy, getPixel(x + ox, y + oy));
    }
}

void main()
{
    FILE *fp;
    int16_t i;
    int16_t x, y, btn;

    clearTextMem();
    writeMsg(1, 1, 0x0F, "drawCircle - (c) 1998 by Nguyen Ngoc Van");
    writeMsg(1, 2, 0x07, "Use mouse to move - ESC/LB to exit");
    writeMsg(1, 3, 0x07, "Press any key to start demo...");
    getch();

    if (!initMouse()) return;

    preCalc();

    fp = fopen("image.cel", "rb");
    if (!fp) return;

    fseek(fp, 32, SEEK_SET);
    fread(pal, 1, 768, fp);
    fread(vbuff1, 1, 64000, fp);
    fclose(fp);
    
    fp = fopen("wall.cel", "rb");
    if (!fp) return;

    fseek(fp, 800, SEEK_SET);
    fread(vbuff3, 1, 64000, fp);
    fclose(fp);

    __asm {
        mov     ax, 0x13
        int     0x10
        mov     dx, 0x03C8
        xor     ax, ax
        out     dx, al
        inc     dx
        lea     si, pal
        mov     cx, 768
        rep     outsb
    }

    setMouseWindow(38, 38, 319 - 38, 199 - 38);
    setMousePos(40, 70);

    do {
        getMousePos(&x, &y, &btn);
        flip(vbuff3, vbuff2);
        for (i = 0; i < 38; i++) drawCircle(x, y, i);
        flip(vbuff2, vmem);
    } while(!kbhit() && btn != 1);

    __asm {
        xor     ax, ax
        int     0x33
        mov     ax, 0x03
        int     0x10
    }
}
