/*------------------------------------------------------------*/
/*        UNIVERSITY OF TECHNOLOGY HO CHI MINH CITY           */
/*             MAJOR OF INFORMATIC TECHNOLOGY                 */
/*       PROCCESSING PICTURES AND TRANSFROM PICTURES          */
/*             Author : NGUYEN NGOC VAN                       */
/*              Class : 00DTH1                                */
/*       Student Code : 00DTH201                              */
/*             Course : 2000 - 2005                           */
/*      Writting Date : 02/10/2001                            */
/*        Last Update : 24/10/2001                            */
/*------------------------------------------------------------*/
/*        Environment : Borland C++ Ver 3.1 Application       */
/*        Source File : PROCPICT.CPP                          */
/*        Memory Mode : Small                                 */
/*            Compile : BCC PROCPICT.CPP                      */
/*        Call to run : PROCPICT (none the project file)      */
/*------------------------------------------------------------*/
#include <dos.h>
#include <stdio.h>
#include <conio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <graphics.h>

#define ENTER   13
#define TAB      9
#define DEL      8
#define NEWLINE 10
#define MAXFN   40
#define ESC     27
#define LEFT    75
#define RIGHT   77
#define UP      72
#define DOWN    80
#define HOME    71
#define END     79
#define TRUE     1
#define FALSE    0
#define BAKC  BLUE
#define LIC  WHITE
#define TRUE  1
#define FLASE 0

#define MIN(a,b) ( (a) < (b) ? (a) : (b) )
#define OFFSET(x,y) (x - 1)*2 + 160*(y - 1)
#ifndef MK_FP
#define MK_FP(seg, ofs) ((void far*)((unsigned long)(seg) << 16 | (ofs)))
#endif

typedef unsigned char byte;
typedef unsigned int word;
typedef byte CHARS[8];

typedef struct {
   long Rows;
   long Cols;
   byte *Data;
} IMAGE; // Dinh dang file anh

typedef struct {
   int Id;
   long FileSize;
   long Reserve;
   long StartAddr;
} FILEHEADER; // Phan dia chi cua anh

typedef struct {
	long BmpSize;
   long Width;
   long Height;
   int NumPlane;
   int NumBit;
   long CommCode;
   char temp[20];
} BMPINFO; // Cac thong tin ve anh

typedef struct {
   byte blue;
   byte green;
   byte red;
   byte reserve;
} PALENTRY; // Mau DAC

typedef struct {
   float c_trast;
   int b_ness;
   int info[16];
} BCN; // Cac tham so cua anh

union {
   struct {
      word lByte:4;
      word hByte:4;
   } parts;

   struct {
      byte allbyte;
   } whole;
} dbl;

typedef struct {
   byte date;       // The date of the program
   byte month;      // The month of the program
   byte regs;       // The register code
   byte num;        // The number of run program
   char serial[20]; // Product code
   char user[31];   // Register name user
	char disk[4];    // The disk letter
} INFO;

BCN bnc[1000];                 /* So diem anh cua mot file      */
CHARS table[256];              /* Dinh nghia mot ma tran ky tu  */
byte mouse_avalid = FALSE;     /* Kiem tra driver mouse         */
word info_num, menu_num;       /* So dong cua file he thong     */
FILEHEADER fh;                 /* Tieu de cua file anh          */
BMPINFO imgf;                  /* Thong tin cua file anh        */
FILE *fbmp;                    /* Con tro file anh              */
struct text_info info;         /* Thong tin cua man hinh        */
struct palettetype pal;        /* Mau nen                       */
char far *buffer =             /* Dat con tro buffer den        */
(char far *)MK_FP(0xB800,0);   /* Dia chi man hinh              */
char disk[4];                  /* Ki tu o dia                   */
int key;        	             /* Bay phim 			             */
float contrast; 	             /* Mau Contrast 			          */
int brightness, mbn;	          /* Mau BrightNess                */
float average;		             /* Gia tri trung binh cua anh    */
int curr_val = 0, end_val = 0; /* Current and end value 	       */
char file_target[34];  	       /* Ban sao cua file dang mo      */
char file_source[34];          /* Duong dan cua file muon mo    */
int signal_file = FALSE;       /* Xac dinh file da duoc mo chua */
char **sys_info, **sys_menu;   /* Mang luu thong tin hien thi   */

/*----------------------------------------*/
/*  CAC HAM THIET KE CHUONG TRINH CHINH   */
/*----------------------------------------*/
void get_password(),
     next_pages(),
     say_goodbye(),
     main_menu(),
     BrightNess(),
     Contrast(),
     ChangeContrast(),
     OutPutToScreen(),
	  ChangeBrightNess(),
     OpenFile(char *),
     Histo(char *),
     Histogram(char *),
	  HistogramfileBMP(char *, float *, int *, int *),
     WriteColorReg(int, int, int, int),
     Process(int),
     Window(int, char **),
     DisplayInfo(FILEHEADER *, BMPINFO *, FILE *);

/*----------------------------------------*/
/*  CAC HAM THIET KE GIAO DIEN MAN HINH   */
/*----------------------------------------*/
void exit_program(),
     del_horizon(),
     hide_mouse(),
     show_mouse(),
     set_pmcl(),
     check_period(),
     registers(),
     close_mouse(),
     initialize_program(),
     del_mem(),
     set_text_attrib(word),
	  set_char_width(byte),
     fill_frame(byte, byte, byte, byte, byte, char),
     set_border_color(byte),
     set_cursor_size(word),
     paint_big_char(CHARS),
     put_big_STR(byte, char *),
     putch_XY(byte, byte, char),
     put_char_XY(byte, byte, char),
     clear(byte, byte, byte, byte, byte),
     frame(byte, byte, byte, byte, byte),
     box_shadow(byte, byte, byte, byte, word, byte, word, char *, byte is_win = 0),
     put_box(byte, byte, byte, byte, word, char),
     write_VRAM(byte, byte, word, const char *, word letter_attr = 0),
     button(byte, byte, word, byte, const char *, byte type = 0, word first_letter = 0),
     write_XY(byte, byte, word, char),
     do_blinking(byte),
     get_file_data(const char *, const char *, char **&, word &),
     decode(const char *, const char *);
char read_key(char &), is_register();
byte click_mouse(byte *, byte *), init_mouse();

/*-----------------------------------------------*/
/*     CAC HAM THIET KE MAN HINH GIOI THIEU      */
/*-----------------------------------------------*/
void Interface(), StartGraphics(), PutPixelPlay(), RotateCircle(),
     WriteShade(int, int, char *, int, int),
     WriteShadow(int, int, char *, int, int, int, int),
     OutTextShade(int, int, char *, int, int, int, int),
     OutTextWidth(int, int,char *, int);

/*----------------------------------*/
/*	     CHUONG TRINH CHINH          */
/*----------------------------------*/
void main()
{
	char path[34];
	/*initialize_program();
	if(!is_register()) {
		strcpy(path,disk);
		strcat(path,sys_info[173]);
		strcat(path,disk);
		system(path);
		get_password();
	}
	Interface();*/
	StartGraphics();
	main_menu();
}

/*-------------------------------------*/
/*	     MENU CUA CHUONG TRINH CHINH    */
/*-------------------------------------*/
void main_menu()
{
	char *ListMenu[] = {
			"   Open     ",
			" Histogram  ",
			" BrightNess ",
			" Contrast   ",
			"   Exit     ",
				};

  int select, i, j, t;
  setgraphmode(getgraphmode());
  /* Ve menu len man hinh va cac options */
  setbkcolor( BAKC );
  setcolor( LIC );
  for( i = 0;i < 5; i++ ) rectangle( 127 * i, 0, 127 * (i + 1), 20 );
  /* Xuat cac options len menu bar */
  setcolor( YELLOW );
  for( i = 0; i < 5; i++ ) outtextxy( 127 * i + 20, 8, ListMenu[i] );
  Window( 0, ListMenu );
  /* Di chuyen hop sang tren cua so thuc don */
  select = 0;
  for(;;)
  {
     switch( key )
     {
	 case LEFT :
	   if( select <= 0 ) select = 4;
		else select--;
	   break;

	 case RIGHT :
	   if( select >= 4 ) select = 0;
	   else select++;
	   break;

	 case HOME : select = 0; break;

	 case END : select = 4; break;

	 case ENTER :
	   Process( select );
	   break;

	 default : break;
     }
     Window( select, ListMenu );
  }
}

/*-------------------------------------------*/
/*	HIEN THI MAU SANG CUA ANH            */
/*-------------------------------------------*/
void BrightNess()
{
  if( signal_file )
  {  /* Da mo File */
     gotoxy( 60, 3 );
     printf( "BN = %2d, CT = %2.1f", brightness, contrast );
     do {
		 switch( key )
       {
	 case UP :
	   if( brightness < 15 )
	   {
	     brightness++;
	     mbn = 1;
	     ChangeBrightNess();
		} break;

	 case DOWN :
	   if( brightness > -15 )
	   {
	     brightness--;
	     mbn = -1;
	     ChangeBrightNess();
	   } break;
       }
       gotoxy( 60, 3 );
       printf( "BN = %2d, CT = %2.1f", brightness, contrast );
	  } while( (key = getch()) != ESC );
  } /* If */
}

/*-------------------------------------------*/
/*           XAC LAP MAU CHU CUA ANH         */
/*-------------------------------------------*/
void Contrast()
{
  if( signal_file )
  {  /* Da mo File */
     gotoxy( 60, 3 );
     printf( "BN = %2d, CT = %2.1f", brightness, contrast );
     do {
       switch( key )
       {
	 case UP :
	   if( contrast < 3.0 )
	   {
	     contrast += 0.1;
		  ChangeContrast();
	   } break;

	 case DOWN :
	   if( contrast > -2.0 )
	   {
	     contrast -= 0.1;
	     ChangeContrast();
		} break;
       }
       gotoxy( 60, 3 );
       printf( "BN = %2d, CT = %2.1f", brightness, contrast );
     } while( (key = getch()) != ESC );
  } /* If */
}

/*---------------------------------------*/
/*      THAY DOI MAU CHU ANH             */
/*---------------------------------------*/
void ChangeContrast()
{
  int i, j, k, m;
  long int s, tj;
  s = 0; tj = 0;

  for( i = 0; i < end_val; i++ )
    if( bnc[i].b_ness == brightness && bnc[i].c_trast == contrast )
    {
		curr_val = i;
      OutPutToScreen();
      return;	/* Thoat khoi vong lap */
    }

  /* Tinh average */
  fseek( fbmp, 118L, SEEK_SET );
  for( k = imgf.Height; k > 0; k--)
    for( m = 0; m < imgf.Width; m += 2 )
      {
	dbl.whole.allbyte = fgetc( fbmp );
	s += (bnc[curr_val].info[dbl.parts.hByte] +
	bnc[curr_val].info[dbl.parts.lByte]);
	tj += 2 ;
      }

  average = (float)s / (float)tj;
  end_val++;	/* tang gia tri end value */
  bnc[end_val].b_ness = brightness;
  bnc[end_val].c_trast = contrast;

  for( j = 0; j < 16; j++ )
  {
    bnc[end_val].info[j] =
    contrast*( (float)(bnc[curr_val].info[j] - average)) + average;
    if(bnc[end_val].info[j] > 15) bnc[end_val].info[j] = 15;
    if(bnc[end_val].info[j] < 0)  bnc[end_val].info[j] = 0;
  }

  curr_val = end_val;  /* Cap nhat gia tri cho current value */
  OutPutToScreen();
}

/*-------------------------------------*/
/*       HIEN THI ANH RA MAN HINH      */
/*-------------------------------------*/
void OutPutToScreen()
{
  int i, j;
  fseek( fbmp, 118L, SEEK_SET ); /* Dua con tro File ve dau File */
  // Quet lai anh cu
  for( i = imgf.Height; i > 0; i-- )
    for( j = 0; j < imgf.Width; j += 2 )
    {
      dbl.whole.allbyte = fgetc( fbmp );
      putpixel( j, i + 50, bnc[curr_val].info[dbl.parts.hByte] );
      putpixel( j + 1, i + 50, bnc[curr_val].info[dbl.parts.lByte] );
    }
}

/*---------------------------------------*/
/*       THAY DOI MAU NEN CUA ANH        */
/*---------------------------------------*/
void ChangeBrightNess(void)
{
   int i, j;

    for( i = 0; i < end_val; i++ )
      if( bnc[i].b_ness == brightness && bnc[i].c_trast == contrast )
      {
	curr_val = i;
	OutPutToScreen();
	return; /* Thoat */
      }

    end_val++;	/* tang gia tri end value */
    bnc[end_val].b_ness = brightness;
    bnc[end_val].c_trast = contrast;

    for( j = 0; j < 16; j++ )
    {
       bnc[end_val].info[j] = bnc[curr_val].info[j] + mbn;
       if( bnc[end_val].info[j] > 15 ) bnc[end_val].info[j] = 15;
       if( bnc[end_val].info[j] < 0 )  bnc[end_val].info[j] = 0;
    }

    curr_val = end_val;	/* Cap nhat gia tri cho current value */
    OutPutToScreen();
}

/*-------------------------------------*/
/*	MO FILE ANH BMP		       */
/*-------------------------------------*/
void OpenFile(char *file_source)
{
   int i, j, k;
   long int s, tjj;
   s = 0;          /* Dung cho average */
   tjj = 0;        /* Dung cho average */
   brightness = 0; /* Gia tri ban dau  */
   contrast = 1;   /* Gia tri ban dau  */

   for( i = 0; i < 961; i++ )
   {
      bnc[i].b_ness = 1000;
      bnc[i].c_trast = 1000.0;
   }
   signal_file = FALSE;
   while( !signal_file )
   {
		setfillstyle( SOLID_FILL, 0 );
      bar( 0, 22, 400, 49 );    	/* Hop thoai FileName           */
      bar( 450, 22, 635, 49 );	        /* Message                      */
      setcolor( 15 );
      rectangle( 0, 22, 400, 49 ); 	/* Khung bao hop thoai FileName */
      rectangle( 450, 22, 635, 49 );	/* Khung bao Message            */
      outtextxy( 10, 35, "Input File Name> " );
      outtextxy( 490, 35, "ENTER To Exit" );
      gotoxy( 19, 3 );
      gets( file_source );

      if( file_source[0] == NULL )
      {
	 bar( 0, 22, getmaxx(), 49 );
	 return; /* Go ENTER de thoat */
      }

      if( ( fbmp = fopen( file_source, "rb" ) ) == NULL )
      {
	 bar( 0, 22, 400, 49 );          /* Hop thoai FileName */
	 bar( 450, 22, 635, 49 );	 /* Message            */
	 rectangle( 450, 22, 635, 49 );  /* Khung bao Message  */
	 outtextxy( 470, 35, "Can not open file" );
	 getch();
      }
      else {
	 signal_file = TRUE;	            /* Da mo duoc file   */
	 strcpy( file_target, file_source); /* Sao ban luu file  */
	 bar( 450, 22, 635, 49 );           /* Xoa Message       */
	 rectangle( 450, 22, 635, 49 );     /* Khung bao Message */
	 outtextxy( 472, 35, "Processed Picture" );
	 /* Xoa phan hien thi anh cu */
	 setfillstyle( SOLID_FILL, 0 );
	 bar( 0, 50, getmaxx(), getmaxy() );
	 gotoxy(1,7);
	 /* Hien thi thong tin ve anh */
	 DisplayInfo( &fh, &imgf, fbmp );
	 /* Xoa phan hien thi thong tin */
	 setfillstyle( SOLID_FILL, 0 );
	 bar( 0, 50, getmaxx(), getmaxy() );
	 fseek( fbmp, 64L, SEEK_CUR );
	 /* Nap anh va hien anh "Big BMP 16 colors" */
	 for( i = imgf.Height; i > 0; i-- )
	   for( j = 0; j < imgf.Width; j += 2 )
	   {
	      dbl.whole.allbyte = fgetc(fbmp);
	      putpixel( j, i + 50, dbl.parts.hByte );
	      putpixel( j + 1, i + 50, dbl.parts.lByte );
	      bnc[curr_val].info[(int)dbl.parts.hByte] = (int)dbl.parts.hByte;
	      bnc[curr_val].info[(int)dbl.parts.lByte] = (int)dbl.parts.lByte;
	      /* Tinh average */
	      s += ( dbl.parts.hByte + dbl.parts.lByte );
	      tjj += 2;
	   }  /* For */
	 /* Gia tri trung binh anh */
	average = (float)(s) / (float)(tjj);
      } /*  Else */
   } /* While */
} /* End of Open */

/*-------------------------------------------------*/
/*      	HIEN THI MAU Histogram	           */
/*-------------------------------------------------*/
void Histo(char *file_source)
{
   if( signal_file )
   {  /* Da mo File */
      gotoxy( 60, 3 );
      printf( "BN = %2d, CT = %2.1f", brightness, contrast );
      do { Histogram( file_source );
      } while( (key = getch()) != ESC );
   } /* If */
}

/*-------------------------------------------------*/
/*     	     HIEN THI BAN DO Histogram	           */
/*-------------------------------------------------*/
void Histogram(char *fn)
{
   int ncolor, maxx, maxy, i, y0, x0, sp, dy, w, p1[768];
	float Histogr[256];
   char s[3];

   HistogramfileBMP( fn, Histogr, &ncolor, p1 );
   cleardevice();
   for( i = 0; i < 16; i++ )
   WriteColorReg( i, p1[i*3], p1[i*3+1], p1[i*3+2] );
   /* Trang tri bieu do */
   maxx = getmaxx();
   maxy = getmaxy();
   x0 = maxx / 30;
   y0 = maxy - maxy / 20;
   dy = y0 - 300;
   line( x0, 250, x0, y0 );        /* Duong doc   */
   line( x0, y0, maxx - 200, y0 ); /* Duong ngang */
   setbkcolor( 0 );
   setcolor( YELLOW );
   settextjustify( 1, 1 );
   settextstyle( 0, 0, 3 );
   outtextxy( 330, 120, "Bieu Do HISTOGRAM" );
	setlinestyle( 4, 0x10, 3 );
   settextjustify( 1, 1 );
   settextstyle( 3, 0, 7 );

   for( i = 0; i < 8; i++ )
     line( x0+1, y0-i*dy / 5.5, maxx - 200, y0-i*dy / 5.5 );
   setlinestyle( 0, 7, 1 ); /* Duong ngang cham cham */
   if( ncolor == 16 )
   { sp = 5; w = 20; }
   else { sp = 20; w = 20; }
   x0 = sp + w;

   for( i = 0; i < ncolor; i++ )
     if( ncolor == 16 )
     {
	 setfillstyle( 1, i == 0 ? 15 - i : i % 16 );
	 /* Bieu do Histor cho anh 16 mau */
	 bar( i * (w+sp) + x0 - 2, (y0 - Histogr[i] * dy ) / 1.1,
	 i * (w+sp) + x0 + 20, y0 );
     }
	  else {
       setfillstyle( 1, i );
       /* Bieu do Histor cho anh 256 mau */
       bar3d( i * (w+sp) + x0, (y0 - Histogr[i] * dy ) / 1.5,
       i * (w+sp) + w + x0, y0 + 30, 7, 1 );
     }
   getch();
   cleardevice();
   OutPutToScreen(); // Hien lai anh cu sau lan cap nhat cuoi cung
   puts("Press any key to return system menu.");
   getch();
   main_menu();
}

/*--------------------------------------------------------*/
/*	    HIEN THI MAU CUA ANH BMP                      */
/*--------------------------------------------------------*/
void HistogramfileBMP(char fn[MAXFN], float Hist[],
int *numcolor, int palette[])
{
	FILEHEADER  fh;
   BMPINFO imgf;
   FILE  *fbmp;
   IMAGE pic;
   PALENTRY color;
   int i,j,x,y;
   long int iHist[256],s;

   clrscr();
   if( ( fbmp = fopen( fn,"rb" ) ) == NULL )
   {
      printf( "Cant not open file\n" );
      exit( 1 );
   }

  fread( &fh, 1, sizeof( fh ), fbmp );
  if( fh.Id != 0x4d42 )
  {
     fclose(fbmp);
     printf( "This is not BMP file" );
	  exit( 1 );
  }
  fread( &imgf, 1, sizeof( imgf ), fbmp );
  if( imgf.NumBit == 4 ) *numcolor = 16;
  else if( imgf.NumBit == 8 ) *numcolor = 256;
       else *numcolor=2;

  /* Nap noi dung cho 16 palette mau */
  for( i  =0; i < *numcolor; i++ )
  {
     color.blue     = fgetc( fbmp ) >> 2;
     color.green    = fgetc( fbmp ) >> 2;
     color.red      = fgetc( fbmp ) >> 2;
     color.reserve  = fgetc( fbmp );
     palette[i*3]   = color.red;
     palette[i*3+1] = color.green;
     palette[i*3+2] = color.blue;
   }

  for( i = 0; i < *numcolor; i++ ) iHist[i] = 01;
  s = 0L;

  for( y = 0; y < imgf.Height; y++ )
    for( x = 0; x < imgf.Width; x+=2 )
      if( *numcolor == 256 )
      {
	dbl.whole.allbyte = fgetc( fbmp );
	j = (int)dbl.whole.allbyte;
	(iHist[j])++;
	s++;
	dbl.whole.allbyte = fgetc( fbmp );
	j = (int)dbl.whole.allbyte;
	( iHist[j] )++;
	s++;
      }
      else {
	dbl.whole.allbyte = fgetc( fbmp );
	j = (int)dbl.whole.allbyte;
	( iHist[j] )++;
	s++;
	j = (int)dbl.whole.allbyte;
	( iHist[j] )++;
	s++;
      }
  for( i = 0; i < *numcolor; i++ ) Hist[i] = (float)iHist[i] / (float)s;
}

/*------------------------------------------------------*/
/*	       LOAD MAU CUA ANH BMP                     */
/*------------------------------------------------------*/
void WriteColorReg( int color, int red, int green, int blue )
{
   union REGS regs;
   regs.x.ax = 0x1010;
   regs.x.bx = color;
   regs.h.ch = green;
   regs.h.cl = blue;
   regs.h.dh = red;
   int86( 0x10, &regs, &regs );
}

/*-------------------------------------*/
/*     THUC HIEN CAC CHUC NANG MENU    */
/* ------------------------------------*/
void Process( int select )
{
   switch( select )
   {
      case 0  : if( signal_file ) fclose( fbmp ); OpenFile(file_source); break;
      case 1  : Histo(file_source); break;
      case 2  : BrightNess(); break;
      case 3  : Contrast(); break;
      case 4  : fclose( fbmp ); closegraph(); say_goodbye(); break;
      default : break;
   }
}

/*--------------------------------*/
/*   TAO CAC HOP SANG TREN MENU   */
/*--------------------------------*/
void Window( int k, char *ch[] )
{
  setfillstyle( SOLID_FILL, LIC ); /* Tao hop sang cho 1 o */
  bar( 127 * k + 1, 1, 127 * (k + 1), 19 );
  setcolor( LIGHTRED );            /* Hien thi text trong thanh bar */
  outtextxy( 127 * k + 20, 8, ch[k] );
  key = getch();                   /* Co thoat khoi trinh don */
  setfillstyle( SOLID_FILL, BAKC );
  bar(127 * k + 1, 1, 127 * (k + 1), 19 );
  setcolor(WHITE);                 /* Tao khung cho 1 o */
  rectangle( 127 * k, 0, 127 * (k + 1), 20 );
  setcolor( YELLOW );
  outtextxy( 127 * k + 20, 8, ch[k] );
}

/*------------------------------------------------------*/
/*     	     HIEN THI THONG TIN VE ANH                  */
/*------------------------------------------------------*/
void DisplayInfo( FILEHEADER *fh, BMPINFO *imgf, FILE *fbmp )
{
  fread(fh, 1, sizeof(*fh), fbmp);
  printf("About Informations :\n\r");
  printf("BMP Id %4x\n\r",fh->Id);
  fread(imgf, 1, sizeof(*imgf), fbmp);
  printf("BMP size %ld\n\r",imgf->BmpSize);
  printf("BMP width %ld\n\r",imgf->Width);
  printf("BMP height %ld\n\r",imgf->Height);
  printf("Press <ENTER> to continue\n\r");
  getch();
}

/*--------------------------------------------*/
/*       HAM TOA GIAO DIEN MAN HINH	      */
/*--------------------------------------------*/
void Interface()
{
  word h;
  StartGraphics();
  setcolor( YELLOW );
  settextstyle( 0, 0, 4 );
  h = textheight( "M" );
  OutTextShade( 15, 25, "NHAN DANG XU LY ANH", h/6, WHITE, LIGHTRED, 1 );
  PutPixelPlay();
  RotateCircle();
  getch();
  closegraph();
}

/*--------------------------------------------------------*/
/* Function : read_key                                    */
/* Mission  : Read a key from the keyboard                */
/* Expects  : (ch) get the key from the keyboard          */
/* Returns  : If the key is a extend key then return code */
/*	      key and 0 value else return 1 value and code*/
/*            key one                                     */
/*--------------------------------------------------------*/
char read_key(char &ch)
{
   union REGS regs;
   regs.h.ah = 0; int86(22, &regs, &regs);
	if(!(regs.h.al)) {
      ch = regs.h.ah; return FALSE;
   }
   else ch = regs.h.al; return TRUE;
}

/*--------------------------------------------*/
/*       KHOI TAO HE THONG DO HOA	      */
/*--------------------------------------------*/
void StartGraphics()
{
   char PathToDriver[19];
   int GraphDriver, GraphMode, ErrorCode;

   strcpy(PathToDriver, disk);
	strcat(PathToDriver, "D:\\BORLANDC\\BGI");
   detectgraph(&GraphDriver, &GraphMode);
   initgraph(&GraphDriver, &GraphMode, PathToDriver);
   ErrorCode = graphresult();
   if(ErrorCode != grOk) {
		clrscr();
      puts("Graphics system error");
      puts(grapherrormsg(ErrorCode));
      exit(1);
   }
}

/*--------------------------------------------*/
/*     CAC MODULE TAO GIAO DIEN MAN HINH      */
/*--------------------------------------------*/
/* Demonstrate the putpixel and Getpixel commands */
void PutPixelPlay()
{
 int i, X, Y;
 randomize();
 /* Putpixelplay */
 for( i = 0; i <= 2000; i++ )
 putpixel( random( getmaxx() + 1 ), random( getmaxy() + 1 ), random( getmaxcolor() + 1 ) );
 setlinestyle( 0, 0, 3 );
 rectangle( 1, 1, 638, 478 );
 setlinestyle( 0, 0, 0 );
 setcolor( YELLOW );
 for( i = 0; i <= 12; i++ )
 ellipse( 500, 100, 268, 92, 25 - i, 25 );
 /* PutStart */
 settextstyle( 0, 0, 1 );
 i = 0;
 do
 { /* Plot random pixels */
    setcolor( getmaxcolor() );
    X=random( getmaxx() + 1 );
    Y=random( getmaxy() + 1 );
    if( Y > 100 )
    {
       i++;
       outtextxy( X, Y, "/x2E" );
       delay( 500 );
       outtextxy( X, Y, "\xFA" );
       delay( 500 );
       outtextxy( X, Y, "\xF9" );
		 delay( 500 );
       outtextxy( X, Y, "\x0F" );
       delay( 500 );
       setcolor( BLACK  );
       outtextxy( X, Y, "\x0F" );
       outtextxy( X, Y, "\xF9" );
     }
 } while( i < 5 );
} /* PutPixelPlay */

/*--------------------------------------------*/
/*          HAM CHO QUAY VONG TRON	      */
/*--------------------------------------------*/
void RotateCircle()
{
  int h, i, j, d = 20;

  for( j = 0; j < 3; j++) {
    setfillstyle( LINE_FILL, LIGHTGREEN );
    for( i = 60; i >= 0; i-- ) {
		setcolor( YELLOW );
      fillellipse( 200, d + 100, i, 60 );
      setcolor( 0 );
      fillellipse( 200, d + 100, i, 60 );
    }

    for( i = 60; i >= 0; i--) {
      setcolor( YELLOW );
      fillellipse( 200, d + 100, i, 60 );
      setcolor( 0 );
      fillellipse( 200, d + 100, i, 60 );
    }
  }

  fillellipse( 200, d + 100, i, 60 );
  setcolor( LIGHTGRAY );

  for( i = 1; i < 10; i++ ) {
    delay( 100 );
    settextjustify( 1, 1 );
	 settextstyle( 2, 0, i );
    outtextxy( 200 + i, d + 70 + i, "De Tai" );
    outtextxy( 200 + i - 1, d + 105 + i, "BIEN DOI ANH KI DI ");
  }

  WriteShadow( 200 + i - 1, d + 70 + i - 1, "DeTai", 1, 15, 2, 9 );
  WriteShadow( 200 + i - 1, d + 105 + i - 1, "BIEN DOI ANH KI DI",
  4, 15, 2, 9 );
  h = textheight( "M" );
  OutTextShade( 320, getmaxy() - 90, "Thu Hien : NGUYEN NGOC VAN",
  h / 6, LIGHTMAGENTA, LIGHTCYAN, 1 );
  OutTextShade( 320, getmaxy() - 40, "Lop 00DTH01", h/6, LIGHTRED,
  YELLOW, 1 );
}/* RoatCircle */

/*--------------------------------------------*/
/*          HAM VE CHU CO BONG CANH	      */
/*--------------------------------------------*/
void WriteShadow( int toadoX, int toadoY, char STR[], int Mb, int Mn, int kchu, int kthuoc )
{
  int i;
  settextjustify( 1, 1 );      /* Canh giua */
  settextstyle( kchu, 0, kthuoc );

  for(i = 1; i <= 8; i++) {
    delay(10);
    if(i <= 5) setcolor(Mb); /* Mb la mau bong cua chu */
    else setcolor(Mn);	       /* Mn la mau noi cua chu  */
    outtextxy(toadoX+i, toadoY, STR);
  }
}

/*----------------------------------------------------------*/
/*               HAM VIET CHU CO BONG CANH	            */
/*----------------------------------------------------------*/
void WriteShade( int x, int y, char s[], int dobong, int mau )
{
  int cl, i;

  cl = getcolor();
  setcolor( mau );
  for( i = 1; i < dobong; i++ )
  outtextxy( x+i, y+i, s );
  setcolor( cl );
  outtextxy( x, y, s );
}

/*--------------------------------------------------*/
/*	      XAC DINH DO RONG CUA CHUOI	    */
/*--------------------------------------------------*/
void OutTextWidth(int x, int y ,char *string, int w)
{
   int i;

   outtextxy(x, y, string);
   for(i = 1; i <= w; i++) {
      outtextxy( x + i, y, string );
      outtextxy( x - i, y, string );
   }
}

/*--------------------------------------------------*/
/*   HAM VE MOT CHUOI CO BONG CANH TAI TOA DO x, y  */
/*--------------------------------------------------*/
void OutTextShade(int x, int y, char *string, int shade, int color1, int color2, int w)
{
   int i;

   setcolor( color2 );
   for( i = shade; i > 0; i-- )
     OutTextWidth( x + i, y + i, string, w );
   setcolor( color1 );
   OutTextWidth( x, y, string, w );
}

// Check Password
void get_password()
{
  char Key, PassW1[11], PassW2[11], buffer[768], is_ascii;
  byte Flag, j, i = 0, select = 0, col = 0, row = 0;
  strcpy(PassW1,sys_info[114]);
  set_cursor_size(0x2020);
  set_text_attrib(0x13);
  set_border_color(25);
  set_char_width(8);
  clrscr();
  movedata(0xF000,0xFA6E,_DS,(unsigned)table,sizeof(table));
  do_blinking(0);
  fill_frame(1,1,80,25,0xFD,178);
  fill_frame(3,2,78,12,0x4E,179);
  put_box(3,2,78,12,0xF0,3);
  set_text_attrib(0x4E);
  put_big_STR(4,"PICTURES");
  box_shadow(20,16,58,22,0x7F,2,0x4F,sys_info[172],1);
  write_VRAM(22,18,0x7A,sys_info[170]);
  write_VRAM(43,18,0x11,"          ");
  button(36,20,0xE0,7,sys_menu[0],1,0xE4);
  gettext(20,16,60,23,buffer);
  init_mouse();
  set_pmcl();
  do {
    i = 0;
    Key = 0;
    puttext(20,16,60,23,buffer);
    show_mouse();
    do {
       gotoxy(43 + i, 18);
       if(kbhit()) {
	  is_ascii = read_key(Key);
	  if(!Key) is_ascii = read_key(Key);
	  if((isalpha(Key) || isdigit(Key)) && is_ascii && i < 10) {
	     write_XY(43 + i, 18, 0x1F, 42); PassW2[i++] = Key;
	  }
	  if(Key == 8 && i > 0) write_XY(43 + --i, 18, 0x1F, 32);
       }
       if(click_mouse(&col, &row) == 1) {
	  if(row == 20 && col >= 36 && col <= 41) {
	     hide_mouse();
	     clear(36,20,42,21,7);
	     write_VRAM(37,20,0xF0,sys_menu[0],0xF4);
		  delay(50);
	     button(36,20,0xE0,7,sys_menu[0],1,0xE4);
	     show_mouse();
	     Key = ENTER;
	  }
	  if(row == 16 && col == 23) {
	     hide_mouse();
	     write_XY(23,16,0x7A,15);
	     delay(50);
	     write_XY(23,16,0x7A,254);
	     show_mouse();
	     Key = ENTER;
	  }
       }
    } while(Key != ENTER);
    PassW2[i] = NULL;
    hide_mouse();
    if(strcmp(PassW1,PassW2)) {
      box_shadow(18,16,60,22,0x7F,2,0x4F,sys_info[171],1);
      write_VRAM(29,18,0x71,sys_info[167]);
		button(26,20,0xF0,7,sys_menu[23],1,0xF4);
      button(43,20,0xE0,7,sys_menu[6],1,0xE4);
      show_mouse();
      key = 0;
      do {
	 if(click_mouse(&col, &row) == 1) {
	    if(row == 20 && col >= 26 && col <= 34) {
	       hide_mouse();
	       button(43,20,0xE0,7,sys_menu[6],1,0xE4);
	       clear(26,20,35,21,7);
	       write_VRAM(27,20,0xF0,sys_menu[23],0xF4);
	       delay(50);
	       button(26,20,0xF0,7,sys_menu[23],1,0xF4);
	       show_mouse();
	       select = 0;
	       key = ENTER;
	    }
	    if(row == 20 && col >= 43 && col <= 50) {
	       hide_mouse();
	       button(26,20,0xE0,7,sys_menu[23],1,0xE4);
			 clear(43,20,51,21,7);
	       write_VRAM(44,20,0xF0,sys_menu[6],0xF4);
	       delay(50);
	       button(43,20,0xF0,7,sys_menu[6],1,0xF4);
	       show_mouse();
	       select = 1;
	       key = ENTER;
	    }
	    if(row == 16 && col == 21) {
	       hide_mouse();
	       write_XY(21,16,0x7A,15);
	       delay(50);
	       write_XY(21,16,0x7A,254);
	       exit_program();
	    }
	 }
	 if(kbhit()) {
	    key = getch();
	    if(!key) key = getch();
	    switch(key) {
			 case LEFT:
		  button(26+select*17,20,0xE0,7,sys_menu[23-17*select],1,0xE4);
		  if(select <= 0) select = 0; else select--;
		  button(26 + select*17, 20, 0xF0, 7, sys_menu[23-17*select], 1, 0xF4);
		  break;
	       case RIGHT:
		  button(26 + select*17, 20, 0xE0, 7, sys_menu[23-17*select], 1, 0xE4);
		  if(select >= 1) select = 1;
		  else select++;
		  button(26 + select*17, 20, 0xF0, 7, sys_menu[23-17*select], 1, 0xF4);
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
      box_shadow(18,10,60,14,0x5F,2,0xF0,sys_info[171],1);
      write_VRAM(30,12,0x5E,sys_info[168]);
      write_VRAM(24,13,0x5A,sys_info[169]);
      getch();
    }
  } while(strcmp(PassW1, PassW2));
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
  box_shadow(14,6,69,17,0x3E,2,0x4F,sys_info[115]);
  write_VRAM(27,15,0x3F,sys_info[116]);
  write_VRAM(16, 8,0x30,sys_info[117]);
  write_VRAM( 16, 9, 0x30,sys_info[118]);
  write_VRAM( 16, 10,0x30,sys_info[119]);
  write_VRAM( 16, 11,0x30,sys_info[120]);
  write_VRAM( 16, 12,0x30,sys_info[121]);
  write_VRAM( 16, 13,0x30,sys_info[122]);
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
   static byte bound[4][6] = // table code to format the frame
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
char *replicate(char Symbol,byte Times)
{
  char *Aux;
  byte k;

  Aux = (char *)malloc( Times + 1 );
  for( k = 0; k < Times; Aux[k++] = Symbol );
  Aux[Times] = '\0';
  return Aux;
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
      poke(0xB800, video_offset , (attr << 8) + chr);
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
   set_char_width(9);       // Restore to old font
   do_blinking(1);          // Setting the standar video model
   if(mouse_avalid)         // Initialize successful ?
   close_mouse();           // Closing mouse function
   del_mem();               // Release block memory
   clrscr();                // Clear the screen result
   exit(EXIT_SUCCESS);      // Terminated the program body
}

// Take out characters from possition
char *Copy(char *dest, int pos, int nchar)
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
void paint_big_char(CHARS entry)
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

   st = Copy(msg, 1, 10);
   len = strlen(st);
   start_column = (( 80 - 8 * len ) / 2) & 0x00FF;
   for(n = 0; n < len; n++) {
      curr_column = start_column + 8 * n + 2 ;
      window(curr_column,start_row,curr_column + 7,start_row + 8);
      paint_big_char(table[st[n]]);
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
   FILE *fptr;
   INFO tmp;
   byte num_regs;

   if(!(fptr = fopen("C:\\WINDOWS\\register.dat", "rb"))) {
      clrscr(); printf("Error loading data file... System halt."); exit(1);
   }
   fread(&tmp,sizeof(INFO),1,fptr);
   fclose(fptr);
   strcpy(disk,tmp.disk);
   return tmp.regs ? TRUE : FALSE;
}

/*------------------------------------*/
/* Funtion : Decode                   */
/* Mission : Decode file sysinfor.sys */
/* Expects : (file_name) file name    */
/* Returns : Nothing                  */
/*------------------------------------*/
void decode(const char *in_file, const char *out_file)
{
   FILE *fp_in, *fp_out;
   int c, key = 100;

   fp_in = fopen(in_file,"rb"); fp_out = fopen(out_file,"wb");
   if(!fp_in || !fp_out) {
      clrscr(); printf("Error loading file... System halt.",in_file); exit(1);
   }
   while((c = fgetc(fp_in)) != EOF) {
      c = c - ~key; fputc(c, fp_out);
   }
   fclose(fp_in); fclose(fp_out);
}

/*------------------------------------------------*/
/* Function : get_file_data                       */
/* Mission  : Reading information into data array */
/* Expects  : (inf_file) the input file           */
/*            (out`_file) the output file          */
/*            (data) the array data               */
/*            (num) the number of elements        */
/* Returns  : Nothing                             */
/*------------------------------------------------*/
void get_file_data(const char *in_file, const char *out_file, char **&data, word &num)
{
   FILE *fp;
   char buffer[80];

   decode(in_file, out_file); fp = fopen(out_file,"rt");
   while(fgets(buffer,80,fp)) num++;
   if(!(data = new char*[num])) {
      clrscr(); printf("Not enough memory!"); exit(1);
   }
   num = 0; rewind(fp);
   while(fgets(buffer,80,fp)) {
      if(!(data[num] = new char[strlen(buffer)+1])) {
	 clrscr(); printf("Not enough memory at pointer %u", num); exit(1);
      }
      buffer[strlen(buffer)-1] = '\0'; strcpy(data[num],buffer); num++;
   }
   fclose(fp); unlink(out_file);
}

/*------------------------------------------*/
/* Function : del_mem                       */
/* Mission  : free block memory of the data */
/* Expects  : Nothing                       */
/* Returns  : Nothing                       */
/*------------------------------------------*/
void del_mem()
{
   word i;
   for(i = 0; i < info_num; i++) delete(sys_info[i]);
   for(i = 0; i < menu_num; i++) delete(sys_menu[i]);
   delete(sys_menu); delete(sys_info);
}

/*----------------------------------------------*/
/* Function : initialize_program                */
/* Mission  : Initialize parameters for program */
/* Expects  : Nothing                           */
/* Returns  : Nothing                           */
/*----------------------------------------------*/
void initialize_program()
{
   char src_path[32], dest_path[32];
   FILE *fp;
   INFO tmp;
   if(!(fp = fopen("C:\\WINDOWS\\register.dat","r+b"))) {
      clrscr(); printf("Error loading data file... System halt."); exit(1);
   }
   fread(&tmp,sizeof(INFO),1,fp);
   if(tmp.num < 20) {
      tmp.num++; fseek(fp,0L,SEEK_SET); fwrite(&tmp,sizeof(INFO),1,fp);
   }
   fclose(fp); strcpy(disk,tmp.disk); strcpy(src_path,disk);
   strcat(src_path,"TOPICS\\SYSTEM\\sysinfor.sys"); strcpy(dest_path,disk);
   strcat(dest_path,"TOPICS\\SYSTEM\\sysinfor.txt");
   get_file_data(src_path,dest_path,sys_info,info_num);
   src_path[17] = dest_path[17] = NULL;
   strcat(src_path,"sysmenus.dll"); strcat(dest_path,"sysmenus.txt");
   get_file_data(src_path,dest_path,sys_menu,menu_num);
}

/*----------------------------------------------------*/
/* Function : set_char_width                          */
/* Mission  : setting the character width with 8 or 9 */
/* Expects  : (hwidth) the width of the character     */
/* Returns  : Nothing                                 */
/*----------------------------------------------------*/
void set_char_width(byte hwidth)
{
   union REGS regs;
   byte x;
   regs.x.bx = (hwidth == 8) ? 0x0001 : 0x0800;
   x = inp(0x3CC) & (255 - 12);
   if(hwidth == 9) x |= 4; outp(0x3C2, x);
   _disable();
   outpw(0x3C4,0x0100);
   outpw(0x3C4,0x01 + (regs.h.bl << 8));
   outpw(0x3C4,0x0300);
   _enable();
   regs.x.ax = 0x1000;
   regs.h.bl = 0x13;
   int86(0x10, &regs, &regs);
}
/*---------------------------------------------------------*/
/*------------------END OF THE PROGRAM---------------------*/
/*---------------------------------------------------------*/
