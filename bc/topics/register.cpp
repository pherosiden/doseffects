#include <dos.h>
#include <stdio.h>
#include <ctype.h>
#include <conio.h>
#include <stdlib.h>
#include <string.h>

#define VIDEO_ADDR	0xB800
#define MASK_BG		0x08
#define OFFSET(x, y) ( (x - 1)*2 + 160*(y - 1) )
#ifndef MK_FP
#define MK_FP(seg, ofs) ((void far*)((unsigned long)(seg) << 16 | (ofs)))
#endif

#define LEFT	75
#define RIGHT	77
#define ENTER	13

#define wATV	0xF0
#define wFLT	0xFC
#define _wATV	0x78
#define _wFLT	0x74

struct text_info txtInfo;    // Save information of the text mode
typedef unsigned char byte;  // Data type byte
typedef unsigned int word;   // Data type word
typedef byte bool;           // Data type boolean
char far *fpVRM =            // The pointer to video memory
(char far *)MK_FP(0xB800,0); // Address of monitor VGA
char szDrive[4];             // The disk letter
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
static char **SysInf, **SysMen;
byte bnInfo, bnMenu;

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
	szString[2] = 0;
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
/* Mission : Decode file sysinfor.sys */
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
void GetData(const char *inFile, const char *outFile, char **&szData, byte &wNumElm)
{
	FILE *fp;
	char szBuffer[110];

	FileDecode(inFile, outFile); fp = fopen(outFile,"rt");
	while(fgets(szBuffer,109,fp)) wNumElm++;
	if(!(szData = new char*[wNumElm])) {
		fprintf(stderr,"Not enough memory!"); exit(1);
	}
	wNumElm = 0; rewind(fp);
	while(fgets(szBuffer,109,fp)) {
		FontVNI(szBuffer);
		if(!(szData[wNumElm] = new char[strlen(szBuffer)+1])) {
			fprintf(stderr,"Not enough memory at pointer %u", wNumElm); exit(1);
		}
		szBuffer[strlen(szBuffer)-1] = 0; strcpy(szData[wNumElm],szBuffer);
		wNumElm++;
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
	for(int i = 0; i < bnInfo; i++) delete(SysInf[i]); delete(SysInf);
}

/*-------------------------------*/
/* Funtion : HaltSys             */
/* Mission : Restore environment */
/* Expects : Nothing             */
/* Returns : Nothing             */
/*-------------------------------*/
void HaltSys()
{
	char szPath[28];

	strcpy(szPath,szDrive); strcat(szPath,SysInf[23]); szPath[23] = '\0';
	strcat(szPath, "off"); SetBorder(0x00); SetCursorSize(0x0607); textattr(0x07); isBlinking(1);
	if(bmAvalid) CloseMouse(); clrscr(); system(szPath); ReleaseData();
	exit(EXIT_SUCCESS);
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
		delay(20);
	}
	outportb(0x3C8, 0);
	for(i = 0; i < 200; i++) outportb(0x3C9, bPalettes[i]);
}

/*----------------------------------------------*/
/* Function : InitData                          */
/* Mission  : Initialize parameters for program */
/* Expects  : Nothing                           */
/* Returns  : Nothing                           */
/*----------------------------------------------*/
void InitData()
{
	char szPath[32];
	strcpy(szPath,szDrive); strcat(szPath,"TOPICS\\SYSTEM\\register.dat");
	GetData(szPath,"register.$$$",SysInf,bnInfo); szPath[3] = '\0';
	strcat(szPath,SysInf[23]); system(szPath);
}

/*---------------------------------*/
/* Function : Register             */
/* Mission  : Starting registering */
/* Expects  : Nothing              */
/* Returns  : Nothing              */
/*---------------------------------*/
void Register()
{
	FILE *fptr;
	tagRegs tmp;
	char szCurrName[25], szCurrID[20], *szName;
	byte flgName = 1, flgID = 1;

	DlgWin(20,8,60,14,0x1F,"Input Serial Number");
	WriteVRM(23,10,0x1F,SysInf[5]); WriteVRM(23,12,0x1F,SysInf[6]);
	WriteChar(36,10,0x4A,24,32); WriteChar(36,12,0x4A,24,32);

	if(!(fptr = fopen(SysInf[21],"r+b"))) {
		fprintf(stderr,SysInf[22]); HaltSys();
	}
	fread(&tmp,sizeof(tagRegs),1,fptr); SetCursorSize(0x0B0A);
	szCurrName[0] = 25;
	gotoxy(36,10); textattr(0x4A); szName = cgets(szCurrName);
	if(strcmp(tmp.user,szName)) {
		WriteChar(36,10,0x4F,24,32); WriteVRM(36,10,0x4F,SysInf[7]); flgName = 0;
	}
	gotoxy(36,12); szCurrID[0] = 20; szName = cgets(szCurrID);
	if(strcmp(szName,SysInf[11])) {
		WriteChar(36,12,0x4F,24,32); WriteVRM(36,12,0x4F,SysInf[8]); flgID = 0;
	}
	if(flgName && flgID) {
		WriteVRM(30,13,0x1A,SysInf[9]); tmp.regs = 1; rewind(fptr);
		fwrite(&tmp,sizeof(tagRegs),1,fptr); fclose(fptr);
	}
	else {
		WriteVRM(30,13,0x1A,SysInf[10]); fclose(fptr);
	}
}

/*-------------------------------------*/
/* Funtion : MenuRegister              */
/* Mission : Showing the menu Register */
/* Expects : Nothing                   */
/* Returns : Nothing                   */
/*-------------------------------------*/
void MenuRegister()
{
	char key;
	byte bSlc = 0, bCol = 0, bRow = 0;

	InitMouse(); Button(22,20,wATV,5,SysInf[25],1,wFLT);
	Button(47,20,_wATV,5,SysInf[26],1,_wFLT);
	do {
		if(kbhit()) {
			key = getch(); if(!key) key = getch();
			switch(toupper(key)) {
			case LEFT :
				Button(22+bSlc*25,20,_wATV,5,SysInf[25+bSlc],1,_wFLT);
				if(bSlc <= 0) bSlc = 1; else bSlc--;
				Button(22+bSlc*25,20,wATV,5,SysInf[25+bSlc],1,wFLT); break;
			case RIGHT :
				Button(22+bSlc*25,20,_wATV,5,SysInf[25+bSlc],1,_wFLT);
				if(bSlc >= 1) bSlc = 0; else bSlc++;
				Button(22+bSlc*25,20,wATV,5,SysInf[25+bSlc],1,wFLT); break;
			}
		}
		if(ClickMouse(bCol, bRow) == 1) {
			if(bRow == 20 && bCol >= 22 && bCol <= 33) {
				HideMouse(); Button(47,20,_wATV,5,SysInf[26],1,_wFLT);
				Clear(22,20,34,21,5); WriteVRM(23,20,wATV,SysInf[25],wFLT);
				delay(50); Button(22,20,wATV,5,SysInf[25],1,wFLT);
				ShowMouse(); bSlc = 0; key = ENTER;
			}
			if(bRow == 20 && bCol >= 47 && bCol <= 57) {
				HideMouse(); Button(22,20,_wATV,5,SysInf[25],1,_wFLT);
				Clear(47,20,58,21,5); WriteVRM(48,20,wATV,SysInf[26],wFLT);
				delay(50); Button(47,20,wATV,5,SysInf[26],1,wFLT);
				ShowMouse(); bSlc = 1; key = ENTER;
			}
			if(bCol == 3 || bCol == 4 && bRow == 2) key = ENTER;
		}
	} while(key != ENTER);
	if(!bSlc) {
		Register(); SetCursorSize(0x2020); getch(); FadeIN();
	}
}

/*--------------------------------------------------*/
/* Funtion : StartRegister                          */
/* Mission : Showing the menu StartRegister message */
/* Expects : Nothing                                */
/* Returns : Nothing                                */
/*--------------------------------------------------*/
void StartRegister()
{
	textattr(0x1F); SetBorder(55); SetCursorSize(0x2020); clrscr();
	isBlinking(0); FillFrame(1,1,80,25,0xFD,178);
	DlgWin(3,3,77,22,0x5F,SysInf[0]);
	WriteVRM(8,5,0x5E,SysInf[1]); WriteVRM(15,6,0x5E,SysInf[2]);
	WriteVRM(5,8,0x5F,SysInf[3]); WriteVRM(5,9,0x5F,SysInf[4]);
	WriteVRM(5,10,0x5F,SysInf[12]); WriteVRM(5,11,0x5F,SysInf[13]);
	WriteVRM(15,12,0x5A,SysInf[14]); WriteVRM(15,13,0x5A,SysInf[15]);
	WriteVRM(15,14,0x5A,SysInf[16]); WriteVRM(5,15,0x5F,SysInf[17]);
	WriteVRM(5,16,0x5F,SysInf[18]); WriteVRM(5,17,0x5F,SysInf[19]);
	WriteVRM(7,18,0x5E,SysInf[20]); MenuRegister();
}

/*----------------------------------------*/
/* Funtion : isRegister                   */
/* Mission : Checking value of regsiter   */
/* Expects : Nothing                      */
/* Returns : Value 0 if not StartRegister */
/*----------------------------------------*/
bool isRegister()
{
	FILE *fptr;
	tagRegs tmp;

	if(!(fptr = fopen(SysInf[21],"rb"))) {
		fprintf(stderr,SysInf[22]); getch(); HaltSys();
	}
	fread(&tmp,sizeof(tagRegs),1,fptr); fclose(fptr);
	return tmp.regs ? 1 : 0;
}

/*--------------------------------------*/
/* Funtion : CheckPeriod                */
/* Mission : Checking the period in use */
/* Expects : Nothing                    */
/* Returns : Nothing                    */
/*--------------------------------------*/
void CheckPeriod()
{
	FILE *fptr;
	tagRegs tmp;
	date da;
	char szPath[31];
	getdate(&da); strcpy(szPath,szDrive);
	strcat(szPath,SysInf[24]);
	if(!(fptr = fopen(SysInf[21], "rb"))) {
		fprintf(stderr, SysInf[22]); getch(); HaltSys();
	}
	fread(&tmp,sizeof(tagRegs),1,fptr); fclose(fptr);
	strcat(szPath, tmp.disk);
	if((tmp.num >= 100) && !tmp.regs) {
		system(szPath); HaltSys();
	}
	if(tmp.month == da.da_mon) {
		if((da.da_day - tmp.day) >= 7) {
			system(szPath); HaltSys();
		}
	}
	else {
		if(((da.da_day + 31) - tmp.day) >= 7) {
			system(szPath); HaltSys();
		}
	}
}

int main(int argc, char *argv[])
{
	if(argc < 2) return 1;
	if(!strcmp(argv[1],"C:\\") || !strcmp(argv[1],"D:\\") || !strcmp(argv[1],"E:\\")) {
		strcpy(szDrive,argv[1]);
		InitData();
		if(!isRegister()) {
			 CheckPeriod(); StartRegister(); HaltSys();
		}
		ReleaseData();
	}
	return 0;
}
