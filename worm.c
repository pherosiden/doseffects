/*----------------------------------------------------*/
/* Packet  : Demo & Effect                            */
/* Effect  : Wormhome                                 */
/* Author  : Nguyen Ngoc Van                          */
/* Memory  : Compact                                  */
/* Heaps   : 640K                                     */
/* Address : pherosiden@gmail.com                     */
/* Website : http://www.codedemo.net                  */
/* Created : 29/03/1998                               */
/* Please sent to me any bugs or suggests.            */
/* You can use freely this code. Have fun :)          */
/*----------------------------------------------------*/
#include <dos.h>
#include <mem.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <conio.h>

uint8_t tmp[16][3] = {0};
uint8_t pal[256][3] = {0};

void waitRetrace()
{
    __asm {
        mov     dx, 0x03DA
    waitH:
        in      al, dx
        and     al, 0x08
        jz      waitH
    waitV:
        in      al, dx
        and     al, 0x08
        jnz     waitV
    }
}

void setPalette()
{
    int16_t i;

    outp(0x03C8, 16);
    for (i = 16; i < 256; i++)
    {
        outp(0x3C9, pal[i][0]);
        outp(0x3C9, pal[i][1]);
        outp(0x3C9, pal[i][2]);
    }
}

void main()
{
    FILE *fp;
    int16_t i, j;

    __asm {
        mov 	ax, 0x13
        int     0x10
    }

    for (i = 1; i < 16; i++)
    {
        for (j = 0; j < 16; j++)
        {
            pal[(i << 4) + j][0] = i << 2;
            pal[(i << 4) + j][1] = j << 1;
            pal[(i << 4) + j][2] = 63;
        }
    }

    setPalette();

    fp = fopen("assets/worm.cel", "rb");
    if (!fp) return;

    fseek(fp, 800, SEEK_SET);
    fread(MK_FP(0xA000, 0), 1, 64000, fp);
    fclose(fp);

    while (!kbhit())
    {
        memcpy(tmp[0], pal[16], sizeof(tmp));
        for (i = 1; i < 16; i++) memcpy(pal[(i - 1) << 4], pal[i << 4], sizeof(tmp));
        memcpy(pal[240], tmp[0], sizeof(tmp));
        waitRetrace();
        setPalette();
    }

    __asm {
        mov     ax, 0x03
        int     0x10
    }
}
