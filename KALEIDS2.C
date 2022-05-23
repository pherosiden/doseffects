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
    int32_t X1, Y1, X2, Y2;
    int32_t XV1, YV1, XV2, YV2;
    int32_t XA, YA, XB, YB;
    int32_t CX, CY, MD;

    uint8_t mode = 0;
    uint32_t step1, step2;

    if (!setVesaMode(800, 600, 8, 0)) return;
    srand(time(NULL));
    
    if (argc > 1 && !strcmp(argv[1], "1")) mode = 1;
    if (mode) makeRainbowPalette();

    CX = centerX;
    CY = centerY;
    MD = CY;

    do
    {
        if (!mode) makeFunkyPalette();

        step1 = 0;
        X1 = random(MD) + 1;
        X2 = random(MD) + 1;
        Y1 = random(X1);
        Y2 = random(X2);

        while (step1 < MAX_STEP1 && inp(0x60) != 1)
        {
            step2 = 0;
            XV1 = random(5) - 2;
            XV2 = random(5) - 2;
            YV1 = random(5) - 2;
            YV2 = random(5) - 2;

            while (step2 < MAX_STEP2 && inp(0x60) != 1)
            {
                XA = (X1 << 2) / 3;
                XB = (X2 << 2) / 3;
                YA = (Y1 << 2) / 3;
                YB = (Y2 << 2) / 3;

                drawLineBob(CX + XA, CY - Y1, CX + XB, CY - Y2);
                drawLineBob(CX - YA, CY + X1, CX - YB, CY + X2);
                drawLineBob(CX - XA, CY - Y1, CX - XB, CY - Y2);
                drawLineBob(CX - YA, CY - X1, CX - YB, CY - X2);
                drawLineBob(CX - XA, CY + Y1, CX - XB, CY + Y2);
                drawLineBob(CX + YA, CY - X1, CX + YB, CY - X2);
                drawLineBob(CX + XA, CY + Y1, CX + XB, CY + Y2);
                drawLineBob(CX + YA, CY + X1, CX + YB, CY + X2);

                X1 = (X1 + XV1) % MD;
                Y1 = (Y1 + YV1) % MD;
                X2 = (X2 + XV2) % MD;
                Y2 = (Y2 + YV2) % MD;

                waitRetrace();
                step2++;
            }
            step1++;
        }

        clearScreen(0);
        if (mode) scrollPalette(0, 255, 64);
    } while (!kbhit());
    closeVesaMode();
}
