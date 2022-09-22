/*------------------------------------------------------*/
/*      UNIVERSITY OF TECHNOLOGY HO CHI MINH CITY       */
/*          FACULTY OF INFORMATION TECHNOLOGY           */
/*               THE CRACKING PROGRAM FILE              */
/*            Author : NGUYEN NGOC VAN                  */
/*             Class : 00DTH1                           */
/*      Student Code : 00DTH201                         */
/*            Course : 2000 - 2005                      */
/*     Writting date : 24/10/2001                       */
/*       Last Update : 12/11/2001                       */
/*------------------------------------------------------*/
#include <dos.h>
#include <stdio.h>
#include <ctype.h>
#include <conio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

#define MAX_DAYS        30
#define SCR_WIDTH       160
#define MASK_BG         0x08
#define OFFSET(x, y)    (((x - 1) + 80 * (y - 1)) << 1)

#define KEY_UP		    72
#define KEY_DOWN	    80
#define KEY_ENTER	    13
#define KEY_SPACER	    32
#define KEY_ESC	        27

#define ATV 	        0xF0
#define FLT 	        0xFC
#define _ATV 	        0x78
#define _FLT 	        0x75

typedef struct {
    time_t      utime;      // Register timestamp
    uint16_t    days;       // The number of days
    uint8_t     key;        // Random key
    char        serial[21]; // License code
    char        user[32];   // User name
    char        path[34];   // The installation path
    char        magic[34];  // Random characters
} REG_INFO;

char scrBuff[1272] = {0};   // Screen buffer
uint8_t bmAvalid = 0;       // Status of the mouse
uint8_t far *txtMem = (uint8_t far*)0xB8000000L;

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
uint8_t clickMouse(uint16_t *col, uint16_t *row)
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
    system("font off");
    system("cls");
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
    if (!len) return 0;

    i = 0;
    while (isspace(szUserName[i++]));
    if (i >= len) return 0;

    for (i = 0; i != len; i++)
    {
        if (!isalpha(szUserName[i]) && !isspace(szUserName[i])) return 0;
    }

    return 1;
}

/*-----------------------------------------------*/
/* Function : genSerialNumber                    */
/* Mission  : Generate product installation key  */
/* Expects  : (szUserName) input user name       */
/*             (CDKey) output product key        */
/* Returns  : Nothing                            */
/*-----------------------------------------------*/
void genSerialNumber(char *szUserName, char *CDKey)
{
    char cKey = 0;
    FILE *fptr;
    REG_INFO tmp;
    uint8_t k = 0;
    uint16_t col = 0, row = 0;
    uint16_t i = 0, len = 0;

    i = 0;
    do {
        cKey = 48 + (rand() % 49);
        if (isdigit(cKey) || isupper(cKey)) CDKey[i++] = cKey;
    } while (i < 19);
    
    CDKey[i] = '\0';

    for (i = 0; i < strlen(CDKey) - 1; i++)
    {
        if (!((i + 1) % 5)) CDKey[i] = 45;
    }
    
    fptr = fopen("register.dat", "wb");
    if (!fptr) return;

    tmp.days = MAX_DAYS;
    tmp.utime = time(0);
    tmp.key = 90 + (rand() % 10);

    len = strlen(CDKey);
    for (i = 0; i < len; i++) tmp.serial[i] = CDKey[i] + tmp.key;
    tmp.serial[i] = '\0';

    len = strlen(szUserName);
    for (i = 0; i < len; i++) tmp.user[i] = szUserName[i] + tmp.key;
    tmp.user[i] = '\0';

    len = strlen(szUserName);
    for (i = 0; i < sizeof(tmp.magic) / 2; i += 2)
    {
        k = rand() % len;
        tmp.magic[i] = k;
        tmp.magic[i + 1] = szUserName[k] + tmp.key;
    }
    tmp.magic[i - 2] = '\0';

    fwrite(&tmp, sizeof(tmp), 1, fptr);
    fclose(fptr);
}

/*-------------------------------------------------*/
/* Function : startCracking                        */
/* Mission  : Showing the message to select option */
/* Expects  : Nothing                              */
/* Returns  : Nothing                              */
/*-------------------------------------------------*/
void startCracking()
{
    char cKey = 0;
    char CDKey[20] = {0};
    char szUserName[31] = {0};

    char *szMenu[] = {
        "   ~La61y ma4   ",
        "  ~Ke61t thu1c  ",
        "  ~Tro75 giu1p  "
    };

    char *szHelp[] = {
        "Hu7o71ng da64n la61y ma4 so61",
        "Ba5n ca62n nha65p te6n cu3a ba5n va2o o6 thu71 nha61t va2 nha61n",
        "nu1t La61y ma4 o73 phi1a be6n pha3i. Ba5n se4 nha65n d9u7o75c ma4",
        "so61 ca2i d9a85t o73 o6 thu71 hai be6n du7o71i. Ba5n co1 the63 ghi",
        "ma4 so61 na2y va2o gia61y  d9e63 tie61n ha2nh ca2i d9a85t chu7o7ng",
        "tri2nh. Nha61n va2o nu1t Ke61t thu1c d9e63 thoa1t.",
        "  ~D9o62ng y1  ",
        "Te6n kho6ng ho75p le65!"
    };
        
    uint16_t k = 0, i = 0;
    uint16_t bCol = 0, bRow = 0;
    uint8_t bSlc = 0, isASCII = 1, noUserName = 0;
    
    memset(szUserName, 0, sizeof(szUserName));
    for (i = 0; i < sizeof(szMenu) / sizeof(szMenu[0]); i++) fontVNI(szMenu[i]);
    for (i = 0; i < sizeof(szHelp) / sizeof(szHelp[0]); i++) fontVNI(szHelp[i]);
    
    setCursorSize(0x0B0A);
    drawButton(51, 8, ATV, 3, szMenu[0], 0, FLT);
    drawButton(51, 10, _ATV, 3, szMenu[1], 1, _FLT);
    moveMouse(35, 10);
    drawButton(51, 12, _ATV, 3, szMenu[2], 1, _FLT);
    writeChar(18, 9, 0x1E, 30, 32);

    do {
        setCursorPos(18 + k, 9);

        if (kbhit())
        {
            if (noUserName)
            {
                noUserName = 0;
                writeChar(18, 9, 0x1E, 30, 32);
            }

            isASCII = readKey(&cKey);
            if (!cKey) isASCII = readKey(&cKey);
            
            if ((isASCII && k < 30 && cKey != 8 && isalpha(cKey)) || (cKey == 32 && k < 30))
            {
                szUserName[k] = cKey;
                printChar(18 + k, 9, 0x1E, cKey);
                k++;
            }

            if (cKey == 8 && k > 0)
            {
                k--;
                printChar(18 + k, 9, 0x1E, 32);
            }

            szUserName[k] = '\0';

            switch (cKey)
            {
            case KEY_UP:
                if (!isASCII)
                {
                    drawButton(51, 8 + bSlc * 2, _ATV, 3, szMenu[bSlc], 1, _FLT);
                    if (bSlc < 1) bSlc = 2; else bSlc--;
                    drawButton(51, 8 + bSlc * 2, ATV, 3, szMenu[bSlc], 0, FLT);
                }
                break;
            case KEY_DOWN:
                if (!isASCII)
                {
                    drawButton(51, 8 + bSlc * 2, _ATV, 3, szMenu[bSlc], 1, _FLT);
                    if (bSlc > 1) bSlc = 0; else bSlc++;
                    drawButton(51, 8 + bSlc * 2, ATV, 3, szMenu[bSlc], 0, FLT);
                }
                break;
            case KEY_ENTER:
                switch (bSlc)
                {
                case 0:
                    hideMouse();
                    drawButton(51, 8 + bSlc * 2, _ATV, 3, szMenu[bSlc], 1, _FLT);
                    bSlc = 0;
                    clearScreen(51, 8 + bSlc * 2, 64, 9, 3);
                    writeVRM(52, 8 + bSlc * 2, ATV, szMenu[bSlc], FLT);
                    delay(50);
                    drawButton(51, 8 + bSlc * 2, ATV, 3, szMenu[bSlc], 0, FLT);
                    showMouse();
                    
                    if (!validUserName(szUserName))
                    {
                        noUserName = 1;
                        setCursorSize(0x0B0A);
                        writeVRM(18, 9, 0x1C, szHelp[7], 0);
                    }
                    else
                    {
                        setCursorSize(0x2020);
                        genSerialNumber(szUserName, CDKey);
                        writeVRM(18, 12, 0x7E, CDKey, 0);
                    }
                    break;
                case 1:
                    bSlc = 1;
                    hideMouse();
                    drawButton(51, 8 + bSlc * 2, _ATV, 3, szMenu[bSlc], 1, _FLT);
                    clearScreen(51, 8 + bSlc * 2, 64, 11, 3);
                    writeVRM(52, 8 + bSlc * 2, ATV, szMenu[bSlc], FLT);
                    delay(50);
                    drawButton(51, 8 + bSlc * 2, ATV, 3, szMenu[bSlc], 0, FLT);
                    showMouse();
                    return;
                case 2:
                    bSlc = 2;
                    drawButton(51, 8 + bSlc * 2, _ATV, 3, szMenu[bSlc], 1, _FLT);
                    clearScreen(51, 8 + bSlc * 2, 64, 13, 3);
                    writeVRM(52, 8 + bSlc * 2, ATV, szMenu[bSlc], FLT);
                    delay(50);
                    drawButton(51, 8 + bSlc * 2, ATV, 3, szMenu[bSlc], 0, FLT);
                    hideMouse();
                    getScreenText(15, 6, 53, 12, scrBuff);
                    shadowBox(15, 6, 65, 16, 0x5F, szHelp[0]);
                    writeVRM(17, 8, 0x5E, szHelp[1], 0);
                    writeVRM(17, 9, 0x5E, szHelp[2], 0);
                    writeVRM(17, 10, 0x5E, szHelp[3], 0);
                    writeVRM(17, 11, 0x5E, szHelp[4], 0);
                    writeVRM(17, 12, 0x5E, szHelp[5], 0);
                    drawButton(36, 14, ATV, 5, szHelp[6], 1, FLT);
                    showMouse();
                    setCursorSize(0x2020);

                    do {
                        if (kbhit())
                        {
                            readKey(&cKey);
                            if ((cKey == KEY_ENTER) || (cKey == KEY_SPACER) || (cKey == KEY_ESC))
                            {
                                hideMouse();
                                clearScreen(36, 14, 46, 15, 5);
                                writeVRM(37, 14, ATV, szHelp[6], FLT);
                                delay(50);
                                drawButton(36, 14, ATV, 5, szHelp[6], 1, FLT);
                                break;
                            }

                            if (clickMouse(&bCol, &bRow) && (bRow == 14 && bCol >= 36 && bCol <= 45))
                            {
                                hideMouse();
                                clearScreen(36, 14, 46, 15, 5);
                                writeVRM(37, 14, ATV, szHelp[6], FLT);
                                delay(50);
                                drawButton(36, 14, ATV, 5, szHelp[6], 1, FLT);
                                break;
                            }
                        }
                    } while (1);

                    setCursorSize(0x0B0A);
                    putScreenText(15, 6, 53, 12, scrBuff);
                    showMouse();
                    break;
                }
                break;
            }
        }

        if (clickMouse(&bCol, &bRow))
        {
            if (bRow == 8 && bCol >= 51 && bCol <= 62)
            {
                hideMouse();
                drawButton(51, 8 + bSlc * 2, _ATV, 3, szMenu[bSlc], 1, _FLT);
                bSlc = 0;
                clearScreen(51, 8 + bSlc * 2, 64, 9, 3);
                writeVRM(52, 8 + bSlc * 2, ATV, szMenu[bSlc], FLT);
                delay(50);
                drawButton(51, 8 + bSlc * 2, ATV, 3, szMenu[bSlc], 0, FLT);
                showMouse();
                
                if (!validUserName(szUserName))
                {
                    noUserName = 1;
                    setCursorSize(0x0B0A);
                    writeVRM(18, 9, 0x1C, szHelp[7], 0);
                }
                else
                {
                    setCursorSize(0x2020);
                    genSerialNumber(szUserName, CDKey);
                    writeVRM(18, 12, 0x7E, CDKey, 0);
                }
            }

            if (bRow == 10 && bCol >= 51 && bCol <= 62)
            {
                hideMouse();
                drawButton(51, 8 + bSlc * 2, _ATV, 3, szMenu[bSlc], 1, _FLT);
                bSlc = 1;
                clearScreen(51, 8 + bSlc * 2, 64, 11, 3);
                writeVRM(52, 8 + bSlc * 2, ATV, szMenu[bSlc], FLT);
                delay(50);
                drawButton(51, 8 + bSlc * 2, ATV, 3, szMenu[bSlc], 0, FLT);
                showMouse();
                return;
            }

            if (bRow == 12 && bCol >= 51 && bCol <= 62)
            {
                drawButton(51, 8 + bSlc * 2, _ATV, 3, szMenu[bSlc], 1, _FLT);
                bSlc = 2;
                clearScreen(51, 8 + bSlc * 2, 64, 13, 3);
                writeVRM(52, 8 + bSlc * 2, ATV, szMenu[bSlc], FLT);
                delay(50);
                drawButton(51, 8 + bSlc * 2, ATV, 3, szMenu[bSlc], 0, FLT);
                hideMouse();
                getScreenText(15, 6, 53, 12, scrBuff);
                shadowBox(15, 6, 65, 16, 0x5F, szHelp[0]);
                writeVRM(17, 8, 0x5E, szHelp[1], 0);
                writeVRM(17, 9, 0x5E, szHelp[2], 0);
                writeVRM(17, 10, 0x5E, szHelp[3], 0);
                writeVRM(17, 11, 0x5E, szHelp[4], 0);
                writeVRM(17, 12, 0x5E, szHelp[5], 0);
                drawButton(36, 14, ATV, 5, szHelp[6], 1, FLT);
                showMouse();
                setCursorSize(0x2020);

                do {
                    if (kbhit())
                    {
                        isASCII = readKey(&cKey);
                        if (!cKey) isASCII = readKey(&cKey);

                        if ((cKey == KEY_ENTER) || (cKey == KEY_SPACER))
                        {
                            hideMouse();
                            clearScreen(36, 14, 46, 15, 5);
                            writeVRM(37, 14, ATV, szHelp[6], FLT);
                            delay(50);
                            drawButton(36, 14, ATV, 5, szHelp[6], 1, FLT);
                            break;
                        }
                    }

                    if (clickMouse(&bCol, &bRow))
                    {
                        if (bRow == 14 && bCol >= 36 && bCol <= 45)
                        {
                            hideMouse();
                            clearScreen(36, 14, 46, 15, 5);
                            writeVRM(37, 14, ATV, szHelp[6], FLT);
                            delay(50);
                            drawButton(36, 14, ATV, 5, szHelp[6], 1, FLT);
                            break;
                        }
                    }
                } while (1);

                setCursorSize(0x0B0A);
                putScreenText(15, 6, 53, 12, scrBuff);
                showMouse();
            }
        }
    } while (1);
}

void main()
{
    int16_t i = 0;
    char cKey = 0;
    char *szMenu[] = {
        "La61y ma4 ca2i d9a85t",
        "Nha65p te6n cu3a ba5n",
        "Ma4 so61 ca2i d9a85t",
    };
    
    system("font on");
    for (i = 0; i < 3; i++) fontVNI(szMenu[i]);
    
    setBorder(30);
    setCursorSize(0x0B0A);
    setBlinking(0);
    fillFrame(1, 1, 80, 25, 0x7D, 178);
    shadowBox(15, 6, 65, 14, 0x3F, szMenu[0]);
    writeVRM(18, 8, 0x3F, szMenu[1],  0);
    writeVRM(18, 11, 0x3F, szMenu[2],  0);
    writeChar(18, 12, 0x7E, 30, 32);

    if (!initMouse()) system("mouse");

    setMousePos();
    srand(time(NULL));
    startCracking();
    cleanup();
}
