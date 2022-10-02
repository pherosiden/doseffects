#include <dos.h>
#include <io.h>
#include <stdio.h>
#include <ctype.h>
#include <conio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <direct.h>

#define MASK_BG         0x08
#define OFFSET(x, y)    (((x - 1) + 80 * (y - 1)) << 1)

#define DEL             8
#define TAB             9
#define LEFT            75
#define RIGHT           77
#define ENTER           13
#define SPACE           32

#define wATV            0xF0
#define wFLT            0xFC
#define _wATV           0x78
#define _wFLT           0x74

typedef struct {
    uint16_t    year;           // Register year
    uint16_t    month;          // Register month
    uint16_t    day;            // Register day
    uint16_t    days;           // The number of days
    uint16_t    key;            // Encryption key
    uint16_t    verid;          // Validate license key
    uint8_t     license[20];    // License key
    char        path[33];       // The installation path
} regs_t;

typedef struct {
    uint16_t day;               // day of month [1..31]
    uint16_t month;             // month of year [1..12]
    uint16_t year;              // year since 1980
} date_t;

uint8_t bmAvalid = 0;           // Status of the mouse
char **sysInfo = NULL;          // Text message
uint16_t sysNum = 0;            // Message count
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
    *pmem = (attr << 8) | chr;
}

/*------------------------------------------------*/
/* Funtion : writeChar                            */
/* Purpose : Writting a character with attribute  */
/* Expects : (x,y) cordinate to write a character */
/*           (attr) attribute of character        */
/*           (bLen) length area                   */
/*           (Chr) symbol needs to write          */
/* Returns : Nothing                              */
/*------------------------------------------------*/
void writeChar(uint8_t x, uint8_t y, uint8_t attr, uint8_t len, char chr)
{
    uint8_t i;
    const uint16_t txt = (attr << 8) | chr;
    uint16_t far *pmem = (uint16_t far*)(txtMem + OFFSET(x, y));
    for (i = 0; i < len; i++) *pmem++ = txt;
}

/*-----------------------------------------------*/
/* Function : writeText                          */
/* Purpose  : Writing a character with attribute */
/* Notices  : Intervention in video memory       */
/* Expects  : (x,y) cordinate needs to writting  */
/*            (attr) The attribute of string     */
/*            (str) the string to format         */
/*            (lets) The attr of first letter    */
/* Returns : Nothing                             */
/*-----------------------------------------------*/
void writeText(uint8_t x, uint8_t y, uint8_t attr, const char *str, uint8_t lets)
{
    uint16_t far *pmem = (uint16_t far*)(txtMem + OFFSET(x, y));

    if (lets)
    {
        char *ptmp = NULL;
        char buff[80] = {0};
        uint8_t i = 0, stop = 0, currx = x, pos;

        strcpy(buff, str);
        for (i = 0; (i < strlen(buff) - 1) && !stop; i++)
        {
            if (buff[i] == 126) stop = 1;
        }

        memmove(&buff[i - 1], &buff[i], strlen(buff) - i + 1);
        pos = i - 1;

        ptmp = buff;
        while (*ptmp) *pmem++ = (attr << 8) | *ptmp++;
        printChar(currx + pos, y, lets, buff[pos]);
    }
    else
    {
        while (*str) *pmem++ = (attr << 8) | *str++;
    }
}

/*-----------------------------------------------*/
/* Function : drawButton                         */
/* Purpose  : Define the button shadow           */
/* Expects  : (x,y) cordinate needs to writting  */
/*            (attr) the attribute of a title    */
/*            (title) the string to format       */
/*            (type) The type of button          */
/*            (lets) The attr of first letter    */
/* Returns : Nothing                             */
/*-----------------------------------------------*/
void drawButton(uint8_t x, uint8_t y, uint8_t attr, uint8_t bkc, const char *title, uint8_t type, uint8_t lets)
{
    const uint8_t bka = bkc << 4;
    const uint16_t len = strlen(title);
    const char styles[] = {16, 17, 223, 220};

    if (type)
    {
        if (lets)
        {
            writeText(x, y, attr, title, lets);
            writeChar(x + 1, y + 1, bka, len - 1, styles[2]);
            writeChar(x + len - 1, y, bka, 1, styles[3]);
        }
        else
        {
            writeText(x, y, attr, title, 0);
            writeChar(x + 1, y + 1, bka, len, styles[2]);
            writeChar(x + len, y, bka, 1, styles[3]);
        }
    }
    else
    {
        if (lets)
        {
            writeText(x, y, attr, title, lets);
            printChar(x, y, attr, styles[0]);
            printChar(x + len - 2, y, attr, styles[1]);
            writeChar(x + 1, y + 1, bka, len - 1, styles[2]);
            writeChar(x + len - 1 , y, bka, 1, styles[3]);
        }
        else
        {
            writeText(x, y, attr, title, 0);
            printChar(x, y, attr, styles[0]);
            printChar(x + len - 1, y, attr, styles[1]);
            writeChar(x + 1, y + 1, bka, len, styles[2]);
            writeChar(x + len, y, bka, 1, styles[3]);
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
        for (x0 = x1; x0 <= x2; x0++, pmem += 2) *(pmem + 1) = attr;
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

/*---------------------------------------------------------*/
/* Function : readKey                                      */
/* Purpose  : Read a key from the keyboard                 */
/* Expects  : (ch) get the key from the keyboard           */
/* Returns  : If the key is a extend key then return code  */
/*	          key and 0 value else return 1 value and code */
/*---------------------------------------------------------*/
char readKey(char *key)
{
    union REGS regs;
    regs.h.ah = 0;
    int86(22, &regs, &regs);
    if (!(regs.h.al))
    {
        *key = regs.h.ah;
        return 0;
    }
    *key = regs.h.al;
    return 1;
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

/*----------------------------------------------*/
/* Function : drawBox                           */
/* Purpose  : Draw a box with color and border  */
/* Expects  : (x1,y1) cordinate top to left     */
/*            (x2,y2) cordinate bottom to right */
/*            (attr) the attribute of the box   */
/* Returns  : Nothing                           */
/*----------------------------------------------*/
void drawBox(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t attr)
{
    drawFrame(x1, y1, x2, y2, attr);
    fillFrame(x1 + 1, y1 + 1, x2 - 1, y2 - 1, attr, 32);
    changeAttrib(x2 + 1, y1 + 1, x2 + 2, y2 + 1, MASK_BG);
    changeAttrib(x1 + 2, y2 + 1, x2 + 2, y2 + 1, MASK_BG);
}

/*----------------------------------------------*/
/* Function : shadowBox                         */
/* Purpose  : Draw a box with shadow (very art) */
/* Expects  : (x1,y1) cordinate top to left     */
/*            (x2,y2) cordinate bottom to right */
/*            (attr) the attribute of the box   */
/*            (title) the title of header       */
/* Returns  : Nothing                           */
/*----------------------------------------------*/
void shadowBox(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t attr, char *title)
{
    const uint8_t bkc = attr << 4;
    const char styles[] = {229, 252, 0};
    const uint16_t center = (x2 - x1 - strlen(title)) >> 1;
    changeAttrib(x2 + 1, y1 + 1, x2 + 2, y2 + 1, MASK_BG);
    changeAttrib(x1 + 2, y2 + 1, x2 + 2, y2 + 1, MASK_BG);
    drawBox(x1, y1, x2, y2, attr);
    writeChar(x1 + 3, y1, bkc, x2 - x1 - 2, 32);
    writeText(x1 + center, y1, bkc, title, 0);
    printChar(x1 + 2, y1, bkc, 226);
    writeText(x1, y1, bkc >> 4, styles, 0);
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
int16_t strPos(char *str, char *substr)
{
    char *ptr = strstr(str, substr);
    if (!ptr) return -1;
    return ptr - str;
}

/*---------------------------------------------*/
/* Funtion : insertChar                        */
/* Purpose : Inserted the char into string     */
/* Expects : (str) The string                  */
/*           (chr) The character need inserted */
/*           (pos) The position inserted       */
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
    memcpy(str + i + 1, str + i + num, strlen(str) - i - 1);
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
    char buff[3] = {0};
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
/*           (ofile) The dest file    */
/* Returns : Number of lines in file  */
/*------------------------------------*/
uint16_t decodeFile(const char *ifile, const char *ofile)
{
    int16_t c, key = 98;
    uint16_t linesCount = 0;
    FILE *inHandle, *outHandle;
    
    sysNum = 0;
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

    sysNum = decodeFile(ifile, ofile);
    sysInfo = (char**)malloc(sysNum * sizeof(char*));
    if (!sysInfo)
    {
        fprintf(stderr, "Not enough memory: %d\n", sysNum);
        exit(1);
    }

    sysNum = 0;
    fp = fopen(ofile, "rt");
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
    unlink(ofile);
}

/*------------------------------------------*/
/* Function : freeData                      */
/* Purpose  : free block memory of the data */
/* Expects  : Nothing                       */
/* Returns  : Nothing                       */
/*------------------------------------------*/
void freeData()
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
    freeData();
    system("font off");
    system("cls");
    exit(1);
}

/*---------------------------------------------*/
/* Funtion : fadeOut                           */
/* Purpose : Debrightness light of the monitor */
/* Expects : Nothing                           */
/* Returns : Nothing                           */
/*---------------------------------------------*/
void fadeOut()
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
/* Function : getDiskSerial                     */
/* Purpose  : Get the disk serial number        */
/* Expects  : (drive) the drive letter          */
/*            (serial) output serial number     */
/* Returns  : Nothing                           */
/*----------------------------------------------*/
void getDiskSerial(char drive, char *serial)
{
    FILE *fp;
    char sbuff[128];
    const char * term = "Volume Serial Number is ";

    sprintf(sbuff, "vol %c: > .vol", drive);
    system(sbuff);
    delay(100);

    fp = fopen(".vol", "rt");
    if (!fp) return;

    while (fgets(sbuff, 128, fp))
    {
        if (strstr(sbuff, term)) break;
    }
    
    fclose(fp);
    unlink(".vol");
    strcpy(serial, &sbuff[strlen(term) + 1]);
}

/*----------------------------------------------*/
/* Function : encodeString                      */
/* Purpose  : Generate encoded key              */
/* Expects  : (user) input user name            */
/* Returns  : The decoded key                   */
/*----------------------------------------------*/
void encodeString(char *str, uint16_t key)
{
    while (*str) *str++ += key;
}

/*----------------------------------------------*/
/* Function : getEncryptKey                     */
/* Purpose  : Generate encryption key           */
/* Expects  : (str) input user name             */
/* Returns  : encryption key                    */
/*----------------------------------------------*/
uint16_t getEncryptKey(char *str)
{
    uint16_t key = 0;
    while (*str) key += *str++;
    srand(key);
    key += rand();
    return key;
}

/*----------------------------------------------*/
/* Function : makeLicenseKey                    */
/* Purpose  : Generate license key              */
/* Expects  : (key) input encryption key        */
/*            (serial) input disk serial number */
/*            (license) output license key      */
/* Returns  : Nothing                           */
/*----------------------------------------------*/
void makeLicenseKey(uint16_t key, char *serial, char *license)
{
    char sbuff1[10];
    char sbuff2[15];
    uint16_t lo = 0, hi = 0;
    uint16_t sum = 0, i = 0;

    sscanf(serial, "%X-%X", &lo, &hi);
    sprintf(sbuff1, "%.4X-%.4X", lo + key, hi + key);
    for (i = 0; i < strlen(sbuff1); i++) sum += sbuff1[i];
    sum += key;
    sprintf(sbuff2, "%.4X-%s", sum, sbuff1);
    sum = 0;
    for (i = 0; i < strlen(sbuff2); i++) sum += sbuff2[i];
    sum += key;
    sprintf(license, "%.4X-%s", sum, sbuff2);
}

/*--------------------------------------*/
/* Funtion : validLicense               */
/* Purpose : Testing user serial number */
/* Expects : Nothing                    */
/* Returns : Nothing                    */
/*--------------------------------------*/
uint8_t validLicense(char *user, char *license)
{
    FILE *fp;
    regs_t regs;

    char serial[20];
    uint16_t i = 0;

    fp = fopen(sysInfo[21], "r+b");
    if (!fp)
    {
        fprintf(stderr, sysInfo[22]);
        cleanup();
    }
    fread(&regs, sizeof(regs_t), 1, fp);

    regs.key = getEncryptKey(user);
    getDiskSerial(regs.path[0], serial);
    makeLicenseKey(regs.key, serial, regs.license);
    if (!strcmp(regs.license, license))
    {
        encodeString(regs.license, regs.key);
        for (i = 0; i < strlen(regs.license); i++) regs.verid += regs.license[i];
        regs.verid += regs.key;
        rewind(fp);
        fwrite(&regs, sizeof(regs), 1, fp);
        fclose(fp);
        return 1;
    }

    fclose(fp);
    return 0;
}

/*----------------------------------------------*/
/* Function : countLeapYears                    */
/* Purpose  : Calculate leap years              */
/* Expects  : (d) The date_t struct             */
/* Returns  : number of days                    */
/*----------------------------------------------*/
int16_t countLeapYears(date_t d)
{
    int16_t years = d.year;
    if (d.month <= 2) years--;
    return years / 4 - years / 100 + years / 400;
}

/*----------------------------------------------*/
/* Function : diffDays                          */
/* Purpose  : Difference days between two dates */
/* Expects  : (dt1, dt2) The date_t struct      */
/* Returns  : Number of days                    */
/*----------------------------------------------*/
int16_t diffDays(date_t dt1, date_t dt2)
{
    int16_t i;
    size_t n1, n2;
    const int16_t monthDays[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    
    n1 = dt1.year * 365 + dt1.day;
    for (i = 0; i < dt1.month - 1; i++) n1 += monthDays[i];
    n1 += countLeapYears(dt1);
 
    n2 = dt2.year * 365 + dt2.day;
    for (i = 0; i < dt2.month - 1; i++) n2 += monthDays[i];
    n2 += countLeapYears(dt2);
 
    return (n2 - n1);
}

/*----------------------------------------------*/
/* Function : initData                          */
/* Purpose  : Initialize parameters for program */
/* Expects  : Nothing                           */
/* Returns  : Nothing                           */
/*----------------------------------------------*/
void initData()
{
    srand(time(0));
    getTextFile("register.sys", "register.$$$");
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
    char key = 0;
    char szUserName[33], szSerial[22];

    uint16_t col = 0, row = 0;
    uint8_t i = 0, j = 0, isASCII = 0, isOK = 0;
    uint8_t selUserName = 0, selSerial = 0, slc = 0;
        
    shadowBox(20, 8, 65, 17, 0x1F, sysInfo[27]);
    writeText(23, 10, 0x1F, sysInfo[5], 0);
    writeText(23, 12, 0x1F, sysInfo[6], 0);
    writeChar(36, 10, 0x4E, 26, 32);
    writeChar(36, 12, 0x4E, 26, 32);

    memset(szUserName, 0, sizeof(szUserName));
    memset(szSerial, 0, sizeof(szSerial));

    setCursorSize(0x0B0A);
    drawButton(26, 15, wATV, 1, sysInfo[28], 1, wFLT);
    drawButton(47, 15, _wATV, 1, sysInfo[29], 1, _wFLT);
    
    isOK = 0;
    selSerial = 0;
    selUserName = 1;
    key = i = j = slc = 0;
    
    while (kbhit()) getch();
    do {
        if (selUserName) setCursorPos(36 + i, 10);
        if (selSerial) setCursorPos(36 + j, 12);
      
        if (kbhit())
        {
            isASCII = readKey(&key);
            if (!key) isASCII = readKey(&key);
            if (selUserName)
            {
                if (((isASCII && key != DEL && isalpha(key)) || (key == SPACE)) && i < 30)
                {
                    szUserName[i] = key;
                    printChar(36 + i, 10, 0x4E, key);
                    i++;
                }

                if (key == DEL && i > 0)
                {
                    i--;
                    printChar(36 + i, 10, 0x4E, 32);
                }

                szUserName[i] = '\0';
            }

            if (selSerial)
            {
                if (isASCII && key != DEL && key != TAB && key != ENTER && j < 19)
                {
                    szSerial[j] = toupper(key);
                    printChar(36 + j, 12, 0x4E, toupper(key));
                    j++;
                }

                if (key == DEL && j > 0)
                {
                    j--;
                    printChar(36 + j, 12, 0x4E, 32);
                }

                szSerial[j] = '\0';
            }

            switch (key)
            {
            case LEFT:
                if (!isASCII)
                {
                    drawButton(26 + slc * 21, 15, _wATV, 1, sysInfo[28 + slc], 1, _wFLT);
                    if (slc < 1) slc = 0; else slc--;
                    drawButton(26 + slc * 21, 15, wATV, 1, sysInfo[28 + slc], 1, wFLT);
                }
                break;
            case RIGHT:
                if (!isASCII)
                {
                    drawButton(26 + slc * 21, 15, _wATV, 1, sysInfo[28 + slc], 1, _wFLT);
                    if (slc > 0) slc = 1; else slc++;
                    drawButton(26 + slc * 21, 15, wATV, 1, sysInfo[28 + slc], 1, wFLT);
                }
                break;
            case TAB:
                if (selUserName)
                {
                    selUserName = 0;
                    selSerial = 1;
                }
                else
                {
                    selSerial = 0;
                    selUserName = 1;
                }
                break;
            case ENTER:
                clearScreen(26 + slc * 21, 15, 36 + slc * 21, 16, 1);
                writeText(27 + slc * 21, 15, wATV, sysInfo[28 + slc], wFLT);
                delay(50);
                drawButton(26 + slc * 21, 15, wATV, 1, sysInfo[28 + slc], 1, wFLT);
                if (!slc)
                {
                    if (validLicense(szUserName, szSerial))
                    {
                        isOK = 1;
                        writeText(30, 13, 0x1A, sysInfo[9], 0);
                    }
                    else
                    {
                        selUserName = 1;
                        selSerial = 0;
                        writeText(30, 13, 0x1A, sysInfo[10], 0);
                    }                    
                }
                else
                {
                    cleanup();
                }
                break;
            }
        }

        if (clickMouse(&col, &row))
        {
            if (row == 15 && col >= 24 && col <= 36)
            {
                slc = 0;
                drawButton(47, 15, _wATV, 1, sysInfo[4], 1, _wFLT);
                clearScreen(26, 15, 36, 16, 1);
                writeText(27, 15, wATV, sysInfo[1], wFLT);
                delay(50);
                drawButton(26, 15, wATV, 1, sysInfo[1], 1, wFLT);

                if (validLicense(szUserName, szSerial))
                {
                    isOK = 1;
                    writeText(30, 13, 0x1A, sysInfo[9], 0);
                }
                else
                {
                    selUserName = 1;
                    selSerial = 0;
                    writeText(30, 13, 0x1A, sysInfo[10], 0);
                }
            }

            if (row == 15 && col >= 47 && col <= 59)
            {
                slc = 1;
                drawButton(26, 15, _wATV, 1, sysInfo[1], 1, _wFLT);
                clearScreen(47, 15, 59, 16, 1);
                writeText(48, 15, wATV, sysInfo[4], wFLT);
                delay(50);
                drawButton(47, 15, wATV, 1, sysInfo[4], 1, wFLT);
                cleanup();
            }

            if (row == 10 && col >= 8 && col <= 38)
            {
                selUserName = 1;
                selSerial = 0;
            }

            if (row == 12 && col >= 8 && col <= 38)
            {
                selSerial = 1;
                selUserName = 0;
            }
        }
    } while (!isOK);
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
                clearScreen(22 + pos * 25, 20, 34 + pos * 25, 21, 5);
                writeText(23 + pos * 25, 20, wATV, sysInfo[25 + pos], wFLT);
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
                drawButton(47, 20, _wATV, 5, sysInfo[26], 1, _wFLT);
                clearScreen(22, 20, 34, 21, 5);
                writeText(23, 20, wATV, sysInfo[25], wFLT);
                delay(50);
                drawButton(22, 20, wATV, 5, sysInfo[25], 1, wFLT);
            }

            if (row == 20 && col >= 47 && col <= 57)
            {
                pos = 1;
                isOK = 1;
                drawButton(22, 20, _wATV, 5, sysInfo[25], 1, _wFLT);
                clearScreen(47, 20, 58, 21, 5);
                writeText(48, 20, wATV, sysInfo[26], wFLT);
                delay(50);
                drawButton(47, 20, wATV, 5, sysInfo[26], 1, wFLT);
            }

            if (col == 3 || col == 4 && row == 2) isOK = 1;
        }
    } while (!isOK);

    if (!pos)
    {
        registerForm();
        setCursorSize(0x2020);
        getch();
        fadeOut();
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
    writeText(8, 5, 0x5E, sysInfo[1], 0);
    writeText(15, 6, 0x5E, sysInfo[2], 0);
    writeText(5, 8, 0x5F, sysInfo[3], 0);
    writeText(5, 9, 0x5F, sysInfo[4], 0);
    writeText(5, 10, 0x5F, sysInfo[12], 0);
    writeText(5, 11, 0x5F, sysInfo[13], 0);
    writeText(15, 12, 0x5A, sysInfo[14], 0);
    writeText(15, 13, 0x5A, sysInfo[15], 0);
    writeText(15, 14, 0x5A, sysInfo[16], 0);
    writeText(5, 15, 0x5F, sysInfo[17], 0);
    writeText(5, 16, 0x5F, sysInfo[18], 0);
    writeText(5, 17, 0x5F, sysInfo[19], 0);
    writeText(7, 18, 0x5E, sysInfo[20], 0);
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
    regs_t regInfo;

    char serial[20];
    char license[20];
    uint16_t i = 0, magic = 0;

    fptr = fopen(sysInfo[21], "rb");
    if (!fptr) return 0;
    fread(&regInfo, sizeof(regs_t), 1, fptr);
    fclose(fptr);

    if (!regInfo.verid || !strlen(regInfo.license)) return 0;

    for (i = 0; i < strlen(regInfo.license); i++) magic += regInfo.license[i];
    magic += regInfo.key;

    if (magic != regInfo.verid) return 0;

    memset(serial, 0, sizeof(serial));
    memset(license, 0, sizeof(license));
    getDiskSerial(regInfo.path[0], serial);
    makeLicenseKey(regInfo.key, serial, license);
    encodeString(license, regInfo.key);
    return !strcmp(license, regInfo.license);
}

/*--------------------------------------*/
/* Funtion : checkExpired               */
/* Purpose : Checking the period in use */
/* Expects : Nothing                    */
/* Returns : Nothing                    */
/*--------------------------------------*/
void checkExpired()
{
    FILE *fptr;
    char szPath[33];
    regs_t regInfo;
    date_t dt1, dt2;
    struct dosdate_t date;
    
    fptr = fopen(sysInfo[21], "rb");
    if (!fptr)
    {
        fprintf(stderr, sysInfo[22]);
        cleanup();
    }

    fread(&regInfo, sizeof(regs_t), 1, fptr);
    fclose(fptr);
    
    strcpy(szPath, regInfo.path);
    strcat(szPath, sysInfo[24]);

    _dos_getdate(&date);
    dt2.year = date.year;
    dt2.month = date.month;
    dt2.day = date.day;
   
    dt1.year = regInfo.year;
    dt1.month = regInfo.month;
    dt1.day = regInfo.day;

    if (diffDays(dt1, dt2) > regInfo.days)
    {
        system(szPath);
        cleanup();
    }
}

void main()
{
    initData();
    if (!isRegister())
    {
        checkExpired();
        startRegister();
    }
    cleanup();
}
