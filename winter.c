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

#define FLAKES 1000
#define FASTEST 360

typedef struct {
    uint8_t x, y;
    uint16_t p;
} Flakes;

uint8_t *vmem = (uint8_t*)0xA0000000L;
uint8_t *tmem = (uint8_t*)0xB8000000L;

uint16_t rz = 0;
uint8_t vbuff[64000] = {0};
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
        test    al, 0x08
        jnz     waitH
    waitV:
        in      al, dx
        test    al, 0x08
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

void flip()
{
    __asm {
        les     di, vmem
        mov     ax, seg vbuff
        mov     ds, ax
        xor     si, si
        mov     cx, 16000
        rep     movsd
    }
}

void snowFall()
{
    int16_t i;

    do {
        retrace();
        for (i = 0; i < FLAKES; i++)
        {
            perturb();
            vmem[flakes[i].p] = vbuff[flakes[i].p];
            if (flakes[i].x >= (rz & 0x0F)) flakes[i].p++;
            if (flakes[i].y >= (rz & 0xFF)) flakes[i].p += 320;
            vmem[flakes[i].p] = (flakes[i].y >> 5) + 240;
        }
    } while (!kbhit());
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
    uint16_t s = 0;

    clearScreen();
    printStr(1, 1, 0x0F, "WINTER - (c) 1998 by Nguyen Ngoc Van");
    printStr(1, 2, 0x07, "Press any key to start demo...");
    getch();

    fp = fopen("assets/winter.cel", "rb");
    if (!fp) return;

    fseek(fp, 32, SEEK_SET);
    fread(pal, 1, 768, fp);
    fread(vbuff, 1, 64000, fp);
    fclose(fp);

    for (i = 0; i < FLAKES; i++)
    {
        perturb();
        s += rz;

        flakes[i].y = (((rz & 0xFF) * FASTEST) & 0xFF) + 5;
        flakes[i].x = (((rz & 0x0F) * flakes[i].y) & 0xFF) + 1;
        flakes[i].p = s;
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
