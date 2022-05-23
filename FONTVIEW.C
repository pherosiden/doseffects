/*----------------------------------------------------*/
/* Packet  : Demo & Effect                            */ 
/* Effect  : Fonts Viewer                             */
/* Author  : Nguyen Ngoc Van                          */
/* Memory  : Small                                    */ 
/* Address : pherosiden@gmail.com                     */ 
/* Website : http://www.codedemo.net                  */
/* Created : 25/05/2000                               */
/* Please sent to me any bugs or suggests.            */ 
/* You can use freely this code. Have fun :)          */
/*----------------------------------------------------*/

#include "gfxlib.c"
#include <direct.h>

void loadFontDir(const char *ext)
{
    DIR *dirp;
    struct dirent *dp;
    int16_t i, height, y = 10;
    char text[256] = {0};
    GFX_FONT *font = NULL;

    // open font directory
    dirp = opendir(ext);
    if (!dirp) return;

    // scan all fonts file
    while ((dp = readdir(dirp)))
    {
        if (!loadFont(dp->d_name, 0)) fatalError("Cannot load font: %s\n", dp->d_name);
        sprintf(text, "%s - The quick brown fox jumps over the lazy dog", dp->d_name);

        // indexed current font
        font = getFont(fontType);

        // view all size of font
        for (i = 0; i <= font->subFonts; i++)
        {
            if (font->subFonts > 0) setFontSize(i);
            height = getFontHeight(text);
            
            // have limit line
            if (y > cmaxY - height)
            {
                getch();
                clearScreen(0);
                y = 10;
            }

            // draw font
            writeString(10, y, text, fromRGB(255, 255, 255), 0);
            horizLine(0, y + height + 1, cmaxX, fromRGB(0, 0, 255));
            y += (height + 3);
        }
        closeFont(0);
    }
    closedir(dirp);
    getch();
}

int main()
{
    initGfxLib(1);
    if (!setVesaMode(800, 600, 32, 85)) return 0;
    loadFontDir("*.XFN");
    closeVesaMode();
    return 0;
}
