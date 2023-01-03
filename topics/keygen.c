/*------------------------------------------------------*/
/*      UNIVERSITY OF TECHNOLOGY HO CHI MINH CITY       */
/*          FACULTY OF INFORMATION TECHNOLOGY           */
/*             THE LICENSE GENERATION FILE              */
/*            Author : NGUYEN NGOC VAN                  */
/*             Class : 00DTH1                           */
/*      Student Code : 00DTH201                         */
/*            Course : 2000 - 2005                      */
/*     Writting date : 24/10/2001                       */
/*       Last Update : 12/11/2001                       */
/*------------------------------------------------------*/
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

#define SCR_WIDTH       160
#define MASK_BG         0x08
#define OFFSET(x, y)    (((x - 1) + 80 * (y - 1)) << 1)

#define UP		        72
#define DOWN	        80
#define ENTER	        13
#define SPACER	        32
#define ESC	            27

#define ATV 	        0xF0
#define FLT 	        0xFC
#define _ATV 	        0x78
#define _FLT 	        0x75

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
    *pmem = (attr << 8) | chr;
}

/*------------------------------------------------*/
/* Funtion : writeChar                            */
/* Purpose : Writting a character with attribute  */
/* Expects : (x,y) cordinate to write a character */
/*           (attr) attribute of character        */
/*           (len) length area                   */
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
        char szTmp[80] = {0};
        uint8_t i = 0, fltStop = 0, currX = x, bPos;

        strcpy(szTmp, str);
        for (i = 0; (i < strlen(szTmp) - 1) && !fltStop; i++)
        {
            if (szTmp[i] == 126) fltStop = 1;
        }

        memmove(&szTmp[i - 1], &szTmp[i], strlen(szTmp) - i + 1);
        bPos = i - 1;

        ptmp = szTmp;
        while (*ptmp) *pmem++ = (attr << 8) | *ptmp++;
        printChar(currX + bPos, y, lets, szTmp[bPos]);
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
    regs.h.bl = doblink;
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
/*           (bka) special character color        */
/*           (chr) special character                */
/* Returns : Nothing                                */
/*--------------------------------------------------*/
void fillFrame(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t bka, char chr)
{
    uint8_t y;
    for (y = y1; y <= y2; y++) writeChar(x1, y, bka, x2 - x1 + 1, chr);
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
/*            (bka) the attribute of the box  */
/* Returns  : Nothing                           */
/*----------------------------------------------*/
void drawBox(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t bka)
{
    drawFrame(x1, y1, x2, y2, bka);
    fillFrame(x1 + 1, y1 + 1, x2 - 1, y2 - 1, bka, 32);
    changeAttrib(x2 + 1, y1 + 1, x2 + 2, y2 + 1, MASK_BG);
    changeAttrib(x1 + 2, y2 + 1, x2 + 2, y2 + 1, MASK_BG);
}

/*----------------------------------------------*/
/* Function : shadowBox                         */
/* Purpose  : Draw a box with shadow (very art) */
/* Expects  : (x1,y1) cordinate top to left     */
/*            (x2,y2) cordinate bottom to right */
/*            (bka) the attribute of the box  */
/*            (title) the title of header     */
/* Returns  : Nothing                           */
/*----------------------------------------------*/
void shadowBox(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t bka, char *title)
{
    const uint8_t bkc = bka << 4;
    const char styles[] = {229, 252, 0};
    const uint16_t center = (x2 - x1 - strlen(title)) >> 1;
    changeAttrib(x2 + 1, y1 + 1, x2 + 2, y2 + 1, MASK_BG);
    changeAttrib(x1 + 2, y2 + 1, x2 + 2, y2 + 1, MASK_BG);
    drawBox(x1, y1, x2, y2, bka);
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
uint8_t clickMouse(uint16_t *col, uint16_t *row)
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
    if (len < 5) return 0;

    i = 0;
    while (isspace(szUserName[i++]));
    if (i >= len) return 0;

    for (i = 0; i != len; i++)
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

/*-----------------------------------------------*/
/* Function : genProductKey                      */
/* Mission  : Generate product installation key  */
/* Expects  : (szUserName) input user name       */
/*             (CDKey) output product key        */
/* Returns  : Nothing                            */
/*-----------------------------------------------*/
void genProductKey(char *user, char *license)
{
    char serial[64];
    uint16_t key = getEncryptKey(user);
    getcwd(serial, 64);
    getDiskSerial(serial[0], serial);
    makeLicenseKey(key, serial, license);
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
        "Hu7o71ng da64n la61y ma4 ca2i d9a85t",
        "Ba5n ca62n nha65p te6n cu3a ba5n va2o o6 thu71 nha61t va2 nha61n",
        "nu1t La61y ma4 o73 phi1a be6n pha3i. Ba5n se4 nha65n d9u7o75c ma4",
        "so61 ca2i d9a85t o73 o6 thu71 hai be6n du7o71i. Ba5n co1 the63 ghi",
        "ma4 so61 na2y va2o gia61y  d9e63 tie61n ha2nh d9a8ng ky1 chu7o7ng",
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
            case UP:
                if (!isASCII)
                {
                    drawButton(51, 8 + bSlc * 2, _ATV, 3, szMenu[bSlc], 1, _FLT);
                    if (bSlc < 1) bSlc = 2; else bSlc--;
                    drawButton(51, 8 + bSlc * 2, ATV, 3, szMenu[bSlc], 0, FLT);
                }
                break;
            case DOWN:
                if (!isASCII)
                {
                    drawButton(51, 8 + bSlc * 2, _ATV, 3, szMenu[bSlc], 1, _FLT);
                    if (bSlc > 1) bSlc = 0; else bSlc++;
                    drawButton(51, 8 + bSlc * 2, ATV, 3, szMenu[bSlc], 0, FLT);
                }
                break;
            case ENTER:
                clearScreen(51, 8 + bSlc * 2, 64, 9 + bSlc * 2, 3);
                writeText(52, 8 + bSlc * 2, ATV, szMenu[bSlc], FLT);
                delay(60);
                drawButton(51, 8 + bSlc * 2, ATV, 3, szMenu[bSlc], 0, FLT);
                switch (bSlc)
                {
                case 0:
                    if (validUserName(szUserName))
                    {
                        setCursorSize(0x2020);
                        genProductKey(szUserName, CDKey);
                        writeText(18, 12, 0x1E, CDKey, 0);
                    }
                    else
                    {
                        noUserName = 1;
                        setCursorSize(0x0B0A);
                        writeText(18, 9, 0x1C, szHelp[7], 0);
                    }
                    break;
                case 1: return;
                case 2:
                    getScreenText(15, 6, 53, 12, scrBuff);
                    shadowBox(15, 6, 65, 16, 0x5F, szHelp[0]);
                    writeText(17, 8, 0x5E, szHelp[1], 0);
                    writeText(17, 9, 0x5E, szHelp[2], 0);
                    writeText(17, 10, 0x5E, szHelp[3], 0);
                    writeText(17, 11, 0x5E, szHelp[4], 0);
                    writeText(17, 12, 0x5E, szHelp[5], 0);
                    drawButton(36, 14, ATV, 5, szHelp[6], 1, FLT);
                    setCursorSize(0x2020);

                    while (kbhit()) getch();
                    do {
                        if (kbhit() || clickMouse(&bCol, &bRow))
                        {
                            if (kbhit() || (bRow == 14 && bCol >= 36 && bCol <= 45))
                            {
                                clearScreen(36, 14, 46, 15, 5);
                                writeText(37, 14, ATV, szHelp[6], FLT);
                                delay(60);
                                drawButton(36, 14, ATV, 5, szHelp[6], 1, FLT);
                                break;
                            }
                        }
                    } while (1);
                    
                    while (kbhit()) getch();
                    setCursorSize(0x0B0A);
                    putScreenText(15, 6, 53, 12, scrBuff);
                    break;
                }
                break;
            }
        }

        if (clickMouse(&bCol, &bRow))
        {
            if (bRow == 8 && bCol >= 51 && bCol <= 62)
            {
                drawButton(51, 8 + bSlc * 2, _ATV, 3, szMenu[bSlc], 1, _FLT);
                bSlc = 0;
                clearScreen(51, 8 + bSlc * 2, 64, 9, 3);
                writeText(52, 8 + bSlc * 2, ATV, szMenu[bSlc], FLT);
                delay(60);
                drawButton(51, 8 + bSlc * 2, ATV, 3, szMenu[bSlc], 0, FLT);
                if (validUserName(szUserName))
                {
                    setCursorSize(0x2020);
                    genProductKey(szUserName, CDKey);
                    writeText(18, 12, 0x1E, CDKey, 0);
                }
                else
                {
                    noUserName = 1;
                    setCursorSize(0x0B0A);
                    writeText(18, 9, 0x1C, szHelp[7], 0);
                }
            }

            if (bRow == 10 && bCol >= 51 && bCol <= 62)
            {
                drawButton(51, 8 + bSlc * 2, _ATV, 3, szMenu[bSlc], 1, _FLT);
                bSlc = 1;
                clearScreen(51, 8 + bSlc * 2, 64, 9, 3);
                writeText(52, 8 + bSlc * 2, ATV, szMenu[bSlc], FLT);
                delay(60);
                drawButton(51, 8 + bSlc * 2, ATV, 3, szMenu[bSlc], 0, FLT);
                return;
            }

            if (bRow == 12 && bCol >= 51 && bCol <= 62)
            {
                drawButton(51, 8 + bSlc * 2, _ATV, 3, szMenu[bSlc], 1, _FLT);
                bSlc = 2;
                clearScreen(51, 8 + bSlc * 2, 64, 13, 3);
                writeText(52, 8 + bSlc * 2, ATV, szMenu[bSlc], FLT);
                delay(60);
                drawButton(51, 8 + bSlc * 2, ATV, 3, szMenu[bSlc], 0, FLT);
                getScreenText(15, 6, 53, 12, scrBuff);
                shadowBox(15, 6, 65, 16, 0x5F, szHelp[0]);
                writeText(17, 8, 0x5E, szHelp[1], 0);
                writeText(17, 9, 0x5E, szHelp[2], 0);
                writeText(17, 10, 0x5E, szHelp[3], 0);
                writeText(17, 11, 0x5E, szHelp[4], 0);
                writeText(17, 12, 0x5E, szHelp[5], 0);
                drawButton(36, 14, ATV, 5, szHelp[6], 1, FLT);
                setCursorSize(0x2020);

                while (kbhit()) getch();
                do {
                    if (kbhit() || clickMouse(&bCol, &bRow))
                    {
                        if (kbhit() || (bRow == 14 && bCol >= 36 && bCol <= 45))
                        {
                            clearScreen(36, 14, 46, 15, 5);
                            writeText(37, 14, ATV, szHelp[6], FLT);
                            delay(60);
                            drawButton(36, 14, ATV, 5, szHelp[6], 1, FLT);
                            break;
                        }
                    }
                } while (1);
                
                while (kbhit()) getch();
                setCursorSize(0x0B0A);
                putScreenText(15, 6, 53, 12, scrBuff);
            }

            if (bRow == 6 && bCol >= 15 && bCol <= 16) break;
        }
    } while (1);
}

void main()
{
    int16_t i = 0;
    char cKey = 0;
    char *szMenu[] = {
        "La61y ma4 ca2i d9a85t",
        "Nha65p ho5 va2 te6n",
        "Ma4 so61 ca2i d9a85t",
    };
    
    system("font on");
    for (i = 0; i < sizeof(szMenu) / sizeof(szMenu[0]); i++) fontVNI(szMenu[i]);
    
    setBorder(30);
    setCursorSize(0x0B0A);
    setBlinking(0);
    fillFrame(1, 1, 80, 25, 0x7D, 178);
    shadowBox(15, 6, 65, 14, 0x3F, szMenu[0]);
    writeText(18, 8, 0x3F, szMenu[1],  0);
    writeText(18, 11, 0x3F, szMenu[2],  0);
    writeChar(18, 12, 0x1E, 30, 32);

    if (!initMouse()) system("mouse");

    setMousePos();
    srand(time(0));
    startCracking();
    cleanup();
}
