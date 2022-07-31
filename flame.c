/*----------------------------------------------------*/
/* Packet  : Demo & Effect                            */ 
/* Effect  : Fire                                     */
/* Author  : Nguyen Ngoc Van                          */
/* Memory  : Small                                    */ 
/* Address : pherosiden@gmail.com                     */ 
/* Website : http://www.codedemo.net                  */
/* Created : 20/01/1998                               */
/* Please sent to me any bugs or suggests.            */ 
/* You can use freely this code. Have fun :)          */
/*----------------------------------------------------*/

#include <dos.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <conio.h>

uint8_t pal[256][3] = {0};
int16_t flames[102][160] = {0};

void readPalette()
{
    int16_t i, j;
    FILE *fp;
    
    fp = fopen("assets/flame.pal", "rt");
    if (!fp) return;

    for (i = 0; i < 256; i++)
    {
        for(j = 0; j < 3; j++)
        {
            fscanf(fp, "%d", &pal[i][j]);
            pal[i][j] >>= 2;
        }
    }

    fclose(fp);

    __asm {
        lea     si, pal
        mov     cx, 768
        mov     dx, 0x03C8
        xor     al, al
        out     dx, al
        inc     dx
        rep     outsb
    }
}

void retrace()
{
    __asm {
        mov     dx, 0x03DA
    waitH:
        in      al, dx
        test    al, 0x08
        jnz     waitH
    waitV:
        in      al, dx
        test    al, 0x08
        jz      waitV
    }
}

void main()
{
    int16_t i;
    uint8_t delta = 0;
        
    __asm {
        mov     ax, 0x13
        int     0x10
    }

    srand(time(NULL));
    readPalette();

    do {
        __asm {
            mov     cx, 16159
            lea     di, flames
            add     di, 320
        nt1:
            mov     ax, ds:[di - 2]
            add     ax, ds:[di]
            add     ax, ds:[di + 2]
            add     ax, ds:[di + 320]
            shr     ax, 2
            jz      nt2
            sub     ax, 1
        nt2:
            mov     word ptr ds:[di - 320], ax
            add     di, 2
            dec     cx
            jnz     nt1
        }

        for (i = 0; i < 160; i++)
        {
            if ((rand() % 10) < 5) delta = (rand() % 2) * 255;
            flames[100][i] = delta;
            flames[101][i] = delta;
        }

        retrace();
        
        __asm {
            lea     si, flames
            mov     ax, 0xA000
            mov     es, ax
            xor     di, di
            mov     dx, 100
        lp3:
            mov     bx, 2
        lp2:
            mov     cx, 160
        lp1:
            mov     al, [si]
            mov     ah, al
            stosw
            lodsw
            loop    lp1
            sub     si, 320
            dec     bx
            jnz     lp2
            add     si, 320
            dec     dx
            jnz     lp3
        }
    } while (!kbhit());

    __asm {
        mov     ax, 0x03
        int     0x10
    }
}
