/*
 *  graph.h     Graphics functions
 *
 * =========================================================================
 *
 *                          Open Watcom Project
 *
 * Copyright (c) 2004-2020 The Open Watcom Contributors. All Rights Reserved.
 * Portions Copyright (c) 1983-2002 Sybase, Inc. All Rights Reserved.
 *
 *    This file is automatically generated. Do not edit directly.
 *
 * =========================================================================
 */
#ifndef _GRAPH_H_INCLUDED
#define _GRAPH_H_INCLUDED

#ifndef _ENABLE_AUTODEPEND
 #pragma read_only_file
#endif

#ifndef __COMDEF_H_INCLUDED
 #include <_comdef.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#pragma library (graph)

#ifdef _M_IX86
 #pragma pack( __push, 1 )
#else
 #pragma pack( __push, 8 )
#endif

typedef short   grcolor;

struct xycoord {                /* structure for pixel position */
        short   xcoord;
        short   ycoord;
};

struct _wxycoord {              /* structure for window position*/
        double  wx;
        double  wy;
};

struct rccoord {                /* structure for text position  */
        short   row;
        short   col;
};

struct videoconfig {            /* structure for _getvideoconfig */
        short   numxpixels;
        short   numypixels;
        short   numtextcols;
        short   numtextrows;
        grcolor numcolors;
        short   bitsperpixel;
        short   numvideopages;
        short   mode;
        short   adapter;
        short   monitor;
        short   memory;
};

struct textsettings {           /* structure for _gettextsettings */
        short   basevectorx;
        short   basevectory;
        short   txpath;
        short   height;
        short   width;
        short   spacing;
        short   horizalign;
        short   vertalign;
};

struct _fontinfo {              /* structure for _getfontinfo */
        short   type;
        short   ascent;
        short   pixwidth;
        short   pixheight;
        short   avgwidth;
        char    filename[ 81 ];
        char    facename[ 32 ];
};

/* Calling conventions for -fpi(87) are different than for -fpc */
#ifdef __FPI__
 #define _arc_w              _arc_w_87
 #define _ellipse_w          _ellipse_w_87
 #define _floodfill_w        _floodfill_w_87
 #define _getimage_w         _getimage_w_87
 #define _getpixel_w         _getpixel_w_87
 #define _getviewcoord_w     _getviewcoord_w_87
 #define _grtext_w           _grtext_w_87
 #define _imagesize_w        _imagesize_w_87
 #define _lineto_w           _lineto_w_87
 #define _moveto_w           _moveto_w_87
 #define _pie_w              _pie_w_87
 #define _putimage_w         _putimage_w_87
 #define _rectangle_w        _rectangle_w_87
 #define _setcharsize_w      _setcharsize_w_87
 #define _setcharspacing_w   _setcharspacing_w_87
 #define _setpixel_w         _setpixel_w_87
 #define _setwindow          _setwindow_87
#endif

/* Video Setup and Query Functions */

_WCRTLINK extern short
    _WCI86FAR _setvideomode( short );
_WCRTLINK extern short
    _WCI86FAR _setvideomoderows( short, short );
_WCRTLINK extern struct videoconfig _WCI86FAR *
    _WCI86FAR _getvideoconfig( struct videoconfig _WCI86FAR * );
_WCRTLINK extern short
    _WCI86FAR _grstatus( void );
_WCRTLINK extern short
    _WCI86FAR _setactivepage( short );
_WCRTLINK extern short
    _WCI86FAR _getactivepage( void );
_WCRTLINK extern short
    _WCI86FAR _setvisualpage( short );
_WCRTLINK extern short
    _WCI86FAR _getvisualpage( void );

#define _MAXRESMODE     (-3)    /* graphics mode with highest res. */
#define _MAXCOLORMODE   (-2)    /* graphics mode with most colours */
#define _DEFAULTMODE    (-1)    /* restore screen to original mode */
#define _TEXTBW40       0       /* 40 x 25 text, 16 grey           */
#define _TEXTC40        1       /* 40 x 25 text, 16/8 color        */
#define _TEXTBW80       2       /* 80 x 25 text, 16 grey           */
#define _TEXTC80        3       /* 80 x 25 text, 16/8 color        */
#define _MRES4COLOR     4       /* 320 x 200, 4 color              */
#define _MRESNOCOLOR    5       /* 320 x 200, 4 grey               */
#define _HRESBW         6       /* 640 x 200, BW                   */
#define _TEXTMONO       7       /* 80 x 25 text, BW                */
#define _HERCMONO       11      /* 720 x 350, BW                   */
#define _MRES16COLOR    13      /* 320 x 200, 16 color             */
#define _HRES16COLOR    14      /* 640 x 200, 16 color             */
#define _ERESNOCOLOR    15      /* 640 x 350, BW                   */
#define _ERESCOLOR      16      /* 640 x 350, 4 or 16 color        */
#define _VRES2COLOR     17      /* 640 x 480, BW                   */
#define _VRES16COLOR    18      /* 640 x 480, 16 color             */
#define _MRES256COLOR   19      /* 320 x 200, 256 color            */
#define _URES256COLOR   0x100   /* 640 x 400, 256 color            */
#define _VRES256COLOR   0x101   /* 640 x 480, 256 color            */
#define _SVRES16COLOR   0x102   /* 800 x 600, 16 color             */
#define _SVRES256COLOR  0x103   /* 800 x 600, 256 color            */
#define _XRES16COLOR    0x104   /* 1024 x 768, 16 color            */
#define _XRES256COLOR   0x105   /* 1024 x 768, 256 color           */
#define _YRES16COLOR    0x106   /* 1280 x 1024, 16 color           */
#define _YRES256COLOR   0x107   /* 1280 x 1024, 256 color          */
#define _SVTEXTC80X60   0x108   /* 80 x 60 text                    */
#define _SVTEXTC132X25  0x109   /* 132 x 25 text                   */
#define _SVTEXTC132X43  0x10A   /* 132 x 43 text                   */
#define _SVTEXTC132X50  0x10B   /* 132 x 50 text                   */
#define _SVTEXTC132X60  0x10C   /* 132 x 60 text                   */
#define _MRES32KCOLOR   0x10D   /* 320 x 200, 32K color            */
#define _MRES64KCOLOR   0x10E   /* 320 x 200, 64K color            */
#define _MRESTRUECOLOR  0x10F   /* 320 x 200, TRUE color           */
#define _VRES32KCOLOR   0x110   /* 640 x 480, 32K color            */
#define _VRES64KCOLOR   0x111   /* 640 x 480, 64K color            */
#define _VRESTRUECOLOR  0x112   /* 640 x 480, 16.8M color          */
#define _SVRES32KCOLOR  0x113   /* 800 x 600, 32K color            */
#define _SVRES64KCOLOR  0x114   /* 800 x 600, 64K color            */
#define _SVRESTRUECOLOR 0x115   /* 800 x 600, 16.8M color          */
#define _XRES32KCOLOR   0x116   /* 1024 x 768, 32K color           */
#define _XRES64KCOLOR   0x117   /* 1024 x 768, 64K color           */
#define _XRESTRUECOLOR  0x118   /* 1024 x 768, 16.8M color         */
#define _YRES32KCOLOR   0x119   /* 1280 x 1024, 32K color          */
#define _YRES64KCOLOR   0x11A   /* 1280 x 1024, 64K color          */
#define _YRESTRUECOLOR  0x11B   /* 1280 x 1024, 16.8M color        */
#define _ZRES256COLOR   0x11C   /* 1600 x 1200, 256 color          */
#define _ZRES32KCOLOR   0x11D   /* 1600 x 1200, 32K color          */
#define _ZRES64KCOLOR   0x11E   /* 1600 x 1200, 64K color          */
#define _ZRESTRUECOLOR  0x11F   /* 1600 x 1200, 16.8M color        */

#define _NODISPLAY      (-1)    /* no display device            */
#define _UNKNOWN        0       /* unknown adapter/monitor type */

#define _MDPA           1       /* monochrome display/printer adapter */
#define _CGA            2       /* colour/graphics monitor adapter    */
#define _HERCULES       3       /* Hercules monochrome adapter card   */
#define _MCGA           4       /* PS/2 Model 30 monitor              */
#define _EGA            5       /* enhanced graphics adapter          */
#define _VGA            6       /* vector graphics array              */
#define _SVGA           7       /* super VGA                          */
#define _HGC            _HERCULES

#define _MONO           1       /* regular monochrome */
#define _COLOR          2       /* regular color      */
#define _ENHANCED       3       /* enhanced color     */
#define _ANALOGMONO     5       /* analog monochrome  */
#define _ANALOGCOLOR    6       /* analog color       */

#define _GROK                   0      /* no error                     */
#define _GRERROR                (-1)   /* graphics error               */
#define _GRMODENOTSUPPORTED     (-2)   /* video mode not supported     */
#define _GRNOTINPROPERMODE      (-3)   /* function n/a in this mode    */
#define _GRINVALIDPARAMETER     (-4)   /* invalid parameter(s)         */
#define _GRINSUFFICIENTMEMORY   (-5)   /* out of memory                */
#define _GRFONTFILENOTFOUND     (-6)   /* can't open font file         */
#define _GRINVALIDFONTFILE      (-7)   /* font file has invalid format */
#define _GRNOOUTPUT             1      /* nothing was done             */
#define _GRCLIPPED              2      /* output clipped               */

/* Colour Setting and Query Functions */

_WCRTLINK extern grcolor
    _WCI86FAR _setcolor( grcolor );
_WCRTLINK extern grcolor
    _WCI86FAR _getcolor( void );
_WCRTLINK extern long
    _WCI86FAR _setbkcolor( long );
_WCRTLINK extern long
    _WCI86FAR _getbkcolor( void );
_WCRTLINK extern long
    _WCI86FAR _remappalette( short, long );
_WCRTLINK extern short
    _WCI86FAR _remapallpalette( long _WCI86FAR * );
_WCRTLINK extern short
    _WCI86FAR _selectpalette( short );

#define _BLACK          0x000000L
#define _BLUE           0x2a0000L
#define _GREEN          0x002a00L
#define _CYAN           0x2a2a00L
#define _RED            0x00002aL
#define _MAGENTA        0x2a002aL
#define _BROWN          0x00152aL
#define _WHITE          0x2a2a2aL
#define _GRAY           0x151515L
#define _LIGHTBLUE      0x3F1515L
#define _LIGHTGREEN     0x153f15L
#define _LIGHTCYAN      0x3f3f15L
#define _LIGHTRED       0x15153fL
#define _LIGHTMAGENTA   0x3f153fL
#define _YELLOW         0x153f3fL
#define _BRIGHTWHITE    0x3f3f3fL
#define _LIGHTYELLOW    _YELLOW

/* Shape and Curve Drawing Functions */

_WCRTLINK extern short
    _WCI86FAR _lineto( short, short );
_WCRTLINK extern short
    _WCI86FAR _lineto_w( double, double );
_WCRTLINK extern short
    _WCI86FAR _rectangle( short, short, short, short, short );
_WCRTLINK extern short
    _WCI86FAR _rectangle_w( short, double, double, double, double );
_WCRTLINK extern short
    _WCI86FAR _rectangle_wxy( short, struct _wxycoord _WCI86FAR *, struct _wxycoord _WCI86FAR * );
_WCRTLINK extern short
    _WCI86FAR _arc( short, short, short, short, short, short, short, short );
_WCRTLINK extern short
    _WCI86FAR _arc_w( double, double, double, double, double, double, double, double );
_WCRTLINK extern short
    _WCI86FAR _arc_wxy( struct _wxycoord _WCI86FAR *,
                                  struct _wxycoord _WCI86FAR *,
                                  struct _wxycoord _WCI86FAR *,
                                  struct _wxycoord _WCI86FAR * );
_WCRTLINK extern short
    _WCI86FAR _ellipse( short, short, short, short, short );
_WCRTLINK extern short
    _WCI86FAR _ellipse_w( short, double, double, double, double );
_WCRTLINK extern short
    _WCI86FAR _ellipse_wxy( short, struct _wxycoord _WCI86FAR *, struct _wxycoord _WCI86FAR * );
_WCRTLINK extern short
    _WCI86FAR _pie( short, short, short, short, short, short, short, short, short );
_WCRTLINK extern short
    _WCI86FAR _pie_w( short, double, double, double, double, double, double, double, double );
_WCRTLINK extern short
    _WCI86FAR _pie_wxy( short, struct _wxycoord _WCI86FAR *,
                                  struct _wxycoord _WCI86FAR *,
                                  struct _wxycoord _WCI86FAR *,
                                  struct _wxycoord _WCI86FAR * );
_WCRTLINK extern short
    _WCI86FAR _polygon( short, short, struct xycoord _WCI86FAR * );
_WCRTLINK extern short
    _WCI86FAR _polygon_w( short, short, double _WCI86FAR * );
_WCRTLINK extern short
    _WCI86FAR _polygon_wxy( short, short, struct _wxycoord _WCI86FAR * );
_WCRTLINK extern short
    _WCI86FAR _floodfill( short, short, grcolor );
_WCRTLINK extern short
    _WCI86FAR _floodfill_w( double, double, grcolor );
_WCRTLINK extern grcolor
    _WCI86FAR _setpixel( short, short );
_WCRTLINK extern grcolor
    _WCI86FAR _setpixel_w( double, double );
_WCRTLINK extern grcolor
    _WCI86FAR _getpixel( short, short );
_WCRTLINK extern grcolor
    _WCI86FAR _getpixel_w( double, double );
_WCRTLINK extern short
    _WCI86FAR _getarcinfo( struct xycoord _WCI86FAR *, struct xycoord _WCI86FAR *, struct xycoord _WCI86FAR * );

/* Position Determination Functions */

_WCRTLINK extern struct xycoord
    _WCI86FAR _getcurrentposition( void );
_WCRTLINK extern struct _wxycoord
    _WCI86FAR _getcurrentposition_w( void );
_WCRTLINK extern struct xycoord
    _WCI86FAR _getviewcoord( short, short );
_WCRTLINK extern struct xycoord
    _WCI86FAR _getviewcoord_w( double, double );
_WCRTLINK extern struct xycoord
    _WCI86FAR _getviewcoord_wxy( struct _wxycoord _WCI86FAR * );
_WCRTLINK extern struct xycoord
    _WCI86FAR _getphyscoord( short, short );
_WCRTLINK extern struct _wxycoord
    _WCI86FAR _getwindowcoord( short, short );
_WCRTLINK extern struct xycoord
    _WCI86FAR _moveto( short, short );
_WCRTLINK extern struct _wxycoord
    _WCI86FAR _moveto_w( double, double );
_WCRTLINK extern struct xycoord
    _WCI86FAR _setvieworg( short, short );

#define _getlogcoord    _getviewcoord        /* for compatibility */
#define _setlogorg      _setvieworg

/* Output Determination Functions */

_WCRTLINK extern void
    _WCI86FAR _setfillmask( unsigned char _WCI86FAR * );
_WCRTLINK extern unsigned char _WCI86FAR *
    _WCI86FAR _getfillmask( unsigned char _WCI86FAR * );
_WCRTLINK extern void
    _WCI86FAR _setlinestyle( unsigned short );
_WCRTLINK extern unsigned short
    _WCI86FAR _getlinestyle( void );
_WCRTLINK extern short
    _WCI86FAR _setplotaction( short );
_WCRTLINK extern short
    _WCI86FAR _getplotaction( void );

#define _setwritemode   _setplotaction      /* for compatibility */
#define _getwritemode   _getplotaction

enum {                          /* plotting action */
        _GOR, _GAND, _GPRESET, _GPSET, _GXOR
};

/* Screen Manipulation Functions */

_WCRTLINK extern void
    _WCI86FAR _clearscreen( short );
_WCRTLINK extern void
    _WCI86FAR _setviewport( short, short, short, short );
_WCRTLINK extern void
    _WCI86FAR _setcliprgn( short, short, short, short );
_WCRTLINK extern void
    _WCI86FAR _getcliprgn( short _WCI86FAR *, short _WCI86FAR *,
                                     short _WCI86FAR *, short _WCI86FAR * );
_WCRTLINK extern short
    _WCI86FAR _displaycursor( short );
_WCRTLINK extern short
    _WCI86FAR _wrapon( short );
_WCRTLINK extern short
    _WCI86FAR _setwindow( short, double, double, double, double );

#define _GCLEARSCREEN   0
#define _GVIEWPORT      1
#define _GWINDOW        2

#define _GBORDER        2
#define _GFILLINTERIOR  3

enum {                          /* cursor display */
        _GCURSOROFF, _GCURSORON
};

enum {                          /* text wrapping */
        _GWRAPOFF, _GWRAPON
};

/* Graphics Text Manipulation Functions and Constants */

_WCRTLINK extern struct textsettings _WCI86FAR *
    _WCI86FAR _gettextsettings( struct textsettings _WCI86FAR * );
_WCRTLINK extern void
    _WCI86FAR _gettextextent( short, short, char _WCI86FAR *,
                           struct xycoord _WCI86FAR *, struct xycoord _WCI86FAR * );
_WCRTLINK extern void
    _WCI86FAR _setcharsize( short, short );
_WCRTLINK extern void
    _WCI86FAR _setcharsize_w( double, double );
_WCRTLINK extern void
    _WCI86FAR _settextalign( short, short );
_WCRTLINK extern void
    _WCI86FAR _settextpath( short );
_WCRTLINK extern void
    _WCI86FAR _settextorient( short, short );
_WCRTLINK extern void
    _WCI86FAR _setcharspacing( short );
_WCRTLINK extern void
    _WCI86FAR _setcharspacing_w( double );
_WCRTLINK extern short
    _WCI86FAR _grtext( short, short, char _WCI86FAR * );
_WCRTLINK extern short
    _WCI86FAR _grtext_w( double, double, char _WCI86FAR * );

enum {                          /* horizontal alignment */
        _NORMAL, _LEFT, _CENTER, _RIGHT
};

enum {                          /* vertical alignment */
        _TOP=1, _CAP, _HALF, _BASE, _BOTTOM
};

enum {                          /* text path */
        _PATH_RIGHT, _PATH_LEFT, _PATH_UP, _PATH_DOWN
};

/* Text Manipulation Functions */

#define _GSCROLLUP      1
#define _GSCROLLDOWN    (-1)
#define _MAXTEXTROWS    (-1)

_WCRTLINK extern void
    _WCI86FAR _settextwindow( short, short, short, short );
_WCRTLINK extern void
    _WCI86FAR _outtext( char _WCI86FAR * );
_WCRTLINK extern grcolor
    _WCI86FAR _settextcolor( grcolor );
_WCRTLINK extern grcolor
    _WCI86FAR _gettextcolor( void );
_WCRTLINK extern struct rccoord
    _WCI86FAR _settextposition( short, short );
_WCRTLINK extern struct rccoord
    _WCI86FAR _gettextposition( void );
_WCRTLINK extern void
    _WCI86FAR _scrolltextwindow( short );
_WCRTLINK extern void
    _WCI86FAR _gettextwindow( short _WCI86FAR *, short _WCI86FAR *,
                                         short _WCI86FAR *, short _WCI86FAR * );
_WCRTLINK extern short
    _WCI86FAR _gettextcursor( void );
_WCRTLINK extern short
    _WCI86FAR _settextcursor( short );
_WCRTLINK extern void
    _WCI86FAR _outmem( unsigned char _WCI86FAR *, short );
_WCRTLINK extern short
    _WCI86FAR _settextrows( short );

/* Image Manipulation Functions */

_WCRTLINK extern void
    _WCI86FAR _getimage( short, short, short, short, char _WCI86HUGE * );
_WCRTLINK extern void
    _WCI86FAR _getimage_w( double, double, double, double, char _WCI86HUGE * );
_WCRTLINK extern void
    _WCI86FAR _getimage_wxy( struct _wxycoord _WCI86FAR *,
                                        struct _wxycoord _WCI86FAR *,
                                        char _WCI86HUGE * );
_WCRTLINK extern void
    _WCI86FAR _putimage( short, short, char _WCI86HUGE *, short );
_WCRTLINK extern void
    _WCI86FAR _putimage_w( double, double, char _WCI86HUGE *, short );
_WCRTLINK extern long
    _WCI86FAR _imagesize( short, short, short, short );
_WCRTLINK extern long
    _WCI86FAR _imagesize_w( double, double, double, double );
_WCRTLINK extern long
    _WCI86FAR _imagesize_wxy( struct _wxycoord _WCI86FAR *,
                                         struct _wxycoord _WCI86FAR * );
/* Font Manipulation Functions */

_WCRTLINK extern short
    _WCI86FAR _registerfonts( char _WCI86FAR * );
_WCRTLINK extern void
    _WCI86FAR _unregisterfonts( void );
_WCRTLINK extern short
    _WCI86FAR _setfont( char _WCI86FAR * );
_WCRTLINK extern short
    _WCI86FAR _getfontinfo( struct _fontinfo _WCI86FAR * );
_WCRTLINK extern void
    _WCI86FAR _outgtext( char _WCI86FAR * );
_WCRTLINK extern short
    _WCI86FAR _getgtextextent( char _WCI86FAR * );
_WCRTLINK extern struct xycoord
    _WCI86FAR _setgtextvector( short, short );
_WCRTLINK extern struct xycoord
    _WCI86FAR _getgtextvector( void );


#pragma pack( __pop )

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
