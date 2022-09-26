/*-----------------------------------------*/
/*          ENCRYPTION MODULE              */
/*  Simple encoded/decoded text file       */
/* Compiler:    Open Watcom C/C++ 1.9      */
/* Environment: MS-DOS 16bits              */
/*-----------------------------------------*/

#include <stdio.h>
#include <stdint.h>
#include <conio.h>
#include <string.h>

int main(int argc, char *argv[])
{
    FILE *fpIn, *fpOut;
    int16_t c, key = 98;
    
    if (argc < 3)
    {
        fprintf(stderr, "Usage: %s <en|de> <input> <output>\n", argv[0]);
        return 0;
    }

    fpIn = fopen(argv[2], "rb");
    fpOut = fopen(argv[3], "wb");
    if (!fpIn || !fpOut)
    {
        fprintf(stderr, "File does not exist\n");
        return 0;
    }

    while ((c = fgetc(fpIn)) != EOF)
    {
        if (!strcmp(argv[1], "en")) c = c + ~key;    // decode/encode -/+
        else if (!strcmp(argv[1], "de")) c = c - ~key;
        fputc(c, fpOut);
    }

    fclose(fpIn);
    fclose(fpOut);

    if (!stricmp(argv[1], "en")) fprintf(stderr, "File: %s encoded.\n", argv[3]);
    else if (!stricmp(argv[1], "de")) fprintf(stderr, "File: %s decoded.\n", argv[3]);
    return 1;
}
