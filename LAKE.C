/*----------------------------------------------------*/
/* Packet  : Demo & Effect                            */
/* Effect  : The Lake                                 */ 
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
#include <mem.h>
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <conio.h>

#define M_PI 	3.141592f

uint8_t pal[768] = {0};
uint8_t sintab[400] = {0};
uint8_t vbuff[64000] = {0};
uint8_t bitmap[64000] = {0};
uint8_t water[64000] = {0};

uint8_t *vmem = (uint8_t*)0xA0000000L;
uint8_t *tmem = (uint8_t*)0xB8000000L;

void calcWater()
{
    int16_t i, DOOD;

    for (i = 1; i <= 72; i++)
    {
        DOOD = sintab[i];
        memcpy(&water[(72- i) * 320 + DOOD], &bitmap[(i - 1) * 320], 320 - DOOD);
    }

    DOOD = sintab[0];
    memcpy(&sintab[0], &sintab[1], 2 * 72 - 1);
    sintab[2 * 72 - 2] = DOOD;
}

void clearScreen()
{
    __asm {
        les     di, tmem
        xor     eax, eax
        mov     cx, 2000
        rep     stosw
    }
}

void printStr(int16_t x, int16_t y, uint8_t col, char *msg)
{
    int16_t len = strlen(msg);

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
        mov     cx, len
    next:
        lodsb
        stosw
        loop    next
    }
}

inline int16_t roundf(float x)
{
    if (x > 0.0) return x + 0.5;
    return x - 0.5;
}

void main()
{
    int16_t i;
    FILE *fp;

    clearScreen();
    printStr(1, 1, 0x0F, "THE LAKE - (c) 1998 by Nguyen Ngoc Van");
    printStr(1, 2, 0x07, "Press any key to start demo...");
    getch();

    for (i = 0; i < 400; i++) sintab[i] = roundf(sin(5 * i * M_PI / 100) * 3 + 3);

    __asm {
        mov     ax, 0x13
        int     0x10
    }

    fp = fopen("palio.cel", "rb");
    if (!fp) return;
    fseek(fp, 32, SEEK_SET);
    fread(pal, 1, 768, fp);
    fread(vbuff, 1, 64000, fp);
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

    memcpy(bitmap, &vbuff[64000 - 2 * 72 * 320], 72 * 320);
    memcpy(vmem, vbuff, 64000);

    do {
        calcWater();
        while (inp(0x03DA) & 8);
        while (!(inp(0x03DA) & 8));
        memcpy(&vmem[320 * (200 - 72)], water, 72 * 320);
    } while(!kbhit());

    __asm {
        mov     ax, 0x03
        int     0x10
    }
}
