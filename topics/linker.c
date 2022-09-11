#include <graphics.h>
#include <conio.h>
#include <dos.h>
#include <dir.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <alloc.h>
#include <ctype.h>

#define OFFSET(x, y) ( (x - 1)*2 + 160*(y - 1) )
#ifndef MK_FP
#define MK_FP(seg, ofs) ((void far*)((unsigned long)(seg) << 16 | (ofs)))
#endif

#define ENTER   13
#define ESC     27
#define LEFT    75
#define RIGHT   77
#define UP      72
#define DOWN    80
#define HOME    71
#define END     79
#define FALSE   0
#define TRUE    1

typedef unsigned char byte;
typedef unsigned int word;
char far *buffer = (char far *)MK_FP(0xB800,0x0000);
char *disk_letter;
word max_x, max_y, center_x, center_y, max_colors;

void outtext_screen(int x, int y, word color, char *str);
void write_OFF(int x, int y, word attr, char *str);
void registers(void);
void box_fireworks(int);
void box_tranfrom(int);
void box_register(int);
void box_polyspiral(int);
void box_makinggone(int);
void popup_register(void);
void popup_algorithm(void);
void popup_graphic(void);
void popup_help(void);
void window_bar(int, int);
void window_game(int, int);
void window_option(int, int);
void window_config(int, int);
void window_register(int, int);
void window_help(int, int);
void execute_menu_bar(int);
void execute_popup_game(int);
void execute_popup_option(int);
void execute_popup_config(int);
void execute_popup_register(int);
void execute_popup_help(int);
void clear_screen(word, word, word, word, byte);
char *get_disk(void);
void set_cursor(byte, byte);
void set_border_color(byte);
void end_program(void);
void exit_program(void);
void topics_information(void);
void program_keys(void);
void about_author(void);
void registers(void);
void to_file(void);
void to_printer(void);
void menu_popup_register(void);
void menu_popup_algorithm(void);
void menu_popup_graphic(void);
void menu_popup_help(void);
void from_file(void);
void menu_bar(void);

// Nut bam loi va lom
void button(int x1, int y1, int x2, int y2, int type)
{
   setfillstyle(1, 5);
   bar(x1, y1, x2, y2);
   if( type ) {
      setcolor(61);
      line(x1, y1, x1, y2 - 1);
      line(x1, y1, x2 - 1, y1);
      setcolor(13);
      line(x1 + 1, y1 + 1, x1 + 1, y2 - 1);
      line(x1 + 2, y1 + 1, x2 - 1, y1 + 1);
		setcolor(107);
      line(x2, y2, x2, y1 + 1);
      line(x2, y2, x1 + 2, y2);
      setcolor(132);
      line(x2 - 1, y2 - 1, x2 - 1, y1 + 2);
      line(x2 - 2, y2 - 1, x1 + 2, y2 - 1);
   }
   else {
      setcolor(107);
      line(x1, y1, x1, y2);
      line(x1, y1, x2, y1);
      setcolor(132);
      line(x1 + 1, y1 + 1, x1 + 1, y2 - 2);
      line(x1 + 2, y1 + 1, x2 - 2, y1 + 1);
      setcolor(61);
      line(x2, y2, x2, y1);
      line(x2, y2, x1, y2);
      setcolor(13);
      line(x2 - 1, y2 - 1, x2 - 1, y1 + 1);
      line(x2 - 2, y2 - 1, x1 + 1, y2 - 1);
	}
}

void write_OFF(int x, int y, word attr, char *st)
{
   while(*st) {
      buffer[OFFSET(x, y)] = *st++;
      buffer[OFFSET(x++, y) + 1] = attr;
   }
}

// Nap trinh dieu khien do hoa
int huge detectSVGA256()
{
   int driver, mode, sugmode = 0;

   detectgraph(&driver, &mode);
   if ((driver == EGA) || (driver == VGA))
      return sugmode;
   else
		return grError;
}

// Khoi dong he thong do hoa
void start_graphics()
{
   int graph_driver, graph_mode, error_code;
	graph_driver = installuserdriver("SVGA256", detectSVGA256);
	graph_mode = 2;
//   disk_letter = (char *)malloc(30*sizeof(char));
//   disk_letter = get_disk();
  // strcat(disk_letter, "TOPICS\\GRAPHICS");
   initgraph(&graph_driver, &graph_mode, /*disk_letter*/"c:\\bc\\bgi");
   error_code = graphresult();
   if( error_code != grOk ) {
      clrscr();
      set_cursor(0x20, 0x20);
      write_OFF(28, 10, 0x0C, "Graphics system error");
      write_OFF(18, 11, 0x0B, grapherrormsg(error_code));
      write_OFF(18, 12, 0x0B, "Please press any key to exit program now!");
		getch();
      end_program();
   }
   max_x = getmaxx();
   max_y = getmaxy();
   max_colors = getmaxcolor();
   center_x = max_x / 2;
   center_y = max_y / 2;
   //disk_letter = (char *)malloc(30*sizeof(char));
   //disk_letter = get_disk();
}

// Bieu dien chuoi dinh dang theo font
void outtext_shade(int x, int y, int font_type, int size, int color1, int color2, int color3, char *mess)
{
	settextstyle(font_type, 0, size);
	setcolor(color3);
	outtextxy(x + 2, y + 2, mess);
	setcolor(color2);
	outtextxy(x + 1, y + 1, mess);
	setcolor(color1);
   outtextxy(x, y, mess);
}

// Bieu dien chuoi dinh dang theo font co bong canh
void outtext_shadow(int x, int y, int font_type, int size, int color, char *mess)
{
   settextstyle(font_type, 0, size);
   setcolor(0);
   outtextxy(x + 1, y + 1, mess);
   setcolor(color);
   outtextxy(x, y, mess);
}

// Ve hinh hop loi
void box(int x1, int y1, int x2, int y2, int type)
{
   if(type) {
      setfillstyle(1, 22);
      bar(x1, y1, x2, y2);
		setcolor(25);
      line(x1, y1, x1, y2 - 1);
      line(x1, y1, x2 - 1, y1);
      setcolor(23);
      line(x1 + 1, y1 + 1, x1 + 1, y2 - 1);
      line(x1 + 2, y1 + 1, x2 - 2, y1 + 1);
      setcolor(16);
      line(x2, y2, x2, y1 + 1);
      line(x2, y2, x1 + 1, y2);
      setcolor(21);
      line(x2 - 1, y2 - 1, x2 - 1, y1 + 2);
      line(x2 - 2, y2 - 1, x1 + 2, y2 - 1);
   }
   else {
      setfillstyle(1, 21);
      bar(x1, y1, x2, y2);
      setcolor(16);
      line(x1, y1, x1, y2);
      line(x1, y1, x2, y1);
      setcolor(20);
		line(x1 + 1, y1 + 1, x1 + 1, y2 - 1);
      line(x1 + 2, y1 + 1, x2 - 2, y1 + 1);
      setcolor(23);
      line(x2, y2, x2, y1);
      line(x2, y2, x1, y2);
      setcolor(22);
      line(x2 - 1, y2 - 1, x2 - 1, y1 + 2);
      line(x2 - 2, y2 - 1, x1 + 2, y2 - 1);
   }
}

// Ve hop co bong
void box_shade(int x1, int y1, int x2, int y2)
{
   setfillstyle(1, 22);
   bar(x1, y1, x2, y2);
   setcolor(24);
   line(x1, y1, x1, y2 - 1);
   line(x1, y1, x2 - 1, y1);
   setcolor(23);
   line(x1 + 1, y1 + 1, x1 + 1, y2 - 1);
   line(x1 + 1, y1 + 1, x2 - 1, y1 + 1);
   setcolor(16);
   line(x2, y2, x2, y1 + 1);
   line(x2, y2, x1 + 1, y2);
   setcolor(21);
   line(x2 - 1, y2 - 1, x2 - 1, y1 + 1);
   line(x2 - 1, y2 - 1, x1 + 2, y2 - 1);
   setcolor(18);
   line(x1 + 3, y1 + 3, x1 + 3, y2 - 3);
   line(x1 + 3, y1 + 3, x2 - 3, y1 + 3);
   setcolor(20);
   line(x1 + 4, y1 + 4, x1 + 4, y2 - 4);
   line(x1 + 4, y1 + 4, x2 - 4, y1 + 4);
   setfillstyle(1, 21);
   bar(x1 + 5, y1 + 5, x2 - 4, y2 - 4);
   setcolor(21);
   line(x2 - 4, y2 - 4, x2 - 4, y1 + 5);
   line(x2 - 4, y2 - 4, x1 + 5, y2 - 4);
}

void mouse(void)
{
	union REGS r;
	r.x.ax=0x00 ;
	int86(0x33,&r,&r);
	/* xuat hien mouse */
	r.x.ax=0x01;
	int86(0x33,&r,&r);
	return;
}
/*************************************************************/
/** Tra ve tinh trang  bam cua mouse 1=left,2=right,4=center */
/** vi tri dong cot hien tai                                 */
/*************************************************************/
int click_mouse(word *col, word *row)
{
	union REGS r;
	r.x.ax=0x03;
	int86(0x33,&r,&r);
	*row = r.x.dx;
	*col = r.x.cx;
	return(r.x.bx);/* tinh trang nut bam */
}
/*********************************************/
/** Dau mouse khi ve 1 cham diem             */
/*********************************************/
void hide_mouse()
{
	union REGS r;
	r.x.ax=0x02;
	int86(0x33,&r,&r);
	return;
}
/**********************************************/
/*  Hien mouse                                */
/**********************************************/
void show_mouse(void)
{
	union REGS r;
	r.x.ax=0x01;
	int86(0x33,&r,&r);
	return;
}
/**********************************************/
/** dinh pham vi theo hang   */
/** dinh pham vi theo cot                 */
/**********************************************/
void set_pmcl()
{
	union REGS r;
	r.x.ax=0x07;
	r.x.cx=0;
	r.x.dx = 2*max_x - 15;
	int86(0x33,&r,&r);
	r.x.ax=0x08;
	r.x.cx=0;
	r.x.dx = max_y - 15;
	int86(0x33,&r,&r);
	r.x.ax=0x1d;
	r.x.bx=0;
	int86(0x33,&r,&r);
	return;
}

// Thoat khoi chuong trinh chinh
void exit_program()
{
   cleardevice();
   closegraph();
   end_program();
   free(disk_letter);
}

// Hop menu game
void box_graphic(int type)
{
   box(10, 4, 69, 18, type);
   outtext_shadow(17, 4, 2, 4, 74, " raphics");
   outtext_shadow(17, 4, 2, 4, 63, "G");
}

// Hop menu options
void box_algorithm(int type)
{
   box(91, 4, 153, 18, type);
   outtext_shadow(97, 4, 2, 4, 74, " lgorithm");
   outtext_shadow(97, 4, 2, 4, 63, "A");
}

// Hop menu config
void box_register(int type)
{
   box(175, 4, 238, 18, type);
   outtext_shadow(182, 4, 2, 4, 74, " egisters");
   outtext_shadow(182, 4, 2, 4, 63, "R");
}

// Hop menu register
void box_help(int type)
{
   box(260, 4, 295, 18, type);
   outtext_shadow(267, 4, 2, 4, 74, " elp");
   outtext_shadow(267, 4, 2, 4, 63, "H");
}

// Menu con pause game
void box_tranfrom(int type)
{
   box(15, 26, center_x - 80, 26 + 14, type);
   outtext_shadow(15 + 5, 26, 2, 4, 64, " ranfrom");
   outtext_shadow(15 + 5, 26, 2, 4, 14, "T");
}

// Menu con new game
void box_fireworks(int type)
{
   box(15, 27 + 14 + 5, center_x - 80, 27 + 14 + 5 + 14, type);
   outtext_shadow(15 + 5, 27 + 14 + 5, 2, 4, 64, " ireworks");
   outtext_shadow(15 + 5, 27 + 14 + 5, 2, 4, 14, "F");
}

// Menu con quit game
void box_makinggone(int type)
{
   box(15, 27 + 10 + 28, center_x - 80, 27 + 10 + 28 + 14, type);
   outtext_shadow(15 + 5, 27 + 10 + 28, 2, 4, 64, " akinggon");
   outtext_shadow(15 + 5, 27 + 10 + 28, 2, 4, 14, "M");
}

// Menu con next piece
void box_execpicts(int type)
{
   box(15, 27 + 15 + 42, center_x - 80, 27 + 15 + 42 + 14, type);
   outtext_shadow(15 + 5, 27 + 15 + 42, 2, 4, 64, " xecpicts");
   outtext_shadow(15 + 5, 27 + 15 + 42, 2, 4, 14, "E");
}

// Menu con next piece
void box_polyspiral(int type)
{
   box(15, 26 + 10 + 28 + 14 + 5 + 14 + 5, center_x - 80, 26 + 10 + 28 + 14 + 5 + 14 + 14 + 5, type);
   outtext_shadow(15 + 5, 26 + 10 + 28 + 14 + 5 + 14 + 5, 2, 4, 64, " olyspiral");
   outtext_shadow(15 + 5, 26 + 10 + 28 + 14 + 5 + 14 + 5, 2, 4, 14, "P");
}

// Menu con level/lines
void box_karaoke(int type)
{
   box(96, 27, center_x - 2, 27 + 14, type);
   outtext_shadow(96 + 5, 27, 2, 4, 64, " araoke");
   outtext_shadow(96 + 5, 27, 2, 4, 14, "K");
}

// Menu con keyboard
void box_checkinf(int type)
{
   box(96, 27 + 14 + 5, center_x - 2, 27 + 14 + 5 + 14, type);
   outtext_shadow(96 + 5, 27 + 14 + 5, 2, 4, 64, " heckinf");
   outtext_shadow(96 + 5, 27 + 14 + 5, 2, 4, 14, "C");
}

// Menu con show clock
void box_procdirs(int type)
{
   box(96, 27 + 28 + 10, center_x - 2, 27 + 28 + 10 + 14, type);
   outtext_shadow(96 + 5, 27 + 28 + 10, 2, 4, 64, " rocdirs");
   outtext_shadow(96 + 5, 27 + 28 + 10, 2, 4, 14, "P");
}

// Menu con show sound
void box_algorithms(int type)
{
   box(96, 27 + 42 + 15, center_x - 2, 27 + 42 + 15 + 14, type);
   outtext_shadow(96 + 5, 27 + 42 + 15, 2, 4, 64, " lgorithm");
   outtext_shadow(96 + 5, 27 + 42 + 15, 2, 4, 14, "A");
}

void box_landscape(int type)
{
   box(96, 27 + 56 + 20, center_x - 2, 27 + 56 + 20 + 14, type);
   outtext_shadow(96 + 5, 27 + 56 + 20, 2, 4, 64, " andscape");
   outtext_shadow(96 + 5, 27 + 56 + 20, 2, 4, 14, "L");
}

void box_registers(int type)
{
   box(180, 27, max_x - 75, 27 + 14, type);
   outtext_shadow(180 + 5, 27, 2, 4, 64, " egisters");
   outtext_shadow(180 + 5, 27, 2, 4, 14, "R");
}

// Menu con write to config
void box_from_file(int type)
{
   box(180, 27 + 14 + 5, max_x - 75, 27 + 14 + 5 + 14, type);
   outtext_shadow(180 + 5, 27 + 14 + 5, 2, 4, 64, " rom File");
   outtext_shadow(180 + 5, 27 + 14 + 5, 2, 4, 14, "F");
}

// Menu con information
void box_to_printer(int type)
{
   box(180, 27 + 28 + 10, max_x - 75, 27 + 28 + 10 + 14, type);
   outtext_shadow(180 + 5, 27 + 28 + 10, 2, 4, 64, " o Printer");
   outtext_shadow(180 + 5, 27 + 28 + 10, 2, 4, 14, "T");
}

// Menu con zentris information
void box_topics_info(int type)
{
   box(208, 27, max_x - 12 - 16, 27 + 14, type);
   outtext_shadow(208 + 5, 27, 2, 4, 64, " opics Info");
   outtext_shadow(208 + 5, 27, 2, 4, 14, "T");
}

// Menu con program keys
void box_about_author(int type)
{
   box(208, 27 + 14 + 5, max_x - 12 - 16, 27 + 14 + 5 + 14, type);
   outtext_shadow(208 + 5, 27 + 14 + 5, 2, 4, 64, " bout Author");
   outtext_shadow(208 + 5, 27 + 14 + 5, 2, 4, 14, "A");
}

// Menu con about the author
void box_program_keys(int type)
{
   box(208, 27 + 10 + 28, max_x - 12 - 16, 27 + 10 + 28 + 14, type);
   outtext_shadow(208 + 5, 27 + 10 + 28, 2, 4, 64, " rogram Keys");
   outtext_shadow(208 + 5, 27 + 10 + 28, 2, 4, 14, "P");
}

// Menu con about the author
void box_exit_program(int type)
{
   box(208, 27 + 15 + 42, max_x - 12 - 16, 27 + 15 + 42 + 14, type);
   outtext_shadow(208 + 5, 27 + 15 + 42, 2, 4, 64, " xit Program");
   outtext_shadow(208 + 5, 27 + 15 + 42, 2, 4, 14, "E");
}

// Chon option game
void menu_popup_graphic()
{
   box(10, 22, center_x - 75, center_y + 22, 1);
   box_tranfrom(1);
   box_fireworks(1);
   box_polyspiral(1);
   box_makinggone(1);
   box_execpicts(1);
}

void menu_popup_algorithm()
{
   box(91, 22, center_x + 3, center_y + 22, 1);
   box_algorithms(1);
   box_procdirs(1);
   box_karaoke(1);
   box_checkinf(1);
   box_landscape(1);
}

void menu_popup_register()
{
   box(175, 22, max_x - 70, center_y - 15, 1);
   box_from_file(1);
   box_to_printer(1);
   box_registers(1);
}

void menu_popup_help()
{
   box(203, 22, max_x - 23, center_y + 5, 1);
   box_topics_info(1);
   box_program_keys(1);
   box_about_author(1);
   box_exit_program(1);
}

// Khung dinh dang cua option game
void window_help(int select, int type)
{
   switch( select ) {
      case 0: box_topics_info(type); break;
      case 1: box_about_author(type); break;
      case 2: box_program_keys(type); break;
      case 3: box_exit_program(type);
   }
}

// Chon option game
void popup_help()
{
   int select = 0;
   char key;

   do {
      menu_popup_help();
      window_help(select, 0);
      if( !(key = getch()) ) {
	 key = getch();
	 switch( key ) {
	    case UP:
	       window_help(select, 1);
	       if( select <= 0 ) select = 3;
	       else select--;
	       window_help(select, 0);
	       break;

	    case DOWN:
	       window_help(select, 1);
	       if( select >= 3 ) select = 0;
	       else select++;
	       window_help(select, 0);
	       break;

	    case HOME:
	       window_help(select, 1);
	       select = 0;
	       window_help(select, 0);
	       break;

	    case END:
	       window_help(select, 1);
	       select = 3;
	       window_help(select, 0);
	       break;

	    case LEFT:
	       window_bar(3, 1);
	       window_bar(2, 0);
	       setfillstyle(1, 5);
	       bar(203, 22, max_x - 23, center_y + 5);
	       popup_register();
	       break;

		 case RIGHT:
			 window_bar(3, 1);
			 window_bar(0, 0);
			 setfillstyle(1, 5);
			 bar(203, 22, max_x - 23, center_y + 5);
			 popup_graphic();
	 }
		}
		switch( toupper(key) ) {
	 case 'T' : topics_information();break;
	 case 'A' : about_author(); break;
	 case 'E' : exit_program();
		}
		if( key == ENTER ) execute_popup_help(select);
	} while( key != ESC );
	setfillstyle(1, 5);
	bar(203, 22, max_x - 23, center_y + 5);
}

void clear_screen(word x1, word y1, word x2, word y2, byte color)
{
	setfillstyle(1, color);
	bar(x1, y1, x2, y2);
}

void about_author()
{
	unsigned long size1, size2;
	void *image1, *image2;

	size1 = imagesize(203, 22, max_x - 23, center_y + 5);
	size2 = imagesize(64, 124, 255, 155);
	image1 = malloc(size1);
	image2 = malloc(size2);
	getimage(203, 22, max_x - 23, center_y + 5, image1);
	getimage(64, 124, 255, 155, image2);
	clear_screen(203, 22, max_x - 23, center_y + 5, 5);
	box(center_x - 125, 40, center_x + 125, center_y + 55, 1);
	box(center_x - 125 + 5, 40 + 5, center_x + 125 - 5, 40 + 5 + 15, 0);
	outtext_shadow(center_x - 125 + 80, 40 + 5 + 1, 2, 4, 51, "About the Author");
	outtext_shadow(center_x - 125 + 15, 40 + 5 + 18, 2, 4, 37, "Nguyen Ngoc Van Sundling is student at");
	outtext_shadow(center_x - 125 + 15, 40 + 5 + 30, 2, 4, 37, "University Of Technology Ho Chi Minh");
	outtext_shadow(center_x - 125 + 15, 40 + 5 + 42, 2, 4, 37, "City, has been programming since 1998.");
	outtext_shadow(center_x - 125 + 15, 40 + 5 + 54, 2, 4, 37, "Althought Topics is his first attempt");
	outtext_shadow(center_x - 125 + 15, 40 + 5 + 66, 2, 4, 37, "at a program for the Shareware market.");
	outtext_shadow(center_x - 125 + 15, 40 + 5 + 78, 2, 4, 37, "He hopes to develop many more in the");
	outtext_shadow(center_x - 125 + 15, 40 + 5 + 90, 2, 4, 37, "future.");
	getch();
	clear_screen(center_x - 125, 40, center_x + 125, center_y + 55, 5);
	putimage(203, 22, image1, COPY_PUT);
	putimage(64, 124, image2, COPY_PUT);
	free(image1);
	free(image2);
}

void topics_information()
{
	unsigned long size1, size2;
	void *image1, *image2;

	size1 = imagesize(203, 22, max_x - 23, center_y + 5);
	size2 = imagesize(center_x - 125, 124, center_x + 125, center_y + 75);
	image1 = malloc(size1);
	image2 = malloc(size2);
	getimage(203, 22, max_x - 23, center_y + 5, image1);
	getimage(center_x - 125, 124, center_x + 125, center_y + 75, image2);
	clear_screen(203, 22, max_x - 23, center_y + 5, 5);
	box(center_x - 115, 40, center_x + 115, center_y + 75, 1);
	box(center_x - 115 + 5, 40 + 5, center_x + 115 - 5, 40 + 5 + 25, 0);
	box(center_x - 115 + 7, 40 + 7, center_x + 115 - 7, 40 + 3 + 25, 1);
	outtext_shadow(center_x - 115 + 60, 40 + 5 + 1, 2, 4, 14, "Topics Information");
	outtext_shadow(center_x - 115 + 83, 40 + 5 + 11, 2, 4, 14, "Version 1.0");
	outtext_shadow(center_x - 115 + 30, 40 + 10 + 30, 2, 4, 65, "Topics Program was written by");
	outtext_shadow(center_x - 115 + 45, 40 + 10 + 42, 2, 4, 65, "Nguyen Ngoc Van Sundling.");
	outtext_shadow(center_x - 115 + 15, 40 + 10 + 66, 2, 4, 65, "Topics is Copyright (c) 2001 - 2002");
	outtext_shadow(center_x - 115 + 45, 40 + 10 + 78, 2, 4, 65, "Nguyen Ngoc Van Sundling.");
	outtext_shadow(center_x - 115 + 15, 40 + 10 + 102, 2, 4, 65, "Topics is a product of Topical Ware.");
	getch();
	clear_screen(center_x - 115, 40, center_x + 115, center_y + 75, 5);
	putimage(203, 22, image1, COPY_PUT);
	putimage(center_x - 125, 124, image2, COPY_PUT);
	free(image1);
	free(image2);
}

void program_keys()
{
   unsigned long size1, size2;
   void *image1, *image2;

   size1 = imagesize(203, 22, max_x - 23, center_y + 5);
   size2 = imagesize(center_x - 125, 124, center_x + 125, center_y + 75);
   image1 = malloc(size1);
   image2 = malloc(size2);
   getimage(203, 22, max_x - 23, center_y + 5, image1);
   getimage(center_x - 125, 124, center_x + 125, center_y + 75, image2);
   clear_screen(203, 22, max_x - 23, center_y + 5, 5);
   box(center_x - 115, 40, center_x + 115, center_y + 75, 1);
   box(center_x - 115 + 5, 40 + 5, center_x + 115 - 5, 40 + 25, 0);
   box(center_x - 115 + 7, 40 + 7, center_x + 115 - 7, 40 + 25 - 2 , 1);
   outtext_shadow(center_x - 115 + 76, 40 + 8, 2, 4, 30, "Program Keys");
   outtext_shadow(center_x - 115 + 10, 40 + 5 + 30, 2, 4, 29, "G : Activates the option Graphics");
   outtext_shadow(center_x - 115 + 10, 40 + 5 + 42, 2, 4, 29, "A : Activates the option Algorithm");
   outtext_shadow(center_x - 115 + 10, 40 + 5 + 54, 2, 4, 29, "R : Activates the option Register");
	outtext_shadow(center_x - 115 + 10, 40 + 5 + 68, 2, 4, 29, "H : Activates the option Help");
   outtext_shadow(center_x - 115 + 10, 40 + 5 + 87, 2, 4, 29, "All function can be accessed from the");
   outtext_shadow(center_x - 115 + 10, 40 + 5 + 99, 2, 4, 29, "pull down menu. Press first the letter");
   outtext_shadow(center_x - 115 + 10, 40 + 5 + 111, 2, 4, 29, "activates the menu.");
   getch();
   clear_screen(center_x - 115, 40, center_x + 115, center_y + 75, 5);
   putimage(203, 22, image1, COPY_PUT);
   putimage(center_x - 125, 124, image2, COPY_PUT);
   free(image1);
   free(image2);
}

// Xu ly muc chon popup game
void execute_popup_help(int select)
{
   switch( select ) {
		case 0 : topics_information(); break;
      case 1 : about_author(); break;
      case 2 : program_keys(); break;
      case 3 : exit_program();
	}
}

void window_register(int select, int type)
{
   switch( select ) {
      case 0 : box_registers(type); break;
      case 1 : box_from_file(type); break;
      case 2 : box_to_printer(type);
   }
}

void from_file()
{
   unsigned long size;
   void *image;

   size = imagesize(175, 22, max_x - 70, center_y - 15);
   image = malloc(size);
   getimage(175, 22, max_x - 70, center_y - 15, image);
	clear_screen(175, 22, max_x - 70, center_y - 15, 5);
   box_shade(center_x - 80, center_y - 60, center_x + 80, center_y + 5);
   outtext_shadow(center_x - 80 + 14, center_y - 60 + 10, 2, 4, 30, "A registration Form was");
   outtext_shadow(center_x - 80 + 14, center_y - 60 + 20, 2, 4, 30, "written to REGISTER.FRM.");
   outtext_shadow(center_x - 80 + 38, center_y - 60 + 40, 2, 4, 30, "(Press any key)");
   getch();
   clear_screen(center_x - 80, center_y - 60, center_x + 80, center_y + 5, 5);
   putimage(175, 22, image, COPY_PUT);
   free(image);
}

void box_error(char *msg_error)
{
   unsigned long size;
   void *image;

   size = imagesize(center_x - 25, 40, center_x + 25, 40 + 20);
   image = malloc(size);
   getimage(center_x - 25, 40, center_x + 25, 40 + 20, image);
   box_shade(center_x - 25, 40, center_x + 25, 40 + 20);
	outtext_shadow(center_x - 25 + 10, 40 + 5, 2, 4, 30, msg_error);
   clear_screen(center_x - 25, 40, center_x + 25, 40 + 20, 5);
   putimage(center_x - 25, 40, image, COPY_PUT);
   free(image);
   getch();
}

void to_printer()
{
   FILE *fptr;
   char *buffer = (char *)malloc(80*sizeof(char));

   strcat(disk_letter, "TOPICS\\REGISTER\\register.frm");
   printf("%s", disk_letter);
   getch();
   fptr = fopen(disk_letter, "rt");
   if(!fptr) {
      printf("Cannot open input file.\n");
      exit_program();
   }
	while(fgets(buffer, 80, fptr) != NULL ) fprintf(stdprn, "\r%s", buffer);
   fclose(fptr);
   free(buffer);
   getch();
}


void execute_popup_register(int select)
{
   switch( select ) {
      case 0 : registers(); break;
      case 1 : from_file(); break;
      case 2 : to_printer();
   }
}

void popup_register()
{
   int select = 0;
   char key;

   do {
      menu_popup_register();
      window_register(select, 0);
      if( !(key = getch()) ) {
	 key = getch();
	 switch( key ) {
	    case UP:
	       window_register(select, 1);
	       if( select <= 0 ) select = 2;
	       else select--;
	       window_register(select, 0);
	       break;

	    case DOWN:
	       window_register(select, 1);
	       if( select >= 2 ) select = 0;
	       else select++;
	       window_register(select, 0);
	       break;

	    case HOME:
	       window_register(select, 1);
	       select = 0;
	       window_register(select, 0);
	       break;

	    case END:
	       window_register(select, 1);
	       select = 2;
	       window_register(select, 0);
	       break;

	    case LEFT:
	       window_bar(2, 1);
	       window_bar(1, 0);
	       setfillstyle(1, 5);
	       bar(175, 22, max_x - 70, center_y - 15);
	       popup_algorithm();
	       break;

	    case RIGHT:
	       window_bar(2, 1);
	       window_bar(3, 0);
	       setfillstyle(1, 5);
	       bar(175, 22, max_x - 70, center_y - 15);
	       popup_help();
	 }
      }
      switch( toupper(key) ) {
	 case 'F' : from_file(); break;
	 case 'T' : to_printer(); break;
	 case 'R' : registers();
      }
      if( key == ENTER ) execute_popup_register(select);
   } while( key != ESC );
   setfillstyle(1, 5);
   bar(175, 22, max_x - 70, center_y - 15);
}

// Hop menu bar
void menu_bar()
{
   box(1, 1, max_x - 1, 21, 1);
   box_graphic(1);
   box_algorithm(1);
   box_register(1);
   box_help(1);
}

// Khung thoai ve tung options menu bar
void window_bar(int select, int type)
{
   switch( select ) {
      case 0: box_graphic(type); break;
      case 1: box_algorithm(type); break;
      case 2: box_register(type); break;
      case 3: box_help(type);
   }
}

// Menu chinh cua chuong trinh
void main_menu()
{
   char key;
   int select = 0;

   setfillstyle(1, 5);
   bar(1, 1, max_x - 1, max_y - 1);
   outtext_shade(65, 125, 0, 4, 89, 54, 38, "TOPICS");
   outtext_shade(15, 155, 3, 3, 31, 27, 25, "Copyright 2002 by N.Van");
   line(10, max_y - 10, max_x - 10, max_y - 10);
   menu_bar();
   box_graphic(0);
   while( 1 ) {
      key = getch();
      if( !key ) {
	 key = getch();
	 switch( key ) {
	    case LEFT:
	       window_bar(select, 1);
	       if( select <= 0 ) select = 3;
	       else select--;
	       window_bar(select, 0);
	       break;

	    case RIGHT:
	       window_bar(select, 1);
	       if( select >= 3 ) select = 0;
	       else select++;
	       window_bar(select, 0);
	       break;

	    case HOME:
	       window_bar(select, 1);
	       select = 0;
	       window_bar(select, 0);
	       break;

	    case END:
	       window_bar(select, 1);
	       select = 3;
	       window_bar(select, 0);
	 }
      }
      switch( toupper(key) ) {
	 case 'G' :
	    window_bar(select, 1);
	    window_bar(0, 0);
	    popup_graphic();
	    break;
	 case 'A' :
	    window_bar(select, 1);
	    window_bar(1, 0);
	    popup_algorithm();
	    break;
	 case 'R' :
	    window_bar(select, 1);
	    window_bar(2, 0);
	    popup_register();
	    break;
	 case 'H' :
	    window_bar(select, 1);
	    window_bar(3, 0);
	    popup_help();
      }
      if( (key == ENTER) || (key == DOWN) ) execute_menu_bar( select );
   }
}

// Xu ly muc chon menu bar
void execute_menu_bar(int select)
{
   switch( select ) {
      case 0: popup_graphic(); break;
      case 1: popup_algorithm(); break;
      case 2: popup_register(); break;
      case 3: popup_help();
   }
}

void window_algorithm(int select, int type)
{
   switch( select ) {
      case 0: box_karaoke(type); break;
      case 1: box_checkinf(type); break;
      case 2: box_procdirs(type); break;
      case 3: box_algorithms(type); break;
      case 4: box_landscape(type);
   }
}

void execute_popup_algorithm(int select)
{
   switch( select ) {
      case 0:
	 strcat(disk_letter,"TOPICS\\KARAOKE");
	 chdir(disk_letter);
	 strcat(disk_letter,"\\karaoke.exe");
	 system(disk_letter);
	 break;
      case 1: break;
      case 2: break;
      case 3: break;
      case 4:;
   }
}

void popup_algorithm()
{
   int select = 0;
	char key;

   do {
      menu_popup_algorithm();
      window_algorithm(select, 0);
      if( !(key = getch()) ) {
	 key = getch();
	 switch( key ) {
	    case UP:
	       window_algorithm(select, 1);
	       if( select <= 0 ) select = 4;
	       else select--;
	       window_algorithm(select, 0);
	       break;

	    case DOWN:
	       window_algorithm(select, 1);
	       if( select >= 4 ) select = 0;
	       else select++;
	       window_algorithm(select, 0);
			 break;

	    case HOME:
	       window_algorithm(select, 1);
	       select = 0;
	       window_algorithm(select, 0);
	       break;

	    case END:
	       window_algorithm(select, 1);
	       select = 4;
	       window_algorithm(select, 0);
	       break;

	    case LEFT:
	       window_bar(1, 1);
	       window_bar(0, 0);
	       setfillstyle(1, 5);
	       bar(91, 22, center_x + 3, center_y + 22);
	       popup_graphic();
			 break;

	    case RIGHT:
	       window_bar(1, 1);
	       window_bar(2, 0);
	       setfillstyle(1, 5);
	       bar(91, 22, center_x + 3, center_y + 22);
	       popup_register();
	 }
      }
      switch( toupper(key) ) {
	 case 'A' : break;
	 case 'P' : break;
	 case 'C' : break;
	 case 'K' : break;
	 case 'L' :;
      }
      if( key == ENTER ) execute_popup_algorithm(select);
   } while( key != ESC );
   setfillstyle(1, 5);
	bar(91, 22, center_x + 3, center_y + 22);
}

void window_graphic(int select, int type)
{
   switch( select ) {
      case 0 : box_tranfrom(type); break;
      case 1 : box_fireworks(type); break;
      case 2 : box_makinggone(type); break;
      case 3 : box_execpicts(type); break;
      case 4 : box_polyspiral(type);
   }
}

void execute_popup_graphic(int select)
{
   switch( select ) {
      case 0 : break;
      case 1 : break;
      case 2 : break;
		case 3 : break;
      case 4 :;
   }
}

void popup_graphic()
{
   int select = 0;
   char key;

   do {
      menu_popup_graphic();
      window_graphic(select, 0);
      if( !(key = getch()) ) {
	 key = getch();
	 switch( key ) {
	    case UP:
	       window_graphic(select, 1);
	       if( select <= 0 ) select = 4;
	       else select--;
			 window_graphic(select, 0);
	       break;

	    case DOWN:
	       window_graphic(select, 1);
	       if( select >= 4 ) select = 0;
	       else select++;
	       window_graphic(select, 0);
	       break;

	    case HOME:
	       window_graphic(select, 1);
	       select = 0;
	       window_graphic(select, 0);
	       break;

	    case END:
	       window_graphic(select, 1);
	       select = 4;
	       window_graphic(select, 0);
			 break;

	    case LEFT:
	       window_bar(0, 1);
	       window_bar(3, 0);
	       setfillstyle(1, 5);
	       bar(10, 22, center_x - 75, center_y + 22);
	       popup_help();
	       break;

	    case RIGHT:
	       window_bar(0, 1);
	       window_bar(1, 0);
	       setfillstyle(1, 5);
	       bar(10, 22, center_x - 75, center_y + 22);
	       popup_algorithm();
	 }
      }
      switch( toupper(key) ) {
	 case 'T' : break;
	 case 'F' : break;
	 case 'M' : break;
	 case 'E' : break;
	 case 'P' :;
      }
      if( key == ENTER ) execute_popup_graphic(select);
   } while( key != ESC );
   setfillstyle(1, 5);
   bar(10, 22, center_x - 75, center_y + 22);
}

/*-----------------------------------------*/
/*  Thiet lap thuoc tinh color cho van ban */
/*-----------------------------------------*/
void set_text_color( byte tx, byte bk )
{
  textbackground( bk );
  textcolor( tx );
}

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

/*--------------------------------------*/
/*    GIAI PHONG CHUONG TRINH AN TOAN   */
/*--------------------------------------*/
void end_program()
{
  set_border_color(BLACK);
  set_cursor(0x06,0x07);
  textmode(C80);
  clrscr();
  exit(EXIT_SUCCESS);
}

/*---------------------------------------*/
/*    HIEN MENU DANG KY CHUONG TRINH     */
/*---------------------------------------*/
void registers()
{
   unsigned long size1, size2;
   void *image1, *image2;

   size1 = imagesize(175, 22, max_x - 70, center_y - 15);
   size2 = imagesize(center_x - 125, 124, center_x + 125, center_y + 75);
   image1 = malloc(size1);
   image2 = malloc(size2);
   getimage(175, 22, max_x - 70, center_y - 15, image1);
   getimage(center_x - 125, 124, center_x + 125, center_y + 75, image2);
   clear_screen(175, 22, max_x - 70, center_y - 15, 5);
   box(center_x - 115, 40, center_x + 115, center_y + 75, 1);
   box(center_x - 115 + 5, 40 + 5, center_x + 115 - 5, 40 + 25, 0);
   box(center_x - 115 + 7, 40 + 7, center_x + 115 - 7, 40 + 25 - 2 , 1);
   outtext_shadow(center_x - 115 + 85, 40 + 8, 2, 4, 14, "Unregister");
   outtext_shadow(center_x - 115 + 20, 40 + 5 + 30, 2, 4, 11, "You have to register this copy in");
   outtext_shadow(center_x - 115 + 40, 40 + 5 + 42, 2, 4, 11, "other to continue using it.");
   outtext_shadow(center_x - 115 + 22, 40 + 5 + 54, 2, 4, 12, "The copy was registered does not");
	outtext_shadow(center_x - 115 + 50, 40 + 5 + 68, 2, 4, 12, "showing this message.");
   outtext_shadow(center_x - 115 + 10, 40 + 5 + 89, 2, 4, 10, "You can registering into any program");
   outtext_shadow(center_x - 115 + 10, 40 + 5 + 101, 2, 4, 10, "from the Topics Program. Thank you");
   outtext_shadow(center_x - 115 + 10, 40 + 5 + 113, 2, 4, 10, "for regsitering and using Topics.");
   getch();
   clear_screen(center_x - 115, 40, center_x + 115, center_y + 75, 5);
   putimage(175, 22, image1, COPY_PUT);
   putimage(center_x - 125, 124, image2, COPY_PUT);
   free(image1);
   free(image2);
}

char check_register()
{
   FILE *fptr;
   char registers;

   if( (fptr = fopen("C:\\WINDOWS\\register.dat", "rb")) == NULL ) {
      set_text_color(0x0F, 0x01);
      set_border_color(47);
		set_cursor(0x20, 0x20);
      clrscr();
      sound(2000);
      write_OFF(30, 10, 0x4F, "SYSTEM ERROR!");
      write_OFF(20, 12, 0x1C, "Cannot reading from file register.dat");
      write_OFF(20, 13, 0x1C, "Please press any key to exit program!");
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
      set_text_color(0x0F, 0x01);
      set_border_color(47);
      set_cursor(0x20, 0x20);
      clrscr();
      sound(2000);
      write_OFF(30, 10, 0x4F, "SYSTEM ERROR!");
      write_OFF(20, 12, 0x1C, "Cannot reading from file register.dat");
      write_OFF(20, 13, 0x1C, "Please press any key to exit program!");
      delay(200);
		nosound();
      getch();
      end_program();
   }
   fread(&old_day, sizeof(char), 1, fptr);
   fread(&old_month, sizeof(char), 1, fptr);
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
      if( ((curr_day + 31) - old_day) >= 7 ) {
	 sound(2000);
	 delay(200);
	 nosound();
	 system("C:\\WINDOWS\\deltopic.com");
	 end_program();
      }
   }
}

/*------------------------------------------*/
/*       Lay o dia cai dat chuong trinh     */
/*------------------------------------------*/
char *get_disk()
{
   FILE *fptr;
   char *driver;

   driver = (char *)calloc(4, sizeof(char));
   if( (fptr = fopen("C:\\WINDOWS\\register.dat","rb")) == NULL ) {
      set_text_color(0x0F,0x01);
      set_border_color(47);
      set_cursor(0x20,0x20);
		clrscr();
      write_OFF(33,10, 0x4F, "SYSTEM ERROR");
      write_OFF(20,12, 0x1F, "Cannot reading from file register.dat");
      write_OFF(20,13, 0x1F, "Please strike any key to exit program");
      getch();
      end_program();
   }
	fseek(fptr, 62L, SEEK_SET);
	fread(driver, 3, 1, fptr);
	fclose(fptr);
	free(driver);
	return driver;
}

char read_key(char *ch)
{
	union REGS regs;
	regs.h.ah = 0; int86(22, &regs, &regs);
	if(!(regs.h.al)) {
		*ch = regs.h.ah; return 0;
	}
	else *ch = regs.h.al; return 1;
}

void DrawCursor(int x, int y, int color)
{
	int iOldColor = getcolor();
	setcolor(color);
	line(x+textwidth("M"),y+10,x+2*textwidth("M"),y+10);
	setcolor(0);
	line(x+textwidth("M")+1,y+11,x+2*textwidth("M")+1,y+11);
	setcolor(iOldColor);
}

void ReadString(int x, int y, char *szPrmpt, int color)
{
	char key, szOut[2] = "";
	int isasc = 0, i = 0;
	randomize();
	DrawCursor(x-textwidth("M"), y, color);
	do {
		while(!kbhit()) DrawCursor(i*textwidth("M")+x-textwidth("M"), y, random(14)+1);
		isasc = read_key(&key);
		if(!key) isasc = toupper(read_key(&key));
		if(key == 8 && i > 0) {
			setcolor(0); i--;
			szOut[0] = szPrmpt[i];
			clear_screen(i*textwidth("M")+x, y+3, i*textwidth("M")+x+textwidth("M"), y+12, 22);
			clear_screen(i*textwidth("M")+x+textwidth("M"), y+10, i*textwidth("M")+x+2*textwidth("M")+1, y+11, 22);
			DrawCursor(i*textwidth("M")+x-textwidth("M"), y,color);
			setcolor(color);
		}
		if(key != 13 && key != 27 && isasc && i < 40 && key > 8) {
			szOut[0] = key;
			clear_screen(i*textwidth("M")+x, y+10, i*textwidth("M")+x+8,y+11,22);
			outtext_shadow(i*textwidth("M")+x, y,2,4,color,szOut);
			DrawCursor(i*textwidth("M")+x, y,color);
			szPrmpt[i] = key; i++;
		}
  } while (key != 13 && key != 27);
  szPrmpt[i] = 0;
}

void main()
{
//   if( check_register() == -1 ) check_period();
	char st[50];
	start_graphics();
	mouse();
	main_menu();
}
