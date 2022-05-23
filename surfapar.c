/*---------------------------------------------------*/
/* Packet  : Demo & Effect                           */
/* Title   : 3D transform                            */
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

float u, udebut, ufin, du;
float v, vdebut, vfin, dv;
float chv, shv, thv;

float (*FX)(float, float);
float (*FY)(float, float);
float (*FZ)(float, float);

float FX1(float u, float v)
{
    return (6 * cos(u)) * cos(v);
}

float FY1(float u, float v)
{
  return (3 * cos(u)) * sin(v);
}

float FZ1(float u, float v)
{
    v = 0;
    return 2 * sin(u) + v;
}

float FX2(float u, float v)
{
    return (6 + 3 * cos(u)) * cos(v);
}

float FY2(float u, float v)
{
  return (6 + 3 * cos(u)) * sin(v);
}

float FZ2(float u, float v)
{
    v = 0;
    return 3 * sin(u) + v;
}

float FX3(float u, float v)
{
    return (3 + 3 * cos(u)) * cos(v);
}

float FY3(float u, float v)
{
  return (3 + 3 * cos(u)) * sin(v);
}

float FZ3(float u, float v)
{
    v = 0;
    return 3 * sin(u) + v;
}

float FX4(float u, float v)
{
    v = 0;
    return u + v;
}

float FY4(float u, float v)
{
    u = 0;
    return v + u;
}

float FZ4(float u, float v)
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
            v +=  dv;
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

void initParameter2()
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

void initParameter3()
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

void initParameter4()
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
    sprintf(buff, "X=[%.1f,%.1f] Y=[%.1f,%.1f] Theta=%.1f Phi=%.1f U=%.1f V=%.1f", udebut, ufin, vdebut, vfin, theta, phi, du, dv);
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

int main()
{
    RGB Pal[256] = {0};

    if (!setVesaMode(800, 600, 8, 85)) return 0;

    if (!loadFont("sys8x16.xfn", 0)) return 0;
    if (!loadFont("trip.xfn", 1)) return 0; 

    getPalette(Pal);
    setBlackPalette();
    
    FX = FX1;
    FY = FY1;
    FZ = FZ1;

    initParameter1();
    initProjection();
    familleDesCourbesEnU();
    familleDesCourbesEnV();
    affichage(14);

    fadeIn(Pal);
    rotatePalette(32, 103, 72);
    saveScreen("screen06.bmp");
    fadeMin();

    FX = FX2;
    FY = FY2;
    FZ = FZ2;
    
    clearScreen(0);
    initParameter2();
    initProjection();
    familleDesCourbesEnU();
    familleDesCourbesEnV();
    affichage(14);
    
    fadeIn(Pal);
    rotatePalette(32, 103, 72);
    saveScreen("screen07.bmp");
    fadeMin();

    FX = FX3;
    FY = FY3;
    FZ = FZ3;

    clearScreen(0);
    initParameter3();
    initProjection();
    familleDesCourbesEnU();
    familleDesCourbesEnV();
    affichage(14);
    
    fadeIn(Pal);
    rotatePalette(32, 103, 72);
    saveScreen("screen08.bmp");
    fadeMin();

    FX = FX4;
    FY = FY4;
    FZ = FZ4;

    clearScreen(0);
    initParameter4();
    initProjection();
    familleDesCourbesEnU();
    familleDesCourbesEnV();
    affichage(14);
    
    fadeIn(Pal);
    rotatePalette(32, 103, 72);
    saveScreen("screen09.bmp");
    fadeMin();

    closeVesaMode();
    closeFont(0);
    closeFont(1);
    return 1;
}
