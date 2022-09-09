/*--------------------------------------------------------------*/
/*        UNIVERSITY OF TECHNOLOGY HO CHI MINH CITY             */
/*             MAJOR OF INFORMATIC TECHNOLOGY                   */
/*           PROCCESSING DIRECTORY ON HARD DISK                 */
/*             Author : NGUYEN NGOC VAN                         */
/*              Class : 00DTH1                                  */
/*       Student Code : 00DTH201                                */
/*             Course : 2000 - 2005                             */
/*      Writting Date : 24/10/2001                              */
/*        Last Update : 12/11/2001                              */
/*--------------------------------------------------------------*/
/*        Environment : Borland C++ Ver 3.1 Application         */
/*        Source File : PROCDIRS.CPP                            */
/*        Memory Mode : Small                                   */
/*            Compile : BCC PROCDIRS.CPP                        */
/*        Call to run : PROCDIRS (none the project file)        */
/*--------------------------------------------------------------*/

#include <graphics.h>
#include <direct.h>
#include <conio.h>
#include <stdio.h>
#include <dos.h>
#include <io.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <alloc.h>

#define OFFSET(x,y) ((x-1)*2+(y-1)*160)
#ifndef MK_FP
#define MK_FP(seg,ofs) ((void far*)((unsigned long)(seg) << 16 | (ofs)))
#endif

#define FA 0
#define VR 1
#define ENTRE 14	   /* So de muc cua mot man hinh   */
#define EZ (20-ENTRE >> 1) /* Dong dau tien cua mot cua so */
#define NOF 0x1F	   /* Chu trang tren nen xanh      */
#define INV 2		   /* Chu den tren nen trang       */
#define ENTER  13
#define ESC    27
#define LEFT   75
#define RIGHT  77
#define HOME   71
#define END    79
#define WIDTH  8
#define HIGH   14
#define DOUBLE 2
#define FILL   3
#define TRUE   1
#define FALSE  0

typedef unsigned char byte;
typedef unsigned int word;
char ATTR = 0x07; /* Mau chuan ban dau */
/* Lay dia chi thuc cua bo nho man hinh */
char far *buffer = (char far *)MK_FP(0xB800,0x0000);
void far *ptr[1];
char *str, *file_name; /* Duong dan den cac tap tin */
unsigned int attr;
struct text_info info;
byte mouse_avalid = FALSE;
typedef struct struct_dir
{ /* Cau truc thong tin cua mot file */
   byte reserve[21];
   byte attribut;      /* Thuoc tinh cua file */
   word hour;          /* Gio tao file        */
   word date;          /* Ngay tao file       */
   unsigned long size; /* Kich thuoc file     */
   char name[13];      /* Ten file            */
   byte state;         /* Trang thai cua file */
} DIRS;

/*------------------------------------------------------*/
/*             Khai bao cac ham cua chuong trinh        */
/*------------------------------------------------------*/
byte get_opt_first(char*, word);
int test_dir(DIRS );
int make_dir(char*);
int dele_dir(char*);
int rename_file(char*, char *);
char *replicate( char, byte );
char *get_driver(void);
char read_key(char *);
char dele_file(char *);
char set_disk(char );
void set_cursor(byte, byte);
void set_attrib(byte, byte, byte, byte, byte, byte);
void write_VRAM(byte, byte, word , const char *, word letter_attr = 0);
void box_shadow(byte, byte, byte, byte, word, byte, word, char *, byte is_win = 0);
void set_border_color(byte);
void putch_XY( byte, byte, char);
void frame(byte, byte, byte, byte, byte);
void set_pos(int, int);
void get_pos(char*, char*);
void scroll_up(int, int, int, int, int, int);
void scroll_down(int, int, int, int, int ,int);
void write_BIOS(int, int, char*, int);
void lineto(int,int, char, int);
void frame_color(int, int, int, int, int, int);
void print_data(DIRS* , byte);
long file_size(FILE *stream);
void set_DTA(DIRS* );
void save_to_array(char*, int, DIRS*, int*);
void dir(char*, int, DIRS*, int*);
void write_str_attr(char*, int, int, int, int, int);
void copy_file(char* , char*);
void outtext_width(int, int, char*, int);
void outtext_shade(int, int, char*, int, int, int, int);
void exec_driver(byte);
void exec_attrib(byte);
void exec_register(byte);
void set_attrib(char*, word);
byte get_page(void);
byte get_opt_next(void);
byte get_text_color(void);
byte get_back_color(void);
void driver(void);
void clear_screen(void);
void screen_information(void);
void copyright(void);
void menu_driver(void);
void menu_attrib(void);
void attribute(void);
void mess_copy_file(void);
void mess_make_dir(void);
void mess_dele_dir(void);
void mess_dele_file(void);
void change(void);
void introduce(void);
void start_graphics(void);
void end_program(void);
void check_period(void);
char check_register(void);
void registers(void);
void register_now(void);
void showing_menu_register(void);
void fadein(void);
void set_text_attrib(byte);

/*--------------------------------*/
/*      Chuong trinh chinh        */
/*--------------------------------*/
void main()
{
   char *disk_letter, strt[64], disk, ch = 0, t,
   record = 0, direc[64], key, buff[256];
   int center, n, row, i;
   DIRS entry[224];
   FILE *fp;

   str = (char *)calloc(64, sizeof(char));
   disk_letter = (char *)calloc(30, sizeof(char));
   file_name = (char *)calloc(64, sizeof(char));
   if(check_register() == -1) {
      registers();
      check_period();
   }
   introduce();
   disk = 'D';
   str[0] = '\0';
   set_disk(disk);
   chdir("D:\\");
   resume:
   getcwd(str,64);
   strcat(str,"*.*");
   set_cursor(11,10);
   set_border_color(63);
   dir(str,0xFF,entry,&n);
   while(1)
   {
     i = 0;
     row = 6;
     dir(str,0xFF,entry,&n);
     write_VRAM(5,row,0x3F,entry[i].name);
     write_VRAM(2,22,0x4F,replicate(' ',78));
     print_data(&entry[i],21);
     while(1)
     {
       center = 40 - (strlen(str) / 2);
       write_VRAM(center,1,0x70,str);
       if( !read_key(&ch) )
       {
	  switch(ch)
	  {
	     case 67 : // Phim F9
		menu_driver();
		clrscr();
		goto resume;

	     case 80 : /* Phim mui ten xuong */
		if(entry[i].state)
		   write_VRAM(5,row,0x1E,entry[i].name);
		else
		   write_VRAM(5,row,0x1F,entry[i].name);
		if((i < n-1) && ( row < 19))
		{
		   row++; i++;
		   write_VRAM(5,row,0x1F,entry[i].name);
		}
		if((row == 19) && (i < n-1))
		{
		   i++;
		   scroll_up(1,0x1F,4,5,70,18);
		   print_data(&entry[i],row-1);
		}
		set_pos(5,22);
		write_VRAM(2,22,0x4F,replicate(' ',78));
		write_VRAM(5,row,0x3F,entry[i].name);
		print_data(&entry[i],21);
		break;

	     case 72 : /* Phim mui ten len */
		if(entry[i].state)
		   write_VRAM(5,row,0x1E,entry[i].name);
		else
		   write_VRAM(5,row,0x1F,entry[i].name);
		if((i > 0) && ( row > 6))
		{
		   row--; i--;
		   write_VRAM(5,row,0x1F,entry[i].name);
		}
		if((row == 6) && (i > 0))
		{
		   i--;
		   scroll_down(1,0x1F,4,5,70,18);
		   print_data(&entry[i],row-1);
		}
		set_pos(5,21);
		write_VRAM(2,22,0x4F,replicate(' ',78));
		write_VRAM(5,row,0x3F,entry[i].name);
		print_data(&entry[i],21);
		break;

	     case 82 : /* Phim insert */
		if( entry[i].state ) /* Dang o trang thai insert */
		{
		   entry[i].state = 0;
		   write_VRAM(5,row,0x1F,entry[i].name);
		   if((row == 19) && (i < n-1))
		   {
		      ++i;
		      scroll_up(1,0x1F,4,5,70,18);
		   }
		   if((i < n-1) && (row < 19))
		   {
		      ++i;
		      ++row;
		   }
		   print_data(&entry[i],row-1);
		   write_VRAM(5,row,0x3F,entry[i].name);
		}
		else { /* Chua o trang thai insert */
		  entry[i].state = 1; /* Bat trang thai Insert */
		  write_VRAM(5,row,0x1E,entry[i].name);
		  if((row == 19) && (i < n-1))
		  {
		     ++i;
		     scroll_up(1,0x1F,4,5,70,18);
		  }
		  if((i < n-1) && (row < 19))
		  {
		     ++i;
		     ++row;
		  }
		  print_data(&entry[i],row-1);
		  write_VRAM(5,row,0x3F,entry[i].name);
		} /* If Insert */
		set_pos(5,22);
		print_data(&entry[i],21);
		break;

	     case 68 : /* F10 - Thoat chuong trinh */
		set_cursor(0x20,0x20);
		box_shadow(15,10,62,14,0x4E,1,0x4A," Exit program ");
		write_BIOS(19,11,"Do you want to exit program now (Y/N)? ",0x4F);
		if( toupper( getch() ) == 'Y' ) end_program();
		else
		{
		  clrscr();  /* Khong dong y thoat chuong trinh */
		  set_cursor(11,10);
		  dir(str,0xFF,entry,&n);
		  write_VRAM(5,row,0x3F,entry[i].name);
		  write_VRAM(center,1,0x70,str);
		}
		break;

	     case 104 :   /* Phim Alt-F1 xem thong tin o dia */
		driver();
		if(ch == 13)
		clrscr(); /* Phim enter */
		dir(str,0xFF,entry,&n);
		write_VRAM(5,row,0x3F,entry[i].name);
		write_VRAM(center,1,0x70,str);
		break;

	     case 60 :   /* Phim F2 */
		copyright();
		if(ch == 27)
		clrscr();  /* Phim ESC */
		dir(str,0xFF,entry,&n);
		write_VRAM(5,row,0x3F,entry[i].name);
		write_VRAM(center,1,0x70,str);
		break;

	     case 62 :  /* Phim F4 Thay doi thuoc tinh */
		attribute();
		if(ch== 13)
		clrscr();  /* Phim enter */
		dir(str,0xFF,entry,&n);
		write_VRAM(5,row,0x3F,entry[i].name);
		write_VRAM(center,1,0x70,str);
		break;

	     case 65 :   /*phim F7 Tao thu muc */
		mess_make_dir();
		if(getch() == 27)
		{
		   clrscr();
		   dir(str,0xFF,entry,&n);
		   write_VRAM(5,row,0x3F,entry[i].name);
		   write_VRAM(center,1,0x70,str);
		   break;
		 }
		 else scanf("%s", &direc);
		 make_dir(direc);
		 if(ch == 13)
		 clrscr();	/* Phim enter */
		 dir(str,0xFF,entry,&n);
		 write_VRAM(5,row,0x3F,entry[i].name);
		 write_VRAM(center,1,0x70,str);
		 break;

	      case 66 :	/* F8 - Xoa tap tin */
		 for(i = 0;i < n-1;i++)
		 {
		    if(entry[i].state)
		    {
		       if(!test_dir(entry[i]))
		       {
			  getcwd(str,64);
			  if(str[strlen(str) - 1] != '\\')
			  strcat(str,"\\");
			  strcat(str,entry[i].name);
			  mess_dele_file();
			  key = getch();
			  if(toupper(key) == 'Y') dele_file(str);
			  if(ch == 27)
			  break;
		       }
		       else {
			 getcwd(str,64);
			 if(str[strlen(str) - 1] != '\\')
			 strcat(str,"\\");
			 strcat(str,entry[i].name);
			 mess_dele_dir();
			 key = getch();
			 if( toupper(key) == 'Y' ) dele_dir(str);
			 else if( toupper(key) == 27 )
			 break;
		       } /* Else*/
		    } /* If */
		 } /* For */
		 clrscr();
		 getcwd(str,64);
		 chdir(str);
		 if(str[strlen(str) - 1] == '\\') strcat(str,"\*.*");
		 else if(str[strlen(str) - 1] != '\\') strcat(str,"\\*.*");
		 i = 0;
		 row = 6;
		 dir(str,0xFF,entry,&n);
		 write_VRAM(5,row,0x3F,entry[i].name);
		 write_VRAM(2,22,0x4F,replicate(' ',78));
		 print_data(&entry[i],21);
		 write_VRAM(center,1,0x70,str);
		 break;

	      case 63 :	/* F5 - Copy file */
		 mess_copy_file();
		 if( getch() == 27)
		 {
		    clrscr();
		    getcwd(str,64);
		    chdir(str);
		    if(str[strlen(str) - 1] == '\\') strcat(str,"\*.*");
		    else
		      if(str[strlen(str) - 1] != '\\') strcat(str,"\\*.*");
		    i = 0;
		    row = 6;
		    dir(str,0xFF,entry,&n);
		    write_VRAM(5,row,0x3F,entry[i].name);
		    write_VRAM(2,22,0x4F,replicate(' ',78));
		    print_data(&entry[i],21);
		    write_VRAM(center,1,0x70,str);
		    break;
		 }
		 else {
		   set_text_attrib(0x74);
		   cscanf("%s",&strt);
		 }
		 for(i = 0; i < n-1; i++)
		 {
		    if(entry[i].state)  /* Co Insert hay khong */
		    {
		       entry[i].state = 0; /* Tat Insert */
		       if(!test_dir(entry[i]))
		       {
			  getcwd(str,64);
			  if(str[strlen(str) - 1] != '\\')
			  strcat(str,"\\");
			  strcat(str,entry[i].name);
			  if(strt[strlen(strt) - 1] != '\\')
			  strcat(strt,"\\");
			  strcat(strt,entry[i].name);
			  copy_file(str,strt);
		       } /* If thu muc */
		    } /* If insert */
		 } /* For */
		 clrscr();
		 getcwd(str,64);
		 chdir(str);
		 if(str[strlen(str) - 1] == '\\') strcat(str,"\*.*");
		 else if(str[strlen(str) - 1] != '\\') strcat(str,"\\*.*");
		 i = 0;
		 row = 6;
		 dir(str,0xFF,entry,&n);
		 write_VRAM(5,row,0x3F,entry[i].name);
		 write_VRAM(2,22,0x4F,replicate(' ',78));
		 print_data(&entry[i],21);
		 write_VRAM(center,1,0x70,str);
		 break;

	      case 64 : /* Phim F6 Doi ten tap tin hay di chuyen tap tin */
		 for(i = 0;i < n-1;i++)
		 {
		    if(entry[i].state) /* Co insert hay khong */
		    {
		       entry[i].state = 0; /* Tat insert */
		       if(!test_dir(entry[i]))
		       {
			  getcwd(str,MAXPATH);
			  if(str[strlen(str) - 1] != '\\')
			  strcat(str,"\\");
			  strcat(str,entry[i].name);
			  if(strt[strlen(strt) - 1] != '\\')
			  strcat(strt,"\\");
			  strcat(strt,entry[i].name);
			  change();
			  if(getch() == 27) break;
			  else {
			    scanf("%s", &strt);
			    rename_file(str,strt);
			  }
		       } /* Tat insert */
		    } /* If insert */
		 } /* For */
		 clrscr();
		 getcwd(str,64);
		 chdir(str);
		 if(str[strlen(str) - 1] == '\\') strcat(str,"\*.*");
		 else if(str[strlen(str) - 1] != '\\') strcat(str,"\\*.*");
		 i = 0;
		 row = 6;
		 dir(str,0xFF,entry,&n);
		 write_VRAM(5,row,0x3F,entry[i].name);
		 write_VRAM(2,22,0x4F,replicate(' ',78));
		 print_data(&entry[i],21);
		 write_VRAM(center,1,0x70,str);
		 break;

	      case 59 : /* Phim F1 giup do */
		 set_text_attrib(30);
		 clrscr();
		 record = 0;
		 disk_letter = get_driver();
		 strcat(disk_letter, "\\TOPICS\\PROCDIRS\\readme.txt");
		 fp = fopen(disk_letter, "rt");
		 while( !feof(fp) )
		 {
		    fgets(buff,256,fp);
		    printf("%s",buff);
		    record++;
		    if(record == 25)
		    {
		      record = 24; /* Chi hien 24 row van ban */
		      if(getch() == 27)
		      {
			 fclose(fp);
			 goto resume;
		       }
		    } /* If color tin */
		 } /* While */
		 fclose(fp);
		 getch();
		 goto resume;

	      case 61 : /* Phim F3 xem khong hieu chinh */
		 set_text_attrib(30);
		 clrscr();
		 fp = fopen(entry[i].name, "rb");
		 while( !feof(fp) )
		 {
		    fgets(buff,256,fp);
		    printf("%s",buff);
		    record++;
		    if(record == 25)
		    {
		       record = 24; /* Chi hien 24 row van ban */
		       if(getch() == 27)
		       {
			  fclose(fp);
			  break;
		       }
		    } /* If color tin */
		 } /* While */
		 fclose(fp);
		 clrscr();
		 getcwd(str,64);
		 chdir(str);
		 if(str[strlen(str) - 1] == '\\') strcat(str,"\*.*");
		 else if(str[strlen(str) - 1] != '\\') strcat(str,"\\*.*");
		 i = 0;
		 row = 6;
		 dir(str,0xFF,entry,&n);
		 write_VRAM(5,row,0x3F,entry[i].name);
		 write_VRAM(2,22,0x4F,replicate(' ',78));
		 print_data(&entry[i],21);
		 write_VRAM(center,1,0x70,str);
		 break;

	  } /* Switch  */
       } /* If Readkey */
       else {
	 if(ch == 13) /* Phim enter */
	 if(test_dir(entry[i])) /* La thu muc */
	 {
	    getcwd(str,64);    /* Duong dan  */
	    if(str[strlen(str) - 1] != '\\') strcat(str,"\\");
	    strcat(str,entry[i].name);
	    chdir(str);
	    strcat(str,"\\*.*");
	    break;
	 }
	 else { /* Khong phai thu muc. Cho thuc hien cac tap tin kha thi */
	   if((strstr(entry[i].name,".EXE"))
	   ||(strstr(entry[i].name,".COM")))
	   system(entry[i].name); /* Thuc hien cac tap tin tuong ung */
	   getch();
	   break;
	 }
       } /* Else doc phim */
     } /* While !ESC */
     free(str);
     free(disk_letter);
     free(file_name);
   } /* While(1) */
} /* Ham main */

/*--------------------------------------*/
/*      Thay doi kich thuoc con tro     */
/*--------------------------------------*/
void set_cursor(byte top, byte bott)
{
  asm {
     mov ah, 1
     mov ch, top
     mov cl, bott
     int 0x10
  }
}

/*---------------------------------------------*/
/*    Cho ra chuoi ky tu gom n ky tu           */
/*---------------------------------------------*/
char *replicate(char symbol, byte times)
{
  char *aux;
  byte k;

  aux = (char *)malloc(times + 1);
  for(k = 0; k < times; aux[k++] = symbol);
  aux[times] = '\0';
  return aux;
}

/*-----------------------------------------*/
/*  Thiet lap thuoc tinh color cho van ban */
/*-----------------------------------------*/
void set_text_attrib(byte attr)
{
   textattr(attr);
}

/*------------------------------------*/
/*  Lay thuoc tinh color chu hien tai */
/*------------------------------------*/
void set_border_color(byte color)
{
  union REGS regs;
  regs.h.ah = 0x10;
  regs.h.al = 1;
  regs.h.bh = color & 63; /* Mau cua vien tu 0 den 63 */
  int86(0x10,&regs,&regs);
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

// Write a character to x, y
void putch_XY(byte x, byte y, char c)
{
  gotoxy(x, y);
  putch(c);
}

// Draw a frame with the edge is lane
void frame( byte x1, byte y1, byte x2, byte y2, byte lane )
{
  static byte bound[4][6] =
  { {218, 196, 191, 179, 217, 192}, /* Cac ma de tao khung */
    {201, 205, 187, 186, 188, 200},
    {213, 205, 184, 179, 190, 212},
    {214, 196, 183, 186, 189, 211}
  };
  char *border;
  byte k;

  lane = (lane - 1) % 4; /* Chi dinh 4 kieu khung */
  border = bound[lane];
  putch_XY(x1, y1, border[0]);
  for(k = x1 + 1; k < x2; putch(border[1]), k++);
  putch(border[2]);
  putch_XY( x1, y2, border[5] );
  for(k = x1 + 1; k < x2; putch(border[1]), k++);
  putch(border[4]);
  for(k = y1 + 1; k < y2; k++) {
    putch_XY(x1, k, border[3]);
    putch_XY(x2, k, border[3]);
  }
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

/*--------------------------------------------------*/
/* Funtion : write_char                             */
/* Mission : Writting a character with attribute    */
/* Expects : (x,y) cordinate to write a character   */
/*           (attr) attribute of character          */
/*           (len) length area                      */
/*           (chr) symbol needs to write            */
/* Returns : Nothing                                */
/*--------------------------------------------------*/
void write_char_VRAM(byte x, byte y, byte attr, byte len, char chr)
{
   word video_offset;
   byte i;

   video_offset = OFFSET(x,y); // Define the address of cordinate x,y
   for( i = 1; i <= len; i++ ) {
      pokeb(0xB800, video_offset , (attr << 8) + chr); // Change the attribute
      video_offset += 2;  // Move to next address
   }
}

/*--------------------------------------------------*/
/* Funtion : fill_frame                             */
/* Mission : to full the box with special character */
/* Expects : (x1,y1) cordinate top to left          */
/*           (x2,y2) cordinate bottom to right      */
/*           (attr) special character color         */
/*           (chr) special character                */
/* Returns : Nothing                                */
/*--------------------------------------------------*/
void fill_frame(byte x1, byte y1, byte x2, byte y2, byte attr, char chr)
{
   byte i = y1;
   for( ;i <= y2; ) write_char_VRAM(x1, i++, attr, x2 - x1 + 1, chr );
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
int click_mouse(byte *col, byte *row)
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

/*-------------------------------------------------*/
/* Function : write_XY                             */
/* Mission  : Write a character to cordinate x, y  */
/* Expects  : (x,y) cordinate to write             */
/*            (chr) character to write             */
/*            (attr) attrib for the character      */
/* Returns  : Nothing                              */
/*-------------------------------------------------*/
void write_XY(byte x, byte y, byte attr, char chr)
{
   buffer[OFFSET(x, y)] = chr;
   buffer[OFFSET(x, y) + 1] = attr;
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
void button(byte x, byte y, word attr, byte back, const char *title, byte type = 0, word first_letter = 0)
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
   byte old_attr;

   old_attr = save_current_attrib();       // Save current attrib
   set_text_attrib(attr);                  // Setting the attribute
   frame(x1, y1, x2, y2, lane);            // Draw a frame
   window(x1 + 1, y1 + 1, x2 - 1, y2 - 1); // Setting a window
   clrscr();                               // Fill color in window setting
   window(1, 1, 80 ,25);                   // Setting normal window
   set_text_attrib(old_attr);              // Setting normal color
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

/*--------------------------------------*/
/*   Lay so trang man hinh hien thoi    */
/*--------------------------------------*/
byte get_page()
{
   union REGS regs;         /* Bien thanh ghi de goi ngat       */
   regs.h.ah = 15;	    /* Ham lay trang man hinh hien thoi */
   int86(0x10,&regs,&regs); /* Goi ngat 10h                     */
   return(regs.h.bh);       /* So trang man hinh hien thoi      */
}

/*-----------------------------------------------------*/
/*  Dat vi tri con tro trong trang man hinh hien thoi  */
/*-----------------------------------------------------*/
void set_pos(int col,int row)
{
   union REGS regs;
   regs.h.ah = 2;	    /* So hieu ham dat vi tri con tro */
   regs.h.bh = get_page();  /* Trang man hinh hien thoi       */
   regs.h.dh = row;         /* Cac vi tri moi cua con tro     */
   regs.h.dl = col;
   int86(0x10,&regs,&regs); /* Goi ham                        */
}

/*------------------------------------------------*/
/*  Lay vi tri con tro trong man hinh hien thoi   */
/*------------------------------------------------*/
void get_pos(char *col,char *row)
{
   union REGS regs;
   regs.h.ah = 3;	    /* So hieu ham lay toa do con tro */
   regs.h.bh = get_page();  /* Trang hien thoi                */
   int86(0x10,&regs,&regs); /* Goi ngat                       */
   *col  = regs.h.dl;       /* Toa do row                     */
   *row = regs.h.dh;        /* Toa do col                     */
}

/*--------------------------------------------------*/
/*  Cuon man hinh len tren. Neu so 0 la so row thi  */
/*  ca man hinh se bi xoa trang                     */
/*--------------------------------------------------*/
void scroll_up(int row,int color,int col_left,int row_left,int col_right,
	       int row_right)
{
   union REGS regs;
   regs.h.ah = 6;        /* So hieu ham cuon man hinh len */
   regs.h.al = row;      /* Dong bat dau                  */
   regs.h.bh = color;    /* Mau cua trang man hinh        */
   regs.h.ch = row_left; /* Toa do cua cua so man hinh    */
   regs.h.cl = col_left;
   regs.h.dh = row_right;
   regs.h.dl = col_right;
   int86(0x10,&regs,&regs);
}

/*--------------------------------------------------*/
/*  Cuon man hinh len tren. Neu so 0 la so row thi  */
/*  se cuon toan bo man hinh                        */
/*--------------------------------------------------*/
void scroll_down(int row,int color,int col_left,int row_left,int col_right,
		 int row_right)
{
   union REGS regs;
   regs.h.ah = 7;	 /* Ham cuon man qhinh xuong   */
   regs.h.al = row;      /* Dong bat dau               */
   regs.h.bh = color;    /* Thuoc tinh row trong       */
   regs.h.ch = row_left; /* Toa do cua cua so man hinh */
   regs.h.cl = col_left;
   regs.h.dh = row_right;
   regs.h.dl = col_right;
   int86(0x10,&regs,&regs);
}

/*-----------------------------------------------------*/
/*   Viet mot ky tu voi thuoc tinh tai vi tri con tro  */
/*-----------------------------------------------------*/
void write_char(char ch, byte color)
{
   union REGS regs;
   regs.h.ah = 9;	   /* So hieu ham viet ki tu ra man hinh */
   regs.h.al = ch;         /* Ky tu dua ra                       */
   regs.h.bh = get_page(); /* Trang man hinh hien thoi           */
   regs.h.bl = color;      /* Mau cua ky tu                      */
   regs.x.cx = 1;	   /* Hien moi lan mot ky tu             */
   int86(0x10,&regs,&regs);
}

/*-----------------------------------------------------------*/
/*  Viet mot chuoi ki tu voi thuoc tinh tai vi tri xac dinh  */
/*-----------------------------------------------------------*/
void write_BIOS(int col,int row,char *text,int color)
{
   union REGS rin,rout;
   set_pos(col,row);      /* Dat vi tri con tro trong trang man hinh */
   rin.h.ah = 14;         /* So hieu ham goi                         */
   rin.h.bh = get_page(); /* Trang man hinh hien thoi                */
   while(*text)           /* Dua ra cho toi khi gap ma NULL          */
   {
      write_char(' ',color); /* Mau cua ky tu dua ra */
      rin.h.al = *text++;    /* Ky tu dua ra         */
      int86(0x10,&rin,&rout);
   }
}

/*-------------------------------------------------------*/
/*   Ham ve duong thang tu vi tri con tro den toa do x,y */
/*-------------------------------------------------------*/
void lineto(int x2,int y2,char ch,int color)
{
   char x1, y1, temp;
   get_pos(&x1,&y1);
   if(y1 == y2)
   {
      if(x1 > x2)
      {
	 temp  = x1;
	 x1 = x2;
	 x2 = temp;
      }
      while((x2 - x1 + 1) != 0)
      {
	 write_char(ch,color);
	 x1++;
	 set_pos(x1,y1);
      }
   }
   if(x1 == x2)
   {
      if(y1 > y2)
      {
	 temp  = y1;
	 y1 = y2;
	 y2 = temp;
      }
      while((y2 - y1+1) != 0)
      {
	 write_char(ch,color);
	 y1++;
	 set_pos(x1,y1);
      }
   }
}

/*-----------------------------*/
/*    Ham ve khung chu nhat    */
/*-----------------------------*/
void frame_color(int x1,int y1,int x2,int y2,int color,int lane)
{
   char bone[][6] = {{201,205,187,186,188,200},{218,196,191,179,217,192}};
   set_pos(x1,y1);
   lane = 0 ? 0 : 0;
   write_char(bone[lane][0],color);
   set_pos(x1+1,y1);
   lineto(x2,y1,bone[lane][1],color);
   set_pos(x2,y1);
   write_char(bone[lane][2],color);
   set_pos(x1,y2);
   write_char(bone[lane][5],color);
   set_pos(x1+1,y2);
   lineto(x2,y2,bone[lane][1],color);
   set_pos(x2,y2);
   write_char(bone[lane][4],color);
   set_pos(x1,y1+1);
   lineto(x1,y2-1,bone[lane][3],color);
   set_pos(x2,y1+1);
   lineto(x2,y2-1,bone[lane][3],color);
}

/*-------------------------------------------------------*/
/*   Ham doc phim nhan                                   */
/*   Neu phim chuc nang thi tra ve gia tri 0 va ma scan  */
/*   Neu phim ASCII thi tra ve gia tri 1 va ma ASCII     */
/*-------------------------------------------------------*/
char read_key(char *ch)
{
   union REGS regs;
   regs.h.ah = 0;
   int86(22,&regs,&regs);
   if(!(regs.h.al))
   {
      *ch = regs.h.ah;
      return(0);
   }
   else *ch = regs.h.al;
   return(1);
}

/*----------------------------------------------------------------*/
/*   Xoa man hinh hien thoi dua con tro ve goc trai tren man hinh */
/*----------------------------------------------------------------*/
void clear_screen()
{
   scroll_up(0,NOF,0,0,79,24); /* Cuon toan bo man hinh       */
   set_pos(0,0);               /* Dua con tro ve dau man hinh */
}

/*-----------------------*/
/*   Dua ra mot de muc   */
/*-----------------------*/
void print_data(DIRS *entry, byte row)
{
   byte i;
   static char *month[]={"JAN","FEV","MAR","APR","MAY","JUN",
			 "JUL","AUG","SEP","OCT","NOV","DEC"};

   char bone[][6]={{201,205,187,186,188,200},{218,196,191,179,217,192}};
   set_pos(4,row);
   for(i=0;(*entry).name[i] && i < 15; printf("%c",(*entry).name[i++]));
   set_pos(23,row);
   if((((*entry).attribut) >> 4 ) & 1)
   printf("%c%s",17," SUB-DIR ");
   printf("%7lu",(*entry).size);
   set_pos(38,row);
   printf(" %2d %s %4d",(*entry).date & 31,
   month[((*entry).date >> 5 & 15) -1],((*entry).date >> 9) + 1980);
   set_pos(55,row);
   printf(" %2d h %2d",(*entry).hour >> 11,(*entry).hour >> 5 & 63);
   set_pos(20,1);
   write_VRAM(21,1,0x1E,"Ñ");
   lineto(20,19,bone[0][9],0x1E);
   write_VRAM(21,21,0x1E,"Ï");
   set_pos(35,1);
   write_VRAM(36,1,0x1E,"Ñ");
   lineto(35,19,bone[0][9],0x1E);
   write_VRAM(36,21,0x1E,"Ï");
   set_pos(53,1);
   write_VRAM(54,1,0x1E,"Ñ");
   lineto(53,19,bone[0][9],0x1E);
   write_VRAM(54,21,0x1E,"Ï");
   set_pos(67,1);
   write_VRAM(68,1,0x1E,"Ñ");
   lineto(67,19,bone[0][9],0x1E);
   write_VRAM(68,21,0x1E,"Ï");
   set_pos(68,row); /* Duy chuyen con tro den row thuoc tinh */
   if( (((*entry).attribut) & 0x01) != 0 ) printf("R"); /* ReadOnly */
   if( (((*entry).attribut) & 0x02) != 0 ) printf("H"); /* Hidden   */
   if( (((*entry).attribut) & 0x04) != 0 ) printf("S"); /* System   */
   if( (((*entry).attribut) & 0x20) != 0 ) printf("A"); /* Archive  */
   if( (((*entry).attribut) & 0x08) != 0 ) printf("V"); /* VolumeID */
}
/*-------------------------------------------*/
/*   Chuan bi man hinh de dua ra cac de muc  */
/*-------------------------------------------*/
void screen_information()
{
   byte i;
   clear_screen();
   frame_color(0,0,79,22,0x1E,DOUBLE);
   frame_color(0,20,79,22,0x1E,DOUBLE);
   write_VRAM(1,21,0x1E,"Ì");
   write_VRAM(80,21,0x1E,"¹");
   write_VRAM(2,24,0x1C,"F1");
   write_VRAM(11,24,0x1C,"F2");
   write_VRAM(24,24,0x1C,"F3");
   write_VRAM(32,24,0x1C,"F4");
   write_VRAM(41,24,0x1C,"F5");
   write_VRAM(48,24,0x1C,"F6");
   write_VRAM(58,24,0x1C,"F7");
   write_VRAM(67,24,0x1C,"F8");
   write_VRAM(73,24,0x1C,"F10");
   write_VRAM(4,24,0x1C,"Help");
   write_VRAM(13,24,0x30,"CopyRight");
   write_VRAM(26,24,0x30,"View");
   write_VRAM(34,24,0x30,"ChAttr");
   write_VRAM(43,24,0x30,"Copy");
   write_VRAM(50,24,0x30,"RenMov");
   write_VRAM(60,24,0x30,"MkDir");
   write_VRAM(69,24,0x30,"Del");
   write_VRAM(76,24,0x30,"Quit");
   write_VRAM(9,EZ,0x1E,"NAME");
   write_VRAM(26,EZ,0x1E,"SIZE");
   write_VRAM(43,EZ,0x1E,"DATE");
   write_VRAM(58,EZ,0x1E,"TIME");
   write_VRAM(72,EZ,0x1E,"RSHVD");
   write_VRAM(1,25,0x0E," Chuong Trinh Quan Ly Thu Muc Tren O Dia Cung. Thuc Hien : Std NGUYEN NGOC VAN  ");
}

/*------------------------*/
/*   Lay de muc dau tien  */
/*------------------------*/
byte get_opt_first(char *path,word attrib)
{
   union REGS regs;
   struct SREGS segments;    /* Dung de nhan thanh ghi doan     */
   segread(&segments);       /* Nap noi dung cac thanh ghi doan */
   regs.h.ah = 0x4E;         /* So hieu ham tim de muc dau tien */
   regs.x.cx = attrib;       /* Thuoc tinh cua file             */
   regs.x.dx = (word)path;   /* Dia chi offset cua duong dan    */
   intdosx(&regs,&regs,&segments); /* Goi ngat 21h cua DOS            */
   return(!regs.x.cflag);          /* Co carry = 0 Tim thay mot file  */
}

/*--------------------------*/
/*  Lay de muc tiep theo    */
/*--------------------------*/
byte get_opt_next()
{
   union REGS regs;
   regs.h.ah = 0x4F;      /* So hieu ham tim de muc tiep theo  */
   intdos(&regs,&regs);   /* Goi ngat 21h cua DOS              */
   return(!regs.x.cflag); /* Flag carry = 0: Tim thay mot file */
}

/*-------------------------------------------*/
/*  Dat DTA den mot bien trong doan du lieu  */
/*-------------------------------------------*/
void set_DTA(DIRS *offset)
{
   union REGS regs;
   struct SREGS segments;
   segread(&segments);         /* Lay noi dung cua cac thanh ghi doan */
   regs.h.ah = 0x1A;           /* Ham dat lai DTA                     */
   regs.x.dx = (word)offset;   /* Dia chi offset trong thanh ghi DX   */
   intdosx(&regs,&regs,&segments); /* Goi ngat cua DOS                */
}
/*-------------------------------------------*/
/*   Ham chep cac entry vao trong mot mang   */
/*-------------------------------------------*/
void save_to_array(char *path,int attribut,DIRS *array,int *n)
{
   DIRS entry;
   set_DTA(&entry);
   *n = 0;
   if(get_opt_first(path,attribut))            /* Tim de muc dau    */
   {
      do
      {
	 strcpy(array[*n].name,entry.name);   /* Nhan ten tap tin  */
	 array[*n].hour = entry.hour;         /* Nhan gio tap tin  */
	 array[*n].date = entry.date;         /* Nhan ngay tap tin */
	 array[*n].attribut = entry.attribut; /* Nhan thuoc tinh   */
	 array[*n].size = entry.size;         /* Nhan kich thuoc   */
	 array[*n].state = 0;                 /* Nhan trang thai   */
	 (*n)++;
      } while(get_opt_next());                /* Tim de muc tiep   */
   }
}

/*----------------------------------------------*/
/*   Dieu khien viec lay va dua ra cac de muc   */
/*----------------------------------------------*/
void dir(char *path,int attrib,DIRS *array,int *n)
{
   int temp = 2;
   DIRS entry;
   save_to_array(path,attrib,array,n);
   set_DTA(&entry);
   screen_information();
   if(get_opt_first(path,attrib))
   {
      do { print_data(&entry,EZ+temp); /* Dua cac de muc ra man hinh */
      } while(get_opt_next() && (++temp) != ENTRE+2);
   }
}

/*------------------------------------------------------------------*/
/*   Ham kiem tra xem mot entry la thu muc hay mot file             */
/*   Ham se tra ve tri 1 neu la thu muc va tra ve tri 0 neu la file */
/*------------------------------------------------------------------*/
int test_dir(DIRS entry)
{
   if( (entry.attribut >> 4) & 1 )
      return(1);
   else
      return(0);
}

/*--------------------------------------------------------------------*/
/*   Ham dua ra man hinh mot mess ki tu bang cach truy nhap truc tiep */
/*   bo nho man hinh tai row,tu col_left den col_right voi tx va bk   */
/*--------------------------------------------------------------------*/
void write_str_attr(char *mess,int row,int col_left,int col_right,int bk, int tx)
{
   int i,address,page,aux,test,color;
   char far *buf;
   union REGS rin,rout;
   buf = (char far *)MK_FP(0xB800,0);/* Lay dia chi thuc bo nho man hinh */
   color = (bk << 4) + tx;	     /* Xac dinh thuoc tinh color        */
   page = (row - 1)/25;              /* Xac dinh trang man hinh page va  */
   aux = row - 1 - page*25;          /* Cac chi so row thuoc page        */
   rin.h.ah = 5;
   rout.h.al = page;
   int86(0x10,&rin,&rout);
   address = page*4096 + aux*160 + (col_left - 1)*2;
   for(i = 0;i <= col_right - col_left;++i)
   {
      if((test = mess[i]) == 0) break;
      buf[address+2*i] = test;     /* Dong van ban   */
      buf[address+2*i+1] = color;  /* Thuoc tinh mau */
   }
}

/*-----------------------*/
/*  Ham xoa mot tap tin  */
/*-----------------------*/
char dele_file(char *str)
{
   union REGS regs;
   regs.h.ah = 0x41;
   regs.x.dx = FP_OFF(str);
   int86(0x21,&regs,&regs);
   if(regs.x.cflag) return(-1);
   return(1);
}

/*---------------------------------*/
/*   Ham chon o dia do dinh truoc  */
/*---------------------------------*/
char set_disk(char disk)
{
   union REGS regs;
   byte temp;
   regs.h.ah = 0x0E;
   switch(toupper(disk))
   {
     case 'A':
       temp = 0;
       break;
     case 'B':
       temp = 1;
       break;
     case 'C':
       temp = 2;
       break;
     case 'D':
       temp = 3;
       break;
     case 'E':
       temp = 4;
       break;
     case 'F':
       temp = 5;
       break;
     case 'G':
       temp = 6;
       break;
     case 'H':
       temp = 7;
       break;
     case 'I':
       temp = 8;
       break;
     case 'J':
       temp = 9;
       break;
   }
   regs.h.dl = temp;
   int86(0x21,&regs,&regs);
   if(regs.x.cflag) return(-1);
   return(temp);
}

/*-------------------------------*/
/*     Ham nhan o dia            */
/*-------------------------------*/
void driver()
{
   union REGS rin,rout;
   unsigned long a,b,c,d;
   word disk;

   set_cursor(0x20,0x20);
   box_shadow(15,10,64,13,0x74,1,0x4F," Select Driver ");
   write_BIOS(19,10,"Enter the serial number you want to view",0x70);
   write_BIOS(19,11,"( As : 1=A, 2=B, 3=C, 4=D, 5=E, 6=F ... )",0x70);
   set_text_attrib(0x70);
   disk = getche() - '0';
   rin.h.ah = 0x36;
   rin.h.dl = disk;
   int86(0x21,&rin,&rout);

   if(rout.x.ax == 0xFFFF)
   {
      sound(2000);
      delay(100);
      write_BIOS(17,11,replicate(' ',45),0x77);
      write_BIOS(17,11,"WARNING : The serial disk number is invalid !",0x7E);
      nosound();
   }
   else {
     a = rout.x.dx;
     b = rout.x.bx;
     c = rout.x.ax;
     d = rout.x.cx;
     box_shadow(15,10,63,13,0x5E,1,0x24, " Driver Information ");
     set_text_attrib(0x5F);
     gotoxy(20,11);
     cprintf("Bytes total disk space : %11lu ",a*c*d);
     gotoxy(20,12);
     cprintf("Bytes available on disk : %11lu ",b*c*d);
   }
  getch();
  clrscr();
  set_cursor(11,10);
}

/*----------------------------------*/
/*        Ban quyen chuong trinh    */
/*----------------------------------*/
void copyright()
{
   set_cursor(0x20,0x20);
   box_shadow(15,10,62,14,0x4F,1,0x4A," Copyright ");
   write_VRAM(17,11,0x4E,"Sinh vien thuc hien :");
   write_VRAM(17,12,0x4F,replicate('~',21));
   write_VRAM(40,12,0x4B,"NGUYEN NGOC VAN");
   write_VRAM(40,13,0x4F,"TRUONG ANH LINH");
   getch();
   clrscr();
   set_cursor(11,10);
}

/*----------------------------*/
/*    Ham doi ten tap tin     */
/*----------------------------*/
int rename_file(char *old_name,char *new_name)
{
   union REGS rin,rout;
   struct SREGS seg ;

   rin.h.ah = 0x56;
   seg.ds = FP_SEG(old_name);
   rin.x.dx = FP_OFF(old_name);
   seg.es = FP_SEG(new_name);
   rin.x.di = FP_OFF(new_name);
   int86x(0x21,&rin,&rout,&seg);
   return rout.x.ax;
}


/*------------------------------------------*/
/*   Ham copy mot tap tin sang mot tap tin  */
/*------------------------------------------*/
void copy_file(char *dest,char *source)
{
   int n;
   char c[1000];
   int fd1,fd2;
   fd1 = _open(dest,0);
   fd2 = _creat(source,FA_ARCH);
   while((n = read(fd1,c,1000)) > 0) write(fd2,c,n);
   close(fd1);
   close(fd2);
}

/*-----------------------------*/
/*       Ham tao thu muc       */
/*-----------------------------*/
int make_dir(char *name_dir)
{
   union REGS rin,rout;
   struct SREGS seg ;

   rin.h.ah = 0x39;
   seg.ds = FP_SEG(name_dir);
   rin.x.dx = FP_OFF(name_dir);
   int86x(0x21,&rin,&rout,&seg);
   return rout.x.ax;
}

/*-----------------------------*/
/*       Ham xoa thu muc       */
/*-----------------------------*/
int dele_dir(char *name_dir)
{
   union REGS rin,rout;
   struct SREGS seg ;

   rin.h.ah = 0x3A;
   seg.ds = FP_SEG(name_dir);
   rin.x.dx = FP_OFF(name_dir);
   int86x(0x21,&rin,&rout,&seg);
   if( rout.x.cflag != 0 )
   {
      write_BIOS(15,10,replicate(' ',50),0x55);
      write_BIOS(18,10,"WARNING : Directory are not empty !",0x5E);
      sound(2000);
      delay(200);
      nosound();
      getch();
   }
   return rout.x.ax;
}

/*--------------------------------------*/
/*  Ham thuc hien chuc nang chon o dia  */
/*--------------------------------------*/
void exec_driver(byte select)
{
  str[0] = '\0';
  switch( select )
  {
     case 0 :
	set_disk('A');
	chdir("A:\\");
	getcwd(str,64);
	strcat(str,"*.*");
	break;

     case 1 :
	set_disk('C');
	chdir("C:\\");
	getcwd(str,64);
	strcat(str,"*.*");
	break;

     case 2 :
	set_disk('D');
	chdir("D:\\");
	getcwd(str,64);
	strcat(str,"*.*");
	break;

     case 3 :
	set_disk('E');
	chdir("E:\\");
	getcwd(str,64);
	strcat(str,"*.*");
	break;

     case 4 :
	set_disk('F');
	chdir("F:\\");
	getcwd(str,64);
	strcat(str,"*.*");
	break;
  } /* Switch */
}

/*-------------------------------*/
/*      Ham lua chon o dia       */
/*-------------------------------*/
void menu_driver()
{
   static char *list_driver[5]= {"A","C","D","E","F"};
   int x = 30, key, select = 0, i;

   box_shadow(25,10,50,12,0x6F,1,0x24," Choose Driver ");
   for(i = 0; i < 5; i++) write_VRAM(x+(i*4),11,0x6F,list_driver[i]);
   write_BIOS(x-1,10,list_driver[0],0x6A);
   do /* Di chuyen hop sang tren cua so thuc don */
   {
      key = getch();
      if( !key )
      {
	 key = getch();
	 switch( key )
	 {
	    case LEFT :
	       write_BIOS(x+(select*4)-1,10,list_driver[select],0x6F);
	       if( select <= 0 ) select = 4;
	       else select--;
	       write_BIOS(x+(select*4)-1,10,list_driver[select],0x6A);
	       break;

	    case RIGHT :
	       write_BIOS(x+(select*4)-1,10,list_driver[select],0x6F);
	       if( select >= 4 ) select = 0;
	       else select++;
	       write_BIOS(x+(select*4)-1,10,list_driver[select],0x6A);
	       break;

	    case HOME :
	      write_BIOS(x+(select*4)-1,10,list_driver[select],0x6F);
	      select = 0;
	      write_BIOS(x+(select*4)-1,10,list_driver[select],0x6A);
	      break;

	    case END :
	      write_BIOS(x+(select*4)-1,10,list_driver[select],0x6F);
	      select = 4;
	      write_BIOS(x+(select*4)-1,10,list_driver[select],0x6A);
	      break;

	    default : break;

	 } /* Switch */
      } /* If */
   } while( key != ESC && key != ENTER);
   if( key == ENTER ) exec_driver( select );
}

void menu_attrib()
{
   static char *list_attrib[4] = { "ReadOnly", "Archive",
				   "Hidden","System" };
   int x = 19, key, select = 0, i;

   for( i = 0; i < 4; i++ ) write_BIOS(x+(11*i),12,list_attrib[i],0x34);
   write_BIOS(x,12,list_attrib[0],0x5E);
   do
   {
      key = getch();
      if( !key )
      {
	 key = getch();
	 switch( key )
	 {
	    case LEFT :
	       write_BIOS(x+(select*11),12,list_attrib[select],0x34);
	       if( select <= 0 ) select = 3;
	       else select--;
	       write_BIOS(x+(select*11),12,list_attrib[select],0x5E);
	       break;

	    case RIGHT :
	       write_BIOS(x+(select*11),12,list_attrib[select],0x34);
	       if( select >= 3 ) select = 0;
	       else select++;
	       write_BIOS(x+(select*11),12,list_attrib[select],0x5E);
	       break;

	    case HOME :
	       write_BIOS(x+(select*11),12,list_attrib[select],0x34);
	       select = 0;
	       write_BIOS(x+(select*11),12,list_attrib[select],0x5E);
	       break;

	    case END :
	       write_BIOS(x+(select*11),12,list_attrib[select],0x34);
	       select = 3;
	       write_BIOS(x+(select*11),12,list_attrib[select],0x5E);
	       break;
	 } /* Switch key */
      } /* If special key */
   } while( key != ESC && key != ENTER );
   if( key == ENTER ) exec_attrib( select );
}

void exec_attrib( byte select )
{
   switch( select )
   {
      case 0 : attr = 0x01; set_attrib(file_name,attr);; break;
      case 1 : attr = 0x20; set_attrib(file_name,attr);; break;
      case 2 : attr = 0x02; set_attrib(file_name,attr); break;
      case 3 : attr = 0x04; set_attrib(file_name,attr); break;
      default : write_VRAM(10,10,0x30,"The attribute you choose not installed.");
   }
}

/*------------------------------------------------*/
/* Ham thiet lap thuoc tinh cho mot file          */
/* Vao : (file_name) ten file ke ca duong dan,    */
/*       (attr) thuoc tinh can cai dat            */
/* Ra  : Khong                                    */
/*------------------------------------------------*/
void set_attrib(char *file_name, word attr)
{
  struct SREGS seg;
  union REGS rin, rout;

  rin.h.ah = 0x43;              /* So hieu ham de set thuoc tinh */
  rin.h.al = 1;                 /* So hieu ham con               */
  rin.x.cx = attr;              /* Thuoc tinh can dat            */
  seg.ds = FP_SEG(file_name);   /* Dia chi segment cua file      */
  rin.x.dx = FP_OFF(file_name); /* Dia chi offset cua file       */
  int86x(0x21,&rin,&rout,&seg); /* Thuc hien gat 21h             */
  if( rout.x.cflag != 0 )       /* Kiem tra loi                  */
  {
     if( rout.x.ax == 2 )       /* Khong tim thay tap tin        */
     {
	write_BIOS(19,9,"File not found !",0xBA);
	sound(2000);
	delay(200);
	nosound();
     }
     if( rout.x.ax == 3 )       /* Khong tim thay duong dan     */
     {
	write_BIOS(19,9,"Path not found !",0xBA);
	sound(2000);
	delay(200);
	nosound();
     }
  }
  getch();
}
/*--------------------------------*/
/*     Thuc hien phim F4          */
/*--------------------------------*/
void attribute()
{
   box_shadow(15,8,63,14,0x3E,2,0x4F," Set Attribute ");
   write_BIOS(18,8,"Enter a file you want to change attribute",0x30);
   write_BIOS(18,9,"[.......................................]",0x30);
   gotoxy(20,10);
   gets(file_name);
   write_BIOS(22,10,"Choose an option to set attribute",0x3F);
   write_BIOS(19,11,replicate(196,39),0x3A);
   set_cursor(0x20,0x20);
   menu_attrib();
}

/*-----------------------------------*/
/*      Ham thuc hien phim F6        */
/*-----------------------------------*/
void change()
{
   box_shadow(16,10,64,13,0x7E,1,0x4F," Copy or Move ");
   write_BIOS(26,10,"Rename or the file move to",0x74);
   write_BIOS(18,11,"________________________________________",0x74);
   gotoxy(19,12);
}

/*--------------------------------*/
/*      Ham thuc hien phim F5     */
/*--------------------------------*/
void mess_copy_file()
{
   box_shadow(15,10,62,12,0x7E,1,0x4F," Copy ");
   write_BIOS(18,10,"Copy to : ",0x74);
}

/*---------------------------------*/
/*      Ham thuc hien phim F7      */
/*---------------------------------*/
void mess_make_dir()
{
   box_shadow(15,10,64,13,0x5E,1,0x24," Make Directory ");
   write_BIOS(20,10,"Enter directory you want to create",0x5F);
   write_BIOS(20,11,"__________________________________",0x5F);
   gotoxy(21,12);
}

/*----------------------------------------------*/
/*    Ham thuc hien phim F8 (Xoa thu muc rong)  */
/*----------------------------------------------*/
void mess_dele_dir()
{
   box_shadow(15,10,67,12,0x5E,1,0x24," Delete Directory ");
   write_BIOS(18,10,"Are you sure to delete this directory (Y/N)? ",0x5F);
}

/*-----------------------------------------*/
/*    Ham thuc hien phim F8 (Xoa tap tin)  */
/*-----------------------------------------*/
void mess_dele_file()
{
   box_shadow(15,10,62,12,0x5E,1,0x24," Delete File ");
   write_BIOS(18,10,"Are you sure to delete this file (Y/N)? ",0x5F);
}

/*--------------------------------------------------*/
/*        XAC DINH DO RONG CUA CHUOI                */
/*--------------------------------------------------*/
void outtext_width(int x, int y ,char *string, int w)
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
void outtext_shade(int x, int y, char *string, int shade, int color1,
int color2, int w)
{
   int i;

   setcolor( color2 );
   for( i = shade; i > 0; i-- ) outtext_width( x + i, y + i, string, w );
   setcolor( color1 );
   outtext_width( x, y, string, w );
}

/*----------------------------------------*/
/* Funtion : get_driver                   */
/* Mission : Getting the letter driver    */
/* Expects : Nothing                      */
/* Returns : Letter driver of disk        */
/*----------------------------------------*/
char *get_driver()
{
   FILE *fptr;
   char *driver, tmp[31];

   driver = (char *)malloc(4*sizeof(char));
   if( (fptr = fopen("C:\\WINDOWS\\register.dat","rb")) == NULL ) {
      set_text_attrib(0x1F);
      set_border_color(47);
      set_cursor(0x20,0x20);
      clrscr();
      write_VRAM(33,10,0x4F,"SYSTEM ERROR");
      write_VRAM(20,12,0x1F,"Cannot reading from file register.dat");
      write_VRAM(20,13,0x1F,"Please strike any key to exit program");
      getch();
      end_program();
   }
   fgets(tmp,31,fptr);
   fgets(tmp,31,fptr);
   fgets(tmp,31,fptr);
   fgets(driver,4,fptr);
   fclose(fptr);
   return driver;
}

/*----------------------------------*/
/*   Ham kiem tra phan cung do hoa  */
/*----------------------------------*/
int huge detect_SVGA256()
{
   int graph_driver, graph_mode, supper_mode = 0;
   detectgraph(&graph_driver, &graph_mode);
   if(graph_driver == VGA || graph_driver == EGA)
      return supper_mode;
   else
      return grError;
}

/*------------------------------------------*/
/*       Gioi thieu chuong trinh chinh      */
/*------------------------------------------*/
void introduce()
{
   char *path;
   int temp[6], x0, ht, h, i, y, graph_mode = 3, maxx, maxy, error_code,
   graph_driver = installuserdriver("SVGA256", detect_SVGA256);

   path = (char *)malloc(19*sizeof(char));
   path = get_driver();
   *(path + 3) = 0;
   strcat(path, "TOPICS\\DRIVERS");
   initgraph(&graph_driver, &graph_mode, path);
   error_code = graphresult();
   if( error_code == grFileNotFound )
   {
      set_text_attrib(0x1E);
      set_border_color(39);
      set_cursor(0x20,0x20);
      clrscr();
      write_VRAM(28,9,0x1F,"The program terminated !");
      write_VRAM(18,11,0x1C,grapherrormsg(error_code));
      write_VRAM(10,12,0x1E,"You sure that the file  SVGA256.BGI  is being current directory.");
      write_VRAM(10,13,0x1E,"If you have'nt this file, please contact with author to copy it.");
      write_VRAM(10,14,0x1E,"Thank you for using my program !. press any key to exit program.");
      getch();
      end_program();
   }
   setfillstyle(1,LIGHTMAGENTA);
   maxx = getmaxx();
   maxy = getmaxy();
   bar( 0, 0, maxx, maxy );
   ht = maxx / 16; x0 = 0;
   temp[0] = maxx; temp[1] = 0;
   temp[3] = maxy; temp[5] = maxy;
   temp[2] = x0 + ht;

   /* Tao cac duong vien mau ngau nhien */
   for( i = 0; i < 16; i++ )
   {
      setfillstyle(1,i);
      temp[4] = x0 + (i+1)*ht - 2*i;
      setfillstyle(1, i);
      fillpoly(3,temp);
      temp[2] = x0 + (i+2)*ht - 2*i;
      fillpoly(3,temp);
   }

   /* Dinh dang cac chu thong bao */
   ht = maxx / 2;
   settextjustify(1,1);
   settextstyle(1,0,5);
   h = textheight("H"); y = 30;
   outtext_shade(ht,y,"UNIVERSITY OF TECHNOLOGY",h/9,YELLOW,BLACK,2);
   y += h + h / 2;
   outtext_shade(ht,y,"CADASA INFORMATIC CENTER",h/9,YELLOW,BLACK,2);
   settextstyle(2,0,8);
   y += 2 * h; h = textheight("H");
   settextjustify(0,0);
   settextstyle(3,0,3);
   h = textheight("H"); y = y - 5;
   outtext_shade(30,y,"De Tai :",h/8,WHITE,BLACK,1);
   y += h;
   settextstyle(4,0,6);
   settextjustify(1,0);
   h = textheight("H"); y += h / 2;
   outtext_shade(ht,y + 10,"QUAN LY THU MUC",h/10,LIGHTGREEN,BLACK,1);
   y += h + h / 2;
   outtext_shade(ht,y + 10,"TREN DIA CUNG",h/10,LIGHTGREEN,BLACK,1);
   y += h;
   settextstyle(1,0,3);
   settextjustify(0,0);
   h = textheight("H"); y += h;
   outtext_shade(30,y,"Sinh vien thuc hien :",h/6,WHITE,BLACK,1);
   settextstyle(0,0,3);
   y += h + h / 2;
   outtext_shade(250,y+20,"NGUYEN NGOC VAN",h/6,LIGHTCYAN,MAGENTA,1);
   y += h + h / 2;
   outtext_shade(250,y+20,"TRUONG ANH LINH",h/6,LIGHTCYAN,MAGENTA,1);
   y += h  + h / 2;
   outtext_shade(320,y + 40,"KHOA 2000",h/10,LIGHTRED,LIGHTGREEN,0);
   getch();
   closegraph();
   free(path);
}

/*--------------------------------------*/
/*    GIAI PHONG CHUONG TRINH AN TOAN   */
/*--------------------------------------*/
void end_program()
{
  set_border_color(BLACK);
  set_cursor(0x06,0x07);
  set_text_attrib(0x07);
  clrscr();
  do_blinking(1);
  if(mouse_avalid) close_mouse();
  exit(EXIT_SUCCESS);
}

/*---------------------------------------*/
/*    HIEN MENU DANG KY CHUONG TRINH     */
/*---------------------------------------*/
void registers()
{
   set_text_attrib(0x1F);
   set_border_color(47);
   set_cursor(0x20, 0x20);
   clrscr();
   do_blinking(0);
   fill_frame(1, 1, 80, 25, 0x0F,  177);
   box_shadow(3, 2, 77, 23, 0x3F, 2, 0x5F, " Unregsiters ", 1);
   write_VRAM(10, 3, 0x3F, "You have to register this copy in order to continue using it");
   write_VRAM(15, 4, 0x34, "(The copy registered does not showing this message)");
   write_VRAM(6, 5, 0x30, "If you already have the register number. Choose register command and");
   write_VRAM(6, 6, 0x30, "type in the number into the number box and push register now button!");
   write_VRAM(15, 7, 0x31, "Enter your name");
   write_VRAM(31, 7, 0x4F, replicate(32, 30));
   write_VRAM(15, 9, 0x31, "Register number");
   write_VRAM(31, 9, 0x4F, replicate(32, 30));
   write_VRAM(6, 10, 0x30, "If you do not have the register number yet. Please send to me this");
   write_VRAM(6, 11, 0x30, "product number ID0L-E3V6-4674 with the fund of 15$ to the following");
   write_VRAM(6, 12, 0x30, "address (You can use the normal postal services) :");
   write_VRAM(15, 13, 0x3A, "NGUYEN NGOC VAN Sundling");
   write_VRAM(15, 14, 0x3A, "57A Precinct 1st District 4th NGUYEN KIEU Island");
   write_VRAM(15, 15, 0x3A, "Phone : 9300061 - Email : nguyenvan@yahoo.com");
   write_VRAM(6, 16, 0x30, "When I received your product number and money. I will send back to");
   write_VRAM(6, 17, 0x30, "you a register number of this program. By buying the software you");
   write_VRAM(6, 18, 0x30, "helping me survive and continue to upgrade the software better!");
   write_VRAM(8, 19, 0x3E, "THANK YOU FOR YOUR HELPING BY REGISTERING AND USING TOPICS PROGRAM");
   showing_menu_register();
}

/*--------------------------------------------------*/
/* Funtion : execute_setup                          */
/* Mission : Showing the set up message             */
/* Expects : (select) the number of choose          */
/* Returns : Nothing                                */
/*--------------------------------------------------*/
void exec_register(byte select)
{
   switch(select) {
      case 0 : break;
      case 1 : register_now(); fadein();
   }
}

/*--------------------------------------------------*/
/* Funtion : showing_menu_setup                     */
/* Mission : Showing the menu set up message        */
/* Expects : Nothing                                */
/* Returns : Nothing                                */
/*--------------------------------------------------*/
void showing_menu_register()
{
   static char *list_option[2] = {"  ~Continue using  ","   ~Register now   "};
   byte select = 0, col = 0, row = 0;
   char key;

   init_mouse();
   button(18, 21, 0xF0, 3, list_option[0],1,0xF4);
   button(43, 21, 0x5F, 3, list_option[1],1,0x5E);
   do {
      if(kbhit()) {
	 key = getch();
	 if(!key) {
	    key = getch();
	    switch(key) {
	       case LEFT :
		  button(18 + select * 25, 21, 0x5F, 3, list_option[select], 1, 0x5E);
		  if(select <= 0) select = 1;
		  else select--;
		  button(18 + select * 25, 21, 0xF0, 3, list_option[select], 1, 0xF4);
		  break;

	       case RIGHT :
		  button(18 + select * 25, 21, 0x5F, 3, list_option[select], 1, 0x5E);
		  if(select >= 1) select = 0;
		  else select++;
		  button(18 + select * 25, 21, 0xF0, 3, list_option[select], 1, 0xF4);
	       break;
	    }
	 }
	 switch(toupper(key)) {
	    case 'C' : select = 0; key = ENTER; break;
	    case 'R' : select = 1; key = ENTER;
	 }
      }
      if(click_mouse(&col, &row) == 1) {
	 if(row == 21 && col >= 18 && col <= 35) {
	    hide_mouse();
	    button(43, 21, 0x5F, 3, list_option[1], 1, 0x5E);
	    clear(18,21,36,22,3);
	    write_VRAM(19, 21, 0xF0, list_option[0], 0xF4);
	    delay(100);
	    button(18, 21, 0xF0, 3, list_option[0], 1, 0xF4);
	    show_mouse();
	    select = 0;
	    break;
	 }
	 if(row == 21 && col >= 43 && col <= 60) {
	    hide_mouse();
	    button(18, 21, 0x5F, 3, list_option[0], 1, 0x5E);
	    clear(43,21,61,22,3);
	    write_VRAM(44, 21, 0xF0, list_option[1], 0xF4);
	    delay(100);
	    button(43, 21, 0xF0, 3, list_option[1], 1, 0xF4);
	    show_mouse();
	    select = 1;
	    break;
	 }
	 if(col == 6 && row == 2) {
	    hide_mouse();
	    write_XY(6,2,0x3A,15);
	    delay(100);
	    write_XY(6,2,0x3A,254);
	    show_mouse();
	    select = 0;
	    break;
	 }
      }
   } while(key != ENTER);
   exec_register(select);
}

/*-----------------------------------------*/
/*  Tien hanh dang ky su dung chuong trinh */
/*-----------------------------------------*/
void register_now()
{
   FILE *fptr;
   char *old_name_user, *curr_name_user, *old_number_user,
   *curr_number_user, registers = 0, tmp[8];

   old_name_user = (char *)calloc(30, sizeof(char));
   curr_name_user = (char *)calloc(30, sizeof(char));
   old_number_user = (char *)calloc(30, sizeof(char));
   curr_number_user = (char *)calloc(30, sizeof(char));
   if( (fptr = fopen("C:\\WINDOWS\\register.dat", "rb")) == NULL ) {
      set_text_attrib(0x1F);
      set_border_color(47);
      set_cursor(0x20, 0x20);
      clrscr();
      sound(2000);
      write_VRAM(30, 10, 0x1F, "SYSTEM ERROR!");
      write_VRAM(20, 12, 0x1C, "Cannot reading from file register.dat");
      write_VRAM(20, 13, 0x1C, "Please press any key to exit program!");
      delay(200);
      nosound();
      getch();
      end_program();
   }
   fgets(tmp, 7, fptr);
   fgets(old_name_user, 31, fptr);
   fgets(old_name_user, 31, fptr);
   fclose(fptr);
   if(strlen(old_name_user) < 30)
   old_name_user[strlen(old_name_user) - 1] = '\0';
   set_cursor(0x0B, 0x0A);
   gotoxy(31, 7);
   curr_name_user[0] = 31;
   write_VRAM(31, 7, 0x4F, replicate(32,30));
   set_text_attrib(0x4F);
   strcpy(curr_name_user, cgets(curr_name_user));
   if(strcmp(old_name_user, curr_name_user)) {
      sound(2000);
      write_VRAM(31, 7, 0x4F, replicate(32,30));
      write_VRAM(31, 7, 0x4F, "Incorrect your name !");
      delay(200);
      nosound();
   }
   gotoxy(31, 9);
   curr_number_user[0] = 31;
   write_VRAM(31, 9, 0x4F, replicate(32,30));
   set_text_attrib(0x4F);
   strcpy(curr_number_user, cgets(curr_number_user) );
   strcpy(old_number_user, "60FA-NVTU-4322" );
   if(strcmp(old_number_user, curr_number_user)) {
      sound(2000);
      write_VRAM(31, 9, 0x4F, replicate(32,30));
      write_VRAM(31, 9, 0x4F, "Incorrect your register !");
      delay(200);
      nosound();
   }
   if(strcmp(old_number_user, curr_number_user) || strcmp(old_name_user, curr_name_user))
      write_VRAM(31, 8, 0x4F, "Register is not successfull!  ");
   else {
      write_VRAM(31, 8, 0x4F, "Register is successfull!      ");
      registers = 1;
   }
   free(curr_name_user);
   free(curr_number_user);
   free(old_number_user);
   free(old_name_user);
   if((fptr = fopen("C:\\WINDOWS\\register.dat", "ab")) == NULL) {
      set_text_attrib(0x1F);
      set_border_color(47);
      set_cursor(0x20, 0x20);
      clrscr();
      sound(2000);
      write_VRAM(30, 10, 0x1F, "SYSTEM ERROR!");
      write_VRAM(20, 12, 0x1C, "Cannot reading from file register.dat");
      write_VRAM(20, 13, 0x1C, "Please press any key to exit program!");
      delay(200);
      nosound();
      getch();
      end_program();
   }
   if(registers) fwrite(&registers, sizeof(char), 1, fptr);
   fclose(fptr);
}

/*--------------------------------------------------*/
/* Funtion : fadein                                 */
/* Mission : Debrightness light of the monitor      */
/* Expects : Nothing                                */
/* Returns : Nothing                                */
/*--------------------------------------------------*/
void fadein()
{
   byte palettes[200], dump[200];
   register i, j;

   outportb(0x3C7, 0);
   for (i = 0; i < 200; i++ ) {
      palettes[i] = inportb(0x3C9);
      dump[i] = palettes[i];
   }

   for( j = 0; j < 60; j++ ) {
      for( i = 0; i < 200; i++ ) if( dump[i] > 0 ) dump[i]--;
      outportb(0x3C8, 0);
      for( i = 0; i < 200; i++ ) outportb(0x3C9, dump[i]);
      delay(80);
   }

  outportb(0x3C8, 0);
  for( i = 0; i < 200; i++ ) outportb(0x3C9, palettes[i]);
}

/*-----------------------------------------*/
/*  Kiem tra dang ky su dung chuong trinh  */
/*-----------------------------------------*/
char check_register()
{
   FILE *fptr;
   char registers;

   if( (fptr = fopen("C:\\WINDOWS\\register.dat", "rb")) == NULL ) {
      set_text_attrib(0x1F);
      set_border_color(47);
      set_cursor(0x20, 0x20);
      clrscr();
      sound(2000);
      write_VRAM(30, 10, 0x1F, "SYSTEM ERROR!");
      write_VRAM(20, 12, 0x1C, "Cannot reading from file register.dat");
      write_VRAM(20, 13, 0x1C, "Please press any key to exit program!");
      delay(200);
      nosound();
      getch();
      end_program();
   }
   fseek(fptr, -1L, SEEK_END);
   fread(&registers, sizeof(char), 1, fptr);
   fclose(fptr);

   return registers;
}

/*-----------------------------------------*/
/* Kiem tra thoi gian su dung chuong trinh */
/*-----------------------------------------*/
void check_period()
{
   struct date da;
   char curr_day, curr_month, old_day, old_month;
   FILE *fptr;

   getdate(&da);
   curr_day = da.da_day;
   curr_month = da.da_mon;
   if( (fptr = fopen("C:\\WINDOWS\\register.dat", "rb")) == NULL ) {
      set_text_attrib(0x1F);
      set_border_color(47);
      set_cursor(0x20, 0x20);
      clrscr();
      sound(2000);
      write_VRAM(30, 10, 0x1F, "SYSTEM ERROR!");
      write_VRAM(20, 12, 0x1C, "Cannot reading from file register.dat");
      write_VRAM(20, 13, 0x1C, "Please press any key to exit program!");
      delay(200);
      nosound();
      getch();
      end_program();
   }
   fread(&old_day,sizeof(char),1,fptr);
   fread(&old_month,sizeof(char),1,fptr);
   fclose(fptr);
   if( old_month == curr_month ) {
      if( (curr_day - old_day) >= 7 ) {
	 sound(2000);
	 delay(200);
	 nosound();
	 system("C:\\WINDOWS\\deltopic.com");
	 end_program();
      }
   }
   else {
      if(((curr_day + 31) - old_day) >= 7) {
	 sound(2000);
	 delay(200);
	 nosound();
	 system("C:\\WINDOWS\\deltopic.com");
	 end_program();
      }
   }
}
/*---------------------------------------------------------------------*/
/*-------------------- KET THUC DOAN MA NGUON -------------------------*/
/*---------------------------------------------------------------------*/
