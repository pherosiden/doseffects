/*----------------------------------------------------*/
/* Packet  : Demo & Effect                            */
/* Effect  : Plasma                                   */
/* Author  : Nguyen Ngoc Van                          */
/* Memory  : Small                                    */
/* Heaps   : 64K                                      */
/* Address : pherosiden@gmail.com                     */ 
/* Website : http://www.codedemo.net                  */
/* Created : 19/02/1998                               */
/* Please sent to me any bugs or suggests.            */ 
/* You can use freely this code. Have fun :)          */
/*----------------------------------------------------*/

#include <dos.h>
#include <time.h>
#include <math.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <conio.h>

#define M_PI 3.141592f

uint8_t fx[320] = {0};
uint8_t fy[200] = {0};
uint8_t pal[768] = {0};

uint8_t *vmem = (uint8_t*)0xA0000000L;
uint8_t *tmem = (uint8_t*)0xB8000000L;

void pokeb(int16_t x, int16_t y, uint8_t col)
{
    vmem[(y << 8) + (y << 6) + x] = col;
}

void waitRetrace()
{
    __asm {
        mov     dx, 0x03DA
    waitV:
        in      al, dx
        test    al, 0x08
        jnz     waitV
    waitH:
        in      al, dx
        test    al, 0x08
        jz      waitH
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

void setPal()
{
    int16_t x, y;

    for (x = 0; x <= 63; x++)
    {
        y = 3 * x;

        pal[y + 0] = 63;         pal[y + 192] = 63 - x;
        pal[y + 1] = 63 - x;     pal[y + 193] = x;
        pal[y + 2] = 0;          pal[y + 194] = 0;

        pal[y + 384] = 0;        pal[y + 576] = x;
        pal[y + 385] = 63 - x;   pal[y + 577] = x;
        pal[y + 386] = x;        pal[y + 578] = 63 - x;
    }

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

void bobPlasma(int16_t *bobx, int16_t *boby)
{
    int16_t x, y, i;

    x = rand() % 2;
    y = rand() % 2;

    if (!x) (*bobx)++; else (*bobx)--;
    if (!y) (*boby)++; else (*boby)--;

    for (x = *bobx; x <= *bobx + 10; x++)
    {
        y = *boby;

        for (i = 0; i < 10; i++)
        {
            y++;
            vmem[(y << 8) + (y << 6) + x]++;
        }
    }
}

void moveData(uint8_t *dst, uint8_t *src, uint16_t len)
{
    __asm {
        lds     si, src
        les     di, dst
        mov     cx, len
        mov     ax, cx
        shr     cx, 2
        rep     movsd
        mov     cl, al
        and     cl, 0x03
        rep     movsb
    }
}

void cyclepallete()
{
    uint8_t tmp[3] = {0};

    moveData(tmp, pal, 3);
    moveData(pal, &pal[3], 765);
    moveData(&pal[765], tmp, 3);
    waitRetrace();

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

void main()
{
    int16_t x, y;
    int16_t bob1x, bob1y;
    int16_t bob2x, bob2y;

    clearScreen();
    printStr(1, 1, 0x0F, "PLASMA DEMO - (c) 1998 by Nguyen Ngoc Van");
    printStr(1, 2, 0x07, "Press any key to start demo...");
    getch();

    bob1x = 200;
    bob1y = 70;
    bob2x = 230;
    bob2y = 130;

    srand(time(NULL));

    __asm {
        mov     ax, 0x13
        int     0x10
    }

    for (x = 0; x <= 319; x++) fx[x] = (cos(M_PI * x / 160.0) + 1) * 127.5;
    for (x = 0; x <= 199; x++) fy[x] = (sin(M_PI * x / 100.0) + 1) * 127.5;

    for (y = 0; y <= 199; y++)
    {
        for (x = 0; x <= 319; x++) pokeb(x, y, fx[x] + fy[y]);
        if (y + 2 <= 199)
        {
            for (x = 0; x <= 319; x++) pokeb(x, (y + 2), y);
        }
    }

    setPal();

    do {
        cyclepallete();
        bobPlasma(&bob1x, &bob1y);
        bobPlasma(&bob2x, &bob2y);
    } while (!kbhit());

    __asm {
        mov     ax, 0x03
        int     0x10
    }
}
