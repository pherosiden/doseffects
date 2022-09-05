/*----------------------------------------------------*/
/* Packet  : Demo & Effect                            */
/* Effect  : Star Field 2D                            */
/* Author  : Nguyen Ngoc Van                          */
/* Memory  : Small                                    */
/* Address : pherosiden@gmail.com                     */
/* Website : http://www.codedemo.net                  */
/* Created : 12/03/1998                               */
/* Please sent to me any bugs or suggests.            */
/* You can use freely this code. Have fun :)          */
/*----------------------------------------------------*/

#include <dos.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <conio.h>

#define MAXSTARS 200
#define MAXSPEED 4

uint16_t star[MAXSTARS] = {0};
uint16_t speed[MAXSTARS] = {0};

uint8_t *vmem = (uint8_t*)0xA0000000L;
uint8_t *tmem = (uint8_t*)0xB8000000L;

inline int16_t random(int16_t x)
{
    if (x == 0) return 0;
    return rand() % x;
}

uint8_t getPixel(uint16_t ofs)
{
    __asm {
        lds     si, vmem
        add     si, ofs
        lodsb
    }
}

void setPixel(uint16_t ofs, uint8_t col)
{
    __asm {
        les     di, vmem
        add     di, ofs
        mov     al, col
        stosb
    }
}

void createStar()
{
    int16_t i;

    srand(time(NULL));

    for (i = 0; i < MAXSTARS; i++) 
    {
        star[i] = random(320) + random(200) * 320;
        speed[i] = random(MAXSPEED) + 1;
        if (getPixel(star[i]) == 0) setPixel(star[i], 100);
    }
}

void moveStar()
{
    int16_t i;
    uint16_t n;

    for (i = 0; i < MAXSTARS; i++)
    {
        n = getPixel(star[i]);
        if (n == 100) setPixel(star[i], 0);
        else
        {
            star[i] += speed[i];
            n = star[i] - (star[i] / 320) * 320;
            if (n >= 320) star[i] -= 320;
            else
            {
                n = getPixel(star[i]);
                if (n == 0) setPixel(star[i], 100);
            }
        }
    }
}

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

void clearScreen()
{
    __asm {
        les     di, tmem
        xor     ax, ax
        mov     cx, 2000
        rep     stosw
    }
}

void main()
{
    clearScreen();
    printStr(1, 1, 0x0F, "START 2D - (c) 1998 by Nguyen Ngoc Van");
    printStr(1, 2, 0x07, "Press any key to start demo...");
    getch();

    __asm {
        mov     ax, 0x13
        int     0x10
    }

    createStar();

    while (!kbhit())
    {
        waitRetrace();
        moveStar();
    }
    
    __asm {
        mov     ax, 0x03
        int     0x10
    }
}
