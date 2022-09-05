/*----------------------------------------------------*/
/* Packet  : Demo & Effect                            */
/* Effect  : New mouse pointer                        */
/* Author  : Nguyen Ngoc Van                          */
/* Memory  : Compact                                  */
/* Heaps   : 640K                                     */
/* Address : pherosiden@gmail.com                     */
/* Website : http://www.codedemo.net                  */
/* Created : 17/02/1998                               */
/* Please sent to me any bugs or suggests.            */
/* You can use freely this code. Have fun :)          */
/*----------------------------------------------------*/

#include <dos.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <conio.h>

const uint16_t cursor[] = {
    0xFF07, 0xFE03, 0xFC01, 0xF800, 0xF000, 0xE000, 0xC000, 0x8000,
    0x0001, 0x0003, 0x0007, 0x000F, 0x001F, 0x003F, 0x007F, 0x00FF,
    0x0000, 0x0078, 0x017C, 0x0366, 0x0766, 0x0EBE, 0x1DDE, 0x1BE0,
    0x57DC, 0x4FB8, 0x5F70, 0x6EE0, 0x75C0, 0x7800, 0x3F00, 0x0000
};

const int16_t hotspotx = -1;
const int16_t hotspoty = 16;
uint8_t pal[768] = {0};

uint8_t *vmem = (uint8_t*)0xA0000000L;
uint8_t *tmem = (uint8_t*)0xB8000000L;

void clearScreen()
{
    __asm {
        les     di, tmem
        xor     ax, ax
        mov     cx, 2000
        rep     stosw
    }
}

void printMsg(int16_t x, int16_t y, uint8_t col, char *msg)
{
    uint16_t len = strlen(msg);

    __asm {
        mov     cx, len
        test    cx, cx
        jz      quit
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
        stosw
        loop    next
    quit:
    }
}

void main()
{
    FILE *fp;
    const uint16_t *mask = cursor;

    clearScreen();
    printMsg(1, 1, 0x0F, "NEW MOUSE POINTER - (c) 1998 by Nguyen Ngoc Van");
    printMsg(1, 2, 0x07, "Press any key to continue ...");

    __asm {
    next:
        xor     ax, ax
        int     0x16
        test  	al, al
        jz      next
        mov     ax, 0x13
        int     0x10
    }

    fp = fopen("assets/angel.cel", "rb");
    if (!fp) return;
    fseek(fp, 32, SEEK_SET);
    fread(pal, 1, 768, fp);
    fread(MK_FP(0xA000, 0), 1, 64000, fp);
    fclose(fp);

    __asm {
        mov     dx, 0x03C8
        xor     al, al
        out     dx, al
        inc     dx
        lea     si, pal
        mov     cx, 768
        rep     outsb
        xor     ax, ax
        int     0x33
        mov     ax, 0x0009
        mov     bx, hotspotx
        mov     cx, hotspoty
        mov     dx, word ptr mask
        mov     es, word ptr mask + 2
        int     0x33
        mov     ax, 0x01
        int     0x33
        xor     ah, ah
        int     0x16
        mov     ax, 0x03
        int     0x10
    }
}
