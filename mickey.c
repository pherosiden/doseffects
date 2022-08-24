/*---------------------------------------------------*/
/* Packet  : Demo & Effect                           */
/* Effect  : Animation (use EMS memory)              */
/* Author  : Nguyen Ngoc Van                         */ 
/* Memory  : Compact                                 */
/* Heaps   : 640K                                    */
/* Address : pherosiden@gmail.com                    */
/* Website : http://www.codedemo.net                 */
/* Created : 15/01/1998                              */
/* Please sent to me any bugs or suggests.           */ 
/* You can use freely this code. Have fun :)         */
/*---------------------------------------------------*/

#include <dos.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <conio.h>
#include "ems.c"

uint16_t handle[15] = {0};
uint8_t *vmem = (uint8_t*)0xA0000000L;
uint8_t *tmem = (uint8_t*)0xB8000000L;
uint8_t *vbuff = (uint8_t*)0x80000000L;

void retrace()
{
    __asm {
        mov     dx, 0x03DA
    waitV:
        in      al, dx
        and     al, 0x08
        jnz     waitV
    waitH:
        in      al, dx
        and     al, 0x08
        jz      waitH
    }
}

void moveData(void *dst, void *src)
{
    __asm {
        les     di, dst
        lds     si, src
        mov     cx, 4000
        rep     movsd
    }
}

void RAM2EMS(uint8_t *pages, uint16_t handle)
{
    int16_t i;
    for (i = 0; i < 4; i++)
    {
        EMSMap(handle, i, 0);
        moveData((uint8_t*)MK_FP(EMSGetFrameSeg(), i), pages);
        pages += 16000;
    }
}

void EMS2RAM(uint8_t *pages, uint16_t handle)
{
    int16_t i;
    for (i = 0; i < 4; i++)
    {
        EMSMap(handle, i, 0);
        moveData(pages, (uint8_t*)MK_FP(EMSGetFrameSeg(), i));
        pages += 16000;
    }
}

void clearMem(uint8_t *mem, uint16_t len)
{
    __asm {
        les     di, mem
        xor     ax, ax
        mov     cx, len
        rep     stosw
    }
}

void printStr(int16_t x, int16_t y, uint8_t col, char *msg)
{
    uint16_t len = strlen(msg);

    __asm {
        mov     cx, len
        test    cx, cx
        jz      quit
        les     di, tmem
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

void main()
{
    FILE *fp;
    int16_t i;
    uint8_t pal[768] = {0};

    clearMem(tmem, 2000);
    printStr(1, 1, 0x0F, "EMS Tester - (c) 1998 by Nguyen Ngoc Van");
    printStr(1, 2, 0x07, "Press any key to continue ...");
    getch();

    if (!EMSDetect()) return;

    __asm {
        mov     ax, 0x13
        int     0x10
    }

    fp = fopen("assets/mickey.dat", "rb");
    if (!fp) return;
    fread(pal, 1, 768, fp);

    for (i = 0; i < 15; i++)
    {
        handle[i] = EMSNew(4);
        fread(vbuff, 1, 64000, fp);
        RAM2EMS(vbuff, handle[i]);
    }

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

    i = 0;
    
    while (!kbhit())
    {
        retrace();
        EMS2RAM(vmem, handle[i]);
        if (i++ >= 14) i = 0;
    }

    __asm {
        mov     ax, 0x03
        int     0x10
    }

    for (i = 0; i < 15; i++) EMSFree(handle[i]);
}
