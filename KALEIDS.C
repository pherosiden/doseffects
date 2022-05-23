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

#include "svga.c"

#define START_COLOR     0
#define END_COLOR       255

typedef struct ScreenData {
    char data, attr;
} CharType;

typedef struct ScreenMem {
    CharType ch[25][80];
} ScreenType;

ScreenType *scr = (ScreenType*)0xB8000000L;

int X1, Y1, X2, Y2;
int XV1, YV1, XV2, YV2;
int XA, YA, XB, YB;
int HC, CX, CY, MD;

void writeAt(int row, int col, char *str)
{
    int color;
    int i, n, x, y, lst;
    
    col--;
    row--;

    while (*str)
    {
        if (row > 4 && row < 21 && col > 25 && col < 53) color = 15;
        else
        {
            x = abs(39 - col);
            y = abs(12 - row);
            n = x * x + y * y;
            lst = n >> 1;
        
            for (i = 1; i < 5; i++)
            {
                if (!lst)
                {
                    color = 0;
                    break;
                }

                color = (n / lst + lst) >> 1;
                lst = color;
            }

            color = (color % 15) + 1;
        }

        scr->ch[row][col].data = *(str++);
        scr->ch[row][col].attr = (char)color;
        
        if (++col > 79)
        {
            row++;
            col = 0;
        }
    }
}

void initialize()
{
    int col, row;
    unsigned long endTime, waitTime;
    
    writeAt( 1, 2, "KaleidsKaleidsKaleidsKaleidsKaleidsKaleidsKaleidsKaleidsKaleidsKaleidsKaleids");
    writeAt( 2, 2, "KaleidsKaleidsKaleidsKaleidsKaleidsKaleidsKaleidsKaleidsKaleidsKaleidsKaleids");
    writeAt( 3, 2, "KaleidsKaleidsKaleidsKaleidsKaleidsKaleidsKaleidsKaleidsKaleidsKaleidsKaleids");
    writeAt( 4, 2, "KaleidsKaleidsKaleidsKaleidsKaleidsKaleidsKaleidsKaleidsKaleidsKaleidsKaleids");
    writeAt( 5, 2, "KaleidsKaleidsKaleidsKal   K A L E I D O S C O P E   idsKaleidsKaleidsKaleids");
    writeAt( 6, 2, "KaleidsKaleidsKaleidsKal                             idsKaleidsKaleidsKaleids");
    writeAt( 7, 2, "KaleidsKaleidsKaleidsKal   Version 3.6 for VESA 2.0  idsKaleidsKaleidsKaleids");
    writeAt( 8, 2, "KaleidsKaleidsKaleidsKal                             idsKaleidsKaleidsKaleids");
    writeAt( 9, 2, "KaleidsKaleidsKaleidsKal     Copyright(c) 2000 by    idsKaleidsKaleidsKaleids");
    writeAt(10, 2, "KaleidsKaleidsKaleidsKal        Nguyen Ngoc Van      idsKaleidsKaleidsKaleids");
    writeAt(11, 2, "KaleidsKaleidsKaleidsKal                             idsKaleidsKaleidsKaleids");
    writeAt(12, 2, "KaleidsKaleidsKaleidsKal                             idsKaleidsKaleidsKaleids");
    writeAt(13, 2, "KaleidsKaleidsKaleidsKal                             idsKaleidsKaleidsKaleids");
    writeAt(14, 2, "KaleidsKaleidsKaleidsKal    Present NOEL 2009 to     idsKaleidsKaleidsKaleids");
    writeAt(15, 2, "KaleidsKaleidsKaleidsKal        BUI XUAN DAO         idsKaleidsKaleidsKaleids");
    writeAt(16, 2, "KaleidsKaleidsKaleidsKal                             idsKaleidsKaleidsKaleids");
    writeAt(17, 2, "KaleidsKaleidsKaleidsKal  I Love You to one more!    idsKaleidsKaleidsKaleids");
    writeAt(18, 2, "KaleidsKaleidsKaleidsKal                             idsKaleidsKaleidsKaleids");
    writeAt(19, 2, "KaleidsKaleidsKaleidsKal                             idsKaleidsKaleidsKaleids");
    writeAt(20, 2, "KaleidsKaleidsKaleidsKal      Press ESC to exit      idsKaleidsKaleidsKaleids");
    writeAt(21, 2, "KaleidsKaleidsKaleidsKal    any key to continue...   idsKaleidsKaleidsKaleids");
    writeAt(22, 2, "KaleidsKaleidsKaleidsKaleidsKaleidsKaleidsKaleidsKaleidsKaleidsKaleidsKaleids");
    writeAt(23, 2, "KaleidsKaleidsKaleidsKaleidsKaleidsKaleidsKaleidsKaleidsKaleidsKaleidsKaleids");
    writeAt(24, 2, "KaleidsKaleidsKaleidsKaleidsKaleidsKaleidsKaleidsKaleidsKaleidsKaleidsKaleids");
    writeAt(25, 2, "KaleidsKaleidsKaleidsKaleidsKaleidsKaleidsKaleidsKaleidsKaleidsKaleidsKaleids");

    endTime = getTicks() + 192;

    do {
        waitTime = getTicks() + 1;
        for (col = 0; col < 25; col++)
        {
            for(row = 1; row < 78; row++)
            {
                if (col < 5 || col > 20 || row < 26 || row > 52)
                {
                    if (!(--scr->ch[col][row].attr)) scr->ch[col][row].attr = 15;
                }
            }
        }
        while (getTicks() <= waitTime);
    } while (!kbhit() && getTicks() <= endTime);

    while(kbhit()) getch();
}

inline int random(int x)
{
    if (x == 0) return 0;
    return rand() % x;
}

void makePalette()
{
    RGB pal[256] = {0};
    rainbowPalette(pal);
    setPalette(pal, 0, 255);
}

void main()
{
    int i;
    initialize();
    if (!initGraph(640, 480, 8)) return;
    srand(time(NULL));

    CX = centerX;
    CY = centerY;
    MD = CY;
    HC = START_COLOR;// + random(END_COLOR - START_COLOR);

    makePalette();

    do
    {
        X1 = random(MD) + 1;
        X2 = random(MD) + 1;
        Y1 = random(X1);
        Y2 = random(X2);

        while ((random(130) > 5) && (inp(0x60) != 1))
        {
            XV1 = random(10) - 5;
            XV2 = random(10) - 5;
            YV1 = random(10) - 5;
            YV2 = random(10) - 5;

            while (random(100) > 20 && (inp(0x60) != 1))
            {
                XA = (X1 << 2) / 3;
                XB = (X2 << 2) / 3;
                YA = (Y1 << 2) / 3;
                YB = (Y2 << 2) / 3;

                drawLine(CX + XA, CY - Y1, CX + XB, CY - Y2, HC);
                drawLine(CX - YA, CY + X1, CX - YB, CY + X2, HC);
                drawLine(CX - XA, CY - Y1, CX - XB, CY - Y2, HC);
                drawLine(CX - YA, CY - X1, CX - YB, CY - X2, HC);
                drawLine(CX - XA, CY + Y1, CX - XB, CY + Y2, HC);
                drawLine(CX + YA, CY - X1, CX + YB, CY - X2, HC);
                drawLine(CX + XA, CY + Y1, CX + XB, CY + Y2, HC);
                drawLine(CX + YA, CY + X1, CX + YB, CY + X2, HC);

                X1 = (X1 + XV1) % MD;
                Y1 = (Y1 + YV1) % MD;
                X2 = (X2 + XV2) % MD;
                Y2 = (Y2 + YV2) % MD;

                HC++;

                if (HC > END_COLOR) HC = START_COLOR;

                waitRetrace();
            }
        }
        clearScreen(0, 0, cmaxX, cmaxY, 0);
    } while(!kbhit());
    closeGraph();
}
