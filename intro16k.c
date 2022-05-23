/*--------------------------------------------------*/
/*  Effect: 16K INTRO                               */
/*  Packet: Demo & Effect                           */
/*  Author: Nguyen Ngoc Van                         */
/*  Create: 2000-08-12                              */
/*  Memory: Compact                                 */
/*  Heaps : 640K                                    */
/*  Original code written in pascal by Cooler       */
/*  Convert to C and optimize by Nguyen Ngoc Van    */
/*  Publish 2000-10-15 on codedemo.net              */
/*  Any suggestion email to pherosiden@gmail.com    */
/*--------------------------------------------------*/

#include <dos.h>
#include <math.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <conio.h>

typedef struct {
    int16_t x, y, z;
    uint8_t color;
} T3DPoint;

typedef struct {
    int16_t v1, v2, v3;
    uint8_t color;
    int16_t z;
} TFace;

typedef struct {
    int16_t cx, cy;
    int16_t dx, dy;
    int16_t life;
} TParticle;

const TFace baseFigure[4] = {
    {0, 3, 1, 200, 0},
    {0, 1, 2, 180, 0},
    {0, 2, 3, 160, 0},
    {1, 3, 2, 140, 0}
};

const TFace logoData[18] = {
    { 1,  5,  4, 0, 0},
    { 0,  2,  3, 0, 0},
    { 3,  2,  6, 0, 0},
    { 7,  9, 10, 0, 0},
    { 9, 10, 13, 0, 0},
    { 8, 11, 12, 0, 0},
    {14, 15, 16, 0, 0},
    {17, 15, 16, 0, 0},
    {17, 16, 18, 0, 0},
    {17, 19, 18, 0, 0},
    {20, 21, 22, 0, 0},
    {21, 27, 23, 0, 0},
    {22, 24, 28, 0, 0},
    {25, 27, 29, 0, 0},
    {26, 30, 28, 0, 0},
    {37, 31, 32, 0, 0},
    {33, 34, 35, 0, 0},
    {31, 36, 32, 0, 0}
};

const T3DPoint baseVertices[4] = {
    { 80,  80,  80, 200},
    {-80, -80,  80, 200},
    {-80,  80, -80, 200},
    { 80, -80, -80, 200}
};

const T3DPoint logoVertices[38] = {
    {-68,  20, 0, 255},
    {-72,  -4, 0, 255},
    {-60, -20, 0, 255},
    {-52, -16, 0, 255},
    {-52,   0, 0, 255},
    {-48,  -4, 0, 255},
    {-20, -24, 0, 255},
    {-56,  20, 0, 255},
    {-56,   8, 0, 255},
    {-32, -16, 0, 255},
    {-20, -20, 0, 255},
    {-20,   0, 0, 255},
    {-20,   8, 0, 255},
    {-20,  20, 0, 255},
    {-16,  20, 0, 255},
    {-16, -16, 0, 255},
    { -8, -20, 0, 255},
    {  4,  20, 0, 255},
    { 12,  16, 0, 255},
    { 12, -20, 0, 255},
    { 16,   0, 0, 255},
    { 24, -20, 0, 255},
    { 24,  20, 0, 255},
    { 23, -12, 0, 255},
    { 23,  12, 0, 255},
    { 28, -18, 0, 255},
    { 28,  18, 0, 255},
    { 32, -20, 0, 255},
    { 32,  20, 0, 255},
    { 40, -16, 0, 255},
    { 40,  16, 0, 255},
    { 48,  16, 0, 255},
    { 56,  20, 0, 255},
    { 56,   8, 0, 255},
    { 44, -20, 0, 255},
    { 52, -20, 0, 255},
    { 68, -20, 0, 255},
    { 20,  28, 0, 255}
};

int16_t     clipx1 = 0;
int16_t     clipx2 = 319;
int16_t     clipy1 = 0;
int16_t     clipy2 = 199;
int16_t     deltaZ = 4096;
int16_t     frames = 0;
int16_t     beatFunc = 0;
int16_t     matrix[3][3] = {0};
int8_t      sinTab[256] = {0};

uint8_t     br1 = 0;
uint8_t     br2 = 0;
uint8_t     sat = 0;
uint16_t    xorigin = 0;
uint16_t    yorigin = 0;
uint16_t    firstPart = 765;

uint8_t     palette[256][3] = {0};
uint8_t     sqrTab[4096] = {0};
uint8_t     vbuff[200][320] = {0};
uint8_t     texture[128][128] = {0};
uint8_t     blobs[128][128] = {0};
uint16_t    tpos[100][320] = {0};

TParticle   particles[1000] = {0};
T3DPoint    vertices[200] = {0};
T3DPoint    scenes[200] = {0};
TFace       faces[200] = {0};

int16_t     *order1 = NULL;
int16_t     *order2 = NULL;
uint8_t     *wiredFont = NULL;
uint8_t     *vmem = (uint8_t*)0xA0000000L;

int16_t random(int16_t x)
{
    if (x == 0) return 0;
    return rand() % x;
}

void closeGraph()
{
    __asm {
        mov    ax, 0x03
        int    0x10
    }
}

void initGraph()
{
    __asm {
        mov    ax, 0x13
        int    0x10
    }
}

void clearBuff(uint32_t col)
{
    __asm {
        mov     ax, seg vbuff
        mov     es, ax
        xor     di, di
        mov     eax, col
        mov     cx, 16000
        rep     stosd
    }
}

void fatalError(const char *fmt, ...)
{
    va_list args;
    closeGraph();
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
    if (order1) free(order1);
    if (order2) free(order2);
    if (wiredFont) free(wiredFont);
    exit(1);
}

void doBeat(int16_t value, int16_t start, int16_t step)
{
    if (frames % step == start) beatFunc = value;
}

int32_t exSin(int16_t x)
{
    return sinTab[(x >> 2) & 255] * (4 - (x & 3)) + sinTab[((x >> 2) + 1) & 255] * (x & 3);
}

void multMatrix(int32_t *m1, int32_t *m2)
 {
    int16_t i, j, k;
    int32_t m3[9] = {0};
    memset(m3, 0, sizeof(m3));

    for (i = 0; i < 3; i++)
    {
        for (j = 0; j < 3; j++)
        {
            for (k = 0; k < 3; k++) m3[3 * i + j] += m1[3 * i + k] * m2[3 * k + j]; 
        }
    }

    memcpy(m1, m3, sizeof(m3));
    memset(m2, 0, sizeof(m3));
}

void genMatrix(int16_t a, int16_t b, int16_t c)
{
    int16_t i, j;
    int16_t asin, acos;
    int32_t m1[3][3], m2[3][3];

    memset(m1, 0, sizeof(m1));
    memset(m2, 0, sizeof(m2));

    acos = sinTab[(a + 64) & 255];
    asin = sinTab[a & 255];
    m1[0][0] = acos;
    m1[0][1] = -asin;
    m1[1][0] = asin;
    m1[1][1] = acos;
    m1[2][2] = 64;

    acos = sinTab[(b + 64) & 255];
    asin = sinTab[b & 255];
    m2[0][0] = acos;
    m2[0][2] = -asin;
    m2[2][0] = asin;
    m2[2][2] = acos;
    m2[1][1] = 64;
    multMatrix(m1[0], m2[0]);

    acos = sinTab[(c + 64) & 255];
    asin = sinTab[c & 255];
    m2[1][1] = acos;
    m2[1][2] = -asin;
    m2[2][1] = asin;
    m2[2][2] = acos;
    m2[0][0] = 64;
    multMatrix(m1[0], m2[0]);

    for (i = 0; i < 3; i++)
    {
        for (j = 0; j < 3; j++) matrix[i][j] = m1[i][j] >> 7;
    }
}

void project(int16_t cnt)
{
    int16_t i, j;
    int32_t val[3] = {0};

    for (i = 0; i < cnt; i++)
    {
        for (j = 0; j < 3; j++)
        {
            val[j] = (int32_t)vertices[i].x * matrix[j][0] + (int32_t)vertices[i].y * matrix[j][1] + (int32_t)vertices[i].z * matrix[j][2];
        }

        scenes[i].z = val[2] >> 11;
        val[2] = (val[2] >> 6) + deltaZ;
        scenes[i].x = (val[0] << 3) / (val[2] * 7);
        scenes[i].y = val[1] / val[2];
        scenes[i].color = vertices[i].color;
    }
}

void triangle(int16_t p1, int16_t p2, int16_t p3)
{
    int16_t v1, v2, v3, y1, y;
    int16_t x1, x2, dx1, dx2;
    int16_t c1, c2, dc1, dc2;
    int16_t xl, xr, cl, cr, dc;
    int16_t vc1, vc2, vc3, i;

    if (scenes[p1].y < scenes[p2].y) v1 = p1; else v1 = p2;
    if (scenes[p3].y < scenes[v1].y) v1 = p3;
    if (scenes[p1].y > scenes[p2].y) v2 = p1; else v2 = p2;
    if (scenes[p3].y > scenes[v2].y) v2 = p3;

    if (v1 != p1 && v2 != p1) v3 = p1;
    if (v1 != p2 && v2 != p2) v3 = p2;
    if (v1 != p3 && v2 != p3) v3 = p3;
    
    y1 = scenes[v1].y;
    vc1 = scenes[v1].color;
    vc2 = scenes[v2].color;
    vc3 = scenes[v3].color;
    
    if (scenes[v1].y == scenes[v3].y)
    {
        x1 = scenes[v1].x << 8;
        x2 = scenes[v3].x << 8;
        i = scenes[v2].y - y1;

        if (i == 0) return;

        dx1 = ((scenes[v2].x - scenes[v1].x) << 8) / i;
        dx2 = ((scenes[v2].x - scenes[v3].x) << 8) / i;

        c1 = vc1 << 8;
        c2 = vc3 << 8;

        dc1 = ((vc2 - vc1) << 8) / i;
        dc2 = ((vc2 - vc3) << 8) / i;
    }
    else
    {
		if (y1 >= clipy1)
		{
			vbuff[(yorigin + y1) % 200][(xorigin + scenes[v1].x) % 320] = vc1;
		}
        
        x1 = scenes[v1].x << 8;
        x2 = x1;

        dx1 = ((scenes[v2].x - scenes[v1].x) << 8) / (scenes[v2].y - y1);
        dx2 = ((scenes[v3].x - scenes[v1].x) << 8) / (scenes[v3].y - y1);
        
        c1 = vc1 << 8;
        c2 = c1;
        
        dc1 = ((vc2 - vc1) << 8) / (scenes[v2].y - y1);
        dc2 = ((vc3 - vc1) << 8) / (scenes[v3].y - y1);
        
        y1++;
        
        x1 += dx1;
        x2 += dx2;
        c1 += dc1;
        c2 += dc2;
    }

    for (y = y1; y < scenes[v2].y; y++)
    {
        if (y >= clipy1 && y <= clipy2)
        {
            if (x1 < x2)
            {
                xl = (x1 + 255) >> 8;
                xr = (x2 + 128) >> 8;
                cl = c1;
                cr = c2;
            }
            else
            {
                xl = (x2 + 255) >> 8;
                xr = (x1 + 128) >> 8;
                cl = c2;
                cr = c1;
            }
            
            if (xr > xl) dc = (cr - cl) / (xr - xl);

            if (xl < clipx1)
            {
                cl += dc * (clipx1 - xl);
                xl = clipx1;
            }

            if (xr > clipx2) xr = clipx2;

            for (i = xl; i <= xr; i++)
            {
                vbuff[(yorigin + y) % 200][(xorigin + i) % 320] = cl >> 8;
                cl += dc;
            }
        }
        
        if (y == scenes[v3].y)
        {
            dx2 = ((scenes[v2].x - scenes[v3].x) << 8) / (scenes[v2].y - scenes[v3].y);
            dc2 = ((vc2 - vc3) << 8) / (scenes[v2].y - scenes[v3].y);
        }

        x1 += dx1;
        x2 += dx2;
        c1 += dc1;
        c2 += dc2;
    }
}

void drawScene(int16_t vert, int16_t fac, int16_t x, int16_t y)
{
    int16_t *torder;
    int16_t i, j, m, c, n1, n2;
    int16_t cnt, v1x, v1y, v2x, v2y;

    xorigin = x;
    yorigin = y;
    clipx1 = -x + 2;
    clipx2 = 319 - x - 2;
    clipy1 = -y + 2;
    clipy2 = 199 - y - 2;
    
    cnt = 0;
    project(vert);

    for (i = 0; i < fac; i++)
    {
        faces[i].z = (scenes[faces[i].v1].z + scenes[faces[i].v2].z + scenes[faces[i].v3].z) / 3;
        v1x = scenes[faces[i].v1].x - scenes[faces[i].v2].x;
        v1y = scenes[faces[i].v1].y - scenes[faces[i].v2].y;
        v2x = scenes[faces[i].v1].x - scenes[faces[i].v3].x;
        v2y = scenes[faces[i].v1].y - scenes[faces[i].v3].y;
        if (v1x * v2y - v2x * v1y > 0) order1[cnt++] = i;
    }

    m = 1;
    for (i = 0; i < 8; i++)
    {
        c = 0;
        for (j = 0; j < cnt; j++)
        {
            if (!((faces[order1[j]].z ^ 0x80) & m)) c++;
        }

        n1 = 0;
        n2 = c;

        for (j = 0; j < cnt; j++)
        {
            if (!((faces[order1[j]].z ^ 0x80) & m)) order2[n1++] = order1[j];
            else order2[n2++] = order1[j];
        }

        m <<= 1;
        torder = order1;
        order1 = order2;
        order2 = torder;
    }

    for (i = cnt - 1; i >= 0; i--)
    {
        if (faces[order1[i]].color)
        {
            scenes[faces[order1[i]].v1].color = faces[order1[i]].color;
            scenes[faces[order1[i]].v2].color = faces[order1[i]].color;
            scenes[faces[order1[i]].v3].color = faces[order1[i]].color;
        }
        triangle(faces[order1[i]].v1, faces[order1[i]].v2, faces[order1[i]].v3);
    }
}

void setPalette()
{
    __asm {
        xor     ax, ax
        mov     dx, 0x03C8
        out     dx, al
        inc     dx
        mov     cx, firstPart
        mov     si, offset palette
    lp1:
        mov     al, [si]
        mov     ah, 127
        sub     ah, al
        mov     al, sat
        mul     ah
        shl     ax, 2
        lodsb
        add     al, ah
        mul     br1
        mov     al, ah
        cmp     al, 0x3F
        jbe     lp3
        mov     al, 0x3F
    lp3:
        out     dx, al
        loop    lp1
        mov     cx, 768
        sub     cx, firstPart
    lp2:
        mov     al, [si]
        mov     ah, 127
        sub     ah, al
        mov     al, sat
        mul     ah
        shl     ax, 2
        lodsb
        add     al, ah
        mul     br2
        mov     al, ah
        cmp     al, 0x3F
        jbe     lp4
        mov     al, 0x3F
    lp4:
        out     dx, al
        loop    lp2
    }
}

void waitRetrace()
{
    __asm {
        mov    dx, 0x03DA
    lph:
        in     al, dx
        test   al, 0x08
        jz     lph
    lpv:
        in     al, dx
        test   al, 0x08
        jnz    lpv
    }
}

void printChar(uint8_t *where, char chr, uint16_t ofs, uint8_t col, uint8_t col2)
{
    __asm {
        les     di, where
        add     di, ofs
        lds     si, wiredFont
        xor     ax, ax
        mov     al, chr
        shl     ax, 7
        add     si, ax
        mov     dx, 32
        mov     bl, col
        mov     bh, col2
    lp1:
        push    dx
        lodsw
        mov     dx, ax
        lodsw
        mov     cx, 16
    lp2:
        rcr     ax, 1
        jnc     lp3
        mov     es:[di], bh
    lp3:
        rcr     dx, 1
        jnc     lp4
        mov     es:[di], bl
    lp4:
        inc     di
        loop    lp2
        add     di, 304
        pop     dx
        dec     dx
        jnz     lp1
    }
}

void flipToScreen()
{
    __asm {
        mov     ax, 7
        mul     beatFunc
        shr     ax, 3
        mov     beatFunc, ax
        les     di, vmem
        mov     ax, seg vbuff
        mov     ds, ax
        xor     si, si
        mov     cx, 16000
        rep     movsd
    }
}

void tunnelBlur()
{
    __asm {
        mov     ax, seg vbuff
        mov     es, ax
        mov     di, 320
        mov     cx, 63360
        xor     bx, bx
    lp:
        xor     ax, ax
        mov     bl, es:[di - 1]
        add     ax, bx
        mov     bl, es:[di + 1]
        add     ax, bx
        mov     bl, es:[di - 320]
        add     ax, bx
        mov     bl, es:[di + 320]
        add     ax, bx
        shr     ax, 2
        stosb
        loop    lp
    }
}

void fireBlur()
{
    __asm {
        mov     ax, seg vbuff
        mov     es, ax
        mov     cx, 63360
        xor     bx, bx
        xor     di, di
    lp1:
        xor     ax, ax
        mov     bl, es:[di]
        add     ax, bx
        mov     bl, es:[di + 319]
        add     ax, bx
        mov     bl, es:[di + 320]
        add     ax, bx
        mov     bl, es:[di + 321]
        add     ax, bx
        shr     ax, 2
        and     al, al
        jz      lp2
        dec     al
    lp2:
        stosb
        loop    lp1
    }
}

void blur16K(uint16_t len)
{
    __asm {
        mov     ax, seg vbuff
        mov     ds, ax
        xor     si, si
        mov     di, len
        dec     di
        mov     bx, len
        shl     bx, 1
        dec     bx
        mov     cx, 16384
        xor     dx, dx
    next:
        xor     ax, ax
        mov     dl, [si]
        add     si, di
        add     ax, dx
        and     si, 3FFFh
        mov     dl, [si]
        add     si, 2
        add     ax, dx
        and     si, 3FFFh
        mov     dl, [si]
        add     si, di
        add     ax, dx
        and     si, 3FFFh
        mov     dl, [si]
        sub     si, bx
        add     ax, dx
        and     si, 3FFFh
        shr     ax, 2
        mov     [si], al
        loop    next
    }
}

void printString(uint8_t *where, const char *str, int16_t x, int16_t y, uint8_t color, uint8_t color2, int8_t fl)
{
    uint16_t ofs, addofs;

    ofs = (y << 8) + (y << 6) + x;
    while (*str)
    {
        if (fl) addofs = random(3) + 320 * random(3); else addofs = 0;
        printChar(where, *str, ofs + addofs, color, color2);
        ofs += 18;
        str++;
    }
}

int16_t range(int16_t b, int16_t min, int16_t max)
{
    if (b > max) return max;
    if (b < min) return min;
    return b;
}

int16_t pike(int16_t x, int16_t arg, int16_t a, int16_t b, int16_t c)
{
    if (x < arg) return a + (b - a) * x / arg;
    return b + (c - b) * (x - arg) / (256 - arg);
}

void initSinTab()
{
    int16_t val = 0;
    const int16_t scale = 64, diver = 128;

    __asm {
        mov     cx, 256
        lea     si, sinTab
        fninit
        fldpi
        fidiv   diver
        fldz
    lp:
        fld     ST(0)
        fsin
        fimul   scale
        fistp   val
        mov     ax, val
        mov     [si], al
        inc     si
        fadd    ST, ST(1)
        loop    lp
    }
}

void getFont()
{
    __asm {
        push    bp
        mov     ax, 0x1130
        mov     bh, 6
        int	    0x10
        mov	    ax, bp
        pop	    bp
        mov     si, ax
        mov     sp, 128
        lds     di, wiredFont
    nchar:
        xor     dx, dx
        xor     bx, bx
        mov     ch, 16
    nline:
        mov     al, es:[si]
        inc     si
        mov     cl, 8
    expand:
        shr     bx, 2
        rcl     al, 1
        jnc     next
        or      bx, 0xC000
    next:
        dec     cl
        jnz     expand
        shl     bx, 1
        mov     ax, bx
        xor     ax, dx
        mov     [di], ax
        mov     ax, bx
        and     ax, dx
        mov     [di + 2], ax
        mov     dx, bx
        shl     dx, 1
        mov     ax, bx
        xor     ax, dx
        mov     [di + 4], ax
        mov     ax, bx
        and     ax, dx
        mov     [di + 6], ax
        add     di, 8
        dec     ch
        jnz     nline
        dec     sp
        jnz     nchar
    }
}

void initialize()
{
    int16_t i, j, x, y, c, row, col;
    const int16_t dirtab[][2] = {{0, 1}, {1, 1}, {1, 0}, {1, -1}, {0, -1}, {-1, -1}, {-1, 0}, {-1, 1}};

    memset(vbuff[0], 0, 64000);
    memset(tpos[0], 0, 64000);
    memset(texture[0], 0, 16384);
    memset(blobs[0], 0, 16384);
    memset(particles, 0, sizeof(particles));
    memset(vertices, 0, sizeof(vertices));
    memset(scenes, 0, sizeof(scenes));
    memset(faces, 0, sizeof(faces));

    order1 = (int16_t*)calloc(1024, sizeof(int16_t));
    order2 = (int16_t*)calloc(1024, sizeof(int16_t));
    if (!order1 || !order2) fatalError("initialize: Cannot allocate orders memory!\n");

    wiredFont = (uint8_t*)calloc(128, 16 * 8);
    if (!wiredFont) fatalError("initialize: Cannot allocate wiredFont memory!\n");
    
    getFont();
    initSinTab();
    srand(time(NULL));

    j = 0;
    for (i = 0; i < 4096; i++)
    {
        if (j * j < i) j++;
        sqrTab[i] = j;
    }

    for (i = 0; i < 128; i++)
    {
        for (j = 0; j < 128; j++)
        {
            blobs[i][j] = range(32 - sqrTab[range((64 - i) * (64 - i) + (64 - j) * (64 - j), 0, 4095)], 0, 32);
        }
    }

    for (i = 1; i <= 25; i++)
    {
        c = random(8);
        x = random(256);
        y = random(256);

        for (j = 1; j <= 2000; j++)
        {
            col = (x >> 1) & 127;
            row = (y >> 1) & 127;
            texture[row][col] += 4;
            x += dirtab[c][0];
            y += dirtab[c][1];
            if (!(j & 1)) c = (c + random(3) - 1) & 7;
        }
    }

    initGraph();
}

void makeTunnel()
{
    __asm {
        mov     ax, seg tpos
        mov     es, ax
        mov     ax, seg vbuff
        mov     ds, ax
        xor     si, si
        xor     di, di
        mov     cx, 32000
    next:
        mov     al, [si]
        shr     al, 1
        mov     bx, es:[di]
        mov     ah, [si + bx]
        shr     ah, 1
        add     al, ah
        mov   	[si], al
        not     si
        not     bx
        mov     al, [64000 + si]
        shr     al, 1       
        mov     ah, [64000 + si + bx]
        shr     ah, 1
        add     al, ah
        mov     [64000 + si], al
        not     si
        inc     si
        add     di, 2
        loop    next
    }
}

void part1()
{
    int16_t i, j;
    int16_t x, y, lx, ly;

    for (i = 0; i < 256; i++)
    {
        palette[i][0] = pike(i, 128, 0, 64, 64);
        palette[i][1] = pike(i, 128, 0, 127, 64);
        palette[i][2] = pike(i, 192, 0, 127, 32);
    }

    for (i = 0; i < 100; i++)
    {
        for (j = 0; j < 320; j++)
        {
            lx = 160 - j;
            ly = 100 - i;
            x = (lx >> 4) + random(3) + (ly >> 5) - 1;
            y = (ly >> 4) + random(3) - (lx >> 5) - 1;
            tpos[i][j] = (y << 8) + (y << 6) + x;
        }
    }

    palette[255][0] = 120;
    palette[255][1] = 120;
    palette[255][2] = 0;

    frames = 0;

    do {
        doBeat(30, 1, 123);
        doBeat(30, 1, 127);

        if (frames <= 120 && br1 < 110) br1 += 10;
        if (frames > 460 && br1 > 0) br1 -= 5;

        br2 = br1;
        sat = beatFunc >> 3;
        br1 += beatFunc;
        setPalette();
        br1 -= beatFunc;

        for (i = 0; i < 100; i++)
        {
            x = rand() % 319;
            y = rand() % 199;
            vbuff[y][x] = 240;
            vbuff[y][x + 1] = 240;
            vbuff[y + 1][x] = 240;
            vbuff[y + 1][x + 1] = 240;
        }
        
        makeTunnel();
        tunnelBlur();
        waitRetrace();
        flipToScreen();
        
        switch (frames / 120)
        {
        case 0:
            printString(vmem, "THE PSYCHO TEAM", 24, 84, 255, 180, 0);
            break;
        case 1:
            printString(vmem, "presents", 84, 84, 255, 180, 0);
            break;
        case 2:
            printString(vmem, "specially for", 40, 32, 250, 180, 0);
            printString(vmem, "MILLENNIUM", 32, 84, 255, 180, 0);
            printString(vmem, "DEMOPARTY", 120, 120, 255, 180, 0);
            break;
        default:
            printString(vmem, "16K INTRO", 80, 84, 255, 180, 0);
            break;
        }
        frames++;
    } while (!kbhit() && frames < 480);
}

void makeTexture()
{
    uint16_t x, y, i, j;

    __asm {
        xor     di, di
        mov     ax, seg vbuff
        mov     es, ax
        mov     dx, 200
    lp1:
        mov     cx, 320
    lp2:
        mov     bh, dl
        mov     bl, cl
        shl     bl, 1
        shr     bx, 1
        and     bx, 3FFFh
        mov     al, [texture + bx]
        shl     al, 1
        stosb
        loop    lp2
        dec     dx
        jnz     lp1
    }
}

void partTitle()
{
    uint8_t c;
    int16_t col, row, i, j, dz;

    for (i = 0; i < 256; i++)
    {
        palette[i][0] = range(i - 160, 0, 127);
        palette[i][1] = range(i, 0, 127);
        palette[i][2] = range(i - 48, 0, 127);
    }
    
    memcpy(faces, logoData, sizeof(logoData));
    memcpy(vertices, logoVertices, sizeof(logoVertices));

    dz = 155;
    deltaZ = 13500;
    frames = 0;

    while (kbhit()) getch();
    do {
        br1 = range(frames * 2, 0, 120);
        sat = range((frames - 140) * 3, 0, 60);
        setPalette();
        makeTexture();    
        genMatrix(range(frames * 2 - 20, 0, 250), 0, -10);
        deltaZ -= dz;
        if (dz > 0) dz--;
        project(38);
        for (i = 0; i < 38; i++) vertices[i].color = ((scenes[i].y * 3) >> 1) + 160;
        drawScene(38, 18, 160, 100);
        waitRetrace();
        flipToScreen();
        frames++;
    } while (!kbhit() && frames <= 160);

    for (i = 0; i < 256; i++)
    {
        palette[i][0] = range(i - 128, 0, 100);
        palette[i][1] = pike(i, 64, 0, 100, 0);
        palette[i][2] = range(i - 48, 0, 127);
    }

    clearBuff(0);
    drawScene(38, 18, 160, 100);
    
    for (i = 0; i < 4; i++) tunnelBlur();
    drawScene(38, 18, 160, 100);
    
    tunnelBlur();
    tunnelBlur();

    for (i = 0; i < 200; i++)
    {
        for (j = 0; j < 320; j++)
        {
            col = j & 127;
            row = i & 127;
            c = vbuff[i][j];
            if (c < 12) vbuff[i][j] = texture[row][col];
            else if (c < 24) vbuff[i][j] = 60 - (c - 16) * (c - 16);
            else if (c > 64) vbuff[i][j] = 180 - (sinTab[(j * 3 + i * 2) & 255] - sinTab[(i * 5) & 255] - sinTab[(i * 4 - j * 7) & 255]) / 3;
            else vbuff[i][j] = 0;
        }
    }

    flipToScreen();
    frames = 0;

    do {
        doBeat(24, 1, 60);
        br1 = 120 - range(frames - 160, 0, 120);
        sat = range(60 - frames * 3, 0, 60) + beatFunc;
        setPalette();
        waitRetrace();
        flipToScreen();
        frames++;
    } while (!kbhit() && frames < 280);
}

void makeBlob(uint16_t x, uint16_t y)
{
    __asm {
        mov     ax, seg vbuff
        mov     es, ax
        mov     ax, y
        shl     ax, 6
        mov     di, ax
        shl     ax, 2
        add     di, ax
        add     di, x
        lea     si, blobs
        mov     dx, 128
    lp1:
        mov     cx, 128
        push    di
    lp2:   
        lodsb
        shl     al, 1
        add     al, es:[di]
        jnc     lp3
        mov     al, 255
    lp3: 
        stosb
        loop    lp2
        pop     di
        add     di, 320
        dec     dx
        jnz     lp1
    }
}

void makeFloor(uint16_t ofs, uint8_t dt, int32_t x, int32_t y, int32_t px, int32_t py)
{
    __asm {
        mov     ax, seg vbuff
        mov     es, ax
        mov     ds, ax
        mov     di, 31680
        mov     si, 32000
        mov     ax, ofs
        mov     cx, 320
        mul     cx
        sub     di, ax
        add     si, ax
        mov     ebx, x
        mov     edx, y
    lp1:
        mov     eax, edx
        xor     eax, ebx
        shr     eax, 2
        shr     ah, 2
        sub     ah, dt
        jb      lp2
    lp3:
        mov     [di], ah
        mov     [si], ah
        add     ebx, px
        add     edx, py
        inc     di
        inc     si
        loop    lp1
        jmp     lp4
    lp2:
        xor     ah, ah
        jmp     lp3
    lp4:
    }
}

void part6()
{
    uint8_t v;
    int16_t i, cu, cv, cx, cy;
    int32_t x1, y1, x2, y2, px, py, x, y;

    for (i = 0; i < 256; i++)
    {
        palette[i][0] = range(i - 64, 0, 100);
        palette[i][1] = range(i, 0, 100);
        palette[i][2] = range(i - 32, 0, 120);
    }

    br1 = 0;
    frames = 0;

    while (kbhit()) getch();
    do {
        doBeat(16, 1, 60);
        
        if (frames <= 140 && br1 < 150) br1 += 5;
        if (frames > 800 && br1 > 0) br1 -= 5;
        
        br2 = br1;
        sat = beatFunc;
        setPalette();

        for (i = 0; i < 100; i++)
        {
            x1 = (exSin(frames + 256) << 16) / (i + 2);
            y1 = (exSin(frames) << 16) / (i + 2);
            x2 = y1;
            y2 = -x1;
            px = (x2 - x1) / 320;
            py = (y2 - y1) / 320;
            x = x1;
            y = y1;

            if (i < 48) v = ((48 - i) * (48 - i)) >> 4;
            else v = 0;

            makeFloor(i, v, x, y, px, py);
        }
        
        for (i = 1; i <= 9; i++)
        {
            cx = 96 + (sinTab[(frames + i * 24) & 255] >> 1) + (sinTab[((i << 4) + frames / 3) & 255] >> 1);
            cy = 32 + (sinTab[((frames << 1) + 100 + i * 20) & 255] >> 2);
            makeBlob(cx, cy);
        }
        
        waitRetrace();
        flipToScreen();
        frames++;
    } while (!kbhit() && frames < 840);
}

void makeCopper(uint16_t x, uint16_t y)
{
    __asm {
        mov     ax, seg vbuff
        mov     ds, ax
        mov     ax, y
        shl     ax, 6
        mov     si, ax
        shl     ax, 2
        add     si, ax
        mov     ax, x
        shr     al, 3
        add     al, 20
        mov     cx, 320
    next: 
        add     [si], al
        inc     si
        loop    next
    }	
}

void makeSprings()
{
    int16_t i = 0, j = 0, w = 0, k = 0;

    do {
        if (0 <= frames && frames <= 480)
        {
            vertices[0].x = sinTab[(i << 3) & 255] >> 1;
            vertices[0].y = sinTab[((i << 3) + 64) & 255] >> 1;
            vertices[0].z = (i >> 1) - 80;
        }
        else
        {
            j = 100 + sinTab[(i * 6 + (i << 1)) & 255] / 3;
            vertices[0].x = j * sinTab[i & 255] >> 7;
            vertices[0].y = j * sinTab[(i * 3 + 64) & 255] >> 7;
            vertices[0].z = j - 100;
        }

        project(1);
        scenes[1] = scenes[0];
        i++;

        if (0 <= frames && frames <= 480)
        {
            vertices[0].x = sinTab[(i << 3) & 255] >> 1;
            vertices[0].y = sinTab[((i << 3) + 64) & 255] >> 1;
            vertices[0].z = (i >> 1) - 80;
        }
        else
        {
            j = 100 + sinTab[(i * 6 + i * 2) & 255] / 3;
            vertices[0].x = j * sinTab[i & 255] >> 7;
            vertices[0].y = j * sinTab[(i * 3 + 64) & 255] >> 7;
            vertices[0].z = j - 100;
        }

        project(1);

        for (j = 0; j < 8; j++)
        {
            w = 100 + ((scenes[0].y * j + scenes[1].y * (8 - j)) >> 3);
            k = 160 + ((scenes[0].x * j + scenes[1].x * (8 - j)) >> 3);
            vbuff[w][k] = 140 - scenes[0].z;
        }
    } while (i <= 320);
}

void makeRecbar(uint16_t ofs, uint16_t pos)
{
    __asm {
        mov     ax, seg vbuff
        mov     ds, ax
        mov     ax, ofs
        shl     ax, 6
        mov     si, ax
        shl     ax, 2
        add     si, ax
        mov     cx, pos
    next:
        mov     al, [si]
        shr     al, 1
        add     al, 96
        mov     [si], al
        not     si
        mov     al, [64000 + si]
        shr     al, 1
        add     al, 96
        mov     [64000 + si], al
        not     si
        inc     si
        loop    next
    }
}

void part5()
{
    int16_t i, j;
    int16_t k, dpos, tpos, ddz;
    int16_t pos[16] = {0}, wid[16] = {0}, spd[16] = {0};
    
    for (i = 0; i < 256; i++)
    {
        palette[i][0] = range(i - 128, 0, 100);
        palette[i][1] = range(i - 32, 0, 120);
        palette[i][2] = range(i + 32, 0, 120);
    }

    for (i = 0; i < 16; i++)
    {
        pos[i] = random(1000);
        wid[i] = 8 + random(32);
        spd[i] = 1 + random(3);
        if (i > 7) spd[i] = -spd[i];
     }

    br1 = 0;
    frames = 0;
    ddz = 1024;
    deltaZ = 21000;

    while (kbhit()) getch();
    do {
        doBeat(800, 1, 60);

        if (frames <= 120 && br1 < 120) br1 += 5;
        if (900 < frames && br1 > 0) br1 -= 5;

        br2 = br1;
        sat = beatFunc / 48;

        clearBuff(0);
        setPalette();

        for (i = 0; i < 16; i++)
        {
            k = pos[i] >> 2;
            dpos = wid[i];
            for (j = k; j <= k + dpos; j++)
            {
                if (j >= 0 && j <= 199) makeCopper(i, j);
            }
            pos[i] = (pos[i] + spd[i]) & 1023;
        }

        if ((0 <= frames && frames <= 120 || frames >= 484) && ddz > 0)
        {
            deltaZ -= ddz;
            ddz -= 32;
        }
      
        if ((460 < frames && frames < 480) && ddz < 1024)
        {
            ddz += 32;
            deltaZ += ddz;
        }

        if (frames > 900 && ddz < 1024)
        {
            deltaZ -= ddz;
            ddz -= 32;
        }

        deltaZ -= beatFunc;
        genMatrix(frames, frames >> 1, (frames * 3) >> 1);
        makeSprings();
        deltaZ += beatFunc;

        tpos = range((frames << 1) - 64, 0, 200) - range((frames << 1) - 1600, 0, 200);
        if (tpos > 0)
        {
            for (i = 0; i < 32; i++)
            {
                dpos = tpos - ((i * i) >> 4);
                if (dpos > 0) makeRecbar(i, dpos);
            }
        }
        
        if (154 < frames && frames < 460)
        {
            printString(vbuff[0], "CODE:", 32, 2, 0, 224, 0);
            printString(vbuff[0], "COOLER", 192, 172, 0, 224, 0);
        }

        if (486 < frames && frames < 800)
        {
            printString(vbuff[0], "MUSIC:", 24, 2, 0, 224, 0);
            printString(vbuff[0], "DJ MoHaX", 170, 172, 0, 224, 0);
        }

        tunnelBlur();
        waitRetrace();
        flipToScreen();
        frames++;
      } while (!kbhit() && frames < 920);
}

void genWheel(int16_t r, int16_t n)
{
    int16_t i, j;

    vertices[0].x = 0;
    vertices[0].y = 0;
    vertices[0].z = 0;

    for (i = 1; i <= n + 1; i++)
    {
        j = (i << 8) / n;
        vertices[i].x = sinTab[(j + 64) & 255] * r / 80;
        vertices[i].y = sinTab[j & 255] * r / 80;
        vertices[i].z = 0;

        j = j - 2 + ((i & 1) << 2);
        vertices[i + n + 1].x = sinTab[(j + 64) & 255] * r >> 6;
        vertices[i + n + 1].y = sinTab[j & 255] * r >> 6;
        vertices[i + n + 1].z = 0;
     }

    for (i = 0; i < n; i++)
    {
        faces[i].v1 = 0;
        faces[i].v3 = i + 2;
        faces[i].v2 = i + 1;
        faces[i].color = 16;

        if (!(i & 1))
        {
            faces[i + n].v1 = n + i + 3;
            faces[i + n].v3 = n + i + 2;
            faces[i + n].v2 = i + 1;
            faces[i + n].color = 16;

            faces[i + n + 1].v1 = n + i + 3;
            faces[i + n + 1].v3 = i + 1;
            faces[i + n + 1].v2 = i + 2;
            faces[i + n + 1].color = 16;
        }
    }
}

void part4()
{
    int16_t i;

    for (i = 0; i < 256; i++)
    {
        palette[i][0] = range(120 - i, 24, 120);
        palette[i][1] = range(100 - i, 32, 120);
        palette[i][2] = range(i - 128, 0, 120);
    }

    frames = 0;
    sat = 0;
    br1 = br2 = 0;
    
    while (kbhit()) getch();
    do {
        if (frames <= 120 && br1 < 120) br1 += 5;
        if (900 < frames && br1 > 0) br1 -= 3;

        setPalette();
        clearBuff(0);

        deltaZ = 4096;
        genMatrix((frames << 2) / 3, 0, 0);
        genWheel(100, 24);
        drawScene(50, 48, 100, 120);

        genMatrix(frames + 10, 0, 0);
        genWheel(120, 32);
        drawScene(66, 64, 224, 150);

        genMatrix((-frames << 1) + 5, 0, 0);
        genWheel(64, 16);
        drawScene(34, 32, 170, 80);

        genMatrix((-frames << 3) / 3 + 5, 0, 0);
        genWheel(45, 12);
        drawScene(26, 24, 40, 160);

        deltaZ = 7200;
        genMatrix(frames / 3, frames, frames >> 1);

        memcpy(faces, baseFigure, sizeof(baseFigure));
        memcpy(vertices, baseVertices, sizeof(baseVertices));
        
        for (i = 0; i < 4; i++)
        {
            vertices[i].color = 160 + i * 30;
            if (frames < 450) faces[i].color = 175 + i * 20;
            else faces[i].color = 0;
        }

        drawScene(4, 4, 160, 100 + (128 - ((3 * frames / 5) & 255) - sinTab[(3 * frames / 5) & 255]));

        waitRetrace();
        flipToScreen();
        frames++;
    } while(!kbhit() && frames < 950);
}

int16_t func(int16_t a, int16_t b)
{
    int16_t p;
    if (frames < 256) p = 48; else p = 176 + (frames >> 1);
    if (a < 4 && b < 4) return 64 + sinTab[p & 255] / 3;
    return 64 + sinTab[(p + 64) & 255] / 3;
}

void makePlasma(int16_t j, int16_t v, int16_t u)
{
    __asm {
        mov     ax, seg vbuff
        mov     es, ax
        xor     di, di
        mov     si, j
        mov     bx, 200
    lp1:
        mov     cx, 320
        push    si
    lp2:
        and     si, 0xFF
        mov     al, [sinTab + si]
        add     al, 80
        shr     al, 3
        add     si, v
        add     es:[di], al
        inc     di
        loop    lp2
        pop     si
        add     si, u
        dec     bx
        jnz     lp1
    }
}

void part4a()
{
    const int8_t a[6] = {1, 1, 5, 11, 6, -2};
    const int8_t b[6] = {-1, 3, 2, 7, 13, -8};
    const int8_t c[6] = {3, 0, -1, 2, 3, 0};

    int32_t koef, pos;
    int16_t mask[200] = {0};
    int16_t i, j, x, y, u, v, vcnt, fcnt;

    for (i = 0; i < 256; i++)
    {
        palette[i][0] = pike(i, 128, 0, 60, 100);
        palette[i][1] = pike(i, 96, 90, 0, 110);
        palette[i][2] = pike(i, 48, 40, 60, 0);
     }

    firstPart = 384;
    memset(palette, 0, 3);

    frames = 0;
    br1 = br2 = 0;

    while (kbhit()) getch();
    do {
        doBeat(20, 1, 70);

        if (frames <= 120 && br1 < 120)
        {
            br1 += 2;
            br2 += 2;
        }

        if (frames > 480 && br1 > 0)
        {
            br1 -= 2;
            br2 -= 2;
        }

        br1 += beatFunc;
        setPalette();
        br1 -= beatFunc;

        memset(mask, 0, sizeof(mask));

        for (i = 0; i < 16; i++)
        {
            j = (i << 6) - (frames & 255);
            if (j >= 0 && j <= 255)
            {
                koef = (exSin(j) << 12) / exSin(j + 256);
                pos = koef << 2;
                y = 199;
                u = i + 1;
                
                do {
                    x = pos >> 12;
                    if (x > 319) x = 319;
                    if (x < mask[y]) break;
                    memset(&vbuff[y][mask[y]], (i << 5) & 63, x - mask[y] + 1);
                    mask[y] = x;
                    pos += koef;
                    y--;
                } while (y);
            }
        }

        for (i = 0; i < 200; i++)
        {
            if (mask[i] < 320) memset(&vbuff[i][mask[i]], (u << 5) & 63, 320 - mask[i]);
        }
   
        for (i = 0; i < 5; i++)
        {
            j = frames * c[i];
            u = a[i];
            v = b[i];
            makePlasma(j, v, u);
        }

        deltaZ = 0x1800;
        genMatrix(frames / 3, frames, frames >> 1);

        memcpy(faces, baseFigure, sizeof(baseFigure));
        memcpy(vertices, baseVertices, sizeof(baseVertices));

        for (i = 0; i < 4; i++) vertices[i].color = 255;

        vcnt = 4;
        fcnt = 4;

        for (i = 0; i < 4; i++)
        {
            vertices[vcnt].x = (vertices[faces[i].v1].x + vertices[faces[i].v2].x) * func(faces[i].v1, faces[i].v2) >> 7;
            vertices[vcnt].y = (vertices[faces[i].v1].y + vertices[faces[i].v2].y) * func(faces[i].v1, faces[i].v2) >> 7;
            vertices[vcnt].z = (vertices[faces[i].v1].z + vertices[faces[i].v2].z) * func(faces[i].v1, faces[i].v2) >> 7;

            vertices[vcnt + 1].x = (vertices[faces[i].v1].x + vertices[faces[i].v3].x) * func(faces[i].v1, faces[i].v3) >> 7;
            vertices[vcnt + 1].y = (vertices[faces[i].v1].y + vertices[faces[i].v3].y) * func(faces[i].v1, faces[i].v3) >> 7;
            vertices[vcnt + 1].z = (vertices[faces[i].v1].z + vertices[faces[i].v3].z) * func(faces[i].v1, faces[i].v3) >> 7;
            
            vertices[vcnt + 2].x = (vertices[faces[i].v3].x + vertices[faces[i].v2].x) * func(faces[i].v3, faces[i].v2) >> 7;
            vertices[vcnt + 2].y = (vertices[faces[i].v3].y + vertices[faces[i].v2].y) * func(faces[i].v3, faces[i].v2) >> 7;
            vertices[vcnt + 2].z = (vertices[faces[i].v3].z + vertices[faces[i].v2].z) * func(faces[i].v3, faces[i].v2) >> 7;

            faces[fcnt].v1 = faces[i].v1;
            faces[fcnt].v2 = vcnt;
            faces[fcnt].v3 = vcnt + 1;
            faces[fcnt].color = 0;

            faces[fcnt + 1].v1 = faces[i].v2;
            faces[fcnt + 1].v2 = vcnt + 2;
            faces[fcnt + 1].v3 = vcnt;
            faces[fcnt + 1].color = 0;

            faces[fcnt + 2].v1 = faces[i].v3;
            faces[fcnt + 2].v2 = vcnt + 1;
            faces[fcnt + 2].v3 = vcnt + 2;
            faces[fcnt + 2].color = 0;

            faces[i].v1 = vcnt;
            faces[i].v2 = vcnt + 2;
            faces[i].v3 = vcnt + 1;
            faces[i].color = 0;

            fcnt += 3;
            vcnt += 3;
        }

        for (i = 0; i < 16; i++)
        {
            vertices[vcnt].x = (vertices[faces[i].v1].x + vertices[faces[i].v2].x) * func(faces[i].v1, faces[i].v2) >> 7;
            vertices[vcnt].y = (vertices[faces[i].v1].y + vertices[faces[i].v2].y) * func(faces[i].v1, faces[i].v2) >> 7;
            vertices[vcnt].z = (vertices[faces[i].v1].z + vertices[faces[i].v2].z) * func(faces[i].v1, faces[i].v2) >> 7;

            vertices[vcnt + 1].x = (vertices[faces[i].v1].x + vertices[faces[i].v3].x) * func(faces[i].v1, faces[i].v3) >> 7;
            vertices[vcnt + 1].y = (vertices[faces[i].v1].y + vertices[faces[i].v3].y) * func(faces[i].v1, faces[i].v3) >> 7;
            vertices[vcnt + 1].z = (vertices[faces[i].v1].z + vertices[faces[i].v3].z) * func(faces[i].v1, faces[i].v3) >> 7;
            
            vertices[vcnt + 2].x = (vertices[faces[i].v3].x + vertices[faces[i].v2].x) * func(faces[i].v3, faces[i].v2) >> 7;
            vertices[vcnt + 2].y = (vertices[faces[i].v3].y + vertices[faces[i].v2].y) * func(faces[i].v3, faces[i].v2) >> 7;
            vertices[vcnt + 2].z = (vertices[faces[i].v3].z + vertices[faces[i].v2].z) * func(faces[i].v3, faces[i].v2) >> 7;

            faces[fcnt].v1 = faces[i].v1;
            faces[fcnt].v2 = vcnt;
            faces[fcnt].v3 = vcnt + 1;
            faces[fcnt].color = 0;

            faces[fcnt + 1].v1 = faces[i].v2;
            faces[fcnt + 1].v2 = vcnt + 2;
            faces[fcnt + 1].v3 = vcnt;
            faces[fcnt + 1].color = 0;

            faces[fcnt + 2].v1 = faces[i].v3;
            faces[fcnt + 2].v2 = vcnt + 1;
            faces[fcnt + 2].v3 = vcnt + 2;
            faces[fcnt + 2].color = 0;

            faces[i].v1 = vcnt;
            faces[i].v2 = vcnt + 2;
            faces[i].v3 = vcnt + 1;
            faces[i].color = 0;

            fcnt += 3;
            vcnt += 3;
        }
        
        for (i = 0; i < vcnt; i++) vertices[i].color = 191 - sinTab[vertices[i].z & 255];
        drawScene(vcnt, fcnt, 160, 100);
        if (frames & 32) printString(vbuff[0], "VOTE!", 224, 172, 192 + beatFunc, 240 + beatFunc, 0);

        waitRetrace();
        flipToScreen();
        frames++;
    } while (!kbhit() && frames < 550);

    firstPart = 765;
}

void makeGreets(int16_t acos, int16_t asin)
{
    __asm {
        mov     ax, seg vbuff
        mov     es, ax
        xor     di, di
        xor     dx, dx
        xor     bx, bx
        mov     ch, 100
        mov     si, 319
    lp1:
        push    dx
        push    bx
        mov     cl, 160
    lp2:
        mov     al, bh
        xor     al, dh
        shr     al, 2
        mov     es:[di], al
        mov     es:[si], al
        add     al, 40h
        not     di
        not     si
        mov     es:[64000 + di], al
        mov     es:[64000 + si], al
        not     di
        not     si
        add     di, 2
        sub     si, 2
        add     bx, acos
        add     dx, asin
        dec     cl
        jnz     lp2
        pop     bx
        pop     dx
        add     bx, asin
        sub     dx, acos
        add     di, 320
        add     si, 960
        dec     ch
        jnz     lp1
    }
}

void part10()
{
    int16_t i, asin, acos;

    for (i = 0; i < 256; i++)
    {
        palette[i][0] = (i >> 1) & 127;
        palette[i][1] = (i & 127);
        palette[i][2] = (i << 1) & 127;
    }

    frames = 0;
    br1 = sat = 0;

    while (kbhit()) getch();
    do {
        if (frames <= 80 && br1 < 120) br1 += 5;
        if (frames > 120 && br1 > 0) br1 -= 5;
        
        asin = exSin(frames);
        acos = exSin(frames + 256);

        setPalette();
        makeGreets(acos, asin);
        printString(vbuff[0], "And now", 96, 70, 255, 254, 0);
        printString(vbuff[0], "some greetz...", 32, 100, 255, 254, 0);
        waitRetrace();
        flipToScreen();
        frames++;
    } while (!kbhit() && frames < 140);
}

void makeIntro0(int16_t acos, int16_t asin)
{
    __asm {
        mov     ax, seg vbuff
        mov     es, ax
        xor     di, di
        mov     eax, 0FFFFFFFFh
        mov     cx, 16000
        rep     stosd
        mov     cx, 200
        xor     dx, dx
        xor     bx, bx
        xor     di, di
        mov     si, 319
    lp1:
        push    dx
        push    bx
        mov     ax, 320
    lp2:
        push    ax
        mov     al, bh
        xor     al, dh
        and     es:[di], al
        and     es:[si], al
        not     di
        not     si
        and     es:[64000 + di], al
        and     es:[64000 + si], al
        not     di
        not     si
        inc     di
        dec     si
        add     bx, acos
        add     dx, asin
        pop     ax
        dec     ax
        jnz     lp2
        pop     bx
        pop     dx
        add     bx, asin
        sub     dx, acos
        add     si, 640
        loop    lp1
    }
}

void makeIntro1(int16_t acos, int16_t asin)
{
    __asm {
        mov     ax, seg vbuff
        mov     es, ax
        xor     di, di
        xor     eax, eax
        mov     cx, 16000
        rep     stosd
        mov     cx, 200
        xor     dx, dx
        xor     bx, bx
        xor     di, di
        mov     si, 319
    lp1:
        push    dx
        push    bx
        mov     ax, 320
    lp2:
        push    ax
        mov     al, bh
        xor     al, dh
        shr     al, 1
        xor     es:[di], al
        xor     es:[si], al
        not     di
        not     si
        xor     es:[64000 + di], al
        xor     es:[64000 + si], al
        not     di
        not     si
        inc     di
        dec     si
        add     bx, acos
        add     dx, asin
        pop     ax
        dec     ax
        jnz     lp2
        pop     bx
        pop     dx
        add     bx, asin
        sub     dx, acos
        add     si, 640
        loop    lp1
    }
}

void makeIntro2(int16_t acos, int16_t asin)
{
    __asm {
        mov     ax, seg vbuff
        mov     es, ax
        xor     di, di
        mov     cx, 16000
        xor     eax, eax
        rep     stosd
        mov     cx, 200
        xor     dx, dx
        xor     bx, bx
        xor     di, di
        mov     si, 319
    lp1:
        push    dx
        push    bx
        mov     ax, 320
    lp2:
        push    ax
        mov     al, bh
        xor     al, dh
        shr     al, 3
        add     es:[di], al
        add     es:[si], al
        not     di
        not     si
        add     es:[64000 + di], al
        add     es:[64000 + si], al
        not     di
        not     si
        inc     di
        dec     si
        add     bx, acos
        add     dx, asin
        pop     ax
        dec     ax
        jnz     lp2
        pop     bx
        pop     dx
        add     bx, asin
        sub     dx, acos
        add     si, 640
        loop    lp1
    }
}

void part11()
{
    int16_t i, j, k, m, asin, acos;

    const char *greets[] = {
        "Accept Corp.",
        "ANTARES",
        "FENOMEN",
        "FREEART",
        "FishBone",
        "Infinite",
        "MonSteR",
        "SkyProject",
        "Perforation",
        "PHG",
        "PROXiUM",
        "T-Rex"
    };

    for (i = 0; i < 256; i++)
    {
        palette[i][0] = range(i - 128, 0, 127);
        palette[i][1] = pike(i, 160, 20, 120, 120);
        palette[i][2] = pike(i, 64, 0, 100, 60);
    }

    br1 = 0;
    frames = 0;
    
    while (kbhit()) getch();
    do {
        doBeat(30, 0, 60);

        if (frames <= 120 && br1 < 120) br1 += 5;
        if (frames > 1420 && br1 > 0) br1 -= 5;

        br2 = br1;
        sat = beatFunc;
        setPalette();

        m = frames / 480;
        switch (m)
        {
        case 0:
            asin = exSin(frames);
            acos = exSin(frames + 256);
            makeIntro0(acos, asin);
            break;
        case 1:
            asin = exSin(frames);
            acos = exSin(frames + 256);
            makeIntro1(acos, asin);
            break;
        case 2:
            asin = exSin(frames);
            acos = exSin(frames + 256);
            makeIntro2(acos, asin);
            break;
        }

        if (frames < 240 + 480 * m)
        {
            k = (frames / 60) % 4;
            for (i = 0; i <= k; i++)
            {
                j = 160 - strlen(greets[i + (m << 2)]) * 9;
                printString(vbuff[0], greets[i + (m << 2)], j, i * 40 + 24, 96, 255 - ((k - i) << 5), 1);
            }
        }
        else
        {
            k = (frames / 60) - (m << 3) - 4;
            for (i = k + 1; i <= 3; i++)
            {
                j = 160 - strlen(greets[i + (m << 2)]) * 9;
                printString(vbuff[0], greets[i + (m << 2)], j, i * 40 + 24, 96, 255 - ((i - k - 1) << 5), 1);
            }
        }

        waitRetrace();
        flipToScreen();
        frames++;
    } while (!kbhit() && frames < 1440);
}

void part2()
{
    int16_t x, y;
    int16_t i, cnt;

    for (i = 0; i < 256; i++)
    {
        palette[i][0] = range(i, 0, 127);
        palette[i][1] = range(i - 48, 0, 120);
        palette[i][2] = pike(i, 32, 0, 32, 0) + pike(i, 128, 0, 0, 200);
    }

    memcpy(faces, baseFigure, sizeof(baseFigure));
    memcpy(vertices, baseVertices, sizeof(baseVertices));

    clearBuff(0);
    flipToScreen();

    br1 = br2 = 0;
    sat = cnt = 0;
    frames = -800;

    while (kbhit()) getch();

    do {
        if (frames <= -680 && br1 < 120) br1 += 4;
        if (frames > 600 && sat < 60) sat += 1;

        setPalette();

        if (frames < 0)
        {
            x = 160 + sinTab[(frames / 3) & 255];
            y = 80 - (sinTab[(frames + 60) & 255] >> 1);

            for (i = 0; i < 4; i++)
            {
                particles[cnt].cx   = (x << 5) + random(100);
                particles[cnt].cy   = (y << 5) + random(100);
                particles[cnt].dx   = -16383 + random(32767);
                particles[cnt].dy   = -16383 + random(32767);
                particles[cnt].life = 10 + random(400);
                cnt++;
            }
        }
        else
        {
            deltaZ = 0x3A00;
            genMatrix(frames, frames << 1, frames << 2);
            drawScene(4, 4, 320 - (frames >> 1) + sinTab[(frames + 40) & 255], 100 + sinTab[frames & 255]);
        }

        i = 0;
        do {
            particles[i].life--;
            if (particles[i].life <= 0)
            {
                particles[i] = particles[cnt - 1];
                cnt--;
            }
            else
            {
                x = particles[i].cx >> 5;
                y = particles[i].cy >> 5;
                vbuff[y][x] = 64 + (particles[i].life >> 1);
                particles[i].cx += (particles[i].dx >> 8);
                particles[i].cy += (particles[i].dy >> 8);
                particles[i].dy += 256;
                particles[i].dx -= (particles[i].dx >> 6);
                particles[i].dy -= (particles[i].dy >> 6);
                if (particles[i].cx < 200 || particles[i].cx > 10100) particles[i].dx = -particles[i].dx;
                if (particles[i].cy > 6200) particles[i].dy = -particles[i].dy;
            }
            i++;
        } while (i < cnt);

        fireBlur();
        waitRetrace();
        flipToScreen();
        frames++;
    } while (!kbhit() && frames < 660);
}

void finalPart()
{
    int16_t i;

    for (i = 0; i < 256; i++)
    {
        palette[i][0] = range(i - 160, 0, 127);
        palette[i][1] = range(i, 0, 127);
        palette[i][2] = range(i - 48, 0, 127);
    }

    makeTexture();
    printString(vbuff[0], "WATCOM C/C++", 64, 64, 192, 96, 0);
    printString(vbuff[0], "16K INTRO!", 128, 96, 192, 96, 0);
    printString(vbuff[0], "(The End)", 80, 160, 192, 64, 0);

    frames = br2 = 0;
    while (kbhit()) getch();

    do {
        sat = range(60 - (frames << 1), 0, 60);
        br1 = range(400 - frames, 0, 100);
        setPalette();
        waitRetrace();
        flipToScreen();
        frames++;
    } while (!kbhit() && br1);
}

void main()
{
    initialize();
    part1();
    partTitle();
    part6();
    part5();
    part4();
    part4a();
    part10();
    part11();
    part2();
    finalPart();
    closeGraph();
    free(order1);
    free(order2);
    free(wiredFont);
}
