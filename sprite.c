/*----------------------------------------------------*/
/* Packet  : Demo & Effect                            */
/* Effect  : Sprite                                   */
/* Author  : Nguyen Ngoc Van                          */ 
/* Memory  : Compact                                  */
/* Heaps   : 640K                                     */
/* Address : pherosiden@gmail.com                     */ 
/* Website : http://www.codedemo.net                  */
/* Created : 05/03/1998                               */
/* Please sent to me any bugs or suggests.            */
/* You can use freely this code. Have fun :)          */
/*----------------------------------------------------*/

#include <dos.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <conio.h>
#include <stdlib.h>
#include <stdint.h>

#define W 34
#define H 63
#define D 4
#define B 15

typedef uint8_t FRAME[W * H];

const uint8_t palbubmio[][3] = {
    {0,	24,	0}, {6,	35, 10}, {12, 47, 20}, {26, 16,	0},
    {37, 26, 0}, {29, 28, 24}, {47, 41, 0}, {20, 59, 30},
    {47, 45, 47}, {53, 53, 2}, {57, 24, 24}, {61, 61, 61},
    {63, 33, 33}, {63, 43, 43}, {63, 63, 49}, {63, 63, 63}
};

typedef struct {
    int16_t x, y;
    int16_t speed, frame;
    uint8_t active;
} SPRITE;

uint8_t vbuff1[200][320] = {0};
uint8_t vbuff2[200][320] = {0};

uint8_t *vmem = (uint8_t*)0xA0000000L;
uint8_t *tmem = (uint8_t*)0xB8000000L;

uint8_t backpal[256][3] = {0};
SPRITE bubmio[10] = {0};
FRAME frames[7] = {0};

void waitRetrace()
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

void flip(uint8_t *src, uint8_t *dst)
{
    __asm {
        les     di, dst
        lds     si, src
        mov     cx, 16000
        rep     movsd
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

void printText(int16_t x, int16_t y, uint8_t col, char *msg)
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

void putFrame(uint16_t x, uint16_t y, FRAME sprt, uint8_t *mem)
{
    __asm {
        lds     si, sprt
        les     di, mem
        mov     di, x
        mov     bx, y
        shl     bx, 6
        add     bh, byte ptr y
        add     di, bx
        mov     dl, H
        xor     ch, ch
        mov     cl, W
        cld
        push    ax
        mov     ax, cx
    draw:
        push    di
        mov     cx, ax
    line:
        mov     bl, [si]
        cmp     bl, B
        jnz     store
    nopaint:
        inc     si
        inc     di
        loop    line
        jmp     next
    store:
        movsb
        loop    line
    next:
        pop     di
        dec     dl
        jz      quit
        add     di, 320
        jmp     draw
    quit:
        pop     ax
    }
}

void copyBlock(uint16_t x, uint16_t y)
{
    __asm {
        mov     ax, seg vbuff1
        mov     es, ax
        mov     ax, seg vbuff2
        mov     ds, ax
        mov     di, x
        mov     bx, y
        shl     bx, 6
        add     bh, byte ptr y
        add     di, bx
        mov     si, di
        mov     al, 60
        mov     bx, H
    store:
        mov     cx, W / 2
        rep     movsw
        add     di, 320 - W
        add     si, 320 - W
        dec     bx
        jnz     store
    }
}

void newBubmio()
{
    int16_t i = 0;

    while (i < 10)
    {
        if (!bubmio[i].active)
        {
            bubmio[i].x = W + rand() % 100;
            bubmio[i].y = 0;
            bubmio[i].active = 1;
            bubmio[i].frame = 0;
            bubmio[i].speed = rand() % 6 + 1;
            break;
        }
        i++;
    }
}

void removeBubmio()
{
    int16_t i = 0;
    while (i < 10)
    {
        if (bubmio[i].active)
        {
            copyBlock(bubmio[i].x, bubmio[i].y);
            bubmio[i].active = 0;
            break;
        }
        i++;
    }
}

void jump()
{
    int16_t i, j;
    char key = 0;

    newBubmio();

    do {
        if (kbhit())
        {
            key = getch();
            switch (key)
            {
                case '+': newBubmio(); break;
                case '-': removeBubmio(); break;
            }
        }

        for (i = 0; i <= 9; i++)
        {
            if (bubmio[i].active)
            {
                copyBlock(bubmio[i].x, bubmio[i].y);
                bubmio[i].x += bubmio[i].speed;
                bubmio[i].y += bubmio[i].speed;

                if (bubmio[i].x >= 320 - W || bubmio[i].y >= 200 - H)
                {
                    bubmio[i].active = 0;
                    newBubmio();
                }
            }
        }

        for (i = 0; i <= 9; i++)
        {
            if (bubmio[i].active)
            {
                j = bubmio[i].frame;
                putFrame(bubmio[i].x, bubmio[i].y, frames[j], vbuff1[0]);
                bubmio[i].frame++;
                if (bubmio[i].frame == 7) bubmio[i].frame = 0;
            }
        }

        for (i = 1; i <= D; i++) waitRetrace();
        flip(vbuff1[0], vmem);
    } while(key != 27);
}

void main()
{
    int16_t i;
    FILE *fp;

    clearScreen();
    printText(1, 1, 0x0F, "Sprite - (c) 1998 by Nguyen Ngoc Van");
    printText(1, 2, 0x07, "Use +/- to Add/Remove sprite. ESC to exit");
    printText(1, 3, 0x07, "Press any key to start...");
    getch();

    srand(time(NULL));

    fp = fopen("assets/bubmio.dat", "rb");
    if (!fp) exit(1);

    for (i = 0; i <= 6; i++) fread(frames[i], 1, W * H, fp);
    fclose(fp);

    __asm {
        mov     ax, 0x13
        int     0x10
    }

    for (i = 0; i <= 15; i++) setRGB(i, palbubmio[i][0], palbubmio[i][1], palbubmio[i][2]);

    fp = fopen("assets/fun.cel", "rb");
    if (!fp) return;
    
    fseek(fp, 32, SEEK_SET);
    fread(backpal, 1, 768, fp);

    for (i = 16; i <= 255; i++)	setRGB(i, backpal[i][0], backpal[i][1], backpal[i][2]);
    
    fread(vbuff1[0], 1, 64000, fp);
    fclose(fp);

    flip(vbuff1[0], vmem);
    flip(vbuff1[0], vbuff2[0]);
    jump();

    __asm {
        mov     ax, 0x03
        int     0x10
    }
}
