/*---------------------------------------------------*/
/* Packet  : Demo & Effect                           */
/* Title   : Loading CEL file (640x480x256b)         */
/* Author  : Nguyen Ngoc Van                         */
/* Memory  : Small                                   */
/* Heaps   : 64K                                     */
/* Address : siden@codedemo.net                      */
/* Website : http://www.codedemo.net                 */
/* Created : 28/04/1998                              */
/* Please sent to me any bugs or suggests.           */
/* You can use freely this code. Have fun :)         */
/*---------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include "svga.c"

RGB Pal[768] = {0};

void loadCEL(char *fileName, uint16_t width, uint16_t height)
{
    int16_t i, j;
    FILE *fp;
    uint8_t *buffer;

    buffer = (uint8_t*)calloc(width, 1);
    if (!buffer) exit(1);

    fp = fopen(fileName, "rb");
    if (!fp) exit(1);

    fseek(fp, 32, SEEK_SET);
    fread(Pal, 768, 1, fp);

    outp(0x03C8, 0);
    for(i = 0; i <= 255; i++)
    {
        outp(0x3C9, Pal[i].r);
        outp(0x3C9, Pal[i].g);
        outp(0x3C9, Pal[i].b);
    }

    for (i = 0; i < height; i++)
    {
        fread(buffer, width, 1, fp);
        for (j = 0; j < width; j++) putPixel(j, i, buffer[j]);
    }

    fclose(fp);
    free(buffer);
}

void main()
{
    initGraph(800, 600, 8);
    loadCEL("lba2.cel", 800, 600);
    while (inp(0x60) != 1);
    closeGraph();
}
