/*--------------------------------------------------------------*/
/*        UNIVERSITY OF TECHNOLOGY HO CHI MINH CITY             */
/*             MAJOR OF INFORMATIC TECHNOLOGY                   */
/*            ALGORITHM TRANSFROM 2 DECRADTERS                  */
/*             Author : NGUYEN NGOC VAN                         */
/*              Class : 00DTH1                                  */
/*       Student Code : 00DTH201                                */
/*             Course : 2000 - 2005                             */
/*      Writting Date : 02/11/2001                              */
/*        Last Update : 24/11/2001                              */
/*--------------------------------------------------------------*/
/*        Environment : Borland C++ Ver 3.1 Application         */
/*        Source File : TRANFROM.CPP                            */
/*        Memory Mode : Small                                   */
/*            Compile : BCC TRANFROM.CPP                        */
/*        Call to run : TRANFROM (none the project file)        */
/*--------------------------------------------------------------*/

#include <dos.h>
#include <math.h>
#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <graphics.h>

#define MAX     20
#define ENTER   13
#define TAB      9
#define DEL      8
#define NEWLINE 10
#define ESC     27
#define LEFT    75
#define RIGHT   77
#define UP      72
#define DOWN    80
#define HOME    71
#define END     79
#define CBG	BLUE
#define TWOPI   6.283185
#define MAXNUM  1000
#define YAXIS   1
#define PI      3.141593
#define TRUE    1
#define FALSE   0
#define MIN(a,b) ( (a) < (b) ? (a) : (b) )
#define OFFSET(x,y) ( (x - 1)*2 + 160*(y - 1) )
#ifndef MK_FP
#define MK_FP(seg, ofs) ((void far*)((unsigned long)(seg) << 16 | (ofs)))
#endif

/*----------------------------------------------------------*/
/*             CAC HAM CHAY CHUONG TRINH CHINH              */
/*----------------------------------------------------------*/
void explain(void);
void Introduction(void);
void decor(void);
void OutTextWidth(int, int, char *, int);
void OutTextShade(int, int, char *, int, int, int, int);
void BackGround(void);
void Main_Menu(void);
void Windows(int, char **, int, int) ;
void Windows0(int, char **, int, int);
void Execult(int);
void Execult0(int);
void PopUp0(void);

/*----------------------------------------------------------*/
/*          CAC HAM THIET KE CHUONG TRINH TRINH BAY         */
/*----------------------------------------------------------*/
float DegToRad(float);
void PopUp1(void);
void Windows1(int, char **, int, int);
void Execult1(int);
void PopUp2(void);
void Windows2(int, char **, int, int);
void Execult2(int);
void PopUp3(void);
void Windows3(int, char **, int, int);
void Execult3(int);
void PopUp4(void);
void Windows4(int, char **, int, int);
void Execult4(int);
void PopUp5(void);
void Windows5(int, char **, int, int);
void Execult5(int);
void Triangle(int, int, int, int, int, int, int);
void Rotate2(int *, int *, int, int, float);
void Rotate_Triangle(int, int, int, int, int, int, int, int, float, int);
void Translate(int *, int *, int, int);
void Stretch(int *, int *, float, float, float, int);
void Elastic_Triangle(int, int, int, int, int, int, float, float, float, int, int);
void Symmetry_Triangle(int, int, int, int, int, int, int, int, int);
void Screen_Result(int, int, int, int, int);
void Clear_Screen_Result(void);
void Draw_Triangle(void);

char far *buffer =          // Con tro tro toi bo nho man hinh
(char far *)MK_FP(0xB800,0);// Dia chi cua man hinh VGA
struct text_info info;      // Bang thong tin cua thouc tinh van ban
typedef unsigned char byte; // Du lieu kieu byte
typedef unsigned int word;  // Du lieu kieu word
typedef byte bits[8];       // Dinh nghia ma tran 8x8 cua mot ky tu
bits table_chars[256];      // Dinh nghia mot ban gom co 256 ky tu
byte mouse_avalid = FALSE;  // The flag initialize mouse
char disk[4];               // The disk letter

/*------------------------------------------------------------*/
/*             CAC HAM TAO GIAO DIEN VAN BAN MAN HINH         */
/*------------------------------------------------------------*/
void say_goodbye();
void start_graphics();
void get_password(void);
void exit_program();
void set_text_attrib(word);
void fill_frame(byte x1, byte y1, byte x2, byte y2, byte attr, char chr);
void set_border_color(byte);
void set_cursor_size(word);
void del_horizon(void);
void paint_big_char(bits);
void put_big_STR(byte, char *);
void putch_XY(byte, byte, char);
void put_char_XY(byte, byte, char);
void clear(byte, byte, byte, byte, byte);
void hide_mouse();
void show_mouse();
void set_pmcl();
void box_shadow(byte x1, byte y1, byte x2, byte y2, word box_attr, byte lane, word msg_attr, char *title, byte is_win = 0);
void put_box(byte, byte, byte, byte, word, char);
void write_VRAM(byte x, byte y, word text_attr, const char *str, word letter_attr = 0);
void button(byte x, byte y, word attr, byte back, const char *title, byte type = 0, word first_letter = 0);
void write_XY(byte, byte, word, char);
void do_blinking(byte);
char is_register();
void check_period();
void registers();
void close_mouse();
void next_pages();
byte click_mouse(byte *, byte *);
byte init_mouse();

// Tao Menu chinh cua chuong trinh
char *ListMenu[] = {
		      "VE DA GIAC",
		      " TINH TIEN",
		      "    XOAY",
		      "   CO GIAN",
		      "   DOI XUNG",
		      "     THOAT",
		   };

struct palettetype pal; // Bang mau nen duoc dinh nghia san

typedef struct point
	       {
		  float x, y;
	       } point; // Toa do diem anh

typedef struct polypoint
	       {
		  int num;
		  point pt[99];
	       } polypoint; // Dinh nghia cac canh cua tam giac

typedef struct {
   byte date;                // The date of the program
   byte month;               // The month of the program
   byte regs;                // The register code
   char serial[20];          // Product code
   char user[31];            // Register name user
   char disk[4];             // The disk letter
} INFO;

point CP;
float CD;

/*-----------------------------------*/
/*  Cho popup 0 va xoa ta phai       */
/*  nhap cd, angle, dist xem tap     */
/*  6 truong hop tao ra.     	     */
/*-----------------------------------*/
void right(float, float *);
void lineforward(float, float);

/*----------------------------------------------------*/
/*  CP is a global variable of point type             */
/*  CD is a global variable of float                  */
/*  TWOPI is a constant 2*pi                          */
/*  ta dung chung kieu khai bao cau truc point        */
/*  nhap tri cho CP.x, CP.y, CD la bien toan cuc      */
/*  sau do nhap tri cac bien cuc bo la dist,angle,    */
/*  incr, num  cho ve da giac Polyspiral              */
/*  truoc khi ve Polyspiral ta phai MoveTo(CP.x,CP.y) */
/*----------------------------------------------------*/
void MoveTo(point);
void Line(point, point);
void LineTo(point);      //Draw from CP to p and update CP
void Right(float);
void LineForward(float);
void Polyspiral(float, float, float, int);

/*---------------------------------------------------------*/
/*        CAC HAM VE DA GIAC TRONG HE TOA DO THUC          */
/*---------------------------------------------------------*/
void MakeNGon(float, int, polypoint *, point, float);
void PrimeRosette(float, int, point, float);

/*--------------------------------------------------------*/
/*       CAC HAM THIET KE CHUONG TRINH UNG DUNG           */
/*--------------------------------------------------------*/
void Triangle1(void);
void Triangle2(void);
void Triangle3(void);
void Triangle4(void);
void Triangle5(void);
void Tempo1(void);
void Tempo2(void);
void Apply0(void);
void Apply1(void);
void Apply2(void);
void Apply3(void);
void Apply4(void);
void Apply5(void);
void Apply6(void);
void Apply7(void);
void Apply8(void);
void ApplyPrim(void);
void ApplyTempo5(void);

/*-------------------------------------------------*/
/*      CAC HAM HUONG DAN SU DUNG CHUONG TRINH     */
/*-------------------------------------------------*/
void Help(void);
void Direction(void);
void Copyringht(void);
/*----------------------------------*/
/*       CHUONG TRINH CHINH         */
/*----------------------------------*/
void main()
{
   char path[25];
   if(!is_register()) {
      strcpy(path,disk);
      strcat(path,"TOPICS\\register ");
      strcat(path,disk);
      system(path);
      get_password();
   }
   explain();
   decor();
   Main_Menu();
}

/*----------------------------------*/
/*        HAM TAO MAIN MENU         */
/*----------------------------------*/
void Main_Menu()
{
    int Select = 0, Key, i, u, xtam, ytam;

    cleardevice();
    // Trinh bay cac thanh bar
    setfillstyle( SOLID_FILL, LIGHTGRAY );
    bar( 2, 2, getmaxx() - 5, 31 );
    setfillstyle( CLOSE_DOT_FILL, WHITE );
    bar( 4, 4, 30, 29 );
    setfillstyle( CLOSE_DOT_FILL, WHITE );
    bar( getmaxx() - 8, 4, getmaxx() - 50, 29);

    // Thanh bar cho cac phep bien hinh
    setfillstyle( SOLID_FILL, WHITE );
    bar( 32, 4, getmaxx() - 46, 29 );
    setfillstyle( SOLID_FILL, LIGHTGREEN );
    bar( 150, 4, getmaxx() - 156, 29 );
    setfillstyle( SOLID_FILL, LIGHTGRAY );
    bar( 6, getmaxy() - 43, getmaxx() - 10, getmaxy() - 10 );
    setfillstyle( SOLID_FILL, WHITE );
    bar( 9, getmaxy() - 41, getmaxx() - 13, getmaxy() - 13 );

    // Thanh bar cho phia duoi menu
    setfillstyle( SOLID_FILL, LIGHTCYAN );
    bar( 9, getmaxy() - 41, getmaxx() - 13, getmaxy() - 13 );

    // Viet sau ky tu noi phia day man hinh
    xtam = 10;
    ytam = getmaxy() - 45;
    for( u = 0; u < 4; u++ )
    {
	settextstyle( TRIPLEX_FONT, 0, 4 );
	setcolor( LIGHTRED );
	outtextxy( xtam + 38, ytam - 4, "TRUONG DH KY THUAT CONG NGHE" );
	xtam += 1;
	ytam += 1;
    }

    // Viet sau ky tu noi phia tren man hinh
    xtam = 170;
    ytam = 2;
    for( u = 0; u < 5; u++ )
    {
	settextstyle( TRIPLEX_FONT, 0, 3 );
	setcolor( LIGHTMAGENTA );
	outtextxy( xtam, ytam - 3, "CAC PHEP BIEN HINH 2D" );
	ytam += 1;
	xtam -= 1;
     }

     // Ve khung man hinh
     setbkcolor( CBG );
     setcolor( getmaxcolor() );
     rectangle( 1, 1, getmaxx() - 4, getmaxy() - 2 );
     rectangle( 4, 31, getmaxx() - 8, getmaxy() - 50 );
     rectangle( 7, 54, getmaxx() - 11, getmaxy() - 53 );
     setfillstyle(SOLID_FILL, BROWN );
     bar( 5, 33, getmaxx() - 9, 52  );
     rectangle( 4, getmaxy() - 45, getmaxx() - 8, getmaxy() - 7 );

     // Tao khung MENU chinh cua chuong trinh
     while( 1 )
     {
	// Mau cua Menu Bar
	setcolor( LIGHTCYAN );
	settextstyle( 0, 0, 1 );
	for( i = 0; i < 6; i++ )
	outtextxy( 100*i + 20, 39 , ListMenu[i] );
	Windows( Select, ListMenu, 15, 4 );
	Key = getch();
	if( !Key )
	{
	   Key = getch();
	   switch( Key )
	   {
	      case LEFT :
		Windows( Select, ListMenu, 6, 15 );
		if( Select <= 0 ) Select = 5;
		else Select-- ;
		Windows( Select, ListMenu, 15, 4 );
		break;

	      case RIGHT :
		Windows( Select, ListMenu, 6, 15 );
		if( Select >= 5 ) Select = 0;
		else Select++ ;
		Windows( Select, ListMenu, 15, 4 );
		break;

	      case HOME :
		Windows( Select, ListMenu, 6, 15 );
		Select = 0;
		Windows( Select, ListMenu, 15, 4 );
		break;

	      case END  :
		Windows( Select, ListMenu, 6, 15 );
		Select=5;
		Windows( Select, ListMenu, 15, 4 );
		break;
	   } // Of switch(Key)
	} // Of if (!Key)
	if( Key == ENTER ) Execult( Select ); // Lam den khi gap ENTER
     } // Of while(1)
} // Of MainNenu

/*---------------------------------------*/
/*       TAO HOP SANG TREN MAIN MENU     */
/*---------------------------------------*/

void Windows(int k, char *ch[], int bcolor, int lcolor)
{
  // Tao hop sang cho 1 o
  setfillstyle( SOLID_FILL, bcolor );
  bar( ((105*k)-1) + 8, 34, 105*(k+1) - 1, 49 );
  // Hien thi text trong thanh bar
  setcolor( lcolor );
  outtextxy( 100*k + 20, 39, ch[k] );
}

/*-----------------------------------------------------*/
/*           TAO HOP SANG TREN MENU POPUP              */
/*-----------------------------------------------------*/
void Windows0(int k, char *ch[], int bcolor, int lcolor)
{
  // Tao hop sang cho 1 o
  setfillstyle( SOLID_FILL, bcolor );
    bar( 12, (20*k) + 62, 108, 20*(k+1) + 61 );
  // Hien thi text trong thanh bar
  setcolor( lcolor );
  outtextxy( 20, 20*k + 68, ch[k] );
}

void Windows1(int k, char *ch[], int bcolor, int lcolor)
{
  // Tao hop sang cho 1 o
  setfillstyle( SOLID_FILL, bcolor );
  bar( 114, (20*k) + 62, 208, 20*(k+1) + 61 );
  // Hien thi text trong thanh bar
  setcolor( lcolor );
  outtextxy( 122, 20*k + 68, ch[k] );
}

void Windows2(int k, char *ch[], int bcolor, int lcolor)
{
  // Tao hop sang cho 1 o
  setfillstyle( SOLID_FILL, bcolor );
  bar( 220, (20*k) + 62, 313, 20*(k+1) + 61 );
  // Hien thi text trong thanh bar
  setcolor( lcolor );
  outtextxy( 228, 20*k + 68, ch[k] );
}

void Windows3(int k, char *ch[], int bcolor, int lcolor)
{
  // Tao hop sang cho 1 o
  setfillstyle( SOLID_FILL, bcolor );
  bar( 327, (20*k) + 62, 418, 20*(k+1) + 61 );
  // Hien thi text trong thanh bar
  setcolor( lcolor );
  outtextxy( 335, 20*k + 68, ch[k] );
}

void Windows4(int k, char *ch[], int bcolor, int lcolor)
{
  // Tao hop sang cho 1 o
  setfillstyle( SOLID_FILL, bcolor );
  bar( 432, (20*k) + 62, 520, 20*(k+1) + 61 );
  // Hien thi text trong thanh bar
  setcolor( lcolor );
  outtextxy( 438, 20*k + 68, ch[k] );
}

void Windows5(int k, char *ch[], int bcolor, int lcolor)
{
  // Tao hop sang cho 1 o
  setfillstyle( SOLID_FILL, bcolor );
  bar( 534, (20*k) + 62, 618, 20*(k+1) + 61 );
  // Hien thi text trong thanh bar
  setcolor( lcolor );
  outtextxy( 542, 20*k + 68, ch[k] );
}

/*------------------------------------------------------*/
/*      Chon cac tieu de de thuc hien tren Main MENU    */
/*------------------------------------------------------*/

void Execult(int Select)
{
  int i;

  switch(Select)
  {
    case 0 :
      PopUp0();
      Main_Menu();
      break;

    case 1 :
      PopUp1();
      Main_Menu();
      break;

    case 2 :
      PopUp2();
      Main_Menu();
      break;

    case 3 :
      PopUp3();
      Main_Menu();
      break;

    case 4 :
      PopUp4();
      Main_Menu();
      break;

    case 5 :
      PopUp5();
      break;
  } // Of switch(Select)
  setgraphmode( getgraphmode() ); // Khoi phuc lai che do do hoa
} // Of Execult

/*-------------------------------------------------------*/
/*             THUC HIEN POPUP CHO VE DA GIAC            */
/*-------------------------------------------------------*/
void PopUp0()
{
   int  Option, i, Key;
   char *PopUp[]={
		   " TAM GIAC",  // Ve tam giac
		   "TAM GIAC 1", // Ung dung cua tam giac 1
		   "TAM GIAC 2", // Ung dung cua tam giac 2
		   "TAM GIAC 3", // Ung dung cua tam giac 3
		 };

   // Tao cac MENU con trong MENU Ve da giac
   Option = 0;
   do
   {
     setfillstyle( SOLID_FILL, LIGHTGRAY );
     bar( 9, 56, 115, 148 );
     setbkcolor( CBG );
     setcolor( BLUE );
     rectangle( 10, 60, 110, 144 );
     rectangle( 8, 58, 112, 146 );

     for( i = 0; i < 4; i++ )
     {
       settextstyle( 0, 0, 1 );
       outtextxy( 20, 20*i + 68, PopUp[i] );
     }

     Windows0( Option, PopUp, 15, 4 );
     Key = getch();

     if( !Key )
     {
       Key = getch();
       switch( Key )
       {
	   case UP :
	     Windows0( Option, PopUp, 8, 15 );
	     if( Option <= 0 ) Option = 3;
	     else Option-- ;
	     Windows0( Option, PopUp, 15, 4 );
	     break;

	   case DOWN :
	     Windows0( Option, PopUp, 8, 15 );
	     if( Option >= 3 ) Option = 0;
	     else Option++ ;
	     Windows0( Option, PopUp, 15, 4 );
	     break;

	   case HOME :
	     Windows0( Option, PopUp, 1, 15 );
	     Option = 0;
	     Windows0( Option, PopUp, 15, 4 );
	     break;

	   case END  :
	     Windows0( Option, PopUp, 1, 15 );
	     Option = 3;
	     Windows0( Option, PopUp, 15, 4 );
	     break;

	   case LEFT:
	     Windows( 0, ListMenu, 6, 11 );
	     Windows( 5, ListMenu, 15, 4 );
	     setfillstyle( EMPTY_FILL, BROWN );
	     bar( 9, 56, 115, 148 );
	     PopUp5();
	     break;

	   case RIGHT:
	     Windows( 0, ListMenu, 6, 11 );
	     Windows( 1, ListMenu, 15, 4 );
	     setfillstyle( EMPTY_FILL, BROWN );
	     bar( 9, 56, 115, 148 );
	     PopUp1();
	     break;
       } // Of switch(Key)
     } // Of if (!Key)
     if( Key == ENTER ) Execult0( Option );
     setfillstyle( EMPTY_FILL, CBG );
     bar( 9, 56, 115, 148 );
   } while( Key != ESC );
}

/*--------------------------------------------------------*/
/*             Ham thuc hien MENU con PopUp0()            */
/*--------------------------------------------------------*/

void Execult0(int Option)
{
  int i, x1, y1, x2, y2, x3, y3, color;

  switch( Option )
  {
    case 0 :
      setfillstyle( SOLID_FILL, LIGHTBLUE );
      bar( 15, 200, getmaxx() - 20, 392 );
      setfillstyle( SOLID_FILL, LIGHTCYAN );
      bar( 23, 208, getmaxx() - 28, 384 );
      setcolor( getmaxcolor() );
      rectangle( 17, 202, getmaxx() - 22, 390 );
      rectangle( 19, 204, getmaxx() - 24, 388 );
      setcolor( CBG );
      settextstyle( TRIPLEX_FONT, 0, 1 );
      outtextxy( 45, 220, "- Nhap 3 cap toa do x1, y1, x2, y2, x3, y3 :" );
      outtextxy( 46, 221, "- Nhap 3 cap toa do x1, y1, x2, y2, x3, y3 :" );
      gotoxy( 10, 17 );
      scanf( "%d", &x1 );
      gotoxy( 15, 17 );
      scanf( "%d", &y1 );
      gotoxy( 20, 17 );
      scanf( "%d", &x2 );
      gotoxy( 25, 17 );
      scanf( "%d", &y2 );
      gotoxy( 30, 17 );
      scanf( "%d", &x3 );
      gotoxy( 35, 17 );
      scanf( "%d", &y3 );
      outtextxy( 45, 290, "- Nhap mau cho hinh ve :" );
      outtextxy( 46, 290, "- Nhap mau cho hinh ve :" );
      gotoxy( 10, 22 );
      scanf( "%d", &color );

      // Xoa vung bar nhap lieu
      setfillstyle( EMPTY_FILL, CBG );
      bar( 8, 200, getmaxx() - 20, 402 );
      Screen_Result( 8, 39, getmaxx() - 12, getmaxy() - 54, 1 );
      setbkcolor( CBG );
      setcolor( getmaxcolor() );
      rectangle( 1, 1, getmaxx() - 1, getmaxy() - 1 );
      Triangle( x1, y1, x2, y2, x3, y3, color );
      setcolor( CYAN );
      getch();
      Clear_Screen_Result();
      Main_Menu();
      break;

    case 1 :
      cleardevice();
      Screen_Result( 8, 39, getmaxx() - 12, getmaxy() - 54, 1 );
      setbkcolor( CBG );
      Triangle1();
      getch();
      Clear_Screen_Result();
      Main_Menu();
      break;

   case 2 :
     cleardevice();
     Screen_Result( 8, 39, getmaxx() - 12, getmaxy() - 54, 1 );
     setbkcolor( CBG );
     Triangle2();
     getch();
     Clear_Screen_Result();
     Main_Menu();
     break;

   case 3 :
     cleardevice();
     Screen_Result( 8, 39, getmaxx() - 12, getmaxy() - 54, 1 );
     setbkcolor( CBG );
     Triangle3();
     getch();
     Clear_Screen_Result();
     Main_Menu();
     break;
  } // Of switch(Option)
  setgraphmode( getgraphmode() );
} // Of Execult0()

/*-------------------------------------------------------*/
/*             THUC HIEN POPUP CHO TINH TIEN             */
/*-------------------------------------------------------*/

void PopUp1()
{
   int  Option, i,Key;
   char *PopUp[]={
		   " TAM GIAC",  // Ve tam giac
		   "TAM GIAC 4", // Ung dung tam giac 4
		   "TAM GIAC 5", // Ung dung tam giac 5
		   "UNG DUNG 0", // Ung dung 0
		 };

   // Tao MENU con
   Option = 0;
   do
   {
     setfillstyle( SOLID_FILL, LIGHTGRAY );
     bar( 108, 56, 215, 148 );
     setbkcolor( CBG );
     setcolor( BLUE );
     rectangle( 112, 60, 210, 144 );
     rectangle( 110, 58, 212, 146 );

     for( i = 0; i < 4; i++ )
     {
       settextstyle( 0, 0, 1 );
       outtextxy( 122, 20*i + 68 , PopUp[i] );
     }

     Windows1( Option, PopUp, 15, 4 );
     Key = getch();
     if( !Key )
     {
       Key = getch();
       switch( Key )
       {
	   case UP :
	     Windows1( Option, PopUp, 8, 15 );
	     if( Option <= 0 ) Option= 3;
	     else Option--;
	     Windows1( Option, PopUp, 15, 4 );
	     break;

	   case DOWN :
	     Windows1( Option, PopUp, 8, 15 );
	     if( Option >= 3 ) Option = 0;
	     else Option++;
	     Windows1( Option, PopUp, 15, 4 );
	     break;

	   case HOME :
	     Windows1( Option, PopUp, 1, 15 );
	     Option = 0;
	     Windows1( Option, PopUp, 15, 4 );
	     break;

	   case END :
	     Windows1( Option, PopUp, 1, 15 );
	     Option = 3;
	     Windows1( Option, PopUp, 15, 4 );
	     break;

	   case LEFT:
	     Windows( 1, ListMenu, 6, 11 );
	     Windows( 0, ListMenu, 15, 4 );
	     setfillstyle( EMPTY_FILL, CBG );
	     bar( 108, 56, 215, 148 );
	     PopUp0();
	     break;

	   case RIGHT:
	     Windows( 1, ListMenu, 6, 11 );
	     Windows( 2, ListMenu, 15, 4 );
	     setfillstyle( EMPTY_FILL, CBG );
	     bar( 108, 56, 215, 148 );
	     PopUp2();
	     break;
       } // Of switch(Key)
     } // Of if (!Key)
     if( Key == ENTER ) Execult1( Option );
     setfillstyle( EMPTY_FILL, CBG );
     bar( 108, 56, 215, 148 );
   } while( Key != ESC );
}

/*-------------------------------------------------------*/
/*        Ham thuc hien MENU con PopUp1()  TINH TIEN     */
/*-------------------------------------------------------*/

void Execult1(int Option)
{
   int x1, y1, x2, y2, x3, y3, color, trx, trY;

   switch( Option )
   {
     case 0 :
       setfillstyle( SOLID_FILL, LIGHTBLUE );
       bar( 15, 200, getmaxx() - 20, 392 );
       setfillstyle( SOLID_FILL, LIGHTCYAN );
       bar( 23, 208, getmaxx() - 28, 384 );
       setcolor( getmaxcolor() );
       rectangle( 17, 202, getmaxx() - 22, 390 );
       rectangle( 19, 204, getmaxx() - 24, 388 );
       setcolor( CBG );
       settextstyle( TRIPLEX_FONT, 0, 1 );
       outtextxy( 45, 220,"- Nhap 3 cap toa do x1, y1, x2, y2, x3, y3 :" );
       outtextxy( 46, 221,"- Nhap 3 cap toa do x1, y1, x2, y2, x3, y3 :" );
       gotoxy( 10, 17 );
       scanf( "%d", &x1 );
       gotoxy( 15, 17 );
       scanf( "%d", &y1 );
       gotoxy( 20, 17 );
       scanf ("%d", &x2 );
       gotoxy( 25, 17 );
       scanf( "%d", &y2 );
       gotoxy( 30, 17 );
       scanf( "%d", &x3 );
       gotoxy( 35, 17 );
       scanf( "%d", &y3 );
       outtextxy( 45, 290, "- Nhap mau, trx, try cho hinh ve :" );
       outtextxy( 46, 290,"- Nhap mau, trx, try cho hinh ve :" );
       gotoxy( 10, 22 );
       scanf( "%d", &color );
       gotoxy( 15, 22 );
       scanf( "%d", &trx );
       gotoxy( 20, 22 );
       scanf( "%d", &trY );

       // Xoa vung nhap lieu
       setfillstyle( EMPTY_FILL, CBG );
       bar( 8, 200, getmaxx() - 20, 402 );
       cleardevice();
       Screen_Result( 8, 39, getmaxx() - 12, getmaxy() - 54, 1 );
       Triangle( x1, y1, x2, y2, x3, y3, color );
       getch();
       Triangle( x1, y1, x2, y2, x3, y3, BLACK );
       Translate( &x1, &y1, trx, trY );
       Translate( &x2, &y2, trx, trY );
       Translate( &x3, &y3, trx, trY );
       Triangle( x1, y1, x2, y2, x3, y3, color );
       getch();
       randomize();

       while( !kbhit() )
       {
	   setwritemode( XOR_PUT );
	   Translate( &x1, &y1, trx, trY );
	   Translate( &x2, &y2, trx, trY );
	   Translate( &x3, &y3, trx, trY );
	   Triangle( x1, y1, x2, y2, x3, y3, random( getmaxcolor() ) + 1 );
	   x1 += 2; x2 += 1; x3 += 1;
	   y1 += 1; y2 += 2; y3 += 3;
       }

       getch();
       setwritemode( OR_PUT );
       Clear_Screen_Result();
       Main_Menu();
       break;

     case 1 :
       cleardevice();
       Screen_Result( 8, 39, getmaxx() - 12, getmaxy() - 54,1 );
       setbkcolor( CBG );
       Triangle4();
       getch();
       Clear_Screen_Result();
       Main_Menu();
       break;

     case 2 :
       cleardevice();
       Screen_Result( 8, 39, getmaxx() - 12, getmaxy() - 54,1 );
       setbkcolor( CBG );
       Triangle5();
       getch();
       Clear_Screen_Result();
       Main_Menu();
       break;

     case 3:
       cleardevice();
       Screen_Result( 8, 39, getmaxx() - 12, getmaxy() - 54, 1 );
       setbkcolor( CBG );
       Apply0();
       getch();
       Clear_Screen_Result();
       Main_Menu();
       break;
   } // Of switch(Option)
   setgraphmode( getgraphmode() );
} // Of Execult1()

/*----------------------------------------------------*/
/*             THUC HIEN POPUP CHO XOAY               */
/*----------------------------------------------------*/

void PopUp2()
{
   char c, Pw[10];
   int  Option, i,Key;
   char *PopUp[]={
		   " TAM GIAC",  // Ve tam giac
		   "UNG DUNG 1", // Ung dung 1
		   "UNG DUNG 2", // Ung dung 2
		   "UNG DUNG 3", // Ung dung 3
		 };

   // Tao cac MENU con
   Option = 0;
   do
   {
     setfillstyle( SOLID_FILL, LIGHTGRAY );
     bar( 215, 56, 320, 148 );
     setbkcolor( CBG );
     setcolor( BLUE );
     rectangle( 218, 60, 315, 144 );
     rectangle( 216, 58, 317, 146 );

     for( i = 0; i < 4; i++ )
     {
       settextstyle( 0, 0, 1 );
       outtextxy( 228, 20*i + 68 , PopUp[i] );
     }

     Windows2( Option, PopUp, 15, 4 );
     Key = getch();

     if( !Key )
     {
       Key = getch();
       switch( Key )
       {
	   case UP :
	     Windows2( Option, PopUp, 8, 15 );
	     if( Option <= 0 ) Option = 3;
	     else Option--;
	     Windows2( Option, PopUp, 15, 4 );
	     break;

	   case DOWN :
	     Windows2( Option, PopUp, 8, 15 );
	     if( Option >= 3 ) Option = 0;
	     else Option++;
	     Windows2( Option, PopUp, 15, 4 );
	     break;

	   case HOME :
	     Windows2( Option, PopUp, 1, 15 );
	     Option = 0;
	     Windows2( Option, PopUp, 15, 4 );
	     break;

	   case END  :
	     Windows2( Option, PopUp, 1, 15 );
	     Option = 3;
	     Windows2( Option, PopUp, 15, 4 );
	     break;

	   case LEFT:
	     Windows( 2, ListMenu, 6, 11 );
	     Windows( 1, ListMenu, 15, 4 );
	     setfillstyle( EMPTY_FILL, CBG );
	     bar( 215, 56, 320, 148 );
	     PopUp1();
	     break;

	   case RIGHT:
	     Windows( 2, ListMenu, 6, 11 );
	     Windows( 3, ListMenu, 15, 4 );
	     setfillstyle( EMPTY_FILL, CBG );
	     bar( 215, 56, 320, 148 );
	     PopUp3();
	     break;
       } // Of switch(Key)
     } // Of if (!Key)
     if( Key == ENTER ) Execult2( Option );
     setfillstyle( EMPTY_FILL, CBG );
     bar( 215, 56, 320, 148 );
   } while( Key != ESC );
   cleardevice();
   Main_Menu();
} // Of PopUp2()

/*------------------------------------------------------------*/
/*             THUC HIEN POPUP2 CUA PHEP XOAY                 */
/*------------------------------------------------------------*/

void Execult2(int Option)
{
   int i, x1, y1, x2, y2, x3, y3, xc, yc, color;
   float angle;

   switch( Option )
   {
     case 0 :
       setfillstyle( SOLID_FILL,LIGHTBLUE );
       bar( 15, 200, getmaxx() - 20, 392 );
       setfillstyle( SOLID_FILL, LIGHTCYAN );
       bar( 23, 208, getmaxx() - 28, 384 );
       setcolor( getmaxcolor() );
       rectangle( 17, 202, getmaxx() - 22, 390 );
       rectangle( 19, 204, getmaxx() - 24, 388 );
       setcolor( CBG );
       settextstyle( TRIPLEX_FONT, 0, 1 );
       outtextxy( 45, 220, "- Nhap 3 cap toa do x1, y1, x2, y2, x3, y3 :" );
       outtextxy( 46, 221, "- Nhap 3 cap toa do x1, y1, x2, y2, x3, y3 :" );
       gotoxy( 10, 17 );
       scanf( "%d", &x1 );
       gotoxy( 15, 17 );
       scanf( "%d", &y1 );
       gotoxy( 20, 17 );
       scanf( "%d", &x2 );
       gotoxy( 25, 17 );
       scanf( "%d", &y2 );
       gotoxy( 30, 17 );
       scanf( "%d", &x3 );
       gotoxy( 35, 17 );
       scanf( "%d", &y3 );
       outtextxy( 45, 290, "- Nhap xc, yc, angle, mau cho hinh ve :" );
       outtextxy( 46, 290, "- Nhap xc, yc, angle, mau cho hinh ve :");
       gotoxy( 10, 22 );
       scanf( "%d",&xc );
       gotoxy( 15, 22 );
       scanf( "%d", &yc );
       gotoxy( 20, 22 );
       scanf( "%f", &angle );
       gotoxy( 25, 22 );
       scanf( "%d", &color );
       // Xoa vung nhap lieu
       setfillstyle( EMPTY_FILL, CBG );
       bar( 8, 200, getmaxx() - 20, 402 );
       // Xoa toan man hinh
       cleardevice();
       Screen_Result( 8, 39, getmaxx() - 12, getmaxy() - 54, 1 );
       Triangle( x1, y1, x2, y2, x3, y3, color );
       getch();
       for( i = 0; i <= angle; i++ )
       Rotate_Triangle( x1, y1, x2, y2, x3, y3, xc, yc, i, color );
       getch();
       Clear_Screen_Result();
       Main_Menu();
       break;

     case 1 :
       cleardevice();
       Screen_Result( 8, 39, getmaxx() - 12, getmaxy() - 54, 1 );
       setbkcolor( CBG );
       Apply1();
       getch();
       Clear_Screen_Result();
       Main_Menu();
       break;

     case 2 :
       cleardevice();
       Screen_Result( 8, 39, getmaxx() - 12, getmaxy() - 54, 1 );
       setbkcolor( CBG );
       Apply2();
       getch();
       Clear_Screen_Result();
       Main_Menu();
       break;

     case 3 :
       cleardevice();
       Screen_Result( 8, 39, getmaxx() - 12, getmaxy() - 54, 1 );
       setbkcolor( CBG );
       Apply3();
       getch();
       Clear_Screen_Result();
       Main_Menu();
       break;
   } // Of switch(Option)
   setgraphmode( getgraphmode() );
} // Of Execult2()

/*-----------------------------------------------------*/
/*            THUC HIEN POPUP3 CUA PHEP CO GIAN        */
/*-----------------------------------------------------*/

void PopUp3()
{
   char c, Pw[10];
   int  Option, i,Key;
   char *PopUp[]={
		   " TAM GIAC",  // Ve tam giac
		   "UNG DUNG 4", // Ung dung 4
		   "UNG DUNG 5", // Ung dung 5
		   "UNG DUNG 6", // Ung dung PR
		 };

   // Tao cac MENU con
   Option = 0;
   do
   {
     setfillstyle( SOLID_FILL, LIGHTGRAY );
     bar( 320, 56, 425, 148 );
     setbkcolor( CBG );
     setcolor( BLUE );
     rectangle( 325, 60, 420, 144 );
     rectangle( 323, 58, 422, 146 );

     for( i = 0; i < 4; i++ )
     {
       settextstyle( 0, 0, 1 );
       outtextxy( 335, 20*i + 68 , PopUp[i] );
     }

     Windows3( Option, PopUp, 15, 4 );
     Key = getch();

     if( !Key )
     {
       Key = getch();
       switch( Key )
       {
	   case UP :
	     Windows3( Option, PopUp, 8, 15 );
	     if( Option <= 0 ) Option= 3;
	     else Option--;
	     Windows3( Option, PopUp, 15, 4 );
	     break;

	   case DOWN :
	     Windows3( Option, PopUp, 8, 15 );
	     if( Option >= 3 ) Option = 0;
	     else Option++;
	     Windows3( Option, PopUp, 15, 4 );
	     break;

	   case HOME :
	     Windows3( Option, PopUp, 1, 15 );
	     Option = 0;
	     Windows3( Option, PopUp, 15, 4 );
	     break;

	   case END  :
	     Windows3( Option, PopUp, 1, 15 );
	     Option = 3;
	     Windows3( Option, PopUp, 15, 4 );
	     break;

	   case LEFT:
	     Windows( 3, ListMenu, 6, 11 );
	     Windows( 2, ListMenu, 15, 4 );
	     setfillstyle( EMPTY_FILL, CBG );
	     bar( 320, 56, 425, 148 );
	     PopUp2();
	     break;

	   case RIGHT:
	     Windows( 3, ListMenu, 6, 11 );
	     Windows( 4, ListMenu, 15, 4 );
	     setfillstyle( EMPTY_FILL, CBG );
	     bar( 320, 56, 425, 148 );
	     PopUp4();
	    break;
       } // Of switch(Key)
     } // Of if (!Key)
     if( Key == ENTER ) Execult3( Option );
     setfillstyle( EMPTY_FILL,CBG );
     bar( 320, 56, 425, 148 );
   } while( Key != ESC );
   cleardevice();
   Main_Menu();
} // Of PopUp3

/*----------------------------------------------------*/
/*         THUC HIEN POPUP3 CUA PHEP CO GIAN          */
/*----------------------------------------------------*/

void Execult3(int Option)
{
  int i, x1, y1, x2, y2, x3, y3, axis, color, u1 = 0,
      v1 = 0, u2 = 0, v2 = 0, u3 = 0, v3 = 0;
  float xoffset, yoffset, factor, angle;

  switch(Option)
  {
    case 0 :
      setfillstyle( SOLID_FILL,LIGHTBLUE );
      bar( 15, 200, getmaxx() - 20, 392 );
      setfillstyle( SOLID_FILL, LIGHTCYAN );
      bar( 23, 208, getmaxx() - 28, 384 );
      setcolor( getmaxcolor() );
      rectangle( 17, 202, getmaxx() - 22, 390 );
      rectangle( 19, 204, getmaxx() - 24, 388 );
      setcolor( CBG );
      settextstyle( TRIPLEX_FONT, 0, 1 );
      outtextxy( 45, 220, "- Nhap 3 cap toa do x1, y1, x2, y2, x3, y3 :" );
      outtextxy( 46, 221, "- Nhap 3 cap toa do x1, y1, x2, y2, x3, y3 :" );
      gotoxy( 10, 17 );
      scanf( "%d", &x1 );
      gotoxy( 15, 17 );
      scanf( "%d", &y1 );
      gotoxy( 20, 17 );
      scanf( "%d", &x2 );
      gotoxy( 25, 17 );
      scanf( "%d", &y2 );
      gotoxy( 30, 17 );
      scanf( "%d", &x3 );
      gotoxy( 35, 17 );
      scanf( "%d", &y3 );
      outtextxy( 45, 290, "- Nhap xoffset, yoffset, factor, axis, color cho hinh ve :" );
      outtextxy( 46, 290, "- Nhap xoffset, yoffset, factor, axis, color cho hinh ve :" );
      gotoxy( 10, 22 );
      scanf( "%f", &xoffset );
      gotoxy( 15, 22 );
      scanf( "%f", &yoffset );
      gotoxy( 20, 22 );
      scanf( "%f", &factor );
      gotoxy( 25, 22 );
      scanf( "%d", &axis );
      gotoxy( 30, 22 );
      scanf( "%d", &color );
      // Xoa vung nhap lieu
      setfillstyle( EMPTY_FILL, CBG );
      bar( 8, 200, getmaxx() - 20, 402 );
      // Xoa toan man hinh
      cleardevice();
      Screen_Result( 8, 39, getmaxx() - 12, getmaxy() - 54, 1 );
      Triangle( x1, y1, x2, y2, x3, y3, color );
      u1 = x1; v1 = y1; u2 = x2; v2 = y2; u3 = x3; v3 = y3;
      getch();
      setwritemode( XOR_PUT );
      Elastic_Triangle( x1, y1, x2, y2, x3, y3, xoffset, yoffset, factor, axis, color );
      getch();
      i = 0;
      randomize();
      while( !kbhit() )
      {
	  if( i == 100 )  { x1 = 0 ; y1 = 0 ; }
	  if( i == 20  )  { x1 = x2; y1 = y2; }
	  if( i == 70  )  { x1 = x3; y1 = y3; }
	  if( i == 50  )  { x2 = u2; y2 = v2; }
	  if( i == 150 )  { x1 = x2; y1 = y2; }
	  if( i == 200 )  { x1 = x3; y1 = y3; }
	  if( i == 250 )  { x1 = x3; y1 = y3; }
	  if( i == 300 )  { x1 = u1; y1 = v1; }
	  if( i == 60  )  { x2 = u2; y2 = v2; }
	  if( i == 90  )  { x3 = u3; y3 = v3; }
	  if( i == 80  )  { x2 = getmaxx() / 2; y2 = getmaxy() / 2; }
	  if( i == 110 )  { x3 = getmaxx() / 4; y3 = getmaxy() / 3; }
	  if( i == 85  )  { x3 = getmaxx() - 50; y3 = getmaxy() / 3 - 50; }

	  if( i == 310 )
	  {
	    i = 1;
	    x1 = random( getmaxx() ); y1 = random( getmaxy() );
	    x2 = random( getmaxx() ); y2 = random( getmaxy() );
	    x3 = random( getmaxx() ); y3 = random( getmaxy() );
	  }

	  setwritemode( OR_PUT );
	  Elastic_Triangle( x1, y1, x2, y2, x3, y3, xoffset, yoffset,
	  factor, axis, random( getmaxcolor() ) + 1 );
	  x1+=1; y1+=1; i+=1;

      } // Of while (!kbhit())

      Clear_Screen_Result();
      Main_Menu();
      break;

    case 1 :
      cleardevice();
      Screen_Result( 8, 39, getmaxx() - 12, getmaxy() - 54, 1 );
      setbkcolor( CBG );
      Apply4();
      getch();
      Clear_Screen_Result();
      Main_Menu();
      break;

    case 2 :
      cleardevice();
      Screen_Result( 8, 39, getmaxx() - 12, getmaxy() - 54, 1 );
      setbkcolor( CBG );
      Apply5();
      getch();
      Clear_Screen_Result();
      Main_Menu();
      break;

    case 3 :
      cleardevice();
      Screen_Result( 8, 39, getmaxx() - 12, getmaxy() - 54, 1 );
      setbkcolor( CBG );
      ApplyPrim();
      getch();
      Clear_Screen_Result();
      Main_Menu();
      break;
  } // Of switch(option)
  setgraphmode( getgraphmode() );
} // Of Execult3

/*--------------------------------------------------------*/
/*             THUC HIEN POPUP CHO VE DOI XUNG            */
/*--------------------------------------------------------*/

void PopUp4()
{
  char c, Pw[10];
  int  Option, i,Key;
  char *PopUp[]={
		  " TAM GIAC",  // Ve tam giac
		  "UNG DUNG 7", // Ung dung 7
		  "UNG DUNG 8", // Ung dung 8
		  "UNG DUNG 9", // Ung dung 9
		};

  // Tao cac MENU con
  Option = 0;
  do
  {
    setfillstyle( SOLID_FILL, LIGHTGRAY );
    bar( 425, 56, 527, 148 );
    setbkcolor( CBG );
    setcolor( BLUE );
    rectangle( 430, 60, 522, 144 );
    rectangle( 428, 58, 524, 146 );

    for( i = 0; i < 4; i++ )
    {
      settextstyle( 0, 0, 1 );
      outtextxy( 438, 20*i + 68, PopUp[i] );
    }

    Windows4( Option, PopUp, 15, 4 );
    Key = getch();
    if( !Key )
    {
      Key = getch();
      switch( Key )
      {
	  case UP :
	    Windows4( Option, PopUp, 8, 15 );
	    if( Option <= 0 ) Option = 3;
	    else Option--;
	    Windows4( Option, PopUp, 15, 4 );
	    break;

	  case DOWN :
	    Windows4( Option, PopUp, 8, 15 );
	    if( Option >= 3 ) Option = 0;
	    else Option++;
	    Windows4( Option, PopUp, 15, 4 );
	    break;

	  case HOME :
	    Windows4( Option, PopUp, 1, 15 );
	    Option = 0;
	    Windows4( Option, PopUp, 15, 4 );
	    break;

	  case END  :
	    Windows4( Option, PopUp, 1, 15 );
	    Option = 3;
	    Windows4( Option, PopUp, 15, 4 );
	    break;

	  case LEFT:
	    Windows( 4, ListMenu, 6, 11 );
	    Windows( 3, ListMenu, 15, 4 );
	    setfillstyle( EMPTY_FILL, CBG );
	    bar( 425, 56, 527, 148 );
	    PopUp3();
	    break;

	  case RIGHT:
	    Windows( 4, ListMenu, 6, 11 );
	    Windows( 5, ListMenu, 15, 4 );
	    setfillstyle( EMPTY_FILL, CBG );
	    bar( 425, 56, 527, 148 );
	    PopUp5();
	    break;
      }// Of switch(Key)
    } // Of if (!Key)
    if( Key == ENTER ) Execult4( Option );
    setfillstyle( EMPTY_FILL, CBG );
    bar( 425, 56, 527, 148 );
  } while( Key != ESC );
} // Of PopUp4

/*------------------------------------------------------------*/
/*           THUC HIEN POPUP4 TAM GIAC DOI XUNG               */
/*------------------------------------------------------------*/

void Execult4(int Option)
{
  int i, x1, y1, x2, y2, x3, y3, xc, yc, color;

  switch( Option )
  {
    case 0 :
      setfillstyle( SOLID_FILL, LIGHTBLUE );
      bar( 15, 200, getmaxx() - 20, 392 );
      setfillstyle( SOLID_FILL, LIGHTCYAN );
      bar( 23, 208, getmaxx() - 28, 384 );
      setcolor( getmaxcolor() );
      rectangle( 17, 202, getmaxx() - 22, 390 );
      rectangle( 19, 204, getmaxx() - 24, 388 );
      setcolor( CBG );
      settextstyle( TRIPLEX_FONT, 0, 1 );
      outtextxy( 45, 220, "- Nhap 3 cap toa do x1, y1, x2, y2, x3, y3 :" );
      outtextxy( 46, 221, "- Nhap 3 cap toa do x1, y1, x2, y2, x3, y3 :" );
      gotoxy( 10, 17 );
      scanf( "%d", &x1 );
      gotoxy( 15, 17 );
      scanf( "%d", &y1 );
      gotoxy( 20, 17 );
      scanf( "%d", &x2 );
      gotoxy( 25, 17 );
      scanf( "%d", &y2 );
      gotoxy( 30, 17 );
      scanf( "%d", &x3 );
      gotoxy( 35, 17 );
      scanf( "%d", &y3 );
      outtextxy( 45, 290, "- Nhap xc, yc, color cho hinh ve :" );
      outtextxy( 46, 290, "- Nhap xc, yc, color cho hinh ve :" );
      gotoxy( 10, 22 );
      scanf( "%d", &xc );
      gotoxy( 15, 22 );
      scanf( "%d", &yc );
      gotoxy( 20, 22 );
      scanf( "%d", &color );
      // Xoa vung nhap lieu
      setfillstyle( EMPTY_FILL, CBG );
      bar( 8, 200, getmaxx() - 20, 402 );
      // Xoa toan man hinh
      cleardevice();
      Screen_Result( 8, 39, getmaxx() - 12, getmaxy() - 54, 1 );
      setcolor( color );
      Triangle( x1, y1, x2, y2, x3, y3, color );
      getch();
      setcolor( color );
      Triangle( x1, y1, x2, y2, x3, y3, color );
      Symmetry_Triangle( x1, y1, x2, y2, x3, y3, xc, yc, color );
      getch();
      Clear_Screen_Result();
      Main_Menu();
      break;

    case 1 :
      cleardevice();
      Screen_Result( 8, 40, getmaxx() - 12, getmaxy() - 54, 1 );
      setbkcolor( CBG );
      Apply6();
      getch();
      Clear_Screen_Result();
      Main_Menu();
      break;

    case 2 :
      cleardevice();
      Screen_Result( 8, 39, getmaxx() - 12, getmaxy() - 54, 1 );
      setbkcolor( CBG );
      Apply7();
      getch();
      Clear_Screen_Result();
      Main_Menu();
      break;

    case 3 :
      cleardevice();
      Screen_Result( 8, 39, getmaxx() - 12, getmaxy() - 54, 1 );
      setbkcolor( CBG );
      Apply8();
      getch();
      Clear_Screen_Result();
      Main_Menu();
      break;
  } // Of switch(Option)
  setgraphmode( getgraphmode() ); // Khoi tao lai che do do hoa truoc do
} // Of Execult4()

/*--------------------------------------------------------*/
/*        THUC HIEN POPUP CHO THOAT CHUONG TRINH          */
/*--------------------------------------------------------*/

void PopUp5()
{
  char c, Pw[10];
  int  Option, i,Key;
  char *PopUp[]={
		   " GIUP DO",  // Giup do
		   "HUONG DAN", // Huong dan su dung chuong trinh
		   "BAN QUYEN", // Ban quyen chuong trinh
		   "  THOAT",   // Thoat khoi chuong trinh
		};

  // Tao cac MENU con
  Option = 0;
  do
  {
      setfillstyle( SOLID_FILL, LIGHTGRAY );
      bar( 527, 56, 625, 148 );
      setbkcolor( CBG );
      setcolor( BLUE );
      rectangle( 532, 60, 620, 144 );
      rectangle( 530, 58, 622, 146  );

      for( i = 0; i < 4; i++ )
      {
	settextstyle( 0, 0, 1 );
	outtextxy( 542, 20*i + 68, PopUp[i] );
      }

      Windows5( Option, PopUp, 15, 4 );
      Key= getch();
      if( !Key )
      {
	  Key = getch();
	  switch( Key )
	  {
	    case UP :
	      Windows5( Option, PopUp, 8, 15 );
	      if( Option <= 0 ) Option = 3;
	      else Option--;
	      Windows5( Option, PopUp, 15, 4 );
	      break;

	    case DOWN :
	      Windows5( Option, PopUp, 8, 15 );
	      if( Option >= 3 ) Option = 0;
	      else Option++;
	      Windows5( Option, PopUp, 15, 4 );
	      break;

	    case HOME :
	      Windows5( Option, PopUp, 1, 15 );
	      Option = 0;
	      Windows5( Option, PopUp, 15, 4 );
	      break;

	    case END  :
	      Windows5( Option, PopUp, 1, 15 );
	      Option = 3;
	      Windows5( Option, PopUp, 15, 4 );
	      break;

	    case LEFT:
	      Windows( 5, ListMenu, 6, 11 );
	      Windows( 4, ListMenu, 15, 4 );
	      setfillstyle( EMPTY_FILL, CBG );
	      bar( 527, 56, 625, 148 );
	      PopUp4();
	      break;

	    case RIGHT:
	      Windows( 5, ListMenu, 6, 11 );
	      Windows( 0, ListMenu, 15, 4 );
	      setfillstyle( EMPTY_FILL, CBG );
	      bar( 527, 56, 625, 148 );
	      PopUp0();
	      break;
	  } // Of switch(Key)
      } // Of if (!Key)
      if( Key == ENTER ) Execult5( Option );
      setfillstyle( EMPTY_FILL, CBG );
      bar( 527, 56, 625, 148 );
  } while( Key != ESC );
} // Of PopUp5

/*-------------------------------------------------------*/
/*            THUC HIEN POPUP5 THOAT CHUONG TRINH        */
/*-------------------------------------------------------*/

void Execult5(int Option)
{
  int i, x1, y1, x2, y2, x3, y3,
      xc, yc, color;

  switch( Option )
  {
    case 0 :
      Help();
      getch();
      Clear_Screen_Result();
      Main_Menu();
      break;

    case 1 :
      Direction();
      getch();
      Clear_Screen_Result();
      Main_Menu();
      break;

    case 2 :
      Copyringht();
      getch();
      Clear_Screen_Result();
      Main_Menu();
      break;

    default :
      say_goodbye();
      break;
  }
  setgraphmode( getgraphmode() ); // Khoi phuc lai che do do hoa truoc do
} // Of Execult5


/*-------------------------------------*/
/* Funtion : get_disk                  */
/* Mission : Getting the letter driver */
/* Expects : Nothing                   */
/* Returns : Letter driver of disk     */
/*-------------------------------------*/
char *get_disk()
{
   FILE *fptr;
   char *driver, tmp[31];

   driver = (char *)malloc(4*sizeof(char));
   if( (fptr = fopen("C:\\WINDOWS\\register.dat","rb")) == NULL ) {
      set_text_attrib(0x1F);
      set_border_color(47);
      set_cursor_size(0x2020);
      clrscr();
      write_VRAM(33,10,0x4F,"SYSTEM ERROR");
      write_VRAM(20,12,0x1F,"Cannot reading from file register.dat");
      write_VRAM(20,13,0x1F,"Please strike any key to exit program");
      getch();
      exit_program();
   }
   fgets(tmp,31,fptr);
   fgets(tmp,31,fptr);
   fgets(tmp,31,fptr);
   fgets(driver,4,fptr);
   fclose(fptr);
   return driver;
}

/*--------------------------------------------*/
/*       KHOI TAO HE THONG DO HOA	      */
/*--------------------------------------------*/
void start_graphics()
{
   char *path_to_driver;
   int graph_driver, graph_mode, error_code;

   path_to_driver = (char *)malloc(19*sizeof(char));
   //path_to_driver = get_disk();
   //path_to_driver[3] = '\0';
   //strcat(path_to_driver, "TOPICS\\DRIVERS");
   detectgraph( &graph_driver, &graph_mode );
   initgraph( &graph_driver, &graph_mode, "d:\\bc\\bgi");
   error_code = graphresult();
   if(error_code != grOk) {
      set_text_attrib(0x1F);
      set_border_color(0x0B);
      set_cursor_size(0x2020);
      clrscr();
      write_VRAM(27,9,0x1F,"Graphics system error !" );
      write_VRAM(18,11,0x1B, grapherrormsg(error_code));
      write_VRAM(18,12,0x1E,"Please press any key to exit program!");
      getch();
      exit_program();
   }
}

/*----------------------------------*/
/*   Ham kiem tra phan cung do hoa  */
/*----------------------------------*/
int huge DetectSVGA256()
{
   int Graph_Driver, Graph_Mode, Supper_Mode = 0;
   detectgraph( &Graph_Driver, &Graph_Mode );
   if( Graph_Driver == VGA )
      return Supper_Mode;
   else
      return grError;
}

/*--------------------------------------------------------*/
/*         PHAN TRINH BAY MAN HINH GIOI THIEU             */
/*--------------------------------------------------------*/
void explain()
{
  int ht, h, y, graph_mode = 2, error_code,
  graph_driver = installuserdriver( "SVGA256", DetectSVGA256 );
  char *path_to_driver;

  path_to_driver = (char *)malloc(19*sizeof(char));
  //path_to_driver = get_disk();
  //path_to_driver[3] = '\0';
  //strcat(path_to_driver, "TOPICS\\DRIVERS");
  initgraph(&graph_driver, &graph_mode, "d:\\bc\bgi");
  error_code = graphresult();
  if(error_code == grFileNotFound) {
     clrscr();
     set_border_color(0x0B);
     set_cursor_size(0x2020);
     write_VRAM(27,9,0x1F,"Graphics system error !");
     write_VRAM(18,11,0x1B,grapherrormsg(error_code));
     write_VRAM(10,12,0x1E,"You sure that the file SVGA256.BGI is  being current directory." );
     write_VRAM(10,13,0x1E,"If you have'nt this file please contact with author to Copy it." );
     write_VRAM(10,14,0x1E,"Thank you for using my program ! press any key to exit program.");
     getch();
     exit_program();
  }
  BackGround();
  ht = getmaxx();
  settextjustify( 1, 1 );
  settextstyle( 0, 0, 5 );
  h = textheight( "H" );
  y = 30;
  OutTextShade( ht / 2,y + 30, "UNIVERSITY OF", h / 5, RED, YELLOW , 1 );
  y += h + h;
  OutTextShade( ht / 2,y + 20, "TECHNOLOGY", h / 5, RED, YELLOW , 1 );
  settextstyle( 0, 0, 3 );
  y += 2 * h + 10;
  h = textheight( "H" );
  OutTextShade( ht / 2, y + 10, "THUC TAP CHUYEN NGHANH", h / 4, LIGHTGREEN, DARKGRAY, 1 );
  y += h;
  settextjustify( 0, 0 );
  settextstyle( SMALL_FONT, 0, 12 );
  h = textheight( "H" );
  y = y + 20;
  OutTextShade( 120, y + 50, "Copyright (c) 2001", h / 5, YELLOW, MAGENTA, 0 );
  settextjustify( 0, 0 );
  settextstyle( 0, 0, 3 );
  h = textheight( "H" );
  y += 2 * h;
  OutTextShade( 200, y + 80 , "By Student", h / 5, WHITE, DARKGRAY, 0 );
  y += 3 * h;
  settextstyle( 0, 0, 5 );
  OutTextShade( 20, y + 80, "NGUYEN NGOC VAN", h / 3, LIGHTRED, LIGHTCYAN, 1 );
  getch();
  closegraph();
}

/*----------------------------------------------------*/
/*     CAC THUAT TOAN GIAI CAC PHEP BIEN HINH 2D.     */
/*----------------------------------------------------*/

void Triangle(int x1, int y1, int x2, int y2, int x3, int y3, int color)
{
   setcolor( color );
   line( x1, y1, x2, y2 );
   line( x1, y1, x3, y3 );
   line( x2, y2, x3, y3 );
}

/*---------------------------*/
/*    HAM DOI DO RA RADIAN   */
/*---------------------------*/
float DegToRad(float angle)
{
   return( PI * angle / 180 );
}

/*---------------------------------------------*/
/*       HAM QUAY QUANH TOA DO (vx, vy)        */
/*---------------------------------------------*/
void Rotate2(int *x, int *y, int vx, int vy, float theta)
{
   int xtemp = *x;
   theta = DegToRad( theta );
   *x = (xtemp-vx) * cos(theta) - (*y-vy) * sin(theta) + vx;
   *y = (xtemp-vx) * sin(theta) + (*y-vy) * cos(theta) + vy;
}

/*--------------------------------------------------*/
/*            THUC HIEN QUAY TAM GIAC		    */
/*--------------------------------------------------*/
void Rotate_Triangle( int x1, int y1, int x2, int y2, int x3, int y3, int xc, int yc, float angle, int color)
{
  int nx1, ny1, nx2, ny2, nx3, ny3;

  setcolor( color );
  nx1 = x1;
  ny1 = y1;
  Rotate2( &nx1, &ny1, xc, yc, angle );
  nx2 = x2;
  ny2 = y2;
  Rotate2( &nx2, &ny2, xc, yc, angle );
  nx3 = x3;
  ny3 = y3;
  Rotate2( &nx3, &ny3, xc, yc, angle );
  line( nx1, ny1, nx2, ny2 );
  line( nx2, ny2, nx3, ny3 );
  line( nx1, ny1, nx3, ny3 );
}

/*------------------------------------------------------*/
/*     HAM TINH TIEN MOT DIEM MOT KHOANG (trx, try)     */
/*------------------------------------------------------*/
void Translate(int *x, int *y, int trx, int trY)
{
   *x += trx;
   *y += trY;
}

/*------------------------------------------------------*/
/*      HAM CO GIAN DOI TUONG QUANH TRUC X HOAC Y       */
/*------------------------------------------------------*/
void Stretch(int *x, int *y, float xoffset, float yoffset, float factor, int axis)
{
  if( axis == YAXIS )  // Co theo truc y
  {
     *x -= xoffset;
     *x *= factor;
     *x += xoffset;
  }
  else
  {
     *y -= yoffset;
     *y *= factor;
     *y += yoffset;
  }
}

/*---------------------------------------------------------------*/
/*           HAM CO GIAN TAM GIAC QUANH TRUC X HOAC Y  	     */
/*---------------------------------------------------------------*/
void Elastic_Triangle(int x1, int y1, int x2, int y2, int x3, int y3, float xoffset, float yoffset, float factor, int axis, int color)
{
  setcolor( color );
  Stretch( &x2, &y2, xoffset, yoffset, factor, axis );
  Stretch( &x3, &y3, xoffset, yoffset, factor, axis );
  Triangle( x1, y1, x2, y2, x3, y3, color );
}

/*---------------------------------------------------------*/
/*        VE TAM GIAC QUA DIEM DOI XUNG xc, yx             */
/*---------------------------------------------------------*/

void Symmetry_Triangle(int x1, int y1, int x2, int y2, int x3, int y3, int xc,  int yc, int color )
{

  int dx1, dy1, dx2, dy2, dx3, dy3,
      x11, y11, x22, y22, x33, y33;

  if( xc >= x1 )
  { dx1 = xc - x1; x11 = xc + dx1; }
  else { dx1=x1-xc; x11 = xc - dx1; }

  if( yc >= y1 )
  { dy1 = yc - y1; y11 = yc + dy1; }
  else { dy1 = y1 - yc; y11 = yc - dy1; }

  if( xc >= x2 )
  { dx2 = xc - x2; x22 = xc + dx2; }
  else { dx2 = x2 - xc; x22 = xc - dx2; }

  if( yc >= y2 )
  { dy2 = yc - y2; y22 = yc + dy2; }
  else { dy2 = y2 - yc; y22 = yc - dy2; }

  if( xc >= x3 )
  { dx3 = xc - x3; x33 = xc + dx3; }
  else { dx3 = x3 - xc; x33 = xc - dx3; }

  if( yc >= y3 )
  { dy3 = yc - y3; y33 = yc + dy3; }
  else { dy3 = y3 - yc; y33 = yc - dy3; }

  setlinestyle( 0, 2, 1 );
  setfillstyle( 3, color + 1 );
  circle( xc, yc, 1 );
  circle( xc, yc, 2 );
  Triangle( x11, y11, x22, y22, x33, y33, color );
}

/*-------------------------------------------------------*/
/*         KHOI TAO LAI TRANG THAI MAN HINH              */
/*-------------------------------------------------------*/
void Screen_Result(int x1, int y1, int x2, int y2, int clip)
{
   int i, u, xtam, ytam;

   cleardevice();

   // Trinh bay cac thanh bar
   setfillstyle( SOLID_FILL, CYAN );
   bar( 1, 1, getmaxx(), getmaxy() );
   setfillstyle( SOLID_FILL, LIGHTGRAY );
   bar( 2, 2, getmaxx() - 5, 31 );
   setfillstyle( CLOSE_DOT_FILL, WHITE );
   bar( 4, 4, 30, 5+30-6 );
   setfillstyle( CLOSE_DOT_FILL, WHITE );
   bar( getmaxx() - 8, 4, getmaxx() - 50, 29 );

   // Thanh bar cho cac phep bien hinh
   setfillstyle( SOLID_FILL, WHITE );
   bar( 34, 4, getmaxx() - 46, 29 );
   setfillstyle( SOLID_FILL, LIGHTGREEN );
   bar( 150, 4, getmaxx() - 156, 29 );
   setfillstyle( SOLID_FILL, LIGHTGRAY );
   bar( 6, getmaxy() - 43, getmaxx() - 10, getmaxy() - 10 );
   setfillstyle( SOLID_FILL, WHITE );
   bar( 9, getmaxy() - 41, getmaxx() - 13, getmaxy() - 13 );

   // Thanh bar cho phia duoi menu
   setfillstyle( SOLID_FILL, LIGHTCYAN );
   bar( 9, getmaxy() - 41, getmaxx() - 13, getmaxy() - 13 );

   xtam = 10;
   ytam = getmaxy() - 45;
   for( u = 0; u < 4; u++ )
   {
      settextstyle( TRIPLEX_FONT, 0, 4 );
      setcolor( LIGHTRED );
      outtextxy( xtam + 38, ytam - 4, "TRUONG DH KY THUAT CONG NGHE" );
      xtam += 1;
      ytam += 1;
   }

   xtam = 170;
   ytam = 2;
   for( u = 0; u < 5; u++ )
   {
      settextstyle( TRIPLEX_FONT, 0, 3 );
      setcolor( LIGHTMAGENTA );
      outtextxy( xtam, ytam - 3, "CAC PHEP BIEN HINH 2D" );
      ytam += 1;
      xtam -= 1;
   }

   // Mau nen
   setbkcolor( CBG );
   setcolor( getmaxcolor() );
   rectangle( 1, 1, getmaxx() - 4, getmaxy() - 2 );
   rectangle( 4, 35, getmaxx() - 8, getmaxy() - 50 );
   rectangle( 6, 37, getmaxx() - 10, getmaxy() - 52 );
   rectangle( 4, getmaxy() - 45, getmaxx() - 8,getmaxy() - 7 );

   // Vung lam viec
   setfillstyle( SOLID_FILL, 1 );
   bar( 8, 39, getmaxx() - 12, getmaxy() - 54 );
   setviewport( x1, y1, x2, y2, clip );
}

/*-------------------------------------*/
/*      XOA MAN HINH DA HIEN THI       */
/*-------------------------------------*/
void Clear_Screen_Result()
{
  clearviewport();
  cleardevice();
  setviewport( 0, 0, getmaxx(), getmaxy(), 0 );
}

/*------------------------------------*/
/*        HAM VE MOT DA GIAC          */
/*------------------------------------*/
void Draw_Triangle()
{
   int x1, y1, x2, y2, x3, y3, color, trx, trY;

   setviewport( 0, getmaxy() / 3, getmaxx() - 50, getmaxy() - 1, 0 );
   setcolor( LIGHTGRAY );
   rectangle( 30,40, getmaxx() - 50, 200 );
   outtextxy( 30, 58,"- Nhap 3 cap toa do x1, y1, x2, y2, x3, y3:" );
   gotoxy( 10, 16 );
   scanf( "%d", &x1 );
   gotoxy( 15, 16 );
   scanf( "%d", &y1 );
   gotoxy( 20, 16 );
   scanf( "%d", &x2 );
   gotoxy( 25, 16 );
   scanf( "%d", &y2 );
   gotoxy( 30, 16 );
   scanf( "%d", &x3 );
   gotoxy( 35, 16 );
   scanf( "%d", &y3 );
   outtextxy( 30, 100, "- Nhap tri cho trX, trY, Color :" );
   gotoxy( 10, 20 );
   scanf( "%d", &trx );
   gotoxy( 15, 20 );
   scanf( "%d", &trY );
   gotoxy(20,20);
   scanf("%d",&color);
   clearviewport();
   cleardevice();
   setbkcolor( BLACK );
   setcolor( WHITE );
   setviewport( 0, 0, getmaxx(), getmaxy(), 0 );
   rectangle( 1, 1, getmaxx() - 1, getmaxy() - 1 );
   highvideo();
   Triangle( x1, y1, x2, y2, x3, y3, color );
   outtextxy( 100, getmaxy() - 20, "Press Any Key ..." );
   getch();
   Translate( &x1, &y1, trx, trY );
   Translate( &x2, &y2, trx, trY);
   Translate( &x3, &y3, trx, trY);
   setwritemode( OR_PUT );
   Triangle( x1, y1, x2, y2, x3, y3, color );
   getch();
   clearviewport();
}

/*---------------------------------------------------*/
/*        CAC UNG DUNG CUA CHUONG TRINH              */
/*---------------------------------------------------*/

void Triangle1()
{
  float cd=10, angle, dist, incr;
  int i, x, y, num;

  CD = 1000;
  CP.x = 310;
  CP.y = 250 - 50;
  dist = 1;
  angle = 140;
  incr = 1;
  num = 700;
  setcolor( LIGHTCYAN );
  MoveTo( CP );
  Polyspiral( dist, angle, incr, num );
  angle = 300;
  dist = 100;
  randomize();
  moveto( getmaxx( ) / 2, getmaxy() / 2 - 50 );
  x = getmaxx() / 2;
  y = getmaxy() / 2 - 50;
  while( !kbhit() )
  {
     setcolor( random( getmaxcolor() ) + 1 );
     lineforward( dist, cd );
     right( angle, &cd );
     lineforward( dist, cd );
     lineto( x, y );
  }
}

void Triangle2()
{
  float cd=10, angle, dist, incr;
  int i, x, y, num;

  CD = 1000;
  CP.x = 330;
  CP.y = 250 - 50;
  dist = 1;
  angle = 140;
  incr = 1;
  num = 700;
  setcolor( LIGHTRED );
  MoveTo( CP );
  Polyspiral( dist, angle, incr, num );

  CP.x = 330;
  CP.y = 250 - 50;
  dist = 1;
  angle = 140;
  incr = 1;
  num = 670;
  setcolor( YELLOW );
  MoveTo( CP );
  Polyspiral( dist, angle, incr, num );
  angle = 100;
  dist = 150;
  moveto( getmaxx() / 2, getmaxy() / 2 + 10 );
  x = getmaxx() / 2;
  y = getmaxy() / 2 - 50;
  randomize();
  while( !kbhit() )
  {
     setcolor( random( getmaxcolor() ) + 1 );
     lineforward( dist, cd );
     right( angle, &cd );
     lineforward( dist, cd );
     lineto( x, y );
  }
}

void Triangle3()
{
  float cd, angle, dist, incr;
  int i, x, y, num;

  CD = 1000;
  CP.x = 330;
  CP.y = 250 - 50;
  dist = 1;
  angle = 140;
  incr = 1;
  num = 700;
  setcolor( YELLOW );
  MoveTo( CP );
  Polyspiral( dist, angle, incr, num );
  cd = 100;
  angle = 300;
  dist = 100;
  x = getmaxx() / 2;
  y = getmaxy() / 2 - 50;
  moveto( getmaxx() / 2, getmaxy() / 2 - 50 );
  randomize();
  while( !kbhit() )
  {
     setcolor( LIGHTBLUE );
     lineforward( dist, cd );
     right( angle, &cd );
     lineforward( dist, cd );
     lineto( x, y );
     setcolor( LIGHTGREEN );
     lineforward( dist - 5, cd );
     right( angle, &cd );
     lineforward( dist - 5, cd );
     lineto( x, y );
     setcolor( LIGHTMAGENTA );
     lineforward( dist - 10, cd );
     right( angle - 50, &cd );
     lineforward( dist - 10, cd );
     lineto( x, y );
     setcolor( LIGHTCYAN );
     lineforward( dist - 20, cd );
     right( angle - 150, &cd );
     lineforward( dist - 20, cd );
     lineto( x, y );
     setcolor( LIGHTRED );
     lineforward( dist - 40, cd );
     right( angle - 170, &cd );
     lineforward( dist - 40, cd );
     lineto( x, y );
  }
}

void Tempo1()
{
  float cd = 10, angle = 100, dist = 150;
  int i = 0, x, y;

  moveto( getmaxx() / 2, getmaxy() / 2 - 50 );
  i = 0;
  x = getmaxx() / 2;
  y = getmaxy() / 2 - 50;
  randomize();
  while( i <= 400 )
  {
     setcolor( random( getmaxcolor() ) + 1 );
     lineforward( dist, cd );
     right( angle, &cd );
     lineforward( dist, cd );
     lineto( x, y );
     i++;
  }
}

void Tempo2()
{
  float cd = 100, angle = 300, dist = 80;
  int i, x, y;

  x = getmaxx() / 2;
  y = getmaxy() / 2 - 50;
  moveto( getmaxx() / 2, getmaxy() / 2 - 50 );
  randomize();
  while( !kbhit() )
  {
     setcolor(LIGHTBLUE);
     lineforward( dist, cd );
     right( angle, &cd);
     lineforward( dist, cd );
     lineto( x, y );
     setcolor( YELLOW );
     lineforward( dist - 5, cd );
     right( angle, &cd );
     lineforward( dist - 5, cd );
     lineto( x, y );
     setcolor( LIGHTMAGENTA );
     lineforward( dist - 10, cd );
     right( angle - 50, &cd );
     lineforward( dist - 10, cd );
     lineto( x, y );
     setcolor( LIGHTCYAN );
     lineforward( dist - 20, cd );
     right( angle - 150, &cd );
     lineforward( dist - 20, cd );
     lineto( x, y );
     setcolor( LIGHTRED );
     lineforward( dist - 40, cd );
     right( angle - 170, &cd );
     lineforward( dist - 40, cd );
     lineto( x, y );
  }
}

void Triangle4()
{
  float cd = 100, angle = 200, dist = 170;
  int i, x, y;

  x = getmaxx() / 2;
  y = getmaxy() / 2 - 50;
  moveto( getmaxx() / 2, getmaxy() / 2 - 50 );
  randomize();
  while( !kbhit() )
  {
     setcolor( random( getmaxcolor() ) + 1 );
     lineforward( dist, cd );
     right( angle, &cd );
     lineforward( dist, cd );
     lineto( x, y );
     setcolor( LIGHTBLUE );
     lineforward( dist - 5, cd );
     right( angle, &cd );
     lineforward( dist - 5, cd );
     lineto( x, y );
     setcolor( YELLOW );
     lineforward( dist - 10, cd );
     right( angle - 50, &cd );
     lineforward( dist - 10, cd );
     lineto( x, y );
     setcolor( RED );
     lineforward( dist - 20, cd );
     right( angle - 150, &cd );
     lineforward( dist - 20, cd );
     lineto( x, y );
     setcolor( LIGHTMAGENTA );
     lineforward( dist - 40, cd );
     right( angle - 170, &cd );
     lineforward( dist - 40, cd );
     lineto( x, y );
     setcolor( LIGHTCYAN );
     lineforward( dist - 70, cd );
     right( angle - 170, &cd );
     lineforward( dist - 70, cd );
     lineto( x, y );
  }
}

void Triangle5()
{
  float dist, angle, incr;
  int num;

  CD = 1000;
  CP.x = 300;
  CP.y = 250 - 50;
  dist = 1;
  angle = 140;
  incr = 1;
  num = 700;
  setcolor( LIGHTMAGENTA );
  MoveTo( CP );
  Polyspiral( dist, angle, incr, num );
  Tempo1();
  Tempo2();
}

void right(float angle, float *cd)
{
  *cd -= angle;
}

void lineforward(float dist, float cd)
{
   float angle;
   point p;

   angle = 6.2845 * cd / 360;
   p.x = getx() + dist * cos(angle);
   p.y = gety() + dist * sin(angle);
   lineto( p.x, p.y );
   moveto( p.x, p.y );
}

void MoveTo(point p)
{
   CP = p;  // Change CP to p
}

void Line(point p,point q)
{
   line( p.x, p.y, q.x, q.y );
}

void LineTo(point p)    // Draw from CP to p and update CP
{
   Line( CP, p );       // Draw clip line
   CP = p;              // Update CP
}

void Right(float angle)
{
  CD -= angle;
}

void LineForward(float dist)
{
   float angle;
   point p;

   angle = TWOPI * CD / 360; // Convert degrees to radians
   p.x = CP.x + dist * cos(angle);
   p.y = CP.y + dist * sin(angle);
   LineTo( p );
   CP = p;
}

// Draw num segments of a spiral with given exterior angle
void Polyspiral(float dist, float angle, float incr, int num)
{
  int i;

  for( i = 0; i < num; i++ )
  {
    LineForward( dist );
    Right( angle );
    dist += incr;
  }
}

/*---------------------------------------------*/
/*             VE MOT DA GIAC		       */
/*  Place vertices of an n-gon in poly	       */
/*---------------------------------------------*/
void MakeNGon(float Radius, int N, polypoint *poly, point center, float start_angle)
{
  int i;
  float del_ang, angle;

  if( (N < 3) || (N > MAXNUM) ) printf("Error, can not draw any triangle!");
  else
  {
     del_ang = TWOPI / N;
     angle = 0;
     poly->num = N;

     for( i = 1; i <= (poly->num); i++ )
     {
	 angle = (i-1) * del_ang + start_angle;
	 poly->pt[i].x = Radius * cos(angle) + center.x;
	 poly->pt[i].y = Radius * sin(angle) + center.y;
     } // End for
 }// End else
}

// Draw prime rosette of N sides, where N is prime
void PrimeRosette(float radius, int N, point center, float start_angle)
{
  int incr, j, index;
  polypoint poly;
  struct linesettingstype kieucu;
  MakeNGon( radius, N, &poly, center, start_angle );
  index = 1;
  getlinesettings( &kieucu );
  setlinestyle( SOLID_LINE, 0,THICK_WIDTH );
  randomize();
  MoveTo( poly.pt[index] );
  for( incr = 0; (int)floor(incr < (N - 1) / 2); incr++ )
    for( j = 0; j < N; j++ )  // Draw n edges
    {
      setcolor( YELLOW );
      index = (index - 1 + incr ) % N + 1;
      LineTo( poly.pt[index] );
    }
  setlinestyle( kieucu.linestyle, kieucu.upattern, kieucu.thickness );
}

/*---------------------------------------*/
/*       CAC UNG DUNG CHUONG TRINH       */
/*---------------------------------------*/
// Ve hinh bao quanh khung ben ngoai
void Apply0()
{
  float dist, angle, incr;
  int num;

  CD = 1000;
  CP.x = 300;
  CP.y = 200;
  dist = 1;
  angle = 140;
  incr = 1;
  num = 500;
  setcolor( LIGHTCYAN );
  MoveTo( CP );
  Polyspiral( dist, angle, incr, num );

  CD = 1000;
  CP.x = 300;
  CP.y = 200;
  dist = 1;
  angle = 140;
  incr = 1;
  num = 400;
  setcolor( LIGHTRED );
  MoveTo( CP );
  Polyspiral( dist, angle, incr, num );

  CD = 1000;
  CP.x = 300;
  CP.y = 200;
  dist = 1;
  angle = 140;
  incr = 1;
  num = 200;
  setcolor( LIGHTGREEN );
  MoveTo( CP );
  Polyspiral( dist, angle, incr, num );

  CD = 1000;
  CP.x = 300;
  CP.y = 200;
  dist = 1;
  angle = 140;
  incr = 1;
  num = 100;
  setcolor( LIGHTMAGENTA );
  MoveTo( CP );
  Polyspiral( dist, angle, incr, num );

}

// Ung dung 1 : Ve hinh vien co mau thay doi
void Apply1()
{
  int i;
  float dist, angle, incr;
  int num = 400;

  CD = 10;
  dist = 1;
  angle = 140;
  incr = 1;
  i = 0;
  while( !kbhit() )
  {
    if( i == 200 ) break;
    setcolor( random( getmaxcolor() ) + 1 );
    CP.x = getmaxx() / 2;
    CP.y = getmaxy() / 2 - 50;
    MoveTo(CP);
    Polyspiral( dist, angle, incr, num );
    i++;
  }
}

// Ung dung 2 : Ve hinh chu nhat co mau thay doi
void Apply2()
{
  int i, num;
  float angle, dist, incr;

  CD = 1000;
  i = 0;
  angle = 90;
  incr = 1;
  dist = 1;
  num = 350;

  while( !kbhit() )
  {
    if( i <= 300 )
    {
       setcolor( random( getmaxcolor() ) + 1 );
       CP.x = getmaxx() / 2;
       CP.y = getmaxy() / 2 - 50;
       MoveTo( CP );
       Polyspiral( dist, angle, incr, num );
       i++;
    }
    else {
      cleardevice();
      i = 1;
    }
  }
}

// Ung dung 3 : Ve hinh vien voi mau ngau nhien
void Apply3()
{
  float dist,angle,incr;
  int num;

  incr = 1;
  dist = 1;
  CD = 1000;
  setcolor( LIGHTRED );
  CP.x = 300;
  CP.y = 200;
  MoveTo( CP );
  num = 500;
  angle = 130;
  Polyspiral( dist, angle, incr, num );

  setcolor( LIGHTMAGENTA );
  CP.x = 300;
  CP.y = 200;
  angle = 140;
  num = 400;
  MoveTo( CP );
  Polyspiral( dist, angle, incr, num );

  setcolor( LIGHTGREEN );
  CP.x = 300;
  CP.y = 200;
  angle = 140;
  num = 340;
  MoveTo( CP );
  Polyspiral( dist, angle, incr, num );

  setcolor( LIGHTCYAN );
  CP.x = 300;
  CP.y = 200;
  MoveTo( CP );
  num = 250;
  angle = 140;
  Polyspiral( dist, angle, incr, num );

  setcolor( LIGHTRED );
  CP.x = 300;
  CP.y = 200;
  MoveTo( CP );
  num = 200;
  angle = 130;
  Polyspiral( dist, angle, incr, num );

  setcolor( LIGHTGREEN );
  CP.x = 300;
  CP.y = 200;
  MoveTo( CP );
  num = 100;
  angle = 100;
  Polyspiral( dist, angle, incr, num );
}

// Ung dung 4 : Ve hinh vien co dau nhon
void Apply4()
{
  float dist,angle,incr;
  int num;

  CD = 1000;
  CP.x =300;
  CP.y = 250 - 50;
  dist = 1;
  angle = 171;
  incr = 1;
  num = 700;
  setcolor( LIGHTRED );
  MoveTo( CP );
  Polyspiral( dist, angle, incr, num );

  CP.x = 300;
  CP.y = 250 - 50;
  dist = 1;
  angle = 170;
  incr = 1;
  num = 600;
  setcolor( LIGHTBLUE );
  MoveTo( CP );
  Polyspiral( dist, angle, incr, num );

  CP.x = 300;
  CP.y = 250 - 50;
  dist = 1;
  angle = 170;
  incr = 1;
  num = 530;
  setcolor( LIGHTCYAN );
  MoveTo( CP );
  Polyspiral( dist, angle, incr, num );

  CP.x = 300;
  CP.y = 250 - 50;
  dist = 1;
  angle = 170;
  incr = 1;
  num = 500;
  setcolor( LIGHTBLUE );
  MoveTo( CP );
  Polyspiral( dist, angle, incr, num );

  setcolor( LIGHTGRAY );
  CP.x = 300;
  CP.y = 250 - 50;
  MoveTo( CP );
  num = 300;
  angle = 170;
  Polyspiral( dist, angle, incr, num );

  setcolor( LIGHTRED );
  CP.x = 300;
  CP.y = 250 - 50;
  MoveTo( CP );
  num = 200;
  angle = 170;
  Polyspiral( dist, angle, incr, num );

  setcolor( LIGHTGREEN );
  CP.x = 300;
  CP.y = 250 - 50;
  MoveTo( CP );
  num = 100;
  angle = 100;
  Polyspiral( dist, angle, incr, num );
}

// Ung dung 5 : Ve hinh ngoi sao 5 canh
void Apply5()
{
  float dist,angle,incr;
  int num;

  CD = 1000;
  CP.x = 300;
  CP.y = 250 - 50;
  dist = 1;
  angle = 144;
  incr = 1;
  num = 700;
  setcolor( LIGHTRED );
  MoveTo( CP );
  Polyspiral( dist, angle, incr, num );

  CP.x = 300;
  CP.y = 250 - 50;
  dist = 1;
  angle = 144;
  incr = 1;
  num = 500;
  setcolor( LIGHTBLUE );
  MoveTo( CP );
  Polyspiral( dist, angle, incr, num );

  setcolor( LIGHTCYAN );
  CP.x = 300;
  CP.y = 250 - 50;
  MoveTo( CP );
  num = 350;
  angle = 144;
  Polyspiral( dist, angle, incr, num );

  setcolor( LIGHTRED );
  CP.x = 300;
  CP.y = 250 - 50;
  MoveTo( CP );
  num = 300;
  angle = 144;
  Polyspiral( dist, angle, incr, num );

  setcolor( LIGHTGREEN );
  CP.x = 300;
  CP.y = 250 - 50;
  MoveTo( CP );
  num = 100;
  angle = 100;
  Polyspiral( dist, angle, incr, num );
}

// Ung dung tam thoi de phuc vu cho ung dung 6
void ApplyTempo5()
{
  float dist,angle,incr;
  int num;

  CD = 1000;
  CP.x = 300;
  CP.y = 250 - 50;
  dist = 1;
  angle = 150;
  incr = 1;
  num = 300;
  setcolor( LIGHTBLUE );
  MoveTo( CP );
  Polyspiral( dist, angle, incr, num );

  setcolor( LIGHTCYAN );
  CP.x = 300;
  CP.y = 250 - 50;
  MoveTo( CP );
  num = 250;
  angle = 144;
  Polyspiral( dist, angle, incr, num );

  setcolor( LIGHTRED );
  CP.x = 300;
  CP.y = 250 - 50;
  MoveTo( CP );
  num = 200;
  angle = 144;
  Polyspiral( dist, angle, incr, num );

  setcolor( LIGHTGREEN );
  CP.x = 300;
  CP.y = 250 - 50;
  MoveTo( CP );
  num = 100;
  angle = 100;
  Polyspiral( dist, angle, incr, num );
}

// Ung dung Prim
void ApplyPrim()
{
  int N = 30;
  float radius = 200, start_angle = 90;

  point center;
  float dist, angle, incr;
  int num;

  CD = 1000;
  CP.x = 300;
  CP.y = 250 - 50;
  dist = 1;
  angle = 171;
  incr = 1;
  num = 800;
  setcolor( LIGHTRED );
  MoveTo( CP );
  Polyspiral( dist, angle, incr, num );

  center.x = 300;
  center.y = 250 - 50;
  CD = 1000;
  CP.x = 300;
  CP.y = 200;
  dist = 1;
  angle = 140;
  incr = 1;
  num = 200;
  setcolor( LIGHTRED );
  MoveTo( CP );
  Polyspiral( dist, angle, incr, num );

  setwritemode( XOR_PUT );
  PrimeRosette( radius, N, center, start_angle );
  setwritemode( OR_PUT );

}

// Ung dung 6 : Ve mat trong dong chinh giua co ngoi sao
void Apply6()
{
  ApplyPrim();
  delay(1000);
  ApplyTempo5();

}

// Ung dung 7 : Ve banh xe chuyen dong co mau thay doi ngau nhien
void Apply7()
{

  float cd = 100, angle = 100, dist = 150;
  int i;
  int x,y;
  x = getmaxx() / 2 - 10;
  y = getmaxy() / 2 - 50;
  moveto( getmaxx() / 2 - 10, getmaxy() / 2 - 50 );
  randomize();
  while( !kbhit() )
  {
     setcolor( LIGHTBLUE );
     lineforward( dist, cd );
     right( angle, &cd );
     lineforward( dist, cd );
     lineto( x, y );

     setcolor( LIGHTGRAY );
     lineforward( dist - 5, cd );
     right( angle, &cd );
     lineforward( dist - 5, cd );
     lineto( x, y );

     setcolor( YELLOW );
     lineforward( dist - 10, cd );
     right(angle - 50, &cd );
     lineforward( dist - 10, cd );
     lineto( x, y );

     setcolor( LIGHTCYAN );
     lineforward( dist - 20, cd );
     right( angle - 150, &cd );
     lineforward( dist - 20, cd );
     lineto( x, y );

     setcolor( LIGHTRED );
     lineforward( dist - 40, cd );
     right( angle - 170, &cd );
     lineforward( dist - 40, cd );
     lineto( x, y );
  }
}

// Ung dung 8 : Ve hinh chong chong
void Apply8()
{
   int i;
   float dist, angle, incr;
   int num;

   incr = 1;
   dist = 1;
   CD = 1000;
   setcolor( LIGHTBLUE );
   CP.x = 300;
   CP.y = 200;
   MoveTo( CP );
   num = 700;
   angle = 130;
   Polyspiral( dist, angle, incr, num );

   setcolor( LIGHTBLUE );
   CP.x = 300;
   CP.y = 200;
   MoveTo( CP );
   num = 690;
   angle = 130;
   Polyspiral( dist, angle, incr, num );

   setcolor( LIGHTRED );
   CP.x = 300;
   CP.y = 200;
   MoveTo( CP );
   num = 600;
   angle = 130;
   Polyspiral( dist, angle, incr, num);

   setcolor( YELLOW );
   CP.x = 300;
   CP.y = 200;
   MoveTo( CP );
   num = 570;
   angle = 130;
   Polyspiral( dist, angle, incr, num );
   angle = 360;
   while( !kbhit() )
   {
     setwritemode( XOR_PUT );
     for( i = 0; i <= angle; i++ )
     Rotate_Triangle( 100, 200, 350, 250, 400, 340, 300, 200, i, 9 );
     for( i = 0; i <= angle; i++ )
     Rotate_Triangle( 100, 200, 350, 250, 400, 340, 300, 200, i, 4 );
     setwritemode( OR_PUT );
     for( i = angle; i >= 0; i-- )
     Rotate_Triangle( 80, 200, 330, 250, 380, 340, 300, 200, i, 10 );
   }
   setwritemode( OR_PUT );
}

/*----------------------------------------------*/
/*  CAC MODULAR HUONG DAN SU DUNG CHUONG TRINH  */
/*----------------------------------------------*/
void Help()
{
   cleardevice();
   setcolor( WHITE );
   rectangle( 1, 1, getmaxx() - 1, getmaxy() - 1 );
   rectangle( 4, 4, getmaxx() - 4, getmaxy() - 4 );
   line( 5, 28, getmaxx() - 5, 28 );
   line( 5, 450, getmaxx() - 5, 450 );
   setbkcolor( BLUE );
   settextstyle( DEFAULT_FONT, HORIZ_DIR, 2 );
   setcolor( LIGHTCYAN );
   outtextxy( 80, 50, "Rat cam on ban da chon muc nay" );
   settextstyle( TRIPLEX_FONT, HORIZ_DIR, 1 );

   setcolor( YELLOW );
   outtextxy( 40, 100, "Chuong trinh gom co 5 menu chinh nhu sau:" );
   outtextxy( 40, 150, "- VE DA GIAC :" );
   outtextxy( 40, 200, "- TINH TIEN :" );
   outtextxy( 40, 250, "- XOAY :" );
   outtextxy( 40, 300, "- CO GIAN :" );
   outtextxy( 40, 350, "- DOI XUNG :" );
   outtextxy( 40, 400, "- THOAT :" );

   setcolor( WHITE );
   outtextxy( 190, 150, "Cac chuc nang lien quan den ve da giac." );
   outtextxy( 180, 200, "Cac chuc nang lien quan den phep tinh tien." );
   outtextxy( 130, 250, "Cac chuc nang lien quan den phep xoay." );
   outtextxy( 160, 300, "Cac chuc nang lien quan den phep co gian." );
   outtextxy( 170, 350, "Cac chuc nang lien quan den phep doi xung." );
   outtextxy( 150, 400, "Cac chuc nang huong dan su dung chuong trinh." );

   settextstyle( DEFAULT_FONT, HORIZ_DIR, 2 );
   setcolor( LIGHTGREEN );
   outtextxy( 270, 10, "GIUP DO" );
   outtextxy( 150, 455, "Press any key to exit" );
}

/*-----------------------------------------*/
/*	  PHAN HUONG DAN CHUONG TRINH	   */
/*-----------------------------------------*/
void Direction()
{
   cleardevice();
   setcolor( WHITE );
   rectangle( 1, 1, getmaxx() - 1, getmaxy() - 1 );
   rectangle( 4, 4, getmaxx() - 4, getmaxy() - 4 );
   line( 5, 28, getmaxx() - 5, 28 );
   line( 5, 450, getmaxx() - 5, 450 );
   setbkcolor( BLUE );
   settextstyle( DEFAULT_FONT, HORIZ_DIR, 2 );
   setcolor( LIGHTCYAN );
   outtextxy( 80, 50, "Rat cam on ban da chon muc nay" );
   settextstyle( TRIPLEX_FONT, HORIZ_DIR, 1 );
   setcolor( YELLOW );
   outtextxy( 50, 90, "- Viec lua chon cac Menu Bar ban chi can dung cac phim" );
   outtextxy( 20, 110, "mui ten trai va phai de duy chuyen vet sang den ten Menu" );
   outtextxy( 20, 130, "ban can va go ENTER de chon no. Sau khi ban chon se co Menu" );
   outtextxy( 20, 150, "PopUp tha xuong, trong do co cac Menu con de chon no ban" );
   outtextxy( 20, 170, "cung dung phim mui ten tren va duoi de duy chuyen vet sang" );
   outtextxy( 20, 190, "den muc ban can va go ENTER de chon no." );
   outtextxy( 50, 240, "- Chu y : Khi Menu PopUp da duoc tha xuong neu ban muon" );
   outtextxy( 20, 260, "doi y khong chon no nua ban cung co the su dung cac phim" );
   outtextxy( 20, 280, "mui ten trai va phai de duy chuyen sang cac Menu PopUp khac" );
   outtextxy( 20, 300, "ma khong can phai go phim ESC de tat Menu PopUp do di. Viec" );
   outtextxy( 20, 320, "chon lua cac Menu con o vi tri dau va o vi tri cuoi ban co" );
   outtextxy( 20, 340, "the chon nhanh bang cac phim HOME va END ung voi Menu Bar" );
   outtextxy( 20, 360, "va Menu PopUp.");
   settextstyle( DEFAULT_FONT, HORIZ_DIR, 2 );
   setcolor( LIGHTGREEN );
   outtextxy( 240, 10, "HUONG DAN" );
   outtextxy( 150, 455, "Press any key to exit" );
}

/*-----------------------------------------*/
/*	PHAN BAN QUYEN CHUONG TRINH	   */
/*-----------------------------------------*/
void Copyringht()
{
   cleardevice();
   setcolor( WHITE );
   rectangle( 1, 1, getmaxx() - 1, getmaxy() - 1 );
   rectangle( 4, 4, getmaxx() - 4, getmaxy() - 4 );
   line( 5, 28, getmaxx() - 5, 28 );
   line( 5, 450, getmaxx() - 5, 450 );
   setbkcolor( BLUE );
   settextstyle( DEFAULT_FONT, HORIZ_DIR, 2 );
   setcolor( LIGHTCYAN );
   outtextxy( 80, 50, "Rat cam on ban da chon muc nay" );
   settextstyle( TRIPLEX_FONT, HORIZ_DIR, 1 );
   setcolor( YELLOW );
   outtextxy( 200, 90, "Khoa Cong Nghe Thong Tin" );
   outtextxy( 150, 115, "Truong Dai Hoc KY THUAT CONG NGHE" );
   outtextxy( 150, 140, "- Co so I  : 110 Cao Thang Quan 3" );
   outtextxy( 150, 165, "- Co so II : 144/24 Dien Bien Phu" );
   outtextxy( 260, 190, "Quan Binh Thanh" );
   outtextxy( 150, 215, "Moi chi tiet xin lien he tai dia chi:" );
   outtextxy( 150, 240, "- Email  : nguyenvan@hcmpt.vnn.vn" );
   outtextxy( 150, 265, "- Website : http://www.unitech.vnn.vn" );
   outtextxy( 150, 290, "- Phone  : 9300061" );
   outtextxy( 10, 330, "- Chu y : Chuong trinh nay la phien ban chay thu nghiem. Cam" );
   outtextxy( 10, 350, "sao chep mot phan hay ca chuong trinh duoi bat ky hinh thuc" );
   outtextxy( 10, 370, "nao, moi vi pham se bi truy to truoc hoi dong cua nha truong." );
   outtextxy( 10, 390, "Neu cac ban co nhu cau ve chuong trinh nguon xin vui long lien" );
   outtextxy( 10, 410, "he tai cac dia chi tren." );
   settextstyle( DEFAULT_FONT, HORIZ_DIR, 2 );
   setcolor( LIGHTGREEN );
   outtextxy( 240, 10, "BAN QUYEN" );
   outtextxy( 150, 455, "Press any key to exit" );
}

/*--------------------------------------------------*/
/*        XAC DINH DO RONG CUA CHUOI                */
/*--------------------------------------------------*/
void OutTextWidth(int x, int y ,char *string, int w)
{
   int i;

   outtextxy( x, y, string );
   for( i = 1; i <= w; i++ )
   {
      outtextxy( x + i, y, string );
      outtextxy( x - i, y, string );
   }
}

/*--------------------------------------------------*/
/*    VIET MOT CHUOI CO BONG CANH TAI TOA DO x, y   */
/*--------------------------------------------------*/
void OutTextShade(int x, int y, char *string, int shade, int color1, int color2, int w)
{
   int i;

   setcolor( color2 );
   for( i = shade; i > 0; i-- ) OutTextWidth( x + i, y + i, string, w );
   setcolor( color1 );
   OutTextWidth( x, y, string, w );
}

/*-----------------------------------------------*/
/*     	  VE KHUNG NEN CHO PHAN GIOI THIEU       */
/*-----------------------------------------------*/
void BackGround()
{
    //Khung hinh chu nhat ben ngoai
    setfillstyle( SOLID_FILL, LIGHTGRAY );
    bar( 0, 10, getmaxx() - 1, getmaxy() - 1 );
    setcolor( WHITE );
    rectangle( 0, 10, 639, 479 );
    //Net ve ben trai
    setcolor( BLACK );
    moveto( 1, 11 );
    lineto( 1, 478 );
    lineto( 7, 472 );
    lineto( 7, 17 );
    lineto( 1, 11 );
    //Net ve ben phai
    setfillstyle( SOLID_FILL, DARKGRAY );
    setcolor( DARKGRAY );
    moveto( 638, 11 );
    lineto( 638, 478 );
    lineto( 632, 472 );
    lineto( 632, 17 );
    lineto( 638, 11 );
    floodfill( 635, 20, DARKGRAY );
    //Net ve ben tren
    moveto( 1, 11 );
    lineto( 638, 11 );
    lineto( 632, 17 );
    lineto( 7, 17 );
    lineto( 1, 11 );
    //Net ve ben duoi
    setfillstyle( SOLID_FILL, DARKGRAY );
    setcolor( DARKGRAY );
    moveto( 1, 478 );
    lineto( 638, 478 );
    lineto( 632, 472 );
    lineto( 7, 472 );
    lineto( 1, 478 );
    floodfill( 10, 475, DARKGRAY );
    //Ve khung hinh nen ben trong
    setfillstyle( SOLID_FILL, CYAN );
    bar( 8, 18, 631, 471 );
}

/*----------------------------------------------------------*/
/*	   HAM TAO CON CHAY VA CAC DIEM LAP LANH            */
/*----------------------------------------------------------*/
void decor()
{
  int a, b, x, y, x0, y0, n, i, j, xx[1001], yy[1001];
  float goc, xt, yt;
  int *p;
  static d = 1;
  // Ve tau vu tru
  start_graphics();
  setcolor( LIGHTRED );
  ellipse( 100, 50, 0, 360, 20, 8 );
  ellipse( 100, 46, 190, 357, 20, 6 );
  line( 107, 44, 110, 38 );
  line( 93, 44, 90, 38 );
  circle( 110, 38, 2 );
  circle( 90, 38, 2 );
  setfillstyle( SOLID_FILL, LIGHTBLUE );
  floodfill( 101, 54, LIGHTRED );
  setfillstyle( SOLID_FILL, LIGHTMAGENTA );
  floodfill( 94, 45, LIGHTRED );
  // Luu anh vao bo nho
  n = imagesize( 79, 36, 121, 59 );
  p = (int *)malloc( n );
  getimage( 79, 36, 121, 59, p );
  cleardevice();
  Introduction();
  // Tao cac ngoi sao nhap nhay
  for( i = 1; i <= 1000; i++ )
  {
      xx[i] = random( getmaxx() );
      yy[i] = random( getmaxy() );
      putpixel( xx[i], yy[i], random( getmaxcolor() ) + 1 );
  }

  // Xac dinh quy dao cua tau
  goc = 2 * M_PI + M_PI / 2;
  x0 = (getmaxx() - 42) / 2;
  y0 = (getmaxy() - 25) / 2;
  a = x0; b = y0;
  // Chu trinh tau chuyen dong va cac ngoi sao nhap nhay
  do
  {
    for( j=1; j<=1000; j++)
    {
      putpixel( xx[j], yy[j], random( MAXCOLORS ) + 1 );
      ++d;
      if( d > 1000 ) d = 1;
    }

    /* Tinh goc chuyen dong cua tau va dat anh len man hinh */
    xt = a * cos( goc ) + x0;
    yt = b * sin( goc ) + y0;
    x = (int)(xt + 0.5);
    y = (int)(yt + 0.5);
    putimage( x, y, p, XOR_PUT );

    // Thay doi goc chuyen dong
    goc -= M_PI / 30;
    if( goc < M_PI / 2 )
    goc = 2 * M_PI + M_PI / 2;
  } while( !kbhit() );
  setgraphmode( getgraphmode() );
}

/*---------------------------------------------*/
/*         HAM TAO GIAO DIEN MAN HINH          */
/*---------------------------------------------*/
void Introduction()
{
    int ht, h, y, sh, i, maxx;
    maxx = getmaxx();
    ht = maxx / 2;
    setbkcolor( BLACK );
    settextjustify( 1, 1 );
    settextstyle( 0, 0, 4 );
    h = textheight( "H" );
    y = 150;
    OutTextShade( ht-10, y, "<De Tai 72>", h / 6,LIGHTCYAN,LIGHTMAGENTA,1);
    setcolor(LIGHTCYAN);
    setlinestyle(0,0,3);
    line( ht - 185, 170, ht + 165 , 170 );
    y += 3 * h;
    settextstyle( 0, 0, 5 );
    settextjustify( 1, 0 );
    h = textheight( "H" );
    y += ( h / 2 ) + 33;
    OutTextShade( ht, y - 10, "CAC PHEP BIEN", h / 5, LIGHTRED, YELLOW, 1 );
    OutTextShade( ht, y + 80,"HINH 2D", h / 5, LIGHTRED, YELLOW, 1 );
}

// Check Password
void get_password()
{
  static const char *list_menu[2] = {"  ~Retry  ","  ~Quits  "};
  char key, *PassW1, *PassW2, buffer[1024];
  byte Flag, j, i, select = 0, col = 0, row = 0;

  PassW1 = "NV60FA4322";
  PassW2 = (char *)malloc(9*sizeof(char));
  set_cursor_size(0x2020);
  set_text_attrib(0x13);
  set_border_color(25);
  clrscr();
  movedata(0xF000,0xFA6E,_DS,(unsigned)table_chars,sizeof(table_chars));
  do_blinking(0);
  fill_frame(1,1,80,25,0xFD,178);
  fill_frame(3,2,78,12,0x4E,179);
  put_box( 3, 2, 78, 12, 0xF0, 3);
  set_text_attrib(0x4E);
  put_big_STR(4, "TRANSFROM");
  box_shadow(20,16,58,22,0x7F,2,0x4F," Password ",1);
  write_VRAM(22,18,0x71,"Enter your password");
  write_VRAM(43,18,0x1F,"          ");
  button(36,20,0xE3,7,"  ~Ok  ",1,0xE4);
  gettext(20,16,60,23,buffer);
  init_mouse();
  set_pmcl();
  do {
    puttext(20,16,60,23,buffer);
    show_mouse();
    i = 0;
    do {
      key = getch();
      if((i < NEWLINE) && (key != ENTER) && (key != DEL)) {
	PassW2[i++] = key;
	write_XY(42 + i,18,0x1F,42);
      }
      else
	if((key == DEL) && (i > 0)) {
	  write_XY(42 + i,18,0x1F,32);
	  i--;
	}
	else
	  if(key != ENTER) {
	    sound(500);
	    delay(100);
	    nosound();
	  }
    } while(key != ENTER);
    do {
       if(click_mouse(&col, &row) == 1) {
	  if(row == 20 && col >= 36 && col <= 41) {
	     hide_mouse();
	     clear(36,20,42,21,7);
	     write_VRAM(37,20,0xF0,"  ~Ok  ",0xF4);
	     delay(100);
	     button(36,20,0xE3,7,"  ~Ok  ",1,0xE4);
	     show_mouse();
	     break;
	  }
	  if(row == 16 && col == 23) {
	     hide_mouse();
	     write_XY(23,16,0x7A,15);
	     delay(100);
	     show_mouse();
	     break;
	  }
       }
    } while(!kbhit());
    hide_mouse();
    Flag = TRUE;
    for(j = 0; j < 10; j++)
      if(PassW1[j] != PassW2[j]) {
	  Flag = FALSE;
	  break;
      }
    if(!Flag) {
      box_shadow(18,16,60,22,0x7F,2,0x4F," Result ",1);
      write_VRAM(29,18,0x71,"Incorrect password !");
      button(26,20,0xF0,7,list_menu[0],1,0xF4);
      button(43,20,0xE8,7,list_menu[1],1,0xE2);
      show_mouse();
      key = 0;
      do {
	 if(click_mouse(&col, &row) == 1) {
	    if(row == 20 && col >= 26 && col <= 34) {
	       hide_mouse();
	       button(43,20,0xE8,7,list_menu[1],1,0xE2);
	       clear(26,20,35,21,7);
	       write_VRAM(27,20,0xF0,list_menu[0],0xF4);
	       delay(100);
	       button(26,20,0xF0,7,list_menu[0],1,0xF4);
	       show_mouse();
	       select = 0;
	       key = ENTER;
	       break;
	    }
	    if(row == 20 && col >= 43 && col <= 50) {
	       hide_mouse();
	       button(26,20,0xE8,7,list_menu[0],1,0xE2);
	       clear(43,20,51,21,7);
	       write_VRAM(44,20,0xF3,list_menu[1],0xF4);
	       delay(100);
	       button(43,20,0xF3,7,list_menu[1],1,0xF4);
	       show_mouse();
	       select = 1;
	       key = ENTER;
	       break;
	    }
	 }
	 if(kbhit()) {
	    key = getch();
	    if(!key) key = getch();
	    switch(key) {
	       case LEFT:
		  button(26 + select*17, 20, 0xE8, 7, list_menu[select], 1, 0xE2);
		  if(select <= 0) select = 0;
		  else select--;
		  button(26 + select*17, 20, 0xF3, 7, list_menu[select], 1, 0xF4);
		  break;
	       case RIGHT:
		  button(26 + select*17, 20, 0xE8, 7, list_menu[select], 1, 0xE2);
		  if(select >= 1) select = 1;
		  else select++;
		  button(26 + select*17, 20, 0xF3, 7, list_menu[select], 1, 0xF4);
	    }
	    switch(toupper(key)) {
	       case 'R': break;
	       case 'Q': exit_program();
	    }
	 }
      } while(key != ENTER);
      fill_frame(18,16,62,23,0xFD,178);
      hide_mouse();
      if(select) exit_program();
    }
    else {
      del_horizon();
      textmode(C80);
      textbackground(BLUE);
      clrscr();
      box_shadow(18,10,60,14,0x5F,2,0x4F," Result ",1);
      write_VRAM(30,12,0x5E,"Correct password");
      write_VRAM(24,13,0x5A,"Press any key to start program");
      getch();
    }
  } while(!Flag);
  free(PassW2);
}

// Say good bye
void say_goodbye()
{
  closegraph();
  set_cursor_size(0x2020);
  set_text_attrib(0x1A);
  set_border_color( MAGENTA );
  clrscr();
  fill_frame(1,1,80,25,LIGHTGREEN,178 );
  box_shadow(14,6,69,17,0x3E,2,0x4F," Goodbye ");
  write_VRAM(27,15,0x3F,"Press <ENTER> key To Exit Program");
  write_VRAM(16, 8,0x30,"Rat cam on cac ban da ung ho va su dung chuong trinh" );
  write_VRAM( 16, 9, 0x30,"cua toi . Mong cac ban gop y ve noi dung  cua chuong" );
  write_VRAM( 16, 10,0x30, "trinh de chuong trinh duoc hoan thien hon . Moi thac" );
  write_VRAM( 16, 11,0x30, "mac hay y kien dong gop xin vui long lien he tai dia" );
  write_VRAM( 16, 12,0x30, "trong phan ban quyen chuong trinh xin chan thanh cam" );
  write_VRAM( 16, 13,0x30, "on cac ban rat nhieu." );
  getch();
  next_pages();
  exit_program();
}

/*------------------------------------------------*/
/* Function : set_text_attrib                     */
/* Mission  : Set the attribute for the text      */
/* Expects  : (attr) the attr for text            */
/* Returns  : Nothing                             */
/*------------------------------------------------*/
void set_text_attrib(word attr)
{
   textattr(attr);
}

/*-------------------------------------------------*/
/* Function : save_current_attrib                  */
/* Mission  : Save current attrib of the text mode */
/* Expects  : Nothing                              */
/* Returns  : Current color of the text mode       */
/*-------------------------------------------------*/
word save_current_attrib()
{
   gettextinfo(&info);
   return info.attribute;
}

/*------------------------------------*/
/* Funtcion : set_border_color        */
/* Mission  : Seting border color     */
/* Expects  : (color) color of border */
/* Returns  : Nothing                 */
/*------------------------------------*/
void set_border_color(byte color)
{
   union REGS regs;

   regs.h.ah = 0x10;          // Serial number
   regs.h.al = 0x01;          // Serial subst number
   regs.h.bh = color & 63;    // Release bits 7
   int86(0x10, &regs, &regs); // Calling interrupt 10h
}

/*--------------------------------------*/
/* Function : set_cursor_size           */
/* Mission  : Resize the cursor         */
/* Expects  : (size) The size of cursor */
/* Returns  : Nothing                   */
/*--------------------------------------*/
void set_cursor_size( word Size )
{
  asm {
    mov ah, 1
    mov cx, Size;
    int 16
  }
}

/*---------------------------------------------------*/
/* Function : putch_XY                               */
/* Mission  : Write a character to cordinate x, y    */
/* Expects  : (x,y) cordinate to write               */
/*            (chr) character to write               */
/* Returns  : Nothing                                */
/*---------------------------------------------------*/
void putch_XY(byte x, byte y, char chr)
{
  gotoxy(x, y);
  putch(chr);
}

/*-------------------------------------------------*/
/* Function : write_XY                             */
/* Mission  : Write a character to cordinate x, y  */
/* Expects  : (x,y) cordinate to write             */
/*            (chr) character to write             */
/*            (attr) attrib for the character      */
/* Returns  : Nothing                              */
/*-------------------------------------------------*/
void write_XY(byte x, byte y, word attr, char chr)
{
   buffer[OFFSET(x, y)] = chr;
   buffer[OFFSET(x, y) + 1] = attr;
}

/*---------------------------------------------------*/
/* Function : frame                                  */
/* Mission  : Draw a frame with the edge is lane     */
/* Expects  : (x1,y1) cordinate top to left          */
/*            (x2,y2) cordinate bottom to right      */
/*            (lane) code formatting the frame with: */
/*            lane = 1 single edge                   */
/*            lane = 2 double edge                   */
/*            lane = 3 left and right double edge    */
/*            lane = 4 top and bottom double edge    */
/* Returns  : Nothing                                */
/*---------------------------------------------------*/
void frame(byte x1, byte y1, byte x2, byte y2, byte lane)
{
   static byte bound[4][6] = // table_chars code to format the frame
   { {218, 196, 191, 179, 217, 192},
     {201, 205, 187, 186, 188, 200},
     {213, 205, 184, 179, 190, 212},
     {214, 196, 183, 186, 189, 211}
   };
   char *border;
   byte k;

   lane = (lane - 1) % 4;
   border = bound[lane];
   putch_XY(x1, y1, border[0]);
   for(k = x1 + 1; k < x2; putch(border[1]), k++);
   putch(border[2]);
   putch_XY(x1, y2, border[5]);
   for(k = x1 + 1; k < x2; putch(border[1]), k++);
   putch( border[4] );
   for(k = y1 + 1; k < y2; k++) {
     putch_XY(x1, k, border[3]);
     putch_XY(x2, k, border[3]);
   }
}

/*---------------------------------------------------*/
/* Function : box                                    */
/* Mission  : Draw a box with color and border       */
/* Expects  : (x1,y1) cordinate top to left          */
/*            (x2,y2) cordinate bottom to right      */
/*            (attr) the attribute of the box        */
/*            lane = 1 single edge                   */
/*            lane = 2 double edge                   */
/*            lane = 3 left and right double edge    */
/*            lane = 4 top and bottom double edge    */
/* Returns  : Nothing                                */
/*---------------------------------------------------*/
void box(byte x1, byte y1, byte x2, byte y2, word attr, byte lane)
{
   word old_attr;

   old_attr = save_current_attrib();       // Save current attrib
   set_text_attrib(attr);                  // Setting the attribute
   frame(x1, y1, x2, y2, lane);            // Draw a frame
   window(x1 + 1, y1 + 1, x2 - 1, y2 - 1); // Setting a window
   clrscr();                               // Fill color in window setting
   window(1, 1, 80 ,25);                   // Setting normal window
   set_text_attrib(old_attr);              // Setting normal color
}

/*---------------------------------------------------*/
/* Function : replicate                              */
/* Mission  : Repeat a character with times          */
/* Expects  : (symbol) character needs repeat        */
/*            (times) number of repeat               */
/* Returns  : The string with a character format     */
/*---------------------------------------------------*/
char *replicate(char symbol, byte times)
{
  char *aux;
  byte k;

  aux = (char *)malloc(times + 1);
  for(k = 0; k < times; aux[k++] = symbol);
  *(aux + times) = 0;
  return aux;
}

/*---------------------------------------------------*/
/* Function : set_attrib                             */
/* Mission  : Chage the attribute of the area screen */
/* Expects  : (x1,y1) cordinate top to left          */
/*            (x2,y2) cordinate bottom to right      */
/*            (attr) the attribute                   */
/* Returns  : Nothing                                */
/*---------------------------------------------------*/
void set_attrib(byte x1, byte y1, byte x2, byte y2, word attr)
{
   byte col, row;

   for(col = x1; col <= x2; col++)
     for(row = y1; row <= y2; row++)
       pokeb(0xB800, OFFSET(col, row) + 1, attr);
}

/*-----------------------------------------------*/
/* Function : do_blinking                        */
/* Mission  : Redefine bits 7 in the text attrib */
/* Expects  : (doblink) = 0, back color is light */
/*                      = 1, text blinking       */
/* Return   : Nothing                            */
/*-----------------------------------------------*/
void do_blinking(byte doblink)
{
   union REGS regs;

   regs.h.ah = 0x10;
   regs.h.al = 0x03;
   regs.h.bl = doblink ? 1 : 0;
   int86(0x10, &regs, &regs);
}

/*-----------------------------------------------*/
/* Function : write_VRAM                         */
/* Mission  : Writing a character with attribute */
/* Notices  : Intervention in video memory       */
/* Expects  : (x,y) cordinate needs to writting  */
/*            (attr) The attribute of string     */
/*            (str) the string to format         */
/* Returns : Nothing                             */
/*-----------------------------------------------*/
void write_VRAM(byte x, byte y, word text_attr, const char *str, word letter_attr)
{
   byte i = 0, flag = 0, current_x = x, pos;
   char *backup;

   if(letter_attr) {
      backup = (char *)malloc(strlen(str)*sizeof(char));
      strcpy(backup, str);
      for(; (i < strlen(backup) - 1) && !flag;)
      if(backup[i++] == 126) flag = 1;
      memmove(&backup[i-1], &backup[i], strlen(backup) - i + 1);
      pos = i - 1;
      i = 0;
      while(backup[i]) {
	 buffer[OFFSET(x, y)] = backup[i++];
	 buffer[OFFSET(x++, y) + 1] = text_attr;
      }
      write_XY(current_x + pos, y, letter_attr, backup[pos]);
      free(backup);
   }
   else {
      while(*str) {
	 buffer[OFFSET(x, y)] = *str++;
	 buffer[OFFSET(x++, y) + 1] = text_attr;
      }
   }
}

/*----------------------------------------------*/
/* Function : button                            */
/* Mission  : Define the button shadow          */
/* Expects  : (x,y) cordinate needs to writting */
/*            (attr) the attribute of a title   */
/*            (title) the string to format      */
/*            (back) color for background       */
/* Returns : Nothing                            */
/*----------------------------------------------*/
void button(byte x, byte y, word attr, byte back, const char *title, byte type, word first_letter)
{
   static char style[4] = {16, 17, 223, 220};
   byte len = strlen(title);
   word type_attr = back << 4;

   if(type) {
      if(first_letter) {
	 write_VRAM(x, y, attr, title, first_letter);
	 write_VRAM(x + 1, y + 1, type_attr, replicate(style[2],len - 1));
	 write_XY(x + len - 1, y, type_attr, style[3]);
      }
      else {
	 write_VRAM(x, y, attr, title);
	 write_VRAM(x + 1, y + 1, type_attr, replicate(style[2],len));
	 write_XY(x + len, y, type_attr, style[3]);
      }
   }
   else {
      if(first_letter) {
	 write_VRAM(x, y, attr, title, first_letter);
	 write_XY(x, y, attr, style[0]);
	 write_XY(x + len - 2, y, attr, style[1]);
	 write_VRAM(x + 1, y + 1, type_attr, replicate(style[2],len - 1));
	 write_XY(x + len - 1 , y, type_attr, style[3]);
      }
      else {
	 write_VRAM(x, y, attr, title);
	 write_XY(x, y, attr, style[0]);
	 write_XY(x + len - 1, y, attr, style[1]);
	 write_VRAM(x + 1, y + 1, type_attr, replicate(style[2],len));
	 write_XY(x + len, y, type_attr, style[3]);
      }
   }
}

/*---------------------------------------------------*/
/* Function : box_shadow                             */
/* Mission  : Draw a box with shadow (very art)      */
/* Expects  : (x1,y1) cordinate top to left          */
/*            (x2,y2) cordinate bottom to right      */
/*            (box_attr) the attribute of the box    */
/*            (msg_attr) the attribute of title      */
/*            (title) the title of header            */
/*            (lane) the code number lane (see frame)*/
/* Returns  : Nothing                                */
/*---------------------------------------------------*/
void box_shadow(byte x1, byte y1, byte x2, byte y2, word box_attr, byte lane, word msg_attr, char *title, byte is_win)
{
   byte len_center = strlen(title) / 2,
   cor_center = (x2 - x1) / 2,
   center = cor_center - len_center;

   set_attrib(x2 + 1, y1 + 1, x2 + 2, y2 + 1, 0x08);
   set_attrib(x1 + 2, y2 + 1, x2 + 2, y2 + 1, 0x08);
   box(x1, y1, x2, y2, box_attr, lane);
   write_VRAM(x1 + center, y1, msg_attr, title);
   if(is_win) {
      write_VRAM(x1+2,y1,box_attr,"[ ]");
      write_XY(x1+3,y1,((box_attr / 16) << 4) + 10, 254);
      write_VRAM(x2-4,y1,box_attr,"[ ]");
      write_XY(x2-3,y1,((box_attr / 16) << 4) + 10, 18);
   }
}

/*---------------------------------------------------*/
/* Function : write_char                             */
/* Mission  : Writting a character with attribute    */
/* Expects  : (x,y) cordinate to write a character   */
/*            (attr) attribute of character          */
/*            (len) length area                      */
/*            (chr) symbol needs to write            */
/* Returns  : Nothing                                */
/*---------------------------------------------------*/
void write_char(byte x, byte y, byte attr, byte len, char chr)
{
    word video_offset;
    byte i;

    video_offset = OFFSET(x,y);
    for(i = 1; i <= len; i++) {
      poke(0xB800, video_offset, (attr << 8) + chr);
      video_offset += 2;
    }
}

/*---------------------------------------------------*/
/* Function : fill_frame                             */
/* Mission  : to full the box with special character */
/* Expects  : (x1,y1) cordinate top to left          */
/*            (x2,y2) cordinate bottom to right      */
/*            (attr) special character color         */
/*            (chr) special character                */
/* Returns  : Nothing                                */
/*---------------------------------------------------*/
void fill_frame(byte x1, byte y1, byte x2, byte y2, byte attr, char chr)
{
   byte i = y1;

   for( ;i <= y2; ) write_char(x1, i++, attr, x2 - x1 + 1, chr);
}

/*----------------------------------------------*/
/* Function : clear                             */
/* Mission  : Clear the part window             */
/* Expects  : (x1,y1) cordinate top to left     */
/*            (x2,y2) cordinate bottom to right */
/*            (color) color needs clear         */
/* Returns  : Nothing                           */
/*----------------------------------------------*/
void clear(byte x1, byte y1, byte x2, byte y2, byte color)
{
   window(x1,y1,x2,y2);
   set_text_attrib((color << 4));
   clrscr();
   window(1,1,80,25);
}

/*---------------------------------*/
/* Function : exit_program         */
/* Mission  : Restore environment  */
/* Expects  : Nothing              */
/* Returns  : Nothing              */
/*---------------------------------*/
void exit_program()
{
   set_border_color(0x00);  // Restore the border color
   set_cursor_size(0x0607); // Restore the size of cursor
   set_text_attrib(0x07);   // Setting the standar fonts
   do_blinking(1);          // Setting the standar video model
   if(mouse_avalid)         // Initialize successful ?
   close_mouse();           // Closing mouse function
   clrscr();                // Clear the screen result
   exit(EXIT_SUCCESS);      // Terminated the program body
}

// Define the address of the Segment
word video_SEG()
{
  if( *(char far *)0x00400049 == 7 )
    return 0xB000;
  else
    return 0xB800;
}

// Define the address of the Offset
byte video_OFFS(byte col, byte row)
{
  if(video_SEG() == 0xB000 )
    return (byte)(col - 1) * 2 + 80 * (row - 1);
  else
    return (byte)(col - 1) * 2 + 160 * (row - 1);
}

// Take out characters from possition
char *copy(char *dest, int pos, int nchar)
{
   char k, *str;

   nchar = MIN(nchar,strlen(dest));
   str = (char *)malloc(nchar + 1);
   if(str == NULL) return "";
   for(k = 0; k < nchar && dest[k + pos - 1]; k++) str[k] = dest[k + pos - 1];
   str[k] = '\0';
   return str;
}

// Paint bigger a character
void paint_big_char(bits entry)
{
   char pattern_line, spot;
   byte curr_line;

   for(pattern_line = 0; pattern_line < 8; pattern_line++) {
      curr_line = entry[pattern_line];
      for(spot = 8; spot > 0; spot--) {
	 if(curr_line % 2) putch_XY(spot,pattern_line + 1,219);
	 curr_line >>= 1;
      }
   }
}

// Put bigger of a character
void put_big_STR(byte start_row, char *msg)
{
   byte n, len, start_column, curr_column;
   char *st;

   st = copy(msg, 1, 10);
   len = strlen(st);
   start_column = (( 80 - 8 * len ) / 2) & 0x00FF;
   for(n = 0; n < len; n++) {
      curr_column = start_column + 8 * n + 2 ;
      window(curr_column,start_row,curr_column + 7,start_row + 8);
      paint_big_char(table_chars[st[n]]);
   }
   window(1,1,80,25);
}

// Put a character to x, y
void put_char_XY(byte x, byte y, char chr)
{
   gotoxy(x,y);
   putch(chr);
   delay(10);
}

// Draw a box with a character
void put_box(byte x1, byte y1, byte x2, byte y2, word attr, char chr)
{
   byte i;

   set_text_attrib(attr);
   // Edge top
   for(i = x1; i <= x2; put_char_XY(i++,y1,chr));
   // Edge right
   for(i = y1; i <= y2; put_char_XY(x2,i++,chr));
   // Edge bottom
   for(i = x2; i >= x1; put_char_XY(i--,y2,chr));
   // Edge left
   for(i = y2; i >= y1; put_char_XY(x1,i--,chr));
}

// Clear the part of the screen
void del_horizon()
{
   byte k;

   textbackground(1);
   for(k = 1; k <= 39; k++) {
      window(40 - k,1,42 - k,25);
      clrscr();
      delay(100);
      window(39 + k,1,41 + k,25);
      clrscr();
   }
}

// Delete a line
void next_pages()
{
   byte i;

   set_text_attrib(0x1F);
   gotoxy(1,1);
   for(i = 1; i <= 25; i++) {
      delline();
      delay(60);
   }
}

/*----------------------------------*/
/* Function : init_mouse            */
/* Mission  : Initialize mouse port */
/* Expects  : Nothing               */
/* Returns  : Nothing               */
/*----------------------------------*/
byte init_mouse()
{
   union REGS regs;

   regs.x.ax = 0x00;
   int86(0x33, &regs, &regs);
   if(regs.x.ax != 0xFFFF) return FALSE;
   regs.x.ax = 0x01;
   int86(0x33, &regs, &regs);
   mouse_avalid = TRUE;
   return TRUE;
}

/*------------------------------------------------------*/
/* Function : click_mouse                               */
/* Mission  : Get status button and cordinate col, row  */
/* Expects  : (*col, *row) cordinate of col and row     */
/* Returns  : Value 1 : left button, 2 : right button,  */
/*            4 : center button and col, row            */
/*------------------------------------------------------*/
byte click_mouse(byte *col, byte *row)
{
   union REGS regs;

   regs.x.ax = 0x03;
   int86(0x33, &regs, &regs);
   *row = regs.x.dx / 8 + 1;
   *col = regs.x.cx / 8 + 1;
   return(regs.x.bx);
}

/*-----------------------------------*/
/* Function : hide_mouse             */
/* Mission  : Hide the mouse pointer */
/* Expects  : Nothing                */
/* Returns  : Nothing                */
/*-----------------------------------*/
void hide_mouse()
{
   union REGS regs;

    regs.x.ax = 0x02;
    int86(0x33, &regs, &regs);
}

/*--------------------------------------*/
/* Function : show_mouse                */
/* Mission  : Showing the mouse pointer */
/* Expects  : Nothing                   */
/* Returns  : Nothing                   */
/*--------------------------------------*/
void show_mouse()
{
   union REGS regs;

   regs.x.ax = 0x01;
   int86(0x33, &regs, &regs);
}

/*------------------------------------------*/
/* Function : set_pmcl                      */
/* Mission  : Setting the limit col and row */
/* Expects  : Nothing                       */
/* Returns  : Nothing                       */
/*------------------------------------------*/
void set_pmcl()
{
   union REGS regs;

   regs.x.ax = 0x07;
   regs.x.cx = 0;
   regs.x.dx = 2*320 - 8;
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
/* Function : move_mouse                          */
/* Mission  : Move mouse pointer to new cordinate */
/* Expects  : (col, row) the new cordinate        */
/* Returns  : Nothing                             */
/*------------------------------------------------*/
void move_mouse(byte x, byte y)
{
   union REGS regs;

   regs.x.ax = 0x0004;
   regs.x.cx = 8*x - 1;
   regs.x.dx = 8*y - 1;
   int86(0x33, &regs, &regs);
}

/*------------------------------------*/
/* Function : close_mouse             */
/* Mission  : Colosing mouse fumction */
/* Expects  : Nothing                 */
/* Returns  : Nothing                 */
/*------------------------------------*/
void close_mouse()
{
   union REGS regs;

   hide_mouse();
   regs.x.ax = 0;
   int86(0x33, &regs, &regs);
}

/*--------------------------------------*/
/* Funtion : is_register                */
/* Mission : Checking value of regsiter */
/* Expects : Nothing                    */
/* Returns : Value 0 if not registers   */
/*--------------------------------------*/
char is_register()
{
   /*FILE *fptr;
   INFO tmp;
   byte num_regs;

   if(!(fptr = fopen("C:\\WINDOWS\\register.dat", "rb"))) {
	  clrscr(); fprintf(stderr,"Error loading data file... System halt."); exit(1);
   }
   fread(&tmp,sizeof(INFO),1,fptr);
   fclose(fptr);
   strcpy(disk,tmp.disk);
   return tmp.regs ? TRUE : FALSE;*/
   return TRUE;
}
/*-----------------------------------------------------------------*/
/*------------------- HET CHUONG TRINH NGUON ----------------------*/
/*-----------------------------------------------------------------*/
