#include <dos.h>
#include <io.h>
#include <stdio.h>
#include <ctype.h>
#include <conio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define OFFSET(x, y)    (((x - 1) + 80 * (y - 1)) << 1)

#define PGDWN   81
#define PGUP    73
#define DOWN    80
#define HOME    71
#define END     79
#define UP      72
#define ESC     27

char **szLines = NULL;      // Text message
uint16_t numLines = 0;      // Message count
uint8_t far *txtMem = (uint8_t far*)0xB8000000L;

/*---------------------------------------------------*/
/* Function : scrollText                             */
/* Mission  : scrollText a line on the monitor       */
/* Expects  : (num) Number lines needs scroll        */
/*            (left,top,right,bot) The coordinate    */
/*            (type) scroll direction (up or down)   */
/* Returns  : Nothing                                */
/*---------------------------------------------------*/
void scrollText(uint8_t num, uint8_t color, uint8_t left, uint8_t top, uint8_t right, uint8_t bot, uint8_t type)
{
    union REGS regs;
    regs.h.ah = type;
    regs.h.al = num;
    regs.h.bh = color;
    regs.h.cl = left;
    regs.h.ch = top;
    regs.h.dl = right;
    regs.h.dh = bot;
    int86(0x10, &regs, &regs);
}

/*----------------------------------------*/
/* Funtion : setBorder                    */
/* Purpose : Set the text border color    */
/* Expects : (color) color of border      */
/* Returns : Nothing                      */
/*----------------------------------------*/
void setBorder(uint8_t color)
{
    union REGS regs;
    regs.h.ah = 0x10;
    regs.h.al = 0x01;
    regs.h.bh = color & 63;
    int86(0x10, &regs, &regs);
}

/*-------------------------------------*/
/* Funtion : setCursorSize             */
/* Mission : Resize the cursor         */
/* Expects : (size) The size of cursor */
/* Returns : Nothing                   */
/*-------------------------------------*/
void setCursorSize(uint16_t size)
{
    union REGS regs;
    regs.h.ah = 0x01;
    regs.x.cx = size;
    int86(0x10, &regs, &regs);
}

/*-------------------------------------*/
/* Funtion : setCursorPos              */
/* Mission : Set cursor position       */
/* Expects : (size) The size of cursor */
/* Returns : Nothing                   */
/*-------------------------------------*/
void setCursorPos(uint8_t x, uint8_t y)
{
    union REGS regs;
    regs.h.ah = 0x02;
    regs.h.bh = 0;
    regs.h.dl = x - 1;
    regs.h.dh = y - 1;
    int86(0x10, &regs, &regs);
}

/*------------------------------------------------*/
/* Funtion : writeChar                            */
/* Purpose : Writting a character with attribute  */
/* Expects : (x,y) cordinate to write a character */
/*           (wAttr) attribute of character       */
/*           (bLen) length area                   */
/*           (Chr) symbol needs to write          */
/* Returns : Nothing                              */
/*------------------------------------------------*/
void writeChar(uint8_t x, uint8_t y, uint8_t attr, uint8_t len, char chr)
{
    uint8_t i;
    const uint16_t txt = (attr << 8) + chr;
    uint16_t far *pmem = (uint16_t far*)(txtMem + OFFSET(x, y));
    for (i = 0; i < len; i++) *pmem++ = txt;
}

/*-----------------------------------------------*/
/* Function : writeVRM                           */
/* Purpose  : Writing a character with attribute */
/* Notices  : Intervention in video memory       */
/* Expects  : (x,y) cordinate needs to writting  */
/*            (txtAttr) The attribute of string  */
/*            (szPrmt) the string to format      */
/*            (fstAttr) The attr of first letter */
/* Returns : Nothing                             */
/*-----------------------------------------------*/
void writeVRM(uint8_t x, uint8_t y, uint8_t attr, const char *msg)
{
    uint16_t far *pmem = (uint16_t far*)(txtMem + OFFSET(x, y));
    while (*msg) *pmem++ = (attr << 8) + *msg++;
}

/*-----------------------------------------------*/
/* Function : setBlinking                        */
/* Purpose  : Redefine bits 7 in the text attrib */
/* Expects  : (doblink) = 0, back color is light */
/*                      = 1, text is blinking    */
/* Return   : Nothing                            */
/*-----------------------------------------------*/
void setBlinking(uint8_t doblink)
{
    union REGS regs;
    regs.h.ah = 0x10;
    regs.h.al = 0x03;
    regs.h.bl = doblink ? 1 : 0;
    int86(0x10, &regs, &regs);
}

/*--------------------------------------------------*/
/* Funtion : fillFrame                              */
/* Purpose : To full the box with special character */
/* Expects : (x1,y1) cordinate top to left          */
/*           (x2,y2) cordinate bottom to right      */
/*           (attr) special character color         */
/*           (chr) special character                */
/* Returns : Nothing                                */
/*--------------------------------------------------*/
void fillFrame(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t attr, char chr)
{
    uint8_t y;
    for (y = y1; y <= y2; y++) writeChar(x1, y, attr, x2 - x1 + 1, chr);
}

/*----------------------------------------------*/
/* Function : clearScreen                       */
/* Purpose  : clearScreen the part of window    */
/* Expects  : (x1,y1) cordinate top to left     */
/*            (x2,y2) cordinate bottom to right */
/*            (color) color needs clear         */
/* Returns  : Nothing                           */
/*----------------------------------------------*/
void clearScreen(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t color)
{
    fillFrame(x1, y1, x2, y2, color << 4, 32);
}

/*-----------------------------------------*/
/* Funtion : strPos                        */
/* Purpose : Getting position of substring */
/* Expects : (str) The string main         */
/*           (substr) The substring        */
/* Returns : Position of substring         */
/*-----------------------------------------*/
int16_t strPos(char *str, char *sub)
{
    char *ptr = strstr(str, sub);
    if (!ptr) return -1;
    return ptr - str;
}

/*---------------------------------------------*/
/* Funtion : insertChar                        */
/* Purpose : Inserted the char into string     */
/* Expects : (str) The string                  */
/*           (chr) The character need inserted */
/*           (pos) The position inserted      */
/* Returns : Nothing                           */
/*---------------------------------------------*/
void insertChar(char *str, char chr, int16_t pos)
{
    if (pos < 0 || pos >= strlen(str)) return;
    *(str + pos) = chr;
}

/*---------------------------------------------*/
/* Funtion : strDelete                         */
/* Purpose : Deleting the characters           */
/* Expects : (str) The string main             */
/*           (i) position to delete            */
/*           (numchar) the number of character */
/* Returns : Nothing                           */
/*---------------------------------------------*/
void strDelete(char *str, int16_t i, int16_t num)
{
    if (i < 0 || i >= strlen(str)) return;
    memcpy(str + i + 1, str + i +num, strlen(str) - i - 1);
}

/*--------------------------------------*/
/* Funtion : schRepl                    */
/* Purpose : Concat the string          */
/* Expects : (str) The string main      */
/*           (sch) The substring        */
/*           (repl) The char to replace */
/* Returns : Nothing                    */
/*--------------------------------------*/
void schRepl(char *str, char *sch, char repl)
{
    int16_t i;
    do {
        i = strPos(str, sch);
        if (i >= 0)
        {
            strDelete(str, i, strlen(sch));
            insertChar(str, repl, i);
        }
    } while (i >= 0);
}

/*----------------------------------------------*/
/* Funtion : chr2Str                            */
/* Purpose : Conver the char to the string      */
/* Expects : (chr, n, buff) Characters convert  */
/* Returns : The pointer to string              */
/*----------------------------------------------*/
void chr2Str(char chr, char n, char *str)
{
    str[0] = chr;
    str[1] = n;
    str[2] = '\0';
}

/*--------------------------------------*/
/* Funtion : fontVNI                    */
/* Purpose : Decode font to Vietnamese  */
/* Expects : (str) The string to decode */
/* Returns : Nothing                    */
/*--------------------------------------*/
void fontVNI(char *str)
{
    char buff[4] = {0};
    schRepl(str, "a8", 128);
    chr2Str(128, '1', buff);
    schRepl(str, buff, 129);
    chr2Str(128, '2', buff);
    schRepl(str, buff, 130);
    chr2Str(128, '3', buff);
    schRepl(str, buff, 131);
    chr2Str(128, '4', buff);
    schRepl(str, buff, 132);
    chr2Str(128, '5', buff);
    schRepl(str, buff, 133);
    schRepl(str, "a6", 134);
    chr2Str(134, '1', buff);
    schRepl(str, buff, 135);
    chr2Str(134, '2', buff);
    schRepl(str, buff, 136);
    chr2Str(134, '3', buff);
    schRepl(str, buff, 137);
    chr2Str(134, '4', buff);
    schRepl(str, buff, 138);
    chr2Str(134, '5', buff);
    schRepl(str, buff, 139);
    schRepl(str, "e6", 140);
    chr2Str(140, '1', buff);
    schRepl(str, buff, 141);
    chr2Str(140, '2', buff);
    schRepl(str, buff, 142);
    chr2Str(140, '3', buff);
    schRepl(str, buff, 143);
    chr2Str(140, '4', buff);
    schRepl(str, buff, 144);
    chr2Str(140, '5', buff);
    schRepl(str, buff, 145);
    schRepl(str, "o7", 146);
    chr2Str(146, '1', buff);
    schRepl(str, buff, 147);
    chr2Str(146, '2', buff);
    schRepl(str, buff, 148);
    chr2Str(146, '3', buff);
    schRepl(str, buff, 149);
    chr2Str(146, '4', buff);
    schRepl(str, buff, 150);
    chr2Str(146, '5', buff);
    schRepl(str, buff, 151);
    schRepl(str, "o6", 152);
    chr2Str(152, '1', buff);
    schRepl(str, buff, 153);
    chr2Str(152, '2', buff);
    schRepl(str, buff, 154);
    chr2Str(152, '3', buff);
    schRepl(str, buff, 155);
    chr2Str(152, '4', buff);
    schRepl(str, buff, 156);
    chr2Str(152, '5', buff);
    schRepl(str, buff, 157);
    schRepl(str, "u7", 158);
    chr2Str(158, '1', buff);
    schRepl(str, buff, 159);
    chr2Str(158, '2', buff);
    schRepl(str, buff, 160);
    chr2Str(158, '3', buff);
    schRepl(str, buff, 161);
    chr2Str(158, '4', buff);
    schRepl(str, buff, 162);
    chr2Str(158, '5', buff);
    schRepl(str, buff, 163);
    schRepl(str, "a1", 164);
    schRepl(str, "a2", 165);
    schRepl(str, "a3", 166);
    schRepl(str, "a4", 167);
    schRepl(str, "a5", 168);
    schRepl(str, "e1", 169);
    schRepl(str, "e2", 170);
    schRepl(str, "e3", 171);
    schRepl(str, "e4", 172);
    schRepl(str, "e5", 173);
    schRepl(str, "i1", 174);
    schRepl(str, "i2", 175);
    schRepl(str, "i3", 181);
    schRepl(str, "i4", 182);
    schRepl(str, "i5", 183);
    schRepl(str, "o1", 184);
    schRepl(str, "o2", 190);
    schRepl(str, "o3", 198);
    schRepl(str, "o4", 199);
    schRepl(str, "o5", 208);
    schRepl(str, "u1", 210);
    schRepl(str, "u2", 211);
    schRepl(str, "u3", 212);
    schRepl(str, "u4", 213);
    schRepl(str, "u5", 214);
    schRepl(str, "y1", 215);
    schRepl(str, "y2", 216);
    schRepl(str, "y3", 221);
    schRepl(str, "y4", 222);
    schRepl(str, "y5", 248);
    schRepl(str, "d9", 249);
    schRepl(str, "D9", 250);
}

/*------------------------------------*/
/* Funtion : decodeFile               */
/* Purpose : Decode file register.sys */
/* Expects : (ifile) The source file  */
/*           (ofile) The dest file  */
/* Returns : Number of lines in file  */
/*------------------------------------*/
uint16_t decodeFile(const char *ifile, const char *ofile)
{
    int16_t c, key = 98;
    uint16_t linesCount = 0;
    FILE *inHandle, *outHandle;
    
    numLines = 0;
    inHandle = fopen(ifile, "rb");
    outHandle = fopen(ofile, "wb");

    if (!inHandle || !outHandle)
    {
        fprintf(stderr, "Error loading file %s. System halt.", ifile);
        exit(1);
    }

    while ((c = fgetc(inHandle)) != EOF)
    {
        c = c - ~key;
        fputc(c, outHandle);
        if (c == 266) linesCount++;
    }

    fclose(inHandle);
    fclose(outHandle);
    return linesCount;
}

/*------------------------------------------------*/
/* Function : getTextFile                         */
/* Purpose  : Reading information into data array */
/* Expects  : (ifile) the input file              */
/*            (ofile) the output file             */
/*            (szData) the array data             */
/*            (wNumElm) the number of elements    */
/* Returns  : Nothing                             */
/*------------------------------------------------*/
void getTextFile(const char *ifile, const char *ofile)
{
    FILE *fp;
    char szBuffer[102];

    numLines = decodeFile(ifile, ofile);
    szLines = (char**)malloc(numLines * sizeof(char*));
    if (!szLines)
    {
        fprintf(stderr, "Not enough memory: %d\n", numLines);
        exit(1);
    }

    numLines = 0;
    fp = fopen(ofile, "rt");
    while (fgets(szBuffer, 102, fp))
    {
        fontVNI(szBuffer);
        szLines[numLines] = (char*)malloc(strlen(szBuffer) + 1);
        if (!szLines)
        {
            fprintf(stderr, "Not enough memory at line: %u", numLines);
            exit(1);
        }
        szBuffer[strlen(szBuffer) - 1] = '\0';
        strcpy(szLines[numLines], szBuffer);
        numLines++;
    }

    fclose(fp);
    unlink(ofile);
}

/*------------------------------------------*/
/* Function : releaseData                   */
/* Purpose  : free block memory of the data */
/* Expects  : Nothing                       */
/* Returns  : Nothing                       */
/*------------------------------------------*/
void releaseData()
{
    int16_t i = 0;
    for (i = 0; i < numLines; i++) free(szLines[i]);
    free(szLines);
}

/*---------------------------------------------*/
/* Function : ViewFile                         */
/* Mission  : Display the information on scren */
/* Expects  : (fileName) The name of file      */
/* Returns  : Nothing                          */
/*---------------------------------------------*/
void viewFile(char *fileName)
{
    char key = 0;
    char szDate[12];

    char *szTitle[] = {
        "Ta65p tin README",
        "Du2ng:    - PgUp - PgDwn - Home - End d9e63 d9ie62u khie63n. ESC d9e63 thoa1t",
        "Nga2y ca65p nha65t",
        "Ca65p nha65t"
    };

    struct dostime_t tm;
    struct dosdate_t da;
    uint16_t currLine = 0, i = 0, j = 0, k = 0;
    
    for (i = 0; i < sizeof(szTitle) / sizeof(szTitle[0]); i++) fontVNI(szTitle[i]);

    getTextFile(fileName, "readme.$$$");
    clearScreen(1, 1, 80, 25, 3);
    setCursorSize(0x2020);
    setBorder(43);
    setBlinking(0);
    writeChar(1, 1, 0x4E, 80, 32);
    writeChar(1, 25, 0x4E, 80, 32);
    writeVRM(31, 1, 0x4F, szTitle[0]);
    writeChar(27, 1, 0x4A, 3, 3);
    writeChar(46, 1, 0x4A, 3, 3);
    writeVRM(2, 25, 0x4F, szTitle[1]);
    writeChar(8, 25, 0x4F, 1, 24);
    writeChar(9, 25, 0x4F, 1, 25);
    
    _dos_getdate(&da);
    sprintf(szDate, "%s: %.2d/%.2d/%d", szTitle[3], da.day, da.month, da.year);
    writeVRM(2, 1, 0x4E, szDate);

    for (i = 0; i < 23; i++) writeVRM(1, 2 + i, 0x30, szLines[i]);
    
    currLine = 23;

    do {
        if (kbhit())
        {
            key = getch();
            if (!key) key = getch();
            switch (key)
            {
            case UP:
                if (currLine > 23)
                {
                    currLine--;
                    scrollText(1, 0x30, 0, 1, 79, 23, 7);
                    writeVRM(1, 2, 0x30, szLines[currLine - 23]);
                }
                break;
            case DOWN:
                if (currLine < numLines)
                {
                    scrollText(1, 0x30, 0, 1, 79, 23, 6);
                    writeVRM(1, 24, 0x30, szLines[currLine++]);
                }
                break;
            case PGDWN:
                if (currLine < numLines)
                {
                    if (numLines - currLine >= 23)
                    {
                        scrollText(23, 0x30, 0, 1, 79, 23, 6);
                        for (i = 0; i < 23; i++) writeVRM(1, 2 + i, 0x30, szLines[currLine++]);
                    }
                    else
                    {
                        scrollText(numLines - currLine, 0x30, 0, 1, 79, 23, 6);
                        k = 23 - (numLines - currLine) + 2;
                        j = currLine;
                        for (i = 0; i < numLines - j; i++) writeVRM(1, k + i, 0x30, szLines[currLine++]);
                    }
                }
                break;
            case PGUP:
                if (currLine > 23)
                {
                    if (currLine <= 46)
                    {
                        scrollText(currLine - 23, 0x30, 0, 1, 79, 23, 7);
                        for (i = 0; i < currLine - 23; i++) writeVRM(1, 2 + i, 0x30, szLines[i]);
                        currLine = 23;
                    }
                    else
                    {
                        scrollText(23, 0x30, 0, 1, 79, 23, 7);
                        for (i = currLine - 46; i < currLine - 23; i++) writeVRM(1, 2 + (i - currLine + 46), 0x30, szLines[i]);
                        currLine -= 23;
                    }
                }
                break;
            case END:
                if (currLine != numLines)
                {
                    currLine = numLines - 23;
                    scrollText(23, 0x30, 0, 1, 79, 23, 6);
                    for (i = 0; i < 23; i++) writeVRM(1, 2 + i, 0x30, szLines[currLine++]);
                }
                break;
            case HOME:
                if (currLine != 23)
                {
                    currLine = 0;
                    scrollText(23, 0x30, 0, 1, 79, 23, 7);
                    for (i = 0; i < 23; i++) writeVRM(1, 2 + i, 0x30, szLines[currLine++]);
                }
            }
        }
        
        _dos_gettime(&tm);
        sprintf(szDate, "%.2d:%.2d:%.2d", tm.hour, tm.minute, tm.second);
        writeVRM(72, 1, 0x4E, szDate);
    } while (key != ESC);
    releaseData();
}

void main()
{
    system("font on");
    viewFile("readme.hlp");
    setCursorSize(0x0607);
    system("font off");
    system("cls");
}
