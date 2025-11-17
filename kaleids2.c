/*----------------------------------------------------*/
/* Packet  : Demo & Effect                            */
/* Title   : NOEL present to XUAN DAO (Girl's friend) */
/* Author  : Nguyen Ngoc Van                          */
/* Memory  : all mode                                 */
/* Heaps   : 64K                                      */
/* Address : pherosiden@gmail.com                     */
/* Website : http://www.codedemo.net                  */
/* Created : 20/12/2000                               */
/* Please sent to me any bugs or suggests.            */
/* You can use freely this code. Have fun :)          */
/*----------------------------------------------------*/

#include "gfxlib.c"

#define MAX_STEP1   30
#define MAX_STEP2   20

void main(int argc, const char* argv[])
{
    int32_t cx, cy;
    int32_t x1, y1, x2, y2;
    int32_t vx1, vy1, vx2, vy2;
    int32_t xa, ya, xb, yb;
    
    uint8_t mode = 0;
    uint32_t step1, step2;

    if (!setVesaMode(1024, 768, 8, 0)) return;
    srand(time(NULL));
    
    if (argc > 1 && !strcmp(argv[1], "1")) mode = 1;
    if (mode) makeRainbowPalette();

    cx = centerX;
    cy = centerY;

    while (!kbhit())
    {
        if (!mode) makeFunkyPalette();

        step1 = 0;
        x1 = random(cy) + 1;
        x2 = random(cy) + 1;
        y1 = random(x1) + 1;
        y2 = random(x2) + 1;

        while (step1 < MAX_STEP1 && inp(0x60) != 1)
        {
            step2 = 0;
            vx1 = random(5) - 2;
            vx2 = random(5) - 2;
            vy1 = random(5) - 2;
            vy2 = random(5) - 2;

            while (step2 < MAX_STEP2 && inp(0x60) != 1)
            {
                xa = (x1 << 2) / 3;
                xb = (x2 << 2) / 3;
                ya = (y1 << 2) / 3;
                yb = (y2 << 2) / 3;

                drawLineBob(cx + xa, cy - y1, cx + xb, cy - y2);
                drawLineBob(cx - ya, cy + x1, cx - yb, cy + x2);
                drawLineBob(cx - xa, cy - y1, cx - xb, cy - y2);
                drawLineBob(cx - ya, cy - x1, cx - yb, cy - x2);
                drawLineBob(cx - xa, cy + y1, cx - xb, cy + y2);
                drawLineBob(cx + ya, cy - x1, cx + yb, cy - x2);
                drawLineBob(cx + xa, cy + y1, cx + xb, cy + y2);
                drawLineBob(cx + ya, cy + x1, cx + yb, cy + x2);

                x1 = (x1 + vx1) % cy;
                y1 = (y1 + vy1) % cy;
                x2 = (x2 + vx2) % cy;
                y2 = (y2 + vy2) % cy;

                waitRetrace();
                step2++;
            }
            step1++;
        }

        clearScreen(0);
        if (mode) scrollPalette(0, 255, 64);
    }
    
    closeVesaMode();
}
