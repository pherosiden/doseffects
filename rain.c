/*----------------------------------------------------*/
/* Packet  : Demo & Effect                            */
/* Effect  : Rain (water effect)                      */
/* Author  : Nguyen Ngoc Van                          */
/* Memory  : Compact                                  */
/* Address : pherosiden@gmail.com                     */
/* Website : http://www.codedemo.net                  */
/* Created : 15/07/1998                               */
/* Please sent to me any bugs or suggests.            */
/* You can use freely this code. Have fun :)          */
/*----------------------------------------------------*/

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <conio.h>
#include <stdio.h>
#include <i86.h>

#define SEMILLA_INIT    0x9234
#define ANCHO_OLAS      2
#define DENSITY_INIT    5
#define DENSITY_SET     6
#define BORDE           2
#define ANCHO           160
#define ALTO            100
#define MAXIMO          150
#define ANCHO_PUNTOS    320

const uint8_t palette[] = {
    0x3F,0x3F,0x3F,0x3C,0x3F,0x3F,0x3A,0x3F,0x3F,0x37,0x3E,0x3F,0x34,0x3E,0x3F,0x32,
    0x3E,0x3F,0x2F,0x3E,0x3F,0x2D,0x3D,0x3F,0x2A,0x3D,0x3F,0x29,0x3C,0x3F,0x28,0x3B,
    0x3F,0x27,0x3B,0x3F,0x27,0x3B,0x3F,0x26,0x3A,0x3F,0x25,0x39,0x3F,0x24,0x38,0x3F,
    0x23,0x37,0x3F,0x22,0x37,0x3F,0x21,0x36,0x3F,0x20,0x35,0x3F,0x1F,0x34,0x3F,0x1E,
    0x33,0x3F,0x1D,0x32,0x3F,0x1C,0x31,0x3F,0x1B,0x30,0x3F,0x1A,0x2F,0x3F,0x19,0x2E,
    0x3F,0x18,0x2D,0x3F,0x17,0x2C,0x3F,0x16,0x2A,0x3F,0x15,0x29,0x3F,0x14,0x28,0x3F,
    0x13,0x27,0x3F,0x12,0x25,0x3E,0x11,0x25,0x3D,0x11,0x23,0x3D,0x10,0x22,0x3C,0x0F,
    0x20,0x3B,0x0F,0x20,0x3A,0x0E,0x1E,0x39,0x0D,0x1D,0x39,0x0C,0x1B,0x38,0x0C,0x1B,
    0x37,0x0B,0x19,0x36,0x0B,0x18,0x35,0x0A,0x16,0x35,0x09,0x16,0x34,0x09,0x14,0x33,
    0x08,0x13,0x32,0x07,0x12,0x31,0x07,0x11,0x31,0x06,0x10,0x30,0x06,0x0F,0x2F,0x05,
    0x0E,0x2F,0x05,0x0D,0x2E,0x04,0x0C,0x2D,0x04,0x0B,0x2C,0x03,0x0A,0x2B,0x03,0x09,
    0x2B,0x02,0x08,0x2A,0x02,0x07,0x29,0x02,0x06,0x28,0x01,0x06,0x27,0x01,0x04,0x27,
    0x01,0x04,0x26,0x01,0x04,0x26,0x01,0x04,0x26,0x01,0x04,0x25,0x01,0x04,0x25,0x01,
    0x04,0x25,0x01,0x04,0x24,0x00,0x04,0x24,0x00,0x04,0x24,0x00,0x04,0x23,0x00,0x03,
    0x23,0x00,0x03,0x22,0x00,0x03,0x22,0x00,0x03,0x22,0x00,0x03,0x21,0x00,0x03,0x21,
    0x00,0x03,0x21,0x00,0x03,0x20,0x00,0x03,0x20,0x00,0x03,0x20,0x00,0x03,0x20,0x00,
    0x03,0x1F,0x00,0x03,0x1F,0x00,0x03,0x1F,0x00,0x03,0x1E,0x00,0x03,0x1E,0x00,0x02,
    0x1E,0x00,0x02,0x1D,0x00,0x02,0x1D,0x00,0x02,0x19,0x00,0x02,0x16,0x00,0x01,0x12,
    0x00,0x01,0x0F,0x00,0x00,0x0A,0x00,0x00,0x05,0x00,0x00,0x00,0x0E,0x00,0x00,0x13,
    0x00,0x00,0x18,0x00,0x00,0x1D,0x00,0x00,0x22,0x00,0x00,0x27,0x00,0x00,0x2C,0x00,
    0x00,0x31,0x00,0x00,0x34,0x00,0x00,0x38,0x00,0x00,0x37,0x00,0x00,0x36,0x00,0x02,
    0x35,0x00,0x04,0x35,0x00,0x05,0x34,0x00,0x08,0x33,0x00,0x09,0x33,0x00,0x0A,0x32,
    0x00,0x0B,0x31,0x00,0x0C,0x30,0x00,0x0D,0x30,0x01,0x0E,0x2F,0x00,0x0F,0x2E,0x00,
    0x10,0x2E,0x00,0x10,0x2D,0x00,0x12,0x2C,0x01,0x13,0x2C,0x01,0x14,0x2B,0x01,0x14,
    0x2A,0x01,0x15,0x29,0x01,0x16,0x29,0x01,0x16,0x28,0x01,0x17,0x29,0x03,0x19,0x2A,
    0x05,0x1B,0x2C,0x08,0x1D,0x2D,0x0B,0x1F,0x2E,0x0D,0x20,0x2F,0x10,0x22,0x31,0x13,
    0x25,0x32,0x17,0x27,0x33,0x1A,0x29,0x34,0x1E,0x2B,0x36,0x21,0x2D,0x37,0x25,0x30,
    0x38,0x29,0x32,0x3A,0x2D,0x34,0x3B,0x31,0x37,0x3C,0x36,0x3A,0x3E,0x3A,0x3C,0x3F,
    0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F
};

const uint16_t lkpFil[] = {
    25,25,26,27,28,3,32,34,37,39,42,45,
    48,52,55,58,61,63,66,68,7,72,73,74,
    75,75,75,74,73,72,7,68,66,63,61,58,
    55,52,48,45,42,39,37,34,32,3,28,27,
    26,25,0xFF
};

const uint16_t lkpCol[] = {
     15, 15, 16, 16, 17, 18, 20, 21, 23, 25, 27, 30,
     33, 36, 39, 42, 45, 49, 52, 56, 60, 64, 68, 72,
     76, 80, 84, 88, 92, 96,  0,104,108,111,115,118,
    121,124,127,130,133,135,137,139,140,142,143,144,
    144,145,145,145,144,144,143,142,140,139,137,135,
    133,130,127,124,121,118,115,111,108,104,100, 96,
     92, 88, 84, 80, 76, 72, 68, 64, 60, 56, 52, 49,
     45, 42, 39, 36, 33, 30, 27, 25, 23, 21, 20, 18,
     17, 16, 16, 15, 0xFF
};

uint8_t     vbuff[64000] = {0};
uint8_t     *vmem = (uint8_t*)0xA0000000L;

uint8_t     densityAdd = 0;

uint16_t    randVal = 0;
uint16_t    actualPage = 0;
uint16_t    otherPage = ALTO * ANCHO * 2;

uint16_t    tblFil[5] = {0};
uint16_t    tblCol[5] = {0};

void init13h()
{
    __asm {
        mov     ax, 0x13
        int     0x10
    }
}

void textMode()
{
    __asm {
        mov     ax, 0x03
        int     0x10
    }
}

void printStr(int16_t x, int16_t y, uint8_t col, char *msg)
{
    uint16_t len = strlen(msg);

    __asm {
        mov     cx, len
        test    cx, cx
        jz      quit
        mov     ax, 0xB800
        mov     es, ax
        lds     si, msg
        mov     di, x
        shl     di, 1
        mov     ax, y
        shl     ax, 5
        add     di, ax
        shl     ax, 2
        add     di, ax
        mov     ah, col
    next:
        lodsb
        stosw
        loop    next
    quit:
    }
}

void clearScreen()
{
    __asm {
        mov     ax, 0xB800
        mov     es, ax
        xor     di, di
        xor     eax, eax
        mov     cx, 1000
        rep     stosd
    }
}

void waitRetrace()
{
    __asm {
        mov     dx, 0x03DA
    waitv:
        in      al, dx
        and     al, 0x08
        jnz     waitv
    waith:
        in      al, dx
        and     al, 0x08
        jz      waith
    }
}

void zeroPal()
{
    __asm {
        mov     dx, 0x03C8
        xor     al, al
        out     dx, al
        inc     dx
        xor     al, al
        mov     cx, 768
    next:
        out     dx, al
        loop    next
    }
}

void setPalette()
{
    __asm {
        mov     ax, seg palette
        mov     ds, ax
        mov     si, offset palette
        mov     dx, 0x03C8
        xor     al, al
        out     dx, al
        inc     dx
        mov     cx, 450
        rep     outsb
    }
}

void theEnd()
{
    zeroPal();
    textMode();
    exit(1);
}

void putFrame()
{
    /*========= OPTIMIZE VERSION =========*/
    __asm {
        mov     ax, seg vbuff
        mov     ds, ax
        mov     si, offset vbuff
        mov     ax, 0xA000
        mov     es, ax
        mov     di, 64000 - 320
        mov     cx, 160
    lp4:
        push    di
        push    cx
        xor     ax, ax
        xor     bx, bx
        xor     dx, dx
    lp3:
        xor     cx, cx
        mov     cx, [si]
        sar     cx, 7
        add     cx, ax
        sub     cx, dx
        jle     lp1
        add     dx, cx
    lp2:
        mov     es:[di], bx
        sub     di, 320
        loop    lp2
    lp1:
        add     si, 2
        add     bx, 0x101
        inc     ax
        cmp     ax, 100
        jnz     lp3
        mov     dh, dl
    lp5:
        sub     di, 320
        mov     es:[di], dx
        add     dx, 0x101
        cmp     dx, MAXIMO * 0x101;
        jnz     lp5
        pop     cx
        pop     di
        add     di, 2
        loop    lp4
    }

    /*========= C VERSION ==========
    int16_t val;
    uint16_t cx, ax, dx, bx;
    uint16_t *si = (uint16_t*)vbuff;
    uint16_t *di = (uint16_t*)&vmem[64000 - 320];
    uint16_t *ofs = NULL;

    for (cx = 0; cx < ANCHO; cx++)
    {
        bx = 0;
        dx = 0;
        ofs = di;

        for (ax = 0; ax < ALTO; ax++)
        {
            val = *si;
            val = (val >> 7) + ax - dx;
            if (val > 0)
            {
                dx += val;
                while (val--)
                {
                    *ofs = bx;
                    ofs -= ANCHO;
                } 
            }
            si++;
            bx += 0x101;
        }
        
        dx = (dx << 8) | (dx & 0x00FF);
        while (dx < MAXIMO * 0x101)
        {
            ofs -= ANCHO;
            *ofs = dx;
            dx += 0x101;
        }
        di++;
    }*/

    if (kbhit()) theEnd();
}

inline uint32_t rotr(uint32_t value, uint8_t count)
{
    return (value >> count) | (value << (32 - count));
}

inline uint32_t rotl(uint32_t value, uint8_t count)
{
    return (value << count) | (value >> (32 - count));
}

void stabylize()
{
    uint32_t eax, edx;
    uint16_t si, di, bx, cx;

    si = actualPage;
    di = otherPage;
    actualPage = di;
    otherPage = si;
    si += BORDE * ALTO;
    di += BORDE * ALTO;
    cx = (ANCHO * ALTO - BORDE * ALTO) >> 1;

    for (bx = 0; bx != cx; bx++)
    {
        eax =  *(uint32_t*)&vbuff[si - ALTO * 2];
        eax += *(uint32_t*)&vbuff[si + ALTO * 2];
        eax += *(uint32_t*)&vbuff[si + 2];
        eax += *(uint32_t*)&vbuff[si - 2];
        eax = rotr(eax, 16);
        eax = (eax & 0xFFFF0000) + ((int16_t)eax >> 1);
        eax = rotr(eax, 16);
        eax = (eax & 0xFFFF0000) + ((int16_t)eax >> 1);
        eax -= *(uint32_t*)&vbuff[di];
        edx = eax;
        edx = (edx & 0xFFFF0000) + ((int16_t)edx >> densityAdd);
        edx = rotr(edx, 16);
        edx = (edx & 0xFFFF0000) + ((int16_t)edx >> densityAdd);
        edx = rotr(edx, 16);
        eax -= edx;
        *(uint32_t*)&vbuff[di] = eax;
        di += 4;
        si += 4;
    }
}

void stabylize2()
{
    int16_t ax;
    uint16_t si, di, bx, cx;

    si = actualPage;
    di = otherPage;
    actualPage = di;
    otherPage = si;
    si += BORDE * ALTO;
    di += BORDE * ALTO;
    cx = ANCHO * ALTO - BORDE * ALTO - 100;

    for (bx = 0; bx != cx; bx++)
    {
        ax =  *(uint16_t*)&vbuff[si - ALTO * 2];
        ax += *(uint16_t*)&vbuff[si + ALTO * 2];
        ax += *(uint16_t*)&vbuff[si + 2];
        ax += *(uint16_t*)&vbuff[si - 2];
        ax += *(uint16_t*)&vbuff[si - ALTO * 2 - 2];
        ax += *(uint16_t*)&vbuff[si - ALTO * 2 - 2];
        ax += *(uint16_t*)&vbuff[si + ALTO * 2 - 2];
        ax += *(uint16_t*)&vbuff[si + ALTO * 2 - 2];
        ax >>= 3;
        *(uint16_t*)&vbuff[di] = ax;
        di += 2;
        si += 2;
    }
}

void putPoint(int16_t idx, uint16_t rnd1, uint16_t rnd2)
{
    uint16_t di;
    uint32_t esi = idx;

    esi = rotl(esi, 16);
    esi = (esi & 0xFFFF0000) + idx;
    di = ((rnd2 + ALTO * rnd1) << 1) + actualPage;

    *(uint32_t*)&vbuff[di               ] = esi;
    *(uint32_t*)&vbuff[di + 4           ] = esi;
    *(uint32_t*)&vbuff[di + ALTO * 2    ] = esi;
    *(uint32_t*)&vbuff[di + ALTO * 2 + 4] = esi;
    *(uint32_t*)&vbuff[di + ALTO * 4    ] = esi;
    *(uint32_t*)&vbuff[di + ALTO * 4 + 4] = esi;
    *(uint32_t*)&vbuff[di + ALTO * 6    ] = esi;
    *(uint32_t*)&vbuff[di + ALTO * 6 + 4] = esi;
}

void putBigPoint(int16_t idx, uint16_t rnd1, uint16_t rnd2)
{
    uint16_t di;
    uint32_t esi = idx;

    esi = rotl(esi, 16);
    esi = (esi & 0xFFFF0000) + idx;
    di = ((rnd2 + ALTO * rnd1) << 1) + actualPage;

   *(uint32_t*)&vbuff[di                 ] = esi;
   *(uint32_t*)&vbuff[di + 4             ] = esi;
   *(uint32_t*)&vbuff[di + 8             ] = esi;
   *(uint32_t*)&vbuff[di + 10            ] = esi;
   *(uint32_t*)&vbuff[di + ALTO * 2      ] = esi;
   *(uint32_t*)&vbuff[di + ALTO * 2 + 4  ] = esi;
   *(uint32_t*)&vbuff[di + ALTO * 2 + 8  ] = esi;
   *(uint32_t*)&vbuff[di + ALTO * 2 + 10 ] = esi;
   *(uint32_t*)&vbuff[di + ALTO * 4      ] = esi;
   *(uint32_t*)&vbuff[di + ALTO * 4 + 4  ] = esi;
   *(uint32_t*)&vbuff[di + ALTO * 4 + 8  ] = esi;
   *(uint32_t*)&vbuff[di + ALTO * 4 + 10 ] = esi;
   *(uint32_t*)&vbuff[di + ALTO * 6      ] = esi;
   *(uint32_t*)&vbuff[di + ALTO * 6 + 4  ] = esi;
   *(uint32_t*)&vbuff[di + ALTO * 6 + 8  ] = esi;
   *(uint32_t*)&vbuff[di + ALTO * 6 + 10 ] = esi;
   *(uint32_t*)&vbuff[di + ALTO * 8      ] = esi;
   *(uint32_t*)&vbuff[di + ALTO * 8 + 4  ] = esi;
   *(uint32_t*)&vbuff[di + ALTO * 8 + 8  ] = esi;
   *(uint32_t*)&vbuff[di + ALTO * 8 + 10 ] = esi;
   *(uint32_t*)&vbuff[di + ALTO * 10     ] = esi;
   *(uint32_t*)&vbuff[di + ALTO * 10 + 4 ] = esi;
   *(uint32_t*)&vbuff[di + ALTO * 10 + 8 ] = esi;
   *(uint32_t*)&vbuff[di + ALTO * 10 + 10] = esi;
}

uint16_t calcRand()
{
    randVal++;
    randVal = ((randVal * 32719 + 3) % 32749) + 0xF000;
    return randVal;
}

void readPoint(uint16_t *rnd1, uint16_t *rnd2)
{
    *rnd1 = (calcRand() & 0xFF) + 32;
    *rnd2 = (calcRand() & 0xFF) + 36;
}

void touch(int16_t idx)
{
    uint16_t rnd1, rnd2;
    readPoint(&rnd1, &rnd2);
    putPoint(idx, rnd1 >> 1, rnd2);
}

void bigTouch(int16_t idx)
{
    uint16_t rnd1, rnd2;
    readPoint(&rnd1, &rnd2);
    putBigPoint(idx, rnd1 >> 1, rnd2);
}

void putLine(int16_t idx, uint16_t rnd1)
{
    uint16_t di, cx;
    uint32_t esi = idx;

    esi = rotl(esi, 16);
    esi = (esi & 0xFFFF0000) + idx;
    di = (ANCHO_OLAS + 2 * 8) + (2 * ALTO * rnd1);

    for (cx = 0; cx != (ALTO / 2 - ANCHO_OLAS - 2 * 8); cx++)
    {
        *(uint32_t*)&vbuff[di            ] = esi;
        *(uint32_t*)&vbuff[di + ALTO * 2 ] = esi;
        *(uint32_t*)&vbuff[di + ALTO * 4 ] = esi;
        *(uint32_t*)&vbuff[di + ALTO * 6 ] = esi;
        *(uint32_t*)&vbuff[di + ALTO * 8 ] = esi;
        *(uint32_t*)&vbuff[di + ALTO * 10] = esi;
        di += 4;
    }
}

void readSinus(uint16_t val, uint16_t *rnd1, uint16_t *rnd2)
{
    uint16_t si, cx, dx;

    while (1)
    {
        si = tblFil[val];
        cx = lkpFil[si];
        if (cx != 0xFF) break;
        tblFil[val] = 0;
    }

    while (1)
    {
        si = tblCol[val];
        dx = lkpCol[si];
        if (dx != 0xFF) break;
        tblCol[val] = 0;
    }
    
    tblCol[val]++;
    tblFil[val]++;

    *rnd1 = dx;
    *rnd2 = cx;
}

void clearSinus()
{
    tblCol[0] = 10;
    tblCol[1] = 20;
    tblCol[2] = 40;
    tblCol[3] = 0;
    tblCol[4] = 0;
    tblFil[0] = 50;
    tblFil[1] = 20;
    tblFil[2] = 40;
    tblFil[3] = 40;
    tblFil[4] = 0;
}

void touchSinus(int16_t idx, uint16_t val)
{
    uint16_t rnd1, rnd2;
    readSinus(val, &rnd1, &rnd2);
    putPoint(idx, rnd1, rnd2);
}

void printFrame(int16_t num)
{
    int16_t i;
    for (i = 0; i < num; i++)
    {
        putFrame();
        stabylize();
    }
}

void P001()
{
    int16_t i, j;
    int16_t idx = 0;

    for (i = 0; i < 40; i++)
    {
        for (j = 0; j < 3; j++)
        {
            bigTouch(idx);
            stabylize();
            putFrame();
            stabylize();
            putFrame();
            stabylize();
            putFrame();
            stabylize();
            putFrame();
        }
        idx += 25;
    }

    for (i = 0; i < 40; i++)
    {
        for (j = 0; j < 3; j++)
        {
            bigTouch(idx);
            stabylize();
            putFrame();
            stabylize();
            putFrame();
            stabylize();
            putFrame();
            stabylize();
            putFrame();
        }
        idx -= 20;
    }
}

void P002()
{
    int16_t i, j;
    
    for (i = 0; i < 5; i++)
    {
        putPoint(3000, 80, 150);
        for (j = 0; j < 50; j++)
        {
            touch(300);
            touch(200);
            touch(100);
            stabylize();
            putFrame();
        }
    }

    for (i = 0; i < 5; i++)
    {
        stabylize();
        putFrame();
    }
}

void P003()
{
    int16_t i;

    for (i = 0; i < 30; i++)
    {
        bigTouch(-4000);
        stabylize2();
        putFrame();
        bigTouch(-4000);
        stabylize2();
        putFrame();
        bigTouch(-4000);
        stabylize2();
        putFrame();
    }

    for (i = 0; i < 20; i++)
    {
        stabylize2();
        putFrame();
    }
}

void P004()
{
    int16_t i, j;

    for (i = 0; i < 7; i++)
    {
        putPoint(3000, 80, 150);
        for (j = 0; j < 5; j++)
        {
            stabylize();
            putFrame();
        }
        
        putPoint(-2000, 80, 150);
        for (j = 0; j < 30; j++)
        {
            stabylize();
            putFrame();
        }
    }

    for (i = 0; i < 80; i++)
    {
        stabylize();
        putFrame();
    }
}

void P005(uint16_t A, uint16_t B)
{
    int16_t i;

    for (i = 0; i < 150; i++)
    {
        if (A == 0)
        {
            touchSinus(800, 0);
            touchSinus(800, 1);
            touchSinus(800, 2);
            touchSinus(800, 3);
            touchSinus(800, 4);
        }
        else if (A == 1)
        {
            touchSinus(800, 1);
            touchSinus(800, 2);
            touchSinus(800, 3);
            touchSinus(800, 4);
        }
        else if (A == 2)
        {
            touchSinus(800, 2);
            touchSinus(800, 3);
            touchSinus(800, 4);
        }
        else if (A == 3)
        {
            touchSinus(800, 3);
            touchSinus(800, 4);
        }
        else if (A == 4)
        {
            touchSinus(800, 4);
        }

        printFrame(B);
    }
}

void P006(uint16_t A, uint16_t B)
{
    int16_t i, j;
    
    for (i = 0; i < 4; i++)
    {
        if (A != 0) putLine(-1500, 1);
        for (j = 0; j < 50; j++)
        {
            if (B != 0) touch(250);
            stabylize();
            putFrame();
            stabylize();
            putFrame();
        }
    }
}

void main()
{
    clearScreen();
    printStr(1, 1, 0x0F, "RAIN EFFECT - (c) 1998 by Nguyen Ngoc Van");
    printStr(1, 2, 0x07, "Press any key to start...");
    getch();

    init13h();

    densityAdd = DENSITY_INIT;

    printFrame(1);
    stabylize();
    zeroPal();
    setPalette();
    
    memset(&vmem[320 * 100], 99, 320);    

    while (!kbhit())
    {
        randVal = SEMILLA_INIT;
        densityAdd = DENSITY_INIT;
        P001();
        /*P002();
        densityAdd = DENSITY_SET;
        P003();
        printFrame(100);
        P003();
        printFrame(100);
        densityAdd = DENSITY_INIT;
        P004();
        printFrame(100);
        clearSinus();
        P005(4, 2);
        P005(3, 2);
        P005(2, 2);
        P005(4, 1);
        P005(4, 1);
        P005(3, 1);
        densityAdd = DENSITY_SET;
        P006(1, 0);
        P006(1, 1);
        P006(0, 1);*/
    }
}
