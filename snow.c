/*----------------------------------------------------*/
/* Packet  : Demo & Effect                            */
/* Effect  : Snow                                     */
/* Author  : Nguyen Ngoc Van                          */
/* Memory  : Compact                                  */
/* Heaps   : 640K                                     */
/* Address : pherosiden@gmail.com                     */
/* Website : http://www.codedemo.net                  */
/* Created : 04/03/1998                               */
/* Please sent to me any bugs or suggests.            */
/* You can use freely this code. Have fun :)          */
/*----------------------------------------------------*/
#include <dos.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <conio.h>

uint8_t *vmem = (uint8_t*)0xA0000000L;
uint8_t *tmem = (uint8_t*)0xB8000000L;

int16_t actflk = 0;
uint8_t pal[768] = {0};
uint16_t flake[320] = {0};

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

void updateFlakes()
{
    int16_t i;
    int16_t flakeaddr, offs;

    for (i = 0; i < actflk; i++)
    {
        flakeaddr = flake[i];

        if (!vmem[flakeaddr + 319] || !vmem[flakeaddr + 320] || !vmem[flakeaddr + 321])
        {

            vmem[flakeaddr] = 0;
            offs = 320;

            if (rand() % 2 == 1)
            {
                if (!vmem[flakeaddr + 319]) offs--;
                else if (!vmem[flakeaddr + 321]) offs++;
            }
            else
            {
                if (!vmem[flakeaddr + 321]) offs++;
                else if (!vmem[flakeaddr + 319]) offs--;
            }

            flakeaddr += offs;
            vmem[flakeaddr] = 255;
            flake[i] = flakeaddr;
        }
        else
        {
            if (vmem[flakeaddr + 319] == 255 || vmem[flakeaddr + 320] == 255 || vmem[flakeaddr + 321] == 255) flake[i] = flakeaddr;
            else
            {
                vmem[flakeaddr] = 36;
                flake[i] = rand() % 320;
            }
        }
    }
}

void initSnow()
{
    FILE *fp;

    if (!(fp = fopen("flake.cel", "rb"))) exit(1);
    fseek(fp, 32, SEEK_SET);
    fread(pal, 1, 768, fp);
    fread(vmem, 1, 64000, fp);
    fclose(fp);

    pal[765] = 63;
    pal[766] = 63;
    pal[767] = 63;

    __asm {
        mov     dx, 0x03C8
        xor     al, al
        out     dx, al
        inc     dx
        lea     si, pal
        mov     cx, 768
        rep     outsb
    }
}

void snowFall()
{
    actflk = 0;
    initSnow();

    do {
        if (actflk < 320) flake[actflk++] = rand() % 320;
        updateFlakes();
        retrace();
    } while(!kbhit());
}

void main()
{
    clearScreen();
    printStr(1, 1, 0x0F, "SNOW - (c) 1998 by Nguyen Ngoc Van");
    printStr(1, 2, 0x07, "Any key : Start demo - ESC : Exit.");
    srand(time(NULL));

    __asm {
    next:
        xor     ax, ax
        int     0x16
        jnz     next
        mov     ax, 0x13
        int     0x10
        call  	snowFall
        mov     ax, 0x03
        int     0x10
    }
}
