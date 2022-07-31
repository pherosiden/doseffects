/*---------------------------------------------------*/
/* Packet  : Demo & Effect                           */
/* Effect  : Chainel-4                               */  
/* Author  : Nguyen Ngoc Van                         */
/* Memory  : Compact                                 */
/* Heaps   : 640K                                    */
/* Address : siden@codedemo.net                      */
/* Website : http://www.codedemo.net                 */
/* Created : 14/01/1998                              */
/* Please sent to me any bugs or suggests.           */
/* You can use freely this code. Have fun :)         */
/*---------------------------------------------------*/

#include <dos.h>
#include <mem.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <conio.h>

uint8_t vbuff[768] = {0};
uint8_t *vmem = (uint8_t*)0xA0000000L;
uint8_t *tmem = (uint8_t*)0xB8000000L;

uint8_t getLow(uint16_t num)
{
    __asm {
        mov     ax, num
        and     ax, 0x00FF
    }
}

uint8_t getHigh(uint16_t num)
{
    __asm {
        mov     ax, num
        and     ax, 0xFF00
        shr     ax, 8
    }
}

void initChain4()
{
    __asm {
        mov     ax, 0x13
        int     0x10
    }

    outp(0x3C4, 0x04);
    outp(0x3C5, (inp(0x3C5) & 0xF7) | 0x04);

    outp(0x3C4, 0x02);
    outp(0x3C5, 0x0F);

    outp(0x3D4, 0x14);
    outp(0x3D5, inp(0x3D5) & 0xBF);

    outp(0x3D4, 0x17);
    outp(0x3D5, inp(0x3D5) | 0x40);

    outp(0x3D4, 0x13);
    outp(0x3D5, 80);
}

void putPixel(int16_t x, int16_t y, uint8_t col)
{
    outpw(0x3C4, ((1 << (x % 4)) << 8) | 2);
    vmem[(y * 80 << 1) + (x >> 2)] = col;
}

void moveTo(uint16_t x, uint16_t y)
{
    uint16_t ofs = (y * 80 << 1) + x;
    outpw(0x3D4, (getHigh(ofs) << 8) | 0x0C);
    outpw(0x3D4, (getLow(ofs) << 8) | 0x0D);
}

int16_t mouseDetect()
{
    __asm {
        xor     ax, ax
        int     0x33
        xor     al, al
    }
}

void getMousePos(uint16_t *x, uint16_t *y, uint16_t *but)
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

void moveMouse(uint16_t x, uint16_t y)
{
    __asm {
        mov     ax, 0x04
        mov     cx, x
        mov     dx, y
        int     0x33
    }
}

void mouseWindow(uint16_t l, uint16_t t, uint16_t r, uint16_t b)
{
    __asm {
        mov     ax, 0x07
        mov     cx, l
        mov     dx, r
        int     0x33

        inc     ax
        mov     cx, t
        mov     dx, b
        int     0x33
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

void writeMsg(int16_t x, int16_t y, uint8_t col, char *msg)
{
    __asm {
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
    }
}

void putImage()
{
    int16_t i, j;
    FILE *fp;

    fp = fopen("assets/chen.cel", "rb");
    if (!fp) return;

    fseek(fp, 32, SEEK_SET);
    fread(vbuff, 1, 768, fp);

    __asm {
        mov     dx, 0x03C8
        xor     ax, ax
        out     dx, al
        inc     dx
        lea     si, vbuff
        mov     cx, 768
        rep     outsb
    }

    for (i = 0; i < 400; i++)
    {
        fread(vbuff, 1, 640, fp);
        for (j = 0; j < 640; j++) putPixel(j, i, vbuff[j]);
    }

    fclose(fp);
}

void main()
{
    uint16_t xpos, ypos, btn;
    
    clearTextMem();
    writeMsg(1, 1, 0x0F, "CHAIN-4 - (c) 1998 by Nguyen Ngoc Van");
    writeMsg(1, 2, 0x07, "Move the mouse  : flash effect");
    writeMsg(1, 3, 0x07, "Any key/Left button : goodbyte");
    writeMsg(1, 4, 0x07, "Press any key to start demo...");
    getch();

    if (!mouseDetect()) return;

    initChain4();
    putImage();

    mouseWindow(0, 0, 79, 199);
    moveMouse(0, 0);

    do {
        getMousePos(&xpos, &ypos, &btn);
        moveTo(xpos, ypos);
    } while(!kbhit() && btn != 1);

    __asm {
        xor     ax, ax
        int     0x33
        mov     ax, 0x03
        int     0x10
    }
}
