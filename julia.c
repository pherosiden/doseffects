/*---------------------------------------------------*/
/* Packet  : Demo & Effect                           */
/* Effect  : Julia & Mandelbrot                      */
/* Author  : Nguyen Ngoc Van                         */
/* Memory  : Small                                   */
/* Address : pherosiden@gmail.com                    */
/* Website : http://www.codedemo.net                 */
/* Created : 17/11/2001                              */  
/* Please sent to me any bugs or suggests.           */
/* You can use freely this code. Have fun :)         */ 
/*---------------------------------------------------*/

#include "gfxlib.c"

// this is used for 256-color mode
void makePalette()
{
    int16_t a, b;
    RGB pal[256] = {0};

    pal[0].r = 0;
    pal[0].g = 0;
    pal[0].b = 0;

    for (a = 1; a <= 85; a++)
    {
        b = a * 255 / 85;
        pal[a      ].r = b;
        pal[85 + a ].g = b;
        pal[170 + a].b = b;
        pal[170 + a].r = 0;
        pal[a      ].g = 0;
        pal[85 + a ].b = 0;
        pal[171 - a].r = b;
        pal[256 - a].g = b;
        pal[86 - a ].b = b;
    }

    setPalette(pal);
}

// this is used to more 256-color mode
uint32_t HSV2RGB(uint8_t h, uint8_t s, uint8_t v)
{
    uint8_t r, g, b;
    uint8_t region, fpart, p, q, t;

    // black
    if (s == 0)
    {
        r = g = b = v;
        return fromRGB(r, g, b);
    }

    // make hue 0-5
    region = h / 43;

    // find remainder part, make it from 0-255
    fpart = (h - (region * 43)) * 6;

    // calculate temp vars, doing integer multiplication
    p = (v * (255 - s)) >> 8;
    q = (v * (255 - ((s * fpart) >> 8))) >> 8;
    t = (v * (255 - ((s * (255 - fpart)) >> 8))) >> 8;

    // assign temp vars based on color cone region
    switch (region)
    {
        case 0:
            r = v; g = t; b = p; break;
        case 1:
            r = q; g = v; b = p; break;
        case 2:
            r = p; g = v; b = t; break;
        case 3:
            r = p; g = q; b = v; break;
        case 4:
            r = t; g = p; b = v; break;
        default:
            r = v; g = p; b = q; break;
    }

    return fromRGB(r, g, b);
}

// width, height : screen size, ake larger to see more detail!
// cre, cim : real and imaginary part of the constant c, determinate shape of the Julia Set
// zoom, mx, my : you can change these to zoom and change position
// iters : after how much iterations the function should stop
// each iteration, it calculates: new = old*old + c, where c is a constant and old starts at current pixel
//
void juliaSet(int32_t width, int32_t height, int32_t iters, double cre, double cim, double zoom, double mx, double my)
{
    int32_t i, x, y;	// loop
    double newre, newim;
    double oldre, oldim;   //real and imaginary parts of new and old
    
    //remove key buffer
    while(kbhit()) getch();

    //loop through every pixel
    for (y = 0; y < height; y++)
    {
        //check for exit
        if (kbhit()) return;

        for (x = 0; x < width; x++)
        {
            //check for exit
            if (kbhit()) return;

            //calculate the initial real and imaginary part of z, based on the pixel location and zoom and position values
            newre = 1.5 * (x - width / 2) / (0.5 * zoom * width) + mx;
            newim = 1.0 * (y - height / 2) / (0.5 * zoom * height) + my;

            //i will represent the number of iterations
            //start the iteration process
            for (i = 1; i <= iters; i++)
            {
                //check for exit
                if (kbhit()) return;

                //remember value of previous iteration
                oldre = newre;
                oldim = newim;

                //the actual iteration, the real and imaginary part are calculated
                newre = oldre * oldre - oldim * oldim + cre;
                newim = 2.0 * oldre * oldim + cim;

                //if the point is outside the circle with radius 2: stop
                if ((newre * newre + newim * newim) > 4) break;
            }

            // for 256 colors
            if (bitsPerPixel == 8) putPixel(x, y, i);
            else putPixel(x, y, HSV2RGB(i & 0xFF, 255, 255 * (i < iters)));
        }
    }
}

// width, height : screen size, ake larger to see more detail!
// zoom, mx, my : you can change these to zoom and change position
// iters : after how much iterations the function should stop
// each iteration, it calculates: newz = oldz*oldz + p, where p is the current pixel, and oldz stars at the origin
//
void mandelbrotSet(int32_t width, int32_t height, int32_t iters, double zoom, double mx, double my)
{
    int32_t i, x, y;	// loop
    double pr, pi; 			//real and imaginary part of the pixel p
    double newre, newim;
    double oldre, oldim;   //real and imaginary parts of new and old z

    //remove key buffer
    while(kbhit()) getch();

    //loop through every pixel
    for (y = 0; y < height; y++)
    {
        //check for exit
        if (kbhit()) return;

        for (x = 0; x < width; x++)
        {
            //check for exit
            if (kbhit()) return;

            //calculate the initial real and imaginary part of z, based on the pixel location and zoom and position values
            pr = 1.5 * (x - width / 2) / (0.5 * zoom * width) + mx;
            pi = 1.0 * (y - height / 2) / (0.5 * zoom * height) + my;
            newre = newim = oldre = oldim = 0; //these should start at 0,0

            //i will represent the number of iterations
            //start the iteration process
            for (i = 1; i <= iters; i++)
            {
                //check for exit
                if (kbhit()) return;

                //remember value of previous iteration
                oldre = newre;
                oldim = newim;

                //the actual iteration, the real and imaginary part are calculated
                newre = oldre * oldre - oldim * oldim + pr;
                newim = 2.0 * oldre * oldim + pi;

                //if the point is outside the circle with radius 2: stop
                if ((newre * newre + newim * newim) > 4) break;
            }

            // for 256 colors
            if (bitsPerPixel == 8) putPixel(x, y, i);
            else putPixel(x, y, HSV2RGB(i & 0xFF, 255, 255 * (i < iters)));
        }
    }
}

int main()
{
    // Init video mode
    if (!setVesaMode(640, 480, 8, 0))
    {
        printf("Cannot init video mode!\n");
        return 1;
    }

    // Make palette for 256 colors
    if (bitsPerPixel == 8) makePalette();
    juliaSet(cmaxX, cmaxY, 256, -0.7, 0.27015, 1, 0, 0);
    if (bitsPerPixel == 8) rotatePalette(1, 255, 0);

    // Clear and display MandelBrot
    clearScreen(0);
    mandelbrotSet(cmaxX, cmaxY, 512, 1179.8039, -0.743153, -0.139405);
    if (bitsPerPixel == 8) rotatePalette(1, 255, 0);
    
    closeVesaMode();
    return 0;
}
