/*--------------------------------------------------------------*/
/*        UNIVERSITY OF TECHNOLOGY HO CHI MINH CITY             */
/*             MAJOR OF INFORMATIC TECHNOLOGY                   */
/*            THE PROGRAM INSTALLATION TOPICS                   */
/*             Author : NGUYEN NGOC VAN                         */
/*              Class : 00DTH1                                  */
/*       Student Code : 00DTH201                                */
/*             Course : 2000 - 2005                             */
/*      Writting Date : 24/09/2001                              */
/*        Last Update : 12/10/2001                              */
/*--------------------------------------------------------------*/
/*        Environment : Borland C++ Ver 3.1 Application         */
/*        Source File : INSTALL.CPP                             */
/*        Memory Mode : Small                                   */
/*            Compile : BCC INSTALL.CPP                         */
/*        Call to run : INSTALL (none the project file)         */
/*--------------------------------------------------------------*/
/*               THE FUNCTIONS SUPPORT PROGRAM                  */
/*--------------------------------------------------------------*/
/* copy_all_files; showing_menu_setup; showing_install_program; */
/* set_text_color; get_text_color    ; get_back_color         ; */
/* restart_system; set_border_color  ; set_cursor_size        ; */
/* exit_program  ; restart_program   ; user_manual            ; */
/* choose_driver ; driver_number     ; check_disk_space       ; */
/* execute_driver; execute_setup     ; install_program        ; */
/* check_user    ; write_char        ; set_attribute          ; */
/* write_OFFS    ; box_shadow        ; fill_frame             ; */
/* repliacte     ; get_page          ; get_pos                ; */
/* putch_XY      ; frame             ; box                    ; */
/* fadein        ; introduction      ; out_text_shade         ; */
/* out_text_width; background        ; start_graphics         ; */
/* detect_SVGA256; update_register   ; update_program         . */
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
#include <graph.h>
#include <stdint.h>
#include <direct.h>
#include <sys/types.h>

#define MASK_BG         0x08
#define OFFSET(x, y)    (((x - 1) + (y - 1) * 80) << 1)

#define UP		72
#define DOWN	80
#define HOME	71
#define END		79
#define TAB		9
#define ESC		27
#define LEFT	75
#define RIGHT	77
#define ENTER	13
#define SPACE	32

#define wATV	0xF0
#define wFLT	0xFC
#define _wATV	0x78
#define _wFLT	0x74
#define _eATV	0xE7
#define _eFLT	0xE6

#define BOOT_SEG   0xFFFFL
#define BOOT_OFF   0x0000L
#define BOOT_ADR   ((BOOT_SEG << 16) | BOOT_OFF)
#define DOS_SEG	 0x0040L
#define RESET_FLAG 0x0072L
#define RESET_ADR	 ((DOS_SEG << 16) | RESET_FLAG)

typedef struct {                 // The struction storing the information
    uint8_t day;                     // The date of the program
    uint8_t month;                   // The month of the program
    uint8_t regs;                    // The register code
    uint8_t num;                     // The number of run program
    char serial[20];              // Product code
    char user[31];                // Register name user
    char disk[4];                 // The disk letter
} REG_INFO;

char *ptrSource;             // The pointer sources
char szDrive[4];                   // Symbol szDrive
uint8_t numFiles = 0;               // Number files to read and files to write
uint8_t bmAvalid = 0;               // Status of the mouse
uint16_t buffSize;                   // The szBuffer storing data

char **sysInfo, **sysMenu;
uint16_t infoNum, menuNum;

char *msgWarn[1], *msgExit[1], *msgCmp[1], *msgError[2];

char key = 0;
char szBuff[1024];
uint8_t msgSlc = 0, chs = 0, slc = 0;
uint16_t bCol = 0, bRow = 0;
uint8_t *txtMem = (uint8_t*)0xB8000000L;

/*-----------------------------------*/
/* Funtion : setBorder               */
/* Mission : Setting border color    */
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
/* Mission : Write a character to cordinate x, y */
/* Expects : (x,y) cordinate to write            */
/*           (chr) character to write            */
/* Returns : Nothing                             */
/*-----------------------------------------------*/
void printChar(uint8_t x, uint8_t y, char chr)
{
    char txt[2];
    txt[0] = chr;
    txt[1] = '\0';
    _settextposition(y, x);
    _outtext(txt);
}

/*-------------------------------------------------*/
/* Function : printXY                              */
/* Mission  : Write a character to cordinate x, y  */
/* Expects  : (x,y) cordinate to write             */
/*            (Chr) character to write             */
/*            (wAttr) attrib for the character     */
/* Returns  : Nothing                              */
/*-------------------------------------------------*/
void printXY(uint8_t x, uint8_t y, uint8_t attr, char chr)
{
    txtMem[OFFSET(x, y)    ] = chr;
    txtMem[OFFSET(x, y) + 1] = attr;
}

/*------------------------------------------------*/
/* Funtion : writeChar                            */
/* Mission : Writting a character with attribute  */
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
/* Mission  : Writing a character with attribute */
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
        for (; (i < strlen(szTmp) - 1) && !fltStop;)
        {
            if (szTmp[i++] == 126) fltStop = 1;
        }

        memmove(&szTmp[i - 1], &szTmp[i], strlen(szTmp) - i + 1);
        bPos = i - 1;
        i = 0;
        while (szTmp[i])
        {
            txtMem[OFFSET(x, y)] = szTmp[i++];
            txtMem[OFFSET(x++, y) + 1] = txtAtr;
        }

        printXY(currX + bPos, y, fstAttr, szTmp[bPos]);
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
/* Mission  : Writing a character with attribute */
/* Notices  : Intervention in video memory       */
/* Expects  : (x,y) cordinate needs to writting  */
/*            (attr) The attribute of string     */
/*            (szString) the string to format    */
/* Returns : Nothing                             */
/*-----------------------------------------------*/
void printVRM(uint8_t x, uint8_t y, uint8_t wAttr, char *szString, ...)
{
    char buffer[255];
    va_list parametres;
    va_start(parametres, szString);
    vsprintf(buffer, szString, parametres);
    writeVRM(x, y, wAttr, buffer, 0);
}


/*-----------------------------------------------*/
/* Function : drawButton                         */
/* Mission  : Define the button shadow           */
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
            printXY(x, y, txtAttr, styles[0]);
            printXY(x + bLen - 2, y, txtAttr, styles[1]);
            writeChar(x + 1, y + 1, wAttr, bLen - 1, styles[2]);
            writeChar(x + bLen - 1 , y, wAttr, 1, styles[3]);
        }
        else
        {
            writeVRM(x, y, txtAttr, szTitle, 0);
            printXY(x, y, txtAttr, styles[0]);
            printXY(x + bLen - 1, y, txtAttr, styles[1]);
            writeChar(x + 1, y + 1, wAttr, bLen, styles[2]);
            writeChar(x + bLen, y, wAttr, 1, styles[3]);
        }
    }
}

/*-----------------------------------------------*/
/* Function : drawFrame                          */
/* Mission  : Draw a frame with the edge is lane */
/* Expects  : (x1,y1) cordinate top to left      */
/*            (x2,y2) cordinate bottom to right  */
/* Returns  : Nothing                            */
/*-----------------------------------------------*/
void drawFrame(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2)
{
    int16_t k;
    char txt[2];

    txt[1] = '\0';
    printChar(x1, y1, 218);

    txt[0] = 193;
    for (k = x1 + 1; k < x2; k++) _outtext(txt);

    txt[0] = 191;
    _outtext(txt);
    printChar(x1, y2, 192);

    txt[0] = 194;
    for (k = x1 + 1; k < x2; k++) _outtext(txt);

    txt[0] = 225;
    _outtext(txt);

    for (k = y1 + 1; k < y2; k++)
    {
        printChar(x1, k, 179);
        printChar(x2, k, 224);
    }
}

/*---------------------------------------------------*/
/* Function : changeAttrib                           */
/* Mission  : Chage the attribute of the area screen */
/* Expects  : (x1,y1) cordinate top to left          */
/*            (x2,y2) cordinate bottom to right      */
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
/* Function : isBlinking                         */
/* Mission  : Redefine bits 7 in the text attrib */
/* Expects  : (doblink) = 0, back color is light */
/*                      = 1, text is blinking    */
/* Return   : Nothing                            */
/*-----------------------------------------------*/
void isBlinking(uint8_t doblink)
{
    union REGS regs;
    regs.h.ah = 0x10;
    regs.h.al = 0x03;
    regs.h.bl = doblink ? 1 : 0;
    int86(0x10, &regs, &regs);
}

/*---------------------------------------------------------*/
/* Function : readKey                                      */
/* Mission  : Read a key from the keyboard                 */
/* Expects  : (ch) get the key from the keyboard           */
/* Returns  : If the key is a extend key then return code  */
/*	           key and 0 value else return 1 value and code */
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

/*----------------------------------------------*/
/* Function : drawBox                           */
/* Mission  : Draw a box with color and border  */
/* Expects  : (x1,y1) cordinate top to left     */
/*            (x2,y2) cordinate bottom to right */
/*            (wAttr) the attribute of the box  */
/* Returns  : Nothing                           */
/*----------------------------------------------*/
void drawBox(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t wAttr)
{
    uint8_t oldBk = _getbkcolor();
    uint8_t oldCol = _gettextcolor();
    _setbkcolor(wAttr >> 4);
    _settextcolor(wAttr & 0x0F);
    drawFrame(x1, y1, x2, y2);
    _settextwindow(y1 + 1, x1 + 1, y2 - 1, x2 - 1);
    _clearscreen(_GWINDOW);
    _settextwindow(1, 1, 25, 80);
    changeAttrib(x2 + 1, y1 + 1, x2 + 2, y2 + 1, 0x08);
    changeAttrib(x1 + 2, y2 + 1, x2 + 2, y2 + 1, 0x08);
    _setbkcolor(oldBk);
    _settextcolor(oldCol);
}

/*----------------------------------------------*/
/* Function : drawShadowBox                     */
/* Mission  : Draw a box with shadow (very art) */
/* Expects  : (x1,y1) cordinate top to left     */
/*            (x2,y2) cordinate bottom to right */
/*            (wAttr) the attribute of the box  */
/*            (szTitle) the title of header     */
/* Returns  : Nothing                           */
/*----------------------------------------------*/
void drawShadowBox(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t wAttr, char *szTitle)
{
    const uint8_t bkc = wAttr << 4;
    const char szStyle[] = {229, 252, 0};
    const uint16_t bCenter = ((x2 - x1) >> 1) - (strlen(szTitle) >> 1);
    changeAttrib(x2 + 1, y1 + 1, x2 + 2, y2 + 1, MASK_BG);
    changeAttrib(x1 + 2, y2 + 1, x2 + 2, y2 + 1, MASK_BG);
    drawBox(x1, y1, x2, y2, wAttr);
    writeChar(x1 + 3, y1, bkc, x2 - x1 - 2, 32);
    writeVRM(x1 + bCenter, y1, bkc, szTitle, 0);
    printXY(x1 + 2, y1, bkc, 226);
    writeVRM(x1, y1, bkc >> 4, szStyle, 0);
}

/*--------------------------------------------------*/
/* Funtion : fillFrame                              */
/* Mission : To full the box with special character */
/* Expects : (x1,y1) cordinate top to left          */
/*           (x2,y2) cordinate bottom to right      */
/*           (wAttr) special character color        */
/*           (chr) special character                */
/* Returns : Nothing                                */
/*--------------------------------------------------*/
void fillFrame(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t wAttr, char Chr)
{
    uint8_t i;
    for (i = y1; i <= y2; i++) writeChar(x1, i, wAttr, x2 - x1 + 1, Chr);
}

/*----------------------------------*/
/* Function : initMouse             */
/* Mission  : Initialize mouse port */
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
/* Mission  : Get status button and cordinate col, row */
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
    return regs.x.bx;
}

/*-----------------------------------*/
/* Function : hideMouse              */
/* Mission  : Hide the mouse pointer */
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
/* Mission  : Showing the mouse pointer */
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
/* Mission  : Setting the limit col and row   */
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
/* Mission  : Move mouse pointer to new cordinate */
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
/* Mission  : Colosing mouse fumction */
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

/*----------------------------------------------*/
/* Function : clearScreen                       */
/* Mission  : clearScreen the part of window    */
/* Expects  : (x1,y1) cordinate top to left     */
/*            (x2,y2) cordinate bottom to right */
/*            (color) color needs clear         */
/* Returns  : Nothing                           */
/*----------------------------------------------*/
void clearScreen(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t color)
{
    _settextwindow(y1, x1, y2, x2);
    _setbkcolor(color);
    _clearscreen(_GWINDOW);
    _settextwindow(1, 1, 25, 80);
}

/*-----------------------------------------*/
/* Funtion : strPos                        */
/* Mission : Getting position of substring */
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
/* Mission : Inserted the char into string     */
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
/* Mission : Deleting the characters           */
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
/* Mission : Concat the string          */
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
/* Mission : Conver the char to the string      */
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
/* Mission : Decode font to Vietnamese  */
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

/*-----------------------------------------*/
/* Funtion : getText                       */
/* Mission : Get string at coordinate      */
/* Expects : (left, top, right, bot) coord */
/* Returns : Nothing                       */
/*-----------------------------------------*/
void getText(int16_t left, int16_t top, int16_t right, int16_t bot, char *dest)
{
    uint8_t *tmem;
    int16_t width, height, adjust;

    width = right - left + 1;
    height = bot - top + 1;
    adjust = 80*2 - 2*width;

    left--;
    top--;
    tmem = txtMem + top * 80 * 2 + left * 2;

    width *= 2;
    while (height--)
    {
        memcpy(dest, tmem, width);
        dest += width;
        tmem += adjust;
    }
}

/*-----------------------------------------*/
/* Funtion : putText                       */
/* Mission : Get string at coordinate      */
/* Expects : (left, top, right, bot) coord */
/* Returns : Nothing                       */
/*-----------------------------------------*/
void putText(int16_t left, int16_t top, int16_t right, int16_t bot, char *dest)
{
    uint8_t *tmem;
    int16_t width, height, adjust;

    width = right - left + 1;
    height = bot - top + 1;
    adjust = 80*2 - 2*width;

    left--;
    top--;
    tmem = txtMem + top * 80 * 2 + left * 2;

    width *= 2;
    while (height--)
    {
        memcpy(tmem, dest, width);
        dest += width;
        tmem += adjust;
    }
}

/*------------------------------------*/
/* Funtion : decodeFile               */
/* Mission : Decode file register.sys */
/* Expects : (inFile) The source file */
/*           (outFile) The dest file  */
/* Returns : Number of lines in file  */
/*------------------------------------*/
void decodeFile(const char *inFile, const char *outFile)
{
    int16_t c, key = 98;
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
    }

    fclose(inHandle);
    fclose(outHandle);
}

/*------------------------------------------------*/
/* Function : getTextFile                         */
/* Mission  : Reading information into data array */
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
    
    decodeFile(inFile, outFile);
    fp = fopen(outFile, "rt");
    if (!fp)
    {
        fprintf(stderr, "Error open file: %s", outFile);
        exit(1);   
    }

    while (fgets(szBuffer, 102, fp)) elems++;

    szData[0] = (char**)malloc(elems * sizeof(char*));
    if (!(szData[0]))
    {
        fprintf(stderr, "Not enough memory for count: %d\n", elems);
        exit(1);
    }

    elems = 0;
    rewind(fp);
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
/* Mission  : free block memory of the data */
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
}

/*-------------------------------*/
/* Funtion : haltSys             */
/* Mission : Restore environment */
/* Expects : Nothing             */
/* Returns : Nothing             */
/*-------------------------------*/
void haltSys()
{
    char szPath[28] = {0};
    strcpy(szPath, sysInfo[23]);
    strcat(szPath, "off");
    setBorder(0x00);
    _settextcursor(0x0607);
    _setbkcolor(0);
    _settextcolor(7);
    isBlinking(1);
    if (bmAvalid) closeMouse();
    _clearscreen(_GWINDOW);
    system(szPath);
    freeData();
    exit(EXIT_SUCCESS);
}

/*---------------------------------------------*/
/* Funtion : fadeIn                            */
/* Mission : Debrightness light of the monitor */
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
/* Mission  : Initialize parameters for program */
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
/* Mission : Restart the computer */
/* Expects : Nothing              */
/* Returns : Nothing              */
/*--------------------------------*/
void sysReboot()
{
    void((far *reset_ptr)()) = (void( far *)() )BOOT_ADR;
    *(int far *)RESET_ADR = 0;
    (*reset_ptr)();
}

/*---------------------------------------*/
/* Funtion : errorFile                   */
/* Mission : Display the error code      */
/* Expects : (handle) which file error   */
/*           (errortype) error file type */
/* Returns : Nothing                     */
/*---------------------------------------*/
void errorFile(char *szHandle, char *szErrorType)
{
    char errorCode[60];
    strcpy(errorCode, szErrorType);
    strcat(errorCode, szHandle);
    drawShadowBox(13,8,65,17,0x4F,sysInfo[1]);
    drawButton(33,15,0xF0,4,sysMenu[3],1,0xF4);
    writeVRM(39-strlen(errorCode)/2,10,0x4A,errorCode, 0);
    writeVRM(15,11,0x4F,sysInfo[53], 0);
    writeVRM(15,12,0x4F,sysInfo[54], 0);
    writeVRM(15,13,0x4F,sysInfo[55], 0);
    do {
        if(clickMouse(&bCol, &bRow) == 1) {
            if(bRow == 15 && bCol >= 33 && bCol <= 45) {
                hideMouse(); clearScreen(33,15,46,16,4);
                writeVRM(34,15,wATV,sysMenu[3],wFLT);
                delay(50); drawButton(33,15,wATV,4,sysMenu[3],1,wFLT);
                showMouse(); break;
            }
            if(bRow == 8 && bCol == 13 || bCol == 14) break;
        }
    } while(!kbhit()); haltSys();
}

/*-------------------------------------------*/
/* Funtion : copyFiles                       */
/* Mission : Copy all files from the disk    */
/* Expects : (szSource) The disk sources     */
/*           (szDest) The disk target        */
/*           (wAttr) file attribute          */
/* Returns : 1 : If complete or 0 if failure */
/*-------------------------------------------*/
uint8_t copyFiles(char *szSource, char *szDest, uint16_t wAttr)
{
    uint8_t resume;
    uint16_t entry, dir_entries;
    struct find_t dir_entry;
    struct find_t *ptr_dir_entry;
    char *ptr_pos;
    int source, target, n, files;
    size_t write_to, num_read, num_write;
    unsigned long total_read, total_write;
    char path[64], spec[64], str_entry[64],
    curr_file[68], new_file[68], new_dir[64];

    resume = 1;
    for(entry = strlen(szSource) - 1; szSource[entry] != '\\'; entry--);
    strcpy(path, szSource);
    path[entry] = 0;
    strcpy(curr_file, szSource);
    curr_file[entry + 1] = 0;
    strcpy(new_file, szSource);
    new_file[entry + 1] = 0;
    new_file[0] = szDest[0];
    strcpy(spec, &szSource[entry+1]);
    total_read = 0;
    total_write = 0;
    if(!_dos_findfirst(szSource, wAttr, &dir_entry)) {
        do {
            files = 0;
            ptr_pos = ptrSource;
            while(ptr_pos + 2*sizeof(dir_entry) < ptrSource + buffSize) {
                memcpy(ptr_pos, &dir_entry, sizeof(dir_entry));
                ptr_dir_entry = (struct find_t*)ptr_pos;
                ptr_pos += sizeof(dir_entry);
                curr_file[entry + 1] = '\0';
                strcat(curr_file, dir_entry.name);
                if(total_read == 0) {
                    if(_dos_open(curr_file, O_RDONLY, &source))
                    errorFile(curr_file,sysInfo[17]);
                }
                if(_dos_read(source, ptr_pos, buffSize - (ptr_pos - ptrSource), &num_read))
                errorFile(curr_file, sysInfo[20]);
                ptr_pos += num_read; total_read += num_read; files++;
                if(total_read == dir_entry.size) {
                    _dos_close(source); total_read = 0;
                    if(_dos_findnext(&dir_entry)) break;
                }
                else break;
            }
            ptr_dir_entry = (struct find_t*)ptrSource;
            ptr_pos = ptrSource + sizeof(dir_entry);
            for(n = 0; n < files; n++) {
                if(total_write == 0) {
                    new_file[entry + 1] = '\0';
                    strcat(new_file, ptr_dir_entry->name);
                    if(_dos_creat(new_file, ptr_dir_entry->attrib, &target))
                    errorFile(new_file,sysInfo[17]);
                    writeChar(30,11,0x99,33,32);
                    writeVRM(30,11,0x9A,new_file,0); delay(50);
                    writeChar(18,12,0xFF,6*numFiles++ / 14+2,219);
                    printVRM(50,13,0x9F,"%3d",	(numFiles % 103) > 100 ? 100 : numFiles % 103);
                }

                if(ptr_dir_entry->size > buffSize - (ptr_pos - ptrSource))
                    write_to = buffSize - (ptr_pos - ptrSource);
                else write_to = ptr_dir_entry->size;

                if((ptr_dir_entry->size - total_write) < write_to)
                write_to = (uint16_t)(ptr_dir_entry->size - total_write);

                if(_dos_write(target, ptr_pos, write_to, &num_write))
                errorFile(new_file,sysInfo[21]);
                if(num_write != write_to) errorFile(new_file,sysInfo[21]);
                total_write += num_write;
                ptr_pos += write_to;
                if(total_write == ptr_dir_entry->size) {
                    total_write = 0;
                    _dos_setftime(target, ptr_dir_entry->wr_date, ptr_dir_entry->wr_time);
                    _dos_close(target);
                }
                ptr_dir_entry = (struct find_t*)ptr_pos;
                ptr_pos += sizeof(dir_entry);
            }
        } while(total_read || !_dos_findnext(&dir_entry));
    }
    sprintf(str_entry, "%s\\*.*", path);
    if(!_dos_findfirst(str_entry,_A_SUBDIR | _A_NORMAL | _A_RDONLY | _A_HIDDEN | _A_SYSTEM, &dir_entry)) {
        do {
            if((dir_entry.attrib & _A_SUBDIR) && (dir_entry.name[0] != '.')) {
                sprintf(str_entry, "%s\\%s\\%s", path, dir_entry.name, spec);
                sprintf(new_dir, "%s\\%s", path, dir_entry.name);
                new_dir[0] = szDest[0];
                mkdir(new_dir);
                _dos_setfileattr(new_dir, dir_entry.attrib);
                resume = copyFiles(str_entry, szDest, wAttr);
            }
        } while(resume && !_dos_findnext(&dir_entry));
    }
    return resume;
}

/*---------------------------------------*/
/* Function : messageBox                 */
/* Mission  : Display the system message */
/* Expects  : (x1,y1,x2,y2) coordinates  */
/*            (szMsg) The messages array */
/*            (n) The elements of array  */
/* Return   : 1 or 0                     */
/*---------------------------------------*/
uint8_t messageBox(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, char *szMsg[], uint8_t n)
{
    char key = 0;
    uint8_t slc = 0, bCenter = x1 + (x2 - x1) / 2, i;

    _settextcursor(0x2020);
    drawShadowBox(x1,y1,x2,y2,0x4F,sysInfo[1]);
    showMouse(); moveMouse(bCenter,y2-3);
    for(i = 0; i < n; i++)
    writeVRM(bCenter-strlen(szMsg[i])/2+1,y1+2+i,0x4A,szMsg[i], 0);
    drawButton(bCenter-14,y2-2,wATV,4,sysMenu[0],1,wFLT);
    drawButton(bCenter+6,y2-2,_eATV,4,sysMenu[2],1,_eFLT);
    do {
        if(clickMouse(&bCol, &bRow) == 1) {
            if(bRow == y2-2 && bCol >= bCenter-14 && bCol <= bCenter-5) {
                hideMouse(); drawButton(bCenter+6,y2-2,_eATV,4,sysMenu[2],1,_eFLT);
                clearScreen(bCenter-14,y2-2,bCenter-3,y2-1,4);
                writeVRM(bCenter-13,y2-2,wATV,sysMenu[0],wFLT); delay(50);
                drawButton(bCenter-14,y2-2,wATV,4,sysMenu[0],1,wFLT); showMouse();
                slc = 0; key = ENTER;
            }
            if(bRow == y2-2 && bCol >= bCenter+6 && bCol <= bCenter+15) {
                hideMouse(); drawButton(bCenter-14,y2-2,_eATV,4,sysMenu[0],1,_eFLT);
                clearScreen(bCenter+6, y2-2,bCenter+16,y2-1,4);
                writeVRM(bCenter+7,y2-2,wATV,sysMenu[2],wFLT); delay(50);
                drawButton(bCenter+6,y2-2,wATV,4,sysMenu[2],1,wFLT); showMouse();
                slc = 1; key = ENTER;
            }
            if(bRow == y1 && bCol == x1 || bCol == x1+1) {slc = 1; key = ENTER;}
        }
        if(kbhit()) {
            key = getch(); if(!key) key = getch();
            switch(toupper(key)) {
            case LEFT :
                drawButton(bCenter-14+slc*20,y2-2,_eATV,4,sysMenu[slc*2],1,_eFLT);
                if(slc < 1) slc = 1; else slc--;
                drawButton(bCenter-14+slc*20,y2-2,wATV,4,sysMenu[slc*2],1,wFLT);
                break;
            case RIGHT :
                drawButton(bCenter-14+slc*20,y2-2,_eATV,4,sysMenu[slc*2],1,_eFLT);
                if(slc > 0) slc = 0; else slc++;
                drawButton(bCenter-14+slc*20,y2-2,wATV,4,sysMenu[slc*2],1,wFLT);
                break;
            }
        }
    } while(key != ENTER);
    return slc;
}

/*---------------------------------------*/
/* Function : warningBox                 */
/* Mission  : Display the system message */
/* Expects  : (x1,y1,x2,y2) coordinates  */
/*            (szMsg) The messages array */
/*            (n) The elements of array  */
/* Return   : 1 or 0                     */
/*---------------------------------------*/
void warningBox(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, char *szMsg[], uint8_t n)
{
    uint8_t bCenter = x1 + (x2 - x1) / 2, i = 0,
    bPos = bCenter - strlen(sysMenu[0])/2;
    char key;

    _settextcursor(0x2020); drawShadowBox(x1,y1,x2,y2,0x4F,sysInfo[1]);
    for(i = 0; i < n; i++)
    writeVRM(bCenter-strlen(szMsg[i])/2,y1+2+i,0x4A,szMsg[i], 0);
    drawButton(bPos,y2-2,wATV,4,sysMenu[0],1,wFLT);
    showMouse(); moveMouse(bCenter,y2-3);
    do {
        if(clickMouse(&bCol, &bRow) == 1) {
            if(bRow == y2-2 && bCol >= bPos  && bCol <= bPos + 9) {
                hideMouse(); clearScreen(bPos,y2-2,bPos+7,y2-1,4);
                writeVRM(bPos+1,y2-2,wATV,sysMenu[0],0xF4); delay(50);
                drawButton(bPos,y2-2,wATV,4,sysMenu[0],1,0xF4); showMouse();
                key = ENTER;
            }
            if(bRow == y1 && bCol == x1 || bCol == x1+1) break;
        }
        if(kbhit()) {
            key = getch(); if(!key) key = getch();
            switch(toupper(key)) {
            case 'D':
            case ESC : key = ENTER;
            }
        }
    } while(key != ENTER);
}

/*--------------------------------------*/
/* Funtion : checkUser                  */
/* Mission : Testing user serial number */
/* Expects : Nothing                    */
/* Returns : Nothing                    */
/*--------------------------------------*/
void checkUser()
{
    char szOldName[31], szOldID[20],
    szCurrName[31], szCurrID[20];
    FILE *fptr;
    REG_INFO trInfo;
    uint8_t i = 0, j = 0, flgCorrectName = 1, flgCorrectID = 0, isASCII = 0;

    _setbkcolor(1);
    _settextcolor(15);
    _settextcursor(0x2020); _clearscreen(_GWINDOW);
    fillFrame(1,1,80,25,0xF6,178);
    drawShadowBox(5,3,75,23,0x5F,sysInfo[2]);
    writeVRM(8,5,0x5A,sysInfo[56], 0); writeVRM(8,6,0x5A,sysInfo[57], 0);
    writeVRM(8,7,0x5A,sysInfo[58], 0); writeVRM(8,8,0x5A,sysInfo[59], 0);
    writeVRM(8,9,0x5A,sysInfo[60], 0); writeVRM(14,12,0x5E,sysMenu[9],0x5F);
    writeVRM(11,11,0x5B,"��", 0); writeVRM(8,14,0x5F,sysInfo[22], 0);
    writeChar(8,15,0x1F,30,32); writeVRM(8,17,0x5F,sysInfo[23], 0);
    writeChar(8,18,0x1F,30,32);
    drawButton(24,21,_wATV,5,sysMenu[1],1,_wFLT); writeVRM(11,12,0x5E,"��", 0);
    drawButton(47,21,wATV,5,sysMenu[4],1,wFLT); showMouse(); moveMouse(10,11);
    slc = 0; label1: writeVRM(14,11+slc,0x5B,sysMenu[slc+8],0x5A); key = 0;
    do {
        if(kbhit()) {
            key = getch(); if(!key) key = getch();
            switch(toupper(key)) {
            case DOWN :
                writeVRM(14,11+slc,0x5E,sysMenu[slc+8],0x5F);
                writeVRM(11,11+slc,0x5E,"��", 0);
                if(slc > 0) slc = 0; else slc++;
                writeVRM(14,11+slc,0x5B,sysMenu[slc+8],0x5A);
                writeVRM(11,11+slc,0x5B,"��", 0); break;
            case UP :
                writeVRM(14,11+slc,0x5E,sysMenu[slc+8],0x5F);
                writeVRM(11,11+slc,0x5E,"��", 0);
                if(slc < 1) slc = 1; else slc--;
                writeVRM(14,11+slc,0x5B,sysMenu[slc+8],0x5A);
                writeVRM(11,11+slc,0x5B,"��", 0); break;
            }
        }
        if(clickMouse(&bCol, &bRow) == 1) {
            if(bRow == 11 && bCol >= 11 && bCol <= 40) {
                hideMouse(); writeVRM(14,12,0x5E,sysMenu[9],0x5F);
                writeVRM(11,12,0x5E,"��", 0); delay(20);
                writeVRM(14,11,0x5B,sysMenu[8],0x5A);
                writeVRM(11,11,0x5B,"��", 0); showMouse(); slc = 0; key = TAB;
            }
            if(bRow == 12 && bCol >= 11 && bCol <= 55) {
                hideMouse(); writeVRM(14,11,0x5E,sysMenu[8],0x5F);
                writeVRM(11,11,0x5E,"��", 0); delay(20);
                writeVRM(14,12,0x5B,sysMenu[9],0x5A);
                writeVRM(11,12,0x5B,"��", 0); showMouse(); slc = 1; key = TAB;
            }
            if(bRow == 15 && bCol >= 8 && bCol <= 38) key = TAB;
            if(bRow == 18 && bCol >= 8 && bCol <= 38) key = TAB;
            if(bRow == 3 && bCol == 5 || bCol == 6) haltSys();
        }
    } while(key != TAB);
    if(slc) {
        hideMouse(); getText(20,10,62,17,szBuff);
        msgSlc = messageBox(20,10,60,16,msgExit,1);
        if(!msgSlc) haltSys(); hideMouse();
        putText(20,10,62,17,szBuff); showMouse(); goto label1;
}
    if((fptr = fopen(/*sysInfo[42]*/"register.dat","rb")) == NULL) {
        _setbkcolor(1);
        _settextcolor(15);
        setBorder(47); _settextcursor(0x2020);
        _clearscreen(_GWINDOW); writeVRM(31,10,0x4F,sysInfo[11], 0);
        writeVRM(20,12,0x1F,sysInfo[19], 0); writeVRM(20,13,0x1F,sysInfo[25], 0);
        getch(); haltSys();
    }
    fread(&trInfo,sizeof(REG_INFO),1,fptr); fclose(fptr);
    strcpy(szOldName, trInfo.user); strcpy(szOldID, trInfo.serial);
    szCurrName[0] = '\0'; szCurrID[0] = '\0'; label2:
    _settextcursor(0x0B0A); _settextposition(15,8); key = i = j = slc = 0;
    drawButton(24,21,wATV,5,sysMenu[1],1,wFLT);
    drawButton(47,21,_wATV,5,sysMenu[4],1,_wFLT);
    do {
        if(flgCorrectName) _settextposition(15,8+i); if(flgCorrectID) _settextposition(18,8+j);
        if(kbhit()) {
            isASCII = readKey(&key); if(!key) isASCII = readKey(&key);
            if(flgCorrectName) {
                if((isASCII && i < 30 && key != 8 && isalpha(key)) ||
                (key == 32 && i < 30)) {
                    szCurrName[i] = key; printXY(8+i,15,0x1F,key); i++;
                }
                if(key == 8 && i > 0) {
                    i--; printXY(8+i,15,0x1F,32);
                }
            }
            if(key == TAB && flgCorrectName) {
                flgCorrectName = 0; flgCorrectID = 1;
            }
            if(flgCorrectID) {
                if(isASCII && j < 19 && key != 8 && key != TAB && key != ENTER) {
                    szCurrID[j] = key; printXY(8+j,18,0x1F,key); j++;
                }
                if(key == 8 && j > 0) {
                    j--; printXY(8+j,18,0x1F,32);
                }
            }
            switch(toupper(key)) {
            case LEFT :
                if(!isASCII) {
                    drawButton(24+slc*23,21,_wATV,5,sysMenu[3*slc+1],1,_wFLT);
                    if(slc < 1) slc = 0; else slc--;
                    drawButton(24+slc*23,21,wATV,5,sysMenu[3*slc+1],1,wFLT);
                } break;
            case RIGHT :
                if(!isASCII) {
                    drawButton(24+slc*23,21,_wATV,5,sysMenu[3*slc+1],1,_wFLT);
                    if(slc > 0) slc = 1; else slc++;
                    drawButton(24+slc*23,21,wATV,5,sysMenu[3*slc+1],1,wFLT);
                }
            }
        }
        if(clickMouse(&bCol, &bRow) == 1) {
            if(bRow == 21 && bCol >= 24 && bCol <= 36) {
                hideMouse(); drawButton(47,21,_wATV,5,sysMenu[4],1,_wFLT);
                clearScreen(24,21,36,22,5); writeVRM(25,21,wATV,sysMenu[1],wFLT);
                delay(50); drawButton(24,21,wATV,5,sysMenu[1],1,wFLT);
                slc = 0; showMouse(); key = ENTER;
            }
            if(bRow == 21 && bCol >= 47 && bCol <= 59) {
                hideMouse(); drawButton(24,21,_wATV,5,sysMenu[1],1,_wFLT);
                clearScreen(47,21,59,22,5); writeVRM(48,21,wATV,sysMenu[2],wFLT);
                delay(50); drawButton(47,21,wATV,5,sysMenu[4],1,wFLT);
                slc = 1; showMouse(); key = ENTER;
            }
        }
    } while(key != ENTER);
    if(i) szCurrName[i] = '\0'; if(j) szCurrID[j] = '\0';
    if(!slc) {
        if(strcmp(szOldName, szCurrName) &&
            strcmp(szOldID, szCurrID)) {
            hideMouse(); getText(20,10,62,17,szBuff);
            flgCorrectName = 1; flgCorrectID = 0;
            warningBox(20,10,60,16,msgWarn,1); hideMouse();
            putText(20,10,62,17,szBuff); writeChar(8,15,0x1F,30,32);
            writeChar(8,18,0x1F,30,32); showMouse(); goto label2;
        }
        else
        if(!strcmp(szOldName, szCurrName) &&
            strcmp(szOldID, szCurrID)) {
            hideMouse(); getText(20,10,62,17,szBuff);
            flgCorrectName = 0; flgCorrectID = 1;
            warningBox(20,10,60,16,msgWarn,1); hideMouse();
            putText(20,10,62,17,szBuff); writeChar(8,18,0x1F,30,32);
            showMouse(); goto label2;
        }
        else
        if(strcmp(szOldName, szCurrName) &&
            !strcmp(szOldID, szCurrID)) {
            hideMouse(); getText(20,10,62,17,szBuff);
            flgCorrectName = 1; flgCorrectID = 0;
            warningBox(20,10,60,16,msgWarn,1); hideMouse();
            putText(20,10,62,17,szBuff);
            writeChar(8,15,0x1F,30,32); showMouse();
            goto label2;
        }
    }
    else {
        hideMouse(); getText(20,10,62,17,szBuff);
        msgSlc = messageBox(20,10,60,16,msgExit,1);
        if(!msgSlc) haltSys(); hideMouse(); putText(20,10,62,17,szBuff);
        showMouse(); goto label2;
    }
}

/*------------------------------------*/
/* Funtion : showHelpFile             */
/* Mission : Show the readme.hlp file */
/* Expects : Nothing                  */
/* Returns : Nothing                  */
/*------------------------------------*/
void showHelpFile()
{
    _setbkcolor(1);
    _settextcolor(15);
    setBorder(63); _settextcursor(0x2020);
    _clearscreen(_GWINDOW); fillFrame(1,1,80,25,0xF6,178);
    drawShadowBox(10,3,74,22,0xBE,sysInfo[3]);
    writeVRM(12,5,0xB0,sysInfo[61], 0); writeVRM(12,6,0xB0,sysInfo[62], 0);
    writeVRM(12,7,0xB0,sysInfo[63], 0); writeVRM(12,8,0xB0,sysInfo[64], 0);
    writeVRM(12,9,0xB0,sysInfo[65], 0); writeVRM(12,10,0xB0,sysInfo[66], 0);
    writeVRM(12,11,0xB0,sysInfo[67], 0); writeVRM(12,12,0xB0,sysInfo[68], 0);
    writeVRM(17,14,0xBC,sysInfo[69], 0); writeVRM(17,15,0xBC,sysInfo[70], 0);
    writeVRM(17,16,0xBC,sysInfo[71], 0); writeVRM(12,18,0xB1,sysInfo[72], 0);
    drawButton(47,20,_wATV,11,sysMenu[4],1,_wFLT);
    drawButton(26,20,wATV,11,sysMenu[1],1,wFLT);
    showMouse(); moveMouse(42,20); slc = chs = key = 0;
    do {
        if(kbhit()) {
            key = getch(); if(!key) key = getch();
            switch(toupper(key)) {
            case LEFT:
                drawButton(26+slc*21,20,_wATV,11,sysMenu[3*slc+1],1,_wFLT);
                if(slc < 1) slc = 0; else slc--;
                drawButton(26+slc*21,20,wATV,11,sysMenu[3*slc+1],1,wFLT); break;
            case RIGHT:
                drawButton(26+slc*21,20,_wATV,11,sysMenu[3*slc+1],1,_wFLT);
                if(slc > 0) slc = 1; else slc++;
                drawButton(26+slc*21,20,wATV,11,sysMenu[3*slc+1],1,wFLT); break;
            }
        }
        if(clickMouse(&bCol, &bRow) == 1) {
            if(bRow == 20 && bCol >= 26 && bCol <= 38) {
                hideMouse(); drawButton(47,20,_wATV,11,sysMenu[4],1,_wFLT);
                clearScreen(26,20,38,21,11); writeVRM(27,20,wATV,sysMenu[1],wFLT);
                delay(50); drawButton(26,20,wATV,11,sysMenu[1],1,wFLT);
                showMouse(); slc = 0; break;
            }
            if(bRow == 20 && bCol >= 46 && bCol <= 59) {
                hideMouse(); drawButton(26,20,_wATV,11,sysMenu[1],1,_wFLT);
                clearScreen(47,20,59,21,11); writeVRM(48,20,wATV,sysMenu[4],wFLT);
                delay(50); drawButton(47,20,wATV,11,sysMenu[4],1,wFLT);
                showMouse(); slc = 1; break;
            }
            if(bRow == 3 && bCol == 10 || bCol == 11) haltSys();
        }
    } while(key != ENTER); hideMouse(); if(slc) haltSys();
}

/*--------------------------------------------*/
/* Funtion : installProgram                   */
/* Mission : Setup the program on computer    */
/* Expects : (szDrive) the szDrive to set up  */
/* Returns : Nothing                          */
/*--------------------------------------------*/
void installProgram()
{
    uint8_t i;
    buffSize = 3500;
    ptrSource = (char*)malloc(buffSize);

    if (!ptrSource)
    {
        _setbkcolor(1);
        _settextcolor(15);
        _settextcursor(0x2020);
        _clearscreen(_GWINDOW);
        fillFrame(1,1,80,25,0xF6,178);
        hideMouse();
        drawShadowBox(15,8,65,17,0x4F,sysInfo[1]);
        writeVRM(27,10,0x4E,sysInfo[31], 0); writeVRM(17,11,0x4F,sysInfo[53], 0);
        writeVRM(17,12,0x4F,sysInfo[54], 0); writeVRM(17,13,0x4F,sysInfo[55], 0);
        drawButton(35,15,wATV,4,sysMenu[0],1,wFLT);
        showMouse();

        do {
            if (clickMouse(&bCol, &bRow) == 1)
            {
                if (bRow == 15 && bCol >= 35 && bCol <= 45)
                {
                    hideMouse();
                    clearScreen(35,15,46,16,4);
                    writeVRM(36,15,wATV,sysMenu[0],wFLT);
                    delay(50);
                    drawButton(35,15,wATV,4,sysMenu[0],1,wFLT);
                    showMouse();
                    break;
                }

                if (bRow == 8 && bCol == 15 || bCol == 16) break;
            }
        } while (!kbhit());
        haltSys();
    }

    setBorder(50);
    _setbkcolor(1);
    _settextcolor(15);
    _settextcursor(0x2020);
    _clearscreen(_GWINDOW);
    fillFrame(1,1,80,25,0xF6,178);
    drawShadowBox(15,6,65,17,0x9F,sysInfo[4]);
    writeVRM(18,11,0x9F,sysInfo[73], 0); writeVRM(18,8,0x9F,sysInfo[38], 0);
    writeVRM(18,9,0x9F,sysInfo[40], 0); writeChar(18,12,0x17,45,176);
    writeVRM(53,13,0x9F,"% ho�n t�t", 0); drawButton(35,15,_wATV,9,sysMenu[2],1,_wFLT);
    copyFiles("A:\\*.*", szDrive, _A_NORMAL | _A_RDONLY | _A_HIDDEN | _A_SYSTEM);
    delay(500);
    fillFrame(15,6,69,21,0xF6,178);
    drawShadowBox(18,10,62,15,0x1F,sysInfo[5]);
    writeVRM(22,12,0x1E,sysInfo[41], 0);
    writeChar(22,13,0x17,37,176);
    writeVRM(49,14,0x1A,"% ho�n t�t", 0);
    for(i = 1; i < 38; i++) {
        writeChar(22,13,0x1F,i,219);
        printVRM(46,14,0x1A,"%3d", 2*i + 26); delay(100);
    }
    fillFrame(15,6,69,21,0xF6,178); warningBox(25,10,55,15,msgCmp,1);
    free(ptrSource);
}

/*----------------------------------------------*/
/* Funtion : restartProgram                     */
/* Mission : Showing the restart system message */
/* Expects : Nothing                            */
/* Returns : Nothing                            */
/*----------------------------------------------*/
void restartProgram()
{
    showMouse();
    _setbkcolor(1);
    _settextcolor(15);
    _settextcursor(0x2020);
    setBorder(50);
    _clearscreen(_GWINDOW);
    fillFrame(1,1,80,25,0xF6,178);
    drawShadowBox(15,8,65,15,0x4F,sysInfo[6]);
    writeVRM(21,11,0x4A,sysMenu[11],0x4B); writeVRM(18,10,0x4F,"��", 0);
    writeVRM(21,10,0x4F,sysMenu[10],0x4E); writeVRM(18,11,0x4A,"��", 0);
    drawButton(26,13,0xB4,4,sysMenu[0],1,0xB1);
    drawButton(45,13,0x9F,4,sysMenu[3],1,0x94); moveMouse(40,13);
    slc = chs = key = 0;
    do {
        if(kbhit()) {
            key = getch(); if(!key) key = getch();
            switch(toupper(key)) {
            case DOWN :
                writeVRM(21,10+slc,0x4A,sysMenu[slc+10],0x4B);
                writeVRM(18,10+slc,0x4A,"��", 0);
                if(slc > 0) slc = 0; else slc++;
                writeVRM(21,10+slc,0x4F,sysMenu[slc+10],0x4E);
                writeVRM(18,10+slc,0x4F,"��", 0); break;
            case UP :
                writeVRM(21,10+slc,0x4A,sysMenu[slc+10],0x4B);
                writeVRM(18,10+slc,0x4A,"��", 0);
                if(slc < 1) slc = 1; else slc--;
                writeVRM(21,10+slc,0x4F,sysMenu[slc+10],0x4E);
                writeVRM(18,10+slc,0x4F,"��", 0); break;
            case LEFT :
                drawButton(26+chs*19,13,0x9F,4,sysMenu[3*chs],1,0x94);
                if(chs < 1) chs = 0; else chs--;
                drawButton(26+chs*19,13,0xB4,4,sysMenu[3*chs],1,0xB1); break;
            case RIGHT :
                drawButton(26+chs*19,13,0x9F,4,sysMenu[3*chs],1,0x94);
                if(chs > 0) chs = 1; else chs++;
                drawButton(26+chs*19,13,0xB4,4,sysMenu[3*chs],1,0xB1); break;
            }
        }
        if(clickMouse(&bCol, &bRow) == 1) {
            if(bRow == 10 && bCol >= 18 && bCol <= 62) {
                hideMouse(); writeVRM(21,11,0x4A,sysMenu[11],0x4B);
                writeVRM(18,11,0x4A,"��", 0); slc = 0;
                writeVRM(21,10,0x4F,sysMenu[10],0x4E);
                writeVRM(18,10,0x4F,"��", 0); showMouse(); delay(50);
            }
            if(bRow == 11 && bCol >= 18 && bCol <= 61) {
                hideMouse(); writeVRM(21,10,0x4A,sysMenu[10],0x4B);
                writeVRM(18,10,0x4A,"��", 0); slc = 1;
                writeVRM(21,11,0x4F,sysMenu[11],0x4E);
                writeVRM(18,11,0x4F,"��", 0); showMouse(); delay(50);
            }
            if(bRow == 13 && bCol >= 26 && bCol <= 35) {
                hideMouse(); drawButton(45,13,0x9F,4,sysMenu[3],1,0x94);
                clearScreen(26,13,36,14,4); writeVRM(27,13,0xB4,sysMenu[0],0xB1);
                delay(50); drawButton(26,13,0xB4,4,sysMenu[0],1,0xB1);
                showMouse(); chs = 0; break;
            }
            if(bRow == 13 && bCol >= 45 && bCol <= 56) {
                hideMouse(); drawButton(26,13,0x9F,4,sysMenu[0],1,0x94);
                clearScreen(45,13,57,14,4); writeVRM(46,13,0xB4,sysMenu[3],0xB1);
                delay(50); drawButton(45,13,0xB4,4,sysMenu[3],1,0xB1);
                showMouse(); chs = 1; break;
            }
            if(bRow == 8 && bCol == 15 || bCol == 16) haltSys();
        }
    } while(key != ENTER);
    if(!chs && !slc) sysReboot();
    else haltSys();
}

/*-------------------------------------*/
/* Funtion : assignDrive               */
/* Mission : Showing the szDrive message */
/* Expects : (slc) number of szDrive     */
/* Returns : Nothing                   */
/*-------------------------------------*/
void assignDrive(uint8_t bSelect)
{
    switch(bSelect) {
        case 0 : strcpy(szDrive,"B:\\"); break; case 1 : strcpy(szDrive,"C:\\"); break;
        case 2 : strcpy(szDrive,"D:\\"); break; case 3 : strcpy(szDrive,"E:\\");
    }
}

/*----------------------------------------*/
/* Funtion : chooseDrive                  */
/* Mission : Showing choose szDrive message */
/* Expects : Nothing                      */
/* Returns : Nothing                      */
/*----------------------------------------*/
void chooseDrive()
{
    uint8_t i;
    _settextcursor(0x2020);
    _setbkcolor(1);
    _settextcolor(15);
    _clearscreen(_GWINDOW);
    fillFrame(1,1,80,25,0xF6,178);
    drawShadowBox(20,8,60,17,0x3F,sysInfo[7]);
    writeVRM(31,10,0x34,sysInfo[24], 0);
    writeChar(29,11,0x35,22,193);
    for(i = 0; i < 4; i++) writeVRM(29,12+i,0x30,sysMenu[12+i],0x3A);
    drawButton(46,12,wATV,3,sysMenu[1],1,wFLT);
    drawButton(46,15,_wATV,3,sysMenu[4],1,_wFLT);
    moveMouse(38,11); slc = chs = 0;
    label:
    key = 0; writeVRM(29,12+slc,0x3F,sysMenu[12+slc],0x3E);
    do {
        if(clickMouse(&bCol, &bRow) == 1) {
            if(bRow == 12 && bCol >= 29 && bCol <= 41) {
                hideMouse(); writeVRM(29,12,0x3F,sysMenu[12],0x3E);
                if(slc != 0) writeVRM(29,12+slc,0x30,sysMenu[12+slc],0x3A);
                showMouse(); slc = 0; delay(20);
            }
            if(bRow == 13 && bCol >= 29 && bCol <= 41) {
                hideMouse(); writeVRM(29,13,0x3F,sysMenu[13],0x3E);
                if(slc != 1) writeVRM(29,12+slc,0x30,sysMenu[12+slc],0x3A);
                showMouse(); slc = 1; delay(20);
            }
            if(bRow == 14 && bCol >= 29 && bCol <= 41) {
                hideMouse(); writeVRM(29,14,0x3F,sysMenu[14],0x3E);
                if(slc != 2) writeVRM(29,12+slc,0x30,sysMenu[12+slc],0x3A);
                showMouse(); slc = 2; delay(20);
            }
            if(bRow == 15 && bCol >= 29 && bCol <= 41) {
                hideMouse(); writeVRM(29,15,0x3F,sysMenu[15],0x3E);
                if(slc != 3) writeVRM(29,12+slc,0x30,sysMenu[12+slc],0x3A);
                showMouse(); slc = 3; delay(20);
            }
            if(bRow == 12 && bCol >= 46 & bCol <= 57) {
                hideMouse(); drawButton(46,15,_wATV,3,sysMenu[4],1,_wFLT);
                clearScreen(46,12,57,13,3); writeVRM(47,12,wFLT,sysMenu[1],wFLT);
                delay(50); drawButton(46,12,wATV,3,sysMenu[1],1,wFLT);
                showMouse(); chs = 0; key = ENTER;
            }
            if(bRow == 15 && bCol >= 46 & bCol <= 57) {
                hideMouse(); drawButton(46,12,_wATV,3,sysMenu[1],1,_wFLT);
                clearScreen(46,15,57,16,3); writeVRM(47,15,wATV,sysMenu[2],wFLT);
                delay(50); drawButton(46,15,wATV,3,sysMenu[4],1,wFLT);
                showMouse(); chs = 1; key = ENTER;
            }
            if(bRow == 8 && bCol == 20 || bCol == 21) haltSys();
        }
        if(kbhit()) {
            key = getch(); if(!key) key = getch();
            switch(toupper(key)) {
            case UP:
                writeVRM(29,12+slc,0x30,sysMenu[12+slc],0x3A);
                if(slc < 1) slc = 3; else slc--;
                writeVRM(29,12+slc,0x3F,sysMenu[12+slc],0x3E); break;
            case DOWN:
                writeVRM(29,12+slc,0x30,sysMenu[12+slc],0x3A);
                if(slc >= 3) slc = 0; else slc++;
                writeVRM(29,12+slc,0x3F,sysMenu[12+slc],0x3E); break;
            case HOME:
                writeVRM(29,12+slc,0x30,sysMenu[12+slc],0x3A); slc = 0;
                writeVRM(29,12+slc,0x3F,sysMenu[12+slc],0x3E); break;
            case END:
                writeVRM(29,12+slc,0x30,sysMenu[12+slc],0x3A); slc = 3;
                writeVRM(29,12+slc,0x3F,sysMenu[12+slc],0x3E); break;
            case TAB:
            do {
                if(kbhit()) {
                    key = getch(); if(!key) key = getch();
                    switch(toupper(key)) {
                    case UP:
                        drawButton(46,12+3*chs,_wATV,3,sysMenu[3*chs+1],1,_wFLT);
                        if(chs < 1) chs = 1; else chs--;
                        drawButton(46,12+3*chs,wATV,3,sysMenu[3*chs+1],1,wFLT); break;
                    case DOWN:
                        drawButton(46,12+3*chs,_wATV,3,sysMenu[3*chs+1],1,_wFLT);
                        if(chs > 0) chs = 0; else chs++;
                        drawButton(46,12+3*chs,wATV,3,sysMenu[3*chs+1],1,wFLT); break;
                    }
                }
                if(clickMouse(&bCol, &bRow) == 1) {
                    if(bRow == 12 && bCol >= 46 & bCol <= 57) {
                        hideMouse(); drawButton(46,15,_wATV,3,sysMenu[4],1,_wFLT);
                        clearScreen(46,12,57,13,3);
                        writeVRM(47,12,wATV,sysMenu[1],wFLT); delay(50);
                        drawButton(46,12,wATV,3,sysMenu[1],1,wFLT); showMouse();
                        chs = 0; key = ENTER;
                    }
                    if(bRow == 15 && bCol >= 46 & bCol <= 57) {
                        hideMouse(); drawButton(46,12,_wATV,3,sysMenu[1],1,_wFLT);
                        clearScreen(46,15,57,16,3);
                        writeVRM(47,15,wATV,sysMenu[4],wFLT); delay(50);
                        drawButton(46,15,wATV,3,sysMenu[4],1,wFLT); showMouse();
                        chs = 1; key = ENTER;
                    }
                    if(bRow == 8 && bCol == 20 || bCol == 21) haltSys();
                }
            } while(key != ENTER); break;
            }
        }
    } while(key != ENTER);
    if(!chs) assignDrive(slc);
    else {
        hideMouse(); getText(20,10,62,17,szBuff);
        msgSlc = messageBox(20,10,60,16,msgExit,1);
        if(!msgSlc) haltSys(); hideMouse();
        putText(20,10,62,17,szBuff); showMouse(); goto label;
    }
}

/*----------------------------------------------------*/
/* Funtion : updateProgram                            */
/* Mission : Deleting files crack.com and install.com */
/* Expects : Nothing                                  */
/* Returns : Nothing                                  */
/*----------------------------------------------------*/
void updateProgram()
{
    FILE *fp;
    REG_INFO trInfo;
    char szPath[15];

    if(!(fp = fopen(sysInfo[42],"r+b"))) {
        _setbkcolor(1);
        _settextcolor(15);
        setBorder(47); _settextcursor(0x2020);
        _clearscreen(_GWINDOW); writeVRM(33,10,0x4F,sysInfo[11], 0);
        writeVRM(20,12,0x1F,sysInfo[19],0); writeVRM(20,13,0x1F,sysInfo[25], 0);
        getch(); haltSys();
    }
    fread(&trInfo,sizeof(REG_INFO),1,fp); strcpy(trInfo.disk,szDrive);
    fseek(fp,0L,SEEK_SET); fwrite(&trInfo,sizeof(REG_INFO),1,fp); fclose(fp);
    strcpy(szPath,szDrive); strcat(szPath,"crack.com"); unlink(szPath);
    szPath[3] = '\0'; strcat(szPath,"install.exe"); unlink(szPath);
    szPath[3] = '\0'; strcat(szPath,"guide.txt"); unlink(szPath);
}

/*-------------------------------------------*/
/* Function : getDrive                       */
/* Mission  : Return the number of the drier */
/* Expects  : (szDrive) Serial number szDrive  */
/* Returns  : number of szDrive                */
/*-------------------------------------------*/
uint8_t getDrive(char *szDrive)
{
    if(!strcmp(szDrive,"B:\\")) return 2; if(!strcmp(szDrive,"C:\\")) return 3;
    if(!strcmp(szDrive,"D:\\")) return 4; if(!strcmp(szDrive,"E:\\")) return 5;
    return 0;
}

/*--------------------------------------------------*/
/* Funtion : checkDisk                              */
/* Mission : Checking disk space and uint8_t available */
/* Expects : Nothing                                */
/* Returns : Nothing                                */
/*--------------------------------------------------*/
void checkDisk()
{
    union REGS inRegs, outRegs;
    unsigned long a, b, c, d;
    uint8_t i, bSkip = 0; setBorder(50);
    _setbkcolor(1);
    _settextcolor(15);
    _settextcursor(0x2020); _clearscreen(_GWINDOW); fillFrame(1,1,80,25,0xF6,178);
    drawShadowBox(15,5,65,20,0x7F,sysInfo[8]);
    writeVRM(18,7,0x70,sysInfo[39], 0); writeVRM(18,8,0x70,sysInfo[40], 0);
    writeVRM(18,10,0x70,sysInfo[29], 0); writeVRM(53,12,0x70,"% ho�n t�t", 0);
    writeChar(18,11,0x17,45,176);
    drawButton(36,18,wATV,7,sysMenu[2],1,wFLT); moveMouse(39,17);
    for(i = 0; i < 45 && !bSkip; i++) {
        writeChar(18+i,11,0x3F,1,219);
        printVRM(50,12,0x70,"%3u", 2*i+12);
        if(clickMouse(&bCol, &bRow) == 1) {
            if(bRow == 18 && bCol >= 36 && bCol <= 45) {
                hideMouse(); clearScreen(36,18,45,19,7);
                writeVRM(37,18,wATV,sysMenu[2],wATV);
                delay(50); drawButton(36,18,wATV,7,sysMenu[2],1,wFLT); showMouse();
                bSkip = 1; break;
            }
        }
        if(kbhit()) {
            key = getch(); if(!key) key = getch();
            switch(toupper(key)) {
            case 'B' :
            case ESC : bSkip = 1;
            } break;
        } delay(100);
    }
    writeVRM(18,14,0x70,sysInfo[30], 0);
    writeChar(18,15,0x17,45,176);
    writeVRM(53,16,0x70,"% ho�n t�t", 0);
    for(i = 0; i < 45 && !bSkip; i++) {
        writeChar(18+i,15,0x3F,1,219);
        printVRM(50,16,0x70,"%3u", 2*i+12);
        if(clickMouse(&bCol, &bRow) == 1) {
            if(bRow == 18 && bCol >= 36 && bCol <= 45) {
                hideMouse(); clearScreen(36,18,45,19,7);
                writeVRM(37,18,wATV,sysMenu[2],wATV);
                delay(50); drawButton(36,18,wATV,7,sysMenu[2],1,wFLT); showMouse(); break;
            }
        }
        if(kbhit()) {
            key = getch(); if(!key) key = getch();
            switch(toupper(key)) {
            case 'C' :
            case ESC : break;
            } break;
        } delay(100);
    }
    inRegs.h.ah = 0x36; inRegs.h.dl = getDrive(szDrive);
    int86(0x21,&inRegs,&outRegs);
    if(outRegs.x.ax == 0xFFFF) {
        drawShadowBox(20,9,55,15,0x4F,sysInfo[1]);
        writeVRM(22,11,0x4F,sysInfo[37], 0);
        drawButton(33,13,wATV,4,sysMenu[0],1,wFLT);
        do {
            if(clickMouse(&bCol, &bRow) == 1) {
                if(bRow == 13 && bCol >= 33 && bCol <= 43) {
                    hideMouse(); clearScreen(33,13,43,14,4);
                    writeVRM(34,13,wATV,sysMenu[0],wFLT); delay(50);
                    drawButton(33,13,wATV,4,sysMenu[0],1,wFLT); showMouse(); break;
                }
                if(bRow == 9 && bCol == 20 || bCol == 21) break;
            }
        } while(!kbhit()); haltSys();
    }
    a = outRegs.x.dx; b = outRegs.x.bx; c = outRegs.x.ax; d = outRegs.x.cx;
    fillFrame(1,1,80,25,0xF6,178); drawShadowBox(15,9,63,16,0x9E,sysInfo[9]);
    printVRM(18,11,0x9F,"T�ng dung l��ng tr�n ��a : %.3f MB", (float)a*c*d/1024.0/1024.0);
    printVRM(18,12,0x9F,"Dung l��ng c�n tr�ng tr�n ��a : %.3f MB", (float)b*c*d/1024.0/1024.0);
    drawButton(34,14,wATV,9,sysMenu[0],1,wFLT);
    moveMouse(38,12);
    if(((b*c*d) / 1024) <= 1024) {
        writeVRM(18,13,0x94,sysInfo[35], 0);
        do {
            if(clickMouse(&bCol, &bRow) == 1) {
                if(bRow == 14 && bCol >= 34 && bCol <= 41) {
                    clearScreen(34,14,42,15,9); writeVRM(35,14,wATV,sysMenu[0],wFLT);
                    delay(50); drawButton(34,14,wATV,9,sysMenu[0],1,wFLT); break;
                }
                if(bRow == 9 && bCol == 15 || bCol == 16) break;
            }
        } while(!kbhit()); haltSys();
    }
    do {
        if(clickMouse(&bCol, &bRow) == 1) {
            if(bRow == 14 && bCol >= 34 && bCol <= 41) {
                hideMouse(); clearScreen(34,14,42,15,9);
                writeVRM(35,14,wATV,sysMenu[0],wFLT); delay(50);
                drawButton(34,14,wATV,9,sysMenu[0],1,wFLT); showMouse(); break;
            }
            if(bRow == 9 && bCol == 15 || bCol == 16) break;
        }
    } while(!kbhit());
}

/*------------------------------------*/
/* Funtion : startInstall             */
/* Mission : Executting the functions */
/* Expects : Nothing                  */
/* Returns : Nothing                  */
/*------------------------------------*/
void startInstall()
{
    char szDisk[17];

    chooseDrive(); checkDisk(); //checkUser();
    installProgram(); updateProgram(); showHelpFile(); fadeIn();
    strcpy(szDisk,szDrive); strcat(szDisk,"TOPICS");
    chdir(szDisk); szDrive[2] = '\0'; system(szDrive); system("readme");
    restartProgram();
}

/*---------------------------------------------*/
/* Funtion : showInstall                       */
/* Mission : Showing information setup program */
/* Expects : Nothing                           */
/* Returns : Nothing                           */
/*---------------------------------------------*/
void showInstall()
{
    uint8_t i = 0, fltStop[3], found = 0, cancel = 0;
    memset(fltStop,0,3);
    _setbkcolor(1);
    _settextcolor(15);
    setBorder(59);
    _settextcursor(0x0B0A);
    _clearscreen(_GWINDOW);
    isBlinking(0);
    fillFrame(1,1,80,25,0xFD,178);
    drawShadowBox(8,2,74,23,0x1F,sysInfo[74]);
    writeVRM(13,4,0x1A,sysInfo[10], 0);
    writeChar(11,5,0x1B,61,205);
    writeVRM(12,6,0x1B,sysInfo[36], 0); writeVRM(23,8,0x1F,sysInfo[44], 0);
    writeVRM(23,9,0x1F,sysInfo[45], 0); writeVRM(23,10,0x1F,sysInfo[46], 0);
    writeVRM(23,12,0x1E,sysInfo[47], 0); writeVRM(23,13,0x1E,sysInfo[48], 0);
    writeVRM(23,14,0x1E,sysInfo[49], 0); writeVRM(23,16,0x1B,sysInfo[50], 0);
    writeVRM(23,17,0x1B,sysInfo[51], 0); writeVRM(23,18,0x1B,sysInfo[52], 0);
    writeChar(9,19,0x1F,65,196);
    printXY(8,19,0x1F,195);
    printXY(74,19,0x1F,180);
    writeVRM(14,11,0x1C,sysMenu[17],0x1E);
    writeVRM(14,15,0x1C,sysMenu[18],0x1E);
    
    if (!initMouse()) system(sysInfo[43]);
    setMousePos();
    
    drawButton(24,21,wATV,1,sysMenu[1],1,wFLT);
    drawButton(48,21,_wATV,1,sysMenu[4],1,_wFLT);
    
    label1:
    found = cancel = 0;
    for (i = 0; i < 3 && !found; i++) if(!fltStop[i]) found = 1;
    if(found) {
        i--; _settextcursor(0x0B0A); _settextposition(7+4*i,15);
        writeVRM(14,7+4*i,0x1A,sysMenu[16+i],0x1F); slc = i;
    }
    key = 0;
    do {
        if(kbhit()) {
            key = getch(); if(!key) key = getch();
            switch(toupper(key)) {
            case UP :
                if(!fltStop[slc]) writeVRM(14,7+4*slc,0x1C,sysMenu[16+slc],0x1E);
                else {
                    writeVRM(14,7+4*slc,0x1C,sysMenu[16+slc],0x1E);
                    printXY(15,7+4*slc,0x1C,251);
                }
                if(slc < 1) slc = 2; else slc--;
                if(!fltStop[slc]) writeVRM(14,7+4*slc,0x1A,sysMenu[16+slc],0x1F);
                else {
                    writeVRM(14,7+4*slc,0x1A,sysMenu[16+slc],0x1F);
                    printXY(15,7+4*slc,0x1A,251);
                } _settextposition(7+4*slc, 15); break;
            case DOWN :
                if(!fltStop[slc]) writeVRM(14,7+4*slc,0x1C,sysMenu[16+slc],0x1E);
                else {
                    writeVRM(14,7+4*slc,0x1C,sysMenu[16+slc],0x1E);
                    printXY(15,7+4*slc,0x1C,251);
                }
                if(slc >= 2) slc = 0; else slc++;
                if(!fltStop[slc]) writeVRM(14,7+4*slc,0x1A,sysMenu[16+slc],0x1F);
                else {
                    writeVRM(14,7+4*slc,0x1A,sysMenu[16+slc],0x1F);
                    printXY(15,7+4*slc,0x1A,251);
                } _settextposition(7+4*slc,15); break;
            case HOME :
                if(!fltStop[slc]) writeVRM(14,7+4*slc,0x1C,sysMenu[16+slc],0x1E);
                else {
                    writeVRM(14,7+4*slc,0x1C,sysMenu[16+slc],0x1E);
                    printXY(15,7+4*slc,0x1C,251);
                } slc = 0;
                if(!fltStop[slc]) writeVRM(14,7+4*slc,0x1A,sysMenu[16+slc],0x1F);
                else {
                    writeVRM(14,7+4*slc,0x1A,sysMenu[16+slc],0x1F);
                    printXY(15,7+4*slc,0x1A,251);
                } _settextposition(7+4*slc,15); break;
            case END :
                if(!fltStop[slc]) writeVRM(14,7+4*slc,0x1C,sysMenu[16+slc],0x1E);
                else {
                    writeVRM(14,7+4*slc,0x1C,sysMenu[16+slc],0x1E);
                    printXY(15,7+4*slc,0x1C,251);
                } slc = 2;
                if(!fltStop[slc]) writeVRM(14,7+4*slc,0x1A,sysMenu[16+slc],0x1F);
                else {
                    writeVRM(14,7+4*slc,0x1A,sysMenu[16+slc],0x1F);
                    printXY(15,7+4*slc,0x1A,251);
                } _settextposition(7+4*slc,15); break;
            case LEFT :
                drawButton(24+chs*24,21,_wATV,1,sysMenu[3*chs+1],1,_wFLT);
                if(chs < 1) chs = 1; else chs--;
                drawButton(24+chs*24,21,wATV,1,sysMenu[3*chs+1],1,wFLT); break;
            case RIGHT :
                drawButton(24+chs*24,21,_wATV,1,sysMenu[3*chs+1],1,_wFLT);
                if(chs > 0) chs = 0; else chs++;
                drawButton(24+chs*24,21,wATV,1,sysMenu[3*chs+1],1,wFLT); break;
            }
            if(key == 32) {
                if(fltStop[slc]) {
                    writeVRM(14,7+4*slc,0x1A,sysMenu[16+slc],0x1F);
                    fltStop[slc] = 0;
                } else { printXY(15,7+4*slc,0x1A,251); fltStop[slc] = 1; }
            }
        }
        if(clickMouse(&bCol, &bRow) == 1) {
            if(bRow == 7 && bCol >= 14 && bCol <= 48) {
                hideMouse();
                if(!fltStop[slc]) writeVRM(14,7+4*slc,0x1C,sysMenu[16+slc],0x1E);
                else {
                    writeVRM(14,7+4*slc,0x1C,sysMenu[16+slc],0x1E);
                    printXY(15,7+4*slc,0x1C,251);
                } slc = 0;
                if(!fltStop[slc]) {
                    writeVRM(14,7,0x1A,sysMenu[16],0x1F);
                    printXY(15,7,0x1A,251); fltStop[0] = 1;
                } else { writeVRM(14,7,0x1A,sysMenu[16],0x1F); fltStop[0] = 0; }
                _settextposition(7,15); showMouse(); delay(150);
            }
            if(bRow == 11 && bCol >= 14 && bCol <= 57) {
                hideMouse();
                if(!fltStop[slc]) writeVRM(14,7+4*slc,0x1C,sysMenu[16+slc],0x1E);
                else {
                    writeVRM(14,7+4*slc,0x1C,sysMenu[16+slc],0x1E);
                    printXY(15,7+4*slc,0x1C,251);
                } slc = 1;
                if(!fltStop[slc]) {
                    writeVRM(14,11,0x1A,sysMenu[17],0x1F);
                    printXY(15,11,0x1A,251); fltStop[1] = 1;
                } else { writeVRM(14,11,0x1A,sysMenu[17],0x1F); fltStop[1] = 0; }
                _settextposition(11,15); showMouse(); delay(150);
            }
            if(bRow == 15 && bCol >= 14 && bCol <= 52) {
                hideMouse();
                if(!fltStop[slc]) writeVRM(14,7+4*slc,0x1C,sysMenu[16+slc],0x1E);
                else {
                    writeVRM(14,7+4*slc,0x1C,sysMenu[16+slc],0x1E);
                    printXY(15,7+4*slc,0x1C,251);
                } slc = 2;
                if(!fltStop[slc]) {
                    writeVRM(14,15,0x1A,sysMenu[18],0x1F);
                    printXY(15,15,0x1A,251); fltStop[2] = 1;
                } else { writeVRM(14,15,0x1A,sysMenu[18],0x1F); fltStop[2] = 0; }
                _settextposition(15,15); showMouse(); delay(150);
            }
            if(bRow == 21 && bCol >= 24 && bCol <= 36) {
                hideMouse(); drawButton(48,21,_wATV,1,sysMenu[4],1,_wFLT);
                clearScreen(24,21,37,22,1); writeVRM(25,21,wATV,sysMenu[1],wFLT);
                delay(50); drawButton(24,21,wATV,1,sysMenu[1],1,wFLT);
                showMouse(); chs = 0; key = ENTER;
            }
            if(bRow == 21 && bCol >= 48 && bCol <= 60) {
                hideMouse(); drawButton(24,21,_wATV,1,sysMenu[1],1,_wFLT);
                clearScreen(48,21,61,22,1); writeVRM(49,21,wATV,sysMenu[4],wFLT);
                delay(50); drawButton(48,21,wATV,1,sysMenu[4],1,wFLT);
                showMouse(); chs = 1; key = ENTER;
            }
            if(bRow == 2 && bCol == 8 || bCol == 9) haltSys();
        }
    } while(key != ENTER);
    if(fltStop[slc]) {
        writeVRM(14,7+4*slc,0x1C,sysMenu[16+slc],0x1E);
        printXY(15,7+4*slc,0x1C,251);
    }
    else writeVRM(14,7+4*slc,0x1C,sysMenu[16+slc],0x1E);
    if(!chs && !cancel) {
        if(fltStop[1] && fltStop[2]) startInstall();
        else {
            hideMouse(); getText(20,10,63,18,szBuff);
            msgSlc = messageBox(20,10,61,17,msgError,2);
            if(msgSlc) haltSys();
            else {
                hideMouse(); putText(20,10,63,18,szBuff); showMouse();
                goto label1;
            }
        }
    }
    else {
        hideMouse();
        getText(20,10,63,17,szBuff);
        msgSlc = messageBox(20,10,61,16,msgExit,1);
        if (!msgSlc) haltSys();
        hideMouse();
        putText(20,10,63,17,szBuff);
        showMouse();
        goto label1;
    }
}

/*-------------------------------------------*/
/* Funtion : main                            */
/* Mission : Linker funtions to run program  */
/* Expects : Nothing                         */
/* Returns : Nothing                         */
/*-------------------------------------------*/
void main()
{
    _setvideomode(_TEXTC80);
    initData();
    showInstall();
    _setvideomode(_DEFAULTMODE);
}
/*-------------------------------------------------------------------------*/
/*------------------END OF SOURCES FILE PROGRAM----------------------------*/
/*-------------------------------------------------------------------------*/