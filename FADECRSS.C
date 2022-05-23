/*---------------------------------------------------*/
/* Packet  : Demo & Effect                           */ 
/* Effect  : Cross Fade                              */
/* Author  : Nguyen Ngoc Van                         */
/* Memory  : Compact                                 */
/* Heaps   : 640K                                    */
/* Address : siden@codedemo.net                      */
/* Website : http://www.codedemo.net                 */
/* Created : 16/01/1998                              */ 
/* Please sent to me any bugs or suggests.           */
/* You can use freely this code. Have fun :)         */
/*---------------------------------------------------*/

#include <dos.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <conio.h>

int8_t dirt = 0;

uint8_t src[256][3] = {0};
uint8_t dst[256][3] = {0};

uint8_t vbuff1[64000] = {0};
uint8_t vbuff2[64000] = {0};

uint8_t *tmem = (uint8_t*)0xB8000000L;
uint8_t *vmem = (uint8_t*)0xA0000000L;

void setRGB(uint8_t n, uint8_t r, uint8_t g, uint8_t b)
{
    __asm {
        mov     dx, 0x03C8
        mov     al, n
        out     dx, al
        inc     dx
        mov     al, r
        out     dx, al
        mov     al, g
        out     dx, al
        mov     al, b
        out     dx, al
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

void clearTextMem()
{
    __asm {
        les     di, tmem
        xor     ax, ax
        mov     cx, 2000
        rep     stosw
    }
}

void waitRetrace()
{
    __asm {
        mov     dx, 0x03DA
    waith:
        in      al, dx
        test    al, 0x08
        jnz     waith
    waitv:
        in      al, dx
        test    al, 0x08
        jz      waitv
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

void flip(uint8_t *src, uint8_t *dst)
{
    __asm {
        les     di, dst
        lds     si, src
        mov     cx, 16000
        rep     movsd
    }
}

uint8_t getPixel(int16_t x, int16_t y, uint8_t *where)
{
    __asm {
        lds     si, where
        mov     bx, y
        shl     bx, 6
        add     bh, byte ptr y
        add     bx, x
        add     si, bx
        lodsb
    }
}

void putPixel(int16_t x, int16_t y, uint8_t c, uint8_t *where)
{
    __asm {
        les     di, where
        mov     bx, y
        shl     bx, 6
        add     bh, byte ptr y
        add     bx, x
        add     di, bx
        mov     al, c
        stosb
    }
}

void movePal(void *dst, void *src)
{
    __asm {
        lds     si, src
        les     di, dst
        mov     cx, 192
        rep     movsd
    }
}

void loadCEL(char *fname, uint8_t *pal, uint8_t *buff)
{
    FILE *fp = fopen(fname, "rb");
    if (!fp) return;

    fseek(fp, 32, SEEK_SET);
    fread(pal, 1, 768, fp);
    fread(buff, 1, 64000, fp);
    fclose(fp);
}

void setItUp()
{
    uint8_t pal1[256][3] = {0};
    uint8_t pal2[256][3] = {0};

    int16_t x, y, k;
    uint8_t change = 0;
    uint8_t col = 0, col1 = 0, col2 = 0;

    for (x = 0; x < 256; x++) setRGB(x, 0, 0, 0);

    loadCEL("to.cel", pal2, vbuff1);
    flip(vbuff1, vmem);
    loadCEL("from.cel", pal1, vbuff1);

    for (y = 0; y < 200; y++)
    {
        for (x = 0; x < 320; x++)
        {
            col1 = getPixel(x, y, vbuff1);
            col2 = getPixel(x, y, vmem);

            if (col1 || col2)
            {
                change = 0;

                for (k = 0; k <= col; k++)
                {
                    if (src[k][0] == pal1[col1][0] && src[k][1] == pal1[col1][1] && src[k][2] == pal1[col1][2] && 
                        dst[k][0] == pal2[col2][0] && dst[k][1] == pal2[col2][1] && dst[k][2] == pal2[col2][2]) 
                    {
                        change = 1;
                        putPixel(x, y, k, vbuff2);
                    }
                }

                if (!change)
                {
                    col++;

                    src[col][0] = pal1[col1][0];
                    src[col][1] = pal1[col1][1];
                    src[col][2] = pal1[col1][2];

                    dst[col][0] = pal2[col2][0];
                    dst[col][1] = pal2[col2][1];
                    dst[col][2] = pal2[col2][2];

                    putPixel(x, y, col, vbuff2);
                }
            }
        }
    }
}

void crossFade(int8_t dirt, uint16_t depth)
{
    int16_t i, j;
    uint8_t tmp[256][3] = {0};
    
    if (dirt)
    {
        movePal(tmp, src);
        for (i = 0; i <= 255; i++) setRGB(i, src[i][0], src[i][1], src[i][2]);

        for (i = 0; i <= depth; i++)
        {
            for (j = 0; j <= 255; j++) setRGB(j, tmp[j][0], tmp[j][1], tmp[j][2]);

            for (j = 0; j <= 255; j++)
            {
                if (tmp[j][0] < dst[j][0]) tmp[j][0]++;
                if (tmp[j][0] > dst[j][0]) tmp[j][0]--;

                if (tmp[j][1] < dst[j][1]) tmp[j][1]++;
                if (tmp[j][1] > dst[j][1]) tmp[j][1]--;

                if (tmp[j][2] < dst[j][2]) tmp[j][2]++;
                if (tmp[j][2] > dst[j][2]) tmp[j][2]--;

                if (inp(0x60) == 1) return;
            }

            waitRetrace();
        }
    }
    else
    {
        movePal(tmp, dst);
        for (i = 0; i <= 255; i++) setRGB(i, dst[i][0], dst[i][1], dst[i][2]);

        for (i = 0; i <= depth; i++)
        {
            for (j = 0; j <= 255; j++) setRGB(j, tmp[j][0], tmp[j][1], tmp[j][2]);

            for (j = 0; j <= 255; j++)
            {
                if (tmp[j][0] < src[j][0]) tmp[j][0]++;
                if (tmp[j][0] > src[j][0]) tmp[j][0]--;

                if (tmp[j][1] < src[j][1]) tmp[j][1]++;
                if (tmp[j][1] > src[j][1]) tmp[j][1]--;

                if (tmp[j][2] < src[j][2]) tmp[j][2]++;
                if (tmp[j][2] > src[j][2]) tmp[j][2]--;

                if (inp(0x60) == 1) return;
            }

            waitRetrace();
        }
    }
}

void main()
{
    int16_t i;
    FILE *fp;

    clearTextMem();
    printStr(1, 1, 0x0F, "Cross Fade - (c) 1998 by Nguyen Ngoc Van");
    printStr(1, 2, 0x07, "Press any key to start demo...");
    getch();

    __asm {
        mov     ax, 0x13
        int     0x10
    }

    fp = fopen("fade.dat", "rb");
    if (!fp)
    {
        fp = fopen("fade.dat", "wb");
        if (!fp) return;

        setItUp();
        fwrite(src, 3, 256, fp);
        fwrite(dst, 3, 256, fp);
        fwrite(vbuff2, 1, 64000, fp);
    }
    else
    {
        fread(src, 3, 256, fp);
        fread(dst, 3, 256, fp);
        fread(vbuff2, 1, 64000, fp);
    }

    fclose(fp);

    for (i = 0; i < 256; i++) setRGB(i, src[i][0], src[i][1], src[i][2]);

    flip(vbuff2, vmem);
    dirt = 1;

    do {
        crossFade(dirt, 63);
        dirt = !dirt;
    } while (!kbhit());

    __asm {
        mov     ax, 0x03
        int     0x10
    }
}
