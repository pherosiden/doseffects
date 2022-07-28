#include <dos.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <conio.h>

#define PGDWN 81
#define PGUP  73
#define DOWN  80
#define HOME  71
#define END   79
#define UP    72

#define VIDEO_ADDR 0xB800
#define OFFSET(x, y) 2*(x - 1) + 160*(y - 1)
#ifndef MK_FP
#define MK_FP(seg,ofs) ((void far*)((unsigned long)(seg) << 16 | (ofs)))
#endif

typedef unsigned char byte;
typedef unsigned int word;
char far *fpVRM = (char far *)MK_FP(0xB800,0);

/*------------------------------------------------*/
/* Funtion : WriteChar                            */
/* Mission : Writting a character with attribute  */
/* Expects : (x,y) cordinate to write a character */
/*           (attr) attribute of character        */
/*           (bLen) length area                   */
/*           (chr) symbol needs to write          */
/* Returns : Nothing                              */
/*------------------------------------------------*/
void WriteChar(byte x, byte y, byte bAttr, byte bLen, char Chr)
{
	word VideoOFS;
	byte i;
	VideoOFS = OFFSET(x,y);
	for(i = 1; i <= bLen; i++) {
		poke(VIDEO_ADDR, VideoOFS , (bAttr << 8) + Chr); VideoOFS += 2;
	}
}

/*-----------------------------------------------*/
/* Function : WriteVRM                           */
/* Mission  : Writing a character with attribute */
/* Notices  : Intervention in video memory       */
/* Expects  : (x,y) cordinate needs to writting  */
/*            (attr) The attribute of string     */
/*            (str) the string to format         */
/* Returns  : Nothing                            */
/*-----------------------------------------------*/
void WriteVRM(byte x, byte y, byte bAttr, const char *szMsg)
{
	while(*szMsg) {
		fpVRM[OFFSET(x,y)] = *szMsg++; fpVRM[OFFSET(x++,y)+1] = bAttr;
	}
}

/*------------------------------------------------*/
/* Function : Scroll                              */
/* Mission  : Scroll a line on the monitor        */
/* Expects  : (bNumRow) Number lines needs scroll */
/*            (bCol,bRow,colLR) The Coordinate    */
/*            (bType) scroll up or down           */
/* Returns  : Nothing                             */
/*------------------------------------------------*/
void Scroll(byte bNumRow, byte bColor, byte bCol, byte bRow, byte colLR, byte rowLR, byte bType)
{
	union REGS regs;
	regs.h.ah = bType; regs.h.al = bNumRow; regs.h.bh = bColor;
	regs.h.ch = bRow; regs.h.cl = bCol; regs.h.dh = rowLR;
	regs.h.dl = colLR; int86(0x10, &regs, &regs);
}

/*---------------------------------------------------*/
/* Function : GetVideoMode                           */
/* Mission  : Get information about mode video       */
/* Expects  : (bVideoMode) The mode of current video */
/*            (bNumCol) The number of lines          */
/*            (bCurrPage) The current page           */
/* Returns : Nothing                                 */
/*---------------------------------------------------*/
void GetVideoMode(byte &bVideoMode, byte &bNumCol, byte &bCurrPage)
{
	union REGS regs;
	regs.h.ah = 15; int86(0x10, &regs, &regs); bVideoMode = regs.h.al;
	bNumCol = regs.h.ah; bCurrPage = regs.h.bh;
}

/*-------------------------------------------*/
/* Function : SetCursorPos                   */
/* Mission  : Set the position of the cursor */
/* Expects  : (bPage) The current page       */
/*            (bCol,bRow) The position       */
/* Returns  : Nothing                        */
/*-------------------------------------------*/
void SetCursorPos(byte bPage, byte bCol, byte bRow)
{
	union REGS regs;
	regs.h.ah = 2; regs.h.bh = bPage; regs.h.dh = bCol;
	regs.h.dl = bRow; int86(0x10, &regs, &regs);
}

/*-----------------------------------*/
/* Function : ClearScreen            */
/* Mission  : Clear the blank screen */
/* Expects  : Nothing                */
/* Returns  : Nothing                */
/*-----------------------------------*/
void ClearScreen()
{
	byte dum_mode, dum_col, dum_page;
	Scroll(0,0x30,0,0,79,24,6); GetVideoMode(dum_mode, dum_col, dum_page);
	SetCursorPos(dum_page, 0, 0);
}

/*-----------------------------------*/
/* Funtion : SetBorder               */
/* Mission : Seting border color     */
/* Expects : (color) color of border */
/* Returns : Nothing                 */
/*-----------------------------------*/
void SetBorder(byte color)
{
	union REGS regs; regs.h.ah = 0x10; regs.h.al = 0x01;
	regs.h.bh = color & 63; int86(0x10, &regs, &regs);
}

/*-------------------------------------*/
/* Funtion : SetCursor                 */
/* Mission : Resize the cursor         */
/* Expects : (size) The size of cursor */
/* Returns : Nothing                   */
/*-------------------------------------*/
void SetCursor(word size)
{
	union REGS regs;
	regs.h.ah = 1; regs.x.cx = size; int86(0x10, &regs, &regs);
}

/*-----------------------------------------------*/
/* Function : isBlinking                         */
/* Mission  : Redefine bits 7 in the text attrib */
/* Expects  : (doblink) = 0, back color is light */
/*                      = 1, text blinking       */
/* Return   : Nothing                            */
/*-----------------------------------------------*/
void isBlinking(byte blink)
{
	union REGS regs;
	regs.h.ah = 0x10; regs.h.al = 0x03;
	regs.h.bl = blink ? 1 : 0; int86(0x10, &regs, &regs);
}

/*-----------------------------------------*/
/* Funtion : strPos                        */
/* Mission : Getting position of substring */
/* Expects : (str) The string main         */
/*           (substr) The substring        */
/* Returns : Position of substring         */
/*-----------------------------------------*/
int StrPos(char *szPrmpt, char *szSubStr)
{
	char *Ptr = strstr(szPrmpt, szSubStr);
	if(!Ptr) return -1;
	return (Ptr - szPrmpt);
}

/*---------------------------------------------*/
/* Funtion : InsertChar                        */
/* Mission : Inserted the char into string     */
/* Expects : (str) The string                  */
/*           (chr) The character need inserted */
/*           (i) The position inserted         */
/* Returns : Nothing                           */
/*---------------------------------------------*/
void InsertChar(char *szPrmpt, char chr, int i)
{
	if(i < 0 || i > strlen(szPrmpt)) return;
	*(szPrmpt+i) = chr;
}

/*---------------------------------------------*/
/* Funtion : StrDelete                         */
/* Mission : Deleting the characters           */
/* Expects : (str) The string main             */
/*           (i) position to delete            */
/*           (numchar) the number of character */
/* Returns : Nothing                           */
/*---------------------------------------------*/
void StrDelete(char *szPrmpt, int i, int nChar)
{
	if(i < 0 || i > strlen(szPrmpt)) return;
	memcpy((szPrmpt+i+1), (szPrmpt+i+nChar), (strlen(szPrmpt) - i - 1));
}

/*--------------------------------------*/
/* Funtion : SchRepl                    */
/* Mission : Concat the string          */
/* Expects : (str) The string main      */
/*           (sch) The substring        */
/*           (repl) The char to replace */
/* Returns : Nothing                    */
/*--------------------------------------*/
void SchRepl(char *szPrmpt, char *szSch, char cRepl)
{
	int i;
	do {
		i = StrPos(szPrmpt,szSch);
		if(i >= 0) {
			StrDelete(szPrmpt,i,strlen(szSch));
			InsertChar(szPrmpt,cRepl,i);
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
/* Mission : Decode font                */
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
/* Expects : (file_name) file name    */
/* Returns : Nothing                  */
/*------------------------------------*/
void FileDecode(const char *szInFile, const char *szOutFile)
{
	FILE *inHandle, *outHandle;
	int c, key = 98;

	inHandle = fopen(szInFile,"rb"); outHandle = fopen(szOutFile,"wb");
	if(!inHandle || !outHandle) {
		printf("Error loading file %s. System halt.",szInFile); exit(1);
	}
	while((c = fgetc(inHandle)) != EOF) {
		c = c - ~key; fputc(c, outHandle);
	}
	fclose(inHandle); fclose(outHandle);
}

/*------------------------------------------------*/
/* Function : GetData                             */
/* Mission  : Reading information into data array */
/* Expects  : (inf_file) the input file           */
/*            (outFile) the output file           */
/*            (data) the array data               */
/*            (num) the number of elements        */
/* Returns  : Nothing                             */
/*------------------------------------------------*/
void GetData(const char *szInFile, const char *szOutFile, char **&szData, word &wNumElm)
{
	FILE *fp;
	char szBuffer[110];

	FileDecode(szInFile, szOutFile); fp = fopen(szOutFile,"rt");
	while(fgets(szBuffer,109,fp)) wNumElm++;
	if(!(szData = new char*[wNumElm])) {
		printf("Not enough memory!"); exit(1);
	}
	wNumElm = 0; rewind(fp);
	while(fgets(szBuffer,109,fp)) {
		FontVNI(szBuffer);
		if(!(szData[wNumElm] = new char[strlen(szBuffer)+1])) {
			printf("Not enough memory at pointer %u", wNumElm); exit(1);
		}
		strcpy(szData[wNumElm],szBuffer); wNumElm++;
	}
	fclose(fp); unlink(szOutFile);
}

/*------------------------------------------*/
/* Function : ReleaseData                   */
/* Mission  : free block memory of the data */
/* Expects  : Nothing                       */
/* Returns  : Nothing                       */
/*------------------------------------------*/
void ReleaseData(char **szLine, word wNumLines)
{
	byte i;
	for(i = 0; i < wNumLines; i++) delete[]szLine[i]; delete[]szLine;
}

/*---------------------------------------------*/
/* Function : ViewFile                         */
/* Mission  : Display the information on scren */
/* Expects  : (szFileName) The name of file    */
/* Returns  : Nothing                          */
/*---------------------------------------------*/
void ViewFile(char *szfileName)
{
	char **szLine, key = 0;
	char *szTitle[] = {"Ta65p tin README", "Du2ng:    - PgUp - PgDwn - Home - End d9e63 d9ie62u khie63n. ESC d9e63 thoa1t","Nga2y ca65p nha65t"};
	struct time t;
	struct date da;
	word wNumLines = 0, wCurrLine = 0, i = 0, j = 0;

	GetData(szfileName,"readme.$$$",szLine, wNumLines); getdate(&da); ClearScreen();
	SetCursor(0x2020); SetBorder(43); isBlinking(0);
	WriteChar(1,1,0x4E,80,32); WriteChar(1,25,0x4E,80,32);
	for(i = 0; i < 3; i++) FontVNI(szTitle[i]); i = 0;
	WriteVRM(31,1,0x4F,szTitle[0]); WriteChar(27,1,0x4A,3,3);
	WriteChar(46,1,0x4A,3,3);
	WriteVRM(2,25,0x4F,szTitle[1]);
	WriteChar(8,25,0x4F,1,24); WriteChar(9,25,0x4F,1,25); gotoxy(2,1);
	printf("C‹p nh‹t : %.2d/%.2d/%d",da.da_day,da.da_mon,da.da_year);
	gotoxy(1,2); for(i = 0; i < 23; i++) printf("%s",szLine[i]); wCurrLine = 23;
	do {
		if(kbhit()) {
			key = getch(); if(!key) key = getch();
			switch(key) {
				case UP :
					if(wCurrLine > 23) {
						wCurrLine--; Scroll(1,0x30,0,1,79,23,7); gotoxy(1,2);
						printf("%s",szLine[wCurrLine - 23]);
					} break;
				case DOWN :
					if(wCurrLine < wNumLines) {
						Scroll(1,0x30,0,1,79,23,6);
						gotoxy(1,24); printf("%s",szLine[wCurrLine++]);
					} break;
				case PGDWN :
					if(wCurrLine < wNumLines) {
						if(wNumLines - wCurrLine >= 23) {
							Scroll(23,0x30,0,1,79,23,6); gotoxy(1,2);
							for(i = 0; i < 23; i++) printf("%s",szLine[wCurrLine++]);
						}
						else {
							Scroll(wNumLines - wCurrLine,0x30,0,1,79,23,6);
							gotoxy(1,23-(wNumLines - wCurrLine)+1); j = wCurrLine;
							for(i = 0; i < wNumLines - j; i++)
							printf("%s",szLine[wCurrLine++]);
						}
					} break;
				case PGUP :
					if(wCurrLine > 23) {
						if(wCurrLine <= 46) {
							Scroll(wCurrLine - 23,0x30,0,1,79,23,7); gotoxy(1,2);
							 for(i = 0; i < wCurrLine - 23; i++)
							 printf("%s",szLine[i]); wCurrLine = 23;
						}
						else {
							Scroll(23,0x30,0,1,79,23,7); gotoxy(1,2);
							for(i = wCurrLine - 46; i < wCurrLine - 23; i++)
							printf("%s",szLine[i]); wCurrLine = wCurrLine - 23;
						}
					} break;
				case END :
					if(wCurrLine != wNumLines) {
						wCurrLine = wNumLines - 23;
						Scroll(23,0x30,0,1,79,23,6); gotoxy(1,2);
						for(i = 0; i < 23; i++) printf("%s",szLine[wCurrLine++]);
					} break;
				case HOME :
					if(wCurrLine != 23) {
						wCurrLine = 0;
						Scroll(23,0x30,0,1,79,23,7); gotoxy(1,2);
						for(i = 0; i < 23; i++) printf("%s",szLine[wCurrLine++]);
					}
			}
		}
		gettime(&t); fpVRM[142] = t.ti_hour/10 + '0';
		fpVRM[144] = t.ti_hour%10 + '0'; fpVRM[146] = ':';
		fpVRM[148] = t.ti_min/10 + '0'; fpVRM[150] = t.ti_min%10 + '0';
		fpVRM[152] = ':'; fpVRM[154] = t.ti_sec/10 + '0';
		fpVRM[156] = t.ti_sec%10 + '0';
	} while(key != 27); ReleaseData(szLine, wNumLines);
}

void main() {
	ViewFile("readme.hlp"); SetCursor(0x0607);
}