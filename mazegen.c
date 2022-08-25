/*-----------------------------------------*/
/* MAZE GENERATING v1.0 by Nguyen Ngoc Van */
/* Press any key to random generation maze */
/*-----------------------------------------*/
#include <dos.h>
#include <mem.h>
#include <time.h>
#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <conio.h>

#define X1 0
#define Y1 0
#define X2 31
#define Y2 31
#define CL 15

typedef struct {
    uint8_t x, y;
} TStack;

TStack st[2000] = {0};

uint8_t maze[32][32] = {0};
uint8_t *vmem = (uint8_t*)0xA0000000L;

int16_t random(int16_t x)
{
    if (x == 0) return 0;
    return rand() % x;
}

void clearMaze()
{
    memset(maze[0], 0, sizeof(maze));
}

void writeBorder()
{
    int16_t z;

    for (z = Y1; z <= Y2; z++)
    {
        maze[z][X1] = 1;
        maze[z][X2] = 1;
    }

    for (z = X1; z <= X2; z++)
    {
        maze[Y1][z] = 1;
        maze[Y2][z] = 1;
    }
}

uint8_t getPix(uint8_t xc, uint8_t yc, int16_t xn, int16_t yn, int16_t xp, int16_t yp)
{
    int16_t x, y;
    uint8_t a = 0;

    for (y = yc + yn; y <= yc + yp; y++)
    {
        for (x = xc + xn; x <= xc + xp; x++)
        {
            if (x >= X1 && x <= X2 && y >= Y1 && y <= Y2)
            {
                if (maze[y][x]) a++;
            }
            else
            {
                a++;
            }
        }
    }

    return a;
}

void generateMaze(uint8_t x, uint8_t y, uint8_t w, uint8_t wiggle)
{
    uint16_t sp = 0;
    uint8_t r = 0;
    uint8_t d[4] = {0};

    maze[y][x] = 1;
    st[0].x = x;
    st[0].y = y;

    do {
        memset(d, 0, sizeof(d));

        do {
            if (random(100) > wiggle) r = random(4) + 1;

            if (r > 0 && d[r - 1]) r = 0;
            else
            {
                switch (r)
                {
                case 1:
                    if (getPix(x, y - 1, -w, -w , w, 0) > 0)
                    {
                        r = 0;
                        d[0] = 1;
                    }
                    break;

                case 2:
                    if (getPix(x, y + 1, -w, 0, w, w) > 0)
                    {
                        r = 0;
                        d[1] = 1;
                    }
                    break;

                case 3:
                    if (getPix(x - 1, y, -w, -w, 0, w) > 0)
                    {
                        r = 0;
                        d[2] = 1;
                    }
                    break;

                case 4:
                    if (getPix(x + 1, y, 0, -w, w, w) > 0)
                    {
                        r = 0;
                        d[3] = 1;
                    }
                    break;
                }
            }
        } while(!r && !(d[0] && d[1] && d[2] && d[3]));

        if (!r)
        {
            x = st[sp].x;
            y = st[sp].y;
            if (sp > 0) sp--;
        }
        else
        {
            switch(r)
            {
            case 1: y--; break;
            case 2: y++; break;
            case 3: x--; break;
            case 4: x++; break;
            }

            maze[y][x] = 1;
            st[sp].x = x;
            st[sp].y = y;
            if (sp < 2000) sp++;
        }
    } while(sp);
}

void drawMaze()
{
    int16_t i, j;
    uint16_t ofs;

    for (i = 0; i < 32; i++)
    {
        for (j = 0; j < 32; j++)
        {
            if (maze[i][j])
            {
                ofs = (i * 3 + 112) + (j * 3 + 52) * 320;
                vmem[ofs + 0] = CL;
                vmem[ofs + 1] = CL;
                vmem[ofs + 2] = CL;

                ofs += 320;
                vmem[ofs + 0] = CL;
                vmem[ofs + 1] = CL;
                vmem[ofs + 2] = CL;

                ofs += 320;
                vmem[ofs + 0] = CL;
                vmem[ofs + 1] = CL;
                vmem[ofs + 2] = CL;
            }
        }
    }
}

void main()
{
    FILE *fptr;
    int16_t i;
    char key;

    srand(time(NULL));

    __asm {
        mov     ax, 0x13
        int     0x10
    }

    do {
        clearMaze();
        memset(vmem, 0, 64000);
        generateMaze(X2 >> 1, Y2 >> 1, 2, 16);
        writeBorder();
        drawMaze();
    } while (getch() != 27);

    __asm {
        mov     ax, 0x03
        int     0x10
    }

    printf("\033[2J\033[1;1H");
    printf("- Write the maze to MAZE.DAT (Y/N)? ");
    key = getch();

    if (key == 'Y' || key == 'y')
    {
        fptr = fopen("assets/maze.dat", "wb");
        if (fptr)
        {
            fwrite(maze[0], 1, sizeof(maze), fptr);
            fclose(fptr);
        }

        printf("\nDone! Any key to exit ...");
        getch();
    }    
}
