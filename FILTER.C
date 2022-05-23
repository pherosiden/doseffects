/*---------------------------------------------------*/
/* Packet  : Demo & Effect                           */
/* Effect  : Image Processing (Use filters)          */
/* Author  : Nguyen Ngoc Van                         */ 
/* Memory  : Compact                                 */
/* Heaps   : 640K                                    */
/* Address : pherosiden@gmail.com                    */
/* Website : http://www.codedemo.net                 */
/* Created : 18/01/1998                              */
/* Please sent to me any bugs or suggests.           */
/* You can use freely this code. Have fun :)         */
/*---------------------------------------------------*/

#include <dos.h>
#include <mem.h>
#include <math.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <conio.h>

uint8_t pal[256][3] = {0};
uint8_t gray[256] = {0};
int8_t kbarr[128] = {0};
uint8_t vbuff1[200][200] = {0};
uint8_t vbuff2[200][200] = {0};
uint8_t *tmem = (uint8_t*)0xB8000000L;

void (interrupt __far *old09)();

void interrupt __far new09()
{
    char chr = inp(0x60);
    if (chr >= 128) kbarr[chr - 128] = 0;
    else kbarr[chr] = 1;

    __asm {
        mov     al, 0x20
        out     0x20, al
    }
}

int16_t random(int16_t x)
{
    if (x == 0) return 0;
    return rand() % x;
}

void drawPic()
{
    __asm {
        mov     ax, 0xA000
        mov     es, ax
        xor     di, di
        mov     ax, seg vbuff2
        mov     ds, ax
        xor     si, si
        mov     cx, 200
        xor     ax, ax
    next:
        push    cx
        mov     cx, 50
        xor     di, di
        mov     bx, ax
        shl     bx, 6
        add     bh, al
        add     di, bx
        rep     movsd
        inc     ax
        pop     cx
        loop    next
    }
}

void flip()
{
    __asm {
        mov     ax, seg vbuff1
        mov     ds, ax
        mov     ax, seg vbuff2
        mov     es, ax
        xor     di, di
        xor     si, si
        mov     cx, 10000
        rep     movsd
    }
}

void clearMem(uint8_t *mem)
{
    __asm {
        les     di, mem
        xor     eax, eax
        mov     cx, 10000
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

void printStr(int16_t x, int16_t y, uint8_t col, const char *msg)
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

void calcGray()
{
    int16_t i;
    for (i = 0; i < 256; i++) gray[i] = pal[i][0] * 0.299 + pal[i][1] * 0.587 + pal[i][2] * 0.114;
}

void shear()
{
    int16_t x, y;
    int16_t dx, dy, r = 0;
    int16_t yt[200] = {0};

    for (x = 0; x < 200; x++)
    {
        if (random(256) < 128) r--;
        else r++;
        yt[x] = r;
    }

    for (y = 0; y < 200; y++)
    {
        if (random(256) < 128) r--;
        else r++;
        for (x = 0; x < 200; x++)
        {
            dx = x + r;
            dy = y + yt[x];
            if (dx >= 0 && dx <= 199 && dy >= 0 && dy <= 199) vbuff2[x][y] = vbuff1[dx][dy];
            else vbuff2[x][y] = 0;
        }
    }
}

void melting()
{
    uint16_t x, y, i, val;

    for (i = 0; i < 40000; i++)
    {
        x = random(200);
        y = random(200);
        while (y < 199 && vbuff2[y][x] <= vbuff2[y + 1][x])
        {
            val = vbuff2[y][x];
            vbuff2[y][x] = vbuff2[y + 1][x];
            vbuff2[y + 1][x] = val;
            y++;
        }
    }
}

void oilTransfer()
{
    const int16_t n = 3;
    int16_t x, y, dx, dy, mfp;
    int16_t histo[256] = {0};

    for (y = 0; y < 200; y++)
    {
        for (x = 0; x < 200; x++)
        {
            memset(histo, 0, 256 * sizeof(int16_t));

            for (dy = y - n; dy <= y + n; dy++)
            {
                for (dx = x - n; dx <= x + n; dx++) histo[vbuff1[dy][dx]]++;
            }

            dy = 0;
            for (dx = 0; dx < 256; dx++)
            {
                if (histo[dx] > dy)
                {
                    dy = histo[dx];
                    mfp = dx;
                }
            }

            vbuff2[y][x] = mfp;
        }
    }
}

void trans(uint8_t type)
{
    int16_t x, y, c;
    int16_t a11, a12, a13, a21, a22, a23;
    int16_t a31, a32, a33, fdiv, bias = -1;

    // Soften (Medium)
    //a11 = 1; a12 = 3; a13 = 1;
    //a21 = 3; a22 = 9; a23 = 3;
    //a31 = 1; a32 = 3; a33 = 1; fdiv = 25;

    // Soften (A lot)
    a11 = 2; a12 = 2; a13 = 2;
    a21 = 2; a22 = 2; a23 = 2;
    a31 = 2; a32 = 2; a33 = 2; fdiv = 18;

    // Fire
    //a11 = 0; a12 = 0; a13 = 1;
    //a21 = 1; a22 = 1; a23 = 1;
    //a31 = 0; a32 = 0; a33 = 1; fdiv = 5;

    // Diagonal "Shatter"
    //a11 = 1; a12 = 0; a13 = 1;
    //a21 = 0; a22 = 0; a23 = 0;
    //a31 = 1; a32 = 0; a33 = 1; fdiv = 4;

    // Find Edge
    //a11 = -1; a12 = -1; a13 = -1;
    //a21 = -1; a22 = 4;  a23 = -1;
    //a31 = -1; a32 = -1; a33 = -1; fdiv = -6;

    switch (type)
    {
        case 255:
            for (y = 0; y < 200; y++) for(x = 0; x < 200; x++) vbuff1[y][x] = gray[vbuff1[y][x]]; break;

        case 1:
            for (y = 0; y < 200; y++) for(x = 0; x < 200; x++) vbuff2[y][x] = 63 - vbuff1[y][x]; break;

        case 2:
            for (y = 0; y < 200; y++) for(x = 0; x < 200; x++) vbuff2[y][x] = vbuff1[y][199 - x]; break;

        case 3:
            for (y = 0; y < 200; y++) for(x = 0; x < 200; x++) vbuff2[y][x] = vbuff1[x][199 - y]; break;

        case 4:
            trans(2);
            for (y = 0; y < 200; y++) for(x = 0; x < 200; x++) vbuff2[y][x] = (vbuff1[y][x] + vbuff2[y][x]) >> 1; break;

        case 5:
            for (y = 0; y < 200; y++) for(x = 0; x < 200; x++) vbuff2[y][x] = vbuff1[(int16_t)(y / 1.4)][(int16_t)(x / 1.4)]; break;

        case 6:
            for (y = 0; y < 200; y++) for(x = 2; x < 200; x++) vbuff2[y][x] = vbuff1[y][x + (x % 8) - 4]; break;

        case 7:
            for (y = 0; y < 200; y++) for(x = 0; x < 200; x++) vbuff2[y][x] = vbuff1[(int16_t)sqrt(y * 199.0)][x]; break;

        case 8:
            for (y = 0; y < 200; y++) for(x = 0; x < 200; x++) vbuff2[y][x] = vbuff1[(y >> 2) << 2][(x >> 2) << 2]; break;

        case 9:
            trans(0);
            for (y = 0; y < 199; y++) for(x = 1; x <= 199; x++) vbuff2[y][x] = 63 - (vbuff2[y][x] - vbuff2[y + 1][x - 1] + 31); break;

        case 0:
            for (y = 1; y < 199; y++)
            {
                for (x = 1; x < 199; x++)
                {
                    vbuff2[y][x] = (
                        a11 * vbuff1[y - 1][x - 1] + a12 * vbuff1[y - 1][x] + a13 * vbuff1[y - 1][x + 1] + 
                        a21 * vbuff1[y][x - 1] + a22 * vbuff1[y][x] + a23 * vbuff1[y][x + 1] + 
                        a31 * vbuff1[y + 1][x - 1] + a32 * vbuff1[y + 1][x] + a33 * vbuff1[y + 1][x + 1]
                    ) / fdiv + bias;
                }
            }
            break;

        case 57: flip();
    }
}

void main()
{
    int16_t i;
    FILE *fp;

    clearTextMem();
    printStr(1, 1, 0x0F, "Image Filter - (c) 1998 by Nguyen Ngoc Van");
    printStr(1, 2, 0x07, "Control Keys");
    printStr(1, 3, 0x07, "A -> M    : switch effects");
    printStr(1, 4, 0x07, "Space Bar : restore image");
    printStr(1, 5, 0x07, "ESC       : goodbye");
    printStr(1, 6, 0x07, "Press a key to start demo");

    __asm {
    next:
        xor     ah, ah
        int     0x16
        and     al, al
        jz      next
    }

    old09 = _dos_getvect(0x09);
    _dos_setvect(0x09, new09);

    srand(time(NULL));

    __asm {
        mov     ax, 0x13
        int     0x10
    }

    fp = fopen("dracula.cel", "rb");
    if (!fp) return;

    fseek(fp, 32, SEEK_SET);
    fread(pal, 768, 1, fp);
    fread(vbuff1[0], 40000, 1, fp);
    fclose(fp);
    calcGray();

    __asm {
        mov     dx, 0x03C8
        xor     al, al
        out     dx, al
        inc     dx
        mov     cx, 256
    next:
        out     dx, al
        out     dx, al
        out     dx, al
        inc     al
        loop    next
    }

    trans(255);
    flip();

    do {
        if (kbarr[0x1E]) trans(1);
        if (kbarr[0x30]) trans(2);
        if (kbarr[0x2E]) trans(3);
        if (kbarr[0x20]) trans(4);
        if (kbarr[0x12]) trans(5);
        if (kbarr[0x21]) trans(6);
        if (kbarr[0x22]) trans(7);
        if (kbarr[0x23]) trans(8);
        if (kbarr[0x17]) trans(9);
        if (kbarr[0x24]) trans(0);
        if (kbarr[0x25]) shear();
        if (kbarr[0x26]) melting();
        if (kbarr[0x32]) oilTransfer();
        if (kbarr[0x39]) trans(57);
        drawPic();
    } while (inp(0x60) != 1);
    
    __asm {
        mov     ax, 0x03
        int     0x10
    }

    _dos_setvect(0x09, old09);
}
