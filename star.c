/*---------------------------------------------------*/
/* Packet  : Demo & Effect                           */
/* Effect  : STAR Field                              */
/* Author  : Nguyen Ngoc Van                         */
/* Memory  : Small                                   */
/* Address : pherosiden@gmail.com                    */
/* Website : http://www.codedemo.net                 */
/* Created : 12/03/1998                              */
/* Please sent to me any bugs or suggests.           */
/* You can use freely this code. Have fun :)         */
/*---------------------------------------------------*/

#include "gfxlib.c"

#define MAXPOINT    100
#define MAXX        640
#define MAXY        480
#define DELAY       50

typedef struct {
    int16_t x, y;
    int16_t state, color;
    int16_t delta;
} STAR;

void drawPoint(STAR p, int16_t state)
{
    int16_t color = state ? p.color : 0;

    switch (p.state)
    {
        case 0:
            putPixel(p.x, p.y, color);
            break;

        case 1:
            drawLine(p.x - 1, p.y, p.x + 1, p.y, color);
            drawLine(p.x, p.y - 1, p.x, p.y + 1, color);
            break;

        case 2:
            drawLine(p.x - 2, p.y, p.x + 2, p.y, color);
            drawLine(p.x, p.y - 2, p.x, p.y + 2, color);
            break;

        case 3:
            drawLine(p.x - 4, p.y, p.x + 4, p.y, color);
            drawLine(p.x, p.y-4, p.x, p.y + 4, color);
            drawRect(p.x - 1, p.y - 1, p.x + 1, p.y + 1, color);
            break;
            
        default:
            break;
    }
}

void main()
{
    int16_t i;
    STAR p[MAXPOINT] = {0};

    if (!setVesaMode(640, 480, 8, 85)) return;

    srand(time(0));

    for (i = 0; i < MAXPOINT; i++)
    {
        p[i].x = random(MAXX);
        p[i].y = random(MAXY);
        p[i].state = random(4);
        p[i].color = 35 + random(69);
        p[i].delta = random(2);
    }

    while (!kbhit())
    {
        for (i = 0; i < MAXPOINT; i++) drawPoint(p[i], 1); delay(DELAY);
        for (i = 0; i < MAXPOINT; i++) drawPoint(p[i], 0);
        for (i = 0; i < MAXPOINT; i++)
        {
            if (p[i].delta)
            {
                p[i].state++;
                if (p[i].state >= 3) p[i].delta = 0;
            }
            else
            {
                p[i].state--;
                if (p[i].state <= 0) p[i].delta = 1;
            }
        }
    }

    closeVesaMode();
}
