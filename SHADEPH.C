/*----------------------------------------------------*/
/* Packet  : Demo & Effect                            */
/* Effect  : Phong Shader                             */
/* Author  : Nguyen Ngoc Van                          */
/* Memory  : Compact                                  */
/* Heaps   : 640K                                     */
/* Address : pherosiden@gmail.com                     */
/* Website : http://www.codedemo.net                  */
/* Created : 27/04/1998                               */ 
/* Please sent to me any bugs or suggests.            */
/* You can use freely this code. Have fun :)          */
/*----------------------------------------------------*/

#include <dos.h>
#include <mem.h>
#include <math.h>
#include <stdlib.h>
#include <stdint.h>
#include <malloc.h>
#include <stdio.h>
#include <conio.h>

int16_t faces[320][3] = {0};
int16_t ut[160] = {0}, vt[160] = {0};
int16_t pind[320] = {0}, polyz[320] = {0};
int16_t fx[320] = {0}, fy[320] = {0}, fz[320] = {0};
int16_t bx[160] = {0}, by[160] = {0}, bz[160] = {0};
int16_t px[160] = {0}, py[160] = {0}, pz[160] = {0};
int16_t nx[160] = {0}, ny[160] = {0}, nz[160] = {0};

int16_t xt1, yt1, zt1;
int16_t nvisibles, nverts, nfaces;
uint8_t alpha, beta, gamma;

int32_t dist = 0;
int32_t cost[256] = {0};
int32_t sint[256] = {0};

int32_t ul[200] = {0}, ur[200] = {0};
int32_t vl[200] = {0}, vr[200] = {0};
int32_t xl[200] = {0}, xr[200] = {0};

uint8_t vbuff[200][320] = {0};
uint8_t texture[256][256] = {0};

uint8_t *vmem = (uint8_t*)0xA0000000L;
uint8_t *tmem = (uint8_t*)0xB8000000L;

void setPalette(uint8_t i, uint8_t r, uint8_t g, uint8_t b)
{
    __asm {
        mov     dx, 0x03C8
        mov     al, i
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

void clearMem(uint8_t *where)
{
    __asm {
        les     di, where
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
        les     di, tmem
        lds     si, msg
        add     di, x
        shl     di, 1
        mov     ax, y
        shl     ax, 5
        add     di, ax
        shl     ax, 2
        add     di, ax
        mov     ah, col
        mov     cx, len
    lp:
        lodsb
        stosw
        loop    lp
    }
}

void retrace()
{
    __asm {
        mov     dx, 0x03DA
    waith:
        in      al, dx
        test    al, 0x08
        jnz     waith
    waitv:
        in      al, dx
        test    al, 0x08
        jz      waitv
    }
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

inline int32_t roundf(float x)
{
    if (x > 0.0) return x + 0.5;
    return x - 0.5;
}

void calcVertexNormals()
{
    int16_t i, j, nf;
    float relx1, rely1, relz1;
    float relx2, rely2, relz2, val;

    for (i = 0; i < nverts; i++)
    {
        relx1 = 0;
        rely1 = 0;
        relz1 = 0;
        nf = 0;

        for (j = 0; j < nfaces; j++)
        {
            if (faces[j][0] == i || faces[j][1] == i || faces[j][2] == i)
            {
                relx1 = relx1 + fx[j];
                rely1 = rely1 + fy[j];
                relz1 = relz1 + fz[j];
                nf++;
            }
        }

        if (nf)
        {
            relx1 = relx1 / nf;
            rely1 = rely1 / nf;
            relz1 = relz1 / nf;

            val = sqrt(relx1 * relx1 + rely1 * rely1 + relz1 * relz1);

            nx[i] = roundf(relx1 / val * 120);
            ny[i] = roundf(rely1 / val * 120);
            nz[i] = roundf(relz1 / val * 120);
        }
    }
}

void createTorusData()
{
    int16_t i, j, n;

    float cx, cy;
    float rx1, ry1, rz1;
    float rx2, ry2, rz2;

    n = 0;
    nverts = 160;
    nfaces = 320;
    
    for (i = 0; i < 16; i++)
    {
        cx = cos(i / 2.54647909) * 180;
        cy = sin(i / 2.54647909) * 180;
    
        for (j = 0; j < 10; j++)
        {
            px[n] = roundf(cx + cos(j / 1.59154931) * cos(i / 2.54647909) * 90);
            py[n] = roundf(cy + cos(j / 1.59154931) * sin(i / 2.54647909) * 90);
            pz[n] = roundf(sin(j / 1.59154931) * 90);
            n++;
        }
    }

    n = 0;

    for (i = 0; i < 16; i++)
    {
        for (j = 0; j < 10; j++)
        {
            faces[n][2] = i * 10 + j;
            faces[n][1] = i * 10 + (j + 1) % 10;
            faces[n][0] = (i * 10 + j + 10) % 160;
            n++;

            faces[n][2] = i * 10 + (j + 1) % 10;
            faces[n][1] = (i * 10 + (j + 1) % 10 + 10) % 160;
            faces[n][0] = (i * 10 + j + 10) % 160;
            n++;
        }
    }

    for (n = 0; n < 320; n++)
    {
        rx1 = px[faces[n][1]] - px[faces[n][0]];
        ry1 = py[faces[n][1]] - py[faces[n][0]];
        rz1 = pz[faces[n][1]] - pz[faces[n][0]];

        rx2 = px[faces[n][2]] - px[faces[n][0]];
        ry2 = py[faces[n][2]] - py[faces[n][0]];
        rz2 = pz[faces[n][2]] - pz[faces[n][0]];

        fx[n] = ry1 * rz2 - ry2 * rz1;
        fy[n] = rz1 * rx2 - rz2 * rx1;
        fz[n] = rx1 * ry2 - rx2 * ry1;
    }
}

void initialize()
{
    int16_t i, j;

    createTorusData();
    calcVertexNormals();

    for (i = 0; i < 256; i++)
    {
        cost[i] = roundf(cos(i / 40.586) * 128);
        sint[i] = roundf(sin(i / 40.586) * 128);
    }

    for (i = 1; i < 64; i++) setPalette(i, i, 10 + i / 1.4, 20 + i / 1.6);

    for (i = 0; i < 256; i++)
    {
        for (j = 0; j < 256; j++) texture[i][j] = roundf(pow(pow(sin(i / 81.487), 2), 2) * pow(pow(sin(j / 81.487), 2), 2) * 62 + 1);
    }
}

void quickSort(int16_t left, int16_t right)
{
    int16_t tmp;
    int16_t lo = left;
    int16_t hi = right;
    int16_t mid = polyz[(lo + hi) >> 1];

    do {
        while (polyz[lo] > mid) lo++;
        while (mid > polyz[hi]) hi--;

        if (lo <= hi)
        {
            tmp = polyz[lo];
            polyz[lo] = polyz[hi];
            polyz[hi] = tmp;

            tmp = pind[lo];
            pind[lo] = pind[hi];
            pind[hi] = tmp;

            lo++;
            hi--;
        }
    } while (lo <= hi);

    if (left < hi) quickSort(left, hi);
    if (lo < right) quickSort(lo, right);
}

void newTexture(int16_t x1, int16_t y1, int16_t u1, int16_t v1, int16_t x2, int16_t y2, int16_t u2, int16_t v2, int16_t x3, int16_t y3, int16_t u3, int16_t v3)
{
    int16_t osx = 0, osy = 0;
    int16_t xofs[320] = { 0 };
    int16_t yofs[320] = { 0 };

    uint16_t u, v, i, j, k, a, b;
    int16_t du21, du31, du32;
    int16_t dx21, dx31, dx32;
    int16_t dv21, dv31, dv32;
    int32_t dy21, dy31, dy32;

    for (i = 0; i < 2; i++)
    {
        if (y3 < y2)
        {
            j = y3; y3 = y2; y2 = j;
            j = x3; x3 = x2; x2 = j;
            j = u3; u3 = u2; u2 = j;
            j = v3; v3 = v2; v2 = j;
        }

        if (y2 < y1)
        {
            j = y1; y1 = y2; y2 = j;
            j = x1; x1 = x2; x2 = j;
            j = u1; u1 = u2; u2 = j;
            j = v1; v1 = v2; v2 = j;
        }

        if (y3 < y1)
        {
            j = y1; y1 = y3; y3 = j;
            j = x1; x1 = x3; x3 = j;
            j = u1; u1 = u3; u3 = j;
            j = v1; v1 = v3; v3 = j;
        }
    }

    if (y1 == y2 && x1 > x2)
    {
        j = x1; x1 = x2; x2 = j;
        j = u1; u1 = u2; u2 = j;
        j = v1; v1 = v2; v2 = j;
    }

    dy21 = y2 - y1;
    dy31 = y3 - y1;
    dy32 = y3 - y2;

    dx21 = x2 - x1;
    dx31 = x3 - x1;
    dx32 = x3 - x2;

    du21 = u2 - u1;
    du31 = u3 - u1;
    du32 = u3 - u2;

    dv21 = v2 - v1;
    dv31 = v3 - v1;
    dv32 = v3 - v2;

    xl[0] = x1; xl[0] <<= 8;
    ul[0] = u1;	ul[0] <<= 8;
    vl[0] = v1; vl[0] <<= 8;

    if (y1 == y2)
    {
        xr[0] = x2; xr[0] <<= 8;
        ur[0] = u2; ur[0] <<= 8;
        vr[0] = v2; vr[0] <<= 8;
    }
    else
    {
        xr[0] = xl[0];
        ur[0] = ul[0];
        vr[0] = vl[0];
    }

    for (i = y1 + 1; i <= y2; i++)
    {
        xl[i - y1] = xl[i - y1 - 1] + (dx31 << 8) / dy31;
        xr[i - y1] = xr[i - y1 - 1] + (dx21 << 8) / dy21;
        ul[i - y1] = ul[i - y1 - 1] + (du31 << 8) / dy31;
        ur[i - y1] = ur[i - y1 - 1] + (du21 << 8) / dy21;
        vl[i - y1] = vl[i - y1 - 1] + (dv31 << 8) / dy31;
        vr[i - y1] = vr[i - y1 - 1] + (dv21 << 8) / dy21;
    }

    for (i = y2 + 1; i <= y3; i++)
    {
        xl[i - y1] = xl[i - y1 - 1] + (dx31 << 8) / dy31;
        xr[i - y1] = xr[i - y1 - 1] + (dx32 << 8) / dy32;
        ul[i - y1] = ul[i - y1 - 1] + (du31 << 8) / dy31;
        ur[i - y1] = ur[i - y1 - 1] + (du32 << 8) / dy32;
        vl[i - y1] = vl[i - y1 - 1] + (dv31 << 8) / dy31;
        vr[i - y1] = vr[i - y1 - 1] + (dv32 << 8) / dy32;
    }

    k = y2 - y1;
    
    if (xl[k] < xr[k])
    {
        u = ul[k];
        v = vl[k];
        osx = u >> 8;
        osy = v >> 8;
        a = (xr[k] >> 8) - (xl[k] >> 8);

        for (i = 0; i <= a; i++)
        {
            xofs[i] = (u >> 8) - osx;
            yofs[i] = (v >> 8) - osy;
            u += ((ur[k] - ul[k]) << 8) / (xr[k] - xl[k] + 1);
            v += ((vr[k] - vl[k]) << 8) / (xr[k] - xl[k] + 1);
        }
    }
    else
    {
        u = ur[k];
        v = vr[k];
        osx = u >> 8;
        osy = v >> 8;
        a = (xl[k] >> 8) - (xr[k] >> 8);

        for (i = 0; i <= a; i++)
        {
            xofs[i] = (u >> 8) - osx;
            yofs[i] = (v >> 8) - osy;
            u += ((ul[k] - ur[k]) << 8) / (xl[k] - xr[k] + 1);
            v += ((vl[k] - vr[k]) << 8) / (xl[k] - xr[k] + 1);
        }
    }

    if (xl[k] < xr[k])
    {
        for (i = 0; i <= y3 - y1; i++)
        {
            osx = ul[i] >> 8;
            osy = vl[i] >> 8;
            a = xl[i] >> 8;
            b = xr[i] >> 8;
            for (j = a; j <= b; j++)
            {
                u = (osx + xofs[j - a]) & 0xff;
                v = (osy + yofs[j - a]) & 0xff;
                vbuff[i + y1][j] = texture[v][u];
            }
        }
    }
    else
    {
        for (i = 0; i <= y3 - y1; i++)
        {
            osx = ur[i] >> 8;
            osy = vr[i] >> 8;
            a = xr[i] >> 8;
            b = xl[i] >> 8;
            for (j = a; j <= b; j++)
            {
                u = (osx + xofs[j - a]) & 0xff;
                v = (osy + yofs[j - a]) & 0xff;
                vbuff[i + y1][j] = texture[v][u];	
            }
        }
    }
}

void rotate(int16_t *px, int16_t *py, int16_t *pz)
{
    int16_t x2, x3, y1, y3, z1, z2;

    y1 = (cost[alpha] * (*py) - sint[alpha] * (*pz)) >> 7;
    z1 = (sint[alpha] * (*py) + cost[alpha] * (*pz)) >> 7;
    x2 = (cost[beta] * (*px) + sint[beta] * z1) >> 7;

    *pz = (cost[beta] * z1 - sint[beta] * (*px)) >> 7;
    *px = (cost[gamma] * x2 - sint[gamma] * y1) >> 7;
    *py = (sint[gamma] * x2 + cost[gamma] * y1) >> 7;
}

void main()
{
    int16_t i;

    clearTextMem();
    printStr(1, 1, 0x0F, "PHONG SHADING - (c) 1998 by Nguyen Ngoc Van");
    printStr(1, 2, 0x07, "Press any key to start demo...");
    getch();

    __asm {
        mov     ax, 0x13
        int     0x10
    }

    initialize();

    dist = 150;
    alpha = 0;
    beta = 0;
    gamma = 0;

    do {
        for (i = 0; i < nverts; i++)
        {
            xt1 = px[i];
            yt1 = py[i];
            zt1 = pz[i];

            rotate(&xt1, &yt1, &zt1);
            zt1 += 468;

            bx[i] = 160 + xt1 * dist / zt1;
            by[i] = 100 + yt1 * dist * 83 / 100 / zt1;
            bz[i] = zt1;

            xt1 = nx[i];
            yt1 = ny[i];
            zt1 = nz[i];

            rotate(&xt1, &yt1, &zt1);
            ut[i] = 128 + xt1;
            vt[i] = 128 + yt1;
        }

        nvisibles = 0;
        
        for (i = 0; i < nfaces; i++)
        {
            if ((bx[faces[i][2]] - bx[faces[i][0]]) * (by[faces[i][1]] - by[faces[i][0]]) - (bx[faces[i][1]] - bx[faces[i][0]]) * (by[faces[i][2]] - by[faces[i][0]]) > 0)
            {
                pind[nvisibles] = i;
                polyz[nvisibles] = bz[faces[i][0]] + bz[faces[i][1]] + bz[faces[i][2]];
                nvisibles++;
            }
        }

        quickSort(0, nvisibles - 1);

        for (i = 0; i < nvisibles; i++) newTexture(
            bx[faces[pind[i]][0]], by[faces[pind[i]][0]],
            ut[faces[pind[i]][0]], vt[faces[pind[i]][0]],
            bx[faces[pind[i]][1]], by[faces[pind[i]][1]],
            ut[faces[pind[i]][1]], vt[faces[pind[i]][1]],
            bx[faces[pind[i]][2]], by[faces[pind[i]][2]],
            ut[faces[pind[i]][2]], vt[faces[pind[i]][2]]);

        alpha = (alpha + 2) % 256;
        beta = (beta + 255) % 256;
        gamma = (gamma + 1) % 256;

        retrace();
        flip(vbuff[0], vmem);
        clearMem(vbuff[0]);
    } while (!kbhit());

    __asm {
        mov     ax, 0x03
        int     0x10
    }
}
