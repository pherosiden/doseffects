/*---------------------------------------------------*/
/* Packet  : Demo & Effect                           */
/* Effect  : Fire                                    */
/* Author  : Nguyen Ngoc Van                         */ 
/* Memory  : Small                                   */
/* Address : pherosiden@gmail.com                    */
/* Website : http://www.codedemo.net                 */
/* Created : 27/01/1998                              */
/* Please sent to me any bugs or suggests.           */
/* You can use freely this code. Have fun :)         */
/*---------------------------------------------------*/
#include <dos.h>
#include <time.h>
#include <stdlib.h>
#include <stdint.h>
#include <conio.h>

uint8_t delta = 0;
int16_t vbuff[102][160] = {0};
uint16_t *vmem = (uint16_t *)0xA0000000L;

void setRGB(uint8_t c, uint8_t r, uint8_t g, uint8_t b)
{
    __asm {
        mov     dx, 0x03C8
        mov     al, c
        out     dx, al
        inc     dx
        mov     al, r
        out     dx, al
        mov     al, g
        out     dx, al
        mov     al, b
        out     dx, al
    }
}

void setFirePal()
{
    int16_t i;
    for (i = 0; i < 64; i++)
    {
        setRGB(i      ,  i,  0, 0);
        setRGB(i +  64, 63,  i, 0);
        setRGB(i + 128, 63, 63, i);
        setRGB(i + 192, 63, 63, 0);
    }
}

void interpolation()
{
    __asm {
        mov     cx, 16160
        lea     di, vbuff + 320
    lp1:
        mov     ax, [di - 2]
        add     ax, [di]
        add     ax, [di + 2]
        add     ax, [di + 320]
        shr     ax, 2
        jz      lp2
        sub     ax, 1
    lp2:
        mov     [di - 320], ax
        add     di, 2
        loop    lp1
    }
}

void purgeBuf()
{
    __asm {
        les     di, vmem
        lea     si, vbuff
        mov     dx, 100
    l3:
        mov     bx, 2
    l2:
        mov     cx, 160
    l1:
        mov     al, [si]
        mov     ah, al
        stosw
        lodsw
        dec     cx
        jnz     l1
        sub     si, 320
        dec     bx
        jnz     l2
        add     si, 320
        dec     dx
        jnz     l3
    }
}

void retrace()
{
    __asm {
        mov     dx, 0x03DA
    waitH:
        in      al, dx
        and     al, 0x08
        jnz     waitH
    waitV:
        in      al, dx
        and     al, 0x08
        jz      waitV
    }
}

void main()
{
    int16_t i;

    __asm {
        mov     ax, 0x13
        int     0x10
    }

    srand(time(NULL));
    setFirePal();

    do {
        for (i = 0; i < 160; i++)
        {
            if ((rand() % 10) < 5) delta = (rand() % 2) * 255;
            vbuff[100][i] = delta;
            vbuff[101][i] = delta;
        }
        interpolation();
        retrace();
        purgeBuf();
    } while (!kbhit());

    __asm {
        mov     ax, 0x03
        int     0x10
    }
}
