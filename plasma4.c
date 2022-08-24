/*----------------------------------------------------*/
/* Packet  : Demo & Effect                            */
/* Effect  : drawPlasma                                   */
/* Author  : Nguyen Ngoc Van                          */ 
/* Memory  : Small                                    */
/* Heaps   : 64K                                      */
/* Address : pherosiden@gmail.com                     */
/* Website : http://www.codedemo.net                  */  
/* Created : 21/02/1998                               */
/* Please sent to me any bugs or suggests.            */
/* You can use freely this code. Have fun :)          */
/*----------------------------------------------------*/

#include <dos.h>
#include <math.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <conio.h>

#define M_PI 3.141592f

uint8_t p1, p2, q1, q2;
uint8_t sintab[256] = {0};

inline int16_t roundf(float x)
{
    if (x >= 0.0) return x + 0.5;
    return x - 0.5;
}

void clearScreen()
{
    __asm {
        mov     ax, 0xb800
        mov     es, ax
        xor     di, di
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
        mov     ax, 0xb800
        mov     es, ax
        xor     di, di
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

void setupSinus()
{
    int16_t i;
    float v = 0.0, vadd;

    vadd = 2.0 * M_PI / 256.0;
    for (i = 0; i <= 255; i++) sintab[i] = roundf(sin(v + i * vadd) * 63) + 64;
}

void setRGB(uint8_t col, uint8_t r, uint8_t g, uint8_t b)
{
    __asm {
        mov     dx, 0x03C8
        mov     al, col
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

void setColors()
{
    int16_t i;

    for (i = 0; i <= 63; i++)
    {
        setRGB(i, i, 0, 0);
        setRGB(i +  64, 63 - i, 0, 0);
        setRGB(i + 128, i, 0, i);
        setRGB(i + 192, 63 - i, 0, 63 - i);
    }
}

void drawPlasma(uint16_t where)
{
    uint8_t temp;
    uint16_t ypos;

    __asm {
        mov     al, p1
        mov     q1, al
        mov     al, p2
        mov     q2, al
        mov     es, where
        xor     di, di
        lea     bx, sintab
        xor     ax, ax
        mov     ypos, 200
    yloop:
        mov     al, q1
        xlat
        mov     dl, al
        mov     al, q2
        xlat
        mov     dh, al
        mov     cx, 320
    xloop:
        mov     al, dl
        xlat
        mov     ah, al
        mov     al, dh
        xlat
        add     al, ah
        stosb
        add     dl, 2
        dec     dh
        loop    xloop
        sub     q1, 2
        add     q2, 1
        dec     ypos
        jnz     yloop
    }
}

void changeAngle()
{
      p1++;
      p2 -= 3;
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

void main()
{
    clearScreen();
    printStr(1, 1, 0x0F, "PLASMA DEMO - (c) 1998 by Nguyen Ngoc Van");
    printStr(1, 2, 0x07, "Press any key to start demo...");
    getch();

    __asm {
        mov     ax, 0x13
        int     0x10
    }

    setupSinus();
    setColors();

    p1 = 187;
    p2 = 230;

    while (!kbhit())
    {
        waitRetrace();
        drawPlasma(0xA000);
        changeAngle();
    }

    __asm {
        mov     ax, 0x03
        int     0x10
    }
}
