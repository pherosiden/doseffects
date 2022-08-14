/*----------------------------------------------------*/
/* Packet  : Demo & Effect                            */
/* Effect  : Fire Work Simulation                     */
/* Author  : Nguyen Ngoc Van                          */
/* Memory  : Compact                                  */
/* Heaps   : 640K                                     */
/* Address : pherosiden@gmail.com                     */
/* Website : http://www.codedemo.net                  */
/* Created : 15/02/1998                               */
/* Please sent to me any bugs or suggests.            */ 
/* You can use freely this code. Have fun :)          */
/*----------------------------------------------------*/

#include <dos.h>
#include <mem.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <conio.h>

#define ANGLE_SCALE         1440
#define XMAX                320
#define NUM_ARROWS          300
#define ARROWS_LENGTH       10
#define EXPLODE_FRAMES      25
#define FADEDOWN_FRAMES     15
#define	PIXEL_CIRCLE        10
#define	NUM_CIRCLE          4
#define	BRIGHT_ARROW        15
#define M_PI                3.14159265f
#define PIXELS_EXPLODE      ((PIXEL_CIRCLE * (NUM_CIRCLE * NUM_CIRCLE) + PIXEL_CIRCLE * NUM_CIRCLE) >> 1)

typedef struct {
    float x, y;
} TPoint;

typedef struct {
    TPoint  shape[ARROWS_LENGTH];
    int8_t  exploded;
    int16_t step;
    int16_t angle;
    int16_t angleAdd;
    uint8_t waitTime;
    int16_t xpos, ypos;
    int16_t color[NUM_CIRCLE];
} TArrow;

typedef struct {
    int16_t x, y;
    uint8_t color, wait;
    uint8_t circle;
} TExplode;

TArrow      *arrows[NUM_ARROWS] = {0};
TPoint      sincos[ANGLE_SCALE] = {0};
TExplode    explodes[PIXEL_CIRCLE * NUM_CIRCLE][PIXELS_EXPLODE] = {0};

uint16_t    cnt = 0;
uint16_t    oldTime = 0;
uint8_t     *tmem = (uint8_t*)0xB8000000L;

void setPixel(int16_t x, int16_t y, uint8_t c)
{
    __asm {
        mov     ax, x
        cmp     ax, 0
        jl      quit
        cmp     ax, 319
        jg      quit
        mov     bx, y
        cmp     bx, 0
        jl      quit
        cmp     bx, 199
        jg      quit
        shl     bx, 6
        add     bh, byte ptr y
        add     bx, ax
        mov     ax, 0xA000
        mov     es, ax
        mov     di, bx
        mov     al, c
        stosb
    quit:
    }
}

inline int16_t random(int16_t rad)
{
    if (rad == 0) return 0;
    return rand() % rad;
}

inline int16_t peek(int16_t seg, int16_t ofs)
{
    return *(uint16_t*)MK_FP(seg, ofs);
}

inline int16_t roundf(float x)
{
    return (int16_t)ceil(x);
}

void newArrow(TArrow *arrow)
{
    int16_t i;

    arrow->exploded = 0;
    arrow->step = random(30) + 64;
    arrow->angle = random(roundf(ANGLE_SCALE / 9.0)) + roundf(ANGLE_SCALE / 5.1428);

    if (arrow->angle < (ANGLE_SCALE >> 2)) arrow->angleAdd = -1;
    else arrow->angleAdd = 1;

    arrow->shape[0].x = random(XMAX / 3) + (XMAX / 3.0);
    arrow->shape[0].y = 220;

    for (i = 1; i < ARROWS_LENGTH; i++)
    {
        arrow->angle += arrow->angleAdd;
        arrow->shape[i] = arrow->shape[i - 1];
        arrow->shape[i].x += sincos[arrow->angle].x;
        arrow->shape[i].y -= sincos[arrow->angle].y;
    }
}

void initExplode(TArrow *arrow)
{
    arrow->step = 0;
    arrow->waitTime = 1;
    arrow->xpos = roundf(arrow->shape[ARROWS_LENGTH - 1].x);
    arrow->ypos = roundf(arrow->shape[ARROWS_LENGTH - 1].y);
    
    switch (random(4))
    {
    case 0:
        arrow->color[0] = 47;
        arrow->color[1] = 79 - 16;
        arrow->color[2] = 143 - 32;
        break;

    case 1:
        arrow->color[0] = 111;
        arrow->color[1] = 79 - 16;
        arrow->color[2] = 143 - 32;
        break;

    case 2:
        arrow->color[0] = 63;
        arrow->color[1] = 47 - 16;
        arrow->color[2] = 111 - 32;
        break;

    case 3:
        arrow->color[0] = 63;
        arrow->color[1] = 127 - 16;
        arrow->color[2] = 63 - 32;
        break;
    }
}

void showArrow(TArrow *arrow, uint8_t from, uint8_t to, char hide)
{
    int16_t i;

    if (hide == 0)
    {
        for (i = from; i < to; i++)
        {
            if (arrow->shape[i].y < 199.5) setPixel(roundf(arrow->shape[i].x), roundf(arrow->shape[i].y), 15 + ARROWS_LENGTH - i);
        }
    }
    else
    {
        for (i = from; i < to; i++)
        {
            if (arrow->shape[i].y < 199.5) setPixel(roundf(arrow->shape[i].x), roundf(arrow->shape[i].y), 0);
        }
    }
}

void showExplode(TArrow *arrow, char hide)
{
    int16_t i;

    if (hide == 0)
    {
        for (i = 0; i < PIXELS_EXPLODE; i++)
        {
            if (arrow->step >= EXPLODE_FRAMES)
            {
                if (random(arrow->step / 10) == 0) setPixel(explodes[arrow->step][i].x + arrow->xpos, explodes[arrow->step][i].y + arrow->ypos, arrow->color[explodes[arrow->step][i].circle] + explodes[arrow->step][i].color);
            }
            else
            {
                setPixel(explodes[arrow->step][i].x + arrow->xpos, explodes[arrow->step][i].y + arrow->ypos, arrow->color[explodes[arrow->step][i].circle] + explodes[arrow->step][i].color);
            }
        }
    }
    else
    {
        for (i = 0; i < PIXELS_EXPLODE; i++)
        {
            setPixel(explodes[arrow->step][i].x + arrow->xpos, explodes[arrow->step][i].y + arrow->ypos, 0);
        }
    }
}

void stepArrow(TArrow *arrow)
{
    memcpy(&arrow->shape[0], &arrow->shape[1], (ARROWS_LENGTH - 1) * sizeof(TPoint));
    arrow->shape[ARROWS_LENGTH - 1] = arrow->shape[ARROWS_LENGTH - 2];
    arrow->angle += arrow->angleAdd;
    arrow->shape[ARROWS_LENGTH - 1].x += sincos[arrow->angle].x * ((arrow->step / 37.0) + 0.75);
    arrow->shape[ARROWS_LENGTH - 1].y -= sincos[arrow->angle].y * ((arrow->step / 37.0) + 0.75);
}

void handleArrow(TArrow *arrow)
{
    int16_t i;

    if (arrow->exploded == 0)
    {
        if (arrow->step > 1)
        {
            arrow->step -= 2;
            stepArrow(arrow);
            showArrow(arrow, 0, 1, 0);
            stepArrow(arrow);
            showArrow(arrow, 0, ARROWS_LENGTH, 0);
        }
        else
        {
            showArrow(arrow, 0, ARROWS_LENGTH, 1);
            initExplode(arrow);
            arrow->exploded = 1;
        }
    }
    else
    {
        showExplode(arrow, 1);
        for (i = 0; i < 2; i++)
        {
            arrow->waitTime--;
            if (arrow->step == EXPLODE_FRAMES + FADEDOWN_FRAMES - 1)
            {
                if (cnt > 997)
                {
                    newArrow(arrow);
                    if (cnt > 1000) cnt -= 1000;
                    else cnt = 0;
                }
                return;
            }

            if (arrow->waitTime == 0)
            {
                arrow->waitTime = explodes[arrow->step][0].wait;
                arrow->step++;
            }
        }

        showExplode(arrow, 0);
    }
}

void initCosSinTable()
{
    int16_t i;
    for (i = 0; i < ANGLE_SCALE; i++)
    {
        sincos[i].x = cos((1.0 * i / ANGLE_SCALE) * 2 * M_PI);
        sincos[i].y = sin((1.0 * i / ANGLE_SCALE) * 2 * M_PI);
    }
}

void initExplodeTable()
{
    float tmp1, tmp2;
    int16_t i, j, k, l, idx;
    
    for (i = 0; i < EXPLODE_FRAMES; i++)
    {
        idx = 0;
        for (j = 1; j <= NUM_CIRCLE; j++)
        {
            for (k = 0; k < j * PIXEL_CIRCLE; k++)
            {
                tmp1 = roundf((1.0 * k) / (1.0 * j * PIXEL_CIRCLE) * ANGLE_SCALE);
                tmp1 += (j - 1.0) * (0.1 * ANGLE_SCALE);

                if (tmp1 > ANGLE_SCALE) tmp1 -= ANGLE_SCALE;
                tmp1 = sincos[roundf(tmp1)].x;

                tmp2 = 0.0;
                for (l = 0; l <= i; l++) tmp2 += tmp1 * ((1.0 * j / NUM_CIRCLE) * 1.5) * (((EXPLODE_FRAMES + 1.0) - l) / (0.8 * EXPLODE_FRAMES));

                explodes[i][idx].x = roundf(tmp2);
                tmp1 = roundf((1.0 * k) / (j * PIXEL_CIRCLE) * ANGLE_SCALE);
                tmp1 += (j - 1.0) * (0.1 * ANGLE_SCALE);

                if (tmp1 > ANGLE_SCALE) tmp1 -= ANGLE_SCALE;
                tmp1 = sincos[roundf(tmp1)].y;

                tmp2 = 0.0;
                for (l = 0; l <= i; l++) tmp2 += tmp1 * ((1.0 * j / NUM_CIRCLE) * 1.5) * (((EXPLODE_FRAMES + 1.0) - l) / (0.8 * EXPLODE_FRAMES));

                explodes[i][idx].y = roundf(tmp2);
                explodes[i][idx].y -= (i >> 2);
                explodes[i][idx].color = (j - 1) * 16 + 1;
                explodes[i][idx].wait = 1;
                explodes[i][idx].circle = j - 1;
                idx++;
            }
        }
    }

    for (i = EXPLODE_FRAMES; i < FADEDOWN_FRAMES + EXPLODE_FRAMES; i++)
    {
        idx = 0;
        for (j = 1; j <= NUM_CIRCLE; j++)
        {
            for (k = 0; k < j * PIXEL_CIRCLE; k++)
            {
                explodes[i][idx].x = explodes[i - 1][idx].x;
                explodes[i][idx].y = explodes[i - 1][idx].y + 1;
                explodes[i][idx].color = explodes[EXPLODE_FRAMES - 1][idx].color + ((15.0 / FADEDOWN_FRAMES) * (1.0 * i - EXPLODE_FRAMES));
                explodes[i][idx].wait = 2;
                explodes[i][idx].circle = j - 1;
                idx++;
            }
        }
    }
}

void setPal(uint8_t col, uint8_t r, uint8_t g, uint8_t b)
{
    __asm {
        mov	    dx, 0x03C8
        mov	    al, col
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

void initPal()
{
    int16_t i;
    const float multiple = (1.0 * BRIGHT_ARROW) / (ARROWS_LENGTH - 2);

    for (i = 0; i <= ARROWS_LENGTH - 2; i++)
    {
        setPal(i + 16, BRIGHT_ARROW - i, (BRIGHT_ARROW - i * multiple), 0);
    }

    setPal(16, (1.0 * BRIGHT_ARROW / 45 * 63), 0, 0);
    setPal(15 + ARROWS_LENGTH, 0, 0, 0);

    for (i = 0; i < 16; i++) setPal(i + 48, (63.0 - i / 15.0 * 63), 0, 0);
    for (i = 0; i < 16; i++) setPal(i + 64, (63.0 - i / 15.0 * 63), 0, (63.0 - i / 15.0 * 63));
    for (i = 0; i < 16; i++) setPal(i + 80, (63.0 - i / 15.0 * 63), (63.0 - i / 15.0 * 63), 0);
    for (i = 0; i < 16; i++) setPal(i + 96, (63.0 - i / 15.0 * 63), (63.0 - i / 15.0 * 63), (63.0 - i / 15.0 * 63));
    for (i = 0; i < 16; i++) setPal(i + 112, 0, (63.0 - i / 15.0 * 63), 0);
    for (i = 0; i < 16; i++) setPal(i + 128, 0, 0, (63.0 - i / 15.0 * 63));
    for (i = 0; i < 16; i++) setPal(i + 144, (63.0 - i / 15.0 * 63), (31.0 - i / 15.0 * 31), 0);
    for (i = 0; i < 16; i++) setPal(i + 160, 0, (31.0 - i / 15.0 * 31), 0);
}

void printStr(int16_t x, int16_t y, uint8_t col, char *msg)
{
    int16_t len = strlen(msg);

    __asm {
        push    ds
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

void main()
{
    int16_t i;
    const uint16_t ARROW_PER_FRAME = roundf((1.0 * NUM_ARROWS / (EXPLODE_FRAMES + FADEDOWN_FRAMES + 80)) * 1000.0);

    clearScreen();
    printStr(1, 1, 0x0F, "FIRE WORK - (c) 1998 by Nguyen Ngoc Van");
    printStr(1, 2, 0x07, "Press any to start demo...");
    getch();

    __asm {
        mov   ax, 0x13
        int   0x10
    }

    srand(time(NULL));
    initCosSinTable();
    initExplodeTable();

    for (i = 0; i < NUM_ARROWS; i++)
    {
        arrows[i] = (TArrow*)calloc(sizeof(TArrow), 1);
        if (!arrows[i])
        {
            printf("Cannot allocate arrow[%d]\n", i);
            exit(1);
        }

        newArrow(arrows[i]);
    }

    initPal();

    do {
        while (peek(0x0040, 0x006C) == oldTime);
        cnt += ARROW_PER_FRAME;
        for (i = 0; i < NUM_ARROWS; i++) handleArrow(arrows[i]);
        if (cnt > 1000) cnt = 0;
        oldTime = peek(0x0040, 0x006C);
    } while (!kbhit());

    __asm {
        mov   ax, 0x03
        int   0x10
    }
    
    for (i = 0; i < NUM_ARROWS; i++) free(arrows[i]);
}
