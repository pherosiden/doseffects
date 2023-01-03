#include <dos.h>
#include <io.h>
#include <stdio.h>
#include <ctype.h>
#include <conio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <direct.h>
#include <stdarg.h>
#include <fcntl.h>

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
    uint16_t    year;           // Register year
    uint16_t    month;          // Register month
    uint16_t    day;            // Register day
    uint16_t    days;           // The number of days
    uint16_t    key;            // Encryption key
    uint16_t    verid;          // Validate license key
    uint8_t     license[20];    // License key
    char        path[33];       // The installation path
} regs_t;

char szInstallPath[32];     // Installation path
uint8_t bmAvalid = 0;       // Status of the mouse
uint8_t far *txtMem = (uint8_t far*)0xB8000000L;
uint16_t nFiles = 0, nDirs = 0;

char *sysInf[] = {
    "Go74 bo3 chu7o7ng tri2nh",
    "To1m ta81t tho6ng tin",
    "D9ang xo1a: %s",
    "Chu7o7ng tri2nh  TOPICS cu3a ba5n d9a4 he61t tho72i gian su73 du5ng. No1",
    "hoa5t d9o65ng kho6ng to61t trong ma1y ba5n nu74a va2 no1 se4 bi5 loa5i bo3",
    "ra kho3i he65 tho61ng cu3a ba5n. Ne61u ba5n ca3m tha61y ra82ng ba5n thi1ch",
    "chu7o7ng tri2nh na2y xin ba5n vui lo2ng d9a8ng ky1 no1 vo71i ta1c gia3!",
    "Ca1m o7n ba5n d9a4 cho5n va2 su73 du5ng  chu7o7ng tri2nh na2y. Xin cha2o",
    "ta5m bie65t va2 he5n ga85p la5i ca1c ba5n...",
    "Ta1c gia3: Nguye64n Ngo5c Va6n",
    "D9ie65n thoa5i: 8267231",
    "Chu7o7ng tri2nh TOPICS d9a4 go74 bo3 hoa2n ta61t.",
    "He61t ha5n su73 du5ng chu7o7ng tri2nh TOPICS",
    "  ~D9o62ng y1  ",
    "  ~Bo3 qua  ",
    "%2u ta65p tin d9a4 d9u7o75c go74 bo3.",
    "%2u thu7 mu5c d9a4 d9u7o75c go74 bo3."
};

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
    char buffer[80];
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
/* Function : setMousePos                     */
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
/* Function : initData                          */
/* Mission  : Initialize parameters for program */
/* Expects  : Nothing                           */
/* Returns  : Nothing                           */
/*----------------------------------------------*/
void initData()
{
    FILE *fp;
    int16_t i;
    regs_t reg;

    fp = fopen("register.dat", "rb");
    if (!fp)
    {
        fprintf(stderr, "System file error!");
        exit(1);
    }
    fread(&reg, sizeof(reg), 1, fp);
    fclose(fp);
    strcpy(szInstallPath, reg.path);
    for (i = 0; i < sizeof(sysInf) / sizeof(sysInf[0]); i++) fontVNI(sysInf[i]);
    system("font on");
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
    __asm {
        mov	ax, 0x03
        int 0x10
    }
}

/*---------------------------------------------*/
/* Funtion : removeFiles                       */
/* Purpose : Copy all files from the disk      */
/* Expects : (psrc) sources directory          */
/*           (pdst) destination directory      */
/* Returns : 1 for success                     */
/*           0 for failure                     */
/*---------------------------------------------*/
void removeFiles(char *psrc)
{
    uint16_t i;
    struct find_t entries;
    
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
            _dos_setfileattr(curFile, _A_NORMAL);
            unlink(curFile);
            printText(17, 10, 0x5E, sysInf[2], curFile);
            writeChar(17, 11, 0x1F, nFiles++ % 48, 219);
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
                sprintf(newDir, "%s\\%s", srcPath, entries.name);
                removeFiles(srcDir);
                _dos_setfileattr(newDir, _A_NORMAL);
                rmdir(newDir);
                nDirs++;
            }
        } while (!_dos_findnext(&entries));
    }
}

/*---------------------------------------*/
/* Funtion : startRemove                 */
/* Mission : Deleting the topics program */
/* Expects : Nothing                     */
/* Returns : Nothing                     */
/*---------------------------------------*/
void startRemove()
{
    char path[33];
    strcpy(path, szInstallPath);
    strcat(path, "*.*");
    setBorder(47);
    fillFrame(1, 1, 80, 25, 0x1F, 32);
    shadowBox(15, 8, 65, 15, 0x5F, sysInf[0]);
    drawButton(35, 13, 0xF7, 5, sysInf[14], 1, 0xF8);
    writeChar(17, 11, 0x17, 47, 176);
    removeFiles(path);
    rmdir(szInstallPath);
}

/*----------------------------------------------------*/
/* Funtion : showResults                              */
/* Mission : Information summary about delete program */
/* Expects : Nothing                                  */
/* Returns : Nothing                                  */
/*----------------------------------------------------*/
void showResults()
{
    uint16_t col = 0, row = 0, isOK = 0;

    fillFrame(1, 1, 80, 25, 0x1F, 32);
    shadowBox(18, 8, 62, 16, 0x7F, sysInf[1]);
    printText(28, 10, 0x70, sysInf[15], nFiles);
    printText(28, 11, 0x70, sysInf[16], nDirs);
    writeText(22, 12, 0x70, sysInf[11], 0);
    drawButton(35, 14, 0x4F, 7, sysInf[13], 1, 0x4A);

    while (kbhit()) getch();
    do {
        if (kbhit() || clickMouse(&col, &row))
        {
            if (kbhit() || (row == 14 && col >= 35 && col <= 45))
            {
                isOK = 1;
                clearScreen(35, 14, 46, 15,7);
                writeText(36, 14, 0x4F, sysInf[13], 0x4A);
                delay(60);
                drawButton(35, 14, 0x4F, 7, sysInf[13], 1, 0x4A);
            }

            if (row == 8 && col == 18 || col == 19) isOK = 1;
        }
    } while (!isOK);
}

/*--------------------------------------------------*/
/* Funtion : showMenu                               */
/* Mission : Showing the menu delete topics message */
/* Expects : Nothing                                */
/* Returns : Nothing                                */
/*--------------------------------------------------*/
void showMenu()
{
    char key = 0, szPath[25];
    int16_t slc = 0, isOK = 0;
    uint16_t col = 0, row = 0;

    drawButton(24, 18, 0xF0, 4, sysInf[13], 1, 0xF4);
    drawButton(46, 18, 0xE7, 4, sysInf[14], 1, 0xE8);

    if (!initMouse())
    {
        strcpy(szPath, szInstallPath);
        strcat(szPath, "drivers\\mouse.com");
        system(szPath);
    }

    setMousePos();

    while (kbhit()) getch();
    do {
        if (kbhit())
        {
            key = getch();
            if (!key) key = getch();
            switch(key)
            {
            case LEFT:
                drawButton(24 + slc * 22, 18, 0xE7, 4, sysInf[13 + slc], 1, 0xE8);
                if (slc <= 0) slc = 1; else slc--;
                drawButton(24 + slc * 22, 18, 0xF0, 4, sysInf[13 + slc], 1, 0xF4);
                break;
            case RIGHT:
                drawButton(24 + slc * 22, 18, 0xE7, 4, sysInf[13 + slc], 1,  0xE8);
                if (slc >= 1) slc = 0; else slc++;
                drawButton(24 + slc * 22, 18, 0xF0, 4, sysInf[13 + slc], 1, 0xF4);
                break;
            case ENTER:
                isOK = 1;
                clearScreen(24 + slc * 22, 18, 34 + slc * 22, 19, 4);
                writeText(25 + slc * 22, 18, 0xF0, sysInf[13 + slc], 0xF4);
                delay(50);
                drawButton(24 + slc * 22, 18, 0xF0, 4, sysInf[13 + slc], 1, 0xF4);
                break;
            }
        }

        if (clickMouse(&col, &row))
        {
            if (row == 18 && col >= 24 && col <= 33)
            {
                slc = 0;
                isOK = 1;
                drawButton(46, 18, 0xE7, 4, sysInf[14], 1, 0xE8);
                clearScreen(24, 18, 34, 19, 4);
                writeText(25, 18, 0xF0, sysInf[13], 0xF4);
                delay(50);
                drawButton(24, 18, 0xF0, 4, sysInf[13], 1, 0xF4);
            }

            if (row == 18 && col >= 46 && col <= 55)
            {
                slc = 1;
                drawButton(24, 18, 0xE7, 4, sysInf[13], 1, 0xE8);
                clearScreen(46, 18, 56, 19, 4);
                writeText(47, 18, 0xF0, sysInf[14], 0xF4);
                delay(50);
                drawButton(46, 18, 0xF0, 4, sysInf[14], 1, 0xF4);
            }

            if (row == 4 && col == 10 || col == 11) isOK = 1;
        }
    } while (!isOK);

    if (!slc)
    {
        startRemove();
        showResults();
    }
}

/*-----------------------------------------------*/
/* Funtion : showRemove                          */
/* Mission : Showing message deleting the topics */
/* Expects : Nothing                             */
/* Returns : Nothing                             */
/*-----------------------------------------------*/
void showRemove()
{
    setBorder(47);
    setCursorSize(0x2020);
    fillFrame(1, 1, 80, 25, 0x1F, 32);
    setBlinking(0);
    shadowBox(10, 4, 70, 20, 0x4F, sysInf[12]);
    writeText(12, 6, 0x4B, sysInf[3], 0);
    writeText(12, 7, 0x4B, sysInf[4], 0);
    writeText(12, 8, 0x4B, sysInf[5], 0);
    writeText(12, 9, 0x4B, sysInf[6], 0);
    writeText(12, 10, 0x4B, sysInf[7], 0);
    writeText(12, 11, 0x4B, sysInf[8], 0);
    writeText(40, 13, 0x4A, sysInf[9], 0);
    writeText(40, 14, 0x4A, sysInf[10], 0);
    writeChar(11, 16, 0x4F, 59, 196);
    printChar(10, 16, 0x4F, 195);
    printChar(70, 16, 0x4F, 180);
    showMenu();
}

void main()
{
    initData();
    showRemove();
    cleanup();
}
