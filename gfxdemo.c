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

//max message lines
#define MAX_TEXT_LINE   23

//max message lenght
#define MAX_MSG_LEN     80

// string buffer
typedef char STRBUFF[MAX_MSG_LEN];

// show text intro message
int32_t fullSpeed = 0;
STRBUFF texts[MAX_TEXT_LINE] = {0};

GFX_IMAGE flares[16] = {0};
GFX_IMAGE bg, txt, scr, old, im;
GFX_IMAGE fade1, fade2, flare;
GFX_IMAGE bump1, bump2, logo, sky;

// inline optimize for multiple call
inline int32_t sqr(int32_t x)
{
    return x * x;
}

// check and exit program
void runExit()
{
    int32_t i;
    GFX_IMAGE im;

    // capture current screen buffer
    newImage(lfbWidth, lfbHeight, &im);
    getImage(0, 0, lfbWidth, lfbHeight, &im);

    // decrease rgb and push back to screen
    for (i = 0; i < 32; i++)
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
    freeImage(&bg);
    freeImage(&txt);
    freeImage(&scr);
    freeImage(&old);
    freeImage(&im);
    for (i = 0; i < 16; i++) freeImage(&flares[i]);
    closeVesaMode();

    // print intro message
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
    strcpy(texts[MAX_TEXT_LINE - 1], str);

    // don't delay each character
    if (fullSpeed)
    {
        waitRetrace();
        putImage(sx, sy, img);
        for (i = 0; i < MAX_TEXT_LINE; i++) writeString(sx + 10, sy + 10 + i * 10, texts[i], col, 2);
        keyPressed(27);
    }
    else
    {
        // show previous text
        for (y = 9; y >= 0; y--)
        {
            // fill original background
            waitRetrace();
            putImage(sx, sy, img);
            for (i = 0; i < MAX_TEXT_LINE - 1; i++) writeString(sx + 10, sy + 10 + i * 10 + y, texts[i], col, 2);
        }

        x = 0;
        len = strlen(str);
        col = fromRGB(255, 255, 255);

        // show current text with delay each character
        for (i = 0; i < len; i++)
        {
            msg[0] = str[i];
            writeString(sx + 10 + x, sy + 10 + 220, msg, col, 2);
            x += getFontWidth(msg);

            // check for delay and skip
            if (!fullSpeed) delay(45);
            if (keyPressed(27)) fullSpeed = 1;
        }
    }
}

void runIntro()
{
    const int32_t tw = 3;
    const int32_t tg = tw + 5;
    const int32_t tu = tg + 3;
    const int32_t to = tu + 5;
    const int32_t ts = to + 3;

    GFX_IMAGE ult, utb, trn, map;
    GFX_IMAGE scr, wcb, wci, gfx, gxb;
    
    int32_t i0, i1, i2;
    uint8_t mov = 0;
    uint8_t *buf1, *buf2;
    uint64_t tmstart, tmwait;

    // load image
    if (!loadImage("assets/gfxulti.png", &ult)) fatalError("runIntro: cannot load gfxulti.png file.\n");
    if (!loadImage("assets/gfxlogo.png", &gfx)) fatalError("runIntro: cannot load gfxlogo.png file.\n");
    if (!loadImage("assets/gfxwelm.png", &wcb)) fatalError("runIntro: cannot load gfxwelm.png file.\n");
    if (!loadImage("assets/map03.png", &map)) fatalError("runIntro: cannot load map03.png file.\n");

    // initialize buffer
    if (!newImage(lfbWidth, lfbHeight, &scr)) fatalError("runIntro: cannot open image.\n");
    if (!newImage(lfbWidth >> 1, lfbHeight >> 1, &trn)) fatalError("runIntro: cannot open image.\n");
    if (!newImage(wcb.mWidth, wcb.mHeight, &wci)) fatalError("runIntro: cannot open image.\n");
    if (!newImage(gfx.mWidth, gfx.mHeight, &gxb)) fatalError("runIntro: cannot open image.\n");
    if (!newImage(ult.mWidth, ult.mHeight, &utb)) fatalError("runIntro: cannot open image.\n");

    i0 = trn.mWidth * trn.mHeight;

    // initialize tunnel buffer
    buf1 = (uint8_t*)calloc(i0, 1);
    buf2 = (uint8_t*)calloc(i0, 1);
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
    } while (!keyPressed(27) && getElapsedTime(tmstart) / timeRes < ts);

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
    } while (dec > 0 && !keyPressed(27));

    // save current background
    width  = fade1.mWidth;
    height = fade1.mHeight;
    putImage(sx, sy, &fade1);
    freeImage(&im2);

    // load next step
    if (!loadImage("assets/gfxtext.png", &im1)) fatalError("runBlocking: cannot load PNG file.\n");
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
    } while (dec > 0 && !keyPressed(27));

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
    if (!newImage(lfbWidth >> 1, lfbHeight >> 1, &im1)) fatalError("runScaleUpImage: cannot open image.\n");
    if (!newImage(lfbWidth >> 1, lfbHeight >> 1, &im2)) fatalError("runScaleUpImage: cannot open image.\n");
    if (!loadImage("assets/gfxspr.png", &im3)) fatalError("runScaleUpImage: cannot open PNG file.\n");

    // setup lookup table
    tables = (int32_t*)calloc(im2.mWidth, sizeof(int32_t));
    if (!tables) fatalError("scaleUpImage: not enough memory for lookup tables.\n");
        
    do {
        // redirect to render buffer
        changeDrawBuffer(im1.mData, im1.mWidth, im1.mHeight);

        // put some random pixel and GFX message
        for (i = 0; i < 400; i++) putPixel(random(im1.mWidth - 4) + 2, random(im1.mHeight - 4) + 2, fromRGB(0, 255, 200));
        if (random(64) == 32) putImageAlpha((im1.mWidth - im3.mWidth) >> 1, (im1.mHeight - im3.mHeight) >> 1, &im3);

        // blur & scale buffer
        blurImage(&im1);
        scaleUpImage(&im2, &im1, tables, 7, 7);

        // restore to screen buffer to draw
        restoreDrawBuffer();
        waitRetrace();
        putImage(sx, sy, &im2);
        copyData(im1.mData, im2.mData, im2.mSize);
    } while (!keyPressed(27));

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
    } while (!keyPressed(27));

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
    } while (pos > 0 && !keyPressed(27));
}

void runRotateImage(int32_t sx, int32_t sy)
{
    GFX_IMAGE im;
    uint32_t degree = 0;
    int32_t *tables = NULL;	
    
    // initialize render buffer
    if (!newImage(fade2.mWidth, fade2.mHeight, &im)) fatalError("runRotateImage: cannot open image.\n");

    // pre-calculate lookup table
    tables = (int32_t*)calloc(2 * fade2.mWidth + fade2.mHeight + 2, sizeof(int32_t));
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
    } while (!keyPressed(27));

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
    } while (!keyPressed(27));

    // cleanup...
    freeImage(&im);
}

void runAntialias(int32_t sx, int32_t sy)
{
    uint32_t i, col;
    GFX_IMAGE img, dst;

    //save current midx, midy
    const int32_t midx = lfbWidth >> 1;
    const int32_t midy = lfbHeight >> 1;

    // initialize image buffer
    if (!newImage(midx, midy, &img)) fatalError("runAntialias: cannot allocate image.\n");
    if (!newImage(midx, midy, &dst)) fatalError("runAntialias: cannot allocate image.\n");

    do {
        // redirect render to image buffer
        changeDrawBuffer(dst.mData, dst.mWidth, dst.mHeight);

        // draw antialias (smooth pixel) circle, line, ellipse
        for (i = 0; i < 3; i++)
        {
            // choose random color
            col = fromRGB(random(255) + 1, random(255) + 1, random(255) + 1);

            // which shape to be drawed
            switch (random(3))
            {
            case 0: drawLineAlpha(random(midx), random(midy), random(midx), random(midy), col); break;
            case 1: drawCircleAlpha(random(midx), random(midy), random(midx) >> 2, col); break;
            case 2: drawEllipseAlpha(random(midx), random(midy), random(midx), random(midy), col); break;
            }
        }

        // restore and rendering...
        restoreDrawBuffer();
        waitRetrace();
        putImage(sx, sy, &dst);

        // fade-out current buffer
        fadeOutImage(&dst, 4);
    } while (!keyPressed(27));

    // cleanup...
    freeImage(&img);
    freeImage(&dst);
}

void runLensFlare()
{
    GFX_IMAGE scr;
    int32_t tx, ty, x, y, i;
    uint64_t time = 0, oldTime = 0, lapTime = 0;
    const int32_t flareput[16] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
    const int32_t flarepos[16] = {-1110, -666, 0, 1087, 1221, 1309, 1776, 2197, 2819, 3130, 3220, 3263, 3663, 3707, 4440, 5125};
    const char *str = "Drag your mouse to see details and left click to exit!";

    // initialize mouse driver and load source image
    if (!initMouse()) fatalError("runLensFlare: cannot init mouse.\n");
    if (!newImage(lfbWidth, lfbHeight, &scr)) fatalError("runLensFlare: cannot open image.\n");

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
    ty = (scr.mHeight - getFontHeight(str)) - 2;

    do {
        //timing for FPS counter
        oldTime = time;
        time = getCurrentTime();

        // redirect render to image buffer
        changeDrawBuffer(scr.mData, scr.mWidth, scr.mHeight);
        putImage(0, 0, &sky);
        fillRectSub(0, 0, lfbWidth, lfbHeight, fromRGB(0, (1.0 * mcd.mdx / cmaxY) * 64.0, (1.0 * mcd.mdx / cmaxY) * 64.0));

        // put all flare image to render buffer
        for (i = 0; i < 16; i++)
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
        putImageAlpha(lfbWidth - logo.mWidth, 1, &logo);
        drawTextImage(tx, ty, fromRGB(255, 255, 255), &scr, str);
        lapTime = ticksToMicroSec(time - oldTime);
        if (lapTime > 10000) drawTextImage(1, 1, fromRGB(255, 255, 255), &scr, "FPS: %.2f", 1000000.0 / lapTime);

        // restore render buffer to screen buffer
        restoreDrawBuffer();
        waitRetrace();
        putImage(0, 0, &scr);
    } while (!keyPressed(27) && !mcd.mbx);

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
        lx = cos(cnt / 13.0) * 133.0 + centerX;
        ly = sin(cnt / 23.0) * 133.0 + centerY;

        // start bumping buffer
        bumpImage(&dst, &bump1, &bump2, lx, ly);

        // rendering...
        waitRetrace();
        putImage(0, 0, &dst);
        clearImage(&dst);
        cnt++;
    } while (!keyPressed(27));

    // cleanup...
    freeImage(&dst);
}

void runPlasmaScale(int32_t sx, int32_t sy)
{
    uint32_t *data = NULL;
    GFX_IMAGE plasma, screen;

    uint8_t sinx[256] = {0};
    uint32_t endx, frames, tectr, y;
    uint16_t cr, cg, cb, a, b, c;
    uint16_t x1, x2, x3, y1, y2, y3;
    
    // initialized lookup table and preload image
    for (y = 0; y < 256; y++) sinx[y] = sin(y * M_PI / 128) * 127 + 128;
    if (!newImage(lfbWidth >> 2, lfbHeight >> 2, &plasma)) fatalError("Cannot open image plasma.\n");; 
    if (!newImage(lfbWidth >> 1, lfbHeight >> 1, &screen)) fatalError("Cannot open image screen.\n");; 

    frames  = 0;
    endx    = plasma.mWidth >> 1;

    do {
        data = (uint32_t*)plasma.mData;
        tectr = frames * 10;
        x1 = sinx[(tectr / 12) & 0xFF];
        x2 = sinx[(tectr / 11) & 0xFF];
        x3 = sinx[frames & 0xFF];
        y1 = sinx[((tectr >> 3) + 64) & 0xFF];
        y2 = sinx[((tectr / 7) + 64) & 0xFF];
        y3 = sinx[((tectr / 12) + 64) & 0xFF];

        // calculate plasma buffer
        for (y = 0; y < plasma.mHeight; y++)
        {
            a = sqr(y - y1) + sqr(x1);
            b = sqr(y - y2) + sqr(x2);
            c = sqr(y - y3) + sqr(x3);

            cr = sinx[(a >> 6) & 0xFF];
            cg = sinx[(b >> 6) & 0xFF];
            cb = sinx[(c >> 6) & 0xFF];
            
            __asm {
                mov    edi, data
                xor    eax, eax
                xor    edx, edx
            next:
                xor    ebx, ebx
                mov    cl, 6
                mov    ebx, eax
                push   eax
                sub    bx, x3
                add    bx, c
                mov    c, bx
                shr    bx, cl
                and    bx, 0x00FF
                mov    bl, sinx[ebx]
                mov    si, bx
                mov    bx, ax
                sub    bx, x2
                add    bx, b
                mov    b, bx
                shr    bx, cl
                and    bx, 0x00FF
                mov    dl, sinx[ebx]
                mov    bx, ax
                sub    bx, x1
                add    bx, a
                mov    a, bx
                shr    bx, cl
                and    bx, 0x00FF
                mov    bl, sinx[ebx]
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
                pop    eax
                inc    eax
                cmp    eax, endx
                jnae   next
            }
            data += plasma.mWidth;
        }

        // bilinear scale plasma buffer
        bilinearScaleImage(&screen, &plasma);
        waitRetrace();
        putImage(sx, sy, &screen);
        frames++;
    } while (!keyPressed(27));

    // clean up...
    freeImage(&plasma);
    freeImage(&screen);
}

int main()
{
    int32_t i, xc, yc;
    char sbuff[80] = {0};
    const int32_t tx = 10;
    const int32_t mx = 20;
    const int32_t my = 20;

    VBE_DRIVER_INFO	drv;
    uint32_t gray32, gray64, gray127, redcol;

    setTimerType(1);
    setQuitCallback(runExit);

    if (!loadFont("assets/sysfont.xfn", 0)) fatalError("Cannot load system font!\n");
    if (!loadImage("assets/gfxbg5.png", &bg)) fatalError("Cannot load image:gfxbg0.png!\n");
    if (!loadImage("assets/gfxbump0.png", &bump1)) fatalError("Cannot load image:gfxbump0.png!\n");
    if (!loadImage("assets/gfxbump1.png", &bump2)) fatalError("Cannot load image:gfxbump1.png!\n");
    if (!loadImage("assets/gfxlogos.png", &logo)) fatalError("Cannot load image:gfxlogos.png!\n");
    if (!loadImage("assets/gfxsky.png", &sky)) fatalError("Cannot load image:gfxsky.png!\n");
    for (i = 0; i < 16; i++)
    {
        sprintf(sbuff, "assets/flare%dx.png", i + 1);
        if (!loadImage(sbuff, &flares[i])) fatalError("Cannot load image: %s!\n", sbuff);
    }
    memset(&drv, 0, sizeof(VBE_DRIVER_INFO));
    getVesaDriverInfo(&drv);

    if (!setVesaMode(800, 600, 32, 0)) fatalError("Cannot init graphic mode.\n");

    runIntro();
    putImage(0, 0, &bg);
    putImageAlpha(lfbWidth - logo.mWidth, lfbHeight - logo.mHeight, &logo);

    xc = centerX + 40;
    yc = centerY + 40;
    gray32 = fromRGB(32, 32, 32);
    gray64 = fromRGB(64, 64, 64);
    gray127 = fromRGB(127, 127, 127);
    redcol = fromRGB(192, 0, 0);

    fillRectPatternAdd(tx, tx, xc - 19, yc - 19, gray32, ptnHatchX);
    fillRect(mx, my, xc - 39, yc - 39, 0);
    fillRectSub(tx, yc, xc - 19, cmaxY - yc - 9, gray32);

    newImage(xc - 19, cmaxY - yc - 9, &txt);
    getImage(tx, yc, xc - 19, cmaxY - yc - 9, &txt);
    
    writeString(xc + tx, 70, GFXLIB_VERSION_STRING, gray127, 2);
    writeString(xc + tx, 90, "A short show of some abilities", gray127, 2);
    writeString(xc + tx, 100, "GFXLIB does provide. Note that", gray127, 2);
    writeString(xc + tx, 110, "this is only a small amount of", gray127, 2);
    writeString(xc + tx, 120, "all available features.", gray127, 2);
    sprintf(sbuff, "Card    : %c%c%c%c %x.%x", drv.VBESignature[0], drv.VBESignature[1], drv.VBESignature[2], drv.VBESignature[3], drv.VBEVersion >> 8, drv.VBEVersion & 0xFF);
    writeString(xc + tx, 140, sbuff, gray127, 2);
    sprintf(sbuff, "Memory  : %u KB", drv.TotalMemory << 6);
    writeString(xc + tx, 150, sbuff, gray127, 2);
    sprintf(sbuff, "Provider: %s", drv.OEMStringPtr);
    writeString(xc + tx, 160, sbuff, gray127, 2);
    sprintf(sbuff, "Version : %0.4X", drv.OemSoftwareRev);
    writeString(xc + tx, 170, sbuff, gray127, 2);
    sprintf(sbuff, "Vendor  : %s", drv.OemVendorNamePtr);
    writeString(xc + tx, 180, sbuff, gray127, 2);
    sprintf(sbuff, "Revision: %s", drv.OemProductRevPtr);
    writeString(xc + tx, 190, sbuff, gray127, 2);
    sprintf(sbuff, "Mode    : %dx%dx%db", lfbWidth, lfbHeight, bitsPerPixel);
    writeString(xc + tx, 200, sbuff, gray127, 2);
    sprintf(sbuff, "CPU manufacturer: %s", cpuVendor);
    writeString(xc + tx, 220, sbuff, gray127, 2);
    sprintf(sbuff, "CPU features    : %s", cpuFeatures);
    writeString(xc + tx, 230, sbuff, gray127, 2);
    sprintf(sbuff, "CPU clock rate  : %u MHz", cpuSpeed);
    writeString(xc + tx, 240, sbuff, gray127, 2);
    sprintf(sbuff, "Total memory    : %u KB", meminfo.LargestBlockAvail >> 10);
    writeString(xc + tx, 250, sbuff, gray127, 2);
    sprintf(sbuff, "Available memory: %u KB", meminfo.NumPhysicalPagesFree << 2);
    writeString(xc + tx, 260, sbuff, gray127, 2);
    if (cpuSpeed < 100 || (!haveMMX && !have3DNow))
    {
        writeString(xc + tx, 290, "WARNING: Your machine is very slow! this", redcol, 2);
        writeString(xc + tx, 300, "mean some features of GFXLIB maybe not", redcol, 2);
        writeString(xc + tx, 310, "works as espectation. I hope you run this", redcol, 2);
        writeString(xc + tx, 320, "demo on the faster machine!", redcol, 2);
    }
    fullSpeed = 1;
    showText(tx, yc, &txt, "Please wait while loading images...");
    if (!loadImage("assets/fade1x.png", &fade1)) fatalError("Cannot load image fade1x.png!\n");
    showText(tx, yc, &txt, " - fade1x.png");
    if (!loadImage("assets/fade2x.png", &fade2)) fatalError("Cannot load image fade2x.png!\n");
    showText(tx, yc, &txt, " - fade2x.png");
    if (!loadImage("assets/flare0x.png", &flare)) fatalError("Cannot load image flare0x.png!\n");
    showText(tx, yc, &txt, " - flare0x.png");
    showText(tx, yc, &txt, "");
    fullSpeed = 0;
    showText(tx, yc, &txt, "This is an early demonstration of the abilities of");
    showText(tx, yc, &txt, "GFXLIB. What you'll see here are just a few of the");
    showText(tx, yc, &txt, "image manipulation effects that are currently");
    showText(tx, yc, &txt, "available. There will be more to show you later...");
    showText(tx, yc, &txt, "Starting...");
    delay(1000);
    runBlocking(mx, my);
    fullSpeed = 0;
    showText(tx, yc, &txt, "----");
    showText(tx, yc, &txt, "What you saw was a combination of the command");
    showText(tx, yc, &txt, "blockOut and the command brightnessImage. The text");
    showText(tx, yc, &txt, "is an alpha mapped image. You may see that working");
    showText(tx, yc, &txt, "with images has gotten very easy in GFXLIB-no");
    showText(tx, yc, &txt, "half-things anymore! Press the enter key!");
    while(!keyPressed(27));
    runAddImage(mx, my);
    fullSpeed = 0;
    showText(tx, yc, &txt, "----");
    showText(tx, yc, &txt, "This was simply another flag of a draw-operation.");
    showText(tx, yc, &txt, "It was used here to force draw to add the content");
    showText(tx, yc, &txt, "of the image to the background of the image. You");
    showText(tx, yc, &txt, "are also able to subtract the image and to work");
    showText(tx, yc, &txt, "with an alpha map like PNG-images can contain one.");
    showText(tx, yc, &txt, "The next effect - press enter...");
    while(!keyPressed(27));
    runCrossFade(mx, my);
    fullSpeed = 0;
    showText(tx, yc, &txt, "----");
    showText(tx, yc, &txt, "This thing is called crossFading or alphaBlending.");
    showText(tx, yc, &txt, "In GFXLIB, the procedure is called blendImage.");
    showText(tx, yc, &txt, "This procedure creates 2 images of one another,");
    showText(tx, yc, &txt, "where you can decide which image covers more of");
    showText(tx, yc, &txt, "the other. For the next, enter...");
    while(!keyPressed(27));
    runBilinearRotateImage(mx, my);
    fullSpeed = 0;
    showText(tx, yc, &txt, "----");
    showText(tx, yc, &txt, "This is an image rotation. The responsible routine");
    showText(tx, yc, &txt, "for this is called bilinearRotateImage. It doesn't");
    showText(tx, yc, &txt, "seem to be very fast here, but in this demo the");
    showText(tx, yc, &txt, "rotation is an optimize version of bilinear image");
    showText(tx, yc, &txt, "interpolation. You can reach on a INTEL MMX-133 up");
    showText(tx, yc, &txt, "to 45 fps at 800x600x32. You can see another ver-");
    showText(tx, yc, &txt, "sion of rotate image is so fast if only rotate and");
    showText(tx, yc, &txt, "show image, check my source code for details.");
    showText(tx, yc, &txt, "Press any key...");
    while(!keyPressed(27));
    runScaleUpImage(mx, my);
    fullSpeed = 0;
    showText(tx, yc, &txt, "----");
    showText(tx, yc, &txt, "Much fancier than the other FX...Yeah, you see");
    showText(tx, yc, &txt, "two effects combined here. Scales and blurred");
    showText(tx, yc, &txt, "image are doing their work here. Check the source");
    showText(tx, yc, &txt, "code to see the details. Press enter... ;)");
    while(!keyPressed(27));
    runAntialias(mx, my);
    fullSpeed = 0;
    showText(tx, yc, &txt, "----");
    showText(tx, yc, &txt, "Anti-aliased lines, circles and ellipses. Possible");
    showText(tx, yc, &txt, "with GFXLIB and also even faster than seen here");
    showText(tx, yc, &txt, "(just slow for show). Ideal for 3D models and the");
    showText(tx, yc, &txt, "like. Enter for the next...");
    while(!keyPressed(27));
    runPlasmaScale(mx, my);
    fullSpeed = 0;
    showText(tx, yc, &txt, "----");
    showText(tx, yc, &txt, "Plasma effect with high colors. This also combines");
    showText(tx, yc, &txt, "the scaled up image with bi-cubic interpolation to");
    showText(tx, yc, &txt, "process the image with the best quality. This ver-");
    showText(tx, yc, &txt, "sion is fully optimized by using a fixed number");
    showText(tx, yc, &txt, "and MMX instructions to maximize speed (extremely");
    showText(tx, yc, &txt, "fast). Enter for the next...");
    while(!keyPressed(27));
    fillRect(mx, my, xc - 39, yc - 39, 0);
    newImage(lfbWidth, lfbHeight, &scr);
    getImage(0, 0, lfbWidth, lfbHeight, &scr);
    runBumpImage();
    newImage(lfbWidth, lfbHeight, &old);
    newImage(lfbWidth >> 1, lfbHeight >> 1, &im);
    getImage(0, 0, lfbWidth, lfbHeight, &old);
    scaleImage(&im, &old, 0);
    putImage(0, 0, &scr);
    putImage(mx, my, &im);
    fullSpeed = 0;
    showText(tx, yc, &txt, "----");
    showText(tx, yc, &txt, "2D bump mapping effect with full screen, this");
    showText(tx, yc, &txt, "effect also combines many images and uses sub-");
    showText(tx, yc, &txt, "tracting and adding pixels to calculate the render");
    showText(tx, yc, &txt, "buffer. Scale the image using Bresenham algorithm");
    showText(tx, yc, &txt, "for quick image interpolation. Enter for the next.");
    while(!keyPressed(27));
    runLensFlare();
    getImage(0, 0, lfbWidth, lfbHeight, &old);
    bilinearScaleImage(&im, &old);
    putImage(0, 0, &scr);
    putImage(mx, my, &im);
    fullSpeed = 0;
    showText(tx, yc, &txt, "----");
    showText(tx, yc, &txt, "The lens flare effect, this effect is a simulation");
    showText(tx, yc, &txt, "of the lens flare in photo shop. It's combined");
    showText(tx, yc, &txt, "many images too, and other pixel manipulations");
    showText(tx, yc, &txt, "such as subtracting and adding for each to the");
    showText(tx, yc, &txt, "render buffer. This is also used for bi-cubic");
    showText(tx, yc, &txt, "interpolation with the best quality for scale.");
    showText(tx, yc, &txt, "Use hardware mouse tracking events. Enter...");
    while(!keyPressed(27));
    fullSpeed = 0;
    showText(tx, yc, &txt, "----");
    showText(tx, yc, &txt, "That's all, folks! More to come soon. In a short");
    showText(tx, yc, &txt, "time, that's enough. See my source code for other");
    showText(tx, yc, &txt, "stuff. If there is something which seems to be a");
    showText(tx, yc, &txt, "bug or any suggestion, please contact me at:");
    showText(tx, yc, &txt, "http://codedemo.net. Many thanks!");
    showText(tx, yc, &txt, "");
    showText(tx, yc, &txt, "Nguyen Ngoc Van -- pherosiden@gmail.com");
    showText(tx, yc, &txt, "");
    showText(tx, yc, &txt, "Enter to exit ;-)");
    while(!keyPressed(27));
    closeFont(0);
    runExit();
    return 0;
}
