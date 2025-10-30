/*----------------------------------------------------*/
/* Packet  : Demo & Effect                            */
/* Effect  : Copper3                                  */
/* Author  : Nguyen Ngoc Van                          */
/* Memory  : Tiny                                     */
/* Heaps   : 640K                                     */
/* Address : pherosiden@gmail.com                     */
/* Website : http://www.codedemo.net                  */
/* Created : 21/07/1998                               */
/* Please sent to me any bugs or suggests.            */
/* You can use freely this code. Have fun :)          */
/* Generate .COM file: wcl -zq -3 -ox -mt copper.c    */
/*----------------------------------------------------*/

#include <stdint.h>
#include <conio.h>

void main()
{
    uint16_t pos[] = {3, 2, 2, 1, 1, 0};

    /*=========== C VERSION ==========*/
    /*uint16_t i, j, k, col;
    while (!kbhit())
    {
        for (i = 390; i > 0; i--)
        {
            if (i == 390)
            {
                while (inp(0x03DA) & 8);
                while (!(inp(0x03DA) & 8));
            }
            else
            {
                while (inp(0x03DA) & 1);
                while (!(inp(0x03DA) & 1));
            }
            
            outp(0x03C8, 0);
            for (j = 0; j < 3; j++)
            {
                k = j << 1;
                if (i == 1)
                {
                    pos[k + 1] -= pos[k];
                    if (pos[k + 1] <= -263) pos[k] = -pos[k];
                }
                col = i + pos[k + 1];
                if (col > 127) col = 0;
                if (col > 63) col = 255 - col;
                outp(0x03C9, col);
            }
        }
    }*/
    /*============ ASM VERSION ============*/
    while (!kbhit())
    {
        __asm {
            mov     ah, 8
            mov     cx, 390
        st0:
            mov     dx, 0x03DA
        st1:
            in      al, dx
            and     al, ah
            jnz     st1
        st2:
            in      al, dx
            and     al, ah
            jz      st2
            mov     dl, 0xC8
            xchg    ax, bx
            out     dx, al
            inc     dx
            lea     si, pos
            mov     bl, 3
        st3:
            lodsw
            loop    st4
            sub     [si], ax
            cmp     word ptr [si], -263
            ja      st4
            neg     word ptr [si - 2]
        st4:
            inc     cx
            lodsw
            add     ax, cx
            cmp     ax, 127
            jbe     st5
            xor     al, al
        st5:
            cmp     al, 64
            jb      st6
            not     al
        st6:
            out     dx, al
            dec     bx
            jpo     st3
            mov     ah, 1
            loop    st0
        }
    }
}
