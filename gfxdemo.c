/*------------------------------------------------*/
/*    GFXLIB demo (c) 2000 by Nguyen Ngoc Van     */
/*     Full support 8/15/16/24/32 bits color      */
/*        Support load/save BMP & PNG files       */
/* Using Linear Frame Buffer for best performance */
/*        Optimize code by using assembly         */
/*           Code by: Nguyen Ngoc Van             */
/*           Website: www.codedemo.net            */
/*             Email: pherosiden@gmail.com        */
/*------------------------------------------------*/

#include "gfxlib.c"
#include <graph.h>

// String buffer
typedef char STRBUFF[80];

// Show text intro message
int32_t fullSpeed = 0;
STRBUFF texts[17] = {0};
GFX_IMAGE fade1, fade2, flare;
GFX_IMAGE bump1, bump2, logo, sky;
GFX_IMAGE flares[16] = {0};

// inline optimize for multiple call
inline int32_t sqr(int32_t x)
{
    return x * x;
}

// check and exit program
void runExit(int32_t halt)
{
    int32_t i;
    GFX_IMAGE im;

    // capture current screen buffer
    newImage(lfbWidth, lfbHeight, &im);
    getImage(0, 0, lfbWidth, lfbHeight, &im);

    // decrease rgb and push back to screen
    for (i = 0; i != 32; i++)
    {
        fadeOutImage(&im, 8);
        waitRetrace();
        putImage(0, 0, &im);
    }

    // cleanup...
    freeImage(&im);
    freeImage(&fade1);
    freeImage(&fade2);
    freeImage(&flare);
    freeImage(&bump1);
    freeImage(&bump2);
    freeImage(&logo);
    freeImage(&sky);
    for (i = 0; i != 16; i++) freeImage(&flares[i]);

    // check for exit program
    if (halt)
    {
        closeVesaMode();
        printf("+-----------------------------------------------------+\n");
        printf("|    GFXLIB demo (c) 1998 - 2002 by Nguyen Ngoc Van   |\n");
        printf("|        Full support 8/15/16/24/32 bits color        |\n");
        printf("|          Support load/save BMP & PNG files          |\n");
        printf("|    Using Linear Frame Buffer for best performance   |\n");
        printf("|           Optimize code by using assember           |\n");
        printf("|      Use double buffering to performance render     |\n");
        printf("|           Code by: Nguyen Ngoc Van                  |\n");
        printf("|             Email: pherosiden@gmail.com             |\n");
        printf("|           Website: http://codedemo.net              |\n");
        printf("|         Reference: http://crossfire-designs.de      |\n");
        printf("+-----------------------------------------------------+\n");
        exit(1);
    }
}

// check for input key
int32_t checkKeyPressed()
{
    // empty keyboard buffer
    while(kbhit())
    {
        // read key pressed
        char key = getch();
        if (key == 27) runExit(1);	// <ESC> for exit program
        if (key == 13) return 1;	// <ENTER> for skip to next state
    }
    return 0;
}

// Show intro message text string
void showText(int32_t sx, int32_t sy, GFX_IMAGE *img, char *str)
{
    uint32_t col;
    int32_t x, y, i, len;
    char msg[2] = {0};
    
    // make scrolling text
    col = fromRGB(127, 127, 127);
    memcpy(texts[0], texts[1], sizeof(texts) - sizeof(texts[0]));
    strcpy(texts[16], str);

    // don't delay each character
    if (fullSpeed)
    {
        waitRetrace();
        putImage(sx, sy, img);
        for (i = 0; i != 17; i++) writeString(sx + 10, sy + 10 + i * 10, texts[i], col, 2);
        checkKeyPressed();
    }
    else
    {
        // show previous text
        for (y = 9; y >= 0; y--)
        {
            // fill original background
            waitRetrace();
            putImage(sx, sy, img);
            for (i = 0; i != 16; i++) writeString(sx + 10, sy + 10 + i * 10 + y, texts[i], col, 2);
        }

        x = 0;
        len = strlen(str);
        col = fromRGB(255, 255, 255);

        // show current text with delay each character
        for (i = 0; i != len; i++)
        {
            msg[0] = str[i];
            writeString(sx + 10 + x, sy + 10 + 160, msg, col, 2);
            x += getFontWidth(msg);

            // check for delay and skip
            if (!fullSpeed) delay(45);
            if (checkKeyPressed()) fullSpeed = 1;
        }
    }
}

void runIntro()
{
    const int32_t tw = 3, tg = tw + 5, tu = tg + 3, to = tu + 5, ts = to + 3;

    GFX_IMAGE ult, utb, trn, map;
    GFX_IMAGE scr, wcb, wci, gfx, gxb;
    
    int32_t i0, i1, i2;
    uint8_t mov = 0;
    uint8_t *buf1, *buf2;
    uint64_t tmstart, tmwait;

    // load image
    if (!loadImage("gfxulti.png", &ult)) fatalError("runIntro: cannot load gfxulti.png file.\n");
    if (!loadImage("gfxlogo.png", &gfx)) fatalError("runIntro: cannot load gfxlogo.png file.\n");
    if (!loadImage("gfxwelm.png", &wcb)) fatalError("runIntro: cannot load gfxwelm.png file.\n");
    if (!loadImage("map03.png", &map)) fatalError("runIntro: cannot load map03.png file.\n");

    // initialize buffer
    if (!newImage(lfbWidth, lfbHeight, &scr)) fatalError("runIntro: cannot open image.\n");
    if (!newImage(lfbWidth >> 1, lfbHeight >> 1, &trn)) fatalError("runIntro: cannot open image.\n");
    if (!newImage(wcb.mWidth, wcb.mHeight, &wci)) fatalError("runIntro: cannot open image.\n");
    if (!newImage(gfx.mWidth, gfx.mHeight, &gxb)) fatalError("runIntro: cannot open image.\n");
    if (!newImage(ult.mWidth, ult.mHeight, &utb)) fatalError("runIntro: cannot open image.\n");

    i0 = trn.mWidth * trn.mHeight;

    // initialize tunnel buffer
    buf1 = (uint8_t*)malloc(i0);
    buf2 = (uint8_t*)malloc(i0);
    if (!buf1 || !buf2) fatalError("runIntro: cannot alloc memory.\n");

    // calculate turnel buffer
    prepareTunnel(&trn, buf1, buf2);

    i0 = 30;
    i1 = 25;
    i2 = 0;

    // start record time
    tmstart = getCurrentTime();
    do {
        // draw and scale buffer
        tmwait = getCurrentTime();
        drawTunnel(&trn, &map, buf1, buf2, &mov, 1);
        scaleImage(&scr, &trn, 0);

        // redirect to image buffer
        changeDrawBuffer(scr.mData, scr.mWidth, scr.mHeight);
        
        // welcome message
        if ((getElapsedTime(tmstart) / timeRes >= tw) && (i0 >= 0))
        {
            if (i0 > 15)
            {
                blurImageEx(&wci, &wcb, i0 & 15);
                brightnessImage(&wci, &wci, (16 - (i0 & 15)) * 15 + 15);
                i0--;
            }
            else if ((getElapsedTime(tmstart) / timeRes >= tw + 3) && (i0 >= 0))
            {
                blurImageEx(&wci, &wcb, 15 - (i0 & 15));
                brightnessImage(&wci, &wci, ((i0 & 15) + 1) * 15 + 15);
                i0--;
            }

            if (i0 >= -1) putImageAdd(centerX - (wci.mWidth >> 1), centerY - (wci.mHeight >> 1), &wci);
            if (getElapsedTime(tmstart) / timeRes >= tg) i0 = -1;
        }
        // logo GFXLIB
        else if ((getElapsedTime(tmstart) / timeRes >= tg) && (i1 > 0))
        {
            blockOutMidImage(&gxb, &gfx, i1, i1);
            brightnessAlpha(&gxb, 255.0 - i1 / 30.0 * 255.0);
            putImageAlpha(centerX - (gxb.mWidth >> 1), centerY - (gxb.mHeight >> 1), &gxb);
            i1--;
            if (getElapsedTime(tmstart) / timeRes >= tu) i1 = 0;
        }
        // the ultimate message
        else if (i1 == 0)
        {
            putImageAlpha(centerX - (gfx.mWidth >> 1), centerY - (gfx.mHeight >> 1), &gfx);
            if ((getElapsedTime(tmstart) / timeRes >= tu) && (i2 <= 15))
            {
                blurImageEx(&utb, &ult, 15 - (i2 & 15));
                brightnessImage(&utb, &utb, ((i2 & 15) + 1) * 15 + 15);
                putImageAdd(centerX - (ult.mWidth >> 1), centerY + (gfx.mHeight >> 1) + 20, &utb);
                i2++;
            }
            else
            {
                putImageAdd(centerX - (ult.mWidth >> 1), centerY + (gfx.mHeight >> 1) + 20, &utb);
                if (getElapsedTime(tmstart) / timeRes >= to) fadeOutCircle((1.0 * getElapsedTime(tmstart) / timeRes - to) / 3.0 * 100.0, 20, 3, 0);
            }
        }

        // restore to render buffer
        restoreDrawBuffer();
        waitRetrace();
        putImage(0, 0, &scr);
        waitFor(tmwait, 50);
    } while (!checkKeyPressed() && getElapsedTime(tmstart) / timeRes < ts);

    // cleanup...
    freeImage(&map);
    freeImage(&trn);
    freeImage(&scr);
    freeImage(&wcb);
    freeImage(&wci);
    freeImage(&ult);
    freeImage(&gfx);
    freeImage(&utb);
    free(buf1);
    free(buf2);
}

void runBlocking(int32_t sx, int32_t sy)
{
    GFX_IMAGE im1, im2, im3;
    int32_t dec, posx, posy, width, height;

    // initialize buffer
    if (!newImage(fade1.mWidth, fade1.mHeight, &im2)) fatalError("runBlocking: cannot open image.\n");

    // blocking background
    dec = fade1.mWidth >> 2;
    do {
        dec--;
        blockOutMidImage(&im2, &fade1, dec << 1, dec << 1);
        brightnessImage(&im2, &im2, 255.0 - 1.0 * dec / (fade1.mWidth >> 2) * 255.0);
        waitRetrace();
        putImage(sx, sy, &im2);
    } while (dec > 0 && !checkKeyPressed());

    // save current background
    width  = fade1.mWidth;
    height = fade1.mHeight;
    putImage(sx, sy, &fade1);
    freeImage(&im2);

    // load next step
    if (!loadImage("gfxtext.png", &im1)) fatalError("runBlocking: cannot load PNG file.\n");
    if (!newImage(im1.mWidth, im1.mHeight, &im2)) fatalError("runBlocking: cannot open image.\n");
    if (!newImage(im1.mWidth, im1.mHeight, &im3)) fatalError("runBlocking: cannot open image.\n");

    // calculate current position and save current buffer
    posx = (width - im2.mWidth) >> 1;
    posy = (height - im2.mHeight) >> 1;
    getImage(sx + posx, sy + posy, im1.mWidth, im1.mHeight, &im3);

    // blocking next step
    dec = im1.mWidth >> 3;
    do {
        // start effect
        dec--;
        blockOutMidImage(&im2, &im1, dec << 1, dec << 1);
        brightnessAlpha(&im2, 255.0 - 1.0 * dec / (im1.mWidth >> 3) * 255.0);

        // render to screen
        waitRetrace();
        putImage(sx + posx, sy + posy, &im3);
        putImageAlpha(sx + posx, sy + posy, &im2);
    } while (dec > 0 && !checkKeyPressed());

    // cleanup...
    freeImage(&im1);
    freeImage(&im2);
    freeImage(&im3);
}

void runScaleUpImage(int32_t sx, int32_t sy)
{
    GFX_IMAGE im1, im2, im3;
    int32_t i, *tables = NULL;

    // initialize buffer
    if (!newImage(320, 240, &im1)) fatalError("runScaleUpImage: cannot open image.\n");
    if (!newImage(320, 240, &im2)) fatalError("runScaleUpImage: cannot open image.\n");
    if (!loadImage("gfxspr.png", &im3)) fatalError("runScaleUpImage: cannot open PNG file.\n");

    // setup lookup table
    tables = (int32_t*)malloc(im2.mWidth * sizeof(int32_t));
    if (!tables) fatalError("scaleUpImage: not enough memory for lookup tables.\n");
        
    do {
        // redirect to render buffer
        changeDrawBuffer(im1.mData, im1.mWidth, im1.mHeight);

        // put some random pixel and GFX message
        for (i = 0; i != 200; i++) putPixel(random(im1.mWidth - 4) + 2, random(im1.mHeight - 4) + 2, fromRGB(0, 255, 200));
        if (random(128) == 64) putImageAlpha((im1.mWidth - im3.mWidth) >> 1, (im1.mHeight - im3.mHeight) >> 1, &im3);

        // blur & scale buffer
        blurImage(&im1);
        scaleUpImage(&im2, &im1, tables, 7, 7);

        // restore to screen buffer to draw
        restoreDrawBuffer();
        waitRetrace();
        putImage(sx, sy, &im2);
        copyData(im1.mData, im2.mData, im2.mSize);
    } while (!checkKeyPressed());

    // cleanup...
    freeImage(&im1);
    freeImage(&im2);
    freeImage(&im3);
    free(tables);
}

void runCrossFade(int32_t sx, int32_t sy)
{
    GFX_IMAGE im;
    int32_t i = 0, up = 1, val = 1;

    // initialize render buffer
    if (!newImage(fade1.mWidth, fade1.mHeight, &im)) fatalError("runCrossFade: cannot open image file.\n");

    do {
        // check blending value
        if (i == 0) val = 1;
        else val = (i << 2) - 1;

        // blend image buffer
        blendImage(&im, &fade1, &fade2, val);

        // render to screen
        waitRetrace();
        putImage(sx, sy, &im);

        // check for change direction
        if (up) i++; else i--;
        if (i == 0 || i == 64) up = !up;
    } while (!checkKeyPressed());

    // cleanup...
    freeImage(&im);
}

void runAddImage(int32_t sx, int32_t sy)
{
    GFX_IMAGE img;
    int32_t pos = 320;
    
    // initialize render buffer
    if (!newImage(fade1.mWidth, fade1.mHeight, &img)) fatalError("runAddImage: cannot open image.\n");

    do {
        pos -= 4;
        
        // redirect to image buffer
        changeDrawBuffer(img.mData, img.mWidth, img.mHeight);

        // put normal background
        putImage(0, 0, &fade1);
        
        // put lens image with adding background pixel
        putImageAdd(320 - cos(pos / 160.0) * 320, 0, &flare);

        // restore and render
        restoreDrawBuffer();
        waitRetrace();
        putImage(sx, sy, &img);
    } while (pos != 0 && !checkKeyPressed());
}

void runRotateImage(int32_t sx, int32_t sy)
{
    GFX_IMAGE im;
    uint32_t degree = 0;
    int32_t *tables = NULL;	
    
    // initialize render buffer
    if (!newImage(fade2.mWidth, fade2.mHeight, &im)) fatalError("runRotateImage: cannot open image.\n");

    // pre-calculate lookup table
    tables = (int32_t*)malloc((2 * fade2.mWidth + fade2.mHeight) * sizeof(int32_t) + 2 * sizeof(int32_t));
    if (!tables) fatalError("rotateImage: cannot alloc lookup tables.\n");

    do {
        // copy background
        copyData(im.mData, fade1.mData, fade1.mSize);

        // rotate buffer
        rotateImage(&im, &fade2, tables, fade2.mWidth >> 1, fade2.mHeight >> 1, degree % 360, 1);

        // render to screen
        waitRetrace();
        putImage(sx, sy, &im);
        degree++;
    } while (!checkKeyPressed());

    // cleanup...
    freeImage(&im);
    free(tables);
}

void runBilinearRotateImage(int32_t sx, int32_t sy)
{
    GFX_IMAGE im;
    uint32_t degree = 0;

    // initialize render buffer
    if (!newImage(fade2.mWidth, fade2.mHeight, &im)) fatalError("runBilinearRotateImage: cannot open image.\n");

    do {
        // copy background
        copyData(im.mData, fade1.mData, fade1.mSize);

        // rotate buffer
        bilinearRotateImage(&im, &fade2, degree % 360, 1, 1);

        // render to screen
        waitRetrace();
        putImage(sx, sy, &im);
        degree++;
    } while (!checkKeyPressed());

    // cleanup...
    freeImage(&im);
}

void runAntialias(int32_t sx, int32_t sy)
{
    GFX_IMAGE img, dst;
    uint32_t i, col;

    // initialize image buffer
    if (!newImage(320, 240, &img)) fatalError("runAntialias: cannot allocate image.\n");
    if (!newImage(320, 240, &dst)) fatalError("runAntialias: cannot allocate image.\n");

    do {
        // redirect render to image buffer
        changeDrawBuffer(dst.mData, dst.mWidth, dst.mHeight);

        // draw antialias (smooth pixel) circle, line, ellipse
        for (i = 0; i != 3; i++)
        {
            // choose random color
            col = fromRGB(random(255) + 1, random(255) + 1, random(255) + 1);

            // which shape to be drawed
            switch (random(3))
            {
            case 0: drawLineAlpha(random(320), random(240), random(320), random(240), col); break;
            case 1: drawCircleAlpha(random(320), random(240), random(320) >> 2, col); break;
            case 2: drawEllipseAlpha(random(320), random(240), random(320), random(240), col); break;
            }
        }

        // restore and rendering...
        restoreDrawBuffer();
        waitRetrace();
        putImage(sx, sy, &dst);

        // fade-out current buffer
        fadeOutImage(&dst, 4);
    } while (!checkKeyPressed());

    // cleanup...
    freeImage(&img);
    freeImage(&dst);
}

void runLens()
{
    GFX_IMAGE scr;
    int32_t tx, ty, x, y, i;
    int32_t flareput[16] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
    int32_t flarepos[16] = {-1110, -666, 0, 1087, 1221, 1309, 1776, 2197, 2819, 3130, 3220, 3263, 3663, 3707, 4440, 5125};
    char *str = "Drag your mouse to see details and left click to exit!";

    // initialize mouse driver and load source image
    if (!initMouse()) fatalError("runLens: cannot init mouse.\n");
    if (!newImage(lfbWidth, lfbHeight, &scr)) fatalError("runLens: cannot open image.\n");

    // install new mouse interrupt
    installMouseHandler();
    mcd.mcx = centerX + 70;
    mcd.mdx = centerY - 80;

    // set limitation
    setMousePos(mcd.mcx, mcd.mdx);
    setMouseRange(0, 0, cmaxX, cmaxY);
    setMouseSensitivity(100, 100, 100);

    // pre-calculate text position
    tx = (scr.mWidth - getFontWidth(str)) >> 1;
    ty = scr.mHeight - getFontHeight(str) - 2;

    do {
        // redirect render to image buffer
        changeDrawBuffer(scr.mData, scr.mWidth, scr.mHeight);
        putImage(0, 0, &sky);
        fillRectSub(0, 0, cmaxX, cmaxY, fromRGB(0, (1.0 * mcd.mdx / cmaxY) * 64.0, (1.0 * mcd.mdx / cmaxY) * 64.0));

        // put all flare image to render buffer
        for (i = 0; i != 16; i++)
        {
            // is show?
            if (flareput[i])
            {
                // merge current image buffer to background
                x = (centerX + ((centerX - mcd.mcx) * (flarepos[i] - 2280) / 2280)) - (flares[i].mWidth >> 1);
                y = (centerY + ((centerY - mcd.mdx) * (flarepos[i] - 2280) / 2280)) - (flares[i].mHeight >> 1);
                putImageAdd(x, y, &flares[i]);
            }
        }

        // put logo and draw text message
        putImageAlpha(lfbWidth - logo.mWidth, lfbHeight - logo.mHeight, &logo);
        drawTextImage(tx, ty, str, fromRGB(255, 255, 255), &scr);

        // restore render buffer to screen buffer
        restoreDrawBuffer();
        waitRetrace();
        putImage(0, 0, &scr);
    } while (!checkKeyPressed() && !mcd.mbx);

    // cleanup...
    freeImage(&scr);
    unInstallMouseHandler();
}

void runBumpImage()
{
    GFX_IMAGE dst;
    int32_t lx, ly, cnt = 0;
    
    // loading source image
    if (!newImage(lfbWidth, lfbHeight, &dst)) fatalError("runBumpImage: cannot open image.\n");

    do {
        // calculate position
        lx = cos(cnt / 13.0) * 133 + 320;
        ly = sin(cnt / 23.0) * 133 + 240;

        // start bumping buffer
        bumpImage(&dst, &bump1, &bump2, lx, ly);

        // rendering...
        waitRetrace();
        putImage(0, 0, &dst);
        clearImage(&dst);
        cnt++;
    } while (!checkKeyPressed());

    // cleanup...
    freeImage(&dst);
}

void runPlasmaScale(int32_t sx, int32_t sy)
{
    uint8_t *data;
    GFX_IMAGE plasma, screen;

    uint8_t sinx[256];
    uint32_t frames, tectr, ofs, y;
    uint16_t x1, x2, x3, y1, y2, y3;
    uint16_t cr, cg, cb, endx, a, b, c;

    uint32_t edi;

    // initialized lookup table and preload image
    for (y = 0; y != 256; y++) sinx[y] = sin(y * M_PI / 128) * 127 + 128;
    if (!newImage(160, 120, &plasma)) fatalError("Cannot open image plasma.\n");; 
    if (!newImage(320, 240, &screen)) fatalError("Cannot open image screen.\n");; 

    frames  = 0;
    endx    = plasma.mWidth >> 1;
    data    = plasma.mData;

    do {
        edi = 0;
        ofs = 0;
        tectr = frames * 10;
        x1 = sinx[(tectr / 12) & 0xFF];
        x2 = sinx[(tectr / 11) & 0xFF];
        x3 = sinx[frames & 0xFF];
        y1 = sinx[((tectr >> 3) + 64) & 0xFF];
        y2 = sinx[(tectr / 7 + 64) & 0xFF];
        y3 = sinx[(tectr / 12 + 64) & 0xFF];

        // calculate plasma buffer
        for (y = 0; y != plasma.mHeight; y++)
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

        // bilinear scale plasma buffer
        bilinearScaleImage(&screen, &plasma);
        waitRetrace();
        putImage(sx, sy, &screen);
        frames++;
    } while (!checkKeyPressed());

    // clean up...
    freeImage(&plasma);
    freeImage(&screen);
}

int main()
{
    int32_t i, xc, yc;
    char sbuff[80] = {0};

    VBE_DRIVER_INFO	drv;
    GFX_IMAGE bg, tx, scr, old, im;
    uint32_t gray32, gray64, gray127, redcol;

    _clearscreen(0);
    printf("GFXLIB initializing....\n");
    initGfxLib(1);

    if (!loadFont("sysfont.xfn", 0)) fatalError("Cannot load system font!\n");
    if (!loadImage("gfxbg0.png", &bg)) fatalError("Cannot load image:gfxbg0.png!\n");
    if (!loadImage("gfxbump0.png", &bump1)) fatalError("Cannot load image:gfxbump0.png!\n");
    if (!loadImage("gfxbump1.png", &bump2)) fatalError("Cannot load image:gfxbump1.png!\n");
    if (!loadImage("gfxlogos.png", &logo)) fatalError("Cannot load image:gfxlogos.png!\n");
    if (!loadImage("gfxsky.png", &sky)) fatalError("Cannot load image:gfxsky.png!\n");
    for (i = 0; i != 16; i++)
    {
        sprintf(sbuff, "flare-%dx.png", i + 1);
        if (!loadImage(sbuff, &flares[i])) fatalError("Cannot load image: %s!\n", sbuff);
    }
    memset(&drv, 0, sizeof(VBE_DRIVER_INFO));
    getVesaDriverInfo(&drv);

    if (!setVesaMode(640, 480, 32, 85)) fatalError("Cannot init graphic mode.\n");

    runIntro();
    putImage(0, 0, &bg);
    putImageAlpha(cmaxX - logo.mWidth, 0, &logo);

    xc = centerX + 40;
    yc = centerY + 40;
    gray32 = fromRGB(32, 32, 32);
    gray64 = fromRGB(64, 64, 64);
    gray127 = fromRGB(127, 127, 127);
    redcol = fromRGB(192, 0, 0);

    fillRectPatternAdd(10, 10, xc - 10, yc - 10, gray32, ptnHatchX);
    fillRectSub(10, yc, xc - 10, yc + 190, gray64);
    fillRect(20, 20, xc - 20, yc - 20, 0);
    
    newImage(xc - 20, 190, &tx);
    getImage(10, yc, xc - 20, 190, &tx);
    
    writeString(xc + 6, 70, "GFXLIB v1.1.WC", gray127, 2);
    writeString(xc + 6, 90, "A short show of some abilities", gray127, 2);
    writeString(xc + 6, 100, "GFXLIB does provide. Note that", gray127, 2);
    writeString(xc + 6, 110, "this is only a small amount of", gray127, 2);
    writeString(xc + 6, 120, "all available features.", gray127, 2);
    sprintf(sbuff, "Card    : %c%c%c%c %x.%x", drv.VBESignature[0], drv.VBESignature[1], drv.VBESignature[2], drv.VBESignature[3], drv.VBEVersion >> 8, drv.VBEVersion & 0xFF);
    writeString(xc + 6, 140, sbuff, gray127, 2);
    sprintf(sbuff, "Memory  : %u KB", drv.TotalMemory << 6);
    writeString(xc + 6, 150, sbuff, gray127, 2);
    sprintf(sbuff, "Provider: %s", drv.OEMStringPtr);
    writeString(xc + 6, 160, sbuff, gray127, 2);
    sprintf(sbuff, "Version : %0.4X", drv.OemSoftwareRev);
    writeString(xc + 6, 170, sbuff, gray127, 2);
    sprintf(sbuff, "Vendor  : %s", drv.OemVendorNamePtr);
    writeString(xc + 6, 180, sbuff, gray127, 2);
    sprintf(sbuff, "Revision: %s", drv.OemProductRevPtr);
    writeString(xc + 6, 190, sbuff, gray127, 2);
    sprintf(sbuff, "Mode    : %dx%dx%db", lfbWidth, lfbHeight, bitsPerPixel);
    writeString(xc + 6, 200, sbuff, gray127, 2);
    sprintf(sbuff, "CPU manufacturer: %s", cpuVendor);
    writeString(xc + 6, 220, sbuff, gray127, 2);
    sprintf(sbuff, "CPU features    : %s", cpuFeatures);
    writeString(xc + 6, 230, sbuff, gray127, 2);
    sprintf(sbuff, "CPU clock rate  : %u MHz", cpuSpeed);
    writeString(xc + 6, 240, sbuff, gray127, 2);
    sprintf(sbuff, "Total memory    : %u KB", meminfo.LargestBlockAvail >> 10);
    writeString(xc + 6, 250, sbuff, gray127, 2);
    sprintf(sbuff, "Available memory: %u KB", meminfo.NumPhysicalPagesFree << 2);
    writeString(xc + 6, 260, sbuff, gray127, 2);
    if (cpuSpeed < 300 || !haveMMX || !haveSSE || !have3DNow)
    {
        writeString(xc + 6, 290, "WARNING: Your machine is very", redcol, 2);
        writeString(xc + 6, 300, "slow! this mean some features", redcol, 2);
        writeString(xc + 6, 310, "of GFXLIB maybe not works", redcol, 2);
        writeString(xc + 6, 320, "as espectation. I hope you run", redcol, 2);
        writeString(xc + 6, 330, "this demo on the faster machine!", redcol, 2);
    }
    fullSpeed = 1;
    showText(10, yc, &tx, "Please wait while loading images...");
    if (!loadImage("fade1x.png", &fade1)) fatalError("Cannot load image fade1x.png!\n");
    showText(10, yc, &tx, " - fade1x.png");
    if (!loadImage("fade2x.png", &fade2)) fatalError("Cannot load image fade2x.png!\n");
    showText(10, yc, &tx, " - fade2x.png");
    if (!loadImage("flare1x.png", &flare)) fatalError("Cannot load image flare1x.png!\n");
    showText(10, yc, &tx, " - flare1x.png");
    showText(10, yc, &tx, "");
    fullSpeed = 0;
    showText(10, yc, &tx, "This is an early demonstration of the");
    showText(10, yc, &tx, "abilities of GFXLIB.");
    showText(10, yc, &tx, "What you will see here are only some");
    showText(10, yc, &tx, "of the image manipulating effects which");
    showText(10, yc, &tx, "are currently built in. More to come");
    showText(10, yc, &tx, "and shown...");
    delay(1000);
    showText(10, yc, &tx, "Starting...");
    runBlocking(20, 20);
    fullSpeed = 0;
    showText(10, yc, &tx, "----");
    showText(10, yc, &tx, "What you saw was a combination of the");
    showText(10, yc, &tx, "command 'blockout' and the command");
    showText(10, yc, &tx, "brightnessImage. The text is an");
    showText(10, yc, &tx, "alphamapped image. You may see: working");
    showText(10, yc, &tx, "with images got very easy in GFXLIB -");
    showText(10, yc, &tx, "no half things anymore! To the next...");
    showText(10, yc, &tx, "Press enter to continue...");
    while(!checkKeyPressed());
    runAddImage(20, 20);
    fullSpeed = 0;
    showText(10, yc, &tx, "----");
    showText(10, yc, &tx, "This was simply another flag of a");
    showText(10, yc, &tx, "draw-operation. It was used here to");
    showText(10, yc, &tx, "force draw to add the content of the");
    showText(10, yc, &tx, "image to the background of the image.");
    showText(10, yc, &tx, "You are also able to subtract the image");
    showText(10, yc, &tx, "and to work with an alphamap like PNG-");
    showText(10, yc, &tx, "images can contain one.");
    showText(10, yc, &tx, "The next effect - press enter...");
    while(!checkKeyPressed());
    runCrossFade(20, 20);
    fullSpeed = 0;
    showText(10, yc, &tx, "----");
    showText(10, yc, &tx, "This thing is called cross-fading or");
    showText(10, yc, &tx, "alpha-blending. In GFXLIB, the");
    showText(10, yc, &tx, "procedure is called 'blendImage'. This");
    showText(10, yc, &tx, "procedure makes of 2 images another,");
    showText(10, yc, &tx, "where you can decide which image");
    showText(10, yc, &tx, "covers more the other.");
    showText(10, yc, &tx, "Enter...");
    while(!checkKeyPressed());
    runBilinearRotateImage(20, 20);
    fullSpeed = 0;
    showText(10, yc, &tx, "----");
    showText(10, yc, &tx, "This is an image rotation. The res-");
    showText(10, yc, &tx, "ponsible routine for this is called");
    showText(10, yc, &tx, "bilinearRotateImage. It doesn't seem");
    showText(10, yc, &tx, "to be very fast here, but in this demo");
    showText(10, yc, &tx, "the rotation is an optimize version of");
    showText(10, yc, &tx, "bilinear image interpolation. You can");
    showText(10, yc, &tx, "reach on a INTEL MMX-133 up to 20 fps");
    showText(10, yc, &tx, "at 640x480x32. You can see another");
    showText(10, yc, &tx, "version of rotate image is so fast if");
    showText(10, yc, &tx, "only rotate and show image, check my");
    showText(10, yc, &tx, "source code for details.");
    showText(10, yc, &tx, "Enter...");
    while(!checkKeyPressed());
    runScaleUpImage(20, 20);
    fullSpeed = 0;
    showText(10, yc, &tx, "----");
    showText(10, yc, &tx, "Much more fancy than the other FX...");
    showText(10, yc, &tx, "Yea, you see two effects combined here.");
    showText(10, yc, &tx, "scaleup and blur image are doing");
    showText(10, yc, &tx, "their work here. Check the source to");
    showText(10, yc, &tx, "see the details.");
    showText(10, yc, &tx, "Enter...");
    while(!checkKeyPressed());
    runAntialias(20, 20);
    fullSpeed = 0;
    showText(10, yc, &tx, "----");
    showText(10, yc, &tx, "Antialiased lines, circles and ellipses.");
    showText(10, yc, &tx, "Possible with GFXLIB and also even faster");
    showText(10, yc, &tx, "than seen here (just slow for show).");
    showText(10, yc, &tx, "Perfect for 3D models and similar.");
    showText(10, yc, &tx, "Enter...");
    while(!checkKeyPressed());
    runPlasmaScale(20, 20);
    fullSpeed = 0;
    showText(10, yc, &tx, "----");
    showText(10, yc, &tx, "Plasma effect with hight color, this also");
    showText(10, yc, &tx, "combine scale up image with bilinear");
    showText(10, yc, &tx, "interpolation to process hight quality.");
    showText(10, yc, &tx, "This version is optimized using integer");
    showText(10, yc, &tx, "number but not really fast here.");
    showText(10, yc, &tx, "Enter...");
    while(!checkKeyPressed());
    fillRect(20, 20, xc - 20, yc - 20, 0);
    newImage(lfbWidth, lfbHeight, &scr);
    getImage(0, 0, lfbWidth, lfbHeight, &scr);
    runBumpImage();
    newImage(lfbWidth, lfbHeight, &old);
    newImage(320, 240, &im);
    getImage(0, 0, lfbWidth, lfbHeight, &old);
    scaleImage(&im, &old, 0);
    putImage(0, 0, &scr);
    putImage(20, 20, &im);
    fullSpeed = 0;
    showText(10, yc, &tx, "----");
    showText(10, yc, &tx, "Bumbing effect with full screen, this");
    showText(10, yc, &tx, "effect combined many images and use");
    showText(10, yc, &tx, "subtract, adding pixel to calculate");
    showText(10, yc, &tx, "render buffer. Scale down image using");
    showText(10, yc, &tx, "Bresenham algorithm for faster speed");
    showText(10, yc, &tx, "of image interpolation.");
    showText(10, yc, &tx, "Enter for next...");
    while(!checkKeyPressed());
    runLens();
    getImage(0, 0, lfbWidth, lfbHeight, &old);
    bilinearScaleImage(&im, &old);
    putImage(0, 0, &scr);
    putImage(20, 20, &im);
    fullSpeed = 0;
    showText(10, yc, &tx, "----");
    showText(10, yc, &tx, "Yeah! Lens effect, this effect also");
    showText(10, yc, &tx, "combined many images too and other");
    showText(10, yc, &tx, "pixel manipulation such as substract,");
    showText(10, yc, &tx, "adding for each render buffer. This");
    showText(10, yc, &tx, "also use bilinear algorithm with hight");
    showText(10, yc, &tx, "quality for scale down image. Use mouse");
    showText(10, yc, &tx, "hardware interrupt to tracking event.");
    showText(10, yc, &tx, "Enter to continue...");
    while(!checkKeyPressed());
    fullSpeed = 0;
    showText(10, yc, &tx, "----");
    showText(10, yc, &tx, "That's all folks! More to come soon.");
    showText(10, yc, &tx, "In short time, that's enought. See");
    showText(10, yc, &tx, "from my source code for another stuffs.");
    showText(10, yc, &tx, "If there occured something which seems");
    showText(10, yc, &tx, "to be a bug or any suggestion, please");
    showText(10, yc, &tx, "contact me immediately. Thanks!");
    showText(10, yc, &tx, "");
    showText(10, yc, &tx, "Nguyen Ngoc Van -- pherosiden@gmail.com");
    showText(10, yc, &tx, "");
    showText(10, yc, &tx, "Enter to exit ;-)");
    while(!checkKeyPressed());
    runExit(0);
    freeImage(&bg);
    freeImage(&tx);
    freeImage(&scr);
    freeImage(&old);
    freeImage(&im);
    closeFont(0);
    closeVesaMode();
    printf("+-----------------------------------------------------+\n");
    printf("|    GFXLIB demo (c) 1998 - 2002 by Nguyen Ngoc Van   |\n");
    printf("|        Full support 8/15/16/24/32 bits color        |\n");
    printf("|          Support load/save BMP & PNG files          |\n");
    printf("|    Using Linear Frame Buffer for best performance   |\n");
    printf("|           Optimize code by using assember           |\n");
    printf("|      Use double buffering to performance render     |\n");
    printf("|           Code by: Nguyen Ngoc Van                  |\n");
    printf("|             Email: pherosiden@gmail.com             |\n");
    printf("|           Website: http://codedemo.net              |\n");
    printf("|         Reference: http://crossfire-designs.de      |\n");
    printf("+-----------------------------------------------------+\n");
    return 0;
}
