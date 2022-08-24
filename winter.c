/*---------------------------------------------------*/
/* Packet  : Demo & Effect                           */
/* Effect  : Winter                                  */
/* Author  : Nguyen Ngoc Van                         */  
/* Memory  : Compact                                 */ 
/* Heaps   : 640K                                    */
/* Address : pherosiden@gmail.com                    */ 
/* Website : http://www.codedemo.net                 */ 
/* Created : 05/03/1998                              */
/* Please sent to me any bugs or suggests.           */
/* You can use freely this code. Have fun :)         */
/*---------------------------------------------------*/

#include <dos.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <conio.h>

#define FLAKES  1000
#define FASTEST 360

typedef struct {
    int16_t x, y;
    int16_t w, h;
} Flakes;

uint8_t *vmem = (uint8_t*)0xA0000000L;
uint8_t *tmem = (uint8_t*)0xB8000000L;

uint16_t rz = 0;
uint8_t tbuff[200][320] = {0};
uint8_t vbuff[200][320] = {0};
uint8_t pal[768] = {0};
Flakes flakes[FLAKES] = {0};

void perturb()
{
    __asm {
        mov     dx, rz
        xor     dx, 0xAA55
        shl     dx, 1
        adc     dx, 0x0118
        mov     rz, dx
    }
}

void retrace()
{
    __asm {
        mov     dx, 0x03DA
    waitH:
        in      al, dx
        and     al, 0x08
        jnz     waitH
    waitV:
        in      al, dx
        and     al, 0x08
        jz      waitV
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

void flip(void* src)
{
    __asm {
        les     di, vmem
        lds     si, src
        mov     cx, 16000
        rep     movsd
    }
}

void snowFall()
{
    int16_t i;

    while (!kbhit())
    {
        for (i = 0; i < FLAKES; i++)
        {
            perturb();
            tbuff[flakes[i].h % 200][flakes[i].w % 320] = vbuff[flakes[i].h % 200][flakes[i].w % 320];
            if (flakes[i].x >= (rz & 0x0F)) flakes[i].w++;
            if (flakes[i].y >= (rz & 0xFF)) flakes[i].h++;
            tbuff[flakes[i].h % 200][flakes[i].w % 320] = (flakes[i].y >> 5) + 240;
        }
        retrace();
        flip(tbuff[0]);
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

void main()
{
    FILE *fp;
    int16_t i;
    uint16_t pos = 0;

    clearScreen();
    printStr(1, 1, 0x0F, "WINTER - (c) 1998 by Nguyen Ngoc Van");
    printStr(1, 2, 0x07, "Press any key to start demo...");
    getch();

    fp = fopen("assets/winter.cel", "rb");
    if (!fp) return;

    fseek(fp, 32, SEEK_SET);
    fread(pal, 1, 768, fp);
    fread(vbuff[0], 1, 64000, fp);
    fclose(fp);

    memcpy(tbuff[0], vbuff[0], 64000);
    memset(flakes, 0, sizeof(flakes));

    for (i = 0; i < FLAKES; i++)
    {
        perturb();
        pos += rz;
        flakes[i].y = (((rz & 0xFF) * FASTEST) & 0xFF) + 5;
        flakes[i].x = (((rz & 0x0F) * flakes[i].y) & 0xFF) + 1;
        flakes[i].w = pos % 320;
        flakes[i].h = pos / 200;
    }

    __asm {
        mov     ax, 0x13
        int     0x10
        mov     dx, 0x03C8
        xor     al, al
        out     dx, al
        inc     dx
        lea     si, pal
        mov     cx, 768
        rep     outsb
        call    flip
        call    snowFall
        mov     ax, 0x03
        int     0x10
    }
}
