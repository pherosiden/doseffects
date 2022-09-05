/*----------------------------------------------------*/
/* Packet  : Demo & Effect                            */
/* Effect  : Motion Clear                             */
/* Author  : Nguyen Ngoc Van                          */
/* Memory  : Compact                                  */
/* Heaps   : 640K                                     */
/* Address : pherosiden@gmail.com                     */
/* Website : http://www.codedemo.net                  */
/* Created : 14/02/1998                               */
/* Please sent to me any bugs or suggests.            */
/* You can use freely this code. Have fun :)          */
/*----------------------------------------------------*/

#include <dos.h>
#include <mem.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <conio.h>

#define LCOS 0.9993908
#define LSIN 0.0348994
#define M_PI 3.141592f

const int16_t verts[][3] = {
    {-50,  50, -50}, {50,  50, -50}, {50,  50, 50}, {-50,  50, 50},
    {-50, -50, -50}, {50, -50, -50}, {50, -50, 50}, {-50, -50, 50}
};

const int16_t faces[][4] = {
    {0, 3, 7, 4}, {3, 2, 6, 7}, {2, 1, 5, 6},
    {1, 0, 4, 5}, {0, 1, 2, 3}, {7, 6, 5, 4}
};

typedef struct {
    float x, y, z;
} VERTEX;

typedef struct {
    int16_t x, y, z;
} POINT;

typedef struct {
    uint8_t r, g, b;
} RGB;

RGB pal[256] = {0};
VERTEX verties[8] = {0};
POINT points[8] = {0};

int16_t face[6][5] = {0};
int16_t xofs, yofs, zofs;

int16_t poly1[200][2] = {0};
int16_t poly2[200][2] = {0};
uint8_t vbuff[200][320] = {0};

void setPal()
{
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

void HSI2RGB(double H, double S, double I, RGB *pal)
{
    float t, rv, gv, bv;

    rv = 1 + S * sin(H - 2 * M_PI / 3);
    gv = 1 + S * sin(H);
    bv = 1 + S * sin(H + 2 * M_PI / 3);

    t = 63.999 * I / 2;

    pal->r = rv * t;
    pal->g = gv * t;
    pal->b = bv * t;
}

void makePal()
{
    int16_t i;

    for (i = 1; i <= 80; i++) HSI2RGB(4.6 - 1.5 * i / 80, i / 80.0, i / 80.0, &pal[i]);

    for (i = 80; i <= 255; i++)
    {
        pal[i] = pal[i - 1];

        if (pal[i].r < 63) pal[i].r++;
        if (pal[i].r < 63) pal[i].r++;

        if (!(i % 2) && pal[i].g < 63) pal[i].g++;
        if (!(i % 2) && pal[i].b < 63) pal[i].b++;
    }

    setPal(pal);
}

void flip(uint16_t dst)
{
    __asm {
        mov     es, dst
        mov     ax, seg vbuff
        mov     ds, ax
        xor     si, si
        xor     di, di
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
        mov     ax, 0xB800
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
        mov     ax, 0xB800
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

void retrace()
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

void init()
{
    int16_t k;

    xofs = 160;
    yofs = 120;
    zofs = -400;

    for (k = 0; k < 8; k++)
    {
        verties[k].x = verts[k][0];
        verties[k].y = verts[k][1];
        verties[k].z = verts[k][2];
    }

    for (k = 0; k < 6; k++)
    {
        face[k][0] = faces[k][0];
        face[k][1] = faces[k][1];
        face[k][2] = faces[k][2];
        face[k][3] = faces[k][3];
    }
}

void sort(int16_t left, int16_t right)
{
    int16_t tmp[5] = {0};
    int16_t lo = left;
    int16_t hi = right;
    int16_t mid = face[(lo + hi) >> 1][4];

    do {
        while (face[lo][4] > mid) lo++;
        while (mid > face[hi][4]) hi--;

        if (lo <= hi)
        {
            memcpy(tmp, face[lo], sizeof(face[lo]));
            memcpy(face[lo], face[hi], sizeof(face[hi]));
            memcpy(face[hi], tmp, sizeof(tmp));

            lo++;
            hi--;
        }
    } while (lo <= hi);

    if (left < hi) sort(left, hi);
    if (lo < right) sort(lo, right);
}

void rotate()
{
    int16_t k;
    VERTEX temp;

    for (k = 0; k < 8; k++)
    {
        temp.y = LCOS * verties[k].y - LSIN * verties[k].z;
        temp.z = LSIN * verties[k].y + LCOS * verties[k].z;

        verties[k].y = temp.y;
        verties[k].z = temp.z;

        temp.x = LCOS * verties[k].x - LSIN * verties[k].y;
        temp.y = LSIN * verties[k].x + LCOS * verties[k].y;

        verties[k].x = temp.x;
        verties[k].y = temp.y;

        points[k].x = (verties[k].x * 256) / (verties[k].z - zofs) + xofs;
        points[k].y = (verties[k].y * 256) / (verties[k].z - zofs) + yofs;
        points[k].z = verties[k].z + 128;
    }

    for (k = 0; k < 6; k++)
    {
        face[k][4] = points[face[k][0]].z + points[face[k][1]].z + points[face[k][2]].z + points[face[k][3]].z;
    }

    sort(0, 5);
}

void doside(int16_t x1, int16_t y1, int16_t z1, int16_t x2, int16_t y2, int16_t z2)
{
    int16_t y, xdec, temp, x, xinc, z, zinc;

    if (y1 == y2) return;

    if (y2 < y1)
    {
        temp = y2; y2 = y1; y1 = temp;
        temp = x2; x2 = x1; x1 = temp;
        temp = z2; z2 = z1; z1 = temp;
    }

    xinc = ((x2 - x1) << 7) / (y2 - y1);  x = x1 << 7;
    zinc = ((z2 - z1) << 7) / (y2 - y1);  z = z1 << 7;

    for (y = y1; y <= y2; y++)
    {
        if (y >= 0 && y <= 199)
        {
            xdec = x >> 7;
            if (xdec < poly1[y][0]) { poly1[y][0] = xdec; poly2[y][0] = z; }
            if (xdec > poly1[y][1]) { poly1[y][1] = xdec; poly2[y][1] = z; }
        }

        x += xinc;
        z += zinc;
    }
}

void gouraudPoly(int16_t x1, int16_t y1, int16_t z1, int16_t x2, int16_t y2, int16_t z2, int16_t x3, int16_t y3, int16_t z3, int16_t x4, int16_t y4, int16_t z4)
{
    int16_t x, y;
    int16_t miny, maxy, col, cinc;

    for (y = 0; y < 200; y++)
    {
        poly1[y][0] = 500; poly1[y][1] = -500;
        poly2[y][0] = 500; poly2[y][1] = -500;
    }

    miny = y1;
    maxy = y1;

    if (y2 < miny) miny = y2; if (y3 < miny) miny = y3;
    if (y4 < miny) miny = y4; if (y2 > maxy) maxy = y2;
    if (y3 > maxy) maxy = y3; if (y4 > maxy) maxy = y4;

    if (miny < 0) miny = 0;
    if (maxy > 199) maxy = 199;
    if (miny > 199 || maxy < 0) return;

    doside(x1, y1, z1, x2, y2, z2); doside(x2, y2, z2, x3, y3, z3);
    doside(x3, y3, z3, x4, y4, z4); doside(x4, y4, z4, x1, y1, z1);

    for (y = miny; y < maxy; y++)
    {
        if (poly1[y][0] < 0) poly1[y][0] = 0;
        if (poly1[y][1] > 319) poly1[y][1] = 319;

        col = poly2[y][0];

        if (poly1[y][0] != poly1[y][1]) cinc = (poly2[y][1] - poly2[y][0]) / (poly1[y][1] - poly1[y][0]);
        else cinc = 0;

        for (x = poly1[y][0]; x < poly1[y][1]; x++)
        {
            vbuff[y][x] = col >> 7;
            col += cinc;
        }
    }
}

void draw()
{
    int16_t k;

    for (k = 0; k < 6; k++)
    {
        if (((points[face[k][1]].x - points[face[k][0]].x) *
            (points[face[k][0]].y - points[face[k][2]].y) -
            (points[face[k][1]].y - points[face[k][0]].y) *
            (points[face[k][0]].x - points[face[k][2]].x)) < 0)

            gouraudPoly(
                points[face[k][0]].x, points[face[k][0]].y, points[face[k][0]].z,
                points[face[k][1]].x, points[face[k][1]].y, points[face[k][1]].z,
                points[face[k][2]].x, points[face[k][2]].y, points[face[k][2]].z,
                points[face[k][3]].x, points[face[k][3]].y, points[face[k][3]].z
            );
    }
}

void motionBlur()
{
    __asm {
        mov     ax, seg vbuff
        mov     es, ax
        mov     di, 320
    blur:
        xor     ah, ah
        mov     al, es:[di - 1]
        mov     dx, ax
        mov     al, es:[di + 1]
        add     dx, ax
        mov     al, es:[di - 320]
        add     dx, ax
        mov     al, es:[di + 320]
        add     dx, ax
        shr     dx, 2
        or  	dl, dl
        jz    	nodec
        dec     dl
    nodec:
        mov     es:[di - 320], dl
        inc     di
        cmp     di, 63680
        jnz     blur
    }
}

void main()
{
    clearScreen();
    printStr(1, 1, 0x0F, "MOTION BLUR - (c) 1998 by Nguyen Ngoc Van");
    printStr(1, 2, 0x07, "Press any key to start demo...");
    getch();

    __asm {
        mov     ax, 0x13
        int     0x10
    }

    init();
    makePal();

    while(!kbhit())
    {
        rotate();
        motionBlur();
        draw();
        retrace();
        flip(0xA000);
    }

    __asm {
        mov     ax, 0x03
        int     0x10
    }
}
