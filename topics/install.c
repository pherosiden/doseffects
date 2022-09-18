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

#define SCR_WIDTH       160
#define MASK_BG         0x08
#define OFFSET(x, y)    (((x - 1) + (y - 1) * 80) << 1)

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

#define BOOT_SEG        0xFFFFL
#define BOOT_OFF        0x0000L
#define BOOT_ADR        ((BOOT_SEG << 16) | BOOT_OFF)
#define DOS_SEG	        0x0040L
#define RESET_FLAG      0x0072L
#define RESET_ADR	    ((DOS_SEG << 16) | RESET_FLAG)

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

char szInstallPath[32];      // Drive letter

uint16_t numFiles = 0;
uint16_t totalFiles = 0;    // Number files to read and files to write
uint8_t bmAvalid = 0;       // Status of the mouse

char **sysInfo, **sysMenu;
uint16_t infoNum, menuNum;

char *msgWarn[1], *msgExit[1];
char *msgCmp[1], *msgError[2];

char key = 0;
char szScreen[792] = {0};
uint8_t msgSlc = 0, chs = 0, slc = 0;
uint16_t bCol = 0, bRow = 0;
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
    txtMem[OFFSET(x, y)] = chr;
    txtMem[OFFSET(x, y) + 1] = attr;
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
    uint16_t rwVideoOFS;

    rwVideoOFS = OFFSET(x, y);
    for (i = 0; i < len; i++)
    {
        txtMem[rwVideoOFS    ] = chr;
        txtMem[rwVideoOFS + 1] = attr;
        rwVideoOFS += 2;
    }
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
    char *szTmp;
    uint8_t i = 0, fltStop = 0, currX = x, bPos;

    if (fstAttr)
    {
        szTmp = (char*)malloc(strlen(szPrmt) + 1);
        if (!szTmp) return;

        strcpy(szTmp, szPrmt);
        for (i = 0; (i < strlen(szTmp) - 1) && !fltStop; i++)
        {
            if (szTmp[i] == 126) fltStop = 1;
        }

        memmove(&szTmp[i - 1], &szTmp[i], strlen(szTmp) - i + 1);
        bPos = i - 1;

        i = 0;
        while (szTmp[i])
        {
            txtMem[OFFSET(x, y)] = szTmp[i++];
            txtMem[OFFSET(x++, y) + 1] = txtAtr;
        }

        printChar(currX + bPos, y, fstAttr, szTmp[bPos]);
        free(szTmp);
    }
    else
    {
        while (*szPrmt)
        {
            txtMem[OFFSET(x, y)] = *szPrmt++;
            txtMem[OFFSET(x++, y) + 1] = txtAtr;
        }
    }
}

/*-----------------------------------------------*/
/* Function : printVRM                           */
/* Purpose  : Writing a character with attribute */
/* Notices  : Intervention in video memory       */
/* Expects  : (x,y) cordinate needs to writting  */
/*            (attr) The attribute of string     */
/*            (szString) the string to format    */
/* Returns : Nothing                             */
/*-----------------------------------------------*/
void printVRM(uint8_t x, uint8_t y, uint8_t wAttr, char *szString, ...)
{
    char buffer[255];
    va_list params;
    va_start(params, szString);
    vsprintf(buffer, szString, params);
    writeVRM(x, y, wAttr, buffer, 0);
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
void drawButton(uint8_t x, uint8_t y, uint8_t txtAttr, uint8_t bkClr, char *szTitle, uint8_t bType, uint8_t fstAttr)
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
/* Purpose  : Modify the attribute of screen text    */
/* Expects  : (x1,y1) cordinate top to left          */
/*            (x2,y2) cordinate right to bottom      */
/*            (wAttr) the attribute                  */
/* Returns  : Nothing                                */
/*---------------------------------------------------*/
void changeAttrib(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t wAttr)
{
    uint8_t col, row;
    for (col = x1; col <= x2; col++)
    {
        for (row = y1; row <= y2; row++) txtMem[OFFSET(col, row) + 1] = wAttr;
    }
}

/*-----------------------------------------------*/
/* Function : setBlinking                        */
/* Purpose  : Set bits 7 in the text attribute   */
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
    char buff[3] = {0};
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

/*----------------------------------------------*/
/* Funtion : getScreenText                      */
/* Purpose : Get string at coordinate           */
/* Expects : (x, y, width, height) screen coord */
/*           buff the buffer to store data      */
/* Returns : Nothing                            */
/*----------------------------------------------*/
void getScreenText(int16_t x, int16_t y, int16_t width, int16_t height, uint8_t *buff)
{
    uint16_t bytes = width << 1;
    //uint16_t far *dst = (uint16_t far*)buff;
    uint8_t far *src = txtMem + OFFSET(x, y);
    
    while (height--)
    {
        _fmemcpy(buff, src, bytes);
        buff += bytes;
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
    uint16_t bytes = width << 1;
    uint8_t far *dst = txtMem + OFFSET(x, y);
    
    while (height--)
    {
        _fmemcpy(dst, buff, bytes);
        buff += bytes;
        dst += SCR_WIDTH;
    }
}

/*-------------------------------------*/
/* Funtion : decodeFile                */
/* Purpose : Decode the language files */
/* Expects : (inFile) The source file  */
/*           (outFile) The dest file   */
/* Returns : Number of lines in file   */
/*-------------------------------------*/
uint16_t decodeFile(const char *inFile, const char *outFile)
{
    int16_t c, key = 98;
    uint16_t linesCount = 0;
    FILE *inHandle, *outHandle;
    
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
/* Function : getTextFile                         */
/* Purpose  : Reading information into data array */
/* Expects  : (inFile) the input file             */
/*            (outFile) the output file           */
/*            (szData) the array data             */
/*            (wNumElm) the number of elements    */
/* Returns  : Nothing                             */
/*------------------------------------------------*/
void getTextFile(const char *inFile, const char *outFile, char ***szData, uint16_t *wNumElm)
{
    FILE *fp;
    uint16_t elems = 0;
    char szBuffer[102] = {0};
    
    elems = decodeFile(inFile, outFile);
    szData[0] = (char**)malloc(elems * sizeof(char*));
    if (!(szData[0]))
    {
        fprintf(stderr, "Not enough memory for count: %d\n", elems);
        exit(1);
    }

    fp = fopen(outFile, "rt");
    if (!fp)
    {
        fprintf(stderr, "Error open file: %s", outFile);
        exit(1);   
    }

    elems = 0;
    while (fgets(szBuffer, 102, fp))
    {
        fontVNI(szBuffer);
        szData[0][elems] = (char*)malloc(strlen(szBuffer) + 1);
        if (!(szData[0][elems]))
        {
            fprintf(stderr, "Not enough memory at line: %u", elems);
            exit(1);
        }
        szBuffer[strlen(szBuffer) - 1] = '\0';
        strcpy(szData[0][elems], szBuffer);
        elems++;
    }

    *wNumElm = elems;
    fclose(fp);
    unlink(outFile);
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
    void((far* reset_ptr)()) = (void(far*)())BOOT_ADR;
    *(int16_t far*)RESET_ADR = 0;
    (*reset_ptr)();
}

/*---------------------------------------*/
/* Funtion : errorFile                   */
/* Purpose : Display the error code      */
/* Expects : (handle) which file error   */
/*           (errortype) error file type */
/* Returns : Nothing                     */
/*---------------------------------------*/
void errorFile(char *szHandle, char *szErrorType)
{
    char errorCode[60];
    strcpy(errorCode, szErrorType);
    strcat(errorCode, szHandle);
    shadowBox(13, 8, 65, 17, 0x4F, sysInfo[1]);
    drawButton(33, 15, 0xF0, 4, sysMenu[3], 1, 0xF4);
    writeVRM(39 - strlen(errorCode) / 2, 10, 0x4A, errorCode, 0);
    writeVRM(15, 11, 0x4F, sysInfo[53], 0);
    writeVRM(15, 12, 0x4F, sysInfo[54], 0);
    writeVRM(15, 13, 0x4F, sysInfo[55], 0);

    do {
        if (clickMouse(&bCol, &bRow))
        {
            if (bRow == 15 && bCol >= 33 && bCol <= 45)
            {
                hideMouse();
                clearScreen(33, 15, 46, 16, 4);
                writeVRM(34, 15, wATV, sysMenu[3], wFLT);
                delay(50);
                drawButton(33, 15, wATV, 4, sysMenu[3], 1, wFLT);
                showMouse();
                break;
            }
            if (bRow == 8 && bCol == 13 || bCol == 14) break;
        }
    } while (!kbhit());
    cleanup();
}

/*---------------------------------------------*/
/* Funtion : countFiles                        */
/* Purpose : Count total files from directory  */
/* Expects : (szDir) sources directory         */
/* Returns : Number of files in directory      */
/*---------------------------------------------*/
void countFiles(char *szDir)
{
    size_t i = 0;
    struct find_t entries;
    char srcPath[64], srcExt[64], srcDir[64];

    for (i = strlen(szDir) - 1; szDir[i] != '\\'; i--);

    strcpy(srcPath, szDir);
    srcPath[i] = '\0';
    strcpy(srcExt, &szDir[i + 1]);

    if (!_dos_findfirst(szDir, _A_NORMAL | _A_RDONLY | _A_HIDDEN | _A_SYSTEM, &entries))
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
/* Expects : (szSrc) sources file path         */
/*           (szDst) destination file path     */
/*           (fileInfo) file attributes        */
/* Returns : 1 for success                     */
/*           0 for failure                     */
/*---------------------------------------------*/
uint8_t copyFile(char *szSrc, char *szDst, struct find_t *fileInfo)
{
    int srcHandle, dstHandle;
    size_t numBytes = bytesCount;
    
    if (_dos_open(szSrc, O_RDONLY, &srcHandle)) errorFile(szSrc, sysInfo[17]);
    if (_dos_creat(szDst, fileInfo->attrib, &dstHandle)) errorFile(szDst, sysInfo[17]);

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
/* Expects : (szSourceDir) sources directory   */
/*           (szDestDir) destination directory */
/*           (wAttr) file attribute            */
/* Returns : 1 for success                     */
/*           0 for failure                     */
/*---------------------------------------------*/
void processDir(char *szSourceDir, char *szDestDir)
{
    size_t i;
    struct find_t entries;
    
    char srcPath[64], srcExt[64], srcDir[64];
    char curFile[68], newFile[68], newDir[64];

    for (i = strlen(szSourceDir) - 1; szSourceDir[i] != '\\'; i--);

    strcpy(srcPath, szSourceDir);
    srcPath[i] = '\0';
    strcpy(srcExt, &szSourceDir[i + 1]);

    if (!_dos_findfirst(szSourceDir, _A_NORMAL | _A_RDONLY | _A_HIDDEN | _A_SYSTEM, &entries))
    {
        do {
            sprintf(curFile, "%s\\%s", srcPath, entries.name);
            sprintf(newFile, "%s\\%s", szDestDir, entries.name);
            if (!copyFile(curFile, newFile, &entries)) errorFile(newFile, sysInfo[21]);

            numFiles++;
            writeChar(30, 11, 0x99, 33, 32);
            writeVRM(30, 11, 0x9E, newFile, 0);
            writeChar(18, 12, 0xFF, 45 * (1.0 * numFiles / totalFiles), 219);
            printVRM(50, 13, 0x9F, "%3d", (int16_t)(100 * (1.0 * numFiles / totalFiles)));
            delay(50);
        } while (!_dos_findnext(&entries));
    }

    sprintf(srcDir, "%s\\*.*", srcPath);
    if (!_dos_findfirst(srcDir, _A_SUBDIR | _A_NORMAL | _A_RDONLY | _A_HIDDEN | _A_SYSTEM, &entries))
    {
        do {
            if ((entries.attrib & _A_SUBDIR) && (entries.name[0] != '.'))
            {
                sprintf(srcDir, "%s\\%s\\%s", srcPath, entries.name, srcExt);
                sprintf(newDir, "%s\\%s", szDestDir, entries.name);
                mkdir(newDir);
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
/*            (szMsg) The messages array */
/*            (n) The elements of array  */
/* Return   : 1 or 0                     */
/*---------------------------------------*/
uint8_t messageBox(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, char *szMsg[], uint8_t n)
{
    char key = 0;
    uint8_t slc = 0, i = 0;
    int16_t bCenter = x1 + (x2 - x1) / 2;

    setCursorSize(0x2020);
    shadowBox(x1, y1, x2, y2, 0x4F, sysInfo[1]);
    showMouse();
    moveMouse(bCenter, y2 - 3);
    
    for (i = 0; i < n; i++) writeVRM(bCenter - strlen(szMsg[i]) / 2 + 1, y1 + 2 + i, 0x4A, szMsg[i], 0);
    drawButton(bCenter - 14, y2 - 2, wATV, 4, sysMenu[0], 1, wFLT);
    drawButton(bCenter + 6, y2 - 2, _eATV, 4, sysMenu[2], 1, _eFLT);

    do {
        if (clickMouse(&bCol, &bRow))
        {
            if (bRow == y2 - 2 && bCol >= bCenter - 14 && bCol <= bCenter - 5)
            {
                hideMouse();
                drawButton(bCenter + 6, y2 - 2, _eATV, 4, sysMenu[2], 1, _eFLT);
                clearScreen(bCenter - 14, y2 - 2, bCenter - 3, y2 - 1, 4);
                writeVRM(bCenter - 13, y2 - 2, wATV, sysMenu[0], wFLT);
                delay(50);
                drawButton(bCenter - 14, y2 - 2, wATV, 4, sysMenu[0], 1, wFLT);
                showMouse();
                slc = 0;
                key = ENTER;
            }

            if (bRow == y2 - 2 && bCol >= bCenter + 6 && bCol <= bCenter + 15)
            {
                hideMouse();
                drawButton(bCenter - 14, y2 - 2, _eATV, 4, sysMenu[0], 1, _eFLT);
                clearScreen(bCenter + 6,  y2 - 2, bCenter + 16, y2 - 1, 4);
                writeVRM(bCenter + 7, y2 - 2, wATV, sysMenu[2], wFLT);
                delay(50);
                drawButton(bCenter + 6, y2 - 2, wATV, 4, sysMenu[2], 1, wFLT);
                showMouse();
                slc = 1;
                key = ENTER;
            }

            if (bRow == y1 && bCol == x1 || bCol == x1 + 1)
            {
                slc = 1;
                key = ENTER;
            }
        }

        if (kbhit())
        {
            key = getch();
            if (!key) key = getch();
            switch (toupper(key))
            {
            case LEFT:
                drawButton(bCenter - 14 + slc * 20, y2 - 2, _eATV, 4, sysMenu[slc * 2], 1, _eFLT);
                if (slc < 1) slc = 1; else slc--;
                drawButton(bCenter - 14 + slc * 20, y2 - 2, wATV, 4, sysMenu[slc * 2], 1, wFLT);
                break;

            case RIGHT:
                drawButton(bCenter - 14 + slc * 20, y2 - 2, _eATV, 4, sysMenu[slc * 2], 1, _eFLT);
                if (slc > 0) slc = 0; else slc++;
                drawButton(bCenter - 14 + slc * 20, y2 - 2, wATV, 4, sysMenu[slc * 2], 1, wFLT);
                break;
            }
        }
    } while (key != ENTER);

    return slc;
}

/*---------------------------------------*/
/* Function : warningBox                 */
/* Purpose  : Display the system message */
/* Expects  : (x1,y1,x2,y2) coordinates  */
/*            (szMsg) The messages array */
/*            (n) The elements of array  */
/* Return   : 1 or 0                     */
/*---------------------------------------*/
void warningBox(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, char *szMsg[], uint8_t n)
{
    char key;
    int16_t i = 0;
    const int16_t bCenter = x1 + (x2 - x1) / 2;
    const int16_t bPos = bCenter - strlen(sysMenu[0]) / 2;
    
    setCursorSize(0x2020);
    shadowBox(x1, y1, x2, y2, 0x4F, sysInfo[1]);

    for (i = 0; i < n; i++) writeVRM(bCenter - strlen(szMsg[i]) / 2, y1 + 2 + i, 0x4A, szMsg[i], 0);
    
    drawButton(bPos, y2 - 2, wATV, 4, sysMenu[0], 1, wFLT);
    showMouse();
    moveMouse(bCenter, y2 - 3);

    do {
        if (clickMouse(&bCol, &bRow))
        {
            if (bRow == y2 - 2 && bCol >= bPos  && bCol <= bPos + 9)
            {
                hideMouse();
                clearScreen(bPos, y2 - 2, bPos + 7, y2 - 1, 4);
                writeVRM(bPos + 1, y2 - 2, wATV, sysMenu[0], 0xF4);
                delay(50);
                drawButton(bPos, y2 - 2, wATV, 4, sysMenu[0], 1, 0xF4);
                showMouse();
                key = ENTER;
            }

            if (bRow == y1 && bCol == x1 || bCol == x1 + 1) break;
        }

        if (kbhit())
        {
            key = getch();
            if (!key) key = getch();
            switch (toupper(key))
            {
            case 'D':
            case ESC: key = ENTER;
            }
        }
    } while (key != ENTER);
}

/*--------------------------------------*/
/* Funtion : checkProductKey            */
/* Purpose : Testing user serial number */
/* Expects : Nothing                    */
/* Returns : Nothing                    */
/*--------------------------------------*/
void checkProductKey()
{
    FILE *fptr;
    REG_INFO regInfo;
    char szOldName[31], szOldID[20];
    char szCurrName[31], szCurrID[20];
    uint8_t flgCorrectName = 1, flgCorrectID = 0;
    uint8_t i = 0, j = 0, isASCII = 0, isOK = 0;

    setCursorSize(0x2020);
    fillFrame(1, 1, 80, 25, 0xF6, 178);
    shadowBox(5, 3, 75, 23, 0x5F, sysInfo[2]);
    writeVRM(8, 5, 0x5A, sysInfo[56], 0);
    writeVRM(8, 6, 0x5A, sysInfo[57], 0);
    writeVRM(8, 7, 0x5A, sysInfo[58], 0);
    writeVRM(8, 8, 0x5A, sysInfo[59], 0);
    writeVRM(8, 9, 0x5A, sysInfo[60], 0);
    writeVRM(14, 12, 0x5E, sysMenu[9], 0x5F);
    writeVRM(11, 11, 0x5B, sysMenu[20], 0);
    writeVRM(8, 14, 0x5F, sysInfo[22], 0);
    writeChar(8, 15, 0x1F, 30, 32);
    writeVRM(8, 17, 0x5F, sysInfo[23], 0);
    writeVRM(11, 12, 0x5E, sysMenu[21], 0);
    writeChar(8, 18, 0x1F, 30, 32);
    drawButton(24, 21, _wATV, 5, sysMenu[1], 1, _wFLT);
    drawButton(47, 21, wATV, 5, sysMenu[4], 1, wFLT);
    showMouse();
    moveMouse(10, 11);

    slc = key = 0;
    writeVRM(14, 11 + slc, 0x5B, sysMenu[slc + 8], 0x5A);
        
    do {
        if (kbhit())
        {
            key = getch();
            if (!key) key = getch();
            switch (toupper(key))
            {
            case DOWN:
                writeVRM(14, 11 + slc, 0x5E, sysMenu[slc + 8], 0x5F);
                writeVRM(11, 11 + slc, 0x5E, sysMenu[21], 0);
                if (slc > 0) slc = 0; else slc++;
                writeVRM(14, 11 + slc, 0x5B, sysMenu[slc + 8], 0x5A);
                writeVRM(11, 11 + slc, 0x5B, sysMenu[20], 0);
                break;
            case UP:
                writeVRM(14, 11 + slc, 0x5E, sysMenu[slc + 8], 0x5F);
                writeVRM(11, 11 + slc, 0x5E, sysMenu[21], 0);
                if (slc < 1) slc = 1; else slc--;
                writeVRM(14, 11 + slc, 0x5B, sysMenu[slc + 8], 0x5A);
                writeVRM(11, 11 + slc, 0x5B, sysMenu[20], 0);
                break;
            case TAB:
                if (!slc) isOK = 1;
                else
                {
                    hideMouse();
                    getScreenText(20, 10, 43, 9, szScreen);
                    msgSlc = messageBox(20, 10, 60, 16, msgExit, 1);
                    if (!msgSlc) cleanup();
                    hideMouse();
                    putScreenText(20, 10, 43, 9, szScreen);
                    showMouse();
                }
                break;
            }
        }

        if (clickMouse(&bCol, &bRow))
        {
            if (bRow == 11 && bCol >= 11 && bCol <= 40)
            {
                hideMouse();
                writeVRM(14, 12, 0x5E, sysMenu[9], 0x5F);
                writeVRM(11, 11, 0x5E, sysMenu[21], 0);
                delay(20);
                writeVRM(14, 11, 0x5B, sysMenu[8], 0x5A);
                writeVRM(11, 12, 0x5B, sysMenu[20], 0);
                showMouse();
                slc = 0;
                isOK = 1;
            }

            if (bRow == 12 && bCol >= 11 && bCol <= 55)
            {
                hideMouse();
                writeVRM(14, 11, 0x5E, sysMenu[8], 0x5F);
                writeVRM(11, 11, 0x5E, sysMenu[21], 0);
                delay(20);
                writeVRM(14, 12, 0x5B, sysMenu[9], 0x5A);
                writeVRM(11, 12, 0x5B, sysMenu[20], 0);
                showMouse();
                slc = 1;
                hideMouse();
                getScreenText(20, 10, 43, 9, szScreen);
                msgSlc = messageBox(20, 10, 60, 16, msgExit, 1);
                if (!msgSlc) cleanup();
                hideMouse();
                putScreenText(20, 10, 43, 9, szScreen);
                showMouse();   
            }
        }
    } while (!isOK);

    fptr = fopen("register.dat", "rb");
    if (!fptr)
    {
        setBorder(47);
        setCursorSize(0x2020);
        clearScreen(1, 1, 80, 25, 1);
        writeVRM(31, 10, 0x4F, sysInfo[11], 0);
        writeVRM(20, 12, 0x1F, sysInfo[19], 0);
        writeVRM(20, 13, 0x1F, sysInfo[25], 0);
        getch();
        cleanup();
    }

    fread(&regInfo, sizeof(REG_INFO), 1, fptr);
    fclose(fptr);

    for (i = 0; i < strlen(regInfo.serial); i++) regInfo.serial[i] -= regInfo.key;
    for (i = 0; i < strlen(regInfo.user); i++) regInfo.user[i] -= regInfo.key;

    strcpy(szOldName, regInfo.user);
    strcpy(szOldID, regInfo.serial);
    szCurrName[0] = '\0';
    szCurrID[0] = '\0';
    
    label2:
    setCursorSize(0x0B0A);
    setCursorPos(8, 15);

    key = i = j = slc = 0;
    drawButton(24, 21, wATV, 5, sysMenu[1], 1, wFLT);
    drawButton(47, 21, _wATV, 5, sysMenu[4], 1, _wFLT);

    do {
       if (flgCorrectName) setCursorPos(8 + i, 15);
       if (flgCorrectID) setCursorPos(8 + j, 18);
      
       if (kbhit())
       {
            isASCII = readKey(&key);
            if (!key) isASCII = readKey(&key);
            if (flgCorrectName)
            {
                if ((isASCII && i < 30 && key != 8 && isalpha(key)) || (key == 32 && i < 30))
                {
                    szCurrName[i] = key;
                    printChar(8 + i, 15, 0x1F, key);
                    i++;
                }

                if (key == 8 && i > 0)
                {
                    i--;
                    printChar(8 + i, 15, 0x1F, 32);
                }
            }

            if (key == TAB && flgCorrectName)
            {
                flgCorrectName = 0;
                flgCorrectID = 1;
            }

            if (flgCorrectID)
            {
                if (isASCII && j < 19 && key != 8 && key != TAB && key != ENTER)
                {
                    szCurrID[j] = key;
                    printChar(8 + j, 18, 0x1F, key);
                    j++;
                }

                if (key == 8 && j > 0)
                {
                    j--;
                    printChar(8 + j, 18, 0x1F, 32);
                }
            }

            switch (toupper(key))
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
            }
        }

        if (clickMouse(&bCol, &bRow))
        {
            if (bRow == 21 && bCol >= 24 && bCol <= 36)
            {
                hideMouse();
                drawButton(47, 21, _wATV, 5, sysMenu[4], 1, _wFLT);
                clearScreen(24, 21, 36, 22, 5);
                writeVRM(25, 21, wATV, sysMenu[1], wFLT);
                delay(50);
                drawButton(24, 21, wATV, 5, sysMenu[1], 1, wFLT);
                showMouse();
                slc = 0;
                key = ENTER;
            }

            if (bRow == 21 && bCol >= 47 && bCol <= 59)
            {
                hideMouse();
                drawButton(24, 21, _wATV, 5, sysMenu[1], 1, _wFLT);
                clearScreen(47, 21, 59, 22, 5);
                writeVRM(48, 21, wATV, sysMenu[2], wFLT);
                delay(50);
                drawButton(47, 21, wATV, 5, sysMenu[4], 1, wFLT);
                showMouse();
                slc = 1;
                key = ENTER;
            }
        }
    } while (key != ENTER);

    if (i) szCurrName[i] = '\0';
    if (j) szCurrID[j] = '\0';

    if (!slc)
    {
        if (strcmp(szOldName, szCurrName) && strcmp(szOldID, szCurrID))
        {
            hideMouse();
            getScreenText(20, 10, 43, 9, szScreen);
            flgCorrectName = 1;
            flgCorrectID = 0;
            warningBox(20, 10, 60, 16, msgWarn, 1);
            hideMouse();
            putScreenText(20, 10, 43, 9, szScreen);
            writeChar(8, 15, 0x1F, 30, 32);
            writeChar(8, 18, 0x1F, 30, 32);
            showMouse();
            goto label2;
        }
        else if (!strcmp(szOldName, szCurrName) && strcmp(szOldID, szCurrID))
        {
            hideMouse();
            getScreenText(20, 10, 43, 9, szScreen);
            flgCorrectName = 0;
            flgCorrectID = 1;
            warningBox(20, 10, 60, 16, msgWarn, 1);
            hideMouse();
            putScreenText(20, 10, 43, 9, szScreen);
            writeChar(8, 18, 0x1F, 30, 32);
            showMouse();
            goto label2;
        }
        else if (strcmp(szOldName, szCurrName) && !strcmp(szOldID, szCurrID))
        {
            hideMouse();
            getScreenText(20, 10, 43, 9, szScreen);
            flgCorrectName = 1;
            flgCorrectID = 0;
            warningBox(20, 10, 60, 16, msgWarn, 1);
            hideMouse();
            putScreenText(20, 10, 43, 9, szScreen);
            writeChar(8, 15, 0x1F, 30, 32);
            showMouse();
            goto label2;
        }
    }
    else
    {
        hideMouse();
        getScreenText(20, 10, 43, 9, szScreen);
        msgSlc = messageBox(20, 10, 60, 16, msgExit, 1);
        if (!msgSlc) cleanup();
        hideMouse();
        putScreenText(20, 10, 43, 9, szScreen);
        showMouse();
        goto label2;
    }
}

/*------------------------------------*/
/* Funtion : showHelpFile             */
/* Purpose : Show the readme.hlp file */
/* Expects : Nothing                  */
/* Returns : Nothing                  */
/*------------------------------------*/
void showHelpFile()
{
    setBorder(63);
    setCursorSize(0x2020);
    fillFrame(1, 1, 80, 25, 0xF6, 178);
    shadowBox(10, 3, 74, 22, 0x3E, sysInfo[3]);
    writeVRM(12, 5, 0x30, sysInfo[61], 0);
    writeVRM(12, 6, 0x30, sysInfo[62], 0);
    writeVRM(12, 7, 0x30, sysInfo[63], 0);
    writeVRM(12, 8, 0x30, sysInfo[64], 0);
    writeVRM(12, 9, 0x30, sysInfo[65], 0);
    writeVRM(12, 10, 0x30, sysInfo[66], 0);
    writeVRM(12, 11, 0x30, sysInfo[67], 0);
    writeVRM(12, 12, 0x30, sysInfo[68], 0);
    writeVRM(17, 14, 0x3C, sysInfo[69], 0);
    writeVRM(17, 15, 0x3C, sysInfo[70], 0);
    writeVRM(17, 16, 0x3C, sysInfo[71], 0);
    writeVRM(12, 18, 0x31, sysInfo[72], 0);
    drawButton(47, 20, _wATV, 3, sysMenu[4], 1, _wFLT);
    drawButton(26, 20, wATV, 3, sysMenu[1], 1, wFLT);
    showMouse();
    moveMouse(42, 20);

    slc = chs = key = 0;

    do {
        if (kbhit())
        {
            key = getch();
            if (!key) key = getch();
            switch (toupper(key))
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
            }
        }

        if (clickMouse(&bCol, &bRow))
        {
            if (bRow == 20 && bCol >= 26 && bCol <= 38)
            {
                hideMouse();
                drawButton(47, 20, _wATV, 3, sysMenu[4], 1, _wFLT);
                clearScreen(26, 20, 38, 21, 11);
                writeVRM(27, 20, wATV, sysMenu[1], wFLT);
                delay(50);
                drawButton(26, 20, wATV, 3, sysMenu[1], 1, wFLT);
                showMouse();
                slc = 0;
                break;
            }

            if (bRow == 20 && bCol >= 46 && bCol <= 59)
            {
                hideMouse();
                drawButton(26, 20, _wATV, 3, sysMenu[1], 1, _wFLT);
                clearScreen(47, 20, 59, 21, 11);
                writeVRM(48, 20, wATV, sysMenu[4], wFLT);
                delay(50);
                drawButton(47, 20, wATV, 3, sysMenu[4], 1, wFLT);
                showMouse();
                slc = 1;
                break;
            }

            if (bRow == 3 && bCol == 10 || bCol == 11) cleanup();
        }
    } while (key != ENTER);

    hideMouse();
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
    uint8_t i;

    if (_dos_allocmem(0xffff, &segBuff))
    {
        bytesCount = segBuff;
        if (_dos_allocmem(bytesCount, &segBuff))
        {
            setCursorSize(0x2020);
            fillFrame(1, 1, 80, 25, 0xF6, 178);
            hideMouse();
            shadowBox(15, 8, 65, 17, 0x4F, sysInfo[1]);
            writeVRM(27, 10, 0x4E, sysInfo[31], 0);
            writeVRM(17, 11, 0x4F, sysInfo[53], 0);
            writeVRM(17, 12, 0x4F, sysInfo[54], 0);
            writeVRM(17, 13, 0x4F, sysInfo[55], 0);
            drawButton(35, 15, wATV, 4, sysMenu[0], 1, wFLT);
            showMouse();

            do {
                if (kbhit())
                {
                    hideMouse();
                    clearScreen(35, 15, 46, 16, 4);
                    writeVRM(36, 15, wATV, sysMenu[0], wFLT);
                    delay(50);
                    drawButton(35, 15, wATV, 4, sysMenu[0], 1, wFLT);
                    showMouse();
                    break;
                }

                if (clickMouse(&bCol, &bRow))
                {
                    if (bRow == 15 && bCol >= 35 && bCol <= 45)
                    {
                        hideMouse();
                        clearScreen(35, 15, 46, 16, 4);
                        writeVRM(36, 15, wATV, sysMenu[0], wFLT);
                        delay(50);
                        drawButton(35, 15, wATV, 4, sysMenu[0], 1, wFLT);
                        showMouse();
                        break;
                    }

                    if (bRow == 8 && bCol == 15 || bCol == 16) break;
                }
            } while (!kbhit());
            cleanup();            
        }
    }

    copyBuff = (uint8_t far*)MK_FP(segBuff, 0);
    
    setBorder(50);
    setCursorSize(0x2020);
    fillFrame(1, 1, 80, 25, 0xF6, 178);
    shadowBox(15, 6, 65, 17, 0x9F, sysInfo[4]);
    writeVRM(18, 11, 0x9F, sysInfo[73], 0);
    writeVRM(18, 8, 0x9F, sysInfo[38], 0);
    writeVRM(18, 9, 0x9F, sysInfo[40], 0);
    writeChar(18, 12, 0x17, 45, 176);
    writeVRM(53, 13, 0x9F, sysInfo[174], 0);
    drawButton(35, 15, _wATV, 9, sysMenu[2], 1, _wFLT);
    countFiles("C:\\TOPICS\\*.*");
    processDir("C:\\TOPICS\\*.*", szInstallPath);
    delay(500);
    fillFrame(15, 6, 69, 21, 0xF6, 178);
    shadowBox(18, 10, 62, 15, 0x1F, sysInfo[5]);
    writeVRM(22, 12, 0x1E, sysInfo[41], 0);
    writeChar(22, 13, 0x17, 37, 176);
    writeVRM(49, 14, 0x1A, sysInfo[174], 0);

    for (i = 1; i < 38; i++)
    {
        writeChar(22, 13, 0x1F, i, 219);
        printVRM(46, 14, 0x1A, "%3d", 2 * i + 26);
        delay(100);
    }

    fillFrame(15, 6, 69, 21, 0xF6, 178);
    warningBox(25, 10, 55, 15, msgCmp, 1);

    _dos_freemem(segBuff);
    segBuff = 0;
}

/*----------------------------------------------*/
/* Funtion : restartProgram                     */
/* Purpose : Showing the restart system message */
/* Expects : Nothing                            */
/* Returns : Nothing                            */
/*----------------------------------------------*/
void restartProgram()
{
    showMouse();
    setCursorSize(0x2020);
    setBorder(50);
    fillFrame(1, 1, 80, 25, 0xF6, 178);
    shadowBox(15, 8, 65, 15, 0x4F, sysInfo[6]);
    writeVRM(21, 11, 0x4A, sysMenu[11], 0x4B);
    writeVRM(18, 10, 0x4F, sysMenu[20], 0);
    writeVRM(21, 10, 0x4F, sysMenu[10], 0x4E);
    writeVRM(18, 11, 0x4A, sysMenu[21], 0);
    drawButton(26, 13, 0xB4, 4, sysMenu[0], 1, 0xB1);
    drawButton(45, 13, 0x9F, 4, sysMenu[3], 1, 0x94);
    moveMouse(40, 13);

    slc = chs = key = 0;

    do {
        if (kbhit())
        {
            key = getch();
            if (!key) key = getch();
            switch (toupper(key))
            {
            case DOWN:
                writeVRM(21, 10 + slc, 0x4A, sysMenu[slc + 10], 0x4B);
                writeVRM(18, 10 + slc, 0x4A, sysMenu[21], 0);
                if (slc > 0) slc = 0; else slc++;
                writeVRM(21, 10 + slc, 0x4F, sysMenu[slc + 10], 0x4E);
                writeVRM(18, 10 + slc, 0x4F, sysMenu[20], 0);
                break;
            case UP:
                writeVRM(21, 10 + slc, 0x4A, sysMenu[slc + 10], 0x4B);
                writeVRM(18, 10 + slc, 0x4A, sysMenu[21], 0);
                if (slc < 1) slc = 1; else slc--;
                writeVRM(21, 10 + slc, 0x4F, sysMenu[slc + 10], 0x4E);
                writeVRM(18, 10 + slc, 0x4F, sysMenu[20], 0);
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
            }
        }

        if (clickMouse(&bCol, &bRow))
        {
            if (bRow == 10 && bCol >= 18 && bCol <= 62)
            {
                slc = 0;
                hideMouse();
                writeVRM(21, 11, 0x4A, sysMenu[11], 0x4B);
                writeVRM(18, 11, 0x4A, sysMenu[21], 0);
                writeVRM(21, 10, 0x4F, sysMenu[10], 0x4E);
                writeVRM(18, 10, 0x4F, sysMenu[20], 0);
                showMouse();
                delay(50);
            }

            if (bRow == 11 && bCol >= 18 && bCol <= 61)
            {
                slc = 1;
                hideMouse();
                writeVRM(21, 10, 0x4A, sysMenu[10], 0x4B);
                writeVRM(18, 10, 0x4A, sysMenu[21], 0);
                writeVRM(21, 11, 0x4F, sysMenu[11], 0x4E);
                writeVRM(18, 11, 0x4F, sysMenu[20], 0);
                showMouse();
                delay(50);
            }

            if (bRow == 13 && bCol >= 26 && bCol <= 35)
            {
                chs = 0;
                hideMouse();
                drawButton(45, 13, 0x9F, 4, sysMenu[3], 1, 0x94);
                clearScreen(26, 13, 36, 14, 4);
                writeVRM(27, 13, 0xB4, sysMenu[0], 0xB1);
                delay(50);
                drawButton(26, 13, 0xB4, 4, sysMenu[0], 1, 0xB1);
                showMouse();
                break;
            }

            if (bRow == 13 && bCol >= 45 && bCol <= 56)
            {
                chs = 1;
                hideMouse();
                drawButton(26, 13, 0x9F, 4, sysMenu[0], 1, 0x94);
                clearScreen(45, 13, 57, 14, 4);
                writeVRM(46, 13, 0xB4, sysMenu[3], 0xB1);
                delay(50);
                drawButton(45, 13, 0xB4, 4, sysMenu[3], 1, 0xB1);
                showMouse();
                break;
            }

            if (bRow == 8 && bCol == 15 || bCol == 16) cleanup();
        }
    } while (key != ENTER);

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
    writeVRM(31, 10, 0x34, sysInfo[24], 0);
    writeChar(29, 11, 0x34, 22, 193);
    for (i = 0; i < 4; i++) writeVRM(29, 12 + i, 0x30, sysMenu[12 + i], 0x3A);
    drawButton(46, 12, wATV, 3, sysMenu[1], 1, wFLT);
    drawButton(46, 14, _wATV, 3, sysMenu[4], 1, _wFLT);
    writeVRM(23, 17, 0x30, sysMenu[22], 0);
    writeChar(33, 17, 0x1F, 25, 32);
    moveMouse(38, 11);

    slc = chs = key = 0;
    strcpy(szInstallPath, "B:\\INSTALL");
    writeVRM(29, 12 + slc, 0x3F, sysMenu[12 + slc], 0x3E);

    do {
        key = 0;
        if (kbhit())
        {
            key = getch();
            if (!key) key = getch();
            switch (key)
            {
            case UP:
                writeVRM(29, 12 + slc, 0x30, sysMenu[12 + slc], 0x3A);
                if (slc < 1) slc = 3; else slc--;
                writeVRM(29, 12 + slc, 0x3F, sysMenu[12 + slc], 0x3E);
                break;
            case DOWN:
                writeVRM(29, 12 + slc, 0x30, sysMenu[12 + slc], 0x3A);
                if (slc >= 3) slc = 0; else slc++;
                writeVRM(29, 12 + slc, 0x3F, sysMenu[12 + slc], 0x3E);
                break;
            case HOME:
                slc = 0;
                writeVRM(29, 12 + slc, 0x30, sysMenu[12 + slc], 0x3A);
                writeVRM(29, 12 + slc, 0x3F, sysMenu[12 + slc], 0x3E);
                break;
            case END:
                slc = 3;
                writeVRM(29, 12 + slc, 0x30, sysMenu[12 + slc], 0x3A);
                writeVRM(29, 12 + slc, 0x3F, sysMenu[12 + slc], 0x3E);
                break;
            case TAB:
                key = 0;
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
                            if (!chs)
                            {
                                isOK = 1;
                                hideMouse();
                                drawButton(46, 14, _wATV, 3, sysMenu[4], 1, _wFLT);
                                clearScreen(46, 12, 57, 13, 3);
                                writeVRM(47, 12, wATV, sysMenu[1], wFLT);
                                delay(50);
                                drawButton(46, 12, wATV, 3, sysMenu[1], 1, wFLT);
                                showMouse();
                            }
                            else
                            {
                                hideMouse();
                                getScreenText(20, 10, 43, 9, szScreen);
                                msgSlc = messageBox(20, 10, 60, 16, msgExit, 1);
                                if (!msgSlc) cleanup();
                                putScreenText(20, 10, 43, 9, szScreen);
                                showMouse();
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
                hideMouse();
                writeVRM(29, 12, 0x3F, sysMenu[12], 0x3E);
                if (slc != 0) writeVRM(29, 12 + slc, 0x30, sysMenu[12 + slc], 0x3A);
                showMouse();
                slc = 0;
                delay(20);
            }

            if (bRow == 13 && bCol >= 29 && bCol <= 41)
            {
                hideMouse();
                writeVRM(29, 13, 0x3F, sysMenu[13], 0x3E);
                if (slc != 1) writeVRM(29, 12 + slc, 0x30, sysMenu[12 + slc], 0x3A);
                showMouse();
                slc = 1;
                delay(20);
            }

            if (bRow == 14 && bCol >= 29 && bCol <= 41)
            {
                hideMouse();
                writeVRM(29, 14, 0x3F, sysMenu[14], 0x3E);
                if (slc != 2) writeVRM(29, 12 + slc, 0x30, sysMenu[12 + slc], 0x3A);
                showMouse();
                slc = 2;
                delay(20);
            }

            if (bRow == 15 && bCol >= 29 && bCol <= 41)
            {
                hideMouse();
                writeVRM(29, 15, 0x3F, sysMenu[15], 0x3E);
                if (slc != 3) writeVRM(29, 12 + slc, 0x30, sysMenu[12 + slc], 0x3A);
                showMouse();
                slc = 3;
                delay(20);
            }

            if (bRow == 12 && bCol >= 46 & bCol <= 57)
            {
                chs = 0;
                isOK = 1;
                hideMouse();
                drawButton(46, 14, _wATV, 3, sysMenu[4], 1, _wFLT);
                clearScreen(46, 12, 57, 13, 3);
                writeVRM(47, 12, wATV, sysMenu[1], wFLT);
                delay(50);
                drawButton(46, 12, wATV, 3, sysMenu[1], 1, wFLT);
                showMouse();
            }

            if (bRow == 14 && bCol >= 46 & bCol <= 57)
            {
                chs = 1;
                hideMouse();
                drawButton(46, 12, _wATV, 3, sysMenu[1], 1, _wFLT);
                clearScreen(46, 14, 57, 16, 3);
                writeVRM(47, 15, wATV, sysMenu[4], wFLT);
                delay(50);
                drawButton(46, 14, wATV, 3, sysMenu[4], 1, wFLT);
                getScreenText(20, 10, 43, 9, szScreen);
                msgSlc = messageBox(20, 10, 60, 16, msgExit, 1);
                if (!msgSlc) cleanup();
                putScreenText(20, 10, 43, 9, szScreen);
                showMouse();                            
            }
        }

        szInstallPath[0] = drives[slc];
        writeVRM(33, 17, 0x1E, szInstallPath, 0);
    } while (!isOK);
}

/*----------------------------------------------------*/
/* Funtion : updateProgram                            */
/* Purpose : Deleting files crack.com and install.com */
/* Expects : Nothing                                  */
/* Returns : Nothing                                  */
/*----------------------------------------------------*/
void updateProgram()
{
    FILE *fp;
    REG_INFO regInfo;
    char szPath[15];

    fp = fopen(sysInfo[42], "r+b");
    if (!fp)
    {
        setBorder(47);
        setCursorSize(0x2020);
        clearScreen(1, 1, 80, 25, 1);
        writeVRM(33, 10, 0x4F, sysInfo[11], 0);
        writeVRM(20, 12, 0x1F, sysInfo[19], 0);
        writeVRM(20, 13, 0x1F, sysInfo[25], 0);
        getch();
        cleanup();
    }

    fread(&regInfo, sizeof(REG_INFO), 1, fp);
    strcpy(regInfo.path, szInstallPath);
    fseek(fp, 0L, SEEK_SET);
    fwrite(&regInfo, sizeof(REG_INFO), 1, fp);
    fclose(fp);

    strcpy(szPath, szInstallPath);
    strcat(szPath, "crack.com");
    unlink(szPath);

    szPath[3] = '\0';
    strcat(szPath, "install.exe");
    unlink(szPath);
    szPath[3] = '\0';
    strcat(szPath, "guide.txt");
    unlink(szPath);
}

/*--------------------------------------------*/
/* Function : getDriveId                      */
/* Purpose  : Return the number of the drier  */
/* Expects  : (szDrive) Serial number szDrive */
/* Returns  : number of szDrive               */
/*--------------------------------------------*/
uint8_t getDriveId(char drive)
{
    if (toupper(drive) == 'B') return 2;
    if (toupper(drive) == 'C') return 3;
    if (toupper(drive) == 'D') return 4;
    if (toupper(drive) == 'E') return 5;
    return 0;
}

/*--------------------------------------------------*/
/* Funtion : checkDiskSpace                         */
/* Purpose : Checking disk space and available      */
/* Expects : Nothing                                */
/* Returns : Nothing                                */
/*--------------------------------------------------*/
void checkDiskSpace()
{
    uint32_t a, b, c, d;
    uint8_t i, bSkip = 0;
    union REGS inRegs, outRegs;
    
    setBorder(50);
    setCursorSize(0x2020);
    fillFrame(1, 1, 80, 25, 0xF6, 178);
    shadowBox(15, 5, 65, 20, 0x7F, sysInfo[8]);
    writeVRM(18, 7, 0x70, sysInfo[39], 0);
    writeVRM(18, 8, 0x70, sysInfo[40], 0);
    writeVRM(18, 10, 0x70, sysInfo[29], 0);
    writeVRM(53, 12, 0x70, sysInfo[174], 0);
    writeChar(18, 11, 0x17, 45, 176);
    drawButton(36, 18, wATV, 7, sysMenu[2], 1, wFLT);
    moveMouse(39, 17);

    for (i = 0; i < 45 && !bSkip; i++)
    {
        writeChar(18 + i, 11, 0x3E, 1, 219);
        printVRM(50, 12, 0x70, "%3u", 2 * i + 12);

        if (clickMouse(&bCol, &bRow))
        {
            if (bRow == 18 && bCol >= 36 && bCol <= 45)
            {
                hideMouse();
                clearScreen(36, 18, 45, 19, 7);
                writeVRM(37, 18, wATV, sysMenu[2], wATV);
                delay(50);
                drawButton(36, 18, wATV, 7, sysMenu[2], 1, wFLT);
                showMouse();
                bSkip = 1;
                break;
            }
        }

        if (kbhit())
        {
            key = getch();
            if (!key) key = getch();
            switch (toupper(key))
            {
            case 'B':
            case ESC: bSkip = 1;
            }
            break;
        }
        delay(100);
    }

    writeVRM(18, 14, 0x70, sysInfo[30], 0);
    writeChar(18, 15, 0x17, 45, 176);
    writeVRM(53, 16, 0x70, sysInfo[174], 0);

    for (i = 0; i < 45 && !bSkip; i++)
    {
        writeChar(18 + i, 15, 0x3E, 1, 219);
        printVRM(50, 16, 0x70, "%3u",  2 * i + 12);

        if (clickMouse(&bCol, &bRow))
        {
            if (bRow == 18 && bCol >= 36 && bCol <= 45)
            {
                hideMouse();
                clearScreen(36, 18, 45, 19, 7);
                writeVRM(37, 18, wATV, sysMenu[2], wATV);
                delay(50);
                drawButton(36, 18, wATV, 7, sysMenu[2], 1, wFLT);
                showMouse();
                break;
            }
        }

        if (kbhit())
        {
            key = getch();
            if (!key) key = getch();
            switch (toupper(key))
            {
            case 'C':
            case ESC: break;
            }
            break;
        }
        delay(100);
    }

    inRegs.h.ah = 0x36;
    inRegs.h.dl = getDriveId(szInstallPath[0]);
    int86(0x21, &inRegs, &outRegs);

    if (outRegs.x.ax == 0xFFFF)
    {
        shadowBox(20, 9, 55, 15, 0x4F, sysInfo[1]);
        writeVRM(22, 11, 0x4F, sysInfo[37], 0);
        drawButton(33, 13, wATV, 4, sysMenu[0], 1, wFLT);

        do {
            if (clickMouse(&bCol, &bRow))
            {
                if (bRow == 13 && bCol >= 33 && bCol <= 43)
                {
                    hideMouse();
                    clearScreen(33, 13, 43, 14, 4);
                    writeVRM(34, 13, wATV, sysMenu[0], wFLT);
                    delay(50);
                    drawButton(33, 13, wATV, 4, sysMenu[0], 1, wFLT);
                    showMouse();
                    break;
                }

                if (bRow == 9 && bCol == 20 || bCol == 21) break;
            }
        } while (!kbhit());
        cleanup();
    }

    a = outRegs.x.dx;
    b = outRegs.x.bx;
    c = outRegs.x.ax;
    d = outRegs.x.cx;

    fillFrame(1, 1, 80, 25, 0xF6, 178);
    shadowBox(15, 9, 63, 16, 0x9F, sysInfo[9]);
    printVRM(20, 11, 0x9E, sysInfo[175], (float)a * c * d / 1024.0 / 1024.0);
    printVRM(20, 12, 0x9E, sysInfo[176], (float)b * c * d / 1024.0 / 1024.0);
    drawButton(34, 14, wATV, 9, sysMenu[0], 1, wFLT);
    moveMouse(38, 12);

    if (((b * c * d) / 1024) <= 1024)
    {
        writeVRM(18, 13, 0x1C, sysInfo[35], 0);
        do {
            if (clickMouse(&bCol, &bRow))
            {
                if (bRow == 14 && bCol >= 34 && bCol <= 41)
                {
                    clearScreen(34, 14, 42, 15, 1);
                    writeVRM(35, 14, wATV, sysMenu[0], wFLT);
                    delay(50);
                    drawButton(34, 14, wATV, 9, sysMenu[0], 1, wFLT);
                    break;
                }

                if (bRow == 9 && bCol == 15 || bCol == 16) break;
            }
        } while (!kbhit());
        cleanup();
    }

    do {
        if (clickMouse(&bCol, &bRow))
        {
            if (bRow == 14 && bCol >= 34 && bCol <= 41)
            {
                hideMouse();
                clearScreen(34, 14, 42, 15, 1);
                writeVRM(35, 14, wATV, sysMenu[0], wFLT);
                delay(50);
                drawButton(34, 14, wATV, 9, sysMenu[0], 1, wFLT);
                showMouse();
                break;
            }
            
            if (bRow == 9 && bCol == 15 || bCol == 16) break;
        }
    } while (!kbhit());
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
    //checkProductKey();
    installProgram();
    updateProgram();
    showHelpFile();
    fadeOut();
    chdir(szInstallPath);
    system("readme");
    restartProgram();
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
    writeVRM(13, 4, 0x1A, sysInfo[10], 0);
    writeChar(11, 5, 0x1B, 61, 205);
    writeVRM(12, 6, 0x1B, sysInfo[36], 0);
    writeVRM(23, 8, 0x1F, sysInfo[44], 0);
    writeVRM(23, 9, 0x1F, sysInfo[45], 0);
    writeVRM(23, 10, 0x1F, sysInfo[46], 0);
    writeVRM(23, 12, 0x1E, sysInfo[47], 0);
    writeVRM(23, 13, 0x1E, sysInfo[48], 0);
    writeVRM(23, 14, 0x1E, sysInfo[49], 0);
    writeVRM(23, 16, 0x1B, sysInfo[50], 0);
    writeVRM(23, 17, 0x1B, sysInfo[51], 0);
    writeVRM(23, 18, 0x1B, sysInfo[52], 0);
    writeChar(9, 19, 0x1F, 65, 196);
    printChar(8, 19, 0x1F, 195);
    printChar(74, 19, 0x1F, 180);
    writeVRM(14, 11, 0x1C, sysMenu[17], 0x1E);
    writeVRM(14, 15, 0x1C, sysMenu[18], 0x1E);
    
    if (!initMouse()) system(sysInfo[43]);
    setMousePos();
    
    drawButton(24, 21, wATV, 1, sysMenu[1], 1, wFLT);
    drawButton(48, 21, _wATV, 1, sysMenu[4], 1, _wFLT);
    
    key = slc = 0;
    writeVRM(14, 7, 0x1A, sysMenu[16], 0x1F);

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
                if (!fltStop[slc]) writeVRM(14, 7 + 4 * slc, 0x1C, sysMenu[16 + slc], 0x1E);
                else
                {
                    writeVRM(14, 7 + 4 * slc, 0x1C, sysMenu[16 + slc], 0x1E);
                    printChar(15, 7 + 4 * slc, 0x1C, 251);
                }

                if (slc < 1) slc = 2; else slc--;
                
                if (!fltStop[slc]) writeVRM(14, 7 + 4 * slc, 0x1A, sysMenu[16 + slc], 0x1F);
                else
                {
                    writeVRM(14, 7 + 4 * slc, 0x1A, sysMenu[16 + slc], 0x1F);
                    printChar(15, 7 + 4 * slc, 0x1A, 251);
                }
                setCursorPos(15, 7 + 4 * slc);
                break;
            case DOWN:
                if (!fltStop[slc]) writeVRM(14, 7 + 4 * slc, 0x1C, sysMenu[16 + slc], 0x1E);
                else
                {
                    writeVRM(14, 7 + 4 * slc, 0x1C, sysMenu[16 + slc], 0x1E);
                    printChar(15, 7 + 4 * slc, 0x1C, 251);
                }

                if (slc >= 2) slc = 0; else slc++;

                if (!fltStop[slc]) writeVRM(14, 7 + 4 * slc, 0x1A, sysMenu[16 + slc], 0x1F);
                else
                {
                    writeVRM(14, 7 + 4 * slc, 0x1A, sysMenu[16 + slc], 0x1F);
                    printChar(15, 7 + 4 * slc, 0x1A, 251);
                }
                setCursorPos(15, 7 + 4 * slc);
                break;
            case HOME:
                if (!fltStop[slc]) writeVRM(14, 7 + 4 * slc, 0x1C, sysMenu[16 + slc], 0x1E);
                else
                {
                    writeVRM(14, 7 + 4 * slc, 0x1C, sysMenu[16 + slc], 0x1E);
                    printChar(15, 7 + 4 * slc, 0x1C, 251);
                }
                
                slc = 0;
                if (!fltStop[slc]) writeVRM(14, 7 + 4 * slc, 0x1A, sysMenu[16 + slc], 0x1F);
                else
                {
                    writeVRM(14, 7 + 4 * slc, 0x1A, sysMenu[16 + slc], 0x1F);
                    printChar(15, 7 + 4 * slc, 0x1A, 251);
                }
                setCursorPos(15, 7 + 4 * slc);
                break;
            case END:
                if (!fltStop[slc]) writeVRM(14, 7 + 4 * slc, 0x1C, sysMenu[16 + slc], 0x1E);
                else
                {
                    writeVRM(14, 7 + 4 * slc, 0x1C, sysMenu[16 + slc], 0x1E);
                    printChar(15, 7 + 4 * slc, 0x1C, 251);
                }
                
                slc = 2;
                if (!fltStop[slc]) writeVRM(14, 7 + 4 * slc, 0x1A, sysMenu[16 + slc], 0x1F);
                else
                {
                    writeVRM(14, 7 + 4 * slc, 0x1A, sysMenu[16 + slc], 0x1F);
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
                    writeVRM(14, 7 + 4 * slc, 0x1A, sysMenu[16 + slc], 0x1F);
                    fltStop[slc] = 0;
                }
                else
                {
                    printChar(15, 7 + 4 * slc, 0x1A, 251);
                    fltStop[slc] = 1;
                }
                break;
            case ENTER:
                if (!chs)
                {
                    hideMouse();
                    drawButton(48, 21, _wATV, 1, sysMenu[4], 1, _wFLT);
                    clearScreen(24, 21, 37, 22, 1);
                    writeVRM(25, 21, wATV, sysMenu[1], wFLT);
                    delay(50);
                    drawButton(24, 21, wATV, 1, sysMenu[1], 1, wFLT);
                    showMouse();
                    if (fltStop[1] && fltStop[2]) isOK = 1;
                    else
                    {
                        hideMouse();
                        getScreenText(20, 10, 44, 9, szScreen);
                        msgSlc = messageBox(20, 10, 61, 17, msgError, 2);
                        if (msgSlc) cleanup();
                        putScreenText(20, 10, 44, 9, szScreen);
                        showMouse();
                    }
                }
                else
                {
                    hideMouse();
                    getScreenText(20, 10, 44, 9, szScreen);
                    msgSlc = messageBox(20, 10, 61, 16, msgExit, 1);
                    if (!msgSlc) cleanup();
                    putScreenText(20, 10, 44, 9, szScreen);
                    showMouse();
                }
                break;
            }
        }

        if (clickMouse(&bCol, &bRow))
        {
            if (bRow == 7 && bCol >= 14 && bCol <= 48)
            {
                hideMouse();
                if (!fltStop[slc]) writeVRM(14, 7 + 4 * slc, 0x1C, sysMenu[16 + slc], 0x1E);
                else
                {
                    writeVRM(14, 7 + 4 * slc, 0x1C, sysMenu[16 + slc], 0x1E);
                    printChar(15, 7 + 4 * slc, 0x1C, 251);
                }
                
                slc = 0;
                if (!fltStop[slc])
                {
                    writeVRM(14, 7, 0x1A, sysMenu[16], 0x1F);
                    printChar(15, 7, 0x1A, 251); fltStop[0] = 1;
                }
                else
                {
                    writeVRM(14, 7, 0x1A, sysMenu[16], 0x1F);
                    fltStop[0] = 0;
                }

                setCursorPos(15, 7);
                showMouse();
                delay(150);
            }

            if (bRow == 11 && bCol >= 14 && bCol <= 57)
            {
                hideMouse();
                if (!fltStop[slc]) writeVRM(14, 7 + 4 * slc, 0x1C, sysMenu[16 + slc], 0x1E);
                else
                {
                    writeVRM(14, 7 + 4 * slc, 0x1C, sysMenu[16 + slc], 0x1E);
                    printChar(15, 7 + 4 * slc, 0x1C, 251);
                }
                
                slc = 1;
                if (!fltStop[slc])
                {
                    writeVRM(14, 11, 0x1A, sysMenu[17], 0x1F);
                    printChar(15, 11, 0x1A, 251); fltStop[1] = 1;
                }
                else
                {
                    writeVRM(14, 11, 0x1A, sysMenu[17], 0x1F);
                    fltStop[1] = 0;
                }

                setCursorPos(15, 11);
                showMouse();
                delay(150);
            }

            if (bRow == 15 && bCol >= 14 && bCol <= 52)
            {
                hideMouse();
                if (!fltStop[slc]) writeVRM(14, 7 + 4 * slc, 0x1C, sysMenu[16 + slc], 0x1E);
                else
                {
                    writeVRM(14, 7 + 4 * slc, 0x1C, sysMenu[16 + slc], 0x1E);
                    printChar(15, 7 + 4 * slc, 0x1C, 251);
                }
                
                slc = 2;
                if (!fltStop[slc])
                {
                    writeVRM(14, 15, 0x1A, sysMenu[18], 0x1F);
                    printChar(15, 15, 0x1A, 251); fltStop[2] = 1;
                }
                else
                {
                    writeVRM(14, 15, 0x1A, sysMenu[18], 0x1F);
                    fltStop[2] = 0;
                }

                setCursorPos(15, 15);
                showMouse();
                delay(150);
            }

            if (bRow == 21 && bCol >= 24 && bCol <= 36)
            {
                hideMouse();
                drawButton(48, 21, _wATV, 1, sysMenu[4], 1, _wFLT);
                clearScreen(24, 21, 37, 22, 1);
                writeVRM(25, 21, wATV, sysMenu[1], wFLT);
                delay(50);
                drawButton(24, 21, wATV, 1, sysMenu[1], 1, wFLT);
                showMouse();
                chs = 0;
                if (fltStop[1] && fltStop[2]) isOK = 1;
                else
                {
                    hideMouse();
                    getScreenText(20, 10, 44, 9, szScreen);
                    msgSlc = messageBox(20, 10, 61, 17, msgError, 2);
                    if (msgSlc) cleanup();
                    putScreenText(20, 10, 44, 9, szScreen);
                    showMouse();
                }
            }

            if (bRow == 21 && bCol >= 48 && bCol <= 60)
            {
                hideMouse();
                drawButton(24, 21, _wATV, 1, sysMenu[1], 1, _wFLT);
                clearScreen(48, 21, 61, 22, 1);
                writeVRM(49, 21, wATV, sysMenu[4], wFLT);
                delay(50);
                drawButton(48, 21, wATV, 1, sysMenu[4], 1, wFLT);
                showMouse();
                chs = 1;
                hideMouse();
                getScreenText(20, 10, 44, 9, szScreen);
                msgSlc = messageBox(20, 10, 61, 16, msgExit, 1);
                if (!msgSlc) cleanup();
                putScreenText(20, 10, 44, 9, szScreen);
                showMouse();
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
