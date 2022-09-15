#include <dos.h>
#include <io.h>
#include <stdio.h>
#include <ctype.h>
#include <conio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <graph.h>

#define MASK_BG         0x08
#define OFFSET(x, y)    (((x - 1) + 80 * (y - 1)) << 1)

#define LEFT    75
#define RIGHT   77
#define ENTER   13

#define wATV    0xF0
#define wFLT    0xFC
#define _wATV   0x78
#define _wFLT   0x74

typedef struct {            // Struction information
    uint8_t     day;        // The date of the program
    uint8_t     month;      // The month of the program
    uint8_t     regs;       // The register code
    uint8_t     num;        // The number of run program
    char        serial[20]; // License code
    char        user[31];   // User name
    char        disk[4];    // The disk letter
} REG_INFO;

char CDKey[20];             // Product code
char szUserName[31];        // Registers name user

uint8_t bmAvalid = 0;       // Status of the mouse
char **sysInfo = NULL;      // Text message
uint16_t sysNum = 0;        // Message count
uint8_t *txtMem = (uint8_t*)0xB8000000L;

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
/* Purpose  : Write a character to cordinate x, y  */
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
/* Purpose  : Draw a frame with the edge is lane */
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
/* Purpose  : Chage the attribute of the area screen */
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
    printXY(x1 + 2, y1, bkc, 226);
    writeVRM(x1, y1, bkc >> 4, szStyle, 0);
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
void fillFrame(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t wAttr, char Chr)
{
    uint8_t i;
    for (i = y1; i <= y2; i++) writeChar(x1, i, wAttr, x2 - x1 + 1, Chr);
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
    return regs.x.bx;
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
    _settextwindow(y1, x1, y2, x2);
    _setbkcolor(color);
    _clearscreen(_GWINDOW);
    _settextwindow(1, 1, 25, 80);
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


/*---------------------------------*/
/* Function : GetSerialNumber      */
/* Mission  : Getting the ID user  */
/* Expects  : Nothing              */
/* Returns  : Nothing              */
/*---------------------------------*/
void GetSerialNumber()
{
	FILE *fptr;
	tagInfo tmp;
	struct date da;
	uint8_t col = 0, row = 0;

	if(!(fptr = fopen("register.dat", "wb"))) HaltSys();
	getdate(&da); tmp.day = da.da_day; tmp.month = da.da_mon;
	tmp.regs = 0; tmp.num = 0; strcpy(tmp.serial, CDKey);
	strcpy(tmp.user, szUserName); strcpy(tmp.disk, "C:\\");
	fwrite(&tmp,sizeof(tagInfo),1,fptr); fclose(fptr);
	do {
		if(ClickMouse(col, row) == 1) {
			if(row == 10 && col >= 51 && col <= 62) {
				HideMouse(); drawButton(51,8,_ATV,3,"   ~L�y m�   ",0,_FLT);
				clearScreen(51,10,64,11,3); writeVRM(52,10,ATV,"   ~Ch�m d�t   ",FLT);
				delay(50); drawButton(51,10,ATV,3,"   ~Ch�m d�t   ",0,FLT);
				HaltSys();
			}
			if((col == 15 || col == 16) && row == 6) HaltSys();
		}
	} while(!kbhit()); HaltSys();
}

/*-------------------------------------------------*/
/* Function : startCracking                        */
/* Mission  : Showing the message to select option */
/* Expects  : Nothing                              */
/* Returns  : Nothing                              */
/*-------------------------------------------------*/
void startCracking()
{
    char *szMenu[] = {
        "   ~La61y ma4   ",
        "  ~Cha61m du71t  ",
        "  ~Tro75 giu1p  "
    };
    
	char cKey;
	uint8_t bSlc = 0, isASCII = 1, bCurrCol = 0, bCol = 0, bRow = 0, i;

	for(i = 0; i < 3; i++) fontVNI(szMenu[i]);
	_settextcursor(0x0B0A); drawButton(51,8,ATV,3,szMenu[0],0,FLT);
	drawButton(51,10,_ATV,3,szMenu[1],1,_FLT); MoveMouse(35,10);
	drawButton(51,12,_ATV,3,szMenu[2],1,_FLT);
	writeChar(18,9,0x1A,30,32); textattr(0x1A);
	do {
		gotoxy(18+bCurrCol,9);
		if(kbhit()) {
			isASCII = ReadKey(cKey);
			if(!cKey) isASCII = ReadKey(cKey);
			if((isASCII && bCurrCol < 30 && cKey != 8 && isalpha(cKey)) ||
				(cKey == 32 && bCurrCol < 30)) {
				szUserName[bCurrCol] = cKey;
				PrintXY(18+bCurrCol,9,0x1A,cKey); bCurrCol++;
			}
			if(cKey == 8 && bCurrCol > 0) {
				bCurrCol--; PrintXY(18+bCurrCol,9,0x1A,32);
			}
			switch(cKey) {
				case UP :
					if(!isASCII) {
						drawButton(51,8+bSlc*2,_ATV,3,szMenu[bSlc],1,_FLT);
						if(bSlc <= 0) bSlc = 2; else bSlc--;
						drawButton(51,8+bSlc*2,ATV,3,szMenu[bSlc],0,FLT);
					} break;
				case DOWN :
					if(!isASCII) {
						drawButton(51,8+bSlc*2,_ATV,3,szMenu[bSlc],1,_FLT);
						if(bSlc >= 2) bSlc = 0; else bSlc++;
						drawButton(51,8+bSlc*2,ATV,3,szMenu[bSlc],0,FLT);
					}
			}
		}
		if(ClickMouse(bCol, bRow) == 1) {
			if(bRow == 8 && bCol >= 51 && bCol <= 62) {
				HideMouse(); drawButton(51,8+bSlc*2,_ATV,3,szMenu[bSlc],1,_FLT);
				bSlc = 0; clearScreen(51,8+bSlc*2,64,9,3);
				writeVRM(52,8+bSlc*2,ATV,szMenu[bSlc],FLT); delay(50);
				drawButton(51,8+bSlc*2,ATV,3,szMenu[bSlc],0,FLT); ShowMouse();
				cKey = ENTER;
			}
			if(bRow == 10 && bCol >= 51 && bCol <= 62) {
				HideMouse(); drawButton(51,8+bSlc*2,_ATV,3,szMenu[bSlc],1,_FLT);
				bSlc = 1; clearScreen(51,8+bSlc*2,64,11,3);
				writeVRM(52,8+bSlc*2,ATV,szMenu[bSlc],FLT); delay(50);
				drawButton(51,8+bSlc*2,ATV,3,szMenu[bSlc],0,FLT); ShowMouse();
				cKey = ENTER;
			}
			if(bRow == 12 && bCol >= 51 && bCol <= 62) {
				char *MsgHelp[] = {
				"Hu7o71ng da64n la61y ma4 so61",
				"Ba5n ca62n nha65p te6n cu3a ba5n va2o o6 thu71 nha61t va2 nha61n",
				"nu1t la61y ma4 o73 phi1a be6n tra1i. Ba5n se4 nha65n d9u7o75c ma4",
				"so61 ca2i d9a85t o73 o6 thu71 hai be6n du7o71i. Ba5n co1 the63 ghi",
				"ma4 so61 na2y va2o gia61y  d9e63 tie61n ha2nh ca2i d9a85t chu7o7ng",
				"tri2nh. Nha61n va2o nu1t tra1i cu2ng d9e63 thoa1t.", "  ~D9o62ng y1  "};
				uint8_t bCol, bRow, i;
				char *buffer = new char[512];
				for(i = 0; i < 7; i++) fontVNI(MsgHelp[i]);
				drawButton(51,8+bSlc*2,_ATV,3,szMenu[bSlc],1,_FLT); bSlc = 2;
				clearScreen(51,8+bSlc*2,64,13,3);
				writeVRM(52,8+bSlc*2,ATV,szMenu[bSlc],FLT); delay(50);
				drawButton(51,8+bSlc*2,ATV,3,szMenu[bSlc],0,FLT); HideMouse();
				gettext(15,6,67,17,buffer);
				shadowBox(15,6,65,16,0x5F,MsgHelp[0]); writeVRM(17,8,0x5E,MsgHelp[1]);
				writeVRM(17,9,0x5E,MsgHelp[2]); writeVRM(17,10,0x5E,MsgHelp[3]);
				writeVRM(17,11,0x5E,MsgHelp[4]); writeVRM(17,12,0x5E,MsgHelp[5]);
				drawButton(36,14,ATV,5,MsgHelp[6],1,FLT); ShowMouse(); _settextcursor(0x2020);
				do {
					if(ClickMouse(bCol, bRow) == 1) {
						if(bRow == 14 && bCol >= 36 && bCol <= 45) {
							HideMouse(); clearScreen(36,14,46,15,5);
							writeVRM(37,14,ATV,MsgHelp[6],FLT); delay(50);
							drawButton(36,14,ATV,5,MsgHelp[6],1,FLT); break;
						}
					}
				} while(!kbhit()); _settextcursor(0x0B0A);
				puttext(15,6,67,17,buffer); ShowMouse(); delete[]buffer;
			}
			if(bRow == 6 && (bCol == 15 || bCol == 16)) HaltSys();
		}
		szUserName[bCurrCol] = NULL;
	} while(cKey != ENTER || !szUserName[0]);
	if(!bSlc) {
		_settextcursor(0x2020); writeVRM(18,12,0x1A,CDKey); GetSerialNumber();
	} else HaltSys();
}

void main()
{
	char cKey = 0;
    char *szMenu[] = {
        "Ma4 so61 ca2i d9a85t",
        "Nha65p te6n cu3a ba5n",
        "So61 ca2i d9a85t",
        "mouse.com",
        "font.com on"
    };
	
    int16_t i = 0;
    
    _setvideomode(_TEXTC80);
	system(szMenu[4]);
	for (i = 0; i < 3; i++) fontVNI(szMenu[i]);
    
    _setbkcolor(1);
    _settextcolor(15);
    setBorder(30);
	_settextcursor(0x0B0A);
    _clearscreen(_GWINDOW);
    setBlinking(0);
	fillFrame(1,1,80,25,0x7D,178);
	shadowBox(15,6,65,14,0x3F,szMenu[0]);
	writeVRM(18,8,0x3F,szMenu[1], 0);
	writeVRM(18,11,0x3F,szMenu[2], 0);
    writeChar(18,12,0x1A,30,32);

	if (!initMouse()) system(szMenu[3]);

	i = 0;
    setMousePos();
    srand(time(NULL));
    
	do {
		cKey = 48 + (rand() % 49);
		if (isdigit(cKey) || isupper(cKey)) CDKey[i++] = cKey;
	} while (i < 19);
    
    CDKey[i] = '\0';

	for (i = 0; i < strlen(CDKey) - 1; i++)
    {
	    if (!((i + 1) % 5)) CDKey[i] = 45;
    }

    startCracking();
    _setvideomode(_DEFAULTMODE);
}
