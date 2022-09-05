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

void loadFontDir(const char *path, const char *ext)
{
    DIR *dirp = NULL;
    struct dirent *dp = NULL;
    GFX_FONT *font = NULL;
    int16_t i, height, y = 10;
    char buff[256] = {0};
    
    // open font directory
    dirp = opendir(path);
    if (!dirp) return;

    // scan all fonts file
    while ((dp = readdir(dirp)))
    {
        // skip if not regular files, not GFX font files
        if (!strstr(dp->d_name, ext)) continue;

        // format related path
        sprintf(buff, "%s/%s", path, dp->d_name);

        if (!loadFont(buff, 0)) fatalError("Cannot load font: %s\n", dp->d_name);
        sprintf(buff, "%s - The quick brown fox jumps over the lazy dog", dp->d_name);

        // indexed current font
        font = getFont(0);

        // view all size of font
        for (i = 0; i <= font->subFonts; i++)
        {
            if (font->subFonts > 0) setFontSize(i);
            height = getFontHeight(buff);
            
            // have limit line
            if (y > cmaxY - height)
            {
                if (getch() == 27)
                {
                    closedir(dirp);
                    closeVesaMode();
                    exit(1);
                }
                clearScreen(0);
                y = 10;
            }

            // draw font
            writeString(10, y, buff, fromRGB(255, 255, 255), 0);
            horizLine(0, y + height + 1, cmaxX, fromRGB(0, 0, 255));
            y += (height + 3);
        }
        closeFont(0);
    }
    closedir(dirp);
}

int main()
{
    if (!setVesaMode(800, 600, 32, 85)) return 0;
    loadFontDir("assets", ".XFN");
    closeVesaMode();
    return 0;
}
