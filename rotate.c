/*----------------------------------------------------*/
/* Packet  : Demo & Effect                            */
/* Effect  : Rotate                                   */ 
/* Author  : Nguyen Ngoc Van                          */
/* Memory  : Small                                    */
/* Heaps   : 64K                                      */
/* Address : pherosiden@gmail.com                     */ 
/* Website : http://www.codedemo.net                  */
/* Created : 24/02/1998                               */
/* Please sent to me any bugs or suggests.            */
/* You can use freely this code. Have fun :)          */
/*----------------------------------------------------*/

#include <dos.h>
#include <math.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <conio.h>

#define SCALING_FACTOR1     10
#define SCALING_FACTOR2     50
#define XOFFSET             160
#define YOFFSET             100
#define ZOFFSET             550
#define VERTICES            (SCALING_FACTOR1 * SCALING_FACTOR2)
#define PALSIZE             63
#define RADIUS1             25
#define RADIUS2             50
#define RADIUS3             25
#define M_PI                3.141592f

typedef struct {
    int16_t x, y;
} Point;

typedef struct {
    int16_t x, y, z;
} Point3D;

Point3D shape[VERTICES] = {0};
Point3D rotatedshape[VERTICES] = {0};

Point shape2d[VERTICES] = {0};
Point face2d[3] = {0};

uint8_t colors[3] = {0};

int16_t vertices = 0, vertexcount, zvalue;
int16_t draworder[VERTICES] = {0};
int16_t maxz = RADIUS1 + RADIUS2 + RADIUS3;
int16_t polycoords[VERTICES][4] = {0};

float phi = 0, theta = 0;

uint8_t vbuff[64000] = {0};
uint8_t *vmem = (uint8_t*)0xA0000000L;

void putPixel(uint16_t x, uint16_t y, uint8_t col)
{
    __asm {
        mov     ax, seg vbuff
        mov     es, ax
        xor     di, di
        mov     bx, y
        shl     bx, 6
        add     bh, byte ptr y
        add     bx, x
        add     di, bx
        mov     al, col
        stosb
    }
}

void clearScreen()
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

void gouraudShade(Point *vertices, uint8_t *colors)
{
    uint8_t startcol, endcol;
    int16_t miny = 0, maxy = 0, count, incsigncol, inccountcol, diffcol;
    int16_t y, linewidth, endvertex1, endvertex2, startvertex1, startvertex2;
    int16_t xdiff1, xdiff2, ydiff1, ydiff2, x1, x2, diffcol1, diffcol2;
    int16_t xcal1 = 0, xcal2 = 0, colcal1 = 0, colcal2 = 0;

    for (count = 0; count < 3; count++)
    {
        if (vertices[count].y < vertices[miny].y) miny = count;
        else if (vertices[count].y > vertices[maxy].y) maxy = count;
    }

    startvertex1 = startvertex2 = miny;

    endvertex1 = miny + 2;
    if (endvertex1 >= 3) endvertex1 -= 3;

    endvertex2 = miny + 1;
    if (endvertex2 >= 3) endvertex2 -= 3;

    xdiff1 = vertices[endvertex1].x - vertices[startvertex1].x;
    ydiff1 = vertices[endvertex1].y - vertices[startvertex1].y;
    xdiff2 = vertices[endvertex2].x - vertices[startvertex1].x;
    ydiff2 = vertices[endvertex2].y - vertices[startvertex1].y;

    diffcol1 = colors[endvertex1] - colors[startvertex1];
    diffcol2 = colors[endvertex2] - colors[startvertex2];

    if (ydiff1 == 0) ydiff1 = 1;
    if (ydiff2 == 0) ydiff2 = 1;

    for (y = vertices[miny].y; y <= vertices[maxy].y; y++)
    {
        x2 = vertices[startvertex1].x + xcal1 / ydiff1;
        xcal1 += xdiff1;

        x1 = vertices[startvertex2].x + xcal2 / ydiff2;
        xcal2 += xdiff2;

        endcol = colors[startvertex1] + colcal1 / ydiff1;
        colcal1 += diffcol1;
        startcol = colors[startvertex2] + colcal2 / ydiff2;
        colcal2 += diffcol2;
        
        if (endcol > startcol) incsigncol = 1;
        else incsigncol = -1;

        diffcol = abs(startcol - endcol);
        linewidth = x2 - x1;
        inccountcol = diffcol - (linewidth >> 1);

        for (count = x1; count < x2; count++)
        {
            putPixel(count, y, startcol);

            while (inccountcol >= 0)
            {
                startcol += incsigncol;
                inccountcol -= linewidth;
            }

            inccountcol += diffcol;
        }

        if (y == vertices[endvertex1].y)
        {
            startvertex1 = endvertex1;
            endvertex1 = endvertex2;

            xdiff1 = vertices[endvertex1].x - vertices[startvertex1].x;
            ydiff1 = vertices[endvertex1].y - vertices[startvertex1].y;

            diffcol1 = colors[endvertex1] - colors[startvertex1];
            
            if (ydiff1 == 0) ydiff1 = 1;
            
            xcal1 = xdiff1;
            colcal1 = diffcol1;
        }

        if (y == vertices[endvertex2].y)
        {
            startvertex2 = endvertex2;
            endvertex2 = endvertex1;

            xdiff2 = vertices[endvertex2].x - vertices[startvertex2].x;
            ydiff2 = vertices[endvertex2].y - vertices[startvertex2].y;

            diffcol2 = colors[endvertex2] - colors[startvertex2];
            
            if (ydiff2 == 0) ydiff2 = 1;
            
            xcal2 = xdiff2;
            colcal2 = diffcol2;
        }
    }
}

void setVideoMode(int16_t mode)
{
    __asm {
        mov     ax, mode
        int     0x10
    }
}

void rotatePoint(int16_t x, int16_t y, int16_t xc, int16_t yc, float sinangle, float cosangle, int16_t *endx, int16_t *endy)
{
    *endx = (x - xc) * cosangle - (y - yc) * sinangle + xc;
    *endy = (y - yc) * cosangle + (x - xc) * sinangle + yc;
}

void waitForRetrace()
{
    while (inp(0x03DA) & 8);
    while (!(inp(0x03DA) & 8));
}

void setRgbPal(uint8_t col, uint8_t r, uint8_t g, uint8_t b)
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

int __watcall compare(void const *a, void const *b)
{
    return rotatedshape[*(int16_t*)a].z - rotatedshape[*(int16_t*)b].z;
}

void rotateShape()
{
    float sinphi = sin(phi);
    float cosphi = cos(phi);
    float sintheta = sin(theta);
    float costheta = cos(theta);

    for (vertexcount = 0; vertexcount < vertices; vertexcount++)
    {
        rotatePoint(shape[vertexcount].y, shape[vertexcount].z, 0, 0, sinphi, cosphi, &rotatedshape[vertexcount].y, &rotatedshape[vertexcount].z);
        rotatePoint(shape[vertexcount].x, rotatedshape[vertexcount].z, 0, 0, sintheta, costheta, &rotatedshape[vertexcount].x, &rotatedshape[vertexcount].z);
    }

    phi += 0.05;
    theta += 0.04;
}

void drawFace(int16_t index1, int16_t index2, int16_t index3)
{
    if ((rotatedshape[index2].x - rotatedshape[index1].x) *
        (rotatedshape[index1].y - rotatedshape[index3].y) <
        (rotatedshape[index2].y - rotatedshape[index1].y) *
        (rotatedshape[index1].x - rotatedshape[index3].x)) return;

    face2d[0].x = shape2d[index1].x;
    face2d[0].y = shape2d[index1].y;

    face2d[1].x = shape2d[index2].x;
    face2d[1].y = shape2d[index2].y;

    face2d[2].x = shape2d[index3].x;
    face2d[2].y = shape2d[index3].y;

    colors[0] = 1 + (PALSIZE - 1) * (rotatedshape[index1].z + maxz) / (maxz << 1);
    colors[1] = 1 + (PALSIZE - 1) * (rotatedshape[index2].z + maxz) / (maxz << 1);
    colors[2] = 1 + (PALSIZE - 1) * (rotatedshape[index3].z + maxz) / (maxz << 1);
    
    if (polycoords[draworder[vertexcount]][0] & 2)
    {
        colors[0] += PALSIZE;
        colors[1] += PALSIZE;
        colors[2] += PALSIZE;
    }

    gouraudShade(face2d, colors);
}

void draw2DFaces()
{
    for (vertexcount = 0; vertexcount < vertices; vertexcount++)
    {
        zvalue = (ZOFFSET + rotatedshape[vertexcount].z) >> 2;
        shape2d[vertexcount].x = XOFFSET + ((rotatedshape[vertexcount].x) << 7) / zvalue;
        shape2d[vertexcount].y = YOFFSET + ((rotatedshape[vertexcount].y) << 7) / zvalue;
    }

    for (vertexcount = 0; vertexcount < vertices; vertexcount++)
    {
        drawFace(polycoords[draworder[vertexcount]][0], polycoords[draworder[vertexcount]][1], polycoords[draworder[vertexcount]][2]);
        drawFace(polycoords[draworder[vertexcount]][0], polycoords[draworder[vertexcount]][2], polycoords[draworder[vertexcount]][3]);
    }
}

void setup()
{
    float distance, mindistance;
    float dx, dy, dz, alpha, beta, modulus, value, x, y, z;
    int16_t count1, count2, index1, index2, rotation;

    for (alpha = 0, count2 = 0; count2 < SCALING_FACTOR2; count2++, alpha += 2 * M_PI / SCALING_FACTOR2)
    {
        x = RADIUS2 * cos(2 * alpha) + RADIUS1 * sin(alpha);
        y = RADIUS2 * sin(2 * alpha) + RADIUS1 * cos(alpha);
        z = RADIUS2 * cos(3 * alpha);

        dx = -2 * RADIUS2 * sin(2 * alpha) + RADIUS1 * cos(alpha);
        dy = 2 * RADIUS2 * cos(2 * alpha) - RADIUS1 * sin(alpha);
        dz = -3 * RADIUS2 * sin(3 * alpha);

        value = sqrt(dx * dx + dz * dz);
        modulus = sqrt(dx * dx + dy * dy + dz * dz);

        for (beta = 0, count1 = 0; count1 < SCALING_FACTOR1; count1++, beta += 2 * M_PI / SCALING_FACTOR1)
        {
            shape[vertices].x =	x - RADIUS3 * (cos(beta) * dz - sin(beta) * dx * dy / modulus) / value;
            shape[vertices].y =	y - RADIUS3 * sin(beta) * value / modulus;
            shape[vertices].z =	z + RADIUS3 * (cos(beta) * dx + sin(beta) * dy * dz / modulus) / value;
            vertices++;
        }
    }

    for (count1 = 0; count1 < SCALING_FACTOR2; count1++)
    {
        index1 = count1 * SCALING_FACTOR1;
        index2 = index1 + SCALING_FACTOR1;
        index2 %= vertices;
        
        rotation = 0;
        mindistance =   (shape[index1].x - shape[index2].x) * (shape[index1].x - shape[index2].x) +
                        (shape[index1].y - shape[index2].y) * (shape[index1].y - shape[index2].y) +
                        (shape[index1].z - shape[index2].z) * (shape[index1].z - shape[index2].z);

        for (count2 = 1; count2 < SCALING_FACTOR1; count2++)
        {
            index2 = count2 + index1 + SCALING_FACTOR1;
            if (count1 == SCALING_FACTOR2 - 1) index2 = count2;

            distance =  (shape[index1].x - shape[index2].x) * (shape[index1].x - shape[index2].x) +
                        (shape[index1].y - shape[index2].y) * (shape[index1].y - shape[index2].y) +
                        (shape[index1].z - shape[index2].z) * (shape[index1].z - shape[index2].z);

            if (distance < mindistance)
            {
                mindistance = distance;
                rotation = count2;
            }
        }

        for (count2 = 0; count2 < SCALING_FACTOR1; count2++)
        {
            index2 = (SCALING_FACTOR1 + count2 + rotation) % SCALING_FACTOR1;
            polycoords[index1 + count2][0] = index1 + count2;
            index2 = count2 + 1;
            index2 %= SCALING_FACTOR1;
            polycoords[index1 + count2][1] = index1 + index2;
            index2 = count2 + rotation + 1;
            index2 %= SCALING_FACTOR1;
            polycoords[index1 + count2][2] = (index1 + index2 + SCALING_FACTOR1) % vertices;
            index2 = count2 + rotation;
            index2 %= SCALING_FACTOR1;
            polycoords[index1 + count2][3] = (index1 + index2 + SCALING_FACTOR1) % vertices;
        }
    }

    for (count1 = 0; count1 < vertices; count1++) draworder[count1] = count1;

    setVideoMode(0x13);

    for (count1 = 1; count1 <= PALSIZE; count1++)
    {
        setRgbPal(count1, 63 - 63 * count1 / PALSIZE, 0, 0);
        setRgbPal(PALSIZE + count1, 0, 0, 63 - 63 * count1 / PALSIZE);
    }
}

void clearTextMem()
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
    int16_t len = strlen(msg);

    __asm {
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
        mov     cx, len
    next:
        lodsb
        stosw
        loop	next
    }
}

void main()
{
    clearTextMem();
    printStr(1, 1, 0x0F, "ROTATE - (c) 1998 by Nguyen Ngoc Van");
    printStr(1, 2, 0x07, "Press any key to start demo...");
    getch();

    setup();

    do {
        rotateShape();
        qsort(draworder, vertices, 2, compare);
        clearScreen();
        draw2DFaces();
        waitForRetrace();
        flip();
    } while(!kbhit());

    setVideoMode(0x03);
}
