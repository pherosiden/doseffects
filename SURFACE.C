/*---------------------------------------------------*/
/* Packet  : Demo & Effect                           */
/* title   : 3D transform                            */
/* Author  : Nguyen Ngoc Van                         */
/* Memory  : Small                                   */
/* Heaps   : 64K                                     */
/* Address : pherosiden@gmail.com                    */
/* Website : http://www.codedemo.net                 */
/* Created : 28/04/1998                              */
/* Update  : 10/11/2000                              */
/* Use protect mode 32bit DOS                        */
/* Please sent to me any bugs or suggests.           */
/* You can use freely this code. Have fun :)         */
/*---------------------------------------------------*/

#include "gfxlib.c"

#define LIMX 640
#define LIMY 400

#define Signe(x) (((x) > 0) ? 1 : ((x) < 0) ? -1 : 0)

int32_t hmax[LIMX];
int32_t hmin[LIMX];

int32_t c1, c2, c3, c4;
int32_t nbreLignes, nbrePoints;

float xa, xb, ya, yb;
float incx, incy;
float f1, f2, f3, f4;
float echx, echy;

char vue = 'R';
int32_t visiPrec = 0, visiCour = 0;

float (*FX)(float, float);

float FX1(float x, float y)
{
     float phi = sqrt(x * x + y * y);
     return 10 * sin(phi) / phi;
}

float FX2(float x, float y)
{
     float phi = sqrt(x * x + y * y);
     return -phi + fabs(sin(phi));
}

float FX3(float x, float y)
{
    return 10 * sin(x) / x * sin(y) / y;
}

float FX4(float x, float y)
{
    return 0.1 * (x * x + y * y) * sin(x) / x;
}

float FX5(float x, float y)
{
    return 0.2 * sin(x) * cos(y) - 3 * exp(-x * x - y * y) * cos(1.75 * (x * x + y * y));
}

void initParameter1()
{
    xa = -9.2;
    xb = 9.2;
    ya = -9.2;
    yb = 9.2;
    nbreLignes = 90;
    vue = 'R';
    nbrePoints = 120;
    de = 1;
    projection = PARALLELE;
    theta = 45;
    phi = 22;
}

void initParameter2()
{
    xa = -9.2;
    xb = 9.2;
    ya = -14.8;
    yb = 14.8;
    nbreLignes = 90;
    vue = 'R';
    nbrePoints = 200;
    de = 1;
    projection = PARALLELE;
    theta = 45;
    phi = 20;
}

void initParameter3()
{
    xa = -9.5;
    xb = 9.5;
    ya = -9.5;
    yb = 9.5;
    nbreLignes = 90;
    vue = 'R';
    nbrePoints = 200;
    de = 1;
    projection = PARALLELE;
    theta = 40;
    phi = 20;
}

void initParameter4()
{
    xa = -9.8;
    xb = 9.8;
    ya = -10.2;
    yb = 10.2;
    nbreLignes = 80;
    vue = 'R';
    nbrePoints = 200;
    de = 1;
    projection = PARALLELE;
    theta = 45;
    phi = 32;
}

void initParameter5()
{
    xa = -4.0;
    xb = 4.0;
    ya = -2.8;
    yb = 2.8;
    nbreLignes = 50;
    vue = 'R';
    nbrePoints = 200;
    de = 1;
    projection = PARALLELE;
    theta = 40;
    phi = 15;
}

void initDiverses()
{
    float aux;
    int32_t i;

    incx = (xb - xa) / nbrePoints;
    incy = (yb - ya) / nbreLignes;
    
    c1 = 70;
    c2 = 710;
    c3 = 57;
    c4 = 145;
    
    memset(hmax, 0, sizeof(hmax));
    
    for (i = 0; i < LIMX; i++) hmin[i] = LIMY;
    
    if (theta < 0 || theta > 180)
    {
        aux = xa; xa = xb; xb = aux; incx = -incx;
        aux = ya; ya = yb; yb = aux; incy = -incy;
    }
}

void rechercheFenetre()
{
    float x, y, z;
    int32_t l, p;

    for (l = 0; l < nbreLignes; l++)
    {
        y = yb - l * incy;
        for (p = 0; p < nbrePoints; p++)
        {
            x = xa + p * incx;
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
     if (vue == 'R') if (echx < echy) echy = echx; else echx = echy;
}

void horizon(int32_t x1, int32_t y1, int32_t x2, int32_t y2)
{
    int32_t x, y, dx;
    float pente;

    dx = Signe(x2 - x1);
    if (dx == 0)
    {
        hmax[(x2 + 1) % LIMX] = max(hmax[x2 % LIMX], y2);
        hmin[(x2 + 1) % LIMX] = min(hmin[x2 % LIMX], y2);
    }
    else
    {
        pente = (y2 - y1) / (x2 - x1);
        for(x = x2 + 1; x <= x1; x++)
        {
            y = pente * (x - x1) + y1;
            hmax[x % LIMX] = max(hmax[x % LIMX], y);
            hmin[x % LIMX] = min(hmin[x % LIMX], y);
        }
    }
}

void visibilite(int32_t x, int32_t y, int32_t *Visi)
{
    if ((y < hmax[x % LIMX]) && (y > hmin[x % LIMX])) *Visi = 0;
    else if (y >= hmax[x % LIMX]) *Visi = 1; else *Visi = -1;
}

void inter(int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t *tabaux, int32_t *xi, int32_t *yi)
{
    float den, xii, yii;
    
    if (x2 == x1)
    {
        xii = x2;
        yii = tabaux[x2 % LIMX];
    }
    else
    {
        den = y2 - y1 - tabaux[x2 % LIMX] + tabaux[x1 % LIMX];
        if (den)
        {
            xii = (x1 * (y2 - tabaux[x2 % LIMX]) + x2 * (tabaux[x1 % LIMX] - y1)) / den;
            yii = (y2 * tabaux[x1 % LIMX] - y1 * tabaux[x2 % LIMX]) / den;
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
    int32_t xi, yi;
    int32_t l, p;
    int32_t xprec, yprec, xcour, ycour;
    float x, y, z;

    for (l = 0; l < nbreLignes; l++)
    {
        y = yb - l * incy;
        x = xa;
        z = FX(x, y);
        
        projette (x, y, z);
        
        xprec = (xproj - f1) * echx + c1;
        yprec = (yproj - f3) * echy + c3;

        visibilite(xprec, yprec, &visiPrec);
        
        for (p = 0; p < nbrePoints; p++)
        {
            x = xa + p * incx;
            z = FX(x, y);
            projette(x, y, z);
            
            xcour = (xproj - f1) * echx + c1;
            ycour = (yproj - f3) * echy + c3;
            
            visibilite(xcour, ycour, &visiCour);
            
            if (!hmax[xcour % LIMX] || hmin[xcour % LIMX] == LIMY) visiCour = visiPrec;
            
            if (visiCour == visiPrec)
            {
                if (visiCour == 1 || visiCour == -1)
                {
                    drawLine(xprec, LIMY - yprec, xcour, LIMY - ycour, 50);
                    horizon(xprec, yprec, xcour, ycour);
                }
            }
            else
            {
                if (visiCour == 0)
                {
                    if (visiPrec == 1) inter(xprec, yprec, xcour, ycour, hmax, &xi, &yi);
                    else inter(xprec, yprec, xcour, ycour, hmin, &xi, &yi);
                    drawLine(xprec, LIMY - yprec, xi, LIMY - yi, 50);
                    horizon(xprec, yprec, xi, yi);
                }
                else
                {
                    if (visiCour == 1)
                    {
                        if (visiPrec == 0)
                        {
                            inter(xprec, yprec, xcour, ycour, hmax, &xi, &yi);
                            drawLine(xi, LIMY - yi, xcour, LIMY - ycour, 50);
                            horizon(xi, yi, xcour, ycour);
                        }
                        else
                        {
                            inter(xprec, yprec, xcour, ycour, hmin, &xi, &yi);
                            drawLine(xprec, LIMY - yprec, xi, LIMY - yi, 50);
                            horizon(xprec, yprec, xi, yi);
                            inter(xprec, yprec, xcour, ycour, hmax, &xi, &yi);
                            drawLine(xi, LIMY - yi, xcour, LIMY - ycour, 50);
                            horizon(xi, yi, xcour, ycour);
                        }
                    }
                    else
                    {
                        if (visiPrec == 0)
                        {
                            inter(xprec, yprec, xcour, ycour, hmin, &xi, &yi);
                            drawLine(xi, LIMY - yi, xcour, LIMY - ycour, 50);
                            horizon(xi, yi, xcour, ycour);
                        }
                        else
                        {
                            inter(xprec, yprec, xcour, ycour, hmax, &xi, &yi);
                            drawLine(xprec, LIMY - yprec, xi, LIMY - yi, 50);
                            horizon(xprec, yprec, xi, yi);
                            inter(xprec, yprec, xcour, ycour, hmin, &xi, &yi);
                            drawLine(xi, LIMY - yi, xcour, LIMY - ycour, 50);
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
    int32_t i, j;
    int32_t width, height, startx, starty;

    char buff[80] = {0};
    const char *title = "Shapes 3D Transform";
    uint32_t oldFont = getFontType();

    setFontType(1);
    width = getFontWidth(title);
    height = getFontHeight(title);
    starty = 2;
    startx = centerX - (width >> 1);
    writeString(startx, starty, title, 40, 1);

    setFontType(0);
    sprintf(buff, "X=[%.1f,%.1f] Y=[%.1f,%.1f] Theta=%.1f Phi=%.1f Lines=%d Points=%d", -xa, xa, -ya, ya, theta, phi, nbreLignes, nbrePoints);
    writeString(centerX - (getFontWidth(buff) >> 1), cmaxY - getFontHeight(buff) - 2, buff, 37, 1);

    for (i = starty; i < height; i++)
    {
        for (j = startx; j < startx + width; j++) if (getPixel(j, i) == 40) putPixel(j, i, 56 + (i / 3));
    }
    
    for (i = 50; i < cmaxX - 50; i++)
    {
        for (j = 50; j < cmaxX - 50; j++) if (getPixel(i, j) == 50) putPixel(i, j, 32 + ((i + j) / range) % 72);
    }
    
    setFontType(oldFont);
}

void resetParameter()
{
    c1 = c2 = c3 = c4 = nbreLignes = nbrePoints = 0;
    xa = xb = ya = yb = incx = incy = f1 = rho = f2 = f3 = f4 = echx = 0;
    echy = de = theta = phi = aux1 = aux2 = aux3 = aux4 = 0;
    aux5 = aux6 = aux7 = aux8 = xobs = yobs = zobs = xproj = yproj = 0;
}

int main()
{
    RGB pal[256] = {0};

    if (!setVesaMode(800, 600, 8, 0)) return 0;

    if (!loadFont("sys8x16.xfn", 0)) return 0;
    if (!loadFont("trip.xfn", 1)) return 0; 

    getPalette(pal);
    setBlackPalette();

    FX = FX1;

    initParameter1();
    initDiverses();
    initProjection();
    rechercheFenetre();
    calculeEchelles();
    dessinefonction();
    affichage(15);

    fadeIn(pal);
    rotatePalette(32, 103, 45);
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

    fadeIn(pal);
    rotatePalette(32, 103, 45);
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

    fadeIn(pal);
    rotatePalette(32, 103, 45);
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

    fadeIn(pal);
    rotatePalette(32, 103, 45);
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

    fadeIn(pal);
    rotatePalette(32, 103, 45);
    fadeMin();

    closeVesaMode();
    closeFont(0);
    closeFont(1);
    
    return 1;
}
