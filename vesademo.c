/*------------------------------------------------*/
/*    GFXLIB demo (c) 2000 by Nguyen Ngoc Van     */
/*     Full support 8/15/16/24/32 bits color      */
/*        Support load/save BMP & PNG files       */
/* Using Linear Frame Buffer for best performance */
/*        Optimize code by using assembly         */
/*           Code by: Nguyen Ngoc Van             */
/*           Website: www.codedemo.net            */
/*             Email: pherosiden@gmail.com        */
/*------------------------------------------------*/

#include "gfxlib.c"
#include <graph.h>
#include <float.h>

// CHR font information
#define CHR_WIDTH   8   // character width
#define CHR_HEIGHT  16  // character height
#define CHR_NUM     19  // number of character in font table
#define CHR_MAX     250 // max of character in font table
#define CHR_START   0   // start of character

#define LIMITX      640
#define LIMITY      400
#define SIGNED(x)   (((x) > 0) ? 1 : ((x) < 0) ? -1 : 0)

// Font buffer
typedef uint8_t FNT_TBL[CHR_WIDTH][CHR_HEIGHT];

// VNI font table
FNT_TBL	chrPixels[CHR_MAX] = {0};

// Character buffer
int32_t grData[120][2] = {0};
uint8_t chrBuff[CHR_NUM * CHR_WIDTH][CHR_HEIGHT] = {0};

// Max cordinate buffer
int32_t maxh[LIMITX] = {0};
int32_t minh[LIMITX] = {0};

int32_t c1, c2, c3, c4;
int32_t lines, points;

float incx, incy;
float echx, echy;

float f1, f2, f3, f4;
float gx1, gx2, gy1, gy2;

float u, udebut, ufin, du;
float v, vdebut, vfin, dv;

char vue = 'r';
int32_t visiPrec = 0;
int32_t visiCour = 0;

float (*FX)(float, float);
float (*FY)(float, float);
float (*FZ)(float, float);

void findRepeat(float *rept)
{
    float lx, ly, r;
    float lim1 = 0.0, lim2 = 0.0;
    float rmax = 200 * M_PI;
    float fx = 0.0, fy = 0.0;
    
    do {
        *rept += 2 * M_PI;
        r = sin(3 * *rept);
        lx = r * cos(*rept);
        ly = r * sin(*rept);
        lim1 = fabs(lx - fx);
        lim2 = fabs(ly - fy);
    } while ((lim1 >= 1000000.0 || lim2 >= 1000000.0) && *rept <= rmax); 
}

void drawCylodiod(int32_t xc, int32_t yc, int32_t rd, uint8_t a, uint8_t b, float rept, uint32_t col)
{
    float theta = 0.0;
    float x1, y1, x2, y2;

    x1 = xc + rd * sin(3 * theta) * cos(theta) * cos(a * theta);
    y1 = yc + rd * sin(3 * theta) * sin(theta) * sin(b * theta);
    moveTo(x1, y1);

    while (theta < rept)
    {
        x2 = xc + rd * sin(3 * theta) * cos(theta) * cos(a * theta);
        y2 = yc + rd * sin(3 * theta) * sin(theta) * sin(b * theta);
        lineTo(x2, y2, theta + col);
        theta += 0.001;
    }
}

void drawPolygon(int32_t xc, int32_t yc, int32_t rd, uint8_t odre, uint8_t pas)
{
    float theta;
    int32_t phi = 0;
    
    while (phi < 360)
    {
        theta = M_PI * phi / 180;
        moveTo(xc + rd * cos(theta), yc + rd * sin(theta));
        lineTo(xc + rd * cos(odre * theta), yc + rd * sin(odre * theta), phi % 192 + 16);
        phi += pas;
    };
}

void rotatePolygon(POINT *pt, int32_t n, int32_t xc, int32_t yc, int32_t rd, int32_t num, uint8_t odre, uint32_t col)
{
    int32_t i, j;
    float phi = 0.0;

    for (i = 0; i < n; i++)
    {
        pt[i].x = xc + rd * cos(phi);
        pt[i].y = yc - rd * sin(phi);
        phi += 2 * M_PI / n;
    }

    for (j = 0; j < num; j++)
    {
        drawPoly(pt, n, j % 69 + col);
        for (i = 0; i < n; i++)
        {
            pt[i].x = pt[i].x + (pt[(i + 1) % n].x - pt[i].x) / odre;
            pt[i].y = pt[i].y + (pt[(i + 1) % n].y - pt[i].y) / odre;
        }
    }
}

void randomPoly(POINT *pt, int32_t n, int32_t xm, int32_t ym, int32_t num, uint8_t odre, uint32_t col)
{
    int32_t i, j, k;
    
    srand(time(NULL));

    for (i = 0; i < n; i++)
    {
        pt[i].x = rand() % xm;
        pt[i].y = rand() % ym;
    }

    for (j = 0; j < num; j++)
    {
        drawPoly(pt, n, j % 69 + col);
        for (k = 0; k < n; k++)
        {
            pt[k].x = pt[k].x + (pt[(k + 1) % n].x - pt[k].x) / odre;
            pt[k].y = pt[k].y + (pt[(k + 1) % n].y - pt[k].y) / odre;
        }
    }
}

void drawHexagon(POINT *pt, int32_t num, int32_t xc, int32_t yc, int32_t n, int32_t rd, uint8_t odre, uint32_t col)
{
    int32_t i, j, k, m;
    float coef = 2 * M_PI / num, phi;

    for (i = 1; i <= num; i++)
    {
        pt[0].x = xc;
        pt[0].y = yc;

        phi = (i - 1) * coef;
        pt[1].x = xc + rd * cos(phi);
        pt[1].y = yc - rd * sin(phi);

        phi = i * coef;
        pt[2].x = xc + rd * cos(phi);
        pt[2].y = yc - rd * sin(phi);

        for (j = 0; j < n; j++)
        {
            drawPoly(pt, 3, j % 69 + col);		
            if (i % 2)
            {
                for (k = 2; k > 0; k--)
                {
                    pt[k].x = pt[k].x + (pt[(k - 1) % 3].x - pt[k].x) / odre;
                    pt[k].y = pt[k].y + (pt[(k - 1) % 3].y - pt[k].y) / odre;
                }

                pt[0].x = pt[0].x + (pt[2].x - pt[0].x) / odre;
                pt[0].y = pt[0].y + (pt[2].y - pt[0].y) / odre;

            }
            else
            {
                for (m = 0; m < 3; m++)
                {
                    pt[m].x = pt[m].x + (pt[(m + 1) % 3].x - pt[m].x) / odre;
                    pt[m].y = pt[m].y + (pt[(m + 1) % 3].y - pt[m].y) / odre;
                }
            }
        }
    }
}

void graphDemo0(int32_t xc, int32_t yc, int32_t xr, int32_t yr)
{
    int32_t i = 0;
    float x0, y0, x1, y1;
    float x = xr * 0.4;
    float y = yr * 0.4;
    float a = 0.0, m;

    for (i = 0; i < 800 && !keyPressed(27); i++)
    {
        x0 = xc + xr * cos(a);
        y0 = yc + yr * sin(5 * a) * cos(a / 1.5);

        m = sin(a);
        x1 = x * m;
        y1 = y * m;

        drawLine(x0, y0, x0 + x1, y0 + y1, i / 12 + 32);
        drawLine(x0, y0, x0 + x1, y0 - y1, i / 12 + 32);
        a += M_PI / 400;
    }
}

void graphDemo1(int32_t xc, int32_t yc, int32_t xr, int32_t yr)
{
    int32_t i = 0;
    float x1, y1, x2, y2;
    float a = 0.0, m, n;

    for (i = 0; i < 500 && !keyPressed(27); i++)
    {
        m = sin(a);
        n = cos(a);

        x1 = xc + (1.2 * (xr + xr / 3.0 * (1 + 0.5 * cos(12 * a)) * n) * n);
        x2 = xc + (1.2 * (yr + yr / 3.0 * (1 + 0.5 * sin(12 * a)) * n) * n);
        y1 = yc - (xr + xr / 3.0 * (1 + 0.5 * cos(10 * a)) * m) * m;
        y2 = yc - (yr + yr / 2.0 * (1 + 0.5 * cos(15 * a)) * m) * m;

        drawLine(x1, y1, x2, y2, i / 7 + 32);
        a += M_PI / 250.5;
    }
}

void graphDemo2(int32_t xc, int32_t yc, int32_t r)
{
    int32_t i = 0;
    float x1, y1, x2, y2;
    float a = 0.0, f;

    for (i = 0; i < 1600 && !keyPressed(27); i++)
    {
        f = r * (1 + 0.25 * cos(20 * a)) * (1 + sin(4 * a));

        x1 = xc + f * cos(a);
        x2 = xc + f * cos(a + M_PI / 5);
        y1 = yc - f * sin(a);
        y2 = yc - f * sin(a + M_PI / 5);

        drawLine(x1, y1, x2, y2, i / 23 + 32);
        a += M_PI / 800;
    }
}

void graphDemo3(int32_t xc, int32_t yc, int32_t r)
{
    int32_t i = 0;
    float x1, y1, x2, y2;
    float a = 0.0, f;
    
    for (i = 0; i < 1600 && !keyPressed(27); i++)
    {
        f = r * (1 + 0.25 * cos(4 * a)) * (1 + sin(8 * a));

        x1 = xc + f * cos(a);
        x2 = xc + f * cos(a + M_PI / 8);
        y1 = yc - f * sin(a);
        y2 = yc - f * sin(a + M_PI / 8);

        drawLine(x1, y1, x2, y2, i / 23 + 32);
        a += M_PI / 800;
    }
}

void graphDemo4(int32_t xc, int32_t yc, int32_t r)
{
    int32_t i = 0;
    float x1, y1, x2, y2;
    float a = 0.0, e;

    for (i = 0; i < 800 && !keyPressed(27); i++)
    {
        e = r * (1 + 0.5 * sin(2.5 * a));

        x1 = xc + e * cos(a);
        x2 = xc + e * cos(a + M_PI / 4);
        y1 = yc - e * sin(a);
        y2 = yc - e * sin(a + M_PI / 4);

        drawLine(x1, y1, x2, y2, i / 12 + 32);
        a += M_PI / 200;
    }
}

void graphDemo5(int32_t xi, int32_t yi, int32_t r, int32_t xr, int32_t yr)
{
    float a, e;
    float x, y, sx, sy;
    int32_t n, phi, i, k;

    for (n = 2; n <= 7; n++)
    {
        for (phi = 1; phi <= 6; phi++)
        {
            k = !(n % 2) ? 2 : 1;
            a = 0;

            for (i = 0; i <= 15 * n * k && !keyPressed(27); i++)
            {
                e = r / 5.0 * sin(n * phi * a) + r * sin(n * a);
                x = xr * (n - 2) + xi + e * cos(a);
                y = yr * (phi - 1) + yi + e * sin(a);

                if (!i)
                {
                    moveTo(x, y);
                    sx = x;
                    sy = y;
                }
                
                lineTo(x, y, 6 * n + phi + 48);
                a += M_PI / 15.0 / n;
            }
            lineTo(sx, sy, 6 * n + phi + 48);
        }
    }
}

void graphDemo6(int32_t xc, int32_t yc, int32_t r)
{
    int32_t xx[120] = {0};
    int32_t yy[120] = {0};

    float x = 4 * r;
    float sx, sy, x1, y1, x2, y2;
    float theta, a = 0.0;

    int32_t px, py, i;

    for (i = 0; i < 120 && !keyPressed(27); i++)
    {
        theta = 66 * sqrt(fabs(cos(3 * a))) + 12 * sqrt(fabs(cos(9 * a)));
        xx[i] = theta * cos(a) * 1.2 / 320.0 * r;
        yy[i] = theta * sin(a) / 320.0 * r;
        a += M_PI / 60;
    }

    for (py = 1; py <= 2; py++)
    {
        for (px = 1; px <= 8; px++)
        {
            for (i = 0; i < 120 && !keyPressed(27); i++)
            {
                x1 = xx[i] + (px * r >> 1) - (r >> 2);
                y1 = yy[i] + (py * r >> 1) - (r >> 2);
                theta = 2 * M_PI * (x - x1) / x;
                x2 = xc + y1 * cos(theta);
                y2 = yc + y1 * sin(theta);
    
                if (i == 0)
                {
                    moveTo(x2, y2);
                    sx = x2;
                    sy = y2;
                }
                lineTo(x2, y2, (120 * (2 * py + px) + i) / 22 + 32);
            }
            lineTo(sx, sy, (120 * (2 * py + px) + i) / 22 + 32);
        }
    }
}

void graphDemo7(int32_t xc, int32_t yc, int32_t r)
{
    int32_t xx[120] = {0};
    int32_t yy[120] = {0};

    float theta, a = 0.0, m, n;
    float x = 4 * r, sx, sy, x1, y1, x2, y2;
    
    int32_t px, py, i;

    for (i = 0; i < 120 && !keyPressed(27); i++)
    {
        theta = 40 * sin(4 * (a + M_PI / 8));
        
        m = sin(a);
        n = cos(a);

        xx[i] = (theta * n + 45 * n * n * n) / 320.0 * r;
        yy[i] = (theta * m + 45 * m * m * m) / 320.0 * r;

        a += M_PI / 60;
    }

    for (py = 1; py <= 2; py++)
    {
        for (px = 1; px <= 8; px++)
        {
            for (i = 0; i < 120 && !keyPressed(27); i++)
            {
                x1 = xx[i] + (px * r >> 1) - (r >> 2);
                y1 = yy[i] + (py * r >> 1) - (r >> 2);

                theta = 2 * M_PI * (x - x1) / x;

                x2 = xc + y1 * cos(theta);
                y2 = yc + y1 * sin(theta);

                if (i == 0)
                {
                    moveTo(x2, y2);
                    sx = x2;
                    sy = y2;
                }
                lineTo(x2, y2, (120 * (2 * py + px) + i) / 22 + 32);
            }
            lineTo(sx, sy, (120 * (2 * py + px) + i) / 22 + 32);
        }
    }
}

void graphDemo8(int32_t xc, int32_t yc, int32_t d, int32_t r)
{
    int32_t i = 0;
    int32_t xx[120] = {0};
    int32_t yy[120] = {0};
        
    float theta, sc, a, m;
    float dd, un, uv, k, s, x, y, px, py, sx, sy, sq;

    a = 0.0;
    un = 12.0;
    uv = d / un;
    k = uv / 2.0;
    sc = uv / 100.0;
    dd = d / 2.0;

    for (i = 0; i < 120 && !keyPressed(27); i++)
    {
        theta = 90 * (0.8 + 0.2 * sin(12 * a)) * (0.5 + 0.5 * sin(4 * a));
        xx[i] = theta * cos(a);
        yy[i] = theta * sin(a);
        a += M_PI / 60;
    }

    for (px = 1; px <= un; px++)
    {
        for (py = 1; py <= un; py++)
        {
            for (i = 0; i < 120 && !keyPressed(27); i++)
            {
                x = xx[i] * sc + px * uv - dd - k;
                y = yy[i] * sc + py * uv - dd - k;
                sq = x * x + y * y;

                if (sq < r * r)
                {
                    s = (x < 0) ? -1 : 1;
                    theta = atan(y / (x + 0.1));
                    m = r * sin(2 * atan(sqrt(sq) / r));
                    x = s * m * cos(theta);
                    y = s * m * sin(theta);
                }

                x = x * 23.0 / 15 + xc;
                y = y * 23.0 / 15 + yc;

                if (i == 0)
                {
                    moveTo(x, y);
                    sx = x;
                    sy = y;
                }
                lineTo(x, y, (120 * (px + py) + i) / 42 + 32);
            }
            lineTo(sx, sy, (120 * (px + py) + i) / 42 + 32);
        }
    }
}

void graphDemo9(int32_t xc, int32_t yc, float rd)
{
    float a, aa, ls, di, r;
    const int32_t data[] = {7, 436, 245, 17, 775, 180, 31, 1020, 130};
    int32_t i, k, s;
    int32_t ste, re, x, y, px, py;
    
    srand(time(NULL));

    px = xc;
    py = yc;
    r = 50 * rd;
    s = 8 - (rand() % 5);
    k = !(s % 2) ? 2 : 1;
    a = 0.0;

    while (!keyPressed(27) && a <= k * M_PI + M_PI / 10.0 / s * 1.0)
    {
        x = (r / 4 * sin(3 * s * a) + r * sin(s * a)) * cos(a) + px;
        y = (r / 4 * sin(3 * s * a) + r * sin(s * a)) * sin(a) + py;
        if (a == 0.0) moveTo(x, y);
        lineTo(x, y, 32);
        a += M_PI / 8.0 / s * 1.0;
    }

    i = 0;
    for (re = 0; re < 3; re++)
    {
        ste = data[3 * re];
        di = data[3 * re + 1] / 6.0 * rd;
        r = data[3 * re + 2] / 6.0 * rd;

        if (re == 1) ls = (2 * M_PI / ste) - 0.1;
        else ls = 0.0;

        aa = 0.0;

        while (aa <= 2 * M_PI - ls)
        {
            px = xc + di * cos(aa);
            py = yc + di * sin(aa);
            s = 8 - (rand() % 5);
            k = !(s % 2) ? 2 : 1;
            a = 0.0;

            while (!keyPressed(27) && a <= k * M_PI + M_PI / 10.0 / s)
            {

                x = (r / 4 * sin(3 * s * a) + r * sin(s * a)) * cos(a) + px;
                y = (r / 4 * sin(3 * s * a) + r * sin(s * a)) * sin(a) + py;

                if (a == 0.0) moveTo(x, y);
                lineTo(x, y, i + 33);

                a += M_PI / 8.0 / s * 1.0;
            }
            
            aa += 2 * M_PI / ste;
            i++;
        }
    }

    a = 0.0;
    i = 0;

    while (a <= 14 * M_PI && !keyPressed(27))
    {
        x = xc + 250 * rd * (1 + 1.0 / 5 * sin(9.06 * a)) * cos(a);
        y = yc + 250 * rd * (1 + 1.0 / 5 * sin(9.06 * a)) * sin(a);
        if (a == 0.0) moveTo(x, y);
        lineTo(x, y, i % 72 + 32);
        a += M_PI / 60;
        i++;
    }
}

void initDemo10(int32_t num, int32_t n)
{
    int32_t i;
    float a = 0.0, r;

    switch (num)
    {
    case 1:
        for (i = 0; i < 120; i++)
        {
            r = 100 * (0.5 + 0.5 * sin(n * a));
            grData[i][0] = r * cos(a);
            grData[i][1] = r * sin(a);
            a += M_PI / 60;
        }
        break;

    case 2:
        for (i = 0; i < 120; i++)
        {
            r = 100 * (0.82 + 0.18 * sin(3 * n * a)) * (0.5 + 0.5 * sin(n * a));
            grData[i][0] = r * cos(a);
            grData[i][1] = r * sin(a);
            a +=  M_PI / 60;
        }
        break;

    case 3:
        for (i = 0; i < 120; i++)
        {
            r = 100 * (0.33 * sin(0.5 * n * a) + sin(n * a));
            grData[i][0] = r * cos(2 * a);
            grData[i][1] = r * sin(2 * a);
            a += M_PI / 30;
        }
        break;

    case 4:
        for (i = 0; i < 120; i++)
        {
            grData[i][0] = 100 * sin(n * a) * cos(a);
            grData[i][1] = 100 * sin(n * a + a) * sin(a);
            a += M_PI / 60;
        }
        break;
    }
}

void graphDemo10(int32_t xc, int32_t yc, int32_t rx, int32_t ry, int32_t col)
{
    int32_t i = 0;
    int32_t data[120][2] = {0};

    for (i = 0; i < 120 && !keyPressed(27); i++)
    {
        data[i][0] = grData[i][0] * rx / 100 + xc;
        data[i][1] = grData[i][1] * ry / 100 + yc;
    }

    for (i = 0; i < 119 && !keyPressed(27); i++) drawLine(data[i][0], data[i][1], data[i + 1][0], data[i + 1][1], col);
    drawLine(data[119][0], data[119][1], data[0][0], data[0][1], col);
}

void makePalette(uint8_t n, uint8_t r, uint8_t g, uint8_t b)
{
    int32_t i;
    int32_t white = 10;
    
    for (i = 0; i <= 63 - white; i++)
    {
        setRGB(n + i, ((r * i) / (63 - white)) << 2, ((g * i) / (63 - white)) << 2, ((b * i) / (63 - white)) << 2);
    }

    for (i = 0; i <= white; i++)
    {
        setRGB(n + i + 63 - white, (r + (63 - r) * i / white) << 2, (g + (63 - g) * i / white) << 2, (b + (63 - b) * i / white) << 2);
    }
}

void graphDemo11()
{
    int32_t x, y, i, col, frames = 0;
    
    srand(time(NULL));
    while (!keyPressed(27) && frames < 420)
    {
        x = rand() % cmaxX;
        y = rand() % cmaxY;
        col = (rand() % 4) * 64;
        for (i = 0; i < 64; i++) fillCircle(x + (64 - i) / 2, y + (64 - i) / 2, (64 - i) * 2, i + col);
        frames++;
    }
}

void checkBounds(int32_t a, int32_t c, int32_t *b)
{
    if (a > c) *b = -1;
    else if (a < 0) *b = 1;
}

void diagonalLine(int32_t x1, int32_t y1, int32_t x2, int32_t y2)
{	
    int32_t dst = 0;
    int32_t dxInc = 0, dyInc = 0;
    int32_t sxInc = 0, syInc = 0;
    int32_t sc = 0,	dc = 0;
    void *plotPixel = putPixelBob;

    __asm {
        mov    ecx, 1
        mov    edx, 1
        mov    edi, y2
        sub    edi, y1
        jge    keepY
        neg    edx
        neg    edi
    keepY:
        mov    dyInc, edx
        mov    esi, x2
        sub    esi, x1
        jge    keepX
        neg    ecx
        neg    esi
    keepX:
        mov    dxInc, ecx
        cmp    esi, edi
        jge    horzSeg
        xor    ecx, ecx
        xchg   esi, edi
        jmp    saveVal
    horzSeg:
        xor    edx, edx
    saveVal:
        mov    dst, edi
        mov    sxInc, ecx
        mov    syInc, edx
        mov    eax, dst
        shl    eax, 1
        mov    sc, eax
        sub    eax, esi
        mov    ebx, eax
        sub    eax, esi
        mov    dc, eax
        mov    ecx, x1
        mov    edx, y1
    mainLoop:
        dec    esi
        jz     finish

        push   edx
        push   ecx
        call   plotPixel
        pop    ecx
        pop    edx

        cmp    ebx, 0
        jge    dline
        add    ecx, sxInc
        add    edx, syInc
        add    ebx, sc
        jmp    mainLoop
    dline:
        add    ecx, dxInc
        add    edx, dyInc
        add    ebx, dc
        jmp    mainLoop
    finish:
    }
}

void graphDemo12()
{
    int32_t frames = 0;
    int32_t x1, y1, x2, y2;
    int32_t dx1, dx2, dy1, dy2;
    
    srand(time(NULL));

    x1 = rand() % lfbWidth;
    x2 = rand() % lfbWidth;
    y1 = rand() % lfbHeight;
    y2 = rand() % lfbHeight;
    
    dx1 = 1;
    dx2 = -1;
    dy1 = 1;
    dy2 = -1;
    
    while (!keyPressed(27) && frames < 4000)
    {
        x1 += dx1;
        x2 += dx2;
        y1 += dy1;
        y2 += dy2;
        
        checkBounds(x1, cmaxX, &dx1);
        checkBounds(x2, cmaxX, &dx2);
        checkBounds(y1, cmaxY, &dy1);
        checkBounds(y2, cmaxY, &dy2);
        
        if (x1 < 0) x1 = 0;
        if (x2 < 0) x2 = 0;
        if (y1 < 0) y1 = 0;
        if (y2 < 0) y2 = 0;

        if (frames % 8 == 0) waitRetrace();
        diagonalLine(x1, y1, x2, y2);
        frames++;
    }
}

void graphDemo13()
{
    int32_t i, j;
    POINT points[] = { {659, 336}, {452, 374}, {602, 128}, {509, 90}, {433, 164}, {300, 71}, {113, 166}, {205, 185}, {113, 279}, {169, 278}, {206, 334}, {263, 279}, {355, 129}, {301, 335}, {432, 204}, {433, 297}, {245, 467}, {414, 392}, {547, 523} };
    
    fillPolygon(points, sizeof(points) / sizeof(points[0]), 50);

    for (j = 50; j <= cmaxY - 50; j++)
    {
        for (i = 50; i <= cmaxX - 50; i++)
        {
            if (getPixel(i, j) == 50) putPixel(i, j, 16 + ((i + j) >> 2) % 192);
        }
    }
}

void graphDemo14()
{
    int32_t i, j;
    POINT randPoints[20] = {0};

    srand(time(NULL));
    randomPolygon(centerX, centerY, 150, 0.7, 0.4, 20, randPoints);
    fillPolygon(randPoints, 20, 50);

    for (j = 0; j <= cmaxY; j++)
    {
        for (i = 0; i <= cmaxX; i++)
        {
            if (getPixel(i, j) == 50) putPixel(i, j, 16 + ((i + j) >> 2) % 192);
        }
    }
}

void graphDemo15()
{
    int32_t i;    
    for (i = 0; i < cmaxY; i++) horizLine(0, i, cmaxX, 1 + (int32_t)(i / 1.87) % 255);
}

float FX1(float x, float y)
{
     float phi = sqrt(x * x + y * y);
     if (phi == 0.0) phi = FLT_MIN;
     return 10 * sin(phi) / phi;
}

float FX2(float x, float y)
{
     float phi = sqrt(x * x + y * y);
     return -phi + fabs(sin(phi));
}

float FX3(float x, float y)
{
    if (x == 0.0) x = FLT_MIN;
    if (y == 0.0) y = FLT_MIN;
    return 10 * sin(x) / x * sin(y) / y;
}

float FX4(float x, float y)
{
    if (x == 0.0) x = FLT_MIN;
    return 0.1 * (x * x + y * y) * sin(x) / x;
}

float FX5(float x, float y)
{
    return 0.2 * sin(x) * cos(y) - 3 * exp(-x * x - y * y) * cos(1.75 * (x * x + y * y));
}

float FX11(float u, float v)
{
    return (6 * cos(u)) * cos(v);
}

float FY11(float u, float v)
{
  return (3 * cos(u)) * sin(v);
}

float FZ11(float u, float v)
{
    v = 0;
    return 2 * sin(u) + v;
}

float FX21(float u, float v)
{
    return (6 + 3 * cos(u)) * cos(v);
}

float FY21(float u, float v)
{
  return (6 + 3 * cos(u)) * sin(v);
}

float FZ21(float u, float v)
{
    v = 0;
    return 3 * sin(u) + v;
}

float FX31(float u, float v)
{
    return (3 + 3 * cos(u)) * cos(v);
}

float FY31(float u, float v)
{
  return (3 + 3 * cos(u)) * sin(v);
}

float FZ31(float u, float v)
{
    v = 0;
    return 3 * sin(u) + v;
}

float FX41(float u, float v)
{
    v = 0;
    return u + v;
}

float FY41(float u, float v)
{
    u = 0;
    return v + u;
}

float FZ41(float u, float v)
{
    return u * u - v * v;
}

void familleDesCourbesEnU()
{
    float x, y, z;
    u = udebut;

    while (u <= ufin)
    {
        v = vdebut;
        x = FX(u, v);
        y = FY(u, v);
        z = FZ(u, v);
        deplaceEn(x, y, z);
        while (v <= vfin)
        {
            x = FX(u, v);
            y = FY(u, v);
            z = FZ(u, v);
            traceVers(x, y, z, 50);
            v += dv;
        }
        u += du;
    }
}

void familleDesCourbesEnV()
{
    float x, y, z;
    v = vdebut;

    while (v <= vfin)
    {
        u = udebut;
        x = FX(u, v);
        y = FY(u, v);
        z = FZ(u, v);
        deplaceEn(x, y, z);
        while (u <= ufin)
        {
            x = FX(u, v);
            y = FY(u, v);
            z = FZ(u, v);
            traceVers(x, y, z, 50);
            u += du;
        }
        v += dv;
    }
}

void initParameter1()
{
    projection = PARALLELE;
    gx1 = -9.2;
    gx2 = 9.2;
    gy1 = -9.2;
    gy2 = 9.2;
    lines = 90;
    vue = 'r';
    points = 120;
    de = 1;
    theta = 45;
    phi = 22;
}

void initParameter2()
{
    projection = PARALLELE;
    gx1 = -9.2;
    gx2 = 9.2;
    gy1 = -14.8;
    gy2 = 14.8;
    lines = 90;
    vue = 'r';
    points = 200;
    de = 1;
    theta = 45;
    phi = 20;
}

void initParameter3()
{
    projection = PARALLELE;
    gx1 = -9.5;
    gx2 = 9.5;
    gy1 = -9.5;
    gy2 = 9.5;
    lines = 90;
    vue = 'r';
    points = 200;
    de = 1;
    theta = 40;
    phi = 20;
}

void initParameter4()
{
    projection = PARALLELE;
    gx1 = -9.8;
    gx2 = 9.8;
    gy1 = -10.2;
    gy2 = 10.2;
    lines = 80;
    vue = 'r';
    points = 200;
    de = 1;
    theta = 45;
    phi = 32;
}

void initParameter5()
{
    projection = PARALLELE;
    gx1 = -4.0;
    gx2 = 4.0;
    gy1 = -2.8;
    gy2 = 2.8;
    lines = 50;
    vue = 'r';
    points = 200;
    de = 1;
    theta = 40;
    phi = 15;
}

void initParameter11()
{
    projection = PARALLELE;
    de = 55;
    theta = 60;
    phi = 30;
    udebut = -M_PI / 2;
    ufin = M_PI / 2;
    du = 0.2;
    vdebut = -M_PI;
    vfin = M_PI;
    dv = 0.2;
}

void initParameter21()
{
    projection = PARALLELE;
    de = 30;
    theta = 30;
    phi = 30;
    udebut = -M_PI;
    ufin = M_PI;
    du = 0.4;
    vdebut = -M_PI;
    vfin = M_PI;
    dv = 0.2;
}

void initParameter31()
{
    projection = PARALLELE;
    de = 40;
    theta = 40;
    phi = 30;
    udebut = -M_PI;
    ufin = M_PI;
    du = 0.4;
    vdebut = -M_PI;
    vfin = M_PI;
    dv = 0.2;
}

void initParameter41()
{
    projection = PARALLELE;
    de = 155;
    theta = 60;
    phi = 30;
    udebut = -1;
    ufin = 1;
    du = 0.1;
    vdebut = -1;
    vfin = 1;
    dv = 0.1;
}

void initDiverses()
{
    int32_t i;
    float aux;

    incx = (gx2 - gx1) / points;
    incy = (gy2 - gy1) / lines;
    
    c1 = 70;
    c2 = 710;
    c3 = 57;
    c4 = 145;
    
    memset(maxh, 0, sizeof(maxh));
    for (i = 0; i < LIMITX; i++) minh[i] = LIMITY;
    
    if (theta < 0 || theta > 180)
    {
        aux = gx1; gx1 = gx2; gx2 = aux; incx = -incx;
        aux = gy1; gy1 = gy2; gy2 = aux; incy = -incy;
    }
}

void rechercheFenetre()
{
    int32_t i, j;
    float x, y, z;
     
    for (i = 0; i < lines; i++)
    {
        y = gy2 - i * incy;
        for (j = 0; j < points; j++)
        {
            x = gx1 + j * incx;
            z = FX(x, y);
            projette(x, y, z);
            
            if (xproj < f1) f1 = xproj;
            if (xproj > f2) f2 = xproj;
            if (xproj < f3) f3 = yproj;
            if (xproj > f4) f4 = yproj;
        }
    }
}

void calculeEchelles()
{
    echx = (c2 - c1) / (f2 - f1);
    echy = (c4 - c3) / (f4 - f3);
    if (vue == 'r')
    {
        if (echx < echy) echy = echx;
        else echx = echy;
    }
}

void horizon(int32_t x1, int32_t y1, int32_t x2, int32_t y2)
{
    int32_t x, y, dx, pente;

    dx = SIGNED(x2 - x1);
    if (dx == 0)
    {
        maxh[(x2 + 1) % LIMITX] = max(maxh[x2 % LIMITX], y2);
        minh[(x2 + 1) % LIMITX] = min(minh[x2 % LIMITX], y2);
    }
    else
    {
        pente = (y2 - y1) / (x2 - x1);
        for (x = x2 + 1; x <= x1; x++)
        {
            y = pente * (x - x1) + y1;
            maxh[x % LIMITX] = max(maxh[x % LIMITX], y);
            minh[x % LIMITX] = min(minh[x % LIMITX], y);
        }
    }
}

void visibilite(int32_t x, int32_t y, int32_t *vs)
{
    if ((y < maxh[x % LIMITX]) && (y > minh[x % LIMITX])) *vs = 0;
    else if (y >= maxh[x % LIMITX]) *vs = 1; else *vs = -1;
}

void inter(int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t *taux, int32_t *xi, int32_t *yi)
{
    int32_t den, xii, yii;
    
    if (x2 == x1)
    {
        xii = x2;
        yii = taux[x2 % LIMITX];
    }
    else
    {
        den = y2 - y1 - taux[x2 % LIMITX] + taux[x1 % LIMITX];
        if (den)
        {
            xii = (x1 * (y2 - taux[x2 % LIMITX]) + x2 * (taux[x1 % LIMITX] - y1)) / den;
            yii = (y2 * taux[x1 % LIMITX] - y1 * taux[x2 % LIMITX]) / den;
        }
        else
        {
            xii = x2;
            yii = y2;
        }
    }
    
    *xi = xii;
    *yi = yii;
}

void dessinefonction()
{
    float x, y, z;

    int32_t xi, yi, i, j;
    int32_t xprec, yprec, xcour, ycour;
    
    for (i = 0; i < lines; i++)
    {
        x = gx1;
        y = gy2 - i * incy;
        z = FX(x, y);
        
        projette (x, y, z);
        
        xprec = (xproj - f1) * echx + c1;
        yprec = (yproj - f3) * echy + c3;

        visibilite(xprec, yprec, &visiPrec);
        
        for (j = 0; j < points && !keyPressed(27); j++)
        {
            x = gx1 + j * incx;
            z = FX(x, y);
            projette(x, y, z);
            
            xcour = (xproj - f1) * echx + c1;
            ycour = (yproj - f3) * echy + c3;
            
            visibilite(xcour, ycour, &visiCour);
            
            if (!maxh[xcour % LIMITX] || minh[xcour % LIMITX] == LIMITY) visiCour = visiPrec;
            
            if (visiCour == visiPrec)
            {
                if (visiCour == 1 || visiCour == -1)
                {
                    drawLine(xprec, LIMITY - yprec, xcour, LIMITY - ycour, 50);
                    horizon(xprec, yprec, xcour, ycour);
                }
            }
            else
            {
                if (visiCour == 0)
                {
                    if (visiPrec == 1) inter(xprec, yprec, xcour, ycour, maxh, &xi, &yi);
                    else inter(xprec, yprec, xcour, ycour, minh, &xi, &yi);
                    drawLine(xprec, LIMITY - yprec, xi, LIMITY - yi, 50);
                    horizon(xprec, yprec, xi, yi);
                }
                else
                {
                    if (visiCour == 1)
                    {
                        if (visiPrec == 0)
                        {
                            inter(xprec, yprec, xcour, ycour, maxh, &xi, &yi);
                            drawLine(xi, LIMITY - yi, xcour, LIMITY - ycour, 50);
                            horizon(xi, yi, xcour, ycour);
                        }
                        else
                        {
                            inter(xprec, yprec, xcour, ycour, minh, &xi, &yi);
                            drawLine(xprec, LIMITY - yprec, xi, LIMITY - yi, 50);
                            horizon(xprec, yprec, xi, yi);
                            inter(xprec, yprec, xcour, ycour, maxh, &xi, &yi);
                            drawLine(xi, LIMITY - yi, xcour, LIMITY - ycour, 50);
                            horizon(xi, yi, xcour, ycour);
                        }
                    }
                    else
                    {
                        if (visiPrec == 0)
                        {
                            inter(xprec, yprec, xcour, ycour, minh, &xi, &yi);
                            drawLine(xi, LIMITY - yi, xcour, LIMITY - ycour, 50);
                            horizon(xi, yi, xcour, ycour);
                        }
                        else
                        {
                            inter(xprec, yprec, xcour, ycour, maxh, &xi, &yi);
                            drawLine(xprec, LIMITY - yprec, xi, LIMITY - yi, 50);
                            horizon(xprec, yprec, xi, yi);
                            inter(xprec, yprec, xcour, ycour, minh, &xi, &yi);
                            drawLine(xi, LIMITY - yi, xcour, LIMITY - ycour, 50);
                            horizon(xi, yi, xcour, ycour);
                        }
                    }
                }
            }
            
            visiPrec = visiCour;
            xprec = xcour;
            yprec = ycour;
        }
    }
}

void affichage(int32_t range)
{
    int32_t i, j, width, height;
    int32_t startx = 0, starty = 2;
    uint8_t savefont = getFontType();

    char buff[80] = {0};
    const char *strTitle = "Shapes 3D Transform";
    
    setFontType(1);
    width = getFontWidth(strTitle);
    height = getFontHeight(strTitle);
    startx = centerX - (width >> 1);
    writeString(startx, starty, strTitle, 40, 1);

    setFontType(0);
    sprintf(buff, "X=[%.1f,%.1f] Y=[%.1f,%.1f] Theta=%.1f Phi=%.1f Lines=%d Points=%d", -gx1, gx1, -gy1, gy1, theta, phi, lines, points);
    writeString(centerX - (getFontWidth(buff) >> 1), cmaxY - getFontHeight(buff) - 2, buff, 37, 1);

    for (i = starty; i < height; i++)
    {
        for (j = startx; j < startx + width; j++) if (getPixel(j, i) == 40) putPixel(j, i, 56 + (i / 3));
    }
    
    for (i = 50; i < cmaxX - 50; i++)
    {
        for (j = 50; j < cmaxX - 50; j++) if (getPixel(i, j) == 50) putPixel(i, j, 32 + ((i + j) / range) % 72);
    }
    
    setFontType(savefont);
}

void resetParameter()
{
    c1 = c2 = c3 = c4 = lines = points = 0;
    gx1 = gx2 = gy1 = gy2 = incx = incy = f1 = rho = f2 = f3 = f4 = echx = 0;
    echy = de = theta = phi = aux1 = aux2 = aux3 = aux4 = 0;
    aux5 = aux6 = aux7 = aux8 = xobs = yobs = zobs = xproj = yproj = 0;
}

void setPixelChar()
{
    int32_t i, j, k;
    char buff[2] = {0};
    GFX_FONT *font = getFont(fontType);
    memset(chrPixels, 0, sizeof(chrPixels));

    for (i = 32; i < CHR_MAX; i++)
    {
        sprintf(buff, "%c", i);
        writeString(0, 0, buff, 1, 0);
        for (j = 0; j < font->info.width; j++)
        {
            for (k = 0; k < font->info.height; k++) if (getPixel(j, k) == 1) chrPixels[i][j][k] = 1;
        }
        fillRect(0, 0, font->info.width, font->info.height, 0);
    }
}

void scrollLed(const char *msg)
{
    uint8_t chr = 0;
    int32_t i, j, k, m = 0;
    const int32_t zx = 5, zy = 3, sy = 50;
    GFX_FONT *font = getFont(fontType);

    drawRectEx(0, sy, cmaxX - 1, sy + (font->info.height << 2), 50, 10);
    
    while (!keyPressed(27))
    {
        chr = msg[m];
        for (k = 0; k < font->info.width; k++)
        {
            for (i = 0; i < CHR_NUM * font->info.width - 1; i++)
            {
                for (j = 0; j < font->info.height; j++) chrBuff[i][j] = chrBuff[i + 1][j];
            }

            for (j = 0; j < font->info.height; j++) chrBuff[CHR_NUM * font->info.width - 1][j] = chrPixels[chr][k][j];
            
            for (i = 0; i < CHR_NUM * font->info.width; i++)
            {
                for (j = 0; j < font->info.height; j++)
                {
                    if (chrBuff[i][j]) putPixel(font->info.width + zx * (i + 2), sy + zy * (j + 2) + 4, 14);
                    else putPixel(font->info.width + zx * (i + 2), sy + zy * (j + 2) + 4, 0);
                }
            }
            delay(15);
        }
        m++;
        if (m >= strlen(msg)) break;
    }
}

// Display sprite using visual page and active page
void displaySprite(const char *fname)
{
    GFX_BITMAP bmp;
    GFX_IMAGE img, img1, img2;

    uint32_t frames = 0;
    int32_t x, y, lx, ly, dx, dy, f1, f2;
    const char *bkg[] = {"1lan8.bmp", "1lan16.bmp", "1lan24.bmp", "1lan32.bmp"};

    dx = 8;
    dy = 1;
    f1 = 0;
    f2 = 0;
    x = (20 + dx) >> 1;
    y = (12 + dy) >> 1;

    // load sprite bitmap
    if (!loadBitmap(fname, &bmp)) fatalError("displaySprite: cannot load sprite bitmap: %s\n", fname);

    // convert bitmap to image
    convertBitmap(&bmp, &img);
    closeBitmap(&bmp);

    // create buffer image data
    if (!newImage(img.mWidth, img.mHeight, &img1)) fatalError("displaySprite: cannot alloc image buffer.\n");
    if (!newImage(img.mWidth, img.mHeight, &img2)) fatalError("displaySprite: cannot alloc image buffer.\n");

    // display bitmap
    showBitmap(bkg[bytesPerPixel - 1]);

    // copy 1st page to 2nd page 
    copyPage(0, 1);
    setActivePage(1);

    while (!keyPressed(27) && frames < 220)
    {
        // display on 1st page
        setActivePage(0);
        setVisualPage(1);
        if (f1) putImage(lx, ly, &img1);

        lx = x;
        ly = y;
        f1 = 1;

        x += dx;
        if (x > cmaxX - img.mWidth || x <= 0)
        {
            x -= dx;
            dx = -dx;
        }

        y += dy;
        if (y > cmaxY - img.mHeight || y <= 0)
        {
            y -= dy;
            dy = -dy;
        }

        dy++;
        getImage(x, y, img.mWidth, img.mHeight, &img1);
        putSprite(x, y, 0, &img);

        // draw on 2nd page
        setActivePage(1);
        setVisualPage(0);
        if (f2) putImage(lx, ly, &img2);

        lx = x;
        ly = y;
        f2 = 1;

        x += dx;
        if (x > cmaxX - img.mWidth || x <= 0)
        {
            x -= dx;
            dx = -dx;
        }

        y += dy;
        if (y > cmaxY - img.mHeight || y <= 0)
        {
            y -= dy;
            dy = -dy;
        }

        dy++;
        getImage(x, y, img.mWidth, img.mHeight, &img2);
        putSprite(x, y, 0, &img);
        frames++;
    }

    freeImage(&img1);
    freeImage(&img2);
    freeImage(&img);
    setActivePage(0);
    setVisualPage(0);
}

// show plasma effect
void displayPlasma()
{
    const char *str[] = {
        "That's all folks!",
        "I hope, you enjoyed this demo",
        "Thank you for making use of my programs",
        "More sourcecode from me available @ my homepage:",
        "http://codedemo.net",
        "",
        "If you have some improvements, additions,",
        "bug reports or something else, please email me",
        "",
        "Nguyen Ngoc Van - pherosiden@gmail.com",
        "",
        "Greets fly to:",
        "scene.org",
        "lodev.org",
        "sources.ru",
        "permadi.com",
        "eyecandyarchive.com",
        "crossfire-designs.de",
        "And all persons which helped me in any way.",
        "",
        "CYA!"
    };

    GFX_IMAGE src, dst;
    RGB pal[256] = {0};

    int32_t size = sizeof(str) / sizeof(str[0]);
    int32_t ypos = 0, endpos = 0, page = 0;
    int32_t x = 0, y = 0, decx = 0, decy = 0;

    uint32_t frames = 0;
    uint8_t dx = 0, dy = 0;
    uint8_t sint[256] = {0};
    uint8_t cost[256] = {0};

    // load text font
    if (!loadFont("HYENA.XFN", 0)) fatalError("displayPlasma: cannot load font.\n");

    // plasma image buffer
    if (!newImage(160, 120, &src)) fatalError("displayPlasma: cannot allocate image buffer.\n");

    // scale palsma image buffer
    if (!newImage(lfbWidth, lfbHeight, &dst)) fatalError("displayPlasma: cannot allocate image buffer.\n");

    initPlasma(sint, cost);

    // display plasma
    while (!keyPressed(27) && frames < 880)
    {
        // create plasma buffer and display on screen
        createPlasma(&dx, &dy, sint, cost, &src);
        waitRetrace();
        putImage(x, y, &src);

        // check limitation
        if (decx) x--; else x++;
        if (decy) y--; else y++;
        if (x <= 0 || x >= cmaxX - src.mWidth) decx = !decx;
        if (y <= 0 || y >= cmaxY - src.mHeight) decy = !decy;
        frames++;
    }

    // setup font palette
    for (x = 0; x < 32; x++) setRGB(x, x << 3, x << 3, x << 3);
    getPalette(pal);

    page = 1;
    ypos = lfbHeight;

    // display scale image and scroll text
    do {
        createPlasma(&dx, &dy, sint, cost, &src);
        scaleImage(&dst, &src, 0);
        setActivePage(page);
        putImage(0, 0, &dst);
        endpos = drawText(ypos--, str, size);
        if (endpos <= 98) fadeDown(pal);
        setVisualPage(page);
        page = !page;
    } while (ypos > -32767 && endpos > -30 && !keyPressed(27));

    // cleanup...
    freeImage(&src);
    freeImage(&dst);
    closeFont(0);
}

void quitMessage()
{
    closeVesaMode();
    printf("+-----------------------------------------------------+\n");
    printf("|    GFXLIB demo (c) 1998 - 2002 by Nguyen Ngoc Van   |\n");
    printf("|        Full support 8/15/16/24/32 bits color        |\n");
    printf("|          Support load/save BMP & PNG files          |\n");
    printf("|    Using Linear Frame Buffer for best performance   |\n");
    printf("|           Optimize code by using assembly           |\n");
    printf("|           Code by: Nguyen Ngoc Van                  |\n");
    printf("|             Email: pherosiden@gmail.com             |\n");
    printf("|           Website: http://codedemo.net              |\n");
    printf("|         Reference: http://crossfire-designs.de      |\n");
    printf("+-----------------------------------------------------+\n");
}

int main(int argc, const char* argv[])
{
    POINT	point[50] = {0};
    RGB 	pal1[256] = {0};
    RGB 	pal2[256] = {0};

    float ratio = 0.0;
    float rept = 0.0;

    int32_t i, j, a = 70, b = 20;
    int32_t introy = 0, msgy = 20;
    const int32_t col[] = {50, 35, 32, 40, 40, 32};

    const char *logoGFX = "GFXLIB";

    const char *strWelcome[] = {
        "GFXLIB Library Demo",
        "Full supports 8/15/16/24/32 bits color",
        "Load/Save BMP & PNG 32 bits image",
        "Copyright (c) 1998 - 2002 by Nguyen Ngoc Van",
        "Please wait to continue..."
    };
    
    char strTitle[][64] = {
        "Khoa Co6ng Nghe65 Tho6ng Tin - Kho1a 2000",
        "Tru7o72ng D9a5i Ho5c Ky4 Thua65t TP.HCM - HUTECH",
        "Thu7 Vie65n D9o62 Ho5a VESA 8/15/16/24/32 bits Ma2u",
        "Copyright (c) 1998 - 2002 by Nguye64n Ngo5c Va6n",
        "Trang chu3: http://codedemo.net"
    };

    char strScroll[] = "*** Ca1m o7n ca1c ba5n d9a4 su73 du5ng chu7o7ng tri2nh na2y. Ba5n co1 the63 ta3i toa2n bo65 ma4 nguo62n cu3a chu7o7ng tri2nh ta5i d9i5a chi3 http://codedemo.net. Chu1c Ca1c Ba5n Tha2nh Co6ng       ";
    char strBanner[] = "Light Banner (c) 1998 - 2002 Nguye64n Ngo5c Va6n";
    char strLoading[] = "D9ang ta3i du74 lie65u a3nh PNG & BMP 32bit ma2u, vui lo2ng d9o75i mo65t la1t....";
    const int32_t numTitle = sizeof(strTitle) / sizeof(strTitle[0]);

    // First message from GFXLIB
    _clearscreen(0);
    printf("GFXLIB initializing....\n");

    // init clock time and random number generation
    initGfxLib(1, quitMessage);
    if (!loadFont("FONTVN.XFN", 0)) fatalError("Cannot load font!\n");
    if (!setVesaMode(800, 600, 32, 85)) fatalError("Cannot init video mode.\n");

    makeFont(strLoading);
    writeString(centerX - (getFontWidth(strLoading) >> 1), centerY - getFontHeight(strLoading), strLoading, fromRGB(255, 255, 64), 0);

    showPNG("caibang.png");
    if (argc > 1 && !strcmp(argv[1], "-s")) saveScreen("screen36.png");
    delay(3000);
    fadeCircle(2, 0);

    showBitmap("1lan32.bmp");
    if (argc > 1 && !strcmp(argv[1], "-s")) saveScreen("screen37.bmp");
    delay(3000);
    fadeCircle(3, 0);

    handleMouse("mouse32.bmp");
    displaySprite("smile32.bmp");
    if (argc > 1 && !strcmp(argv[1], "-s")) saveScreen("screen38.bmp");

    closeFont(0);
    closeVesaMode();

    if (!setVesaMode(800, 600, 8, 85)) fatalError("Cannot init video mode.\n");
    introy = centerY - ((numTitle * CHR_HEIGHT + 20 + b * 2) >> 1);
    
    switch (lfbWidth)
    {
        case  640: ratio = 1.0;	 break;
        case  800: ratio = 1.25; break;
        case 1024: ratio = 1.75; break;
        case 2048: ratio = 2.25; break;
        default:   ratio = 1.5;	 break;
    }

    getPalette(pal2);
    setBlackPalette();
    
    if (!loadFont("ODHL.XFN", 0)) fatalError("Cannot load font!\n");
    writeString(centerX - (getFontWidth(strWelcome[0]) >> 1), msgy, strWelcome[0], 40, 1);
    writeString(centerX - (getFontWidth(strWelcome[1]) >> 1), msgy + 60, strWelcome[1], 32, 1);
    writeString(centerX - (getFontWidth(strWelcome[2]) >> 1), msgy + 120, strWelcome[2], 35, 1);
    writeString(centerX - (getFontWidth(strWelcome[3]) >> 1), msgy + 170, strWelcome[3], 35, 1);
    writeString(centerX - (getFontWidth(strWelcome[4]) >> 1), cmaxY - 70, strWelcome[4], 35, 1);
    closeFont(0);

    fadeIn(pal2);
    if (argc > 1 && !strcmp(argv[1], "-s")) saveScreen("screen01.bmp");
    delay(3000);
    fadeRollo(2, 0);

    clearScreen(0);
    setBlackPalette();
    graphDemo0(centerX, centerY, 200 * ratio, 100 * ratio);
    fadeIn(pal2);
    rotatePalette(32, 103, 72);
    if (argc > 1 && !strcmp(argv[1], "-s")) saveScreen("screen02.bmp");

    clearScreen(0);
    setBlackPalette();
    graphDemo1(centerX, centerY, 160 * ratio, 40 * ratio);
    fadeIn(pal2);
    rotatePalette(32, 103, 72);
    if (argc > 1 && !strcmp(argv[1], "-s")) saveScreen("screen03.bmp");
    
    clearScreen(0);
    setBlackPalette();
    graphDemo2(centerX, centerY, 80 * ratio);
    fadeIn(pal2);
    rotatePalette(32, 103, 72);
    if (argc > 1 && !strcmp(argv[1], "-s")) saveScreen("screen04.bmp");
    
    clearScreen(0);
    setBlackPalette();
    graphDemo3(centerX, centerY, 80 * ratio);
    fadeIn(pal2);
    rotatePalette(32, 103, 72);
    if (argc > 1 && !strcmp(argv[1], "-s")) saveScreen("screen05.bmp");
    
    clearScreen(0);
    setBlackPalette();
    graphDemo4(centerX, centerY, 120 * ratio);
    fadeIn(pal2);
    rotatePalette(32, 103, 72);
    if (argc > 1 && !strcmp(argv[1], "-s")) saveScreen("screen06.bmp");
    
    clearScreen(0);
    setBlackPalette();
    graphDemo5(cmaxX / 7, cmaxY / 5 - 10, 28 * ratio, 90 * ratio, 62 * ratio);
    fadeIn(pal2);
    rotatePalette(32, 103, 72);
    if (argc > 1 && !strcmp(argv[1], "-s")) saveScreen("screen07.bmp");
    
    clearScreen(0);
    setBlackPalette();
    graphDemo6(centerX, centerY, 200 * ratio);
    fadeIn(pal2);
    rotatePalette(32, 103, 72);
    if (argc > 1 && !strcmp(argv[1], "-s")) saveScreen("screen08.bmp");
    
    clearScreen(0);
    setBlackPalette();
    graphDemo7(centerX, centerY, 200 * ratio);
    fadeIn(pal2);
    rotatePalette(32, 103, 72);
    if (argc > 1 && !strcmp(argv[1], "-s")) saveScreen("screen09.bmp");
    
    clearScreen(0);
    setBlackPalette();
    graphDemo8(centerX, centerY, 245 * ratio, 100 * ratio);
    fadeIn(pal2);
    rotatePalette(32, 103, 72);
    if (argc > 1 && !strcmp(argv[1], "-s")) saveScreen("screen10.bmp");
    
    clearScreen(0);
    setBlackPalette();
    graphDemo9(centerX, centerY, 0.6 * ratio);
    fadeIn(pal2);
    rotatePalette(32, 103, 72);
    if (argc > 1 && !strcmp(argv[1], "-s")) saveScreen("screen11.bmp");
    
    clearScreen(0);
    setBlackPalette();
    
    initDemo10(1, 4);
    for (i = 0; i <= 95; i++)
    graphDemo10(centerX >> 1, centerY >> 1, 140 - i, 140 - i, 64 + i / 3);
    
    initDemo10(2, 2);
    for (i = 0; i <= 119; i++)
    graphDemo10(centerX + (centerX >> 1), centerY >> 1, 170 - i, 170 - i, 64 + i / 3);
    
    initDemo10(3, 5);
    for (i = 0; i <= 39; i++)
    graphDemo10(centerX >> 1, centerY + (centerY >> 1), 110 - (i << 1), 110 - (i << 1), 83 - (i >> 1));
    
    initDemo10(4, 7);
    for (i = 0; i <= 19; i++)
    graphDemo10(centerX + (centerX >> 1), centerY + (centerY >> 1), 130 - (i << 2), 130 - (i << 2), 64 + i);
    fadeIn(pal2);
    rotatePalette(64, 103, 40);
    if (argc > 1 && !strcmp(argv[1], "-s")) saveScreen("screen12.bmp");

    clearScreen(0);
    makeLinearPalette();
    getPalette(pal1);
    setBlackPalette();
    drawPolygon(centerX, centerY, centerX - 140, 11, 1);
    fadeIn(pal1);
    rotatePalette(16, 177, 162);
    if (argc > 1 && !strcmp(argv[1], "-s")) saveScreen("screen13.bmp");
    
    clearScreen(0);
    setBlackPalette();

    findRepeat(&rept);
    drawCylodiod(cmaxX / 7 - 20, cmaxY / 5 - 30, 80, 1, 1, rept, 40);
    drawCylodiod(cmaxX / 3 + 30, cmaxY / 5 - 30, 80, 1, 2, rept, 40);
    drawCylodiod(centerX + 90, cmaxY / 5 - 30, 80, 1, 3, rept, 40);
    drawCylodiod(cmaxX - 100, cmaxY / 5 - 30, 80, 2, 1, rept, 40);
    
    drawCylodiod(cmaxX / 7 - 20, centerY - 10, 80, 2, 2, rept, 40);
    drawCylodiod(cmaxX / 3 + 30, centerY - 10, 80, 2, 3, rept, 40);
    drawCylodiod(centerX + 90, centerY - 10, 80, 3, 1, rept, 40);
    drawCylodiod(cmaxX - 100, centerY - 20, 80, 3, 2, rept, 40);
    
    drawCylodiod(cmaxX / 7 - 20, cmaxY - 100, 80, 3, 3, rept, 40);
    drawCylodiod(cmaxX / 3 + 30, cmaxY - 100, 80, 4, 1, rept, 40);
    drawCylodiod(centerX + 90, cmaxY - 100, 80, 4, 2, rept, 40);
    drawCylodiod(cmaxX - 100, cmaxY - 100, 80, 4, 3, rept, 40);
    fadeIn(pal2);
    if (argc > 1 && !strcmp(argv[1], "-s")) saveScreen("screen14.bmp");
    delay(3000);

    clearScreen(0);
    setBlackPalette();

    drawCylodiod(cmaxX / 7 - 20, cmaxY / 5 - 30, 80, 5, 1, rept, 40);
    drawCylodiod(cmaxX / 3 + 30, cmaxY / 5 - 30, 80, 5, 2, rept, 40);
    drawCylodiod(centerX + 90, cmaxY / 5 - 30, 80, 5, 3, rept, 40);
    drawCylodiod(cmaxX - 100, cmaxY / 5 - 30, 80, 5, 4, rept, 40);

    drawCylodiod(cmaxX / 7 - 20, centerY - 10, 80, 1, 4, rept, 40);
    drawCylodiod(cmaxX / 3 + 30, centerY - 10, 80, 2, 4, rept, 40);
    drawCylodiod(centerX + 90, centerY - 10, 80, 3, 4, rept, 40);
    drawCylodiod(cmaxX - 100, centerY - 20, 80, 4, 4, rept, 40);
    
    drawCylodiod(cmaxX / 7 - 20, cmaxY - 100, 80, 6, 1, rept, 40);
    drawCylodiod(cmaxX / 3 + 30, cmaxY - 100, 80, 6, 2, rept, 40);
    drawCylodiod(centerX + 90, cmaxY - 100, 80, 6, 3, rept, 40);
    drawCylodiod(cmaxX - 100, cmaxY - 100, 80, 6, 4, rept, 40);
    fadeIn(pal2);
    if (argc > 1 && !strcmp(argv[1], "-s")) saveScreen("screen15.bmp");
    delay(3000);

    clearScreen(0);
    setBlackPalette();
    rotatePolygon(point, 6, centerX, centerY, centerX - 140, 100, 20, 40);
    fadeIn(pal2);
    rotatePalette(40, 103, 64);
    if (argc > 1 && !strcmp(argv[1], "-s")) saveScreen("screen16.bmp");
    
    clearScreen(0);
    setBlackPalette();
    randomPoly(point, 12, cmaxX, cmaxY, 40, 20, 37);
    fadeIn(pal2);
    rotatePalette(37, 103, 67);
    if (argc > 1 && !strcmp(argv[1], "-s")) saveScreen("screen17.bmp");

    clearScreen(0);
    setBlackPalette();
    drawHexagon(point, 12, centerX, centerY, 35, centerX - 140, 20, 40);
    fadeIn(pal2);
    rotatePalette(40, 103, 64);
    if (argc > 1 && !strcmp(argv[1], "-s")) saveScreen("screen18.bmp");

    clearScreen(0);
    makePalette(0, 63, 32, 16);
    makePalette(64, 32, 63, 16);
    makePalette(128, 16, 16, 63);
    makePalette(128 + 64, 63, 16, 16);
    graphDemo11();
    if (argc > 1 && !strcmp(argv[1], "-s")) saveScreen("screen19.bmp");
    fadeRollo(2, 0);
    
    clearScreen(0);
    makeFunkyPalette();
    graphDemo12();
    if (argc > 1 && !strcmp(argv[1], "-s")) saveScreen("screen20.bmp");
    fadeRollo(1, 0);

    clearScreen(0);
    makeLinearPalette();
    getPalette(pal1);
    setBlackPalette();
    graphDemo13();
    fadeIn(pal1);
    rotatePalette(16, 207, 192);
    if (argc > 1 && !strcmp(argv[1], "-s")) saveScreen("screen21.bmp");

    clearScreen(0);
    makeLinearPalette();
    getPalette(pal1);
    setBlackPalette();
    graphDemo14();
    fadeIn(pal1);
    rotatePalette(16, 207, 192);
    if (argc > 1 && !strcmp(argv[1], "-s")) saveScreen("screen22.bmp");

    clearScreen(0);
    makeRainbowPalette();
    getPalette(pal1);
    setBlackPalette();
    graphDemo15();
    fadeIn(pal1);
    rotatePalette(1, 255, 255);
    if (argc > 1 && !strcmp(argv[1], "-s")) saveScreen("screen23.bmp");

    clearScreen(0);
    setBlackPalette();

    if (!loadFont("SYS8X16.XFN", 0)) fatalError("Cannot load font!\n");
    if (!loadFont("TRIP.XFN", 1)) fatalError("Cannot load font!\n");
    
    FX = FX1;

    initParameter1();
    initDiverses();
    initProjection();
    rechercheFenetre();
    calculeEchelles();
    dessinefonction();
    affichage(15);

    fadeIn(pal2);
    rotatePalette(32, 103, 45);
    if (argc > 1 && !strcmp(argv[1], "-s")) saveScreen("screen24.bmp");
    fadeMin();
    
    clearScreen(0);
    resetParameter();

    FX = FX2;

    initParameter2();
    initDiverses();
    initProjection();
    rechercheFenetre();
    calculeEchelles();
    dessinefonction();
    affichage(15);

    fadeIn(pal2);
    rotatePalette(32, 103, 45);
    if (argc > 1 && !strcmp(argv[1], "-s")) saveScreen("screen25.bmp");
    fadeMin();

    clearScreen(0);
    resetParameter();

    FX = FX3;

    initParameter3();
    initDiverses();
    initProjection();
    rechercheFenetre();
    calculeEchelles();
    dessinefonction();
    affichage(15);

    fadeIn(pal2);
    rotatePalette(32, 103, 45);
    if (argc > 1 && !strcmp(argv[1], "-s")) saveScreen("screen26.bmp");
    fadeMin();

    clearScreen(0);
    resetParameter();

    FX = FX4;

    initParameter4();
    initDiverses();
    initProjection();
    rechercheFenetre();
    calculeEchelles();
    dessinefonction();
    affichage(15);

    fadeIn(pal2);
    rotatePalette(32, 103, 45);
    if (argc > 1 && !strcmp(argv[1], "-s")) saveScreen("screen27.bmp");
    fadeMin();

    clearScreen(0);
    resetParameter();

    FX = FX5;

    initParameter5();
    initDiverses();
    initProjection();
    rechercheFenetre();
    calculeEchelles();
    dessinefonction();
    affichage(15);

    fadeIn(pal2);
    rotatePalette(32, 103, 45);
    if (argc > 1 && !strcmp(argv[1], "-s")) saveScreen("screen28.bmp");
    fadeMin();
    
    FX = FX11;
    FY = FY11;
    FZ = FZ11;
    
    clearScreen(0);
    initParameter11();
    initProjection();
    familleDesCourbesEnU();
    familleDesCourbesEnV();
    affichage(14);

    fadeIn(pal2);
    rotatePalette(32, 103, 72);
    if (argc > 1 && !strcmp(argv[1], "-s")) saveScreen("screen29.bmp");
    fadeMin();

    FX = FX21;
    FY = FY21;
    FZ = FZ21;
    
    clearScreen(0);
    initParameter21();
    initProjection();
    familleDesCourbesEnU();
    familleDesCourbesEnV();
    affichage(14);
    
    fadeIn(pal2);
    rotatePalette(32, 103, 72);
    if (argc > 1 && !strcmp(argv[1], "-s")) saveScreen("screen30.bmp");
    fadeMin();

    FX = FX31;
    FY = FY31;
    FZ = FZ31;

    clearScreen(0);
    initParameter31();
    initProjection();
    familleDesCourbesEnU();
    familleDesCourbesEnV();
    affichage(14);
    
    fadeIn(pal2);
    rotatePalette(32, 103, 72);
    if (argc > 1 && !strcmp(argv[1], "-s")) saveScreen("screen31.bmp");
    fadeMin();

    FX = FX41;
    FY = FY41;
    FZ = FZ41;

    clearScreen(0);
    initParameter41();
    initProjection();
    familleDesCourbesEnU();
    familleDesCourbesEnV();
    affichage(14);
    
    fadeIn(pal2);
    rotatePalette(32, 103, 72);
    if (argc > 1 && !strcmp(argv[1], "-s")) saveScreen("screen32.bmp");
    fadeMin();

    closeFont(0);
    closeFont(1);
    clearScreen(0);
    
    j = introy;
    if (!loadFont("FONTVN.XFN", 0)) fatalError("Cannot load font!\n");
    for (i = 0; i < numTitle; i++)
    {
        j += i * getFontHeight(strTitle[i]);
        makeFont(strTitle[i]);
        writeString(centerX - (getFontWidth(strTitle[i]) >> 1), introy + i * getFontHeight(strTitle[i]), strTitle[i], col[i], 1);
    }
    
    fillEllipse(centerX, j - introy, a, b, 50);
    writeString(centerX - (getFontWidth(logoGFX) >> 1), j - introy - CHR_WIDTH, logoGFX, 32, 1);

    fadeIn(pal2);
    memcpy(&pal1[32], &pal2[32], 72 * sizeof(RGB));
    rotatePalette(32, 103, 250);
    if (argc > 1 && !strcmp(argv[1], "-s")) saveScreen("screen33.bmp");

    clearScreen(0);
    setPalette(pal2);
    setPixelChar();
    makeFont(strBanner);
    writeString(centerX - (getFontWidth(strBanner) >> 1), cmaxY - getFontHeight(strBanner) - 2, strBanner, 40, 1);
    makeFont(strScroll);
    scrollLed(strScroll);
    if (argc > 1 && !strcmp(argv[1], "-s")) saveScreen("screen34.bmp");

    closeFont(0);
    clearScreen(0);
    displayPlasma();
    if (argc > 1 && !strcmp(argv[1], "-s")) saveScreen("screen35.bmp");

    quitMessage();
    return 0;
}
