/*---------------------------------------------------*/
/* Packet  : Demo & Effect                           */
/* Effect  : Fire                                    */
/* Author  : Nguyen Ngoc Van                         */
/* Memory  : Small                                   */
/* Address : pherosiden@gmail.com                    */
/* Website : http://www.codedemo.net                 */ 
/* Created : 02/02/1998                              */
/* Please sent to me any bugs or suggests.           */
/* You can use freely this code. Have fun :)         */ 
/*---------------------------------------------------*/

#include <dos.h>
#include <mem.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <conio.h>

#define M_PI    3.141592f

typedef struct {
    int16_t cx, cy;
    int16_t dx, dy;
    int16_t life;
} PART;

PART particles[1024] = {0};
    
uint8_t br = 0;
int16_t frames = 0;
int16_t sintab[256] = {0};
uint8_t pal[256][3] = {0};
uint8_t vbuff[200][320] = {0};

int16_t roundf(float x)
{
    if (x > 0.0) return x + 0.5;
    return x - 0.5;
}

void setPalette()
{
    __asm {
        mov     dx, 0x03C8
        xor     al, al
        out     dx, al
        inc     dx
        mov     cx, 768
        lea     si, pal
    next:
        mov     bl, 127
        sub     bl, [si]
        mov     al, bl
        shl     ax, 8
        mul     bl
        shl     ax, 2
        lodsb
        add     al, ah
        mul     br
        mov     al, ah
        shr     ax, 8
        out     dx, al
        loop    next
    }
}

void retrace()
{
    __asm {
        mov     dx, 0x03DA
    waitV:
        in      al, dx
        test    al, 0x08
        jnz     waitV
    waitH:
        in      al, dx
        test    al, 0x08
        jz      waitH
    }
}

void flip(uint16_t dst)
{
    __asm {
        mov     es, dst
        mov     ax, seg vbuff
        mov     ds, ax
        xor     si, si
        xor     di, di
        mov     cx, 16000
        rep     movsd
    }
}

void clearTextMem()
{
    __asm {
        mov     ax, 0xB800
        mov     es, ax
        xor     di, di
        xor     ax, ax
        mov     cx, 2000
        rep     stosw
    }
}

void printStr(int16_t x, int16_t y, uint8_t col, char *msg)
{
    int16_t len = strlen(msg);

    __asm {
        mov     ax, 0xB800
        mov     es, ax
        xor     di, di
        lds     si, msg
        add     di, x
        shl     di, 1
        mov     bx, y
        shl     bx, 5
        add     di, bx
        shl     bx, 2
        add     di, bx
        mov     ah, col
        mov     cx, len
    next:
        lodsb
        stosw
        loop    next
    }
}

void fireBlur()
{
    __asm {
        mov     ax, seg vbuff
        mov     es, ax
        mov     ds, ax
        xor     di, di
        xor     si, si
        mov     cx, 63360
        xor     bx, bx
    lp1:
        xor     ax, ax
        mov     bl, [di]
        add     ax, bx
        mov     bl, [di + 319]
        add     ax, bx
        mov     bl, [di + 320]
        add     ax, bx
        mov     bl, [di + 321]
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

inline int16_t sat(int16_t b, int16_t min, int16_t max)
{
    if (b > max) return max;
    if (b < min) return min;
    return b;
}

inline int16_t pike(int16_t x, int16_t arg, int16_t a, int16_t b, int16_t c)
{
    if (x < arg) return a + (b - a) * x / arg;
    return b + (c - b) * (x - arg) / (256 - arg);
}

inline int16_t random(int16_t x)
{
    if (x == 0) return 0;
    return rand() % x;
}

void showFire(PART *parts)
{
    int16_t i, cnt;
    int16_t x, y;

    for (i = 0; i < 256; i++)
    {
        pal[i][0] = sat(i, 0, 128);
        pal[i][1] = sat(i - 48, 0, 120);
        pal[i][2] = pike(i, 32, 0, 32, 0) + pike(i, 128, 0, 0, 200);
    }

    cnt = 0;
    frames = 0;
    br = 120;
    setPalette();

    do {
        if (frames > 800) frames = 0;
        if (br < 120) br += 4;

        x = 160 + sintab[(frames / 3) & 0xFF];
        y = 80 - (sintab[(frames + 60) & 0xFF] >> 1);

        for (i = 0; i < 4; i++)
        {
            parts[cnt].cx = (x << 5) + random(100);
            parts[cnt].cy = (y << 5) + random(100);

            parts[cnt].dx = -16383 + random(32767);
            parts[cnt].dy = -16383 + random(32767);

            parts[cnt].life = 10 + random(400);
            
            cnt++;
        }

        i = 0;

        do {
            parts[i].life--;

            if (parts[i].life <= 0)
            {
                parts[i] = parts[cnt - 1];
                cnt--;
            }
            else
            {
                vbuff[parts[i].cy >> 5][parts[i].cx >> 5] = 64 + (parts[i].life >> 1);

                parts[i].cx += (parts[i].dx >> 8);
                parts[i].cy += (parts[i].dy >> 8);

                parts[i].dy += 256;

                parts[i].dx -= (parts[i].dx >> 6);
                parts[i].dy -= (parts[i].dy >> 6);

                if (parts[i].cx < 200 || parts[i].cx > 10100) parts[i].dx = -parts[i].dx;
                if (parts[i].cy > 6200) parts[i].dy = -parts[i].dy;
            }

            i++;
        } while (i < cnt);

        fireBlur();
        flip(0xA000);
        retrace();
        frames++;
    } while(!kbhit());
}

void main()
{
    int16_t i;
    void *parts = particles;

    clearTextMem();
    printStr(1, 1, 0x0F, "Fire Blur - (c) 1998 by Nguyen Ngoc Van");
    printStr(1, 2, 0x07, "Press any key to start demo . . .");
    getch();

    srand(time(NULL));
    
    for (i = 0; i < 256; i++) sintab[i] = roundf(sin(i * M_PI / 128) * 64);

    __asm {
        mov     ax, 0x13
        int     0x10
        push    parts
        call    showFire
        pop     parts
        mov     ax, 0x03
        int     0x10
    }
}
