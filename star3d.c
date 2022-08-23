/*----------------------------------------------------*/
/* Packet  : Demo & Effect                            */ 
/* Effect  : Star Field 3D                            */
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
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <conio.h>

uint8_t *vmem = (uint8_t*)0xA0000000L;
uint8_t *tmem = (uint8_t*)0xB8000000L;

typedef struct {
    int16_t x, y, z;
} Point3D;

typedef struct {
    int16_t x, y;
} Point2D;

Point3D stars[1000] = {0};
Point2D pos[1000] = {0};

uint8_t kbarr[128] = {0};
uint8_t density, direction;
uint8_t bounce, bx, by;

int16_t speed, xofs, yofs;

void (interrupt *old09)();

void interrupt new09()
{
    uint8_t chr = inp(0x60);
    if (chr > 128) kbarr[chr - 128] = 0;
    else kbarr[chr] = 1;

    __asm {
        mov     al, 0x20
        out     0x20, al
    }
}

inline int16_t random(int16_t x)
{
    if (x == 0) return 0;
    return rand() % x;
}

void waitRetrace()
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

void putPixel(uint16_t x, uint16_t y, uint8_t col)
{
    __asm {
        les     di, vmem
        mov     bx, y
        shl     bx, 6
        add     bh, byte ptr y
        add     bx, x
        add     di, bx
        mov     al, col
        stosb
    }
}

void setDAC(uint8_t col, uint8_t r, uint8_t g, uint8_t b)
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

void putMsg(int16_t x, int16_t y, uint8_t col, char *msg)
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

void clearScreen()
{
    __asm {
        les     di, tmem
        xor     ax, ax
        mov     cx, 2000
        rep     stosw
    }
}

void initStars()
{
    int16_t i;

    for (i = 0; i < 1000; i++)
    {
        do
        {
            stars[i].x = random(320) - 160;
            stars[i].y = random(200) - 100;
            stars[i].z = i + 1;
        } while (!stars[i].x || !stars[i].y);
    }
}

void calcStars()
{
    int16_t i;

    for (i = 0; i < 1000; i++)
    {
        pos[i].x = ((stars[i].x << density) / stars[i].z) + xofs;
        pos[i].y = ((stars[i].y << density) / stars[i].z) + yofs;
    }
}

void drawStars()
{
    int16_t i;

    for (i = 0; i < 1000; i++)
    {
        if (pos[i].x > 0 && pos[i].x < 320 && pos[i].y > 0 && pos[i].y < 200)
        {
            if (stars[i].z < 100) putPixel(pos[i].x, pos[i].y, 4);
            else if (stars[i].z < 200) putPixel(pos[i].x, pos[i].y, 3);
            else if (stars[i].z < 300) putPixel(pos[i].x, pos[i].y, 2);
            else if (stars[i].z < 400) putPixel(pos[i].x, pos[i].y, 1);
        }
    }
}

void clearStars()
{
    int16_t i;

    for (i = 0; i < 1000; i++)
    {
        if (pos[i].x > 0 && pos[i].x < 320 && pos[i].y > 0 && pos[i].y < 200) putPixel(pos[i].x, pos[i].y, 0);
    }
}

void moveStars(uint8_t how)
{
    int16_t i;

    for (i = 0; i < 1000; i++)
    {
        if (how)
        {
            stars[i].z -= speed;
            if (stars[i].z < 1) stars[i].z = 1000 - 1;
        }
        else
        {
            stars[i].z += speed;
            if (stars[i].z > 1000 - 1) stars[i].z = 1;
        }
    }
}

void bounceStars()
{
    if (bx) xofs += speed; else xofs -= speed;
    if (by) yofs += speed; else yofs -= speed;
    if (xofs < 1 || xofs > 319) bx = !bx;
    if (yofs < 1 || yofs > 199) by = !by;
}

void main()
{
    clearScreen();
    putMsg(1, 1, 0x0F, "Star Field 3D - (c) 1998 by Nguyen Ngoc Van");
    putMsg(1, 2, 0x0F, "Control Keys :");
    putMsg(1, 3, 0x07, "+/-       : +/- speed");
    putMsg(1, 4, 0x07, ">/<       : +/- density");
    putMsg(1, 5, 0x07, "Arrows    : Moves the starfield");
    putMsg(1, 6, 0x07, "B         : Toggles bouncing");
    putMsg(1, 7, 0x07, "Space Bar : Changes direction");
    putMsg(1, 8, 0x07, "Press any key to start...");

    __asm {
    next:
        xor     ah, ah
        int     0x16
        and     al, 0
        jnz     next
    }

    old09 = _dos_getvect(0x09);
    _dos_setvect(0x09, new09);

    __asm {
        mov     ax, 0x13
        int     0x10
    }

    srand(time(NULL));
    initStars();

    setDAC(1, 0, 0, 30);
    setDAC(2, 10, 10, 40);
    setDAC(3, 20, 20, 50);
    setDAC(4, 30, 30, 60);

    direction = 1;
    speed = 2;
    density = 7;
    xofs = 160;

    yofs = 100;
    bounce = 0;
    bx = 1;
    by = 0;

    do {
        calcStars();
        drawStars();
        waitRetrace();
        clearStars();
        moveStars(direction);

        if (bounce) bounceStars();

        if (kbarr[78])
        {
            if (speed < 30) speed++;
            kbarr[78] = 0;
        }

        if (kbarr[74])
        {
            if (speed > 0) speed--;
            kbarr[74] = 0;
        }

        if (kbarr[52]) if (density < 9) density++;
        if (kbarr[51]) if (density > 6) density--;
        if (kbarr[75]) if (xofs > 0) xofs--;
        if (kbarr[77]) if (xofs < 319) xofs++;
        if (kbarr[72]) if (yofs > 0) yofs--;
        if (kbarr[80]) if (yofs < 199) yofs++;

        if (kbarr[48])
        {
            bounce = !bounce;
            kbarr[48] =0;
        }

        if (kbarr[57])
        {
            direction = !direction;
            kbarr[57] = 0;
        }
    } while (kbarr[1] != 1);

    __asm {
        mov     ax, 0x03
        int     0x10
    }

    _dos_setvect(0x09, old09);
}
