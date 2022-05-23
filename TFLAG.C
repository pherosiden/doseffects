/*----------------------------------------------------*/
/* Packet  : Demo & Effect                            */
/* Effect  : Flake                                    */
/* Author  : Nguyen Ngoc Van                          */ 
/* Memory  : Compact                                  */
/* Address : pherosiden@gmail.com                     */
/* Website : http://www.codedemo.net                  */ 
/* Created : 01/04/1998                               */
/* Please sent to me any bugs or suggests.            */ 
/* You can use freely this code. Have fun :)          */
/*----------------------------------------------------*/

#include <dos.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <conio.h>
#include <string.h>

#define WIDTH	180
#define HEIGHT	100
#define FASE	2
#define WAVEX	2
#define WAVEY	3

#define KX      (FASE * 2 * 3.141592f / WIDTH)
#define BLUE    (127 - 20)
#define GULT    (63 - 20)
#define VITT    (255 - 20)
#define RED     (191 - 20)

int16_t sintab[4 * WIDTH] = {0};
uint8_t pal[768] = {0};
uint8_t flagbuff[WIDTH * HEIGHT] = {0};
uint8_t vbuff[(WIDTH + 40) * (HEIGHT + 40)];
uint8_t chartab[64 * 60] = {0};
uint8_t *vmem = (uint8_t*)0xA0000000L;

void getBIOSChar(char col)
{
    char color, number;

    __asm {
        lea     di, chartab
        mov     ax, 0xF000
        mov     ds, ax
        mov     si, 0xFA6E
        mov     ax, 32
        shl     ax, 3
        add     si, ax
        mov     number, 59
        xor     ah, ah
    charslp:
        mov     dx, 8
        mov     bl, col
        mov     color, bl
    charlp:
        lodsb
        sub     color, 3
        mov     bl, al
        mov     cx, 8
    rowlp:
        mov     al, color
        shl     bl, 1
        jc      carry
        xor     al, al
    carry:
        stosb
        loop    rowlp
        dec     dx
        jnz     charlp
        dec     number
        jnz     charslp
    }
}

void eraseChar(int16_t x, int16_t y)
{
    __asm {
        les     di, vmem
        mov     bx, y
        mov     di, bx
        xchg    bh, bl
        shl     di, 6
        add     di, bx
        add     di, x
        xor     eax, eax
        mov     dx, 8
    next:
        mov     cx, 2
        rep     stosd
        add     di, 320 - 8
        dec     dx
        jnz     next
    }
}

void showChar(int16_t x, int16_t y, char chr)
{
    __asm {
        les     di, vmem
        mov     bx, y
        mov     di, bx
        xchg    bh, bl
        shl     di, 6
        add     di, bx
        add     di, x
        lea     si, chartab
        mov     al, chr
        xor     ah, ah
        sub     ax, 32
        shl     ax, 6
        add     si, ax
        mov     dx, 8
    showch:
        mov     cx, 2
        rep     movsd
        add     di, 320 - 8
        dec     dx
        jnz     showch
    }
}

void showString(int16_t x, int16_t y, char *msg)
{
    while (*msg++)
    {
        showChar(x, y, *msg);	
        x += 8;
    }
}

void charOnFlag(int16_t x, int16_t y, char *flag, char chr)
{
    __asm {
        les     di, flag
        mov     ax, y
        mov     bx, WIDTH
        mul     bx
        add     ax, x
        add     di, ax
        lea     si, chartab
        mov     al, chr
        xor     ah, ah
        sub     ax, 32
        shl     ax, 6
        add     si, ax
        mov     dx, 8
    lp1:
        mov     cx, 8
    lp2:
        lodsb
        cmp     al, 0
        jz      lp3
        mov     es:[di], al
    lp3:
        inc     di
        loop    lp2
        add     di, WIDTH - 8
        dec     dx
        jnz     lp1
    }
}

void retrace()
{
    __asm {
        mov     dx, 0x03DA
    waitV:
        in      al, dx
        test    al, 0x08
        jnz     waitV
    waitH:
        in      al,dx
        test    al, 0x08
        jz      waitH
    }
}

void setPalette(uint8_t *pal)
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

void clearPalette()
{
    __asm {
        mov     dx, 0x03C8
        xor     al, al
        out     dx, al
        inc     dx
        mov     cx, 768
    next:
        out     dx, al
        loop    next
    }
}

void fadeIn(uint8_t *pal)
{
    int16_t i, j;
    uint8_t tmp[768] = {0};

    for (j = 0; j < 64; j++)
    {
        outp(0x03C8, 0);
        retrace();

        for (i = 0; i < 768; i++)
        {
            outp(0x3C9, tmp[i]);
            if (tmp[i] < pal[i]) tmp[i]++;
        }
    }
}

void fadeOut(uint8_t *pal)
{
    int16_t i, j;

    for (j = 0; j < 64; j++)
    {
        outp(0x03C8, 0);
        retrace();

        for (i = 0; i < 768; i++)
        {
            outp(0x3C9, pal[i]);
            if (pal[i] > 0) pal[i]--;
        }
    }
}

void fadeMax(uint8_t *pal)
{
    int16_t i, j;

    for (j = 0; j < 64; j++)
    {
        outp(0x03C8, 0);
        if (!(i % 2)) retrace();

        for (i = 0; i < 768; i++)
        {
            outp(0x3C9, pal[i]);
            if (pal[i] < 63) pal[i]++;
        }
    }
}

void coolPal(uint8_t *pal)
{
    int16_t i, r = 0, g = 0, b = 0, col = 0;

    for (i = 0; i < 64; i++)
    {
        pal[col++] = r;
        pal[col++] = g;
        pal[col++] = b;

        if (r < 63) r++;
        if (g < 63) g++;
    }

    r = 0;
    g = 0;

    for (i = 0; i < 64; i++)
    {
        pal[col++] = r;
        pal[col++] = g;
        pal[col++] = b;

        if (b < 63) b++;
    }

    b = 0;

    for (i = 0; i < 64; i++)
    {
        pal[col++] = r;
        pal[col++] = g;
        pal[col++] = b;

        if (r < 63) r++;
    }

    r = 0;

    for (i = 0; i < 64; i++)
    {
        pal[col++] = r;
        pal[col++] = g;
        pal[col++] = b;

        if (r < 63) r++;
        if (g < 63) g++;
        if (b < 63) b++;
    }

    setPalette(pal);
}

void putPixel(int16_t x, int16_t y, uint8_t col)
{
    __asm {
        les     di, vmem
        add     di, x
        mov     bx, y
        shl     bx, 6
        add     bh, byte ptr y
        add     di, bx
        mov     al, col
        stosb
    }
}

void drawFlag(char *flag, int16_t px, int16_t py)
{
    int16_t x, y;
    for (x = 0; x < WIDTH; x++)
    {
        for (y = 0; y < HEIGHT; y++) putPixel(px + x, py + y, flag[WIDTH * y + x]);
    }
}

void setMode(int16_t mode)
{
    __asm {
        mov     ax, mode
        int     0x10
    }
}

void Trikolor(char *flag)
{
    int16_t x, y;

    for (x = 0; x < WIDTH; x++)
    {
        for (y = 0; y < HEIGHT / 3; y++) flag[WIDTH * y + x] = BLUE;
    }

    for (x = 0; x < WIDTH; x++)
    {
        for (y = HEIGHT / 3; y < HEIGHT; y++) flag[WIDTH * y + x] = VITT;
    }

    for (x = 0; x < WIDTH; x++)
    {
        for (y = 2 * HEIGHT / 3; y < HEIGHT; y++) flag[WIDTH * y + x] = RED;
    }
}

void SvenskFlag(char *flag)
{
    int16_t x, y;

    for (x = 0; x < WIDTH / 4 + WIDTH / 40; x++)
    {
        for (y = 0; y < HEIGHT; y++) flag[WIDTH * y + x] = BLUE;
    }

    for (x = WIDTH / 4 + WIDTH / 40; x < WIDTH / 3 + WIDTH / 15; x++)
    {
        for (y = 0; y < HEIGHT; y++) flag[WIDTH * y + x] = GULT;
    }

    for (x = WIDTH / 3 + WIDTH / 15; x < WIDTH; x++)
    {
        for (y = 0; y < HEIGHT; y++) flag[WIDTH * y + x] = BLUE;
    }

    for (x = 0; x < WIDTH; x++)
    {
        for (y = HEIGHT / 2 - HEIGHT / 9; y < HEIGHT / 2 + HEIGHT / 9; y++) flag[WIDTH * y + x] = GULT;
    }
}

void main()
{
    int16_t x, y, px, py, pz, fo, bar, tmp, idx;
    int16_t tx, ty, t2x, t2y, flagaddr;

    uint8_t *flag = flagbuff;
    uint8_t *dbuff = vbuff;
    
    const char *text1 = "(C) 1998 BY NGUYEN NGOC VAN";
    const char *text2 = "FLY THE FLAG";

    getBIOSChar(255 - 20);

    ty = 24;
    tx = (320 - (strlen(text1) << 3)) >> 1;
    
    t2y = 190;
    t2x = 15 + (320 - (strlen(text2) << 4)) >> 1;
    
    setMode(0x13);

    for (x = 0; x < WIDTH; x++)
    {
        sintab[x] = 0;
        sintab[WIDTH + x] = sin(KX * x) * 20;
        sintab[2 * WIDTH + x] = sintab[WIDTH + x];
        sintab[3 * WIDTH + x] = sintab[WIDTH + x];
    }

    coolPal(pal);
    Trikolor(flag);
    charOnFlag(10, 46, flag, 'H');
    charOnFlag(20, 46, flag, 'E');
    charOnFlag(30, 46, flag, 'L');
    charOnFlag(40, 46, flag, 'L');
    charOnFlag(50, 46, flag, 'O');
    charOnFlag(70, 46, flag, 'W');
    charOnFlag(80, 46, flag, 'O');
    charOnFlag(90, 46, flag, 'R');
    charOnFlag(100, 46, flag, 'L');
    charOnFlag(110, 46, flag, 'D');

    clearPalette();
    drawFlag(flag, 160 - WIDTH / 2, 70);
    fadeIn(pal);
    idx = 0;

    do {
        __asm {
            les     di, dbuff
            mov     cx, (WIDTH + 40) * (HEIGHT + 40) / 4
            xor     eax, eax
            rep     stosd
            lds     si, flag
            mov     tmp, si
            les     di, dbuff
            add     di, (WIDTH + 40) * 20 + 20
            mov     bar, di
        }

        for (x = 0; x < WIDTH; x++)
        {
            py = sintab[idx + x];
            pz = sintab[idx + x + WIDTH / (2 * FASE)];
            fo = (WIDTH + 40) * (pz >> WAVEY);

            __asm {
                mov     di, bar
                add     di, x
                add     di, fo
                mov     si, tmp
                add     si, x
                mov     cx, HEIGHT
                mov     bx, py
            next:
                lodsb
                add     ax, bx
                stosb
                add     si, (WIDTH - 1)
                add     di, (WIDTH + 40 - 1)
                loop    next
            }
        }

        __asm {
            lds     si, dbuff
            add     si, 20
            mov     di, si
        }

        for (y = 0; y < HEIGHT + 40; y++)
        {
            px = sintab[idx + y] >> WAVEX;

            if (px < 0)
            {
                __asm {
                    cld
                    add     di, px
                    mov     cx, (WIDTH + 10) / 4
                    rep     movsd
                    add     si, 30
                    mov     di, si
                }
            }

            if (px > 0)
            {
                __asm {
                    std
                    add     si, WIDTH
                    mov     di, si
                    add     di, px
                    mov     cx, (WIDTH + 10) / 4
                    rep     movsd
                    add     si, WIDTH + 50
                    mov     di, si
                    cld
                }
            }

            if (!px)
            {
                __asm {
                    add     si, WIDTH + 40
                    mov     di, si
                }
            }
        }

        idx++;

        if (idx == 2 * WIDTH) idx = WIDTH;
        retrace();

        for (x = 0; text1[x]; x++)
        {
            eraseChar(tx + (x << 3), ty + sintab[idx + x]);
            showChar(tx + (x << 3), ty + sintab[idx + x + 1], text1[x]);
        }

        for (x = 0; text2[x]; x++)
        {
            eraseChar(t2x + (x << 4) + (sintab[idx + x]), t2y);
            showChar(t2x + (x << 4) + (sintab[idx + x + 1]), t2y, text2[x]);
        }

        __asm {
            les     di, vmem
            add     di, (320 - WIDTH - 40) / 2 + 320 * 50
            lds     si, dbuff
            mov     dx, HEIGHT + 40
        next:
            mov     cx, (WIDTH + 40) / 4
            rep     movsd
            add     di, 320 - WIDTH - 40
            dec     dx
            jnz     next
        }
    } while(!kbhit());

    fadeMax(pal);
    fadeOut(pal);
    setMode(3);
}
