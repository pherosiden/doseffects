/*----------------------------------------------------*/
/* Packet  : Demo & Effect                            */
/* Effect  : Line Bobbing                             */
/* Author  : Nguyen Ngoc Van                          */ 
/* Memory  : Compact                                  */
/* Heaps   : 640K                                     */
/* Address : pherosiden@gmail.com                     */
/* Website : http://www.codedemo.net                  */
/* Created : 14/08/2001                               */
/* Please sent to me any bugs or suggests.            */
/* You can use freely this code. Have fun :)          */
/*----------------------------------------------------*/

#include "gfxlib.c"

void *bobPixel = NULL;
int32_t dxadd = 0, dyadd = 0;
int32_t sxadd = 0, syadd = 0;

void checkBounds(int32_t a, int32_t *b, int32_t c)
{
    if (a > c) *b = -1;
    else if (a < 0) *b = 1;
}

void diagonalLine(int32_t x1, int32_t y1, int32_t x2, int32_t y2)
{
    __asm {
        mov    ecx, 1
        mov    edx, 1
        mov    edi, y2
        sub    edi, y1
        jge    keepy
        neg    edx
        neg    edi
    keepy:
        mov    dyadd, edx
        mov    esi, x2
        sub    esi, x1
        jge    keepx
        neg    ecx
        neg    esi
    keepx:
        mov    dxadd, ecx
        cmp    esi, edi
        jge    horzi
        xor    ecx, ecx
        xchg   esi, edi
        jmp    start
    horzi:
        xor    edx, edx
    start:
        mov    sxadd, ecx
        mov    syadd, edx
        mov    eax, edi
        shl    eax, 1
        mov    edi, eax
        sub    eax, esi
        mov    ebx, eax
        sub    eax, esi
        mov    ecx, x1
        mov    edx, y1
    next:
        dec    esi
        jz     quit
        push   eax
        push   edx
        push   ecx
        call   bobPixel
        pop    ecx
        pop    edx
        pop    eax
        cmp    ebx, 0
        jge    dline
        add    ecx, sxadd
        add    edx, syadd
        add    ebx, edi
        jmp    next
    dline:
        add    ecx, dxadd
        add    edx, dyadd
        add    ebx, eax
        jmp    next
    quit:
    }
}

void lineBob(uint32_t cnt)
{
    int32_t x1, y1, x2, y2;
    int32_t dx1, dx2, dy1, dy2;
    
    x1 = rand() % lfbWidth;
    x2 = rand() % lfbWidth;
    y1 = rand() % lfbHeight;
    y2 = rand() % lfbHeight;
    
    dx1 = 1;
    dy1 = 1;
    dx2 = -1;
    dy2 = -1;
    
    while (!kbhit() && cnt < 4000)
    {
        x1 += dx1;
        x2 += dx2;
        y1 += dy1;
        y2 += dy2;
        
        checkBounds(x1, &dx1, cmaxX);
        checkBounds(x2, &dx2, cmaxX);
        checkBounds(y1, &dy1, cmaxY);
        checkBounds(y2, &dy2, cmaxY);

        if (x1 < 0) x1 = 0;
        if (x2 < 0) x2 = 0;
        if (y1 < 0) y1 = 0;
        if (y2 < 0) y2 = 0;
        
        if (!(cnt % 10)) waitRetrace();

        diagonalLine(x1, y1, x2, y2);
        cnt++;
    }
}

int main()
{
    uint32_t cnt = 0;

    bobPixel = putPixelBob;

    if (!setVesaMode(640, 480, 8, 0)) return 1;

    srand(time(NULL));
    
    do {
        cnt = 0;
        makeFunkyPalette();	
        lineBob(cnt);
        clearScreen(0);
    } while (!kbhit());
    
    closeVesaMode();
    return 1;
}
