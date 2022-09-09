/*--------------------------------------------------------------*/
/*        UNIVERSITY OF TECHNOLOGY HO CHI MINH CITY             */
/*             MAJOR OF INFORMATIC TECHNOLOGY                   */
/*              THE PROGRAM INSTALLION TOPICS                   */
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
#include <dir.h>
#include <dos.h>
#include <io.h>
#include <ctype.h>
#include <fcntl.h>
#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#define VIDEO_ADDR	0xB800
#define MASK_BG		0x08
#define OFFSET(x,y)	( (x -1)*2 + 160*(y-1) )
#ifndef MK_FP
#define MK_FP(seg,ofs) ((void far*)((unsigned long)(seg) << 16 | (ofs)))
#endif

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

struct text_info txtInfo;        // Current text window information
typedef unsigned char byte;      // Data type byte
typedef unsigned int word;       // Data type word
typedef byte bool;               // Data type boolean
char far *fpVRM =                // The far pointer to video memory
(char far *)MK_FP(VIDEO_ADDR,0); // Space memory monitor
char near *ptrSource;             // The pointer sources
char Drive[4];                   // Symbol Drive
byte numFiles = 0;               // Number files to read and files to write
byte bmAvalid = 0;               // Status of the mouse
word buffSize;                   // The szBuffer storing data
struct tagRegs {                 // The struction storing the information
	byte day;                     // The date of the program
	byte month;                   // The month of the program
	byte regs;                    // The register code
	byte num;                     // The number of run program
	char serial[20];              // Product code
	char user[31];                // Register name user
	char disk[4];                 // The disk letter
};

static char **SysInf, **SysMen;
static char *msgWarn[1], *msgExit[1], *msgCmp[1], *msgError[2];
word wnInfo, wnMenu;
char key = 0, szBuff[512];
byte msgSlc = 0, chs = 0, slc = 0, bCol = 0, bRow = 0;

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
/* Funtion : SetCursorSize                 */
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
/*            (attr) The attribute of string     */
/*            (szString) the string to format    */
/* Returns : Nothing                             */
/*-----------------------------------------------*/
void PrintVRM(byte x, byte y, word wAttr, char *szString, ...)
{
	va_list parametres; char sortie[255]; va_start(parametres,szString);
	vsprintf(sortie, szString, parametres); WriteVRM(x,y,wAttr,sortie);
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

/*---------------------------------------------------------*/
/* Function : ReadKey                                      */
/* Mission  : Read a key from the keyboard                 */
/* Expects  : (ch) get the key from the keyboard           */
/* Returns  : If the key is a extend key then return code  */
/*	           key and 0 value else return 1 value and code */
/*---------------------------------------------------------*/
char ReadKey(char &key)
{
	union REGS regs; regs.h.ah = 0; int86(22, &regs, &regs);
	if(!(regs.h.al)) {
		key = regs.h.ah; return 0;
	}
	else key = regs.h.al; return 1;
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

/*------------------------------------*/
/* Funtion : FileDecode               */
/* Mission : Decode files             */
/* Expects : (inFile) The source file */
/*           (outFile) The dest file  */
/* Returns : Nothing                  */
/*------------------------------------*/
void FileDecode(const char *inFile, const char *outFile)
{
	FILE *inHandle, *outHandle;
	int c, key = 98;

	inHandle = fopen(inFile,"rb"); outHandle = fopen(outFile,"wb");
	if(!inHandle || !outHandle) {
		fprintf(stderr,"Error loading file %s. System halt.",inFile); exit(1);
	}
	while((c = fgetc(inHandle)) != EOF) {
		c = c - ~key; fputc(c, outHandle);
	}
	fclose(inHandle); fclose(outHandle);
}

/*------------------------------------------------*/
/* Function : GetData                             */
/* Mission  : Reading information into data array */
/* Expects  : (inFile) the input file             */
/*            (outFile) the output file           */
/*            (szData) the array data             */
/*            (wNumElm) the number of elements    */
/* Returns  : Nothing                             */
/*------------------------------------------------*/
void GetData(const char *inFile, const char *outFile, char **&szData, word &wNumElm)
{
	FILE *fp;
	char szBuffer[100];

	FileDecode(inFile, outFile); fp = fopen(outFile,"rt");
	while(fgets(szBuffer,99,fp)) wNumElm++;
	if(!(szData = new char*[wNumElm])) {
		fprintf(stderr,"Not enough memory!"); exit(1);
	}
	wNumElm = 0; rewind(fp);
	while(fgets(szBuffer,99,fp)) {
		FontVNI(szBuffer);
		if(!(szData[wNumElm] = new char[strlen(szBuffer)+1])) {
			fprintf(stderr,"Not enough memory at pointer %u", wNumElm); exit(1);
		}
		szBuffer[strlen(szBuffer)-1] = '\0'; strcpy(szData[wNumElm],szBuffer); wNumElm++;
	}
	fclose(fp); unlink(outFile);
}

/*------------------------------------------*/
/* Function : ReleaseData                   */
/* Mission  : free block memory of the data */
/* Expects  : Nothing                       */
/* Returns  : Nothing                       */
/*------------------------------------------*/
void ReleaseData()
{
	word i;

	for(i = 0; i < wnInfo; i++) delete[]SysInf[i]; delete[]SysInf;
	for(i = 0; i < wnMenu; i++) delete[]SysMen[i]; delete[]SysMen;
	delete[]msgExit[0]; delete[]msgCmp[0]; delete[]msgWarn[0];
	delete[]msgError[0]; delete[]msgError[1];
}

/*----------------------------------------------*/
/* Function : InitData                          */
/* Mission  : Initialize parameters for program */
/* Expects  : Nothing                           */
/* Returns  : Nothing                           */
/*----------------------------------------------*/
void InitData()
{
	char szSource[100], szDest[100];

	strcpy(szSource,"sysmenus.dll");
	strcpy(szDest,"sysmenus.$$$");
	GetData(szSource,szDest,SysMen,wnMenu);
	szSource[17] = szDest[3] = NULL;
	strcpy(szSource,"sysinfor.sys");
	strcpy(szDest,"sysinfor.$$$");
	GetData(szSource,szDest,SysInf,wnInfo);
	msgWarn[0] = new char[40]; msgExit[0] = new char[40];
	msgCmp[0] = new char[40]; msgError[0] = new char[40];
	msgError[1] = new char[40]; strcpy(msgExit[0],SysInf[26]);
	strcpy(msgCmp[0],SysInf[28]); strcpy(msgWarn[0],SysInf[27]);
	strcpy(msgError[0],SysInf[32]); strcpy(msgError[1],SysInf[33]);
	system("font on");
}

/*-------------------------------*/
/* Funtion : HaltSys             */
/* Mission : Restore environment */
/* Expects : Nothing             */
/* Returns : Nothing             */
/*-------------------------------*/
void HaltSys()
{
	SetBorder(0x00); SetCursorSize(0x0607); textattr(0x07);
	isBlinking(1); if(bmAvalid) CloseMouse(); clrscr();
	ReleaseData(); system("font off");
	exit(EXIT_SUCCESS);
}

/*--------------------------------*/
/* Funtion : SysReboot            */
/* Mission : Restart the computer */
/* Expects : Nothing              */
/* Returns : Nothing              */
/*--------------------------------*/
void SysReboot()
{
	void((far *reset_ptr)()) = (void( far *)() )BOOT_ADR;
	*(int far *)RESET_ADR = 0;
	(*reset_ptr)();
}

/*---------------------------------------------*/
/* Funtion : FadeIN                            */
/* Mission : Debrightness light of the monitor */
/* Expects : Nothing                           */
/* Returns : Nothing                           */
/*---------------------------------------------*/
void FadeIN()
{
	byte bPalettes[200], bDummp[200];
   register i, j;

	outportb(0x3C7, 0);
	for(i = 0; i < 200; i++) {
		bPalettes[i] = inportb(0x3C9);
		bDummp[i] = bPalettes[i];
	}
   for(j = 0; j < 60; j++) {
		for(i = 0; i < 200; i++) if( bDummp[i] > 0 ) bDummp[i]--;
		outportb(0x3C8, 0);
		for(i = 0; i < 200; i++) outportb(0x3C9, bDummp[i]);
      delay(80);
	}
	outportb(0x3C8, 0);
	for(i = 0; i < 200; i++) outportb(0x3C9, bPalettes[i]);
}

/*---------------------------------------*/
/* Funtion : ErrorFile                   */
/* Mission : Display the error code      */
/* Expects : (handle) which file error   */
/*           (errortype) error file type */
/* Returns : Nothing                     */
/*---------------------------------------*/
void ErrorFile(char *szHandle, char *szErrorType)
{
	char error_code[60];
	strcpy(error_code,szErrorType);
	strcat(error_code,szHandle);
	DlgWin(13,8,65,17,0x4F,SysInf[1]);
	Button(33,15,0xF0,4,SysMen[3],1,0xF4);
	WriteVRM(39-strlen(error_code)/2,10,0x4A,error_code);
	WriteVRM(15,11,0x4F,SysInf[53]);
	WriteVRM(15,12,0x4F,SysInf[54]);
	WriteVRM(15,13,0x4F,SysInf[55]);
	do {
		if(ClickMouse(bCol, bRow) == 1) {
			if(bRow == 15 && bCol >= 33 && bCol <= 45) {
				HideMouse(); Clear(33,15,46,16,4);
				WriteVRM(34,15,wATV,SysMen[3],wFLT);
				delay(50); Button(33,15,wATV,4,SysMen[3],1,wFLT);
				ShowMouse(); break;
			}
			if(bRow == 8 && bCol == 13 || bCol == 14) break;
		}
	} while(!kbhit()); HaltSys();
}

/*-------------------------------------------*/
/* Funtion : CopyFiles                       */
/* Mission : Copy all files from the disk    */
/* Expects : (szSource) The disk sources     */
/*           (szDest) The disk target        */
/*           (wAttr) file attribute          */
/* Returns : 1 : If complete or 0 if failure */
/*-------------------------------------------*/
bool CopyFiles(char *szSource, char *szDest, word wAttr)
{
	bool resume;
   word entry, dir_entries;
   struct find_t dir_entry;
	struct find_t near *ptr_dir_entry;
	char near  *ptr_pos;
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
				ptr_dir_entry = (find_t *)ptr_pos;
				ptr_pos += sizeof(dir_entry);
				curr_file[entry + 1] = '\0';
				strcat(curr_file, dir_entry.name);
				if(total_read == 0) {
					if(_dos_open(curr_file, O_RDONLY, &source))
					ErrorFile(curr_file,SysInf[17]);
				}
				if(_dos_read(source, ptr_pos, buffSize - (ptr_pos - ptrSource), &num_read))
				ErrorFile(curr_file, SysInf[20]);
				ptr_pos += num_read; total_read += num_read; files++;
				if(total_read == dir_entry.size) {
					_dos_close(source); total_read = 0;
					if(_dos_findnext(&dir_entry)) break;
				}
				else break;
			}
			ptr_dir_entry = (find_t*)ptrSource;
			ptr_pos = ptrSource + sizeof(dir_entry);
			for(n = 0; n < files; n++) {
				if(total_write == 0) {
					new_file[entry + 1] = '\0';
					strcat(new_file, ptr_dir_entry->name);
					if(_dos_creat(new_file, ptr_dir_entry->attrib, &target))
					ErrorFile(new_file,SysInf[17]);
					WriteChar(30,11,0x99,33,32);
					WriteVRM(30,11,0x9A,new_file); delay(50);
					WriteChar(18,12,0xFF,6*numFiles++ / 14+2,219);
					PrintVRM(50,13,0x9F,"%3d",	(numFiles % 103) > 100 ? 100 : numFiles % 103);
				}

				if(ptr_dir_entry->size > buffSize - (ptr_pos - ptrSource))
					write_to = buffSize - (ptr_pos - ptrSource);
				else write_to = ptr_dir_entry->size;

				if((ptr_dir_entry->size - total_write) < write_to)
				write_to = (word)(ptr_dir_entry->size - total_write);

				if(_dos_write(target, ptr_pos, write_to, &num_write))
				ErrorFile(new_file,SysInf[21]);
				if(num_write != write_to) ErrorFile(new_file,SysInf[21]);
				total_write += num_write;
				ptr_pos += write_to;
				if(total_write == ptr_dir_entry->size) {
					total_write = 0;
					_dos_setftime(target, ptr_dir_entry->wr_date, ptr_dir_entry->wr_time);
					_dos_close(target);
				}
				ptr_dir_entry = (find_t *)ptr_pos; ptr_pos += sizeof(dir_entry);
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
				resume = CopyFiles(str_entry, szDest, wAttr);
			}
		} while(resume && !_dos_findnext(&dir_entry));
	}
	return resume;
}

/*---------------------------------------*/
/* Function : MsgBox                     */
/* Mission  : Display the system message */
/* Expects  : (x1,y1,x2,y2) coordinates  */
/*            (szMsg) The messages array */
/*            (n) The elements of array  */
/* Return   : 1 or 0                     */
/*---------------------------------------*/
byte MsgBox(byte x1, byte y1, byte x2, byte y2, const char *szMsg[], byte n)
{
	char key = 0;
	byte slc = 0, bCenter = x1 + (x2 - x1) / 2, i;

	SetCursorSize(0x2020);
	DlgWin(x1,y1,x2,y2,0x4F,SysInf[1]);
	ShowMouse(); MoveMouse(bCenter,y2-3);
	for(i = 0; i < n; i++)
	WriteVRM(bCenter-strlen(szMsg[i])/2+1,y1+2+i,0x4A,szMsg[i]);
	Button(bCenter-14,y2-2,wATV,4,SysMen[0],1,wFLT);
	Button(bCenter+6,y2-2,_eATV,4,SysMen[2],1,_eFLT);
	do {
		if(ClickMouse(bCol, bRow) == 1) {
			if(bRow == y2-2 && bCol >= bCenter-14 && bCol <= bCenter-5) {
				HideMouse(); Button(bCenter+6,y2-2,_eATV,4,SysMen[2],1,_eFLT);
				Clear(bCenter-14,y2-2,bCenter-3,y2-1,4);
				WriteVRM(bCenter-13,y2-2,wATV,SysMen[0],wFLT); delay(50);
				Button(bCenter-14,y2-2,wATV,4,SysMen[0],1,wFLT); ShowMouse();
				slc = 0; key = ENTER;
			}
			if(bRow == y2-2 && bCol >= bCenter+6 && bCol <= bCenter+15) {
				HideMouse(); Button(bCenter-14,y2-2,_eATV,4,SysMen[0],1,_eFLT);
				Clear(bCenter+6, y2-2,bCenter+16,y2-1,4);
				WriteVRM(bCenter+7,y2-2,wATV,SysMen[2],wFLT); delay(50);
				Button(bCenter+6,y2-2,wATV,4,SysMen[2],1,wFLT); ShowMouse();
				slc = 1; key = ENTER;
			}
			if(bRow == y1 && bCol == x1 || bCol == x1+1) {slc = 1; key = ENTER;}
		}
		if(kbhit()) {
			key = getch(); if(!key) key = getch();
			switch(toupper(key)) {
			case LEFT :
				Button(bCenter-14+slc*20,y2-2,_eATV,4,SysMen[slc*2],1,_eFLT);
				if(slc <= 0) slc = 1; else slc--;
				Button(bCenter-14+slc*20,y2-2,wATV,4,SysMen[slc*2],1,wFLT);
				break;
			case RIGHT :
				Button(bCenter-14+slc*20,y2-2,_eATV,4,SysMen[slc*2],1,_eFLT);
				if(slc >= 1) slc = 0; else slc++;
				Button(bCenter-14+slc*20,y2-2,wATV,4,SysMen[slc*2],1,wFLT);
				break;
			}
		}
	} while(key != ENTER);
	return slc;
}

/*---------------------------------------*/
/* Function : WarnBox                    */
/* Mission  : Display the system message */
/* Expects  : (x1,y1,x2,y2) coordinates  */
/*            (szMsg) The messages array */
/*            (n) The elements of array  */
/* Return   : 1 or 0                     */
/*---------------------------------------*/
void WarnBox(byte x1, byte y1, byte x2, byte y2, const char *szMsg[], byte n)
{
	byte bCenter = x1 + (x2 - x1) / 2, i = 0,
	bPos = bCenter - strlen(SysMen[0])/2;
	char key;

	SetCursorSize(0x2020); DlgWin(x1,y1,x2,y2,0x4F,SysInf[1]);
	for(i = 0; i < n; i++)
	WriteVRM(bCenter-strlen(szMsg[i])/2,y1+2+i,0x4A,szMsg[i]);
	Button(bPos,y2-2,wATV,4,SysMen[0],1,wFLT);
	ShowMouse(); MoveMouse(bCenter,y2-3);
	do {
		if(ClickMouse(bCol, bRow) == 1) {
			if(bRow == y2-2 && bCol >= bPos  && bCol <= bPos + 9) {
				HideMouse(); Clear(bPos,y2-2,bPos+7,y2-1,4);
				WriteVRM(bPos+1,y2-2,wATV,SysMen[0],0xF4); delay(50);
				Button(bPos,y2-2,wATV,4,SysMen[0],1,0xF4); ShowMouse();
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
/* Funtion : UserCheck                  */
/* Mission : Testing user serial number */
/* Expects : Nothing                    */
/* Returns : Nothing                    */
/*--------------------------------------*/
void UserCheck()
{
	char szOldName[31], szOldID[20],
	szCurrName[31], szCurrID[20];
	FILE *fptr;
	tagRegs trInfo;
	byte i = 0, j = 0, flgCorrectName = 1, flgCorrectID = 0, isASCII = 0;

	textattr(0x1F); SetCursorSize(0x2020); clrscr();
	FillFrame(1,1,80,25,0xF6,178);
	DlgWin(5,3,75,23,0x5F,SysInf[2]);
	WriteVRM(8,5,0x5A,SysInf[56]); WriteVRM(8,6,0x5A,SysInf[57]);
	WriteVRM(8,7,0x5A,SysInf[58]); WriteVRM(8,8,0x5A,SysInf[59]);
	WriteVRM(8,9,0x5A,SysInf[60]); WriteVRM(14,12,0x5E,SysMen[9],0x5F);
	WriteVRM(11,11,0x5B,"çè"); WriteVRM(8,14,0x5F,SysInf[22]);
	WriteChar(8,15,0x1F,30,32); WriteVRM(8,17,0x5F,SysInf[23]);
	WriteChar(8,18,0x1F,30,32);
	Button(24,21,_wATV,5,SysMen[1],1,_wFLT); WriteVRM(11,12,0x5E,"éê");
	Button(47,21,wATV,5,SysMen[4],1,wFLT); ShowMouse(); MoveMouse(10,11);
	slc = 0; label1: WriteVRM(14,11+slc,0x5B,SysMen[slc+8],0x5A); key = 0;
	do {
		if(kbhit()) {
			key = getch(); if(!key) key = getch();
			switch(toupper(key)) {
			case DOWN :
				WriteVRM(14,11+slc,0x5E,SysMen[slc+8],0x5F);
				WriteVRM(11,11+slc,0x5E,"éê");
				if(slc >= 1) slc = 0; else slc++;
				WriteVRM(14,11+slc,0x5B,SysMen[slc+8],0x5A);
				WriteVRM(11,11+slc,0x5B,"çè"); break;
			case UP :
				WriteVRM(14,11+slc,0x5E,SysMen[slc+8],0x5F);
				WriteVRM(11,11+slc,0x5E,"éê");
				if(slc <= 0) slc = 1; else slc--;
				WriteVRM(14,11+slc,0x5B,SysMen[slc+8],0x5A);
				WriteVRM(11,11+slc,0x5B,"çè"); break;
			}
		}
		if(ClickMouse(bCol, bRow) == 1) {
			if(bRow == 11 && bCol >= 11 && bCol <= 40) {
				HideMouse(); WriteVRM(14,12,0x5E,SysMen[9],0x5F);
				WriteVRM(11,12,0x5E,"éê"); delay(20);
				WriteVRM(14,11,0x5B,SysMen[8],0x5A);
				WriteVRM(11,11,0x5B,"çè"); ShowMouse(); slc = 0; key = TAB;
			}
			if(bRow == 12 && bCol >= 11 && bCol <= 55) {
				HideMouse(); WriteVRM(14,11,0x5E,SysMen[8],0x5F);
				WriteVRM(11,11,0x5E,"éê"); delay(20);
				WriteVRM(14,12,0x5B,SysMen[9],0x5A);
				WriteVRM(11,12,0x5B,"çè"); ShowMouse(); slc = 1; key = TAB;
			}
			if(bRow == 15 && bCol >= 8 && bCol <= 38) key = TAB;
			if(bRow == 18 && bCol >= 8 && bCol <= 38) key = TAB;
			if(bRow == 3 && bCol == 5 || bCol == 6) HaltSys();
		}
	} while(key != TAB);
	if(slc) {
		HideMouse(); gettext(20,10,62,17,szBuff);
		msgSlc = MsgBox(20,10,60,16,msgExit,1);
		if(!msgSlc) HaltSys(); HideMouse();
		puttext(20,10,62,17,szBuff); ShowMouse(); goto label1;
   }
	if((fptr = fopen(/*SysInf[42]*/"register.dat","rb")) == NULL) {
		 textattr(0x1F); SetBorder(47); SetCursorSize(0x2020);
		 clrscr(); WriteVRM(31,10,0x4F,SysInf[11]);
		 WriteVRM(20,12,0x1F,SysInf[19]); WriteVRM(20,13,0x1F,SysInf[25]);
		 getch(); HaltSys();
	}
	fread(&trInfo,sizeof(tagRegs),1,fptr); fclose(fptr);
	strcpy(szOldName, trInfo.user); strcpy(szOldID, trInfo.serial);
	szCurrName[0] = '\0'; szCurrID[0] = '\0'; label2:
	SetCursorSize(0x0B0A); gotoxy(8,15); key = i = j = slc = 0;
	Button(24,21,wATV,5,SysMen[1],1,wFLT);
	Button(47,21,_wATV,5,SysMen[4],1,_wFLT);
	do {
		if(flgCorrectName) gotoxy(8+i,15); if(flgCorrectID) gotoxy(8+j,18);
      if(kbhit()) {
			isASCII = ReadKey(key); if(!key) isASCII = ReadKey(key);
			if(flgCorrectName) {
				if((isASCII && i < 30 && key != 8 && isalpha(key)) ||
				(key == 32 && i < 30)) {
					szCurrName[i] = key; PrintXY(8+i,15,0x1F,key); i++;
				}
				if(key == 8 && i > 0) {
					i--; PrintXY(8+i,15,0x1F,32);
				}
			}
			if(key == TAB && flgCorrectName) {
				flgCorrectName = 0; flgCorrectID = 1;
			}
			if(flgCorrectID) {
				if(isASCII && j < 19 && key != 8 && key != TAB && key != ENTER) {
					szCurrID[j] = key; PrintXY(8+j,18,0x1F,key); j++;
				}
				if(key == 8 && j > 0) {
					j--; PrintXY(8+j,18,0x1F,32);
				}
			}
			switch(toupper(key)) {
			case LEFT :
				if(!isASCII) {
					Button(24+slc*23,21,_wATV,5,SysMen[3*slc+1],1,_wFLT);
					if(slc <= 0) slc = 0; else slc--;
					Button(24+slc*23,21,wATV,5,SysMen[3*slc+1],1,wFLT);
				} break;
			case RIGHT :
				if(!isASCII) {
					Button(24+slc*23,21,_wATV,5,SysMen[3*slc+1],1,_wFLT);
					if(slc >= 1) slc = 1; else slc++;
					Button(24+slc*23,21,wATV,5,SysMen[3*slc+1],1,wFLT);
				}
			}
		}
		if(ClickMouse(bCol, bRow) == 1) {
			if(bRow == 21 && bCol >= 24 && bCol <= 36) {
				HideMouse(); Button(47,21,_wATV,5,SysMen[4],1,_wFLT);
				Clear(24,21,36,22,5); WriteVRM(25,21,wATV,SysMen[1],wFLT);
				delay(50); Button(24,21,wATV,5,SysMen[1],1,wFLT);
				slc = 0; ShowMouse(); key = ENTER;
			}
			if(bRow == 21 && bCol >= 47 && bCol <= 59) {
				HideMouse(); Button(24,21,_wATV,5,SysMen[1],1,_wFLT);
				Clear(47,21,59,22,5); WriteVRM(48,21,wATV,SysMen[2],wFLT);
				delay(50); Button(47,21,wATV,5,SysMen[4],1,wFLT);
				slc = 1; ShowMouse(); key = ENTER;
			}
		}
	} while(key != ENTER);
	if(i) szCurrName[i] = '\0'; if(j) szCurrID[j] = '\0';
	if(!slc) {
		if(strcmp(szOldName, szCurrName) &&
			strcmp(szOldID, szCurrID)) {
			HideMouse(); gettext(20,10,62,17,szBuff);
			flgCorrectName = 1; flgCorrectID = 0;
			WarnBox(20,10,60,16,msgWarn,1); HideMouse();
			puttext(20,10,62,17,szBuff); WriteChar(8,15,0x1F,30,32);
			WriteChar(8,18,0x1F,30,32); ShowMouse(); goto label2;
		}
		else
		if(!strcmp(szOldName, szCurrName) &&
			strcmp(szOldID, szCurrID)) {
			HideMouse(); gettext(20,10,62,17,szBuff);
			flgCorrectName = 0; flgCorrectID = 1;
			WarnBox(20,10,60,16,msgWarn,1); HideMouse();
			puttext(20,10,62,17,szBuff); WriteChar(8,18,0x1F,30,32);
			ShowMouse(); goto label2;
		}
		else
		if(strcmp(szOldName, szCurrName) &&
			 !strcmp(szOldID, szCurrID)) {
			 HideMouse(); gettext(20,10,62,17,szBuff);
			 flgCorrectName = 1; flgCorrectID = 0;
			 WarnBox(20,10,60,16,msgWarn,1); HideMouse();
			 puttext(20,10,62,17,szBuff);
			 WriteChar(8,15,0x1F,30,32); ShowMouse();
			 goto label2;
		 }
	}
	else {
		HideMouse(); gettext(20,10,62,17,szBuff);
		msgSlc = MsgBox(20,10,60,16,msgExit,1);
		if(!msgSlc) HaltSys(); HideMouse(); puttext(20,10,62,17,szBuff);
		ShowMouse(); goto label2;
	}
}

/*------------------------------------*/
/* Funtion : Unregister               */
/* Mission : Show the readme.hlp file */
/* Expects : Nothing                  */
/* Returns : Nothing                  */
/*------------------------------------*/
void Unregister()
{
	textattr(0x1F); SetBorder(63); SetCursorSize(0x2020);
	clrscr(); FillFrame(1,1,80,25,0xF6,178);
	DlgWin(10,3,74,22,0xBE,SysInf[3]);
	WriteVRM(12,5,0xB0,SysInf[61]); WriteVRM(12,6,0xB0,SysInf[62]);
	WriteVRM(12,7,0xB0,SysInf[63]); WriteVRM(12,8,0xB0,SysInf[64]);
	WriteVRM(12,9,0xB0,SysInf[65]); WriteVRM(12,10,0xB0,SysInf[66]);
	WriteVRM(12,11,0xB0,SysInf[67]); WriteVRM(12,12,0xB0,SysInf[68]);
	WriteVRM(17,14,0xBC,SysInf[69]); WriteVRM(17,15,0xBC,SysInf[70]);
	WriteVRM(17,16,0xBC,SysInf[71]); WriteVRM(12,18,0xB1,SysInf[72]);
	Button(47,20,_wATV,11,SysMen[4],1,_wFLT);
	Button(26,20,wATV,11,SysMen[1],1,wFLT);
	ShowMouse(); MoveMouse(42,20); slc = chs = key = 0;
	do {
		if(kbhit()) {
			key = getch(); if(!key) key = getch();
			switch(toupper(key)) {
			case LEFT:
				Button(26+slc*21,20,_wATV,11,SysMen[3*slc+1],1,_wFLT);
				if(slc <= 0) slc = 0; else slc--;
				Button(26+slc*21,20,wATV,11,SysMen[3*slc+1],1,wFLT); break;
			case RIGHT:
				Button(26+slc*21,20,_wATV,11,SysMen[3*slc+1],1,_wFLT);
				if(slc >= 1) slc = 1; else slc++;
				Button(26+slc*21,20,wATV,11,SysMen[3*slc+1],1,wFLT); break;
			}
		}
		if(ClickMouse(bCol, bRow) == 1) {
			if(bRow == 20 && bCol >= 26 && bCol <= 38) {
				HideMouse(); Button(47,20,_wATV,11,SysMen[4],1,_wFLT);
				Clear(26,20,38,21,11); WriteVRM(27,20,wATV,SysMen[1],wFLT);
				delay(50); Button(26,20,wATV,11,SysMen[1],1,wFLT);
				ShowMouse(); slc = 0; break;
			}
			if(bRow == 20 && bCol >= 46 && bCol <= 59) {
				HideMouse(); Button(26,20,_wATV,11,SysMen[1],1,_wFLT);
				Clear(47,20,59,21,11); WriteVRM(48,20,wATV,SysMen[4],wFLT);
				delay(50); Button(47,20,wATV,11,SysMen[4],1,wFLT);
				ShowMouse(); slc = 1; break;
			}
			if(bRow == 3 && bCol == 10 || bCol == 11) HaltSys();
		}
	} while(key != ENTER); HideMouse(); if(slc) HaltSys();
}

/*-----------------------------------------*/
/* Funtion : InstallProgram                */
/* Mission : Setup the program on computer */
/* Expects : (Drive) the Drive to set up   */
/* Returns : Nothing                       */
/*-----------------------------------------*/
void InstallProgram()
{
	byte i;
	buffSize = 3500;

	if(!(ptrSource = new char[buffSize])) {
		textattr(0x1F); SetCursorSize(0x2020); clrscr();
		FillFrame(1,1,80,25,0xF6,178); HideMouse();
		DlgWin(15,8,65,17,0x4F,SysInf[1]);
		WriteVRM(27,10,0x4E,SysInf[31]); WriteVRM(17,11,0x4F,SysInf[53]);
		WriteVRM(17,12,0x4F,SysInf[54]); WriteVRM(17,13,0x4F,SysInf[55]);
		Button(35,15,wATV,4,SysMen[0],1,wFLT); ShowMouse();
		do {
			if(ClickMouse(bCol, bRow) == 1) {
				if(bRow == 15 && bCol >= 35 && bCol <= 45) {
					HideMouse(); Clear(35,15,46,16,4);
					WriteVRM(36,15,wATV,SysMen[0],wFLT); delay(50);
					Button(35,15,wATV,4,SysMen[0],1,wFLT); ShowMouse(); break;
				}
				if(bRow == 8 && bCol == 15 || bCol == 16) break;
			}
		} while(!kbhit()); HaltSys();
	}
	SetBorder(50); textattr(0x1F); SetCursorSize(0x2020);
	clrscr(); FillFrame(1,1,80,25,0xF6,178);
	DlgWin(15,6,65,17,0x9F,SysInf[4]);
	WriteVRM(18,11,0x9F,SysInf[73]); WriteVRM(18,8,0x9F,SysInf[38]);
	WriteVRM(18,9,0x9F,SysInf[40]); WriteChar(18,12,0x17,45,176);
	WriteVRM(53,13,0x9F,"% ho¥n t‡t"); Button(35,15,_wATV,9,SysMen[2],1,_wFLT);
	CopyFiles("A:\\*.*", Drive, _A_NORMAL | _A_RDONLY | _A_HIDDEN | _A_SYSTEM);
	delay(500);
	FillFrame(15,6,69,21,0xF6,178);
	DlgWin(18,10,62,15,0x1F,SysInf[5]);
	WriteVRM(22,12,0x1E,SysInf[41]);
	WriteChar(22,13,0x17,37,176);
	WriteVRM(49,14,0x1A,"% ho¥n t‡t");
	for(i = 1; i < 38; i++) {
		WriteChar(22,13,0x1F,i,219);
		PrintVRM(46,14,0x1A,"%3d", 2*i + 26); delay(100);
	}
	FillFrame(15,6,69,21,0xF6,178); WarnBox(25,10,55,15,msgCmp,1);
	delete[]ptrSource;
}

/*----------------------------------------------*/
/* Funtion : RestartProgram                     */
/* Mission : Showing the restart system message */
/* Expects : Nothing                            */
/* Returns : Nothing                            */
/*----------------------------------------------*/
void RestartProgram()
{
	ShowMouse(); textattr(0x1F); SetCursorSize(0x2020);
	SetBorder(50); clrscr(); FillFrame(1,1,80,25,0xF6,178);
	DlgWin(15,8,65,15,0x4F,SysInf[6]);
	WriteVRM(21,11,0x4A,SysMen[11],0x4B); WriteVRM(18,10,0x4F,"çè");
	WriteVRM(21,10,0x4F,SysMen[10],0x4E); WriteVRM(18,11,0x4A,"éê");
	Button(26,13,0xB4,4,SysMen[0],1,0xB1);
	Button(45,13,0x9F,4,SysMen[3],1,0x94); MoveMouse(40,13);
	slc = chs = key = 0;
	do {
		if(kbhit()) {
			key = getch(); if(!key) key = getch();
			switch(toupper(key)) {
			case DOWN :
				WriteVRM(21,10+slc,0x4A,SysMen[slc+10],0x4B);
				WriteVRM(18,10+slc,0x4A,"éê");
				if(slc >= 1) slc = 0; else slc++;
				WriteVRM(21,10+slc,0x4F,SysMen[slc+10],0x4E);
				WriteVRM(18,10+slc,0x4F,"çè"); break;
			case UP :
				WriteVRM(21,10+slc,0x4A,SysMen[slc+10],0x4B);
				WriteVRM(18,10+slc,0x4A,"éê");
				if(slc <= 0) slc = 1; else slc--;
				WriteVRM(21,10+slc,0x4F,SysMen[slc+10],0x4E);
				WriteVRM(18,10+slc,0x4F,"çè"); break;
			case LEFT :
				Button(26+chs*19,13,0x9F,4,SysMen[3*chs],1,0x94);
				if(chs <= 0) chs = 0; else chs--;
				Button(26+chs*19,13,0xB4,4,SysMen[3*chs],1,0xB1); break;
			case RIGHT :
				Button(26+chs*19,13,0x9F,4,SysMen[3*chs],1,0x94);
				if(chs >= 1) chs = 1; else chs++;
				Button(26+chs*19,13,0xB4,4,SysMen[3*chs],1,0xB1); break;
			}
		}
		if(ClickMouse(bCol, bRow) == 1) {
			if(bRow == 10 && bCol >= 18 && bCol <= 62) {
				HideMouse(); WriteVRM(21,11,0x4A,SysMen[11],0x4B);
				WriteVRM(18,11,0x4A,"éê"); slc = 0;
				WriteVRM(21,10,0x4F,SysMen[10],0x4E);
				WriteVRM(18,10,0x4F,"çè"); ShowMouse(); delay(50);
			}
			if(bRow == 11 && bCol >= 18 && bCol <= 61) {
				HideMouse(); WriteVRM(21,10,0x4A,SysMen[10],0x4B);
				WriteVRM(18,10,0x4A,"éê"); slc = 1;
				WriteVRM(21,11,0x4F,SysMen[11],0x4E);
				WriteVRM(18,11,0x4F,"çè"); ShowMouse(); delay(50);
			}
			if(bRow == 13 && bCol >= 26 && bCol <= 35) {
				HideMouse(); Button(45,13,0x9F,4,SysMen[3],1,0x94);
				Clear(26,13,36,14,4); WriteVRM(27,13,0xB4,SysMen[0],0xB1);
				delay(50); Button(26,13,0xB4,4,SysMen[0],1,0xB1);
				ShowMouse(); chs = 0; break;
			}
			if(bRow == 13 && bCol >= 45 && bCol <= 56) {
				HideMouse(); Button(26,13,0x9F,4,SysMen[0],1,0x94);
				Clear(45,13,57,14,4); WriteVRM(46,13,0xB4,SysMen[3],0xB1);
				delay(50); Button(45,13,0xB4,4,SysMen[3],1,0xB1);
				ShowMouse(); chs = 1; break;
			}
			if(bRow == 8 && bCol == 15 || bCol == 16) HaltSys();
		}
	} while(key != ENTER);
	if(!chs && !slc) SysReboot();
	else HaltSys();
}

/*-------------------------------------*/
/* Funtion : AssignDrive               */
/* Mission : Showing the Drive message */
/* Expects : (slc) number of Drive     */
/* Returns : Nothing                   */
/*-------------------------------------*/
void AssignDrive(byte bSelect)
{
	switch(bSelect) {
		case 0 : strcpy(Drive,"B:\\"); break; case 1 : strcpy(Drive,"C:\\"); break;
		case 2 : strcpy(Drive,"D:\\"); break; case 3 : strcpy(Drive,"E:\\");
	}
}

/*----------------------------------------*/
/* Funtion : ChooseDrive                  */
/* Mission : Showing choose Drive message */
/* Expects : Nothing                      */
/* Returns : Nothing                      */
/*----------------------------------------*/
void ChooseDrive()
{
	byte i;
	SetCursorSize(0x2020); textattr(0x1F); clrscr();
	FillFrame(1,1,80,25,0xF6,178);
	DlgWin(20,8,60,17,0x3F,SysInf[7]);
	WriteVRM(31,10,0x34,SysInf[24]);
	WriteChar(29,11,0x35,22,193);
	for(i = 0; i < 4; i++) WriteVRM(29,12+i,0x30,SysMen[12+i],0x3A);
	Button(46,12,wATV,3,SysMen[1],1,wFLT);
	Button(46,15,_wATV,3,SysMen[4],1,_wFLT);
	MoveMouse(38,11); slc = chs = 0;
	label:
	key = 0; WriteVRM(29,12+slc,0x3F,SysMen[12+slc],0x3E);
	do {
		if(ClickMouse(bCol, bRow) == 1) {
			if(bRow == 12 && bCol >= 29 && bCol <= 41) {
				HideMouse(); WriteVRM(29,12,0x3F,SysMen[12],0x3E);
				if(slc != 0) WriteVRM(29,12+slc,0x30,SysMen[12+slc],0x3A);
				ShowMouse(); slc = 0; delay(20);
			}
			if(bRow == 13 && bCol >= 29 && bCol <= 41) {
				HideMouse(); WriteVRM(29,13,0x3F,SysMen[13],0x3E);
				if(slc != 1) WriteVRM(29,12+slc,0x30,SysMen[12+slc],0x3A);
				ShowMouse(); slc = 1; delay(20);
			}
			if(bRow == 14 && bCol >= 29 && bCol <= 41) {
				HideMouse(); WriteVRM(29,14,0x3F,SysMen[14],0x3E);
				if(slc != 2) WriteVRM(29,12+slc,0x30,SysMen[12+slc],0x3A);
				ShowMouse(); slc = 2; delay(20);
			}
			if(bRow == 15 && bCol >= 29 && bCol <= 41) {
				HideMouse(); WriteVRM(29,15,0x3F,SysMen[15],0x3E);
				if(slc != 3) WriteVRM(29,12+slc,0x30,SysMen[12+slc],0x3A);
				ShowMouse(); slc = 3; delay(20);
			}
			if(bRow == 12 && bCol >= 46 & bCol <= 57) {
				HideMouse(); Button(46,15,_wATV,3,SysMen[4],1,_wFLT);
				Clear(46,12,57,13,3); WriteVRM(47,12,wFLT,SysMen[1],wFLT);
				delay(50); Button(46,12,wATV,3,SysMen[1],1,wFLT);
				ShowMouse(); chs = 0; key = ENTER;
			}
			if(bRow == 15 && bCol >= 46 & bCol <= 57) {
				HideMouse(); Button(46,12,_wATV,3,SysMen[1],1,_wFLT);
				Clear(46,15,57,16,3); WriteVRM(47,15,wATV,SysMen[2],wFLT);
				delay(50); Button(46,15,wATV,3,SysMen[4],1,wFLT);
				ShowMouse(); chs = 1; key = ENTER;
			}
			if(bRow == 8 && bCol == 20 || bCol == 21) HaltSys();
		}
		if(kbhit()) {
			key = getch(); if(!key) key = getch();
			switch(toupper(key)) {
			case UP:
				WriteVRM(29,12+slc,0x30,SysMen[12+slc],0x3A);
				if(slc <= 0) slc = 3; else slc--;
				WriteVRM(29,12+slc,0x3F,SysMen[12+slc],0x3E); break;
			case DOWN:
				WriteVRM(29,12+slc,0x30,SysMen[12+slc],0x3A);
				if(slc >= 3) slc = 0; else slc++;
				WriteVRM(29,12+slc,0x3F,SysMen[12+slc],0x3E); break;
			case HOME:
				WriteVRM(29,12+slc,0x30,SysMen[12+slc],0x3A); slc = 0;
				WriteVRM(29,12+slc,0x3F,SysMen[12+slc],0x3E); break;
			case END:
				WriteVRM(29,12+slc,0x30,SysMen[12+slc],0x3A); slc = 3;
				WriteVRM(29,12+slc,0x3F,SysMen[12+slc],0x3E); break;
			case TAB:
			do {
				if(kbhit()) {
					key = getch(); if(!key) key = getch();
					switch(toupper(key)) {
					case UP:
						Button(46,12+3*chs,_wATV,3,SysMen[3*chs+1],1,_wFLT);
						if(chs <= 0) chs = 1; else chs--;
						Button(46,12+3*chs,wATV,3,SysMen[3*chs+1],1,wFLT); break;
					case DOWN:
						Button(46,12+3*chs,_wATV,3,SysMen[3*chs+1],1,_wFLT);
						if(chs >= 1) chs = 0; else chs++;
						Button(46,12+3*chs,wATV,3,SysMen[3*chs+1],1,wFLT); break;
					}
				}
				if(ClickMouse(bCol, bRow) == 1) {
					if(bRow == 12 && bCol >= 46 & bCol <= 57) {
						HideMouse(); Button(46,15,_wATV,3,SysMen[4],1,_wFLT);
						Clear(46,12,57,13,3);
						WriteVRM(47,12,wATV,SysMen[1],wFLT); delay(50);
						Button(46,12,wATV,3,SysMen[1],1,wFLT); ShowMouse();
						chs = 0; key = ENTER;
					}
					if(bRow == 15 && bCol >= 46 & bCol <= 57) {
						HideMouse(); Button(46,12,_wATV,3,SysMen[1],1,_wFLT);
						Clear(46,15,57,16,3);
						WriteVRM(47,15,wATV,SysMen[4],wFLT); delay(50);
						Button(46,15,wATV,3,SysMen[4],1,wFLT); ShowMouse();
						chs = 1; key = ENTER;
					}
					if(bRow == 8 && bCol == 20 || bCol == 21) HaltSys();
				}
			} while(key != ENTER); break;
			}
		}
	} while(key != ENTER);
	if(!chs) AssignDrive(slc);
	else {
		HideMouse(); gettext(20,10,62,17,szBuff);
		msgSlc = MsgBox(20,10,60,16,msgExit,1);
		if(!msgSlc) HaltSys(); HideMouse();
		puttext(20,10,62,17,szBuff); ShowMouse(); goto label;
	}
}

/*----------------------------------------------------*/
/* Funtion : UpdateProgram                            */
/* Mission : Deleting files crack.com and install.com */
/* Expects : Nothing                                  */
/* Returns : Nothing                                  */
/*----------------------------------------------------*/
void UpdateProgram()
{
	FILE *fp;
	tagRegs trInfo;
	char szPath[15];

	if(!(fp = fopen(SysInf[42],"r+b"))) {
		textattr(0x1F); SetBorder(47); SetCursorSize(0x2020);
		clrscr(); WriteVRM(33,10,0x4F,SysInf[11]);
		WriteVRM(20,12,0x1F,SysInf[19]); WriteVRM(20,13,0x1F,SysInf[25]);
		getch(); HaltSys();
	}
	fread(&trInfo,sizeof(tagRegs),1,fp); strcpy(trInfo.disk,Drive);
	fseek(fp,0L,SEEK_SET); fwrite(&trInfo,sizeof(tagRegs),1,fp); fclose(fp);
	strcpy(szPath,Drive); strcat(szPath,"crack.com"); unlink(szPath);
	szPath[3] = '\0'; strcat(szPath,"install.exe"); unlink(szPath);
	szPath[3] = '\0'; strcat(szPath,"guide.txt"); unlink(szPath);
}

/*-------------------------------------------*/
/* Function : GetDrive                       */
/* Mission  : Return the number of the drier */
/* Expects  : (szDrive) Serial number Drive  */
/* Returns  : number of Drive                */
/*-------------------------------------------*/
byte GetDrive(char *szDrive)
{
	if(!strcmp(szDrive,"B:\\")) return 2; if(!strcmp(szDrive,"C:\\")) return 3;
	if(!strcmp(szDrive,"D:\\")) return 4; if(!strcmp(szDrive,"E:\\")) return 5;
	return 0;
}

/*--------------------------------------------------*/
/* Funtion : DiskCheck                              */
/* Mission : Checking disk space and byte available */
/* Expects : Nothing                                */
/* Returns : Nothing                                */
/*--------------------------------------------------*/
void DiskCheck()
{
	union REGS inRegs, outRegs;
	unsigned long a, b, c, d;
	byte i, bSkip = 0; SetBorder(50); textattr(0x1F);
	SetCursorSize(0x2020); clrscr(); FillFrame(1,1,80,25,0xF6,178);
	DlgWin(15,5,65,20,0x7F,SysInf[8]);
	WriteVRM(18,7,0x70,SysInf[39]); WriteVRM(18,8,0x70,SysInf[40]);
	WriteVRM(18,10,0x70,SysInf[29]); WriteVRM(53,12,0x70,"% ho¥n t‡t");
	WriteChar(18,11,0x17,45,176);
	Button(36,18,wATV,7,SysMen[2],1,wFLT); MoveMouse(39,17);
	for(i = 0; i < 45 && !bSkip; i++) {
		WriteChar(18+i,11,0x3F,1,219);
		PrintVRM(50,12,0x70,"%3u", 2*i+12);
		if(ClickMouse(bCol, bRow) == 1) {
			if(bRow == 18 && bCol >= 36 && bCol <= 45) {
				HideMouse(); Clear(36,18,45,19,7);
				WriteVRM(37,18,wATV,SysMen[2],wATV);
				delay(50); Button(36,18,wATV,7,SysMen[2],1,wFLT); ShowMouse();
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
	WriteVRM(18,14,0x70,SysInf[30]);
	WriteChar(18,15,0x17,45,176);
	WriteVRM(53,16,0x70,"% ho¥n t‡t");
	for(i = 0; i < 45 && !bSkip; i++) {
		WriteChar(18+i,15,0x3F,1,219);
		PrintVRM(50,16,0x70,"%3u", 2*i+12);
		if(ClickMouse(bCol, bRow) == 1) {
			if(bRow == 18 && bCol >= 36 && bCol <= 45) {
				HideMouse(); Clear(36,18,45,19,7);
				WriteVRM(37,18,wATV,SysMen[2],wATV);
				delay(50); Button(36,18,wATV,7,SysMen[2],1,wFLT); ShowMouse(); break;
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
	inRegs.h.ah = 0x36; inRegs.h.dl = GetDrive(Drive);
	int86(0x21,&inRegs,&outRegs);
	if(outRegs.x.ax == 0xFFFF) {
		DlgWin(20,9,55,15,0x4F,SysInf[1]);
		WriteVRM(22,11,0x4F,SysInf[37]);
		Button(33,13,wATV,4,SysMen[0],1,wFLT);
		do {
			if(ClickMouse(bCol, bRow) == 1) {
				if(bRow == 13 && bCol >= 33 && bCol <= 43) {
					HideMouse(); Clear(33,13,43,14,4);
					WriteVRM(34,13,wATV,SysMen[0],wFLT); delay(50);
					Button(33,13,wATV,4,SysMen[0],1,wFLT); ShowMouse(); break;
				}
				if(bRow == 9 && bCol == 20 || bCol == 21) break;
			}
		} while(!kbhit()); HaltSys();
	}
	a = outRegs.x.dx; b = outRegs.x.bx; c = outRegs.x.ax; d = outRegs.x.cx;
	FillFrame(1,1,80,25,0xF6,178); DlgWin(15,9,63,16,0x9E,SysInf[9]);
	PrintVRM(18,11,0x9F,"T›ng dung lž—ng trŒn ù¶a : %.3f MB", (float)a*c*d/1024.0/1024.0);
	PrintVRM(18,12,0x9F,"Dung lž—ng c¾n tr™ng trŒn ù¶a : %.3f MB", (float)b*c*d/1024.0/1024.0);
	Button(34,14,wATV,9,SysMen[0],1,wFLT);
	MoveMouse(38,12);
	if(((b*c*d) / 1024) <= 1024) {
		WriteVRM(18,13,0x94,SysInf[35]);
		do {
			if(ClickMouse(bCol, bRow) == 1) {
				if(bRow == 14 && bCol >= 34 && bCol <= 41) {
					Clear(34,14,42,15,9); WriteVRM(35,14,wATV,SysMen[0],wFLT);
					delay(50); Button(34,14,wATV,9,SysMen[0],1,wFLT); break;
				}
				if(bRow == 9 && bCol == 15 || bCol == 16) break;
			}
		} while(!kbhit()); HaltSys();
	}
	do {
		if(ClickMouse(bCol, bRow) == 1) {
			if(bRow == 14 && bCol >= 34 && bCol <= 41) {
				HideMouse(); Clear(34,14,42,15,9);
				WriteVRM(35,14,wATV,SysMen[0],wFLT); delay(50);
				Button(34,14,wATV,9,SysMen[0],1,wFLT); ShowMouse(); break;
			}
			if(bRow == 9 && bCol == 15 || bCol == 16) break;
		}
	} while(!kbhit());
}

/*------------------------------------*/
/* Funtion : StartInstall             */
/* Mission : Executting the functions */
/* Expects : Nothing                  */
/* Returns : Nothing                  */
/*------------------------------------*/
void StartInstall()
{
	char szDisk[17];

	ChooseDrive(); DiskCheck(); //UserCheck();
	InstallProgram(); UpdateProgram(); Unregister(); FadeIN();
	strcpy(szDisk,Drive); strcat(szDisk,"TOPICS");
	chdir(szDisk); Drive[2] = '\0'; system(Drive); system("readme");
	RestartProgram();
}

/*---------------------------------------------*/
/* Funtion : Install                           */
/* Mission : Showing information setup program */
/* Expects : Nothing                           */
/* Returns : Nothing                           */
/*---------------------------------------------*/
void Install()
{
	byte i = 0, fltStop[3], found = 0, cancel = 0;
	memset(fltStop,0,3); textattr(0x1F);
	SetBorder(59); SetCursorSize(0x0B0A);
	clrscr(); isBlinking(0); FillFrame(1,1,80,25,0xFE,178);
	DlgWin(8,2,74,23,0x1F,SysInf[74]);
	WriteVRM(13,4,0x1A,SysInf[10]);
	WriteChar(11,5,0x1B,61,205);
	WriteVRM(12,6,0x1B,SysInf[36]); WriteVRM(23,8,0x1F,SysInf[44]);
	WriteVRM(23,9,0x1F,SysInf[45]); WriteVRM(23,10,0x1F,SysInf[46]);
	WriteVRM(23,12,0x1E,SysInf[47]); WriteVRM(23,13,0x1E,SysInf[48]);
	WriteVRM(23,14,0x1E,SysInf[49]); WriteVRM(23,16,0x1B,SysInf[50]);
	WriteVRM(23,17,0x1B,SysInf[51]); WriteVRM(23,18,0x1B,SysInf[52]);
	WriteChar(9,19,0x20,65,196); PrintXY(8,19,0x1F,195);
	PrintXY(74,19,0x1F,180); WriteVRM(14,11,0x1C,SysMen[17],0x1E);
	WriteVRM(14,15,0x1C,SysMen[18],0x1E); if(!InitMouse())
	system(SysInf[43]); SetMousePos(); Button(24,21,wATV,1,SysMen[1],1,wFLT);
	Button(48,21,_wATV,1,SysMen[4],1,_wFLT); label1:
	found = cancel = 0;
	for(i = 0; i < 3 && !found; i++) if(!fltStop[i]) found = 1;
	if(found) {
		i--; SetCursorSize(0x0B0A); gotoxy(15,7+4*i);
		WriteVRM(14,7+4*i,0x1A,SysMen[16+i],0x1F); slc = i;
	}
	key = 0;
	do {
		if(kbhit()) {
			key = getch(); if(!key) key = getch();
			switch(toupper(key)) {
			case UP :
				if(!fltStop[slc]) WriteVRM(14,7+4*slc,0x1C,SysMen[16+slc],0x1E);
				else {
					WriteVRM(14,7+4*slc,0x1C,SysMen[16+slc],0x1E);
					PrintXY(15,7+4*slc,0x1C,251);
				}
				if(slc <= 0) slc = 2; else slc--;
				if(!fltStop[slc]) WriteVRM(14,7+4*slc,0x1A,SysMen[16+slc],0x1F);
				else {
					WriteVRM(14,7+4*slc,0x1A,SysMen[16+slc],0x1F);
					PrintXY(15,7+4*slc,0x1A,251);
				} gotoxy(15,7+4*slc); break;
			case DOWN :
				if(!fltStop[slc]) WriteVRM(14,7+4*slc,0x1C,SysMen[16+slc],0x1E);
				else {
					WriteVRM(14,7+4*slc,0x1C,SysMen[16+slc],0x1E);
					PrintXY(15,7+4*slc,0x1C,251);
				}
				if(slc >= 2) slc = 0; else slc++;
				if(!fltStop[slc]) WriteVRM(14,7+4*slc,0x1A,SysMen[16+slc],0x1F);
				else {
					WriteVRM(14,7+4*slc,0x1A,SysMen[16+slc],0x1F);
					PrintXY(15,7+4*slc,0x1A,251);
				} gotoxy(15,7+4*slc); break;
			case HOME :
				if(!fltStop[slc]) WriteVRM(14,7+4*slc,0x1C,SysMen[16+slc],0x1E);
				else {
					WriteVRM(14,7+4*slc,0x1C,SysMen[16+slc],0x1E);
					PrintXY(15,7+4*slc,0x1C,251);
				} slc = 0;
				if(!fltStop[slc]) WriteVRM(14,7+4*slc,0x1A,SysMen[16+slc],0x1F);
				else {
					WriteVRM(14,7+4*slc,0x1A,SysMen[16+slc],0x1F);
					PrintXY(15,7+4*slc,0x1A,251);
				} gotoxy(15,7+4*slc); break;
			case END :
				if(!fltStop[slc]) WriteVRM(14,7+4*slc,0x1C,SysMen[16+slc],0x1E);
				else {
					WriteVRM(14,7+4*slc,0x1C,SysMen[16+slc],0x1E);
					PrintXY(15,7+4*slc,0x1C,251);
				} slc = 2;
				if(!fltStop[slc]) WriteVRM(14,7+4*slc,0x1A,SysMen[16+slc],0x1F);
				else {
					WriteVRM(14,7+4*slc,0x1A,SysMen[16+slc],0x1F);
					PrintXY(15,7+4*slc,0x1A,251);
				} gotoxy(15,7+4*slc); break;
			case LEFT :
				Button(24+chs*24,21,_wATV,1,SysMen[3*chs+1],1,_wFLT);
				if(chs <= 0) chs = 1; else chs--;
				Button(24+chs*24,21,wATV,1,SysMen[3*chs+1],1,wFLT); break;
			case RIGHT :
				Button(24+chs*24,21,_wATV,1,SysMen[3*chs+1],1,_wFLT);
				if(chs >= 1) chs = 0; else chs++;
				Button(24+chs*24,21,wATV,1,SysMen[3*chs+1],1,wFLT); break;
			}
			if(key == 32) {
				if(fltStop[slc]) {
					WriteVRM(14,7+4*slc,0x1A,SysMen[16+slc],0x1F);
					fltStop[slc] = 0;
				} else { PrintXY(15,7+4*slc,0x1A,251); fltStop[slc] = 1; }
			}
		}
		if(ClickMouse(bCol, bRow) == 1) {
			if(bRow == 7 && bCol >= 14 && bCol <= 48) {
				HideMouse();
				if(!fltStop[slc]) WriteVRM(14,7+4*slc,0x1C,SysMen[16+slc],0x1E);
				else {
					WriteVRM(14,7+4*slc,0x1C,SysMen[16+slc],0x1E);
					PrintXY(15,7+4*slc,0x1C,251);
				} slc = 0;
				if(!fltStop[slc]) {
					WriteVRM(14,7,0x1A,SysMen[16],0x1F);
					PrintXY(15,7,0x1A,251); fltStop[0] = 1;
				} else { WriteVRM(14,7,0x1A,SysMen[16],0x1F); fltStop[0] = 0; }
				gotoxy(15,7); ShowMouse(); delay(150);
			}
			if(bRow == 11 && bCol >= 14 && bCol <= 57) {
				HideMouse();
				if(!fltStop[slc]) WriteVRM(14,7+4*slc,0x1C,SysMen[16+slc],0x1E);
				else {
					WriteVRM(14,7+4*slc,0x1C,SysMen[16+slc],0x1E);
					PrintXY(15,7+4*slc,0x1C,251);
				} slc = 1;
				if(!fltStop[slc]) {
					WriteVRM(14,11,0x1A,SysMen[17],0x1F);
					PrintXY(15,11,0x1A,251); fltStop[1] = 1;
				} else { WriteVRM(14,11,0x1A,SysMen[17],0x1F); fltStop[1] = 0; }
				gotoxy(15,11); ShowMouse(); delay(150);
			}
			if(bRow == 15 && bCol >= 14 && bCol <= 52) {
				HideMouse();
				if(!fltStop[slc]) WriteVRM(14,7+4*slc,0x1C,SysMen[16+slc],0x1E);
				else {
					WriteVRM(14,7+4*slc,0x1C,SysMen[16+slc],0x1E);
					PrintXY(15,7+4*slc,0x1C,251);
				} slc = 2;
				if(!fltStop[slc]) {
					WriteVRM(14,15,0x1A,SysMen[18],0x1F);
					PrintXY(15,15,0x1A,251); fltStop[2] = 1;
				} else { WriteVRM(14,15,0x1A,SysMen[18],0x1F); fltStop[2] = 0; }
				gotoxy(15,15); ShowMouse(); delay(150);
			}
			if(bRow == 21 && bCol >= 24 && bCol <= 36) {
				HideMouse(); Button(48,21,_wATV,1,SysMen[4],1,_wFLT);
				Clear(24,21,37,22,1); WriteVRM(25,21,wATV,SysMen[1],wFLT);
				delay(50); Button(24,21,wATV,1,SysMen[1],1,wFLT);
				ShowMouse(); chs = 0; key = ENTER;
			}
			if(bRow == 21 && bCol >= 48 && bCol <= 60) {
				HideMouse(); Button(24,21,_wATV,1,SysMen[1],1,_wFLT);
				Clear(48,21,61,22,1); WriteVRM(49,21,wATV,SysMen[4],wFLT);
				delay(50); Button(48,21,wATV,1,SysMen[4],1,wFLT);
				ShowMouse(); chs = 1; key = ENTER;
			}
			if(bRow == 2 && bCol == 8 || bCol == 9) HaltSys();
		}
	} while(key != ENTER);
	if(fltStop[slc]) {
		WriteVRM(14,7+4*slc,0x1C,SysMen[16+slc],0x1E);
		PrintXY(15,7+4*slc,0x1C,251);
	}
	else WriteVRM(14,7+4*slc,0x1C,SysMen[16+slc],0x1E);
	if(!chs && !cancel) {
		if(fltStop[1] && fltStop[2]) StartInstall();
		else {
			HideMouse(); gettext(20,10,63,18,szBuff);
			msgSlc = MsgBox(20,10,61,17,msgError,2);
			if(msgSlc) HaltSys();
			else {
				HideMouse(); puttext(20,10,63,18,szBuff); ShowMouse();
				goto label1;
			}
		}
	}
	else {
		HideMouse(); gettext(20,10,63,17,szBuff);
		msgSlc = MsgBox(20,10,61,16,msgExit,1); if(!msgSlc) HaltSys();
		HideMouse(); puttext(20,10,63,17,szBuff); ShowMouse(); goto label1;
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
	InitData(); Install();
}
/*-------------------------------------------------------------------------*/
/*------------------END OF SOURCES FILE PROGRAM----------------------------*/
/*-------------------------------------------------------------------------*/