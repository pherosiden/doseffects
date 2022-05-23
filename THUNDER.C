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

typedef struct {
    uint8_t r, g, b;
} RGB;

RGB dst[256] = {0};
RGB src[256] = {0};

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
        test    al, 0x08
        jnz     waitH
    waitV:
        in      al, dx
        test    al, 0x08
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

void movePal(RGB *dst, RGB *src)
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

    while (ints > 0 && y < 200 && x > 0 && x < 320)
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
            if (dst[i].r > 0) dst[i].r--;
            if (dst[i].g > 0) dst[i].g--;
            if (dst[i].b > 0) dst[i].b--;

            if (inp(0x60) == 1) return;
        }
        
        retrace();
        setPal();
    }
}

void thunderBolt()
{
    int16_t dx;

    do {
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
    } while(!kbhit());
}

void main()
{
    int16_t i;
    
    clearTextMem();
    printStr(1, 1, 0x0F, "THUNDER BOLT - (c) 1998 by Nguyen Ngoc Van");
    printStr(1, 2, 0x07, "Press any key to continue ...");
    getch();
    srand(time(NULL));

    __asm {
        mov     ax, 0x13
        int     0x10
    }

    for (i = 0; i < 64; i++)
    {
        dst[i      ].r = 0;  dst[i      ].g = 0;  dst[i      ].b = i;
        dst[i +  64].r = 0;  dst[i +  64].g = i;  dst[i +  64].b = 63;
        dst[i + 128].r = i;  dst[i + 128].g = 63; dst[i + 128].b = 63;
        dst[i + 192].r = 63; dst[i + 192].g = 63; dst[i + 192].b = 63;
    }
    
    movePal(src, dst);
    thunderBolt();

    __asm {
        mov     ax, 0x03
        int     0x10
    }
}
