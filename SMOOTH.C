/*--------------------------------------------*/
/*   Smooth scrolling font                    */   
/* Compile: bcc -mt -tDc -O3 -3               */
/*--------------------------------------------*/

#include <dos.h>
#include <conio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <conio.h>

const char *scrolledText = "Smooth Text Scroll in text mode (c) 1998 by NGUYEN NGOC VAN ";
uint8_t masktab[] = {1, 2, 4, 8, 16, 32, 64, 128};
uint8_t fonttab[256][32] = {0};
int16_t len, col;

void pokeb(uint16_t seg, uint16_t ofs, uint8_t val)
{
    *((uint8_t*)MK_FP(seg, ofs)) = val;
}

uint8_t peekb(uint16_t seg, uint16_t ofs)
{
    return *((uint8_t*)MK_FP(seg, ofs));
}

void setCharWidth()
{
    __asm {
        // Change To 640 Horz Res
        mov     dx, 0x3CC
        in      al, dx

        and     al, -13
        mov     dx, 0x3C2
        out     dx, al

        // Turn Off Sequence Controller
        mov     dx, 0x3C4
        xor     al, al
        out     dx, al
        mov     dx, 0x3C5
        xor     al, al
        out     dx, al

        // Reset Sequence Controller
        mov     dx, 0x3C4
        xor     al, al
        out     dx, al
        mov     dx, 0x3C5
        mov     al, 3
        out     dx, al

        // Switch To 8 Pixel Wide Fonts
        mov     dx, 0x3C4
        mov     al, 1
        out     dx, al
        mov     dx, 0x3C5
        in      al, dx
        or      al, 1
        out     dx, al

        // Turn Off Sequence Controller
        mov     dx, 0x3C4
        xor     al, al
        out     dx, al
        mov     dx, 0x3C5
        xor     al, al
        out     dx, al

        // Reset Sequence Controller
        mov     dx, 0x3C4
        xor     al, al
        out     dx, al
        mov     dx, 0x3C5
        mov     al, 3
        out     dx, al

        // Center Screen
        mov     dx, 0x03DA
        in      al, dx
        mov     dx, 0x3C0
        mov     al, 0x13
        or      al, 0x20
        out     dx, al
        xor     al, al
        out     dx, al
    }
}

void writeScrollTextCharacters(uint8_t row)
{
    int16_t i;

    __asm {
        // Set Fonts 0 & 1
        mov     bl, 4
        mov     ax, 0x1103
        int     0x10
    }

    // Write Characters
    for (i = 0; i < 80; i++)
    {
        // Set Characters
        pokeb(0xB800, (row << 7) + (row << 5) + (i << 1), i);

        // Set Attribute To Secondary Font
        pokeb(0xB800, (row << 7) + (row << 5) + (i << 1) + 1,
        peekb(0xB800, (row << 7) + (row << 5) + (i << 1) + 1) | 0x08);
    }
}

void setAccessToFontMemory()
{
    __asm {
        // Turn Off Sequence Controller
        mov     dx, 0x3C4
        xor     al, al
        out     dx, al
        mov     dx, 0x3C5
        mov     al, 1
        out     dx, al

        // Reset Sequence Controller
        mov     dx, 0x3C4
        xor     al, al
        out     dx, al
        mov     dx, 0x3C5
        mov     al, 3
        out     dx, al

        // Change From Odd/Even Addressing to Linear
        mov     dx, 0x3C4
        mov     al, 4
        out     dx, al
        mov     dx, 0x3C5
        mov     al, 7
        out     dx, al

        // Switch Write Access To Plane 2
        mov     dx, 0x3C4
        mov     al, 2
        out     dx, al
        mov     dx, 0x3C5
        mov     al, 4
        out     dx, al

        // Set Read Map Reg To Plane 2
        mov     dx, 0x3CE
        mov     al, 4
        out     dx, al
        mov     dx, 0x3CF
        mov     al, 2
        out     dx, al

        // Set Graphics Mode Reg
        mov     dx, 0x3CE
        mov     al, 5
        out     dx, al
        mov     dx, 0x3CF
        xor     al, al
        out     dx, al

        // Set Misc. Reg
        mov     dx, 0x3CE
        mov     al, 6
        out     dx, al
        mov     dx, 0x3CF
        mov     al, 12
        out     dx, al
    }
}

void setAccessToTextMemory()
{
    __asm {
        // Turn Off Sequence Controller
        mov     dx, 0x3C4
        xor     al, al
        out     dx, al
        mov     dx, 0x3C5
        mov     al, 1
        out     dx, al

        // Reset Sequence Controller
        mov     dx, 0x3C4
        xor     al, al
        out     dx, al
        mov     dx, 0x3C5
        mov     al, 3
        out     dx, al

        // Change To Odd/Even Addressing
        mov     dx, 0x3C4
        mov     al, 4
        out     dx, al
        mov     dx, 0x3C5
        mov     al, 3
        out     dx, al

        // Switch Write Access
        mov     dx, 0x3C4
        mov     al, 2
        out     dx, al
        mov     dx, 0x3C5
        mov     al, 3
        out     dx, al

        // Set Read Map Reg
        mov     dx, 0x3CE
        mov     al, 4
        out     dx, al
        mov     dx, 0x3CF
        xor     al, al
        out     dx, al

        // Set Graphics Mode Reg
        mov     dx, 0x3CE
        mov     al, 5
        out     dx, al
        mov     dx, 0x3CF
        mov     al, 0x10
        out     dx, al

        // Set Misc. Reg
        mov     dx, 0x3CE
        mov     al, 6
        out     dx, al
        mov     dx, 0x3CF
        mov     al, 14
        out     dx, al
    }
}

void makeFontDefTable()
{
    int16_t i, j;

    setAccessToFontMemory();

    for (i = 0; i < 256; i++) for (j = 0; j < 32; j++)
    fonttab[i][j] = peekb(0xB800, (i << 5) + j);

    setAccessToTextMemory();
}

void clearSecondFontMemory()
{
    int16_t i;

    setAccessToFontMemory();
    for (i = 0; i < 32 * 256; i++) pokeb(0xB800, 0x4000 + i, 0);
    setAccessToTextMemory();
}

void scrollMessage()
{
    int16_t j;
    
    setAccessToFontMemory();

    __asm {
        // Wait For Retrace
        mov     dx, 0x03DA

    waitR:
        in      al, dx
        test    al, 0x08
        jz      waitR

        // Scroll Text One Pixel To The Left
        mov     ax, 0xB800 + (0x4000 / 16)
        mov     es, ax
        mov     cx, 32

    row:
        mov     di, (79 * 32) - 1
        add     di, cx
        shl     byte ptr es:[di], 1
        pushf
        sub     di, 32
        popf
        push    cx
        mov     cx, 79

    chars:
        rcl     byte ptr es:[di], 1
        pushf
        sub     di, 32
        popf
        loop    chars
        pop     cx
        loop    row
    }

    if (col < 0)
    {
          col = 7;
          len++;
    }
    else
    {
        col--;
    }

    if (len >= strlen(scrolledText)) len = 0;

    // Write New Column Of Pixels
    for (j = 0; j <= 31; j++)
        pokeb(0xB800, 0x4000 + (79 * 32) + len,
        peekb(0xB800, 0x4000 + (79 * 32) + j) |
        (fonttab[scrolledText[len]][j] & masktab[col]) >> col);

    setAccessToTextMemory();
}

void turnCursorOff()
{
    __asm {
        mov     dx, 0x3D4
        mov     al, 0x0A
        out     dx, al
        mov     dx, 0x3D5
        in      al, dx
        or      al, 32
        out     dx, al
    }
}

void turnCursorOn()
{
    __asm {
        mov     dx, 0x3D4
        mov     al, 0x0A
        out     dx, al
        mov     dx, 0x3D5
        in      al, dx
        and     al, 0x20
        not     al
        out     dx, al
    }
}

void setTextMode()
{
    __asm {
        mov     ax, 0x03
        int     0x10
    }
}

void main()
{
    len = 0;
    col = 8;

    setTextMode();
    turnCursorOff();
    setCharWidth();
    makeFontDefTable();
    clearSecondFontMemory();
    writeScrollTextCharacters(23);
    do scrollMessage(); while(!kbhit());
    setTextMode();
    turnCursorOn();
}
