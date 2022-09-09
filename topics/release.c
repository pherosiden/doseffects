#include <dir.h>
#include <dos.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <string.h>
#include <stdarg.h>

#define LEFT	75
#define RIGHT	77
#define ENTER	13

#define VIDEO_ADDR	0xB800
#define MASK_BG		0x08
#define OFFSET(x, y) ( (x - 1)*2 + 160*(y - 1) )
#ifndef MK_FP
#define MK_FP(seg, ofs) ((void far*)((unsigned long)(seg) << 16 | (ofs)))
#endif

#define LEFT  75
#define RIGHT 77
#define ENTER 13

#define wATV 0xF0
#define wFLT 0xFC
#define _wATV 0x78
#define _wFLT 0x74

struct text_info txtInfo;    // Save information of the text mode
typedef unsigned char byte;  // Data type byte
typedef unsigned int word;   // Data type word
typedef byte bool;           // Data type boolean
char far *fpVRM =            // The pointer to video memory
(char far *)MK_FP(0xB800,0); // Address of monitor VGA
char szDrive[15];            // The disk letter
byte bmAvalid = 0;           // Status of the mouse
struct tagRegs {             // Struction information
	byte day;                 // The date of the program
	byte month;               // The month of the program
	byte regs;                // The register code
	byte num;                 // The number of run program
	char serial[20];          // Product code
	char user[31];            // Register name user
	char disk[4];             // The disk letter
};
static char *SysInf[] = {
"Go74 bo3 chu7o7ng tri2nh", "To1m ta81t tho6ng tin",
"D9ang xo1a, xin d9o75i mo65t chu1t...",
"Chu7o7ng tri2nh  TOPICS cu3a ba5n d9a4 he61t tho72i gian su73 du5ng. No1",
"hoa5t d9o65ng kho6ng to61t trong ma1y ba5n nu74a va2 no1 se4 bi5 loa5i bo3",
"ra kho3i he65 tho61ng cu3a ba5n. Ne61u ba5n ca3m tha61y ra82ng ba5n thi1ch",
"chu7o7ng tri2nh na2y xin ba5n vui lo2ng d9a8ng ky1 no1 vo71i ta1c gia3!",
"Ca1m o7n ba5n d9a4 cho5n va2 su73 du5ng  chu7o7ng tri2nh na2y. Xin cha2o",
"ta5m bie65t va2 he5n ga85p la5i ca1c ba5n...",
"Nguye64n Ngo5c Va6n author.", "D9ie65n thoa5i : 8267231",
"Chu7o7ng tri2nh TOPICS d9a4 go74 bo3 hoa2n ta61t.",
"Qu1a ha5n su73 du5ng chu7o7ng tri2nh TOPICS","TOPICS\\DRIVERS\\font on",
"  ~D9o62ng y1  ","  ~Bo3 qua  ","C:\\WINDOWS\\register.dat"};
word nFiles = 0, nDirs = 0;

/*-------------------------------------------------*/
/* Function : GetTextAttr                          */
/* Mission  : Save current attrib of the text mode */
/* Expects  : Nothing                              */
/* Returns  : Current color of the text mode       */
/*-------------------------------------------------*/
word GetTextAttr()
{
	gettextinfo(&txtInfo); return txtInfo.attribute;
}

/*-------------------------------------*/
/* Funtion : SetCursorSize             */
/* Mission : Resize the cursor         */
/* Expects : (size) The size of cursor */
/* Returns : Nothing                   */
/*-------------------------------------*/
void SetCursorSize(word size)
{
	asm {
		mov ah, 1
		mov cx, size
		int 0x10
	}
}

/*-----------------------------------*/
/* Funtion : SetBorder               */
/* Mission : Setting border color    */
/* Expects : (color) color of border */
/* Returns : Nothing                 */
/*-----------------------------------*/
void SetBorder(byte color)
{
	union REGS regs; regs.h.ah = 0x10; regs.h.al = 0x01;
	regs.h.bh = color & 63; int86(0x10, &regs, &regs);
}

/*-----------------------------------------------*/
/* Funtion : PrintChar                           */
/* Mission : Write a character to cordinate x, y */
/* Expects : (x,y) cordinate to write            */
/*           (chr) character to write            */
/* Returns : Nothing                             */
/*-----------------------------------------------*/
void PrintChar(byte x, byte y, char chr)
{
	gotoxy(x,y); putch(chr);
}

/*-------------------------------------------------*/
/* Function : PrintXY                              */
/* Mission  : Write a character to cordinate x, y  */
/* Expects  : (x,y) cordinate to write             */
/*            (Chr) character to write             */
/*            (wAttr) attrib for the character     */
/* Returns  : Nothing                              */
/*-------------------------------------------------*/
void PrintXY(byte x, byte y, word wAttr, char Chr)
{
	fpVRM[OFFSET(x, y)] = Chr; fpVRM[OFFSET(x, y) + 1] = wAttr;
}

/*------------------------------------------------*/
/* Funtion : WriteChar                            */
/* Mission : Writting a character with attribute  */
/* Expects : (x,y) cordinate to write a character */
/*           (wAttr) attribute of character       */
/*           (bLen) length area                   */
/*           (Chr) symbol needs to write          */
/* Returns : Nothing                              */
/*------------------------------------------------*/
void WriteChar(byte x, byte y, byte wAttr, byte bLen, char Chr)
{
	register word rwVideoOFS;
	register byte i;

	rwVideoOFS = OFFSET(x,y);
	for(i = 1; i <= bLen; i++) {
		poke(VIDEO_ADDR, rwVideoOFS, (wAttr << 8) + Chr); rwVideoOFS += 2;
	}
}

/*-----------------------------------------------*/
/* Function : WriteVRM                           */
/* Mission  : Writing a character with attribute */
/* Notices  : Intervention in video memory       */
/* Expects  : (x,y) cordinate needs to writting  */
/*            (txtAttr) The attribute of string  */
/*            (szPrmt) the string to format      */
/*            (fstAttr) The attr of first letter */
/* Returns : Nothing                             */
/*-----------------------------------------------*/
void WriteVRM(byte x, byte y, word txtAtr, const char *szPrmt, word fstAttr = 0)
{
	byte i = 0, fltStop = 0, currX = x, bPos;
	char *szTmp;

	if(fstAttr) {
		szTmp = new char[strlen(szPrmt)+1]; strcpy(szTmp, szPrmt);
		for(;(i < strlen(szTmp) - 1) && !fltStop;)
		if(szTmp[i++] == 126) fltStop = 1;
		memmove(&szTmp[i-1], &szTmp[i], strlen(szTmp) - i + 1);
		bPos = i - 1; i = 0;
		while(szTmp[i]) {
			fpVRM[OFFSET(x, y)] = szTmp[i++];
			fpVRM[OFFSET(x++, y) + 1] = txtAtr;
		}
		PrintXY(currX + bPos, y, fstAttr, szTmp[bPos]); delete[]szTmp;
	}
	else {
		while(*szPrmt) {
			fpVRM[OFFSET(x, y)] = *szPrmt++; fpVRM[OFFSET(x++, y)+1] = txtAtr;
		}
	}
}

/*-----------------------------------------------*/
/* Function : PrintVRM                           */
/* Mission  : Writing a character with attribute */
/* Notices  : Intervention in video memory       */
/* Expects  : (x,y) cordinate needs to writting  */
/*            (wAttr) The attribute of string    */
/*            (szMsg) the string to format       */
/* Returns : Nothing                             */
/*-----------------------------------------------*/
void PrintVRM(byte x, byte y, word wAttr, char *szMsg, ...)
{
	va_list vlPara; char szBuff[255]; va_start(vlPara,szMsg);
	vsprintf(szBuff, szMsg, vlPara); WriteVRM(x,y,wAttr,szBuff);
}

/*-----------------------------------------------*/
/* Function : Button                             */
/* Mission  : Define the button shadow           */
/* Expects  : (x,y) cordinate needs to writting  */
/*            (txtAttr) the attribute of a title */
/*            (szTitle) the string to format     */
/*            (bType) The type of button         */
/*            (fstAttr) The attr of first letter */
/* Returns : Nothing                             */
/*-----------------------------------------------*/
void Button(byte x, byte y, word txtAttr, byte bckClr, const char *szTitle, byte bType = 0, word fstAttr = 0)
{
	static const char Style[4] = {16, 17, 223, 220};
	byte bLen = strlen(szTitle);
	word wAttr = (bckClr << 4) + 1;

	if(bType) {
		if(fstAttr) {
			WriteVRM(x, y, txtAttr, szTitle, fstAttr);
			WriteChar(x+1, y+1, wAttr, bLen - 1, Style[2]);
			WriteChar(x+bLen-1, y, wAttr, 1, Style[3]);
		}
		else {
			WriteVRM(x, y, txtAttr, szTitle);
			WriteChar(x+1, y+1, wAttr, bLen, Style[2]);
			WriteChar(x+bLen, y, wAttr, 1, Style[3]);
		}
	}
	else {
		if(fstAttr) {
			WriteVRM(x, y, txtAttr, szTitle, fstAttr);
			PrintXY(x, y, txtAttr, Style[0]);
			PrintXY(x+bLen-2, y, txtAttr, Style[1]);
			WriteChar(x+1, y+1, wAttr, bLen - 1, Style[2]);
			WriteChar(x+bLen-1 , y, wAttr, 1, Style[3]);
		}
		else {
			WriteVRM(x, y, txtAttr, szTitle); PrintXY(x, y, txtAttr, Style[0]);
			PrintXY(x+bLen-1, y, txtAttr, Style[1]);
			WriteChar(x+1, y+1, wAttr, bLen, Style[2]);
			WriteChar(x+bLen, y, wAttr, 1, Style[3]);
		}
	}
}

/*-----------------------------------------------*/
/* Function : Frame                              */
/* Mission  : Draw a frame with the edge is lane */
/* Expects  : (x1,y1) cordinate top to left      */
/*            (x2,y2) cordinate bottom to right  */
/* Returns  : Nothing                            */
/*-----------------------------------------------*/
void Frame(byte x1, byte y1, byte x2, byte y2)
{
	register byte k;
	PrintChar(x1, y1, 218);
	for(k = x1 + 1; k < x2; putch(193), k++);
	putch(191); PrintChar(x1, y2,192);
	for(k = x1 + 1; k < x2; putch(194), k++); putch(225);
	for(k = y1 + 1; k < y2; k++) {
		PrintChar(x1,k,179);
		PrintChar(x2,k,224);
	}
}

/*---------------------------------------------------*/
/* Function : ChangeAttrib                           */
/* Mission  : Chage the attribute of the area screen */
/* Expects  : (x1,y1) cordinate top to left          */
/*            (x2,y2) cordinate bottom to right      */
/*            (wAttr) the attribute                  */
/* Returns  : Nothing                                */
/*---------------------------------------------------*/
void ChangeAttr(byte x1, byte y1, byte x2, byte y2, word wAttr)
{
	register byte bCol, bRow;

	for(bCol = x1; bCol <= x2; bCol++)
		for(bRow = y1; bRow <= y2; bRow++)
			pokeb(VIDEO_ADDR, OFFSET(bCol, bRow) + 1, wAttr);
}

/*-----------------------------------------------*/
/* Function : isBlinking                         */
/* Mission  : Redefine bits 7 in the text attrib */
/* Expects  : (doblink) = 0, back color is light */
/*                      = 1, text is blinking    */
/* Return   : Nothing                            */
/*-----------------------------------------------*/
void isBlinking(byte doblink)
{
	union REGS regs; regs.h.ah = 0x10; regs.h.al = 0x03;
	regs.h.bl = doblink ? 1 : 0; int86(0x10, &regs, &regs);
}

/*----------------------------------------------*/
/* Function : DlgBox                            */
/* Mission  : Draw a box with color and border  */
/* Expects  : (x1,y1) cordinate top to left     */
/*            (x2,y2) cordinate bottom to right */
/*            (wAttr) the attribute of the box  */
/* Returns  : Nothing                           */
/*----------------------------------------------*/
void DlgBox(byte x1, byte y1, byte x2, byte y2, word wAttr)
{
	word oldAttr;
	oldAttr = GetTextAttr(); textattr(wAttr);
	Frame(x1, y1, x2, y2); window(x1+1, y1+1, x2-1, y2-1);
	clrscr(); window(1, 1, 80 ,25);
	ChangeAttr(x2+1, y1+1, x2+2, y2+1, 0x08);
	ChangeAttr(x1+2, y2+1, x2+2, y2+1, 0x08);
	textattr(oldAttr);
}

/*----------------------------------------------*/
/* Function : DlgWin                            */
/* Mission  : Draw a box with shadow (very art) */
/* Expects  : (x1,y1) cordinate top to left     */
/*            (x2,y2) cordinate bottom to right */
/*            (wAttr) the attribute of the box  */
/*            (szTitle) the title of header     */
/* Returns  : Nothing                           */
/*----------------------------------------------*/
void DlgWin(byte x1, byte y1, byte x2, byte y2, word wAttr, char *szTitle)
{
	byte bCenter = ((x2 - x1) >> 1) - (strlen(szTitle) >> 1), bkc;
	const char szStyle[3] = {229,252,0};
	bkc = (wAttr << 4);
	ChangeAttr(x2+1, y1+1, x2+2, y2+1, MASK_BG);
	ChangeAttr(x1+2, y2+1, x2+2, y2+1, MASK_BG);
	DlgBox(x1, y1, x2, y2, wAttr);
	WriteChar(x1+3, y1, bkc, x2-x1-2, 32);
	WriteVRM(x1+bCenter, y1, bkc, szTitle);
	PrintXY(x1+2, y1, bkc, 226);
	WriteVRM(x1,y1,bkc >> 4,szStyle);
}


/*--------------------------------------------------*/
/* Funtion : FillFrame                              */
/* Mission : To full the box with special character */
/* Expects : (x1,y1) cordinate top to left          */
/*           (x2,y2) cordinate bottom to right      */
/*           (wAttr) special character color        */
/*           (chr) special character                */
/* Returns : Nothing                                */
/*--------------------------------------------------*/
void FillFrame(byte x1, byte y1, byte x2, byte y2, byte wAttr, char Chr)
{
	register byte i = y1;
	for(;i <= y2;) WriteChar(x1, i++, wAttr, x2 - x1 + 1, Chr);
}

/*----------------------------------*/
/* Function : InitMouse             */
/* Mission  : Initialize mouse port */
/* Expects  : Nothing               */
/* Returns  : 1 if success. 0 fail  */
/*----------------------------------*/
byte InitMouse()
{
	union REGS regs; regs.x.ax = 0x00; int86(0x33, &regs, &regs);
	if(regs.x.ax != 0xFFFF) return 0; regs.x.ax = 0x01;
	int86(0x33, &regs, &regs); bmAvalid = 1; return 1;
}

/*-----------------------------------------------------*/
/* Function : ClickMouse                               */
/* Mission  : Get status button and cordinate col, row */
/* Expects  : (bCol, bRow) cordinate of col and row    */
/* Returns  : Value 1 : left button, 2 : right button, */
/*            4 : center button and col, row           */
/*-----------------------------------------------------*/
byte ClickMouse(byte &bCol, byte &bRow)
{
	union REGS regs; regs.x.ax = 0x03; int86(0x33, &regs, &regs);
	bRow = (regs.x.dx >> 3) + 1; bCol = (regs.x.cx >> 3) + 1; return regs.x.bx;
}

/*-----------------------------------*/
/* Function : HideMouse              */
/* Mission  : Hide the mouse pointer */
/* Expects  : Nothing                */
/* Returns  : Nothing                */
/*-----------------------------------*/
void HideMouse()
{
	union REGS regs; regs.x.ax = 0x02; int86(0x33, &regs, &regs);
}

/*--------------------------------------*/
/* Function : ShowMouse                 */
/* Mission  : Showing the mouse pointer */
/* Expects  : Nothing                   */
/* Returns  : Nothing                   */
/*--------------------------------------*/
void ShowMouse()
{
	union REGS regs; regs.x.ax = 0x01; int86(0x33, &regs, &regs);
}

/*--------------------------------------------*/
/* Function : SetPos                          */
/* Mission  : Setting the limit bCol and bRow */
/* Expects  : Nothing                         */
/* Returns  : Nothing                         */
/*--------------------------------------------*/
void SetMousePos()
{
	union REGS regs; regs.x.ax = 0x07; regs.x.cx = 0; regs.x.dx = 2*320 - 8;
	int86(0x33, &regs, &regs); regs.x.ax = 0x08; regs.x.cx = 0;
	regs.x.dx = 200 - 8; int86(0x33, &regs, &regs); regs.x.ax = 0x1D;
	regs.x.bx = 0; int86(0x33, &regs, &regs);
}

/*------------------------------------------------*/
/* Function : MoveMouse                           */
/* Mission  : Move mouse pointer to new cordinate */
/* Expects  : (x, y) the new cordinate            */
/* Returns  : Nothing                             */
/*------------------------------------------------*/
void MoveMouse(byte x, byte y)
{
	union REGS regs; regs.x.ax = 0x0004; regs.x.cx = (x << 3) - 1;
	regs.x.dx = (y << 3) - 1; int86(0x33, &regs, &regs);
}

/*------------------------------------*/
/* Function : CloseMouse              */
/* Mission  : Colosing mouse fumction */
/* Expects  : Nothing                 */
/* Returns  : Nothing                 */
/*------------------------------------*/
void CloseMouse()
{
	union REGS regs; HideMouse(); regs.x.ax = 0; int86(0x33, &regs, &regs);
}

/*----------------------------------------------*/
/* Function : Clear                             */
/* Mission  : Clear the part of window          */
/* Expects  : (x1,y1) cordinate top to left     */
/*            (x2,y2) cordinate bottom to right */
/*            (color) color needs clear         */
/* Returns  : Nothing                           */
/*----------------------------------------------*/
void Clear(byte x1, byte y1, byte x2, byte y2, byte color)
{
	window(x1,y1,x2,y2); textattr((color << 4));
	clrscr(); window(1,1,80,25);
}

/*-----------------------------------------*/
/* Funtion : strPos                        */
/* Mission : Getting position of substring */
/* Expects : (str) The string main         */
/*           (substr) The substring        */
/* Returns : Position of substring         */
/*-----------------------------------------*/
int StrPos(char *str, char *szSubstr)
{
	char *ptr = strstr(str, szSubstr);
	if(!ptr) return -1;
	return (ptr - str);
}

/*---------------------------------------------*/
/* Funtion : InsertChar                        */
/* Mission : Inserted the char into string     */
/* Expects : (str) The string                  */
/*           (chr) The character need inserted */
/*           (iPos) The position inserted      */
/* Returns : Nothing                           */
/*---------------------------------------------*/
void InsertChar(char *str, char chr, int iPos)
{
	if(iPos < 0 || iPos > strlen(str)) return;
	*(str+iPos) = chr;
}

/*---------------------------------------------*/
/* Funtion : StrDelete                         */
/* Mission : Deleting the characters           */
/* Expects : (str) The string main             */
/*           (i) position to delete            */
/*           (numchar) the number of character */
/* Returns : Nothing                           */
/*---------------------------------------------*/
void StrDelete(char *str, int i, int numchar)
{
	if(i < 0 || i > strlen(str)) return;
	memcpy((str+i+1), (str+i+numchar), (strlen(str)-i-1));
}

/*--------------------------------------*/
/* Funtion : SchRepl                    */
/* Mission : Concat the string          */
/* Expects : (str) The string main      */
/*           (sch) The substring        */
/*           (repl) The char to replace */
/* Returns : Nothing                    */
/*--------------------------------------*/
void SchRepl(char *str, char *sch, char repl)
{
	int i;

	do {
		i = StrPos(str,sch);
		if(i >= 0) {
			StrDelete(str,i,strlen(sch));
			InsertChar(str,repl,i);
		}
	} while(i >= 0);
}

/*--------------------------------------------*/
/* Funtion : Chr2Str                          */
/* Mission : Conver the char to the string    */
/* Expects : (chr, n) The character to conver */
/* Returns : The pointer to string            */
/*--------------------------------------------*/
char *Chr2Str(char chr, char n)
{
	char *szString = new char[3];
	szString[0] = chr;
	szString[1] = n;
	szString[2] = '\0';
	return szString;
}

/*--------------------------------------*/
/* Funtion : FontVNI                    */
/* Mission : Decode font to Vietnamese  */
/* Expects : (str) The string to decode */
/* Returns : Nothing                    */
/*--------------------------------------*/
void FontVNI(char *szPrmpt)
{
	SchRepl(szPrmpt,"a8",128);
	SchRepl(szPrmpt,Chr2Str(128,'1'),129);
	SchRepl(szPrmpt,Chr2Str(128,'2'),130);
	SchRepl(szPrmpt,Chr2Str(128,'3'),131);
	SchRepl(szPrmpt,Chr2Str(128,'4'),132);
	SchRepl(szPrmpt,Chr2Str(128,'5'),133);
	SchRepl(szPrmpt,"a6",134);
	SchRepl(szPrmpt,Chr2Str(134,'1'),135);
	SchRepl(szPrmpt,Chr2Str(134,'2'),136);
	SchRepl(szPrmpt,Chr2Str(134,'3'),137);
	SchRepl(szPrmpt,Chr2Str(134,'4'),138);
	SchRepl(szPrmpt,Chr2Str(134,'5'),139);
	SchRepl(szPrmpt,"e6",140);
	SchRepl(szPrmpt,Chr2Str(140,'1'),141);
	SchRepl(szPrmpt,Chr2Str(140,'2'),142);
	SchRepl(szPrmpt,Chr2Str(140,'3'),143);
	SchRepl(szPrmpt,Chr2Str(140,'4'),144);
	SchRepl(szPrmpt,Chr2Str(140,'5'),145);
	SchRepl(szPrmpt,"o7",146);
	SchRepl(szPrmpt,Chr2Str(146,'1'),147);
	SchRepl(szPrmpt,Chr2Str(146,'2'),148);
	SchRepl(szPrmpt,Chr2Str(146,'3'),149);
	SchRepl(szPrmpt,Chr2Str(146,'4'),150);
	SchRepl(szPrmpt,Chr2Str(146,'5'),151);
	SchRepl(szPrmpt,"o6",152);
	SchRepl(szPrmpt,Chr2Str(152,'1'),153);
	SchRepl(szPrmpt,Chr2Str(152,'2'),154);
	SchRepl(szPrmpt,Chr2Str(152,'3'),155);
	SchRepl(szPrmpt,Chr2Str(152,'4'),156);
	SchRepl(szPrmpt,Chr2Str(152,'5'),157);
	SchRepl(szPrmpt,"u7",158);
	SchRepl(szPrmpt,Chr2Str(158,'1'),159);
	SchRepl(szPrmpt,Chr2Str(158,'2'),160);
	SchRepl(szPrmpt,Chr2Str(158,'3'),161);
	SchRepl(szPrmpt,Chr2Str(158,'4'),162);
	SchRepl(szPrmpt,Chr2Str(158,'5'),163);
	SchRepl(szPrmpt,"a1",164);
	SchRepl(szPrmpt,"a2",165);
	SchRepl(szPrmpt,"a3",166);
	SchRepl(szPrmpt,"a4",167);
	SchRepl(szPrmpt,"a5",168);
	SchRepl(szPrmpt,"e1",169);
	SchRepl(szPrmpt,"e2",170);
	SchRepl(szPrmpt,"e3",171);
	SchRepl(szPrmpt,"e4",172);
	SchRepl(szPrmpt,"e5",173);
	SchRepl(szPrmpt,"i1",174);
	SchRepl(szPrmpt,"i2",175);
	SchRepl(szPrmpt,"i3",181);
	SchRepl(szPrmpt,"i4",182);
	SchRepl(szPrmpt,"i5",183);
	SchRepl(szPrmpt,"o1",184);
	SchRepl(szPrmpt,"o2",190);
	SchRepl(szPrmpt,"o3",198);
	SchRepl(szPrmpt,"o4",199);
	SchRepl(szPrmpt,"o5",208);
	SchRepl(szPrmpt,"u1",210);
	SchRepl(szPrmpt,"u2",211);
	SchRepl(szPrmpt,"u3",212);
	SchRepl(szPrmpt,"u4",213);
	SchRepl(szPrmpt,"u5",214);
	SchRepl(szPrmpt,"y1",215);
	SchRepl(szPrmpt,"y2",216);
	SchRepl(szPrmpt,"y3",221);
	SchRepl(szPrmpt,"y4",222);
	SchRepl(szPrmpt,"y5",248);
	SchRepl(szPrmpt,"d9",249);
	SchRepl(szPrmpt,"D9",250);
}

/*----------------------------------------------*/
/* Function : InitData                          */
/* Mission  : Initialize parameters for program */
/* Expects  : Nothing                           */
/* Returns  : Nothing                           */
/*----------------------------------------------*/
void InitData()
{
	char szPath[30];
	byte i;
	strcpy(szPath,szDrive); strcat(szPath,SysInf[13]); system(szPath);
	for(i = 0; i < 16; i++) FontVNI(SysInf[i]);
}

/*-------------------------------*/
/* Funtion : HaltSys             */
/* Mission : Restore environment */
/* Expects : Nothing             */
/* Returns : Nothing             */
/*-------------------------------*/
void HaltSys()
{
	SetBorder(0x00); SetCursorSize(0x0607); textattr(0x07); isBlinking(1);
	if(bmAvalid) CloseMouse();
	asm {
		mov ah, 0
		mov al, 3
		int 0x10
	} exit(EXIT_SUCCESS);
}

/*--------------------------------------------*/
/* Funtion : DeleteFiles                      */
/* Mission : Deleting all files from the disk */
/* Expects : (file_spec) sources files        */
/*           (attrib) file attribute          */
/* Returns : Nothing                          */
/*--------------------------------------------*/
bool DeleteFiles(char *szFileSpec, word wAttr)
{
	bool blResume;
	word wEntry;
	struct find_t ftDirEntry;
	char szPath[64], szTarget[64], szEntries[64],
	szCurrFiles[64], szCurrDirs[64];

	blResume = 1;
	for(wEntry = strlen(szFileSpec) - 1; szFileSpec[wEntry] != '\\'; wEntry--);
	strcpy(szPath, szFileSpec); szPath[wEntry] = 0;
	strcpy(szCurrFiles, szFileSpec); szCurrFiles[wEntry + 1] = 0;
	strcpy(szTarget, &szFileSpec[wEntry + 1]);
	if(!_dos_findfirst(szFileSpec, wAttr, &ftDirEntry)) {
      do {
			szCurrFiles[wEntry + 1] = 0; strcat(szCurrFiles, ftDirEntry.name);
			if(ftDirEntry.attrib != _A_NORMAL || ftDirEntry.attrib != _A_ARCH)
			_dos_setfileattr(szCurrFiles, _A_NORMAL); unlink(szCurrFiles);
			nFiles++; WriteChar(17,11,0x1F,nFiles % 48,219); delay(10);
		} while(!_dos_findnext(&ftDirEntry));
	}
	sprintf(szEntries, "%s\\*.*", szPath);
	if(!_dos_findfirst(szEntries, _A_SUBDIR | _A_NORMAL | _A_RDONLY | _A_HIDDEN | _A_SYSTEM, &ftDirEntry)) {
		do {
			if((ftDirEntry.attrib & _A_SUBDIR) && (ftDirEntry.name[0] != '.')) {
				sprintf(szEntries, "%s\\%s\\%s", szPath, ftDirEntry.name, szTarget);
				sprintf(szCurrDirs, "%s\\%s", szPath, ftDirEntry.name);
				blResume = DeleteFiles(szEntries, wAttr); rmdir(szCurrDirs); nDirs++;
			}
		} while(blResume && !_dos_findnext(&ftDirEntry));
	}
	return blResume;
}

/*---------------------------------------*/
/* Funtion : StartDelete                 */
/* Mission : Deleting the topics program */
/* Expects : Nothing                     */
/* Returns : Nothing                     */
/*---------------------------------------*/
void StartDelete()
{
	SetBorder(47); textattr(0x1F); clrscr();
	DlgWin(15,8,65,15,0x5F,SysInf[0]);
	Button(35,13,0xF7,5,SysInf[15],1,0xF8);
	WriteVRM(17,10,0x5E,SysInf[2]);
	WriteChar(17,11,0x17,47,176);
	strcat(szDrive,"TOPICS"); chdir(szDrive);
	strcat(szDrive,"\\*.*");
	DeleteFiles(szDrive,_A_NORMAL | _A_RDONLY | _A_HIDDEN | _A_SYSTEM);
	szDrive[3] = '\0'; chdir(szDrive); strcat(szDrive, "TOPICS");
	rmdir(szDrive); unlink(SysInf[16]);
}

/*----------------------------------------------------*/
/* Funtion : SummaryResult                            */
/* Mission : Information summary about delete program */
/* Expects : Nothing                                  */
/* Returns : Nothing                                  */
/*----------------------------------------------------*/
void SummaryResult()
{
	byte bCol = 0, bRow = 0;

	textattr(0x1F); clrscr();
	DlgWin(18,8,62,16,0x7F,SysInf[1]);
	PrintVRM(28,10,0x70,"%2u t‹p tin ù§ ùž—c g– bÆ.",nFiles);
	PrintVRM(28,11,0x70,"%2u thž mÖc ù§ ùž—c g– bÆ.",nDirs);
	WriteVRM(22,12,0x70,SysInf[11]); Button(35,14,0x4F,7,SysInf[14],1,0x4A);
	do {
		if(ClickMouse(bCol, bRow) == 1) {
			if(bRow == 14 && bCol >= 35 && bCol <= 45) {
				HideMouse(); Clear(35,14,46,15,7);
				WriteVRM(36,14,0x4F,SysInf[14],0x4A); delay(50);
				Button(35,14,0x4F,7,SysInf[14],1,0x4A); ShowMouse(); break;

			}
			if(bRow == 8 && bCol == 18 || bCol == 19) break;
		}
	} while(!kbhit()); HaltSys();
}

/*--------------------------------------------------*/
/* Funtion : DeleteTopics                           */
/* Mission : Showing the menu delete topics message */
/* Expects : Nothing                                */
/* Returns : Nothing                                */
/*--------------------------------------------------*/
void DeleteTopics()
{
	int slc = 0;
	char key, szPath[25];
	byte bCol = 0, bRow = 0;

	Button(24,18,0xF0,4,SysInf[14],1,0xF4);
	Button(46,18,0xE7,4,SysInf[15],1,0xE8);
	if(!InitMouse()) {
		strcpy(szPath,szDrive);
		strcat(szPath,"TOPICS\\DRIVERS\\mouse.com"); system(szPath);
	} SetMousePos();
	do {
		if(kbhit()) {
			key = getch(); if(!key) key = getch();
			switch(toupper(key)) {
			case LEFT:
				Button(24+slc*22,18,0xE7,4,SysInf[14+slc],1,0xE8);
				if(slc <= 0) slc = 1; else slc--;
				Button(24+slc*22,18,0xF0,4,SysInf[14+slc],1,0xF4); break;
			case RIGHT:
				Button(24+slc*22,18,0xE7,4,SysInf[14+slc],1, 0xE8);
				if(slc >= 1) slc = 0; else slc++;
				Button(24+slc*22,18,0xF0,4,SysInf[14+slc],1,0xF4); break;
			}
		}
		if(ClickMouse(bCol, bRow) == 1) {
			if(bRow == 18 && bCol >= 24 && bCol <= 33 ) {
				HideMouse(); Button(46,18,0xE7,4,SysInf[15],1,0xE8);
				slc = 0; Clear(24,18,34,19,4);
				WriteVRM(25,18,0xF0,SysInf[14],0xF4); delay(50);
				Button(24,18,0xF0,4,SysInf[14],1,0xF4); ShowMouse(); key = ENTER;
			}
			if(bRow == 18 && bCol >= 46 && bCol <= 55) {
				Button(24,18,0xE7,4,SysInf[14],1,0xE8); slc = 1;
				Clear(46,18,56,19,4); WriteVRM(47,18,0xF0,SysInf[15],0xF4);
				delay(50); Button(46,18,0xF0,4,SysInf[15],1,0xF4); key = ENTER;
			}
			if(bRow == 4 && bCol == 10 || bCol == 11) key = ENTER;
		}
	} while(key != ENTER);
	if(!slc) {
		StartDelete(); SummaryResult(); HaltSys();
	}
	else HaltSys();
}

/*-----------------------------------------------*/
/* Funtion : MessageDeleteTopics                 */
/* Mission : Showing message deleting the topics */
/* Expects : Nothing                             */
/* Returns : Nothing                             */
/*-----------------------------------------------*/
void MessageDeleteTopics()
{
   textattr(0x1F); SetBorder(47);
	SetCursorSize(0x2020); clrscr(); isBlinking(0);
	DlgWin(10,4,70,20,0x4F,SysInf[12]);
	WriteVRM(12,6,0x4B,SysInf[3]); WriteVRM(12,7,0x4B,SysInf[4]);
	WriteVRM(12,8,0x4B,SysInf[5]); WriteVRM(12,9,0x4B,SysInf[6]);
	WriteVRM(12,10,0x4B,SysInf[7]); WriteVRM(12,11,0x4B,SysInf[8]);
	WriteVRM(40,13,0x4A,SysInf[9]); WriteVRM(40,14,0x4A,SysInf[10]);
	WriteChar(11,16,0x50,59,196); PrintXY(10,16,0x4F,195);
	PrintXY(70,16,0x4F,180); DeleteTopics();
}

/*------------------------------------------*/
/* Funtion : main                           */
/* Mission : Linker funtions to run program */
/* Expects : Nothing                        */
/* Returns : Nothing                        */
/*------------------------------------------*/
int main(int argc, char *argv[])
{
	if(argc < 2) return 1;
	if(!strcmp(argv[1],"C:\\") || !strcmp(argv[1],"D:\\") || !strcmp(argv[1],"E:\\")) {
		strcpy(szDrive, argv[1]); InitData(); MessageDeleteTopics();
	}
	return 0;
}
