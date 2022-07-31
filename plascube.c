/*----------------------------------------------------*/
/* Packet  : Demo & Effect                            */
/* Effect  : Plasma Cube Mapping                      */
/* Author  : Nguyen Ngoc Van                          */
/* Memory  : Compact                                  */
/* Heaps   : 640K                                     */
/* Address : pherosiden@gmail.com                     */
/* Website : http://www.codedemo.net                  */
/* Created : 15/04/1998                               */
/* Please sent to me any bugs or suggests.            */
/* You can use freely this code. Have fun :)          */
/*----------------------------------------------------*/

#include <dos.h>
#include <mem.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <conio.h>

#define LCOS 0.9993908
#define LSIN 0.0348994
#define M_PI 3.141592f

const int16_t verts[][3] = {
    {-40,  40, -40}, {40,  40, -40}, {40,  40, 40}, {-40,  40, 40},
    {-40, -40, -40}, {40, -40, -40}, {40, -40, 40}, {-40, -40, 40}
};

const int16_t face[][4] = {
    {0, 3, 7, 4}, {3, 2, 6, 7}, {2, 1, 5, 6},
    {1, 0, 4, 5}, {0, 1, 2, 3}, {7, 6, 5, 4}
};

typedef struct tagVertex {
    float x, y, z;
} Vertex;

typedef struct tagPoint {
    int16_t x, y, z;
} Point;

Vertex vertices[8] = {0};
Point points[8] = {0};

int16_t ofsx, ofsy, ofsz;
int16_t faces[6][5] = {0};
int16_t polya[200][2] = {0};
int16_t polyb[200][2] = {0};
int16_t polyc[200][2] = {0};

uint16_t u, v, us, vs;
uint16_t lookup[360] = {0};

uint8_t zoom = 0;

uint8_t vbuff[64000] = {0};
uint8_t texture[128][128] = {0};

uint8_t *vmem = (uint8_t*)0xA0000000L;
uint8_t *tmem = (uint8_t*)0xB8000000L;

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

void flip()
{
    __asm {
        les     di, vmem
        mov     ax, seg vbuff
        mov     ds, ax
        xor     si, si
        mov     cx, 16000
        rep     movsd
    }
}

void clearMem()
{
    __asm {
        mov     ax, seg vbuff
        mov     es, ax
        xor     di, di
        xor     eax, eax
        mov     cx, 16000
        rep     stosd
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
        mov     ax, y
        shl     ax, 5
        add     di, ax
        shl     ax, 2
        add     di, ax
        mov     ah, col
        mov     cx, len
    next:
        lodsb
        stosw
        loop    next
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
        in      al, dx
        test    al, 0x08
        jz      waitH
    }
}

void putPixel(int16_t x, int16_t y, uint8_t col)
{
    __asm {
        mov     ax, seg vbuff
        mov     es, ax
        mov     di, x
        mov     ax, y
        shl     ax, 6
        add     ah, byte ptr y
        add     di, ax
        mov     al, col
        stosb
    }
}

void init()
{
    int16_t i;

    ofsx = 160;
    ofsy = 100;
    ofsz = -890;

    us = 40;
    vs = 50;
    zoom = 1;

    __asm {
        mov     ax, 0x13
        int     0x10
    }

    for (i = 0; i < 360; i++) lookup[i] = cos(i * M_PI / 180) * 128;

    for (i = 0; i <= 7; i++)
    {
        vertices[i].x = verts[i][0];
        vertices[i].y = verts[i][1];
        vertices[i].z = verts[i][2];
    }

    for (i = 0; i <= 5; i++)
    {
        faces[i][0] = face[i][0];
        faces[i][1] = face[i][1];
        faces[i][2] = face[i][2];
        faces[i][3] = face[i][3];
    }
}

void sort(int16_t left, int16_t right)
{
    int16_t tmp[5] = {0};
    int16_t lo = left;
    int16_t hi = right;
    int16_t mid = faces[(lo + hi) >> 1][4];

    do {
        while (faces[lo][4] > mid) lo++;
        while (mid > faces[hi][4]) hi--;

        if (lo <= hi)
        {
            memcpy(tmp, faces[lo], sizeof(faces[lo]));
            memcpy(faces[lo], faces[hi], sizeof(faces[hi]));
            memcpy(faces[hi], tmp, sizeof(tmp));

            lo++;
            hi--;
        }
    } while (lo <= hi);

    if (left < hi) sort(left, hi);
    if (lo < right) sort(lo, right);
}

void rotate()
{
    int16_t i;
    Vertex tmp;

    for (i = 0; i <= 7; i++)
    {
        tmp.y = LCOS * vertices[i].y - LSIN * vertices[i].z;
        tmp.z = LSIN * vertices[i].y + LCOS * vertices[i].z;

        vertices[i].y = tmp.y;
        vertices[i].z = tmp.z;

        tmp.x = LCOS * vertices[i].x - LSIN * vertices[i].y;
        tmp.y = LSIN * vertices[i].x + LCOS * vertices[i].y;

        vertices[i].x = tmp.x;
        vertices[i].y = tmp.y;

        points[i].x = (vertices[i].x * 256) / (vertices[i].z - ofsz) + ofsx;
        points[i].y = (vertices[i].y * 256) / (vertices[i].z - ofsz) + ofsy;
        points[i].z = vertices[i].z;
    }

    for (i = 0; i <= 5; i++)
        faces[i][4] = points[faces[i][0]].z + points[faces[i][1]].z +
                      points[faces[i][2]].z + points[faces[i][3]].z;

    sort(0, 5);
}

void updatePlasma()
{
    int16_t i, j;

    u = us;
    for (i = 0; i <= 127; i++)
    {
        v = vs;
        for (j = 0; j <= 127; j++)
        {
            texture[i][j] = ((((lookup[i] + lookup[u] + lookup[j] + lookup[v]) << 8) >> 2) >> 8) + 128;
            v = (v + 4) % 360;
        }
        u = (u + 3) % 360;
    }
}

void doSide(int16_t x1, int16_t y1, int16_t u1, int16_t v1, int16_t x2, int16_t y2, int16_t u2, int16_t v2)
{
    int16_t x, y, xinc;
    int16_t u, uinc;
    int16_t v, vinc;
    int16_t tmp, xdec;

    if (y1 == y2) return;

    if (y2 < y1)
    {
        tmp = y2; y2 = y1; y1 = tmp;
        tmp = x2; x2 = x1; x1 = tmp;
        tmp = u2; u2 = u1; u1 = tmp;
        tmp = v2; v2 = v1; v1 = tmp;
    }

    xinc = ((x2 - x1) << 7) / (y2 - y1); x = x1 << 7;
    uinc = ((u2 - u1) << 7) / (y2 - y1); u = u1 << 7;
    vinc = ((v2 - v1) << 7) / (y2 - y1); v = v1 << 7;

    for (y = y1; y <= y2; y++)
    {
        if (y >= 0 && y <= 199)
        {
            xdec = x >> 7;
            if (xdec < polya[y][0])
            {
                polya[y][0] = xdec;
                polyb[y][0] = u;
                polyc[y][0] = v;
            }
            
            if (xdec > polya[y][1])
            {
                polya[y][1] = xdec;
                polyb[y][1] = u;
                polyc[y][1] = v;
            }
        }

        x += xinc;
        u += uinc;
        v += vinc;
    }
}

void textureMapPoly(int16_t x1, int16_t y1, int16_t u1, int16_t v1, int16_t x2, int16_t y2, int16_t u2, int16_t v2, int16_t x3, int16_t y3, int16_t u3, int16_t v3, int16_t x4, int16_t y4, int16_t u4, int16_t v4)
{
    int16_t uval, ui, vval, vi;
    int16_t x, y, miny, maxy;

    for (y = 0; y <= 199; y++)
    {
        polya[y][0] =  21474;
        polya[y][1] = -21474;
        polyb[y][0] =  21474;
        polyb[y][1] = -21474;
        polyc[y][0] =  21474;
        polyc[y][1] = -21474;
    }

    miny = y1;
    maxy = y1;

    if (y2 < miny) miny = y2;
    if (y3 < miny) miny = y3;
    if (y4 < miny) miny = y4;
    if (y2 > maxy) maxy = y2;
    if (y3 > maxy) maxy = y3;
    if (y4 > maxy) maxy = y4;

    if (miny < 0)  miny = 0;
    if (maxy > 199) maxy = 199;
    if (miny > 199 || maxy < 0) return;

    doSide(x1, y1, u1, v1, x2, y2, u2, v2);
    doSide(x2, y2, u2, v2, x3, y3, u3, v3);
    doSide(x3, y3, u3, v3, x4, y4, u4, v4);
    doSide(x4, y4, u4, v4, x1, y1, u1, v1);

    for (y = miny; y <= maxy; y++)
    {
        uval = polyb[y][0];
        ui = (polya[y][0] != polya[y][1]) ? (polyb[y][1] - polyb[y][0]) / (polya[y][1] - polya[y][0]) : 0;

        vval = polyc[y][0];
        vi = (polya[y][0] != polya[y][1]) ? (polyc[y][1] - polyc[y][0]) / (polya[y][1] - polya[y][0]) : 0;

        if (polya[y][0] < 0) polya[y][0] = 0;
        if (polya[y][1] > 319) polya[y][1] = 319;

        for (x = polya[y][0]; x <= polya[y][1]; x++)
        {
            putPixel(x, y, texture[uval >> 7][vval >> 7]);
            uval += ui;
            vval += vi;
        }
    }
}

void drawCube()
{
    int16_t i;

    for (i = 0; i <= 5; i++)
    {
        if ((points[faces[i][1]].x - points[faces[i][0]].x) *
           (points[faces[i][0]].y - points[faces[i][2]].y) -
           (points[faces[i][1]].y - points[faces[i][0]].y) *
           (points[faces[i][0]].x - points[faces[i][2]].x) < 0)

            textureMapPoly(
            points[faces[i][0]].x, points[faces[i][0]].y, 0, 0,
            points[faces[i][1]].x, points[faces[i][1]].y, 127, 0,
            points[faces[i][2]].x, points[faces[i][2]].y, 127, 127,
            points[faces[i][3]].x, points[faces[i][3]].y, 0, 127);
    }
}

void main()
{
    int16_t i;

    clearScreen();
    printStr(1, 1, 0x0F, "PLASMA CUBE - (c) 1998 by Nguyen Ngoc Van");
    printStr(1, 2, 0x07, "Press any key to start demo...");
    getch();
   
    init();
    for (i = 0; i <= 63; i++)
    {
        setRGB(i + 128, 0, i, 63);
        setRGB(i + 192, i, 63, 63);
    }

    do {
        rotate();

        vs = (vs + 4) % 360;
        us = (us + 3) % 360;

        if (zoom)
        {
            ofsz += 15;
            if (ofsz > -190) zoom = !zoom;
        }
        else
        {
            ofsz -= 15;
            if (ofsz < -890) zoom = !zoom;
        }

        updatePlasma();
        clearMem();
        drawCube();
        retrace();
        flip();
    } while(!kbhit());

    __asm {
        mov     ax, 0x03
        int     0x10
    }
}
