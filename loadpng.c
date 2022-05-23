#include "gfxlib.c"

int main(int argc, char* argv[])
{
    int bits = 0;
    char *pos = NULL;

    if (argc < 3)
    {
        printf("Usage: %s [bitdepth] [filename.bmp]\n", argv[0]);
        return 1;
    }

    pos = strrchr(argv[2], '.');
    if (!pos)
    {
        printf("Specify file extension (.bmp or .png).\n");
        return 1;
    }

    bits = atoi(argv[1]);
    if (bits != 8 && bits != 15 && bits != 16 && bits != 24 && bits != 32)
    {
        printf("Error bit depth:%d\n", bits);
        return 0;
    }

    if (!setVesaMode(800, 600, bits, 0))
    {
        printf("ERROR INIT MODE (800x600x%db)\n", bits);
        return 0;
    }

    if (!strcmpi(pos, ".bmp"))
    {
        showBitmap(argv[2]);
        saveScreen("1.bmp");
    }
    else if (!strcmpi(pos, ".png"))
    {
        showPNG(argv[2]);
        saveScreen("1.png");
    }
    
    getch();
    closeVesaMode();

    return 1;
}
