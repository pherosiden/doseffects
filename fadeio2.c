/*---------------------------------------------------*/
/* Demo how to add an extra data into .exe file      */
/* Packet  : Demo & Effect                           */ 
/* Effect  : Fade (In/Out/Min/Max)                   */  
/* Author  : Nguyen Ngoc Van                         */ 
/* Memory  : Compact                                 */
/* Heaps   : 640K                                    */
/* Address : siden@codedemo.net                      */
/* Website : http://www.codedemo.net                 */
/* Created : 16/01/1998                              */
/* Compile : wcl -zq -6 -ox -mc -fp6 fadeio2.c       */
/*           copy /B fadeio2.exe+arnold.cel          */
/* Please sent to me any bugs or suggests.           */
/* You can use freely this code. Have fun :)         */ 
/*---------------------------------------------------*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

uint16_t bytesLastPage, numPages;
uint8_t *vmem = (uint8_t*)0xA0000000L;
uint8_t *tmem = (uint8_t*)0xB8000000L;

typedef struct tagRGB {
    uint8_t r, g, b;
} RGB;

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
        and 	al, 0x08
        jz      waitV
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

void setPalette(void *pal)
{
    __asm {
        mov     dx, 0x03C8
        xor     al, al
        out     dx, al
        inc     dx
        lds     si, pal
        mov     cx, 768
        rep     outsb
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

void printStr(int16_t x, int16_t y, uint8_t col, char *msg)
{
    int16_t len = strlen(msg);

    __asm {
        lds     si, msg
        les     di, tmem
        mov     di, x
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

void fadeIn(RGB *dst, RGB *src)
{
    int16_t i, j;

    for (j = 63; j >= 0; j--)
    {
        for (i = 0; i < 256; i++)
        {
            if (j - src[i].r < 0) dst[i].r++;
            if (j - src[i].g < 0) dst[i].g++;
            if (j - src[i].b < 0) dst[i].b++;
        }

        retrace();
        setPalette(dst);
    }
}

void fadeOut(RGB *dst, RGB *src)
{
    int16_t i, j;

    for (j = 63; j >= 0; j--)
    {
        for (i = 0; i < 256; i++)
        {
            if (j - src[i].r > 0) dst[i].r--;
            if (j - src[i].g > 0) dst[i].g--;
            if (j - src[i].b > 0) dst[i].b--;
        }

        retrace();
        setPalette(dst);
    }
}

void fadeMax(RGB *pal)
{
    int16_t i, j;

    for (j = 0; j < 64; j++)
    {
        for (i = 0; i < 256; i++)
        {
            if (pal[i].r < 63) pal[i].r++;
            if (pal[i].g < 63) pal[i].g++;
            if (pal[i].b < 63) pal[i].b++;
        }

        retrace();
        setPalette(pal);
    }
}

void fadeMin(RGB *pal)
{
    int16_t i, j;

    for (j = 0; j < 64; j++)
    {
        for (i = 0; i < 256; i++)
        {
            if (pal[i].r > 0) pal[i].r--;
            if (pal[i].g > 0) pal[i].g--;
            if (pal[i].b > 0) pal[i].b--;
        }

        retrace();
        setPalette(pal);
    }
}

void main(int argc, char *argv[])
{
    FILE *fp;
    int16_t i;
    uint8_t *vbuff;
    uint16_t pal, data;
    
    RGB dst[256] = {0};
    RGB src[256] = {0};

    clearTextMem();
    printStr(1, 1, 0x0F, "Fader - (c) 1998 by Nguyen Ngoc Van");
    printStr(1, 2, 0x07, "Press any key to continue...");

    __asm {
    next:
        xor     ah, ah
        int     0x16
        and     al, al
        jz      next
    }

    vbuff = (uint8_t*)calloc(64000, 1);
    if (!vbuff) return;

    fp = fopen(argv[0], "rb");
    if (!fp) return;

    fseek(fp, 2, SEEK_SET);
    fread(&bytesLastPage, 2, 1, fp);
    fread(&numPages, 2, 1, fp);
    fseek(fp, bytesLastPage + ((numPages - 1) << 9) + 32, SEEK_SET);
    pal = fread(src, 1, 768, fp);
    data = fread(vbuff, 1, 64000, fp);
    fclose(fp);

    if (pal != 768 || data != 64000)
    {
        free(vbuff);
        printf("Cannot find image data.\n");
        printf("Usage copy /B fadeio2.exe+arnold.cel to add image data.\n");
        return;
    }

    __asm {
        mov     ax, 0x13
        int     0x10
        mov     dx, 0x03C8
        xor     al, al
        out     dx, al
        inc     dx
        mov     cx, 768
    next:
        out     dx, al
        loop    next
    }

    flip(vbuff, vmem);
    fadeIn(dst, src);
    fadeMax(dst);
    fadeOut(dst, src);
    fadeMin(dst);

    __asm {
        mov     ax, 0x03
        int     0x10
    }

    free(vbuff);
}
