#include "svga.c"

#define MEMORY      50
#define MAX_COLOR   255
#define MAX_DELTA   16

typedef struct {
    int16_t lx1, ly1;
    int16_t lx2, ly2;
    uint8_t lcolor;
} LINE_NODE;

LINE_NODE lineNode[MEMORY];

int16_t xmax, ymax;
int16_t gx1, gx2, gy1, gy2;
int16_t dx1, dy1, dx2, dy2;
int16_t currLine, incCount;
uint8_t currColors = 0;
uint8_t changeColors = 1;

int16_t random(int16_t x)
{
    if (x == 0) return x;
    return rand() % x;
}

void init()
{
    int16_t i;

    currColors = 1;
    currLine = 0;
    incCount = 0;
    xmax = cmaxX - 2;
    ymax = cmaxY - 2;

    for (i = 0; i < MEMORY; i++)
    {
        lineNode[i].lx1 = centerX;
        lineNode[i].lx2 = centerX;
        lineNode[i].ly1 = centerY;
        lineNode[i].ly2 = centerY;
        lineNode[i].lcolor = 0;
    }

    gx1 = centerX;
    gx2 = centerX;
    gy1 = centerY;
    gy2 = centerY;
}

void adjustX(int16_t *x, int16_t *deltaX)
{
    int16_t testX;

    testX = *x + *deltaX;
    if (testX < 1 || testX > xmax)
    {
        testX = *x;
        *deltaX = -(*deltaX);
    }
    *x = testX;
}

void adjustY(int16_t *y, int16_t *deltaY)
{
    int16_t testY;

    testY = *y + *deltaY;
    if (testY < 1 || testY > ymax)
    {
        testY = *y;
        *deltaY = -(*deltaY);
    }
    *y = testY;
}

void selectNewDelta()
{
    dx1 = random(MAX_DELTA) - (MAX_DELTA >> 1);
    dx2 = random(MAX_DELTA) - (MAX_DELTA >> 1);
    dy1 = random(MAX_DELTA) - (MAX_DELTA >> 1);
    dy2 = random(MAX_DELTA) - (MAX_DELTA >> 1);
    incCount = 2 * (1 + random(4));
}

void saveCurrentLine(uint8_t currCol)
{
    lineNode[currLine].lx1 = gx1;
    lineNode[currLine].ly1 = gy1;
    lineNode[currLine].lx2 = gx2;
    lineNode[currLine].ly2 = gy2;
    lineNode[currLine].lcolor = currCol;
}

void updateLine()
{
    currLine++;
    if (currLine >= MEMORY) currLine = 0;
    currColors++;
    if (currColors >= MAX_COLOR) currColors = 1;
    incCount--;
}

void drawCurrentLine()
{
    drawLine(gx1, gy1, gx2, gy2, currColors);
    drawLine(xmax - gx1, gy1, xmax - gx2, gy2, currColors);
    drawLine(gx1, ymax - gy1, gx2, ymax - gy2, currColors);
    drawLine(xmax - gx1, ymax - gy1, xmax - gx2, ymax - gy2, currColors);
    saveCurrentLine(currColors);
}

void eraseCurrentLine()
{
    drawLine(lineNode[currLine].lx1, lineNode[currLine].ly1, lineNode[currLine].lx2, lineNode[currLine].ly2, 0);
    drawLine(xmax - lineNode[currLine].lx1, lineNode[currLine].ly1, xmax - lineNode[currLine].lx2, lineNode[currLine].ly2, 0);
    drawLine(lineNode[currLine].lx1, ymax - lineNode[currLine].ly1, lineNode[currLine].lx2, ymax - lineNode[currLine].ly2, 0);
    drawLine(xmax - lineNode[currLine].lx1, ymax - lineNode[currLine].ly1, xmax - lineNode[currLine].lx2, ymax - lineNode[currLine].ly2, 0);
}

void retrace()
{
    while (inp(0x03DA) & 8);
    while (!(inp(0x03DA) & 8));
}

void doArt()
{
    srand(time(NULL));
    
    do {
        eraseCurrentLine();

        if (incCount == 0) selectNewDelta();
        
        adjustX(&gx1, &dx1);
        adjustX(&gx2, &dx2);
        adjustY(&gy1, &dy1);
        adjustY(&gy2, &dy2);

        if (random(5) == 3)
        {
            gx1 = (gx1 + gx2) >> 1;
            gy2 = (gy1 + gy2) >> 1;
        }

        retrace();
        drawCurrentLine();
        updateLine();
    } while (!kbhit());
}

void main()
{
    initGraph(640, 480, 8);
    init();
    doArt();
    closeGraph();
}
