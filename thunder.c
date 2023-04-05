/*----------------------------------------------------*/
/* Packet  : Demo & Effect                            */
/* Effect  : Thunder Bolt                             */
/* Author  : Nguyen Ngoc Van                          */
/* Memory  : Compact                                  */
/* Heaps   : 640K                                     */
/* Address : pherosiden@gmail.com                     */
/* Website : http://www.codedemo.net                  */
/* Created : 10/03/1998                               */
/* Please sent to me any bugs or suggests.            */
/* You can use freely this code. Have fun :)          */
/*----------------------------------------------------*/
#include <dos.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <conio.h>

uint8_t dst[256][3] = {0};
uint8_t src[256][3] = {0};
uint8_t *vmem = (uint8_t*)0xA0000000L;
uint8_t *tmem = (uint8_t*)0xB8000000L;

inline int16_t random(int16_t x)
{
    if (x == 0) return 0;
    return rand() % x;
}

void setPal()
{
    __asm {
        mov     dx, 0x03C8
        xor     al, al
        out     dx, al
        inc     dx
        lea     si, dst
        mov     cx, 768
        rep     outsb
    }
}

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
        and     al, 0x08
        jz      waitV
    }
}

void setColor(uint8_t n, uint8_t r, uint8_t g, uint8_t b)
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

void clearScr()
{
    __asm {
        les     di, vmem
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

void printStr(int16_t x, int16_t y, uint8_t col, char *msg)
{
    uint16_t len = strlen(msg);

    __asm {
        mov     cx, len
        test    cx, cx
        jz      quit
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
        stosw
        loop    next
    quit:
    }
}

void movePal(void *dst, void *src)
{
    __asm {
        les     di, dst
        lds     si, src
        mov     cx, 192
        rep     movsd
    }
}

void addItem(int16_t x, int16_t y, uint8_t item)
{
    __asm {
        les     di, vmem
        add     di, x
        mov     ax, y
        shl     ax, 6
        add     ah, byte ptr y
        add     di, ax
        mov     al, es:[di]
        cmp     al, item
        jae     quit
        mov     al, item
        stosb
    quit:
    }
}

void processItem(int16_t x, int16_t y, int16_t ints, int16_t dx)
{
    int16_t part, dy;

    while (ints > 0 && y < 190 && x > 10 && x < 310 && !kbhit())
    {
        addItem(x + 1, y, (ints - 3) * 20 + 50);
        addItem(x,     y, (ints * 20) + 50);
        addItem(x - 1, y, (ints - 2) * 20 + 50);

        if ((!random(4) && (y > 170)) || (!random(4) && abs(dx) > 5) || !random(20)) ints--;

        y++;

        if (dx > 0) x -= random(dx);
        else x += random(-dx);
        
        if (abs(dx) < 2)
        {
            x -= random(5) + 1;
            dx = -2;
        }

        if (abs(dx) > 5)
        {
            x += random(5) + 1;
            dx = 2;
        }

        if ((y > 33 && !random(3)) || ints < 4)
        {
            part = random(10);

            do {
                dy = random(10);
            } while (!dy);

            processItem(x, y, (ints * part / 10) - (y / 20), -random(dy));
            processItem(x, y, (ints * (10 - part) / 10) - (y / 20), random(dy));
        }
    }
}

void fadeOut()
{
    int16_t i, j;

    for (j = 63; j >= 0; j--)
    {
        for (i = 0; i <= 255; i++)
        {
            if (dst[i][0] > 0) dst[i][0]--;
            if (dst[i][1] > 0) dst[i][1]--;
            if (dst[i][2] > 0) dst[i][2]--;

            if (kbhit()) return;
        }
        
        retrace();
        setPal();
    }
}

void thunderBolt()
{
    int16_t dx;

    while (!kbhit())
    {
        do {
            dx = random(6) - 3;
        } while(abs(dx) <= 1);

        retrace();
        setPal();
        processItem(random(100) + 110, 0, 10, dx);
        retrace();
        setColor(0, 63, 63, 63);
        fadeOut();
        movePal(dst, src);
        clearScr();
    }
}

void main()
{
    int16_t i;
    
    clearTextMem();
    printStr(1, 1, 0x0F, "THUNDER BOLT - (c) 1998 by Nguyen Ngoc Van");
    printStr(1, 2, 0x07, "Press any key to continue ...");
    getch();

    __asm {
        mov     ax, 0x13
        int     0x10
    }

    memset(src, 0, 768);
    memset(dst, 0, 768);

    for (i = 0; i < 64; i++)
    {
        dst[i      ][0] = 0;  dst[i      ][1] = 0;  dst[i      ][2] = i;
        dst[i +  64][0] = 0;  dst[i +  64][1] = i;  dst[i +  64][2] = 63;
        dst[i + 128][0] = i;  dst[i + 128][1] = 63; dst[i + 128][2] = 63;
        dst[i + 192][0] = 63; dst[i + 192][1] = 63; dst[i + 192][2] = 63;
    }
    
    srand(time(NULL));
    movePal(src, dst);
    thunderBolt();

    __asm {
        mov     ax, 0x03
        int     0x10
    }
}
