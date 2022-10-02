/*--------------------------------------------------------------*/
/*        UNIVERSITY OF TECHNOLOGY HO CHI MINH CITY             */
/*            FACULTY OF INFORMATION TECHNOLOGY                 */
/*             Author : NGUYEN NGOC VAN                         */
/*              Class : 00DTH1                                  */
/*       Student Code : 00DTH201                                */
/*             Course : 2000 - 2005                             */
/*      Writting Date : 24/09/2001                              */
/*        Last Update : 12/10/2001                              */
/*--------------------------------------------------------------*/
#include <dos.h>
#include <io.h>
#include <ctype.h>
#include <fcntl.h>
#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdint.h>
#include <direct.h>
#include <time.h>

#define MAX_DAYS        30
#define SCR_WIDTH       160
#define MASK_BG         0x08
#define OFFSET(x, y)    (((x - 1) + (y - 1) * 80) << 1)

#define INST_DIR        "B:\\INSTALL"
#define PROG_DIR        "C:\\TOPICS\\*.*"

#define UP              72
#define DOWN            80
#define HOME            71
#define END             79
#define DEL             8
#define TAB             9
#define ESC             27
#define LEFT            75
#define RIGHT           77
#define ENTER           13
#define SPACE           32

#define wATV            0xF0
#define wFLT            0xFC
#define _wATV           0x78
#define _wFLT           0x74
#define _eATV           0xE7
#define _eFLT           0xE6

#define DOS_SEG	        0x0040L
#define RESET_FLAG      0x0072L
#define BOOT_SEG        0xFFFFL
#define BOOT_OFF        0x0000L
#define BOOT_ADR        ((BOOT_SEG << 16) | BOOT_OFF)
#define RESET_ADR	    ((DOS_SEG << 16) | RESET_FLAG)

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

char szInstallPath[32];         // The installation path

uint8_t bmAvalid = 0;           // Status of the mouse
uint16_t numFiles = 0;          // Number of files in current directory
uint16_t totalFiles = 0;        // Number files to read and write

char **sysInfo, **sysMenu;
uint16_t infoNum, menuNum;

char *msgWarn[1], *msgExit[1];
char *msgCmp[1], *msgError[2];

char key = 0;
char szScreen[792] = {0};

uint16_t bCol = 0, bRow = 0;
uint8_t msgSlc = 0, chs = 0, slc = 0;
uint8_t far *txtMem = (uint8_t far*)0xB8000000L;

size_t segBuff = 0;
size_t bytesCount = 0;
uint8_t far *copyBuff = NULL;

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
/* Funtion : getCursorSize             */
/* Mission : Get current cursor size   */
/* Expects : (size) The size of cursor */
/* Returns : Current cursor size       */
/*-------------------------------------*/
uint16_t getCursorSize()
{
    union REGS regs;
    regs.h.ah = 0x03;
    regs.h.bh = 0;
    int86(0x10, &regs, &regs);
    return regs.x.cx;
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
/*           (wAttr) attribute of character       */
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

/*------------------------------------------------*/
/* Function : writeText                           */
/* Purpose  : Writing a character with attribute  */
/* Notices  : Direct write to video memory        */
/* Expects  : (x,y) cordinate needs to write      */
/*            (attr) The attrib of string         */
/*            (str) The string contents           */
/*            (lets) The attrib hot key letter    */
/* Returns : Nothing                              */
/*------------------------------------------------*/
void writeText(uint8_t x, uint8_t y, uint8_t attr, const char *str, uint8_t lets)
{
    uint16_t far *pmem = (uint16_t far*)(txtMem + OFFSET(x, y));

    if (lets)
    {
        char *ptmp = NULL;
        char buff[80] = {0};
        uint8_t i = 0, fltStop = 0, currX = x, bPos;

        strcpy(buff, str);
        for (i = 0; (i < strlen(buff) - 1) && !fltStop; i++)
        {
            if (buff[i] == 126) fltStop = 1;
        }

        memmove(&buff[i - 1], &buff[i], strlen(buff) - i + 1);
        bPos = i - 1;

        ptmp = buff;
        while (*ptmp) *pmem++ = (attr << 8) | *ptmp++;
        printChar(currX + bPos, y, lets, buff[bPos]);
    }
    else
    {
        while (*str) *pmem++ = (attr << 8) | *str++;
    }
}

/*-----------------------------------------------*/
/* Function : printText                          */
/* Purpose  : Writing a character with attribute */
/* Notices  : Intervention in video memory       */
/* Expects  : (x,y) cordinate needs to writting  */
/*            (attr) The attribute of string     */
/*            (str) the string to format         */
/* Returns : Nothing                             */
/*-----------------------------------------------*/
void printText(uint8_t x, uint8_t y, uint8_t attr, char *str, ...)
{
    char buffer[255];
    va_list params;
    va_start(params, str);
    vsprintf(buffer, str, params);
    writeText(x, y, attr, buffer, 0);
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
void drawButton(uint8_t x, uint8_t y, uint8_t attr, uint8_t bkc, char *title, uint8_t type, uint8_t lets)
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
/* Purpose  : Modify the attribute of screen text    */
/* Expects  : (x1,y1) cordinate top to left          */
/*            (x2,y2) cordinate right to bottom      */
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

/*-------------------------------------------------------*/
/* Function : setBlinking                                */
/* Purpose  : Set bits 7 in the text attribute           */
/* Expects  : (doblink) = 0, background color is lighter */
/*                      = 1, text is blinking            */
/* Return   : Nothing                                    */
/*-------------------------------------------------------*/
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
/*           (attr) special character color        */
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

/*----------------------------------------------*/
/* Funtion : getScreenText                      */
/* Purpose : Get string at coordinate           */
/* Expects : (x, y, width, height) screen coord */
/*           buff the buffer to store data      */
/* Returns : Nothing                            */
/*----------------------------------------------*/
void getScreenText(int16_t x, int16_t y, int16_t width, int16_t height, uint8_t *buff)
{
    uint8_t far *src = txtMem + OFFSET(x, y);

    width <<= 1;
    while (height--)
    {
        _fmemcpy(buff, src, width);
        buff += width;
        src += SCR_WIDTH;
    }
}

/*----------------------------------------------*/
/* Funtion : putScreenText                      */
/* Purpose : Get string at coordinate           */
/* Expects : (x, y, width, height) screen coord */
/*           buff the buffer to store data      */
/* Returns : Nothing                            */
/*----------------------------------------------*/
void putScreenText(int16_t x, int16_t y, int16_t width, int16_t height, uint8_t *buff)
{
    uint8_t far *dst = txtMem + OFFSET(x, y);

    width <<= 1;
    while (height--)
    {
        _fmemcpy(dst, buff, width);
        buff += width;
        dst += SCR_WIDTH;
    }
}

/*-------------------------------------*/
/* Funtion : decodeFile                */
/* Purpose : Decode the language files */
/* Expects : (ifile) The source file   */
/*           (ofile) The dest file     */
/* Returns : Number of lines in file   */
/*-------------------------------------*/
uint16_t decodeFile(const char *ifile, const char *ofile)
{
    int16_t c, key = 98;
    uint16_t linesCount = 0;
    FILE *ihandle, *ohandle;
    
    ihandle = fopen(ifile, "rb");
    ohandle = fopen(ofile, "wb");

    if (!ihandle || !ohandle)
    {
        fprintf(stderr, "Error loading file %s. System halt.", ifile);
        exit(1);
    }

    while ((c = fgetc(ihandle)) != EOF)
    {
        c = c - ~key;
        fputc(c, ohandle);
        if (c == 266) linesCount++;
    }

    fclose(ihandle);
    fclose(ohandle);
    return linesCount;
}

/*------------------------------------------------*/
/* Function : getTextFile                         */
/* Purpose  : Reading information into data array */
/* Expects  : (ifile) the input file              */
/*            (ofile) the output file             */
/*            (data) the array data               */
/*            (num) the number of elements        */
/* Returns  : Nothing                             */
/*------------------------------------------------*/
void getTextFile(const char *ifile, const char *ofile, char ***data, uint16_t *num)
{
    FILE *fp;
    uint16_t elems = 0;
    char szBuffer[102] = {0};
    
    elems = decodeFile(ifile, ofile);
    data[0] = (char**)malloc(elems * sizeof(char*));
    if (!(data[0]))
    {
        fprintf(stderr, "Not enough memory for count: %d\n", elems);
        exit(1);
    }

    fp = fopen(ofile, "rt");
    if (!fp)
    {
        fprintf(stderr, "Error open file: %s", ofile);
        exit(1);   
    }

    elems = 0;
    while (fgets(szBuffer, 102, fp))
    {
        fontVNI(szBuffer);
        data[0][elems] = (char*)malloc(strlen(szBuffer) + 1);
        if (!(data[0][elems]))
        {
            fprintf(stderr, "Not enough memory at line: %u", elems);
            exit(1);
        }
        szBuffer[strlen(szBuffer) - 1] = '\0';
        strcpy(data[0][elems], szBuffer);
        elems++;
    }

    *num = elems;
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
    int16_t i;

    for (i = 0; i < infoNum; i++) free(sysInfo[i]);
    free(sysInfo);

    for (i = 0; i < menuNum; i++) free(sysMenu[i]);
    free(sysMenu);

    free(msgExit[0]);
    free(msgCmp[0]);
    free(msgWarn[0]);
    free(msgError[0]);
    free(msgError[1]);

    if (segBuff)
    {
        _dos_freemem(segBuff);
        segBuff = 0;
    }
}

/*-------------------------------*/
/* Funtion : cleanup             */
/* Mission : Restore environment */
/* Expects : Nothing             */
/* Returns : Nothing             */
/*-------------------------------*/
void cleanup()
{
    setBorder(0x00);
    setBlinking(1);
    if (bmAvalid) closeMouse();
    setCursorSize(0x0607);
    freeData();
    system("font off");
    system("cls");
    exit(1);
}

/*-------------------------------------*/
/* Funtion : fadeOut                   */
/* Purpose : Debrightness of the text  */
/* Expects : Nothing                   */
/* Returns : Nothing                   */
/*-------------------------------------*/
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
/* Function : initData                          */
/* Purpose  : Initialize parameters for program */
/* Expects  : Nothing                           */
/* Returns  : Nothing                           */
/*----------------------------------------------*/
void initData()
{
    getTextFile("sysmenu.dll", "sysmenu.$$$", &sysMenu, &menuNum);
    getTextFile("sysinfo.sys", "sysinfo.$$$", &sysInfo, &infoNum);
    msgWarn[0] = (char*)malloc(40);
    msgExit[0] = (char*)malloc(40);
    msgCmp[0] = (char*)malloc(40);
    msgError[0] = (char*)malloc(40);
    msgError[1] = (char*)malloc(40);
    strcpy(msgExit[0], sysInfo[26]);
    strcpy(msgCmp[0], sysInfo[28]);
    strcpy(msgWarn[0], sysInfo[27]);
    strcpy(msgError[0], sysInfo[32]);
    strcpy(msgError[1], sysInfo[33]);
    system("font on");
}

/*--------------------------------*/
/* Funtion : sysReboot            */
/* Purpose : Restart the computer */
/* Expects : Nothing              */
/* Returns : Nothing              */
/*--------------------------------*/
void sysReboot()
{
    void((far *resetFunc)()) = (void(far*)())BOOT_ADR;
    *(int16_t far*)RESET_ADR = 0;
    (*resetFunc)();
}

/*---------------------------------------*/
/* Funtion : errorFile                   */
/* Purpose : Display the error code      */
/* Expects : (handle) which file error   */
/*           (errortype) error file type */
/* Returns : Nothing                     */
/*---------------------------------------*/
void errorFile(char *msg, char *type)
{
    char errorMsg[60], isOK = 0;
    strcpy(errorMsg, type);
    strcat(errorMsg, msg);
    shadowBox(13, 8, 65, 17, 0x4F, sysInfo[1]);
    drawButton(33, 15, 0xF0, 4, sysMenu[3], 1, 0xF4);
    writeText(39 - strlen(errorMsg) / 2, 10, 0x4A, errorMsg, 0);
    writeText(15, 11, 0x4F, sysInfo[53], 0);
    writeText(15, 12, 0x4F, sysInfo[54], 0);
    writeText(15, 13, 0x4F, sysInfo[55], 0);

    while (kbhit()) getch();
    do {
        if (kbhit() || clickMouse(&bCol, &bRow))
        {
            if (kbhit() || bRow == 15 && bCol >= 33 && bCol <= 45)
            {
                isOK = 1;
                clearScreen(33, 15, 46, 16, 4);
                writeText(34, 15, wATV, sysMenu[3], wFLT);
                delay(60);
                drawButton(33, 15, wATV, 4, sysMenu[3], 1, wFLT);
            }

            if (bRow == 8 && bCol == 13 || bCol == 14) isOK = 1;
        }
    } while (!isOK);
    cleanup();
}

/*---------------------------------------------*/
/* Funtion : makePath                          */
/* Purpose : Create directory with full path   */
/* Expects : (spath) sources path              */
/* Returns : Nothing                           */
/*---------------------------------------------*/
void makePath(char *spath)
{
    char sbuff[64];
    char *next = spath;

    if (!_access(spath, 0) || !spath || !strlen(spath)) return;

    memset(sbuff, 0, sizeof(sbuff));
    while (next && strlen(next) > 0)
    {
        next = strchr(next, '\\');
        if (!next) strcpy(sbuff, spath);
        else
        {
            strncpy(sbuff, spath, next - spath);
            next++;
        }
        
        if (_access(sbuff, 0)) _mkdir(sbuff);
    }
}

/*---------------------------------------------*/
/* Funtion : countFiles                        */
/* Purpose : Count total files from directory  */
/* Expects : (dirName) sources directory       */
/* Returns : Number of files in directory      */
/*---------------------------------------------*/
void countFiles(char *dirName)
{
    size_t i = 0;
    struct find_t entries;
    char srcPath[64], srcExt[64], srcDir[64];

    for (i = strlen(dirName) - 1; dirName[i] != '\\'; i--);

    strcpy(srcPath, dirName);
    srcPath[i] = '\0';
    strcpy(srcExt, &dirName[i + 1]);

    if (!_dos_findfirst(dirName, _A_NORMAL | _A_RDONLY | _A_HIDDEN | _A_SYSTEM, &entries))
    {
        do {
            totalFiles++;
        } while (!_dos_findnext(&entries));
    }

    sprintf(srcDir, "%s\\*.*", srcPath);
    if (!_dos_findfirst(srcDir, _A_SUBDIR | _A_NORMAL | _A_RDONLY | _A_HIDDEN | _A_SYSTEM, &entries))
    {
        do {
            if ((entries.attrib & _A_SUBDIR) && (entries.name[0] != '.'))
            {
                sprintf(srcDir, "%s\\%s\\%s", srcPath, entries.name, srcExt);
                countFiles(srcDir);
            }
        } while (!_dos_findnext(&entries));
    }
}

/*---------------------------------------------*/
/* Funtion : copyFile                          */
/* Purpose : Copy file from source to dest     */
/* Expects : (src) sources file path           */
/*           (dst) destination file path       */
/*           (fileInfo) file attributes        */
/* Returns : 1 for success                     */
/*           0 for failure                     */
/*---------------------------------------------*/
uint8_t copyFile(char *src, char *dst, struct find_t *fileInfo)
{
    int srcHandle, dstHandle;
    size_t numBytes = bytesCount;
    
    if (_dos_open(src, O_RDONLY, &srcHandle)) errorFile(src, sysInfo[17]);
    if (_dos_creat(dst, fileInfo->attrib, &dstHandle)) errorFile(dst, sysInfo[17]);

    while (numBytes)
    {
        if (_dos_read(srcHandle, copyBuff, numBytes, &numBytes))
        {
            _dos_close(srcHandle);
            return 0;
        }

        if (_dos_write(dstHandle, copyBuff, numBytes, &numBytes))
        {
            _dos_close(dstHandle);
            return 0;
        }
    }

    _dos_setftime(dstHandle, fileInfo->wr_date, fileInfo->wr_time);
    _dos_close(srcHandle);
    _dos_close(dstHandle);
    return 1;
}

/*---------------------------------------------*/
/* Funtion : processDir                        */
/* Purpose : Copy all files from the disk      */
/* Expects : (psrc) sources directory          */
/*           (pdst) destination directory      */
/* Returns : 1 for success                     */
/*           0 for failure                     */
/*---------------------------------------------*/
void processDir(char *psrc, char *pdst)
{
    size_t i;
    struct find_t entries;
    int16_t progress, percent;
    
    char srcPath[64], srcExt[64], srcDir[64];
    char curFile[68], newFile[68], newDir[64];

    for (i = strlen(psrc) - 1; psrc[i] != '\\'; i--);

    strcpy(srcPath, psrc);
    srcPath[i] = '\0';
    strcpy(srcExt, &psrc[i + 1]);

    if (!_dos_findfirst(psrc, _A_NORMAL | _A_RDONLY | _A_HIDDEN | _A_SYSTEM, &entries))
    {
        do {
            sprintf(curFile, "%s\\%s", srcPath, entries.name);
            sprintf(newFile, "%s\\%s", pdst, entries.name);
            if (!copyFile(curFile, newFile, &entries)) errorFile(newFile, sysInfo[21]);

            numFiles++;
            progress = 45.0 * numFiles / totalFiles;
            percent = 100.0 * numFiles / totalFiles;

            writeChar(30, 11, 0x99, 33, 32);
            writeText(30, 11, 0x9E, newFile, 0);
            writeChar(18, 12, 0xFF, progress, 219);
            printText(50, 13, 0x9F, "%3d", percent);
            delay(60);
        } while (!_dos_findnext(&entries));
    }

    sprintf(srcDir, "%s\\*.*", srcPath);
    if (!_dos_findfirst(srcDir, _A_SUBDIR | _A_NORMAL | _A_RDONLY | _A_HIDDEN | _A_SYSTEM, &entries))
    {
        do {
            if ((entries.attrib & _A_SUBDIR) && (entries.name[0] != '.'))
            {
                sprintf(srcDir, "%s\\%s\\%s", srcPath, entries.name, srcExt);
                sprintf(newDir, "%s\\%s", pdst, entries.name);
                _mkdir(newDir);
                _dos_setfileattr(newDir, entries.attrib);
                processDir(srcDir, newDir);
            }
        } while (!_dos_findnext(&entries));
    }
}

/*---------------------------------------*/
/* Function : messageBox                 */
/* Purpose  : Display the system message */
/* Expects  : (x1,y1,x2,y2) coordinates  */
/*            (msg) The messages array   */
/*            (n) The number of elements */
/* Return   : Selected order             */
/*---------------------------------------*/
uint8_t messageBox(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, char *msg[], int16_t n)
{
    char key = 0, isOK = 0;
    uint8_t slc = 0, i = 0;
    const uint16_t oldCursor = getCursorSize();
    const int16_t center = x1 + (x2 - x1) / 2;

    setCursorSize(0x2020);
    shadowBox(x1, y1, x2, y2, 0x4F, sysInfo[1]);
    moveMouse(center, y2 - 3);
    
    for (i = 0; i < n; i++) writeText(center - strlen(msg[i]) / 2 + 1, y1 + 2 + i, 0x4A, msg[i], 0);
    drawButton(center - 14, y2 - 2, wATV, 4, sysMenu[0], 1, wFLT);
    drawButton(center + 6, y2 - 2, _eATV, 4, sysMenu[2], 1, _eFLT);

    while (kbhit()) getch();
    do {
        if (kbhit())
        {
            key = getch();
            if (!key) key = getch();
            switch (key)
            {
            case LEFT:
                drawButton(center - 14 + slc * 20, y2 - 2, _eATV, 4, sysMenu[slc * 2], 1, _eFLT);
                if (slc < 1) slc = 1; else slc--;
                drawButton(center - 14 + slc * 20, y2 - 2, wATV, 4, sysMenu[slc * 2], 1, wFLT);
                break;
            case RIGHT:
                drawButton(center - 14 + slc * 20, y2 - 2, _eATV, 4, sysMenu[slc * 2], 1, _eFLT);
                if (slc > 0) slc = 0; else slc++;
                drawButton(center - 14 + slc * 20, y2 - 2, wATV, 4, sysMenu[slc * 2], 1, wFLT);
                break;
            case ENTER:
                isOK = 1;
                clearScreen(center - 14 + slc * 20, y2 - 2, center - 3 + slc * 20, y2 - 1, 4);
                writeText(center - 13 + slc * 20, y2 - 2, wATV, sysMenu[2 * slc], wFLT);
                delay(60);
                drawButton(center - 14 + slc * 20, y2 - 2, wATV, 4, sysMenu[2 * slc], 1, wFLT);
                break;
            }
        }
        
        if (clickMouse(&bCol, &bRow))
        {
            if (bRow == y2 - 2 && bCol >= center - 14 && bCol <= center - 5)
            {
                slc = 0;
                isOK = 1;
                drawButton(center + 6, y2 - 2, _eATV, 4, sysMenu[2], 1, _eFLT);
                clearScreen(center - 14, y2 - 2, center - 3, y2 - 1, 4);
                writeText(center - 13, y2 - 2, wATV, sysMenu[0], wFLT);
                delay(60);
                drawButton(center - 14, y2 - 2, wATV, 4, sysMenu[0], 1, wFLT);
            }

            if (bRow == y2 - 2 && bCol >= center + 6 && bCol <= center + 15)
            {
                slc = 1;
                isOK = 1;
                drawButton(center - 14, y2 - 2, _eATV, 4, sysMenu[0], 1, _eFLT);
                clearScreen(center + 6,  y2 - 2, center + 16, y2 - 1, 4);
                writeText(center + 7, y2 - 2, wATV, sysMenu[2], wFLT);
                delay(60);
                drawButton(center + 6, y2 - 2, wATV, 4, sysMenu[2], 1, wFLT);
            }

            if (bRow == y1 && bCol == x1 || bCol == x1 + 1)
            {
                slc = 1;
                isOK = 1;
            }
        }        
    } while (!isOK);
    
    while (kbhit()) getch();
    setCursorSize(oldCursor);
    return slc;
}

/*---------------------------------------*/
/* Function : warningBox                 */
/* Purpose  : Display the system message */
/* Expects  : (x1,y1,x2,y2) coordinates  */
/*            (msg) The messages array   */
/*            (n) The elements of array  */
/* Return   : 1 or 0                     */
/*---------------------------------------*/
void warningBox(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, char *msg[], int16_t n)
{
    int16_t i = 0;
    char key = 0, isOK = 0;
    const uint16_t oldCursor = getCursorSize();
    const int16_t bCenter = x1 + (x2 - x1) / 2;
    const int16_t bPos = bCenter - strlen(sysMenu[0]) / 2;
    
    setCursorSize(0x2020);
    shadowBox(x1, y1, x2, y2, 0x4F, sysInfo[1]);

    for (i = 0; i < n; i++) writeText(bCenter - strlen(msg[i]) / 2, y1 + 2 + i, 0x4A, msg[i], 0);
    
    drawButton(bPos, y2 - 2, wATV, 4, sysMenu[0], 1, wFLT);
    moveMouse(bCenter, y2 - 3);

    while (kbhit()) getch();
    do {
        if (kbhit() || clickMouse(&bCol, &bRow))
        {
            if (kbhit() || (bRow == y2 - 2 && bCol >= bPos && bCol <= bPos + 9))
            {
                isOK = 1;
                clearScreen(bPos, y2 - 2, bPos + 7, y2 - 1, 4);
                writeText(bPos + 1, y2 - 2, wATV, sysMenu[0], 0xF4);
                delay(60);
                drawButton(bPos, y2 - 2, wATV, 4, sysMenu[0], 1, 0xF4);
            }

            if (bRow == y1 && bCol == x1 || bCol == x1 + 1) isOK = 1;
        }
    } while (!isOK);

    while (kbhit()) getch();
    setCursorSize(oldCursor);
}


/*---------------------------------------*/
/* Function : validUserName              */
/* Mission  : Check user name is valid   */
/* Expects  : (szUserName) the user name */
/* Returns  : 1 for ok                   */
/*            0 for invalid              */
/*---------------------------------------*/
uint8_t validUserName(char *szUserName)
{
    uint16_t i = 0, len = 0;

    len = strlen(szUserName);
    if (len < 5) return 0;

    i = 0;
    while (isspace(szUserName[i++]));
    if (i >= len) return 0;

    for (i = 0; i < len; i++)
    {
        if (!isalpha(szUserName[i]) && !isspace(szUserName[i])) return 0;
    }

    return 1;
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
/* Function : getEncryptKey                     */
/* Purpose  : Generate encryption key           */
/* Expects  : (str) input user name             */
/* Returns  : encryption key                    */
/*----------------------------------------------*/
uint16_t getEncryptKey(char *str)
{
    uint16_t key = 0;
    while (*str) key += *str++;
    return key;
}

/*----------------------------------------------*/
/* Function : makeProductKey                    */
/* Purpose  : Generate license key              */
/* Expects  : (key) input encryption key        */
/*            (serial) input disk serial number */
/*            (license) output license key      */
/* Returns  : Nothing                           */
/*----------------------------------------------*/
void makeProductKey(uint16_t key, char *serial, char *license)
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

/*-----------------------------------------------*/
/* Function : genProductKey                      */
/* Mission  : Generate product installation key  */
/* Expects  : (user) input user name             */
/*            (cdkey) output product key         */
/* Returns  : Nothing                            */
/*-----------------------------------------------*/
void genProductKey(char *user, char *cdkey)
{
    char serial[64];
    uint16_t key = getEncryptKey(user);
    getcwd(serial, 64);
    getDiskSerial(serial[0], serial);
    makeProductKey(key, serial, cdkey);
}

/*--------------------------------------*/
/* Funtion : validProductKey            */
/* Purpose : Testing user serial number */
/* Expects : Nothing                    */
/* Returns : Nothing                    */
/*--------------------------------------*/
uint8_t validProductKey(char *user, char *cdkey)
{
    char sbuff[20];
    if (!validUserName(user)) return 0;
    genProductKey(user, sbuff);
    return !strcmp(sbuff, cdkey);
}

/*--------------------------------------*/
/* Funtion : checkProductKey            */
/* Purpose : Testing user serial number */
/* Expects : Nothing                    */
/* Returns : Nothing                    */
/*--------------------------------------*/
void checkProductKey()
{
    char szUserName[31], szSerial[20];
    uint8_t selUserName = 0, selSerial = 0;
    uint8_t i = 0, j = 0, isASCII = 0, isOK = 0;

    setCursorSize(0x2020);
    fillFrame(1, 1, 80, 25, 0xF6, 178);
    shadowBox(5, 3, 75, 23, 0x5F, sysInfo[2]);
    writeText(8, 5, 0x5A, sysInfo[56], 0);
    writeText(8, 6, 0x5A, sysInfo[57], 0);
    writeText(8, 7, 0x5A, sysInfo[58], 0);
    writeText(8, 8, 0x5A, sysInfo[59], 0);
    writeText(8, 9, 0x5A, sysInfo[60], 0);
    writeText(14, 12, 0x5E, sysMenu[9], 0x5F);
    writeText(11, 11, 0x5B, sysMenu[20], 0);
    writeText(8, 14, 0x5F, sysInfo[22], 0);
    writeChar(8, 15, 0x1F, 30, 32);
    writeText(8, 17, 0x5F, sysInfo[23], 0);
    writeText(11, 12, 0x5E, sysMenu[21], 0);
    writeChar(8, 18, 0x1F, 30, 32);
    drawButton(24, 21, _wATV, 5, sysMenu[1], 1, _wFLT);
    drawButton(47, 21, wATV, 5, sysMenu[4], 1, wFLT);
    moveMouse(10, 11);

    slc = key = 0;
    writeText(14, 11 + slc, 0x5B, sysMenu[slc + 8], 0x5A);

    do {
        if (kbhit())
        {
            key = getch();
            if (!key) key = getch();
            switch (key)
            {
            case DOWN:
                writeText(14, 11 + slc, 0x5E, sysMenu[slc + 8], 0x5F);
                writeText(11, 11 + slc, 0x5E, sysMenu[21], 0);
                if (slc > 0) slc = 0; else slc++;
                writeText(14, 11 + slc, 0x5B, sysMenu[slc + 8], 0x5A);
                writeText(11, 11 + slc, 0x5B, sysMenu[20], 0);
                break;
            case UP:
                writeText(14, 11 + slc, 0x5E, sysMenu[slc + 8], 0x5F);
                writeText(11, 11 + slc, 0x5E, sysMenu[21], 0);
                if (slc < 1) slc = 1; else slc--;
                writeText(14, 11 + slc, 0x5B, sysMenu[slc + 8], 0x5A);
                writeText(11, 11 + slc, 0x5B, sysMenu[20], 0);
                break;
            case TAB:
                if (!slc) isOK = 1;
                else
                {
                    getScreenText(20, 10, 43, 9, szScreen);
                    msgSlc = messageBox(20, 10, 60, 16, msgExit, sizeof(msgExit) / sizeof(msgExit[0]));
                    if (!msgSlc) cleanup();
                    putScreenText(20, 10, 43, 9, szScreen);
                }
                break;
            }
        }

        if (clickMouse(&bCol, &bRow))
        {
            if (bRow == 11 && bCol >= 11 && bCol <= 40)
            {
                slc = 0;
                isOK = 1;
                writeText(14, 12, 0x5E, sysMenu[9], 0x5F);
                writeText(11, 12, 0x5E, sysMenu[21], 0);
                writeText(14, 11, 0x5B, sysMenu[8], 0x5A);
                writeText(11, 11, 0x5B, sysMenu[20], 0);
            }

            if (bRow == 12 && bCol >= 11 && bCol <= 55)
            {
                slc = 1;
                writeText(14, 11, 0x5E, sysMenu[8], 0x5F);
                writeText(11, 11, 0x5E, sysMenu[21], 0);
                writeText(14, 12, 0x5B, sysMenu[9], 0x5A);
                writeText(11, 12, 0x5B, sysMenu[20], 0);
                getScreenText(20, 10, 43, 9, szScreen);
                msgSlc = messageBox(20, 10, 60, 16, msgExit, sizeof(msgExit) / sizeof(msgExit[0]));
                if (!msgSlc) cleanup();
                putScreenText(20, 10, 43, 9, szScreen);
            }
        }
    } while (!isOK);

    memset(szUserName, 0, sizeof(szUserName));
    memset(szSerial, 0, sizeof(szSerial));

    setCursorSize(0x0B0A);
    setCursorPos(8, 15);
    drawButton(24, 21, wATV, 5, sysMenu[1], 1, wFLT);
    drawButton(47, 21, _wATV, 5, sysMenu[4], 1, _wFLT);
    
    isOK = 0;
    selSerial = 0;
    selUserName = 1;
    key = i = j = slc = 0;

    do {
        if (selUserName) setCursorPos(8 + i, 15);
        if (selSerial) setCursorPos(8 + j, 18);
      
        if (kbhit())
        {
            isASCII = readKey(&key);
            if (!key) isASCII = readKey(&key);
            if (selUserName)
            {
                if ((isASCII && i < 30 && key != DEL && isalpha(key)) || (key == SPACE && i < 30))
                {
                    szUserName[i] = key;
                    printChar(8 + i, 15, 0x1E, key);
                    i++;
                }

                if (key == DEL && i > 0)
                {
                    i--;
                    printChar(8 + i, 15, 0x1E, 32);
                }

                szUserName[i] = '\0';
            }

            if (selSerial)
            {
                if (isASCII && j < 19 && key != 8 && key != TAB && key != ENTER)
                {
                    szSerial[j] = toupper(key);
                    printChar(8 + j, 18, 0x1E, toupper(key));
                    j++;
                }

                if (key == DEL && j > 0)
                {
                    j--;
                    printChar(8 + j, 18, 0x1E, 32);
                }

                szSerial[j] = '\0';
            }

            switch (key)
            {
            case LEFT:
                if (!isASCII)
                {
                    drawButton(24 + slc * 23, 21, _wATV, 5, sysMenu[3 * slc + 1], 1, _wFLT);
                    if (slc < 1) slc = 0; else slc--;
                    drawButton(24 + slc * 23, 21, wATV, 5, sysMenu[3 * slc + 1], 1, wFLT);
                }
                break;
            case RIGHT:
                if (!isASCII)
                {
                    drawButton(24 + slc * 23, 21, _wATV, 5, sysMenu[3 * slc + 1], 1, _wFLT);
                    if (slc > 0) slc = 1; else slc++;
                    drawButton(24 + slc * 23, 21, wATV, 5, sysMenu[3 * slc + 1], 1, wFLT);
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
                clearScreen(24 + slc * 23, 21, 36 + slc * 23, 22, 5);
                writeText(25 + slc * 23, 21, wATV, sysMenu[3 * slc + 1], wFLT);
                delay(60);
                drawButton(24 + slc * 23, 21, wATV, 5, sysMenu[3 * slc + 1], 1, wFLT);
                if (!slc)
                {
                    if (validProductKey(szUserName, szSerial)) isOK = 1;
                    else
                    {
                        selUserName = 1;
                        selSerial = 0;
                        getScreenText(20, 10, 43, 9, szScreen);
                        warningBox(20, 10, 60, 16, msgWarn, sizeof(msgWarn) / sizeof(msgWarn[0]));
                        putScreenText(20, 10, 43, 9, szScreen);
                    }
                }
                else
                {
                    getScreenText(20, 10, 43, 9, szScreen);
                    msgSlc = messageBox(20, 10, 60, 16, msgExit, sizeof(msgExit) / sizeof(msgExit[0]));
                    if (!msgSlc) cleanup();
                    putScreenText(20, 10, 43, 9, szScreen);
                }
                break;
            }
        }

        if (clickMouse(&bCol, &bRow))
        {
            if (bRow == 21 && bCol >= 24 && bCol <= 36)
            {
                slc = 0;
                drawButton(47, 21, _wATV, 5, sysMenu[4], 1, _wFLT);
                clearScreen(24, 21, 36, 22, 5);
                writeText(25, 21, wATV, sysMenu[1], wFLT);
                delay(60);
                drawButton(24, 21, wATV, 5, sysMenu[1], 1, wFLT);

                if (validProductKey(szUserName, szSerial)) isOK = 1;
                else
                {
                    selUserName = 1;
                    selSerial = 0;
                    getScreenText(20, 10, 43, 9, szScreen);
                    warningBox(20, 10, 60, 16, msgWarn, sizeof(msgWarn) / sizeof(msgWarn[0]));
                    putScreenText(20, 10, 43, 9, szScreen);
                }
            }

            if (bRow == 21 && bCol >= 47 && bCol <= 59)
            {
                slc = 1;
                drawButton(24, 21, _wATV, 5, sysMenu[1], 1, _wFLT);
                clearScreen(47, 21, 59, 22, 5);
                writeText(48, 21, wATV, sysMenu[4], wFLT);
                delay(60);
                drawButton(47, 21, wATV, 5, sysMenu[4], 1, wFLT);
                getScreenText(20, 10, 43, 9, szScreen);
                msgSlc = messageBox(20, 10, 60, 16, msgExit, sizeof(msgExit) / sizeof(msgExit[0]));
                if (!msgSlc) cleanup();
                putScreenText(20, 10, 43, 9, szScreen);
            }

            if (bRow == 15 && bCol >= 8 && bCol <= 38)
            {
                selUserName = 1;
                selSerial = 0;
            }

            if (bRow == 18 && bCol >= 8 && bCol <= 38)
            {
                selSerial = 1;
                selUserName = 0;
            }
        }
    } while (!isOK);
}

/*------------------------------------*/
/* Funtion : showRegisterInfo         */
/* Purpose : Show user register info  */
/* Expects : Nothing                  */
/* Returns : Nothing                  */
/*------------------------------------*/
void showRegisterInfo()
{
    char isOK = 0;

    setBorder(63);
    setCursorSize(0x2020);
    fillFrame(1, 1, 80, 25, 0xF6, 178);
    shadowBox(10, 3, 74, 22, 0x3E, sysInfo[3]);
    writeText(12, 5, 0x30, sysInfo[61], 0);
    writeText(12, 6, 0x30, sysInfo[62], 0);
    writeText(12, 7, 0x30, sysInfo[63], 0);
    writeText(12, 8, 0x30, sysInfo[64], 0);
    writeText(12, 9, 0x30, sysInfo[65], 0);
    writeText(12, 10, 0x30, sysInfo[66], 0);
    writeText(12, 11, 0x30, sysInfo[67], 0);
    writeText(12, 12, 0x30, sysInfo[68], 0);
    writeText(17, 14, 0x3C, sysInfo[69], 0);
    writeText(17, 15, 0x3C, sysInfo[70], 0);
    writeText(17, 16, 0x3C, sysInfo[71], 0);
    writeText(12, 18, 0x31, sysInfo[72], 0);
    drawButton(47, 20, _wATV, 3, sysMenu[4], 1, _wFLT);
    drawButton(26, 20, wATV, 3, sysMenu[1], 1, wFLT);
    moveMouse(42, 20);

    slc = chs = key = 0;
    while (kbhit()) getch();

    do {
        if (kbhit())
        {
            key = getch();
            if (!key) key = getch();
            switch (key)
            {
            case LEFT:
                drawButton(26 + slc * 21, 20, _wATV, 3, sysMenu[3 * slc + 1], 1, _wFLT);
                if (slc < 1) slc = 0; else slc--;
                drawButton(26 + slc * 21, 20, wATV, 3, sysMenu[3 * slc + 1], 1, wFLT);
                break;
            case RIGHT:
                drawButton(26 + slc * 21, 20, _wATV, 3, sysMenu[3 * slc + 1], 1, _wFLT);
                if (slc > 0) slc = 1; else slc++;
                drawButton(26 + slc * 21, 20, wATV, 3, sysMenu[3 * slc + 1], 1, wFLT);
                break;
            case ENTER:
                isOK = 1;
                clearScreen(26 + slc * 21, 20, 38 + slc * 21, 21, 11);
                writeText(27 + slc * 21, 20, wATV, sysMenu[3 * slc + 1], wFLT);
                delay(60);
                drawButton(26 + slc * 21, 20, wATV, 3, sysMenu[3 * slc + 1], 1, wFLT);
                break;
            }
        }

        if (clickMouse(&bCol, &bRow))
        {
            if (bRow == 20 && bCol >= 26 && bCol <= 38)
            {
                slc = 0;
                isOK = 1;
                drawButton(47, 20, _wATV, 3, sysMenu[4], 1, _wFLT);
                clearScreen(26, 20, 38, 21, 11);
                writeText(27, 20, wATV, sysMenu[1], wFLT);
                delay(60);
                drawButton(26, 20, wATV, 3, sysMenu[1], 1, wFLT);
            }

            if (bRow == 20 && bCol >= 46 && bCol <= 59)
            {
                slc = 1;
                isOK = 1;
                drawButton(26, 20, _wATV, 3, sysMenu[1], 1, _wFLT);
                clearScreen(47, 20, 59, 21, 11);
                writeText(48, 20, wATV, sysMenu[4], wFLT);
                delay(60);
                drawButton(47, 20, wATV, 3, sysMenu[4], 1, wFLT);
            }

            if (bRow == 3 && bCol == 10 || bCol == 11)
            {
                slc = 1;
                isOK = 1;
            }
        }
    } while (!isOK);

    if (slc) cleanup();
}

/*--------------------------------------------*/
/* Funtion : installProgram                   */
/* Purpose : Setup the program on computer    */
/* Expects : (szDrive) the szDrive to set up  */
/* Returns : Nothing                          */
/*--------------------------------------------*/
void installProgram()
{
    uint8_t i = 0, isOK = 0;

    if (_dos_allocmem(0xffff, &segBuff))
    {
        bytesCount = segBuff;
        if (_dos_allocmem(bytesCount, &segBuff))
        {
            setCursorSize(0x2020);
            fillFrame(1, 1, 80, 25, 0xF6, 178);
            shadowBox(15, 8, 65, 17, 0x4F, sysInfo[1]);
            writeText(27, 10, 0x4E, sysInfo[31], 0);
            writeText(17, 11, 0x4F, sysInfo[53], 0);
            writeText(17, 12, 0x4F, sysInfo[54], 0);
            writeText(17, 13, 0x4F, sysInfo[55], 0);
            drawButton(35, 15, wATV, 4, sysMenu[0], 1, wFLT);

            while (kbhit()) getch();
            do {
                if (kbhit() || clickMouse(&bCol, &bRow))
                {
                    if (kbhit() || (bRow == 15 && bCol >= 35 && bCol <= 45))
                    {
                        isOK = 1;
                        clearScreen(35, 15, 46, 16, 4);
                        writeText(36, 15, wATV, sysMenu[0], wFLT);
                        delay(60);
                        drawButton(35, 15, wATV, 4, sysMenu[0], 1, wFLT);
                    }

                    if (bRow == 8 && bCol == 15 || bCol == 16) isOK = 1;
                }
            } while (!isOK);
            cleanup();            
        }
    }

    copyBuff = (uint8_t far*)MK_FP(segBuff, 0);
    
    setBorder(50);
    setCursorSize(0x2020);
    fillFrame(1, 1, 80, 25, 0xF6, 178);
    shadowBox(15, 6, 65, 17, 0x9F, sysInfo[4]);
    writeText(18, 11, 0x9F, sysInfo[73], 0);
    writeText(18, 8, 0x9F, sysInfo[38], 0);
    writeText(18, 9, 0x9F, sysInfo[40], 0);
    writeChar(18, 12, 0x17, 45, 176);
    writeText(53, 13, 0x9F, sysInfo[171], 0);
    drawButton(35, 15, _wATV, 9, sysMenu[2], 1, _wFLT);
    makePath(szInstallPath);
    countFiles(PROG_DIR);
    processDir(PROG_DIR, szInstallPath);
    delay(500);
    fillFrame(15, 6, 69, 21, 0xF6, 178);
    shadowBox(18, 10, 62, 15, 0x1F, sysInfo[5]);
    writeText(22, 12, 0x1E, sysInfo[41], 0);
    writeChar(22, 13, 0x17, 37, 176);
    writeText(49, 14, 0x1A, sysInfo[171], 0);

    for (i = 1; i < 38; i++)
    {
        writeChar(22, 13, 0x1F, i, 219);
        printText(46, 14, 0x1A, "%3d", 2 * i + 26);
        delay(60);
    }

    fillFrame(15, 6, 69, 21, 0xF6, 178);
    warningBox(25, 10, 55, 16, msgCmp, sizeof(msgCmp) / sizeof(msgCmp[0]));
    
    _dos_freemem(segBuff);
    segBuff = 0;
}

/*----------------------------------------------*/
/* Funtion : restartSystem                      */
/* Purpose : Showing the restart system message */
/* Expects : Nothing                            */
/* Returns : Nothing                            */
/*----------------------------------------------*/
void restartSystem()
{
    char isOK = 0;

    system("font on");
    setCursorSize(0x2020);
    setBorder(50);
    fillFrame(1, 1, 80, 25, 0xF6, 178);
    shadowBox(15, 8, 65, 15, 0x4F, sysInfo[6]);
    writeText(21, 11, 0x4A, sysMenu[11], 0x4B);
    writeText(18, 10, 0x4F, sysMenu[20], 0);
    writeText(21, 10, 0x4F, sysMenu[10], 0x4E);
    writeText(18, 11, 0x4A, sysMenu[21], 0);
    drawButton(26, 13, 0xB4, 4, sysMenu[0], 1, 0xB1);
    drawButton(45, 13, 0x9F, 4, sysMenu[3], 1, 0x94);
    moveMouse(40, 13);

    slc = chs = key = 0;
    while (kbhit()) getch();

    do {
        if (kbhit())
        {
            key = getch();
            if (!key) key = getch();
            switch (key)
            {
            case DOWN:
                writeText(21, 10 + slc, 0x4A, sysMenu[slc + 10], 0x4B);
                writeText(18, 10 + slc, 0x4A, sysMenu[21], 0);
                if (slc > 0) slc = 0; else slc++;
                writeText(21, 10 + slc, 0x4F, sysMenu[slc + 10], 0x4E);
                writeText(18, 10 + slc, 0x4F, sysMenu[20], 0);
                break;
            case UP:
                writeText(21, 10 + slc, 0x4A, sysMenu[slc + 10], 0x4B);
                writeText(18, 10 + slc, 0x4A, sysMenu[21], 0);
                if (slc < 1) slc = 1; else slc--;
                writeText(21, 10 + slc, 0x4F, sysMenu[slc + 10], 0x4E);
                writeText(18, 10 + slc, 0x4F, sysMenu[20], 0);
                break;
            case LEFT:
                drawButton(26 + chs * 19, 13, 0x9F, 4, sysMenu[3 * chs], 1, 0x94);
                if (chs < 1) chs = 0; else chs--;
                drawButton(26 + chs * 19, 13, 0xB4, 4, sysMenu[3 * chs], 1, 0xB1);
                break;
            case RIGHT:
                drawButton(26 + chs * 19, 13, 0x9F, 4, sysMenu[3 * chs], 1, 0x94);
                if (chs > 0) chs = 1; else chs++;
                drawButton(26 + chs * 19, 13, 0xB4, 4, sysMenu[3 * chs], 1, 0xB1);
                break;
            case ENTER:
                isOK = 1;
                clearScreen(26 + chs * 19, 13, 36 + chs * 19, 14, 4);
                writeText(27 + chs * 19, 13, 0xB4, sysMenu[3 * chs], 0xB1);
                delay(60);
                drawButton(26 + chs * 19, 13, 0xB4, 4, sysMenu[3 * chs], 1, 0xB1);
                break;
            }
        }

        if (clickMouse(&bCol, &bRow))
        {
            if (bRow == 10 && bCol >= 18 && bCol <= 62)
            {
                slc = 0;
                writeText(21, 11, 0x4A, sysMenu[11], 0x4B);
                writeText(18, 11, 0x4A, sysMenu[21], 0);
                writeText(21, 10, 0x4F, sysMenu[10], 0x4E);
                writeText(18, 10, 0x4F, sysMenu[20], 0);
            }

            if (bRow == 11 && bCol >= 18 && bCol <= 61)
            {
                slc = 1;
                writeText(21, 10, 0x4A, sysMenu[10], 0x4B);
                writeText(18, 10, 0x4A, sysMenu[21], 0);
                writeText(21, 11, 0x4F, sysMenu[11], 0x4E);
                writeText(18, 11, 0x4F, sysMenu[20], 0);
            }

            if (bRow == 13 && bCol >= 26 && bCol <= 35)
            {
                chs = 0;
                isOK = 1;
                drawButton(45, 13, 0x9F, 4, sysMenu[3], 1, 0x94);
                clearScreen(26, 13, 36, 14, 4);
                writeText(27, 13, 0xB4, sysMenu[0], 0xB1);
                delay(60);
                drawButton(26, 13, 0xB4, 4, sysMenu[0], 1, 0xB1);
            }

            if (bRow == 13 && bCol >= 45 && bCol <= 56)
            {
                chs = 1;
                isOK = 1;
                drawButton(26, 13, 0x9F, 4, sysMenu[0], 1, 0x94);
                clearScreen(45, 13, 57, 14, 4);
                writeText(46, 13, 0xB4, sysMenu[3], 0xB1);
                delay(60);
                drawButton(45, 13, 0xB4, 4, sysMenu[3], 1, 0xB1);
            }

            if (bRow == 8 && bCol == 15 || bCol == 16) isOK = 1;
        }
    } while (!isOK);

    if (!chs && !slc) sysReboot();
    else cleanup();
}

/*----------------------------------------*/
/* Funtion : chooseDrive                  */
/* Purpose : Showing choose drive message */
/* Expects : Nothing                      */
/* Returns : Nothing                      */
/*----------------------------------------*/
void chooseDrive()
{
    uint8_t i = 0, k = 0;
    uint8_t isASCII = 0, isOK = 0;
    const char drives[] = {'B', 'C', 'D', 'E'};

    setCursorSize(0x2020);
    fillFrame(1, 1, 80, 25, 0xF6, 178);
    shadowBox(20, 8, 60, 18, 0x3F, sysInfo[7]);
    writeText(31, 10, 0x34, sysInfo[24], 0);
    writeChar(29, 11, 0x34, 22, 193);
    for (i = 0; i < sizeof(drives) / sizeof(drives[0]); i++) writeText(29, 12 + i, 0x30, sysMenu[12 + i], 0x3A);
    drawButton(46, 12, _wATV, 3, sysMenu[1], 1, _wATV);
    drawButton(46, 14, _wATV, 3, sysMenu[4], 1, _wFLT);
    writeText(23, 17, 0x30, sysMenu[22], 0);
    writeChar(33, 17, 0x1F, 25, 32);
    moveMouse(38, 11);

    slc = chs = key = 0;
    strcpy(szInstallPath, INST_DIR);
    writeText(29, 12 + slc, 0x3F, sysMenu[12 + slc], 0x3E);
    
    while (kbhit()) getch();
    do {
        key = 0;
        if (kbhit())
        {
            key = getch();
            if (!key) key = getch();
            switch (key)
            {
            case UP:
                writeText(29, 12 + slc, 0x30, sysMenu[12 + slc], 0x3A);
                if (slc < 1) slc = 3; else slc--;
                writeText(29, 12 + slc, 0x3F, sysMenu[12 + slc], 0x3E);
                break;
            case DOWN:
                writeText(29, 12 + slc, 0x30, sysMenu[12 + slc], 0x3A);
                if (slc >= 3) slc = 0; else slc++;
                writeText(29, 12 + slc, 0x3F, sysMenu[12 + slc], 0x3E);
                break;
            case HOME:
                slc = 0;
                writeText(29, 12 + slc, 0x30, sysMenu[12 + slc], 0x3A);
                writeText(29, 12 + slc, 0x3F, sysMenu[12 + slc], 0x3E);
                break;
            case END:
                slc = 3;
                writeText(29, 12 + slc, 0x30, sysMenu[12 + slc], 0x3A);
                writeText(29, 12 + slc, 0x3F, sysMenu[12 + slc], 0x3E);
                break;
            case TAB:
                key = 0;
                drawButton(46, 12 + 2 * chs, wATV, 3, sysMenu[3 * chs + 1], 1, wFLT);
                do {
                    if (kbhit())
                    {
                        key = getch();
                        if (!key) key = getch();
                        switch (key)
                        {
                        case UP:
                            drawButton(46, 12 + 2 * chs, _wATV, 3, sysMenu[3 * chs + 1], 1, _wFLT);
                            if (chs < 1) chs = 1; else chs--;
                            drawButton(46, 12 + 2 * chs, wATV, 3, sysMenu[3 * chs + 1], 1, wFLT);
                            break;
                        case DOWN:
                            drawButton(46, 12 + 2 * chs, _wATV, 3, sysMenu[3 * chs + 1], 1, _wFLT);
                            if (chs > 0) chs = 0; else chs++;
                            drawButton(46, 12 + 2 * chs, wATV, 3, sysMenu[3 * chs + 1], 1, wFLT);
                            break;
                        case ENTER:
                            clearScreen(46, 12 + 2 * chs, 57, 13 + 2 * chs, 3);
                            writeText(47, 12 + 2 * chs, wATV, sysMenu[3 * chs + 1], wFLT);
                            delay(60);
                            drawButton(46, 12 + 2 * chs, wATV, 3, sysMenu[3 * chs + 1], 1, wFLT);
                            if (!chs) isOK = 1;
                            else
                            {
                                getScreenText(20, 10, 43, 9, szScreen);
                                msgSlc = messageBox(20, 10, 60, 16, msgExit, sizeof(msgExit) / sizeof(msgExit[0]));
                                if (!msgSlc) cleanup();
                                putScreenText(20, 10, 43, 9, szScreen);
                            }
                            break;
                        case TAB:
                            key = 0;
                            k = strlen(szInstallPath);
                            do {
                                setCursorSize(0x0B0A);
                                setCursorPos(33 + k, 17);
                                if (kbhit())
                                {
                                    isASCII = readKey(&key);
                                    if (!key) isASCII = readKey(&key);
                                    if (((isASCII && isalpha(key)) || key == '\\' || isdigit(key)) && k < 20)
                                    {
                                        szInstallPath[k] = key;
                                        printChar(33 + k, 17, 0x1E, key);
                                        k++;
                                    }

                                    if (key == DEL && k > 3)
                                    {
                                        k--;
                                        printChar(33 + k, 17, 0x1E, 32);
                                    }

                                    szInstallPath[k] = '\0';
                                }
                            } while (key != ENTER && key != TAB);
                            setCursorSize(0x2020);
                            break;
                        }
                    }
                } while (!isOK && key != TAB && key != ENTER);
                break;
            }
        }

        if (clickMouse(&bCol, &bRow))
        {
            if (bRow == 12 && bCol >= 29 && bCol <= 41)
            {
                writeText(29, 12, 0x3F, sysMenu[12], 0x3E);
                if (slc != 0) writeText(29, 12 + slc, 0x30, sysMenu[12 + slc], 0x3A);
                delay(20);
                slc = 0;
            }

            if (bRow == 13 && bCol >= 29 && bCol <= 41)
            {
                writeText(29, 13, 0x3F, sysMenu[13], 0x3E);
                if (slc != 1) writeText(29, 12 + slc, 0x30, sysMenu[12 + slc], 0x3A);
                delay(20);
                slc = 1;
            }

            if (bRow == 14 && bCol >= 29 && bCol <= 41)
            {
                writeText(29, 14, 0x3F, sysMenu[14], 0x3E);
                if (slc != 2) writeText(29, 12 + slc, 0x30, sysMenu[12 + slc], 0x3A);
                delay(20);
                slc = 2;
            }

            if (bRow == 15 && bCol >= 29 && bCol <= 41)
            {
                writeText(29, 15, 0x3F, sysMenu[15], 0x3E);
                if (slc != 3) writeText(29, 12 + slc, 0x30, sysMenu[12 + slc], 0x3A);
                delay(20);
                slc = 3;
            }

            if (bRow == 12 && bCol >= 46 & bCol <= 57)
            {
                chs = 0;
                isOK = 1;
                drawButton(46, 14, _wATV, 3, sysMenu[4], 1, _wFLT);
                clearScreen(46, 12, 57, 13, 3);
                writeText(47, 12, wATV, sysMenu[1], wFLT);
                delay(60);
                drawButton(46, 12, wATV, 3, sysMenu[1], 1, wFLT);
            }

            if (bRow == 14 && bCol >= 46 & bCol <= 57)
            {
                chs = 1;
                drawButton(46, 12, _wATV, 3, sysMenu[1], 1, _wFLT);
                clearScreen(46, 14, 57, 16, 3);
                writeText(47, 15, wATV, sysMenu[4], wFLT);
                delay(60);
                drawButton(46, 14, wATV, 3, sysMenu[4], 1, wFLT);
                getScreenText(20, 10, 43, 9, szScreen);
                msgSlc = messageBox(20, 10, 60, 16, msgExit, sizeof(msgExit) / sizeof(msgExit[0]));
                if (!msgSlc) cleanup();
                putScreenText(20, 10, 43, 9, szScreen);
            }

            if (bRow == 17 && bCol >= 33 && bCol <= 53)
            {
                key = 0;
                k = strlen(szInstallPath);
                do {
                    setCursorSize(0x0B0A);
                    setCursorPos(33 + k, 17);
                    if (kbhit())
                    {
                        isASCII = readKey(&key);
                        if (!key) isASCII = readKey(&key);
                        if (((isASCII && isalpha(key)) || key == '\\' || isdigit(key)) && k < 20)
                        {
                            szInstallPath[k] = key;
                            printChar(33 + k, 17, 0x1E, key);
                            k++;
                        }

                        if (key == DEL && k > 3)
                        {
                            k--;
                            printChar(33 + k, 17, 0x1E, 32);
                        }

                        szInstallPath[k] = '\0';
                    }
                } while (key != ENTER && key != TAB);
                setCursorSize(0x2020);                
            }
        }

        szInstallPath[0] = drives[slc];
        writeText(33, 17, 0x1E, szInstallPath, 0);
    } while (!isOK);
}

/*----------------------------------------------------*/
/* Funtion : updateProgram                            */
/* Purpose : Update register info and delete files    */
/* Expects : Nothing                                  */
/* Returns : Nothing                                  */
/*----------------------------------------------------*/
void updateProgram()
{
    FILE *fp;
    regs_t regInfo;
    struct dosdate_t date;

    fp = fopen(sysInfo[42], "wb");
    if (!fp)
    {
        setBorder(47);
        setCursorSize(0x2020);
        clearScreen(1, 1, 80, 25, 1);
        writeText(33, 10, 0x4F, sysInfo[11], 0);
        writeText(20, 12, 0x1F, sysInfo[19], 0);
        writeText(20, 13, 0x1F, sysInfo[25], 0);
        getch();
        cleanup();
    }

    memset(&regInfo, 0, sizeof(regInfo));
    _dos_getdate(&date);
    strcat(szInstallPath, "\\");
    regInfo.year = date.year;
    regInfo.month = date.month;
    regInfo.day = date.day;
    regInfo.days = MAX_DAYS;
    strcpy(regInfo.path, szInstallPath);
    fwrite(&regInfo, sizeof(regs_t), 1, fp);
    fclose(fp);
}

/*--------------------------------------------*/
/* Function : getDriveId                      */
/* Purpose  : Return the number of the drier  */
/* Expects  : (szDrive) Serial number szDrive */
/* Returns  : number of szDrive               */
/*--------------------------------------------*/
uint8_t getDriveId(char drive)
{
    return toupper(drive) - 'A' + 1;
}

/*--------------------------------------------------*/
/* Funtion : checkDiskSpace                         */
/* Purpose : Checking disk space and available      */
/* Expects : Nothing                                */
/* Returns : Nothing                                */
/*--------------------------------------------------*/
void checkDiskSpace()
{
    struct diskfree_t dsk;
    uint8_t i = 0, isOK = 0;
    double freeSpace, totalSpace;
        
    setBorder(50);
    setCursorSize(0x2020);
    fillFrame(1, 1, 80, 25, 0xF6, 178);
    shadowBox(15, 5, 65, 20, 0x7F, sysInfo[8]);
    writeText(18, 7, 0x70, sysInfo[39], 0);
    writeText(18, 8, 0x70, sysInfo[40], 0);
    writeText(18, 10, 0x70, sysInfo[29], 0);
    writeText(53, 12, 0x70, sysInfo[171], 0);
    writeChar(18, 11, 0x17, 45, 176);
    drawButton(36, 18, wATV, 7, sysMenu[2], 1, wFLT);
    moveMouse(39, 17);

    while (kbhit()) getch();
    for (i = 0; i < 45; i++)
    {
        writeChar(18 + i, 11, 0x3E, 1, 219);
        printText(50, 12, 0x70, "%3u", 2 * i + 12);

        if (kbhit() || clickMouse(&bCol, &bRow))
        {
            if (kbhit() || (bRow == 18 && bCol >= 36 && bCol <= 45))
            {
                clearScreen(36, 18, 45, 19, 7);
                writeText(37, 18, wATV, sysMenu[2], wATV);
                delay(60);
                drawButton(36, 18, wATV, 7, sysMenu[2], 1, wFLT);
                break;
            }
        }
        delay(60);
    }

    writeText(18, 14, 0x70, sysInfo[30], 0);
    writeChar(18, 15, 0x17, 45, 176);
    writeText(53, 16, 0x70, sysInfo[171], 0);

    while (kbhit()) getch();
    for (i = 0; i < 45; i++)
    {
        writeChar(18 + i, 15, 0x3E, 1, 219);
        printText(50, 16, 0x70, "%3u",  2 * i + 12);

        if (kbhit() || clickMouse(&bCol, &bRow))
        {
            if (kbhit() || (bRow == 18 && bCol >= 36 && bCol <= 45))
            {
                clearScreen(36, 18, 45, 19, 7);
                writeText(37, 18, wATV, sysMenu[2], wATV);
                delay(60);
                drawButton(36, 18, wATV, 7, sysMenu[2], 1, wFLT);
                break;
            }
        }
        delay(60);
    }

    if (_dos_getdiskfree(getDriveId(szInstallPath[0]), &dsk))
    {
        shadowBox(20, 9, 55, 15, 0x4F, sysInfo[1]);
        writeText(22, 11, 0x4F, sysInfo[37], 0);
        drawButton(33, 13, wATV, 4, sysMenu[0], 1, wFLT);

        while (kbhit()) getch();
        do {
            if (kbhit() || clickMouse(&bCol, &bRow))
            {
                if (kbhit() || (bRow == 13 && bCol >= 33 && bCol <= 43))
                {
                    isOK = 1;
                    clearScreen(33, 13, 43, 14, 4);
                    writeText(34, 13, wATV, sysMenu[0], wFLT);
                    delay(60);
                    drawButton(33, 13, wATV, 4, sysMenu[0], 1, wFLT);
                }

                if (bRow == 9 && bCol == 20 || bCol == 21) isOK = 1;
            }
        } while (!isOK);
        cleanup();
    }

    totalSpace = ((1.0 * dsk.total_clusters * dsk.sectors_per_cluster * dsk.bytes_per_sector) / 1024) / 1024;
    freeSpace = ((1.0 * dsk.avail_clusters * dsk.sectors_per_cluster * dsk.bytes_per_sector) / 1024) / 1024;

    fillFrame(1, 1, 80, 25, 0xF6, 178);
    shadowBox(15, 9, 63, 16, 0x9F, sysInfo[9]);
    printText(20, 11, 0x9E, sysInfo[172], totalSpace);
    printText(20, 12, 0x9E, sysInfo[173], freeSpace);
    drawButton(34, 14, wATV, 9, sysMenu[0], 1, wFLT);
    moveMouse(38, 12);

    if (freeSpace < 5) writeText(18, 13, 0x9C, sysInfo[35], 0);

    while (kbhit()) getch();
    do {
        if (kbhit() || clickMouse(&bCol, &bRow))
        {
            if (kbhit() || (bRow == 14 && bCol >= 34 && bCol <= 41))
            {
                isOK = 1;
                clearScreen(34, 14, 42, 15, 1);
                writeText(35, 14, wATV, sysMenu[0], wFLT);
                delay(60);
                drawButton(34, 14, wATV, 9, sysMenu[0], 1, wFLT);
            }

            if (bRow == 9 && bCol == 15 || bCol == 16) isOK = 1;
        }
    } while (!isOK);

    if (freeSpace < 5) cleanup();
}

/*------------------------------------*/
/* Funtion : startInstall             */
/* Purpose : Executting the functions */
/* Expects : Nothing                  */
/* Returns : Nothing                  */
/*------------------------------------*/
void startInstall()
{
    chooseDrive();
    checkDiskSpace();
    checkProductKey();
    installProgram();
    updateProgram();
    showRegisterInfo();
    fadeOut();
    system("readme");
    restartSystem();
}

/*---------------------------------------------*/
/* Funtion : showInstall                       */
/* Purpose : Showing information setup program */
/* Expects : Nothing                           */
/* Returns : Nothing                           */
/*---------------------------------------------*/
void showInstall()
{
    uint8_t fltStop[3] = {0};
    uint8_t i = 0, isOK = 0;

    memset(fltStop, 0, 3);
    setBorder(59);
    setCursorSize(0x0B0A);
    setBlinking(0);
    fillFrame(1, 1, 80, 25, 0xFD, 178);
    shadowBox(8, 2, 74, 23, 0x1F, sysInfo[74]);
    writeText(13, 4, 0x1A, sysInfo[10], 0);
    writeChar(11, 5, 0x1B, 61, 205);
    writeText(12, 6, 0x1B, sysInfo[36], 0);
    writeText(23, 8, 0x1F, sysInfo[44], 0);
    writeText(23, 9, 0x1F, sysInfo[45], 0);
    writeText(23, 10, 0x1F, sysInfo[46], 0);
    writeText(23, 12, 0x1E, sysInfo[47], 0);
    writeText(23, 13, 0x1E, sysInfo[48], 0);
    writeText(23, 14, 0x1E, sysInfo[49], 0);
    writeText(23, 16, 0x1B, sysInfo[50], 0);
    writeText(23, 17, 0x1B, sysInfo[51], 0);
    writeText(23, 18, 0x1B, sysInfo[52], 0);
    writeChar(9, 19, 0x1F, 65, 196);
    printChar(8, 19, 0x1F, 195);
    printChar(74, 19, 0x1F, 180);
    writeText(14, 11, 0x1C, sysMenu[17], 0x1E);
    writeText(14, 15, 0x1C, sysMenu[18], 0x1E);
    
    if (!initMouse()) system(sysInfo[43]);
    setMousePos();
    
    drawButton(24, 21, wATV, 1, sysMenu[1], 1, wFLT);
    drawButton(48, 21, _wATV, 1, sysMenu[4], 1, _wFLT);
    
    key = slc = 0;
    writeText(14, 7, 0x1A, sysMenu[16], 0x1F);

    while (kbhit()) getch();
    do {
        setCursorSize(0x0B0A);
        setCursorPos(15, 7 + 4 * slc);
        if (kbhit())
        {
            key = getch();
            if (!key) key = getch();
            switch (toupper(key))
            {
            case UP:
                if (!fltStop[slc]) writeText(14, 7 + 4 * slc, 0x1C, sysMenu[16 + slc], 0x1E);
                else
                {
                    writeText(14, 7 + 4 * slc, 0x1C, sysMenu[16 + slc], 0x1E);
                    printChar(15, 7 + 4 * slc, 0x1C, 251);
                }

                if (slc < 1) slc = 2; else slc--;
                
                if (!fltStop[slc]) writeText(14, 7 + 4 * slc, 0x1A, sysMenu[16 + slc], 0x1F);
                else
                {
                    writeText(14, 7 + 4 * slc, 0x1A, sysMenu[16 + slc], 0x1F);
                    printChar(15, 7 + 4 * slc, 0x1A, 251);
                }
                setCursorPos(15, 7 + 4 * slc);
                break;
            case DOWN:
                if (!fltStop[slc]) writeText(14, 7 + 4 * slc, 0x1C, sysMenu[16 + slc], 0x1E);
                else
                {
                    writeText(14, 7 + 4 * slc, 0x1C, sysMenu[16 + slc], 0x1E);
                    printChar(15, 7 + 4 * slc, 0x1C, 251);
                }

                if (slc >= 2) slc = 0; else slc++;

                if (!fltStop[slc]) writeText(14, 7 + 4 * slc, 0x1A, sysMenu[16 + slc], 0x1F);
                else
                {
                    writeText(14, 7 + 4 * slc, 0x1A, sysMenu[16 + slc], 0x1F);
                    printChar(15, 7 + 4 * slc, 0x1A, 251);
                }
                setCursorPos(15, 7 + 4 * slc);
                break;
            case HOME:
                if (!fltStop[slc]) writeText(14, 7 + 4 * slc, 0x1C, sysMenu[16 + slc], 0x1E);
                else
                {
                    writeText(14, 7 + 4 * slc, 0x1C, sysMenu[16 + slc], 0x1E);
                    printChar(15, 7 + 4 * slc, 0x1C, 251);
                }
                
                slc = 0;
                if (!fltStop[slc]) writeText(14, 7 + 4 * slc, 0x1A, sysMenu[16 + slc], 0x1F);
                else
                {
                    writeText(14, 7 + 4 * slc, 0x1A, sysMenu[16 + slc], 0x1F);
                    printChar(15, 7 + 4 * slc, 0x1A, 251);
                }
                setCursorPos(15, 7 + 4 * slc);
                break;
            case END:
                if (!fltStop[slc]) writeText(14, 7 + 4 * slc, 0x1C, sysMenu[16 + slc], 0x1E);
                else
                {
                    writeText(14, 7 + 4 * slc, 0x1C, sysMenu[16 + slc], 0x1E);
                    printChar(15, 7 + 4 * slc, 0x1C, 251);
                }
                
                slc = 2;
                if (!fltStop[slc]) writeText(14, 7 + 4 * slc, 0x1A, sysMenu[16 + slc], 0x1F);
                else
                {
                    writeText(14, 7 + 4 * slc, 0x1A, sysMenu[16 + slc], 0x1F);
                    printChar(15, 7 + 4 * slc, 0x1A, 251);
                }
                setCursorPos(15, 7 + 4 * slc);
                break;
            case LEFT:
                drawButton(24 + chs * 24, 21, _wATV, 1, sysMenu[3 * chs + 1], 1, _wFLT);
                if (chs < 1) chs = 1; else chs--;
                drawButton(24 + chs * 24, 21, wATV, 1, sysMenu[3 * chs + 1], 1, wFLT);
                break;
            case RIGHT:
                drawButton(24 + chs * 24, 21, _wATV, 1, sysMenu[3 * chs + 1], 1, _wFLT);
                if (chs > 0) chs = 0; else chs++;
                drawButton(24 + chs * 24, 21, wATV, 1, sysMenu[3 * chs + 1], 1, wFLT);
                break;
            case SPACE:
                if (fltStop[slc])
                {
                    writeText(14, 7 + 4 * slc, 0x1A, sysMenu[16 + slc], 0x1F);
                    fltStop[slc] = 0;
                }
                else
                {
                    printChar(15, 7 + 4 * slc, 0x1A, 251);
                    fltStop[slc] = 1;
                }
                break;
            case ENTER:
                clearScreen(24 + chs * 24, 21, 37 + chs * 24, 22, 1);
                writeText(25 + chs * 24, 21, wATV, sysMenu[3 * chs + 1], wFLT);
                delay(60);
                drawButton(24 + chs * 24, 21, wATV, 1, sysMenu[3 * chs + 1], 1, wFLT);
                if (!chs)
                {
                    if (fltStop[1] && fltStop[2]) isOK = 1;
                    else
                    {
                        getScreenText(20, 10, 44, 9, szScreen);
                        msgSlc = messageBox(20, 10, 61, 17, msgError, sizeof(msgError) / sizeof(msgError[0]));
                        if (msgSlc) cleanup();
                        putScreenText(20, 10, 44, 9, szScreen);
                    }
                }
                else
                {
                    getScreenText(20, 10, 44, 9, szScreen);
                    msgSlc = messageBox(20, 10, 61, 16, msgExit, sizeof(msgExit) / sizeof(msgExit[0]));
                    if (!msgSlc) cleanup();
                    putScreenText(20, 10, 44, 9, szScreen);
                }
                break;
            }
        }

        if (clickMouse(&bCol, &bRow))
        {
            if (bRow == 7 && bCol >= 14 && bCol <= 48)
            {
                if (!fltStop[slc]) writeText(14, 7 + 4 * slc, 0x1C, sysMenu[16 + slc], 0x1E);
                else
                {
                    writeText(14, 7 + 4 * slc, 0x1C, sysMenu[16 + slc], 0x1E);
                    printChar(15, 7 + 4 * slc, 0x1C, 251);
                }
                
                slc = 0;
                if (!fltStop[slc])
                {
                    writeText(14, 7, 0x1A, sysMenu[16], 0x1F);
                    printChar(15, 7, 0x1A, 251); fltStop[0] = 1;
                }
                else
                {
                    writeText(14, 7, 0x1A, sysMenu[16], 0x1F);
                    fltStop[0] = 0;
                }

                setCursorPos(15, 7);
                delay(150);
            }

            if (bRow == 11 && bCol >= 14 && bCol <= 57)
            {
                if (!fltStop[slc]) writeText(14, 7 + 4 * slc, 0x1C, sysMenu[16 + slc], 0x1E);
                else
                {
                    writeText(14, 7 + 4 * slc, 0x1C, sysMenu[16 + slc], 0x1E);
                    printChar(15, 7 + 4 * slc, 0x1C, 251);
                }
                
                slc = 1;
                if (!fltStop[slc])
                {
                    writeText(14, 11, 0x1A, sysMenu[17], 0x1F);
                    printChar(15, 11, 0x1A, 251); fltStop[1] = 1;
                }
                else
                {
                    writeText(14, 11, 0x1A, sysMenu[17], 0x1F);
                    fltStop[1] = 0;
                }

                setCursorPos(15, 11);
                delay(150);
            }

            if (bRow == 15 && bCol >= 14 && bCol <= 52)
            {
                if (!fltStop[slc]) writeText(14, 7 + 4 * slc, 0x1C, sysMenu[16 + slc], 0x1E);
                else
                {
                    writeText(14, 7 + 4 * slc, 0x1C, sysMenu[16 + slc], 0x1E);
                    printChar(15, 7 + 4 * slc, 0x1C, 251);
                }
                
                slc = 2;
                if (!fltStop[slc])
                {
                    writeText(14, 15, 0x1A, sysMenu[18], 0x1F);
                    printChar(15, 15, 0x1A, 251); fltStop[2] = 1;
                }
                else
                {
                    writeText(14, 15, 0x1A, sysMenu[18], 0x1F);
                    fltStop[2] = 0;
                }

                setCursorPos(15, 15);
                delay(150);
            }

            if (bRow == 21 && bCol >= 24 && bCol <= 36)
            {
                drawButton(48, 21, _wATV, 1, sysMenu[4], 1, _wFLT);
                clearScreen(24, 21, 37, 22, 1);
                writeText(25, 21, wATV, sysMenu[1], wFLT);
                delay(60);
                drawButton(24, 21, wATV, 1, sysMenu[1], 1, wFLT);
                chs = 0;
                if (fltStop[1] && fltStop[2]) isOK = 1;
                else
                {
                    getScreenText(20, 10, 44, 9, szScreen);
                    msgSlc = messageBox(20, 10, 61, 17, msgError, sizeof(msgError) / sizeof(msgError[0]));
                    if (msgSlc) cleanup();
                    putScreenText(20, 10, 44, 9, szScreen);
                }
            }

            if (bRow == 21 && bCol >= 48 && bCol <= 60)
            {
                drawButton(24, 21, _wATV, 1, sysMenu[1], 1, _wFLT);
                clearScreen(48, 21, 61, 22, 1);
                writeText(49, 21, wATV, sysMenu[4], wFLT);
                delay(60);
                drawButton(48, 21, wATV, 1, sysMenu[4], 1, wFLT);
                chs = 1;
                getScreenText(20, 10, 44, 9, szScreen);
                msgSlc = messageBox(20, 10, 61, 16, msgExit, sizeof(msgExit) / sizeof(msgExit[0]));
                if (!msgSlc) cleanup();
                putScreenText(20, 10, 44, 9, szScreen);
            }
        }
    } while (!isOK);

    startInstall();
}

/*-------------------------------------------*/
/* Funtion : main                            */
/* Purpose : Linker funtions to run program  */
/* Expects : Nothing                         */
/* Returns : Nothing                         */
/*-------------------------------------------*/

void main()
{
    initData();
    showInstall();
}
