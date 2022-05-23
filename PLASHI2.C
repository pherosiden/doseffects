/*----------------------------------------------------*/
/* Packet  : Demo & Effect                            */
/* Effect  : Plasma high colors                       */
/* Author  : Nguyen Ngoc Van                          */
/* Memory  : Compact                                  */
/* Heaps   : 640K                                     */
/* Address : pherosiden@gmail.com                     */
/* Website : http://www.codedemo.net                  */
/* Created : 15/04/1998                               */
/* Please sent to me any bugs or suggests.            */
/* You can use freely this code. Have fun :)          */
/*----------------------------------------------------*/

#include "gfxlib.c"

inline int32_t sqr(int32_t x)
{
    return x * x;
}

int main()
{
    GFX_IMAGE plasma, screen;

    void *data;
    int32_t y, exfade;

    uint32_t tmstart;
    uint8_t sinx[256];
    uint32_t frames, tectr, ofs;
    uint16_t x1, x2, x3, y1, y2, y3;
    uint16_t cr, cg, cb, endx, a, b, c;

    for (ofs = 0; ofs < 256; ofs++) sinx[ofs] = sin(ofs * M_PI / 128) * 127 + 128;

    if (!setVesaMode(320, 240, 32, 85)) fatalError("Cannot init video mode.\n");
    if (!newImage(160, 120, &plasma)) fatalError("Cannot open image plasma.\n"); 
    if (!newImage(320, 240, &screen)) fatalError("Cannot open image screen.\n"); 

    frames  = 0;
    exfade  = 256;
    endx    = plasma.mWidth >> 1;
    data    = plasma.mData;
    tmstart = GetTicks();

    while (exfade)
    {
        ofs = 0;
        tectr = frames * 10;
        x1 = sinx[(tectr / 12) & 0xFF];
        x2 = sinx[(tectr / 11) & 0xFF];
        x3 = sinx[frames & 0xFF];
        y1 = sinx[((tectr >> 3) + 64) & 0xFF];
        y2 = sinx[(tectr / 7 + 64) & 0xFF];
        y3 = sinx[(tectr / 12 + 64) & 0xFF];

        for (y = 0; y < plasma.mHeight; y++)
        {
            a = sqr(y - y1) + sqr(x1);
            b = sqr(y - y2) + sqr(x2);
            c = sqr(y - y3) + sqr(x3);

            cr = sinx[(a >> 6) & 0xFF];
            cg = sinx[(b >> 6) & 0xFF];
            cb = sinx[(c >> 6) & 0xFF];
            
            __asm {
                xor    ax, ax
                mov    edi, data
                add    edi, ofs
                xor    dx, dx
            lp:
                xor    ebx, ebx
                mov    cl, 6
                mov    bx, ax
                push   ax
                sub    bx, x3
                add    bx, c
                mov    c, bx
                shr    bx, cl
                and    bx, 0x00FF
                mov    bl, byte ptr sinx[ebx]
                mov    si, bx
                mov    bx, ax
                sub    bx, x2
                add    bx, b
                mov    b, bx
                shr    bx, cl
                and    bx, 0x00FF
                mov    dl, byte ptr sinx[ebx]
                mov    bx, ax
                sub    bx, x1
                add    bx, a
                mov    a, bx
                shr    bx, cl
                and    bx, 0x00FF
                mov    bl, byte ptr sinx[ebx]
                mov    ax, bx
                add    ax, cr
                mov    cr, bx
                shl    ebx, 16
                shl    eax, 15
                mov    ax, dx
                add    ax, cg
                mov    cg, dx
                shl    ax, 7
                mov    cx, si
                add    cx, cb
                mov    cb, si
                shr    cx, 1
                mov    al, cl
                mov    [edi], eax
                mov    bx, si
                mov    bh, byte ptr cg
                mov    [edi + 4], ebx
                add    edi, 8
                pop    ax
                inc    ax
                cmp    ax, endx
                jnae   lp
            }
            ofs += plasma.mRowBytes;
        }

        if (kbhit())
        {
            exfade -= 4;
            if (exfade < 0) exfade = 0;
            brightnessImage(&plasma, &plasma, exfade);
        }
        
        bilinearScaleImage(&screen, &plasma);
        waitRetrace();
        putImage(0, 0, &screen);
        frames++;
    };

    tmstart = GetTicks() - tmstart;
    freeImage(&plasma);
    freeImage(&screen);
    closeVesaMode();
    printf("%.2f frames per second.\n", frames * 18.2f / tmstart);
    return 0;
}
