/*------------------------------------------------------*/
/*      UNIVERSITY OF TECHNOLOGY HO CHI MINH CITY       */
/*          THE INFORMATIC TECHNOLOGY BRANCH            */
/*               THE CRACKING PROGRAM FILE              */
/*            Author : NGUYEN NGOC VAN                  */
/*             Class : 00DTH1                           */
/*      Student Code : 00DTH201                         */
/*            Course : 2000 - 2005                      */
/*     Writting date : 24/10/2001                       */
/*       Last Update : 12/11/2001                       */
/*------------------------------------------------------*/
/*       Environment : Borland C++ Ver 3.1 Application  */
/*       Source file : CRACK.CPP                        */
/*           Compile : BCC -mt CRACK.CPP                */
/*            Linker : EXE2BIN CRACK.EXE CRACK.COM      */
/*      Momery Model : Tiny                             */
/*       Call To Run : CRACK (none project file)        */
/*------------------------------------------------------*/
/*         THE MAIN FUNCTIONS SUPPORT SOURCES           */
/*------------------------------------------------------*/
/* showing_menu_user; set_cursor_size; set_border_color;*/
/* get_text_color   ; get_back_color ; set_attrib      ;*/
/* box_shadow       ; fill_frame     ; replicate       ;*/
/* write_char       ; replicate      ; write_VRAM      ;*/
/* exit_program     ; execute_user   ; button          ;*/
/* do_blinking      ; write_XY       ; frame           ;*/
/*------------------------------------------------------*/
#include <dir.h>
#include <dos.h>
#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>

#define UP		72
#define DOWN	80
#define ENTER	13
#define ATV 	0xF0
#define FLT 	0xFC
#define _ATV 	0x78
#define _FLT 	0x75

#define VIDEO_ADDR	0xB800
#define MASK_BG 		0x08
#define OFFSET(x,y) 	( (x -1)*2 + 160*(y-1) )
#ifndef MK_FP
#define MK_FP(seg,ofs) ((void far*)((unsigned long)(seg) << 16 | (ofs)))
#endif

// Grobal variable
struct text_info txtInfo;    // Current text window information
typedef unsigned char byte;  // Data type byte
typedef unsigned int word;   // Data type word
typedef byte bool;           // Data type boolean
char far *fpVRM =            // The far pointer to video memory
(char far *)MK_FP(0xB800,0); // Space memory monitor
char CDKey[20];              // Product code
char szUserName[31];         // Registers name user
byte bmAvalid = 0;           // Status of the mouse

struct tagInfo {             // The struction storing the information
	byte day;                 // The date of the program
	byte month;               // The month of the program
	byte regs;                // The register code
	byte num;                 // The number of run program
	char serial[20];          // Product code
	char user[31];            // Register name user
	char disk[4];             // The disk letter
};

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
/* Funtion : SetCursor                 */
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
	system("font off"); exit(EXIT_SUCCESS);
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
	byte col = 0, row = 0;

	if(!(fptr = fopen("register.dat", "wb"))) HaltSys();
	getdate(&da); tmp.day = da.da_day; tmp.month = da.da_mon;
	tmp.regs = 0; tmp.num = 0; strcpy(tmp.serial, CDKey);
	strcpy(tmp.user, szUserName); strcpy(tmp.disk, "C:\\");
	fwrite(&tmp,sizeof(tagInfo),1,fptr); fclose(fptr);
	do {
		if(ClickMouse(col, row) == 1) {
			if(row == 10 && col >= 51 && col <= 62) {
				HideMouse(); Button(51,8,_ATV,3,"   ~L‡y m§   ",0,_FLT);
				Clear(51,10,64,11,3); WriteVRM(52,10,ATV,"   ~Ch‡m dŸt   ",FLT);
				delay(50); Button(51,10,ATV,3,"   ~Ch‡m dŸt   ",0,FLT);
				HaltSys();
			}
			if((col == 15 || col == 16) && row == 6) HaltSys();
		}
	} while(!kbhit()); HaltSys();
}

/*-------------------------------------------------*/
/* Function : StartCracking                        */
/* Mission  : Showing the message to select option */
/* Expects  : Nothing                              */
/* Returns  : Nothing                              */
/*-------------------------------------------------*/
void StartCracking()
{
	static char
	*szMenu[] = {"   ~La61y ma4   ","  ~Cha61m du71t  ", "  ~Tro75 giu1p  "};
	char cKey;
	byte bSlc = 0, isASCII = 1, bCurrCol = 0, bCol = 0, bRow = 0, i;

	for(i = 0; i < 3; i++) FontVNI(szMenu[i]);
	SetCursorSize(0x0B0A); Button(51,8,ATV,3,szMenu[0],0,FLT);
	Button(51,10,_ATV,3,szMenu[1],1,_FLT); MoveMouse(35,10);
	Button(51,12,_ATV,3,szMenu[2],1,_FLT);
	WriteChar(18,9,0x1A,30,32); textattr(0x1A);
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
						Button(51,8+bSlc*2,_ATV,3,szMenu[bSlc],1,_FLT);
						if(bSlc <= 0) bSlc = 2; else bSlc--;
						Button(51,8+bSlc*2,ATV,3,szMenu[bSlc],0,FLT);
					} break;
				case DOWN :
					if(!isASCII) {
						Button(51,8+bSlc*2,_ATV,3,szMenu[bSlc],1,_FLT);
						if(bSlc >= 2) bSlc = 0; else bSlc++;
						Button(51,8+bSlc*2,ATV,3,szMenu[bSlc],0,FLT);
					}
			}
		}
		if(ClickMouse(bCol, bRow) == 1) {
			if(bRow == 8 && bCol >= 51 && bCol <= 62) {
				HideMouse(); Button(51,8+bSlc*2,_ATV,3,szMenu[bSlc],1,_FLT);
				bSlc = 0; Clear(51,8+bSlc*2,64,9,3);
				WriteVRM(52,8+bSlc*2,ATV,szMenu[bSlc],FLT); delay(50);
				Button(51,8+bSlc*2,ATV,3,szMenu[bSlc],0,FLT); ShowMouse();
				cKey = ENTER;
			}
			if(bRow == 10 && bCol >= 51 && bCol <= 62) {
				HideMouse(); Button(51,8+bSlc*2,_ATV,3,szMenu[bSlc],1,_FLT);
				bSlc = 1; Clear(51,8+bSlc*2,64,11,3);
				WriteVRM(52,8+bSlc*2,ATV,szMenu[bSlc],FLT); delay(50);
				Button(51,8+bSlc*2,ATV,3,szMenu[bSlc],0,FLT); ShowMouse();
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
				byte bCol, bRow, i;
				char *buffer = new char[512];
				for(i = 0; i < 7; i++) FontVNI(MsgHelp[i]);
				Button(51,8+bSlc*2,_ATV,3,szMenu[bSlc],1,_FLT); bSlc = 2;
				Clear(51,8+bSlc*2,64,13,3);
				WriteVRM(52,8+bSlc*2,ATV,szMenu[bSlc],FLT); delay(50);
				Button(51,8+bSlc*2,ATV,3,szMenu[bSlc],0,FLT); HideMouse();
				gettext(15,6,67,17,buffer);
				DlgWin(15,6,65,16,0x5F,MsgHelp[0]); WriteVRM(17,8,0x5E,MsgHelp[1]);
				WriteVRM(17,9,0x5E,MsgHelp[2]); WriteVRM(17,10,0x5E,MsgHelp[3]);
				WriteVRM(17,11,0x5E,MsgHelp[4]); WriteVRM(17,12,0x5E,MsgHelp[5]);
				Button(36,14,ATV,5,MsgHelp[6],1,FLT); ShowMouse(); SetCursorSize(0x2020);
				do {
					if(ClickMouse(bCol, bRow) == 1) {
						if(bRow == 14 && bCol >= 36 && bCol <= 45) {
							HideMouse(); Clear(36,14,46,15,5);
							WriteVRM(37,14,ATV,MsgHelp[6],FLT); delay(50);
							Button(36,14,ATV,5,MsgHelp[6],1,FLT); break;
						}
					}
				} while(!kbhit()); SetCursorSize(0x0B0A);
				puttext(15,6,67,17,buffer); ShowMouse(); delete[]buffer;
			}
			if(bRow == 6 && (bCol == 15 || bCol == 16)) HaltSys();
		}
		szUserName[bCurrCol] = NULL;
	} while(cKey != ENTER || !szUserName[0]);
	if(!bSlc) {
		SetCursorSize(0x2020); WriteVRM(18,12,0x1A,CDKey); GetSerialNumber();
	} else HaltSys();
}

/*-------------------------------------------*/
/* Function : main                           */
/* Mission  : Linker funtions to run program */
/* Expects  : Nothing                        */
/* Returns  : Nothing                        */
/*-------------------------------------------*/
void main()
{
	char cKey, *szMenu[] =
	{"Ma4 so61 ca2i d9a85t", "Nha65p te6n cu3a ba5n",
	"So61 ca2i d9a85t", "mouse.com",
	"font on"};
	byte i = 0;

	system(szMenu[4]);
	for(i = 0; i < 3; i++) FontVNI(szMenu[i]); i = 0;
	textattr(0x1F); SetBorder(30);
	SetCursorSize(0x0B0A); clrscr(); isBlinking(0);
	FillFrame(1,1,80,25,0x7D,178);
	DlgWin(15,6,65,14,0x3F,szMenu[0]);
	WriteVRM(18,8,0x3F,szMenu[1]);
	WriteVRM(18,11,0x3F,szMenu[2]); WriteChar(18,12,0x1A,30,32);
	if(!InitMouse()) system(szMenu[3]);
	SetMousePos(); randomize();
	do {
		cKey = random(49) + 48;
		if(isdigit(cKey) || isupper(cKey)) CDKey[i++] = cKey;
	} while(i < 19); CDKey[i] = NULL;
	for(i = 0; i < strlen(CDKey) - 1; i++)
	if(!((i + 1) % 5)) CDKey[i] = 45; StartCracking();
}
/*------------------------------------------------------------------------*/
/*------------------END OF CRACKING PROGRAM FILE--------------------------*/
/*------------------------------------------------------------------------*/