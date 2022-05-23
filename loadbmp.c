/*---------------------------------------------------*/
/* Packet  : Demo & Effect                           */
/* Title   : Loading BMP file (640x480x24b)          */  
/* Author  : Nguyen Ngoc Van                         */
/* Memory  : Small                                   */  
/* Heaps   : 64K                                     */
/* Address : siden@codedemo.net                      */
/* Website : http://www.codedemo.net                 */
/* Created : 28/04/1998                              */
/* Please sent to me any bugs or suggests.           */  
/* You can use freely this code. Have fun :)         */
/*---------------------------------------------------*/
#include "svga.c"

typedef struct tagBMPHEADER
{
    uint16_t    bfType;            // must be 'BM' 
    uint32_t    bfSize;            // size of the whole .bmp file
    uint16_t    bfReserved1;       // must be 0
    uint16_t    bfReserved2;   	   // must be 0
    uint32_t    bfOffBits;         // offset to image data, bytes
    uint32_t    biSize;            // size of the structure
    int32_t     biWidth;           // image width
    int32_t     biHeight;          // image height
    uint16_t    biPlanes;          // number of colour planes
    uint16_t    biBitCount;        // bits per pixel
    uint32_t    biCompression;     // compression
    uint32_t    biSizeImage;       // size of the image in bytes
    int32_t     biXPelsPerMeter;   // pixels per meter X
    int32_t     biYPelsPerMeter;   // pixels per meter Y
    uint32_t    biClrUsed;         // colors used
    uint32_t    biClrImportant;    // important colors
} BMPHEADER;

void loadBMP(char *FileBMP)
{
    uint16_t x, y;
    RGBA *apal = NULL;
    RGB *pal = NULL;
    BMPHEADER bmp;
    FILE *fpBMP = NULL;

    if (!(fpBMP = fopen(FileBMP, "rb")))
    {
        closeGraph();
        printf("File %s does not exist", FileBMP);
        exit(1);
    }

    fread(&bmp, 1, sizeof(BMPHEADER), fpBMP);
    if (bmp.bfType != 0x4d42)
    {
        printf("Incorrect magic header!");
        exit(1);
    }
    
    if (bmp.biBitCount != 16 && bmp.biBitCount != 24 && bmp.biBitCount != 32)
    {
        printf("Incorrect bits per pixel format!");
        exit(1);
    }

    if (bmp.biBitCount == 32)
    {
        apal = (RGBA*)calloc(bmp.biWidth, sizeof(RGBA));
        if (!apal)
        {
            printf("Not enough memory to allocation");
            exit(1);
        }
    }
    else
    {
        pal = (RGB*)calloc(bmp.biWidth, sizeof(RGB));
        if (!pal)
        {
            printf("Not enough memory to allocation");
            exit(1);
        }
    }
    
    for (y = 0; y < bmp.biHeight; y++)
    {
        if (bmp.biBitCount == 32) fread(apal, sizeof(RGBA), bmp.biWidth, fpBMP);
        else fread(pal, sizeof(RGB), bmp.biWidth, fpBMP);
        
        for (x = 0; x < bmp.biWidth; x++)
        {
            if (bmp.biBitCount == 32) putPixel(x, bmp.biHeight - y, fromRGB(apal[x].b, apal[x].g, apal[x].r));
            else putPixel(x, bmp.biHeight - y, fromRGB(pal[x].b, pal[x].g, pal[x].r));
        }
    }
    
    fclose(fpBMP);
    if (apal) free(apal);
    if (pal) free(pal);
}

void main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Usage: loadBMP bmpFile");
        exit(1);
    }

    if (!initGraph(800, 600, 32))
    {
        printf("Error initialize VESA mode");
        exit(1);
    }
    
    loadBMP(argv[1]);
    while (!kbhit());
    closeGraph();
}
