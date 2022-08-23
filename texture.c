/*----------------------------------------------------*/
/* Packet  : Demo & Effect                            */
/* Effect  : Texture Mapping                          */ 
/* Author  : Nguyen Ngoc Van                          */
/* Memory  : Compact                                  */
/* Heaps   : 640K                                     */
/* Address : pherosiden@gmail.com                     */
/* Website : http://www.codedemo.net                  */
/* Created : 20/03/1998                               */ 
/* Please sent to me any bugs or suggests.            */
/* You can use freely this code. Have fun :)          */
/*----------------------------------------------------*/
#include <dos.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <conio.h>

#define MAXP 18
#define YOFS 100
#define XOFS 160
#define ZOFS -500
#define RADI 0.01745329f

typedef struct {
    int16_t x, y, z;
} POINT;

const int16_t face[MAXP][4][3] = {
    {{-10, -10, 10 }, {10 , -10, 10 }, {10 , 10 , 10 }, {-10, 10 , 10 }},
    {{-10, 10 , -10}, {10 , 10 , -10}, {10 , -10, -10}, {-10, -10, -10}},
    {{-10, 10 , 10 }, {-10, 10 , -10}, {-10, -10, -10}, {-10, -10, 10 }},
    {{10 , -10, 10 }, {10 , -10, -10}, {10 , 10 , -10}, {10 , 10 , 10 }},
    {{10 , 10 , 10 }, {10 , 10 , -10}, {-10, 10 , -10}, {-10, 10 , 10 }},
    {{-10, -10, 10 }, {-10, -10, -10}, {10 , -10, -10}, {10 , -10, 10 }},

    {{-10, -10,-20 }, {10 , -10,-20 }, {10 , 10 ,-20 }, {-10, 10 ,-20 }},
    {{-10, 10 , -30}, {10 , 10 , -30}, {10 , -10, -30}, {-10, -10, -30}},
    {{-10, 10 ,-20 }, {-10, 10 , -30}, {-10, -10, -30}, {-10, -10,-20 }},
    {{10 , -10,-20 }, {10 , -10, -30}, {10 , 10 , -30}, {10 , 10 ,-20 }},
    {{10 , 10 ,-20 }, {10 , 10 , -30}, {-10, 10 , -30}, {-10, 10 ,-20 }},
    {{-10, -10,-20 }, {-10, -10, -30}, {10 , -10, -30}, {10 , -10,-20 }},

    {{-30, -10, 10 }, {-20, -10, 10 }, {-20, 10 , 10 }, {-30, 10 , 10 }},
    {{-30, 10 , -10}, {-20, 10 , -10}, {-20, -10, -10}, {-30, -10, -10}},
    {{-30, 10 , 10 }, {-30, 10 , -10}, {-30, -10, -10}, {-30, -10, 10 }},
    {{-20, -10, 10 }, {-20, -10, -10}, {-20, 10 , -10}, {-20, 10 , 10 }},
    {{-20, 10 , 10 }, {-20, 10 , -10}, {-30, 10 , -10}, {-30, 10 , 10 }},
    {{-30, -10, 10 }, {-30, -10, -10}, {-20, -10, -10}, {-20, -10, 10 }}
};

POINT lines[MAXP][4] = {0};
POINT trans[MAXP][4] = {0};
POINT center1[MAXP] = {0};
POINT center2[MAXP] = {0};

int16_t miny, maxy;
int16_t topclipx, botclipy;

int16_t order[MAXP] = {0};
int16_t lookup[360][2] = {0};

int16_t left[401][3] = {0};
int16_t right[401][3] = {0};
uint8_t texture[128][128] = {0};

uint8_t vbuff1[200][320] = {0};
uint8_t vbuff2[200][320] = {0};
uint8_t *vmem = (uint8_t*)0xA0000000L;
uint8_t *tmem = (uint8_t*)0xB8000000L;

inline int16_t roundf(float x)
{
    if (x >= 0.0) return x + 0.5;
    return x - 0.5;
}

void flip(uint8_t *src, uint8_t *dst)
{
    __asm {
        lds     si, src
        les     di, dst
        mov     cx, 16000
        rep     movsd
    }
}

void printStr(int16_t x, int16_t y, uint8_t col, char *msg)
{
    int16_t len = strlen(msg);

    __asm {
        les     di, tmem
        lds     si, msg
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

void clearScr()
{
    __asm {
        les     di, tmem
        xor     ax, ax
        mov     cx, 2000
        rep     stosw
    }
}

void setUpPoints()
{
    int16_t i;

    for (i = 0; i < 360; i++)
    {
        lookup[i][0] = roundf(sin(i * RADI) * 16384);
        lookup[i][1] = roundf(cos(i * RADI) * 16384);
    }

    for (i = 0; i < MAXP; i++)
    {
        center1[i].x = (lines[i][0].x + lines[i][1].x + lines[i][2].x + lines[i][3].x) >> 2;
        center1[i].y = (lines[i][0].y + lines[i][1].y + lines[i][2].y + lines[i][3].y) >> 2;
        center1[i].z = (lines[i][0].z + lines[i][1].z + lines[i][2].z + lines[i][3].z) >> 2;
    }
}

void loadImage()
{
    FILE *fp;
    uint8_t pal[768] = {0};

    fp = fopen("assets/tface.cel", "rb");
    if (!fp) return;

    fseek(fp, 32, SEEK_SET);
    fread(pal, 1, 768, fp);
    fread(vbuff2[0], 1, 64000, fp);
    fclose(fp);

    fp = fopen("assets/robot.cel", "rb");
    if (!fp) return;

    fseek(fp, 800, SEEK_SET);
    fread(texture[0], 1, 16384, fp);
    fclose(fp);

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

void rotatePoints(int16_t x, int16_t y, int16_t z)
{
    int16_t i, j;
    int32_t a, b, c;

    for (i = 0; i < MAXP; i++)
    {
        for (j = 0; j < 4; j++)
        {
            b = lookup[y][1];
            c = lines[i][j].x;
            a = b * c >> 14;
            b = lookup[y][0];
            c = lines[i][j].z;
            a += b * c >> 14;
            trans[i][j].x = a;
            trans[i][j].y = lines[i][j].y;

            b = -lookup[y][0];
            c = lines[i][j].x;
            a = b * c >> 14;
            b = lookup[y][1];
            c = lines[i][j].z;
            a += b * c >> 14;
            trans[i][j].z = a;

            if (x)
            {
                b = lookup[x][1];
                c = trans[i][j].y;
                a = b * c >> 14;
                b = lookup[x][0];
                c = trans[i][j].z;
                a -= b * c >> 14;
                b = lookup[x][0];
                c = trans[i][j].y;
                trans[i][j].y = a;
                a = b * c >> 14;
                b = lookup[x][1];
                c = trans[i][j].z;
                a += b * c >> 14;
                trans[i][j].z = a;
            }

            if (z)
            {
                b = lookup[z][1];
                c = trans[i][j].x;
                a = b * c >> 14;
                b = lookup[z][0];
                c = trans[i][j].y;
                a -= b * c >> 14;
                b = lookup[z][0];
                c = trans[i][j].x;
                trans[i][j].x = a;
                a = b * c >> 14;
                b = lookup[z][1];
                c = trans[i][j].y;
                a += b * c >> 14;
                trans[i][j].y = a;
            }
        }
    }

    for (i = 0; i < MAXP; i++)
    {
        b = lookup[y][1];
        c = center1[i].x;
        a = b * c >> 14;
        b = lookup[y][0];
        c = center1[i].z;
        a += b * c >> 14;
        center2[i].x = a;
        center2[i].y = center1[i].y;
        b = -lookup[y][0];
        c = center1[i].x;
        a = b * c >> 14;
        b = lookup[y][1];
        c = center1[i].z;
        a += b * c >> 14;
        center2[i].z = a;

        if (x)
        {
            b = lookup[x][1];
            c = center2[i].y;
            a = b * c >> 14;
            b = lookup[x][0];
            c = center2[i].z;
            a -= b * c >> 14;
            b = lookup[x][0];
            c = center2[i].y;
            center2[i].y = a;
            a = b * c >> 14;
            b = lookup[x][1];
            c = center2[i].z;
            a += b * c >> 14;
            center2[i].z = a;
        }

        if (z)
        {
            b = lookup[z][1];
            c = center2[i].x;
            a = b * c >> 14;
            b = lookup[z][0];
            c = center2[i].y;
            a -= b * c >> 14;
            b = lookup[z][0];
            c = center2[i].x;
            center2[i].x = a;
            a = b * c >> 14;
            b = lookup[z][1];
            c = center2[i].y;
            a += b * c >> 14;
            center2[i].y = a;
        }
    }
}

void scanLeftSide(int16_t x1, int16_t x2, int16_t ytop, int16_t height, uint8_t side)
{
    uint16_t ofs;
    int16_t x, y, px, py;
    int16_t xadd, pxadd, pyadd;

    if (height == 0) return;

    xadd = ((x2 - x1) << 6) / height;

    switch (side)
    {
    case 1:
        px = 127 << 6;
        py = 0;
        pxadd = -(127 << 6) / height;
        pyadd = 0;
        break;

    case 2:
        px = 127 << 6;
        py = 127 << 6;
        pxadd = 0;
        pyadd = -(127 << 6) / height;
        break;

    case 3:
        px = 0;
        py = 127 << 6;
        pxadd = (127 << 6) / height;
        pyadd = 0;
        break;

    case 4:
        px = 0;
        py = 0;
        pxadd = 0;
        pyadd = (127 << 6) / height;
        break;
    }

    x = x1 << 6;

    for (y = 0; y <= height; y++)
    {
        ofs = ytop + y; 
        if (ofs > 200) ofs = 200;

        left[ofs + 200][0] = x >> 6;
        left[ofs + 200][1] = px >> 6;
        left[ofs + 200][2] = py >> 6;
        x += xadd;
        px += pxadd;
        py += pyadd;
  }
}

void scanRightSide(int16_t x1, int16_t x2, int16_t ytop, int16_t height, uint8_t side)
{
    uint16_t ofs;
    int16_t x, y, px, py;
    int16_t xadd, pxadd, pyadd;

    if (height == 0) return;

    xadd = ((x2 - x1) << 6) / height;

    switch(side)
    {
    case 1:
        px = 0;
        py = 0;
        pxadd = (127 << 6) / height;
        pyadd = 0;
        break;

    case 2:
        px = 127 << 6;
        py = 0;
        pxadd = 0;
        pyadd = (127 << 6) / height;
        break;

    case 3:
        px = 127 << 6;
        py = 127 << 6;
        pxadd = -(127 << 6) / height;
        pyadd = 0;
        break;

    case 4:
        px = 0;
        py = 127 << 6;
        pxadd = 0;
        pyadd = -(127 << 6) / height;
        break;
    }

    x = x1 << 6;

    for (y = 0; y <= height; y++)
    {
        ofs = ytop + y; 
        if (ofs > 200) ofs = 200;

        right[ofs + 200][0] = x >> 6;
        right[ofs + 200][1] = px >> 6;
        right[ofs + 200][2] = py >> 6;
        x += xadd;
        px += pxadd;
        py += pyadd;
    }
}

void textureMapping()
{
    int16_t px1, py1, px2, py2;
    int16_t pxadd, pyadd, x, y;
    int16_t polyx1, polyx2, width;

    if (miny < 0) miny = 0;
    if (maxy > 199) maxy = 199;

    if (miny < topclipx) miny = topclipx;
    if (maxy > botclipy) maxy = botclipy;

    if (maxy - miny < 2) return;
    if (miny > 199) return;
    if (maxy < 0) return;

    for (y = miny; y <= maxy; y++)
    {
        polyx1 = left[y + 200][0];
        px1 = left[y + 200][1] << 7;
        py1 = left[y + 200][2] << 7;

        polyx2 = right[y + 200][0];
        px2 = right[y + 200][1] << 7;
        py2 = right[y + 200][2] << 7;

        width = polyx2 - polyx1 + 1;
        if (width <= 0) width = 1;

        pxadd = (px2 - px1) / width;
        pyadd = (py2 - py1) / width;

        for (x = 0; x < width; x++)
        {
            vbuff1[y][polyx1 + x] = texture[px1 >> 7][py1 >> 7];
            px1 += pxadd;
            py1 += pyadd;
        }
    }
}

void textureMapPoly(int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t x3, int16_t y3, int16_t x4, int16_t y4)
{
    maxy = 0;
    miny = 32767;
    
    if (y1 < miny) miny = y1;
    if (y1 > maxy) maxy = y1;
    if (y2 < miny) miny = y2;
    if (y2 > maxy) maxy = y2;
    if (y3 < miny) miny = y3;
    if (y3 > maxy) maxy = y3;
    if (y4 < miny) miny = y4;
    if (y4 > maxy) maxy = y4;

    if (miny > maxy - 5) return;

    if (y2 < y1) scanLeftSide(x2, x1, y2, y1 - y2 + 1, 1);
    else scanRightSide(x1, x2, y1, y2 - y1 + 1, 1);

    if (y3 < y2) scanLeftSide(x3, x2, y3, y2 - y3 + 1, 2);
    else scanRightSide(x2, x3, y2, y3 - y2 + 1, 2);

    if (y4 < y3) scanLeftSide(x4, x3, y4, y3 - y4 + 1, 3);
    else scanRightSide(x3, x4, y3, y4 - y3 + 1, 3);

    if (y1 < y4) scanLeftSide(x1, x4, y1, y4 - y1 + 1, 4);
    else scanRightSide(x4, x1, y4, y1 - y4 + 1, 4);

    textureMapping();
}

void drawPoints()
{
    int32_t nx;
    int16_t i, j;
    int16_t temp, normal;
    int16_t x1, y1, x2, y2;
    int16_t x3, y3, x4, y4;

    for (j = 0; j < MAXP; j++)
    {
        i = order[j];

        if (trans[i][0].z + ZOFS < 0 && trans[i][1].z + ZOFS < 0 && trans[i][2].z + ZOFS < 0 && trans[i][3].z + ZOFS < 0)
        {
            temp = trans[i][0].z + ZOFS;
            nx = trans[i][0].x;
            x1 = ((nx << 8) + (nx >> 8)) / temp + XOFS;
            nx = trans[i][0].y;
            y1 = ((nx << 8) + (nx >> 8)) / temp + YOFS;

            temp = trans[i][1].z + ZOFS;
            nx = trans[i][1].x;
            x2 = ((nx << 8) + (nx >> 8)) / temp + XOFS;
            nx = trans[i][1].y;
            y2 = ((nx << 8) + (nx >> 8)) / temp + YOFS;

            temp = trans[i][2].z + ZOFS;
            nx = trans[i][2].x;
            x3 = ((nx << 8) + (nx >> 8)) / temp + XOFS;
            nx = trans[i][2].y;
            y3 = ((nx << 8) + (nx >> 8)) / temp + YOFS;

            temp = trans[i][3].z + ZOFS;
            nx = trans[i][3].x;
            x4 = ((nx << 8) + (nx >> 8)) / temp + XOFS;
            nx = trans[i][3].y;
            y4 = ((nx << 8) + (nx >> 8)) / temp + YOFS;

            normal = (y1 - y3) * (x2 - x1) - (x1 - x3) * (y2 - y1);
            if (normal < 0) textureMapPoly(x1, y1, x2, y2, x3, y3, x4, y4);
        }
    }
}

void sortPoints()
{
    int16_t i;
    int16_t temp;

    for (i = 0; i < MAXP; i++) order[i] = i;

    i = 0;

    while (i < MAXP - 1)
    {
        if (center2[i].z > center2[i + 1].z)
        {
            temp = center2[i + 1].x;
            center2[i + 1].x = center2[i].x;
            center2[i].x = temp;

            temp = center2[i + 1].y;
            center2[i + 1].y = center2[i].y;
            center2[i].y = temp;

            temp = center2[i + 1].z;
            center2[i + 1].z = center2[i].z;
            center2[i].z = temp;

            temp = order[i + 1];
            order[i + 1] = order[i];
            order[i] = temp;

            i = 0;
        }

        i++;
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

void moveAround()
{
    int16_t i, j;
    int16_t deg1 = 0;
    int16_t deg2 = 0;
    
    for (i = 0; i < MAXP; i++)
    {
        for (j = 0; j < 4; j++)
        {
            lines[i][j].x = face[i][j][0] << 3;
            lines[i][j].y = face[i][j][1] << 3;
            lines[i][j].z = face[i][j][2] << 3;
        }
    }

    setUpPoints();
    loadImage();

    topclipx = 0;
    botclipy = 199;

    do {
        rotatePoints(deg2, deg1, deg2);
        sortPoints();
        drawPoints();
        retrace();
        flip(vbuff1[0], vmem);
        flip(vbuff2[0], vbuff1[0]);
        deg1 = (deg1 + 1) % 360;
        deg2 = (deg2 + 1) % 360;
    } while(!kbhit());
}

void main()
{
    clearScr();
    printStr(1, 1, 0x0F,  "TEXTURE MAPPING - (c) 1998 by Nguyen Ngoc Van");
    printStr(1, 2, 0x07,  "Press any key to continue ...");
    getch();
    
    __asm {
        mov     ax, 0x13
        int     0x10
        call    moveAround
        mov     ax, 0x03
        int     0x10
    }
}
