/*-----------------------------------------------------*/
/* Packet  : Demo & Effect                             */ 
/* Effect  : Ray-Casting                               */
/* Author  : Nguyen Ngoc Van                           */
/* Memory  : Compact                                   */
/* Heaps   : 640K                                      */
/* Address : pherosiden@gmail.com                      */
/* Website : http://www.codedemo.net                   */
/* Created : 14/03/1998                                */
/* Please sent to me any bugs or suggests.             */
/* You can use freely this code. Have fun :)           */
/*-----------------------------------------------------*/
#include <dos.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <conio.h>

#define     CLR     111
#define     HORIZ   100
#define     WIDTH   320
#define     HEIGHT  200
#define     RAD     0.01745329

double      head = 0.0;
double      turn = 0.0;
double      step = 0.0;

double      sinx, cosx;
double      px, py, newpx, newpy;

int32_t     verts, pass, hph, hmh, horiz;
int32_t     px128, py128, sinl, cosl, magic;

uint8_t     *vmem = (uint8_t*)0xA0000000L;
uint8_t     *tmem = (uint8_t*)0xB8000000L;

uint8_t     showmaze = 1;
uint8_t     vbuff[200][320] = {0};
uint8_t     walls[128][128] = {0};
uint8_t     floors[128][128] = {0};
uint8_t     ceils[128][128] = {0};
uint8_t     shade[16][256] = {0};
uint8_t     maze[32][32] = {0};
uint8_t     kbarr[128] = {0};
uint8_t     pal[256][3] = {0};

void (interrupt *old09)();

void interrupt new09()
{
    uint8_t chr = inp(0x60);
    
    if (chr >= 128) kbarr[chr - 128] = 0;
    else kbarr[chr] = 1;

    __asm {
        mov     al, 0x20
        out     0x20, al
    }
}

void moveData(uint8_t *src, uint8_t *dst, uint16_t len)
{
    __asm {
        lds     si, src
        les     di, dst
        mov     cx, len
        shr     cx, 2
        rep     movsd
    }
}

void clearMem(uint8_t *where, uint16_t len)
{
    __asm {
        les     di, where
        xor     eax, eax
        mov     cx, len
        shr     cx, 2
        rep     stosd
    }
}

void printStr(int16_t x, int16_t y, uint8_t col, char *msg)
{
    uint16_t len = strlen(msg);

    __asm {
        mov     cx, len
        test    cx, cx
        jz      quit
        les     di, tmem
        lds     si, msg
        add     di, x
        shl     di, 1
        mov     bx, y
        shl     bx, 5
        add     di, bx
        shl     bx, 2
        add     di, bx
        mov     ah, col
    start:
        lodsb
        stosw
        loop    start
    quit:
    }
}

void drawWallFloor(uint16_t vofs, uint16_t dark)
{
    __asm {
        shl     dark, 8
        mov     ebx, verts
        mov     ax, seg vbuff
        mov     es, ax
        mov     di, vofs
        mov     eax, hmh
        mov     ecx, hph
        sub     ecx, eax
        jc      done
        shl     ax, 6
        add     di, ax
        shl     ax, 2
        add     di, ax
        mov     esi, horiz
        and     si, 7Fh
        mov     dx, si
        xor     ax, ax
    cycl0:
        push    ebx
        shr     ebx, 16
        shl     bx, 7
        mov     si, dx
        add     si, bx
        mov     al, [walls + si]
        mov     si, ax
        add     si, dark
        mov     al, [shade + si]
        mov     es:[di], al
        add     di, WIDTH
        pop     ebx
        add     ebx, pass
        loop    cycl0
        mov     eax, hph
        cmp     eax, HEIGHT
        jae     done
        mov     cx, seg vbuff
        mov     es, cx
        mov     di, vofs
        push    di
        shl     ax, 6
        add     di, ax
        shl     ax, 2
        add     di, ax
        rol     edi, 16
        pop     di
        mov     eax, hmh
        shl     ax, 6
        add     di, ax
        shl     ax, 2
        add     di, ax
        sub     di, WIDTH
        ror     edi, 16
        mov     ebx, hph
        sub     bx, HORIZ
    cycl1:
        xor     edx, edx
        mov     eax, magic
        div     ebx
        mov     ecx, eax
        mov     eax, sinl
        mul     ecx
        shr     eax, 20
        add     eax, px128
        and     ax, 7Fh
        mov     si, ax
        mov     eax, cosl
        mul     ecx
        shr     eax, 20
        add     eax, py128
        and     ax, 7Fh
        shl     ax, 7
        add     si, ax
        shr     ecx, 10
        test    cx, cx
        jnz     nzero
    nzero:
        cmp     cx, 15
        jbe     no15
        mov     cx, 15
    no15:
        shl     cx, 8
        push    si
        xor     ax, ax
        mov     al, [floors + si]
        mov     si, ax
        add     si, cx
        mov     al, [shade + si]
        mov     es:[di], al
        add     di, WIDTH
        rol     edi, 16
        pop     si
        mov     al, [ceils + si]
        mov     si, ax
        add     si, cx
        mov     al, [shade + si]
        mov     es:[di], al
        sub     di, WIDTH
        ror     edi, 16
        inc     bx
        cmp     bx, HORIZ
        jnz     cycl1
    done:
    }
}

void loadCEL(char *fname, uint8_t *where)
{
    FILE *fpt;

    fpt = fopen(fname, "rb");
    if (!fpt) return;

    fseek(fpt, 32, SEEK_SET);
    fread(pal, 1, 768, fpt);
    fread(where, 1, 128 * 128, fpt);
    fclose(fpt);

    __asm {
        mov     dx, 0x03C8
        xor     al, al
        out     dx, al
        inc     dx
        lea     si, pal
        mov     cx, 768
        rep     outsb
    }
}

void loadMaze(char *fname)
{
    FILE *fptr;
    uint16_t x;

    fptr = fopen(fname, "rb");
    if (!fptr) return;

    for (x = 0; x < 32; x++) fread(maze[x], 1, 32, fptr);
    fclose(fptr);
    px = py = 1.5;
}

void drawMaze()
{
    int16_t i, j;

    for (i = 0; i < 32; i++)
    {
        for (j = 0; j < 32; j++)
        {
            if (maze[i][j])
            {
                vbuff[j * 3 + 0][i * 3 + 0] = CLR;
                vbuff[j * 3 + 0][i * 3 + 1] = CLR;
                vbuff[j * 3 + 0][i * 3 + 2] = CLR;
                vbuff[j * 3 + 1][i * 3 + 0] = CLR;
                vbuff[j * 3 + 1][i * 3 + 1] = CLR;
                vbuff[j * 3 + 1][i * 3 + 2] = CLR;
                vbuff[j * 3 + 2][i * 3 + 0] = CLR;
                vbuff[j * 3 + 2][i * 3 + 1] = CLR;
                vbuff[j * 3 + 2][i * 3 + 2] = CLR;
            }
        }
    }
}

void setShade()
{
    int16_t i, j, k;
    int16_t r, g, b;
    int16_t diff1, diff2;

    for (i = 0; i < 256; i++) shade[0][i] = i;

    for (k = 1; k < 16; k++)
    {
        for (i = 0; i < 256; i++)
        {
            r = (pal[i][0] >> 2) * (16 - k);
            g = (pal[i][1] >> 2) * (16 - k);
            b = (pal[i][2] >> 2) * (16 - k);

            diff1 = 1000;
            for (j = 0; j < 256; j++)
            {
                diff2 = abs((pal[j][0] << 2) - r) + abs((pal[j][1] << 2) - g) + abs((pal[j][2] << 2) - b);
                if (diff2 < diff1)
                {
                    diff1 = diff2;
                    shade[k][i] = j;
                }
            }
        }
    }
}

inline int16_t range(int16_t x, int16_t y)
{
    return (x < 0 || y < 0 || x > 27 || y > 27 || maze[y][x] > 0);
}

inline int32_t roundf(double x)
{
    return (int32_t)x;
}

void computeView()
{
    uint16_t vofs;
    uint16_t dark;
    int32_t height;
    double spacer;
    double slope, x1, x2, y1, y2;
    double angle, dist1, dist2;

    px128 = px * 128.0;
    py128 = py * 128.0;
    angle = head + 32;

    for (vofs = 0; vofs < WIDTH; vofs++)
    {
        angle -= 0.2;
        magic = roundf(128.0 * 1024.0 / cos((angle - head) * RAD));
        sinl = roundf(sin(angle * RAD) * 128.0 * 1024.0);
        cosl = roundf(cos(angle * RAD) * 128.0 * 1024.0);

        if (angle != 180.0 && angle != 0.0)
        {
            if (sinl > 0)
            {
                slope = tan((90 - angle) * RAD);
                x1 = floor(px) + 1.0;
                y1 = py + (x1 - px) * slope;
                while (!range(x1, y1))
                {
                    x1++;
                    y1 += slope;
                }
            }
            else
            {
                slope = tan((angle - 270) * RAD);
                x1 = floor(px) - 0.0000001;
                y1 = py + (px - x1) * slope;
                while (!range(x1, y1))
                {
                    x1--;
                    y1 += slope;
                }
            }
        }

        if (angle != 270.0 && angle != 90.0)
        {
            if (cosl > 0)
            {
                slope = tan(angle * RAD);
                y2 = floor(py) + 1.0;
                x2 = px + (y2 - py) * slope;
                while (!range(x2, y2))
                {
                    y2++;
                    x2 += slope;
                }
            }
            else
            {
                slope = tan((angle - 180) * RAD);
                y2 = floor(py) - 0.000001;
                x2 = px - (py - y2) * slope;
                while (!range(x2, y2))
                {
                    y2--;
                    x2 -= slope;
                }
            }
        }

        dist1 = (px - x1) * (px - x1) + (py - y1) * (py - y1);
        dist2 = (px - x2) * (px - x2) + (py - y2) * (py - y2);

        if (dist1 > dist2)
        {
            spacer = sqrt(dist2) * 1024.0;
            horiz = labs(128L - 128 * x2);
        }
        else
        {
            spacer = sqrt(dist1) * 1024.0;
            horiz = labs(128L - 128 * y1);
        }

        height = roundf(magic / spacer);
        pass = (64L * 65536L) / height;
        hmh = HORIZ - height;
        hph = HORIZ + height;

        dark = roundf(spacer) >> 10;
        if (dark > 15) dark = 15;

        if (hmh < 0)
        {
            verts = -(hmh * pass);
            hmh = 0;
            hph = HEIGHT;
        }
        else verts = 0;

        drawWallFloor(vofs, dark);
    }
}

void checkEvalKeys()
{
    if (kbarr[15])
    {
        showmaze = !showmaze;
        kbarr[15] = 0;
    }

    if (kbarr[75])
    {
        if (turn < 1.0) turn = 1.0;
        else if (turn < 8.0) turn *= 2.0;
    }
    else if (kbarr[77])
    {
        if (turn > -1.0) turn = -1.0;
        else if (turn > -8.0) turn *= 2.0;
    }
    else turn *= 0.6;

    if (kbarr[80])
    {
        if (step > -0.1) step = -0.1;
        else if (step > -0.25) step *= 1.1;
    }
    else if (kbarr[72])
    {
        if (step < 0.1) step = 0.1;
        else if (step < 0.25) step *= 1.1;
    }
    else step *= 0.8;
}

void waitRetrace()
{
    __asm {
        mov     dx, 0x03DA
    waitV:
        in      al, dx
        and     al, 0x08
        jnz     waitV
    waitH:
        in      al, dx
        and     al, 0x08
        jz      waitH
    }
}

void main()
{
    clearMem(tmem, 4000);
    printStr(1, 1, 0x0F, "RAYCASTING - (c) 1998 by Nguyen Ngoc Van");
    printStr(1, 2, 0x07, "Control Keys:");
    printStr(1, 3, 0x07, "      TAB   : Show maze");
    printStr(1, 4, 0x07, "      ARROW : Move");
    printStr(1, 5, 0x07, "      ESC   : Exit");
    printStr(1, 6, 0x07, "Press any key to continue ...");
    getch();

    old09 = _dos_getvect(0x09);
    _dos_setvect(0x09, new09);

    __asm {
        mov     ax, 0x13
        int     0x10
    }

    loadMaze("assets/maze.dat");
    loadCEL("assets/wallr.cel", walls[0]);
    loadCEL("assets/floor.cel", floors[0]);
    loadCEL("assets/ceil.cel", ceils[0]);
    setShade();

    do {
        checkEvalKeys();
        head += turn;

        if (head >= 360.0) head = 0.0;
        else if (head <= 0.0) head = 360.0;

        if (step > 0.0)
        {
            sinx = sin(head * RAD);
            newpx = px + sinx * (step + 0.5);
            cosx = cos(head * RAD);
            newpy = py + cosx * (step + 0.5);
        }
        else if (step < 0.0)
        {
            sinx = sin(head * RAD);
            newpx = px + sinx * (step - 0.5);
            cosx = cos(head * RAD);
            newpy = py + cosx * (step - 0.5);
        }

        if (!maze[roundf(py + 0.1)][roundf(newpx)] && !maze[roundf(py - 0.1)][roundf(newpx)]) px += sinx * step;
        if (!maze[roundf(newpy)][roundf(px + 0.1)] && !maze[roundf(newpy)][roundf(px - 0.1)]) py += cosx * step;

        computeView();
        if (showmaze) drawMaze();
        waitRetrace();
        moveData(vbuff[0], vmem, 64000);
    } while (!kbarr[1]);

    __asm {
        mov     ax, 0x03
        int     0x10
    }

    _dos_setvect(0x09, old09);
}
