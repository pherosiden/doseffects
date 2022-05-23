/*---------------------------------------------*/
/*   Text View Program - View Text File        */
/* Environment : Open Watcom C/C++ 1.9         */
/* Memory Model: Tiny/Small/Compact            */
/* Author      : Nguyen Ngoc Van               */
/* Email       : pherosiden@gmail.com          */
/* Create      : 15/11/1998                    */
/* Last Update : 30/12/1998                    */
/* Generate .COM file: wcl -zq -6 -fp6 -ox -mt */
/*---------------------------------------------*/
#include <dos.h>
#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

char **lines;
int16_t linecount;

void setDAC(char *dac)
{
    __asm {
        les     dx, dac
        mov     ax, 0x1002
        int     0x10
    }
}

void setMode(int16_t mode)
{
    __asm {
        mov     ax, mode
        int     0x10
    }
}

void getPalette(char no, int16_t count, char *pal)
{
    __asm {
        mov     dx, 0x3C7
        mov     al, no
        out     dx, al
        add     dx, 2
        les     di, pal
        mov     cx, count
        rep     insb
    }
}

void setPalette(char no, int16_t count, int16_t num, char *pal)
{
    __asm {
        mov     dx, 0x03C8
        mov     al, no
        out     dx, al
        inc     dx
        lds     si, pal
        add     si, num
        mov     cx, count
        rep     outsb
    }
}

void fillFrame(int16_t x1, int16_t y1, int16_t x2, int16_t y2, char col, char chr)
{
    __asm {
        mov     ax, 0xB800
        mov     es, ax
        xor     di, di
        mov     al, chr
        mov     ah, col
        mov     cx, y2
        sub     cx, y1
        inc     cx
    next:
        push 	cx
        mov     cx, x2
        sub     cx, x1
        inc     cx
        mov     di, x1
        shl     di, 1
        mov     bx, y1
        shl     bx, 5
        add     di, bx
        shl     bx, 2
        add     di, bx
        rep     stosw
        inc     y1
        pop     cx
        loop    next
    }
}

void printText(int16_t x, int16_t y, char col, char *msg)
{
    __asm {
        lds     si, msg
        mov     ax, 0xB800
        mov     es, ax
        xor     di, di
        add     di, x
        shl     di, 1
        mov     bx, y
        shl     bx, 5
        add     di, bx
        shl     bx, 2
        add     di, bx
        mov     ah, col
    next:
        lodsb
        test    al, al
        jz      quit
        cmp     al, 0x09
        je      puttab
        stosw
        jmp     next
    puttab:
        mov     al, 32
        stosw
        stosw
        stosw
        jmp     next
    quit:
    }
}

void hideCursor()
{
    __asm {
        mov     ah, 0x01
        mov     cx, 0x2020
        int     0x10
    }
}

void clearScreen()
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

void setColor()
{
    int16_t i;
    const char col[] = {0, 104, 54, 31};
    char dac[17] = {0};
    char pal[768] = {0};

    setMode(0x13);
    getPalette(0, 256 * 3, pal);
    setMode(0x03);

    for (i = 0; i < 4; i++) setPalette(i, 3 * 1, 3 * col[i], pal);
    setPalette(4, 14 * 3, 64 * 3, pal);

    dac[16] = 0;
    for (i = 0; i < 16; i++) dac[i] = i;
    setDAC(dac);
}

void readFile(char *fname)
{
    FILE *fp;
    char buffer[256] = {0};

    fp = fopen(fname, "rt");
    if (!fp)
    {
        setMode(0x03);
        printf("File %s does not exist\n", fname);
        exit(1);
    }

    linecount = 0;
    while (fgets(buffer, 255, fp)) linecount++;
    
    lines = (char**)calloc(linecount, sizeof(char*));
    if (!lines)
    {
        setMode(0x03);
        printf("Not enough memory to allocation\n");
        exit(1);
    }

    linecount = 0;
    rewind(fp);

    while (fgets(buffer, 255, fp))
    {
        lines[linecount] = (char*)calloc(strlen(buffer) + 1, 1);
        if (!lines[linecount])
        {
            setMode(0x03);
            printf("Not enough memory at line %d\n", linecount);
            exit(1);
        }

        buffer[strlen(buffer) - 1] = 0;
        strcpy(lines[linecount], buffer);
        linecount++;
    }

    fclose(fp);
}

void drawScreen()
{
    hideCursor();
    fillFrame(0, 0, 79, 0, 0x23, 32);
    fillFrame(0, 1, 79, 23, 0x13, 32);
    fillFrame(0, 24, 79, 24, 0x23, 32);
    printText(2, 0, 0x23, "Text View v1.0 (c) 1998 by Nguyen Ngoc Van");
    printText(58, 0, 0x23, "Line      Column");
    printText(2, 24, 0x23, "Arrows, PgUp, PgDn, Home, End-Scroll        F, N-Find       Esc-Quit");
}

void freeLines()
{
    int16_t i;
    for (i = 0; i < linecount; i++) free(lines[i]);
    free(lines);
}

void showPageText(int16_t x, int16_t y)
{
    int16_t i;
    int16_t len;
    char dummpy[82] = {0};

    for (i = 0; i < 23; i++)
    {
        len = strlen(lines[y + i]) - x;

        if (len < 0) len = 0;
        if (len > 80) len = 80;

        memmove(dummpy, &lines[y + i][x], len);

        dummpy[len] = 0;
        printText(0, i + 1, 0x14 + (i >> 1), dummpy);
    }
}

void textView(char *fname)
{
    int16_t x = 0, y = 0;
    char key, num[5] = {0};

    readFile(fname);
    drawScreen();

    do {
        fillFrame(63, 0, 67, 0, 0x23, 32);
        sprintf(num, "%d", y + 1);
        printText(63, 0, 0x28, num);
        fillFrame(75, 0, 79, 0, 0x23, 32);
        sprintf(num, "%d", x + 1);
        printText(75, 0, 0x28, num);
        showPageText(x, y);

        if (!(key = getch())) key = getch();

        switch (key)
        {
            case 72: y--; break;
            case 80: y++; break;
            case 73: y -= 23; break;
            case 81: y += 23; break;
            case 75: x -= 20; break;
            case 77: x += 20; break;
            case 71: x = 0; y = 0; break;
            case 79: x = 0; y = linecount - 23; break;
        }

        if (y > linecount - 23) y = linecount - 23;
        if (y < 0) y = 0;

        if (x > 236) x = 236;
        if (x < 0) x = 0;

        fillFrame(0, 1, 79, 23, 0x13, 32);
    } while(key != 27);
}

void main(int argc, char *argv[])
{
    if (argc < 2)
    {
        clearScreen();
        printText(1, 1, 0x0F, "Text View - View Text File v1.0 (c) 1998 by Nguyen Ngoc Van");
        printText(1, 2, 0x0F, "Syntax: textview <text file>"); return;
    }

    setColor();
    textView(argv[1]);
    setMode(0x03);
    freeLines();
}
