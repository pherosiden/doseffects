/*----------------------------------------------------*/
/* Packet  : Demo & Effect                            */
/* Effect  : Fast Circle Filled                       */
/* Author  : Nguyen Ngoc Van                          */
/* Memory  : Compact                                  */
/* Heaps   : 640K                                     */
/* Address : siden@codedemo.net                       */
/* Website : http://www.codedemo.net                  */
/* Created : 17/03/1999                               */
/* Please sent to me any bugs or suggests.            */
/* You can use freely this code. Have fun :)          */
/*----------------------------------------------------*/

#include "gfxlib.c"

uint32_t frames = 0;
uint32_t st = 0, et = 0;

void makePal(uint8_t n, uint8_t r, uint8_t g, uint8_t b)
{
    int32_t i;
    int32_t palWhite = 10;
    
    for (i = 0; i <= 63 - palWhite; i++)
    {
        setRGB(n + i, ((r * i) / (63 - palWhite)) << 2, ((g * i) / (63 - palWhite)) << 2, ((b * i) / (63 - palWhite)) << 2);
    }

    for (i = 0; i <= palWhite; i++)
    {
        setRGB(n + i + 63 - palWhite, (r + (63 - r) * i / palWhite) << 2, (g + (63 - g) * i / palWhite) << 2, (b + (63 - b) * i / palWhite) << 2);
    }
}

void ballCircle()
{
    int32_t col;
    int32_t x, y, i;
        
    makePal(0, 63, 32, 16);
    makePal(64, 32, 63, 16);
    makePal(128, 16, 16, 63);
    makePal(128 + 64, 63, 16, 16);
    
    st = GetTicks();

    while (!keyPressed(27))
    {
        x = rand() % lfbWidth;
        y = rand() % lfbHeight;
        col = (rand() % 4) * 64;
        for (i = 0; i < 64; i++) fillCircle(x + (64 - i) / 2, y + (64 - i) / 2, (64 - i) * 2, i + col);
        frames++;
    }
}

void runExit()
{
    et = GetTicks();
    fadeCircle(0, 0);
    closeVesaMode();
    printf("FPS:%.2f\n", (frames * 18.2) / (et - st));
}

int main()
{
    initGfxLib(1, runExit);
    if (!setVesaMode(800, 600, 8, 85)) return 1;
    srand(time(NULL));
    ballCircle();
    runExit();
    return 1;
}
