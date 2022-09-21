#include <dos.h>
#include <io.h>
#include <stdio.h>
#include <ctype.h>
#include <conio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define MASK_BG         0x08
#define OFFSET(x, y)    (((x - 1) + 80 * (y - 1)) << 1)

#define LEFT    75
#define RIGHT   77
#define ENTER   13

#define wATV    0xF0
#define wFLT    0xFC
#define _wATV   0x78
#define _wFLT   0x74

typedef struct {
    uint8_t     day;        // The date of the program
    uint8_t     month;      // The month of the program
    uint16_t    year;       // The year of the program
    uint8_t     regs;       // The register code
    uint8_t     key;        // Random key
    char        serial[20]; // License code
    char        user[31];   // User name
    char        path[33];   // The installation path
    char        magic[33];  // Random characters
} REG_INFO;

uint8_t bmAvalid = 0;       // Status of the mouse
char **sysInfo = NULL;      // Text message
uint16_t sysNum = 0;        // Message count
uint8_t far *txtMem = (uint8_t far*)0xB8000000L;

/*-----------------------------------*/
/* Funtion : setBorder               */
/* Purpose : Setting border color    */
/* Expects : (color) color of border */
/* Returns : Nothing                 */
/*-----------------------------------*/
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

/*-----------------------------------------------*/
/* Funtion : printChar                           */
/* Purpose : Write a character to cordinate x, y */
/* Expects : (x,y) cordinate to write            */
/*           (chr) character to write            */
/* Returns : Nothing                             */
/*-----------------------------------------------*/
void printChar(uint8_t x, uint8_t y, uint8_t attr, char chr)
{
    uint16_t far *pmem = (uint16_t far*)(txtMem + OFFSET(x, y));
    *pmem = (attr << 8) + chr;
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
void writeVRM(uint8_t x, uint8_t y, uint8_t txtAtr, const char *szPrmt, uint8_t fstAttr)
{
    uint16_t far *pmem = (uint16_t far*)(txtMem + OFFSET(x, y));

    if (fstAttr)
    {
        char *ptmp = NULL;
        char szTmp[80] = {0};
        uint8_t i = 0, fltStop = 0, currX = x, bPos;

        strcpy(szTmp, szPrmt);
        for (i = 0; (i < strlen(szTmp) - 1) && !fltStop; i++)
        {
            if (szTmp[i] == 126) fltStop = 1;
        }

        memmove(&szTmp[i - 1], &szTmp[i], strlen(szTmp) - i + 1);
        bPos = i - 1;

        ptmp = szTmp;
        while (*ptmp) *pmem++ = (txtAtr << 8) + *ptmp++;
        printChar(currX + bPos, y, fstAttr, szTmp[bPos]);
    }
    else
    {
        while (*szPrmt) *pmem++ = (txtAtr << 8) + *szPrmt++;
    }
}

/*-----------------------------------------------*/
/* Function : drawButton                         */
/* Purpose  : Define the button shadow           */
/* Expects  : (x,y) cordinate needs to writting  */
/*            (txtAttr) the attribute of a title */
/*            (szTitle) the string to format     */
/*            (bType) The type of button         */
/*            (fstAttr) The attr of first letter */
/* Returns : Nothing                             */
/*-----------------------------------------------*/
void drawButton(uint8_t x, uint8_t y, uint8_t txtAttr, uint8_t bkClr, const char *szTitle, uint8_t bType, uint8_t fstAttr)
{
    const uint8_t wAttr = bkClr << 4;
    const uint16_t bLen = strlen(szTitle);
    const char styles[] = {16, 17, 223, 220};

    if (bType)
    {
        if (fstAttr)
        {
            writeVRM(x, y, txtAttr, szTitle, fstAttr);
            writeChar(x + 1, y + 1, wAttr, bLen - 1, styles[2]);
            writeChar(x + bLen - 1, y, wAttr, 1, styles[3]);
        }
        else
        {
            writeVRM(x, y, txtAttr, szTitle, 0);
            writeChar(x + 1, y + 1, wAttr, bLen, styles[2]);
            writeChar(x + bLen, y, wAttr, 1, styles[3]);
        }
    }
    else
    {
        if (fstAttr)
        {
            writeVRM(x, y, txtAttr, szTitle, fstAttr);
            printChar(x, y, txtAttr, styles[0]);
            printChar(x + bLen - 2, y, txtAttr, styles[1]);
            writeChar(x + 1, y + 1, wAttr, bLen - 1, styles[2]);
            writeChar(x + bLen - 1 , y, wAttr, 1, styles[3]);
        }
        else
        {
            writeVRM(x, y, txtAttr, szTitle, 0);
            printChar(x, y, txtAttr, styles[0]);
            printChar(x + bLen - 1, y, txtAttr, styles[1]);
            writeChar(x + 1, y + 1, wAttr, bLen, styles[2]);
            writeChar(x + bLen, y, wAttr, 1, styles[3]);
        }
    }
}

/*-----------------------------------------------*/
/* Function : drawFrame                          */
/* Purpose  : Draw a frame with the edge is lane */
/* Expects  : (x1,y1) cordinate top to left      */
/*            (x2,y2) cordinate bottom to right  */
/* Returns  : Nothing                            */
/*-----------------------------------------------*/
void drawFrame(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t attr)
{
    int16_t k;

    for (k = x1 + 1; k < x2; k++)
    {
        printChar(k, y1, attr, 193);
        printChar(k, y2, attr, 194);
    }

    for (k = y1 + 1; k < y2; k++)
    {
        printChar(x1, k, attr, 179);
        printChar(x2, k, attr, 224);
    }

    printChar(x1, y1, attr, 218);
    printChar(x2, y1, attr, 191);
    printChar(x1, y2, attr, 192);
    printChar(x2, y2, attr, 225);
}

/*---------------------------------------------------*/
/* Function : changeAttrib                           */
/* Purpose  : Chage the attribute of the area screen */
/* Expects  : (x1,y1) cordinate top to left          */
/*            (x2,y2) cordinate bottom to right      */
/*            (attr) the attribute                   */
/* Returns  : Nothing                                */
/*---------------------------------------------------*/
void changeAttrib(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t attr)
{
    uint8_t x0 = x1, y0 = y1;
    const uint8_t addofs = (80 - (x2 - x1 + 1)) << 1;
    uint8_t far *pmem = (uint8_t far*)(txtMem + OFFSET(x1, y1));

    for (y0 = y1; y0 <= y2; y0++)
    {
        for (x0 = x1; x0 <= x2; x0++)
        {
            *(pmem + 1) = attr;
            pmem += 2;
        }
        pmem += addofs;
    }
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
/*           (wAttr) special character color        */
/*           (chr) special character                */
/* Returns : Nothing                                */
/*--------------------------------------------------*/
void fillFrame(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t wAttr, char chr)
{
    uint8_t y;
    for (y = y1; y <= y2; y++) writeChar(x1, y, wAttr, x2 - x1 + 1, chr);
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

/*----------------------------------------------*/
/* Function : drawBox                           */
/* Purpose  : Draw a box with color and border  */
/* Expects  : (x1,y1) cordinate top to left     */
/*            (x2,y2) cordinate bottom to right */
/*            (wAttr) the attribute of the box  */
/* Returns  : Nothing                           */
/*----------------------------------------------*/
void drawBox(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t wAttr)
{
    drawFrame(x1, y1, x2, y2, wAttr);
    fillFrame(x1 + 1, y1 + 1, x2 - 1, y2 - 1, wAttr, 32);
    changeAttrib(x2 + 1, y1 + 1, x2 + 2, y2 + 1, 0x08);
    changeAttrib(x1 + 2, y2 + 1, x2 + 2, y2 + 1, 0x08);
}

/*----------------------------------------------*/
/* Function : shadowBox                         */
/* Purpose  : Draw a box with shadow (very art) */
/* Expects  : (x1,y1) cordinate top to left     */
/*            (x2,y2) cordinate bottom to right */
/*            (wAttr) the attribute of the box  */
/*            (szTitle) the title of header     */
/* Returns  : Nothing                           */
/*----------------------------------------------*/
void shadowBox(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t wAttr, char *szTitle)
{
    const uint8_t bkc = wAttr << 4;
    const char szStyle[] = {229, 252, 0};
    const uint16_t bCenter = ((x2 - x1) >> 1) - (strlen(szTitle) >> 1);
    changeAttrib(x2 + 1, y1 + 1, x2 + 2, y2 + 1, MASK_BG);
    changeAttrib(x1 + 2, y2 + 1, x2 + 2, y2 + 1, MASK_BG);
    drawBox(x1, y1, x2, y2, wAttr);
    writeChar(x1 + 3, y1, bkc, x2 - x1 - 2, 32);
    writeVRM(x1 + bCenter, y1, bkc, szTitle, 0);
    printChar(x1 + 2, y1, bkc, 226);
    writeVRM(x1, y1, bkc >> 4, szStyle, 0);
}

/*----------------------------------*/
/* Function : initMouse             */
/* Purpose  : Initialize mouse port */
/* Expects  : Nothing               */
/* Returns  : 1 if success. 0 fail  */
/*----------------------------------*/
uint16_t initMouse()
{
    union REGS regs;
    regs.x.ax = 0x00;
    int86(0x33, &regs, &regs);
    if (regs.x.ax != 0xFFFF) return 0;
    regs.x.ax = 0x01;
    int86(0x33, &regs, &regs);
    bmAvalid = 1;
    return 1;
}

/*-----------------------------------------------------*/
/* Function : clickMouse                               */
/* Purpose  : Get status button and cordinate col, row */
/* Expects  : (col, row) cordinate of col and row      */
/* Returns  : Value 1 : left button, 2 : right button, */
/*            4 : center button and col, row           */
/*-----------------------------------------------------*/
uint16_t clickMouse(uint16_t *col, uint16_t *row)
{
    union REGS regs;
    regs.x.ax = 0x03;
    int86(0x33, &regs, &regs);
    *col = (regs.x.cx >> 3) + 1;
    *row = (regs.x.dx >> 3) + 1;
    return regs.x.bx == 1;
}

/*-----------------------------------*/
/* Function : hideMouse              */
/* Purpose  : Hide the mouse pointer */
/* Expects  : Nothing                */
/* Returns  : Nothing                */
/*-----------------------------------*/
void hideMouse()
{
    union REGS regs;
    regs.x.ax = 0x02;
    int86(0x33, &regs, &regs);
}

/*--------------------------------------*/
/* Function : showMouse                 */
/* Purpose  : Showing the mouse pointer */
/* Expects  : Nothing                   */
/* Returns  : Nothing                   */
/*--------------------------------------*/
void showMouse()
{
    union REGS regs;
    regs.x.ax = 0x01;
    int86(0x33, &regs, &regs);
}

/*--------------------------------------------*/
/* Function : SetPos                          */
/* Purpose  : Setting the limit col and row   */
/* Expects  : Nothing                         */
/* Returns  : Nothing                         */
/*--------------------------------------------*/
void setMousePos()
{
    union REGS regs;
    regs.x.ax = 0x07;
    regs.x.cx = 0;
    regs.x.dx = 2 * 320 - 8;
    int86(0x33, &regs, &regs);
    regs.x.ax = 0x08;
    regs.x.cx = 0;
    regs.x.dx = 200 - 8;
    int86(0x33, &regs, &regs);
    regs.x.ax = 0x1D;
    regs.x.bx = 0;
    int86(0x33, &regs, &regs);
}

/*------------------------------------------------*/
/* Function : moveMouse                           */
/* Purpose  : Move mouse pointer to new cordinate */
/* Expects  : (x, y) the new cordinate            */
/* Returns  : Nothing                             */
/*------------------------------------------------*/
void moveMouse(uint16_t x, uint16_t y)
{
    union REGS regs;
    regs.x.ax = 0x0004;
    regs.x.cx = (x << 3) - 1;
    regs.x.dx = (y << 3) - 1;
    int86(0x33, &regs, &regs);
}

/*------------------------------------*/
/* Function : closeMouse              */
/* Purpose  : Colosing mouse fumction */
/* Expects  : Nothing                 */
/* Returns  : Nothing                 */
/*------------------------------------*/
void closeMouse()
{
    union REGS regs;
    hideMouse();
    regs.x.ax = 0;
    int86(0x33, &regs, &regs);
}

/*-----------------------------------------*/
/* Funtion : strPos                        */
/* Purpose : Getting position of substring */
/* Expects : (str) The string main         */
/*           (substr) The substring        */
/* Returns : Position of substring         */
/*-----------------------------------------*/
int16_t strPos(char *str, char *szSubstr)
{
    char *ptr = strstr(str, szSubstr);
    if (!ptr) return -1;
    return ptr - str;
}

/*---------------------------------------------*/
/* Funtion : insertChar                        */
/* Purpose : Inserted the char into string     */
/* Expects : (str) The string                  */
/*           (chr) The character need inserted */
/*           (iPos) The position inserted      */
/* Returns : Nothing                           */
/*---------------------------------------------*/
void insertChar(char *str, char chr, int16_t iPos)
{
    if (iPos < 0 || iPos >= strlen(str)) return;
    *(str + iPos) = chr;
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
void fontVNI(char *szPrmpt)
{
    char buff[4] = {0};
    schRepl(szPrmpt, "a8", 128);
    chr2Str(128, '1', buff);
    schRepl(szPrmpt, buff, 129);
    chr2Str(128, '2', buff);
    schRepl(szPrmpt, buff, 130);
    chr2Str(128, '3', buff);
    schRepl(szPrmpt, buff, 131);
    chr2Str(128, '4', buff);
    schRepl(szPrmpt, buff, 132);
    chr2Str(128, '5', buff);
    schRepl(szPrmpt, buff, 133);
    schRepl(szPrmpt, "a6", 134);
    chr2Str(134, '1', buff);
    schRepl(szPrmpt, buff, 135);
    chr2Str(134, '2', buff);
    schRepl(szPrmpt, buff, 136);
    chr2Str(134, '3', buff);
    schRepl(szPrmpt, buff, 137);
    chr2Str(134, '4', buff);
    schRepl(szPrmpt, buff, 138);
    chr2Str(134, '5', buff);
    schRepl(szPrmpt, buff, 139);
    schRepl(szPrmpt, "e6", 140);
    chr2Str(140, '1', buff);
    schRepl(szPrmpt, buff, 141);
    chr2Str(140, '2', buff);
    schRepl(szPrmpt, buff, 142);
    chr2Str(140, '3', buff);
    schRepl(szPrmpt, buff, 143);
    chr2Str(140, '4', buff);
    schRepl(szPrmpt, buff, 144);
    chr2Str(140, '5', buff);
    schRepl(szPrmpt, buff, 145);
    schRepl(szPrmpt, "o7", 146);
    chr2Str(146, '1', buff);
    schRepl(szPrmpt, buff, 147);
    chr2Str(146, '2', buff);
    schRepl(szPrmpt, buff, 148);
    chr2Str(146, '3', buff);
    schRepl(szPrmpt, buff, 149);
    chr2Str(146, '4', buff);
    schRepl(szPrmpt, buff, 150);
    chr2Str(146, '5', buff);
    schRepl(szPrmpt, buff, 151);
    schRepl(szPrmpt, "o6", 152);
    chr2Str(152, '1', buff);
    schRepl(szPrmpt, buff, 153);
    chr2Str(152, '2', buff);
    schRepl(szPrmpt, buff, 154);
    chr2Str(152, '3', buff);
    schRepl(szPrmpt, buff, 155);
    chr2Str(152, '4', buff);
    schRepl(szPrmpt, buff, 156);
    chr2Str(152, '5', buff);
    schRepl(szPrmpt, buff, 157);
    schRepl(szPrmpt, "u7", 158);
    chr2Str(158, '1', buff);
    schRepl(szPrmpt, buff, 159);
    chr2Str(158, '2', buff);
    schRepl(szPrmpt, buff, 160);
    chr2Str(158, '3', buff);
    schRepl(szPrmpt, buff, 161);
    chr2Str(158, '4', buff);
    schRepl(szPrmpt, buff, 162);
    chr2Str(158, '5', buff);
    schRepl(szPrmpt, buff, 163);
    schRepl(szPrmpt, "a1", 164);
    schRepl(szPrmpt, "a2", 165);
    schRepl(szPrmpt, "a3", 166);
    schRepl(szPrmpt, "a4", 167);
    schRepl(szPrmpt, "a5", 168);
    schRepl(szPrmpt, "e1", 169);
    schRepl(szPrmpt, "e2", 170);
    schRepl(szPrmpt, "e3", 171);
    schRepl(szPrmpt, "e4", 172);
    schRepl(szPrmpt, "e5", 173);
    schRepl(szPrmpt, "i1", 174);
    schRepl(szPrmpt, "i2", 175);
    schRepl(szPrmpt, "i3", 181);
    schRepl(szPrmpt, "i4", 182);
    schRepl(szPrmpt, "i5", 183);
    schRepl(szPrmpt, "o1", 184);
    schRepl(szPrmpt, "o2", 190);
    schRepl(szPrmpt, "o3", 198);
    schRepl(szPrmpt, "o4", 199);
    schRepl(szPrmpt, "o5", 208);
    schRepl(szPrmpt, "u1", 210);
    schRepl(szPrmpt, "u2", 211);
    schRepl(szPrmpt, "u3", 212);
    schRepl(szPrmpt, "u4", 213);
    schRepl(szPrmpt, "u5", 214);
    schRepl(szPrmpt, "y1", 215);
    schRepl(szPrmpt, "y2", 216);
    schRepl(szPrmpt, "y3", 221);
    schRepl(szPrmpt, "y4", 222);
    schRepl(szPrmpt, "y5", 248);
    schRepl(szPrmpt, "d9", 249);
    schRepl(szPrmpt, "D9", 250);
}

/*------------------------------------*/
/* Funtion : decodeFile               */
/* Purpose : Decode file register.sys */
/* Expects : (inFile) The source file */
/*           (outFile) The dest file  */
/* Returns : Number of lines in file  */
/*------------------------------------*/
uint16_t decodeFile(const char *inFile, const char *outFile)
{
    int16_t c, key = 98;
    uint16_t linesCount = 0;
    FILE *inHandle, *outHandle;
    
    sysNum = 0;
    inHandle = fopen(inFile, "rb");
    outHandle = fopen(outFile, "wb");

    if (!inHandle || !outHandle)
    {
        fprintf(stderr, "Error loading file %s. System halt.", inFile);
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
/* Function : getText                             */
/* Purpose  : Reading information into data array */
/* Expects  : (inFile) the input file             */
/*            (outFile) the output file           */
/*            (szData) the array data             */
/*            (wNumElm) the number of elements    */
/* Returns  : Nothing                             */
/*------------------------------------------------*/
void getText(const char *inFile, const char *outFile)
{
    FILE *fp;
    char szBuffer[102];

    sysNum = decodeFile(inFile, outFile);
    sysInfo = (char**)malloc(sysNum * sizeof(char*));
    if (!sysInfo)
    {
        fprintf(stderr, "Not enough memory: %d\n", sysNum);
        exit(1);
    }

    sysNum = 0;
    fp = fopen(outFile, "rt");
    while (fgets(szBuffer, 102, fp))
    {
        fontVNI(szBuffer);
        sysInfo[sysNum] = (char*)malloc(strlen(szBuffer) + 1);
        if (!sysInfo)
        {
            fprintf(stderr, "Not enough memory at line: %u", sysNum);
            exit(1);
        }
        szBuffer[strlen(szBuffer) - 1] = '\0';
        strcpy(sysInfo[sysNum], szBuffer);
        sysNum++;
    }

    fclose(fp);
    unlink(outFile);
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
    for (i = 0; i < sysNum; i++) free(sysInfo[i]);
    free(sysInfo);
}

/*-------------------------------*/
/* Funtion : cleanup             */
/* Purpose : Restore environment */
/* Expects : Nothing             */
/* Returns : Nothing             */
/*-------------------------------*/
void cleanup()
{
    setBorder(0x00);
    setCursorSize(0x0607);
    setCursorPos(1, 1);
    setBlinking(1);
    if (bmAvalid) closeMouse();
    releaseData();
    system("font off");
    system("cls");
}

/*---------------------------------------------*/
/* Funtion : fadeIn                            */
/* Purpose : Debrightness light of the monitor */
/* Expects : Nothing                           */
/* Returns : Nothing                           */
/*---------------------------------------------*/
void fadeIn()
{
    int16_t i, j;
    uint8_t palette[200], dummy[200];
    
    outp(0x3C7, 0);
    for (i = 0; i < 200; i++)
    {
        palette[i] = inp(0x3C9);
        dummy[i] = palette[i];
    }

    for (j = 0; j < 60; j++)
    {
        for (i = 0; i < 200; i++) if (dummy[i] > 0) dummy[i]--;
        outp(0x3C8, 0);
        for (i = 0; i < 200; i++) outp(0x3C9, dummy[i]);
        delay(1);
    }
    outp(0x3C8, 0);
    for (i = 0; i < 200; i++) outp(0x3C9, palette[i]);
}

/*----------------------------------------------*/
/* Function : initData                          */
/* Purpose  : Initialize parameters for program */
/* Expects  : Nothing                           */
/* Returns  : Nothing                           */
/*----------------------------------------------*/
void initData()
{
    getText("register.sys", "register.$$$");
    system("font on");
}

/*---------------------------------*/
/* Function : registerForm         */
/* Purpose  : Starting registering */
/* Expects  : Nothing              */
/* Returns  : Nothing              */
/*---------------------------------*/
void registerForm()
{
    FILE *fptr;
    REG_INFO regInfo;
    char szCurrName[25], szCurrID[20], *szName;
    uint8_t flgName = 1, flgID = 1;

    shadowBox(20, 8, 60, 15, 0x1F, sysInfo[27]);
    writeVRM(23, 10, 0x1F, sysInfo[5], 0);
    writeVRM(23, 12, 0x1F, sysInfo[6], 0);
    writeChar(36, 10, 0x4A, 24, 32);
    writeChar(36, 12, 0x4A, 24, 32);

    fptr = fopen(sysInfo[21], "r+b");
    if (!fptr)
    {
        fprintf(stderr, sysInfo[22]);
        cleanup();
        exit(1);
    }

    fread(&regInfo, sizeof(REG_INFO), 1, fptr);
    setCursorSize(0x0B0A);
    szCurrName[0] = 25;
    setCursorPos(36, 10);
    szName = cgets(szCurrName);
    if (strcmp(regInfo.user, szName))
    {
        writeChar(36, 10, 0x4F, 24, 32);
        writeVRM(36, 10, 0x4F, sysInfo[7], 0);
        flgName = 0;
    }

    setCursorPos(36, 12);
    szCurrID[0] = 20;
    szName = cgets(szCurrID);
    if (strcmp(szName, sysInfo[11]))
    {
        writeChar(36, 12, 0x4F, 24, 32);
        writeVRM(36, 12, 0x4F, sysInfo[8], 0);
        flgID = 0;
    }

    if (flgName && flgID)
    {
        writeVRM(30, 14, 0x1A, sysInfo[9], 0);
        regInfo.regs = 1;
        rewind(fptr);
        fwrite(&regInfo, sizeof(REG_INFO), 1, fptr);
        fclose(fptr);
    }
    else
    {
        writeVRM(30, 14, 0x1A, sysInfo[10], 0);
        fclose(fptr);
    }
}

/*-------------------------------------*/
/* Funtion : menuRegister              */
/* Purpose : Show register form        */
/* Expects : Nothing                   */
/* Returns : Nothing                   */
/*-------------------------------------*/
void menuRegister()
{
    char key = 0, isOK = 0;
    uint16_t pos = 0, col = 0, row = 0;

    initMouse();
    drawButton(22, 20, wATV, 5, sysInfo[25], 1, wFLT);
    drawButton(47, 20, _wATV, 5, sysInfo[26], 1, _wFLT);

    while (kbhit()) getch();
    do {
        if (kbhit())
        {
            key = getch();
            if (!key) key = getch();
            switch (key)
            {
            case LEFT:
                drawButton(22 + pos * 25, 20, _wATV, 5, sysInfo[25 + pos], 1, _wFLT);
                if (pos == 0) pos = 1; else pos--;
                drawButton(22 + pos * 25, 20, wATV, 5, sysInfo[25 + pos], 1, wFLT);
                break;
            case RIGHT:
                drawButton(22 + pos * 25, 20, _wATV, 5, sysInfo[25 + pos], 1, _wFLT);
                if (pos > 0) pos = 0; else pos++;
                drawButton(22 + pos * 25, 20, wATV, 5, sysInfo[25 + pos], 1, wFLT);
                break;
            case ENTER:
                isOK = 1;
                clearScreen(22, 20, 34, 21, 5);
                writeVRM(23 + pos * 25, 20, wATV, sysInfo[25 + pos], wFLT);
                delay(50);
                drawButton(22 + pos * 25, 20, wATV, 5, sysInfo[25 + pos], 1, wFLT);
                break;
            }
        }

        if (clickMouse(&col, &row))
        {
            if (row == 20 && col >= 22 && col <= 33)
            {
                pos = 0;
                isOK = 1;
                hideMouse();
                drawButton(47, 20, _wATV, 5, sysInfo[26], 1, _wFLT);
                clearScreen(22, 20, 34, 21, 5);
                writeVRM(23, 20, wATV, sysInfo[25], wFLT);
                delay(50);
                drawButton(22, 20, wATV, 5, sysInfo[25], 1, wFLT);
                showMouse();
            }

            if (row == 20 && col >= 47 && col <= 57)
            {
                pos = 1;
                isOK = 1;
                hideMouse();
                drawButton(22, 20, _wATV, 5, sysInfo[25], 1, _wFLT);
                clearScreen(47, 20, 58, 21, 5);
                writeVRM(48, 20, wATV, sysInfo[26], wFLT);
                delay(50);
                drawButton(47, 20, wATV, 5, sysInfo[26], 1, wFLT);
                showMouse();
            }

            if (col == 3 || col == 4 && row == 2) isOK = 1;
        }
    } while (!isOK);

    if (!pos)
    {
        registerForm();
        setCursorSize(0x2020);
        getch();
        fadeIn();
    }
}

/*--------------------------------------------------*/
/* Funtion : startRegister                          */
/* Purpose : Showing the menu startRegister message */
/* Expects : Nothing                                */
/* Returns : Nothing                                */
/*--------------------------------------------------*/
void startRegister()
{
    setBorder(55);
    setCursorSize(0x2020);
    setBlinking(0);
    fillFrame(1, 1, 80, 25, 0xFD, 178);
    shadowBox(3, 3, 77, 22, 0x5F, sysInfo[0]);
    writeVRM(8, 5, 0x5E, sysInfo[1], 0);
    writeVRM(15, 6, 0x5E, sysInfo[2], 0);
    writeVRM(5, 8, 0x5F, sysInfo[3], 0);
    writeVRM(5, 9, 0x5F, sysInfo[4], 0);
    writeVRM(5, 10, 0x5F, sysInfo[12], 0);
    writeVRM(5, 11, 0x5F, sysInfo[13], 0);
    writeVRM(15, 12, 0x5A, sysInfo[14], 0);
    writeVRM(15, 13, 0x5A, sysInfo[15], 0);
    writeVRM(15, 14, 0x5A, sysInfo[16], 0);
    writeVRM(5, 15, 0x5F, sysInfo[17], 0);
    writeVRM(5, 16, 0x5F, sysInfo[18], 0);
    writeVRM(5, 17, 0x5F, sysInfo[19], 0);
    writeVRM(7, 18, 0x5E, sysInfo[20], 0);
    menuRegister();
}

/*----------------------------------------*/
/* Funtion : isRegister                   */
/* Purpose : Checking value of regsiter   */
/* Expects : Nothing                      */
/* Returns : Value 0 if not startRegister */
/*----------------------------------------*/
uint8_t isRegister()
{
    FILE *fptr;
    REG_INFO regInfo;

    fptr = fopen(sysInfo[21], "rb");
    if (!fptr)
    {
        fprintf(stderr, sysInfo[22]);
        cleanup();
        exit(1);
    }

    fread(&regInfo, sizeof(REG_INFO), 1, fptr);
    fclose(fptr);
    return regInfo.regs ? 1 : 0;
}

/*--------------------------------------*/
/* Funtion : checkLicense               */
/* Purpose : Checking the period in use */
/* Expects : Nothing                    */
/* Returns : Nothing                    */
/*--------------------------------------*/
void checkLicense()
{
    FILE *fptr;
    REG_INFO regInfo;
    char szPath[33];
    struct dosdate_t da;

    _dos_getdate(&da);
    strcpy(szPath, sysInfo[24]);
    fptr = fopen(sysInfo[21], "rb");
    if (!fptr)
    {
        fprintf(stderr, sysInfo[22]);
        cleanup();
        exit(1);
    }

    fread(&regInfo, sizeof(REG_INFO), 1, fptr);
    fclose(fptr);
    strcat(szPath, regInfo.path);

    if (regInfo.month == da.month)
    {
        if ((da.day - regInfo.day) >= 7)
        {
            system(szPath);
            cleanup();
            exit(1);
        }
    }
    else
    {
        if (((da.day + 31) - regInfo.day) >= 7)
        {
            system(szPath);
            cleanup();
            exit(1);
        }
    }
}

void main()
{
    initData();
    if (!isRegister())
    {
        //checkLicense();
        startRegister();
    }
    cleanup();
}
