/*---------------------------------------------------------------*/
/*                  GFXLIB Graphics Library                      */
/*            Full supports 8/15/16/24/32 bits color             */
/*       Support both page flipping and double buffering         */
/*   Using linear frame buffer and VESA 3.0 core functions       */
/*             Use double buffer for rendering                   */
/*            Environment: Open Watcom C/C++ 1.9                 */
/*             OS: DOS32A 32bits protected mode                  */
/* Compile: wcl386 -wx -zq -ecc -op -ol -ot -bcl=dos32a gfxlib.c */
/*             Target OS: DOS32A / PMODEW                        */
/*                Author: Nguyen Ngoc Van                        */
/*                Create: 25/05/2001                             */
/*           Last update: 10/06/2022                             */
/*               Version: 1.2.6                                  */
/*               Website: http://codedemo.net                    */
/*                 Email: pherosiden@gmail.com                   */
/*            References: http://crossfire-designs.de            */
/*---------------------------------------------------------------*/
/* NOTE: You can use this code freely and any purpose without    */
/* any warnings. Please leave the copyright.                     */
/*---------------------------------------------------------------*/

#include <dos.h>
#include <math.h>
#include <time.h>
#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdint.h>
#include <float.h>
#include <graph.h>

// Enable MMX features
#define _USE_MMX

// VBE Capabilities flags
#define VBE_CAPS_DAC8           1           // bit 0
#define VBE_CAPS_NOTVGA         2           // bit 1
#define VBE_CAPS_BLANKFN9       4           // bit 2
#define VBE_CAPS_HWSTEREO       8           // bit 3
#define VBE_CAPS_STEREOEVC      16          // bit 4

// VBE Mode attributes flags
#define VBE_MASK_MODEHW         1           // bit 0
#define VBE_MASK_HASLFB         128         // bit 7
#define VBE_MASK_HW3BUF         1024        // bit 10

// VBE Memory model flags
#define VBE_MM_PACKED           4
#define VBE_MM_DCOLOR           6
#define VBE_MM_YUV              7

// VBE protected mode constant
#define VBE_DATA_SIZE           0x600       // VBE data size
#define VBE_STACK_SIZE          0x2000      // VBE stack size
#define VBE_CODE_SIZE           0x8000      // VBE code size

// VBE CRTC timing value
#define CRTC_DOUBLE_SCANLINE    1           // bit 0
#define CRTC_INTERLACED         2           // bit 1
#define CRTC_HSYNC_NEGATIVE     4           // bit 2
#define CRTC_VSYNC_NEGATIVE     8           // bit 3

// Projection constant
#define ECHE                    0.77        // must be round for each monitor

// Define M_PI
#define M_PI                    3.14159265  // redefine PI constant
#define M_2PI                   6.28318530  // redefine 2PI constant

// Time defination
#define MIN_TRIES               10
#define MAX_RUN                 5
#define PIT_RATE                0x1234DD
#define WAIT_TIME               50000

// Rountine functions
#define GetTicks()              *((uint32_t*)0x046C)
#define Swap(a, b)              {a ^= b; b ^= a; a ^= b;}
#define clamp(x, lo, hi)        (min(max(x, lo), hi))

// Standard timing default parameters
#define MARGIN_PERCENT          1.8         // % of active image width/height
#define CELL_GRAN               8           // character cell granularity
#define H_SYNC_PERCENT          8.0         // % of line period for nominal hsync
#define MIN_VSYNC_BP            550         // min. time of vsync + back porch (microsec)
#define V_SYNC                  4           // # lines for vsync
#define MIN_V_PORCH             3           // min. # lines for vertical front porch
#define MIN_V_BP                6           // min. # lines for vertical back porch
#define CLOCK_STEP              250         // pixel clock step size (kHz)

// Generalized blanking limitation formula constants
#define M                       600         // blanking formula gradient
#define C                       40          // blanking formula offset
#define K                       128         // blanking formula scaling factor
#define J                       20          // blanking formula scaling factor weighting

// C' and M' are part of the Blanking Duty Cycle computation
#define M_PRIME                 (M * K / 256)
#define C_PRIME                 (((C - J) * K / 256) + J)

// Fixed number
#define FIXED(x)                ((uint32_t)((x) * 1024))

// XFN font style
#define GFX_FONT_FIXED          0x01        // fixed font (all character have same size)
#define GFX_FONT_MULTI          0x02        // multiple font
#define GFX_FONT_ANIMATE        0x04        // animation font
#define GFX_FONT_ANIPOS         0x08        // random position font
#define GFX_FONT_SCALEABLE      0x10        // scaleable font
#define GFX_FONT_VECTOR         0x20        // vector font (like CHR, BGI font)
#define GFX_FONT_LOCKED         0x01000000  // locked font
#define GFX_BUFF_SIZE           131072      // maximum GFXLIB buffer
#define GFX_MAX_FONT            5           // maximum GFXLIB font loaded at same time

// Bitmap mouse and button
#define MOUSE_WIDTH             24          // mouse width
#define MOUSE_HEIGHT            24          // mouse height
#define MOUSE_SIZE              576         // mouse size (mouse width * mouse height)

#define BUTTON_WIDTH            48          // button width
#define BUTTON_HEIGHT           24          // button height
#define BUTTON_SIZE             1152        // button size (button width * button height)
#define BUTTON_BITMAPS          3

#define LEFT_BUTTON             0           // mouse left button pressed
#define MIDDLE_BUTTON           1           // mouse middle button pressed
#define RIGHT_BUTTON            2           // mouse right button pressed

#define STATE_NORM              0           // mouse state nornal
#define STATE_ACTIVE            1           // mouse state active
#define STATE_PRESSED           2           // mouse state pressed
#define STATE_WAITING           3           // mouse state waiting

#define NUM_BUTTONS             2           // 'Exit' and 'Click' button
#define NUM_MOUSE_BITMAPS       9           // total mouse pointer bitmaps

// Cohen-Sutherland clip constanst
#define CLIP_LEFT               1           // left code
#define CLIP_RIGHT              2           // right code
#define CLIP_BOTTOM             4           // bottom code
#define CLIP_TOP                8           // top code

// Fill poly constant
#define MAX_POLY_CORNERS        200         // max polygon corners
#define MAX_STACK_SIZE          2000        // max stack size use to scan

// CPUID defines constanst
#define CPUID_EAX               0           // index of EAX value in array
#define CPUID_EBX               1           // index of EBX value in array
#define CPUID_ECX               2           // index of ECX value in array
#define CPUID_EDX               3           // index of EDX value in array

#define CPUID_STD_BASE          0x00000000  // standard leaves base
#define CPUID_HV_BASE           0x40000000  // hypervisor leaves base
#define CPUID_EXT_BASE          0x80000000  // extended leaves base
#define CPUID_EXT_BASE1         0x80000001  // AMD entended leaves base
#define CPUID_BRAND_1ST         0x80000002  // first brand string leaf
#define CPUID_CENTAUR_BASE      0xC0000000  // centaur leaves base
#define CPUID_BRAND_COUNT       3           // number of brand leaves

// Processor manufacturer
#define PROCESSOR_INTEL         0           // INTEL processor
#define PROCESSOR_AMD           1           // AMD processor
#define PROCESSOR_UNKNOWN       2           // unknown processor

// Simulation real mode registers interfaces
#pragma pack(push, 1)
typedef struct
{
    uint32_t    edi;
    uint32_t    esi;
    uint32_t    ebp;
    uint32_t    rsvd;
    uint32_t    ebx;
    uint32_t    edx;
    uint32_t    ecx;
    uint32_t    eax;
    uint16_t    flags;
    uint16_t    es;
    uint16_t    ds;
    uint16_t    fs;
    uint16_t    gs;
    uint16_t    ip;
    uint16_t    cs;
    uint16_t    sp;
    uint16_t    ss;
} RM_REGS;

// VBE driver information
typedef struct
{
    uint8_t     VBESignature[4];        // VBE Info Block signature ('VESA')
    uint16_t    VBEVersion;             // VBE version (must be 0x0200 or 0x0300)
    uint32_t    OEMStringPtr;           // OEM string
    uint32_t    Capabilities;           // capabilities of graphics controller
    uint32_t    VideoModePtr;           // video mode pointer
    uint16_t    TotalMemory;            // # 64kb memory blocks

    // VBE 2.0 extensions
    uint16_t    OemSoftwareRev;         // VBE implementation software revision
    uint32_t    OemVendorNamePtr;       // vendor name
    uint32_t    OemProductNamePtr;      // product name
    uint32_t    OemProductRevPtr;       // product revision
    uint8_t     Reserved[222];          // VBE implementation scratch area
    uint8_t     OemData[256];           // data area for OEM strings
} VBE_DRIVER_INFO;

// VBE mode information
typedef struct
{
    // for all VBE revisions
    uint16_t    ModeAttributes;         // mode attributes
    uint8_t     WinAAttributes;         // window A attributes
    uint8_t     WinBAttributes;         // window B attributes
    uint16_t    WinGranularity;         // window granularity
    uint16_t    WinSize;                // window size
    uint16_t    WinASegment;            // window A start segment
    uint16_t    WinBSegment;            // window B start segment
    uint32_t    WinFuncPtr;             // real mode pointer to window function
    uint16_t    BytesPerScanline;       // bytes per scan line

    // VBE 1.2+
    uint16_t    XResolution;            // horizontal resolution in pixels (characters)
    uint16_t    YResolution;            // vertical resolution in pixels (characters)
    uint8_t     XCharSize;              // character cell width in pixels
    uint8_t     YCharSize;              // character cell height in pixels
    uint8_t     NumberOfPlanes;         // number of memory planes
    uint8_t     BitsPerPixel;           // bits per pixel
    uint8_t     NumberOfBanks;          // number of banks
    uint8_t     MemoryModel;            // memory model type
    uint8_t     BankSize;               // bank size in KB
    uint8_t     NumberOfImagePages;     // number of images
    uint8_t     Reserved1;              // reserved for page function

    // VBE 1.2+ Direct color fields (required for direct/6 and YUV/7 memory models)
    uint8_t     RedMaskSize;            // size of direct color red mask in bits
    uint8_t     RedFieldPosition;       // bit posision of LSB of red mask
    uint8_t     GreenMaskSize;          // size of direct color green mask in bits
    uint8_t     GreenFieldPosition;     // bit posision of LSB of green mask
    uint8_t     BlueMaskSize;           // size of direct color blue mask in bits
    uint8_t     BlueFieldPosition;      // bit posision of LSB of blue mask
    uint8_t     RsvdMaskSize;           // size of direct color reserved mask in bits
    uint8_t     RsvdFieldPosition;      // bit posision of LSB of reserved mask
    uint8_t     DirectColorModeInfo;    // direct color mode attributes

    // VBE 2.0+
    uint32_t    PhysBasePtr;            // physical address for flat memory frame buffer
    uint32_t    OffScreenMemOffset;     // reserved must be 0
    uint16_t    OffScreenMemSize;       // reserved must be 0

    // VBE 3.0+
    uint16_t    LinBytesPerScanline;    // linear modes: bytes per scanline
    uint8_t     BnkNumberOfImagePages;  // banked modes: number of images
    uint8_t     LinNumberOfImagePages;  // linear modes: number of images
    uint8_t     LinRedMaskSize;         // linear modes: size of direct color red mask
    uint8_t     LinRedFieldPosition;    // linear modes: bit position of LSB of red mask
    uint8_t     LinGreenMaskSize;       // linear modes: size of direct color green mask
    uint8_t     LinGreenFieldPosition;  // linear modes: bit position of LSB of green mask
    uint8_t     LinBlueMaskSize;        // linear modes: size of direct color blue mask
    uint8_t     LinBlueFieldPosition;   // linear modes: bit position of LSB of blue mask
    uint8_t     LinRsvdMaskSize;        // linear modes: size of direct color rsvd mask
    uint8_t     LinRsvdFieldPosition;   // linear modes: bit position of LSB of rsvd mask
    uint32_t    MaxPixelClock;          // maximum pixel clock (in Hz) for graphics mode
    uint8_t     Reserved2[189];         // padding data
} VBE_MODE_INFO;

// VESA 2.0 protected mode interface
typedef struct
{
    uint16_t    SetWindow;              // SetWindow selector
    uint16_t    setDisplayStart;        // setDisplayStart selector
    uint16_t    setPalette;             // SetPallete selector
    uint16_t    IOPrivInfo;             // IO privileged info
} VBE_PM_INFO;

// VESA 3.0 protected mode info block
typedef struct
{
    uint8_t     Signature[4];           // PM Info Block signature ('PMID')
    uint16_t    EntryPoint;             // offset of PM entry points within BIOS
    uint16_t    PMInitialize;           // offset of PM initialization entry points
    uint16_t    BIOSDataSel;            // selector to BIOS data area emulation block
    uint16_t    A0000Sel;               // selector to 0xa0000
    uint16_t    B0000Sel;               // selector to 0xb0000
    uint16_t    B8000Sel;               // selector to 0xb8000
    uint16_t    CodeSegSel;             // selector to access code segment as data
    uint8_t     InProtectMode;          // true if in protected mode
    uint8_t     Checksum;               // sum of all bytes in this struct must match 0
} VBE_PM_INFO_BLOCK;

// VESA 3.0 CRTC timings structure
typedef struct
{
    uint16_t    HorizontalTotal;        // horizontal total in pixels
    uint16_t    HorizontalSyncStart;    // horizontal sync start in pixels
    uint16_t    HorizontalSyncEnd;      // horizontal sync end in pixels
    uint16_t    VerticalTotal;          // vertical total in lines
    uint16_t    VerticalSyncStart;      // vertical sync start in lines
    uint16_t    VerticalSyncEnd;        // vertical sync end in lines
    uint8_t     Flags;                  // flags (double-scan, interlaced, h/v-sync polarity)
    uint32_t    PixelClock;             // pixel clock in units of Hz
    uint16_t    RefreshRate;            // refresh rate in units of 0.01 Hz
    uint8_t     Reserved[40];           // reserved for later
} VBE_CRTC_INFO_BLOCK;

// VESA 3.0 far call memory struct (48 bits address)
typedef struct
{
    uint32_t    offset;                 // 32 bits offset
    uint16_t    segment;                // 16 bits segment
} VBE_FAR_CALL;

// VBE 3.0 call registers
typedef struct {
    uint16_t    ax;
    uint16_t    bx;
    uint16_t    cx;
    uint16_t    dx;
    uint16_t    si;
    uint16_t    di;
    uint16_t    es;
} VBE_CALL_REGS16;

// VBE 3.0 call stack
typedef struct {
    uint32_t    *esp;
    uint32_t    *ss;
} VBE_CALL_STACK;

// Palette structure
typedef struct
{
    uint8_t   b;
    uint8_t   g;
    uint8_t   r;
} RGB;

// RGB structure (use for VESA 2.0 setPaletteRange function)
typedef struct
{
    uint8_t   r;
    uint8_t   g;
    uint8_t   b;
    uint8_t   a;
} PAL;

// ARGB structure (32bits color)
typedef struct
{
    uint8_t   b;
    uint8_t   g;
    uint8_t   r;
    uint8_t   a;
} ARGB;

typedef struct
{
    double   x;
    double   y;
} POINT;

typedef struct
{
    int32_t count;                      // number of stack elements
    int32_t data[MAX_STACK_SIZE];       // stack data
} GFX_STACK;

// GFXLIB vector stroke info
typedef struct
{
    uint8_t     code;                   // stroke code (0: unuse, 1: moveto, 2: lineto)
    uint8_t     x, y;                   // stroke coordinates
} GFX_STROKE_INFO;

// GFXLIB vector stroke data
typedef struct
{
    uint8_t     width;                  // stroke width
    uint8_t     height;                 // stroke height
    uint16_t    numOfLines;             // number of strokes
} GFX_STROKE_DATA;

// GFXLIB font info table
typedef struct
{
    uint32_t    startOffset;            // offset of the font start
    uint8_t     bitsPerPixel;           // bits per pixel
    uint16_t    bytesPerLine;           // bytes per line (BMP-font)
    uint16_t    width;                  // font width
    uint16_t    height;                 // font height
    uint16_t    baseLine;               // baseLine of the character
    uint16_t    descender;              // font desender
    uint16_t    startChar;              // start of character
    uint16_t    endChar;                // end of character
    uint8_t     distance;               // distance between characters
    uint8_t     randomX;                // only <> 0 if flag anipos on
    uint8_t     randomY;                // only <> 0 if flag anipos on
    uint32_t    usedColors;             // only use for BMP8 font
    uint32_t    spacer;                 // distance for non-existing chars
    uint8_t     reserved[10];           // reserved for later use
} GFX_CHAR_INFO;

// GFXLIB font header
typedef struct
{
    char            sign[4];            // font signature 'Fnt2'
    uint16_t        version;            // version number 0x0101
    char            name[32];           // name of font
    char            copyRight[32];      // font copy-right (use for BGI font)
    char            fontType[4];        // font type BMP1, BMP8, VECT, ...
    uint16_t        subFonts;           // number of sub-fonts (difference size)
    uint8_t*        dataPtr;            // address of raw font data
    uint32_t        memSize;            // memory size on load raw data
    uint32_t        flags;              // font flags (ANIPOS, ANIMATION, MULTI, ...)
    GFX_CHAR_INFO   info;               // sub-fonts data info
} GFX_FONT;

// the structure for store bitmap data
typedef struct
{
    uint32_t    bmWidth;                // bitmap width (in pixel)
    uint32_t    bmHeight;               // bitmap height (in pixel)
    uint32_t    bmPixels;               // byte per pixels
    uint32_t    bmRowBytes;             // bytes per sscan line
    uint8_t     bmExtra[768];           // extra data (palate for 256 color, rgb mask for 15/16 bit, rgb pos)
    uint8_t*    bmData;                 // raw image data
} GFX_BITMAP;

// the structure of image data (base image for GFXLIB)
typedef struct
{
    uint32_t    mWidth;                 // image width
    uint32_t    mHeight;                // image height
    uint32_t    mPixels;                // image bytes per pixels
    uint32_t    mRowBytes;              // image bytes per line
    uint32_t    mSize;                  // image size in bytes
    uint8_t*    mData;                  // image raw data
} GFX_IMAGE;

// mouse callback data
typedef struct {
    uint16_t    max;                    // mouse code event
    uint16_t    mbx;                    // callback param bx
    uint16_t    mcx;                    // callback param cx
    uint16_t    mdx;                    // callback param dx
} MOUSE_CALLBACK_DATA;

// the structure for animated mouse pointers
typedef struct tagMOUSEBITMAP MOUSE_BITMAP;
struct tagMOUSEBITMAP
{
    uint32_t        mbHotX;             // mouse hotspot x
    uint32_t        mbHotY;             // mouse hotspot y
    uint8_t*        mbData;             // mouse bitmap data
    MOUSE_BITMAP*   mbNext;             // points to next mouse data
};

// the structure for a bitmap mouse pointer.
typedef struct
{
    uint32_t        msState;            // status
    uint32_t        msNumBtn;           // number of buttons
    uint32_t        msPosX;             // current pos x
    uint32_t        msPosY;             // current pos y
    uint32_t        msWidth;            // mouse image width
    uint32_t        msHeight;           // mouse image height
    uint32_t        msPixels;           // mouse image byte per pixel
    uint8_t*        msUnder;            // mouse under bacground
    MOUSE_BITMAP*   msBitmap;           // hold mouse bitmap info
} GFX_MOUSE_IMAGE;

// the structure for a bitmap button.
typedef struct
{
    uint32_t    btPosX;                 // button x
    uint32_t    btPosY;                 // button y
    uint32_t    btState;                // button state (normal, hover, click, disable, ...)
    uint32_t    btWidth;                // button width (each state)
    uint32_t    btHeight;               // button height (each state)
    uint32_t    btPixels;               // button bytes per pixel
    uint8_t*    btData[BUTTON_BITMAPS]; // hold mouse bitmap data
} GFX_BUTTON_IMAGE;

// BMP header format
typedef struct
{
    uint16_t    bfType;                 // must be 'BM' 
    uint32_t    bfSize;                 // size of the whole bitmap file
    uint16_t    bfReserved1;            // must be 0
    uint16_t    bfReserved2;            // must be 0
    uint32_t    bfOffBits;              // offset to data, bytes
} BMP_HEADER;

// BMP info format
typedef struct
{
    uint32_t    biSize;                 // size of the structure
    int         biWidth;                // bitmap bmWidth
    int         biHeight;               // bitmap bmHeight
    uint16_t    biPlanes;               // number of colour planes
    uint16_t    biBitCount;             // bits per pixel
    uint32_t    biCompression;          // compression
    uint32_t    biSizeImage;            // size of the data in bytes
    int         biXPelsPerMeter;        // pixels per meter x
    int         biYPelsPerMeter;        // pixels per meter y
    uint32_t    biClrUsed;              // colors used
    uint32_t    biClrImportant;         // important colors
} BMP_INFO;

// DMPI memory status info block
typedef struct {
    uint32_t    LargestBlockAvail;      // total system memory
    uint32_t    MaxUnlockedPage;        // maximun unlockable page
    uint32_t    LargestLockablePage;    // largest lockable page
    uint32_t    LinAddrSpace;           // linear space address
    uint32_t    NumFreePagesAvail;      // number of free page avaiable
    uint32_t    NumPhysicalPagesFree;   // number of physical free (page)
    uint32_t    TotalPhysicalPages;     // total physical page
    uint32_t    FreeLinAddrSpace;       // total free linear address space
    uint32_t    SizeOfPageFile;         // size of page
    uint32_t    Reserved[3];
} MEM_INFO;

// Rotate clip data
typedef struct
{
    int32_t     srcw, srch;             // source width and height
    int32_t     dstw, dsth;             // destination width and height
    int32_t     srcx, srcy;             // source strart x, y

    int32_t     ax, ay;                 // left, right
    int32_t     bx, by;                 // top, bottom
    int32_t     cx, cy;                 // center x, y

    int32_t     boundWidth;             // boundary width (pixels units)
    int32_t     currUp0, currUp1;       // current up
    int32_t     currDown0, currDown1;   // current down

    int32_t     yUp, yDown;             // y top and down

    int32_t     outBound0, outBound1;   // in-bound and out-bound
    int32_t     inBound0, inBound1;     // in-bound and out-bound
} ROTATE_CLIP;

#pragma pack(pop)

// INTEL CPU features description
const char *edxFeatures[][2] =
{
    {"On-chip x87", "FPU"},
    {"Virtual 8086 Mode Enhancements", "VME"},
    {"Debugging Extensions", "DE"},
    {"Page Size Extensions", "PSE"},
    {"Time Stamp Counter", "TSC"},
    {"RDMSR/WRMSR Instructions", "MSR"},
    {"Physical Address Extensions", "PAE"},
    {"Machine Check Exception", "MCE"},
    {"CMPXCHG8B Instruction", "CX8"},
    {"On-chip APIC", "APIC"},
    {"Reserved 1", "R1!!"},                         // bit 10
    {"SYSENTER/SYSEXIT Instructions", "SEP"},
    {"Memory Type Range Registers", "MTRR"},
    {"PTE Global Bit", "PGE"},
    {"Machine Check Architecture", "MCA"},
    {"Conditional Move Instructions", "CMOV"},
    {"Page Attribute Table", "PAT"},
    {"36-bit Page Size Extension", "PSE-36"},
    {"Processor Serial Number", "PSN"},
    {"CFLUSH Instruction", "CLFSH"},
    {"Reserved 2", "R2!!"},                         // bit 20
    {"Debug Store", "DS"},
    {"Thermal Monitor and Clock Control", "ACPI"},
    {"MMX Technology", "MMX"},
    {"FXSAVE/FXRSTOR Instructions", "FXSR"},
    {"SSE Extensions", "SSE"},
    {"SSE2 Extensions", "SSE2"},
    {"Self Snoop", "SS"},
    {"Hyper-threading Technology", "HTT"},
    {"Thermal Monitor", "TM"},
    {"Reserved 3", "R3!!"},                         // bit 30
    {"Pending Break Enable", "PBE"}
};

// INTEL CPU extended features description
const char *ecxFeatures[][2] =
{
    {"Streaming SIMD Extensions 3", "SSE3"},
    {"PCLMULDQ Instruction", "PCLMULDQ"},
    {"64-bit Debug Store", "DTES64"},
    {"MONITOR/MWAIT Instructions", "MONITOR"},
    {"CPL Qualified Debug Store", "DS-CPL"},
    {"Virtual Machine Extensions", "VMX"},
    {"Safer Mode Extensions", "SMX"},
    {"Enhanced Intel SpeedStep", "EIST"},
    {"Thermal Monitor 2", "TM2"},
    {"Supplemental SSE3", "SSSE3"},
    {"L1 Context ID", "CNTX-ID"},                   // bit 10
    {"Reserved 1", "R1!!"},
    {"Fused Multiply Add", "FMA"},
    {"CMPXCHG16B Instruction", "CX16"},
    {"xTPR Update Control", "xTPR"},
    {"Perfmon/Debug Capability", "PDCM"},
    {"Reserved 2", "R2!!"},
    {"Process Context Identifiers", "PCID"},
    {"Direct Cache Access", "DCA"},
    {"Streaming SIMD Extensions 4.1", "SSE4.1"},
    {"Streaming SIMD Extensions 4.2", "SSE4.2"},    // bit 20
    {"Extended xAPIC Support", "x2APIC"},
    {"MOVBE Instruction", "MOVBE"},
    {"POPCNT Instruction", "POPCNT"},
    {"Time Stamp Counter Deadline", "TSC-DEADLINE"},
    {"AES Instruction Extensions", "AES"},
    {"XSAVE/XRSTOR States", "XSAVE"},
    {"OS-Enabled Ext State Mgmt", "OSXSAVE"},
    {"Advanced Vector Extensions", "AVX"},
    {"Reserved 3", "R3!!"},
    {"Reserved 4", "R4!!"},                         // bit 30
    {"Hypervisor Present", "HVP"}
};

// AMD CPU extended features description
const char *amdFeatures[][2] = 
{
    {"Reserved 1", "R1!!"},
    {"Reserved 2", "R2!!"},
    {"Reserved 3", "R3!!"},
    {"Reserved 4", "R4!!"},
    {"Reserved 5", "R5!!"},
    {"Reserved 6", "R6!!"},
    {"Reserved 7", "R7!!"},
    {"Reserved 8", "R8!!"},
    {"Reserved 9", "R9!!"},
    {"Reserved 10", "R10!!"},
    {"Reserved 11", "R11!!"},                       // bit 10
    {"Reserved 12", "R12!!"},
    {"Reserved 13", "R13!!"},
    {"Reserved 14", "R14!!"},
    {"Reserved 15", "R15!!"},
    {"Reserved 16", "R16!!"},
    {"Reserved 17", "R17!!"},
    {"Reserved 18", "R18!!"},
    {"Reserved 19", "R19!!"},
    {"Support SMP", "SMP"},
    {"Reserved 20", "R20!!"},                       // bit 20
    {"Reserved 21", "R21!!"},
    {"MMX extended", "MMX+"},
    {"Reserved 22", "R22!!"},
    {"Reserved 23", "R23!!"},
    {"Reserved 24", "R24!!"},
    {"Reserved 25", "R25!!"},
    {"Reserved 26", "R26!!"},
    {"Reserved 27", "R27!!"},
    {"Reserved 28", "R28!!"},
    {"Extended 3D Now!", "3DNow+!"},                // bit 30
    {"3D Now!", "3DNow!"}
};

// Pointer functions handler
uint32_t    (*fromRGB)(uint8_t, uint8_t, uint8_t) = NULL;
void        (*toRGB)(uint32_t, RGB*) = NULL;
void        (*clearScreen)(uint32_t) = NULL;
void        (*fillRect)(int32_t, int32_t, int32_t, int32_t, uint32_t) = NULL;
void        (*fillRectSub)(int32_t, int32_t, int32_t, int32_t, uint32_t) = NULL;
void        (*fillRectAdd)(int32_t, int32_t, int32_t, int32_t, uint32_t) = NULL;
void        (*fillRectPattern)(int32_t, int32_t, int32_t, int32_t, uint32_t, uint8_t*) = NULL;
void        (*fillRectPatternAdd)(int32_t, int32_t, int32_t, int32_t, uint32_t, uint8_t*) = NULL;
void        (*fillRectPatternSub)(int32_t, int32_t, int32_t, int32_t, uint32_t, uint8_t*) = NULL;
void        (*putPixel)(int32_t, int32_t, uint32_t) = NULL;
void        (*putPixelAdd)(int32_t, int32_t, uint32_t) = NULL;
void        (*putPixelSub)(int32_t, int32_t, uint32_t) = NULL;
uint32_t    (*getPixel)(int32_t, int32_t) = NULL;
void        (*horizLine)(int32_t, int32_t, int32_t, uint32_t) = NULL;
void        (*horizLineAdd)(int32_t, int32_t, int32_t, uint32_t) = NULL;
void        (*horizLineSub)(int32_t, int32_t, int32_t, uint32_t) = NULL;
void        (*vertLine)(int32_t, int32_t, int32_t, uint32_t) = NULL;
void        (*vertLineAdd)(int32_t, int32_t, int32_t, uint32_t) = NULL;
void        (*vertLineSub)(int32_t, int32_t, int32_t, uint32_t) = NULL;
void        (*getImage)(int32_t, int32_t, int32_t, int32_t, GFX_IMAGE*) = NULL;
void        (*putImage)(int32_t, int32_t, GFX_IMAGE*) = NULL;
void        (*putImageAdd)(int32_t, int32_t, GFX_IMAGE*) = NULL;
void        (*putImageSub)(int32_t, int32_t, GFX_IMAGE*) = NULL;
void        (*putSprite)(int32_t, int32_t, uint32_t, GFX_IMAGE*) = NULL;
void        (*putSpriteAdd)(int32_t, int32_t, uint32_t, GFX_IMAGE*) = NULL;
void        (*putSpriteSub)(int32_t, int32_t, uint32_t, GFX_IMAGE*) = NULL;
void        (*scaleImage)(GFX_IMAGE*, GFX_IMAGE*, int32_t) = NULL;
void        (*fadeOutImage)(GFX_IMAGE*, uint8_t) = NULL;
void        (*quitCallback)() = NULL;

// Linear frame buffer
uint8_t     *lfbPtr = NULL;                     // address of render buffer
uint32_t    lfbSize = 0;                        // size of render buffer (width * height * bytesperpixel)
int32_t     lfbWidth = 0, lfbHeight = 0;        // draw buffer width and height
uint8_t     bitsPerPixel = 0;                   // bits per pixel
uint32_t    bytesPerPixel = 0;                  // bytes per pixel
uint32_t    bytesPerScanline = 0;               // bytes per lines (width * bytesperpixel)
uint32_t    vbeSegment = 0;                     // segment of VBE block
uint32_t    crtcSegment = 0;                    // segment of CRTC block

// VESA 2.0 protect mode interfaces
VBE_PM_INFO *pmInfo = NULL;                     // protect mode interface info
void        *fnSetDisplayStart = NULL;          // setDisplayStart function address
void        *fnSetWindow = NULL;                // SetWindow function address
void        *fnSetPalette = NULL;               // setPalette function address
uint32_t    *pmBasePtr = NULL;                  // address of base code
uint16_t    pmSelector = 0;                     // protect mode selector

// Page flipping
uint32_t    activePage = 0;                     // current active page
uint32_t    pageOffset = 0;                     // page start offset (use for visual page and active page)
uint32_t    numOfPages = 0;                     // number of virtual screen page

// Palette mask
uint32_t    rpos = 0, gpos = 0, bpos = 0;
uint32_t    rmask = 0, gmask = 0, bmask = 0;
uint32_t    rshift = 0, gshift = 0, bshift = 0;

// Clip cordinate handle
int32_t     cminX = 0, cminY = 0;               // min view port
int32_t     cmaxX = 0, cmaxY = 0;               // max viewport
int32_t     centerX = 0, centerY = 0;           // center points
int32_t     currentX = 0, currentY = 0;         // current draw cursor position

// Save current buffer
uint8_t*    oldBuff = NULL;                     // saved lfb buffer
int32_t     oldWidth = 0, oldHeight = 0;        // saved buffer height

// Saved screen view-port
int32_t     oldMinX = 0, oldMinY = 0;           // saved left-top
int32_t     oldMaxX = 0, oldMaxY = 0;           // saved right-bottom

// 3D projection
enum        {PERSPECTIVE, PARALLELE};
double      de = 0.0, rho = 0.0, theta = 0.0, phi = 0.0;
double      aux1 = 0.0, aux2 = 0.0, aux3 = 0.0, aux4 = 0.0;
double      aux5 = 0.0, aux6 = 0.0, aux7 = 0.0, aux8 = 0.0;
double      xobs = 0.0, yobs = 0.0, zobs = 0.0;
double      xproj = 0.0, yproj = 0.0;
int32_t     xecran = 0, yecran = 0;
uint8_t     projection = 0;

// Timer defination
uint32_t    cpuSpeed = 0;                       // CPU clock rate in MHz
uint32_t    timeRes = 100;                      // timer resolution (default for clock time)
uint8_t     timeType = 0;                       // timer type: 0 - clock time, 1 - cpu time

// CPU features
int32_t     haveMMX = 0;                        // check for have MMX extended
int32_t     haveSSE = 0;                        // check for have SSE extended
int32_t     have3DNow = 0;                      // check for have 3DNow extended

// CPU info
char        cpuVendor[16] = {0};                // CPU vendor string
char        cpuFeatures[16] = {0};              // CPU features string

// Mouse callback data
MOUSE_CALLBACK_DATA mcd;

// Memory status info
MEM_INFO    meminfo;

// GFXLIB font data
GFX_FONT    gfxFonts[GFX_MAX_FONT] = {0};       // GFXLIB font loadable at the same time
uint8_t     *fontPalette[GFX_MAX_FONT] = {0};   // GFXLIB font palette data (BMP8 type)
uint8_t     *gfxBuff = NULL;                    // GFXLIB buffer
uint32_t    subFont = 0;                        // GFXLIB sub-fonts
uint32_t    fontType = 0;                       // current selected font (use for multiple loaded font)
uint32_t    randSeed = 0;                       // global random seed
uint32_t    factor = 0x8088405;                 // global factor
uint32_t    stackOffset = 0;                    // kernel stack offset

// Pattern filled styles
uint8_t     ptnLine[]           = {0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00};
uint8_t     ptnLiteSlash[]      = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80};
uint8_t     ptnSlash[]          = {0x07, 0x0E, 0x1C, 0x38, 0x70, 0xE0, 0xC1, 0x83};
uint8_t     ptnBackSlash[]      = {0x07, 0x83, 0xC1, 0xE0, 0x70, 0x38, 0x1C, 0x0E};
uint8_t     ptnLiteBackSlash[]  = {0x5A, 0x2D, 0x96, 0x4B, 0xA5, 0xD2, 0x69, 0xB4};
uint8_t     ptnHatch[]          = {0xFF, 0x88, 0x88, 0x88, 0xFF, 0x88, 0x88, 0x88};
uint8_t     ptnHatchX[]         = {0x18, 0x24, 0x42, 0x81, 0x81, 0x42, 0x24, 0x18};
uint8_t     ptnInterLeave[]     = {0xCC, 0x33, 0xCC, 0x33, 0xCC, 0x33, 0xCC, 0x33};
uint8_t     ptnWideDot[]        = {0x80, 0x00, 0x08, 0x00, 0x80, 0x00, 0x08, 0x00};
uint8_t     ptnCloseDot[]       = {0x88, 0x00, 0x22, 0x00, 0x88, 0x00, 0x22, 0x00};

// Round up integer
inline int32_t fround(double x)
{
    if (x > 0.0) return x + 0.5;
    return x - 0.5;
}

// Generate random value from number
inline int32_t random(int32_t a)
{
    return (a == 0) ? 0 : rand() % a;
}

// Generate random value in range
inline int32_t randomRange(int32_t a, int32_t b)
{
    return (a < b) ? (a + (rand() % (b - a + 1))) : (b + (rand() % (a - b + 1)));
}

// Wrapper memcpy to use inline asm code
// prevent old compiler not optimize this function
void copyData(void *dst, void *src, uint32_t size)
{
    // Validate size
    if (!size) return;

#ifdef _USE_MMX
    __asm {
        pusha
        mov     edi, dst
        mov     esi, src
        mov     ecx, size
        shr     ecx, 3
        jz      skip
    move:
        movq    mm0, [esi]
        movq    [edi], mm0
        add     esi, 8
        add     edi, 8
        dec     ecx
        jnz     move
        emms
    skip:
        mov     ecx, size
        and     ecx, 7
        rep     movsb
        popa
    }
#else
    __asm {
        pusha
        mov     edi, dst
        mov     esi, src
        mov     ecx, size
        mov     ax, cx
        shr     ecx, 2
        rep     movsd
        mov     ecx, size
        and     ecx, 3
        rep     movsb
        popa
    }
#endif    
}

// A CPUID instruction wrapper
void CPUID(uint32_t *cpuinfo, uint32_t type)
{
    __asm {
        mov     eax, type
        mov     edi, cpuinfo
        cpuid
        mov     [edi     ], eax
        mov     [edi + 4 ], ebx
        mov     [edi + 8 ], ecx
        mov     [edi + 12], edx
    }
}

// Get current FPC clock (64 bits)
uint64_t getRDTSC()
{
    uint64_t val = 0;

    __asm {
        xor     eax, eax
        cpuid
        rdtsc
        lea     edi, val
        mov     [edi    ], eax
        mov     [edi + 4], edx
    }
    return val;
}

// Get current CPU ticks (64 bits)
uint64_t getPIT()
{
    uint64_t val = 0;

    __asm {
        xor     eax, eax
        cli
        out     43h, al
        mov     edi, 046Ch
        mov     edx, [edi]
        in      al, 40h
        db      0xEB, 0x00, 0xEB, 0x00, 0xEB, 0x00
        mov     ah, al
        in      al, 40h
        db      0xEB, 0x00, 0xEB, 0x00, 0xEB, 0x00
        xchg    ah, al
        neg     ax
        mov     edi, eax
        sti
        mov     ebx, 10000h
        mov     eax, edx
        xor     edx, edx
        mul     ebx
        add     eax, edi
        adc     edx, 0
        lea     edi, val
        mov     [edi    ], eax
        mov     [edi + 4], edx
    }
    return val;
}

// Get current CPU clock rate in Mhz
double getCpuClockRate()
{
    uint32_t i = 0;
    uint64_t time0 = 0, time1 = 0;
    uint64_t stamp0 = 0, stamp1 = 0;
    uint64_t ticks = 0, cycles = 0;

    double total = 0;
    double freq[MIN_TRIES] = {0};

    // try to calculate CPU clock rate
    for (i = 0; i < MIN_TRIES; i++)
    {
        // start record clock cycles
        stamp0 = getRDTSC();

        // wait for ticks count
        time0 = getPIT();
        while ((time1 = getPIT()) < (time0 + WAIT_TIME));

        // stop record clock cycles
        stamp1 = getRDTSC();

        // calculate CPU frequences
        cycles = stamp1 - stamp0;
        ticks = (time1 - time0) * timeRes / PIT_RATE;
        freq[i] = 1.0 * cycles / ticks;
    }

    // calculate agverage
    for (i = 0; i < MIN_TRIES; i++) total += freq[i];
    return total / MIN_TRIES;
}

// Get current CPU speed in MHz
uint32_t getCpuSpeed()
{
    double speed = 0.0;
    uint32_t i = 0;
    for (i = 0; i < MAX_RUN; i++) speed += getCpuClockRate();
    return speed / MAX_RUN;
}

// Get CPU details feartures info
void printCpuInfo(int32_t verbose)
{
    uint32_t    i, eax, edx, ecx;
    uint8_t     cputype = 0;
    uint32_t    cpuinfo[4] = {0};
    char        cpuvendor[16] = {0};

    // Obtain the number of CPUID leaves and the vendor string.
    CPUID(cpuinfo, 0);

    // Get highest CPUID features
    eax = cpuinfo[CPUID_EAX];

    // Get short manufactures
    *(uint32_t*)(cpuvendor    ) = cpuinfo[CPUID_EBX];
    *(uint32_t*)(cpuvendor + 4) = cpuinfo[CPUID_EDX];
    *(uint32_t*)(cpuvendor + 8) = cpuinfo[CPUID_ECX];

    if (!strcmpi(cpuvendor, "AuthenticAMD")) cputype = PROCESSOR_AMD;
    else if (!strcmpi(cpuvendor, "GenuineIntel")) cputype = PROCESSOR_INTEL;
    else cputype = PROCESSOR_UNKNOWN;

    if (cputype != PROCESSOR_UNKNOWN) printf("CPU vendor: %s\n", cpuvendor);
    else printf("CPU vendor: unknown\n");
    printf("Processor features:\n");

    // have any highest CPUID features?
    if (eax >= 1)
    {
        if (cputype == PROCESSOR_INTEL)
        {
            // Obtain the model and features.
            CPUID(cpuinfo, 1);
            edx = cpuinfo[CPUID_EDX];
            ecx = cpuinfo[CPUID_ECX];

            // Interpret the feature bits.
            for (i = 0; i < 32; i++)
            {
                if (edx & (1L << i))
                {
                    if (verbose) printf("%s (%s)\n", edxFeatures[i][0], edxFeatures[i][1]);
                    else printf("%s ", edxFeatures[i][1]);
                }
            }

            // Interpret the extended feature bits.
            for (i = 0; i < 32; i++)
            {
                if (ecx & (1L << i))
                {
                    if (verbose) printf("%s (%s)\n", ecxFeatures[i][0], ecxFeatures[i][1]);
                    else printf("%s ", ecxFeatures[i][1]);
                }
            }
        }
    }

    if (cputype == PROCESSOR_AMD)
    {
        // Get highest extended feature
        CPUID(cpuinfo, CPUID_EXT_BASE);
        eax = cpuinfo[0];
    
        // check for have any extended features     
        if (eax >= CPUID_EXT_BASE1)
        {
            // Get extended features
            CPUID(cpuinfo, CPUID_EXT_BASE1);
            edx = cpuinfo[CPUID_EDX];

            for (i = 0; i < 32; i++)
            {
                if (edx & (1L << i))
                {
                    if (verbose) printf("%s (%s)\n", amdFeatures[i][0], amdFeatures[i][1]);
                    else printf("%s ", amdFeatures[i][1]);
                }
            }
        }
    }
}

// Dump short CPU features info
void getCpuInfo()
{
    uint32_t    eax, edx;
    uint8_t     cputype = 0;
    uint32_t    cpuinfo[4] = {0};
    char        vendor[16] = {0};

    // Reset buffer
    haveMMX = haveSSE = have3DNow = 0;
    memset(cpuVendor, 0, sizeof(cpuVendor));
    memset(cpuFeatures, 0, sizeof(cpuFeatures));
    
    // Obtain the number of CPUID leaves and the vendor string.
    CPUID(cpuinfo, 0);

    // Get highest CPUID features
    eax = cpuinfo[CPUID_EAX];

    // Get short manufactures
    *(uint32_t*)(vendor    ) = cpuinfo[CPUID_EBX];
    *(uint32_t*)(vendor + 4) = cpuinfo[CPUID_EDX];
    *(uint32_t*)(vendor + 8) = cpuinfo[CPUID_ECX];

    if (!strcmp(vendor, "AuthenticAMD")) cputype = PROCESSOR_AMD;
    else if (!strcmp(vendor, "GenuineIntel")) cputype = PROCESSOR_INTEL;
    else cputype = PROCESSOR_UNKNOWN;

    if (cputype != PROCESSOR_UNKNOWN) strcpy(cpuVendor, vendor);
    else strcpy(cpuVendor, "unknown");

    if (eax >= 1)
    {
        if (cputype == PROCESSOR_INTEL)
        {
            // Obtain the model and features.
            CPUID(cpuinfo, 1);
            edx = cpuinfo[CPUID_EDX];

            // MMX feature bits
            if (edx & (1L << 23))
            {
                strcat(cpuFeatures, edxFeatures[23][1]);
                strcat(cpuFeatures, " ");
                haveMMX = 1;
            }

            // SSE feature bits
            if (edx & (1L << 25))
            {
                strcat(cpuFeatures, edxFeatures[25][1]);
                strcat(cpuFeatures, " ");
                haveSSE = 1;
            }

            // SSE2 feature bits
            if (edx & (1L << 26))
            {
                strcat(cpuFeatures, edxFeatures[26][1]);
                strcat(cpuFeatures, " ");
                haveSSE = 1;
            }
        }
    }

    if (cputype == PROCESSOR_AMD)
    {
        // Get highest extended feature
        CPUID(cpuinfo, CPUID_EXT_BASE);
        eax = cpuinfo[0];
    
        // check for have any extended features     
        if (eax >= CPUID_EXT_BASE1)
        {       
            // Get extended features
            CPUID(cpuinfo, CPUID_EXT_BASE1);
            edx = cpuinfo[CPUID_EDX];

            // 3DNow!
            if (edx & (1L << 31))
            {
                strcat(cpuFeatures, amdFeatures[31][1]);
                strcat(cpuFeatures, " ");
                have3DNow = 1;
            }

            // 3DNow+!
            if (edx & (1L << 30))
            {
                strcat(cpuFeatures, amdFeatures[30][1]);
                strcat(cpuFeatures, " ");
                have3DNow = 1;
            }

            // MMX+!
            if (edx & (1L << 30))
            {
                strcat(cpuFeatures, amdFeatures[22][1]);
                strcat(cpuFeatures, " ");
                haveMMX = 1;
            }
        }
    }

    // remove steal character
    edx = strlen(cpuFeatures);
    if (edx > 1) cpuFeatures[edx - 1] = '\0';
    else strcpy(cpuFeatures, "none");
}

// Get system memory status info
void getMemoryInfo()
{
    // Reset buffer
    memset(&meminfo, 0, sizeof(MEM_INFO));

    // call DMPI function to get system memory status info
    __asm {
        mov     eax, 0500h
        lea     edi, meminfo
        int     31h
    }
}

// Initialize the timer to use system time or cpu clock time
void setTimerType(uint8_t type)
{
    _clearscreen(0);
    printf("GFXLIB initializing....\n");

    timeType = type;

    if (timeType)
    {
        // Timing resolution of 1 microsecond
        timeRes = 1000000;
        cpuSpeed = getCpuSpeed();
    }
    else
    {
        // Timing resolution of 100 milisecond
        timeRes = 100;
        cpuSpeed = 0;
    }
}

// Convert CPU ticks to microsecond
uint64_t ticksToMicroSec(uint64_t ticks)
{
    if (cpuSpeed > 0) return ticks / cpuSpeed;
    return ticks;
}

// Get current system time (in 100ms or 1 microsecond)
uint64_t getCurrentTime()
{
    struct dostime_t dt;
    
    // timer is use RDTSC (timing resolution in 1 microsecond)
    if (timeType) return getRDTSC();

    // timer is use system time (timing resolution in 100 milisecond)
    _dos_gettime(&dt);
    return (dt.hsecond + (dt.second + (dt.minute + dt.hour * 60) * 60) * 100);
}

// Get elapsed time from the begining time
uint64_t getElapsedTime(uint64_t tmstart)
{
    if (timeType) return ticksToMicroSec(getCurrentTime() - tmstart);
    return (getCurrentTime() - tmstart);
}

// Wait for time out
void waitFor(uint64_t tmstart, uint64_t ms)
{
    uint64_t tmwait = (ms * timeRes) / 1000;
    while (getElapsedTime(tmstart) < tmwait);
}

// delay CPU execution
void delay(uint32_t ms)
{
    uint64_t tmwait = (ms * timeRes) / 1000;
    uint64_t tmstart = getCurrentTime();
    while (getElapsedTime(tmstart) < tmwait);
}

// DPMI alloc DOS memory block
uint32_t allocDosSegment(uint32_t pageSize)
{
    __asm {
        mov     ebx, pageSize
        cmp     ebx, 65535
        ja      error
        add     ebx, 15
        shr     ebx, 4
        mov     eax, 0100h
        int     31h
        jc      error
        and     eax, 0000FFFFh
        shl     edx, 16
        add     eax, edx
        jmp     quit
    error:
        xor     eax, eax
    quit:
    }
}

// DPMI free DOS memory block
void freeDosSegment(uint32_t *selSegment)
{
    __asm {
        mov     edi, selSegment
        mov     edx, [edi]
        push    edi
        shr     edx, 16
        mov     eax, 0101h
        int     31h
        pop     edi
        mov     dword ptr [edi], 0
    }
}

// Simulation real mode interrupt
int32_t simRealModeInt(uint8_t num, RM_REGS *rmRegs)
{
    __asm {
        xor     ebx, ebx
        mov     bl, num
        mov     edi, rmRegs
        xor     ecx, ecx
        mov     eax, 0300h
        int     31h
        jc      error
        mov     eax, 1
        jmp     quit
    error:
        xor     eax, eax
    quit:
    }
}

// DPMI alloc selector
uint16_t allocSelector()
{
    __asm {
        xor     eax, eax
        mov     ecx, 1
        int     31h
        jnc     quit
        xor     ax, ax
    quit:
    }
}

// DPMI free selector
void freeSelector(uint16_t *sel)
{
    __asm {
        mov     edi, sel
        mov     bx, [edi]
        mov     dword ptr [edi], 0
        mov     eax, 0001h
        int     31h
    }
}

// DPMI set selector access rights
int32_t setSelectorRights(uint16_t sel, uint16_t accRights)
{
    __asm {
        mov     bx, sel
        mov     cx, accRights
        mov     eax, 0009h
        int     31h
        jc      error
        mov     eax, 1
        jmp     quit
    error:
        xor     eax, eax
    quit:
    }
}

// DPMI set selector base
int32_t setSelectorBase(uint16_t sel, uint32_t linearAddr)
{
    __asm {
        mov     bx, sel
        mov     ecx, linearAddr
        mov     edx, ecx
        shr     ecx, 16
        and     edx, 0FFFFh
        mov     eax, 0007h
        int     31h
        jc      error
        mov     eax, 1
        jmp     quit
    error:
        xor     eax, eax
    quit:
    }
}

// DPMI set selector limit
int32_t setSelectorLimit(uint16_t sel, uint32_t selLimit)
{
    __asm {
        mov     bx, sel
        mov     ecx, selLimit
        mov     edx, ecx
        shr     ecx, 16
        and     edx, 0FFFFh
        mov     eax, 0008h
        int     31h
        jc      error
        mov     eax, 1
        jmp     quit
    error:
        xor     eax, eax
    quit:
    }
}

// DPMI map real address to linear address
uint32_t mapPhysicalAddress(uint32_t physAddr, uint32_t len)
{
    __asm {
        mov     ebx, physAddr
        mov     esi, len
        mov     ecx, ebx
        mov     edi, esi
        shr     ebx, 16
        and     ecx, 0FFFFh
        shr     esi, 16
        and     edi, 0FFFFh
        mov     eax, 0800h
        int     31h
        jc      error
        shl     ebx, 16
        and     ecx, 0FFFFh
        mov     eax, ebx
        or      eax, ecx
        jmp     quit
    error:
        xor     eax, eax
    quit:
    }
}

// DPMI free linear address
void freePhysicalAddress(uint32_t* linearAddr)
{
    __asm {
        mov     edi, linearAddr
        mov     bx, [edi + 2]
        mov     cx, [edi]
        mov     dword ptr [edi], 0
        mov     eax, 0801h
        int     31h
    }
}

// Convert real pointer to linear pointer
uint32_t mapRealPointer(uint32_t rmSegOfs)
{
    __asm {
        mov     eax, rmSegOfs
        mov     edx, eax
        and     eax, 0FFFF0000h
        and     edx, 0000FFFFh
        shr     eax, 12
        add     eax, edx
    }
}

// Get VESA driver info
int32_t getVesaDriverInfo(VBE_DRIVER_INFO *info)
{
    RM_REGS         regs;
    VBE_DRIVER_INFO *drvInfo;

    // Alloc 1K memory to store VESA driver and mode info
    if (vbeSegment == 0) vbeSegment = allocDosSegment(1024);
    if (vbeSegment == 0 || vbeSegment == 0xFFFF) return 0;

    // Setup pointer memory
    drvInfo = (VBE_DRIVER_INFO*)((vbeSegment & 0x0000FFFF) << 4);
    memset(drvInfo, 0, sizeof(VBE_DRIVER_INFO));
    memcpy(drvInfo->VBESignature, "VBE2", 4); // Request for VESA 2.0+

    memset(&regs, 0, sizeof(regs));
    regs.eax = 0x4F00;
    regs.es  = vbeSegment;
    regs.edi = 0;
    simRealModeInt(0x10, &regs);

    // Check VESA 2.0+ to support linear frame buffer
    if (regs.eax == 0x004F && !memcmp(drvInfo->VBESignature, "VESA", 4) && drvInfo->VBEVersion >= 0x0200)
    {
        // Convert real string pointer to linear address
        drvInfo->OEMStringPtr       = mapRealPointer(drvInfo->OEMStringPtr);
        drvInfo->VideoModePtr       = mapRealPointer(drvInfo->VideoModePtr);
        drvInfo->OemVendorNamePtr   = mapRealPointer(drvInfo->OemVendorNamePtr);
        drvInfo->OemProductNamePtr  = mapRealPointer(drvInfo->OemProductNamePtr);
        drvInfo->OemProductRevPtr   = mapRealPointer(drvInfo->OemProductRevPtr);
        memcpy(info, drvInfo, sizeof(VBE_DRIVER_INFO));
        return 1;
    }

    return 0;
}

// Get VBE mode info
int32_t getVesaModeInfo(uint16_t mode, VBE_MODE_INFO *info)
{
    RM_REGS         regs;
    VBE_MODE_INFO   *modeInfo;

    if (!(mode && vbeSegment)) return 0;
    if (mode == 0xFFFF) return 0;

    // Setup memory pointer
    modeInfo = (VBE_MODE_INFO*)(((vbeSegment & 0x0000FFFF) << 4) + 512);
    memset(modeInfo, 0, sizeof(VBE_MODE_INFO));

    memset(&regs, 0, sizeof(regs));
    regs.eax = 0x4F01;
    regs.ecx = mode;
    regs.es  = vbeSegment;
    regs.edi = 512;
    simRealModeInt(0x10, &regs);

    // Check for error call
    if (regs.eax != 0x004F) return 0;
    if (!(modeInfo->ModeAttributes & VBE_MASK_MODEHW)) return 0; // Mode must supported by hardware
    if (!(modeInfo->ModeAttributes & VBE_MASK_HASLFB)) return 0; // Mode must supported linear frame buffer
    if (!modeInfo->PhysBasePtr) return 0; // Physical address must be not null
    memcpy(info, modeInfo, sizeof(VBE_MODE_INFO));
    return 1;
}

// Close current VBE mode
void closeVesaMode()
{
    RM_REGS regs;

    // Free memory
    memset(&regs, 0, sizeof(regs));
    regs.eax = 0x03;
    simRealModeInt(0x10, &regs);
    freePhysicalAddress((uint32_t*)&lfbPtr);
    lfbPtr = NULL;

    // Free selector
    if (pmSelector)
    {
        freePhysicalAddress((uint32_t*)&pmBasePtr);
        freeSelector(&pmSelector);
        pmBasePtr = NULL;
        pmSelector = 0;
    }

    // Free real mode VBE memory
    if (vbeSegment)
    {
        freeDosSegment(&vbeSegment);
        vbeSegment = 0;
    }

    // Free real mode CRTC memory
    if (crtcSegment)
    {
        freeDosSegment(&crtcSegment);
        crtcSegment = 0;
    }

    // Free protect demo info
    if (pmInfo)
    {
        free(pmInfo);
        pmInfo = NULL;
        fnSetDisplayStart = NULL;
        fnSetWindow = NULL;
        fnSetPalette = NULL;
    }

    // Free gfx font buffer
    if (gfxBuff)
    {
        free(gfxBuff);
        gfxBuff = NULL;
    }
}

// Raise error message and exit program
void fatalError(const char *fmt, ...)
{
    va_list args;
    closeVesaMode();
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
    exit(1);
}

// Check for break program execution
int32_t keyPressed(int32_t keyQuit)
{
    int32_t key = 0;

    // Check for pressed key
    while (kbhit())
    {
        // Read pressed key input
        key = getch();
        if (key == keyQuit)
        {
            // Call callback function before exit program
            if (quitCallback) quitCallback();
            exit(1);
        }
    }

    return key;
}

// Set handler when exit program
void setQuitCallback(void (*fnQuit)())
{
    quitCallback = fnQuit;
}

// VESA 3.0, calculate CRTC timing using GTF formular
void calcCrtcTimingGTF(VBE_CRTC_INFO_BLOCK *crtc, int32_t hpixels, int32_t vlines, int32_t freq, int32_t interlaced, int32_t margins)
{
    uint8_t     doubleScan = 0;
    uint32_t    marginLeftRight = 0;
    uint32_t    marginTopBottom = 0;
    uint32_t    horizPeriodEst;
    uint32_t    vertSyncPlusBP;
    uint32_t    vertLinesTotal;
    uint32_t    horizPixelsTotal;
    uint32_t    idealDutyCycle;
    uint32_t    horizBlank;
    uint32_t    pixelClock;

    // Re-calculate horizontial pixels
    hpixels = hpixels / CELL_GRAN * CELL_GRAN;

    // Check for double scanline
    if (vlines < 400)
    {
        doubleScan = 1;
        vlines <<= 1;
    }

    // Calculate margins
    if (margins)
    {
        marginLeftRight = (hpixels * FIXED(MARGIN_PERCENT) / (FIXED(100) * CELL_GRAN)) * CELL_GRAN;
        marginTopBottom = vlines * FIXED(MARGIN_PERCENT) / FIXED(100);
    }

    // Estimate the horizontal period
    horizPeriodEst = (2 * FIXED(FIXED(1000000) / freq) - 2 * FIXED(MIN_VSYNC_BP)) / (2 * (vlines + 2 * marginTopBottom + MIN_V_PORCH) + interlaced);

    // Find the number of lines in vSync + back porch
    vertSyncPlusBP = FIXED(MIN_VSYNC_BP) / horizPeriodEst + 1;

    // Correct range
    if (vertSyncPlusBP < V_SYNC + MIN_V_BP) vertSyncPlusBP = V_SYNC + MIN_V_BP;

    // Find the total number of lines in the vertical period
    vertLinesTotal = ((vlines + 2 * marginTopBottom + vertSyncPlusBP + MIN_V_PORCH) << interlaced) + interlaced;

    // Find the total number of pixels
    horizPixelsTotal = hpixels + 2 * marginLeftRight;

    // Find the ideal blanking duty cycle
    idealDutyCycle = C_PRIME - (M_PRIME * horizPeriodEst / FIXED(1000));

    // Correct range
    if (idealDutyCycle < 20) idealDutyCycle = 20;

    // Find the number of pixels in blanking time
    horizBlank = horizPixelsTotal * idealDutyCycle / ((100 - idealDutyCycle) * (2 * CELL_GRAN)) * (2 * CELL_GRAN);

    // Final total number of pixels
    horizPixelsTotal += horizBlank;

    // Find the pixel clock frequency
    pixelClock = FIXED(horizPixelsTotal) * (1000 / CLOCK_STEP) / horizPeriodEst * CLOCK_STEP * 1000; // in Hz

    // Store CRTC data
    crtc->HorizontalTotal           = horizPixelsTotal;
    crtc->HorizontalSyncEnd         = horizPixelsTotal - horizBlank / 2;
    crtc->HorizontalSyncStart       = crtc->HorizontalSyncEnd - horizPixelsTotal * FIXED(H_SYNC_PERCENT) / (FIXED(100) * CELL_GRAN) * CELL_GRAN;
    crtc->VerticalTotal             = vertLinesTotal;
    crtc->VerticalSyncStart         = vlines + MIN_V_PORCH;
    crtc->VerticalSyncEnd           = vlines + MIN_V_PORCH + V_SYNC;
    crtc->PixelClock                = pixelClock;
    crtc->RefreshRate               = 100 * (pixelClock / (horizPixelsTotal * vertLinesTotal));  // in 0.01 Hz
    crtc->Flags                     = CRTC_HSYNC_NEGATIVE | CRTC_VSYNC_NEGATIVE;
    if (interlaced) crtc->Flags     |= CRTC_INTERLACED;
    if (doubleScan) crtc->Flags     |= CRTC_DOUBLE_SCANLINE;
}

// VESA 3.0, Get VESA closest clock, pixel clock in Hz
uint32_t getClosestPixelClock(uint16_t mode, uint32_t pixelClock)
{
    RM_REGS regs;
    memset(&regs, 0, sizeof(RM_REGS));
    regs.eax = 0x4F0B;
    regs.ebx = 0;
    regs.ecx = pixelClock;
    regs.edx = mode;
    simRealModeInt(0x10, &regs);
    return (regs.eax != 0x004F) ? 0 : regs.ecx;
}

// VESA 2.0+, get protected mode info (use in protect mode)
int32_t getProtectModeFunctions()
{
    RM_REGS     regs;
    uint16_t    *ptrIO;
    uint32_t    pmCodeSize;
    uint32_t    pmPhysAddr;

    memset(&regs, 0, sizeof(RM_REGS));
    regs.eax = 0x4F0A;
    regs.ebx = 0;
    simRealModeInt(0x10, &regs);
    if (regs.eax != 0x004F) return 0;

    // Have protect mode interface defined?
    if (!pmInfo && regs.ecx > 0)
    {
        pmInfo = (VBE_PM_INFO*)calloc(regs.ecx & 0x0000FFFF, 1);
        if (!pmInfo) return 0;

        // Copy protect mode info data
        memcpy(pmInfo, (char*)(regs.es << 4 | regs.edi), regs.ecx & 0x0000FFFF);

        // Need memory mapped IO?
        if (pmInfo->IOPrivInfo)
        {
            // Get IO memory info
            ptrIO = (uint16_t*)((char*)pmInfo + pmInfo->IOPrivInfo);

            // Skip port tables
            while (*ptrIO != 0xFFFF) ptrIO++;

            // Goto selector base
            ptrIO++;

            // Have correct selector base?
            if (*ptrIO != 0xFFFF)
            {
                // Alloc new selector
                if (pmSelector == 0) pmSelector = allocSelector();
                if (pmSelector == 0 || pmSelector == 0xFFFF) return 0;
                if (!setSelectorRights(pmSelector, 0x8092)) return 0;

                // Get physical address and size of selector
                pmPhysAddr = *(uint32_t*)ptrIO;
                pmCodeSize = *(ptrIO + 2);

                // Map to linear address
                if (!(pmBasePtr = (uint32_t*)mapPhysicalAddress(pmPhysAddr, pmCodeSize))) return 0;
                if (!setSelectorBase(pmSelector, (uint32_t)pmBasePtr)) return 0;
                if (!setSelectorLimit(pmSelector, max(pmCodeSize - 1, 0xFFFF))) return 0;
            }
        }

        // Lookup functions
        fnSetWindow         = (char*)pmInfo + pmInfo->SetWindow;
        fnSetDisplayStart   = (char*)pmInfo + pmInfo->setDisplayStart;
        fnSetPalette        = (char*)pmInfo + pmInfo->setPalette;
    }

    return 1;
}

// VESA 2.0+ set palette range (use in protect mode)
void setPaletteRange(PAL *pal, int32_t from, int32_t to)
{
    int32_t i;
    PAL tmp[256] = {0};
    PAL *palData = tmp;

    if (!fnSetPalette) return;
    if (from < 0) from = 0;
    if (from > 255) from = 255;
    if (to < 0) to = 0;
    if (to > 255) to = 255;
    if (from > to) Swap(from, to);

    // swap the palette into the funny order VESA uses
    for (i = from; i <= to; i++)
    {
        tmp[i].r = pal[i].b;
        tmp[i].g = pal[i].g;
        tmp[i].b = pal[i].r;
    }

    __asm {
        mov     ax, pmSelector
        mov     ds, ax
        mov     ebx, 80h
        mov     ecx, to - from + 1
        mov     edx, from
        mov     edi, palData
        call    fnSetPalette
    }
}

// VESA 3.0 hardware tripple buffering
int32_t scheduleDisplayStart(uint32_t xpos, uint32_t ypos)
{
    RM_REGS regs;
    uint32_t offset = xpos * bytesPerPixel + ypos * bytesPerScanline;

    memset(&regs, 0, sizeof(regs));
    regs.eax = 0x4F07;
    regs.ebx = 0x02;
    regs.ecx = offset;
    simRealModeInt(0x10, &regs);
    return (regs.eax == 0x004F);
}

// VESA 3.0 hardware tripple buffering
int32_t getScheduleDisplayStartStatus()
{
    RM_REGS regs;
    memset(&regs, 0, sizeof(regs));
    regs.eax = 0x4F07;
    regs.ebx = 0x04;
    simRealModeInt(0x10, &regs);
    return (regs.eax == 0x004F) && (regs.ecx > 0);
}

// VESA 2.0+ set display start address (use in protect mode)
// Hardware scrolling is best for page flipping
int32_t setDisplayStart(uint32_t xpos, uint32_t ypos)
{
    RM_REGS regs;
    uint32_t val = 0, offset = 0;

    // VESA 2.0, call direct from protect mode interface
    if (fnSetDisplayStart)
    {
        __asm {
            mov     eax, xpos
            mul     bytesPerPixel
            mov     edx, eax
            mov     eax, ypos
            mul     bytesPerScanline
            add     edx, eax
            shr     edx, 2
            mov     ax, pmSelector
            mov     es, ax
            mov     ebx, 80h
            mov     ecx, edx
            and     ecx, 0FFFFh
            shr     edx, 16
            call    fnSetDisplayStart
            mov     val, eax
        }
        return (val == 0x004F);
    }

    // VESA 3.0, call from BIOS
    offset = xpos * bytesPerPixel + ypos * bytesPerScanline;
    memset(&regs, 0, sizeof(regs));
    regs.eax = 0x4F07;
    regs.ebx = 0x80;
    regs.ecx = offset & 0xffff;
    regs.edx = offset >> 16;
    regs.es  = pmSelector;
    simRealModeInt(0x10, &regs);
    return (regs.eax == 0x004F);    
}

// Set drawable page
void setActivePage(uint32_t page)
{
    if (page > numOfPages - 1) fatalError("setActivePage: out of visual screen page: %u\n", numOfPages);
    activePage = page;
    pageOffset = page * lfbHeight;
}

// Set visible page
void setVisualPage(uint32_t page)
{
    if (page > numOfPages - 1) fatalError("setVisualPage: out of visual screen page: %u\n", numOfPages);
    setDisplayStart(0, page * lfbHeight);
}

// Display all VBE info and mode info
void displayVesaInfo()
{
    uint16_t        *modePtr;
    VBE_DRIVER_INFO drvInfo;
    VBE_MODE_INFO   modeInfo;

    memset(&drvInfo, 0, sizeof(VBE_DRIVER_INFO));
    getVesaDriverInfo(&drvInfo);

    printf("VESA DRIVER INFO\n");
    printf("----------------\n");

    printf("     Signature: '%c%c%c%c'\n", drvInfo.VBESignature[0], drvInfo.VBESignature[1], drvInfo.VBESignature[2], drvInfo.VBESignature[3]);
    printf("       Version: %x.%x\n", (drvInfo.VBEVersion >> 8), (drvInfo.VBEVersion & 0xFF));
    printf("    OEM String: '%s'\n", (char*)drvInfo.OEMStringPtr);
    printf("  Capabilities: %0.8X\n", drvInfo.Capabilities);
    printf(" DAC Registers: %s\n", (drvInfo.Capabilities & VBE_CAPS_DAC8) ? "Switchable to 8-bits" : "Fixed at 6-bits");
    printf(" Use Blank 09h: %s\n", (drvInfo.Capabilities & VBE_CAPS_BLANKFN9) ? "Yes" : "No");
    printf("VGA Compatible: %s\n", (drvInfo.Capabilities & VBE_CAPS_NOTVGA) ? "No" : "Yes"); 
    printf("  Video Memory: %uMB\n", (drvInfo.TotalMemory << 6) >> 10);

    // VESA 2.0+, just print OEM info
    if (drvInfo.VBEVersion >= 0x0200)
    {
        printf("  OEM Soft Rev: %0.4X\n", drvInfo.OemSoftwareRev);
        printf("    OEM Vendor: '%s'\n", (char*)drvInfo.OemVendorNamePtr);
        printf("   OEM Product: '%s'\n", (char*)drvInfo.OemProductNamePtr);
        printf("  OEM Revision: '%s'\n", (char*)drvInfo.OemProductRevPtr);
    }

    getch();
    printf("\nVESA MODE INFO\n");

    // Print all VBE modes support
    modePtr = (uint16_t*)drvInfo.VideoModePtr;
    while (modePtr != NULL && *modePtr != 0xFFFF)
    {
        memset(&modeInfo, 0, sizeof(VBE_MODE_INFO));
        if (getVesaModeInfo(*modePtr, &modeInfo) != 0)
        {
            printf("---------------------------------------------------------\n");
            printf("              Mode Number: %0.4X\n", *modePtr);
            printf("               Attributes: %.4X\n", modeInfo.ModeAttributes);
            printf("       Hardware Supported: %s\n", (modeInfo.ModeAttributes & VBE_MASK_MODEHW) ? "Yes" : "No");
            printf("      Linear Frame Buffer: %s\n", (modeInfo.ModeAttributes & VBE_MASK_HASLFB) ? "Yes" : "No");
            printf("Hardware Triple Buffering: %s\n", (modeInfo.ModeAttributes & VBE_MASK_HW3BUF) ? "Yes" : "No");
            printf("              XResolution: %u\n", modeInfo.XResolution);
            printf("              YResolution: %u\n", modeInfo.YResolution);
            printf("           Bits per Pixel: %u\n", modeInfo.BitsPerPixel);
            printf("    Physical Frame Buffer: %.8X\n", modeInfo.PhysBasePtr);
            printf("             Memory Model: ");

            switch (modeInfo.MemoryModel)
            {
                case VBE_MM_PACKED: printf("Packed Pixel\n"); break;
                case VBE_MM_DCOLOR: printf("Direct Color\n"); break;
                case VBE_MM_YUV: printf("YUV\n"); break;
                default: printf("%.2X\n", modeInfo.MemoryModel);
            }
            
            if ((modeInfo.MemoryModel == VBE_MM_DCOLOR) || (modeInfo.MemoryModel == VBE_MM_YUV))
            {
                if (drvInfo.VBEVersion >= 0x0300)
                {
                    printf("    Number of Image Pages: %u\n", modeInfo.LinNumberOfImagePages);
                    printf("       Bytes per Scanline: %u\n", modeInfo.LinBytesPerScanline);
                    printf("            Red Mask Size: %u\n", modeInfo.LinRedMaskSize);
                    printf("            Red Field pos: %u\n", modeInfo.LinRedFieldPosition);
                    printf("          Green Mask Size: %u\n", modeInfo.LinGreenMaskSize);
                    printf("          Green Field pos: %u\n", modeInfo.LinGreenFieldPosition);
                    printf("           Blue Mask Size: %u\n", modeInfo.LinBlueMaskSize);
                    printf("           Blue Field pos: %u\n", modeInfo.LinBlueFieldPosition);
                    printf("          Max Pixel Clock: %u\n", modeInfo.MaxPixelClock);
                }
                else
                {
                    printf("    Number of Image Pages: %u\n", modeInfo.NumberOfImagePages);
                    printf("       Bytes per Scanline: %u\n", modeInfo.BytesPerScanline);
                    printf("            Red Mask Size: %u\n", modeInfo.RedMaskSize);
                    printf("            Red Field pos: %u\n", modeInfo.RedFieldPosition);
                    printf("          Green Mask Size: %u\n", modeInfo.GreenMaskSize);
                    printf("          Green Field pos: %u\n", modeInfo.GreenFieldPosition);
                    printf("           Blue Mask Size: %u\n", modeInfo.BlueMaskSize);
                    printf("           Blue Field pos: %u\n", modeInfo.BlueFieldPosition);
                }
            }

            getch();
        }

        modePtr++;
    }
}

// Extract rgb value to r,g,b values
inline void toRGB8(uint32_t col, RGB *rgb)
{
    __asm {
        mov     edi, rgb
        mov     al, byte ptr col
        mov     ah, al
        mov     [edi], ax
        mov     [edi + 2], al
    }
}

inline void toRGB15(uint32_t col, RGB *rgb)
{
    __asm {
        mov     ax, word ptr col
        mov     bx, ax
        shl     al, 3
        mov     edi, rgb
        mov     [edi], al
        mov     ax, bx
        and     bh, 1111100b
        and     ax, 1111100000b
        shr     ax, 2
        add     bh, bh
        mov     [edi + 1], al
        mov     [edi + 2], bh
    }
}

inline void toRGB16(uint32_t col, RGB *rgb)
{
    __asm {
        mov     ax, word ptr col
        mov     bx, ax
        and     ax, 1111100000011111b
        shl     al, 3
        mov     edi, rgb
        and     bx, 11111100000b
        shl     bx, 5
        mov     bl, al
        mov     [edi + 2], ah
        mov     [edi], bx
    }
}

inline void toRGB2432(uint32_t col, RGB *rgb)
{
    __asm {
        mov     edi, rgb
        mov     al, byte ptr col
        mov     [edi], al
        mov     ax, word ptr col[1]
        mov     [edi + 1], ax
    }
}

// Merge r,g,b values to rgb value
inline uint32_t fromRGB8(uint8_t r, uint8_t g, uint8_t b)
{
    __asm {
        xor     eax, eax
        xor     bx, bx
        mov     al, b
        mov     bl, g
        add     ax, bx
        add     ax, bx
        mov     bl, r
        add     ax, bx
        shr     ax, 2
    }
}

inline uint32_t fromRGB15(uint8_t r, uint8_t g, uint8_t b)
{
    __asm {
        xor     eax, eax
        mov     al, b
        mov     ah, g
        mov     bl, r
        shr     ah, 3
        and     bl, 11111000b
        shr     ax, 3
        shr     bl, 1
        or      ah, bl
    }
}

inline uint32_t fromRGB16(uint8_t r, uint8_t g, uint8_t b)
{
    __asm {
        xor     eax, eax
        mov     al, b
        mov     ah, g
        shr     ah, 2
        mov     bl, r
        shr     ax, 3
        and     bl, 11111000b
        or      ah, bl
    }
}

inline uint32_t fromRGB2432(uint8_t r, uint8_t g, uint8_t b)
{
    __asm {
        xor     eax, eax
        xor     ebx, ebx
        mov     al, b
        mov     ah, g
        mov     bl, r
        shl     ebx, 16
        or      eax, ebx
    }
}

inline uint32_t fromRGBA(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
    __asm {
        xor     eax, eax
        xor     ebx, ebx
        mov     al, b
        mov     ah, g
        mov     bl, r
        mov     bh, a
        shl     ebx, 16
        or      eax, ebx
    }
}

// Vertical and Horizon retrace
void waitRetrace()
{
    __asm {
        mov     dx, 03DAh
    waitH:
        in      al, dx
        test    al, 08h
        jz      waitH
    waitV:
        in      al, dx
        test    al, 08h
        jnz     waitV
    }
}

// Set current screen view port for clipping
// !!!changeViewPort and restoreViewPort must be a pair functions!!!
void changeViewPort(int32_t x1, int32_t y1, int32_t x2, int32_t y2)
{
    //save current view port
    oldMinX = cminX;
    oldMinY = cminY;
    oldMaxX = cmaxX;
    oldMaxY = cmaxY;
    oldWidth = lfbWidth;
    oldHeight = lfbHeight;

    //update new clip view port
    cminX = x1;
    cminY = y1;
    cmaxX = x2;
    cmaxY = y2;

    //update buffer width and height
    lfbWidth = cmaxX - cminX + 1;   
    lfbHeight = cmaxY - cminY + 1;

    //update center x,y
    centerX = (lfbWidth >> 1) - 1;
    centerY = (lfbHeight >> 1) - 1;

    //update row bytes
    bytesPerScanline = lfbWidth * bytesPerPixel;
}

// must call after changeViewPort call
// !!!changeViewPort and restoreViewPort must be a pair functions!!!
void restoreViewPort()
{
    cminX = oldMinX;
    cminY = oldMinY;
    cmaxX = oldMaxX;
    cmaxY = oldMaxY;
    lfbWidth = oldWidth;
    lfbHeight = oldHeight;
    centerX = (lfbWidth >> 1) - 1;
    centerY = (lfbHeight >> 1) - 1;
    bytesPerScanline = lfbWidth * bytesPerPixel;
}

// Set the new draw buffer
// !!!changeDrawBuffer and restoreDrawBuffer must be a pair functions!!!
void changeDrawBuffer(uint8_t* newBuff, int32_t newWidth, int32_t newHeight)
{
    oldBuff = lfbPtr;
    lfbPtr = newBuff;
    changeViewPort(0, 0, newWidth - 1, newHeight - 1);
}

// must call after setDrawBuffer call
// !!!changeDrawBuffer and restoreDrawBuffer must be a pair functions!!!
void restoreDrawBuffer()
{
    lfbPtr = oldBuff;
    restoreViewPort();
}

// Pixels functions
void putPixel8(int32_t x, int32_t y, uint32_t col)
{
    __asm {
        mov     ebx, x
        cmp     ebx, cminX
        jl      quit
        cmp     ebx, cmaxX
        jg      quit
        mov     eax, y
        cmp     eax, cminY
        jl      quit
        cmp     eax, cmaxY
        jg      quit
        add     eax, pageOffset
        mul     lfbWidth
        add     eax, ebx
        mov     edi, lfbPtr
        add     edi, eax
        mov     eax, col
        stosb
    quit:
    }
}

void putPixel1516(int32_t x, int32_t y, uint32_t col)
{
    __asm {
        mov     ebx, x
        cmp     ebx, cminX
        jl      quit
        cmp     ebx, cmaxX
        jg      quit
        mov     eax, y
        cmp     eax, cminY
        jl      quit
        cmp     eax, cmaxY
        jg      quit
        add     eax, pageOffset
        mul     lfbWidth
        add     eax, ebx
        shl     eax, 1
        mov     edi, lfbPtr
        add     edi, eax
        mov     eax, col
        stosw
    quit:
    }
}

void putPixel24(int32_t x, int32_t y, uint32_t col)
{
    __asm {
        mov     ebx, x
        cmp     ebx, cminX
        jl      quit
        cmp     ebx, cmaxX
        jg      quit
        mov     eax, y
        cmp     eax, cminY
        jl      quit
        cmp     eax, cmaxY
        jg      quit
        add     eax, pageOffset
        mul     lfbWidth
        add     eax, ebx
        mov     ebx, eax
        shl     eax, 1
        add     eax, ebx
        mov     edi, lfbPtr
        add     edi, eax
        mov     eax, col
        stosw
        shr     eax, 16
        stosb
    quit:
    } 
}

void putPixel32(int32_t x, int32_t y, uint32_t col)
{
    __asm {
        mov     ebx, x
        cmp     ebx, cminX
        jl      quit
        cmp     ebx, cmaxX
        jg      quit
        mov     eax, y
        cmp     eax, cminY
        jl      quit
        cmp     eax, cmaxY
        jg      quit
        add     eax, pageOffset
        mul     lfbWidth
        add     eax, ebx
        shl     eax, 2
        mov     edi, lfbPtr
        add     edi, eax
        mov     eax, col
        stosd
    quit:
    }
}

void putPixelBob(int32_t x, int32_t y)
{
    __asm {
        mov     ebx, x
        cmp     ebx, cminX
        jl      quit
        cmp     ebx, cmaxX
        jg      quit
        mov     eax, y
        cmp     eax, cminY
        jl      quit
        cmp     eax, cmaxY
        jg      quit
        mul     lfbWidth
        add     eax, ebx
        mov     esi, lfbPtr
        add     esi, eax
        mov     ebx, eax
        lodsb
        mov     edi, lfbPtr
        add     edi, ebx
        inc     al
        stosb
    quit:
    }
}

// Get pixel functions
uint32_t getPixel8(int32_t x, int32_t y)
{
    __asm {
        mov     ebx, x
        cmp     ebx, cminX
        jl      quit
        cmp     ebx, cmaxX
        jg      quit
        mov     eax, y
        cmp     eax, cminY
        jl      quit
        cmp     eax, cmaxY
        jg      quit
        add     eax, pageOffset
        mul     lfbWidth
        add     eax, ebx
        mov     esi, lfbPtr
        add     esi, eax
        xor     eax, eax
        lodsb
    quit:
    }
}

uint32_t getPixel1516(int32_t x, int32_t y)
{
    __asm {
        mov     ebx, x
        cmp     ebx, cminX
        jl      quit
        cmp     ebx, cmaxX
        jg      quit
        mov     eax, y
        cmp     eax, cminY
        jl      quit
        cmp     eax, cmaxY
        jg      quit
        add     eax, pageOffset
        mul     lfbWidth
        add     eax, ebx
        shl     eax, 1
        mov     esi, lfbPtr
        add     esi, eax
        xor     eax, eax
        lodsw
    quit:
    }
}

uint32_t getPixel24(int32_t x, int32_t y)
{
    __asm {
        mov     ebx, x
        cmp     ebx, cminX
        jl      quit
        cmp     ebx, cmaxX
        jg      quit
        mov     eax, y
        cmp     eax, cminY
        jl      quit
        cmp     eax, cmaxY
        jg      quit
        add     eax, pageOffset
        mul     lfbWidth
        add     eax, ebx
        mov     ebx, eax
        shl     eax, 1
        add     eax, ebx
        mov     esi, lfbPtr
        add     esi, eax
        xor     eax, eax
        xor     ebx, ebx
        mov     ax, [esi]
        mov     bl, [esi + 2]
        shl     ebx, 16
        or      eax, ebx
    quit:
    }
}

uint32_t getPixel32(int32_t x, int32_t y)
{
    __asm {
        mov     ebx, x
        cmp     ebx, cminX
        jl      quit
        cmp     ebx, cmaxX
        jg      quit
        mov     eax, y
        cmp     eax, cminY
        jl      quit
        cmp     eax, cmaxY
        jg      quit
        add     eax, pageOffset
        mul     lfbWidth
        add     eax, ebx
        shl     eax, 2
        mov     esi, lfbPtr
        add     esi, eax
        xor     eax, eax
        lodsd
    quit:
    }
}

// Put pixel add with destination
void putPixelAdd32(int32_t x, int32_t y, uint32_t col)
{
    __asm {
        mov     ebx, x
        cmp     ebx, cminX
        jl      quit
        cmp     ebx, cmaxX
        jg      quit
        mov     eax, y
        cmp     eax, cminY
        jl      quit
        cmp     eax, cmaxY
        jg      quit
        add     eax, pageOffset
        mul     lfbWidth
        add     eax, ebx
        shl     eax, 2
        mov     edi, lfbPtr
        add     edi, eax
        mov     eax, [edi]
        add     al, byte ptr col
        jnc     bstep
        mov     al, 255
    bstep:
        add     ah, byte ptr col[1]
        jnc     gstep
        mov     ah, 255
    gstep:
        mov     ebx, eax
        shr     ebx, 16
        add     bl, byte ptr col[2]
        jnc     rstep
        mov     bl, 255
    rstep:
        shl     ebx, 16
        and     eax, 00FFFFh
        or      eax, ebx
        stosd
    quit:
    }
}

void putPixelAdd24(int32_t x, int32_t y, uint32_t col)
{
    __asm {
        mov     ebx, x
        cmp     ebx, cminX
        jl      quit
        cmp     ebx, cmaxX
        jg      quit
        mov     eax, y
        cmp     eax, cminY
        jl      quit
        cmp     eax, cmaxY
        jg      quit
        add     eax, pageOffset
        mul     lfbWidth
        add     eax, ebx
        mov     ebx, eax
        shl     eax, 1
        add     eax, ebx
        mov     edi, lfbPtr
        add     edi, eax
        mov     ax, [edi]
        add     al, byte ptr col
        jnc     bstep
        mov     al, 255
    bstep:
        add     ah, byte ptr col[1]
        jnc     gstep
        mov     ah, 255
    gstep:
        mov     bl, [edi + 2]
        add     bl, byte ptr col[2]
        jnc     rstep
        mov     bl, 255
    rstep:
        stosw
        mov     al, bl
        stosb
    quit:
    }
}

// 16 bits pixel use RGB 565
void putPixelAdd16(int32_t x, int32_t y, uint32_t col)
{
    __asm {
        mov     ebx, x
        cmp     ebx, cminX
        jl      quit
        cmp     ebx, cmaxX
        jg      quit
        mov     eax, y
        cmp     eax, cminY
        jl      quit
        cmp     eax, cmaxY
        jg      quit
        add     eax, pageOffset
        mul     lfbWidth
        add     eax, ebx
        shl     eax, 1
        mov     edi, lfbPtr
        add     edi, eax
        mov     ax, [edi]
        mov     bx, word ptr col
        mov     dx, bx
        and     bx, 1111100000011111b
        and     dx, 11111100000b
        mov     cx, ax
        and     ax, 1111100000011111b
        and     cx, 11111100000b
        add     al, bl
        cmp     al, 11111b
        jna     bstep
        mov     al, 11111b
    bstep:
        add     ah, bh
        jnc     gstep
        mov     ah, 11111000b
    gstep:
        add     cx, dx
        cmp     cx, 11111100000b
        jna     rstep
        mov     cx, 11111100000b
    rstep:
        or      ax, cx
        stosw
    quit:
    }
}

// 15 bits pixel use RGB 555
void putPixelAdd15(int32_t x, int32_t y, uint32_t col)
{
    __asm {
        mov     ebx, x
        cmp     ebx, cminX
        jl      quit
        cmp     ebx, cmaxX
        jg      quit
        mov     eax, y
        cmp     eax, cminY
        jl      quit
        cmp     eax, cmaxY
        jg      quit
        add     eax, pageOffset
        mul     lfbWidth
        add     eax, ebx
        shl     eax, 1
        mov     edi, lfbPtr
        add     edi, eax
        mov     ax, [edi]
        mov     bx, word ptr col
        mov     dx, bx
        and     bx, 111110000011111b
        and     dx, 1111100000b
        mov     cx, ax
        and     ax, 111110000011111b
        and     cx, 1111100000b
        add     al, bl
        cmp     al, 11111b
        jna     bstep
        mov     al, 11111b
    bstep:
        add     ah, bh
        cmp     ah, 1111100b
        jna     gstep
        mov     ah, 1111100b
    gstep:
        add     cx, dx
        cmp     cx, 1111100000b
        jna     rstep
        mov     cx, 1111100000b
    rstep:
        or      ax, cx
        stosw
    quit:
    }
}

// Put pixel sub with destination
void putPixelSub32(int32_t x, int32_t y, uint32_t col)
{
    __asm {
        mov     ebx, x
        cmp     ebx, cminX
        jl      quit
        cmp     ebx, cmaxX
        jg      quit
        mov     eax, y
        cmp     eax, cminY
        jl      quit
        cmp     eax, cmaxY
        jg      quit
        add     eax, pageOffset
        mul     lfbWidth
        add     eax, ebx
        shl     eax, 2
        mov     edi, lfbPtr
        add     edi, eax
        mov     eax, [edi]
        sub     al, byte ptr col
        jnc     bstep
        xor     al, al
    bstep:
        sub     ah, byte ptr col[1]
        jnc     gstep
        xor     ah, ah
    gstep:
        mov     ebx, eax
        shr     ebx, 16
        sub     bl, byte ptr col[2]
        jnc     rstep
        xor     bl, bl
    rstep:
        shl     ebx, 16
        and     eax, 00FFFFh
        or      eax, ebx
        stosd
    quit:
    }
}

void putPixelSub24(int32_t x, int32_t y, uint32_t col)
{
    __asm {
        mov     ebx, x
        cmp     ebx, cminX
        jl      quit
        cmp     ebx, cmaxX
        jg      quit
        mov     eax, y
        cmp     eax, cminY
        jl      quit
        cmp     eax, cmaxY
        jg      quit
        add     eax, pageOffset
        mul     lfbWidth
        add     eax, ebx
        mov     ebx, eax
        shl     eax, 1
        add     eax, ebx
        mov     edi, lfbPtr
        add     edi, eax
        mov     ax, [edi]
        sub     al, byte ptr col
        jnc     bstep
        xor     al, al
    bstep:
        sub     ah, byte ptr col[1]
        jnc     gstep
        xor     ah, ah
    gstep:
        mov     bl, [edi + 2]
        sub     bl, byte ptr col[2]
        jnc     rstep
        xor     bl, bl
    rstep:
        stosw
        mov     al, bl
        stosb
    quit:
    }
}

// 16 bits pixel use RGB 565
void putPixelSub16(int32_t x, int32_t y, uint32_t col)
{
    __asm {
        mov     ebx, x
        cmp     ebx, cminX
        jl      quit
        cmp     ebx, cmaxX
        jg      quit
        mov     eax, y
        cmp     eax, cminY
        jl      quit
        cmp     eax, cmaxY
        jg      quit
        add     eax, pageOffset
        mul     lfbWidth
        add     eax, ebx
        shl     eax, 1
        mov     edi, lfbPtr
        add     edi, eax
        mov     ax, [edi]
        mov     bx, word ptr col
        mov     dx, bx
        and     bx, 1111100000011111b
        and     dx, 11111100000b
        mov     cx, ax
        and     ax, 1111100000011111b
        and     cx, 11111100000b
        sub     al, bl
        jnc     bstep
        xor     al, al
    bstep:
        sub     ah, bh
        jnc     gstep
        xor     ah, ah
    gstep:
        sub     cx, dx
        jc      rstep
        or      ax, cx
    rstep:
        stosw
    quit:
    }
}

// 15 bits pixel use RGB 555
void putPixelSub15(int32_t x, int32_t y, uint32_t col)
{
    __asm {
        mov     ebx, x
        cmp     ebx, cminX
        jl      quit
        cmp     ebx, cmaxX
        jg      quit
        mov     eax, y
        cmp     eax, cminY
        jl      quit
        cmp     eax, cmaxY
        jg      quit
        add     eax, pageOffset
        mul     lfbWidth
        add     eax, ebx
        shl     eax, 1
        mov     edi, lfbPtr
        add     edi, eax
        mov     ax, [edi]
        mov     bx, word ptr col
        mov     dx, bx
        and     bx, 111110000011111b
        and     dx, 1111100000b
        mov     cx, ax
        and     ax, 111110000011111b
        and     cx, 1111100000b
        sub     al, bl
        jnc     bstep
        xor     al, al
    bstep:
        sub     ah, bh
        jnc     gstep
        xor     ah, ah
    gstep:
        sub     cx, dx
        jc      rstep
        or      ax, cx
    rstep:
        stosw
    quit:
    }
}

// Clear screen with color functions
void clearScreen8(uint32_t col)
{
#ifdef _USE_MMX    
    __asm {
        mov         eax, pageOffset
        mul         bytesPerScanline
        mov         edi, lfbPtr
        add         edi, eax
        mov         ecx, lfbSize
        shr         ecx, 3
        movd        mm0, col
        punpcklbw   mm0, mm0
        punpcklwd   mm0, mm0
        punpckldq   mm0, mm0
    move:
        movq        [edi], mm0
        add         edi, 8
        dec         ecx
        jnz         move
        emms
    }
#else
    __asm {
        mov         eax, pageOffset
        mul         bytesPerScanline
        mov         edi, lfbPtr
        add         edi, eax
        mov         ecx, lfbSize
        shr         ecx, 2
        mov         eax, col
        mov         ah, al
        mov         ebx, eax
        shl         eax, 16
        or          eax, ebx
        rep         stosd
    }
#endif
}

void clearScreen1516(uint32_t col)
{
#ifdef _USE_MMX    
    __asm {
        mov         eax, pageOffset
        mul         bytesPerScanline
        mov         edi, lfbPtr
        add         edi, eax
        mov         ecx, lfbSize
        shr         ecx, 3
        mov         eax, col
        shl         eax, 16
        or          eax, col
        movd        mm0, eax
        punpckldq   mm0, mm0
    move:
        movq        [edi], mm0
        add         edi, 8
        dec         ecx
        jnz         move
        emms
    }
#else
    __asm {
        mov         eax, pageOffset
        mul         bytesPerScanline
        mov         edi, lfbPtr
        add         edi, eax
        mov         ecx, lfbSize
        shr         ecx, 1
        mov         eax, col
        shl         eax, 16
        or          eax, col
        rep         stosd
    }
#endif
}

void clearScreen24(uint32_t col)
{
    __asm {
        mov     eax, pageOffset
        mul     bytesPerScanline 
        mov     edi, lfbPtr
        add     edi, eax
        mov     ecx, lfbSize
    next:
        mov     eax, col
        stosw
        shr     eax, 16
        stosb
        dec     ecx
        jnz     next
    }
}

void clearScreen32(uint32_t col)
{
#ifdef _USE_MMX    
    __asm {
        mov         eax, pageOffset
        mul         bytesPerScanline
        mov         edi, lfbPtr
        add         edi, eax
        mov         ecx, lfbSize
        shr         ecx, 3
        movd        mm0, col
        punpckldq   mm0, mm0
    move:
        movq        [edi], mm0
        add         edi, 8
        dec         ecx
        jnz         move
        emms
    }
#else
    __asm {
        mov         eax, pageOffset
        mul         bytesPerScanline
        mov         edi, lfbPtr
        add         edi, eax
        mov         ecx, lfbSize
        mov         eax, col
        rep         stosd
    }
#endif
}

// Fill box with color functions
void fillRect8(int32_t x1, int32_t y1, int32_t width, int32_t height, uint32_t col)
{
    int32_t lwidth, lheight, x2, y2;
    int32_t lminX, lminY, lmaxX, lmaxY;
    
    // Calculate new position
    x2 = x1 + (width - 1);
    y2 = y1 + (height - 1);

    // Clip image to context boundaries
    lminX = max(x1, cminX);
    lminY = max(y1, cminY);
    lmaxX = min(x2, cmaxX);
    lmaxY = min(y2, cmaxY);

    // Initialize loop variables
    lwidth  = (lmaxX - lminX) + 1;
    lheight = (lmaxY - lminY) + 1;

    // Check range
    if (lwidth <= 0 || lheight <= 0) return;

#ifdef _USE_MMX
    __asm {
        mov         edi, lfbPtr
        mov         eax, lminY
        add         eax, pageOffset
        mul         lfbWidth
        add         eax, lminX
        add         edi, eax
        mov         eax, col
        mov         edx, lfbWidth
        sub         edx, lwidth
        mov         ebx, lwidth
        and         ebx, 7
        movd        mm0, eax
        punpcklbw   mm0, mm0
        punpcklwd   mm0, mm0
        punpckldq   mm0, mm0
    next:
        mov         ecx, lwidth
        shr         ecx, 3
        jz          skip
    store:
        movq        [edi], mm0
        add         edi, 8
        dec         ecx
        jnz         store
    skip:
        mov         ecx, ebx
        rep         stosb
        add         edi, edx
        dec         lheight
        jnz         next
        emms
    }
#else
    __asm {
        mov         edi, lfbPtr
        mov         eax, lminY
        add         eax, pageOffset
        mul         lfbWidth
        add         eax, lminX
        add         edi, eax
        mov         edx, lfbWidth
        sub         edx, lwidth
        push        edx
        mov         ebx, lwidth
        shr         ebx, 2
        mov         edx, lwidth
        and         edx, 3
        mov         eax, col
        mov         ah, al
        mov         ecx, eax
        shl         eax, 16
        or          eax, ecx
    next:
        mov         ecx, ebx
        rep         stosd
        mov         ecx, edx
        rep         stosb
        add         edi, [esp]
        dec         lheight
        jnz         next
        pop         edx
    }
#endif
}

void fillRect1516(int32_t x1, int32_t y1, int32_t width, int32_t height, uint32_t col)
{
    int32_t lwidth, lheight, x2, y2;
    int32_t lminX, lminY, lmaxX, lmaxY;
    
    // Calculate new position
    x2 = x1 + (width - 1);
    y2 = y1 + (height - 1);
    
    // Clip image to context boundaries
    lminX = max(x1, cminX);
    lminY = max(y1, cminY);
    lmaxX = min(x2, cmaxX);
    lmaxY = min(y2, cmaxY);

    // Initialize loop variables
    lwidth  = (lmaxX - lminX) + 1;
    lheight = (lmaxY - lminY) + 1;

    // Check range
    if (lwidth <= 0 || lheight <= 0) return;

#ifdef _USE_MMX
    __asm {
        mov         edi, lfbPtr
        mov         eax, lminY
        add         eax, pageOffset
        mul         lfbWidth
        add         eax, lminX
        shl         eax, 1
        add         edi, eax
        mov         edx, lfbWidth
        sub         edx, lwidth
        shl         edx, 1
        mov         eax, col
        mov         ebx, lwidth
        and         ebx, 3
        movd        mm0, eax
        punpcklwd   mm0, mm0
        punpckldq   mm0, mm0
    next:
        mov         ecx, lwidth
        shr         ecx, 2
        jz          skip
    store:
        movq        [edi], mm0
        add         edi, 8
        dec         ecx
        jnz         store
    skip:
        mov         ecx, ebx
        rep         stosw
        add         edi, edx
        dec         lheight
        jnz         next
        emms
    }
#else
    __asm {
        mov         edi, lfbPtr
        mov         eax, lminY
        add         eax, pageOffset
        mul         lfbWidth
        add         eax, lminX
        shl         eax, 1
        add         edi, eax
        mov         edx, lfbWidth
        sub         edx, lwidth
        shl         edx, 1
        push        edx
        mov         ebx, lwidth
        shr         ebx, 1
        mov         edx, lwidth
        and         edx, 1
        mov         eax, col
        shl         eax, 16
        or          eax, col
    next:
        mov         ecx, ebx
        rep         stosd
        mov         ecx, edx
        rep         stosw
        add         edi, [esp]
        dec         lheight
        jnz         next
        pop         edx
    }
#endif
}

void fillRect24(int32_t x1, int32_t y1, int32_t width, int32_t height, uint32_t col)
{
    int32_t lwidth, lheight, x2, y2;
    int32_t lminX, lminY, lmaxX, lmaxY;
    
    // Calculate new position
    x2 = x1 + (width - 1);
    y2 = y1 + (height - 1);
    
    // Clip image to context boundaries
    lminX = max(x1, cminX);
    lminY = max(y1, cminY);
    lmaxX = min(x2, cmaxX);
    lmaxY = min(y2, cmaxY);

    // Initialize loop variables
    lwidth  = (lmaxX - lminX) + 1;
    lheight = (lmaxY - lminY) + 1;

    // Check range
    if (lwidth <= 0 || lheight <= 0) return;

    __asm {
        mov     edi, lfbPtr
        mov     eax, lminY
        add     eax, pageOffset
        mul     lfbWidth
        add     eax, lminX
        mov     ebx, eax
        shl     eax, 1
        add     eax, ebx
        add     edi, eax
        mov     ebx, lfbWidth
        sub     ebx, lwidth
        mov     eax, ebx
        shl     ebx, 1
        add     ebx, eax
    next:
        mov     ecx, lwidth
    plot:
        mov     eax, col
        stosw
        shr     eax, 16
        stosb
        dec     ecx
        jnz     plot
        add     edi, ebx
        dec     lheight
        jnz     next
    }
}

void fillRect32(int32_t x1, int32_t y1, int32_t width, int32_t height, uint32_t col)
{
    int32_t lwidth, lheight, x2, y2;
    int32_t lminX, lminY, lmaxX, lmaxY;
    
    // Calculate new position
    x2 = x1 + (width - 1);
    y2 = y1 + (height - 1);

    // Clip image to context boundaries
    lminX = max(x1, cminX);
    lminY = max(y1, cminY);
    lmaxX = min(x2, cmaxX);
    lmaxY = min(y2, cmaxY);

    // Initialize loop variables
    lwidth  = (lmaxX - lminX) + 1;
    lheight = (lmaxY - lminY) + 1;

    // Check range
    if (lwidth <= 0 || lheight <= 0) return;

#ifdef _USE_MMX
    __asm {
        mov         edi, lfbPtr
        mov         eax, lminY
        add         eax, pageOffset
        mul         lfbWidth
        add         eax, lminX
        shl         eax, 2
        add         edi, eax
        mov         edx, lfbWidth
        sub         edx, lwidth
        shl         edx, 2
        mov         eax, col
        mov         ebx, lwidth
        and         ebx, 1
        movd        mm0, eax
        punpckldq   mm0, mm0
    next:
        mov         ecx, lwidth
        shr         ecx, 1
        jz          skip
    store:
        movq        [edi], mm0
        add         edi, 8
        dec         ecx
        jnz         store
    skip:
        mov         ecx, ebx
        rep         stosd
        add         edi, edx
        dec         lheight
        jnz         next
        emms
    }
#else
    __asm {
        mov         edi, lfbPtr
        mov         eax, lminY
        add         eax, pageOffset
        mul         lfbWidth
        add         eax, lminX
        shl         eax, 2
        add         edi, eax
        mov         ebx, lfbWidth
        sub         ebx, lwidth
        shl         ebx, 2
        mov         eax, col
    next:
        mov         ecx, lwidth
        rep         stosd
        add         edi, ebx
        dec         lheight
        jnz         next
    }
#endif
}

// Fill rectangle with adding current pixel color
void fillRectAdd32(int32_t x1, int32_t y1, int32_t width, int32_t height, uint32_t col)
{
    int32_t lwidth, lheight, x2, y2;
    int32_t lminX, lminY, lmaxX, lmaxY;
    
    // Calculate new position
    x2 = x1 + (width - 1);
    y2 = y1 + (height - 1);
    
    // Clip image to context boundaries
    lminX = max(x1, cminX);
    lminY = max(y1, cminY);
    lmaxX = min(x2, cmaxX);
    lmaxY = min(y2, cmaxY);

    // Initialize loop variables
    lwidth  = (lmaxX - lminX) + 1;
    lheight = (lmaxY - lminY) + 1;

    // Check range
    if (lwidth <= 0 || lheight <= 0) return;

#ifdef _USE_MMX
    __asm {
        mov         edi, lfbPtr
        mov         eax, lminY
        add         eax, pageOffset
        mul         lfbWidth
        add         eax, lminX
        shl         eax, 2
        add         edi, eax
        mov         edx, lfbWidth
        sub         edx, lwidth
        shl         edx, 2
        movd        mm1, col
        mov         ebx, lwidth
        shr         ebx, 1
    next:
        test        ebx, ebx
        jz          once
        mov         ecx, ebx
        punpckldq   mm1, mm1
    move:
        movq        mm0, [edi]
        paddusb     mm0, mm1
        movq        [edi], mm0
        add         edi, 8
        dec         ecx
        jnz         move
    once:
        test        lwidth, 1
        jz          skip
        movd        mm0, [edi]
        paddusb     mm0, mm1
        movd        [edi], mm0
        add         edi, 4
    skip:
        add         edi, edx
        dec         lheight
        jnz         next
        emms
    }
#else
    __asm {
        mov         edi, lfbPtr
        mov         eax, lminY
        add         eax, pageOffset
        mul         lfbWidth
        add         eax, lminX
        shl         eax, 2
        add         edi, eax
        mov         edx, lfbWidth
        sub         edx, lwidth
        shl         edx, 2
    next:
        mov         ecx, lwidth
    plot:
        mov         eax, [edi]
        add         al, byte ptr col
        jnc         bstep
        mov         al, 255
    bstep:
        add         ah, byte ptr col[1]
        jnc         gstep
        mov         ah, 255
    gstep:
        mov         ebx, eax
        shr         ebx, 16
        add         bl, byte ptr col[2]
        jnc         rstep
        mov         bl, 255
    rstep:
        shl         ebx, 16
        and         eax, 00FFFFh
        or          eax, ebx
        stosd
        dec         ecx
        jnz         plot
        add         edi, edx
        dec         lheight
        jnz         next
    }
#endif    
}

void fillRectAdd24(int32_t x1, int32_t y1, int32_t width, int32_t height, uint32_t col)
{
    int32_t lwidth, lheight, x2, y2;
    int32_t lminX, lminY, lmaxX, lmaxY;
    
    // Calculate new position
    x2 = x1 + (width - 1);
    y2 = y1 + (height - 1);
    
    // Clip image to context boundaries
    lminX = max(x1, cminX);
    lminY = max(y1, cminY);
    lmaxX = min(x2, cmaxX);
    lmaxY = min(y2, cmaxY);

    // Initialize loop variables
    lwidth  = (lmaxX - lminX) + 1;
    lheight = (lmaxY - lminY) + 1;

    // Check range
    if (lwidth <= 0 || lheight <= 0) return;

    __asm {
        mov     edi, lfbPtr
        mov     eax, lminY
        add     eax, pageOffset
        mul     lfbWidth
        add     eax, lminX
        mov     ebx, eax
        shl     eax, 1
        add     eax, ebx
        add     edi, eax
        mov     edx, lfbWidth
        sub     edx, lwidth
        mov     eax, edx
        shl     edx, 1
        add     edx, eax
    next:
        mov     ecx, lwidth
    plot:
        mov     ax, [edi]
        add     al, byte ptr col
        jnc     bstep
        mov     al, 255
    bstep:
        add     ah, byte ptr col[1]
        jnc     gstep
        mov     ah, 255
    gstep:
        mov     bl, [edi + 2]
        add     bl, byte ptr col[2]
        jnc     rstep
        mov     bl, 255
    rstep:
        stosw
        mov     al, bl
        stosb
        dec     ecx
        jnz     plot
        add     edi, edx
        dec     lheight
        jnz     next
    }
}

void fillRectAdd16(int32_t x1, int32_t y1, int32_t width, int32_t height, uint32_t col)
{
    int32_t lwidth, lheight, x2, y2;
    int32_t lminX, lminY, lmaxX, lmaxY;
    
    // Calculate new position
    x2 = x1 + (width - 1);
    y2 = y1 + (height - 1);
    
    // Clip image to context boundaries
    lminX = max(x1, cminX);
    lminY = max(y1, cminY);
    lmaxX = min(x2, cmaxX);
    lmaxY = min(y2, cmaxY);

    // Initialize loop variables
    lwidth  = (lmaxX - lminX) + 1;
    lheight = (lmaxY - lminY) + 1;

    // Check range
    if (lwidth <= 0 || lheight <= 0) return;

    __asm {
        mov     edi, lfbPtr
        mov     eax, lminY
        add     eax, pageOffset
        mul     lfbWidth
        add     eax, lminX
        shl     eax, 1
        add     edi, eax
        mov     ecx, lfbWidth
        sub     ecx, lwidth
        shl     ecx, 1
        mov     bx, word ptr col
        mov     dx, bx
        and     bx, 1111100000011111b
        and     dx, 11111100000b
    next:
        push    ecx
        push    lwidth
    plot:
        mov     ax, [edi]
        mov     cx, ax
        and     ax, 1111100000011111b
        and     cx, 11111100000b
        add     al, bl
        cmp     al, 11111b
        jna     bstep
        mov     al, 11111b
    bstep:
        add     ah, bh
        jnc     gstep
        mov     ah, 11111000b
    gstep:
        add     cx, dx
        cmp     cx, 11111100000b
        jna     rstep
        mov     cx, 11111100000b
    rstep:
        or      ax, cx
        stosw
        dec     lwidth
        jnz     plot
        pop     lwidth
        pop     ecx
        add     edi, ecx
        dec     lheight
        jnz     next
    }
}

void fillRectAdd15(int32_t x1, int32_t y1, int32_t width, int32_t height, uint32_t col)
{
    int32_t lwidth, lheight, x2, y2;
    int32_t lminX, lminY, lmaxX, lmaxY;
    
    // Calculate new position
    x2 = x1 + (width - 1);
    y2 = y1 + (height - 1);
    
    // Clip image to context boundaries
    lminX = max(x1, cminX);
    lminY = max(y1, cminY);
    lmaxX = min(x2, cmaxX);
    lmaxY = min(y2, cmaxY);

    // Initialize loop variables
    lwidth  = (lmaxX - lminX) + 1;
    lheight = (lmaxY - lminY) + 1;

    // Check range
    if (lwidth <= 0 || lheight <= 0) return;

    __asm {
        mov     edi, lfbPtr
        mov     eax, lminY
        add     eax, pageOffset
        mul     lfbWidth
        add     eax, lminX
        shl     eax, 1
        add     edi, eax
        mov     ecx, lfbWidth
        sub     ecx, lwidth
        shl     ecx, 1
        mov     bx, word ptr col
        mov     dx, bx
        and     bx, 111110000011111b
        and     dx, 1111100000b
    next:
        push    ecx
        push    lwidth
    plot:
        mov     ax, [edi]
        mov     cx, ax
        and     ax, 111110000011111b
        and     cx, 1111100000b
        add     al, bl
        cmp     al, 11111b
        jna     bstep
        mov     al, 11111b
    bstep:
        add     ah, bh
        cmp     ah, 1111100b
        jna     gstep
        mov     ah, 1111100b
    gstep:
        add     cx, dx
        cmp     cx, 1111100000b
        jna     rstep
        mov     cx, 1111100000b
    rstep:
        or      ax, cx
        stosw
        dec     lwidth
        jnz     plot
        pop     lwidth
        pop     ecx
        add     edi, ecx
        dec     lheight
        jnz     next
    }
}

// Fill rectangle with substraction current pixel color
void fillRectSub32(int32_t x1, int32_t y1, int32_t width, int32_t height, uint32_t col)
{
    int32_t lwidth, lheight, x2, y2;
    int32_t lminX, lminY, lmaxX, lmaxY;
    
    // Calculate new position
    x2 = x1 + (width - 1);
    y2 = y1 + (height - 1);

    // Clip image to context boundaries
    lminX = max(x1, cminX);
    lminY = max(y1, cminY);
    lmaxX = min(x2, cmaxX);
    lmaxY = min(y2, cmaxY);

    // Initialize loop variables
    lwidth  = (lmaxX - lminX) + 1;
    lheight = (lmaxY - lminY) + 1;

    // Check range
    if (lwidth <= 0 || lheight <= 0) return;

#ifdef _USE_MMX
    __asm {
        mov         edi, lfbPtr
        mov         eax, lminY
        add         eax, pageOffset
        mul         lfbWidth
        add         eax, lminX
        shl         eax, 2
        add         edi, eax
        mov         edx, lfbWidth
        sub         edx, lwidth
        shl         edx, 2
        movd        mm1, col
        mov         ebx, lwidth
        shr         ebx, 1
    next:
        test        ebx, ebx
        jz          once
        mov         ecx, ebx
        punpckldq   mm1, mm1
    move:
        movq        mm0, [edi]
        psubusb     mm0, mm1
        movq        [edi], mm0
        add         edi, 8
        dec         ecx
        jnz         move
    once:
        test        lwidth, 1
        jz          skip
        movd        mm0, [edi]
        psubusb     mm0, mm1
        movd        [edi], mm0
        add         edi, 4
    skip:
        add         edi, edx
        dec         lheight
        jnz         next
        emms
    }
#else
    __asm {
        mov         edi, lfbPtr
        mov         eax, lminY
        add         eax, pageOffset
        mul         lfbWidth
        add         eax, lminX
        shl         eax, 2
        add         edi, eax
        mov         edx, lfbWidth
        sub         edx, lwidth
        shl         edx, 2
    next:
        mov         ecx, lwidth
    plot:
        mov         eax, [edi]
        sub         al, byte ptr col
        jnc         bstep
        xor         al, al
    bstep:
        sub         ah, byte ptr col[1]
        jnc         gstep
        xor         ah, ah
    gstep:
        mov         ebx, eax
        shr         ebx, 16
        sub         bl, byte ptr col[2]
        jnc         rstep
        xor         bl, bl
    rstep:
        shl         ebx, 16
        and         eax, 00FFFFh
        or          eax, ebx
        stosd
        dec         ecx
        jnz         plot
        add         edi, edx
        dec         lheight
        jnz         next
    }
#endif    
}

void fillRectSub24(int32_t x1, int32_t y1, int32_t width, int32_t height, uint32_t col)
{
    int32_t lwidth, lheight, x2, y2;
    int32_t lminX, lminY, lmaxX, lmaxY;
    
    // Calculate new position
    x2 = x1 + (width - 1);
    y2 = y1 + (height - 1);

    // Clip image to context boundaries
    lminX = max(x1, cminX);
    lminY = max(y1, cminY);
    lmaxX = min(x2, cmaxX);
    lmaxY = min(y2, cmaxY);

    // Initialize loop variables
    lwidth  = (lmaxX - lminX) + 1;
    lheight = (lmaxY - lminY) + 1;

    // Check range
    if (lwidth <= 0 || lheight <= 0) return;

    __asm {
        mov     edi, lfbPtr
        mov     eax, lminY
        add     eax, pageOffset
        mul     lfbWidth
        add     eax, lminX
        mov     ebx, eax
        shl     eax, 1
        add     eax, ebx
        add     edi, eax
        mov     edx, lfbWidth
        sub     edx, lwidth
        mov     eax, edx
        shl     edx, 1
        add     edx, eax
    next:
        mov     ecx, lwidth
    plot:
        mov     ax, [edi]
        sub     al, byte ptr col
        jnc     bstep
        xor     al, al
    bstep:
        sub     ah, byte ptr col[1]
        jnc     gstep
        xor     ah, ah
    gstep:
        mov     bl, [edi + 2]
        sub     bl, byte ptr col[2]
        jnc     rstep
        xor     bl, bl
    rstep:
        stosw
        mov     al, bl
        stosb
        dec     ecx
        jnz     plot
        add     edi, edx
        dec     lheight
        jnz     next
    }
}

void fillRectSub16(int32_t x1, int32_t y1, int32_t width, int32_t height, uint32_t col)
{
    int32_t lwidth, lheight, x2, y2;
    int32_t lminX, lminY, lmaxX, lmaxY;
    
    // Calculate new position
    x2 = x1 + (width - 1);
    y2 = y1 + (height - 1);
    
    // Clip image to context boundaries
    lminX = max(x1, cminX);
    lminY = max(y1, cminY);
    lmaxX = min(x2, cmaxX);
    lmaxY = min(y2, cmaxY);

    // Initialize loop variables
    lwidth  = (lmaxX - lminX) + 1;
    lheight = (lmaxY - lminY) + 1;

    // Check range
    if (lwidth <= 0 || lheight <= 0) return;

    __asm {
        mov     edi, lfbPtr
        mov     eax, lminY
        add     eax, pageOffset
        mul     lfbWidth
        add     eax, lminX
        shl     eax, 1
        add     edi, eax
        mov     ecx, lfbWidth
        sub     ecx, lwidth
        shl     ecx, 1
        mov     bx, word ptr col
        mov     dx, bx
        and     bx, 1111100000011111b
        and     dx, 11111100000b
    next:
        push    ecx
        push    lwidth
    plot:
        mov     ax, [edi]
        mov     cx, ax
        and     ax, 1111100000011111b
        and     cx, 11111100000b
        sub     al, bl
        jnc     bstep
        xor     al, al
    bstep:
        sub     ah, bh
        jnc     gstep
        xor     ah, ah
    gstep:
        sub     cx, dx
        jc      rstep
        or      ax, cx
    rstep:
        stosw
        dec     lwidth
        jnz     plot
        pop     lwidth
        pop     ecx
        add     edi, ecx
        dec     lheight
        jnz     next
    }
}

void fillRectSub15(int32_t x1, int32_t y1, int32_t width, int32_t height, uint32_t col)
{
    int32_t lwidth, lheight, x2, y2;
    int32_t lminX, lminY, lmaxX, lmaxY;
    
    // Calculate new position
    x2 = x1 + (width - 1);
    y2 = y1 + (height - 1);
    
    // Clip image to context boundaries
    lminX = max(x1, cminX);
    lminY = max(y1, cminY);
    lmaxX = min(x2, cmaxX);
    lmaxY = min(y2, cmaxY);

    // Initialize loop variables
    lwidth  = (lmaxX - lminX) + 1;
    lheight = (lmaxY - lminY) + 1;

    // Check range
    if (lwidth <= 0 || lheight <= 0) return;

    __asm {
        mov     edi, lfbPtr
        mov     eax, lminY
        add     eax, pageOffset
        mul     lfbWidth
        add     eax, lminX
        shl     eax, 1
        add     edi, eax
        mov     ecx, lfbWidth
        sub     ecx, lwidth
        shl     ecx, 1
        mov     bx, word ptr col
        mov     dx, bx
        and     bx, 111110000011111b
        and     dx, 1111100000b
    next:
        push    ecx
        push    lwidth
    plot:
        mov     ax, [edi]
        mov     cx, ax
        and     ax, 111110000011111b
        and     cx, 1111100000b
        sub     al, bl
        jnc     bstep
        xor     al, al
    bstep:
        sub     ah, bh
        jnc     gstep
        xor     ah, ah
    gstep:
        sub     cx, dx
        jc      rstep
        or      ax, cx
    rstep:
        stosw
        dec     lwidth
        jnz     plot
        pop     lwidth
        pop     ecx
        add     edi, ecx
        dec     lheight
        jnz     next
    }
}

void fillRectPattern32(int32_t x1, int32_t y1, int32_t width, int32_t height, uint32_t col, uint8_t *pattern)
{
    int32_t lwidth, lheight, x2, y2;
    int32_t lminX, lminY, lmaxX, lmaxY;
    
    // Calculate new position
    x2 = x1 + (width - 1);
    y2 = y1 + (height - 1);
    
    // Clip image to context boundaries
    lminX = max(x1, cminX);
    lminY = max(y1, cminY);
    lmaxX = min(x2, cmaxX);
    lmaxY = min(y2, cmaxY);

    // Initialize loop variables
    lwidth  = (lmaxX - lminX) + 1;
    lheight = (lmaxY - lminY) + 1;

    // Check range
    if (lwidth <= 0 || lheight <= 0) return;

#ifdef _USE_MMX
    __asm {
        mov         edi, lfbPtr
        mov         eax, lminY
        add         eax, pageOffset
        mul         lfbWidth
        add         eax, lminX
        shl         eax, 2
        add         edi, eax
        mov         edx, lfbWidth
        sub         edx, lwidth
        shl         edx, 2
        mov         esi, pattern
        movd        mm0, col
    plot:
        mov         ecx, lminX
        and         ecx, 7
        mov         ebx, lheight
        and         ebx, 7
        mov         al, [esi + ebx]
        rol         al, cl
        mov         ecx, lwidth
        shr         ecx, 1
        jz          once
        punpckldq   mm0, mm0
    next:
        test        al, 1
        jz          skip1
        test        al, 2
        jz          skip2
        movq        [edi], mm0
        jmp         skip0
    skip2:
        movd        [edi + 4], mm0
        jmp         skip0
    skip1:
        test        al, 2
        jz          skip0
        movd        [edi], mm0
    skip0:
        add         edi, 8
        rol         al, 2
        dec         ecx
        jnz         next
    once:
        test        lwidth, 1
        jz          end1
        test        al, 2
        jz          end0
        movd        [edi], mm0
    end0:
        add         edi, 4
    end1:
        add         edi, edx
        dec         lheight
        jnz         plot
        emms        
    }
#else
    __asm {
        mov         edi, lfbPtr
        mov         eax, lminY
        add         eax, pageOffset
        mul         lfbWidth
        add         eax, lminX
        shl         eax, 2
        add         edi, eax
        mov         edx, lfbWidth
        sub         edx, lwidth
        shl         edx, 2
        mov         esi, pattern
    plot:
        mov         ecx, lminX
        and         ecx, 7
        mov         ebx, lheight
        and         ebx, 7
        mov         al, [esi + ebx]
        rol         al, cl
        mov         ebx, col
        mov         ecx, lwidth
    next:
        test        al, 1
        jz          step
        mov         [edi], ebx
    step:
        add         edi, 4
        rol         al, 1
        dec         ecx
        jnz         next
        add         edi, edx
        dec         lheight
        jnz         plot
    }
#endif
}

void fillRectPattern24(int32_t x1, int32_t y1, int32_t width, int32_t height, uint32_t col, uint8_t *pattern)
{
    int32_t lwidth, lheight, x2, y2;
    int32_t lminX, lminY, lmaxX, lmaxY;
    
    // Calculate new position
    x2 = x1 + (width - 1);
    y2 = y1 + (height - 1);
    
    // Clip image to context boundaries
    lminX = max(x1, cminX);
    lminY = max(y1, cminY);
    lmaxX = min(x2, cmaxX);
    lmaxY = min(y2, cmaxY);

    // Initialize loop variables
    lwidth  = (lmaxX - lminX) + 1;
    lheight = (lmaxY - lminY) + 1;

    // Check range
    if (lwidth <= 0 || lheight <= 0) return;

    __asm {
        mov     edi, lfbPtr
        mov     eax, lminY
        add     eax, pageOffset
        mul     lfbWidth
        add     eax, lminX
        mov     ebx, eax
        shl     eax, 1
        add     eax, ebx
        add     edi, eax
        mov     edx, lfbWidth
        sub     edx, lwidth
        mov     eax, edx
        shl     edx, 1
        add     edx, eax
        mov     esi, pattern
    plot:
        mov     ecx, lminX
        and     ecx, 7
        mov     ebx, lheight
        and     ebx, 7
        mov     al, [esi + ebx]
        rol     al, cl
        mov     ecx, lwidth
    next:
        test    al, 1
        jz      step
        mov     ebx, col
        mov     [edi], bx
        shr     ebx, 16
        mov     [edi + 2], bl
    step:
        add     edi, 3
        rol     al, 1
        dec     ecx
        jnz     next
        add     edi, edx
        dec     lheight
        jnz     plot
    }
}

void fillRectPattern1516(int32_t x1, int32_t y1, int32_t width, int32_t height, uint32_t col, uint8_t *pattern)
{
    int32_t lwidth, lheight, x2, y2;
    int32_t lminX, lminY, lmaxX, lmaxY;
    
    // Calculate new position
    x2 = x1 + (width - 1);
    y2 = y1 + (height - 1);

    // Clip image to context boundaries
    lminX = max(x1, cminX);
    lminY = max(y1, cminY);
    lmaxX = min(x2, cmaxX);
    lmaxY = min(y2, cmaxY);

    // Initialize loop variables
    lwidth  = (lmaxX - lminX) + 1;
    lheight = (lmaxY - lminY) + 1;

    // Check range
    if (lwidth <= 0 || lheight <= 0) return;

    __asm {
        mov     edi, lfbPtr
        mov     eax, lminY
        add     eax, pageOffset
        mul     lfbWidth
        add     eax, lminX
        shl     eax, 1
        add     edi, eax
        mov     edx, lfbWidth
        sub     edx, lwidth
        shl     edx, 1
        mov     esi, pattern
    plot:
        mov     ecx, lminX
        and     ecx, 7
        mov     ebx, lheight
        and     ebx, 7
        mov     al, [esi + ebx]
        rol     al, cl
        mov     ebx, col
        mov     ecx, lwidth
    next:
        test    al, 1
        jz      step
        mov     [edi], bx
    step:
        add     edi, 2
        rol     al, 1
        dec     ecx
        jnz     next
        add     edi, edx
        dec     lheight
        jnz     plot
    }
}

void fillRectPattern8(int32_t x1, int32_t y1, int32_t width, int32_t height, uint32_t col, uint8_t *pattern)
{
    int32_t lwidth, lheight, x2, y2;
    int32_t lminX, lminY, lmaxX, lmaxY;
    
    // Calculate new position
    x2 = x1 + (width - 1);
    y2 = y1 + (height - 1);

    // Clip image to context boundaries
    lminX = max(x1, cminX);
    lminY = max(y1, cminY);
    lmaxX = min(x2, cmaxX);
    lmaxY = min(y2, cmaxY);

    // Initialize loop variables
    lwidth  = (lmaxX - lminX) + 1;
    lheight = (lmaxY - lminY) + 1;

    // Check range
    if (lwidth <= 0 || lheight <= 0) return;

    __asm {
        mov     edi, lfbPtr
        mov     eax, lminY
        add     eax, pageOffset
        mul     lfbWidth
        add     eax, lminX
        add     edi, eax
        mov     edx, lfbWidth
        sub     edx, lwidth
        mov     esi, pattern
    plot:
        mov     ecx, lminX
        and     ecx, 7
        mov     ebx, lheight
        and     ebx, 7
        mov     al, [esi + ebx]
        rol     al, cl
        mov     ebx, col
        mov     ecx, lwidth
    next:
        test    al, 1
        jz      step
        mov     [edi], bl
    step:
        inc     edi
        rol     al, 1
        dec     ecx
        jnz     next
        add     edi, edx
        dec     lheight
        jnz     plot
    }
}

void fillRectPatternAdd32(int32_t x1, int32_t y1, int32_t width, int32_t height, uint32_t col, uint8_t *pattern)
{
    int32_t lwidth, lheight, x2, y2;
    int32_t lminX, lminY, lmaxX, lmaxY;
    
    // Calculate new position
    x2 = x1 + (width - 1);
    y2 = y1 + (height - 1);
    
    // Clip image to context boundaries
    lminX = max(x1, cminX);
    lminY = max(y1, cminY);
    lmaxX = min(x2, cmaxX);
    lmaxY = min(y2, cmaxY);

    // Initialize loop variables
    lwidth  = (lmaxX - lminX) + 1;
    lheight = (lmaxY - lminY) + 1;

    // Check range
    if (lwidth <= 0 || lheight <= 0) return;

#ifdef _USE_MMX
    __asm {
        mov         edi, lfbPtr
        mov         eax, lminY
        add         eax, pageOffset
        mul         lfbWidth
        add         eax, lminX
        shl         eax, 2
        add         edi, eax
        mov         edx, lfbWidth
        sub         edx, lwidth
        shl         edx, 2
        mov         esi, pattern
        movd        mm1, col
    plot:
        mov         ecx, lminX
        and         ecx, 7
        mov         ebx, lheight
        and         ebx, 7
        mov         al, [esi + ebx]
        rol         al, cl
        mov         ecx, lwidth
        shr         ecx, 1
        jz          once
        punpckldq   mm1, mm1
    next:
        test        al, 1
        jz          skip1
        test        al, 2
        jz          skip2
        movq        mm0, [edi]
        paddusb     mm0, mm1
        movq        [edi], mm0
        jmp         skip0
    skip2:
        movd        mm0, [edi + 4]
        paddusb     mm0, mm1
        movd        [edi + 4], mm0
        jmp         skip0
    skip1:
        test        al, 2
        jz          skip0
        movd        mm0, [edi]
        paddusb     mm0, mm1
        movd        [edi], mm0
    skip0:
        add         edi, 8
        rol         al, 2
        dec         ecx
        jnz         next
    once:
        test        lwidth, 1
        jz          end1
        test        al, 2
        jz          end0
        movd        mm0, [edi]
        paddusb     mm0, mm1
        movd        [edi], mm0
    end0:
        add         edi, 4
    end1:
        add         edi, edx
        dec         lheight
        jnz         plot
        emms
    }
#else
    __asm {
        mov         edi, lfbPtr
        mov         eax, lminY
        add         eax, pageOffset
        mul         lfbWidth
        add         eax, lminX
        shl         eax, 2
        add         edi, eax
        mov         edx, lfbWidth
        sub         edx, lwidth
        shl         edx, 2
        mov         esi, pattern
    plot:
        push        edx
        mov         ecx, lminX
        and         ecx, 7
        mov         ebx, lheight
        and         ebx, 7
        mov         al, [esi + ebx]
        rol         al, cl
        mov         ecx, lwidth
    next:
        test        al, 1
        jz          step
        mov         ebx, [edi]
        add         bl, byte ptr col
        jnc         bstep
        mov         bl, 255
    bstep:
        add         bh, byte ptr col[1]
        jnc         gstep
        mov         bh, 255
    gstep:
        mov         edx, ebx
        shr         edx, 16
        add         dl, byte ptr col[2]
        jnc         rstep
        mov         dl, 255
    rstep:
        shl         edx, 16
        and         ebx, 00FFFFh
        or          ebx, edx
        mov         [edi], ebx
    step:
        add         edi, 4
        rol         al, 1
        dec         ecx
        jnz         next
        pop         edx
        add         edi, edx
        dec         lheight
        jnz         plot
    }
#endif    
}

void fillRectPatternAdd24(int32_t x1, int32_t y1, int32_t width, int32_t height, uint32_t col, uint8_t *pattern)
{
    int32_t lwidth, lheight, x2, y2;
    int32_t lminX, lminY, lmaxX, lmaxY;
    
    // Calculate new position
    x2 = x1 + (width - 1);
    y2 = y1 + (height - 1);
    
    // Clip image to context boundaries
    lminX = max(x1, cminX);
    lminY = max(y1, cminY);
    lmaxX = min(x2, cmaxX);
    lmaxY = min(y2, cmaxY);

    // Initialize loop variables
    lwidth  = (lmaxX - lminX) + 1;
    lheight = (lmaxY - lminY) + 1;

    // Check range
    if (lwidth <= 0 || lheight <= 0) return;

    __asm {
        mov     edi, lfbPtr
        mov     eax, lminY
        add     eax, pageOffset
        mul     lfbWidth
        add     eax, lminX
        mov     ebx, eax
        shl     eax, 1
        add     eax, ebx
        add     edi, eax
        mov     edx, lfbWidth
        sub     edx, lwidth
        mov     eax, edx
        shl     edx, 1
        add     edx, eax
        mov     esi, pattern
    plot:
        push    edx
        mov     ecx, lminX
        and     ecx, 7
        mov     ebx, lheight
        and     ebx, 7
        mov     al, [esi + ebx]
        rol     al, cl
        mov     ecx, lwidth
    next:
        test    al, 1
        jz      step
        mov     bx, [edi]
        add     bl, byte ptr col
        jnc     bstep
        mov     bl, 255
    bstep:
        add     bh, byte ptr col[1]
        jnc     gstep
        mov     bh, 255
    gstep:
        mov     dl, [edi + 2]
        add     dl, byte ptr col[2]
        jnc     rstep
        mov     dl, 255
    rstep:
        mov     [edi], bx
        mov     [edi + 2], dl
    step:
        add     edi, 3
        rol     al, 1
        dec     ecx
        jnz     next
        pop     edx
        add     edi, edx
        dec     lheight
        jnz     plot
    }
}

void fillRectPatternAdd16(int32_t x1, int32_t y1, int32_t width, int32_t height, uint32_t col, uint8_t *pattern)
{
    int32_t lwidth, lheight, x2, y2;
    int32_t lminX, lminY, lmaxX, lmaxY;
    
    // Calculate new position
    x2 = x1 + (width - 1);
    y2 = y1 + (height - 1);

    // Clip image to context boundaries
    lminX = max(x1, cminX);
    lminY = max(y1, cminY);
    lmaxX = min(x2, cmaxX);
    lmaxY = min(y2, cmaxY);

    // Initialize loop variables
    lwidth  = (lmaxX - lminX) + 1;
    lheight = (lmaxY - lminY) + 1;

    // Check range
    if (lwidth <= 0 || lheight <= 0) return;

    __asm {
        mov     edi, lfbPtr
        mov     eax, lminY
        add     eax, pageOffset
        mul     lfbWidth
        add     eax, lminX
        shl     eax, 1
        add     edi, eax
        mov     ecx, lfbWidth
        sub     ecx, lwidth
        shl     ecx, 1
        mov     esi, pattern
        mov     bx, word ptr col
        mov     dx, bx
        and     bx, 1111100000011111b
        and     dx, 11111100000b
    plot:
        push    ecx
        push    lwidth
        mov     ecx, lheight
        and     ecx, 7
        mov     al, [esi + ecx]
        mov     ecx, lminX
        and     ecx, 7
        rol     al, cl
    next:
        test    al, 1
        jz      step
        push    eax
        mov     ax, [edi]
        mov     cx, ax
        and     ax, 1111100000011111b
        and     cx, 11111100000b
        add     al, bl
        cmp     al, 11111b
        jna     bstep
        mov     al, 11111b
    bstep:
        add     ah, bh
        jnc     gstep
        mov     ah, 11111000b
    gstep:
        add     cx, dx
        cmp     cx, 11111100000b
        jna     rstep
        mov     cx, 11111100000b
    rstep:
        or      ax, cx
        mov     [edi], ax
        pop     eax
    step:
        add     edi, 2
        rol     al, 1
        dec     lwidth
        jnz     next
        pop     lwidth
        pop     ecx
        add     edi, ecx
        dec     lheight
        jnz     plot
    }
}

void fillRectPatternAdd15(int32_t x1, int32_t y1, int32_t width, int32_t height, uint32_t col, uint8_t *pattern)
{
    int32_t lwidth, lheight, x2, y2;
    int32_t lminX, lminY, lmaxX, lmaxY;
    
    // Calculate new position
    x2 = x1 + (width - 1);
    y2 = y1 + (height - 1);

    // Clip image to context boundaries
    lminX = max(x1, cminX);
    lminY = max(y1, cminY);
    lmaxX = min(x2, cmaxX);
    lmaxY = min(y2, cmaxY);

    // Initialize loop variables
    lwidth  = (lmaxX - lminX) + 1;
    lheight = (lmaxY - lminY) + 1;

    // Check range
    if (lwidth <= 0 || lheight <= 0) return;

    __asm {
        mov     edi, lfbPtr
        mov     eax, lminY
        add     eax, pageOffset
        mul     lfbWidth
        add     eax, lminX
        shl     eax, 1
        add     edi, eax
        mov     ecx, lfbWidth
        sub     ecx, lwidth
        shl     ecx, 1
        mov     esi, pattern
        mov     bx, word ptr col
        mov     dx, bx
        and     bx, 111110000011111b
        and     dx, 1111100000b
    plot:
        push    ecx
        push    lwidth
        mov     ecx, lheight
        and     ecx, 7
        mov     al, [esi + ecx]
        mov     ecx, lminX
        and     ecx, 7
        rol     al, cl
    next:
        test    al, 1
        jz      step
        push    eax
        mov     ax, [edi]
        mov     cx, ax
        and     ax, 111110000011111b
        and     cx, 1111100000b
        add     al, bl
        cmp     al, 11111b
        jna     bstep
        mov     al, 11111b
    bstep:
        add     ah, bh
        cmp     ah, 1111100b
        jna     gstep
        mov     ah, 1111100b
    gstep:
        add     cx, dx
        cmp     cx, 1111100000b
        jna     rstep
        mov     cx, 1111100000b
    rstep:
        or      ax, cx
        mov     [edi], ax
        pop     eax
    step:
        add     edi, 2
        rol     al, 1
        dec     lwidth
        jnz     next
        pop     lwidth
        pop     ecx
        add     edi, ecx
        dec     lheight
        jnz     plot
    }
}

void fillRectPatternSub32(int32_t x1, int32_t y1, int32_t width, int32_t height, uint32_t col, uint8_t *pattern)
{
    int32_t lwidth, lheight, x2, y2;
    int32_t lminX, lminY, lmaxX, lmaxY;
    
    // Calculate new position
    x2 = x1 + (width - 1);
    y2 = y1 + (height - 1);
    
    // Clip image to context boundaries
    lminX = max(x1, cminX);
    lminY = max(y1, cminY);
    lmaxX = min(x2, cmaxX);
    lmaxY = min(y2, cmaxY);

    // Initialize loop variables
    lwidth  = (lmaxX - lminX) + 1;
    lheight = (lmaxY - lminY) + 1;

    // Check range
    if (lwidth <= 0 || lheight <= 0) return;

#ifdef _USE_MMX
    __asm {
        mov         edi, lfbPtr
        mov         eax, lminY
        add         eax, pageOffset
        mul         lfbWidth
        add         eax, lminX
        shl         eax, 2
        add         edi, eax
        mov         edx, lfbWidth
        sub         edx, lwidth
        shl         edx, 2
        mov         esi, pattern
        movd        mm1, col
    plot:
        mov         ecx, lminX
        and         ecx, 7
        mov         ebx, lheight
        and         ebx, 7
        mov         al, [esi + ebx]
        rol         al, cl
        mov         ecx, lwidth
        shr         ecx, 1
        jz          once
        punpckldq   mm1, mm1
    next:
        test        al, 1
        jz          skip1
        test        al, 2
        jz          skip2
        movq        mm0, [edi]
        psubusb     mm0, mm1
        movq        [edi], mm0
        jmp         skip0
    skip2:
        movd        mm0, [edi + 4]
        psubusb     mm0, mm1
        movd        [edi + 4], mm0
        jmp         skip0
    skip1:
        test        al, 2
        jz          skip0
        movd        mm0, [edi]
        psubusb     mm0, mm1
        movd        [edi], mm0
    skip0:
        add         edi, 8
        rol         al, 2
        dec         ecx
        jnz         next
    once:
        test        lwidth, 1
        jz          end1
        test        al, 2
        jz          end0
        movd        mm0, [edi]
        psubusb     mm0, mm1
        movd        [edi], mm0
    end0:
        add         edi, 4
    end1:
        add         edi, edx
        dec         lheight
        jnz         plot
        emms
    }
#else
    __asm {
        mov         edi, lfbPtr
        mov         eax, lminY
        add         eax, pageOffset
        mul         lfbWidth
        add         eax, lminX
        shl         eax, 2
        add         edi, eax
        mov         edx, lfbWidth
        sub         edx, lwidth
        shl         edx, 2
        mov         esi, pattern
    plot:
        push        edx
        mov         ecx, lminX
        and         ecx, 7
        mov         ebx, lheight
        and         ebx, 7
        mov         al, [esi + ebx]
        rol         al, cl
        mov         ecx, lwidth
    next:
        test        al, 1
        jz          step
        mov         ebx, [edi]
        sub         bl, byte ptr col
        jnc         bstep
        xor         bl, bl
    bstep:
        sub         bh, byte ptr col[1]
        jnc         gstep
        xor         bh, bh
    gstep:
        mov         edx, ebx
        shr         edx, 16
        sub         dl, byte ptr col[2]
        jnc         rstep
        xor         dl, dl
    rstep:
        shl         edx, 16
        and         ebx, 00FFFFh
        or          ebx, edx
        mov         [edi], ebx
    step:
        add         edi, 4
        rol         al, 1
        dec         ecx
        jnz         next
        pop         edx
        add         edi, edx
        dec         lheight
        jnz         plot
    }
#endif
}

void fillRectPatternSub24(int32_t x1, int32_t y1, int32_t width, int32_t height, uint32_t col, uint8_t *pattern)
{
    int32_t lwidth, lheight, x2, y2;
    int32_t lminX, lminY, lmaxX, lmaxY;
    
    // Calculate new position
    x2 = x1 + (width - 1);
    y2 = y1 + (height - 1);
    
    // Clip image to context boundaries
    lminX = max(x1, cminX);
    lminY = max(y1, cminY);
    lmaxX = min(x2, cmaxX);
    lmaxY = min(y2, cmaxY);

    // Initialize loop variables
    lwidth  = (lmaxX - lminX) + 1;
    lheight = (lmaxY - lminY) + 1;

    // Check range
    if (lwidth <= 0 || lheight <= 0) return;

    __asm {
        mov     edi, lfbPtr
        mov     eax, lminY
        add     eax, pageOffset
        mul     lfbWidth
        add     eax, lminX
        mov     ebx, eax
        shl     eax, 1
        add     eax, ebx
        add     edi, eax
        mov     edx, lfbWidth
        sub     edx, lwidth
        mov     eax, edx
        shl     edx, 1
        add     edx, eax
        sub     edx, lwidth
        mov     esi, pattern
    plot:
        push    edx
        mov     ecx, lminX
        and     ecx, 7
        mov     ebx, lheight
        and     ebx, 7
        mov     al, [esi + ebx]
        rol     al, cl
        mov     ecx, lwidth
    next:
        test    al, 1
        jz      step
        mov     bx, [edi]
        sub     bl, byte ptr col
        jnc     bstep
        xor     bl, bl
    bstep:
        sub     bh, byte ptr col[1]
        jnc     gstep
        xor     bh, bh
    gstep:
        mov     dl, [edi + 2]
        sub     dl, byte ptr col[2]
        jnc     rstep
        xor     dl, dl
    rstep:
        mov     [edi], bx
        mov     [edi + 2], dl
    step:
        add     edi, 3
        rol     al, 1
        dec     ecx
        jnz     next
        pop     edx
        add     edi, edx
        dec     lheight
        jnz     plot
    }
}

void fillRectPatternSub16(int32_t x1, int32_t y1, int32_t width, int32_t height, uint32_t col, uint8_t *pattern)
{
    int32_t lwidth, lheight, x2, y2;
    int32_t lminX, lminY, lmaxX, lmaxY;
    
    // Calculate new position
    x2 = x1 + (width - 1);
    y2 = y1 + (height - 1);

    // Clip image to context boundaries
    lminX = max(x1, cminX);
    lminY = max(y1, cminY);
    lmaxX = min(x2, cmaxX);
    lmaxY = min(y2, cmaxY);

    // Initialize loop variables
    lwidth  = (lmaxX - lminX) + 1;
    lheight = (lmaxY - lminY) + 1;

    // Check range
    if (lwidth <= 0 || lheight <= 0) return;

    __asm {
        mov     edi, lfbPtr
        mov     eax, lminY
        add     eax, pageOffset
        mul     lfbWidth
        add     eax, lminX
        shl     eax, 1
        add     edi, eax
        mov     ecx, lfbWidth
        sub     ecx, lwidth
        shl     ecx, 1
        mov     esi, pattern
        mov     bx, word ptr col
        mov     dx, bx
        and     bx, 1111100000011111b
        and     dx, 11111100000b
    plot:
        push    ecx
        push    lwidth
        mov     ecx, lheight
        and     ecx, 7
        mov     al, [esi + ecx]
        mov     ecx, lminX
        and     ecx, 7
        rol     al, cl
    next:
        test    al, 1
        jz      step
        push    eax
        mov     ax, [edi]
        mov     cx, ax
        and     ax, 1111100000011111b
        and     cx, 11111100000b
        sub     al, bl
        jnc     bstep
        xor     al, al
    bstep:
        sub     ah, bh
        jnc     gstep
        xor     ah, ah
    gstep:
        sub     cx, dx
        jc      rstep
        or      ax, cx
    rstep:
        mov     [edi], ax
        pop     eax
    step:
        add     edi, 2
        rol     al, 1
        dec     lwidth
        jnz     next
        pop     lwidth
        pop     ecx
        add     edi, ecx
        dec     lheight
        jnz     plot
    }
}

void fillRectPatternSub15(int32_t x1, int32_t y1, int32_t width, int32_t height, uint32_t col, uint8_t *pattern)
{
    int32_t lwidth, lheight, x2, y2;
    int32_t lminX, lminY, lmaxX, lmaxY;
    
    // Calculate new position
    x2 = x1 + (width - 1);
    y2 = y1 + (height - 1);

    // Clip image to context boundaries
    lminX = max(x1, cminX);
    lminY = max(y1, cminY);
    lmaxX = min(x2, cmaxX);
    lmaxY = min(y2, cmaxY);

    // Initialize loop variables
    lwidth  = (lmaxX - lminX) + 1;
    lheight = (lmaxY - lminY) + 1;

    // Check range
    if (lwidth <= 0 || lheight <= 0) return;

    __asm {
        mov     edi, lfbPtr
        mov     eax, lminY
        add     eax, pageOffset
        mul     lfbWidth
        add     eax, lminX
        shl     eax, 1
        add     edi, eax
        mov     ecx, lfbWidth
        sub     ecx, lwidth
        shl     ecx, 1
        mov     esi, pattern
        mov     bx, word ptr col
        mov     dx, bx
        and     bx, 111110000011111b
        and     dx, 1111100000b
    plot:
        push    ecx
        push    lwidth
        mov     ecx, lheight
        and     ecx, 7
        mov     al, [esi + ecx]
        mov     ecx, lminX
        and     ecx, 7
        rol     al, cl
    next:
        test    al, 1
        jz      step
        push    eax
        mov     ax, [edi]
        mov     cx, ax
        and     ax, 111110000011111b
        and     cx, 1111100000b
        sub     al, bl
        jnc     bstep
        xor     al, al
    bstep:
        sub     ah, bh
        jnc     gstep
        xor     ah, ah
    gstep:
        sub     cx, dx
        jc      rstep
        or      ax, cx
    rstep:
        mov     [edi], ax
        pop     eax
    step:
        add     edi, 2
        rol     al, 1
        dec     lwidth
        jnz     next
        pop     lwidth
        pop     ecx
        add     edi, ecx
        dec     lheight
        jnz     plot
    }
}

// Create a new image
int32_t newImage(int32_t width, int32_t height, GFX_IMAGE *img)
{
    // Calcule buffer size, add to width and height
    uint32_t lineBytes = width * bytesPerPixel;
    uint32_t size = height * lineBytes;
    if (!size) return 0;

    img->mData = (uint8_t*)calloc(size, 1);
    if (!img->mData) return 0;

    // store image width and height
    memset(img->mData, 0, size);
    img->mWidth    = width;
    img->mHeight   = height;
    img->mPixels   = bytesPerPixel;
    img->mSize     = size;
    img->mRowBytes = lineBytes;

    return 1;
}

// Cleanup image buffer
void freeImage(GFX_IMAGE *img)
{
    if (img && img->mData)
    {
        free(img->mData);
        img->mData     = NULL;
        img->mWidth    = 0;
        img->mHeight   = 0;
        img->mPixels   = 0;
        img->mSize     = 0;
        img->mRowBytes = 0;
    }
}

// Clear image data buffer
void clearImage(GFX_IMAGE *img)
{
    void *data = img->mData;
    uint32_t size = img->mSize;
    if (!size) return;

#ifdef _USE_MMX
    __asm {
        pxor    mm0, mm0
        mov     edi, data
        mov     ecx, size
        shr     ecx, 3
        jz      skip
    move:
        movq    [edi], mm0
        add     edi, 8
        dec     ecx
        jnz     move
    skip:
        mov     ecx, size
        and     ecx, 7
        rep     movsb
        emms
    }
#else
    __asm {
        mov     edi, data
        xor     eax, eax
        mov     ecx, size
        shr     ecx, 2
        rep     stosd
        mov     ecx, size
        and     ecx, 3
        rep     stosb
    }
#endif    
}

// Copy full current page to another page (non-clipping)
void copyPage(int32_t from, int32_t to)
{
    GFX_IMAGE img;
    int32_t oldPage = activePage;
    if (!newImage(lfbWidth, lfbHeight, &img)) fatalError("copyPage: cannot open image.\n");
    setActivePage(from);
    getImage(0, 0, lfbWidth, lfbHeight, &img);
    setActivePage(to);
    putImage(0, 0, &img);
    setActivePage(oldPage);
    freeImage(&img);
}

// Get image buffer functions
void getImage8(int32_t x1, int32_t y1, int32_t width, int32_t height, GFX_IMAGE *img)
{
    void *imgData = img->mData;
    int32_t x2, y2, lwidth, lheight;
    int32_t lminX, lminY, lmaxX, lmaxY;

    // Calculate new position
    x2 = x1 + (width - 1);
    y2 = y1 + (height - 1);

    // Clip image to context boundaries
    lminX = max(x1, cminX);
    lminY = max(y1, cminY);
    lmaxX = min(x2, cmaxX);
    lmaxY = min(y2, cmaxY);

    // Initialize loop variables
    lwidth  = (lmaxX - lminX) + 1;
    lheight = (lmaxY - lminY) + 1;

    // Check range
    if (lwidth <= 0 || lheight <= 0) return;

    // store real image contents
    img->mWidth    = lwidth;
    img->mHeight   = lheight;
    img->mPixels   = bytesPerPixel;
    img->mRowBytes = lwidth * bytesPerPixel;
    img->mSize     = lheight * img->mRowBytes;

#ifdef _USE_MMX
    __asm {
        mov     edi, imgData
        mov     esi, lfbPtr
        mov     eax, lminY
        add     eax, pageOffset
        mul     lfbWidth
        add     eax, lminX
        add     esi, eax
        mov     edx, lfbWidth
        sub     edx, lwidth
        mov     eax, lwidth
        shr     eax, 3
        mov     ebx, lwidth
        and     ebx, 7
    next:
        test    eax, eax
        jz      once
        mov     ecx, eax
    plot:
        movq    mm0, [esi]
        movq    [edi], mm0
        add     edi, 8
        add     esi, 8
        dec     ecx
        jnz     plot
    once:
        mov     ecx, ebx
        rep     movsb
    end:
        add     esi, edx
        dec     lheight
        jnz     next
        emms
    }
#else
    __asm {
        mov     edi, imgData
        mov     esi, lfbPtr
        mov     eax, lminY
        add     eax, pageOffset
        mul     lfbWidth
        add     eax, lminX
        add     esi, eax
        mov     edx, lfbWidth
        sub     edx, lwidth
        mov     eax, lwidth
        shr     eax, 2
        mov     ebx, lwidth
        and     ebx, 3
    next:
        mov     ecx, eax
        rep     movsd
        mov     ecx, ebx
        rep     movsb
        add     esi, edx
        dec     lheight
        jnz     next
    }
#endif    
}

void getImage1516(int32_t x1, int32_t y1, int32_t width, int32_t height, GFX_IMAGE *img)
{
    void *imgData = img->mData;
    int32_t x2, y2, lwidth, lheight;
    int32_t lminX, lminY, lmaxX, lmaxY;

    // Calculate new position
    x2 = x1 + (width - 1);
    y2 = y1 + (height - 1);

    // Clip image to context boundaries
    lminX = max(x1, cminX);
    lminY = max(y1, cminY);
    lmaxX = min(x2, cmaxX);
    lmaxY = min(y2, cmaxY);

    // Initialize loop variables
    lwidth  = (lmaxX - lminX) + 1;
    lheight = (lmaxY - lminY) + 1;

    // Check range
    if (lwidth <= 0 || lheight <= 0) return;

    // Store real image contents
    img->mWidth    = lwidth;
    img->mHeight   = lheight;
    img->mPixels   = bytesPerPixel;
    img->mRowBytes = lwidth * bytesPerPixel;
    img->mSize     = lheight * img->mRowBytes;

#ifdef _USE_MMX
    __asm {
        mov     edi, imgData
        mov     esi, lfbPtr
        mov     eax, lminY
        add     eax, pageOffset
        mul     lfbWidth
        add     eax, lminX
        shl     eax, 1
        add     esi, eax
        mov     edx, lfbWidth
        sub     edx, lwidth
        shl     edx, 1
        mov     eax, lwidth
        shr     eax, 2
        mov     ebx, lwidth
        and     ebx, 3
    next:
        test    eax, eax
        jz      once
        mov     ecx, eax
    plot:
        movq    mm0, [esi]
        movq    [edi], mm0
        add     edi, 8
        add     esi, 8
        dec     ecx
        jnz     plot
    once:
        mov     ecx, ebx
        rep     movsw
    end:
        add     esi, edx
        dec     lheight
        jnz     next
        emms
    }
#else
    __asm {
        mov     edi, imgData
        mov     esi, lfbPtr
        mov     eax, lminY
        add     eax, pageOffset
        mul     lfbWidth
        add     eax, lminX
        shl     eax, 1
        add     esi, eax
        mov     edx, lfbWidth
        sub     edx, lwidth
        shl     edx, 1
        mov     eax, lwidth
        shr     eax, 1
        mov     ebx, lwidth
        and     ebx, 1
    next:
        mov     ecx, eax
        rep     movsd
        mov     ecx, ebx
        rep     movsw
        add     esi, edx
        dec     lheight
        jnz     next
    }
#endif
}

void getImage24(int32_t x1, int32_t y1, int32_t width, int32_t height, GFX_IMAGE *img)
{
    void *imgData = img->mData;
    int32_t x2, y2, lwidth, lheight;
    int32_t lminX, lminY, lmaxX, lmaxY;

    // Calculate new position
    x2 = x1 + (width - 1);
    y2 = y1 + (height - 1);

    // Clip image to context boundaries
    lminX = max(x1, cminX);
    lminY = max(y1, cminY);
    lmaxX = min(x2, cmaxX);
    lmaxY = min(y2, cmaxY);

    // Initialize loop variables
    lwidth  = (lmaxX - lminX) + 1;
    lheight = (lmaxY - lminY) + 1;

    // Check range
    if (lwidth <= 0 || lheight <= 0) return;

    // Store real image contents
    img->mWidth    = lwidth;
    img->mHeight   = lheight;
    img->mPixels   = bytesPerPixel;
    img->mRowBytes = lwidth * bytesPerPixel;
    img->mSize     = lheight * img->mRowBytes;

    __asm {
        mov     edi, imgData
        mov     esi, lfbPtr
        mov     eax, lminY
        add     eax, pageOffset
        mul     lfbWidth
        add     eax, lminX
        mov     ebx, eax
        shl     eax, 1
        add     eax, ebx
        add     esi, eax
        mov     edx, lfbWidth
        sub     edx, lwidth
        mov     eax, edx
        shl     edx, 1
        add     edx, eax
        mov     ecx, lwidth
        lea     ecx, [ecx + ecx * 2]
        mov     eax, ecx
        shr     eax, 2
        and     ecx, 3
        mov     ebx, ecx
    next:
        mov     ecx, eax
        rep     movsd
        mov     ecx, ebx
        rep     movsb
        add     esi, edx
        dec     lheight
        jnz     next
    }
}

void getImage32(int32_t x1, int32_t y1, int32_t width, int32_t height, GFX_IMAGE *img)
{
    void *imgData = img->mData;
    int32_t x2, y2, lwidth, lheight;
    int32_t lminX, lminY, lmaxX, lmaxY;

    // Calculate new position
    x2 = x1 + (width - 1);
    y2 = y1 + (height - 1);

    // Clip image to context boundaries
    lminX = max(x1, cminX);
    lminY = max(y1, cminY);
    lmaxX = min(x2, cmaxX);
    lmaxY = min(y2, cmaxY);

    // Initialize loop variables
    lwidth  = (lmaxX - lminX) + 1;
    lheight = (lmaxY - lminY) + 1;

    // Check range
    if (lwidth <= 0 || lheight <= 0) return;

    // Store real image contents
    img->mWidth    = lwidth;
    img->mHeight   = lheight;
    img->mPixels   = bytesPerPixel;
    img->mRowBytes = lwidth * bytesPerPixel;
    img->mSize     = lheight * img->mRowBytes;

#ifdef _USE_MMX
    __asm {
        mov     edi, imgData
        mov     esi, lfbPtr
        mov     eax, lminY
        add     eax, pageOffset
        mul     lfbWidth
        add     eax, lminX
        shl     eax, 2
        add     esi, eax
        mov     edx, lfbWidth
        sub     edx, lwidth
        shl     edx, 2
        mov     ebx, lwidth
        shr     ebx, 1
    next:
        test    ebx, ebx
        jz      once
        mov     ecx, ebx
    plot:
        movq    mm0, [esi]
        movq    [edi], mm0
        add     edi, 8
        add     esi, 8
        dec     ecx
        jnz     plot
    once:
        test    lwidth, 1
        jz      end
        movd    mm0, [esi]
        movd    [edi], mm0
        add     edi, 4
        add     esi, 4
    end:
        add     esi, edx
        dec     lheight
        jnz     next
        emms
    }
#else
    __asm {
        mov     edi, imgData
        mov     esi, lfbPtr
        mov     eax, lminY
        add     eax, pageOffset
        mul     lfbWidth
        add     eax, lminX
        shl     eax, 2
        add     esi, eax
        mov     edx, lfbWidth
        sub     edx, lwidth
        shl     edx, 2
    next:
        mov     ecx, lwidth
        rep     movsd
        add     esi, edx
        dec     lheight
        jnz     next
    }
#endif    
}

// Put image data to screen functions
void putImage8(int32_t x1, int32_t y1, GFX_IMAGE *img)
{
    int32_t x2, y2, lwidth, lheight;
    int32_t lminX, lminY, lmaxX, lmaxY;
    
    void *imgData = img->mData;
    int32_t imgWidth  = img->mWidth;
    int32_t imgHeight = img->mHeight;
    
    // Calculate new position
    x2 = x1 + (imgWidth - 1);
    y2 = y1 + (imgHeight - 1);

    // Clip image to context boundaries
    lminX = max(x1, cminX);
    lminY = max(y1, cminY);
    lmaxX = min(x2, cmaxX);
    lmaxY = min(y2, cmaxY);

    // Initialize loop variables
    lwidth  = (lmaxX - lminX) + 1;
    lheight = (lmaxY - lminY) + 1;

    // Check range
    if (lwidth <= 0 || lheight <= 0) return;

#ifdef _USE_MMX
    __asm {
        mov     edi, lfbPtr
        mov     eax, lminY
        add     eax, pageOffset
        mul     lfbWidth
        add     eax, lminX
        add     edi, eax
        mov     esi, imgData
        mov     eax, lminY
        sub     eax, y1
        mul     imgWidth
        mov     ebx, lminX
        sub     ebx, x1
        add     eax, ebx
        add     esi, eax
        mov     ebx, lfbWidth
        sub     ebx, lwidth
        push    ebx
        mov     edx, imgWidth
        sub     edx, lwidth
        mov     eax, lwidth
        shr     eax, 3
        mov     ebx, lwidth
        and     ebx, 7
    next:
        test    eax, eax
        jz      once
        mov     ecx, eax
    plot:
        movq    mm0, [esi]
        movq    [edi], mm0
        add     edi, 8
        add     esi, 8
        dec     ecx
        jnz     plot
    once:
        mov     ecx, ebx
        rep     movsb
    end:
        add     edi, [esp]
        add     esi, edx
        dec     lheight
        jnz     next
        pop     ebx
        emms
    }
#else
    __asm {
        mov     edi, lfbPtr
        mov     eax, lminY
        add     eax, pageOffset
        mul     lfbWidth
        add     eax, lminX
        add     edi, eax
        mov     esi, imgData
        mov     eax, lminY
        sub     eax, y1
        mul     imgWidth
        mov     ebx, lminX
        sub     ebx, x1
        add     eax, ebx
        add     esi, eax
        mov     ebx, lfbWidth
        sub     ebx, lwidth
        push    ebx
        mov     edx, imgWidth
        sub     edx, lwidth
        mov     eax, lwidth
        shr     eax, 2
        mov     ebx, lwidth
        and     ebx, 3
    next:
        mov     ecx, eax
        rep     movsd
        mov     ecx, ebx
        rep     movsb
        add     edi, [esp]
        add     esi, edx
        dec     lheight
        jnz     next
        pop     ebx
    }
#endif
}

void putImage1516(int32_t x1, int32_t y1, GFX_IMAGE *img)
{
    int32_t x2, y2, lwidth, lheight;
    int32_t lminX, lminY, lmaxX, lmaxY;
    
    void *imgData = img->mData;
    int32_t imgWidth  = img->mWidth;
    int32_t imgHeight = img->mHeight;
    
    // Calculate new position
    x2 = x1 + (imgWidth - 1);
    y2 = y1 + (imgHeight - 1);

    // Clip image to context boundaries
    lminX = max(x1, cminX);
    lminY = max(y1, cminY);
    lmaxX = min(x2, cmaxX);
    lmaxY = min(y2, cmaxY);

    // Initialize loop variables
    lwidth  = (lmaxX - lminX) + 1;
    lheight = (lmaxY - lminY) + 1;

    // Check range
    if (lwidth <= 0 || lheight <= 0) return;

#ifdef _USE_MMX
    __asm {
        mov     edi, lfbPtr
        mov     eax, lminY
        add     eax, pageOffset
        mul     lfbWidth
        add     eax, lminX
        shl     eax, 1
        add     edi, eax
        mov     esi, imgData
        mov     eax, lminY
        sub     eax, y1
        mul     imgWidth
        mov     ebx, lminX
        sub     ebx, x1
        add     eax, ebx
        shl     eax, 1
        add     esi, eax
        mov     ebx, lfbWidth
        sub     ebx, lwidth
        shl     ebx, 1
        push    ebx
        mov     edx, imgWidth
        sub     edx, lwidth
        shl     edx, 1
        mov     eax, lwidth
        shr     eax, 2
        mov     ebx, lwidth
        and     ebx, 3
    next:
        test    eax, eax
        jz      once
        mov     ecx, eax
    plot:
        movq    mm0, [esi]
        movq    [edi], mm0
        add     edi, 8
        add     esi, 8
        dec     ecx
        jnz     plot
    once:
        mov     ecx, ebx
        rep     movsw
    end:
        add     edi, [esp]
        add     esi, edx
        dec     lheight
        jnz     next
        pop     ebx
        emms
    }
#else
    __asm {
        mov     edi, lfbPtr
        mov     eax, lminY
        add     eax, pageOffset
        mul     lfbWidth
        add     eax, lminX
        shl     eax, 1
        add     edi, eax
        mov     esi, imgData
        mov     eax, lminY
        sub     eax, y1
        mul     imgWidth
        mov     ebx, lminX
        sub     ebx, x1
        add     eax, ebx
        shl     eax, 1
        add     esi, eax
        mov     ebx, lfbWidth
        sub     ebx, lwidth
        shl     ebx, 1
        push    ebx
        mov     edx, imgWidth
        sub     edx, lwidth
        shl     edx, 1
        mov     eax, lwidth
        shr     eax, 1
        mov     ebx, lwidth
        and     ebx, 1
    next:
        mov     ecx, eax
        rep     movsd
        mov     ecx, ebx
        rep     movsw
        add     edi, [esp]
        add     esi, edx
        dec     lheight
        jnz     next
        pop     ebx
    }
#endif
}

void putImage24(int32_t x1, int32_t y1, GFX_IMAGE *img)
{
    int32_t x2, y2, lwidth, lheight;
    int32_t lminX, lminY, lmaxX, lmaxY;
    
    void *imgData = img->mData;
    int32_t imgWidth  = img->mWidth;
    int32_t imgHeight = img->mHeight;
    
    // Calculate new position
    x2 = x1 + (imgWidth - 1);
    y2 = y1 + (imgHeight - 1);

    // Clip image to context boundaries
    lminX = max(x1, cminX);
    lminY = max(y1, cminY);
    lmaxX = min(x2, cmaxX);
    lmaxY = min(y2, cmaxY);

    // Initialize loop variables
    lwidth  = (lmaxX - lminX) + 1;
    lheight = (lmaxY - lminY) + 1;

    // Check range
    if (lwidth <= 0 || lheight <= 0) return;

    __asm {
        mov     edi, lfbPtr
        mov     eax, lminY
        add     eax, pageOffset
        mul     lfbWidth
        add     eax, lminX
        mov     ebx, eax
        shl     eax, 1
        add     eax, ebx
        add     edi, eax
        mov     esi, imgData
        mov     eax, lminY
        sub     eax, y1
        mul     imgWidth
        mov     ebx, lminX
        sub     ebx, x1
        add     eax, ebx
        mov     ebx, eax
        shl     eax, 1
        add     eax, ebx
        add     esi, eax
        mov     ebx, lfbWidth
        sub     ebx, lwidth
        mov     eax, ebx
        shl     ebx, 1
        add     ebx, eax
        push    ebx
        mov     edx, imgWidth
        sub     edx, lwidth
        mov     eax, edx
        shl     edx, 1
        add     edx, eax
        mov     ecx, lwidth
        lea     ecx, [ecx + ecx * 2]
        mov     eax, ecx
        shr     eax, 2
        and     ecx, 3
        mov     ebx, ecx
    next:
        mov     ecx, eax
        rep     movsd
        mov     ecx, ebx
        rep     movsb
        add     edi, [esp]
        add     esi, edx
        dec     lheight
        jnz     next
        pop     ebx
    }
}

void putImage32(int32_t x1, int32_t y1, GFX_IMAGE *img)
{
    int32_t x2, y2, lwidth, lheight;
    int32_t lminX, lminY, lmaxX, lmaxY;
    
    void *imgData = img->mData;
    int32_t imgWidth  = img->mWidth;
    int32_t imgHeight = img->mHeight;

    // Calculate new position
    x2 = x1 + (imgWidth - 1);
    y2 = y1 + (imgHeight - 1);

    // Clip image to context boundaries
    lminX = max(x1, cminX);
    lminY = max(y1, cminY);
    lmaxX = min(x2, cmaxX);
    lmaxY = min(y2, cmaxY);

    // Initialize loop variables
    lwidth  = (lmaxX - lminX) + 1;
    lheight = (lmaxY - lminY) + 1;

    // Check range
    if (lwidth <= 0 || lheight <= 0) return;

#ifdef _USE_MMX
    __asm {
        mov     edi, lfbPtr
        mov     eax, lminY
        add     eax, pageOffset
        mul     lfbWidth
        add     eax, lminX
        shl     eax, 2
        add     edi, eax
        mov     esi, imgData
        mov     eax, lminY
        sub     eax, y1
        mul     imgWidth
        mov     ebx, lminX
        sub     ebx, x1
        add     eax, ebx
        shl     eax, 2
        add     esi, eax
        mov     ebx, lfbWidth
        sub     ebx, lwidth
        shl     ebx, 2
        mov     edx, imgWidth
        sub     edx, lwidth
        shl     edx, 2
    next:
        mov     ecx, lwidth
        shr     ecx, 1
        jz      once
    plot:
        movq    mm0, [esi]
        movq    [edi], mm0
        add     edi, 8
        add     esi, 8
        dec     ecx
        jnz     plot
    once:
        test    lwidth, 1
        jz      end
        movd    mm0, [esi]
        movd    [edi], mm0
        add     edi, 4
        add     esi, 4
    end:
        add     edi, ebx
        add     esi, edx
        dec     lheight
        jnz     next
        emms
    }
#else
    __asm {
        mov     edi, lfbPtr
        mov     eax, lminY
        add     eax, pageOffset
        mul     lfbWidth
        add     eax, lminX
        shl     eax, 2
        add     edi, eax
        mov     esi, imgData
        mov     eax, lminY
        sub     eax, y1
        mul     imgWidth
        mov     ebx, lminX
        sub     ebx, x1
        add     eax, ebx
        shl     eax, 2
        add     esi, eax
        mov     ebx, lfbWidth
        sub     ebx, lwidth
        shl     ebx, 2
        mov     edx, imgWidth
        sub     edx, lwidth
        shl     edx, 2
    next:
        mov     ecx, lwidth
        rep     movsd
        add     edi, ebx
        add     esi, edx
        dec     lheight
        jnz     next
    }
#endif
}

// Put 32 bits transparent image
void putImageAlpha(int32_t x1, int32_t y1, GFX_IMAGE *img)
{
    int32_t x2, y2, lwidth, lheight;
    int32_t lminX, lminY, lmaxX, lmaxY;
    
    void *imgData = img->mData;
    int32_t imgWidth  = img->mWidth;
    int32_t imgHeight = img->mHeight;
    
    // Check for 32bit support
    if (bytesPerPixel != 4) return;
    if (img->mPixels != 4) return;

    // Calculate new position
    x2 = x1 + (imgWidth - 1);
    y2 = y1 + (imgHeight - 1);

    // Clip image to context boundaries
    lminX = max(x1, cminX);
    lminY = max(y1, cminY);
    lmaxX = min(x2, cmaxX);
    lmaxY = min(y2, cmaxY);

    // Initialize loop variables
    lwidth  = (lmaxX - lminX) + 1;
    lheight = (lmaxY - lminY) + 1;

    // Check range
    if (lwidth <= 0 || lheight <= 0) return;

#ifdef _USE_MMX
    __asm {
        mov         edi, lfbPtr
        mov         eax, lminY
        add         eax, pageOffset
        mul         lfbWidth
        add         eax, lminX
        shl         eax, 2
        add         edi, eax
        mov         esi, imgData
        mov         eax, lminY
        sub         eax, y1
        mul         imgWidth
        mov         ebx, lminX
        sub         ebx, x1
        add         eax, ebx
        shl         eax, 2
        add         esi, eax
        mov         ebx, lfbWidth
        sub         ebx, lwidth
        shl         ebx, 2
        mov         edx, imgWidth
        sub         edx, lwidth
        shl         edx, 2
    next:
        xor         eax, eax
        pxor        mm2, mm2
        mov         ecx, lwidth
    plot:
        mov         al, [esi + 3]
        movd        mm3, eax
        punpcklwd   mm3, mm3
        punpckldq   mm3, mm3
        movd        mm0, [esi]
        movd        mm1, [edi]
        punpcklbw   mm0, mm2
        movq        mm4, mm1
        punpcklbw   mm1, mm2
        psubw       mm0, mm1
        pmullw      mm0, mm3
        psrlw       mm0, 8
        packuswb    mm0, mm2
        paddb       mm0, mm4
        movd        [edi], mm0
        add         edi, 4
        add         esi, 4
        dec         ecx
        jnz         plot
        add         edi, ebx
        add         esi, edx
        dec         lheight
        jnz         next
        emms    
    }
#else
    __asm {
        mov         edi, lfbPtr
        mov         eax, lminY
        add         eax, pageOffset
        mul         lfbWidth
        add         eax, lminX
        shl         eax, 2
        add         edi, eax
        mov         esi, imgData
        mov         eax, lminY
        sub         eax, y1
        mul         imgWidth
        mov         ebx, lminX
        sub         ebx, x1
        add         eax, ebx
        shl         eax, 2
        add         esi, eax
        mov         ebx, lfbWidth
        sub         ebx, lwidth
        shl         ebx, 2
        mov         edx, imgWidth
        sub         edx, lwidth
        shl         edx, 2
    next:
        push        ebx
        push        edx
        mov         ecx, lwidth
    plot:
        mov         al, [esi]
        mul         byte ptr[esi + 3]
        mov         bx, ax
        mov         al, [edi]
        mov         dl, 255
        sub         dl, [esi + 3]
        mul         dl
        add         ax, bx
        shr         ax, 8
        stosb
        mov         al, [esi + 1]
        mul         byte ptr[esi + 3]
        mov         bx, ax
        mov         al, [edi]
        mov         dl, 255
        sub         dl, [esi + 3]
        mul         dl
        add         ax, bx
        shr         ax, 8
        stosb
        mov         al, [esi + 2]
        mul         byte ptr[esi + 3]
        mov         bx, ax
        mov         al, [edi]
        mov         dl, 255
        sub         dl, [esi + 3]
        mul         dl
        add         ax, bx
        shr         ax, 8
        stosb
        inc         edi
        add         esi, 4
        dec         ecx
        jnz         plot
        pop         edx
        pop         ebx
        add         edi, ebx
        add         esi, edx
        dec         lheight
        jnz         next
    }
#endif    
}

void putImageAdd32(int32_t x1, int32_t y1, GFX_IMAGE *img)
{
    int32_t x2, y2, lwidth, lheight;
    int32_t lminX, lminY, lmaxX, lmaxY;
    
    void *imgData = img->mData;
    int32_t imgWidth  = img->mWidth;
    int32_t imgHeight = img->mHeight;
    
    // Check compatible pixel mode
    if (img->mPixels != bytesPerPixel) return;

    // Calculate new position
    x2 = x1 + (imgWidth - 1);
    y2 = y1 + (imgHeight - 1);

    // Clip image to context boundaries
    lminX = max(x1, cminX);
    lminY = max(y1, cminY);
    lmaxX = min(x2, cmaxX);
    lmaxY = min(y2, cmaxY);

    // Initialize loop variables
    lwidth  = (lmaxX - lminX) + 1;
    lheight = (lmaxY - lminY) + 1;

    // Check range
    if (lwidth <= 0 || lheight <= 0) return;

#ifdef _USE_MMX
    __asm {
        mov     edi, lfbPtr
        mov     eax, lminY
        add     eax, pageOffset
        mul     lfbWidth
        add     eax, lminX
        shl     eax, 2
        add     edi, eax
        mov     esi, imgData
        mov     eax, lminY
        sub     eax, y1
        mul     imgWidth
        mov     ebx, lminX
        sub     ebx, x1
        add     eax, ebx
        shl     eax, 2
        add     esi, eax
        mov     ebx, lfbWidth
        sub     ebx, lwidth
        shl     ebx, 2
        mov     edx, imgWidth
        sub     edx, lwidth
        shl     edx, 2
    next:
        mov     ecx, lwidth
        shr     ecx, 1
        jz      once
    move:
        movq    mm0, [esi]
        paddusb mm0, [edi]
        movq    [edi], mm0
        add     edi, 8
        add     esi, 8
        dec     ecx
        jnz     move
    once:
        test    lwidth, 1
        jz      skip
        movd    mm0, [esi]
        paddusb mm0, [edi]
        movd    [edi], mm0
        add     edi, 4
        add     esi, 4
    skip:
        add     edi, ebx
        add     esi, edx
        dec     lheight
        jnz     next
        emms
    }
#else
    __asm {
        mov     edi, lfbPtr
        mov     eax, lminY
        add     eax, pageOffset
        mul     lfbWidth
        add     eax, lminX
        shl     eax, 2
        add     edi, eax
        mov     esi, imgData
        mov     eax, lminY
        sub     eax, y1
        mul     imgWidth
        mov     ebx, lminX
        sub     ebx, x1
        add     eax, ebx
        shl     eax, 2
        add     esi, eax
        mov     ebx, lfbWidth
        sub     ebx, lwidth
        shl     ebx, 2
        mov     edx, imgWidth
        sub     edx, lwidth
        shl     edx, 2
    next:
        push    ebx
        mov     ecx, lwidth
    plot:
        lodsd
        add     al, [edi]
        jnc     bstep
        mov     al, 255
    bstep:
        add     ah, [edi + 1]
        jnc     gstep
        mov     ah, 255
    gstep:
        mov     ebx, eax
        shr     ebx, 16
        add     bl, [edi + 2]
        jnc     rstep
        mov     bl, 255
    rstep:
        shl     ebx, 16
        and     eax, 00FFFFh
        or      eax, ebx
        stosd
        dec     ecx
        jnz     plot
        pop     ebx
        add     edi, ebx
        add     esi, edx
        dec     lheight
        jnz     next
    }
#endif
}

void putImageAdd24(int32_t x1, int32_t y1, GFX_IMAGE *img)
{
    int32_t x2, y2, lwidth, lheight;
    int32_t lminX, lminY, lmaxX, lmaxY;
    
    void *imgData = img->mData;
    int32_t imgWidth  = img->mWidth;
    int32_t imgHeight = img->mHeight;
    
    // Check compatible pixel mode
    if (img->mPixels != bytesPerPixel) return;

    // Calculate new position
    x2 = x1 + (imgWidth - 1);
    y2 = y1 + (imgHeight - 1);

    // Clip image to context boundaries
    lminX = max(x1, cminX);
    lminY = max(y1, cminY);
    lmaxX = min(x2, cmaxX);
    lmaxY = min(y2, cmaxY);

    // Initialize loop variables
    lwidth  = (lmaxX - lminX) + 1;
    lheight = (lmaxY - lminY) + 1;

    // Check range
    if (lwidth <= 0 || lheight <= 0) return;

    __asm {
        mov     edi, lfbPtr
        mov     eax, lminY
        add     eax, pageOffset
        mul     lfbWidth
        add     eax, lminX
        mov     ebx, eax
        shl     eax, 1
        add     eax, ebx
        add     edi, eax
        mov     esi, imgData
        mov     eax, lminY
        sub     eax, y1
        mul     imgWidth
        mov     ebx, lminX
        sub     ebx, x1
        add     eax, ebx
        mov     ebx, eax
        shl     eax, 1
        add     eax, ebx
        add     esi, eax
        mov     ebx, lfbWidth
        sub     ebx, lwidth
        mov     eax, ebx
        shl     ebx, 1
        add     ebx, eax
        mov     edx, imgWidth
        sub     edx, lwidth
        mov     ecx, edx
        shl     edx, 1
        add     edx, ecx
    next:
        push    ebx
        mov     ecx, lwidth
        
    plot:
        mov     ax, [edi]
        add     al, [esi]
        jnc     bstep
        mov     al, 255
    bstep:
        add     ah, [esi + 1]
        jnc     gstep
        mov     ah, 255
    gstep:
        mov     bl, [edi + 2]
        add     bl, [esi + 2]
        jnc     rstep
        mov     bl, 255
    rstep:
        stosw
        mov     al, bl
        stosb
        add     esi, 3
        dec     ecx
        jnz     plot
        pop     ebx
        add     edi, ebx
        add     esi, edx
        dec     lheight
        jnz     next
    }
}

void putImageAdd16(int32_t x1, int32_t y1, GFX_IMAGE *img)
{
    int32_t x2, y2, lwidth, lheight;
    int32_t lminX, lminY, lmaxX, lmaxY;
    
    void *imgData = img->mData;
    int32_t imgWidth  = img->mWidth;
    int32_t imgHeight = img->mHeight;
    
    // Check compatible pixel mode
    if (img->mPixels != bytesPerPixel) return;

    // Calculate new position
    x2 = x1 + (imgWidth - 1);
    y2 = y1 + (imgHeight - 1);

    // Clip image to context boundaries
    lminX = max(x1, cminX);
    lminY = max(y1, cminY);
    lmaxX = min(x2, cmaxX);
    lmaxY = min(y2, cmaxY);

    // Initialize loop variables
    lwidth  = (lmaxX - lminX) + 1;
    lheight = (lmaxY - lminY) + 1;

    // Check range
    if (lwidth <= 0 || lheight <= 0) return;

    __asm {
        mov     edi, lfbPtr
        mov     eax, lminY
        add     eax, pageOffset
        mul     lfbWidth
        add     eax, lminX
        shl     eax, 1
        add     edi, eax
        mov     esi, imgData
        mov     eax, lminY
        sub     eax, y1
        mul     imgWidth
        mov     ebx, lminX
        sub     ebx, x1
        add     eax, ebx
        shl     eax, 1
        add     esi, eax
        mov     ebx, lfbWidth
        sub     ebx, lwidth
        shl     ebx, 1
        mov     edx, imgWidth
        sub     edx, lwidth
        shl     edx, 1
    next:
        push    lwidth
        push    ebx
        push    edx
    plot:
        lodsw
        mov     bx, [edi]
        mov     dx, bx
        and     bx, 1111100000011111b
        and     dx, 11111100000b
        mov     cx, ax
        and     ax, 1111100000011111b
        and     cx, 11111100000b
        add     al, bl
        cmp     al, 11111b
        jna     bstep
        mov     al, 11111b
    bstep:
        add     ah, bh
        jnc     gstep
        mov     ah, 11111000b
    gstep:
        add     cx, dx
        cmp     cx, 11111100000b
        jna     rstep
        mov     cx, 11111100000b
    rstep:
        or      ax, cx
        stosw
        dec     lwidth
        jnz     plot
        pop     edx
        pop     ebx  
        pop     lwidth
        add     edi, ebx
        add     esi, edx
        dec     lheight
        jnz     next
    }
}

void putImageAdd15(int32_t x1, int32_t y1, GFX_IMAGE *img)
{
    int32_t x2, y2, lwidth, lheight;
    int32_t lminX, lminY, lmaxX, lmaxY;
    
    void *imgData = img->mData;
    int32_t imgWidth  = img->mWidth;
    int32_t imgHeight = img->mHeight;
    
    // Check compatible pixel mode
    if (img->mPixels != bytesPerPixel) return;

    // Calculate new position
    x2 = x1 + (imgWidth - 1);
    y2 = y1 + (imgHeight - 1);

    // Clip image to context boundaries
    lminX = max(x1, cminX);
    lminY = max(y1, cminY);
    lmaxX = min(x2, cmaxX);
    lmaxY = min(y2, cmaxY);

    // Initialize loop variables
    lwidth  = (lmaxX - lminX) + 1;
    lheight = (lmaxY - lminY) + 1;

    // Check range
    if (lwidth <= 0 || lheight <= 0) return;

    __asm {
        mov     edi, lfbPtr
        mov     eax, lminY
        add     eax, pageOffset
        mul     lfbWidth
        add     eax, lminX
        shl     eax, 1
        add     edi, eax
        mov     esi, imgData
        mov     eax, lminY
        sub     eax, y1
        mul     imgWidth
        mov     ebx, lminX
        sub     ebx, x1
        add     eax, ebx
        shl     eax, 1
        add     esi, eax
        mov     ebx, lfbWidth
        sub     ebx, lwidth
        shl     ebx, 1
        mov     edx, imgWidth
        sub     edx, lwidth
        shl     edx, 1
    next:
        push    lwidth
        push    ebx
        push    edx
    plot:
        lodsw
        mov     bx, [edi]
        mov     dx, bx
        and     bx, 111110000011111b
        and     dx, 1111100000b
        mov     cx, ax
        and     ax, 111110000011111b
        and     cx, 1111100000b
        add     al, bl
        cmp     al, 11111b
        jna     bstep
        mov     al, 11111b
    bstep:
        add     ah, bh
        cmp     ah, 1111100b
        jna     gstep
        mov     ah, 1111100b
    gstep:
        add     cx, dx
        cmp     cx, 1111100000b
        jna     rstep
        mov     cx, 1111100000b
    rstep:
        or      ax, cx
        stosw
        dec     lwidth
        jnz     plot
        pop     edx
        pop     ebx  
        pop     lwidth
        add     edi, ebx
        add     esi, edx
        dec     lheight
        jnz     next
    }
}

void putImageSub32(int32_t x1, int32_t y1, GFX_IMAGE *img)
{
    int32_t x2, y2, lwidth, lheight;
    int32_t lminX, lminY, lmaxX, lmaxY;
    
    void *imgData = img->mData;
    int32_t imgWidth  = img->mWidth;
    int32_t imgHeight = img->mHeight;
    
    // Check for compatible pixel mode
    if (img->mPixels != bytesPerPixel) return;

    // Calculate new position
    x2 = x1 + (imgWidth - 1);
    y2 = y1 + (imgHeight - 1);

    // Clip image to context boundaries
    lminX = max(x1, cminX);
    lminY = max(y1, cminY);
    lmaxX = min(x2, cmaxX);
    lmaxY = min(y2, cmaxY);

    // Initialize loop variables
    lwidth  = (lmaxX - lminX) + 1;
    lheight = (lmaxY - lminY) + 1;

    // Check range
    if (lwidth <= 0 || lheight <= 0) return;

#ifdef _USE_MMX
    __asm {
        mov     edi, lfbPtr
        mov     eax, lminY
        add     eax, pageOffset
        mul     lfbWidth
        add     eax, lminX
        shl     eax, 2
        add     edi, eax
        mov     esi, imgData
        mov     eax, lminY
        sub     eax, y1
        mul     imgWidth
        mov     ebx, lminX
        sub     ebx, x1
        add     eax, ebx
        shl     eax, 2
        add     esi, eax
        mov     ebx, lfbWidth
        sub     ebx, lwidth
        shl     ebx, 2
        mov     edx, imgWidth
        sub     edx, lwidth
        shl     edx, 2
    next:
        mov     ecx, lwidth
        shr     ecx, 1
        jz      once
    move:
        movq    mm0, [esi]
        psubusb mm0, [edi]
        movq    [edi], mm0
        add     edi, 8
        add     esi, 8
        dec     ecx
        jnz     move
    once:
        test    lwidth, 1
        jz      skip
        movd    mm0, [esi]
        psubusb mm0, [edi]
        movd    [edi], mm0
        add     edi, 4
        add     esi, 4
    skip:
        add     edi, ebx
        add     esi, edx
        dec     lheight
        jnz     next
        emms
    }
#else    
    __asm {
        mov     edi, lfbPtr
        mov     eax, lminY
        add     eax, pageOffset
        mul     lfbWidth
        add     eax, lminX
        shl     eax, 2
        add     edi, eax
        mov     esi, imgData
        mov     eax, lminY
        sub     eax, y1
        mul     imgWidth
        mov     ebx, lminX
        sub     ebx, x1
        add     eax, ebx
        shl     eax, 2
        add     esi, eax
        mov     ebx, lfbWidth
        sub     ebx, lwidth
        shl     ebx, 2
        mov     edx, imgWidth
        sub     edx, lwidth
        shl     edx, 2
    next:
        mov     ecx, lwidth
        push    ebx
    plot:
        mov     eax, [edi]
        sub     al, [esi]
        jnc     bstep
        xor     al, al
    bstep:
        sub     ah, [esi + 1]
        jnc     gstep
        xor     ah, ah
    gstep:
        mov     ebx, eax
        shr     ebx, 16
        sub     bl, [esi + 2]
        jnc     rstep
        xor     bl, bl
    rstep:
        shl     ebx, 16
        and     eax, 00FFFFh
        or      eax, ebx
        stosd
        add     esi, 4
        dec     ecx
        jnz     plot
        pop     ebx
        add     edi, ebx
        add     esi, edx
        dec     lheight
        jnz     next
    }
#endif
}

void putImageSub24(int32_t x1, int32_t y1, GFX_IMAGE *img)
{
    int32_t x2, y2, lwidth, lheight;
    int32_t lminX, lminY, lmaxX, lmaxY;
    
    void *imgData = img->mData;
    int32_t imgWidth  = img->mWidth;
    int32_t imgHeight = img->mHeight;

    // Check for compatible pixel mode
    if (img->mPixels != bytesPerPixel) return;

    // Calculate new position
    x2 = x1 + (imgWidth - 1);
    y2 = y1 + (imgHeight - 1);

    // Clip image to context boundaries
    lminX = max(x1, cminX);
    lminY = max(y1, cminY);
    lmaxX = min(x2, cmaxX);
    lmaxY = min(y2, cmaxY);

    // Initialize loop variables
    lwidth  = (lmaxX - lminX) + 1;
    lheight = (lmaxY - lminY) + 1;

    // Check range
    if (lwidth <= 0 || lheight <= 0) return;

    __asm {
        mov     edi, lfbPtr
        mov     eax, lminY
        add     eax, pageOffset
        mul     lfbWidth
        add     eax, lminX
        mov     ebx, eax
        shl     eax, 1
        add     eax, ebx
        add     edi, eax
        mov     esi, imgData
        mov     eax, lminY
        sub     eax, y1
        mul     imgWidth
        mov     ebx, lminX
        sub     ebx, x1
        add     eax, ebx
        mov     ebx, eax
        shl     eax, 1
        add     eax, ebx
        add     esi, eax
        mov     ebx, lfbWidth
        sub     ebx, lwidth
        mov     eax, ebx
        shl     ebx, 1
        add     ebx, eax
        mov     edx, imgWidth
        sub     edx, lwidth
        mov     ecx, edx
        shl     edx, 1
        add     edx, ecx
    next:
        mov     ecx, lwidth
        push    ebx
    plot:
        mov     ax, [edi]
        sub     al, [esi]
        jnc     bstep
        xor     al, al
    bstep:
        sub     ah, [esi + 1]
        jnc     gstep
        xor     ah, ah
    gstep:
        mov     bl, [edi + 2]
        sub     bl, [esi + 2]
        jnc     rstep
        xor     bl, bl
    rstep:
        stosw
        mov     al, bl
        stosb
        add     esi, 3
        dec     ecx
        jnz     plot
        pop     ebx
        add     edi, ebx
        add     esi, edx
        dec     lheight
        jnz     next
    }
}

void putImageSub16(int32_t x1, int32_t y1, GFX_IMAGE *img)
{
    int32_t x2, y2, lwidth, lheight;
    int32_t lminX, lminY, lmaxX, lmaxY;
    
    void *imgData = img->mData;
    int32_t imgWidth  = img->mWidth;
    int32_t imgHeight = img->mHeight;
    
    // Check compatible pixel mode
    if (img->mPixels != bytesPerPixel) return;

    // Calculate new position
    x2 = x1 + (imgWidth - 1);
    y2 = y1 + (imgHeight - 1);

    // Clip image to context boundaries
    lminX = max(x1, cminX);
    lminY = max(y1, cminY);
    lmaxX = min(x2, cmaxX);
    lmaxY = min(y2, cmaxY);

    // Initialize loop variables
    lwidth  = (lmaxX - lminX) + 1;
    lheight = (lmaxY - lminY) + 1;

    // Check range
    if (lwidth <= 0 || lheight <= 0) return;

    __asm {
        mov     edi, lfbPtr
        mov     eax, lminY
        add     eax, pageOffset
        mul     lfbWidth
        add     eax, lminX
        shl     eax, 1
        add     edi, eax
        mov     esi, imgData
        mov     eax, lminY
        sub     eax, y1
        mul     imgWidth
        mov     ebx, lminX
        sub     ebx, x1
        add     eax, ebx
        shl     eax, 1
        add     esi, eax
        mov     ebx, lfbWidth
        sub     ebx, lwidth
        shl     ebx, 1
        mov     edx, imgWidth
        sub     edx, lwidth
        shl     edx, 1
    next:
        push    lwidth
        push    ebx
        push    edx
    plot:
        mov     ax, [edi]
        mov     bx, [esi]
        mov     dx, bx
        and     bx, 1111100000011111b
        and     dx, 11111100000b
        mov     cx, ax
        and     ax, 1111100000011111b
        and     cx, 11111100000b
        sub     al, bl
        jnc     bstep
        xor     al, al
    bstep:
        sub     ah, bh
        jnc     gstep
        xor     ah, ah
    gstep:
        sub     cx, dx
        jc      rstep
        or      ax, cx
    rstep:
        stosw
        add     esi, 2
        dec     lwidth
        jnz     plot
        pop     edx
        pop     ebx  
        pop     lwidth
        add     edi, ebx
        add     esi, edx
        dec     lheight
        jnz     next
    }
}

void putImageSub15(int32_t x1, int32_t y1, GFX_IMAGE *img)
{
    int32_t x2, y2, lwidth, lheight;
    int32_t lminX, lminY, lmaxX, lmaxY;
    
    void *imgData = img->mData;
    int32_t imgWidth  = img->mWidth;
    int32_t imgHeight = img->mHeight;
    
    // Check compatible pixel mode
    if (img->mPixels != bytesPerPixel) return;

    // Calculate new position
    x2 = x1 + (imgWidth - 1);
    y2 = y1 + (imgHeight - 1);

    // Clip image to context boundaries
    lminX = max(x1, cminX);
    lminY = max(y1, cminY);
    lmaxX = min(x2, cmaxX);
    lmaxY = min(y2, cmaxY);

    // Initialize loop variables
    lwidth  = (lmaxX - lminX) + 1;
    lheight = (lmaxY - lminY) + 1;

    // Check range
    if (lwidth <= 0 || lheight <= 0) return;

    __asm {
        mov     edi, lfbPtr
        mov     eax, lminY
        add     eax, pageOffset
        mul     lfbWidth
        add     eax, lminX
        shl     eax, 1
        add     edi, eax
        mov     esi, imgData
        mov     eax, lminY
        sub     eax, y1
        mul     imgWidth
        mov     ebx, lminX
        sub     ebx, x1
        add     eax, ebx
        shl     eax, 1
        add     esi, eax
        mov     ebx, lfbWidth
        sub     ebx, lwidth
        shl     ebx, 1
        mov     edx, imgWidth
        sub     edx, lwidth
        shl     edx, 1
    next:
        push    lwidth
        push    ebx
        push    edx
    plot:
        mov     ax, [edi]
        mov     bx, [esi]
        mov     dx, bx
        and     bx, 111110000011111b
        and     dx, 1111100000b
        mov     cx, ax
        and     ax, 111110000011111b
        and     cx, 1111100000b
        sub     al, bl
        jnc     bstep
        xor     al, al
    bstep:
        sub     ah, bh
        jnc     gstep
        xor     ah, ah
    gstep:
        sub     cx, dx
        jc      rstep
        or      ax, cx
    rstep:
        stosw
        add     esi, 2
        dec     lwidth
        jnz     plot
        pop     edx
        pop     ebx  
        pop     lwidth
        add     edi, ebx
        add     esi, edx
        dec     lheight
        jnz     next
    }
}

// Sprite functions
void putSprite8(int32_t x1, int32_t y1, uint32_t key, GFX_IMAGE *img)
{
    int32_t x2, y2, lwidth, lheight;
    int32_t lminX, lminY, lmaxX, lmaxY;
    
    void *imgData = img->mData;
    int32_t imgWidth  = img->mWidth;
    int32_t imgHeight = img->mHeight;
    
    // Calculate new position
    x2 = x1 + (imgWidth - 1);
    y2 = y1 + (imgHeight - 1);

    // Clip image to context boundaries
    lminX = max(x1, cminX);
    lminY = max(y1, cminY);
    lmaxX = min(x2, cmaxX);
    lmaxY = min(y2, cmaxY);

    // Initialize loop variables
    lwidth  = (lmaxX - lminX) + 1;
    lheight = (lmaxY - lminY) + 1;

    // Check range
    if (lwidth <= 0 || lheight <= 0) return;

#ifdef _USE_MMX
    __asm {
        mov         edi, lfbPtr
        mov         eax, lminY
        add         eax, pageOffset
        mul         lfbWidth
        add         eax, lminX
        add         edi, eax
        mov         esi, imgData
        mov         eax, lminY
        sub         eax, y1
        mul         imgWidth
        mov         ebx, lminX
        sub         ebx, x1
        add         eax, ebx
        add         esi, eax
        mov         ebx, lfbWidth
        sub         ebx, lwidth
        mov         edx, imgWidth
        sub         edx, lwidth
        movd        mm4, key
        punpcklbw   mm4, mm4
        punpcklwd   mm4, mm4
        punpckldq   mm4, mm4
        mov         eax, 0ffffffffh
        movd        mm5, eax
        punpckldq   mm5, mm5
        movq        mm6, mm5
    next:
        mov         ecx, lwidth
        shr         ecx, 3
        jz          once
    plot:
        movq        mm0, [esi]
        movq        mm2, mm4
        pand        mm0, mm6
        pcmpeqd     mm2, mm0
        movq        mm1, [edi]
        pand        mm1, mm2
        pxor        mm2, mm5
        pand        mm0, mm2
        por         mm1, mm0
        movq        [edi], mm1
        add         esi, 8
        add         edi, 8
        dec         ecx
        jnz         plot
    once:
        mov         ecx, lwidth
        and         ecx, 7
        jz          skip
    again:
        lodsb
        cmp         al, byte ptr[key]
        je          end
        mov         [edi], al
    end:
        inc         edi
        dec         ecx
        jnz         again
    skip:
        add         edi, ebx
        add         esi, edx
        dec         lheight
        jnz         next
        emms        
    }
#else
    __asm {
        mov         edi, lfbPtr
        mov         eax, lminY
        add         eax, pageOffset
        mul         lfbWidth
        add         eax, lminX
        add         edi, eax
        mov         esi, imgData
        mov         eax, lminY
        sub         eax, y1
        mul         imgWidth
        mov         ebx, lminX
        sub         ebx, x1
        add         eax, ebx
        add         esi, eax
        mov         ebx, lfbWidth
        sub         ebx, lwidth
        mov         edx, imgWidth
        sub         edx, lwidth
    next:
        mov         ecx, lwidth
    plot:
        lodsb
        cmp         al, byte ptr[key]
        je          skip
        mov         [edi], al
    skip:
        inc         edi
        dec         ecx
        jnz         plot
        add         edi, ebx
        add         esi, edx
        dec         lheight
        jnz         next
    }
#endif
}

void putSprite1516(int32_t x1, int32_t y1, uint32_t key, GFX_IMAGE *img)
{
    int32_t x2, y2, lwidth, lheight;
    int32_t lminX, lminY, lmaxX, lmaxY;
    
    void *imgData = img->mData;
    int32_t imgWidth  = img->mWidth;
    int32_t imgHeight = img->mHeight;
    
    // Calculate new position
    x2 = x1 + (imgWidth - 1);
    y2 = y1 + (imgHeight - 1);

    // Clip image to context boundaries
    lminX = max(x1, cminX);
    lminY = max(y1, cminY);
    lmaxX = min(x2, cmaxX);
    lmaxY = min(y2, cmaxY);

    // Initialize loop variables
    lwidth  = (lmaxX - lminX) + 1;
    lheight = (lmaxY - lminY) + 1;

    // Check range
    if (lwidth <= 0 || lheight <= 0) return;

#ifdef _USE_MMX
    __asm {
        mov         edi, lfbPtr
        mov         eax, lminY
        add         eax, pageOffset
        mul         lfbWidth
        add         eax, lminX
        shl         eax, 1
        add         edi, eax
        mov         esi, imgData
        mov         eax, lminY
        sub         eax, y1
        mul         imgWidth
        mov         ebx, lminX
        sub         ebx, x1
        add         eax, ebx
        shl         eax, 1
        add         esi, eax
        mov         ebx, lfbWidth
        sub         ebx, lwidth
        shl         ebx, 1
        mov         edx, imgWidth
        sub         edx, lwidth
        shl         edx, 1
        movd        mm4, key
        punpcklwd   mm4, mm4
        punpckldq   mm4, mm4
        mov         eax, 0ffffffffh
        movd        mm5, eax
        punpckldq   mm5, mm5
        movq        mm6, mm5
    next:
        mov         ecx, lwidth
        shr         ecx, 2
        jz          once
    plot:
        movq        mm0, [esi]
        movq        mm2, mm4
        pand        mm0, mm6
        pcmpeqd     mm2, mm0
        movq        mm1, [edi]
        pand        mm1, mm2
        pxor        mm2, mm5
        pand        mm0, mm2
        por         mm1, mm0
        movq        [edi], mm1
        add         esi, 8
        add         edi, 8
        dec         ecx
        jnz         plot
    once:
        mov         ecx, lwidth
        and         ecx, 3
        jz          skip
    again:
        lodsw
        cmp         ax, word ptr key
        je          end
        mov         [edi], ax
    end:
        add         edi, 2
        dec         ecx
        jnz         again
    skip:
        add         edi, ebx
        add         esi, edx
        dec         lheight
        jnz         next
        emms
    }
#else
    __asm {
        mov         edi, lfbPtr
        mov         eax, lminY
        add         eax, pageOffset
        mul         lfbWidth
        add         eax, lminX
        shl         eax, 1
        add         edi, eax
        mov         esi, imgData
        mov         eax, lminY
        sub         eax, y1
        mul         imgWidth
        mov         ebx, lminX
        sub         ebx, x1
        add         eax, ebx
        shl         eax, 1
        add         esi, eax
        mov         ebx, lfbWidth
        sub         ebx, lwidth
        shl         ebx, 1
        mov         edx, imgWidth
        sub         edx, lwidth
        shl         edx, 1
    next:
        mov         ecx, lwidth
    plot:
        lodsw
        cmp         ax, word ptr key
        je          skip
        mov         [edi], ax
    skip:
        add         edi, 2
        dec         ecx
        jnz         plot
        add         edi, ebx
        add         esi, edx
        dec         lheight
        jnz         next
    }
#endif
}

void putSprite24(int32_t x1, int32_t y1, uint32_t key, GFX_IMAGE *img)
{
    int32_t x2, y2, lwidth, lheight;
    int32_t lminX, lminY, lmaxX, lmaxY;
    
    void *imgData = img->mData;
    int32_t imgWidth  = img->mWidth;
    int32_t imgHeight = img->mHeight;

    // Calculate new position
    x2 = x1 + (imgWidth - 1);
    y2 = y1 + (imgHeight - 1);

    // Clip image to context boundaries
    lminX = max(x1, cminX);
    lminY = max(y1, cminY);
    lmaxX = min(x2, cmaxX);
    lmaxY = min(y2, cmaxY);

    // Initialize loop variables
    lwidth  = (lmaxX - lminX) + 1;
    lheight = (lmaxY - lminY) + 1;

    // Check range
    if (lwidth <= 0 || lheight <= 0) return;

    __asm {
        mov     edi, lfbPtr
        mov     eax, lminY
        add     eax, pageOffset
        mul     lfbWidth
        add     eax, lminX
        mov     ebx, eax
        shl     eax, 1
        add     eax, ebx
        add     edi, eax
        mov     esi, imgData
        mov     eax, lminY
        sub     eax, y1
        mul     imgWidth
        mov     ebx, lminX
        sub     ebx, x1
        add     eax, ebx
        mov     ebx, eax
        shl     eax, 1
        add     eax, ebx
        add     esi, eax
        mov     ebx, lfbWidth
        sub     ebx, lwidth
        mov     eax, ebx
        shl     ebx, 1
        add     ebx, eax
        mov     edx, imgWidth
        sub     edx, lwidth
        mov     eax, edx
        shl     edx, 1
        add     edx, eax
    next:
        push    edx
        mov     ecx, lwidth
    plot:
        lodsw
        mov     edx, eax
        lodsb
        shl     eax, 16
        or      edx, eax
        and     edx, 00FFFFFFh
        cmp     edx, key
        je      skip
        mov     [edi], dx
        shr     edx, 16
        mov     [edi + 2], dl
    skip:
        add     edi, 3
        dec     ecx
        jnz     plot
        pop     edx
        add     edi, ebx
        add     esi, edx
        dec     lheight
        jnz     next
    }
}

void putSprite32(int32_t x1, int32_t y1, uint32_t key, GFX_IMAGE *img)
{
    int32_t x2, y2, lwidth, lheight;
    int32_t lminX, lminY, lmaxX, lmaxY;
    
    void *imgData = img->mData;
    int32_t imgWidth  = img->mWidth;
    int32_t imgHeight = img->mHeight;

    // Calculate new position
    x2 = x1 + (imgWidth - 1);
    y2 = y1 + (imgHeight - 1);

    // Clip image to context boundaries
    lminX = max(x1, cminX);
    lminY = max(y1, cminY);
    lmaxX = min(x2, cmaxX);
    lmaxY = min(y2, cmaxY);

    // Initialize loop variables
    lwidth  = (lmaxX - lminX) + 1;
    lheight = (lmaxY - lminY) + 1;

    // Check range
    if (lwidth <= 0 || lheight <= 0) return;

#ifdef _USE_MMX
    __asm {
        mov         edi, lfbPtr
        mov         eax, lminY
        add         eax, pageOffset
        mul         lfbWidth
        add         eax, lminX
        shl         eax, 2
        add         edi, eax
        mov         esi, imgData
        mov         eax, lminY
        sub         eax, y1
        mul         imgWidth
        mov         ebx, lminX
        sub         ebx, x1
        add         eax, ebx
        shl         eax, 2
        add         esi, eax
        mov         ebx, lfbWidth
        sub         ebx, lwidth
        shl         ebx, 2
        mov         edx, imgWidth
        sub         edx, lwidth
        shl         edx, 2
        movd        mm4, key
        punpckldq   mm4, mm4
        mov         eax, 0ffffffffh
        movd        mm5, eax
        punpckldq   mm5, mm5
        mov         eax, 0ffffffh
        movd        mm6, eax
        punpckldq   mm6, mm6
    next:
        mov         ecx, lwidth
        shr         ecx, 1
        jz          once
    plot:
        movq        mm0, [esi]
        movq        mm2, mm4
        pand        mm0, mm6
        pcmpeqd     mm2, mm0
        movq        mm1, [edi]
        pand        mm1, mm2
        pxor        mm2, mm5
        pand        mm0, mm2
        por         mm1, mm0
        movq        [edi], mm1
        add         esi, 8
        add         edi, 8
        dec         ecx
        jnz         plot
    once:
        test        lwidth, 1
        jz          skip
        add         esi, 4
        add         edi, 4
        mov         eax, [esi]
        cmp         eax, key
        je          skip
        mov         [edi - 4], eax
    skip:
        add         edi, ebx
        add         esi, edx
        dec         lheight
        jnz         next
        emms
    }
#else
    __asm {
        mov         edi, lfbPtr
        mov         eax, lminY
        add         eax, pageOffset
        mul         lfbWidth
        add         eax, lminX
        shl         eax, 2
        add         edi, eax
        mov         esi, imgData
        mov         eax, lminY
        sub         eax, y1
        mul         imgWidth
        mov         ebx, lminX
        sub         ebx, x1
        add         eax, ebx
        shl         eax, 2
        add         esi, eax
        mov         ebx, lfbWidth
        sub         ebx, lwidth
        shl         ebx, 2
        mov         edx, imgWidth
        sub         edx, lwidth
        shl         edx, 2
    next:
        mov         ecx, lwidth
    plot:
        lodsd
        and         eax, 00FFFFFFh
        cmp         eax, key
        je          skip
        mov         [edi], eax
    skip:
        add         edi, 4
        dec         ecx
        jnz         plot
        add         edi, ebx
        add         esi, edx
        dec         lheight
        jnz         next
    }
#endif
}

void putSpriteAdd32(int32_t x1, int32_t y1, uint32_t key, GFX_IMAGE *img)
{
    int32_t x2, y2, lwidth, lheight;
    int32_t lminX, lminY, lmaxX, lmaxY;
    
    void *imgData = img->mData;
    int32_t imgWidth  = img->mWidth;
    int32_t imgHeight = img->mHeight;

    // Calculate new position
    x2 = x1 + (imgWidth - 1);
    y2 = y1 + (imgHeight - 1);

    // Clip image to context boundaries
    lminX = max(x1, cminX);
    lminY = max(y1, cminY);
    lmaxX = min(x2, cmaxX);
    lmaxY = min(y2, cmaxY);

    // Initialize loop variables
    lwidth  = (lmaxX - lminX) + 1;
    lheight = (lmaxY - lminY) + 1;

    // Check range
    if (lwidth <= 0 || lheight <= 0) return;

#ifdef _USE_MMX
    __asm {
        mov         edi, lfbPtr
        mov         eax, lminY
        add         eax, pageOffset
        mul         lfbWidth
        add         eax, lminX
        shl         eax, 2
        add         edi, eax
        mov         esi, imgData
        mov         eax, lminY
        sub         eax, y1
        mul         imgWidth
        mov         ebx, lminX
        sub         ebx, x1
        add         eax, ebx
        shl         eax, 2
        add         esi, eax
        mov         ebx, lfbWidth
        sub         ebx, lwidth
        shl         ebx, 2
        mov         edx, imgWidth
        sub         edx, lwidth
        shl         edx, 2
        movd        mm4, key
        punpckldq   mm4, mm4
        mov         eax, 0ffffffffh
        movd        mm5, eax
        punpckldq   mm5, mm5
        mov         eax, 0ffffffh
        movd        mm6, eax
        punpckldq   mm6, mm6
    next:
        mov         ecx, lwidth
        shr         ecx, 1
        jz          once
    plot:
        movq        mm0, [esi]
        movq        mm2, mm4
        pand        mm0, mm6
        pcmpeqd     mm2, mm0
        pxor        mm2, mm5
        pand        mm0, mm2
        paddusb     mm0, [edi]
        movq        [edi], mm0
        add         esi, 8
        add         edi, 8
        dec         ecx
        jnz         plot
    once:
        test        lwidth, 1
        jz          skip
        add         esi, 4
        add         edi, 4
        mov         eax, [esi]
        cmp         eax, key
        je          skip
        movd        mm3, eax
        paddusb     mm3, [edi - 4]
        movd        [edi - 4], mm3
    skip:
        add         edi, ebx
        add         esi, edx
        dec         lheight
        jnz         next
        emms    
    }
#else
    __asm {
        mov         edi, lfbPtr
        mov         eax, lminY
        add         eax, pageOffset
        mul         lfbWidth
        add         eax, lminX
        shl         eax, 2
        add         edi, eax
        mov         esi, imgData
        mov         eax, lminY
        sub         eax, y1
        mul         imgWidth
        mov         ebx, lminX
        sub         ebx, x1
        add         eax, ebx
        shl         eax, 2
        add         esi, eax
        mov         ebx, lfbWidth
        sub         ebx, lwidth
        shl         ebx, 2
        mov         edx, imgWidth
        sub         edx, lwidth
        shl         edx, 2
    next:
        push        ebx
        mov         ecx, lwidth
    plot:
        lodsd
        and         eax, 00FFFFFFh
        cmp         eax, key
        je          skip
        add         al, [edi]
        jnc         bstep
        mov         al, 255
    bstep:
        add         ah, [edi + 1]
        jnc         gstep
        mov         ah, 255
    gstep:
        mov         ebx, eax
        shr         ebx, 16
        add         bl, [edi + 2]
        jnc         rstep
        mov         bl, 255
    rstep:
        shl         ebx, 16
        and         eax, 00FFFFh
        or          eax, ebx
        mov         [edi], eax
    skip:
        add         edi, 4
        dec         ecx
        jnz         plot
        pop         ebx
        add         edi, ebx
        add         esi, edx
        dec         lheight
        jnz         next
    }
#endif    
}

void putSpriteAdd24(int32_t x1, int32_t y1, uint32_t key, GFX_IMAGE *img)
{
    int32_t x2, y2, lwidth, lheight;
    int32_t lminX, lminY, lmaxX, lmaxY;
    
    void *imgData = img->mData;
    int32_t imgWidth  = img->mWidth;
    int32_t imgHeight = img->mHeight;

    // Calculate new position
    x2 = x1 + (imgWidth - 1);
    y2 = y1 + (imgHeight - 1);

    // Clip image to context boundaries
    lminX = max(x1, cminX);
    lminY = max(y1, cminY);
    lmaxX = min(x2, cmaxX);
    lmaxY = min(y2, cmaxY);

    // Initialize loop variables
    lwidth  = (lmaxX - lminX) + 1;
    lheight = (lmaxY - lminY) + 1;

    // Check range
    if (lwidth <= 0 || lheight <= 0) return;

    __asm {
        mov     edi, lfbPtr
        mov     eax, lminY
        add     eax, pageOffset
        mul     lfbWidth
        add     eax, lminX
        mov     ebx, eax
        shl     eax, 1
        add     eax, ebx
        add     edi, eax
        mov     esi, imgData
        mov     eax, lminY
        sub     eax, y1
        mul     imgWidth
        mov     ebx, lminX
        sub     ebx, x1
        add     eax, ebx
        mov     ebx, eax
        shl     eax, 1
        add     eax, ebx
        add     esi, eax
        mov     ebx, lfbWidth
        sub     ebx, lwidth
        mov     eax, ebx
        shl     ebx, 1
        add     ebx, eax
        mov     edx, imgWidth
        sub     edx, lwidth
        mov     ecx, edx
        shl     edx, 1
        add     edx, ecx
    next:
        push    ebx
        mov     ecx, lwidth
    plot:
        lodsw
        mov     ebx, eax
        lodsb
        shl     eax, 16
        or      eax, ebx
        and     eax, 00FFFFFFh
        cmp     eax, key
        je      skip
        add     al, [edi]
        jnc     bstep
        mov     al, 255
    bstep:
        add     ah, [edi + 1]
        jnc     gstep
        mov     ah, 255
    gstep:
        mov     ebx, eax
        shr     ebx, 16
        add     bl, [edi + 2]
        jnc     rstep
        mov     bl, 255
    rstep:
        mov     [edi], ax
        mov     [edi + 2], bl
    skip:
        add     edi, 3
        dec     ecx
        jnz     plot
        pop     ebx
        add     edi, ebx
        add     esi, edx
        dec     lheight
        jnz     next
    }
}

void putSpriteAdd16(int32_t x1, int32_t y1, uint32_t key, GFX_IMAGE *img)
{
    int32_t x2, y2, lwidth, lheight;
    int32_t lminX, lminY, lmaxX, lmaxY;
    
    void *imgData = img->mData;
    int32_t imgWidth  = img->mWidth;
    int32_t imgHeight = img->mHeight;

    // Calculate new position
    x2 = x1 + (imgWidth - 1);
    y2 = y1 + (imgHeight - 1);

    // Clip image to context boundaries
    lminX = max(x1, cminX);
    lminY = max(y1, cminY);
    lmaxX = min(x2, cmaxX);
    lmaxY = min(y2, cmaxY);

    // Initialize loop variables
    lwidth  = (lmaxX - lminX) + 1;
    lheight = (lmaxY - lminY) + 1;

    // Check range
    if (lwidth <= 0 || lheight <= 0) return;

    __asm {
        mov     edi, lfbPtr
        mov     eax, lminY
        add     eax, pageOffset
        mul     lfbWidth
        add     eax, lminX
        shl     eax, 1
        add     edi, eax
        mov     esi, imgData
        mov     eax, lminY
        sub     eax, y1
        mul     imgWidth
        mov     ebx, lminX
        sub     ebx, x1
        add     eax, ebx
        shl     eax, 1
        add     esi, eax
        mov     ebx, lfbWidth
        sub     ebx, lwidth
        shl     ebx, 1
        mov     edx, imgWidth
        sub     edx, lwidth
        shl     edx, 1
    next:
        push    ebx
        push    edx
        push    lwidth
    plot:
        lodsw
        cmp     ax, word ptr key
        je      skip
        mov     bx, [edi]
        mov     dx, bx
        and     bx, 1111100000011111b
        and     dx, 11111100000b
        mov     cx, ax
        and     ax, 1111100000011111b
        and     cx, 11111100000b
        add     al, bl
        cmp     al, 11111b
        jna     bstep
        mov     al, 11111b
    bstep:
        add     ah, bh
        jnc     gstep
        mov     ah, 11111000b
    gstep:
        add     cx, dx
        cmp     cx, 11111100000b
        jna     rstep
        mov     cx, 11111100000b
    rstep:
        or      ax, cx
        mov     [edi], ax
    skip:
        add     edi, 2
        dec     lwidth
        jnz     plot
        pop     lwidth
        pop     edx
        pop     ebx
        add     edi, ebx
        add     esi, edx
        dec     lheight
        jnz     next
    }
}

void putSpriteAdd15(int32_t x1, int32_t y1, uint32_t key, GFX_IMAGE *img)
{
    int32_t x2, y2, lwidth, lheight;
    int32_t lminX, lminY, lmaxX, lmaxY;
    
    void *imgData = img->mData;
    int32_t imgWidth  = img->mWidth;
    int32_t imgHeight = img->mHeight;

    // Calculate new position
    x2 = x1 + (imgWidth - 1);
    y2 = y1 + (imgHeight - 1);

    // Clip image to context boundaries
    lminX = max(x1, cminX);
    lminY = max(y1, cminY);
    lmaxX = min(x2, cmaxX);
    lmaxY = min(y2, cmaxY);

    // Initialize loop variables
    lwidth  = (lmaxX - lminX) + 1;
    lheight = (lmaxY - lminY) + 1;

    // Check range
    if (lwidth <= 0 || lheight <= 0) return;

    __asm {
        mov     edi, lfbPtr
        mov     eax, lminY
        add     eax, pageOffset
        mul     lfbWidth
        add     eax, lminX
        shl     eax, 1
        add     edi, eax
        mov     esi, imgData
        mov     eax, lminY
        sub     eax, y1
        mul     imgWidth
        mov     ebx, lminX
        sub     ebx, x1
        add     eax, ebx
        shl     eax, 1
        add     esi, eax
        mov     ebx, lfbWidth
        sub     ebx, lwidth
        shl     ebx, 1
        mov     edx, imgWidth
        sub     edx, lwidth
        shl     edx, 1
    next:
        push    ebx
        push    edx
        push    lwidth
    plot:
        lodsw
        cmp     ax, word ptr key
        je      skip
        mov     bx, [edi]
        mov     dx, bx
        and     bx, 111110000011111b
        and     dx, 1111100000b
        mov     cx, ax
        and     ax, 111110000011111b
        and     cx, 1111100000b
        add     al, bl
        cmp     al, 11111b
        jna     bstep
        mov     al, 11111b
    bstep:
        add     ah, bh
        cmp     ah, 1111100b
        jna     gstep
        mov     ah, 1111100b
    gstep:
        add     cx, dx
        cmp     cx, 1111100000b
        jna     rstep
        mov     cx, 1111100000b
    rstep:
        or      ax, cx
        mov     [edi], ax
    skip:
        add     edi, 2
        dec     lwidth
        jnz     plot
        pop     lwidth
        pop     edx
        pop     ebx
        add     edi, ebx
        add     esi, edx
        dec     lheight
        jnz     next
    }
}

void putSpriteSub32(int32_t x1, int32_t y1, uint32_t key, GFX_IMAGE *img)
{
    int32_t x2, y2, lwidth, lheight;
    int32_t lminX, lminY, lmaxX, lmaxY;
    
    void *imgData = img->mData;
    int32_t imgWidth  = img->mWidth;
    int32_t imgHeight = img->mHeight;

    // Calculate new position
    x2 = x1 + (imgWidth - 1);
    y2 = y1 + (imgHeight - 1);

    // Clip image to context boundaries
    lminX = max(x1, cminX);
    lminY = max(y1, cminY);
    lmaxX = min(x2, cmaxX);
    lmaxY = min(y2, cmaxY);

    // Initialize loop variables
    lwidth  = (lmaxX - lminX) + 1;
    lheight = (lmaxY - lminY) + 1;

    // Check range
    if (lwidth <= 0 || lheight <= 0) return;

#ifdef _USE_MMX
    __asm {
        mov         edi, lfbPtr
        mov         eax, lminY
        add         eax, pageOffset
        mul         lfbWidth
        add         eax, lminX
        shl         eax, 2
        add         edi, eax
        mov         esi, imgData
        mov         eax, lminY
        sub         eax, y1
        mul         imgWidth
        mov         ebx, lminX
        sub         ebx, x1
        add         eax, ebx
        shl         eax, 2
        add         esi, eax
        mov         ebx, lfbWidth
        sub         ebx, lwidth
        shl         ebx, 2
        mov         edx, imgWidth
        sub         edx, lwidth
        shl         edx, 2
        movd        mm4, key
        punpckldq   mm4, mm4
        mov         eax, 0ffffffffh
        movd        mm5, eax
        punpckldq   mm5, mm5
        mov         eax, 0ffffffh
        movd        mm6, eax
        punpckldq   mm6, mm6
    next:
        mov         ecx, lwidth
        shr         ecx, 1
        jz          once
    plot:
        movq        mm0, [esi]
        movq        mm2, mm4
        pand        mm0, mm6
        pcmpeqd     mm2, mm0
        movq        mm1, [edi]
        pand        mm1, mm2
        pxor        mm2, mm5
        pand        mm0, mm2
        psubusb     mm0, [edi]
        por         mm0, mm1
        movq        [edi], mm0
        add         esi, 8
        add         edi, 8
        dec         ecx
        jnz         plot
    once:
        test        lwidth, 1
        jz          skip
        add         esi, 4
        add         edi, 4
        mov         eax, [esi]
        cmp         eax, key
        je          skip
        movd        mm3, eax
        psubusb     mm3, [edi - 4]
        movd        [edi - 4], mm3
    skip:
        add         edi, ebx
        add         esi, edx
        dec         lheight
        jnz         next
        emms    
    }
#else    
    __asm {
        mov         edi, lfbPtr
        mov         eax, lminY
        add         eax, pageOffset
        mul         lfbWidth
        add         eax, lminX
        shl         eax, 2
        add         edi, eax
        mov         esi, imgData
        mov         eax, lminY
        sub         eax, y1
        mul         imgWidth
        mov         ebx, lminX
        sub         ebx, x1
        add         eax, ebx
        shl         eax, 2
        add         esi, eax
        mov         ebx, lfbWidth
        sub         ebx, lwidth
        shl         ebx, 2
        mov         edx, imgWidth
        sub         edx, lwidth
        shl         edx, 2
    next:
        mov         ecx, lwidth
        push        ebx
    plot:
        mov         eax, [esi]
        and         eax, 00FFFFFFh
        cmp         eax, key
        je          skip
        mov         eax, [edi]
        sub         al, [esi]
        jnc         bstep
        xor         al, al
    bstep:
        sub         ah, [esi + 1]
        jnc         gstep
        xor         ah, ah
    gstep:
        mov         ebx, eax
        shr         ebx, 16
        sub         bl, [esi + 2]
        jnc         rstep
        xor         bl, bl
    rstep:
        shl         ebx, 16
        and         eax, 00FFFFh
        or          eax, ebx
        mov         [edi], eax
    skip:
        add         edi, 4
        add         esi, 4
        dec         ecx
        jnz         plot
        pop         ebx
        add         edi, ebx
        add         esi, edx
        dec         lheight
        jnz         next
    }
#endif
}

void putSpriteSub24(int32_t x1, int32_t y1, uint32_t key, GFX_IMAGE *img)
{
    int32_t x2, y2, lwidth, lheight;
    int32_t lminX, lminY, lmaxX, lmaxY;
    
    void *imgData = img->mData;
    int32_t imgWidth  = img->mWidth;
    int32_t imgHeight = img->mHeight;

    // Calculate new position
    x2 = x1 + (imgWidth - 1);
    y2 = y1 + (imgHeight - 1);

    // Clip image to context boundaries
    lminX = max(x1, cminX);
    lminY = max(y1, cminY);
    lmaxX = min(x2, cmaxX);
    lmaxY = min(y2, cmaxY);

    // Initialize loop variables
    lwidth  = (lmaxX - lminX) + 1;
    lheight = (lmaxY - lminY) + 1;

    // Check range
    if (lwidth <= 0 || lheight <= 0) return;

    __asm {
        mov     edi, lfbPtr
        mov     eax, lminY
        add     eax, pageOffset
        mul     lfbWidth
        add     eax, lminX
        mov     ebx, eax
        shl     eax, 1
        add     eax, ebx
        add     edi, eax
        mov     esi, imgData
        mov     eax, lminY
        sub     eax, y1
        mul     imgWidth
        mov     ebx, lminX
        sub     ebx, x1
        add     eax, ebx
        mov     ebx, eax
        shl     eax, 1
        add     eax, ebx
        add     esi, eax
        mov     ebx, lfbWidth
        sub     ebx, lwidth
        mov     eax, ebx
        shl     ebx, 1
        add     ebx, eax
        mov     edx, imgWidth
        sub     edx, lwidth
        mov     ecx, edx
        shl     edx, 1
        add     edx, ecx
    next:
        push    ebx
        mov     ecx, lwidth
    plot:
        mov     ax, [esi]
        mov     bl, [esi + 2]
        shl     ebx, 16
        or      eax, ebx
        and     eax, 00FFFFFFh
        cmp     eax, key
        je      skip
        mov     ax, [edi]
        sub     al, [esi]
        jnc     bstep
        xor     al, al
    bstep:
        sub     ah, [esi + 1]
        jnc     gstep
        xor     ah, ah
    gstep:
        mov     bl, [edi + 2]
        sub     bl, [esi + 2]
        jnc     rstep
        xor     bl, bl
    rstep:
        mov     [edi], ax
        mov     [edi + 2], bl
    skip:
        add     edi, 3
        add     esi, 3
        dec     ecx
        jnz     plot
        pop     ebx
        add     edi, ebx
        add     esi, edx
        dec     lheight
        jnz     next
    }
}

void putSpriteSub16(int32_t x1, int32_t y1, uint32_t key, GFX_IMAGE *img)
{
    int32_t x2, y2, lwidth, lheight;
    int32_t lminX, lminY, lmaxX, lmaxY;
    
    void *imgData = img->mData;
    int32_t imgWidth  = img->mWidth;
    int32_t imgHeight = img->mHeight;

    // Calculate new position
    x2 = x1 + (imgWidth - 1);
    y2 = y1 + (imgHeight - 1);

    // Clip image to context boundaries
    lminX = max(x1, cminX);
    lminY = max(y1, cminY);
    lmaxX = min(x2, cmaxX);
    lmaxY = min(y2, cmaxY);

    // Initialize loop variables
    lwidth  = (lmaxX - lminX) + 1;
    lheight = (lmaxY - lminY) + 1;

    // Check range
    if (lwidth <= 0 || lheight <= 0) return;

    __asm {
        mov     edi, lfbPtr
        mov     eax, lminY
        add     eax, pageOffset
        mul     lfbWidth
        add     eax, lminX
        shl     eax, 1
        add     edi, eax
        mov     esi, imgData
        mov     eax, lminY
        sub     eax, y1
        mul     imgWidth
        mov     ebx, lminX
        sub     ebx, x1
        add     eax, ebx
        shl     eax, 1
        add     esi, eax
        mov     ebx, lfbWidth
        sub     ebx, lwidth
        shl     ebx, 1
        mov     edx, imgWidth
        sub     edx, lwidth
        shl     edx, 1
    next:
        push    ebx
        push    edx
        push    lwidth
    plot:
        mov     ax, [esi]
        cmp     ax, word ptr key
        je      skip
        mov     ax, [edi]
        mov     bx, [esi]
        mov     dx, bx
        and     bx, 1111100000011111b
        and     dx, 11111100000b
        mov     cx, ax
        and     ax, 1111100000011111b
        and     cx, 11111100000b
        sub     al, bl
        jnc     bstep
        xor     al, al
    bstep:
        sub     ah, bh
        jnc     gstep
        xor     ah, ah
    gstep:
        sub     cx, dx
        jc      rstep
        or      ax, cx
    rstep:
        mov     [edi], ax
    skip:
        add     edi, 2
        add     esi, 2
        dec     lwidth
        jnz     plot
        pop     lwidth
        pop     edx
        pop     ebx
        add     edi, ebx
        add     esi, edx
        dec     lheight
        jnz     next
    }
}

void putSpriteSub15(int32_t x1, int32_t y1, uint32_t key, GFX_IMAGE *img)
{
    int32_t x2, y2, lwidth, lheight;
    int32_t lminX, lminY, lmaxX, lmaxY;
    
    void *imgData = img->mData;
    int32_t imgWidth  = img->mWidth;
    int32_t imgHeight = img->mHeight;

    // Calculate new position
    x2 = x1 + (imgWidth - 1);
    y2 = y1 + (imgHeight - 1);

    // Clip image to context boundaries
    lminX = max(x1, cminX);
    lminY = max(y1, cminY);
    lmaxX = min(x2, cmaxX);
    lmaxY = min(y2, cmaxY);

    // Initialize loop variables
    lwidth  = (lmaxX - lminX) + 1;
    lheight = (lmaxY - lminY) + 1;

    // Check range
    if (lwidth <= 0 || lheight <= 0) return;

    __asm {
        mov     edi, lfbPtr
        mov     eax, lminY
        add     eax, pageOffset
        mul     lfbWidth
        add     eax, lminX
        shl     eax, 1
        add     edi, eax
        mov     esi, imgData
        mov     eax, lminY
        sub     eax, y1
        mul     imgWidth
        mov     ebx, lminX
        sub     ebx, x1
        add     eax, ebx
        shl     eax, 1
        add     esi, eax
        mov     ebx, lfbWidth
        sub     ebx, lwidth
        shl     ebx, 1
        mov     edx, imgWidth
        sub     edx, lwidth
        shl     edx, 1
    next:
        push    ebx
        push    edx
        push    lwidth
    plot:
        mov     ax, [esi]
        cmp     ax, word ptr key
        je      skip
        mov     ax, [edi]
        mov     bx, [esi]
        mov     dx, bx
        and     bx, 111110000011111b
        and     dx, 1111100000b
        mov     cx, ax
        and     ax, 111110000011111b
        and     cx, 1111100000b
        sub     al, bl
        jnc     bstep
        xor     al, al
    bstep:
        sub     ah, bh
        jnc     gstep
        xor     ah, ah
    gstep:
        sub     cx, dx
        jc      rstep
        or      ax, cx
    rstep:
        mov     [edi], ax
    skip:
        add     edi, 2
        add     esi, 2
        dec     lwidth
        jnz     plot
        pop     lwidth
        pop     edx
        pop     ebx
        add     edi, ebx
        add     esi, edx
        dec     lheight
        jnz     next
    }
}

// Draw line functions
void horizLine8(int32_t x, int32_t y, int32_t sx, uint32_t col)
{
	//check for clip-y
	if (y > cmaxY || y < cminY) return;
	if (x > cmaxX || sx <= 0) return;

	// check clip boundary
	if (x < cminX)
	{
		// re-calculate sx
		sx -= (cminX - x) + 1;
		x = cminX;
	}

	// inbound check
	if (sx > cmaxX - x) sx = (cmaxX - x) + 1;
	if (sx <= 0) return;

#ifdef _USE_MMX
    __asm {
        mov         eax, y
        add         eax, pageOffset
        mul         lfbWidth
        add         eax, x
        mov         edi, lfbPtr
        add         edi, eax
        mov         eax, col
        mov         ecx, sx
        shr         ecx, 3
        jz          once
        movd        mm0, eax
        punpcklbw   mm0, mm0
        punpcklwd   mm0, mm0
        punpckldq   mm0, mm0
    plot:
        movq        [edi], mm0
        add         edi, 8
        dec         ecx
        jnz         plot
    once:
        mov         ecx, sx
        and         ecx, 7
        jz          end
        rep         stosb
    end:
        emms
    }
#else
    __asm {
        mov         eax, y
        add         eax, pageOffset
        mul         lfbWidth
        add         eax, x
        mov         edi, lfbPtr
        add         edi, eax
        mov         edx, sx
        shr         edx, 2
        mov         ebx, sx
        and         ebx, 3
        mov         eax, col
        mov         ah, al
        mov         ecx, eax
        shl         eax, 16
        or          eax, ecx
        mov         ecx, edx
        rep         stosd
        mov         ecx, ebx
        rep         stosb
    }
#endif
}

void horizLine1516(int32_t x, int32_t y, int32_t sx, uint32_t col)
{
	//check for clip-y
	if (y > cmaxY || y < cminY) return;
	if (x > cmaxX || sx <= 0) return;

	// check clip boundary
	if (x < cminX)
	{
		// re-calculate sx
		sx -= (cminX - x) + 1;
		x = cminX;
	}

	// inbound check
	if (sx > cmaxX - x) sx = (cmaxX - x) + 1;
	if (sx <= 0) return;

#ifdef _USE_MMX
    __asm {
        mov         eax, y
        add         eax, pageOffset
        mul         lfbWidth
        add         eax, x
        shl         eax, 1
        mov         edi, lfbPtr
        add         edi, eax
        mov         eax, col
        mov         ecx, sx
        shr         ecx, 2
        jz          once
        movd        mm0, eax
        punpcklwd   mm0, mm0
        punpckldq   mm0, mm0
    plot:
        movq        [edi], mm0
        add         edi, 8
        dec         ecx
        jnz         plot
    once:
        mov         ecx, sx
        and         ecx, 3
        jz          end
        rep         stosw
    end:
        emms
    }
#else
    __asm {
        mov         eax, y
        add         eax, pageOffset
        mul         lfbWidth
        add         eax, x
        shl         eax, 1
        mov         edi, lfbPtr
        add         edi, eax
        mov         edx, sx
        shr         edx, 1
        mov         ebx, sx
        and         ebx, 1
        mov         eax, col
        shl         eax, 16
        or          eax, col
        mov         ecx, edx
        rep         stosd
        mov         ecx, ebx
        rep         stosw
    }
#endif
}

void horizLine24(int32_t x, int32_t y, int32_t sx, uint32_t col)
{
	//check for clip-y
	if (y > cmaxY || y < cminY) return;
	if (x > cmaxX || sx <= 0) return;

	// check clip boundary
	if (x < cminX)
	{
		// re-calculate sx
		sx -= (cminX - x) + 1;
		x = cminX;
	}

	// inbound check
	if (sx > cmaxX - x) sx = (cmaxX - x) + 1;
	if (sx <= 0) return;

    __asm {
        mov     eax, y
        add     eax, pageOffset
        mul     lfbWidth
        add     eax, x
        mov     ebx, eax
        shl     eax, 1
        add     eax, ebx
        mov     edi, lfbPtr
        add     edi, eax
        mov     ecx, sx
        mov     eax, col
    step:
        mov     [edi], eax
        add     edi, 3
        dec     ecx
        jnz     step
    }
}

void horizLine32(int32_t x, int32_t y, int32_t sx, uint32_t col)
{
	//check for clip-y
	if (y > cmaxY || y < cminY) return;
	if (x > cmaxX || sx <= 0) return;

	// check clip boundary
	if (x < cminX)
	{
		// re-calculate sx
		sx -= (cminX - x) + 1;
		x = cminX;
	}

	// inbound check
	if (sx > cmaxX - x) sx = (cmaxX - x) + 1;
	if (sx <= 0) return;

#ifdef _USE_MMX
    __asm {
        mov         eax, y
        add         eax, pageOffset
        mul         lfbWidth
        add         eax, x
        shl         eax, 2
        mov         edi, lfbPtr
        add         edi, eax
        movd        mm0, col
        mov         ecx, sx
        shr         ecx, 1
        jz          once
        punpckldq   mm0, mm0
    plot:
        movq        [edi], mm0
        add         edi, 8
        dec         ecx
        jnz         plot
    once:
        test        sx, 1
        jz          end
        movd        [edi], mm0
    end:
        emms
    }
#else
    __asm {
        mov         eax, y
        add         eax, pageOffset
        mul         lfbWidth
        add         eax, x
        shl         eax, 2
        mov         edi, lfbPtr
        add         edi, eax
        mov         ecx, sx
        mov         eax, col
        rep         stosd
    }
#endif    
}

void horizLineAdd32(int32_t x, int32_t y, int32_t sx, uint32_t col)
{
	//check for clip-y
	if (y > cmaxY || y < cminY) return;
	if (x > cmaxX || sx <= 0) return;

	// check clip boundary
	if (x < cminX)
	{
		// re-calculate sx
		sx -= (cminX - x) + 1;
		x = cminX;
	}

	// inbound check
	if (sx > cmaxX - x) sx = (cmaxX - x) + 1;
	if (sx <= 0) return;

#ifdef _USE_MMX
    __asm {
        mov         eax, y
        add         eax, pageOffset
        mul         lfbWidth
        add         eax, x
        shl         eax, 2
        mov         edi, lfbPtr
        add         edi, eax
        movd        mm1, col
        mov         ecx, sx
        shr         ecx, 1
        jz          once
        punpckldq   mm1, mm1
    next:
        movq        mm0, [edi]
        paddusb     mm0, mm1
        movq        [edi], mm0
        add         edi, 8
        dec         ecx
        jnz         next
    once:
        test        sx, 1
        jz          end
        movd        mm0, [edi]
        paddusb     mm0, mm1
        movd        [edi], mm0
    end:
        emms
    }
#else
    __asm {
        mov         eax, y
        add         eax, pageOffset
        mul         lfbWidth
        add         eax, x
        shl         eax, 2
        mov         edi, lfbPtr
        add         edi, eax
        mov         ecx, sx
    next:
        mov         eax, [edi]
        add         al, byte ptr col
        jnc         bstep
        mov         al, 255
    bstep:
        add         ah, byte ptr col[1]
        jnc         gstep
        mov         ah, 255
    gstep:
        mov         ebx, eax
        shr         ebx, 16
        add         bl, byte ptr col[2]
        jnc         rstep
        mov         bl, 255
    rstep:
        shl         ebx, 16
        and         eax, 00FFFFh
        or          eax, ebx
        stosd
        dec         ecx
        jnz         next
    }
#endif
}

void horizLineAdd24(int32_t x, int32_t y, int32_t sx, uint32_t col)
{
	//check for clip-y
	if (y > cmaxY || y < cminY) return;
	if (x > cmaxX || sx <= 0) return;

	// check clip boundary
	if (x < cminX)
	{
		// re-calculate sx
		sx -= (cminX - x) + 1;
		x = cminX;
	}

	// inbound check
	if (sx > cmaxX - x) sx = (cmaxX - x) + 1;
	if (sx <= 0) return;

    __asm {
        mov     eax, y
        add     eax, pageOffset
        mul     lfbWidth
        add     eax, x
        mov     ebx, eax
        shl     eax, 1
        add     eax, ebx
        mov     edi, lfbPtr
        add     edi, eax
        mov     ecx, sx
    next:
        mov     ax, [edi]
        add     al, byte ptr col
        jnc     bstep
        mov     al, 255
    bstep:
        add     ah, byte ptr col[1]
        jnc     gstep
        mov     ah, 255
    gstep:
        mov     bl, [edi + 2]
        add     bl, byte ptr col[2]
        jnc     rstep
        mov     bl, 255
    rstep:
        stosw
        mov     al, bl
        stosb
        dec     ecx
        jnz     next
    }
}

void horizLineAdd16(int32_t x, int32_t y, int32_t sx, uint32_t col)
{
	//check for clip-y
	if (y > cmaxY || y < cminY) return;
	if (x > cmaxX || sx <= 0) return;

	// check clip boundary
	if (x < cminX)
	{
		// re-calculate sx
		sx -= (cminX - x) + 1;
		x = cminX;
	}

	// inbound check
	if (sx > cmaxX - x) sx = (cmaxX - x) + 1;
	if (sx <= 0) return;

    __asm {
        mov     eax, y
        add     eax, pageOffset
        mul     lfbWidth
        add     eax, x
        shl     eax, 1
        mov     edi, lfbPtr
        add     edi, eax
        mov     bx, word ptr col
        mov     dx, bx
        and     bx, 1111100000011111b
        and     dx, 11111100000b
    next:
        mov     ax, [edi]
        mov     cx, ax
        and     ax, 1111100000011111b
        and     cx, 11111100000b
        add     al, bl
        cmp     al, 11111b
        jna     bstep
        mov     al, 11111b
    bstep:
        add     ah, bh
        jnc     gstep
        mov     ah, 11111000b
    gstep:
        add     cx, dx
        cmp     cx, 11111100000b
        jna     rstep
        mov     cx, 11111100000b
    rstep:
        or      ax, cx
        stosw
        dec     sx
        jnz     next
    }
}

void horizLineAdd15(int32_t x, int32_t y, int32_t sx, uint32_t col)
{
	//check for clip-y
	if (y > cmaxY || y < cminY) return;
	if (x > cmaxX || sx <= 0) return;

	// check clip boundary
	if (x < cminX)
	{
		// re-calculate sx
		sx -= (cminX - x) + 1;
		x = cminX;
	}

	// inbound check
	if (sx > cmaxX - x) sx = (cmaxX - x) + 1;
	if (sx <= 0) return;

    __asm {
        mov     eax, y
        add     eax, pageOffset
        mul     lfbWidth
        add     eax, x
        shl     eax, 1
        mov     edi, lfbPtr
        add     edi, eax
        mov     bx, word ptr col
        mov     dx, bx
        and     bx, 111110000011111b
        and     dx, 1111100000b
    next:
        mov     ax, [edi]
        mov     cx, ax
        and     ax, 111110000011111b
        and     cx, 1111100000b
        add     al, bl
        cmp     al, 11111b
        jna     bstep
        mov     al, 11111b
    bstep:
        add     ah, bh
        cmp     ah, 1111100b
        jna     gstep
        mov     ah, 1111100b
    gstep:
        add     cx, dx
        cmp     cx, 1111100000b
        jna     rstep
        mov     cx, 1111100000b
    rstep:
        or      ax, cx
        stosw
        dec     sx
        jnz     next
    }
}

void horizLineSub32(int32_t x, int32_t y, int32_t sx, uint32_t col)
{
	//check for clip-y
	if (y > cmaxY || y < cminY) return;
	if (x > cmaxX || sx <= 0) return;

	// check clip boundary
	if (x < cminX)
	{
		// re-calculate sx
		sx -= (cminX - x) + 1;
		x = cminX;
	}

	// inbound check
	if (sx > cmaxX - x) sx = (cmaxX - x) + 1;
	if (sx <= 0) return;

#ifdef _USE_MMX
    __asm {
        mov         eax, y
        add         eax, pageOffset
        mul         lfbWidth
        add         eax, x
        shl         eax, 2
        mov         edi, lfbPtr
        add         edi, eax
        movd        mm1, col
        mov         ecx, sx
        shr         ecx, 1
        jz          once
        punpckldq   mm1, mm1
    next:
        movq        mm0, [edi]
        psubusb     mm0, mm1
        movq        [edi], mm0
        add         edi, 8
        dec         ecx
        jnz         next
    once:
        test        sx, 1
        jz          end
        movd        mm0, [edi]
        psubusb     mm0, mm1
        movd        [edi], mm0
    end:
        emms
    }
#else
    __asm {
        mov         eax, y
        add         eax, pageOffset
        mul         lfbWidth
        add         eax, x
        shl         eax, 2
        mov         edi, lfbPtr
        add         edi, eax
        mov         ecx, sx
    next:
        mov         eax, [edi]
        sub         al, byte ptr col
        jnc         bstep
        xor         al, al
    bstep:
        sub         ah, byte ptr col[1]
        jnc         gstep
        xor         ah, ah
    gstep:
        mov         ebx, eax
        shr         ebx, 16
        sub         bl, byte ptr col[2]
        jnc         rstep
        xor         bl, bl
    rstep:
        shl         ebx, 16
        and         eax, 00FFFFh
        or          eax, ebx
        stosd
        dec         ecx
        jnz         next
    }
#endif
}

void horizLineSub24(int32_t x, int32_t y, int32_t sx, uint32_t col)
{
	//check for clip-y
	if (y > cmaxY || y < cminY) return;
	if (x > cmaxX || sx <= 0) return;

	// check clip boundary
	if (x < cminX)
	{
		// re-calculate sx
		sx -= (cminX - x) + 1;
		x = cminX;
	}

	// inbound check
	if (sx > cmaxX - x) sx = (cmaxX - x) + 1;
	if (sx <= 0) return;

    __asm {
        mov     eax, y
        add     eax, pageOffset
        mul     lfbWidth
        add     eax, x
        mov     ebx, eax
        shl     eax, 1
        add     eax, ebx
        mov     edi, lfbPtr
        add     edi, eax
        mov     ecx, sx
    next:
        mov     ax, [edi]
        sub     al, byte ptr col
        jnc     bstep
        xor     al, al
    bstep:
        sub     ah, byte ptr col[1]
        jnc     gstep
        xor     ah, ah
    gstep:
        mov     bl, [edi + 2]
        sub     bl, byte ptr col[2]
        jnc     rstep
        xor     bl, bl
    rstep:
        stosw
        mov     al, bl
        stosb
        dec     ecx
        jnz     next
    }
}

void horizLineSub16(int32_t x, int32_t y, int32_t sx, uint32_t col)
{
	//check for clip-y
	if (y > cmaxY || y < cminY) return;
	if (x > cmaxX || sx <= 0) return;

	// check clip boundary
	if (x < cminX)
	{
		// re-calculate sx
		sx -= (cminX - x) + 1;
		x = cminX;
	}

	// inbound check
	if (sx > cmaxX - x) sx = (cmaxX - x) + 1;
	if (sx <= 0) return;

    __asm {
        mov     eax, y
        add     eax, pageOffset
        mul     lfbWidth
        add     eax, x
        shl     eax, 1
        mov     edi, lfbPtr
        add     edi, eax
        mov     bx, word ptr col
        mov     dx, bx
        and     bx, 1111100000011111b
        and     dx, 11111100000b
    next:
        mov     ax, [edi]
        mov     cx, ax
        and     ax, 1111100000011111b
        and     cx, 11111100000b
        sub     al, bl
        jnc     bstep
        xor     al, al
    bstep:
        sub     ah, bh
        jnc     gstep
        xor     ah, ah
    gstep:
        sub     cx, dx
        jc      rstep
        or      ax, cx
    rstep:      
        stosw
        dec     sx
        jnz     next
    }
}

void horizLineSub15(int32_t x, int32_t y, int32_t sx, uint32_t col)
{
	//check for clip-y
	if (y > cmaxY || y < cminY) return;
	if (x > cmaxX || sx <= 0) return;

	// check clip boundary
	if (x < cminX)
	{
		// re-calculate sx
		sx -= (cminX - x) + 1;
		x = cminX;
	}

	// inbound check
	if (sx > cmaxX - x) sx = (cmaxX - x) + 1;
	if (sx <= 0) return;

    __asm {
        mov     eax, y
        add     eax, pageOffset
        mul     lfbWidth
        add     eax, x
        shl     eax, 1
        mov     edi, lfbPtr
        add     edi, eax
        mov     bx, word ptr col
        mov     dx, bx
        and     bx, 111110000011111b
        and     dx, 1111100000b
    next:
        mov     ax, [edi]
        mov     cx, ax
        and     ax, 111110000011111b
        and     cx, 1111100000b
        sub     al, bl
        jnc     bstep
        xor     al, al
    bstep:
        sub     ah, bh
        jnc     gstep
        xor     ah, ah
    gstep:
        sub     cx, dx
        jc      rstep
        or      ax, cx
    rstep:
        stosw
        dec     sx
        jnz     next
    }
}

void vertLine8(int32_t x, int32_t y, int32_t sy, uint32_t col)
{
	//check for clip-x
	if (x > cmaxX || x < cminX) return;
	if (y > cmaxY || sy <= 0) return;

	if (y < cminY)
	{
		//re-calculate sy
		sy -= (cminY - y) + 1;
		y = cminY;
	}

	//inbound check
	if (sy > cmaxY - y) sy = (cmaxY - y) + 1;
	if (sy <= 0) return;

    __asm {
        mov     eax, y
        add     eax, pageOffset
        mul     lfbWidth
        add     eax, x
        mov     edi, lfbPtr
        add     edi, eax
        mov     ecx, sy
        mov     al, byte ptr col
    next:
        stosb
        add     edi, cmaxX
        dec     ecx
        jnz     next
    }
}

void vertLine1516(int32_t x, int32_t y, int32_t sy, uint32_t col)
{
	//check for clip-x
	if (x > cmaxX || x < cminX) return;
	if (y > cmaxY || sy <= 0) return;

	if (y < cminY)
	{
		//re-calculate sy
		sy -= (cminY - y) + 1;
		y = cminY;
	}

	//inbound check
	if (sy > cmaxY - y) sy = (cmaxY - y) + 1;
	if (sy <= 0) return;

    __asm {
        mov     eax, y
        add     eax, pageOffset
        mul     lfbWidth
        add     eax, x
        shl     eax, 1
        mov     edi, lfbPtr
        add     edi, eax
        mov     ecx, sy
        mov     ebx, cmaxX
        shl     ebx, 1
        mov     ax, word ptr col
    next:
        stosw
        add     edi, ebx
        dec     ecx
        jnz     next
    }
}

void vertLine24(int32_t x, int32_t y, int32_t sy, uint32_t col)
{
	//check for clip-x
	if (x > cmaxX || x < cminX) return;
	if (y > cmaxY || sy <= 0) return;

	if (y < cminY)
	{
		//re-calculate sy
		sy -= (cminY - y) + 1;
		y = cminY;
	}

	//inbound check
	if (sy > cmaxY - y) sy = (cmaxY - y) + 1;
	if (sy <= 0) return;

    __asm {
        mov     eax, y
        add     eax, pageOffset
        mul     lfbWidth
        add     eax, x
        mov     ebx, eax
        shl     eax, 1
        add     eax, ebx
        mov     edi, lfbPtr
        add     edi, eax
        mov     ecx, sy
        mov     ebx, cmaxX
        shl     ebx, 1
        add     ebx, cmaxX
    next:
        mov     eax, col
        stosw
        shr     eax, 16
        stosb
        add     edi, ebx
        dec     ecx
        jnz     next
    }
}

void vertLine32(int32_t x, int32_t y, int32_t sy, uint32_t col)
{
	//check for clip-x
	if (x > cmaxX || x < cminX) return;
	if (y > cmaxY || sy <= 0) return;

	if (y < cminY)
	{
		//re-calculate sy
		sy -= (cminY - y) + 1;
		y = cminY;
	}

	//inbound check
	if (sy > cmaxY - y) sy = (cmaxY - y) + 1;
	if (sy <= 0) return;

    __asm {
        mov     eax, y
        add     eax, pageOffset
        mul     lfbWidth
        add     eax, x
        shl     eax, 2
        mov     edi, lfbPtr
        add     edi, eax
        mov     ecx, sy
        mov     ebx, cmaxX
        shl     ebx, 2
        mov     eax, col
    next:
        stosd
        add     edi, ebx
        dec     ecx
        jnz     next
    }
}

void vertLineAdd32(int32_t x, int32_t y, int32_t sy, uint32_t col)
{
 	//check for clip-x
	if (x > cmaxX || x < cminX) return;
	if (y > cmaxY || sy <= 0) return;

	if (y < cminY)
	{
		//re-calculate sy
		sy -= (cminY - y) + 1;
		y = cminY;
	}

	//inbound check
	if (sy > cmaxY - y) sy = (cmaxY - y) + 1;
	if (sy <= 0) return;

    __asm {
        mov     eax, y
        add     eax, pageOffset
        mul     lfbWidth
        add     eax, x
        shl     eax, 2
        mov     edi, lfbPtr
        add     edi, eax
        mov     ecx, sy
        mov     ebx, cmaxX
        shl     ebx, 2
    next:
        mov     eax, [edi]
        add     al, byte ptr col
        jnc     bstep
        mov     al, 255
    bstep:
        add     ah, byte ptr col[1]
        jnc     gstep
        mov     ah, 255
    gstep:
        mov     edx, eax
        shr     edx, 16
        add     dl, byte ptr col[2]
        jnc     rstep
        mov     dl, 255
    rstep:
        shl     edx, 16
        and     eax, 00FFFFh
        or      eax, edx
        stosd 
        add     edi, ebx 
        dec     ecx
        jnz     next
    }
}

void vertLineAdd24(int32_t x, int32_t y, int32_t sy, uint32_t col)
{
	//check for clip-x
	if (x > cmaxX || x < cminX) return;
	if (y > cmaxY || sy <= 0) return;

	if (y < cminY)
	{
		//re-calculate sy
		sy -= (cminY - y) + 1;
		y = cminY;
	}

	//inbound check
	if (sy > cmaxY - y) sy = (cmaxY - y) + 1;
	if (sy <= 0) return;

    __asm {
        mov     eax, y
        add     eax, pageOffset
        mul     lfbWidth
        add     eax, x
        mov     ebx, eax
        shl     eax, 1
        add     eax, ebx
        mov     edi, lfbPtr
        add     edi, eax
        mov     ecx, sy
        mov     ebx, cmaxX
        shl     ebx, 1
        add     ebx, cmaxX
    next:
        mov     ax, [edi]
        add     al, byte ptr col
        jnc     bstep
        mov     al, 255
    bstep:
        add     ah, byte ptr col[1]
        jnc     gstep
        mov     ah, 255
    gstep:
        mov     dl, [edi + 2]
        add     dl, byte ptr col[2]
        jnc     rstep
        mov     dl, 255
    rstep:
        stosw
        mov     al, dl
        stosb
        add     edi, ebx 
        dec     ecx
        jnz     next
    }
}

void vertLineAdd16(int32_t x, int32_t y, int32_t sy, uint32_t col)
{
	//check for clip-x
	if (x > cmaxX || x < cminX) return;
	if (y > cmaxY || sy <= 0) return;

	if (y < cminY)
	{
		//re-calculate sy
		sy -= (cminY - y) + 1;
		y = cminY;
	}

	//inbound check
	if (sy > cmaxY - y) sy = (cmaxY - y) + 1;
	if (sy <= 0) return;

    __asm {
        mov     eax, y
        add     eax, pageOffset
        mul     lfbWidth
        add     eax, x
        shl     eax, 1
        mov     edi, lfbPtr
        add     edi, eax
        mov     ebx, cmaxX
        shl     ebx, 1
        push    ebx
        mov     bx, word ptr col
        mov     dx, bx
        and     bx, 1111100000011111b
        and     dx, 11111100000b
    next:
        mov     ax, [edi]
        mov     cx, ax
        and     ax, 1111100000011111b
        and     cx, 11111100000b
        add     al, bl
        cmp     al, 11111b
        jna     bstep
        mov     al, 11111b
    bstep:
        add     ah, bh
        jnc     gstep
        mov     ah, 11111000b
    gstep:
        add     cx, dx
        cmp     cx, 11111100000b
        jna     rstep
        mov     cx, 11111100000b
    rstep:
        or      ax, cx
        stosw
        add     edi, [esp]
        dec     sy 
        jnz     next
        pop     ebx
    }
}

void vertLineAdd15(int32_t x, int32_t y, int32_t sy, uint32_t col)
{
	//check for clip-x
	if (x > cmaxX || x < cminX) return;
	if (y > cmaxY || sy <= 0) return;

	if (y < cminY)
	{
		//re-calculate sy
		sy -= (cminY - y) + 1;
		y = cminY;
	}

	//inbound check
	if (sy > cmaxY - y) sy = (cmaxY - y) + 1;
	if (sy <= 0) return;

    __asm {
        mov     eax, y
        add     eax, pageOffset
        mul     lfbWidth
        add     eax, x
        shl     eax, 1
        mov     edi, lfbPtr
        add     edi, eax
        mov     ebx, cmaxX
        shl     ebx, 1
        push    ebx
        mov     bx, word ptr col
        mov     dx, bx
        and     bx, 111110000011111b
        and     dx, 1111100000b
    next:
        mov     ax, [edi]
        mov     cx, ax
        and     ax, 111110000011111b
        and     cx, 1111100000b
        add     al, bl
        cmp     al, 11111b
        jna     bstep
        mov     al, 11111b
    bstep:
        add     ah, bh
        cmp     ah, 1111100b
        jna     gstep
        mov     ah, 1111100b
    gstep:
        add     cx, dx
        cmp     cx, 1111100000b
        jna     rstep
        mov     cx, 1111100000b
    rstep:
        or      ax, cx
        stosw
        add     edi, [esp]
        dec     sy 
        jnz     next
        pop     ebx
    }
}

void vertLineSub32(int32_t x, int32_t y, int32_t sy, uint32_t col)
{
	//check for clip-x
	if (x > cmaxX || x < cminX) return;
	if (y > cmaxY || sy <= 0) return;

	if (y < cminY)
	{
		//re-calculate sy
		sy -= (cminY - y) + 1;
		y = cminY;
	}

	//inbound check
	if (sy > cmaxY - y) sy = (cmaxY - y) + 1;
	if (sy <= 0) return;

    __asm {
        mov     eax, y
        add     eax, pageOffset
        mul     lfbWidth
        add     eax, x
        shl     eax, 2
        mov     edi, lfbPtr
        add     edi, eax
        mov     ecx, sy
        mov     ebx, cmaxX
        shl     ebx, 2
    next:
        mov     eax, [edi]
        sub     al, byte ptr col
        jnc     bstep
        xor     al, al
    bstep:
        sub     ah, byte ptr col[1]
        jnc     gstep
        xor     ah, ah
    gstep:
        mov     edx, eax
        shr     edx, 16
        sub     dl, byte ptr col[2]
        jnc     rstep
        xor     dl, dl
    rstep:
        shl     edx, 16
        and     eax, 00FFFFh
        or      eax, edx
        stosd 
        add     edi, ebx 
        dec     ecx
        jnz     next
    }
}

void vertLineSub24(int32_t x, int32_t y, int32_t sy, uint32_t col)
{
	//check for clip-x
	if (x > cmaxX || x < cminX) return;
	if (y > cmaxY || sy <= 0) return;

	if (y < cminY)
	{
		//re-calculate sy
		sy -= (cminY - y) + 1;
		y = cminY;
	}

	//inbound check
	if (sy > cmaxY - y) sy = (cmaxY - y) + 1;
	if (sy <= 0) return;

    __asm {
        mov     eax, y
        add     eax, pageOffset
        mul     lfbWidth
        add     eax, x
        mov     ebx, eax
        shl     eax, 1
        add     eax, ebx
        mov     edi, lfbPtr
        add     edi, eax
        mov     ecx, sy
        mov     ebx, cmaxX
        shl     ebx, 1
        add     ebx, cmaxX
    next:
        mov     ax, [edi]
        sub     al, byte ptr col
        jnc     bstep
        xor     al, al
    bstep:
        sub     ah, byte ptr col[1]
        jnc     gstep
        xor     ah, ah
    gstep:
        mov     dl, [edi + 2]
        sub     dl, byte ptr col[2]
        jnc     rstep
        xor     dl, dl
    rstep:
        stosw
        mov     al, dl
        stosb
        add     edi, ebx 
        dec     ecx
        jnz     next
    }
}

void vertLineSub16(int32_t x, int32_t y, int32_t sy, uint32_t col)
{
	//check for clip-x
	if (x > cmaxX || x < cminX) return;
	if (y > cmaxY || sy <= 0) return;

	if (y < cminY)
	{
		//re-calculate sy
		sy -= (cminY - y) + 1;
		y = cminY;
	}

	//inbound check
	if (sy > cmaxY - y) sy = (cmaxY - y) + 1;
	if (sy <= 0) return;

    __asm {
        mov     eax, y
        add     eax, pageOffset
        mul     lfbWidth
        add     eax, x
        shl     eax, 1
        mov     edi, lfbPtr
        add     edi, eax
        mov     ebx, cmaxX
        shl     ebx, 1
        push    ebx
        mov     bx, word ptr col
        mov     dx, bx
        and     bx, 1111100000011111b
        and     dx, 11111100000b
    next:
        mov     ax, [edi]
        mov     cx, ax
        and     ax, 1111100000011111b
        and     cx, 11111100000b
        sub     al, bl
        jnc     bstep
        xor     al, al
    bstep:
        sub     ah, bh
        jnc     gstep
        xor     ah, ah
    gstep:
        sub     cx, dx
        jc      rstep
        or      ax, cx
    rstep:
        stosw
        add     edi, [esp]
        dec     sy
        jnz     next
        pop     ebx
    }
}

void vertLineSub15(int32_t x, int32_t y, int32_t sy, uint32_t col)
{
	//check for clip-x
	if (x > cmaxX || x < cminX) return;
	if (y > cmaxY || sy <= 0) return;

	if (y < cminY)
	{
		//re-calculate sy
		sy -= (cminY - y) + 1;
		y = cminY;
	}

	//inbound check
	if (sy > cmaxY - y) sy = (cmaxY - y) + 1;
	if (sy <= 0) return;
    
    __asm {
        mov     eax, y
        add     eax, pageOffset
        mul     lfbWidth
        add     eax, x
        shl     eax, 1
        mov     edi, lfbPtr
        add     edi, eax
        mov     ebx, cmaxX
        shl     ebx, 1
        push    ebx
        mov     bx, word ptr col
        mov     dx, bx
        and     bx, 111110000011111b
        and     dx, 1111100000b
    next:
        mov     ax, [edi]
        mov     cx, ax
        and     ax, 111110000011111b
        and     cx, 1111100000b
        sub     al, bl
        jnc     bstep
        xor     al, al
    bstep:
        sub     ah, bh
        jnc     gstep
        xor     ah, ah
    gstep:
        sub     cx, dx
        jc      rstep
        or      ax, cx
    rstep:
        stosw
        add     edi, [esp]
        dec     sy
        jnz     next
        pop     ebx
    }
}

// Smooth scaling with Bresenham (internal function)
// Because of several simplifications of the algorithm,
// the zoom range is restricted between 0.5 and 2. That
// is: dstwidth must be >= srcwidth/2 and <= 2*srcwidth.
// smooth is used to calculate average pixel and mid-points
void scaleLine8(void *dst, void *src, int32_t dw, int32_t sw, int32_t smooth)
{
    uint16_t val = 0;

    if (smooth)
    {
        __asm {
            pusha
            mov     ebx, dw
            mov     eax, dw
            cmp     eax, sw
            jna     start
            dec     ebx
        start:
            mov     esi, src
            mov     edi, dst
            mov     ecx, ebx
            xor     ebx, ebx
            mov     edx, dw
            shr     edx, 1
        next:
            cmp     ebx, edx
            jnae    quit
            mov     al, [esi]
            mov     val, ax
            mov     al, [esi + 1]
            add     ax, val
            shr     ax, 1
            stosb
            jmp     skip
        quit:
            mov     al, [esi]
            stosb
        skip:
            add     ebx, sw
            cmp     ebx, dw
            jnae    cycle
            sub     ebx, dw
            inc     esi
        cycle:
            loop    next
            mov     eax, dw
            cmp     eax, sw
            jna     end
            movsb
        end:
            popa
        }
    }
    else
    {
        __asm {
            pusha
            xor     edx, edx
            mov     esi, src
            mov     edi, dst
            mov     eax, sw
            mov     ebx, dw
            div     ebx
            xor     ebx, ebx
            mov     ecx, dw
        next:
            movsb
            dec     esi
            add     esi, eax
            add     ebx, edx
            cmp     ebx, dw
            jnae    quit
            sub     ebx, dw
            inc     esi
        quit:
            loop    next
            popa
        }
    }
}

void scaleLine16(void *dst, void *src, int32_t dw, int32_t sw, int32_t smooth)
{
    if (smooth)
    {
        __asm {
            pusha
            mov     ebx, dw
            mov     eax, dw
            cmp     eax, sw
            jna     start
            dec     ebx
        start:
            mov     esi, src
            mov     edi, dst
            mov     ecx, ebx
            xor     ebx, ebx
            mov     edx, dw
            shr     edx, 1
        next:
            cmp     ebx, edx
            jnae    quit
            mov     ax, [esi]
            add     ax, [esi + 2]
            shr     ax, 1
            stosw
            jmp     skip
        quit:
            mov     ax, [esi]
            stosw
        skip:
            add     ebx, sw
            cmp     ebx, dw
            jnae    cycle
            sub     ebx, dw
            add     esi, 2
        cycle:
            loop    next
            mov     eax, dw
            cmp     eax, sw
            jna     end
            movsw
        end:
            popa
        }
    }
    else
    {
        __asm {
            pusha
            xor     edx, edx
            mov     esi, src
            mov     edi, dst
            mov     eax, sw
            mov     ebx, dw
            div     ebx
            shl     eax, 1
            xor     ebx, ebx
            mov     ecx, dw
        next:
            movsw
            sub     esi, 2
            add     esi, eax
            add     ebx, edx
            cmp     ebx, dw
            jnae    quit
            sub     ebx, dw
            add     esi, 2
        quit:
            loop    next
            popa
        }
    }
}

void scaleLine24(void *dst, void *src, int32_t dw, int32_t sw, int32_t smooth)
{
    uint16_t val = 0;

    if (smooth)
    {
        __asm {
            pusha
            mov     ebx, dw
            mov     eax, dw
            cmp     eax, sw
            jna     start
            dec     ebx
        start:
            mov     esi, src
            mov     edi, dst
            mov     ecx, ebx
            xor     ebx, ebx
            mov     edx, dw
            shr     edx, 1
        next:
            cmp     ebx, edx
            jnae    quit
            mov     al, [esi]
            mov     val, ax
            mov     al, [esi + 4]
            add     ax, val
            shr     ax, 1
            stosb
            mov     al, [esi + 1]
            mov     val, ax
            mov     al, [esi + 5]
            add     ax, val
            shr     ax, 1
            stosb
            mov     al, [esi + 2]
            mov     val, ax
            mov     al, [esi + 6]
            add     ax, val
            shr     ax, 1
            stosb
            jmp     skip
        quit:
            mov     eax, [esi]
            stosw
            shr     eax, 16
            stosb
        skip:
            add     ebx, sw
            cmp     ebx, dw
            jnae    cycle
            sub     ebx, dw
            add     esi, 3
        cycle:
            loop    next
            mov     eax, dw
            cmp     eax, sw
            jna     end
            movsw
            movsb
        end:
            popa
        }
    }
    else
    {
        __asm {
            pusha
            xor     edx, edx
            mov     esi, src
            mov     edi, dst
            mov     eax, sw
            mov     ebx, dw
            div     ebx
            xor     ebx, ebx
            mov     ecx, dw
        next:
            movsw
            movsb
            sub     esi, 3
            add     esi, eax
            add     ebx, edx
            cmp     ebx, dw
            jnae    quit
            sub     ebx, dw
            add     esi, 3
        quit:
            loop    next
            popa
        }
    }
}

// Coarse scaling with Bresenham (internal function)
void scaleLine32(void *dst, void *src, int32_t dw, int32_t sw, int32_t smooth)
{
    uint16_t val = 0;

    if (smooth)
    {
        __asm {
            pusha
            mov     ebx, dw
            mov     eax, dw
            cmp     eax, sw
            jna     start
            dec     ebx
        start:
            mov     esi, src
            mov     edi, dst
            mov     ecx, ebx
            xor     ebx, ebx
            mov     edx, dw
            shr     edx, 1
        next:
            cmp     ebx, edx
            jnae    quit
            mov     al, [esi]
            mov     val, ax
            mov     al, [esi + 4]
            add     ax, val
            shr     ax, 1
            stosb
            mov     al, [esi + 1]
            mov     val, ax
            mov     al, [esi + 5]
            add     ax, val
            shr     ax, 1
            stosb
            mov     al, [esi + 2]
            mov     val, ax
            mov     al, [esi + 6]
            add     ax, val
            shr     ax, 1
            stosb
            inc     edi
            jmp     skip
        quit:
            mov     eax, [esi]
            stosd
        skip:
            add     ebx, sw
            cmp     ebx, dw
            jnae    cycle
            sub     ebx, dw
            add     esi, 4
        cycle:
            loop    next
            mov     eax, dw
            cmp     eax, sw
            jna     end
            movsd
        end:
            popa
        }
    }
    else
    {
        __asm {
            pusha
            xor     edx, edx
            mov     esi, src
            mov     edi, dst
            mov     eax, sw
            mov     ebx, dw
            div     ebx
            shl     eax, 2
            xor     ebx, ebx
            mov     ecx, dw
        next:
            movsd
            sub     esi, 4
            add     esi, eax
            add     ebx, edx
            cmp     ebx, dw
            jnae    quit
            sub     ebx, dw
            add     esi, 4
        quit:
            dec     ecx
            jnz     next
            popa
        }
    }
}

// Scale image using Bresenham algorithm 
void scaleImage8(GFX_IMAGE *dst, GFX_IMAGE *src, int32_t smooth)
{
    // save local value
    uint32_t dstWidth  = dst->mWidth;
    uint32_t srcWidth  = src->mWidth;
    uint32_t dstHeight = dst->mHeight;
    uint32_t srcHeight = src->mHeight;
    uint32_t intp = 0, modp = 0;

    // make pointer to call in asm mode
    void *mcopy = copyData;
    void *scaler = scaleLine8;

    // save local address
    void *oldPtr = NULL;
    void *dstPtr = dst->mData;
    void *srcPtr = src->mData;

    __asm {
        mov     edi, dstPtr
        mov     esi, srcPtr
        xor     edx, edx
        mov     eax, srcHeight
        div     dstHeight
        mov     modp, edx
        mul     srcWidth
        mov     intp, eax
        xor     ebx, ebx
        mov     ecx, dstHeight
    next:
        cmp     esi, oldPtr
        jne     skip
        mov     eax, edi
        sub     eax, dstWidth
        push    dstWidth
        push    eax
        push    edi
        call    mcopy
        jmp     quit
    skip:
        mov     edx, smooth
        and     edx, 0Fh
        push    edx
        push    srcWidth
        push    dstWidth
        push    esi
        push    edi
        call    scaler
        mov     oldPtr, esi
    quit:
        add     edi, dstWidth
        add     esi, intp
        add     ebx, modp
        cmp     ebx, dstHeight
        jnae    cycle
        sub     ebx, dstHeight
        add     esi, srcWidth
    cycle:
        dec     ecx
        jnz     next
    }
}

void scaleImage1516(GFX_IMAGE *dst, GFX_IMAGE *src, int32_t smooth)
{
    // save local value
    uint32_t dstWidth  = dst->mWidth;
    uint32_t srcWidth  = src->mWidth;
    uint32_t dstHeight = dst->mHeight;
    uint32_t srcHeight = src->mHeight;

    uint32_t dsi = 0, ddi = 0;
    uint32_t intp = 0, modp = 0;

    // make pointer to call in asm
    void *mcopy = copyData;
    void *scaler = scaleLine16;

    // save local address
    void *oldPtr = NULL;
    void *dstPtr = dst->mData;
    void *srcPtr = src->mData;

    __asm {
        mov     edi, dstPtr
        mov     esi, srcPtr
        xor     edx, edx
        mov     eax, srcHeight
        div     dstHeight
        mov     modp, edx
        mul     srcWidth
        shl     eax, 1
        mov     intp, eax
        mov     eax, dstWidth
        shl     eax, 1
        mov     ddi, eax
        mov     eax, srcWidth
        shl     eax, 1
        mov     dsi, eax
        xor     ebx, ebx
        mov     ecx, dstHeight
    next:
        cmp     esi, oldPtr
        jne     skip
        mov     eax, edi
        sub     eax, ddi
        push    ddi
        push    eax
        push    edi
        call    mcopy
        jmp     quit
    skip:
        mov     edx, smooth
        and     edx, 0Fh
        push    edx
        push    srcWidth
        push    dstWidth
        push    esi
        push    edi
        call    scaler
        mov     oldPtr, esi
    quit:
        add     edi, ddi
        add     esi, intp
        add     ebx, modp
        cmp     ebx, dstHeight
        jnae    cycle
        sub     ebx, dstHeight
        add     esi, dsi
    cycle:
        dec     ecx
        jnz     next
    }
}

void scaleImage24(GFX_IMAGE *dst, GFX_IMAGE *src, int32_t smooth)
{
    // save local value
    uint32_t dstWidth  = dst->mWidth;
    uint32_t srcWidth  = src->mWidth;
    uint32_t dstHeight = dst->mHeight;
    uint32_t srcHeight = src->mHeight;

    uint32_t dsi = 0, ddi = 0;
    uint32_t intp = 0, modp = 0;

    // make pointer to call in asm
    void *mcopy = copyData;
    void *scaler = scaleLine24;

    // save local address
    void *oldPtr = NULL;
    void *dstPtr = dst->mData;
    void *srcPtr = src->mData;

    __asm {
        mov     edi, dstPtr
        mov     esi, srcPtr
        xor     edx, edx
        mov     eax, srcHeight
        div     dstHeight
        mov     modp, edx
        mul     srcWidth
        shl     eax, 1
        add     eax, srcWidth
        mov     intp, eax
        mov     eax, dstWidth
        shl     eax, 1
        add     eax, dstWidth
        mov     ddi, eax
        mov     eax, srcWidth
        shl     eax, 1
        add     eax, srcWidth
        mov     dsi, eax
        xor     ebx, ebx
        mov     ecx, dstHeight
    next:
        cmp     esi, oldPtr
        jne     skip
        mov     eax, edi
        sub     eax, ddi
        push    ddi
        push    eax
        push    edi
        call    mcopy
        jmp     quit
    skip:
        mov     edx, smooth
        and     edx, 0Fh
        push    edx
        push    srcWidth
        push    dstWidth
        push    esi
        push    edi
        call    scaler
        mov     oldPtr, esi
    quit:
        add     edi, ddi
        add     esi, intp
        add     ebx, modp
        cmp     ebx, dstHeight
        jnae    cycle
        sub     ebx, dstHeight
        add     esi, dsi
    cycle:
        dec     ecx
        jnz     next
    }
}

// Scale image using Bresenham algorithm 
void scaleImage32(GFX_IMAGE *dst, GFX_IMAGE *src, int32_t smooth)
{
    // save local value
    uint32_t dstWidth  = dst->mWidth;
    uint32_t srcWidth  = src->mWidth;
    uint32_t dstHeight = dst->mHeight;
    uint32_t srcHeight = src->mHeight;

    uint32_t dsi = 0, ddi = 0;
    uint32_t intp = 0, modp = 0;

    // make pointer to call in asm
    void *mcopy = copyData;
    void *scaler = scaleLine32;

    // save local address
    void *oldPtr = NULL;
    void *dstPtr = dst->mData;
    void *srcPtr = src->mData;

    __asm {
        mov     edi, dstPtr
        mov     esi, srcPtr
        xor     edx, edx
        mov     eax, srcHeight
        div     dstHeight
        mov     modp, edx
        mul     srcWidth
        shl     eax, 2
        mov     intp, eax
        mov     eax, dstWidth
        shl     eax, 2
        mov     ddi, eax
        mov     eax, srcWidth
        shl     eax, 2
        mov     dsi, eax
        xor     ebx, ebx
        mov     ecx, dstHeight
    next:
        cmp     esi, oldPtr
        jne     skip
        mov     eax, edi
        sub     eax, ddi
        push    ddi
        push    eax
        push    edi
        call    mcopy
        jmp     quit
    skip:
        mov     edx, smooth
        and     edx, 0Fh
        push    edx
        push    srcWidth
        push    dstWidth
        push    esi
        push    edi
        call    scaler
        mov     oldPtr, esi
    quit:
        add     edi, ddi
        add     esi, intp
        add     ebx, modp
        cmp     ebx, dstHeight
        jnae    cycle
        sub     ebx, dstHeight
        add     esi, dsi
    cycle:
        dec     ecx
        jnz     next
    }
}

// Check point in boundary
inline int32_t pointInBound(const ROTATE_CLIP* clip, const int32_t scx, const int32_t scy)
{
    return  (((scx >= (-(clip->boundWidth << 16))) && ((scx >> 16) < (clip->srcw + clip->boundWidth))) &&
             ((scy >= (-(clip->boundWidth << 16))) && ((scy >> 16) < (clip->srch + clip->boundWidth))));
}

// Check point in source (excluded boundary)
inline int32_t pointInSrc(const ROTATE_CLIP* clip, const int32_t scx, const int32_t scy)
{
    return  (((scx >= (clip->boundWidth << 16)) && ((scx >> 16) < (clip->srcw - clip->boundWidth))) &&
             ((scy >= (clip->boundWidth << 16)) && ((scy >> 16) < (clip->srch - clip->boundWidth))));
}

// Begin find point
inline void findBeginIn(const ROTATE_CLIP* clip, int32_t* dstx, int32_t* scx, int32_t* scy)
{
    *scx -= clip->ax;
    *scy -= clip->ay;

    while (pointInBound(clip, *scx, *scy))
    {
        (*dstx)--;
        *scx -= clip->ax;
        *scy -= clip->ay;
    }

    *scx += clip->ax;
    *scy += clip->ay;
}

// Begin find point in source
inline int32_t findBegin(ROTATE_CLIP* clip, const int32_t dsty, int32_t* dstx0, const int32_t dstx1)
{
    int32_t i = 0;
    const int32_t testx0 = *dstx0 - 1;
    int32_t scx = clip->ax * testx0 + clip->bx * dsty + clip->cx;
    int32_t scy = clip->ay * testx0 + clip->by * dsty + clip->cy;

    for (i = testx0; i <= dstx1; i++)
    {
        if (pointInBound(clip, scx, scy))
        {
            *dstx0 = i;

            if (i == testx0) findBeginIn(clip, dstx0, &scx, &scy);

            if (*dstx0 < 0)
            {
                scx -= clip->ax * (*dstx0);
                scy -= clip->ay * (*dstx0);
            }

            clip->srcx = scx;
            clip->srcy = scy;

            return 1;
        }
        else
        {
            scx += clip->ax;
            scy += clip->ay;
        }
    }

    return 0;
}

// End find point
inline void findEnd(const ROTATE_CLIP* clip, const int32_t dsty, const int32_t dstx0, int32_t* dstx1)
{
    int32_t scx, scy;
    int32_t testx1 = *dstx1;

    if (testx1 < dstx0) testx1 = dstx0;

    scx = clip->ax * testx1 + clip->bx * dsty + clip->cx;
    scy = clip->ay * testx1 + clip->by * dsty + clip->cy;

    if (pointInBound(clip, scx, scy))
    {
        testx1++;
        scx += clip->ax;
        scy += clip->ay;

        while (pointInBound(clip, scx, scy))
        {
            testx1++;
            scx += clip->ax;
            scy += clip->ay;
        }

        *dstx1 = testx1;
    }
    else
    {
        scx -= clip->ax;
        scy -= clip->ay;
        while (!pointInBound(clip, scx, scy))
        {
            testx1--;
            scx -= clip->ax;
            scy -= clip->ay;
        }

        *dstx1 = testx1;
    }
}

// Update point x
inline void updateInX(ROTATE_CLIP* clip)
{
    int32_t scx, scy, i;

    if (!clip->boundWidth || clip->outBound0 >= clip->outBound1)
    {
        clip->inBound0 = clip->outBound0;
        clip->inBound1 = clip->outBound1;
    }
    else
    {
        scx = clip->srcx;
        scy = clip->srcy;
        i = clip->outBound0;

        while (i < clip->outBound1)
        {
            if (pointInSrc(clip, scx, scy)) break;
            scx += clip->ax;
            scy += clip->ay;
            i++;
        }

        clip->inBound0 = i;

        scx = clip->srcx + (clip->outBound1 - clip->outBound0) * clip->ax;
        scy = clip->srcy + (clip->outBound1 - clip->outBound0) * clip->ay;

        i = clip->outBound1;

        while (i > clip->inBound0)
        {
            scx -= clip->ax;
            scy -= clip->ay;
            if (pointInSrc(clip, scx, scy)) break;
            i--;
        }

        clip->inBound1 = i;
    }
}

// Update up point x
inline void updateUpX(ROTATE_CLIP* clip)
{
    if (clip->currUp0 < 0) clip->outBound0 = 0;
    else clip->outBound0 = clip->currUp0;
        
    if (clip->currUp1 >= clip->dstw) clip->outBound1 = clip->dstw;
    else clip->outBound1 = clip->currUp1;

    updateInX(clip);
}

// Update down point x
inline void updateDownX(ROTATE_CLIP* clip)
{
    if (clip->currDown0 < 0) clip->outBound0 = 0;
    else clip->outBound0 = clip->currDown0;

    if (clip->currDown1 >= clip->dstw) clip->outBound1 = clip->dstw;
    else clip->outBound1 = clip->currDown1;

    updateInX(clip);
}

// Initialize clipping data
inline int32_t intiClip(ROTATE_CLIP* clip, const int32_t dcx, const int32_t dcy, const int32_t bwidth)
{
    clip->boundWidth = bwidth;
    clip->yDown = dcx;
    clip->currDown0 = dcy;
    clip->currDown1 = dcy;

    if (findBegin(clip, clip->yDown, &clip->currDown0, clip->currDown1)) findEnd(clip, clip->yDown, clip->currDown0, &clip->currDown1);

    clip->yUp = clip->yDown;
    clip->currUp0 = clip->currDown0;
    clip->currUp1 = clip->currDown1;
        
    updateUpX(clip);

    return clip->currDown0 < clip->currDown1;
}

// Proccess next line down
inline int32_t nextLineDown(ROTATE_CLIP* clip)
{
    clip->yDown++;
    if (!findBegin(clip, clip->yDown, &clip->currDown0, clip->currDown1)) return 0;
    findEnd(clip, clip->yDown, clip->currDown0, &clip->currDown1);
    updateDownX(clip);
    return clip->currDown0 < clip->currDown1;
}

// Proccess next line up
inline int32_t nextLineUp(ROTATE_CLIP* clip)
{
    clip->yUp--;
    if (!findBegin(clip, clip->yUp, &clip->currUp0, clip->currUp1)) return 0;
    findEnd(clip, clip->yUp, clip->currUp0, &clip->currUp1);
    updateUpX(clip);
    return clip->currUp0 < clip->currUp1;
}

// Boundary clip points at (x,y)
inline int32_t clampPoint(const int32_t width, const int32_t height, int32_t* x, int32_t* y)
{
    int32_t ret = 1;

    if (*x < 0)
    { 
        *x = 0;
        ret = 0;
    }
    else if (*x >= width)
    { 
        *x = width - 1;
        ret = 0;
    }

    if (*y < 0)
    {
        *y = 0;
        ret = 0;
    }
    else if (*y >= height)
    {
        *y = height - 1;
        ret = 0;
    }

    return ret;
}

// Clamp pixels at offset (x,y)
inline uint32_t clampPixels(const GFX_IMAGE* img, int32_t x, int32_t y)
{
    const uint32_t* psrc = (const uint32_t*)img->mData;
    const int32_t insrc = clampPoint(img->mWidth, img->mHeight, &x, &y);
    uint32_t result = psrc[y * img->mWidth + x]; 
    if (!insrc)
    {
        ARGB *pcol = (ARGB*)&result;
        pcol->a = 0;
    }
    return result;
}

// Alpha-Blending pixel
inline uint32_t alphaBlend(const uint32_t dstCol, const uint32_t srcCol)
{
#ifdef _USE_MMX
    __asm {
        pxor        mm7, mm7
        movd        mm0, srcCol
        movd        mm2, dstCol
        punpcklbw   mm0, mm7
        punpcklbw   mm2, mm7
        movq        mm1, mm0
        punpckhwd   mm1, mm1
        psubw       mm0, mm2
        punpckhdq   mm1, mm1
        psllw       mm2, 8
        pmullw      mm0, mm1
        paddw       mm2, mm0
        psrlw       mm2, 8
        packuswb    mm2, mm7
        movd        eax, mm2
        emms
    }
#else
    const uint8_t cover = srcCol >> 24;
    const uint8_t rcover = 255 - cover;
    const uint32_t rb = ((dstCol & 0x00ff00ff) * rcover + (srcCol & 0x00ff00ff) * cover);
    const uint32_t ag = (((dstCol & 0xff00ff00) >> 8) * rcover + ((srcCol & 0xff00ff00) >> 8) * cover);
    return ((rb & 0xff00ff00) >> 8) | (ag & 0xff00ff00);
#endif
}

// Bilinear get pixel with FIXED-POINT (signed 16.16)
inline uint32_t bilinearGetPixelCenter(const GFX_IMAGE* psrc, const int32_t sx, const int32_t sy)
{
    const int32_t width = psrc->mWidth;
    const uint32_t* pixel = (uint32_t*)psrc->mData;

#ifdef _USE_MMX
    __asm {
        mov         eax, sx
        mov         edx, sy
        pxor        mm7, mm7
        shl         edx, 16
        shl         eax, 16
        shr         edx, 24
        shr         eax, 24
        movd        mm6, edx
        movd        mm5, eax
        mov         edx, sy
        mov         eax, width
        shl         eax, 2
        sar         edx, 16
        imul        edx, eax
        add         edx, pixel
        add         eax, edx
        mov         ecx, sx
        sar         ecx, 16
        movd        mm2, dword ptr[eax + ecx * 4]
        movd        mm0, dword ptr[eax + ecx * 4 + 4]
        punpcklwd   mm5, mm5
        punpcklwd   mm6, mm6
        movd        mm3, dword ptr[edx + ecx * 4]
        movd        mm1, dword ptr[edx + ecx * 4 + 4]
        punpckldq   mm5, mm5
        punpcklbw   mm0, mm7
        punpcklbw   mm1, mm7
        punpcklbw   mm2, mm7
        punpcklbw   mm3, mm7
        psubw       mm0, mm2
        psubw       mm1, mm3
        psllw       mm2, 8
        psllw       mm3, 8
        pmullw      mm0, mm5
        pmullw      mm1, mm5
        punpckldq   mm6, mm6
        paddw       mm0, mm2
        paddw       mm1, mm3
        psrlw       mm0, 8
        psrlw       mm1, 8
        psubw       mm0, mm1
        psllw       mm1, 8
        pmullw      mm0, mm6
        paddw       mm0, mm1
        psrlw       mm0, 8
        packuswb    mm0, mm7
        movd        eax, mm0
        emms
    }
#else
    const uint32_t *p0 = &pixel[(sy >> 16) * width + (sx >> 16)];
    const uint32_t *p1 = &p0[width];

    const uint32_t pu = (uint8_t)(sx >> 8);
    const uint32_t pv = (uint8_t)(sy >> 8);
    const uint32_t w3 = (pu * pv) >> 8;
    const uint32_t w2 = pu - w3;
    const uint32_t w1 = pv - w3;
    const uint32_t w0 = 256 - w1 - w2 - w3;

    uint32_t br = ( p0[0] & 0x00ff00ff) * w0;
    uint32_t ga = ((p0[0] & 0xff00ff00) >> 8) * w0;

    br += ( p0[1] & 0x00ff00ff) * w2;
    ga += ((p0[1] & 0xff00ff00) >> 8) * w2;
    br += ( p1[0] & 0x00ff00ff) * w1;
    ga += ((p1[0] & 0xff00ff00) >> 8) * w1;
    br += ( p1[1] & 0x00ff00ff) * w3;
    ga += ((p1[1] & 0xff00ff00) >> 8) * w3;

    return (ga & 0xff00ff00) | ((br & 0xff00ff00) >> 8);
#endif
}

// Bilinear get pixel with FIXED-POINT (signed 16.16)
inline uint32_t bilinearGetPixelBorder(const GFX_IMAGE* psrc, const int32_t sx, const int32_t sy)
{
    GFX_IMAGE img = {0};
    uint32_t pixels[4] = {0};

    // Convert to fixed points
    const int32_t lx = sx >> 16;
    const int32_t ly = sy >> 16;

    // Load the 4 neighboring pixels
    pixels[0] = clampPixels(psrc, lx    , ly    );
    pixels[1] = clampPixels(psrc, lx + 1, ly    );
    pixels[2] = clampPixels(psrc, lx    , ly + 1);
    pixels[3] = clampPixels(psrc, lx + 1, ly + 1);
    
    img.mWidth = 2;
    img.mHeight = 2;
    img.mRowBytes = 8;
    img.mData = (uint8_t*)pixels;
    return bilinearGetPixelCenter(&img, sx & 0xffff, sy & 0xffff);
}

// Bilinear rotate scan line (sub-routine of full optimize version)
// Improve smooth border when rotating will make image look better
void bilinearRotateLine(uint32_t* pdst, const int32_t boundx0, const int32_t inx0, const int32_t inx1, const int32_t boundx1, const GFX_IMAGE* psrc, int32_t sx, int32_t sy, const int32_t addx, const int32_t addy)
{
    int32_t x = 0;
    for (x = boundx0; x < inx0; x++, sx += addx, sy += addy)    pdst[x] = alphaBlend(pdst[x], bilinearGetPixelBorder(psrc, sx, sy));
    for (x = inx0; x < inx1; x++, sx += addx, sy += addy)       pdst[x] = bilinearGetPixelCenter(psrc, sx, sy);
    for (x = inx1; x < boundx1; x++, sx += addx, sy += addy)    pdst[x] = alphaBlend(pdst[x], bilinearGetPixelBorder(psrc, sx, sy));
}

// Maximize optimize version (extremely fast)
// Use sticks:
// 1. fixed-points
// 2. separate inbound and outbound pixel calculation
// 3. MMX instructions
// 4. clipping data
void bilinearRotateImage(const GFX_IMAGE* dst, const GFX_IMAGE* src, const double degree, const double scalex, const double scaley)
{
    uint32_t *yline;
    ROTATE_CLIP clip;

    const double scalexy = 1.0 / (scalex * scaley);
    const double rscalex = scalexy * scaley;
    const double rscaley = scalexy * scalex;

    const double angle = (degree * M_PI) / 180.0;
    const double sina = sin(-angle);
    const double cosa = cos(-angle);
    const int32_t sini = sina * 65536; // Convert to fixed points (no truncated)
    const int32_t cosi = cosa * 65536; // Convert to fixed points (no truncated)

    uint32_t* pdst = (uint32_t*)dst->mData;

    const int32_t srcw = src->mWidth;
    const int32_t srch = src->mHeight;
    const int32_t dstw = dst->mWidth;
    const int32_t dsth = dst->mHeight;
    
    const int32_t ax = rscalex * cosi;
    const int32_t ay = rscalex * sini;
    const int32_t bx = -rscaley * sini;
    const int32_t by = rscaley * cosi;

    const int32_t dcx = dstw >> 1;
    const int32_t dcy = dsth >> 1;
    const int32_t scx = srcw << 15; // (srcw >> 1) << 16 convert to fixed points
    const int32_t scy = srch << 15; // (srch >> 1) << 16 convert to fixed points

    // Rotation points
    const int32_t cx = scx - (dcx * rscalex * cosi - dcy * rscaley * sini);
    const int32_t cy = scy - (dcx * rscalex * sini + dcy * rscaley * cosi); 
    
    clip.ax = ax;
    clip.bx = bx;
    clip.ay = ay;
    clip.by = by;
    clip.cx = cx;
    clip.cy = cy;
    clip.dstw = dstw;
    clip.dsth = dsth;
    clip.srcw = srcw;
    clip.srch = srch;
    
    // Clipping data
    if (!intiClip(&clip, dcx, dcy, 1)) return;

    yline = &pdst[dstw * clip.yDown];
    while (1)
    {
        if (clip.yDown >= dsth) break;
        if (clip.yDown >= 0) bilinearRotateLine(yline, clip.outBound0, clip.inBound0, clip.inBound1, clip.outBound1, src, clip.srcx, clip.srcy, ax, ay);
        if (!nextLineDown(&clip)) break;
        yline += dstw;
    }

    yline = &pdst[dstw * clip.yUp];
    while (nextLineUp(&clip))
    {
        if (clip.yUp < 0) break;
        yline -= dstw;
        if (clip.yUp < dsth) bilinearRotateLine(yline, clip.outBound0, clip.inBound0, clip.inBound1, clip.outBound1, src, clip.srcx, clip.srcy, ax, ay);
    }
}

// Maximize optimize version (extremely fast)
// Using sticks:
// 1. FIXED-POINT
// 2. separate inbound and outbound pixels calculation
// 3. MMX instructions
void bilinearScaleImage(const GFX_IMAGE* dst, const GFX_IMAGE* src)
{
    int32_t srcx, srcy, x, y;
    int32_t startx, starty, endx, endy;
    uint32_t* pdst = (uint32_t*)dst->mData;

    const int32_t srcw = src->mWidth;
    const int32_t srch = src->mHeight;
    const int32_t dstw = dst->mWidth;
    const int32_t dsth = dst->mHeight;
    const int32_t scalex = (srcw << 16) / dstw + 1;
    const int32_t scaley = (srch << 16) / dsth + 1;
    const int32_t errorx = (scalex >> 1) - 32768;
    const int32_t errory = (scaley >> 1) - 32768;

    startx = (65536 - errorx) / scalex + 1;
    if (startx >= dstw) startx = dstw;

    starty = (65536 - errory) / scaley + 1;
    if (starty >= dsth) starty = dsth;

    endx = (((srcw - 2) << 16) - errorx) / scalex + 1;
    if (endx < startx) endx = startx;

    endy = (((srch - 2) << 16) - errory) / scaley + 1;
    if (endy < starty) endy = starty;

    srcy = errory;
    for (y = 0; y < starty; y++, srcy += scaley)
    {
        for (x = 0, srcx = errorx; x < dstw; x++, srcx += scalex) *pdst++ = bilinearGetPixelBorder(src, srcx, srcy);
    }

    for (y = starty; y < endy; y++, srcy += scaley)
    {
        srcx = errorx;
        for (x = 0; x < startx; x++, srcx += scalex)    *pdst++ = bilinearGetPixelBorder(src, srcx, srcy);
        for (x = startx; x < endx; x++, srcx += scalex) *pdst++ = bilinearGetPixelCenter(src, srcx, srcy);
        for (x = endx; x < dstw; x++, srcx += scalex)   *pdst++ = bilinearGetPixelBorder(src, srcx, srcy);
    }

    for (y = endy; y < dsth; y++, srcy += scaley)
    {
        for (x = 0, srcx = errorx; x < dstw; x++, srcx += scalex) *pdst++ = bilinearGetPixelBorder(src, srcx, srcy);
    }
}

// fade image effects
void fadeOutImage32(GFX_IMAGE *img, uint8_t step)
{
    void *data = img->mData;
    uint32_t size = img->mSize >> 2;

#ifdef _USE_MMX
    __asm {
        mov         edi, data
        xor         eax, eax
        mov         al, step
        movd        mm1, eax
        punpcklbw   mm1, mm1
        punpcklwd   mm1, mm1
        mov         ecx, size
        shr         ecx, 1
        jz          once
        punpckldq   mm1, mm1
    plot:
        movq        mm0, [edi]
        psubusb     mm0, mm1
        movq        [edi], mm0
        add         edi, 8
        dec         ecx
        jnz         plot
    once:
        test        size, 1
        jz          end
        movd        mm0, [edi]
        psubusb     mm0, mm1
        movd        [edi], mm0
    end:
        emms
    }
#else
    __asm {
        mov         edi, data
        mov         ecx, size
    next:
        mov         eax, [edi]
        sub         al, step
        jnc         bstep
        xor         al, al
    bstep:
        sub         ah, step
        jnc         gstep
        xor         ah, ah
    gstep:
        mov         ebx, eax
        shr         ebx, 16
        sub         bl, step
        jnc         rstep
        xor         bl, bl
    rstep:
        shl         ebx, 16
        and         eax, 00FFFFh
        or          eax, ebx
        stosd
        dec         ecx
        jnz         next

    }
#endif
}

void fadeOutImage24(GFX_IMAGE *img, uint8_t step)
{
    void *data = img->mData;
    uint32_t size = img->mSize / 3;

    __asm {
        mov     edi, data
        mov     ecx, size
    next:
        mov     ax, [edi]
        sub     al, step
        jnc     bstep
        xor     al, al
    bstep:
        sub     ah, step
        jnc     gstep
        xor     ah, ah
    gstep:
        mov     bl, [edi + 2]
        sub     bl, step
        jnc     rstep
        xor     bl, bl
    rstep:
        stosw
        mov     al, bl
        stosb  
        dec     ecx
        jnz     next
    }
}

void fadeOutImage16(GFX_IMAGE *img, uint8_t step)
{
    void *data = img->mData;
    uint32_t size = img->mSize >> 1;

    __asm {
        mov     edi, data
        mov     ecx, size
    next:
        mov     ax, [edi]
        mov     bx, ax
        and     ax, 1111100000011111b
        and     bx, 11111100000b
        sub     al, step
        jnc     bstep
        xor     al, al
    bstep:
        shr     ah, 3
        sub     ah, step
        jnc     gstep
        xor     ah, ah
    gstep:
        shr     bx, 5
        sub     bl, step
        jnc     rstep
        xor     bl, bl
    rstep:
        shl     ah, 3
        shl     bx, 5
        or      ax, bx
        stosw
        dec     ecx
        jnz     next
    }
}

void fadeOutImage15(GFX_IMAGE *img, uint8_t step)
{
    void *data = img->mData;
    uint32_t size = img->mSize >> 1;

    __asm {
        mov     edi, data
        mov     ecx, size
    next:
        mov     ax, [edi]
        mov     bx, ax
        and     ax, 111110000011111b
        and     bx, 1111100000b
        sub     al, step
        jnc     bstep
        xor     al, al
    bstep:
        shr     ah, 2
        sub     ah, step
        jnc     gstep
        xor     ah, ah
    gstep:
        shr     bx, 5
        sub     bl, step
        jnc     rstep
        xor     bl, bl
    rstep:
        shl     ah, 2
        shl     bx, 5
        or      ax, bx
        stosw
        dec     ecx
        jnz     next
    }
}

// Initialize VESA mode by resolution and bits per pixel
int32_t setVesaMode(int32_t px, int32_t py, uint8_t bits, uint32_t refreshRate)
{
    RM_REGS             regs;
    VBE_DRIVER_INFO     drvInfo;
    VBE_MODE_INFO       modeInfo;
    VBE_CRTC_INFO_BLOCK *CRTCPtr;

    uint16_t    *modePtr;
    uint32_t    pixelClock;
    uint32_t    lineWidth;
    
    // Check VBE driver info
    memset(&drvInfo, 0, sizeof(drvInfo));
    if (!getVesaDriverInfo(&drvInfo)) return 0;

    // Catch mode number info in VBE mode info block
    modePtr = (uint16_t*)drvInfo.VideoModePtr;
    if (modePtr == NULL) return 0;

    // Find VESA mode match with request
    while (modePtr != NULL && *modePtr != 0xFFFF)
    {
        // Get current mode info
        memset(&modeInfo, 0, sizeof(VBE_MODE_INFO));
        if (getVesaModeInfo(*modePtr, &modeInfo) && (px == modeInfo.XResolution) && (py == modeInfo.YResolution) && (bits == modeInfo.BitsPerPixel)) break;
        modePtr++;
    }

    // Is valid mode number?
    if (*modePtr == 0xFFFF) return 0;

    // setup init VESA function
    memset(&regs, 0, sizeof(regs));
    regs.eax = 0x4F02;
    regs.ebx = *modePtr | 0x4000; // Add linear frame buffer param D14

    // Check if request refresh rate and must be VBE 3.0
    if (refreshRate >= 50 && drvInfo.VBEVersion >= 0x0300)
    {
        // Setup memory to passing VBE
        if (crtcSegment == 0) crtcSegment = allocDosSegment(512);
        if (crtcSegment == 0 || crtcSegment == 0xFFFF) return 0;

        // Calculate CRTC timing using GTF formular
        CRTCPtr = (VBE_CRTC_INFO_BLOCK*)((crtcSegment & 0x0000FFFF) << 4);
        memset(CRTCPtr, 0, sizeof(VBE_CRTC_INFO_BLOCK));
        calcCrtcTimingGTF(CRTCPtr, px, py, refreshRate << 10, 0, 0);

        // Calculate actual pixel clock
        pixelClock = getClosestPixelClock(*modePtr, CRTCPtr->PixelClock);
        if (pixelClock > 0)
        {
            // Re-calculate pixel clock and refresh rate
            CRTCPtr->PixelClock = pixelClock;
            CRTCPtr->RefreshRate = 100 * (CRTCPtr->PixelClock / (CRTCPtr->HorizontalTotal * CRTCPtr->VerticalTotal));

            // D15=don't clear screen, D14=linear/flat buffer, D11=Use user specified CRTC values for refresh rate
            regs.ebx |= 0x0800;
            regs.es  = crtcSegment;
            regs.edi = 0;
        }
    }

    // Start init VESA mode
    simRealModeInt(0x10, &regs);
    if (regs.eax != 0x004F) return 0;

    // Calculate linear address size and map physical address to linear address
    lfbSize = modeInfo.YResolution * modeInfo.BytesPerScanline;
    if (!(lfbPtr = (uint8_t*)mapPhysicalAddress(modeInfo.PhysBasePtr, lfbSize))) return 0;

    // Only DIRECT COLOR and PACKED PIXEL mode are allowed
    if (modeInfo.MemoryModel == VBE_MM_DCOLOR || modeInfo.MemoryModel == VBE_MM_PACKED)
    {
        // Setup protect mode interface to use setDisplayStart function
        getProtectModeFunctions();

        // Only for VBE 3.0
        if (drvInfo.VBEVersion >= 0x0300)
        {
            // Calculate mask for RGB
            rmask = ((1UL << modeInfo.LinRedMaskSize) - 1) << modeInfo.LinRedFieldPosition;
            gmask = ((1UL << modeInfo.LinGreenMaskSize) - 1) << modeInfo.LinGreenFieldPosition;
            bmask = ((1UL << modeInfo.LinBlueMaskSize) - 1) << modeInfo.LinBlueFieldPosition;

            // Calculate RGB shifter
            rshift = 8 - modeInfo.LinRedMaskSize;
            gshift = 8 - modeInfo.LinGreenMaskSize;
            bshift = 8 - modeInfo.LinBlueMaskSize;

            // Save RGB position
            rpos = modeInfo.LinRedFieldPosition;
            gpos = modeInfo.LinGreenFieldPosition;
            bpos = modeInfo.LinBlueFieldPosition;

            // Save line size (in bytes)
            bytesPerScanline = modeInfo.LinBytesPerScanline;

            // Number of visual screen pages
            numOfPages = modeInfo.LinNumberOfImagePages;
        }
        else
        {
            // Calculate mask for RGB
            rmask = ((1UL << modeInfo.RedMaskSize) - 1) << modeInfo.RedFieldPosition;
            gmask = ((1UL << modeInfo.GreenMaskSize) - 1) << modeInfo.GreenFieldPosition;
            bmask = ((1UL << modeInfo.BlueMaskSize) - 1) << modeInfo.BlueFieldPosition;

            // Calculate RGB shifter
            rshift = 8 - modeInfo.RedMaskSize;
            gshift = 8 - modeInfo.GreenMaskSize;
            bshift = 8 - modeInfo.BlueMaskSize;

            // Save RGB position
            rpos = modeInfo.RedFieldPosition;
            gpos = modeInfo.GreenFieldPosition;
            bpos = modeInfo.BlueFieldPosition;

            // Save line size (in bytes)
            bytesPerScanline = modeInfo.BytesPerScanline;

            // Number of visual screen pages
            numOfPages = modeInfo.NumberOfImagePages;
        }

        // Save bits per pixels
        bitsPerPixel = modeInfo.BitsPerPixel;
        bytesPerPixel = (bitsPerPixel + 7) >> 3;

        // Save x, y resolution
        lfbWidth = modeInfo.XResolution;
        lfbHeight = modeInfo.YResolution;

        // Check for logical width to increasing
        lineWidth = max(bytesPerScanline, lfbWidth * bytesPerPixel);
        if (lineWidth > bytesPerScanline)
        {
            memset(&regs, 0, sizeof(regs));
            regs.eax = 0x4F06;
            regs.ebx = 0;
            regs.ecx = lineWidth / bytesPerPixel;
            simRealModeInt(0x10, &regs);
            if (regs.eax != 0x004F || lineWidth > regs.ebx) return 0;
        }

        // Mapping functions pointer
        switch (bitsPerPixel)
        {
        case 8:
            toRGB               = toRGB8;
            fromRGB             = fromRGB8;
            putPixel            = putPixel8;
            getPixel            = getPixel8;
            fillRect            = fillRect8;
            horizLine           = horizLine8;
            vertLine            = vertLine8;
            getImage            = getImage8;
            putImage            = putImage8;
            putSprite           = putSprite8;
            clearScreen         = clearScreen8;
            scaleImage          = scaleImage8;
            fillRectPattern     = fillRectPattern8;
            break;

        case 15:
            toRGB               = toRGB15;
            fromRGB             = fromRGB15;
            putPixel            = putPixel1516;
            putPixelAdd         = putPixelAdd15;
            putPixelSub         = putPixelSub15;
            getPixel            = getPixel1516;
            fillRect            = fillRect1516;
            fillRectAdd         = fillRectAdd15;
            fillRectSub         = fillRectSub15;
            horizLine           = horizLine1516;
            horizLineAdd        = horizLineAdd15;
            horizLineSub        = horizLineSub15;
            vertLine            = vertLine1516;
            vertLineAdd         = vertLineAdd15;
            vertLineSub         = vertLineSub15;
            getImage            = getImage1516;
            putImage            = putImage1516;
            putImageAdd         = putImageAdd15;
            putImageSub         = putImageSub15;
            putSprite           = putSprite1516;
            putSpriteAdd        = putSpriteAdd15;
            putSpriteSub        = putSpriteSub15;
            clearScreen         = clearScreen1516;
            scaleImage          = scaleImage1516;
            fadeOutImage        = fadeOutImage15;
            fillRectPattern     = fillRectPattern1516;
            fillRectPatternAdd  = fillRectPatternAdd15;
            fillRectPatternSub  = fillRectPatternSub15;
            break;

        case 16:
            toRGB               = toRGB16;
            fromRGB             = fromRGB16;
            putPixel            = putPixel1516;
            putPixelAdd         = putPixelAdd16;
            putPixelSub         = putPixelSub16;
            getPixel            = getPixel1516;
            fillRect            = fillRect1516;
            fillRectAdd         = fillRectAdd16;
            fillRectSub         = fillRectSub16;
            horizLine           = horizLine1516;
            horizLineAdd        = horizLineAdd16;
            horizLineSub        = horizLineSub16;
            vertLine            = vertLine1516;
            vertLineAdd         = vertLineAdd16;
            vertLineSub         = vertLineSub16;
            getImage            = getImage1516;
            putImage            = putImage1516;
            putImageAdd         = putImageAdd16;
            putImageSub         = putImageSub16;
            putSprite           = putSprite1516;
            putSpriteAdd        = putSpriteAdd16;
            putSpriteSub        = putSpriteSub16;
            clearScreen         = clearScreen1516;
            scaleImage          = scaleImage1516;
            fadeOutImage        = fadeOutImage16;
            fillRectPattern     = fillRectPattern1516;
            fillRectPatternAdd  = fillRectPatternAdd16;
            fillRectPatternSub  = fillRectPatternSub16;
            break;

        case 24:
            toRGB               = toRGB2432;
            fromRGB             = fromRGB2432;
            putPixel            = putPixel24;
            putPixelAdd         = putPixelAdd24;
            putPixelSub         = putPixelSub24;
            getPixel            = getPixel24;
            fillRect            = fillRect24;
            fillRectAdd         = fillRectAdd24;
            fillRectSub         = fillRectSub24;
            horizLine           = horizLine24;
            horizLineAdd        = horizLineAdd24;
            horizLineSub        = horizLineSub24;
            vertLine            = vertLine24;
            vertLineAdd         = vertLineAdd24;
            vertLineSub         = vertLineSub24;
            getImage            = getImage24;
            putImage            = putImage24;
            putImageAdd         = putImageAdd24;
            putImageSub         = putImageSub24;
            putSprite           = putSprite24;
            putSpriteAdd        = putSpriteAdd24;
            putSpriteSub        = putSpriteSub24;
            clearScreen         = clearScreen24;
            scaleImage          = scaleImage24;
            fadeOutImage        = fadeOutImage24;
            fillRectPattern     = fillRectPattern24;
            fillRectPatternAdd  = fillRectPatternAdd24;
            fillRectPatternSub  = fillRectPatternSub24;
            break;

        case 32:
            toRGB               = toRGB2432;
            fromRGB             = fromRGB2432;
            putPixel            = putPixel32;
            putPixelAdd         = putPixelAdd32;
            putPixelSub         = putPixelSub32;
            getPixel            = getPixel32;
            fillRect            = fillRect32;
            fillRectAdd         = fillRectAdd32;
            fillRectSub         = fillRectSub32;
            horizLine           = horizLine32;
            horizLineAdd        = horizLineAdd32;
            horizLineSub        = horizLineSub32;
            vertLine            = vertLine32;
            vertLineAdd         = vertLineAdd32;
            vertLineSub         = vertLineSub32;
            getImage            = getImage32;
            putImage            = putImage32;
            putImageAdd         = putImageAdd32;
            putImageSub         = putImageSub32;
            putSprite           = putSprite32;
            putSpriteAdd        = putSpriteAdd32;
            putSpriteSub        = putSpriteSub32;
            clearScreen         = clearScreen32;
            scaleImage          = scaleImage32;
            fadeOutImage        = fadeOutImage32;
            fillRectPattern     = fillRectPattern32;
            fillRectPatternAdd  = fillRectPatternAdd32;
            fillRectPatternSub  = fillRectPatternSub32;
            break;
        }

        // Initialize center x, y
        centerX = (lfbWidth >> 1) - 1;
        centerY = (lfbHeight >> 1) - 1;

        // Initialize view port size
        cminX   = 0;
        cminY   = 0;
        cmaxX   = lfbWidth - 1;
        cmaxY   = lfbHeight - 1;

        // Initialize random number generation
        randSeed = time(NULL);
        srand(randSeed);

        // Initialize GFXLIB buffer
        gfxBuff = (uint8_t*)calloc(GFX_BUFF_SIZE, 1);
        if (!gfxBuff)
        {
            printf("Cannot initialize GFXLIB buffer!\n");
            exit(1);
        }

        // Filled cpu and memory info structure
        getCpuInfo();
        getMemoryInfo();

        return *modePtr;
    }

    return 0;
}

// Get/Set current hardware palette entries
void setRGB(uint16_t index, uint8_t r, uint8_t g, uint8_t b)
{
    __asm {
        mov     dx, 03C8h
        mov     ax, index
        out     dx, al
        inc     dx
        mov     al, r
        shr     al, 2
        out     dx, al
        mov     al, g
        shr     al, 2
        out     dx, al
        mov     al, b
        shr     al, 2
        out     dx, al
    }
}

void getRGB(uint16_t index, void *pal)
{
    __asm {
        mov     edi, pal
        mov     dx, 03C7h
        mov     ax, index
        out     dx, al
        add     dx, 2
        in      al, dx
        shl     al, 2
        mov     [edi + 2], al
        in      al, dx
        shl     al, 2
        mov     [edi + 1], al
        in      al, dx
        shl     al, 2
        mov     [edi], al
    }
}

// Make VESA palette entries
void shiftPalette(void *pal)
{
    __asm {
        mov     edi, pal
        mov     ecx, 256
    step:
        mov     al, [edi + 2]
        shl     al, 2
        mov     [edi + 2], al
        mov     al, [edi + 1]
        shl     al, 2
        mov     [edi + 1], al
        mov     al, [edi]
        shl     al, 2
        mov     [edi], al
        add     edi, 3
        dec     ecx
        jnz     step
    }
}

// Get hardware palette entries
void getPalette(void *pal)
{
    __asm {
        xor     al, al
        mov     dx, 03C7h
        out     dx, al
        add     dx, 2
        mov     edi, pal
        mov     ecx, 256
    step:
        in      al, dx
        shl     al, 2
        mov     [edi + 2], al
        in      al, dx
        shl     al, 2
        mov     [edi + 1], al
        in      al, dx
        shl     al, 2
        mov     [edi], al
        add     edi, 3
        dec     ecx
        jnz     step
    }
}

// Set hardware palette entries
void setPalette(void *pal)
{
    __asm {
        xor     al, al
        mov     dx, 03C8h
        out     dx, al
        inc     dx
        mov     esi, pal
        mov     ecx, 256
    step:
        mov     al, [esi + 2]
        shr     al, 2
        out     dx, al
        mov     al, [esi + 1]
        shr     al, 2
        out     dx, al
        mov     al, [esi]
        shr     al, 2
        out     dx, al
        add     esi, 3
        dec     ecx
        jnz     step
    }
}

// Make linear palette (7 circle colors)
void makeLinearPalette()
{
    int32_t i, j;
    RGB pal[256] = {0};

    for (i = 0; i < 32; i++)
    {
        j = 16 + i;
        pal[j].r = 63;
        pal[j].g = 0;
        pal[j].b = 63 - (i << 1);

        j = 48 + i;
        pal[j].r = 63;
        pal[j].g = i << 1;
        pal[j].b = 0;

        j = 80 + i;
        pal[j].r = 63 - (i << 1);
        pal[j].g = 63;
        pal[j].b = 0;

        j = 112 + i;
        pal[j].r = 0;
        pal[j].g = 63;
        pal[j].b = i << 1;

        j = 144 + i;
        pal[j].r = 0;
        pal[j].g = 63 - (i << 1);
        pal[j].b = 63;

        j = 176 + i;
        pal[j].r = i << 1;
        pal[j].g = 0;
        pal[j].b = 63;
    }

    shiftPalette(pal);
    setPalette(pal);
}

// yet another funky palette
void makeFunkyPalette()
{
    RGB pal[256] = {0};
    uint8_t ry, gy, by;
    int32_t i, r, g, b, rx, gx, bx;

    r = 0;
    g = 0;
    b = 0;

    ry = 1;
    gy = 1;
    by = 1;

    rx = (rand() % 5) + 1;
    gx = (rand() % 5) + 1;
    bx = (rand() % 5) + 1;

    for (i = 0; i < 256; i++)
    {
        pal[i].r = r;
        pal[i].g = g;
        pal[i].b = b;

        if (ry) r += rx; else r -= rx;
        if (gy) g += gx; else g -= gx;
        if (by) b += bx; else b -= bx;

        if ((r + rx > 63) || (r - rx < 0))
        {
            ry = !ry;
            rx = (rand() % 5) + 1;
        }

        if ((g + gx > 63) || (g - gx < 0))
        {
            gy = !gy;
            gx = (rand() % 5) + 1;
        }

        if ((b + bx > 63) || (b - bx < 0))
        {
            by = !by;
            bx = (rand() % 5) + 1;
        }
    }

    shiftPalette(pal);
    setPalette(pal);
}

// Make rainbow palette
void makeRainbowPalette()
{
    int32_t i;
    RGB pal[256] = {0};
    
    for (i = 0; i < 32; i++)
    {
        pal[i      ].r = (i << 1);
        pal[63 - i ].r = (i << 1);
        pal[i + 64 ].g = (i << 1);
        pal[127 - i].g = (i << 1);
        pal[i + 128].b = (i << 1);
        pal[191 - i].b = (i << 1);
        pal[i + 192].r = (i << 1);
        pal[i + 192].g = (i << 1);
        pal[i + 192].b = (i << 1);
        pal[255 - i].r = (i << 1);
        pal[255 - i].g = (i << 1);
        pal[255 - i].b = (i << 1);
    }
    
    shiftPalette(pal);
    setPalette(pal);
}

// Make black current palette
void setBlackPalette()
{
    RGB pal[256] = {0};
    setPalette(pal);
}

// Make black current palette
void setWhitePalette()
{
    RGB pal[256] = {255};
    setPalette(pal);
}

// Scroll current palette
void scrollPalette(int32_t from, int32_t to, int32_t step)
{
    RGB tmp = {0};
    RGB pal[256] = {0};
    uint32_t count = (to - from) * sizeof(RGB);

    getPalette(pal);
    
    while (step--)
    {
        memcpy(&tmp, &pal[from], sizeof(tmp));
        memcpy(&pal[from], &pal[from + 1], count);
        memcpy(&pal[to], &tmp, sizeof(tmp));
    }

    waitRetrace();
    setPalette(pal);
}

// Rotate current palette
void rotatePalette(int32_t from, int32_t to, int32_t loop)
{
    RGB tmp = {0};
    RGB pal[256] = {0};
    uint32_t count = (to - from) * sizeof(RGB);

    getPalette(pal);

    if (loop > 0)
    {
        // Check for key pressed
        while (!keyPressed(27) && loop--)
        {
            memcpy(&tmp, &pal[from], sizeof(tmp));
            memcpy(&pal[from], &pal[from + 1], count);
            memcpy(&pal[to], &tmp, sizeof(tmp));
            waitRetrace();
            setPalette(pal);
        }
    }
    else
    {
        // Check for key pressed
        while (!keyPressed(27))
        {
            memcpy(&tmp, &pal[from], sizeof(tmp));
            memcpy(&pal[from], &pal[from + 1], count);
            memcpy(&pal[to], &tmp, sizeof(tmp));
            waitRetrace();
            setPalette(pal);
        }
    }
}

// All fade functions (In/Max/Out/Min)
// Fade from black palette to current palette
void fadeIn(RGB *dest)
{
    int32_t i, j, k;
    RGB src[256] = {0};

    getPalette(src);
    for (i = 63; i >= 0 && !keyPressed(27); i--)
    {
        for (j = 0; j < 256; j++)
        {
            k = i << 2;
            if (dest[j].r > k && src[j].r < 252) src[j].r += 4;
            if (dest[j].g > k && src[j].g < 252) src[j].g += 4;
            if (dest[j].b > k && src[j].b < 252) src[j].b += 4;
        }
        waitRetrace();
        setPalette(src);
    }
}

// Fade from current palette to destination palette
void fadeOut(RGB *dest)
{
    int32_t i, j, k;
    RGB src[256] = {0};

    getPalette(src);
    for (i = 63; i >= 0 && !keyPressed(27); i--)
    {
        for (j = 0; j < 256; j++)
        {
            k = i << 2;
            if (dest[j].r < k && src[j].r > 4) src[j].r -= 4;
            if (dest[j].g < k && src[j].g > 4) src[j].g -= 4;
            if (dest[j].b < k && src[j].b > 4) src[j].b -= 4;
        }
        waitRetrace();
        setPalette(src);
    }
}

// Fade from current palette to maximun palette
void fadeMax()
{
    int32_t i, j;
    RGB src[256] = {0};

    getPalette(src);
    for (i = 0; i < 64 && !keyPressed(27); i++)
    {
        for (j = 0; j < 256; j++)
        {
            if (src[j].r < 252) src[j].r += 4; else src[j].r = 255;
            if (src[j].g < 252) src[j].g += 4; else src[j].g = 255;
            if (src[j].b < 252) src[j].b += 4; else src[j].b = 255;
        }
        waitRetrace();
        setPalette(src);
    }
}

// Fade from current palette to minimum palette
void fadeMin()
{
    int32_t i, j;
    RGB src[256] = {0};

    getPalette(src);
    for (i = 0; i < 64 && !keyPressed(27); i++)
    {
        for (j = 0; j < 256; j++)
        {
            if (src[j].r > 4) src[j].r -= 4; else src[j].r = 0;
            if (src[j].g > 4) src[j].g -= 4; else src[j].g = 0;
            if (src[j].b > 4) src[j].b -= 4; else src[j].b = 0;
        }
        waitRetrace();
        setPalette(src);
    }
}

// downto current palette
void fadeDown(RGB *pal)
{
    int32_t i;
    for (i = 0; i < 256; i++)
    {
        if (pal[i].r > 4) pal[i].r -= 2; else pal[i].r = 0;
        if (pal[i].g > 4) pal[i].g -= 2; else pal[i].g = 0;
        if (pal[i].b > 4) pal[i].b -= 2; else pal[i].b = 0;
    }
    setPalette(pal);
}

// calculate lookup table for drawing circle
void calcCircle(int32_t r, int32_t *points)
{
    if (r <= 0) return;

    __asm {
        mov     ebx, 1
        sub     ebx, r
        mov     edi, points
        mov     esi, edi
        mov     eax, r
        shl     eax, 2
        add     esi, eax
        mov     eax, r
        xor     ecx, ecx
    start:
        or      ebx, ebx
        jns     next
        mov     edx, ecx
        shl     edx, 1
        add     edx, 3
        add     ebx, edx
        inc     ecx
        sub     esi, 4
        jmp     stop
    next:
        mov     edx, ecx
        sub     edx, eax
        shl     edx, 1
        add     edx, 3
        add     ebx, edx
        inc     ecx
        dec     eax
        sub     esi, 4
        add     edi, 4
    stop:
        mov     [edi], ecx
        mov     [esi], eax
        cmp     eax, ecx
        jg      start
    }
}

// calculate lookup table for drawing ellipse
void calcEllipse(int32_t rx, int32_t ry, int32_t *points)
{
    int32_t r = 0, a = 0, b = 0;
    int32_t x = 0, mx = 0, my = 0;
    int32_t aq = 0, bq = 0, xd = 0, yd = 0;

    if (rx <= 0 || ry <= 0) return;

    __asm {
        mov     eax, rx
        mov     x, eax
        neg     eax
        mov     mx, eax
        xor     edx, edx
        mov     eax, rx
        mul     eax
        mov     aq, eax
        shl     eax, 1
        mov     xd, eax
        mov     eax, ry
        mul     eax
        mov     bq, eax
        shl     eax, 1
        mov     yd, eax
        mov     eax, rx
        mul     bq
        mov     r, eax
        shl     eax, 1
        mov     a, eax
    next:
        cmp     r, 0
        jle     skip
        inc     my
        mov     eax, b
        add     eax, xd
        mov     b, eax
        sub     r, eax
    skip:
        cmp     r, 0
        jg      quit
        dec     x
        inc     mx
        mov     eax, a
        sub     eax, yd
        mov     a, eax
        add     r, eax
    quit:
        mov     edi, points
        mov     ebx, ry
        sub     ebx, my
        shl     ebx, 2
        add     edi, ebx
        mov     eax, mx
        neg     eax
        stosd
        cmp     x, 0
        jg      next
    }
}

// Very fast fill circle and ellipse
void fillCircle(int32_t xc, int32_t yc, int32_t rad, uint32_t col)
{
    int32_t i;
    int32_t points[500] = {0};

    if (rad <= 0) return;
    if (rad >= 500) fatalError("fillCircle: radius must be [0-499].\n");

    yc -= rad;
    calcCircle(rad, points);

    for (i = 0; i <= rad - 1; i++, yc++) horizLine(xc - points[i], yc, points[i] << 1, col);
    for (i = rad - 1; i >= 0; i--, yc++) horizLine(xc - points[i], yc, points[i] << 1, col);
}

void fillEllipse(int32_t xc, int32_t yc, int32_t rx, int32_t ry, uint32_t col)
{
    int32_t i;
    int32_t points[500] = {0};

    if (rx <= 0 || ry <= 0) return;
    if (rx >= 500 || ry >= 500) fatalError("fillEllipse: rx, ry must be [0-499].\n");

    yc -= ry;

    if (rx != ry) calcEllipse(rx, ry, points);
    else calcCircle(rx, points);

    for (i = 0; i <= ry - 1; i++, yc++) horizLine(xc - points[i], yc, points[i] << 1, col);
    for (i = ry - 1; i >= 0; i--, yc++) horizLine(xc - points[i], yc, points[i] << 1, col);
}

// Fill polygon using Darel Rex Finley algorithm
// Test vectors (screen resolution: 800x600)
// pt1[] = {{300, 100}, {192, 209}, {407, 323}, {320, 380}, {214, 350}, {375, 209}};
// pt2[] = {{169, 164}, {169, 264}, {223, 300}, {296, 209}, {214, 255}, {223, 200}, {386, 192}, {341, 273}, {404, 300}, {431, 146}};
// pt3[] = {{97, 56}, {115, 236}, {205, 146}, {276, 146}, {151, 325}, {259, 433}, {510, 344}, {510, 218}, {242, 271}, {384, 110}};
// pt4[] = {{256, 150}, {148, 347}, {327, 329}, {311, 204}, {401, 204}, {418, 240}, {257, 222}, {293, 365}, {436, 383}, {455, 150}};
// pt5[] = {{287, 76}, {129, 110}, {42, 301}, {78, 353}, {146, 337}, {199, 162}, {391, 180}, {322, 353}, {321, 198}, {219, 370}, {391, 405}, {444, 232}, {496, 440}, {565, 214}};
void fillPolygon(POINT *points, int32_t num, uint32_t col)
{
    int32_t nodex[MAX_POLY_CORNERS] = {0};
    int32_t nodes = 0, y = 0, i = 0, j = 0, swap = 0;
    int32_t left = 0, right = 0, top = 0, bottom = 0;

    // initialize clipping
    left = right = points[0].x;
    top = bottom = points[0].y;

    // clipping points
    for (i = 1; i != num; i++)
    {
        if (points[i].x < left) left = points[i].x;
        if (points[i].x > right) right = points[i].x;
        if (points[i].y < top) top = points[i].y;
        if (points[i].y > bottom) bottom = points[i].y;
    }

    // loop through the rows of the image
    for (y = top; y != bottom; y++)
    {
        // build a list of polygon intercepts on the current line
        nodes = 0;
        j = num - 1;

        for (i = 0; i != num; i++)
        {
            // intercept found, record it
            if ((points[i].y < y && points[j].y >= y) || (points[j].y < y && points[i].y >= y)) nodex[nodes++] = points[i].x + (y - points[i].y) / (points[j].y - points[i].y) * (points[j].x - points[i].x);
            if (nodes >= MAX_POLY_CORNERS) return;
            j = i;
        }

        // sort the nodes, via a simple "Bubble" sort
        i = 0;
        while (i < nodes - 1)
        {
            if (nodex[i] > nodex[i + 1])
            {
                swap = nodex[i];
                nodex[i] = nodex[i + 1];
                nodex[i + 1] = swap;
                if (i) i--;
            }
            else i++;
        }

        // fill the pixels between node pairs
        for (i = 0; i < nodes; i += 2)
        {
            if (nodex[i] >= right) break;
            if (nodex[i + 1] > left)
            {
                if (nodex[i] < left) nodex[i] = left;
                if (nodex[i + 1] > right) nodex[i + 1] = right;
                horizLine(nodex[i], y, nodex[i + 1] - nodex[i], col);
            }
        }
    }
}

// generate uniform random number distribution
double uniformRand(double min, double max)
{
    return (double)rand() / (RAND_MAX + 1.0) * (max - min) + min;
}

// generate normal random number distribution
double gaussianRand(double mean, double stddev)
{
    double x, y, r, d, n1;
    static double n2 = 0.0;
    static int32_t cached = 0;
    
    if (!cached)
    {
        do
        {
            x = 2.0 * rand() / RAND_MAX - 1;
            y = 2.0 * rand() / RAND_MAX - 1;
            r = x * x + y * y;
        } while (r == 0.0 || r > 1.0);

        d = sqrt(-2.0 * log(r) / r);
        n1 = x * d;
        n2 = y * d;
        cached = 1;
        return n1 * stddev + mean;
    }

    cached = 0;
    return n2 * stddev + mean;
}

//generate random polygon
void randomPolygon(int32_t cx, int32_t cy, int32_t avgRadius, double irregularity, double spikeyness, int32_t numVerts, POINT* points)
{
    int32_t i = 0;
    double koef = 0;
    double tmp, sum = 0;
    double lower, upper;
    double angle, radius;
    double gaussValue, rad;
    double angleSteps[1024] = { 0 };
    
    //valid params
    spikeyness = clamp(spikeyness, 0, 1) * avgRadius;
    irregularity = clamp(irregularity, 0, 1) * 2 * M_PI / numVerts;
    
    //generate n angle steps
    lower = (2 * M_PI / numVerts) - irregularity;
    upper = (2 * M_PI / numVerts) + irregularity;
    
    for (i = 0; i < numVerts; i++)
    {
        tmp = uniformRand(lower, upper);
        angleSteps[i] = tmp;
        sum += tmp;
    }

    //normalize the steps so that points 0 and points n+1 are the same
    koef = sum / (2 * M_PI);
    for (i = 0; i < numVerts; i++) angleSteps[i] /= koef;

    //now generate the points
    angle = uniformRand(0, 2 * M_PI);
    radius = 2.0 * avgRadius;

    for (i = 0; i < numVerts; i++)
    {
        gaussValue = gaussianRand(avgRadius, spikeyness);
        rad = clamp(gaussValue, 0, radius);
        points[i].x = cx + rad * cos(angle);
        points[i].y = cy + rad * sin(angle);
        angle += angleSteps[i];
    }
}

// another fade functions
void fadeCircle(int32_t dir, uint32_t col)
{
    int32_t i, x, y;

    switch (dir)
    {
    case 0:
        for (i = 0; i < 29 && !keyPressed(27); i++)
        {
            waitRetrace();
            for (y = 0; y <= cmaxY / 40; y++)
            for (x = 0; x <= cmaxX / 40; x++) fillCircle(x * 40 + 20, y * 40 + 20, i, col);
        }
        break;

    case 1:
        for (i = -cmaxY / 40; i < 29 && !keyPressed(27); i++)
        {
            waitRetrace();
            for (y = 0; y <= cmaxY / 40; y++)
            for (x = 0; x <= cmaxX / 40; x++)
            if (cmaxY / 40 - y + i < 29) fillCircle(x * 40 + 20, y * 40 + 20, cmaxY / 40 - y + i, col);
        }
        break;

    case 2:
        for (i = -cmaxX / 40; i < 29 && !keyPressed(27); i++)
        {
            waitRetrace();
            for (y = 0; y <= cmaxY / 40; y++)
            for (x = 0; x <= cmaxX / 40; x++)
            if (cmaxX / 40 - x + i < 29) fillCircle(x * 40 + 20, y * 40 + 20, cmaxX / 40 - x + i, col);
        }
        break;

    case 3:
        for (i = -cmaxX / 40; i < 60 && !keyPressed(27); i++)
        {
            waitRetrace();
            for (y = 0; y <= cmaxY / 40; y++)
            for (x = 0; x <= cmaxX / 40; x++)
            if (cmaxX / 40 - x - y + i < 29) fillCircle(x * 40 + 20, y * 40 + 20, cmaxX / 40 - x - y + i, col);
        }
        break;
    }
}

void fadeRollo(int32_t dir, uint32_t col)
{
    int32_t i, j;

    switch (dir)
    {
    case 0:
        for (i = 0; i < 20 && !keyPressed(27); i++)
        {
            waitRetrace();
            for (j = 0; j <= cmaxY / 10; j++) horizLine(0, j * 20 + i, cmaxX, col);
        }
        break;

    case 1:
        for (i = 0; i < 20 && !keyPressed(27); i++)
        {
            waitRetrace();
            for (j = 0; j <= cmaxX / 10; j++) vertLine(j * 20 + i, 0, cmaxY, col);
        }
        break;

    case 2:
        for (i = 0; i < 20 && !keyPressed(27); i++)
        {
            waitRetrace();
            for (j = 0; j <= cmaxX / 10; j++)
            {
                vertLine(j * 20 + i, 0, cmaxY, col);
                if (j * 10 < cmaxY) horizLine(0, j * 20 + i, cmaxX, col);
            }
        }
        break;
    }
}

// Cohen-Sutherland clipping line
int32_t getCode(int32_t x, int32_t y)
{
    int32_t code = 0;
    if (y > cmaxY) code |= CLIP_TOP;
    else if (y < cminY) code |= CLIP_BOTTOM;
    if (x > cmaxX) code |= CLIP_RIGHT;
    else if (x < cminX) code |= CLIP_LEFT;
    return code;
}

// Cohen-Sutherland clipping line (xs,ys)-(xe, ye)
void clipLine(int32_t* xs, int32_t* ys, int32_t* xe, int32_t* ye)
{
    int32_t x, y, accepted = 0;
    int32_t outcode, outcode0, outcode1;
    int32_t x0 = *xs, y0 = *ys, x1 = *xe, y1 = *ye;

    // the region out codes for the endpoints
    outcode0 = getCode(x0, y0);
    outcode1 = getCode(x1, y1);

    while (1)
    {
        // accept because both endpoints are in screen or on the border, trivial accept
        if (!(outcode0 | outcode1))
        {
            accepted = 1;
            break;
        }

        // the line isn't visible on screen, trivial reject
        else if (outcode0 & outcode1) break;

        // if no trivial reject or accept, continue the loop
        else
        {
            outcode = outcode0 ? outcode0 : outcode1;
            if (outcode & CLIP_TOP)
            {
                // top
                x = x0 + (x1 - x0) * (cmaxY - y0) / (y1 - y0);
                y = cmaxY;
            }
            else if (outcode & CLIP_BOTTOM)
            {
                // bottom
                x = x0 + (x1 - x0) * (cminY - y0) / (y1 - y0);
                y = cminY;
            }
            else if (outcode & CLIP_RIGHT)
            {
                // right
                y = y0 + (y1 - y0) * (cmaxX - x0) / (x1 - x0);
                x = cmaxX;
            }
            else
            {
                // left
                y = y0 + (y1 - y0) * (cminX - x0) / (x1 - x0);
                x = cminX;
            }

            if (outcode == outcode0)
            {
                // first endpoint was clipped
                x0 = x;
                y0 = y;
                outcode0 = getCode(x0, y0);
            }
            else
            {
                // second endpoint was clipped
                x1 = x;
                y1 = y;
                outcode1 = getCode(x1, y1);
            }
        }
    }

    // accepted line
    if (accepted)
    {
        *xs = x0; *ys = y0;
        *xe = x1; *ye = y1;
    }
}

// Drawing functions (using Xiaolin Wu’s algorithm with support anti-aliased)
void drawLine(int32_t x0, int32_t y0, int32_t x1, int32_t y1, uint32_t rgb)
{
    int32_t e2 = 0;
    int32_t dx = abs(x1 - x0);
    int32_t sx = (x0 < x1) ? 1 : -1;
    int32_t dy = -abs(y1 - y0);
    int32_t sy = (y0 < y1) ? 1 : -1;
    int32_t err = dx + dy;

    while (1)
    {
        putPixel(x0, y0, rgb);
        e2 = err << 1;

        if (e2 >= dy)
        {
            if (x0 == x1) break;
            err += dy;
            x0 += sx;
        }

        if (e2 <= dx)
        {
            if (y0 == y1) break;
            err += dx;
            y0 += sy;
        }
    }
}

void drawLineBob(int32_t x0, int32_t y0, int32_t x1, int32_t y1)
{
    int32_t e2 = 0;
    int32_t dx = abs(x1 - x0);
    int32_t sx = (x0 < x1) ? 1 : -1;
    int32_t dy = -abs(y1 - y0);
    int32_t sy = (y0 < y1) ? 1 : -1;
    int32_t err = dx + dy;

    while (1)
    {
        putPixelBob(x0, y0);
        e2 = err << 1;

        if (e2 >= dy)
        {
            if (x0 == x1) break;
            err += dy;
            x0 += sx;
        }

        if (e2 <= dx)
        {
            if (y0 == y1) break;
            err += dx;
            y0 += sy;
        }
    }
}

void drawLineAdd(int32_t x0, int32_t y0, int32_t x1, int32_t y1, uint32_t rgb)
{
    int32_t e2 = 0;
    int32_t dx = abs(x1 - x0);
    int32_t sx = (x0 < x1) ? 1 : -1;
    int32_t dy = -abs(y1 - y0);
    int32_t sy = (y0 < y1) ? 1 : -1;
    int32_t err = dx + dy;

    while (1)
    {
        putPixelAdd(x0, y0, rgb);
        e2 = err << 1;

        if (e2 >= dy)
        {
            if (x0 == x1) break;
            err += dy;
            x0 += sx;
        }

        if (e2 <= dx)
        {
            if (y0 == y1) break;
            err += dx;
            y0 += sy;
        }
    }
}

void drawLineSub(int32_t x0, int32_t y0, int32_t x1, int32_t y1, uint32_t rgb)
{
    int32_t e2 = 0;
    int32_t dx = abs(x1 - x0);
    int32_t sx = (x0 < x1) ? 1 : -1;
    int32_t dy = -abs(y1 - y0);
    int32_t sy = (y0 < y1) ? 1 : -1;
    int32_t err = dx + dy;

    while (1)
    {
        putPixelSub(x0, y0, rgb);
        e2 = err << 1;

        if (e2 >= dy)
        {
            if (x0 == x1) break;
            err += dy;
            x0 += sx;
        }

        if (e2 <= dx)
        {
            if (y0 == y1) break;
            err += dx;
            y0 += sy;
        }
    }
}

void drawCircle(int32_t xm, int32_t ym, int32_t rad, uint32_t rgb)
{
    int32_t x, y, err;

    if (rad <= 0) return;

    x = -rad;
    y = 0;
    err = (1 - rad) << 1;

    do {
        putPixel(xm - x, ym + y, rgb);
        putPixel(xm - y, ym - x, rgb);
        putPixel(xm + x, ym - y, rgb);
        putPixel(xm + y, ym + x, rgb);
        rad = err;
        if (rad <= y) err += ++y * 2 + 1;
        if (rad > x || err > y) err += ++x * 2 + 1;
    } while (x < 0);
}

void drawCircleAdd(int32_t xm, int32_t ym, int32_t rad, uint32_t rgb)
{
    int32_t x, y, err;

    if (rad <= 0) return;

    x = -rad;
    y = 0;
    err = (1 - rad) << 1;

    do {
        putPixelAdd(xm - x, ym + y, rgb);
        putPixelAdd(xm - y, ym - x, rgb);
        putPixelAdd(xm + x, ym - y, rgb);
        putPixelAdd(xm + y, ym + x, rgb);
        rad = err;
        if (rad <= y) err += ++y * 2 + 1;
        if (rad > x || err > y) err += ++x * 2 + 1;
    } while (x < 0);
}

void drawCircleSub(int32_t xm, int32_t ym, int32_t rad, uint32_t rgb)
{
    int32_t x, y, err;

    if (rad <= 0) return;

    x = -rad;
    y = 0;
    err = (1 - rad) << 1;

    do {
        putPixelSub(xm - x, ym + y, rgb);
        putPixelSub(xm - y, ym - x, rgb);
        putPixelSub(xm + x, ym - y, rgb);
        putPixelSub(xm + y, ym + x, rgb);
        rad = err;
        if (rad <= y) err += ++y * 2 + 1;
        if (rad > x || err > y) err += ++x * 2 + 1;
    } while (x < 0);
}

void drawEllipse(int32_t xm, int32_t ym, int32_t xr, int32_t yr, uint32_t rgb)
{
    int32_t e2, dx;
    int32_t x, y, dy, err;
    
    if (xr <= 0 || yr <= 0) return;

    x = -xr;
    y = 0;
    e2 = yr;
    dx = (1 + 2 * x) * e2 * e2;
    dy = x * x;
    err = dx + dy;

    do {
        putPixel(xm - x, ym + y, rgb);
        putPixel(xm + x, ym + y, rgb);
        putPixel(xm + x, ym - y, rgb);
        putPixel(xm - x, ym - y, rgb);

        e2 = err << 1;
        if (e2 >= dx)
        {
            x++;
            err += dx += 2 * yr * yr;
        }
        if (e2 <= dy)
        {
            y++;
            err += dy += 2 * xr * xr;
        }
    } while (x <= 0);

    while (y++ < yr)
    {
        putPixel(xm, ym + y, rgb);
        putPixel(xm, ym - y, rgb);
    }
}

void drawEllipseAdd(int32_t xm, int32_t ym, int32_t xr, int32_t yr, uint32_t rgb)
{
    int32_t e2, dx;
    int32_t x, y, dy, err;
    
    if (xr <= 0 || yr <= 0) return;

    x = -xr;
    y = 0;
    e2 = yr;
    dx = (1 + 2 * x) * e2 * e2;
    dy = x * x;
    err = dx + dy;

    do {
        putPixelAdd(xm - x, ym + y, rgb);
        putPixelAdd(xm + x, ym + y, rgb);
        putPixelAdd(xm + x, ym - y, rgb);
        putPixelAdd(xm - x, ym - y, rgb);

        e2 = err << 1;
        if (e2 >= dx)
        {
            x++;
            err += dx += 2 * yr * yr;
        }
        if (e2 <= dy)
        {
            y++;
            err += dy += 2 * xr * xr;
        }
    } while (x <= 0);

    while (y++ < yr)
    {
        putPixelAdd(xm, ym + y, rgb);
        putPixelAdd(xm, ym - y, rgb);
    }
}

void drawEllipseSub(int32_t xm, int32_t ym, int32_t xr, int32_t yr, uint32_t rgb)
{
    int32_t e2, dx;
    int32_t x, y, dy, err;
    
    if (xr <= 0 || yr <= 0) return;

    x = -xr;
    y = 0;
    e2 = yr;
    dx = (1 + 2 * x) * e2 * e2;
    dy = x * x;
    err = dx + dy;

    do {
        putPixelSub(xm - x, ym + y, rgb);
        putPixelSub(xm + x, ym + y, rgb);
        putPixelSub(xm + x, ym - y, rgb);
        putPixelSub(xm - x, ym - y, rgb);

        e2 = err << 1;
        if (e2 >= dx)
        {
            x++;
            err += dx += 2 * yr * yr;
        }
        if (e2 <= dy)
        {
            y++;
            err += dy += 2 * xr * xr;
        }
    } while (x <= 0);

    while (y++ < yr)
    {
        putPixelSub(xm, ym + y, rgb);
        putPixelSub(xm, ym - y, rgb);
    }
}

void drawEllipseRect(int32_t x0, int32_t y0, int32_t x1, int32_t y1, uint32_t rgb)
{
    int32_t a = abs(x1 - x0), b = abs(y1 - y0), b1 = b & 1;
    double dx = 4 * (1.0 - a) * b * b, dy = 4 * (b1 + 1) * a * a;
    double err = dx + dy + b1 * a * a, e2;

    if (a <= 0 || b <= 0) return;
    if (x0 > x1) { x0 = x1; x1 += a; }
    if (y0 > y1) y0 = y1;

    y0 += (b + 1) / 2;
    y1 = y0 - b1;

    a = 8 * a * a;
    b1 = 8 * b * b;

    do {            
        putPixel(x1, y0, rgb);
        putPixel(x0, y0, rgb);
        putPixel(x0, y1, rgb);
        putPixel(x1, y1, rgb);
        e2 = 2 * err;
        if (e2 <= dy) { y0++; y1--; err += dy += a; }
        if (e2 >= dx || 2 * err > dy) { x0++; x1--; err += dx += b1; }
    } while (x0 <= x1);

    while (y0 - y1 <= b)
    {
        putPixel(x0 - 1, y0, rgb);
        putPixel(x1 + 1, y0++, rgb);
        putPixel(x0 - 1, y1, rgb);
        putPixel(x1 + 1, y1--, rgb);
    }
}

void drawEllipseRectAdd(int32_t x0, int32_t y0, int32_t x1, int32_t y1, uint32_t rgb)
{
    int32_t a = abs(x1 - x0), b = abs(y1 - y0), b1 = b & 1;
    double dx = 4 * (1.0 - a) * b * b, dy = 4 * (b1 + 1) * a * a;
    double err = dx + dy + b1 * a * a, e2;

    if (a <= 0 || b <= 0) return;
    if (x0 > x1) { x0 = x1; x1 += a; }
    if (y0 > y1) y0 = y1;

    y0 += (b + 1) / 2;
    y1 = y0 - b1;

    a = 8 * a * a;
    b1 = 8 * b * b;

    do {            
        putPixelAdd(x1, y0, rgb);
        putPixelAdd(x0, y0, rgb);
        putPixelAdd(x0, y1, rgb);
        putPixelAdd(x1, y1, rgb);
        e2 = 2 * err;
        if (e2 <= dy) { y0++; y1--; err += dy += a; }
        if (e2 >= dx || 2 * err > dy) { x0++; x1--; err += dx += b1; }
    } while (x0 <= x1);

    while (y0 - y1 <= b)
    {
        putPixelAdd(x0 - 1, y0, rgb);
        putPixelAdd(x1 + 1, y0++, rgb);
        putPixelAdd(x0 - 1, y1, rgb);
        putPixelAdd(x1 + 1, y1--, rgb);
    }
}

void drawEllipseRectSub(int32_t x0, int32_t y0, int32_t x1, int32_t y1, uint32_t rgb)
{
    int32_t a = abs(x1 - x0), b = abs(y1 - y0), b1 = b & 1;
    double dx = 4 * (1.0 - a) * b * b, dy = 4 * (b1 + 1) * a * a;
    double err = dx + dy + b1 * a * a, e2;

    if (a <= 0 || b <= 0) return;
    if (x0 > x1) { x0 = x1; x1 += a; }
    if (y0 > y1) y0 = y1;

    y0 += (b + 1) / 2;
    y1 = y0 - b1;

    a = 8 * a * a;
    b1 = 8 * b * b;

    do {            
        putPixelSub(x1, y0, rgb);
        putPixelSub(x0, y0, rgb);
        putPixelSub(x0, y1, rgb);
        putPixelSub(x1, y1, rgb);
        e2 = 2 * err;
        if (e2 <= dy) { y0++; y1--; err += dy += a; }
        if (e2 >= dx || 2 * err > dy) { x0++; x1--; err += dx += b1; }
    } while (x0 <= x1);

    while (y0 - y1 <= b)
    {
        putPixelSub(x0 - 1, y0, rgb);
        putPixelSub(x1 + 1, y0++, rgb);
        putPixelSub(x0 - 1, y1, rgb);
        putPixelSub(x1 + 1, y1--, rgb);
    }
}

void drawRect(int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint32_t col)
{
    horizLine(x1, y1, x2 - x1 + 1, col);
    vertLine(x1, y1, y2 - y1 + 1, col);
    horizLine(x1, y2, x2 - x1 + 1, col);
    vertLine(x2, y1, y2 - y1 + 1, col);
}

void drawRectAdd(int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint32_t col)
{
    horizLineAdd(x1, y1, x2 - x1 + 1, col);
    vertLineAdd(x1, y1, y2 - y1 + 1, col);
    horizLineAdd(x1, y2, x2 - x1 + 1, col);
    vertLineAdd(x2, y1, y2 - y1 + 1, col);
}

void drawRectSub(int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint32_t col)
{
    horizLineSub(x1, y1, x2 - x1 + 1, col);
    vertLineSub(x1, y1, y2 - y1 + 1, col);
    horizLineSub(x1, y2, x2 - x1 + 1, col);
    vertLineSub(x2, y1, y2 - y1 + 1, col);
}

void drawRectEx(int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t rad, uint32_t col)
{
    int32_t width, mid, x, y;
    int32_t points[500] = {0};

    mid = (y2 - y1) >> 1;
    if (rad >= mid - 1) rad = mid - 1;

    width = abs(x2 - x1);
    calcCircle(rad, points);

    horizLine(x1 + rad - points[0], y1, width - (rad - points[0]) * 2 + 1, col);
    vertLine(x1, y1 + rad, y2 - y1 - rad * 2 + 1, col);
    horizLine(x1 + rad - points[0], y2, width - (rad - points[0]) * 2 + 1, col);
    vertLine(x2, y1 + rad, y2 - y1 - rad * 2 + 1, col);

    for (y = 1; y <= rad; y++)
    {
        for (x = rad - points[y]; x <= rad - points[y - 1]; x++)
        {
            putPixel(x1 + x, y1 + y, col);
            putPixel(x2 - x, y1 + y, col);
        }
    }

    for (y = 1; y <= rad; y++)
    {
        for (x = rad - points[y]; x <= rad - points[y - 1]; x++)
        {
            putPixel(x1 + x, y2 - y, col);
            putPixel(x2 - x, y2 - y, col);
        }
    }
}

void drawRectExAdd(int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t rad, uint32_t col)
{
    int32_t width, mid, x, y;
    int32_t points[500] = {0};

    mid = (y2 - y1) >> 1;
    if (rad >= mid - 1) rad = mid - 1;

    width = abs(x2 - x1);
    calcCircle(rad, points);

    horizLineAdd(x1 + rad - points[0], y1, width - (rad - points[0]) * 2 + 1, col);
    vertLineAdd(x1, y1 + rad, y2 - y1 - rad * 2 + 1, col);
    horizLineAdd(x1 + rad - points[0], y2, width - (rad - points[0]) * 2 + 1, col);
    vertLineAdd(x2, y1 + rad, y2 - y1 - rad * 2 + 1, col);

    for (y = 1; y <= rad; y++)
    {
        for (x = rad - points[y]; x <= rad - points[y - 1]; x++)
        {
            putPixelAdd(x1 + x, y1 + y, col);
            putPixelAdd(x2 - x, y1 + y, col);
        }
    }

    for (y = 1; y <= rad; y++)
    {
        for (x = rad - points[y]; x <= rad - points[y - 1]; x++)
        {
            putPixelAdd(x1 + x, y2 - y, col);
            putPixelAdd(x2 - x, y2 - y, col);
        }
    }
}

void drawRectExSub(int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t rad, uint32_t col)
{
    int32_t width, mid, x, y;
    int32_t points[500] = {0};

    mid = (y2 - y1) >> 1;
    if (rad >= mid - 1) rad = mid - 1;

    width = abs(x2 - x1);
    calcCircle(rad, points);

    horizLineSub(x1 + rad - points[0], y1, width - (rad - points[0]) * 2 + 1, col);
    vertLineSub(x1, y1 + rad, y2 - y1 - rad * 2 + 1, col);
    horizLineSub(x1 + rad - points[0], y2, width - (rad - points[0]) * 2 + 1, col);
    vertLineSub(x2, y1 + rad, y2 - y1 - rad * 2 + 1, col);

    for (y = 1; y <= rad; y++)
    {
        for (x = rad - points[y]; x <= rad - points[y - 1]; x++)
        {
            putPixelSub(x1 + x, y1 + y, col);
            putPixelSub(x2 - x, y1 + y, col);
        }
    }

    for (y = 1; y <= rad; y++)
    {
        for (x = rad - points[y]; x <= rad - points[y - 1]; x++)
        {
            putPixelSub(x1 + x, y2 - y, col);
            putPixelSub(x2 - x, y2 - y, col);
        }
    }
}

void drawBox(int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t dx, int32_t dy, uint32_t col)
{
    int32_t x11, y11;
    int32_t x22, y22;

    x11 = x1 + dx;
    y11 = y1 - dy;
    x22 = x2 + dx;
    y22 = y2 - dy;

    drawRect(x1, y1, x2, y2, col);
    drawRect(x11, y11, x22, y22, col);
    drawLine(x1, y1, x11, y11, col);
    drawLine(x2, y1, x22, y11, col);
    drawLine(x2, y2, x22, y22, col);
    drawLine(x1, y2, x11, y22, col);
}

void drawBoxAdd(int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t dx, int32_t dy, uint32_t col)
{
    int32_t x11, y11;
    int32_t x22, y22;

    x11 = x1 + dx;
    y11 = y1 - dy;
    x22 = x2 + dx;
    y22 = y2 - dy;

    drawRectAdd(x1, y1, x2, y2, col);
    drawRectAdd(x11, y11, x22, y22, col);
    drawLineAdd(x1, y1, x11, y11, col);
    drawLineAdd(x2, y1, x22, y11, col);
    drawLineAdd(x2, y2, x22, y22, col);
    drawLineAdd(x1, y2, x11, y22, col);
}

void drawBoxSub(int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t dx, int32_t dy, uint32_t col)
{
    int32_t x11, y11;
    int32_t x22, y22;

    x11 = x1 + dx;
    y11 = y1 - dy;
    x22 = x2 + dx;
    y22 = y2 - dy;

    drawRectSub(x1, y1, x2, y2, col);
    drawRectSub(x11, y11, x22, y22, col);
    drawLineSub(x1, y1, x11, y11, col);
    drawLineSub(x2, y1, x22, y11, col);
    drawLineSub(x2, y2, x22, y22, col);
    drawLineSub(x1, y2, x11, y22, col);
}

void drawBoxEx(int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t rad, uint32_t col)
{
    int32_t width, mid, x, y;
    int32_t points[500] = {0};

    mid = (y2 - y1) >> 1;
    if (rad >= mid - 1) rad = mid - 1;

    width = abs(x2 - x1);
    calcCircle(rad, points);

    horizLine(x1 + rad - points[0], y1, width - (rad - points[0]) * 2 + 1, col);
    vertLine(x1, y1 + rad, y2 - y1 - rad * 2 + 1, col);
    horizLine(x1 + rad - points[0], y2, width - (rad - points[0]) * 2 + 1, col);
    vertLine(x2, y1 + rad, y2 - y1 - rad * 2 + 1, col);

    for (y = 1; y <= rad; y++)
    {
        for (x = rad - points[y]; x <= rad - points[y - 1]; x++)
        {
            putPixel(x1 + x, y1 + y, col);
            putPixel(x2 - x, y1 + y, col);
        }
    }

    for (y = 1; y <= rad; y++)
    {
        for (x = rad - points[y]; x <= rad - points[y - 1]; x++)
        {
            putPixel(x1 + x, y2 - y, col);
            putPixel(x2 - x, y2 - y, col);
        }
    }

    for (y = 1; y <= rad; y++) horizLine(x1 + rad - points[y - 1] + 1, y1 + y, width - (rad * 2 - points[y - 1] * 2) - 1, col);
    fillRect(x1 + 1, y1 + rad + 1, x2 - 1, y2 - rad - 1, col);
    for (y = rad; y >= 1; y--) horizLine(x1 + rad - points[y - 1] + 1, y2 - y, width - (rad * 2 - points[y - 1] * 2) - 1, col);
}

void drawBoxExAdd(int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t rad, uint32_t col)
{
    int32_t width, mid, x, y;
    int32_t points[500] = {0};

    mid = (y2 - y1) >> 1;
    if (rad >= mid - 1) rad = mid - 1;

    width = abs(x2 - x1);
    calcCircle(rad, points);

    horizLineAdd(x1 + rad - points[0], y1, width - (rad - points[0]) * 2 + 1, col);
    vertLineAdd(x1, y1 + rad, y2 - y1 - rad * 2 + 1, col);
    horizLineAdd(x1 + rad - points[0], y2, width - (rad - points[0]) * 2 + 1, col);
    vertLineAdd(x2, y1 + rad, y2 - y1 - rad * 2 + 1, col);

    for (y = 1; y <= rad; y++)
    {
        for (x = rad - points[y]; x <= rad - points[y - 1]; x++)
        {
            putPixelAdd(x1 + x, y1 + y, col);
            putPixelAdd(x2 - x, y1 + y, col);
        }
    }

    for (y = 1; y <= rad; y++)
    {
        for (x = rad - points[y]; x <= rad - points[y - 1]; x++)
        {
            putPixelAdd(x1 + x, y2 - y, col);
            putPixelAdd(x2 - x, y2 - y, col);
        }
    }

    for (y = 1; y <= rad; y++) horizLineAdd(x1 + rad - points[y - 1] + 1, y1 + y, width - (rad * 2 - points[y - 1] * 2) - 1, col);
    fillRectAdd(x1 + 1, y1 + rad + 1, x2 - 1, y2 - rad - 1, col);
    for (y = rad; y >= 1; y--) horizLineAdd(x1 + rad - points[y - 1] + 1, y2 - y, width - (rad * 2 - points[y - 1] * 2) - 1, col);
}

void drawBoxExSub(int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t rad, uint32_t col)
{
    int32_t width, mid, x, y;
    int32_t points[500] = {0};

    mid = (y2 - y1) >> 1;
    if (rad >= mid - 1) rad = mid - 1;

    width = abs(x2 - x1);
    calcCircle(rad, points);

    horizLineSub(x1 + rad - points[0], y1, width - (rad - points[0]) * 2 + 1, col);
    vertLineSub(x1, y1 + rad, y2 - y1 - rad * 2 + 1, col);
    horizLineSub(x1 + rad - points[0], y2, width - (rad - points[0]) * 2 + 1, col);
    vertLineSub(x2, y1 + rad, y2 - y1 - rad * 2 + 1, col);

    for (y = 1; y <= rad; y++)
    {
        for (x = rad - points[y]; x <= rad - points[y - 1]; x++)
        {
            putPixelSub(x1 + x, y1 + y, col);
            putPixelSub(x2 - x, y1 + y, col);
        }
    }

    for (y = 1; y <= rad; y++)
    {
        for (x = rad - points[y]; x <= rad - points[y - 1]; x++)
        {
            putPixelSub(x1 + x, y2 - y, col);
            putPixelSub(x2 - x, y2 - y, col);
        }
    }

    for (y = 1; y <= rad; y++) horizLineSub(x1 + rad - points[y - 1] + 1, y1 + y, width - (rad * 2 - points[y - 1] * 2) - 1, col);
    fillRectSub(x1 + 1, y1 + rad + 1, x2 - 1, y2 - rad - 1, col);
    for (y = rad; y >= 1; y--) horizLineSub(x1 + rad - points[y - 1] + 1, y2 - y, width - (rad * 2 - points[y - 1] * 2) - 1, col);
}

void drawPoly(POINT *points, int32_t num, uint32_t col)
{
    int32_t i;

    if (num < 3) return;
    for (i = 0; i < num; i++)
    {
        drawLine(points[i].x, points[i].y, points[(i + 1) % num].x, points[(i + 1) % num].y, col);
    }
}

void drawPolyAdd(POINT *points, int32_t num, uint32_t col)
{
    int32_t i;

    if (num < 3) return;
    for (i = 0; i < num; i++)
    {
        drawLineAdd(points[i].x, points[i].y, points[(i + 1) % num].x, points[(i + 1) % num].y, col);
    }
}

void drawPolySub(POINT *points, int32_t num, uint32_t col)
{
    int32_t i;

    if (num < 3) return;
    for (i = 0; i < num; i++)
    {
        drawLineSub(points[i].x, points[i].y, points[(i + 1) % num].x, points[(i + 1) % num].y, col);
    }
}

void moveTo(int32_t x, int32_t y)
{
    currentX = x;
    currentY = y;
}

void lineTo(int32_t x, int32_t y, uint32_t col)
{
    drawLine(currentX, currentY, x, y, col);
    moveTo(x, y);
}

void lineToAdd(int32_t x, int32_t y, uint32_t col)
{
    drawLineAdd(currentX, currentY, x, y, col);
    moveTo(x, y);
}

void lineToSub(int32_t x, int32_t y, uint32_t col)
{
    drawLineSub(currentX, currentY, x, y, col);
    moveTo(x, y);
}

void putPixelAlpha(int32_t x, int32_t y, uint32_t rgb, uint8_t alpha)
{
    // Only 32bit support alpha-blend mode
    if (bytesPerPixel != 4) return;

    // Check clip boundary
    if (x < cminX || x > cmaxX || y < cminY || y > cmaxY) return;

#ifdef _USE_MMX
    __asm {
        mov         eax, y
        add         eax, pageOffset
        mul         lfbWidth
        add         eax, x
        shl         eax, 2
        mov         edi, lfbPtr
        add         edi, eax
        xor         eax, eax
        pxor        mm2, mm2
        mov         al, alpha
        neg         eax
        add         eax, 256
        movd        mm3, eax
        punpcklwd   mm3, mm3
        punpckldq   mm3, mm3
        movd        mm0, rgb
        movd        mm1, [edi]
        punpcklbw   mm0, mm2
        movq        mm4, mm1
        punpcklbw   mm1, mm2
        psubw       mm0, mm1
        pmullw      mm0, mm3
        psrlw       mm0, 8
        packuswb    mm0, mm2
        paddb       mm0, mm4
        movd        [edi], mm0
        emms
    }
#else
    __asm {
        mov         eax, y
        add         eax, pageOffset
        mul         lfbWidth
        add         eax, x
        shl         eax, 2
        mov         edi, lfbPtr
        add         edi, eax
        mov         al, [edi]
        mul         alpha
        mov         bx, ax
        mov         al, byte ptr[rgb]
        mov         cl, 255
        sub         cl, alpha
        mul         cl
        add         ax, bx
        shr         ax, 8
        stosb
        mov         al, [edi]
        mul         alpha
        mov         bx, ax
        mov         al, byte ptr[rgb + 1]
        mov         cl, 255
        sub         cl, alpha
        mul         cl
        add         ax, bx
        shr         ax, 8
        stosb
        mov         al, [edi]
        mul         alpha
        mov         bx, ax
        mov         al, byte ptr[rgb + 2]
        mov         cl, 255
        sub         cl, alpha
        mul         cl
        add         ax, bx
        shr         ax, 8
        stosb
    }
#endif
}

void drawLineAlpha(int32_t x0, int32_t y0, int32_t x1, int32_t y1, uint32_t rgb)
{
    int32_t x2 = 0, e2 = 0;
    int32_t dx = abs(x1 - x0);
    int32_t sx = x0 < x1 ? 1 : -1;
    int32_t dy = abs(y1 - y0);
    int32_t sy = y0 < y1 ? 1 : -1;
    int32_t err = dx - dy;
    int32_t ed = (dx + dy == 0) ? 1 : sqrt(dx * dx + dy * dy);

    while (1)
    {
        putPixelAlpha(x0, y0, rgb, 255 * abs(err - dx + dy) / ed);
        e2 = err;
        x2 = x0;

        if (2 * e2 >= -dx)
        {
            if (x0 == x1) break;
            if (e2 + dy < ed) putPixelAlpha(x0, y0 + sy, rgb, 255 * (e2 + dy) / ed);

            err -= dy;
            x0 += sx;
        }

        if (2 * e2 <= dy)
        {
            if (y0 == y1) break;
            if (dx - e2 < ed) putPixelAlpha(x2 + sx, y0, rgb, 255 * (dx - e2) / ed);

            err += dx;
            y0 += sy;
        }
    }
}

void lineToAlpha(int32_t x, int32_t y, uint32_t col)
{
    // Only 32bit support alpha-blend mode
    if (bytesPerPixel != 4) return;
    drawLineAlpha(currentX, currentY, x, y, col);
    moveTo(x, y);
}

void drawCircleAlpha(int32_t xm, int32_t ym, int32_t rad, uint32_t rgb)
{
    int32_t x, y, alpha, x2, e2, err;

    // Only 32bit support alpha-blend mode
    if (bytesPerPixel != 4) return;
    if (rad <= 0) return;

    x = -rad;
    y = 0;
    err = (1 - rad) << 1;
    rad = 1 - err;

    do {
        alpha = 255 * abs(err - 2 * (x + y) - 2) / rad;
        putPixelAlpha(xm - x, ym + y, rgb, alpha);
        putPixelAlpha(xm - y, ym - x, rgb, alpha);
        putPixelAlpha(xm + x, ym - y, rgb, alpha);
        putPixelAlpha(xm + y, ym + x, rgb, alpha);

        e2 = err;
        x2 = x;

        if (err + y > 0)
        {
            alpha = 255 * (err - 2 * x - 1) / rad;
            if (alpha < 256)
            {
                putPixelAlpha(xm - x, ym + y + 1, rgb, alpha);
                putPixelAlpha(xm - y - 1, ym - x, rgb, alpha);
                putPixelAlpha(xm + x, ym - y - 1, rgb, alpha);
                putPixelAlpha(xm + y + 1, ym + x, rgb, alpha);
            }

            err += ++x * 2 + 1;
        }

        if (e2 + x2 <= 0)
        {
            alpha = 255 * (2 * y + 3 - e2) / rad;
            if (alpha < 256)
            {
                putPixelAlpha(xm - x2 - 1, ym + y, rgb, alpha);
                putPixelAlpha(xm - y, ym - x2 - 1, rgb, alpha);
                putPixelAlpha(xm + x2 + 1, ym - y, rgb, alpha);
                putPixelAlpha(xm + y, ym + x2 + 1, rgb, alpha);
            }
            err += ++y * 2 + 1;
        }
    } while (x < 0);
}

void drawEllipseAlpha(int32_t x0, int32_t y0, int32_t x1, int32_t y1, uint32_t rgb)
{
    int32_t f;
    int32_t a, b, b1;
    double dx, dy, ed, alpha, err;

    // Only 32bit support alpha-blend mode
    if (bytesPerPixel != 4) return;
    if (x0 <= 0 || y0 <= 0 || x1 <= 0 || y1 <= 0) return;

    a = abs(x1 - x0);
    b = abs(y1 - y0);

    //check for line
    if (a <= 0 || b <= 0) return;

    b1 = b & 1;
    dx = 4.0 * (a - 1.0) * b * b;
    dy = 4.0 * (b1 + 1.0) * a * a;
    err = b1 * a * a - dx + dy;

    if (x0 > x1)
    {
        x0 = x1;
        x1 += a;
    }

    if (y0 > y1) y0 = y1;

    y0 += (b + 1) / 2;
    y1 = y0 - b1;
    a = 8 * a * a;
    b1 = 8 * b * b;

    while (1)
    {
        ed = max(dx, dy);
        alpha = min(dx, dy);

        if (y0 == y1 + 1 && err > dy && a > b1) ed = 255.0 * 4 / a;
        else ed = 255 / (ed + 2 * ed * alpha * alpha / (4 * ed * ed + alpha * alpha));

        alpha = ed * fabs(err + dx - dy);

        putPixelAlpha(x0, y0, rgb, alpha);
        putPixelAlpha(x0, y1, rgb, alpha);
        putPixelAlpha(x1, y0, rgb, alpha);
        putPixelAlpha(x1, y1, rgb, alpha);

        if ((f = 2 * err + dy >= 0))
        {
            if (x0 >= x1) break;
            alpha = ed * (err + dx);
            if (alpha < 255)
            {
                putPixelAlpha(x0, y0 + 1, rgb, alpha);
                putPixelAlpha(x0, y1 - 1, rgb, alpha);
                putPixelAlpha(x1, y0 + 1, rgb, alpha);
                putPixelAlpha(x1, y1 - 1, rgb, alpha);
            }
        }

        if (2 * err <= dx)
        {
            alpha = ed * (dy - err);
            if (alpha < 255)
            {
                putPixelAlpha(x0 + 1, y0, rgb, alpha);
                putPixelAlpha(x1 - 1, y0, rgb, alpha);
                putPixelAlpha(x0 + 1, y1, rgb, alpha);
                putPixelAlpha(x1 - 1, y1, rgb, alpha);
            }

            y0++;
            y1--;
            err += dy += a; 
        }

        if (f)
        {
            x0++;
            x1--;
            err -= dx -= b1;
        }
    }

    if (--x0 == x1++)
    {
        while (y0 - y1 < b)
        {
            alpha = 255.0 * 4 * fabs(err + dx) / b1;
            putPixelAlpha(x0, ++y0, rgb, alpha);
            putPixelAlpha(x1, y0, rgb, alpha);
            putPixelAlpha(x0, --y1, rgb, alpha);
            putPixelAlpha(x1, y1, rgb, alpha);
            err += dy += a; 
        }
    }
}

void drawQuadBezierSeg(int32_t x0, int32_t y0, int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint32_t rgb)
{
    int32_t sx = x2 - x1, sy = y2 - y1;
    int32_t xx = x0 - x1, yy = y0 - y1, xy;
    double dx, dy, err, cur = xx * sy - yy * sx;

    if (sx * sx + sy * sy > xx * xx + yy * yy)
    {
        x2 = x0;
        x0 = sx + x1;
        y2 = y0;
        y0 = sy + y1;
        cur = -cur;
    }

    if (cur != 0)
    {
        xx += sx;
        xx *= sx = x0 < x2 ? 1 : -1;
        yy += sy;
        yy *= sy = y0 < y2 ? 1 : -1;
        xy = 2 * xx * yy;
        xx *= xx;
        yy *= yy;

        if (cur * sx * sy < 0)
        {
            xx = -xx;
            yy = -yy;
            xy = -xy;
            cur = -cur;
        }

        dx = 4.0 * sy * cur * (x1 - x0) + xx - xy;
        dy = 4.0 * sx * cur * (y0 - y1) + yy - xy;
        xx += xx;
        yy += yy;
        err = dx + dy + xy;

        do {
            putPixel(x0, y0, rgb);
            if (x0 == x2 && y0 == y2) return;
            y1 = 2 * err < dx;
            if (2 * err > dy)
            {
                x0 += sx;
                dx -= xy;
                err += dy += yy;
            }
            if (y1)
            {
                y0 += sy;
                dy -= xy;
                err += dx += xx;
            }
        } while (dy < 0 && dx > 0);
    }
    drawLine(x0, y0, x2, y2, rgb);
}

void drawQuadBezier(int32_t x0, int32_t y0, int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint32_t rgb)
{
    int32_t x = x0 - x1, y = y0 - y1;
    double t = x0 - 2 * x1 + x2, r;

    if (x * (x2 - x1) > 0)
    {
        if ((y * (y2 - y1) > 0) && fabs((y0 - 2 * y1 + y2) / t * x) > abs(y))
        {
            x0 = x2; x2 = x + x1; y0 = y2; y2 = y + y1;
        }
        t = (x0 - x1) / t;
        r = (1 - t) * ((1 - t) * y0 + 2.0 * t * y1) + t * t * y2;
        t = (x0 * x2 - x1 * x1) * t / (x0 - x1);
        x = floor(t + 0.5);
        y = floor(r + 0.5);      
        r = (y1 - y0) * (t - x0) / (x1 - x0) + y0;
        drawQuadBezierSeg(x0, y0, x, floor(r + 0.5), x, y, rgb);
        r = (y1 - y2) * (t - x2) / (x1 - x2) + y2;
        x0 = x1 = x; y0 = y; y1 = floor(r + 0.5);
    }

    if ((y0 - y1) * (y2 - y1) > 0)
    {
        t = y0 - 2 * y1 + y2;
        t = (y0 - y1) / t;       
        r = (1 - t) * ((1 - t) * x0 + 2.0 * t * x1) + t * t * x2;
        t = (y0 * y2 - y1 * y1) * t / (y0 - y1);
        x = floor(r + 0.5);
        y = floor(t + 0.5);
        r = (x1 - x0) * (t - y0) / (y1 - y0) + x0;
        drawQuadBezierSeg(x0, y0, floor(r + 0.5), y, x, y, rgb);
        r = (x1 - x2) * (t - y2) / (y1 - y2) + x2;
        x0 = x; x1 = floor(r + 0.5); y0 = y1 = y;
    }
    drawQuadBezierSeg(x0, y0, x1, y1, x2, y2, rgb);
}

void drawQuadRationalBezierSeg(int32_t x0, int32_t y0, int32_t x1, int32_t y1, int32_t x2, int32_t y2, double w, uint32_t rgb)
{
    int32_t sx = x2 - x1, sy = y2 - y1;
    double dx = x0 - x2, dy = y0 - y2, xx = x0 - x1, yy = y0 - y1;
    double xy = xx * sy + yy * sx, cur = xx * sy - yy * sx, err;

    if (cur != 0.0 && w > 0.0)
    {
        if (sx * sx + sy * sy > xx * xx + yy * yy)
        {
            x2 = x0;
            x0 -= dx;
            y2 = y0;
            y0 -= dy;
            cur = -cur;
        }

        xx = 2.0 * (4.0 * w * sx * xx + dx * dx);
        yy = 2.0 * (4.0 * w * sy * yy + dy * dy);
        sx = x0 < x2 ? 1 : -1;
        sy = y0 < y2 ? 1 : -1;
        xy = -2.0 * sx * sy * (2.0 * w * xy + dx * dy);

        if (cur * sx * sy < 0.0)
        {
            xx = -xx;
            yy = -yy;
            xy = -xy;
            cur = -cur;
        }

        dx = 4.0 * w * (x1 - x0) * sy * cur + xx / 2.0 + xy;
        dy = 4.0 * w * (y0 - y1) * sx * cur + yy / 2.0 + xy;

        if (w < 0.5 && (dy > xy || dx < xy))
        {
            cur = (w + 1.0) / 2.0;
            w = sqrt(w);
            xy = 1.0 / (w + 1.0);
            sx = floor((x0 + 2.0 * w * x1 + x2) * xy / 2.0 + 0.5);
            sy = floor((y0 + 2.0 * w * y1 + y2) * xy / 2.0 + 0.5);
            dx = floor((w * x1 + x0) * xy + 0.5);
            dy = floor((y1 * w + y0) * xy + 0.5);
            drawQuadRationalBezierSeg(x0, y0, dx, dy, sx, sy, cur, rgb);
            dx = floor((w * x1 + x2) * xy + 0.5);
            dy = floor((y1 * w + y2) * xy + 0.5);
            drawQuadRationalBezierSeg(sx, sy, dx, dy, x2, y2, cur, rgb);
            return;
        }

        err = dx + dy - xy;

        do {
            putPixel(x0, y0, rgb);
            if (x0 == x2 && y0 == y2) return;
            x1 = 2 * err > dy;
            y1 = 2 * (err + yy) < -dy;
            if (2 * err < dx || y1) { y0 += sy; dy += xy; err += dx += xx; }
            if (2 * err > dx || x1) { x0 += sx; dx += xy; err += dy += yy; }
        } while (dy <= xy && dx >= xy);
    }
    drawLine(x0, y0, x2, y2, rgb);
}

void drawQuadRationalBezier(int32_t x0, int32_t y0, int32_t x1, int32_t y1, int32_t x2, int32_t y2, double w, uint32_t rgb)
{
    int32_t x = x0 - 2 * x1 + x2, y = y0 - 2 * y1 + y2;
    double xx = x0 - x1, yy = y0 - y1, ww, t, q;

    if (xx * (x2 - x1) > 0)
    {
        if (yy * (y2 - y1) > 0 && fabs(xx * y) > fabs(yy * x))
        {
            x0 = x2;
            x2 = xx + x1;
            y0 = y2;
            y2 = yy + y1;
        }

        if (x0 == x2 || w == 1.0) t = (x0 - x1) / (double)x;
        else
        {
            q = sqrt(4.0 * w * w * (x0 - x1) * (x2 - x1) + (x2 - x0) * (x2 - x0));
            if (x1 < x0) q = -q;
            t = (2.0 * w * (x0 - x1) - x0 + x2 + q) / (2.0 * (1.0 - w) * (x2 - x0));
        }

        q = 1.0 / (2.0 * t * (1.0 - t) * (w - 1.0) + 1.0);
        xx = (t * t * (x0 - 2.0 * w * x1 + x2) + 2.0 * t * (w * x1 - x0) + x0) * q;
        yy = (t * t * (y0 - 2.0 * w * y1 + y2) + 2.0 * t * (w * y1 - y0) + y0) * q;
        ww = t * (w - 1.0) + 1.0;
        ww *= ww * q;
        w = ((1.0 - t) * (w - 1.0) + 1.0) * sqrt(q);
        x = floor(xx + 0.5);
        y = floor(yy + 0.5);
        yy = (xx - x0) * (y1 - y0) / (x1 - x0) + y0;
        drawQuadRationalBezierSeg(x0, y0, x, floor(yy + 0.5), x, y, ww, rgb);
        yy = (xx - x2) * (y1 - y2) / (x1 - x2) + y2;
        y1 = floor(yy + 0.5);
        x0 = x1 = x;
        y0 = y;
    }

    if ((y0 - y1) * (y2 - y1) > 0)
    {
        if (y0 == y2 || w == 1.0) t = (y0 - y1) / (y0 - 2.0 * y1 + y2);
        else
        {
            q = sqrt(4.0 * w * w * (y0 - y1) * (y2 - y1) + (y2 - y0) * (y2-y0));
            if (y1 < y0) q = -q;
            t = (2.0 * w * (y0 - y1) - y0 + y2 + q) / (2.0 * (1.0 - w) * (y2 - y0));
        }
        q = 1.0 / (2.0 * t * (1.0 - t) * (w - 1.0) + 1.0);
        xx = (t * t * (x0 - 2.0 * w * x1 + x2) + 2.0 * t * (w * x1 - x0) + x0) * q;
        yy = (t * t * (y0 - 2.0 * w * y1 + y2) + 2.0 * t * (w * y1 - y0) + y0) * q;
        ww = t * (w - 1.0) + 1.0;
        ww *= ww * q;
        w = ((1.0 - t) * (w - 1.0) + 1.0) * sqrt(q);
        x = floor(xx + 0.5);
        y = floor(yy + 0.5);
        xx = (x1 - x0) * (yy - y0) / (y1 - y0) + x0;
        drawQuadRationalBezierSeg(x0, y0, floor(xx + 0.5), y, x, y, ww, rgb);
        xx = (x1 - x2) * (yy - y2) / (y1 - y2) + x2;
        x1 = floor(xx + 0.5);
        x0 = x;
        y0 = y1 = y;
    }
    drawQuadRationalBezierSeg(x0, y0, x1, y1, x2, y2, w * w, rgb);
}

void drawRotatedEllipseRect(int32_t x0, int32_t y0, int32_t x1, int32_t y1, int32_t zd, uint32_t rgb)
{
    int32_t xd = x1 - x0, yd = y1 - y0;
    double w = xd * yd;

    if (zd == 0) { drawEllipseRect(x0, y0, x1, y1, rgb); return; }
    if (w != 0.0) w = (w - zd) / (w + w);

    xd = floor(xd * w + 0.5);
    yd = floor(yd * w + 0.5);
    drawQuadRationalBezierSeg(x0, y0 + yd, x0, y0, x0 + xd, y0, 1.0 - w, rgb);
    drawQuadRationalBezierSeg(x0, y0 + yd, x0, y1, x1 - xd, y1, w, rgb);
    drawQuadRationalBezierSeg(x1, y1 - yd, x1, y1, x1 - xd, y1, 1.0 - w, rgb);
    drawQuadRationalBezierSeg(x1, y1 - yd, x1, y0, x0 + xd, y0, w, rgb);
}

void drawRotatedEllipse(int32_t x, int32_t y, int32_t a, int32_t b, double angle, uint32_t rgb)
{
    double xd = a * a, yd = b * b;
    double s = sin(angle), zd = (xd - yd) * s;
    xd = sqrt(xd - zd * s), yd = sqrt(yd + zd * s);
    a = xd + 0.5; b = yd + 0.5; zd = zd * a * b / (xd * yd);
    drawRotatedEllipseRect(x - a, y - b, x + a, y + b, 4 * zd * cos(angle), rgb);
}

void drawCubicBezierSeg(int32_t x0, int32_t y0, double x1, double y1, double x2, double y2, int32_t x3, int32_t y3, uint32_t rgb)
{
    int32_t f, fx, fy, leg = 1;
    int32_t sx = x0 < x3 ? 1 : -1, sy = y0 < y3 ? 1 : -1;
    double xc = -fabs(x0 + x1 - x2 - x3), xa = xc - 4 * sx * (x1 - x2), xb = sx * (x0 - x1 - x2 + x3);
    double yc = -fabs(y0 + y1 - y2 - y3), ya = yc - 4 * sy * (y1 - y2), yb = sy * (y0 - y1 - y2 + y3);
    double ab, ac, bc, cb, xx, xy, yy, dx, dy, ex, *pxy, EP = 0.01;

    if (xa == 0 && ya == 0)
    {
        sx = floor((3 * x1 - x0 + 1) / 2);
        sy = floor((3 * y1 - y0 + 1) / 2);
        drawQuadBezierSeg(x0, y0, sx, sy, x3, y3, rgb);
        return;
    }

    x1 = (x1 - x0) * (x1 - x0) + (y1 - y0) * (y1 - y0) + 1;
    x2 = (x2 - x3) * (x2 - x3) + (y2 - y3) * (y2 - y3) + 1;

    do {
        ab = xa * yb - xb * ya;
        ac = xa * yc - xc * ya;
        bc = xb * yc - xc * yb;
        ex = ab * (ab + ac - 3 * bc) + ac * ac;
        f = ex > 0 ? 1 : sqrt(1 + 1024 / x1);
        ab *= f; ac *= f; bc *= f; ex *= f * f;
        xy = 9 * (ab + ac + bc) / 8;
        cb = 8 * (xa - ya);
        dx = 27 * (8 * ab * (yb * yb - ya * yc) + ex * (ya + 2 * yb + yc)) / 64 - ya * ya * (xy - ya);
        dy = 27 * (8 * ab * (xb * xb - xa * xc) - ex * (xa + 2 * xb + xc)) / 64 - xa * xa * (xy + xa);

        xx = 3 * (3 * ab * (3 * yb * yb - ya * ya - 2 * ya * yc) - ya * (3 * ac * (ya + yb) + ya * cb)) / 4;
        yy = 3 * (3 * ab * (3 * xb * xb - xa * xa - 2 * xa * xc) - xa * (3 * ac * (xa + xb) + xa * cb)) / 4;
        xy = xa * ya * (6 * ab + 6 * ac - 3 * bc + cb); ac = ya * ya; cb = xa * xa;
        xy = 3 * (xy + 9 * f * (cb * yb * yc - xb * xc * ac) - 18 * xb * yb * ab) / 8;

        if (ex < 0)
        {
            dx = -dx; dy = -dy; xx = -xx; yy = -yy; xy = -xy; ac = -ac; cb = -cb;
        }

        ab = 6 * ya * ac; ac = -6 * xa * ac; bc = 6 * ya * cb; cb = -6 * xa * cb;
        dx += xy; ex = dx + dy; dy += xy;

        for (pxy = &xy, fx = fy = f; x0 != x3 && y0 != y3;)
        {
            putPixel(x0, y0, rgb);
            do {
                if (dx > *pxy || dy < *pxy) goto exit;
                y1 = 2 * ex - dy;
                if (2 * ex >= dx)
                {
                    fx--;
                    ex += dx += xx;
                    dy += xy += ac;
                    yy += bc;
                    xx += ab;
                }
                if (y1 <= 0)
                {
                    fy--;
                    ex += dy += yy;
                    dx += xy += bc;
                    xx += ac;
                    yy += cb;
                }
            } while (fx > 0 && fy > 0);

            if (2 * fx <= f) { x0 += sx; fx += f; }
            if (2 * fy <= f) { y0 += sy; fy += f; }
            if (pxy == &xy && dx < 0 && dy > 0) pxy = &EP;
        }
        exit:
        xx = x0; x0 = x3; x3 = xx; sx = -sx; xb = -xb;
        yy = y0; y0 = y3; y3 = yy; sy = -sy; yb = -yb; x1 = x2;
    } while (leg--);
    drawLine(x0, y0, x3, y3, rgb);
}

void drawCubicBezier(int32_t x0, int32_t y0, int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t x3, int32_t y3, uint32_t rgb)
{
    int32_t n = 0, i = 0;
    int32_t xc = x0 + x1 - x2 - x3, xa = xc - 4 * (x1 - x2);
    int32_t xb = x0 - x1 - x2 + x3, xd = xb + 4 * (x1 + x2);
    int32_t yc = y0 + y1 - y2 - y3, ya = yc - 4 * (y1 - y2);
    int32_t yb = y0 - y1 - y2 + y3, yd = yb + 4 * (y1 + y2);
    double fx0 = x0, fx1, fx2, fx3, fy0 = y0, fy1, fy2, fy3;
    double t1 = xb * xb - xa * xc, t2, t[5];

    if (xa == 0)
    {
        if (abs(xc) < 2 * abs(xb)) t[n++] = xc / (2.0 * xb);
    }
    else if (t1 > 0.0)
    {
        t2 = sqrt(t1);
        t1 = (xb - t2) / xa; if (fabs(t1) < 1.0) t[n++] = t1;
        t1 = (xb + t2) / xa; if (fabs(t1) < 1.0) t[n++] = t1;
    }

    t1 = yb * yb - ya * yc;
    if (ya == 0)
    {
        if (abs(yc) < 2 * abs(yb)) t[n++] = yc / (2.0 * yb);
    }
    else if (t1 > 0.0)
    {
        t2 = sqrt(t1);
        t1 = (yb - t2) / ya; if (fabs(t1) < 1.0) t[n++] = t1;
        t1 = (yb + t2) / ya; if (fabs(t1) < 1.0) t[n++] = t1;
    }

    for (i = 1; i != n; i++)
    {
        if ((t1 = t[i - 1]) > t[i])
        {
            t[i - 1] = t[i];
            t[i] = t1;
            i = 0;
        }
    }

    t1 = -1.0; t[n] = 1.0;
    for (i = 0; i <= n; i++)
    {
        t2 = t[i];
        fx1 = (t1 * (t1 * xb - 2 * xc) - t2 * (t1 * (t1 * xa - 2 * xb) + xc) + xd) / 8 - fx0;
        fy1 = (t1 * (t1 * yb - 2 * yc) - t2 * (t1 * (t1 * ya - 2 * yb) + yc) + yd) / 8 - fy0;
        fx2 = (t2 * (t2 * xb - 2 * xc) - t1 * (t2 * (t2 * xa - 2 * xb) + xc) + xd) / 8 - fx0;
        fy2 = (t2 * (t2 * yb - 2 * yc) - t1 * (t2 * (t2 * ya - 2 * yb) + yc) + yd) / 8 - fy0;
        fx0 -= fx3 = (t2 * (t2 * (3 * xb - t2 * xa) - 3 * xc) + xd) / 8;
        fy0 -= fy3 = (t2 * (t2 * (3 * yb - t2 * ya) - 3 * yc) + yd) / 8;
        x3 = floor(fx3 + 0.5);
        y3 = floor(fy3 + 0.5);
        if (fx0 != 0.0) { fx1 *= fx0 = (x0 - x3) / fx0; fx2 *= fx0; }
        if (fy0 != 0.0) { fy1 *= fy0 = (y0 - y3) / fy0; fy2 *= fy0; }
        if (x0 != x3 || y0 != y3) drawCubicBezierSeg(x0, y0, x0 + fx1, y0 + fy1, x0 + fx2, y0 + fy2, x3, y3, rgb);
        x0 = x3; y0 = y3; fx0 = fx3; fy0 = fy3; t1 = t2;
    }
}

void drawLineWidthAlpha(int32_t x0, int32_t y0, int32_t x1, int32_t y1, double wd, uint32_t rgb)
{
    int32_t dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1; 
    int32_t dy = abs(y1 - y0), sy = y0 < y1 ? 1 : -1; 
    int32_t err = dx - dy, e2, x2, y2;
    double ed = dx + dy == 0 ? 1 : sqrt(dx * dx + dy * dy);

    // Only 32bit support alpha-blend mode
    if (bytesPerPixel != 4) return;

    wd = (wd + 1) / 2;
    while (1)
    {
        putPixelAlpha(x0, y0, rgb, max(0, 255 * (abs(err - dx + dy) / ed - wd + 1)));

        e2 = err; x2 = x0;
        if (2 * e2 >= -dx)
        {
            for (e2 += dy, y2 = y0; e2 < ed * wd && (y1 != y2 || dx > dy); e2 += dx) putPixelAlpha(x0, y2 += sy, rgb, max(0, 255 * (abs(e2) / ed - wd + 1)));
            if (x0 == x1) break;
            e2 = err;
            err -= dy;
            x0 += sx; 
        }
        if (2 * e2 <= dy)
        {
            for (e2 = dx-e2; e2 < ed * wd && (x1 != x2 || dx < dy); e2 += dy) putPixelAlpha(x2 += sx, y0, rgb, max(0, 255 * (abs(e2) / ed - wd + 1)));
            if (y0 == y1) break;
            err += dx;
            y0 += sy; 
        }
    }
}

void drawQuadBezierSegBlend(int32_t x0, int32_t y0, int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint32_t rgb)
{
    int32_t sx = x2 - x1, sy = y2 - y1;
    int32_t xx = x0 - x1, yy = y0 - y1, xy;
    double dx, dy, err, ed, cur = xx * sy - yy * sx;

    // Only 32bit support alpha-blend mode
    if (bytesPerPixel != 4) return;

    if (sx * sx + sy * sy > xx * xx + yy * yy)
    {
        x2 = x0;
        x0 = sx + x1;
        y2 = y0;
        y0 = sy + y1;
        cur = -cur;
    }

    if (cur != 0)
    {
        xx += sx; xx *= sx = x0 < x2 ? 1 : -1;
        yy += sy; yy *= sy = y0 < y2 ? 1 : -1;
        xy = 2 * xx * yy;
        xx *= xx;
        yy *= yy;
        if (cur * sx * sy < 0)
        {
            xx = -xx;
            yy = -yy;
            xy = -xy;
            cur = -cur;
        }

        dx = 4.0 * sy * (x1 - x0) * cur + xx - xy;
        dy = 4.0 * sx * (y0 - y1) * cur + yy - xy;
        xx += xx; yy += yy; err = dx + dy + xy;

        do {
            cur = min(dx + xy, -xy - dy);
            ed = max(dx + xy, -xy - dy);
            ed += 2 * ed * cur * cur / (4 * ed * ed + cur * cur);
            putPixelAlpha(x0, y0, rgb, 255 * fabs(err - dx - dy - xy) / ed);

            if (x0 == x2 || y0 == y2) break;

            x1 = x0; cur = dx - err; y1 = 2 * err + dy < 0;
            if (2 * err + dx > 0)
            {
                if (err - dy < ed) putPixelAlpha(x0, y0 + sy, rgb, 255 * fabs(err - dy) / ed);
                x0 += sx; dx -= xy; err += dy += yy;
            }
            if (y1)
            {
                if (cur < ed) putPixelAlpha(x1 + sx, y0, rgb, 255 * fabs(cur) / ed);
                y0 += sy; dy -= xy; err += dx += xx;
            }
        } while (dy < dx);
    }
    drawLineAlpha(x0, y0, x2, y2, rgb);
}

void drawQuadRationalBezierSegAlpha(int32_t x0, int32_t y0, int32_t x1, int32_t y1, int32_t x2, int32_t y2, double w, uint32_t rgb)
{
    int32_t f;
    int32_t sx = x2 - x1, sy = y2 - y1;
    double dx = x0 - x2, dy = y0 - y2, xx = x0 - x1, yy = y0 - y1;
    double xy = xx * sy + yy * sx, cur = xx * sy - yy * sx, err, ed;
    
    // Only 32bit support alpha-blend mode
    if (bytesPerPixel != 4) return;

    if (cur != 0.0 && w > 0.0)
    {
        if (sx * sx + sy * sy > xx * xx + yy * yy)
        {
            x2 = x0; x0 -= dx; y2 = y0; y0 -= dy; cur = -cur;
        }

        xx = 2.0 * (4.0 * w * sx * xx + dx * dx);
        yy = 2.0 * (4.0 * w * sy * yy + dy * dy);
        sx = x0 < x2 ? 1 : -1;
        sy = y0 < y2 ? 1 : -1;
        xy = -2.0 * sx * sy * (2.0 * w * xy + dx * dy);

        if (cur * sx * sy < 0)
        {
            xx = -xx; yy = -yy; cur = -cur; xy = -xy;
        }

        dx = 4.0 * w * (x1 - x0) * sy * cur + xx / 2.0 + xy;
        dy = 4.0 * w * (y0 - y1) * sx * cur + yy / 2.0 + xy;

        if (w < 0.5 && dy > dx)
        {
            cur = (w + 1.0) / 2.0;
            w = sqrt(w);
            xy = 1.0 / (w + 1.0);
            sx = floor((x0 + 2.0 * w * x1 + x2) * xy / 2.0 + 0.5);
            sy = floor((y0 + 2.0 * w * y1 + y2) * xy / 2.0 + 0.5);
            dx = floor((w * x1 + x0) * xy + 0.5);
            dy = floor((y1 * w + y0) * xy + 0.5);
            drawQuadRationalBezierSegAlpha(x0, y0, dx, dy, sx, sy, cur, rgb);
            dx = floor((w * x1 + x2) * xy + 0.5);
            dy = floor((y1 * w + y2) * xy + 0.5);
            drawQuadRationalBezierSegAlpha(sx, sy, dx, dy, x2, y2, cur, rgb);
            return;
        }

        err = dx + dy - xy;

        do {
            cur = min(dx - xy, xy - dy);
            ed = max(dx - xy, xy - dy);
            ed += 2 * ed * cur * cur / (4. * ed * ed + cur * cur);
            x1 = 255 * fabs(err - dx - dy + xy) / ed;
            if (x1 < 256) putPixelAlpha(x0, y0, rgb, x1);
            if (f = 2 * err + dy < 0)
            {
                if (y0 == y2) return;
                if (dx - err < ed) putPixelAlpha(x0 + sx, y0, rgb, 255 * fabs(dx - err) / ed);
            }
            if (2 * err + dx > 0)
            {
                if (x0 == x2) return;
                if (err - dy < ed) putPixelAlpha(x0, y0 + sy, rgb, 255 * fabs(err - dy) / ed);
                x0 += sx; dx += xy; err += dy += yy;
            }
            if (f) { y0 += sy; dy += xy; err += dx += xx; }
        } while (dy < dx);
    }
    drawLineAlpha(x0, y0, x2, y2, rgb);
}

void drawCubicBezierSegAlpha(int32_t x0, int32_t y0, double x1, double y1, double x2, double y2, int32_t x3, int32_t y3, uint32_t rgb)
{
    int32_t f, fx, fy, leg = 1;
    int32_t sx = x0 < x3 ? 1 : -1, sy = y0 < y3 ? 1 : -1;
    double xc = -fabs(x0 + x1 - x2 - x3), xa = xc - 4 * sx * (x1 - x2), xb = sx * (x0 - x1 - x2 + x3);
    double yc = -fabs(y0 + y1 - y2 - y3), ya = yc - 4 * sy * (y1 - y2), yb = sy * (y0 - y1 - y2 + y3);
    double ab, ac, bc, ba, xx, xy, yy, dx, dy, ex, px, py, ed, ip, EP = 0.01;

    // Only 32bit support alpha-blend mode
    if (bytesPerPixel != 4) return;

    if (xa == 0 && ya == 0)
    {
        sx = floor((3 * x1 - x0 + 1) / 2);
        sy = floor((3 * y1 - y0 + 1) / 2);
        drawQuadBezierSegBlend(x0, y0, sx, sy, x3, y3, rgb);
        return;
    }

    x1 = (x1 - x0) * (x1 - x0) + (y1 - y0) * (y1 - y0) + 1;
    x2 = (x2 - x3) * (x2 - x3) + (y2 - y3) * (y2 - y3) + 1;

    do {
        ab = xa * yb - xb * ya;
        ac = xa * yc - xc * ya;
        bc = xb * yc - xc * yb;
        ip = 4 * ab * bc - ac * ac;
        ex = ab * (ab + ac - 3 * bc) + ac * ac;
        f = ex > 0 ? 1 : sqrt(1 + 1024 / x1);
        ab *= f; ac *= f; bc *= f; ex *= f * f;
        xy = 9 * (ab + ac + bc) / 8;
        ba = 8 * (xa - ya);
        dx = 27 * (8 * ab * (yb * yb - ya * yc) + ex * (ya + 2 * yb + yc)) / 64 - ya * ya * (xy - ya);
        dy = 27 * (8 * ab * (xb * xb - xa * xc) - ex * (xa + 2 * xb + xc)) / 64 - xa * xa * (xy + xa);

        xx = 3 * (3 * ab * (3 * yb * yb - ya * ya - 2 * ya * yc) - ya * (3 * ac * (ya + yb) + ya * ba)) / 4;
        yy = 3 * (3 * ab * (3 * xb * xb - xa * xa - 2 * xa * xc) - xa * (3 * ac * (xa + xb) + xa * ba)) / 4;
        xy = xa * ya * (6 * ab + 6 * ac - 3 * bc + ba); ac = ya * ya; ba = xa * xa;
        xy = 3 * (xy + 9 * f * (ba * yb * yc - xb * xc * ac) - 18 * xb * yb * ab) / 8;

        if (ex < 0)
        {
            dx = -dx; dy = -dy; xx = -xx; yy = -yy; xy = -xy; ac = -ac; ba = -ba;
        }

        ab = 6 * ya * ac; ac = -6 * xa * ac; bc = 6 * ya * ba; ba = -6 * xa * ba;
        dx += xy; ex = dx + dy; dy += xy;

        for (fx = fy = f; x0 != x3 && y0 != y3;)
        {
            y1 = min(fabs(xy - dx), fabs(dy - xy));
            ed = max(fabs(xy - dx), fabs(dy - xy));
            ed = f * (ed + 2 * ed * y1 * y1 / (4 * ed * ed + y1 * y1));
            y1 = 255 * fabs(ex - (f - fx + 1) * dx - (f - fy + 1) * dy + f * xy) / ed;
            if (y1 < 256) putPixelAlpha(x0, y0, rgb, y1);
            px = fabs(ex - (f - fx + 1) * dx + (fy - 1) * dy);
            py = fabs(ex + (fx - 1) * dx - (f - fy + 1) * dy);
            y2 = y0;
            do {
                if ((ip >= -EP) && (dx + xx > xy || dy + yy < xy)) goto exit;
                y1 = 2 * ex + dx;
                if (2 * ex + dy > 0)
                {
                    fx--; ex += dx += xx; dy += xy += ac; yy += bc; xx += ab;
                } else if (y1 > 0) goto exit;
                if (y1 <= 0)
                {
                    fy--; ex += dy += yy; dx += xy += bc; xx += ac; yy += ba;
                }
            } while (fx > 0 && fy > 0);

            if (2 * fy <= f)
            {
                if (py < ed) putPixelAlpha(x0 + sx, y0, rgb, 255 * py / ed);
                y0 += sy; fy += f;
            }

            if (2 * fx <= f)
            {
                if (px < ed) putPixelAlpha(x0, y2 + sy, rgb, 255 * px / ed);
                x0 += sx; fx += f;
            }
        }

        exit:
        if (2 * ex < dy && 2 * fy <= f + 2)
        {
            if (py < ed) putPixelAlpha(x0 + sx, y0, rgb, 255 * py / ed);
            y0 += sy;
        }

        if (2 * ex > dx && 2 * fx <= f + 2)
        {
            if (px < ed) putPixelAlpha(x0, y2 + sy, rgb, 255 * px / ed);
            x0 += sx;
        }

        xx = x0; x0 = x3; x3 = xx; sx = -sx; xb = -xb;
        yy = y0; y0 = y3; y3 = yy; sy = -sy; yb = -yb; x1 = x2;
    } while (leg--);
    drawLineAlpha(x0, y0, x3, y3, rgb);
}

void initStack(GFX_STACK *stack)
{
    stack->count = 0;
    memset(stack->data, -1, sizeof(stack->data)); 
}

void push(GFX_STACK *stack, int32_t item)
{
    if (stack->count > MAX_STACK_SIZE) fatalError("push: Stack Full (n > %d).\n", MAX_STACK_SIZE);
    stack->data[stack->count++] = item;
}

int32_t pop(GFX_STACK *stack)
{
    if (stack->count == 0) fatalError("pop: Stack Empty.\n");
    return stack->data[--stack->count];
}

int32_t isEmpty(GFX_STACK *stack)
{
    return stack->count == 0;
}

int32_t scanLeft(int32_t sx, int32_t sy, uint32_t fcol, uint32_t bcol)
{
    uint32_t col;

    do {
        sx--;
        col = getPixel(sx, sy);
    } while (col != bcol && col != fcol && sx > 0);

    return (!sx) ? -1 : ++sx;
}

int32_t scanRight(int32_t sx, int32_t sy, uint32_t fcol, uint32_t bcol)
{
    uint32_t col;

    do {
        sx++;
        col = getPixel(sx, sy);
    } while (col != bcol && col != fcol && sx < 2000);

    return (sx == 2000) ? -1 : --sx;
}

void regionFill(int32_t seedx, int32_t seedy, uint32_t bcol, uint32_t fcol)
{
    GFX_STACK stack;
    int32_t sx, sy;
    int32_t xl, xll;
    int32_t xr, xrr;

    initStack(&stack);
    push(&stack, seedx);
    push(&stack, seedy);

    while (!isEmpty(&stack))
    {
        sy = pop(&stack);
        sx = pop(&stack);

        xl = scanLeft(sx, sy, fcol, bcol);
        xr = scanRight(sx, sy, fcol, bcol);

        horizLine(xl, sy, xr - xl + 1, fcol);
        sy++;

        xll = scanLeft((xl + xr) >> 1, sy, fcol, bcol);
        xrr = scanRight((xl + xr) >> 1, sy, fcol, bcol);

        if (xll == -1 || xrr == MAX_STACK_SIZE) break;

        if (xll < xrr)
        {
            push(&stack, (xll + xrr) >> 1);
            push(&stack, sy);
        }
    }

    initStack(&stack);
    push(&stack, seedx);
    push(&stack, seedy - 1);

    while (!isEmpty(&stack))
    {
        sy = pop(&stack);
        sx = pop(&stack);

        xl = scanLeft(sx, sy, fcol, bcol);
        xr = scanRight(sx, sy, fcol, bcol);

        horizLine(xl, sy, xr - xl + 1, fcol);
        sy--;

        xll = scanLeft((xl + xr) >> 1, sy, fcol, bcol);
        xrr = scanRight((xl + xr) >> 1, sy, fcol, bcol);

        if (xll < 0 || xrr >= MAX_STACK_SIZE) break;

        if (xll < xrr)
        {
            push(&stack, (xll + xrr) >> 1);
            push(&stack, sy);
        }
    }
}

void createPlasma(uint8_t *dx, uint8_t *dy, uint8_t *sint, uint8_t *cost, GFX_IMAGE *img)
{
    uint8_t lx = (*dx) += 2;
    uint8_t ly = (*dy)--;

    void *data = img->mData;
    uint32_t ofs = img->mWidth;
    uint8_t sx = img->mWidth >> 1;
    uint8_t sy = img->mHeight >> 1;

    __asm {
        mov     edi, data
        xor     eax, eax
        xor     dh, dh
    next0:
        xor     bh, bh
        mov     al, dh
        mov     bl, ly
        add     ax, bx
        cmp     ax, 255
        jbe     skip0
        sub     ax, 255
    skip0:
        mov     esi, sint
        add     esi, eax
        mov     cl, [esi]
        mov     al, lx
        mov     esi, sint
        add     esi, eax
        mov     ch, [esi]
        xor     dl, dl
    next1:
        xor     bh, bh
        mov     al, dl
        mov     bl, cl
        add     ax, bx
        cmp     ax, 255
        jbe     skip1
        sub     ax, 255
    skip1:
        mov     esi, sint
        add     esi, eax
        mov     bl, [esi]
        mov     al, dh
        add     al, ch
        mov     esi, cost
        add     esi, eax
        add     bl, [esi]
        shr     bl, 1
        add     bl, 128
        mov     bh, bl
        mov     esi, ofs
        mov     [edi], bx
        mov     [edi + esi], bx
        add     edi, 2
        inc     dl
        cmp     dl, sx
        jb      next1
        add     edi, esi
        inc     dh
        cmp     dh, sy
        jb      next0
    }
}

void initPlasma(uint8_t *sint, uint8_t *cost)
{
    int32_t i;
    RGB pal[256] = {0};

    for (i = 0; i < 256; i++)
    {
        sint[i] = sin(2 * M_PI * i / 255) * 128 + 128;
        cost[i] = cos(2 * M_PI * i / 255) * 128 + 128;
    }

    for (i = 0; i < 64; i++)
    {
        setRGB(i, i, 0, i << 2);
        setRGB(127 - i, i, 0, i << 2);
        setRGB(127 + i, i << 2, i << 1, 0);
        setRGB(254 - i, i << 2, i << 1, 0);
    }

    getPalette(pal);
    for (i = 127; i >= 0; i--) setRGB(i + 128, pal[i << 1].r, pal[i << 1].g, pal[i << 1].b);
}

void prepareTunnel(GFX_IMAGE *dimg, uint8_t *buff1, uint8_t *buff2)
{
    const int32_t maxAng = 2048;
    const double angDec = 0.85;
    const double dstInc = 0.02;
    const double preCalc = M_PI / (maxAng >> 2);

    const int32_t dcx = dimg->mWidth >> 1;
    const int32_t dcy = dimg->mHeight >> 1;

    int32_t x, y, ofs;
    double z = 250.0;
    double dst = 1.0;
    double ang = maxAng - 1.0;

    do {
        x = fround(z * sin(ang * preCalc)) + dcx;
        y = fround(z * cos(ang * preCalc)) + dcy;

        ang -= angDec;
        if (ang < 0)
        {
            ang += maxAng;
            dst += dst * dstInc;
            z -= angDec;
        }

        if (x >= 0 && x < dimg->mWidth && y >= 0 && y < dimg->mHeight)
        {
            ofs = y * dimg->mWidth + x;
            buff1[ofs] = fround(dst);
            buff2[ofs] = fround(dst - ang / 4);
        }
    } while (z >= 0);
}

void drawTunnel(GFX_IMAGE *dimg, GFX_IMAGE *simg, uint8_t *buff1, uint8_t *buff2, uint8_t *mov, uint8_t step)
{
    uint8_t val = 0;
    uint32_t size = dimg->mSize >> 2;
    uint32_t *dst = (uint32_t*)dimg->mData;
    uint32_t *src = (uint32_t*)simg->mData;

    if (bytesPerPixel != 4) return;

    *mov += step;
    while (size--)
    {
        val = *buff1++ + *mov;
        *dst++ = *(src + ((val << 8) | *buff2++));
    }
}

inline uint32_t RGB2INT(uint8_t r, uint8_t g, uint8_t b)
{
    return (r << 16) | (g << 8) | (b);
}

inline RGB INT2RGB(uint32_t color)
{
    RGB rgb;
    rgb.r = (color >> 16) & 0xFF;
    rgb.g = (color >> 8 ) & 0xFF;
    rgb.b = (color      ) & 0xFF;
    return rgb;
}

void blurImageEx(GFX_IMAGE *dst, GFX_IMAGE *src, int32_t blur)
{
    void *pdst = dst->mData;
    void *psrc = src->mData;
    uint32_t size = src->mSize >> 2;

    // only support 32bit color
    if (bytesPerPixel != 4) return;

    // check for small source size
    if (blur <= 0 || size <= 2 * blur) return;

    // check for max blur
    if (blur > 127) blur = 127;

    __asm {
        mov     esi, psrc
        mov     edi, pdst
        mov     ecx, size
        sub     ecx, blur
        sub     ecx, blur
        push    ecx
        mov     eax, 4
        xor     ebx, ebx
    nr1bx:
        xor     ecx, ecx
        push    esi
        push    edi
        push    eax
    lp1xb:
        xor     eax, eax
        mov     al, [esi]
        push    edi
        mov     edx, blur
        add     eax, edx
        mov     edi, 1
    lpi1xb:
        mov     bl, [esi + edi * 4]
        add     eax, ebx
        cmp     edi, ecx
        ja      nb1xb
        neg     edi
        mov     bl, [esi + edi * 4]
        neg     edi
        add     eax, ebx
        inc     edx
    nb1xb:
        inc     edi
        cmp     edi, blur
        jnae    lpi1xb
        pop     edi
        mov     ebx, edx
        xor     edx, edx
        div     ebx
        mov     edx, ebx
        mov     [edi], al
        add     esi, 4
        add     edi, 4
        inc     ecx
        cmp     ecx, blur
        jne     lp1xb
        pop     eax
        pop     edi
        pop     esi
        inc     edi
        inc     esi
        dec     eax
        jnz     nr1bx
        dec     ecx
        shl     ecx, 2
        add     esi, ecx
        add     edi, ecx
        pop     ecx
        mov     edx, blur
        add     edx, edx
        inc     edx
        mov     eax, 4
    nrxb:
        push    ecx
        push    esi
        push    edi
        push    eax
    lpxb:
        xor     eax, eax
        mov     al, [esi]
        push    ecx
        mov     ecx, blur
        add     eax, ecx
    lpixb:
        mov     bl, [esi + ecx * 4]
        neg     ecx
        add     eax, ebx
        mov     bl, [esi + ecx * 4]
        neg     ecx
        add     eax, ebx
        loop    lpixb
        pop     ecx
        mov     ebx, edx
        xor     edx, edx
        div     ebx
        mov     edx, ebx
        mov     [edi], al
        add     esi, 4
        add     edi, 4
        loop    lpxb
        pop     eax
        pop     edi
        pop     esi
        pop     ecx
        inc     edi
        inc     esi
        dec     eax
        jnz     nrxb
        dec     ecx
        shl     ecx, 2
        add     esi, ecx
        add     edi, ecx
        mov     eax, 4
    nr2xb:
        mov     ecx, blur
        push    esi
        push    edi
        push    eax
    lp2xb:
        xor     eax, eax
        mov     al, [esi]
        push    edi
        mov     edx, blur
        add     eax, edx
        mov     edi, 1
    lpi2xb:
        cmp     edi, ecx
        jae     nb2xb
        mov     bl, [esi + edi * 4]
        add     eax, ebx
        inc     edx
    nb2xb:
        neg     edi
        mov     bl, [esi + edi * 4]
        neg     edi
        add     eax, ebx
        inc     edi
        cmp     edi, blur
        jnae    lpi2xb
        pop     edi
        mov     ebx, edx
        xor     edx, edx
        div     ebx
        mov     edx, ebx
        mov     [edi], al
        add     esi, 4
        add     edi, 4
        loop    lp2xb
        pop     eax
        pop     edi
        pop     esi
        inc     edi
        inc     esi
        dec     eax
        jnz     nr2xb
    }
}

void brightnessImage(GFX_IMAGE *dst, GFX_IMAGE *src, uint8_t bright)
{
    void *psrc = src->mData;
    void *pdst = dst->mData;
    uint32_t size = src->mSize >> 2;

    // only support 32bit color
    if (bytesPerPixel != 4) return;

    // check light range
    if (bright == 0 || bright == 255) return;

    __asm {
        mov     ecx, size
        mov     edi, pdst
        mov     esi, psrc
        xor     edx, edx
        mov     dl, bright
    next:
        mov     ebx, [esi]
        mov     al, bh
        and     ebx, 00FF00FFh
        imul    ebx, edx
        shr     ebx, 8
        mul     dl
        mov     bh, ah
        mov     eax, ebx
        stosd
        add     esi, 4
        loop    next
    }
}

void blockOutMid(void *dst, void *src, uint32_t count, uint32_t blk)
{
    __asm {
        mov     edx, count
        mov     ebx, blk
        mov     edi, dst
        mov     esi, src
        mov     ecx, ebx
        shr     ecx, 17
        lea     esi, [esi + ecx * 4 + 4]
        mov     ecx, ebx
        shr     ecx, 16
        and     ebx, 00FFFFh
    next:
        mov     eax, [esi]
        sub     edx, ecx
        cmp     edx, 0
        jg      skip
        add     ecx, edx
        rep     stosd
        jmp     quit
    skip:
        lea     esi, [esi + ecx * 4]
        rep     stosd
        mov     ecx, ebx
        cmp     edx, 0
        jg      next
    quit:
    }
}

void brightnessAlpha(GFX_IMAGE *img, uint8_t bright)
{
    void *data = img->mData;
    uint32_t size = img->mSize >> 2;

    // only support 32bit color
    if (bytesPerPixel != 4) return;

    // check for minimum
    if (bright == 0 || bright == 255) return;

    __asm {
        mov     ecx, size
        mov     edi, data
        xor     eax, eax
        mov     bl, bright
    next:
        mov     al, [edi + 3]
        mul     bl
        mov     [edi + 3], ah
        add     edi, 4
        dec     ecx
        jnz     next
    }
}

void blockOutMidImage(GFX_IMAGE *dst, GFX_IMAGE *src, int32_t xb, int32_t yb)
{
    int32_t y, mid, cx, cy;
    uint32_t *pdst = (uint32_t*)dst->mData;
    uint32_t *psrc = (uint32_t*)src->mData;

    // only support 32bit color
    if (bytesPerPixel != 4) return;

    // check minimum blocking
    if (xb == 0) xb = 1;
    if (yb == 0) yb = 1;

    // nothing to do, make source and destination are the same
    if (xb == 1 && yb == 1) memcpy(pdst, psrc, src->mSize);
    else
    {
        // calculate deltax, deltay
        cx = (((src->mWidth >> 1) % xb) << 16) | xb;
        cy = (yb - (src->mHeight >> 1) % yb);

        // process line by line
        for (y = 0; y != src->mHeight; y++)
        {
            // blocking line by line
            if ((y + cy) % yb == 0 || y == 0)
            {
                mid = y + (cy >> 1);
                if (mid >= src->mHeight) mid = (src->mHeight + y) >> 1;
                blockOutMid(pdst, &psrc[mid * src->mWidth], src->mWidth, cx);
            }
            // already blocking, copy it
            else memcpy(pdst, pdst - dst->mWidth, dst->mRowBytes);
            pdst += dst->mWidth;
        }
    }
}

void fadeOutCircle(double pc, int32_t size, int32_t type, uint32_t col)
{
    int32_t val, dsize, max, x, y;

    if (pc > 100) pc = 100;
    if (pc < 0 ) pc = 0;

    max = size * 1.4;
    dsize = size << 1;

    switch (type)
    {
    case 0:
        for (y = 0; y < lfbHeight / dsize; y++)
        for (x = 0; x < lfbWidth / dsize; x++)
        fillCircle(x * dsize + size, y * dsize + size, max * pc / 100, col);
        break;
    case 1:
        for (y = 0; y < lfbHeight / dsize; y++)
        for (x = 0; x < lfbWidth / dsize; x++)
        {
            val = (max + (lfbHeight / dsize - y) * 2) * pc / 100;
            if (val > max) val = max;
            fillCircle(x * dsize + size, y * dsize + size, val, col);
        }
        break;
    case 2:
        for (y = 0; y < lfbHeight / dsize; y++)
        for (x = 0; x < lfbWidth / dsize; x++)
        {
            val = (max + (lfbWidth / dsize - x) * 2) * pc / 100;
            if (val > max) val = max;
            fillCircle(x * dsize + size, y * dsize + size, val, col);
        }
        break;
    case 3:
        for (y = 0; y < lfbHeight / dsize; y++)
        for (x = 0; x < lfbWidth / dsize; x++)
        {
            val = (max + (lfbWidth / size - (x + y))) * pc / 100;
            if (val > max) val = max;
            fillCircle(x * dsize + size, y * dsize + size, val, col);
        }
        break;
    }
}

void scaleUpLine(void *dst, void *src, void *tables, int32_t count, int32_t yval)
{
    __asm {
        mov     ecx, count
        mov     ebx, src
        add     ebx, yval
        mov     esi, tables
        mov     edi, dst
    next:
        lodsd
        mov     eax, [ebx + eax * 4]
        stosd
        dec     ecx
        jnz     next
    }
}

void scaleUpImage(GFX_IMAGE *dst, GFX_IMAGE *src, int32_t *tables, int32_t xfact, int32_t yfact)
{
    int32_t i, y;
    uint32_t *pdst = (uint32_t*)dst->mData;

    if (bytesPerPixel != 4) fatalError("scaleUpImage: only 32 bits supported.\n");

    // init lookup table
    for (i = 0; i < src->mWidth; i++) tables[i] = fround(1.0 * i / (src->mWidth - 1) * ((src->mWidth - 1) - (xfact << 1))) + xfact;

    // scaleup line by line
    for (i = 0; i < src->mHeight; i++)
    {
        y = fround(1.0 * i / (src->mHeight - 1) * ((src->mHeight - 1) - (yfact << 1))) + yfact;
        scaleUpLine(pdst, src->mData, tables, src->mWidth, y * src->mRowBytes);
        pdst += dst->mWidth;
    }
}

void blurImage(GFX_IMAGE *img)
{
    uint32_t tmp = 0;
    uint32_t width  = img->mWidth;
    uint32_t height = img->mHeight;
    void *data = img->mData;

    if (bytesPerPixel != 4) fatalError("blurImage: only 32 bits supported.\n");

    __asm {
        mov     edi, data
    step:
        mov     edx, width
        sub     edx, 2
        mov     tmp, edx
        mov     edx, width
        shl     edx, 2
        mov     ebx, [edi]
        mov     esi, [edi + 4]
        and     ebx, 00FF00FFh
        and     esi, 00FF00FFh
        add     ebx, ebx
        mov     ecx, [edi + edx]
        add     esi, ebx
        and     ecx, 00FF00FFh
        add     esi, ecx
        mov     al, [edi + 5]
        mov     bl, [edi + 1]
        add     ebx, ebx
        mov     cl, [edi + edx + 1]
        add     eax, ebx
        xor     ebx, ebx
        shr     esi, 2
        add     eax, ecx
        and     esi, 00FF00FFh
        shl     eax, 6
        and     eax, 0000FF00h
        or      eax, esi
        stosd
    next:
        mov     esi, [edi - 4]
        mov     ecx, [edi + 4]
        and     esi, 00FF00FFh
        and     ecx, 00FF00FFh
        mov     ebx, [edi]
        add     esi, ecx
        and     ebx, 00FF00FFh
        mov     ecx, [edi + edx]
        add     esi, ebx
        and     ecx, 00FF00FFh
        xor     eax, eax
        mov     al, [edi - 3]
        add     esi, ecx
        mov     cl, [edi + 5]
        mov     bl, [edi + 1]
        add     eax, ecx
        mov     cl, [edi + edx + 1]
        add     eax, ebx
        shr     esi, 2
        add     eax, ecx
        and     esi, 00FF00FFh
        shl     eax, 6
        and     eax, 0000FF00h
        or      eax, esi
        stosd
        dec     tmp
        jnz     next
        mov     ebx, [edi]
        mov     esi, [edi - 4]
        and     ebx, 00FF00FFh
        and     esi, 00FF00FFh
        add     ebx, ebx
        mov     ecx, [edi + edx]
        add     esi, ebx
        and     ecx, 00FF00FFh
        add     esi, ecx
        mov     al, [edi - 3]
        mov     bl, [edi - 4]
        add     ebx, ebx
        mov     cl, [edi + edx + 1]
        add     eax, ebx
        xor     ebx, ebx
        shr     esi, 2
        add     eax, ecx
        and     esi, 00FF00FFh
        shl     eax, 6
        and     eax, 0000FF00h
        or      eax, esi
        stosd
        dec     height
        jnz     step
    }
}

void blendImage(GFX_IMAGE *dst, GFX_IMAGE *src1, GFX_IMAGE *src2, uint8_t cover)
{
    void *psrc1 = src1->mData;
    void *psrc2 = src2->mData;
    void *pdst  = dst->mData;
    uint32_t count = src1->mSize >> 2;

    if (bytesPerPixel != 4) fatalError("blendImage: only 32 bits supported.\n");

#ifdef _USE_MMX
    __asm {
        mov         ecx, count
        shr         ecx, 1
        jz          end
        mov         edi, pdst
        mov         edx, psrc1
        mov         esi, psrc2
        movzx       eax, cover
        xor         eax, 0FFh
        movd        mm3, eax
        punpcklwd   mm3, mm3
        pxor        mm2, mm2
        punpckldq   mm3, mm3
    again:
        movq        mm0, [edx]
        movq        mm1, [esi]
        movq        mm5, mm0
        punpcklbw   mm0, mm2
        movq        mm4, mm1
        punpcklbw   mm1, mm2
        psubw       mm0, mm1
        pmullw      mm0, mm3
        movq        mm6, mm4
        psrlw       mm0, 8
        psrlq       mm5, 32
        psrlq       mm4, 32
        punpcklbw   mm5, mm2
        punpcklbw   mm4, mm2
        psubw       mm5, mm4
        pmullw      mm5, mm3
        psrlw       mm5, 8
        packuswb    mm0, mm5
        paddb       mm0, mm6
        movq        [edi], mm0
        add         edx, 8
        add         edi, 8
        add         esi, 8
        dec         ecx
        jnz         again
        emms
    end:
    }
#else
    __asm {
        mov     edi, pdst
        mov     esi, psrc2
        mov     edx, psrc1
        movzx   ecx, cover
        neg     cl
    next:
        push    edx
        mov     ebx, [esi]
        mov     edx, [edx]
        mov     al, dh
        and     edx, 00FF00FFh
        mov     ah, bh
        and     ebx, 00FF00FFh
        sub     edx, ebx
        imul    edx, ecx
        shr     edx, 8
        add     ebx, edx
        xor     edx, edx
        mov     dl, ah
        xor     ah, ah
        mov     bh, dl
        sub     ax, dx
        mul     cx
        add     bh, ah
        pop     edx
        mov     [edi], ebx
        add     esi, 4
        add     edx, 4
        add     edi, 4
        dec     count
        jnz     next
    }
#endif
}

void rotateLine(uint32_t *dst, uint32_t *src, int32_t *tables, int32_t width, int32_t siny, int32_t cosy)
{
    int32_t pos = (width + 1) << 3;

    __asm {
        mov     ecx, width
        dec     ecx
        mov     esi, src
        mov     edi, dst
        mov     ebx, tables
    next:
        mov     eax, [ebx + ecx * 8 + 8]
        mov     edx, [ebx + ecx * 8 + 12]
        add     eax, cosy
        sub     edx, siny
        sar     eax, 1
        js      skip
        sar     edx, 1
        js      skip
        cmp     eax, [ebx + 4]
        jnl     skip
        cmp     edx, [ebx]
        jnl     skip
        shl     eax, 2
        add     eax, pos
        mov     eax, [ebx + eax]
        shl     edx, 2
        add     eax, edx
        mov     eax, [esi + eax]
        mov     [edi], eax
    skip:
        add     edi, 4
        dec     ecx
        jns     next
    }
}

void rotateImage(GFX_IMAGE *dst, GFX_IMAGE *src, int32_t *tables, int32_t axisx, int32_t axisy, double angle, double scale)
{
    double th, sint, cost;
    double sinx, cosx, siny, cosy;

    int32_t x, y, primex, primey, lineWidth = 0;
    uint32_t *psrc = (uint32_t*)src->mData;
    uint32_t *pdst = (uint32_t*)dst->mData;

    if (bytesPerPixel != 4) fatalError("rotateImage: only 32 bits supported.\n");

    // recalculate axisx, axisy
    axisx = dst->mWidth - axisx;
    axisy = dst->mHeight - axisy;

    // store source image width, height
    tables[0] = src->mWidth;
    tables[1] = src->mHeight;

    // calculate rotation data
    th   = (180 - angle) * M_PI / 180;
    sint = sin(th) / scale;
    cost = cos(th) / scale;

    primex = (-axisx << 1) + 1;
    sinx   = primex * sint - 1;
    cosx   = primex * cost - 1 + src->mWidth;
    sint   *= 2;
    cost   *= 2;

    // init lookup tables
    for (x = 0; x < dst->mWidth; x++)
    {
        tables[(x << 1) + 2] = sinx;
        tables[(x << 1) + 3] = cosx;
        sinx += sint;
        cosx += cost;
    }

    sint /= 2;
    cost /= 2;

    for (y = 0; y < src->mHeight; y++)
    {
        tables[y + ((src->mWidth + 1) << 1)] = lineWidth;
        lineWidth += src->mRowBytes;
    }

    primey = ((dst->mHeight - 1 - axisy) << 1) + 1;
    siny   = primey * sint;
    cosy   = primey * cost + src->mHeight;
    sint   *= 2;
    cost   *= 2;

    // process rotate line by line
    for (y = 0; y < dst->mHeight; y++)
    {
        rotateLine(pdst, psrc, tables, dst->mWidth, siny, cosy);
        pdst += dst->mWidth;
        siny -= sint;
        cosy -= cost;
    }
}

void bumpImage(GFX_IMAGE *dst, GFX_IMAGE *src1, GFX_IMAGE *src2, int32_t lx, int32_t ly)
{
    void *src1data = src1->mData;
    void *src2data = src2->mData;
    void *dstdata  = dst->mData;

    const int32_t bmax = 400;
    const int32_t xstart = 100;
    const int32_t ystart = 100;
    const int32_t endx = lfbWidth - xstart;
    const int32_t endy = lfbHeight - ystart;

    int32_t dstwidth  = dst->mWidth;
    int32_t src1width = src1->mWidth;
    int32_t src2width = src2->mWidth;
    int32_t src1len   = src1->mRowBytes - 1;
    
    int32_t nx = 0, ny = 0, vlx = 0, vly = 0;
    int32_t x = 0, y = 0, osrc2 = 0, osrc1 = 0, odst = 0;
    
    __asm {
        mov     eax, ystart
        mov     y, eax
    starty:
        mov     ebx, y
        mov     eax, src1width
        mul     ebx
        add     eax, 99
        shl     eax, 2
        mov     osrc1, eax
        mov     eax, src2width
        mul     ebx
        add     eax, 99
        shl     eax, 2
        mov     osrc2, eax
        mov     eax, dstwidth
        mul     ebx
        add     eax, 99
        shl     eax, 2
        mov     odst, eax
        mov     eax, xstart
        mov     x, eax
    startx:
        mov     eax, x
        sub     eax, lx
        mov     vlx, eax
        mov     eax, y
        sub     eax, ly
        mov     vly, eax
        mov     ecx, vlx
        mov     eax, vly
        mov     ebx, bmax
        neg     ebx
        cmp     ecx, bmax
        jnl     stop
        cmp     eax, bmax
        jnl     stop
        cmp     ecx, ebx
        jng     stop
        cmp     eax, ebx
        jng     stop
        xor     eax, eax
        xor     ebx, ebx
        mov     edi, src1data
        add     edi, osrc1
        mov     al, [edi + 1]
        or      al, al
        jz      stop
        mov     bl, [edi - 1]
        sub     eax, ebx
        mov     nx, eax
        mov     ecx, src1len
        mov     al, [edi + ecx]
        sub     edi, ecx
        mov     bl, [edi]
        sub     eax, ebx
        mov     ny, eax
        mov     eax, vlx
        sub     eax, nx
        jns     nsx
        neg     eax
    nsx:
        shr     eax, 1
        cmp     eax, 127
        jna     nax
        mov     eax, 127
    nax:
        mov     ebx, 127
        sub     ebx, eax
        jns     nsx2
        mov     ebx, 1
    nsx2:
        mov     eax, vly
        sub     eax, ny
        jns     nsy
        neg     eax
    nsy:
        shr     eax, 1
        cmp     eax, 127
        jna     nay
        mov     eax, 127
    nay:
        mov     ecx, 127
        sub     ecx, eax
        jns     nsy2
        mov     ecx, 1
    nsy2:
        add     ebx, ecx
        cmp     ebx, 128
        jna     stop
        sub     ebx, 128
        mov     edi, src2data
        add     edi, osrc2
        mov     ecx, [edi]
        mov     edi, dstdata
        add     edi, odst
        xor     eax, eax
        mov     al, cl
        mul     ebx
        shr     eax, 5
        cmp     eax, 255
        jna     nextb
        mov     eax, 255
    nextb:
        mov     [edi], al
        mov     al, ch
        mul     ebx
        shr     eax, 5
        cmp     eax, 255
        jna     nextg
        mov     eax, 255
    nextg:
        mov     [edi + 1], al
        shr     ecx, 16
        mov     al, cl
        mul     ebx
        shr     eax, 5
        cmp     eax, 255
        jna     nextr
        mov     eax, 255
    nextr:
        mov     [edi + 2], al
    stop:
        add     osrc2, 4
        add     osrc1, 4
        add     odst, 4
        inc     x
        mov     eax, endx
        cmp     x, eax
        jna     startx
        inc     y
        mov     eax, endy
        cmp     y, eax
        jna     starty
    }
}

// Init projection params
void initProjection()
{
    double th, ph;

    th = M_PI * theta / 180;
    ph = M_PI * phi / 180;

    aux1 = sin(th);
    aux2 = sin(ph);
    aux3 = cos(th);
    aux4 = cos(ph);

    aux5 = aux3 * aux2;
    aux6 = aux1 * aux2;
    aux7 = aux3 * aux4;
    aux8 = aux1 * aux4;
}

// projection points
void projette(double x, double y, double z)
{
    xobs = -x * aux1 + y * aux3;
    yobs = -x * aux5 - y * aux6 + z * aux4;

    if (projection == PERSPECTIVE)
    {
        zobs = -x * aux7 - y * aux8 - z * aux2 + rho;
        if (zobs == 0.0) zobs = DBL_MIN;
        xproj = de * xobs / zobs;
        yproj = de * yobs / zobs;
    }
    else if (projection == PARALLELE)
    {
        xproj = de * xobs;
        yproj = de * yobs;
    }
    else fatalError("projette: Unknown projection type [PERSPECTIVE, PARALLELE].\n");
}

// Move current cursor in 3D mode
void deplaceEn(double x, double y, double z)
{
    projette(x, y, z);
    xecran = centerX + xproj * ECHE;
    yecran = centerY - yproj;
    moveTo(xecran, yecran);
}

// Draw line from current cursor in 3D mode
void traceVers(double x, double y, double z, uint32_t col)
{
    projette(x, y, z);
    xecran = centerX + xproj * ECHE;
    yecran = centerY - yproj;
    lineTo(xecran, yecran, col);
}

void traceVersAdd(double x, double y, double z, uint32_t col)
{
    projette(x, y, z);
    xecran = centerX + xproj * ECHE;
    yecran = centerY - yproj;
    lineToAdd(xecran, yecran, col);
}

void traceVersSub(double x, double y, double z, uint32_t col)
{
    projette(x, y, z);
    xecran = centerX + xproj * ECHE;
    yecran = centerY - yproj;
    lineToSub(xecran, yecran, col);
}

int32_t strPos(char *str, const char *sub)
{
    char *ptr = strstr(str, sub);
    return ptr ? (ptr - str) : -1;
}

void insertChar(char *str, char chr, int32_t pos)
{
    if (pos < 0 || pos >= strlen(str)) return;
    str[pos] = chr;
}

void strDelete(char *str, int32_t i, int32_t numChar)
{
    if (i < 0 || i >= strlen(str)) return;
    memmove(&str[i + 1], &str[i + numChar], strlen(str) - i - 1);
}

void schRepl(char *str, const char *schr, char repl)
{
    int32_t pos = strPos(str, schr);
    while (pos >= 0)
    {
        strDelete(str, pos, strlen(schr));
        insertChar(str, repl, pos);
        pos = strPos(str, schr);
    }
}

void chr2Str(char chr, char num, char *str)
{
    str[0] = chr;
    str[1] = num;
    str[2] = 0;
}

// encode string to VNI string (format type VNI)
void makeFont(char *str)
{
    char buff[4] = {0};
    schRepl(str, "a8", 128);
    chr2Str(128, '1', buff);
    schRepl(str, buff, 129);
    chr2Str(128, '2', buff);
    schRepl(str, buff, 130);
    chr2Str(128, '3', buff);
    schRepl(str, buff, 131);
    chr2Str(128, '4', buff);
    schRepl(str, buff, 132);
    chr2Str(128, '5', buff);
    schRepl(str, buff, 133);
    schRepl(str, "a6", 134);
    chr2Str(134, '1', buff);
    schRepl(str, buff, 135);
    chr2Str(134, '2', buff);
    schRepl(str, buff, 136);
    chr2Str(134, '3', buff);
    schRepl(str, buff, 137);
    chr2Str(134, '4', buff);
    schRepl(str, buff, 138);
    chr2Str(134, '5', buff);
    schRepl(str, buff, 139);
    schRepl(str, "e6", 140);
    chr2Str(140, '1', buff);
    schRepl(str, buff, 141);
    chr2Str(140, '2', buff);
    schRepl(str, buff, 142);
    chr2Str(140, '3', buff);
    schRepl(str, buff, 143);
    chr2Str(140, '4', buff);
    schRepl(str, buff, 144);
    chr2Str(140, '5', buff);
    schRepl(str, buff, 145);
    schRepl(str, "o7", 146);
    chr2Str(146, '1', buff);
    schRepl(str, buff, 147);
    chr2Str(146, '2', buff);
    schRepl(str, buff, 148);
    chr2Str(146, '3', buff);
    schRepl(str, buff, 149);
    chr2Str(146, '4', buff);
    schRepl(str, buff, 150);
    chr2Str(146, '5', buff);
    schRepl(str, buff, 151);
    schRepl(str, "o6", 152);
    chr2Str(152, '1', buff);
    schRepl(str, buff, 153);
    chr2Str(152, '2', buff);
    schRepl(str, buff, 154);
    chr2Str(152, '3', buff);
    schRepl(str, buff, 155);
    chr2Str(152, '4', buff);
    schRepl(str, buff, 156);
    chr2Str(152, '5', buff);
    schRepl(str, buff, 157);
    schRepl(str, "u7", 158);
    chr2Str(158, '1', buff);
    schRepl(str, buff, 159);
    chr2Str(158, '2', buff);
    schRepl(str, buff, 160);
    chr2Str(158, '3', buff);
    schRepl(str, buff, 161);
    chr2Str(158, '4', buff);
    schRepl(str, buff, 162);
    chr2Str(158, '5', buff);
    schRepl(str, buff, 163);
    schRepl(str, "a1", 164);
    schRepl(str, "a2", 165);
    schRepl(str, "a3", 166);
    schRepl(str, "a4", 167);
    schRepl(str, "a5", 168);
    schRepl(str, "e1", 169);
    schRepl(str, "e2", 170);
    schRepl(str, "e3", 171);
    schRepl(str, "e4", 172);
    schRepl(str, "e5", 173);
    schRepl(str, "i1", 174);
    schRepl(str, "i2", 175);
    schRepl(str, "i3", 181);
    schRepl(str, "i4", 182);
    schRepl(str, "i5", 183);
    schRepl(str, "o1", 184);
    schRepl(str, "o2", 190);
    schRepl(str, "o3", 198);
    schRepl(str, "o4", 199);
    schRepl(str, "o5", 208);
    schRepl(str, "u1", 210);
    schRepl(str, "u2", 211);
    schRepl(str, "u3", 212);
    schRepl(str, "u4", 213);
    schRepl(str, "u5", 214);
    schRepl(str, "y1", 215);
    schRepl(str, "y2", 216);
    schRepl(str, "y3", 221);
    schRepl(str, "y4", 222);
    schRepl(str, "y5", 248);
    schRepl(str, "d9", 249);
    schRepl(str, "D9", 250);
}

// Calculate file size in bytes
uint32_t getFileSize(FILE *fpFile)
{
    uint32_t pos, len;
    pos = ftell(fpFile);
    fseek(fpFile, 0UL, SEEK_END);
    len = ftell(fpFile);
    fseek(fpFile, pos, SEEK_SET);
    return len;
}

// Generate random word array
void randomBuffer(void *buff, int32_t count, int32_t range)
{
    // Check range
    if (!count || !randSeed || !range) return;

    __asm {
        mov     edi, buff
        mov     ecx, count
        mov     ebx, randSeed
    next:
        mov     eax, ebx
        mul     factor
        inc     eax
        mov     ebx, eax
        shr     eax, 16
        mul     range
        shr     eax, 16
        stosw
        dec     ecx
        jnz     next
        mov     randSeed, ebx
    }
}

//get current loaded GFX font
GFX_FONT* getFont(int32_t type)
{
    if (type >= GFX_MAX_FONT) type = GFX_MAX_FONT - 1;
    if (type < 0) type = 0;
    return &gfxFonts[type ? type : fontType];
}

//get current loaded font type
uint32_t getFontType()
{
    return fontType;
}

// Set current selected font
void setFontType(uint32_t type)
{
    // Check current range
    if (type >= GFX_MAX_FONT) type = GFX_MAX_FONT - 1;
    fontType = type;
}

// Set current font size
void setFontSize(uint32_t size)
{
    GFX_FONT *font = getFont(fontType);

    // Have sub-fonts
    if (font->subFonts > 0)
    {
        // Correct sub-fonts number
        if (size > font->subFonts) size = font->subFonts;
        // Copy sub-fonts header
        memcpy(&font->info, &font->dataPtr[(font->info.endChar - font->info.startChar + 1) * 4 * (font->subFonts + 1) + size * sizeof(GFX_CHAR_INFO)], sizeof(GFX_CHAR_INFO));
    }
    subFont = size;
}

// Get height of string with current font in pixel
int32_t getFontHeight(const char *str)
{
    uint32_t i = 0;
    uint32_t height = 0;
    uint32_t mempos = 0;
    uint32_t size = 0;
    uint32_t len = strlen(str);
    GFX_FONT *font = getFont(fontType);

    // Check for font is loaded
    if (!font->dataPtr) return 0;
    if (!str || !len) return 0;

    // fixed font, all characters have a same height
    if (font->flags & GFX_FONT_FIXED) height = font->info.height;
    else
    {
        // vector font
        if (font->flags & GFX_FONT_VECTOR)
        {
            for (i = 0; i < len; i++)
            {
                // skip invalid character
                if (str[i] < font->info.startChar || str[i] > font->info.endChar) continue;

                // position of raw data of current character
                mempos = *(uint32_t*)&font->dataPtr[font->info.startOffset + ((str[i] - font->info.startChar) << 2)];
                size = font->dataPtr[mempos + 1];
                if (size > height) height = size;
            }
            height *= subFont;
        }
        else
        {
            // BMP1 font
            if (font->info.bitsPerPixel == 1)
            {
                for (i = 0; i < len; i++)
                {
                    // skip invalid character
                    if (str[i] < font->info.startChar || str[i] > font->info.endChar) continue;

                    // position of raw data of current character
                    mempos = *(uint32_t*)&font->dataPtr[font->info.startOffset + ((str[i] - font->info.startChar) << 2)];
                    size = font->dataPtr[mempos + 1];
                    if (size > height) height = size;
                }
            }
            else if (font->info.bitsPerPixel >= 2 && font->info.bitsPerPixel <= 32)
            {
                // BMP8 and RGB font
                for (i = 0; i < len; i++)
                {
                    // skip invalid character
                    if (str[i] < font->info.startChar || str[i] > font->info.endChar) continue;

                    // position of raw data of current character
                    mempos = *(uint32_t*)&font->dataPtr[font->info.startOffset + ((str[i] - font->info.startChar) << 2)];
                    size = *(uint32_t*)&font->dataPtr[mempos + 4] + *(uint32_t*)&font->dataPtr[mempos + 12];
                    if (size > height) height = size;
                }
            }
        }
    }

    // animation font
    if (font->flags & GFX_FONT_ANIPOS) height += font->info.randomY;
    return height;
}

// Get width of string with current font in pixel
int32_t getFontWidth(const char *str)
{
    GFX_STROKE_DATA *data;
    uint32_t i = 0;
    uint32_t mempos = 0;
    uint32_t width = 0;
    uint32_t size = 0;
    uint32_t len = strlen(str);
    GFX_FONT *font = getFont(fontType);

    // Check for font is loaded
    if (!font->dataPtr) return 0;
    if (!str || !len) return 0;

    // fixed font, all characters have a same width
    if (font->flags & GFX_FONT_FIXED) width = (font->info.width + font->info.distance) * len;
    else
    {
        // vector font
        if (font->flags & GFX_FONT_VECTOR)
        {
            for (i = 0; i < len; i++)
            {
                // skip invalid character
                if (str[i] < font->info.startChar || str[i] > font->info.endChar) size = font->info.spacer;
                else
                {
                    // position of raw data of current character
                    mempos = *(uint32_t*)&font->dataPtr[font->info.startOffset + ((str[i] - font->info.startChar) << 2)];
                    data = (GFX_STROKE_DATA*)&font->dataPtr[mempos];
                    size = data->width * subFont;
                }
                width += size + font->info.distance;
            }
        }
        else
        {
            // BMP1 font
            if (font->info.bitsPerPixel == 1)
            {
                for (i = 0; i < len; i++)
                {
                    // skip invalid character
                    if (str[i] < font->info.startChar || str[i] > font->info.endChar)
                    {
                        width += font->info.spacer + font->info.distance;
                        continue;
                    }

                    // position of raw data of current character
                    mempos = *(uint32_t*)&font->dataPtr[font->info.startOffset + ((str[i] - font->info.startChar) << 2)];
                    width += *(uint8_t*)&font->dataPtr[mempos] + font->info.distance;
                }
            }
            else if (font->info.bitsPerPixel >= 2 && font->info.bitsPerPixel <= 32)
            {
                // BMP8 and RGB font
                for (i = 0; i < len; i++)
                {
                    // skip invalid character
                    if (str[i] < font->info.startChar || str[i] > font->info.endChar)
                    {
                        width += font->info.spacer + font->info.distance;
                        continue;
                    }

                    // position of raw data of current character
                    mempos = *(uint32_t*)&font->dataPtr[font->info.startOffset + ((str[i] - font->info.startChar) << 2)];
                    width += *(uint32_t*)&font->dataPtr[mempos] + *(uint32_t*)&font->dataPtr[mempos + 8] + font->info.distance;
                }
            }
        }
    }

    // animation font
    if (font->flags & GFX_FONT_ANIPOS) width += font->info.randomX;
    return width - font->info.distance;
}

// Load font name to memory
int32_t loadFont(char *fname, uint32_t type)
{
    FILE *fp;

    // Check for type range
    if (type >= GFX_MAX_FONT) return 0;

    // Read font header
    fp = fopen(fname, "rb");
    if (!fp) return 0;
    fread(&gfxFonts[type], 1, sizeof(GFX_FONT), fp);

    // Check font signature, version number and memory size
    if (memcmp(gfxFonts[type].sign, "Fnt2", 4) || gfxFonts[type].version != 0x0101 || !gfxFonts[type].memSize)
    {
        fclose(fp);
        return 0;
    }

    // Allocate raw data buffer
    gfxFonts[type].dataPtr = (uint8_t*)calloc(gfxFonts[type].memSize, 1);
    if (!gfxFonts[type].dataPtr)
    {
        fclose(fp);
        return 0;
    }

    // Read raw font data
    fread(gfxFonts[type].dataPtr, 1, gfxFonts[type].memSize, fp);
    fclose(fp);

    // Reset font header for old font
    if (gfxFonts[type].flags & GFX_FONT_MULTI) setFontSize(0);

    // Default sub-fonts
    if (gfxFonts[type].flags & GFX_FONT_VECTOR) subFont = 1;
    else subFont = 0;

    // BMP8 font palette
    if (gfxFonts[type].info.usedColors > 1)
    {
        // BMP8 use up to 128 colors (128 * 4)
        fontPalette[type] = (uint8_t*)calloc(512, 1);
        if (!fontPalette[type])
        {
            free(gfxFonts[type].dataPtr);
            return 0;
        }
        memset(fontPalette[type], 0, 512);
    }
    return 1;
}

// Release current loaded font
void closeFont(uint32_t type)
{
    // Check for type range
    if (type >= GFX_MAX_FONT) return;

    // Free font raw data buffer
    if (gfxFonts[type].dataPtr)
    {
        free(gfxFonts[type].dataPtr);
        gfxFonts[type].dataPtr = NULL;
    }

    // Free font palette
    if (fontPalette[type])
    {
        free(fontPalette[type]);
        fontPalette[type] = NULL;
    }

    // Reset header
    memset(&gfxFonts[type], 0, sizeof(GFX_FONT));
}

// Draw a stroke of BGI font (YES we also support BGI font)
int32_t outStroke(int32_t x, int32_t y, char chr, uint32_t col, uint32_t mode)
{
    GFX_STROKE_DATA *data;
    GFX_STROKE_INFO *stroke;

    uint32_t mx, my;
    uint32_t i, mempos;
    GFX_FONT *font = getFont(fontType);

    // check for font is loaded
    if (!font->dataPtr) return 0;

    // check for non-drawable character
    if (font->info.startChar > chr || font->info.endChar < chr) return font->info.spacer;

    // memory position of character
    mempos = *(uint32_t*)&font->dataPtr[font->info.startOffset + ((chr - font->info.startChar) << 2)];
    data = (GFX_STROKE_DATA*)&font->dataPtr[mempos];
    stroke = (GFX_STROKE_INFO*)&font->dataPtr[mempos + sizeof(GFX_STROKE_DATA)];
    
    // scan for all lines
    for (i = 0; i < data->numOfLines; i++)
    {
        mx = x + stroke->x * subFont;
        my = y + stroke->y * subFont;
        
        // check for drawable
        if (stroke->code == 1) moveTo(mx, my);
        else
        {
            // automatic antialias when in 32-bits mode
            if (bytesPerPixel == 4) lineToAlpha(mx, my, col);
            else if (mode == 2) lineToAdd(mx, my, col);
            else if (mode == 3) lineToSub(mx, my, col);
            else lineTo(mx, my, col);
        }
        stroke++;
    }

    return data->width * subFont;
}

// Draw string with current loaded font
void writeString(int32_t x, int32_t y, const char *str, uint32_t col, uint32_t mode)
{
    uint32_t cx, cy;
    uint32_t i, len;
    uint32_t width, height, addx, addy;
    uint32_t data, dataPos, mempos;
    GFX_FONT *font = getFont(fontType);

    // Check for font is loaded
    if (!font->dataPtr) return;

    len = strlen(str);

    // Check for vector font
    if (font->flags & GFX_FONT_VECTOR)
    {
        for (i = 0; i < len; i++)
        {
            x += outStroke(x, y, str[i], col, mode) + font->info.distance;
            if (mode == 1) col++;
        }
        return;
    }

    // BMP1 font format
    if (font->info.bitsPerPixel == 1)
    {
        for (i = 0; i < len; i++)
        {
            // Invalid character, update position
            if (str[i] < font->info.startChar || str[i] > font->info.endChar)
            {
                if (!(font->flags & GFX_FONT_FIXED)) x += font->info.spacer + font->info.distance;
                else x += font->info.width + font->info.distance;
                continue;
            }

            // Memory position for each character
            mempos = *(uint32_t*)&font->dataPtr[font->info.startOffset + ((str[i] - font->info.startChar) << 2)];
            width = font->dataPtr[mempos];
            height = font->dataPtr[mempos + 1];
            mempos += 2;

            // Scans for font width and height
            for (cy = 0; cy < height; cy++)
            {
                dataPos = 0;
                data = *(uint32_t*)&font->dataPtr[mempos];
                for (cx = 0; cx < width; cx++)
                {
                    if ((cx > 31) && !(cx & 31))
                    {
                        dataPos += 4;
                        data = *(uint32_t*)&font->dataPtr[mempos + dataPos];
                    }
                    if (data & (1 << (cx & 31)))
                    {
                        if (mode == 2) putPixelAdd(x + cx, y + cy, col);
                        else if (mode == 3) putPixelSub(x + cx, y + cy, col);
                        else putPixel(x + cx, y + cy, col);
                    }
                }
                mempos += font->info.bytesPerLine;
            }
            x += width + font->info.distance;
            if (mode == 1) col++;
        }
    }
    // BMP8 font format
    else if (font->info.bitsPerPixel >= 2 && font->info.bitsPerPixel <= 7)
    {
        // Calculate font palette, use for hi-color and true-color
        if (bitsPerPixel != 8)
        {
            ARGB *pcol = (ARGB*)&col;
            for (i = 1; i <= font->info.usedColors; i++)
            {
                ARGB* pixels = (ARGB*)&fontPalette[fontType][i << 2];
                pixels->r = pcol->r * (i + 1) / font->info.usedColors;
                pixels->g = pcol->g * (i + 1) / font->info.usedColors;
                pixels->b = pcol->b * (i + 1) / font->info.usedColors;
            }
        }

        // Genertate random position for animation font
        if (font->flags & GFX_FONT_ANIPOS)
        {
            randomBuffer(gfxBuff, len + 1, font->info.randomX);
            randomBuffer(&gfxBuff[512], len + 1, font->info.randomY);
        }

        for (i = 0; i < len; i++)
        {
            // Invalid character, update character position
            if (str[i] < font->info.startChar || str[i] > font->info.endChar)
            {
                if (!(font->flags & GFX_FONT_FIXED)) x += font->info.spacer + font->info.distance;
                else x += font->info.width + font->info.distance;
                continue;
            }

            // Lookup character position
            mempos = *(uint32_t*)&font->dataPtr[font->info.startOffset + ((str[i] - font->info.startChar) << 2)];
            addx = *(uint32_t*)&font->dataPtr[mempos];
            addy = *(uint32_t*)&font->dataPtr[mempos + 4];

            // Update position for animation font
            if (font->flags & GFX_FONT_ANIPOS)
            {
                addx += gfxBuff[i << 1];
                addy += gfxBuff[512 + (i << 1)];
            }

            // Get font width and height
            width = *(uint32_t*)&font->dataPtr[mempos + 8];
            height = *(uint32_t*)&font->dataPtr[mempos + 12];
            mempos += 16;
            x += addx;

            // Scans font raw data
            for (cy = 0; cy < height; cy++)
            {
                for (cx = 0; cx < width; cx++)
                {
                    data = font->dataPtr[mempos++];
                    if (!(data & 0x80))
                    {
                        if (bitsPerPixel == 8) putPixel(x + cx, y + cy + addy, data);
                        else if (mode == 2) putPixelAdd(x + cx, y + cy + addy, *(uint32_t*)&fontPalette[fontType][data << 2]);
                        else if (mode == 3) putPixelSub(x + cx, y + cy + addy, *(uint32_t*)&fontPalette[fontType][data << 2]);
                        else putPixel(x + cx, y + cy + addy, *(uint32_t*)&fontPalette[fontType][data << 2]);
                    }
                }
            }

            // Update next position
            if (font->flags & GFX_FONT_ANIPOS) x -= gfxBuff[i << 1];
            if (font->flags & GFX_FONT_FIXED) x += font->info.width + font->info.distance;
            else x += width + font->info.distance;
        }
    }
    // Alpha channel font
    else if (font->info.bitsPerPixel == 32)
    {
        // Genertate random position for animation font
        if (font->flags & GFX_FONT_ANIPOS)
        {
            randomBuffer(gfxBuff, len + 1, font->info.randomX);
            randomBuffer(&gfxBuff[512], len + 1, font->info.randomY);
        }

        for (i = 0; i < len; i++)
        {
            // Invalid character, update character position
            if (str[i] < font->info.startChar || str[i] > font->info.endChar)
            {
                if (!(font->flags & GFX_FONT_FIXED)) x += font->info.spacer + font->info.distance;
                else x += font->info.width + font->info.distance;
                continue;
            }

            // Lookup character position
            mempos = *(uint32_t*)&font->dataPtr[font->info.startOffset + ((str[i] - font->info.startChar) << 2)];
            addx = *(uint32_t*)&font->dataPtr[mempos];
            addy = *(uint32_t*)&font->dataPtr[mempos + 4];

            // Update position for animation font
            if (font->flags & GFX_FONT_ANIPOS)
            {
                addx += gfxBuff[i << 1];
                addy += gfxBuff[(i << 1) + 512];
            }

            // Get font width and height
            width = *(uint32_t*)&font->dataPtr[mempos + 8];
            height = *(uint32_t*)&font->dataPtr[mempos + 12];
            mempos += 16;
            x += addx;

            // Scans raw font data
            for (cy = 0; cy < height; cy++)
            {
                for (cx = 0; cx < width; cx++)
                {
                    data = *(uint32_t*)&font->dataPtr[mempos];
                    putPixelAlpha(x + cx, y + addy + cy, data, 255);
                    mempos += 4;
                }
            }

            // Update next position
            if (font->flags & GFX_FONT_ANIPOS) x -= gfxBuff[i << 1];
            if (font->flags & GFX_FONT_FIXED) x += font->info.width + font->info.distance;
            else x += width + font->info.distance;
        }
    }
}

// draw muti-line string font
int32_t drawText(int32_t ypos, const char **str, int32_t size)
{
    int32_t i;

    // Check for font loaded
    if (!gfxFonts[fontType].dataPtr) return 0;

    for (i = 0; i < size; i++)
    {
        if (ypos > -30) writeString(centerX - (getFontWidth(str[i]) >> 1), ypos, str[i], 62, 0);
        ypos += getFontHeight(str[i]);
        if (ypos > cmaxY) break;
    }

    return ypos;
}

// draw text into image buffer
void drawTextImage(int32_t x, int32_t y, uint32_t col, GFX_IMAGE *img, const char *format, ...)
{
    va_list args;
    char buffer[1024] = { 0 };

    // save current screen address
    uint8_t *oldPtr = lfbPtr;

    // check for font loaded
    if (!gfxFonts[fontType].dataPtr) return;

    // parse arguments
    va_start(args, format);
    vsprintf(buffer, format, args);
    va_end(args);

    // change screen address to image buffer
    lfbPtr = img->mData;
    writeString(x, y, buffer, col, 2);

    // must be restored after draw
    lfbPtr = oldPtr;
}

////////////////////////////////
//      BEGIN BMP SECTION     //
////////////////////////////////
int32_t loadBitmap(const char *fname, GFX_BITMAP *bmp)
{
    FILE *fp;
    BMP_HEADER bf;
    BMP_INFO bi;
    ARGB bmPal[256] = {0};

    uint32_t i, bmOfs, bmScanlineBytes;
    uint32_t bmSize, bmRowBytes, bmPaddingBytes;

    // Open file and check
    fp = fopen(fname, "rb");
    if (!fp) return 0;

    // Read header and check magic 'BM' signature
    fread(&bf, sizeof(BMP_HEADER), 1, fp);
    if (bf.bfType != 0x4d42)
    {
        fclose(fp);
        return 0;
    }

    // Read BMP info and check correct bits
    fread(&bi, sizeof(BMP_INFO), 1, fp);
    if (bi.biBitCount != 8 && bi.biBitCount != 16 && bi.biBitCount != 24 && bi.biBitCount != 32)
    {
        fclose(fp);
        return 0;
    }

    // Save bitmap width, height, bytes per pixels
    bmp->bmWidth = bi.biWidth;
    bmp->bmHeight = bi.biHeight;
    bmp->bmPixels = (bi.biBitCount + 7) >> 3;

    // Calculate bitmap data size
    bmSize = bf.bfSize - bf.bfOffBits;
    bmp->bmData = (uint8_t*)calloc(bmSize, 1);
    if (!bmp->bmData)
    {
        fclose(fp);
        return 0;
    }

    // For 8 bits, need palette table
    if (bi.biBitCount == 8)
    {
        // Read and convert palette
        fread(bmPal, sizeof(bmPal), 1, fp);
        for (i = 0; i < 256; i++)
        {
            bmp->bmExtra[3 * i]     = bmPal[i].r;
            bmp->bmExtra[3 * i + 1] = bmPal[i].g;
            bmp->bmExtra[3 * i + 2] = bmPal[i].b;
        }
    }

    // For 16 bits, need RGB mask table
    else if (bi.biBitCount == 16) fread(bmp->bmExtra, 3 * sizeof(int), 1, fp);

    // Calculate bytes per scaline
    bmRowBytes = bmp->bmWidth * bmp->bmPixels;

    // BMP requires each row to have a multiple of 4 bytes
    // so sometimes padding bytes are added between rows
    bmPaddingBytes = bmRowBytes % 4;
    bmScanlineBytes = bmPaddingBytes ? bmRowBytes + (4 - bmPaddingBytes) : bmRowBytes;

    // save bytes per scan line (include padding bytes)
    bmp->bmRowBytes = bmScanlineBytes;

    // Seek to data offset, read and revert data
    fseek(fp, bf.bfOffBits, SEEK_SET);
    bmOfs = (bmp->bmHeight - 1) * bmScanlineBytes;
    for (i = 0; i < bmp->bmHeight; i++)
    {
        fread(&bmp->bmData[bmOfs], bmScanlineBytes, 1, fp);
        bmOfs -= bmScanlineBytes;
    }

    fclose(fp);
    return 1;
}

// Save screen buffer to BMP file
int32_t saveBitmap(const char *fname, GFX_BITMAP *bmp)
{
    BMP_HEADER  bf;             // BMP file header
    BMP_INFO    bi;             // BMP file info
    RGB         aux[256] = {0}; // Current Palette entries
    ARGB        pal[256] = {0}; // BMP Palette entries

    // Open new file
    FILE  *fp = fopen(fname, "wb");

    // Calculate actual data size
    uint32_t dataSize = bmp->bmWidth * bmp->bmHeight * bmp->bmPixels;

    // Calculate line size (in bytes)
    uint32_t lineWidth = bmp->bmWidth * bmp->bmPixels;

    // Calculate padding size (round up to dword per line)
    uint32_t addPadding = (4 - lineWidth % 4) % 4;

    // addPadding data and offsets
    uint32_t hdrPadding = 0, i = 0;

    // Data offset
    uint8_t *ptrData = NULL;

    if (!fp) return 0;

    memset(&bf, 0, sizeof(bf));
    memset(&bi, 0, sizeof(bi));

    // Check for 8 bits color
    if (bmp->bmPixels == 1)
    {
        bi.biClrUsed = 256;         // Turn on color mode used
        hdrPadding = sizeof(pal);   // Calculate padding header size
        getPalette(aux);            // Add palette entries

        // Calculate bitmap palette
        for (i = 0; i < 256; i++)
        {
            pal[i].r = aux[i].r;
            pal[i].g = aux[i].g;
            pal[i].b = aux[i].b;
        }
    }
    // for 15/16 bit colors add RGB mask entries (12 bytes)
    else if (bmp->bmPixels == 2)
    {
        bi.biCompression = 3;               // Turn on bit field compression for 16 bits BI_FIELDS (mode 565), for 555 use BI_RGB, bi.biCompression = 0
        hdrPadding = 3 * sizeof(int32_t);   // Calculate padding header r,g,b mask size (12 bytes)
        *((uint32_t*)pal    ) = rmask;      // Red masked value
        *((uint32_t*)pal + 1) = gmask;      // Green masked value
        *((uint32_t*)pal + 2) = bmask;      // Blue masked value
    }

    // Fill BMP info
    bf.bfType       = 0x4d42; // BM signature
    bf.bfSize       = sizeof(bf) + sizeof(bi) + dataSize + hdrPadding + bmp->bmHeight * addPadding; // Actual data size
    bf.bfOffBits    = sizeof(bf) + sizeof(bi) + hdrPadding; // offset of data buffer
    bi.biSize       = sizeof(bi);
    bi.biWidth      = bmp->bmWidth;
    bi.biHeight     = bmp->bmHeight;
    bi.biPlanes     = 1;
    bi.biBitCount   = bitsPerPixel;
    bi.biSizeImage  = bf.bfSize - bf.bfOffBits;

    // Write header, and pading data (palette or RGB mask)
    fwrite(&bf, 1, sizeof(bf), fp);
    fwrite(&bi, 1, sizeof(bi), fp);
    fwrite(pal, 1, hdrPadding, fp);

    // Write inverted data (BMP file is stored in inverted)
    ptrData = &bmp->bmData[dataSize - lineWidth];
    for (i = 0; i < bmp->bmHeight; i++)
    {
        fwrite(ptrData, 1, lineWidth, fp); // Write raw data
        fwrite(aux, 1, addPadding, fp); // BMP file need padding to dword for each line
        ptrData -= lineWidth; // Skip to previous line
    }

    fclose(fp);
    return 1;
}

// close bitmap
void closeBitmap(GFX_BITMAP *bmp)
{
    if (bmp && bmp->bmData)
    {
        free(bmp->bmData);
        bmp->bmData = NULL;
        bmp->bmWidth = 0;
        bmp->bmHeight = 0;
        bmp->bmPixels = 0;
        bmp->bmRowBytes = 0;
        memset(bmp->bmExtra, 0, 768);
    }
}

// Load and display BMP file
// (support 8/15/16/24/32 bits color)
void showBitmap(const char *fname)
{
    GFX_BITMAP bmp;

    int32_t rm, gm, bm;
    int32_t bmWidth, bmHeight, bmPadding;
    int32_t bmScanlineBytes, bmRowBytes;
    uint8_t *bmData, *bmPal;

    // Load bitmap to buffer
    if (!loadBitmap(fname, &bmp)) fatalError("showBitmap: cannot load bitmap image: %s\n", fname);

    // save local values
    bmWidth         = bmp.bmWidth;
    bmHeight        = bmp.bmHeight;
    bmData          = bmp.bmData;
    bmScanlineBytes = bmp.bmRowBytes;
    bmRowBytes      = bmWidth * bmp.bmPixels;
    bmPadding       = (4 - bmRowBytes % 4) % 4;

    // load palette
    if (bmp.bmPixels == 1) bmPal = bmp.bmExtra;

    // load grb mask color
    else if (bmp.bmPixels == 2)
    {
        rm = *((uint32_t*)bmp.bmExtra    );
        gm = *((uint32_t*)bmp.bmExtra + 1);
        bm = *((uint32_t*)bmp.bmExtra + 2);
    }

    // build-in support multi-color
    switch (bitsPerPixel)
    {
    case 32:
        switch (bmp.bmPixels)
        {
        case 1:
            __asm {
                mov     eax, pageOffset
                mul     bytesPerScanline
                mov     edi, lfbPtr
                add     edi, eax
                mov     esi, bmData
                mov     ebx, bmPal
                mov     ecx, bmWidth
                mov     eax, lfbWidth
                sub     eax, bmWidth
                shl     eax, 2
                push    eax
            next:
                xor     edx, edx
                mov     dl, [esi]
                lea     edx, [edx + 2 * edx]
                mov     al, [ebx + edx + 2]
                stosb
                mov     al, [ebx + edx + 1]
                stosb
                mov     al, [ebx + edx]
                stosb
                inc     edi
                inc     esi
                dec     ecx
                jnz     next
                mov     ecx, bmWidth
                add     edi, [esp]
                add     esi, bmPadding
                dec     bmHeight
                jnz     next
                pop     eax
            }
            break;

        case 2:
            if (gm == 0x03E0) // rgb555 format
            {
                __asm {
                    mov     eax, pageOffset
                    mul     bytesPerScanline
                    mov     edi, lfbPtr
                    add     edi, eax
                    mov     esi, bmData
                    mov     ecx, bmWidth
                    mov     ebx, lfbWidth
                    sub     ebx, bmWidth
                    shl     ebx, 2
                next:
                    mov     eax, [esi]
                    and     eax, bm
                    shl     eax, 3
                    stosb
                    mov     eax, [esi]
                    and     eax, gm
                    shr     eax, 5
                    shl     eax, 3
                    stosb
                    mov     eax, [esi]
                    and     eax, rm
                    shr     eax, 10
                    shl     eax, 3
                    stosb
                    inc     edi
                    add     esi, 2
                    dec     ecx
                    jnz     next
                    mov     ecx, bmWidth
                    add     edi, ebx
                    add     esi, bmPadding
                    dec     bmHeight
                    jnz     next
                }
            }
            else if (gm == 0x07E0) // rgb565 format
            {
                __asm {
                    mov     eax, pageOffset
                    mul     bytesPerScanline
                    mov     edi, lfbPtr
                    add     edi, eax
                    mov     esi, bmData
                    mov     ecx, bmWidth
                    mov     ebx, lfbWidth
                    sub     ebx, bmWidth
                    shl     ebx, 2
                next:
                    mov     eax, [esi]
                    and     eax, bm
                    shl     eax, 3
                    stosb
                    mov     eax, [esi]
                    and     eax, gm
                    shr     eax, 5
                    shl     eax, 2
                    stosb
                    mov     eax, [esi]
                    and     eax, rm
                    shr     eax, 11
                    shl     eax, 3
                    stosb
                    inc     edi
                    add     esi, 2
                    dec     ecx
                    jnz     next
                    mov     ecx, bmWidth
                    add     edi, ebx
                    add     esi, bmPadding
                    dec     bmHeight
                    jnz     next
                }
            }
            break;

        case 3:
            __asm {
                mov     eax, pageOffset
                mov     ebx, bytesPerScanline
                mul     ebx
                mov     edi, lfbPtr
                add     edi, eax
                mov     esi, bmData
                mov     ecx, bmWidth    
                mov     ebx, lfbWidth
                sub     ebx, bmWidth
                shl     ebx, 2
            next:
                movsw
                movsb
                inc     edi
                dec     ecx
                jnz     next
                mov     ecx, bmWidth
                add     edi, ebx
                add     esi, bmPadding
                dec     bmHeight
                jnz     next
            }
            break;

        case 4:
            __asm {
                mov     eax, pageOffset
                mul     bytesPerScanline
                mov     edi, lfbPtr
                add     edi, eax
                mov     esi, bmData
                mov     edx, bmScanlineBytes
                sub     edx, bmRowBytes
                mov     eax, bytesPerScanline
                sub     eax, bmRowBytes
            next:
                mov     ecx, bmWidth
                rep     movsd
                add     edi, eax
                add     esi, edx
                dec     bmHeight
                jnz     next
            }
            break;
        }
        break;

    case 24:
        switch (bmp.bmPixels)
        {
        case 1:
            __asm {
                mov     eax, pageOffset
                mul     bytesPerScanline
                mov     edi, lfbPtr
                add     edi, eax
                mov     esi, bmData
                mov     ebx, bmPal
                mov     ecx, bmWidth
                mov     eax, lfbWidth
                sub     eax, bmWidth
                mov     ebx, eax
                shl     eax, 1
                add     eax, ebx
                push    eax
            next:
                xor     edx, edx
                mov     dl, [esi]
                lea     edx, [edx + 2 * edx]
                mov     al, [ebx + edx + 2]
                stosb
                mov     al, [ebx + edx + 1]
                stosb
                mov     al, [ebx + edx]
                stosb
                inc     esi
                dec     ecx
                jnz     next
                mov     ecx, bmWidth
                add     edi, [esp]
                add     esi, bmPadding
                dec     bmHeight
                jnz     next
                pop     eax
            }
            break;

        case 2:
            if (gm == 0x03E0) // rgb555 format
            {
                __asm {
                    mov     eax, pageOffset
                    mul     bytesPerScanline
                    mov     edi, lfbPtr
                    add     edi, eax
                    mov     esi, bmData
                    mov     ecx, bmWidth
                    mov     ebx, lfbWidth
                    sub     ebx, bmWidth
                    mov     eax, ebx
                    shl     ebx, 1
                    add     ebx, eax
                next:
                    mov     eax, [esi]
                    and     eax, bm
                    shl     eax, 3
                    stosb
                    mov     eax, [esi]
                    and     eax, gm
                    shr     eax, 5
                    shl     eax, 3
                    stosb
                    mov     eax, [esi]
                    and     eax, rm
                    shr     eax, 10
                    shl     eax, 3
                    stosb
                    add     esi, 2
                    dec     ecx
                    jnz     next
                    mov     ecx, bmWidth
                    add     edi, ebx
                    add     esi, bmPadding
                    dec     bmHeight
                    jnz     next
                }
            }
            else if (gm == 0x07E0) // rgb565 format
            {
                __asm {
                    mov     eax, pageOffset
                    mul     bytesPerScanline
                    mov     edi, lfbPtr
                    add     edi, eax
                    mov     esi, bmData
                    mov     ecx, bmWidth
                    mov     ebx, lfbWidth
                    sub     ebx, bmWidth
                    mov     eax, ebx
                    shl     ebx, 1
                    add     ebx, eax
                next:
                    mov     eax, [esi]
                    and     eax, bm
                    shl     eax, 3
                    stosb
                    mov     eax, [esi]
                    and     eax, gm
                    shr     eax, 5
                    shl     eax, 2
                    stosb
                    mov     eax, [esi]
                    and     eax, rm
                    shr     eax, 11
                    shl     eax, 3
                    stosb
                    add     esi, 2
                    dec     ecx
                    jnz     next
                    mov     ecx, bmWidth
                    add     edi, ebx
                    add     esi, bmPadding
                    dec     bmHeight
                    jnz     next
                }
            }
            break;

        case 3:
            __asm {
                mov     eax, pageOffset
                mul     bytesPerScanline
                mov     edi, lfbPtr
                add     edi, eax
                mov     esi, bmData
                mov     edx, bmScanlineBytes
                sub     edx, bmRowBytes
                mov     eax, bytesPerScanline
                sub     eax, bmRowBytes
                mov     ecx, bmWidth
            next:
                movsw
                movsb
                dec     ecx
                jnz     next
                add     edi, eax
                add     esi, edx
                mov     ecx, bmWidth
                dec     bmHeight
                jnz     next
            }
            break;
        }
        break;

    case 16:
        switch (bmp.bmPixels)
        {
        case 1:
            __asm {
                mov     eax, pageOffset
                mul     bytesPerScanline
                mov     edi, lfbPtr
                add     edi, eax
                mov     esi, bmData
                mov     ebx, bmPal
                mov     ecx, bmWidth    
                mov     eax, lfbWidth
                sub     eax, bmWidth
                shl     eax, 1
                push    eax
            next:
                xor     eax, eax
                xor     edx, edx
                mov     dl, [esi]
                lea     edx, [edx + 2 * edx]
                mov     al, [ebx + edx]
                push    eax
                mov     al, [ebx + edx + 1]
                push    eax
                mov     al, [ebx + edx + 2]
                push    eax
                pop     eax
                shr     eax, 3
                and     eax, bmask
                pop     edx
                shr     edx, 2
                shl     edx, 5
                and     edx, gmask
                or      eax, edx
                pop     edx
                shr     edx, 3
                shl     edx, 11
                and     edx, rmask
                or      eax, edx
                stosw
                inc     esi
                dec     ecx
                jnz     next
                mov     ecx, bmWidth
                add     edi, [esp]
                add     esi, bmPadding
                dec     bmHeight
                jnz     next
                pop     eax
            }
            break;

        case 2:
            if (gm == 0x03E0) // rgb555 format
            {
                __asm {
                    mov     eax, pageOffset
                    mul     bytesPerScanline
                    mov     edi, lfbPtr
                    add     edi, eax
                    mov     esi, bmData
                    mov     ecx, bmWidth
                    mov     eax, lfbWidth
                    sub     eax, bmWidth
                    shl     eax, 1
                    push    eax
                next:
                    xor     edx, edx
                    xor     eax, eax
                    mov     eax, [esi]
                    and     eax, rm
                    shr     eax, 10
                    shl     eax, 3
                    shr     eax, 3
                    shl     eax, 11
                    and     eax, rmask
                    mov     ebx, [esi]
                    and     ebx, gm
                    shr     ebx, 5
                    shl     ebx, 3
                    shr     ebx, 2
                    shl     ebx, 5
                    and     ebx, gmask
                    mov     edx, [esi]
                    and     edx, bm
                    shl     edx, 3
                    shr     edx, 3
                    and     edx, bmask
                    or      eax, ebx
                    or      eax, edx
                    stosw
                    add     esi, 2
                    dec     ecx
                    jnz     next
                    mov     ecx, bmWidth
                    add     edi, [esp]
                    add     esi, bmPadding
                    dec     bmHeight
                    jnz     next
                    pop     eax
                }
            }
            else if (gm == 0x07E0) // rgb565 format
            {
                __asm {
                    mov     eax, pageOffset
                    mul     bytesPerScanline
                    mov     edi, lfbPtr
                    add     edi, eax
                    mov     esi, bmData
                    mov     edx, bmScanlineBytes
                    sub     edx, bmRowBytes
                    mov     eax, bytesPerScanline
                    sub     eax, bmRowBytes
                next:
                    mov     ecx, bmWidth
                    rep     movsw
                    add     edi, eax
                    add     esi, edx
                    dec     bmHeight
                    jnz     next
                }
            }
            break;
        }
        break;

    case 15:
        switch (bmp.bmPixels)
        {
        case 1:
            __asm {
                mov     eax, pageOffset
                mul     bytesPerScanline
                mov     edi, lfbPtr
                add     edi, eax
                mov     esi, bmData
                mov     ebx, bmPal
                mov     ecx, bmWidth
                mov     eax, lfbWidth
                sub     eax, bmWidth
                shl     eax, 1
                push    eax
            next:
                xor     edx, edx
                xor     eax, eax
                mov     dl, [esi]
                lea     edx, [edx + 2 * edx]
                mov     al, [ebx + edx]
                push    eax
                mov     al, [ebx + edx + 1]
                push    eax
                mov     al, [ebx + edx + 2]
                push    eax
                pop     eax
                shr     eax, 3
                and     eax, bmask
                pop     edx
                shr     edx, 3
                shl     edx, 5
                and     edx, gmask
                or      eax, edx
                pop     edx
                shr     edx, 3
                shl     edx, 10
                and     edx, rmask
                or      eax, edx
                stosw
                inc     esi
                dec     ecx
                jnz     next
                mov     ecx, bmWidth
                add     edi, [esp]
                add     esi, bmPadding
                dec     bmHeight
                jnz     next
                pop     eax
            }
            break;

        case 2:
            if (gm == 0x03E0) // rgb555 format
            {
                __asm {
                    mov     eax, pageOffset
                    mul     bytesPerScanline
                    mov     edi, lfbPtr
                    add     edi, eax
                    mov     esi, bmData
                    mov     edx, bmScanlineBytes
                    sub     edx, bmRowBytes
                    mov     eax, bytesPerScanline
                    sub     eax, bmRowBytes
                next:
                    mov     ecx, bmWidth
                    rep     movsw
                    add     edi, eax
                    add     esi, edx
                    dec     bmHeight
                    jnz     next
                }
            }
            else if (gm == 0x07E0) // rgb565 format
            {
                __asm {
                    mov     eax, pageOffset
                    mov     ebx, bytesPerScanline
                    mul     ebx
                    mov     edi, lfbPtr
                    add     edi, eax
                    mov     esi, bmData
                    mov     ecx, bmWidth    
                    mov     eax, lfbWidth
                    sub     eax, bmWidth
                    shl     eax, 1
                    push    eax
                next:
                    mov     eax, [esi]
                    and     eax, rm
                    shr     eax, 11
                    shl     eax, 3
                    shr     eax, 3
                    shl     eax, 10
                    and     eax, rmask
                    mov     ebx, [esi]
                    and     ebx, gm
                    shr     ebx, 5
                    shl     ebx, 2
                    shr     ebx, 3
                    shl     ebx, 5
                    and     ebx, gmask
                    mov     edx, [esi]
                    and     edx, bm
                    shl     edx, 3
                    shr     edx, 3
                    and     edx, bmask
                    or      eax, ebx
                    or      eax, edx
                    stosw
                    add     esi, 2
                    dec     ecx
                    jnz     next
                    mov     ecx, bmWidth
                    add     edi, [esp]
                    add     esi, bmPadding
                    dec     bmHeight
                    jnz     next
                    pop     eax
                }
            }
            break;
        }
        break;

    case 8:
        setPalette(bmPal);
        __asm {
            mov     eax, pageOffset
            mul     bytesPerScanline
            mov     edi, lfbPtr
            add     edi, eax
            mov     esi, bmData
            mov     eax, bytesPerScanline
            sub     eax, bmRowBytes
            mov     edx, bmScanlineBytes
            sub     edx, bmRowBytes
        next:
            mov     ecx, bmWidth
            rep     movsb
            add     edi, eax
            add     esi, edx
            dec     bmHeight
            jnz     next
        }
        break;
    }

    closeBitmap(&bmp);
}

// Convert bitmap struct to image struct
void convertBitmap(GFX_BITMAP *bmp, GFX_IMAGE *img)
{
    uint32_t size = bmp->bmRowBytes * bmp->bmHeight;
    img->mData = (uint8_t*)calloc(size, 1);
    if (!img->mData) fatalError("convertBitmap: cannot alloc memory.\n");
    img->mWidth     = bmp->bmWidth;
    img->mHeight    = bmp->bmHeight;
    img->mPixels    = bmp->bmPixels;
    img->mSize      = size;
    img->mRowBytes  = bmp->bmRowBytes;
    memcpy(img->mData, bmp->bmData, size);
}

/*===================================================*/
/*                PNG image loader                   */
/*       https://github.com/lvandeve/lodepng         */
/*===================================================*/

#define PNG_VERSION_STRING  "20220109"

typedef enum PNGColorType
{
    LCT_GREY            = 0,
    LCT_RGB             = 2,
    LCT_PALETTE         = 3,
    LCT_GREY_ALPHA      = 4,
    LCT_RGBA            = 6,
    LCT_MAX_OCTET_VALUE = 255
} PNGColorType;

typedef struct PNGDecompressSettings PNGDecompressSettings;
struct PNGDecompressSettings
{
    uint32_t    ignoreAdler32;
    uint32_t    ignoreLen;
    uint32_t    maxOutputSize;
    uint32_t    (*customZlib)(uint8_t**, uint32_t*, const uint8_t*, uint32_t, const PNGDecompressSettings*);
    uint32_t    (*customInflate)(uint8_t**, uint32_t*, const uint8_t*, uint32_t, const PNGDecompressSettings*);
    const void  *customContext;
};

extern const PNGDecompressSettings PNGDefaultDecompressSettings;

typedef struct PNGCompressSettings PNGCompressSettings;
struct PNGCompressSettings
{
    uint32_t    btype;
    uint32_t    useLZ77;
    uint32_t    windowSize;
    uint32_t    minMatch;
    uint32_t    niceMatch;
    uint32_t    lazyMatching;
    uint32_t    (*customZlib)(uint8_t**, uint32_t*, const uint8_t*, uint32_t, const PNGCompressSettings*);
    uint32_t    (*customDeflate)(uint8_t**, uint32_t*, const uint8_t*, uint32_t, const PNGCompressSettings*);
    const void  *customContext;
};

extern const PNGCompressSettings PNGDefaultCompressSettings;

typedef struct PNGColorMode
{
    PNGColorType    colorType;
    uint32_t        bitDepth;
    uint8_t*        palette;
    uint32_t        paletteSize;
    uint32_t        keyDefined;
    uint32_t        keyR;
    uint32_t        keyG;
    uint32_t        keyB;
} PNGColorMode;

typedef struct PNGTime
{
    uint32_t    year;
    uint32_t    month;
    uint32_t    day;
    uint32_t    hour;
    uint32_t    minute;
    uint32_t    second;
} PNGTime;

typedef struct PNGInfo
{
    uint32_t        compressionMethod;
    uint32_t        filterMethod;
    uint32_t        interlaceMethod;
    PNGColorMode    colorMode;
    uint32_t        backgroundDefined;
    uint32_t        backgroundR;
    uint32_t        backgroundG;
    uint32_t        backgroundB;
    uint32_t        textNum;
    char**          textKeys;
    char**          textStrings;
    uint32_t        itextNum;
    char**          itextKeys;
    char**          itextLangTags;
    char**          itextTransKeys;
    char**          itextStrings;
    uint32_t        timeDefined;
    PNGTime         time;
    uint32_t        physDefined;
    uint32_t        physX;
    uint32_t        physY;
    uint32_t        physUnit;
    uint32_t        gamaDefined;
    uint32_t        gamaGamma;
    uint32_t        chrmDefined;
    uint32_t        chrmWhiteX;
    uint32_t        chrmWhiteY;
    uint32_t        chrmRedX;
    uint32_t        chrmRedY;
    uint32_t        chrmGreenX;
    uint32_t        chrmGreenY;
    uint32_t        chrmBlueX;
    uint32_t        chrmBlueY;
    uint32_t        srgbDefined;
    uint32_t        srgbIntent;
    uint32_t        iccpDefined;
    char*           iccpName;
    uint8_t*        iccpProfile;
    uint32_t        iccpProfileSize;
    uint8_t*        unknownChunksData[3];
    uint32_t        unknownChunksSize[3];
} PNGInfo;

typedef struct PNGDecoderSettings
{
    PNGDecompressSettings   zlibSettings;
    uint32_t                ignoreCRC;
    uint32_t                ignoreCritical;
    uint32_t                ignoreEnd;
    uint32_t                colorConvert;
    uint32_t                readTextChunks;
    uint32_t                rememberUnknownChunks;
    uint32_t                maxTextSize;
    uint32_t                maxIccSize;
} PNGDecoderSettings;

typedef enum PNGFilterStrategy
{
    LFS_ZERO            = 0,
    LFS_ONE             = 1,
    LFS_TWO             = 2,
    LFS_THREE           = 3,
    LFS_FOUR            = 4,
    LFS_MINSUM,
    LFS_ENTROPY,
    LFS_BRUTE_FORCE,
    LFS_PREDEFINED
} PNGFilterStrategy;

typedef struct PNGColorStats
{
    uint32_t    colored;
    uint32_t    key;
    uint16_t    keyR;
    uint16_t    keyG;
    uint16_t    keyB;
    uint32_t    alpha;
    uint32_t    numColors;
    uint8_t     palette[1024];
    uint32_t    bits;
    uint32_t    numPixels;     
    uint32_t    allowPalette;
    uint32_t    allowGreyscale;
} PNGColorStats;

typedef struct PNGEncoderSettings
{
    PNGCompressSettings zlibSettings;
    uint32_t            autoConvert;
    uint32_t            fillterPaletteZero;
    PNGFilterStrategy   fillterStrategy;
    const uint8_t*      predefinedFilters;
    uint32_t            forcePalette;
    uint32_t            addID;
    uint32_t            textCompression;
} PNGEncoderSettings;

typedef struct PNGState
{
    PNGDecoderSettings  decoder;
    PNGEncoderSettings  encoder;
    PNGColorMode        rawInfo;
    PNGInfo             pngInfo;
    uint32_t            error;
} PNGState;

/*==================================================*/
/* Tools for C, and common code for PNG and Zlib    */
/*==================================================*/

#define CERROR_BREAK(errorvar, code) { errorvar = code; break; }
#define ERROR_BREAK(code) CERROR_BREAK(error, code)
#define CERROR_RETURN_ERROR(errorvar, code) { errorvar = code; return code; }
#define CERROR_TRY_RETURN(call) { int32_t error = call; if (error) return error; }
#define CERROR_RETURN(errorvar, code) { errorvar = code; return; }

typedef struct uiVector
{
    uint32_t *data;
    uint32_t size;
    uint32_t allocSize;
} uiVector;

void uiVectorCleanup(void* p)
{
    ((uiVector*)p)->size = ((uiVector*)p)->allocSize = 0;
    free(((uiVector*)p)->data);
    ((uiVector*)p)->data = NULL;
}

uint32_t uiVectorResize(uiVector *p, uint32_t size)
{
    uint32_t allocSize = size * sizeof(uint32_t);
    if (allocSize > p->allocSize)
    {
        uint32_t newSize = allocSize + (p->allocSize >> 1);
        void *data = realloc(p->data, newSize);
        if (data)
        {
            p->allocSize = newSize;
            p->data = (uint32_t*)data;
        }
        else return 0;
    }
    p->size = size;
    return 1;
}

void uiVectorInit(uiVector* p)
{
    p->data = NULL;
    p->size = p->allocSize = 0;
}

uint32_t uiVectorPushBack(uiVector *p, uint32_t c)
{
    if (!uiVectorResize(p, p->size + 1)) return 0;
    p->data[p->size - 1] = c;
    return 1;
}

typedef struct ucVector
{
    uint8_t* data;
    uint32_t size;
    uint32_t allocSize;
} ucVector;

uint32_t ucVectorReserve(ucVector* p, uint32_t size)
{
    if (size > p->allocSize)
    {
        uint32_t newSize = size + (p->allocSize >> 1u);
        void* data = realloc(p->data, newSize);
        if (data)
        {
            p->allocSize = newSize;
            p->data = (uint8_t*)data;
        }
        else return 0;
    }
    return 1;
}

uint32_t ucVectorResize(ucVector *p, uint32_t size)
{
    p->size = size;
    return ucVectorReserve(p, size);
}

ucVector ucVectorInit(uint8_t* buffer, uint32_t size)
{
    ucVector v;
    v.data = buffer;
    v.size = v.allocSize = size;
    return v;
}

void stringCleanup(char** out)
{
    free(*out);
    *out = NULL;
}

char* allocStringSized(const char* in, uint32_t insize)
{
    char* out = (char*)calloc(insize + 1, 1);
    if (out)
    {
        memcpy(out, in, insize);
        out[insize] = 0;
    }
    return out;
}

char* allocString(const char* in)
{
    return allocStringSized(in, strlen(in));
}

uint32_t PNGRead32bitInt(const uint8_t *buffer)
{
    return (buffer[0] << 24) | (buffer[1] << 16) | (buffer[2] << 8) | buffer[3];
}

void PNGSet32bitInt(uint8_t *buffer, uint32_t value)
{
    buffer[0] = (uint8_t)((value >> 24) & 0xff);
    buffer[1] = (uint8_t)((value >> 16) & 0xff);
    buffer[2] = (uint8_t)((value >>  8) & 0xff);
    buffer[3] = (uint8_t)((value      ) & 0xff);
}

int32_t PNGAddofl(uint32_t a, uint32_t b, uint32_t* result)
{
    *result = a + b;
    return *result < a;
}

int32_t PNGMulofl(uint32_t a, uint32_t b, uint32_t* result)
{
    *result = a * b;
    return (a != 0 && *result / a != b);
}

int32_t PNGGtofl(uint32_t a, uint32_t b, uint32_t c)
{
    uint32_t d;
    if (PNGAddofl(a, b, &d)) return 1;
    return d > c;
}

typedef struct
{
    ucVector    *data;
    uint8_t     bp;
} PNGBitWriter;

void PNGBitWriterInit(PNGBitWriter* writer, ucVector* data)
{
    writer->data = data;
    writer->bp = 0;
}

#define WRITEBIT(writer, bit) {\
    if (((writer->bp) & 7u) == 0) {\
        if (!ucVectorResize(writer->data, writer->data->size + 1)) return;\
        writer->data->data[writer->data->size - 1] = 0;\
    }\
    (writer->data->data[writer->data->size - 1]) |= (bit << ((writer->bp) & 7u));\
    ++writer->bp;\
}

void writeBits(PNGBitWriter* writer, uint32_t value, uint32_t nbits)
{
    if (nbits == 1)
    {
        WRITEBIT(writer, value);
    }
    else
    {
        uint32_t i;
        for (i = 0; i != nbits; ++i)
        {
            WRITEBIT(writer, (uint8_t)((value >> i) & 1));
        }
    }
}

void writeBitsReversed(PNGBitWriter* writer, uint32_t value, uint32_t nbits)
{
    uint32_t i;
    for (i = 0; i != nbits; ++i)
    {
        WRITEBIT(writer, (uint8_t)((value >> (nbits - 1u - i)) & 1u));
    }
}

typedef struct
{
  const uint8_t *data;
  uint32_t      size;
  uint32_t      bitsize;
  uint32_t      bp;
  uint32_t      buffer;
} PNGBitReader;

uint32_t PNGBitReaderInit(PNGBitReader* reader, const uint8_t* data, uint32_t size)
{
    uint32_t temp;

    reader->data = data;
    reader->size = size;

    if (PNGMulofl(size, 8u, &reader->bitsize)) return 105;
    if (PNGAddofl(reader->bitsize, 64u, &temp)) return 105;

    reader->bp = 0;
    reader->buffer = 0;
    return 0;
}

void ensureBits9(PNGBitReader* reader, uint32_t nbits)
{
    uint32_t start = reader->bp >> 3u;
    uint32_t size = reader->size;

    if (start + 1u < size)
    {
        reader->buffer = (uint32_t)reader->data[start + 0] | ((uint32_t)reader->data[start + 1] << 8u);
        reader->buffer >>= (reader->bp & 7u);
    }
    else
    {
        reader->buffer = 0;
        if (start + 0u < size) reader->buffer = reader->data[start + 0];
        reader->buffer >>= (reader->bp & 7u);
    }
    nbits;
}

void ensureBits17(PNGBitReader* reader, uint32_t nbits)
{
    uint32_t start = reader->bp >> 3u;
    uint32_t size = reader->size;

    if (start + 2u < size)
    {
        reader->buffer = (uint32_t)reader->data[start + 0] | ((uint32_t)reader->data[start + 1] << 8u) | ((uint32_t)reader->data[start + 2] << 16u);
        reader->buffer >>= (reader->bp & 7u);
    }
    else
    {
        reader->buffer = 0;
        if (start + 0u < size) reader->buffer |= reader->data[start + 0];
        if (start + 1u < size) reader->buffer |= ((uint32_t)reader->data[start + 1] << 8u);
        reader->buffer >>= (reader->bp & 7u);
    }
    nbits;
}

void ensureBits25(PNGBitReader* reader, uint32_t nbits)
{
    uint32_t start = reader->bp >> 3u;
    uint32_t size = reader->size;

    if (start + 3u < size)
    {
        reader->buffer = (uint32_t)reader->data[start + 0] | ((uint32_t)reader->data[start + 1] << 8u) | ((uint32_t)reader->data[start + 2] << 16u) | ((uint32_t)reader->data[start + 3] << 24u);
        reader->buffer >>= (reader->bp & 7u);
    }
    else
    {
        reader->buffer = 0;
        if (start + 0u < size) reader->buffer |= reader->data[start + 0];
        if (start + 1u < size) reader->buffer |= ((uint32_t)reader->data[start + 1] << 8u);
        if (start + 2u < size) reader->buffer |= ((uint32_t)reader->data[start + 2] << 16u);
        reader->buffer >>= (reader->bp & 7u);
    }
    nbits;
}

void ensureBits32(PNGBitReader* reader, uint32_t nbits)
{
    uint32_t start = reader->bp >> 3u;
    uint32_t size = reader->size;

    if (start + 4u < size)
    {
        reader->buffer = (uint32_t)reader->data[start + 0] | ((uint32_t)reader->data[start + 1] << 8u) | ((uint32_t)reader->data[start + 2] << 16u) | ((uint32_t)reader->data[start + 3] << 24u);
        reader->buffer >>= (reader->bp & 7u);
        reader->buffer |= (((uint32_t)reader->data[start + 4] << 24u) << (8u - (reader->bp & 7u)));
    }
    else
    {
        reader->buffer = 0;
        if (start + 0u < size) reader->buffer |= reader->data[start + 0];
        if (start + 1u < size) reader->buffer |= ((uint32_t)reader->data[start + 1] << 8u);
        if (start + 2u < size) reader->buffer |= ((uint32_t)reader->data[start + 2] << 16u);
        if (start + 3u < size) reader->buffer |= ((uint32_t)reader->data[start + 3] << 24u);
        reader->buffer >>= (reader->bp & 7u);
    }
    nbits;
}

uint32_t peekBits(PNGBitReader* reader, uint32_t nbits)
{
    return reader->buffer & ((1u << nbits) - 1u);
}

void advanceBits(PNGBitReader* reader, uint32_t nbits)
{
    reader->buffer >>= nbits;
    reader->bp += nbits;
}

uint32_t readBits(PNGBitReader* reader, uint32_t nbits)
{
    uint32_t result = peekBits(reader, nbits);
    advanceBits(reader, nbits);
    return result;
}

uint32_t reverseBits(uint32_t bits, uint32_t num)
{
    uint32_t i, result = 0;
    for (i = 0; i < num; i++) result |= ((bits >> (num - i - 1u)) & 1u) << i;
    return result;
}

/*====================*/
/* Deflate - Huffman  */ 
/*====================*/

#define FIRST_LENGTH_CODE_INDEX     257
#define LAST_LENGTH_CODE_INDEX      285
#define NUM_DEFLATE_CODE_SYMBOLS    288
#define NUM_DISTANCE_SYMBOLS        32
#define NUM_CODE_LENGTH_CODES       19

const uint32_t LENGTHBASE[29] = {3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17, 19, 23, 27, 31, 35, 43, 51, 59, 67, 83, 99, 115, 131, 163, 195, 227, 258};
const uint32_t LENGTHEXTRA[29] = {0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 0};
const uint32_t DISTANCEBASE[30] = {1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49, 65, 97, 129, 193, 257, 385, 513, 769, 1025, 1537, 2049, 3073, 4097, 6145, 8193, 12289, 16385, 24577};
const uint32_t DISTANCEEXTRA[30] = {0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13};
const uint32_t CLCL_ORDER[NUM_CODE_LENGTH_CODES] = {16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};

typedef struct HuffmanTree
{
    uint32_t*   codes;
    uint32_t*   lengths;
    uint32_t    maxBitLen;
    uint32_t    numCodes;
    uint8_t*    tableLen;
    uint16_t*   tableValue;
} HuffmanTree;

void huffmanTreeInit(HuffmanTree* tree)
{
    tree->codes = NULL;
    tree->lengths = NULL;
    tree->tableLen = NULL;
    tree->tableValue = NULL;
}

void huffmanTreeCleanup(HuffmanTree* tree)
{
    free(tree->codes);
    free(tree->lengths);
    free(tree->tableLen);
    free(tree->tableValue);
}

#define FIRSTBITS       9u
#define INVALIDSYMBOL   65535u

uint32_t huffmanTreeMakeTable(HuffmanTree* tree)
{
    const uint32_t headSize = 1u << FIRSTBITS;
    const uint32_t mask = (1u << FIRSTBITS) - 1u;
    uint32_t i, numPresent, pointer, size;
    uint32_t* maxLens = (uint32_t*)calloc(headSize, sizeof(uint32_t));

    if (!maxLens) return 83;

    memset(maxLens, 0, headSize * sizeof(*maxLens));

    for (i = 0; i < tree->numCodes; i++)
    {
        uint32_t index;
        uint32_t symbol = tree->codes[i];
        uint32_t l = tree->lengths[i];
        
        if (l <= FIRSTBITS) continue;
        index = reverseBits(symbol >> (l - FIRSTBITS), FIRSTBITS);
        maxLens[index] = max(maxLens[index], l);
    }

    size = headSize;

    for (i = 0; i < headSize; ++i)
    {
        uint32_t l = maxLens[i];
        if (l > FIRSTBITS) size += (1u << (l - FIRSTBITS));
    }

    tree->tableLen = (uint8_t*)calloc(size, sizeof(*tree->tableLen));
    tree->tableValue = (uint16_t*)calloc(size, sizeof(*tree->tableValue));

    if (!tree->tableLen || !tree->tableValue)
    {
        free(maxLens);
        return 83;
    }

    for (i = 0; i < size; ++i) tree->tableLen[i] = 16;

    pointer = headSize;
    for (i = 0; i < headSize; ++i)
    {
        uint32_t l = maxLens[i];
        
        if (l <= FIRSTBITS) continue;

        tree->tableLen[i] = l;
        tree->tableValue[i] = pointer;
        pointer += (1u << (l - FIRSTBITS));
    }

    free(maxLens);

    numPresent = 0;
    for (i = 0; i < tree->numCodes; ++i)
    {
        uint32_t l = tree->lengths[i];
        uint32_t symbol = tree->codes[i];
        uint32_t reverse = reverseBits(symbol, l);
        if (l == 0) continue;

        numPresent++;

        if (l <= FIRSTBITS)
        {
            uint32_t j;
            uint32_t num = 1u << (FIRSTBITS - l);
            
            for (j = 0; j < num; ++j)
            {
                uint32_t index = reverse | (j << l);
                if (tree->tableLen[index] != 16) return 55;
                tree->tableLen[index] = l;
                tree->tableValue[index] = i;
            }
        }
        else
        {
            uint32_t j;
            uint32_t index = reverse & mask;
            uint32_t maxlen = tree->tableLen[index];
            uint32_t tablelen = maxlen - FIRSTBITS;
            uint32_t start = tree->tableValue[index];
            uint32_t num = 1u << (tablelen - (l - FIRSTBITS));
            
            if (maxlen < l) return 55;
            for (j = 0; j < num; ++j)
            {
                uint32_t reverse2 = reverse >> FIRSTBITS;
                uint32_t index2 = start + (reverse2 | (j << (l - FIRSTBITS)));
                tree->tableLen[index2] = l;
                tree->tableValue[index2] = i;
            }
        }
    }

    if (numPresent < 2)
    {
        for (i = 0; i < size; ++i)
        {
            if (tree->tableLen[i] == 16)
            {
                tree->tableLen[i] = (i < headSize) ? 1 : (FIRSTBITS + 1);
                tree->tableValue[i] = INVALIDSYMBOL;
            }
        }
    }
    else
    {
        for (i = 0; i < size; ++i)
        {
            if (tree->tableLen[i] == 16) return 55;
        }
    }
    return 0;
}

uint32_t huffmanTreeMakeFromLengths2(HuffmanTree* tree)
{
    uint32_t* blcount;
    uint32_t* nextCode;
    uint32_t error = 0;
    uint32_t bits, n;

    tree->codes = (uint32_t*)calloc(tree->numCodes, sizeof(uint32_t));
    blcount = (uint32_t*)calloc(tree->maxBitLen + 1, sizeof(uint32_t));
    nextCode = (uint32_t*)calloc(tree->maxBitLen + 1, sizeof(uint32_t));

    if (!tree->codes || !blcount || !nextCode) error = 83;

    if (!error)
    {
        for (n = 0; n != tree->maxBitLen + 1; n++) blcount[n] = nextCode[n] = 0;
        for (bits = 0; bits != tree->numCodes; ++bits) ++blcount[tree->lengths[bits]];
        for (bits = 1; bits <= tree->maxBitLen; ++bits)
        {
            nextCode[bits] = (nextCode[bits - 1] + blcount[bits - 1]) << 1u;
        }

        for (n = 0; n != tree->numCodes; ++n)
        {
            if (tree->lengths[n] != 0)
            {
                tree->codes[n] = nextCode[tree->lengths[n]]++;
                tree->codes[n] &= ((1u << tree->lengths[n]) - 1u);
            }
        }
    }

    free(blcount);
    free(nextCode);

    if (!error) error = huffmanTreeMakeTable(tree);
    return error;
}

uint32_t huffmanTreeMakeFromLengths(HuffmanTree* tree, const uint32_t* bitlen, uint32_t numCodes, uint32_t maxBitLen)
{
    uint32_t i;
    tree->lengths = (uint32_t*)calloc(numCodes, sizeof(uint32_t));
    if (!tree->lengths) return 83;

    for (i = 0; i != numCodes; ++i) tree->lengths[i] = bitlen[i];
    tree->numCodes = (uint32_t)numCodes;
    tree->maxBitLen = maxBitLen;
    return huffmanTreeMakeFromLengths2(tree);
}

typedef struct BPMNode
{
    int32_t         weight;
    uint32_t        index;
    struct BPMNode* tail;
    int32_t         used;
} BPMNode;

typedef struct BPMLists
{
    uint32_t    memsize;
    BPMNode*    memory;
    uint32_t    numfree;
    uint32_t    nextfree;
    BPMNode**   freelist;
    uint32_t    listsize;
    BPMNode**   chains0;
    BPMNode**   chains1;
} BPMLists;

BPMNode* BPMNodeCreate(BPMLists* lists, int32_t weight, uint32_t index, BPMNode* tail)
{
    uint32_t i;
    BPMNode* result;

    if (lists->nextfree >= lists->numfree)
    {
        for (i = 0; i != lists->memsize; ++i) lists->memory[i].used = 0;

        for (i = 0; i != lists->listsize; ++i)
        {
            BPMNode* node;
            for (node = lists->chains0[i]; node != 0; node = node->tail) node->used = 1;
            for (node = lists->chains1[i]; node != 0; node = node->tail) node->used = 1;
        }

        lists->numfree = 0;
        for (i = 0; i != lists->memsize; ++i)
        {
            if (!lists->memory[i].used) lists->freelist[lists->numfree++] = &lists->memory[i];
        }
        lists->nextfree = 0;
    }

    result = lists->freelist[lists->nextfree++];
    result->weight = weight;
    result->index = index;
    result->tail = tail;

    return result;
}

void BPMNodeSort(BPMNode* leaves, uint32_t num)
{
    uint32_t p, q, r, i, j, k, width, counter = 0;
    BPMNode* mem = (BPMNode*)calloc(num, sizeof(*leaves));

    for (width = 1; width < num; width *= 2)
    {
        BPMNode* a = (counter & 1) ? mem : leaves;
        BPMNode* b = (counter & 1) ? leaves : mem;

        for (p = 0; p < num; p += 2 * width)
        {
            q = (p + width > num) ? num : (p + width);
            r = (p + 2 * width > num) ? num : (p + 2 * width);
            i = p, j = q;
            for (k = p; k < r; k++)
            {
                if (i < q && (j >= r || a[i].weight <= a[j].weight)) b[k] = a[i++];
                else b[k] = a[j++];
            }
        }
        counter++;
    }

    if (counter & 1) memcpy(leaves, mem, sizeof(*leaves) * num);
    free(mem);
}

void boundaryPM(BPMLists* lists, BPMNode* leaves, uint32_t numPresent, int32_t c, int32_t num)
{
    uint32_t lastIndex = lists->chains1[c]->index;

    if (c == 0)
    {
        if (lastIndex >= numPresent) return;
        lists->chains0[c] = lists->chains1[c];
        lists->chains1[c] = BPMNodeCreate(lists, leaves[lastIndex].weight, lastIndex + 1, 0);
    }
    else
    {
        int32_t sum = lists->chains0[c - 1]->weight + lists->chains1[c - 1]->weight;
        lists->chains0[c] = lists->chains1[c];

        if (lastIndex < numPresent && sum > leaves[lastIndex].weight)
        {
            lists->chains1[c] = BPMNodeCreate(lists, leaves[lastIndex].weight, lastIndex + 1, lists->chains1[c]->tail);
            return;
        }

        lists->chains1[c] = BPMNodeCreate(lists, sum, lastIndex, lists->chains1[c - 1]);

        if (num + 1 < (int32_t)(2 * numPresent - 2))
        {
            boundaryPM(lists, leaves, numPresent, c - 1, num);
            boundaryPM(lists, leaves, numPresent, c - 1, num);
        }
    }
}

uint32_t PNGHuffmanCodeLengths(uint32_t* lengths, const uint32_t* frequencies, uint32_t numCodes, uint32_t maxBitLen)
{
    uint32_t i, error = 0;
    uint32_t numPresent = 0;
    BPMNode* leaves;

    if (numCodes == 0) return 80;
    if ((1u << maxBitLen) < (uint32_t)numCodes) return 80;

    leaves = (BPMNode*)calloc(numCodes, sizeof(*leaves));
    if (!leaves) return 83;

    for (i = 0; i != numCodes; i++)
    {
        if (frequencies[i] > 0)
        {
            leaves[numPresent].weight = (int32_t)frequencies[i];
            leaves[numPresent].index = i;
            ++numPresent;
        }
    }

    memset(lengths, 0, numCodes * sizeof(*lengths));

    if (numPresent == 0)
    {
        lengths[0] = lengths[1] = 1;
    }
    else if (numPresent == 1)
    {
        lengths[leaves[0].index] = 1;
        lengths[leaves[0].index == 0 ? 1 : 0] = 1;
    }
    else
    {
        BPMLists lists;
        BPMNode* node;

        BPMNodeSort(leaves, numPresent);

        lists.listsize = maxBitLen;
        lists.memsize = 2 * maxBitLen * (maxBitLen + 1);
        lists.nextfree = 0;
        lists.numfree = lists.memsize;
        lists.memory = (BPMNode*)calloc(lists.memsize, sizeof(*lists.memory));
        lists.freelist = (BPMNode**)calloc(lists.memsize, sizeof(BPMNode*));
        lists.chains0 = (BPMNode**)calloc(lists.listsize, sizeof(BPMNode*));
        lists.chains1 = (BPMNode**)calloc(lists.listsize, sizeof(BPMNode*));
        if (!lists.memory || !lists.freelist || !lists.chains0 || !lists.chains1) error = 83;

        if (!error)
        {
            for (i = 0; i != lists.memsize; ++i) lists.freelist[i] = &lists.memory[i];

            BPMNodeCreate(&lists, leaves[0].weight, 1, 0);
            BPMNodeCreate(&lists, leaves[1].weight, 2, 0);

            for (i = 0; i != lists.listsize; ++i)
            {
                lists.chains0[i] = &lists.memory[0];
                lists.chains1[i] = &lists.memory[1];
            }

            for (i = 2; i != 2 * numPresent - 2; ++i) boundaryPM(&lists, leaves, numPresent, (int32_t)maxBitLen - 1, (int32_t)i);

            for (node = lists.chains1[maxBitLen - 1]; node; node = node->tail)
            {
                for (i = 0; i != node->index; ++i) ++lengths[leaves[i].index];
            }
        }

        free(lists.memory);
        free(lists.freelist);
        free(lists.chains0);
        free(lists.chains1);
    }

    free(leaves);
    return error;
}

uint32_t huffmanTreeMakeFromFrequencies(HuffmanTree* tree, const uint32_t* frequencies, uint32_t minCodes, uint32_t numCodes, uint32_t maxBitLen)
{
    uint32_t error = 0;
    while (!frequencies[numCodes - 1] && numCodes > minCodes) --numCodes;

    tree->lengths = (uint32_t*)calloc(numCodes, sizeof(uint32_t));
    if (!tree->lengths) return 83;

    tree->maxBitLen = maxBitLen;
    tree->numCodes = numCodes;
    error = PNGHuffmanCodeLengths(tree->lengths, frequencies, numCodes, maxBitLen);
    if (!error) error = huffmanTreeMakeFromLengths2(tree);

    return error;
}

uint32_t generateFixedLitLenTree(HuffmanTree* tree)
{
    uint32_t i, error = 0;
    uint32_t* bitlen = (uint32_t*)calloc(NUM_DEFLATE_CODE_SYMBOLS, sizeof(uint32_t));

    if (!bitlen) return 83;

    for (i =   0; i <= 143; ++i) bitlen[i] = 8;
    for (i = 144; i <= 255; ++i) bitlen[i] = 9;
    for (i = 256; i <= 279; ++i) bitlen[i] = 7;
    for (i = 280; i <= 287; ++i) bitlen[i] = 8;

    error = huffmanTreeMakeFromLengths(tree, bitlen, NUM_DEFLATE_CODE_SYMBOLS, 15);
    free(bitlen);
    return error;
}

uint32_t generateFixedDistanceTree(HuffmanTree* tree)
{
    uint32_t i, error = 0;
    uint32_t* bitlen = (uint32_t*)calloc(NUM_DISTANCE_SYMBOLS, sizeof(uint32_t));

    if (!bitlen) return 83;

    for (i = 0; i != NUM_DISTANCE_SYMBOLS; ++i) bitlen[i] = 5;
    error = huffmanTreeMakeFromLengths(tree, bitlen, NUM_DISTANCE_SYMBOLS, 15);
    free(bitlen);
    return error;
}

uint32_t huffmanDecodeSymbol(PNGBitReader* reader, const HuffmanTree* codetree)
{
    uint16_t code = peekBits(reader, FIRSTBITS);
    uint16_t l = codetree->tableLen[code];
    uint16_t value = codetree->tableValue[code];

    if (l <= FIRSTBITS)
    {
        advanceBits(reader, l);
        return value;
    }
    else
    {
        advanceBits(reader, FIRSTBITS);
        value += peekBits(reader, l - FIRSTBITS);
        advanceBits(reader, codetree->tableLen[value] - FIRSTBITS);
        return codetree->tableValue[value];
    }
}

/*===========================*/
/* Inflator (Decompressor)   */
/*===========================*/

uint32_t getTreeInflateFixed(HuffmanTree* tree_ll, HuffmanTree* tree_d)
{
    uint32_t error = generateFixedLitLenTree(tree_ll);
    if (error) return error;
    return generateFixedDistanceTree(tree_d);
}

uint32_t getTreeInflateDynamic(HuffmanTree* tree_ll, HuffmanTree* tree_d, PNGBitReader* reader)
{
    uint32_t error = 0;
    uint32_t n, HLIT, HDIST, HCLEN, i;
    uint32_t* bitlen_ll = 0;
    uint32_t* bitlen_d = 0;
    uint32_t* bitlen_cl = 0;
    HuffmanTree tree_cl;

    if (reader->bitsize - reader->bp < 14) return 49;
    ensureBits17(reader, 14);

    HLIT = readBits(reader, 5) + 257;
    HDIST = readBits(reader, 5) + 1;
    HCLEN = readBits(reader, 4) + 4;

    bitlen_cl = (uint32_t*)calloc(NUM_CODE_LENGTH_CODES, sizeof(uint32_t));
    if (!bitlen_cl) return 83;

    huffmanTreeInit(&tree_cl);

    while (!error)
    {
        if (PNGGtofl(reader->bp, HCLEN * 3, reader->bitsize))
        {
            ERROR_BREAK(50);
        }

        for (i = 0; i != HCLEN; ++i)
        {
            ensureBits9(reader, 3);
            bitlen_cl[CLCL_ORDER[i]] = readBits(reader, 3);
        }

        for (i = HCLEN; i != NUM_CODE_LENGTH_CODES; ++i)
        {
            bitlen_cl[CLCL_ORDER[i]] = 0;
        }

        error = huffmanTreeMakeFromLengths(&tree_cl, bitlen_cl, NUM_CODE_LENGTH_CODES, 7);
        if (error) break;

        bitlen_ll = (uint32_t*)calloc(NUM_DEFLATE_CODE_SYMBOLS, sizeof(uint32_t));
        bitlen_d = (uint32_t*)calloc(NUM_DISTANCE_SYMBOLS, sizeof(uint32_t));
        if (!bitlen_ll || !bitlen_d) ERROR_BREAK(83);

        memset(bitlen_ll, 0, NUM_DEFLATE_CODE_SYMBOLS * sizeof(*bitlen_ll));
        memset(bitlen_d, 0, NUM_DISTANCE_SYMBOLS * sizeof(*bitlen_d));

        i = 0;
        while (i < HLIT + HDIST)
        {
            uint32_t code;
            ensureBits25(reader, 22);
            code = huffmanDecodeSymbol(reader, &tree_cl);

            if (code <= 15)
            {
                if (i < HLIT) bitlen_ll[i] = code;
                else bitlen_d[i - HLIT] = code;
                ++i;
            }
            else if (code == 16)
            {
                uint32_t value;
                uint32_t repLength = 3;
                
                if (i == 0) ERROR_BREAK(54);

                repLength += readBits(reader, 2);

                if (i < HLIT + 1) value = bitlen_ll[i - 1];
                else value = bitlen_d[i - HLIT - 1];
                
                for (n = 0; n < repLength; ++n)
                {
                    if (i >= HLIT + HDIST) ERROR_BREAK(13);
                    if (i < HLIT) bitlen_ll[i] = value;
                    else bitlen_d[i - HLIT] = value;
                    ++i;
                }
            }
            else if (code == 17)
            {
                uint32_t repLength = 3;
                repLength += readBits(reader, 3);

                for (n = 0; n < repLength; ++n)
                {
                    if (i >= HLIT + HDIST) ERROR_BREAK(14);
                    if (i < HLIT) bitlen_ll[i] = 0;
                    else bitlen_d[i - HLIT] = 0;
                    ++i;
                }
            }
            else if (code == 18)
            {
                uint32_t repLength = 11;
                repLength += readBits(reader, 7);

                for (n = 0; n < repLength; ++n)
                {
                    if (i >= HLIT + HDIST) ERROR_BREAK(15);
                    if (i < HLIT) bitlen_ll[i] = 0;
                    else bitlen_d[i - HLIT] = 0;
                    ++i;
                }
            }
            else
            {
                ERROR_BREAK(16);
            }

            if (reader->bp > reader->bitsize)
            {
                ERROR_BREAK(50);
            }
        }

        if (error) break;

        if (bitlen_ll[256] == 0) ERROR_BREAK(64);

        error = huffmanTreeMakeFromLengths(tree_ll, bitlen_ll, NUM_DEFLATE_CODE_SYMBOLS, 15);
        if (error) break;

        error = huffmanTreeMakeFromLengths(tree_d, bitlen_d, NUM_DISTANCE_SYMBOLS, 15);
        break;
    }

    free(bitlen_cl);
    free(bitlen_ll);
    free(bitlen_d);
    huffmanTreeCleanup(&tree_cl);
    return error;
}

uint32_t inflateHuffmanBlock(ucVector* out, PNGBitReader* reader, uint32_t btype, uint32_t maxOutputSize)
{
    uint32_t error = 0;
    HuffmanTree tree_ll;
    HuffmanTree tree_d;

    int32_t done = 0;
    const uint32_t reservedSize = 260;
    
    if (!ucVectorReserve(out, out->size + reservedSize)) return 83;

    huffmanTreeInit(&tree_ll);
    huffmanTreeInit(&tree_d);

    if (btype == 1) error = getTreeInflateFixed(&tree_ll, &tree_d);
    else error = getTreeInflateDynamic(&tree_ll, &tree_d, reader);

    while (!error && !done)
    {
        uint32_t code_ll;
        ensureBits32(reader, 30);
        code_ll = huffmanDecodeSymbol(reader, &tree_ll);
        if (code_ll <= 255)
        {
            out->data[out->size++] = (uint8_t)code_ll;
            code_ll = huffmanDecodeSymbol(reader, &tree_ll);
        }
        if (code_ll <= 255)
        {
            out->data[out->size++] = (uint8_t)code_ll;
        }
        else if (code_ll >= FIRST_LENGTH_CODE_INDEX && code_ll <= LAST_LENGTH_CODE_INDEX)
        {
            uint32_t code_d, distance;
            uint32_t numExtraBits_l, numExtraBits_d;
            uint32_t start, backward, length;

            length = LENGTHBASE[code_ll - FIRST_LENGTH_CODE_INDEX];
            numExtraBits_l = LENGTHEXTRA[code_ll - FIRST_LENGTH_CODE_INDEX];
            if (numExtraBits_l != 0)
            {
                ensureBits25(reader, 5);
                length += readBits(reader, numExtraBits_l);
            }

            ensureBits32(reader, 28);
            code_d = huffmanDecodeSymbol(reader, &tree_d);
            if (code_d > 29)
            {
                if (code_d <= 31)
                {
                    ERROR_BREAK(18);
                }
                else
                {
                    ERROR_BREAK(16);
                }
            }

            distance = DISTANCEBASE[code_d];

            numExtraBits_d = DISTANCEEXTRA[code_d];
            if (numExtraBits_d != 0)
            {
                distance += readBits(reader, numExtraBits_d);
            }

            start = out->size;
            if (distance > start) ERROR_BREAK(52);
            backward = start - distance;

            out->size += length;
            if (distance < length)
            {
                uint32_t forward;
                memcpy(out->data + start, out->data + backward, distance);
                start += distance;
                for (forward = distance; forward < length; ++forward)
                {
                    out->data[start++] = out->data[backward++];
                }
            }
            else
            {
                memcpy(out->data + start, out->data + backward, length);
            }
        }
        else if (code_ll == 256)
        {
            break;
        }
        else
        {
            ERROR_BREAK(16);
        }
        if (out->allocSize - out->size < reservedSize)
        {
            if (!ucVectorReserve(out, out->size + reservedSize)) ERROR_BREAK(83);
        }
        if (reader->bp > reader->bitsize)
        {
            ERROR_BREAK(51);
        }

        if (maxOutputSize && out->size > maxOutputSize)
        {
            ERROR_BREAK(109);
        }
    }

    huffmanTreeCleanup(&tree_ll);
    huffmanTreeCleanup(&tree_d);

    return error;
}

uint32_t inflateNoCompression(ucVector* out, PNGBitReader* reader, const PNGDecompressSettings* settings)
{
    uint32_t bytePos;
    uint32_t size = reader->size;
    uint32_t LEN, NLEN, error = 0;

    bytePos = (reader->bp + 7u) >> 3u;

    if (bytePos + 4 >= size) return 52;

    LEN = (uint32_t)reader->data[bytePos] + ((uint32_t)reader->data[bytePos + 1] << 8u); bytePos += 2;
    NLEN = (uint32_t)reader->data[bytePos] + ((uint32_t)reader->data[bytePos + 1] << 8u); bytePos += 2;

    if (!settings->ignoreLen && LEN + NLEN != 65535)
    {
        return 21;
    }

    if (!ucVectorResize(out, out->size + LEN)) return 83;

    if (bytePos + LEN > size) return 23;

    memcpy(out->data + out->size - LEN, reader->data + bytePos, LEN);
    bytePos += LEN;

    reader->bp = bytePos << 3u;

    return error;
}

uint32_t PNGInflatev(ucVector* out, const uint8_t* in, uint32_t insize, const PNGDecompressSettings* settings)
{
    uint32_t BFINAL = 0;
    PNGBitReader reader;
    uint32_t error = PNGBitReaderInit(&reader, in, insize);

    if (error) return error;

    while (!BFINAL)
    {
        uint32_t BTYPE;
        if (reader.bitsize - reader.bp < 3) return 52;
        ensureBits9(&reader, 3);
        BFINAL = readBits(&reader, 1);
        BTYPE = readBits(&reader, 2);

        if (BTYPE == 3) return 20;
        else if (BTYPE == 0) error = inflateNoCompression(out, &reader, settings);
        else error = inflateHuffmanBlock(out, &reader, BTYPE, settings->maxOutputSize);

        if (!error && settings->maxOutputSize && out->size > settings->maxOutputSize) error = 109;
        if (error) break;
    }

    return error;
}

uint32_t PNGInflate(uint8_t** out, uint32_t* outSize, const uint8_t* in, uint32_t insize, const PNGDecompressSettings* settings)
{
    ucVector v = ucVectorInit(*out, *outSize);
    uint32_t error = PNGInflatev(&v, in, insize, settings);
    *out = v.data;
    *outSize = v.size;
    return error;
}

uint32_t inflatev(ucVector* out, const uint8_t* in, uint32_t insize, const PNGDecompressSettings* settings)
{
    if (settings->customInflate)
    {
        uint32_t error = settings->customInflate(&out->data, &out->size, in, insize, settings);
        out->allocSize = out->size;

        if (error)
        {
            error = 110;
            if (settings->maxOutputSize && out->size > settings->maxOutputSize) error = 109;
        }
        return error;
    }
    else
    {
        return PNGInflatev(out, in, insize, settings);
    }
}

/*===========================*/
/*   Deflator (Compressor)   */ 
/*===========================*/

const uint32_t MAX_SUPPORTED_DEFLATE_LENGTH = 258;

uint32_t searchCodeIndex(const uint32_t* array, uint32_t arraySize, uint32_t value)
{
    uint32_t left = 1;
    uint32_t right = arraySize - 1;

    while (left <= right)
    {
        uint32_t mid = (left + right) >> 1;
        if (array[mid] >= value) right = mid - 1;
        else left = mid + 1;
    }

    if (left >= arraySize || array[left] > value) left--;
    return left;
}

void addLengthDistance(uiVector* values, uint32_t length, uint32_t distance)
{
    uint32_t lengthCode = (uint32_t)searchCodeIndex(LENGTHBASE, 29, length);
    uint32_t extraLength = (uint32_t)(length - LENGTHBASE[lengthCode]);
    uint32_t distCode = (uint32_t)searchCodeIndex(DISTANCEBASE, 30, distance);
    uint32_t extraDistance = (uint32_t)(distance - DISTANCEBASE[distCode]);

    uint32_t pos = values->size;
    uint32_t ok = uiVectorResize(values, values->size + 4);

    if (ok)
    {
        values->data[pos + 0] = lengthCode + FIRST_LENGTH_CODE_INDEX;
        values->data[pos + 1] = extraLength;
        values->data[pos + 2] = distCode;
        values->data[pos + 3] = extraDistance;
    }
}

const uint32_t HASH_NUM_VALUES = 65536;
const uint32_t HASH_BIT_MASK = 65535;

typedef struct HASH
{
    int32_t*    head;
    uint16_t*   chain;
    int32_t*    val;
    int32_t*    headz;
    uint16_t*   chainz;
    uint16_t*   zeros;
} HASH;

uint32_t hashInit(HASH* hash, uint32_t windowSize)
{
    uint32_t i;
    hash->head = (int32_t*)calloc(HASH_NUM_VALUES, sizeof(int32_t));
    hash->val = (int32_t*)calloc(windowSize, sizeof(int32_t));
    hash->chain = (uint16_t*)calloc(windowSize, sizeof(uint16_t));

    hash->zeros = (uint16_t*)calloc(windowSize, sizeof(uint16_t));
    hash->headz = (int32_t*)calloc(MAX_SUPPORTED_DEFLATE_LENGTH + 1, sizeof(int32_t));
    hash->chainz = (uint16_t*)calloc(windowSize, sizeof(uint16_t));

    if (!hash->head || !hash->chain || !hash->val  || !hash->headz|| !hash->chainz || !hash->zeros) return 83;

    for (i = 0; i != HASH_NUM_VALUES; ++i) hash->head[i] = -1;
    for (i = 0; i != windowSize; ++i) hash->val[i] = -1;
    for (i = 0; i != windowSize; ++i) hash->chain[i] = i;

    for (i = 0; i <= MAX_SUPPORTED_DEFLATE_LENGTH; ++i) hash->headz[i] = -1;
    for (i = 0; i != windowSize; ++i) hash->chainz[i] = i;

    return 0;
}

void hashCleanup(HASH* hash)
{
    free(hash->head);
    free(hash->val);
    free(hash->chain);
    free(hash->zeros);
    free(hash->headz);
    free(hash->chainz);
}

uint32_t getHash(const uint8_t* data, uint32_t size, uint32_t pos)
{
    uint32_t result = 0;
    if (pos + 2 < size)
    {
        result ^= (uint32_t)(data[pos + 0] << 0u);
        result ^= (uint32_t)(data[pos + 1] << 4u);
        result ^= (uint32_t)(data[pos + 2] << 8u);
    }
    else
    {
        uint32_t amount, i;
        if (pos >= size) return 0;

        amount = size - pos;
        for (i = 0; i != amount; i++) result ^= (uint32_t)(data[pos + i] << (i * 8u));
    }

    return result & HASH_BIT_MASK;
}

uint32_t countZeros(const uint8_t* data, uint32_t size, uint32_t pos)
{
    const uint8_t* start = data + pos;
    const uint8_t* end = start + MAX_SUPPORTED_DEFLATE_LENGTH;
    if (end > data + size) end = data + size;

    data = start;
    while (data != end && *data == 0) ++data;

    return (uint32_t)(data - start);
}

void updateHashChain(HASH* hash, uint32_t wpos, uint32_t hashVal, uint16_t numZeros)
{
    hash->val[wpos] = (int32_t)hashVal;
    if (hash->head[hashVal] != -1) hash->chain[wpos] = hash->head[hashVal];
    hash->head[hashVal] = (int32_t)wpos;

    hash->zeros[wpos] = numZeros;
    if (hash->headz[numZeros] != -1) hash->chainz[wpos] = hash->headz[numZeros];
    hash->headz[numZeros] = (int32_t)wpos;
}

uint32_t encodeLZ77(uiVector* out, HASH* hash, const uint8_t* in, uint32_t inpos, uint32_t inSize, uint32_t windowSize, uint32_t minMatch, uint32_t niceMatch, uint32_t lazyMatching)
{
    uint32_t pos;
    uint32_t i, error = 0;

    uint32_t maxChainLength = windowSize >= 8192 ? windowSize : windowSize / 8;
    uint32_t maxLazyMatch = windowSize >= 8192 ? MAX_SUPPORTED_DEFLATE_LENGTH : 64;

    uint32_t useZeros = 1;
    uint32_t numZeros = 0;

    uint32_t offset;
    uint32_t len;
    uint32_t lazy = 0;
    uint32_t lazyLength = 0, lazyOffset = 0;
    uint32_t hashVal;
    uint32_t currOffset, currLength;
    uint32_t prevOffset;
    const uint8_t *lastPtr, *forePtr, *backPtr;
    uint32_t hashPos;

    if (windowSize == 0 || windowSize > 32768) return 60;
    if ((windowSize & (windowSize - 1)) != 0) return 90;

    if (niceMatch > MAX_SUPPORTED_DEFLATE_LENGTH) niceMatch = MAX_SUPPORTED_DEFLATE_LENGTH;

    for (pos = inpos; pos < inSize; ++pos)
    {
        uint32_t wpos = pos & (windowSize - 1);
        uint32_t chainLength = 0;

        hashVal = getHash(in, inSize, pos);

        if (useZeros && hashVal == 0)
        {
            if (numZeros == 0) numZeros = countZeros(in, inSize, pos);
            else if (pos + numZeros > inSize || in[pos + numZeros - 1] != 0) --numZeros;
        }
        else
        {
            numZeros = 0;
        }

        updateHashChain(hash, wpos, hashVal, (uint16_t)numZeros);

        len = 0;
        offset = 0;

        hashPos = hash->chain[wpos];
        lastPtr = &in[inSize < pos + MAX_SUPPORTED_DEFLATE_LENGTH ? inSize : pos + MAX_SUPPORTED_DEFLATE_LENGTH];

        prevOffset = 0;
        for (;;)
        {
            if (chainLength++ >= maxChainLength) break;
            currOffset = hashPos <= (uint32_t)(wpos ? wpos - hashPos : wpos - hashPos + windowSize);

            if (currOffset < prevOffset) break;

            prevOffset = currOffset;
            if (currOffset > 0)
            {
                forePtr = &in[pos];
                backPtr = &in[pos - currOffset];

                if (numZeros >= 3)
                {
                    uint32_t skip = hash->zeros[hashPos];
                    if (skip > numZeros) skip = numZeros;
                    backPtr += skip;
                    forePtr += skip;
                }

                while (forePtr != lastPtr && *backPtr == *forePtr)
                {
                    ++backPtr;
                    ++forePtr;
                }

                currLength = (uint32_t)(forePtr - &in[pos]);
                if (currLength > len)
                {
                    len = currLength;
                    offset = currOffset;
                    if (currLength >= niceMatch) break;
                }
            }

            if (hashPos == hash->chain[hashPos]) break;

            if (numZeros >= 3 && len > numZeros)
            {
                hashPos = hash->chainz[hashPos];
                if (hash->zeros[hashPos] != numZeros) break;
            }
            else
            {
                hashPos = hash->chain[hashPos];
                if (hash->val[hashPos] != (int32_t)hashVal) break;
            }
        }

        if (lazyMatching)
        {
            if (!lazy && len >= 3 && len <= maxLazyMatch && len < MAX_SUPPORTED_DEFLATE_LENGTH)
            {
                lazy = 1;
                lazyLength = len;
                lazyOffset = offset;
                continue;
            }

            if (lazy)
            {
                lazy = 0;
                if (pos == 0) ERROR_BREAK(81);

                if (len > lazyLength + 1)
                {
                    if (!uiVectorPushBack(out, in[pos - 1])) ERROR_BREAK(83);
                }
                else
                {
                    len = lazyLength;
                    offset = lazyOffset;
                    hash->head[hashVal] = -1;
                    hash->headz[numZeros] = -1;
                    --pos;
                }
            }
        }

        if (len >= 3 && offset > windowSize) ERROR_BREAK(86);

        if (len < 3)
        {
            if (!uiVectorPushBack(out, in[pos])) ERROR_BREAK(83);
        }
        else if (len < minMatch || (len == 3 && offset > 4096))
        {
            if (!uiVectorPushBack(out, in[pos])) ERROR_BREAK(83);
        }
        else
        {
            addLengthDistance(out, len, offset);
            for (i = 1; i != len; i++)
            {
                pos++;
                wpos = pos & (windowSize - 1);
                hashVal = getHash(in, inSize, pos);

                if (useZeros && hashVal == 0)
                {
                    if (numZeros == 0) numZeros = countZeros(in, inSize, pos);
                    else if (pos + numZeros > inSize || in[pos + numZeros - 1] != 0) --numZeros;
                }
                else
                {
                    numZeros = 0;
                }

                updateHashChain(hash, wpos, hashVal, (uint16_t)numZeros);
            }
        }
    }

    return error;
}

uint32_t deflateNoCompression(ucVector* out, const uint8_t* data, uint32_t dataSize)
{
    uint32_t i, numDeflateBlocks = (dataSize + 65534u) / 65535u;
    uint32_t dataPos = 0;

    for (i = 0; i != numDeflateBlocks; ++i)
    {
        uint32_t BFINAL, BTYPE, LEN, NLEN;
        uint8_t firstByte;
        uint32_t pos = out->size;

        BFINAL = (i == numDeflateBlocks - 1);
        BTYPE = 0;

        LEN = 65535;
        if (dataSize - dataPos < 65535u) LEN = (uint32_t)dataSize - dataPos;
        NLEN = 65535 - LEN;

        if (!ucVectorResize(out, out->size + LEN + 5)) return 83;

        firstByte = (uint8_t)(BFINAL + ((BTYPE & 1u) << 1u) + ((BTYPE & 2u) << 1u));
        out->data[pos + 0] = firstByte;
        out->data[pos + 1] = (uint8_t)(LEN & 255);
        out->data[pos + 2] = (uint8_t)(LEN >> 8u);
        out->data[pos + 3] = (uint8_t)(NLEN & 255);
        out->data[pos + 4] = (uint8_t)(NLEN >> 8u);
        memcpy(out->data + pos + 5, data + dataPos, LEN);
        dataPos += LEN;
    }

    return 0;
}

void writeLZ77data(PNGBitWriter* writer, const uiVector* lz77Encoded, const HuffmanTree* tree_ll, const HuffmanTree* tree_d)
{
    uint32_t i = 0;

    for (i = 0; i != lz77Encoded->size; ++i)
    {
        uint32_t val = lz77Encoded->data[i];
        writeBitsReversed(writer, tree_ll->codes[val], tree_ll->lengths[val]);

        if (val > 256)
        {
            uint32_t lengthIndex = val - FIRST_LENGTH_CODE_INDEX;
            uint32_t nLengthExtraBits = LENGTHEXTRA[lengthIndex];
            uint32_t lengthExtraBits = lz77Encoded->data[++i];

            uint32_t distanceCode = lz77Encoded->data[++i];

            uint32_t distanceIndex = distanceCode;
            uint32_t nDistanceExtraBits = DISTANCEEXTRA[distanceIndex];
            uint32_t distanceExtraBits = lz77Encoded->data[++i];

            writeBits(writer, lengthExtraBits, nLengthExtraBits);
            writeBitsReversed(writer, tree_d->codes[distanceCode], tree_d->lengths[distanceCode]);
            writeBits(writer, distanceExtraBits, nDistanceExtraBits);
        }
    }
}

uint32_t deflateDynamic(PNGBitWriter* writer, HASH* hash, const uint8_t* data, uint32_t dataPos, uint32_t dataEnd, const PNGCompressSettings* settings, uint32_t final)
{
    uint32_t error = 0;
    uiVector lz77Encoded;
    HuffmanTree tree_ll;
    HuffmanTree tree_d;
    HuffmanTree tree_cl;
    uint32_t* frequencies_ll = 0;
    uint32_t* frequencies_d = 0;
    uint32_t* frequencies_cl = 0;
    uint32_t* bitlen_lld = 0;
    uint32_t* bitlen_lld_e = 0;
    uint32_t dataSize = dataEnd - dataPos;

    uint32_t i;
    uint32_t BFINAL = final;
    uint32_t numCodes_ll, numCodes_d, numCodes_lld, numCodes_lld_e, numCodes_cl;
    uint32_t HLIT, HDIST, HCLEN;

    uiVectorInit(&lz77Encoded);
    huffmanTreeInit(&tree_ll);
    huffmanTreeInit(&tree_d);
    huffmanTreeInit(&tree_cl);

    frequencies_ll = (uint32_t*)calloc(286, sizeof(*frequencies_ll));
    frequencies_d = (uint32_t*)calloc(30, sizeof(*frequencies_d));
    frequencies_cl = (uint32_t*)calloc(NUM_CODE_LENGTH_CODES, sizeof(*frequencies_cl));

    if (!frequencies_ll || !frequencies_d || !frequencies_cl) error = 83;

    while (!error)
    {
        memset(frequencies_ll, 0, 286 * sizeof(*frequencies_ll));
        memset(frequencies_d, 0, 30 * sizeof(*frequencies_d));
        memset(frequencies_cl, 0, NUM_CODE_LENGTH_CODES * sizeof(*frequencies_cl));

        if (settings->useLZ77)
        {
            error = encodeLZ77(&lz77Encoded, hash, data, dataPos, dataEnd, settings->windowSize, settings->minMatch, settings->niceMatch, settings->lazyMatching);
            if (error) break;
        }
        else
        {
            if (!uiVectorResize(&lz77Encoded, dataSize)) ERROR_BREAK(83);
            for (i = dataPos; i < dataEnd; ++i) lz77Encoded.data[i - dataPos] = data[i];
        }

        for (i = 0; i != lz77Encoded.size; ++i)
        {
            uint32_t symbol = lz77Encoded.data[i];
            ++frequencies_ll[symbol];
            if (symbol > 256)
            {
                uint32_t dist = lz77Encoded.data[i + 2];
                ++frequencies_d[dist];
                i += 3;
            }
        }

        frequencies_ll[256] = 1;

        error = huffmanTreeMakeFromFrequencies(&tree_ll, frequencies_ll, 257, 286, 15);
        if (error) break;

        error = huffmanTreeMakeFromFrequencies(&tree_d, frequencies_d, 2, 30, 15);
        if (error) break;

        numCodes_ll = min(tree_ll.numCodes, 286);
        numCodes_d = min(tree_d.numCodes, 30);
        numCodes_lld = numCodes_ll + numCodes_d;
        bitlen_lld = (uint32_t*)calloc(numCodes_lld, sizeof(*bitlen_lld));
        bitlen_lld_e = (uint32_t*)calloc(numCodes_lld, sizeof(*bitlen_lld_e));

        if (!bitlen_lld || !bitlen_lld_e) ERROR_BREAK(83);
        numCodes_lld_e = 0;

        for (i = 0; i != numCodes_ll; ++i) bitlen_lld[i] = tree_ll.lengths[i];
        for (i = 0; i != numCodes_d; ++i) bitlen_lld[numCodes_ll + i] = tree_d.lengths[i];

        for (i = 0; i != numCodes_lld; ++i)
        {
            uint32_t j = 0;
            while (i + j + 1 < numCodes_lld && bitlen_lld[i + j + 1] == bitlen_lld[i]) ++j;

            if (bitlen_lld[i] == 0 && j >= 2)
            {
                ++j;
                if (j <= 10)
                {
                    bitlen_lld_e[numCodes_lld_e++] = 17;
                    bitlen_lld_e[numCodes_lld_e++] = j - 3;
                }
                else
                {
                    if (j > 138) j = 138;
                    bitlen_lld_e[numCodes_lld_e++] = 18;
                    bitlen_lld_e[numCodes_lld_e++] = j - 11;
                }
                i += (j - 1);
            }
            else if (j >= 3)
            {
                uint32_t k;
                uint32_t num = j / 6u, rest = j % 6u;
                bitlen_lld_e[numCodes_lld_e++] = bitlen_lld[i];
                for (k = 0; k < num; ++k)
                {
                    bitlen_lld_e[numCodes_lld_e++] = 16;
                    bitlen_lld_e[numCodes_lld_e++] = 6 - 3;
                }
                if (rest >= 3)
                {
                    bitlen_lld_e[numCodes_lld_e++] = 16;
                    bitlen_lld_e[numCodes_lld_e++] = rest - 3;
                }
                else j -= rest;
                i += j;
            }
            else
            {
                bitlen_lld_e[numCodes_lld_e++] = bitlen_lld[i];
            }
        }

        for (i = 0; i != numCodes_lld_e; ++i)
        {
            ++frequencies_cl[bitlen_lld_e[i]];
            if (bitlen_lld_e[i] >= 16) ++i;
        }

        error = huffmanTreeMakeFromFrequencies(&tree_cl, frequencies_cl, NUM_CODE_LENGTH_CODES, NUM_CODE_LENGTH_CODES, 7);
        if (error) break;

        numCodes_cl = NUM_CODE_LENGTH_CODES;
        while (numCodes_cl > 4u && tree_cl.lengths[CLCL_ORDER[numCodes_cl - 1u]] == 0)
        {
            numCodes_cl--;
        }

        writeBits(writer, BFINAL, 1);
        writeBits(writer, 0, 1);
        writeBits(writer, 1, 1);

        HLIT = (uint32_t)(numCodes_ll - 257);
        HDIST = (uint32_t)(numCodes_d - 1);
        HCLEN = (uint32_t)(numCodes_cl - 4);

        writeBits(writer, HLIT, 5);
        writeBits(writer, HDIST, 5);
        writeBits(writer, HCLEN, 4);

        
        for (i = 0; i != numCodes_cl; ++i) writeBits(writer, tree_cl.lengths[CLCL_ORDER[i]], 3);

        for (i = 0; i != numCodes_lld_e; ++i)
        {
            writeBitsReversed(writer, tree_cl.codes[bitlen_lld_e[i]], tree_cl.lengths[bitlen_lld_e[i]]);

            if (bitlen_lld_e[i] == 16) writeBits(writer, bitlen_lld_e[++i], 2);
            else if (bitlen_lld_e[i] == 17) writeBits(writer, bitlen_lld_e[++i], 3);
            else if (bitlen_lld_e[i] == 18) writeBits(writer, bitlen_lld_e[++i], 7);
        }

        writeLZ77data(writer, &lz77Encoded, &tree_ll, &tree_d);

        if (tree_ll.lengths[256] == 0) ERROR_BREAK(64);

        writeBitsReversed(writer, tree_ll.codes[256], tree_ll.lengths[256]);

        break;
    }

    uiVectorCleanup(&lz77Encoded);
    huffmanTreeCleanup(&tree_ll);
    huffmanTreeCleanup(&tree_d);
    huffmanTreeCleanup(&tree_cl);
    free(frequencies_ll);
    free(frequencies_d);
    free(frequencies_cl);
    free(bitlen_lld);
    free(bitlen_lld_e);

    return error;
}

uint32_t deflateFixed(PNGBitWriter* writer, HASH* hash, const uint8_t* data, uint32_t dataPos, uint32_t dataEnd, const PNGCompressSettings* settings, uint32_t final)
{
    HuffmanTree tree_ll;
    HuffmanTree tree_d;

    uint32_t BFINAL = final;
    uint32_t error = 0;
    uint32_t i;

    huffmanTreeInit(&tree_ll);
    huffmanTreeInit(&tree_d);

    error = generateFixedLitLenTree(&tree_ll);
    if (!error) error = generateFixedDistanceTree(&tree_d);

    if (!error)
    {
        writeBits(writer, BFINAL, 1);
        writeBits(writer, 1, 1);
        writeBits(writer, 0, 1);

        if (settings->useLZ77)
        {
            uiVector lz77Encoded;
            uiVectorInit(&lz77Encoded);
            error = encodeLZ77(&lz77Encoded, hash, data, dataPos, dataEnd, settings->windowSize, settings->minMatch, settings->niceMatch, settings->lazyMatching);
            if (!error) writeLZ77data(writer, &lz77Encoded, &tree_ll, &tree_d);
            uiVectorCleanup(&lz77Encoded);
        }
        else
        {
            for (i = dataPos; i < dataEnd; ++i)
            {
                writeBitsReversed(writer, tree_ll.codes[data[i]], tree_ll.lengths[data[i]]);
            }
        }

        if (!error) writeBitsReversed(writer, tree_ll.codes[256], tree_ll.lengths[256]);
    }

    huffmanTreeCleanup(&tree_ll);
    huffmanTreeCleanup(&tree_d);

    return error;
}

uint32_t PNGDeflatev(ucVector* out, const uint8_t* in, uint32_t insize, const PNGCompressSettings* settings)
{
    uint32_t error = 0;
    uint32_t i, blockSize, numDeflateBlocks;

    HASH hash;
    PNGBitWriter writer;

    PNGBitWriterInit(&writer, out);

    if (settings->btype > 2) return 61;
    else if (settings->btype == 0) return deflateNoCompression(out, in, insize);
    else if (settings->btype == 1) blockSize = insize;
    else
    {
        blockSize = insize / 8u + 8;
        if (blockSize < 65536) blockSize = 65536;
        if (blockSize > 262144) blockSize = 262144;
    }

    numDeflateBlocks = (insize + blockSize - 1) / blockSize;
    if (numDeflateBlocks == 0) numDeflateBlocks = 1;

    error = hashInit(&hash, settings->windowSize);

    if (!error)
    {
        for (i = 0; i != numDeflateBlocks && !error; ++i)
        {
            uint32_t final = (i == numDeflateBlocks - 1);
            uint32_t start = i * blockSize;
            uint32_t end = start + blockSize;

            if (end > insize) end = insize;

            if (settings->btype == 1) error = deflateFixed(&writer, &hash, in, start, end, settings, final);
            else if (settings->btype == 2) error = deflateDynamic(&writer, &hash, in, start, end, settings, final);
        }
    }

    hashCleanup(&hash);

    return error;
}

uint32_t PNGDeflate(uint8_t** out, uint32_t* outSize, const uint8_t* in, uint32_t insize, const PNGCompressSettings* settings)
{
    ucVector v = ucVectorInit(*out, *outSize);
    uint32_t error = PNGDeflatev(&v, in, insize, settings);
    *out = v.data;
    *outSize = v.size;
    return error;
}

uint32_t deflate(uint8_t** out, uint32_t* outSize, const uint8_t* in, uint32_t insize, const PNGCompressSettings* settings)
{
    if (settings->customDeflate)
    {
        uint32_t error = settings->customDeflate(out, outSize, in, insize, settings);
        return error ? 111 : 0;
    }
    else
    {
        return PNGDeflate(out, outSize, in, insize, settings);
    }
}

/*===========================*/
/*          adler32          */
/*===========================*/

uint32_t updateAdler32(uint32_t adler, const uint8_t* data, uint32_t len)
{
    uint32_t s1 = adler & 0xffffu;
    uint32_t s2 = (adler >> 16u) & 0xffffu;

    while (len > 0)
    {
        uint32_t amount = len > 5552u ? 5552u : len;
        len -= amount;

        while (amount > 0)
        {
            s1 += (*data++);
            s2 += s1;
            amount--;
        }

        s1 %= 65521u;
        s2 %= 65521u;
    }

    return (s2 << 16u) | s1;
}

uint32_t adler32(const uint8_t* data, uint32_t len)
{
    return updateAdler32(1L, data, len);
}

/*===========================*/
/*           zlib            */
/*===========================*/

uint32_t PNGZlibDecompressv(ucVector* out, const uint8_t* in, uint32_t insize, const PNGDecompressSettings* settings)
{
    uint32_t error = 0;
    uint32_t CM, CINFO, FDICT;

    if (insize < 2) return 53;

    if ((in[0] * 256 + in[1]) % 31 != 0)
    {
        return 24;
    }

    CM = in[0] & 15;
    CINFO = (in[0] >> 4) & 15;
    FDICT = (in[1] >> 5) & 1;

    if (CM != 8 || CINFO > 7)
    {
        return 25;
    }

    if (FDICT != 0)
    {
        return 26;
    }

    error = inflatev(out, in + 2, insize - 2, settings);
    if (error) return error;

    if (!settings->ignoreAdler32)
    {
        uint32_t ADLER32 = PNGRead32bitInt(&in[insize - 4]);
        uint32_t checksum = adler32(out->data, (uint32_t)(out->size));
        if (checksum != ADLER32) return 58;
    }

    return 0;
}

uint32_t PNGZlibDecompress(uint8_t** out, uint32_t* outSize, const uint8_t* in, uint32_t insize, const PNGDecompressSettings* settings)
{
    ucVector v = ucVectorInit(*out, *outSize);
    uint32_t error = PNGZlibDecompressv(&v, in, insize, settings);
    *out = v.data;
    *outSize = v.size;
    return error;
}

uint32_t zlibDecompress(uint8_t** out, uint32_t* outSize, uint32_t expectedSize, const uint8_t* in, uint32_t insize, const PNGDecompressSettings* settings)
{
    uint32_t error;

    if (settings->customZlib)
    {
        error = settings->customZlib(out, outSize, in, insize, settings);
        if (error)
        {
            error = 110;
            if (settings->maxOutputSize && *outSize > settings->maxOutputSize) error = 109;
        }
    }
    else
    {
        ucVector v = ucVectorInit(*out, *outSize);
        if (expectedSize)
        {
            ucVectorResize(&v, *outSize + expectedSize);
            v.size = *outSize;
        }
        error = PNGZlibDecompressv(&v, in, insize, settings);
        *out = v.data;
        *outSize = v.size;
    }

    return error;
}

uint32_t PNGZlibCompress(uint8_t** out, uint32_t* outSize, const uint8_t* in, uint32_t insize, const PNGCompressSettings* settings)
{
    uint32_t i;
    uint32_t error;
    uint8_t* deflateData = 0;
    uint32_t deflateSize = 0;

    error = deflate(&deflateData, &deflateSize, in, insize, settings);

    *out = NULL;
    *outSize = 0;

    if (!error)
    {
        *outSize = deflateSize + 6;
        *out = (uint8_t*)calloc(*outSize, 1);
        if (!*out) error = 83;
    }

    if (!error)
    {
        uint32_t ADLER32 = adler32(in, (uint32_t)insize);
        uint32_t CMF = 120;
        uint32_t FLEVEL = 0;
        uint32_t FDICT = 0;
        uint32_t CMFFLG = 256 * CMF + FDICT * 32 + FLEVEL * 64;
        uint32_t FCHECK = 31 - CMFFLG % 31;
        CMFFLG += FCHECK;

        (*out)[0] = (uint8_t)(CMFFLG >> 8);
        (*out)[1] = (uint8_t)(CMFFLG & 255);
        for (i = 0; i != deflateSize; ++i) (*out)[i + 2] = deflateData[i];
        PNGSet32bitInt(&(*out)[*outSize - 4], ADLER32);
    }

    free(deflateData);
    return error;
}

uint32_t zlibCompress(uint8_t** out, uint32_t* outSize, const uint8_t* in, uint32_t insize, const PNGCompressSettings* settings)
{
    if (settings->customZlib)
    {
        uint32_t error = settings->customZlib(out, outSize, in, insize, settings);
        return error ? 111 : 0;
    }
    else
    {
        return PNGZlibCompress(out, outSize, in, insize, settings);
    }
}

#define DEFAULT_WINDOWSIZE 2048

void PNGCompressSettingsInit(PNGCompressSettings* settings)
{
    settings->btype = 2;
    settings->useLZ77 = 1;
    settings->windowSize = DEFAULT_WINDOWSIZE;
    settings->minMatch = 3;
    settings->niceMatch = 128;
    settings->lazyMatching = 1;
    settings->customZlib = 0;
    settings->customDeflate = 0;
    settings->customContext = 0;
}

const PNGCompressSettings PNGDefaultCompressSettings = {2, 1, DEFAULT_WINDOWSIZE, 3, 128, 1, 0, 0, 0};

void PNGDecompressSettingsInit(PNGDecompressSettings* settings)
{
    settings->ignoreAdler32 = 0;
    settings->ignoreLen = 0;
    settings->maxOutputSize= 0;
    settings->customZlib = 0;
    settings->customInflate = 0;
    settings->customContext = 0;
}

const PNGDecompressSettings PNGDefaultDecompressSettings = {0, 0, 0, 0, 0, 0};

/*===========================*/
/*           crc32           */
/*===========================*/

// CRC polynomial: 0xedb88320 
uint32_t crc32Tables[256] = {
    0u,          1996959894u, 3993919788u, 2567524794u,  124634137u, 1886057615u, 3915621685u, 2657392035u,
    249268274u,  2044508324u, 3772115230u, 2547177864u,  162941995u, 2125561021u, 3887607047u, 2428444049u,
    498536548u,  1789927666u, 4089016648u, 2227061214u,  450548861u, 1843258603u, 4107580753u, 2211677639u,
    325883990u,  1684777152u, 4251122042u, 2321926636u,  335633487u, 1661365465u, 4195302755u, 2366115317u,
    997073096u,  1281953886u, 3579855332u, 2724688242u, 1006888145u, 1258607687u, 3524101629u, 2768942443u,
    901097722u,  1119000684u, 3686517206u, 2898065728u,  853044451u, 1172266101u, 3705015759u, 2882616665u,
    651767980u,  1373503546u, 3369554304u, 3218104598u,  565507253u, 1454621731u, 3485111705u, 3099436303u,
    671266974u,  1594198024u, 3322730930u, 2970347812u,  795835527u, 1483230225u, 3244367275u, 3060149565u,
    1994146192u,   31158534u, 2563907772u, 4023717930u, 1907459465u,  112637215u, 2680153253u, 3904427059u,
    2013776290u,  251722036u, 2517215374u, 3775830040u, 2137656763u,  141376813u, 2439277719u, 3865271297u,
    1802195444u,  476864866u, 2238001368u, 4066508878u, 1812370925u,  453092731u, 2181625025u, 4111451223u,
    1706088902u,  314042704u, 2344532202u, 4240017532u, 1658658271u,  366619977u, 2362670323u, 4224994405u,
    1303535960u,  984961486u, 2747007092u, 3569037538u, 1256170817u, 1037604311u, 2765210733u, 3554079995u,
    1131014506u,  879679996u, 2909243462u, 3663771856u, 1141124467u,  855842277u, 2852801631u, 3708648649u,
    1342533948u,  654459306u, 3188396048u, 3373015174u, 1466479909u,  544179635u, 3110523913u, 3462522015u,
    1591671054u,  702138776u, 2966460450u, 3352799412u, 1504918807u,  783551873u, 3082640443u, 3233442989u,
    3988292384u, 2596254646u,   62317068u, 1957810842u, 3939845945u, 2647816111u,   81470997u, 1943803523u,
    3814918930u, 2489596804u,  225274430u, 2053790376u, 3826175755u, 2466906013u,  167816743u, 2097651377u,
    4027552580u, 2265490386u,  503444072u, 1762050814u, 4150417245u, 2154129355u,  426522225u, 1852507879u,
    4275313526u, 2312317920u,  282753626u, 1742555852u, 4189708143u, 2394877945u,  397917763u, 1622183637u,
    3604390888u, 2714866558u,  953729732u, 1340076626u, 3518719985u, 2797360999u, 1068828381u, 1219638859u,
    3624741850u, 2936675148u,  906185462u, 1090812512u, 3747672003u, 2825379669u,  829329135u, 1181335161u,
    3412177804u, 3160834842u,  628085408u, 1382605366u, 3423369109u, 3138078467u,  570562233u, 1426400815u,
    3317316542u, 2998733608u,  733239954u, 1555261956u, 3268935591u, 3050360625u,  752459403u, 1541320221u,
    2607071920u, 3965973030u, 1969922972u,   40735498u, 2617837225u, 3943577151u, 1913087877u,   83908371u,
    2512341634u, 3803740692u, 2075208622u,  213261112u, 2463272603u, 3855990285u, 2094854071u,  198958881u,
    2262029012u, 4057260610u, 1759359992u,  534414190u, 2176718541u, 4139329115u, 1873836001u,  414664567u,
    2282248934u, 4279200368u, 1711684554u,  285281116u, 2405801727u, 4167216745u, 1634467795u,  376229701u,
    2685067896u, 3608007406u, 1308918612u,  956543938u, 2808555105u, 3495958263u, 1231636301u, 1047427035u,
    2932959818u, 3654703836u, 1088359270u,  936918000u, 2847714899u, 3736837829u, 1202900863u,  817233897u,
    3183342108u, 3401237130u, 1404277552u,  615818150u, 3134207493u, 3453421203u, 1423857449u,  601450431u,
    3009837614u, 3294710456u, 1567103746u,  711928724u, 3020668471u, 3272380065u, 1510334235u,  755167117u
};

uint32_t crc32(const uint8_t* buf, uint32_t len)
{
    uint32_t n;
    uint32_t c = 0xffffffffL;

    for (n = 0; n != len; n++)
    {
        c = crc32Tables[(c ^ buf[n]) & 0xff] ^ (c >> 8u);
    }

    return c ^ 0xffffffffL;
}

uint8_t readBitFromReversedStream(uint32_t* bitpointer, const uint8_t* bitstream)
{
    uint8_t result = (uint8_t)((bitstream[(*bitpointer) >> 3u] >> (7u - ((*bitpointer) & 7u))) & 1u);
    ++(*bitpointer);
    return result;
}

uint32_t readBitsFromReversedStream(uint32_t* bitpointer, const uint8_t* bitstream, uint32_t nbits)
{
    uint32_t i;
    uint32_t result = 0;
    for (i = 0 ; i < nbits; ++i)
    {
        result <<= 1u;
        result |= (uint32_t)readBitFromReversedStream(bitpointer, bitstream);
    }
    return result;
}

void setBitOfReversedStream(uint32_t* bitpointer, uint8_t* bitstream, uint8_t bit)
{
    if (bit == 0)   bitstream[(*bitpointer) >> 3u] &=  (uint8_t)(~(1u << (7u - ((*bitpointer) & 7u))));
    else            bitstream[(*bitpointer) >> 3u] |=  (1u << (7u - ((*bitpointer) & 7u)));
    ++(*bitpointer);
}

/*===========================*/
/*        PNG chunks         */
/*===========================*/

uint32_t PNGChunkLength(const uint8_t* chunk)
{
    return PNGRead32bitInt(&chunk[0]);
}

void PNGChunkType(char type[5], const uint8_t* chunk)
{
    uint32_t i;
    for (i = 0; i != 4; i++) type[i] = (char)chunk[4 + i];
    type[4] = 0;
}

uint8_t PNGChunkTypeEquals(const uint8_t* chunk, const char* type)
{
    if (strlen(type) != 4) return 0;
    return (chunk[4] == type[0] && chunk[5] == type[1] && chunk[6] == type[2] && chunk[7] == type[3]);
}

uint8_t PNGChunkAncillary(const uint8_t* chunk)
{
    return((chunk[4] & 32) != 0);
}

uint8_t PNGChunkPrivate(const uint8_t* chunk)
{
    return((chunk[6] & 32) != 0);
}

uint8_t PNGChunkSafeToCopy(const uint8_t* chunk)
{
    return((chunk[7] & 32) != 0);
}

uint8_t* PNGChunkData(uint8_t* chunk)
{
    return &chunk[8];
}

const uint8_t* PNGChunkDataConst(const uint8_t* chunk)
{
    return &chunk[8];
}

uint32_t PNGChunkCheckCRC(const uint8_t* chunk)
{
    uint32_t len = PNGChunkLength(chunk);
    uint32_t crc = PNGRead32bitInt(&chunk[len + 8]);
    uint32_t checksum = crc32(&chunk[4], len + 4);

    if (crc != checksum) return 1;
    return 0;
}

void PNGChunkGenerateCRC(uint8_t* chunk)
{
    uint32_t len = PNGChunkLength(chunk);
    uint32_t crc = crc32(&chunk[4], len + 4);
    PNGSet32bitInt(chunk + 8 + len, crc);
}

uint8_t* PNGChunkNext(uint8_t* chunk, uint8_t* end)
{
    if (chunk >= end || end - chunk < 12) return end;
    if (chunk[0] == 0x89 && chunk[1] == 0x50 && chunk[2] == 0x4e && chunk[3] == 0x47 && chunk[4] == 0x0d && chunk[5] == 0x0a && chunk[6] == 0x1a && chunk[7] == 0x0a)
    {
        return chunk + 8;
    }
    else
    {
        uint8_t* result;
        uint32_t totalChunkLength;
        if (PNGAddofl(PNGChunkLength(chunk), 12, &totalChunkLength)) return end;
        result = chunk + totalChunkLength;
        if (result < chunk) return end;
        return result;
    }
}

const uint8_t* PNGChunkNextConst(const uint8_t* chunk, const uint8_t* end)
{
    if (chunk >= end || end - chunk < 12) return end;
    if (chunk[0] == 0x89 && chunk[1] == 0x50 && chunk[2] == 0x4e && chunk[3] == 0x47 && chunk[4] == 0x0d && chunk[5] == 0x0a && chunk[6] == 0x1a && chunk[7] == 0x0a)
    {
        return chunk + 8;
    }
    else
    {
        const uint8_t* result;
        uint32_t totalChunkLength;
        if (PNGAddofl(PNGChunkLength(chunk), 12, &totalChunkLength)) return end;
        result = chunk + totalChunkLength;
        if (result < chunk) return end;
        return result;
    }
}

uint8_t* PNGChunkFind(uint8_t* chunk, uint8_t* end, const char type[5])
{
    for (;;)
    {
        if (chunk >= end || end - chunk < 12) return 0;
        if (PNGChunkTypeEquals(chunk, type)) return chunk;
        chunk = PNGChunkNext(chunk, end);
    }
}

const uint8_t* PNGChunkFindConst(const uint8_t* chunk, const uint8_t* end, const char type[5])
{
    for (;;)
    {
        if (chunk >= end || end - chunk < 12) return 0;
        if (PNGChunkTypeEquals(chunk, type)) return chunk;
        chunk = PNGChunkNextConst(chunk, end);
    }
}

uint32_t PNGChunkAppend(uint8_t** out, uint32_t* outSize, const uint8_t* chunk)
{
    uint32_t i;
    uint32_t totalChunkLength, newLength;
    uint8_t *chunkStart, *newBuffer;

    if (PNGAddofl(PNGChunkLength(chunk), 12, &totalChunkLength)) return 77;
    if (PNGAddofl(*outSize, totalChunkLength, &newLength)) return 77;

    newBuffer = (uint8_t*)realloc(*out, newLength);
    if (!newBuffer) return 83;

    (*out) = newBuffer;
    (*outSize) = newLength;
    chunkStart = &(*out)[newLength - totalChunkLength];

    for (i = 0; i != totalChunkLength; ++i) chunkStart[i] = chunk[i];

    return 0;
}

uint32_t PNGChunkInit(uint8_t** chunk, ucVector* out, uint32_t length, const char* type)
{
    uint32_t newLength = out->size;
    if (PNGAddofl(newLength, length, &newLength)) return 77;
    if (PNGAddofl(newLength, 12, &newLength)) return 77;
    if (!ucVectorResize(out, newLength)) return 83;
    *chunk = out->data + newLength - length - 12u;

    PNGSet32bitInt(*chunk, length);
    memcpy(*chunk + 4, type, 4);

    return 0;
}

uint32_t PNGChunkCreatev(ucVector* out, uint32_t length, const char* type, const uint8_t* data)
{
    uint8_t* chunk;
    CERROR_TRY_RETURN(PNGChunkInit(&chunk, out, length, type));
    memcpy(chunk + 8, data, length);
    PNGChunkGenerateCRC(chunk);
    return 0;
}

uint32_t checkColorValidity(PNGColorType colorType, uint32_t bd)
{
    switch (colorType)
    {
        case LCT_GREY:       if (!(bd == 1 || bd == 2 || bd == 4 || bd == 8 || bd == 16)) return 37; break;
        case LCT_RGB:        if (!(                                 bd == 8 || bd == 16)) return 37; break;
        case LCT_PALETTE:    if (!(bd == 1 || bd == 2 || bd == 4 || bd == 8            )) return 37; break;
        case LCT_GREY_ALPHA: if (!(                                 bd == 8 || bd == 16)) return 37; break;
        case LCT_RGBA:       if (!(                                 bd == 8 || bd == 16)) return 37; break;
        case LCT_MAX_OCTET_VALUE: return 31;
        default: return 31;
    }

    return 0;
}

uint32_t getNumColorChannels(PNGColorType colorType)
{
    switch(colorType)
    {
        case LCT_GREY: return 1;
        case LCT_RGB: return 3;
        case LCT_PALETTE: return 1;
        case LCT_GREY_ALPHA: return 2;
        case LCT_RGBA: return 4;
        case LCT_MAX_OCTET_VALUE: return 0;
        default: return 0;
    }
}

uint32_t PNGGetBppLCT(PNGColorType colorType, uint32_t bitDepth)
{
    return getNumColorChannels(colorType) * bitDepth;
}

void PNGColorModeInit(PNGColorMode* info)
{
    info->keyDefined = 0;
    info->keyR = info->keyG = info->keyB = 0;
    info->colorType = LCT_RGBA;
    info->bitDepth = 8;
    info->palette = 0;
    info->paletteSize = 0;
}

void PNGColorModeAllocPalette(PNGColorMode* info)
{
    uint32_t i;

    if (!info->palette) info->palette = (uint8_t*)calloc(1024, 1);
    if (!info->palette) return;

    for (i = 0; i != 256; ++i)
    {
        info->palette[i * 4 + 0] = 0;
        info->palette[i * 4 + 1] = 0;
        info->palette[i * 4 + 2] = 0;
        info->palette[i * 4 + 3] = 255;
    }
}

void PNGPaletteClear(PNGColorMode* info)
{
    if (info->palette) free(info->palette);
    info->palette = 0;
    info->paletteSize = 0;
}

void PNGColorModeCleanup(PNGColorMode* info)
{
    PNGPaletteClear(info);
}

uint32_t PNGColorModeCopy(PNGColorMode* dest, const PNGColorMode* source)
{
    PNGColorModeCleanup(dest);
    memcpy(dest, source, sizeof(PNGColorMode));

    if (source->palette)
    {
        dest->palette = (uint8_t*)calloc(1024, 1);
        if (!dest->palette && source->paletteSize) return 83;
        memcpy(dest->palette, source->palette, source->paletteSize * 4);
    }

    return 0;
}

PNGColorMode PNGColorModeMake(PNGColorType colorType, uint32_t bitDepth)
{
    PNGColorMode result;
    PNGColorModeInit(&result);
    result.colorType = colorType;
    result.bitDepth = bitDepth;
    return result;
}

int32_t PNGColorModeEqual(const PNGColorMode* a, const PNGColorMode* b)
{
    uint32_t i;

    if (a->colorType != b->colorType) return 0;
    if (a->bitDepth != b->bitDepth) return 0;
    if (a->keyDefined != b->keyDefined) return 0;

    if (a->keyDefined)
    {
        if (a->keyR != b->keyR) return 0;
        if (a->keyG != b->keyG) return 0;
        if (a->keyB != b->keyB) return 0;
    }

    if (a->paletteSize != b->paletteSize) return 0;

    for (i = 0; i != a->paletteSize * 4; ++i)
    {
        if (a->palette[i] != b->palette[i]) return 0;
    }

    return 1;
}

uint32_t PNGPaletteAdd(PNGColorMode* info, uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
    if (!info->palette)
    {
        PNGColorModeAllocPalette(info);
        if (!info->palette) return 83;
    }

    if (info->paletteSize >= 256)
    {
        return 108;
    }

    info->palette[4 * info->paletteSize + 0] = r;
    info->palette[4 * info->paletteSize + 1] = g;
    info->palette[4 * info->paletteSize + 2] = b;
    info->palette[4 * info->paletteSize + 3] = a;
    ++info->paletteSize;
    return 0;
}

uint32_t PNGGetBpp(const PNGColorMode* info)
{
    return PNGGetBppLCT(info->colorType, info->bitDepth);
}

uint32_t PNGGetChannels(const PNGColorMode* info)
{
    return getNumColorChannels(info->colorType);
}

uint32_t PNGIsGreyscaleType(const PNGColorMode* info)
{
    return info->colorType == LCT_GREY || info->colorType == LCT_GREY_ALPHA;
}

uint32_t PNGIsAlphaType(const PNGColorMode* info)
{
    return (info->colorType & 4) != 0;
}

uint32_t PNGIsPaletteType(const PNGColorMode* info)
{
    return info->colorType == LCT_PALETTE;
}

uint32_t PNGHasPaletteAlpha(const PNGColorMode* info)
{
    uint32_t i;
    for (i = 0; i != info->paletteSize; ++i)
    {
        if (info->palette[i * 4 + 3] < 255) return 1;
    }
    return 0;
}

uint32_t PNGCanHaveAlpha(const PNGColorMode* info)
{
    return info->keyDefined || PNGIsAlphaType(info) || PNGHasPaletteAlpha(info);
}

uint32_t PNGGetRawSizeLCT(uint32_t w, uint32_t h, PNGColorType colorType, uint32_t bitDepth)
{
    uint32_t bpp = PNGGetBppLCT(colorType, bitDepth);
    uint32_t n = w * h;
    return ((n / 8u) * bpp) + ((n & 7u) * bpp + 7u) / 8u;
}

uint32_t PNGGetRawSize(uint32_t w, uint32_t h, const PNGColorMode* color)
{
    return PNGGetRawSizeLCT(w, h, color->colorType, color->bitDepth);
}

uint32_t PNGGetRawSizeIDAT(uint32_t w, uint32_t h, uint32_t bpp)
{
    uint32_t line = ((w / 8u) * bpp) + 1u + ((w & 7u) * bpp + 7u) / 8u;
    return h * line;
}

int32_t PNGPixelOverflow(uint32_t w, uint32_t h, const PNGColorMode* pngcolor, const PNGColorMode* rawcolor)
{
    uint32_t bpp = max(PNGGetBpp(pngcolor), PNGGetBpp(rawcolor));
    uint32_t numPixels, total;
    uint32_t line;

    if (PNGMulofl(w, h, &numPixels)) return 1;
    if (PNGMulofl(numPixels, 8, &total)) return 1;

    if (PNGMulofl(w / 8u, bpp, &line)) return 1;
    if (PNGAddofl(line, ((w & 7u) * bpp + 7u) / 8u, &line)) return 1;

    if (PNGAddofl(line, 5, &line)) return 1;
    if (PNGMulofl(line, h, &total)) return 1;

    return 0;
}

void PNGUnknownChunksInit(PNGInfo* info)
{
    uint32_t i;
    for (i = 0; i != 3; i++) info->unknownChunksData[i] = 0;
    for (i = 0; i != 3; i++) info->unknownChunksSize[i] = 0;
}

void PNGUnknownChunksCleanup(PNGInfo* info)
{
    uint32_t i;
    for (i = 0; i != 3; i++) free(info->unknownChunksData[i]);
}

uint32_t PNGUnknownChunksCopy(PNGInfo* dest, const PNGInfo* src)
{
    uint32_t i;

    PNGUnknownChunksCleanup(dest);
    for (i = 0; i != 3; i++)
    {
        uint32_t j;
        dest->unknownChunksSize[i] = src->unknownChunksSize[i];
        dest->unknownChunksData[i] = (uint8_t*)calloc(src->unknownChunksSize[i], 1);
        if (!dest->unknownChunksData[i] && dest->unknownChunksSize[i]) return 83;
        for (j = 0; j != src->unknownChunksSize[i]; j++)
        {
            dest->unknownChunksData[i][j] = src->unknownChunksData[i][j];
        }
    }

    return 0;
}

void PNGTextInit(PNGInfo* info)
{
    info->textNum = 0;
    info->textKeys = NULL;
    info->textStrings = NULL;
}

void PNGTextCleanup(PNGInfo* info)
{
    uint32_t i;
    for (i = 0; i != info->textNum; i++)
    {
        stringCleanup(&info->textKeys[i]);
        stringCleanup(&info->textStrings[i]);
    }

    free(info->textKeys);
    free(info->textStrings);
}

uint32_t PNGAddTextSized(PNGInfo* info, const char* key, const char* str, uint32_t size)
{
    char** newKeys = (char**)(realloc(info->textKeys, sizeof(char*) * (info->textNum + 1)));
    char** newStrings = (char**)(realloc(info->textStrings, sizeof(char*) * (info->textNum + 1)));

    if (!newKeys || !newStrings)
    {
        return 83;
    }

    ++info->textNum;
    info->textKeys = newKeys;
    info->textStrings = newStrings;

    info->textKeys[info->textNum - 1] = allocString(key);
    info->textStrings[info->textNum - 1] = allocStringSized(str, size);
    if (!info->textKeys[info->textNum - 1] || !info->textStrings[info->textNum - 1]) return 83;

    return 0;
}

uint32_t PNGAddText(PNGInfo* info, const char* key, const char* str)
{
    return PNGAddTextSized(info, key, str, strlen(str));
}

uint32_t PNGTextCopy(PNGInfo* dest, const PNGInfo* source)
{
    uint32_t i = 0;
    dest->textKeys = NULL;
    dest->textStrings = NULL;
    dest->textNum = 0;
    for (i = 0; i != source->textNum; ++i)
    {
        CERROR_TRY_RETURN(PNGAddText(dest, source->textKeys[i], source->textStrings[i]));
    }
    return 0;
}

void PNGClearText(PNGInfo* info)
{
    PNGTextCleanup(info);
}

void PNGITextInit(PNGInfo* info)
{
    info->itextNum = 0;
    info->itextKeys = NULL;
    info->itextLangTags = NULL;
    info->itextTransKeys = NULL;
    info->itextStrings = NULL;
}

void PNGITextCleanup(PNGInfo* info)
{
    uint32_t i;
    for (i = 0; i != info->itextNum; ++i)
    {
        stringCleanup(&info->itextKeys[i]);
        stringCleanup(&info->itextLangTags[i]);
        stringCleanup(&info->itextTransKeys[i]);
        stringCleanup(&info->itextStrings[i]);
    }
    free(info->itextKeys);
    free(info->itextLangTags);
    free(info->itextTransKeys);
    free(info->itextStrings);
}

uint32_t PNGAddITextSized(PNGInfo* info, const char* key, const char* langtag, const char* transkey, const char* str, uint32_t size)
{
    char** newKeys = (char**)(realloc(info->itextKeys, sizeof(char*) * (info->itextNum + 1)));
    char** newLangTags = (char**)(realloc(info->itextLangTags, sizeof(char*) * (info->itextNum + 1)));
    char** newTransKeys = (char**)(realloc(info->itextTransKeys, sizeof(char*) * (info->itextNum + 1)));
    char** newStrings = (char**)(realloc(info->itextStrings, sizeof(char*) * (info->itextNum + 1)));

    if (newKeys) info->itextKeys = newKeys;
    if (newLangTags) info->itextLangTags = newLangTags;
    if (newTransKeys) info->itextTransKeys = newTransKeys;
    if (newStrings) info->itextStrings = newStrings;

    if (!newKeys || !newLangTags || !newTransKeys || !newStrings) return 83;

    ++info->itextNum;

    info->itextKeys[info->itextNum - 1] = allocString(key);
    info->itextLangTags[info->itextNum - 1] = allocString(langtag);
    info->itextTransKeys[info->itextNum - 1] = allocString(transkey);
    info->itextStrings[info->itextNum - 1] = allocStringSized(str, size);

    return 0;
}

uint32_t PNGAddIText(PNGInfo* info, const char* key, const char* langtag, const char* transkey, const char* str)
{
    return PNGAddITextSized(info, key, langtag, transkey, str, strlen(str));
}

uint32_t PNGITextCopy(PNGInfo* dest, const PNGInfo* source)
{
    uint32_t i = 0;
    dest->itextKeys = NULL;
    dest->itextLangTags = NULL;
    dest->itextTransKeys = NULL;
    dest->itextStrings = NULL;
    dest->itextNum = 0;
    for (i = 0; i != source->itextNum; ++i)
    {
        CERROR_TRY_RETURN(PNGAddIText(dest, source->itextKeys[i], source->itextLangTags[i], source->itextTransKeys[i], source->itextStrings[i]));
    }
    return 0;
}

void PNGClearIText(PNGInfo* info)
{
    PNGITextCleanup(info);
}

void PNGClearICC(PNGInfo* info)
{
    stringCleanup(&info->iccpName);
    free(info->iccpProfile);
    info->iccpProfile = NULL;
    info->iccpProfileSize = 0;
    info->iccpDefined = 0;
}

uint32_t PNGAssignICC(PNGInfo* info, const char* name, const uint8_t* profile, uint32_t profileSize)
{
    if (profileSize == 0) return 100;

    info->iccpName = allocString(name);
    info->iccpProfile = (uint8_t*)calloc(profileSize, 1);

    if (!info->iccpName || !info->iccpProfile) return 83;

    memcpy(info->iccpProfile, profile, profileSize);
    info->iccpProfileSize = profileSize;

    return 0;
}

uint32_t PNGSetICC(PNGInfo* info, const char* name, const uint8_t* profile, uint32_t profileSize)
{
    if (info->iccpName) PNGClearICC(info);
    info->iccpDefined = 1;
    return PNGAssignICC(info, name, profile, profileSize);
}

void PNGInfoInit(PNGInfo* info)
{
    PNGColorModeInit(&info->colorMode);
    info->interlaceMethod = 0;
    info->compressionMethod = 0;
    info->filterMethod = 0;
    info->backgroundDefined = 0;
    info->backgroundR = info->backgroundG = info->backgroundB = 0;

    PNGTextInit(info);
    PNGITextInit(info);

    info->timeDefined = 0;
    info->physDefined = 0;

    info->gamaDefined = 0;
    info->chrmDefined = 0;
    info->srgbDefined = 0;
    info->iccpDefined = 0;
    info->iccpName = NULL;
    info->iccpProfile = NULL;
    PNGUnknownChunksInit(info);
}

void PNGInfoCleanup(PNGInfo* info)
{
    PNGColorModeCleanup(&info->colorMode);
    PNGTextCleanup(info);
    PNGITextCleanup(info);
    PNGClearICC(info);
    PNGUnknownChunksCleanup(info);
}

uint32_t PNGInfoCopy(PNGInfo* dest, const PNGInfo* source)
{
    PNGInfoCleanup(dest);
    memcpy(dest, source, sizeof(PNGInfo));
    PNGColorModeInit(&dest->colorMode);
    CERROR_TRY_RETURN(PNGColorModeCopy(&dest->colorMode, &source->colorMode));
    CERROR_TRY_RETURN(PNGTextCopy(dest, source));
    CERROR_TRY_RETURN(PNGITextCopy(dest, source));

    if (source->iccpDefined)
    {
        CERROR_TRY_RETURN(PNGAssignICC(dest, source->iccpName, source->iccpProfile, source->iccpProfileSize));
    }

    PNGUnknownChunksInit(dest);
    CERROR_TRY_RETURN(PNGUnknownChunksCopy(dest, source));
    return 0;
}

void addColorBits(uint8_t* out, uint32_t index, uint32_t bits, uint32_t in)
{
    uint32_t m = bits == 1 ? 7 : bits == 2 ? 3 : 1;
    uint32_t p = index & m;
    in &= (1u << bits) - 1u;
    in = in << (bits * (m - p));

    if (p == 0) out[index * bits / 8u] = in;
    else out[index * bits / 8u] |= in;
}

typedef struct ColorTree ColorTree;

struct ColorTree
{
    ColorTree* children[16];
    int32_t index;
};

void colorTreeInit(ColorTree* tree)
{
    memset(tree->children, 0, 16 * sizeof(*tree->children));
    tree->index = -1;
}

void colorTreeCleanup(ColorTree* tree)
{
    uint32_t i;
    for (i = 0; i != 16; i++)
    {
        if (tree->children[i])
        {
            colorTreeCleanup(tree->children[i]);
            free(tree->children[i]);
        }
    }
}

int32_t colorTreeGet(ColorTree* tree, uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
    uint32_t bit = 0;

    for (bit = 0; bit < 8; bit++)
    {
        uint32_t i = 8 * ((r >> bit) & 1) + 4 * ((g >> bit) & 1) + 2 * ((b >> bit) & 1) + 1 * ((a >> bit) & 1);
        if (!tree->children[i]) return -1;
        else tree = tree->children[i];
    }

    return tree ? tree->index : -1;
}

int32_t colorTreeHas(ColorTree* tree, uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
    return colorTreeGet(tree, r, g, b, a) >= 0;
}

uint32_t colorTreeAdd(ColorTree* tree, uint8_t r, uint8_t g, uint8_t b, uint8_t a, uint32_t index)
{
    uint32_t bit;

    for (bit = 0; bit < 8; bit++)
    {
        uint32_t i = 8 * ((r >> bit) & 1) + 4 * ((g >> bit) & 1) + 2 * ((b >> bit) & 1) + 1 * ((a >> bit) & 1);

        if (!tree->children[i])
        {
            tree->children[i] = (ColorTree*)calloc(1, sizeof(ColorTree));
            if (!tree->children[i]) return 83;
            colorTreeInit(tree->children[i]);
        }

        tree = tree->children[i];
    }

    tree->index = (int32_t)index;
    return 0;
}

uint32_t RGBA8ToPixel(uint8_t* out, uint32_t i, const PNGColorMode* mode, ColorTree* tree, uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
    if (mode->colorType == LCT_GREY)
    {
        uint8_t grey = r;
        if (mode->bitDepth == 8) out[i] = grey;
        else if (mode->bitDepth == 16) out[i * 2 + 0] = out[i * 2 + 1] = grey;
        else
        {
            grey = (grey >> (8u - mode->bitDepth)) & ((1u << mode->bitDepth) - 1u);
            addColorBits(out, i, mode->bitDepth, grey);
        }
    }
    else if (mode->colorType == LCT_RGB)
    {
        if (mode->bitDepth == 8)
        {
            out[i * 3 + 0] = r;
            out[i * 3 + 1] = g;
            out[i * 3 + 2] = b;
        }
        else
        {
            out[i * 6 + 0] = out[i * 6 + 1] = r;
            out[i * 6 + 2] = out[i * 6 + 3] = g;
            out[i * 6 + 4] = out[i * 6 + 5] = b;
        }
    }
    else if (mode->colorType == LCT_PALETTE)
    {
        int32_t index = colorTreeGet(tree, r, g, b, a);
        if (index < 0) return 82;

        if (mode->bitDepth == 8) out[i] = index;
        else addColorBits(out, i, mode->bitDepth, (uint32_t)index);
    }
    else if (mode->colorType == LCT_GREY_ALPHA)
    {
        uint8_t grey = r;
        if (mode->bitDepth == 8)
        {
            out[i * 2 + 0] = grey;
            out[i * 2 + 1] = a;
        }
        else if (mode->bitDepth == 16)
        {
            out[i * 4 + 0] = out[i * 4 + 1] = grey;
            out[i * 4 + 2] = out[i * 4 + 3] = a;
        }
    }
    else if (mode->colorType == LCT_RGBA)
    {
        if (mode->bitDepth == 8)
        {
            out[i * 4 + 0] = r;
            out[i * 4 + 1] = g;
            out[i * 4 + 2] = b;
            out[i * 4 + 3] = a;
        }
        else
        {
            out[i * 8 + 0] = out[i * 8 + 1] = r;
            out[i * 8 + 2] = out[i * 8 + 3] = g;
            out[i * 8 + 4] = out[i * 8 + 5] = b;
            out[i * 8 + 6] = out[i * 8 + 7] = a;
        }
    }

    return 0;
}

void RGBA16ToPixel(uint8_t* out, int32_t i, const PNGColorMode* mode, uint16_t r, uint16_t g, uint16_t b, uint16_t a)
{
    if (mode->colorType == LCT_GREY)
    {
        uint16_t grey = r;
        out[i * 2 + 0] = (grey >> 8) & 255;
        out[i * 2 + 1] = grey & 255;
    }
    else if (mode->colorType == LCT_RGB)
    {
        out[i * 6 + 0] = (r >> 8) & 255;
        out[i * 6 + 1] = r & 255;
        out[i * 6 + 2] = (g >> 8) & 255;
        out[i * 6 + 3] = g & 255;
        out[i * 6 + 4] = (b >> 8) & 255;
        out[i * 6 + 5] = b & 255;
    }
    else if (mode->colorType == LCT_GREY_ALPHA)
    {
        uint16_t grey = r;
        out[i * 4 + 0] = (grey >> 8) & 255;
        out[i * 4 + 1] = grey & 255;
        out[i * 4 + 2] = (a >> 8) & 255;
        out[i * 4 + 3] = a & 255;
    }
    else if (mode->colorType == LCT_RGBA)
    {
        out[i * 8 + 0] = (r >> 8) & 255;
        out[i * 8 + 1] = r & 255;
        out[i * 8 + 2] = (g >> 8) & 255;
        out[i * 8 + 3] = g & 255;
        out[i * 8 + 4] = (b >> 8) & 255;
        out[i * 8 + 5] = b & 255;
        out[i * 8 + 6] = (a >> 8) & 255;
        out[i * 8 + 7] = a & 255;
    }
}

void getPixelColorRGBA8(uint8_t* r, uint8_t* g, uint8_t* b, uint8_t* a, const uint8_t* in, uint32_t i, const PNGColorMode* mode)
{
    if (mode->colorType == LCT_GREY)
    {
        if (mode->bitDepth == 8)
        {
            *r = *g = *b = in[i];
            if (mode->keyDefined && *r == mode->keyR) *a = 0;
            else *a = 255;
        }
        else if (mode->bitDepth == 16)
        {
            *r = *g = *b = in[i * 2 + 0];
            if (mode->keyDefined && 256U * in[i * 2 + 0] + in[i * 2 + 1] == mode->keyR) *a = 0;
            else *a = 255;
        }
        else
        {
            uint32_t highest = ((1U << mode->bitDepth) - 1U);
            uint32_t j = i * mode->bitDepth;
            uint32_t value = readBitsFromReversedStream(&j, in, mode->bitDepth);
            *r = *g = *b = (value * 255) / highest;

            if (mode->keyDefined && value == mode->keyR) *a = 0;
            else *a = 255;
        }
    }
    else if (mode->colorType == LCT_RGB)
    {
        if (mode->bitDepth == 8)
        {
            *r = in[i * 3 + 0]; *g = in[i * 3 + 1]; *b = in[i * 3 + 2];
            if (mode->keyDefined && *r == mode->keyR && *g == mode->keyG && *b == mode->keyB) *a = 0;
            else *a = 255;
        }
        else
        {
            *r = in[i * 6 + 0];
            *g = in[i * 6 + 2];
            *b = in[i * 6 + 4];
            if (mode->keyDefined && 256U * in[i * 6 + 0] + in[i * 6 + 1] == mode->keyR && 256U * in[i * 6 + 2] + in[i * 6 + 3] == mode->keyG && 256U * in[i * 6 + 4] + in[i * 6 + 5] == mode->keyB) *a = 0;
            else *a = 255;
        }
    }
    else if (mode->colorType == LCT_PALETTE)
    {
        uint32_t index;
        if (mode->bitDepth == 8) index = in[i];
        else
        {
            uint32_t j = i * mode->bitDepth;
            index = readBitsFromReversedStream(&j, in, mode->bitDepth);
        }

        *r = mode->palette[index * 4 + 0];
        *g = mode->palette[index * 4 + 1];
        *b = mode->palette[index * 4 + 2];
        *a = mode->palette[index * 4 + 3];
    }
    else if (mode->colorType == LCT_GREY_ALPHA)
    {
        if (mode->bitDepth == 8)
        {
            *r = *g = *b = in[i * 2 + 0];
            *a = in[i * 2 + 1];
        }
        else
        {
            *r = *g = *b = in[i * 4 + 0];
            *a = in[i * 4 + 2];
        }
    }
    else if (mode->colorType == LCT_RGBA)
    {
        if (mode->bitDepth == 8)
        {
            *r = in[i * 4 + 0];
            *g = in[i * 4 + 1];
            *b = in[i * 4 + 2];
            *a = in[i * 4 + 3];
        }
        else
        {
            *r = in[i * 8 + 0];
            *g = in[i * 8 + 2];
            *b = in[i * 8 + 4];
            *a = in[i * 8 + 6];
        }
    }
}

void getPixelColorsRGBA8(uint8_t* buffer, uint32_t numPixels, const uint8_t* in, const PNGColorMode* mode)
{
    uint32_t i;
    uint32_t numChannels = 4;
    
    if (mode->colorType == LCT_GREY)
    {
        if (mode->bitDepth == 8)
        {
            for (i = 0; i != numPixels; ++i, buffer += numChannels)
            {
                buffer[0] = buffer[1] = buffer[2] = in[i];
                buffer[3] = 255;
            }
            
            if (mode->keyDefined)
            {
                buffer -= numPixels * numChannels;
                for (i = 0; i != numPixels; ++i, buffer += numChannels)
                {
                    if (buffer[0] == mode->keyR) buffer[3] = 0;
                }
            }
        }
        else if (mode->bitDepth == 16)
        {
            for (i = 0; i != numPixels; ++i, buffer += numChannels)
            {
                buffer[0] = buffer[1] = buffer[2] = in[i * 2];
                buffer[3] = mode->keyDefined && 256U * in[i * 2 + 0] + in[i * 2 + 1] == mode->keyR ? 0 : 255;
            }
        }
        else
        {
            uint32_t j = 0;
            uint32_t highest = ((1U << mode->bitDepth) - 1U);
            for (i = 0; i != numPixels; ++i, buffer += numChannels)
            {
                uint32_t value = readBitsFromReversedStream(&j, in, mode->bitDepth);
                buffer[0] = buffer[1] = buffer[2] = (value * 255) / highest;
                buffer[3] = mode->keyDefined && value == mode->keyR ? 0 : 255;
            }
        }
    }
    else if (mode->colorType == LCT_RGB)
    {
        if (mode->bitDepth == 8)
        {
            for (i = 0; i != numPixels; ++i, buffer += numChannels)
            {
                memcpy(buffer, &in[i * 3], 3);
                buffer[3] = 255;
            }

            if (mode->keyDefined)
            {
                buffer -= numPixels * numChannels;
                for (i = 0; i != numPixels; ++i, buffer += numChannels)
                {
                    if (buffer[0] == mode->keyR && buffer[1]== mode->keyG && buffer[2] == mode->keyB) buffer[3] = 0;
                }
            }
        }
        else
        {
            for (i = 0; i != numPixels; ++i, buffer += numChannels)
            {
                buffer[0] = in[i * 6 + 0];
                buffer[1] = in[i * 6 + 2];
                buffer[2] = in[i * 6 + 4];
                buffer[3] = mode->keyDefined
                && 256U * in[i * 6 + 0] + in[i * 6 + 1] == mode->keyR
                && 256U * in[i * 6 + 2] + in[i * 6 + 3] == mode->keyG
                && 256U * in[i * 6 + 4] + in[i * 6 + 5] == mode->keyB ? 0 : 255;
            }
        }
    }
    else if (mode->colorType == LCT_PALETTE)
    {
        if (mode->bitDepth == 8)
        {
            for (i = 0; i != numPixels; ++i, buffer += numChannels)
            {
                uint32_t index = in[i];
                memcpy(buffer, &mode->palette[index * 4], 4);
            }
        }
        else
        {
            uint32_t j = 0;
            for (i = 0; i != numPixels; ++i, buffer += numChannels)
            {
                uint32_t index = readBitsFromReversedStream(&j, in, mode->bitDepth);
                memcpy(buffer, &mode->palette[index * 4], 4);
            }
        }
    }
    else if (mode->colorType == LCT_GREY_ALPHA)
    {
        if (mode->bitDepth == 8)
        {
            for (i = 0; i != numPixels; ++i, buffer += numChannels)
            {
                buffer[0] = buffer[1] = buffer[2] = in[i * 2 + 0];
                buffer[3] = in[i * 2 + 1];
            }
        }
        else
        {
            for (i = 0; i != numPixels; ++i, buffer += numChannels)
            {
                buffer[0] = buffer[1] = buffer[2] = in[i * 4 + 0];
                buffer[3] = in[i * 4 + 2];
            }
        }
    }
    else if (mode->colorType == LCT_RGBA)
    {
        if (mode->bitDepth == 8)
        {
            memcpy(buffer, in, numPixels * 4);
        }
        else
        {
            for (i = 0; i != numPixels; ++i, buffer += numChannels)
            {
                buffer[0] = in[i * 8 + 0];
                buffer[1] = in[i * 8 + 2];
                buffer[2] = in[i * 8 + 4];
                buffer[3] = in[i * 8 + 6];
            }
        }
    }
}

void getPixelColorsRGB8(uint8_t* buffer, uint32_t numPixels, const uint8_t* in, const PNGColorMode* mode)
{
    uint32_t i;
    const uint32_t numChannels = 3;
    
    if (mode->colorType == LCT_GREY)
    {
        if (mode->bitDepth == 8)
        {
            for (i = 0; i != numPixels; ++i, buffer += numChannels)
            {
                buffer[0] = buffer[1] = buffer[2] = in[i];
            }
        }
        else if (mode->bitDepth == 16)
        {
            for (i = 0; i != numPixels; ++i, buffer += numChannels)
            {
                buffer[0] = buffer[1] = buffer[2] = in[i * 2];
            }
        }
        else
        {
            uint32_t j = 0;
            uint32_t highest = ((1U << mode->bitDepth) - 1U);
            for (i = 0; i != numPixels; ++i, buffer += numChannels)
            {
                uint32_t value = readBitsFromReversedStream(&j, in, mode->bitDepth);
                buffer[0] = buffer[1] = buffer[2] = (value * 255) / highest;
            }
        }
    }
    else if (mode->colorType == LCT_RGB)
    {
        if (mode->bitDepth == 8)
        {
            memcpy(buffer, in, numPixels * 3);
        }
        else
        {
            for (i = 0; i != numPixels; ++i, buffer += numChannels)
            {
                buffer[0] = in[i * 6 + 0];
                buffer[1] = in[i * 6 + 2];
                buffer[2] = in[i * 6 + 4];
            }
        }
    }
    else if (mode->colorType == LCT_PALETTE)
    {
        if (mode->bitDepth == 8)
        {
            for (i = 0; i != numPixels; ++i, buffer += numChannels)
            {
                uint32_t index = in[i];
                memcpy(buffer, &mode->palette[index * 4], 3);
            }
        }
        else
        {
            uint32_t j = 0;
            for (i = 0; i != numPixels; ++i, buffer += numChannels)
            {
                uint32_t index = readBitsFromReversedStream(&j, in, mode->bitDepth);
                memcpy(buffer, &mode->palette[index * 4], 3);
            }
        }
    }
    else if (mode->colorType == LCT_GREY_ALPHA)
    {
        if (mode->bitDepth == 8)
        {
            for (i = 0; i != numPixels; ++i, buffer += numChannels)
            {
                buffer[0] = buffer[1] = buffer[2] = in[i * 2 + 0];
            }
        }
        else
        {
            for (i = 0; i != numPixels; ++i, buffer += numChannels)
            {
                buffer[0] = buffer[1] = buffer[2] = in[i * 4 + 0];
            }
        }
    }
    else if (mode->colorType == LCT_RGBA)
    {
        if (mode->bitDepth == 8)
        {
            for (i = 0; i != numPixels; ++i, buffer += numChannels)
            {
                memcpy(buffer, &in[i * 4], 3);
            }
        }
        else
        {
            for (i = 0; i != numPixels; ++i, buffer += numChannels)
            {
                buffer[0] = in[i * 8 + 0];
                buffer[1] = in[i * 8 + 2];
                buffer[2] = in[i * 8 + 4];
            }
        }
    }
}

void getPixelColorRGBA16(uint16_t* r, uint16_t* g, uint16_t* b, uint16_t* a, const uint8_t* in, uint32_t i, const PNGColorMode* mode)
{
    if (mode->colorType == LCT_GREY)
    {
        *r = *g = *b = 256 * in[i * 2 + 0] + in[i * 2 + 1];
        if (mode->keyDefined && 256U * in[i * 2 + 0] + in[i * 2 + 1] == mode->keyR) *a = 0;
        else *a = 65535;
    }
    else if (mode->colorType == LCT_RGB)
    {
        *r = 256u * in[i * 6 + 0] + in[i * 6 + 1];
        *g = 256u * in[i * 6 + 2] + in[i * 6 + 3];
        *b = 256u * in[i * 6 + 4] + in[i * 6 + 5];
        if (mode->keyDefined && 256u * in[i * 6 + 0] + in[i * 6 + 1] == mode->keyR && 256u * in[i * 6 + 2] + in[i * 6 + 3] == mode->keyG && 256u * in[i * 6 + 4] + in[i * 6 + 5] == mode->keyB) *a = 0;
        else *a = 65535;
    }
    else if (mode->colorType == LCT_GREY_ALPHA)
    {
        *r = *g = *b = 256u * in[i * 4 + 0] + in[i * 4 + 1];
        *a = 256u * in[i * 4 + 2] + in[i * 4 + 3];
    }
    else if (mode->colorType == LCT_RGBA)
    {
        *r = 256u * in[i * 8 + 0] + in[i * 8 + 1];
        *g = 256u * in[i * 8 + 2] + in[i * 8 + 3];
        *b = 256u * in[i * 8 + 4] + in[i * 8 + 5];
        *a = 256u * in[i * 8 + 6] + in[i * 8 + 7];
    }
}

uint32_t PNGConvert(uint8_t* out, const uint8_t* in, const PNGColorMode* modeOut, const PNGColorMode* modeIn, uint32_t w, uint32_t h)
{
    uint32_t i, error = 0;
    ColorTree tree;
    uint32_t numPixels = w * h;

    if (modeIn->colorType == LCT_PALETTE && !modeIn->palette)
    {
        return 107;
    }

    if (PNGColorModeEqual(modeOut, modeIn))
    {
        uint32_t numBytes = PNGGetRawSize(w, h, modeIn);
        memcpy(out, in, numBytes);
        return 0;
    }

    if (modeOut->colorType == LCT_PALETTE)
    {
        uint32_t paletteSize = modeOut->paletteSize;
        const uint8_t* palette = modeOut->palette;
        uint32_t palSize = (uint32_t)1u << modeOut->bitDepth;

        if (paletteSize == 0)
        {
            paletteSize = modeIn->paletteSize;
            palette = modeIn->palette;

            if (modeIn->colorType == LCT_PALETTE && modeIn->bitDepth == modeOut->bitDepth)
            {
                uint32_t numBytes = PNGGetRawSize(w, h, modeIn);
                memcpy(out, in, numBytes);
                return 0;
            }
        }

        if (paletteSize < palSize) palSize = paletteSize;
        colorTreeInit(&tree);
        for (i = 0; i != palSize; ++i)
        {
            const uint8_t* p = &palette[i * 4];
            error = colorTreeAdd(&tree, p[0], p[1], p[2], p[3], (uint32_t)i);
            if (error) break;
        }
    }

    if (!error)
    {
        if (modeIn->bitDepth == 16 && modeOut->bitDepth == 16)
        {
            for (i = 0; i != numPixels; ++i)
            {
                uint16_t r = 0, g = 0, b = 0, a = 0;
                getPixelColorRGBA16(&r, &g, &b, &a, in, i, modeIn);
                RGBA16ToPixel(out, i, modeOut, r, g, b, a);
            }
        }
        else if (modeOut->bitDepth == 8 && modeOut->colorType == LCT_RGBA)
        {
            getPixelColorsRGBA8(out, numPixels, in, modeIn);
        }
        else if (modeOut->bitDepth == 8 && modeOut->colorType == LCT_RGB)
        {
            getPixelColorsRGB8(out, numPixels, in, modeIn);
        }
        else
        {
            uint8_t r = 0, g = 0, b = 0, a = 0;
            for (i = 0; i != numPixels; ++i)
            {
                getPixelColorRGBA8(&r, &g, &b, &a, in, i, modeIn);
                error = RGBA8ToPixel(out, i, modeOut, &tree, r, g, b, a);
                if (error) break;
            }
        }
    }

    if (modeOut->colorType == LCT_PALETTE)
    {
        colorTreeCleanup(&tree);
    }

    return error;
}

uint32_t PNGConvertRGB(uint32_t* outR, uint32_t* outG, uint32_t* outB, uint32_t inR, uint32_t inG, uint32_t inB, const PNGColorMode* modeOut, const PNGColorMode* modeIn)
{
    uint32_t r = 0, g = 0, b = 0;
    uint32_t mul = 65535 / ((1u << modeIn->bitDepth) - 1u);
    uint32_t shift = 16 - modeOut->bitDepth;

    if (modeIn->colorType == LCT_GREY || modeIn->colorType == LCT_GREY_ALPHA)
    {
        r = g = b = inR * mul;
    }
    else if (modeIn->colorType == LCT_RGB || modeIn->colorType == LCT_RGBA)
    {
        r = inR * mul;
        g = inG * mul;
        b = inB * mul;
    }
    else if (modeIn->colorType == LCT_PALETTE)
    {
        if (inR >= modeIn->paletteSize) return 82;
        r = modeIn->palette[inR * 4 + 0] * 257u;
        g = modeIn->palette[inR * 4 + 1] * 257u;
        b = modeIn->palette[inR * 4 + 2] * 257u;
    }
    else
    {
        return 31;
    }

    if (modeOut->colorType == LCT_GREY || modeOut->colorType == LCT_GREY_ALPHA)
    {
        *outR = r >> shift ;
    }
    else if (modeOut->colorType == LCT_RGB || modeOut->colorType == LCT_RGBA)
    {
        *outR = r >> shift ;
        *outG = g >> shift ;
        *outB = b >> shift ;
    }
    else if (modeOut->colorType == LCT_PALETTE)
    {
        uint32_t i;
        if ((r >> 8) != (r & 255) || (g >> 8) != (g & 255) || (b >> 8) != (b & 255)) return 82;
        for (i = 0; i < modeOut->paletteSize; i++)
        {
            uint32_t j = i * 4;
            if ((r >> 8) == modeOut->palette[j + 0] && (g >> 8) == modeOut->palette[j + 1] && (b >> 8) == modeOut->palette[j + 2])
            {
                *outR = i;
                return 0;
            }
        }
        return 82;
    }
    else
    {
        return 31;
    }

    return 0;
}

void PNGColorStatsInit(PNGColorStats* stats)
{
    stats->colored = 0;
    stats->key = 0;
    stats->keyR = stats->keyG = stats->keyB = 0;
    stats->alpha = 0;
    stats->numColors = 0;
    stats->bits = 1;
    stats->numPixels = 0;
    stats->allowPalette = 1;
    stats->allowGreyscale = 1;
}

uint32_t getValueRequiredBits(uint8_t value)
{
    if (value == 0 || value == 255) return 1;
    if (value % 17 == 0) return value % 85 == 0 ? 2 : 4;
    return 8;
}

uint32_t PNGComputeColorStats(PNGColorStats* stats, const uint8_t* in, uint32_t w, uint32_t h, const PNGColorMode* modeIn)
{
    uint32_t i, error = 0;
    ColorTree tree;
    uint32_t numPixels = w * h;

    uint32_t coloredDone = PNGIsGreyscaleType(modeIn) ? 1 : 0;
    uint32_t alphaDone = PNGCanHaveAlpha(modeIn) ? 0 : 1;
    uint32_t numColorsDone = 0;
    uint32_t bpp = PNGGetBpp(modeIn);
    uint32_t bitsDone = (stats->bits == 1 && bpp == 1) ? 1 : 0;
    uint32_t sixteen = 0;
    uint32_t maxNumColors = 257;
    
    if (bpp <= 8) maxNumColors = min(257, stats->numColors + (1u << bpp));

    stats->numPixels += numPixels;

    if (!stats->allowPalette) numColorsDone = 1;

    colorTreeInit(&tree);

    if (stats->alpha) alphaDone = 1;
    if (stats->colored) coloredDone = 1;
    if (stats->bits == 16) numColorsDone = 1;
    if (stats->bits >= bpp) bitsDone = 1;
    if (stats->numColors >= maxNumColors) numColorsDone = 1;

    if (!numColorsDone)
    {
        for (i = 0; i < stats->numColors; i++)
        {
            const uint8_t* color = &stats->palette[i * 4];
            error = colorTreeAdd(&tree, color[0], color[1], color[2], color[3], i);
            if (error) goto cleanup;
        }
    }

    if (modeIn->bitDepth == 16 && !sixteen)
    {
        uint16_t r = 0, g = 0, b = 0, a = 0;
        for (i = 0; i != numPixels; ++i)
        {
            getPixelColorRGBA16(&r, &g, &b, &a, in, i, modeIn);
            if ((r & 255) != ((r >> 8) & 255) || (g & 255) != ((g >> 8) & 255) || (b & 255) != ((b >> 8) & 255) || (a & 255) != ((a >> 8) & 255))
            {
                stats->bits = 16;
                sixteen = 1;
                bitsDone = 1;
                numColorsDone = 1;
                break;
            }
        }
    }

    if (sixteen)
    {
        uint16_t r = 0, g = 0, b = 0, a = 0;

        for (i = 0; i != numPixels; ++i)
        {
            getPixelColorRGBA16(&r, &g, &b, &a, in, i, modeIn);

            if (!coloredDone && (r != g || r != b))
            {
                stats->colored = 1;
                coloredDone = 1;
            }

            if (!alphaDone)
            {
                uint32_t matchKey = (r == stats->keyR && g == stats->keyG && b == stats->keyB);
                if (a != 65535 && (a != 0 || (stats->key && !matchKey)))
                {
                    stats->alpha = 1;
                    stats->key = 0;
                    alphaDone = 1;
                }
                else if (a == 0 && !stats->alpha && !stats->key)
                {
                    stats->key = 1;
                    stats->keyR = r;
                    stats->keyG = g;
                    stats->keyB = b;
                }
                else if (a == 65535 && stats->key && matchKey)
                {
                    stats->alpha = 1;
                    stats->key = 0;
                    alphaDone = 1;
                }
            }

            if (alphaDone && numColorsDone && coloredDone && bitsDone) break;
        }

        if (stats->key && !stats->alpha)
        {
            for (i = 0; i != numPixels; ++i)
            {
                getPixelColorRGBA16(&r, &g, &b, &a, in, i, modeIn);
                if (a != 0 && r == stats->keyR && g == stats->keyG && b == stats->keyB)
                {
                    stats->alpha = 1;
                    stats->key = 0;
                    alphaDone = 1;
                }
            }
        }
    }
    else
    {
        uint8_t r = 0, g = 0, b = 0, a = 0;
        for (i = 0; i != numPixels; ++i)
        {
            getPixelColorRGBA8(&r, &g, &b, &a, in, i, modeIn);

            if (!bitsDone && stats->bits < 8)
            {
                uint32_t bits = getValueRequiredBits(r);
                if (bits > stats->bits) stats->bits = bits;
            }

            bitsDone = (stats->bits >= bpp);

            if (!coloredDone && (r != g || r != b))
            {
                stats->colored = 1;
                coloredDone = 1;
                if (stats->bits < 8) stats->bits = 8;
            }

            if (!alphaDone)
            {
                uint32_t matchKey = (r == stats->keyR && g == stats->keyG && b == stats->keyB);
                if (a != 255 && (a != 0 || (stats->key && !matchKey)))
                {
                    stats->alpha = 1;
                    stats->key = 0;
                    alphaDone = 1;
                    if (stats->bits < 8) stats->bits = 8;
                }
                else if (a == 0 && !stats->alpha && !stats->key)
                {
                    stats->key = 1;
                    stats->keyR = r;
                    stats->keyG = g;
                    stats->keyB = b;
                }
                else if (a == 255 && stats->key && matchKey)
                {
                    stats->alpha = 1;
                    stats->key = 0;
                    alphaDone = 1;
                    if (stats->bits < 8) stats->bits = 8;
                }
            }

            if (!numColorsDone)
            {
                if (!colorTreeHas(&tree, r, g, b, a))
                {
                    error = colorTreeAdd(&tree, r, g, b, a, stats->numColors);
                    if (error) goto cleanup;
                    if (stats->numColors < 256)
                    {
                        uint8_t* p = stats->palette;
                        uint32_t n = stats->numColors;
                        p[n * 4 + 0] = r;
                        p[n * 4 + 1] = g;
                        p[n * 4 + 2] = b;
                        p[n * 4 + 3] = a;
                    }
                    ++stats->numColors;
                    numColorsDone = stats->numColors >= maxNumColors;
                }
            }

            if (alphaDone && numColorsDone && coloredDone && bitsDone) break;
        }

        if (stats->key && !stats->alpha)
        {
            for (i = 0; i != numPixels; ++i)
            {
                getPixelColorRGBA8(&r, &g, &b, &a, in, i, modeIn);
                if (a != 0 && r == stats->keyR && g == stats->keyG && b == stats->keyB)
                {
                    stats->alpha = 1;
                    stats->key = 0;
                    alphaDone = 1;
                    if (stats->bits < 8) stats->bits = 8;
                }
            }
        }

        stats->keyR += (stats->keyR << 8);
        stats->keyG += (stats->keyG << 8);
        stats->keyB += (stats->keyB << 8);
    }

    cleanup:
    colorTreeCleanup(&tree);
    return error;
}

uint32_t PNGColorStatsAdd(PNGColorStats* stats, uint32_t r, uint32_t g, uint32_t b, uint32_t a)
{
    uint32_t error = 0;
    uint8_t image[8];
    PNGColorMode mode;

    PNGColorModeInit(&mode);
    image[0] = r >> 8; image[1] = r; image[2] = g >> 8; image[3] = g;
    image[4] = b >> 8; image[5] = b; image[6] = a >> 8; image[7] = a;
    mode.bitDepth = 16;
    mode.colorType = LCT_RGBA;
    error = PNGComputeColorStats(stats, image, 1, 1, &mode);
    PNGColorModeCleanup(&mode);
    return error;
}

uint32_t autoChooseColor(PNGColorMode* modeOut, const PNGColorMode* modeIn, const PNGColorStats* stats)
{
    uint32_t error = 0;
    uint32_t paletteBits;
    uint32_t i, n;
    uint32_t numPixels = stats->numPixels;
    uint32_t paletteOk, grayOk;

    uint32_t alpha = stats->alpha;
    uint32_t key = stats->key;
    uint32_t bits = stats->bits;

    modeOut->keyDefined = 0;

    if (key && numPixels <= 16)
    {
        alpha = 1;
        key = 0;
        if (bits < 8) bits = 8;
    }

    grayOk = !stats->colored;

    if (!stats->allowGreyscale) grayOk = 0;
    if (!grayOk && bits < 8) bits = 8;

    n = stats->numColors;
    paletteBits = n <= 2 ? 1 : (n <= 4 ? 2 : (n <= 16 ? 4 : 8));
    paletteOk = n <= 256 && bits <= 8 && n != 0;

    if (numPixels < n * 2) paletteOk = 0;
    if (grayOk && !alpha && bits <= paletteBits) paletteOk = 0;
    if (!stats->allowPalette) paletteOk = 0;

    if (paletteOk)
    {
        const uint8_t* p = stats->palette;
        PNGPaletteClear(modeOut);
        for (i = 0; i != stats->numColors; ++i)
        {
            error = PNGPaletteAdd(modeOut, p[i * 4 + 0], p[i * 4 + 1], p[i * 4 + 2], p[i * 4 + 3]);
            if (error) break;
        }

        modeOut->colorType = LCT_PALETTE;
        modeOut->bitDepth = paletteBits;

        if (modeIn->colorType == LCT_PALETTE && modeIn->paletteSize >= modeOut->paletteSize && modeIn->bitDepth == modeOut->bitDepth)
        {
            PNGColorModeCleanup(modeOut);
            PNGColorModeCopy(modeOut, modeIn);
        }
    }
    else
    {
        modeOut->bitDepth = bits;
        modeOut->colorType = alpha ? (grayOk ? LCT_GREY_ALPHA : LCT_RGBA) : (grayOk ? LCT_GREY : LCT_RGB);
        if (key)
        {
            uint32_t mask = (1u << modeOut->bitDepth) - 1u;
            modeOut->keyR = stats->keyR & mask;
            modeOut->keyG = stats->keyG & mask;
            modeOut->keyB = stats->keyB & mask;
            modeOut->keyDefined = 1;
        }
    }

    return error;
}

uint8_t paethPredictor(short a, short b, short c)
{
    short pa = abs(b - c);
    short pb = abs(a - c);
    short pc = abs(a + b - c - c);
    if (pb < pa) { a = b; pa = pb; }
    return (pc < pa) ? c : a;
}

const uint32_t ADAM7_IX[7] = { 0, 4, 0, 2, 0, 1, 0 };
const uint32_t ADAM7_IY[7] = { 0, 0, 4, 0, 2, 0, 1 };
const uint32_t ADAM7_DX[7] = { 8, 8, 4, 4, 2, 2, 1 };
const uint32_t ADAM7_DY[7] = { 8, 8, 8, 4, 4, 2, 2 };

void adam7GetPassValues(uint32_t passw[7], uint32_t passh[7], uint32_t fillterPassStart[8], uint32_t paddedPassStart[8], uint32_t passStart[8], uint32_t w, uint32_t h, uint32_t bpp)
{
    uint32_t i;

    for (i = 0; i != 7; i++)
    {
        passw[i] = (w + ADAM7_DX[i] - ADAM7_IX[i] - 1) / ADAM7_DX[i];
        passh[i] = (h + ADAM7_DY[i] - ADAM7_IY[i] - 1) / ADAM7_DY[i];
        if (passw[i] == 0) passh[i] = 0;
        if (passh[i] == 0) passw[i] = 0;
    }

    fillterPassStart[0] = paddedPassStart[0] = passStart[0] = 0;

    for (i = 0; i != 7; i++)
    {
        fillterPassStart[i + 1] = fillterPassStart[i] + ((passw[i] && passh[i]) ? passh[i] * (1u + (passw[i] * bpp + 7u) / 8u) : 0);
        paddedPassStart[i + 1] = paddedPassStart[i] + passh[i] * ((passw[i] * bpp + 7u) / 8u);
        passStart[i + 1] = passStart[i] + (passh[i] * passw[i] * bpp + 7u) / 8u;
    }
}

/*========================*/
/*      PNG decoder       */ 
/*========================*/

uint32_t PNGInspect(uint32_t* w, uint32_t* h, PNGState* state, const uint8_t* in, uint32_t insize)
{
    uint32_t width, height;
    PNGInfo* info = &state->pngInfo;

    if (insize == 0 || in == 0)
    {
        CERROR_RETURN_ERROR(state->error, 48);
    }

    if (insize < 33)
    {
        CERROR_RETURN_ERROR(state->error, 27);
    }

    PNGInfoCleanup(info);
    PNGInfoInit(info);

    if (in[0] != 137 || in[1] != 80 || in[2] != 78 || in[3] != 71 || in[4] != 13 || in[5] != 10 || in[6] != 26 || in[7] != 10)
    {
        CERROR_RETURN_ERROR(state->error, 28);
    }

    if (PNGChunkLength(in + 8) != 13)
    {
        CERROR_RETURN_ERROR(state->error, 94);
    }

    if (!PNGChunkTypeEquals(in + 8, "IHDR"))
    {
        CERROR_RETURN_ERROR(state->error, 29);
    }

    width = PNGRead32bitInt(&in[16]);
    height = PNGRead32bitInt(&in[20]);
    if (w) *w = width;
    if (h) *h = height;

    info->colorMode.bitDepth = in[24];
    info->colorMode.colorType = (PNGColorType)in[25];
    info->compressionMethod = in[26];
    info->filterMethod = in[27];
    info->interlaceMethod = in[28];

    if (width == 0 || height == 0) CERROR_RETURN_ERROR(state->error, 93);

    state->error = checkColorValidity(info->colorMode.colorType, info->colorMode.bitDepth);
    if (state->error) return state->error;

    if (info->compressionMethod != 0) CERROR_RETURN_ERROR(state->error, 32);
    if (info->filterMethod != 0) CERROR_RETURN_ERROR(state->error, 33);
    if (info->interlaceMethod > 1) CERROR_RETURN_ERROR(state->error, 34);

    if (!state->decoder.ignoreCRC)
    {
        uint32_t CRC = PNGRead32bitInt(&in[29]);
        uint32_t checksum = crc32(&in[12], 17);
        if (CRC != checksum)
        {
            CERROR_RETURN_ERROR(state->error, 57);
        }
    }

    return state->error;
}

uint32_t unfilterScanline(uint8_t* recon, const uint8_t* scanline, const uint8_t* precon, uint32_t byteWidth, uint8_t filterType, uint32_t length)
{
    uint32_t i;

    switch (filterType)
    {
    case 0:
        for (i = 0; i != length; ++i) recon[i] = scanline[i];
        break;

    case 1:
    {
        uint32_t j = 0;
        for (i = 0; i != byteWidth; ++i) recon[i] = scanline[i];
        for (i = byteWidth; i != length; ++i, ++j) recon[i] = scanline[i] + recon[j];
        break;
    }

    case 2:
        if (precon)
        {
            for (i = 0; i != length; ++i) recon[i] = scanline[i] + precon[i];
        }
        else
        {
            for (i = 0; i != length; ++i) recon[i] = scanline[i];
        }
        break;

    case 3:
        if (precon)
        {
            uint32_t j = 0;
            for (i = 0; i != byteWidth; ++i) recon[i] = scanline[i] + (precon[i] >> 1u);
            if (byteWidth >= 4)
            {
                for (; i + 3 < length; i += 4, j += 4)
                {
                    uint8_t s0 = scanline[i + 0], s1 = scanline[i + 1], s2 = scanline[i + 2], s3 = scanline[i + 3];
                    uint8_t r0 = recon[j + 0], r1 = recon[j + 1], r2 = recon[j + 2], r3 = recon[j + 3];
                    uint8_t p0 = precon[i + 0], p1 = precon[i + 1], p2 = precon[i + 2], p3 = precon[i + 3];
                    recon[i + 0] = s0 + ((r0 + p0) >> 1u);
                    recon[i + 1] = s1 + ((r1 + p1) >> 1u);
                    recon[i + 2] = s2 + ((r2 + p2) >> 1u);
                    recon[i + 3] = s3 + ((r3 + p3) >> 1u);
                }
            }
            else if (byteWidth >= 3)
            {
                for (; i + 2 < length; i += 3, j += 3)
                {
                    uint8_t s0 = scanline[i + 0], s1 = scanline[i + 1], s2 = scanline[i + 2];
                    uint8_t r0 = recon[j + 0], r1 = recon[j + 1], r2 = recon[j + 2];
                    uint8_t p0 = precon[i + 0], p1 = precon[i + 1], p2 = precon[i + 2];
                    recon[i + 0] = s0 + ((r0 + p0) >> 1u);
                    recon[i + 1] = s1 + ((r1 + p1) >> 1u);
                    recon[i + 2] = s2 + ((r2 + p2) >> 1u);
                }
            }
            else if (byteWidth >= 2)
            {
                for (; i + 1 < length; i += 2, j += 2)
                {
                    uint8_t s0 = scanline[i + 0], s1 = scanline[i + 1];
                    uint8_t r0 = recon[j + 0], r1 = recon[j + 1];
                    uint8_t p0 = precon[i + 0], p1 = precon[i + 1];
                    recon[i + 0] = s0 + ((r0 + p0) >> 1u);
                    recon[i + 1] = s1 + ((r1 + p1) >> 1u);
                }
            }

            for (; i != length; ++i, ++j) recon[i] = scanline[i] + ((recon[j] + precon[i]) >> 1u);
        }
        else
        {
            uint32_t j = 0;
            for (i = 0; i != byteWidth; ++i) recon[i] = scanline[i];
            for (i = byteWidth; i != length; ++i, ++j) recon[i] = scanline[i] + (recon[j] >> 1u);
        }
        break;

    case 4:
        if (precon)
        {
            uint32_t j = 0;
            for (i = 0; i != byteWidth; ++i)
            {
                recon[i] = (scanline[i] + precon[i]);
            }

            if (byteWidth >= 4)
            {
                for (; i + 3 < length; i += 4, j += 4)
                {
                    uint8_t s0 = scanline[i + 0], s1 = scanline[i + 1], s2 = scanline[i + 2], s3 = scanline[i + 3];
                    uint8_t r0 = recon[j + 0], r1 = recon[j + 1], r2 = recon[j + 2], r3 = recon[j + 3];
                    uint8_t p0 = precon[i + 0], p1 = precon[i + 1], p2 = precon[i + 2], p3 = precon[i + 3];
                    uint8_t q0 = precon[j + 0], q1 = precon[j + 1], q2 = precon[j + 2], q3 = precon[j + 3];
                    recon[i + 0] = s0 + paethPredictor(r0, p0, q0);
                    recon[i + 1] = s1 + paethPredictor(r1, p1, q1);
                    recon[i + 2] = s2 + paethPredictor(r2, p2, q2);
                    recon[i + 3] = s3 + paethPredictor(r3, p3, q3);
                }
            }
            else if (byteWidth >= 3)
            {
                for (; i + 2 < length; i += 3, j += 3)
                {
                    uint8_t s0 = scanline[i + 0], s1 = scanline[i + 1], s2 = scanline[i + 2];
                    uint8_t r0 = recon[j + 0], r1 = recon[j + 1], r2 = recon[j + 2];
                    uint8_t p0 = precon[i + 0], p1 = precon[i + 1], p2 = precon[i + 2];
                    uint8_t q0 = precon[j + 0], q1 = precon[j + 1], q2 = precon[j + 2];
                    recon[i + 0] = s0 + paethPredictor(r0, p0, q0);
                    recon[i + 1] = s1 + paethPredictor(r1, p1, q1);
                    recon[i + 2] = s2 + paethPredictor(r2, p2, q2);
                }
            }
            else if (byteWidth >= 2)
            {
                for (; i + 1 < length; i += 2, j += 2)
                {
                    uint8_t s0 = scanline[i + 0], s1 = scanline[i + 1];
                    uint8_t r0 = recon[j + 0], r1 = recon[j + 1];
                    uint8_t p0 = precon[i + 0], p1 = precon[i + 1];
                    uint8_t q0 = precon[j + 0], q1 = precon[j + 1];
                    recon[i + 0] = s0 + paethPredictor(r0, p0, q0);
                    recon[i + 1] = s1 + paethPredictor(r1, p1, q1);
                }
            }

            for (; i != length; ++i, ++j)
            {
                recon[i] = (scanline[i] + paethPredictor(recon[i - byteWidth], precon[i], precon[j]));
            }
        }
        else
        {
            uint32_t j = 0;
            for (i = 0; i != byteWidth; ++i)
            {
                recon[i] = scanline[i];
            }

            for (i = byteWidth; i != length; ++i, ++j)
            {
                recon[i] = (scanline[i] + recon[j]);
            }
        }
        break;

    default: return 36;
    }
    return 0;
}

uint32_t unfilter(uint8_t* out, const uint8_t* in, uint32_t w, uint32_t h, uint32_t bpp)
{
    uint32_t y;
    uint8_t* prevLine = 0;
    uint32_t byteWidth = (bpp + 7u) / 8u;
    uint32_t lineBytes = PNGGetRawSizeIDAT(w, 1, bpp) - 1u;

    for (y = 0; y < h; ++y)
    {
        uint32_t outindex = lineBytes * y;
        uint32_t inindex = (1 + lineBytes) * y;
        uint8_t filterType = in[inindex];
        CERROR_TRY_RETURN(unfilterScanline(&out[outindex], &in[inindex + 1], prevLine, byteWidth, filterType, lineBytes));
        prevLine = &out[outindex];
    }

    return 0;
}

void adam7Deinterlace(uint8_t* out, const uint8_t* in, uint32_t w, uint32_t h, uint32_t bpp)
{
    uint32_t passw[7], passh[7];
    uint32_t filterPassStart[8], paddedPassStart[8], passStart[8];
    uint32_t i;

    adam7GetPassValues(passw, passh, filterPassStart, paddedPassStart, passStart, w, h, bpp);

    if (bpp >= 8)
    {
        for (i = 0; i != 7; ++i)
        {
            uint32_t x, y, b;
            uint32_t byteWidth = bpp / 8u;

            for (y = 0; y < passh[i]; ++y)
            for (x = 0; x < passw[i]; ++x)
            {
                uint32_t pixelinstart = passStart[i] + (y * passw[i] + x) * byteWidth;
                uint32_t pixeloutstart = ((ADAM7_IY[i] + (uint32_t)y * ADAM7_DY[i]) * (uint32_t)w + ADAM7_IX[i] + (uint32_t)x * ADAM7_DX[i]) * byteWidth;
                for (b = 0; b < byteWidth; ++b)
                {
                    out[pixeloutstart + b] = in[pixelinstart + b];
                }
            }
        }
    }
    else
    {
        for (i = 0; i != 7; ++i)
        {
            uint32_t x, y, b;
            uint32_t ilineBits = bpp * passw[i];
            uint32_t olineBits = bpp * w;
            uint32_t obp, ibp;

            for (y = 0; y < passh[i]; ++y)
            for (x = 0; x < passw[i]; ++x)
            {
                ibp = (8 * passStart[i]) + (y * ilineBits + x * bpp);
                obp = (ADAM7_IY[i] + (uint32_t)y * ADAM7_DY[i]) * olineBits + (ADAM7_IX[i] + (uint32_t)x * ADAM7_DX[i]) * bpp;
                for (b = 0; b < bpp; ++b)
                {
                    uint8_t bit = readBitFromReversedStream(&ibp, in);
                    setBitOfReversedStream(&obp, out, bit);
                }
            }
        }
    }
}

void removePaddingBits(uint8_t* out, const uint8_t* in, uint32_t olineBits, uint32_t ilineBits, uint32_t h)
{
    uint32_t y;
    uint32_t diff = ilineBits - olineBits;
    uint32_t ibp = 0, obp = 0;

    for (y = 0; y < h; ++y)
    {
        uint32_t x;
        for (x = 0; x < olineBits; ++x)
        {
            uint8_t bit = readBitFromReversedStream(&ibp, in);
            setBitOfReversedStream(&obp, out, bit);
        }

        ibp += diff;
    }
}

uint32_t postProcessScanlines(uint8_t* out, uint8_t* in, uint32_t w, uint32_t h, const PNGInfo* pngInfo)
{
    uint32_t bpp = PNGGetBpp(&pngInfo->colorMode);
    if (bpp == 0) return 31;

    if (pngInfo->interlaceMethod == 0)
    {
        if (bpp < 8 && w * bpp != ((w * bpp + 7u) / 8u) * 8u)
        {
            CERROR_TRY_RETURN(unfilter(in, in, w, h, bpp));
            removePaddingBits(out, in, w * bpp, ((w * bpp + 7u) / 8u) * 8u, h);
        }
        else CERROR_TRY_RETURN(unfilter(out, in, w, h, bpp));
    }
    else
    {
        uint32_t i;
        uint32_t passw[7], passh[7]; uint32_t filterPassStart[8], paddedPassStart[8], passStart[8];
        adam7GetPassValues(passw, passh, filterPassStart, paddedPassStart, passStart, w, h, bpp);

        for (i = 0; i != 7; ++i)
        {
            CERROR_TRY_RETURN(unfilter(&in[paddedPassStart[i]], &in[filterPassStart[i]], passw[i], passh[i], bpp));
            if (bpp < 8)
            {
                removePaddingBits(&in[passStart[i]], &in[paddedPassStart[i]], passw[i] * bpp, ((passw[i] * bpp + 7u) / 8u) * 8u, passh[i]);
            }
        }

        adam7Deinterlace(out, in, w, h, bpp);
    }

    return 0;
}

uint32_t readChunk_PLTE(PNGColorMode* color, const uint8_t* data, uint32_t chunkLength)
{
    uint32_t pos = 0, i;
    color->paletteSize = chunkLength / 3u;
    if (color->paletteSize == 0 || color->paletteSize > 256) return 38;

    PNGColorModeAllocPalette(color);

    if (!color->palette && color->paletteSize)
    {
        color->paletteSize = 0;
        return 83;
    }

    for (i = 0; i != color->paletteSize; ++i)
    {
        color->palette[4 * i + 0] = data[pos++];
        color->palette[4 * i + 1] = data[pos++];
        color->palette[4 * i + 2] = data[pos++];
        color->palette[4 * i + 3] = 255;
    }

    return 0;
}

uint32_t readChunk_tRNS(PNGColorMode* color, const uint8_t* data, uint32_t chunkLength)
{
    uint32_t i;
    if (color->colorType == LCT_PALETTE)
    {
        if (chunkLength > color->paletteSize) return 39;
        for (i = 0; i != chunkLength; ++i) color->palette[4 * i + 3] = data[i];
    }
    else if (color->colorType == LCT_GREY)
    {
        if (chunkLength != 2) return 30;
        color->keyDefined = 1;
        color->keyR = color->keyG = color->keyB = 256u * data[0] + data[1];
    }
    else if (color->colorType == LCT_RGB)
    {
        if (chunkLength != 6) return 41;
        color->keyDefined = 1;
        color->keyR = 256u * data[0] + data[1];
        color->keyG = 256u * data[2] + data[3];
        color->keyB = 256u * data[4] + data[5];
    }
    else return 42;

    return 0;
}

uint32_t readChunk_bKGD(PNGInfo* info, const uint8_t* data, uint32_t chunkLength)
{
    if (info->colorMode.colorType == LCT_PALETTE)
    {
        if (chunkLength != 1) return 43;
        if (data[0] >= info->colorMode.paletteSize) return 103;
        info->backgroundDefined = 1;
        info->backgroundR = info->backgroundG = info->backgroundB = data[0];
    }
    else if (info->colorMode.colorType == LCT_GREY || info->colorMode.colorType == LCT_GREY_ALPHA)
    {
        if (chunkLength != 2) return 44;
        info->backgroundDefined = 1;
        info->backgroundR = info->backgroundG = info->backgroundB = 256u * data[0] + data[1];
    }
    else if (info->colorMode.colorType == LCT_RGB || info->colorMode.colorType == LCT_RGBA)
    {
        if (chunkLength != 6) return 45;
        info->backgroundDefined = 1;
        info->backgroundR = 256u * data[0] + data[1];
        info->backgroundG = 256u * data[2] + data[3];
        info->backgroundB = 256u * data[4] + data[5];
    }

    return 0;
}

uint32_t readChunk_tEXt(PNGInfo* info, const uint8_t* data, uint32_t chunkLength)
{
    uint32_t error = 0;
    char *key = 0, *str = 0;

    while (!error)
    {
        uint32_t length, string2Begin;

        length = 0;
        while (length < chunkLength && data[length] != 0) ++length;
        if (length < 1 || length > 79) CERROR_BREAK(error, 89);

        key = (char*)calloc(length + 1, 1);
        if (!key) CERROR_BREAK(error, 83);

        memcpy(key, data, length);
        key[length] = 0;

        string2Begin = length + 1;
        length = (uint32_t)(chunkLength < string2Begin ? 0 : chunkLength - string2Begin);
        str = (char*)calloc(length + 1, 1);
        if (!str) CERROR_BREAK(error, 83);
        memcpy(str, data + string2Begin, length);
        str[length] = 0;
        error = PNGAddText(info, key, str);
        break;
    }

    free(key);
    free(str);
    return error;
}

uint32_t readChunk_zTXt(PNGInfo* info, const PNGDecoderSettings* decoder, const uint8_t* data, uint32_t chunkLength)
{
    uint32_t error = 0;
    PNGDecompressSettings zlibSettings = decoder->zlibSettings;

    uint32_t length, string2Begin;
    char *key = 0;
    uint8_t* str = 0;
    uint32_t size = 0;

    while (!error)
    {
        for (length = 0; length < chunkLength && data[length] != 0; ++length);
        if (length + 2 >= chunkLength) CERROR_BREAK(error, 75);
        if (length < 1 || length > 79) CERROR_BREAK(error, 89);

        key = (char*)calloc(length + 1, 1);
        if (!key) CERROR_BREAK(error, 83);

        memcpy(key, data, length);
        key[length] = 0;

        if (data[length + 1] != 0) CERROR_BREAK(error, 72);

        string2Begin = length + 2;
        if (string2Begin > chunkLength) CERROR_BREAK(error, 75);

        length = chunkLength - string2Begin;
        zlibSettings.maxOutputSize = decoder->maxTextSize;
        error = zlibDecompress(&str, &size, 0, &data[string2Begin], length, &zlibSettings);
        if (error && size > zlibSettings.maxOutputSize) error = 112;
        if (error) break;
        error = PNGAddTextSized(info, key, (char*)str, size);
        break;
    }

    free(key);
    free(str);
    return error;
}

uint32_t readChunk_iTXt(PNGInfo* info, const PNGDecoderSettings* decoder, const uint8_t* data, uint32_t chunkLength)
{
    uint32_t i;
    uint32_t error = 0;
    
    PNGDecompressSettings zlibSettings = decoder->zlibSettings;

    uint32_t length, begin, compressed;
    char *key = 0, *langtag = 0, *transkey = 0;

    while (!error)
    {
        if (chunkLength < 5) CERROR_BREAK(error, 30);
        for (length = 0; length < chunkLength && data[length] != 0; ++length);
        if (length + 3 >= chunkLength) CERROR_BREAK(error, 75);
        if (length < 1 || length > 79) CERROR_BREAK(error, 89);

        key = (char*)calloc(length + 1, 1);
        if (!key) CERROR_BREAK(error, 83);
        memcpy(key, data, length);
        key[length] = 0;

        compressed = data[length + 1];
        if (data[length + 2] != 0) CERROR_BREAK(error, 72);

        begin = length + 3;
        length = 0;
        for (i = begin; i < chunkLength && data[i] != 0; ++i) ++length;

        langtag = (char*)calloc(length + 1, 1);
        if (!langtag) CERROR_BREAK(error, 83);

        memcpy(langtag, data + begin, length);
        langtag[length] = 0;

        begin += length + 1;
        length = 0;
        for (i = begin; i < chunkLength && data[i] != 0; ++i) ++length;

        transkey = (char*)calloc(length + 1, 1);
        if (!transkey) CERROR_BREAK(error, 83);

        memcpy(transkey, data + begin, length);
        transkey[length] = 0;

        begin += length + 1;
        length = chunkLength < begin ? 0 : chunkLength - begin;

        if (compressed)
        {
            uint8_t* str = 0;
            uint32_t size = 0;
            zlibSettings.maxOutputSize = decoder->maxTextSize;
            error = zlibDecompress(&str, &size, 0, &data[begin], length, &zlibSettings);
            if (error && size > zlibSettings.maxOutputSize) error = 112;
            if (!error) error = PNGAddITextSized(info, key, langtag, transkey, (char*)str, size);
            free(str);
        }
        else
        {
            error = PNGAddITextSized(info, key, langtag, transkey, (char*)(data + begin), length);
        }
        break;
    }

    free(key);
    free(langtag);
    free(transkey);

    return error;
}

uint32_t readChunk_tIME(PNGInfo* info, const uint8_t* data, uint32_t chunkLength)
{
    if (chunkLength != 7) return 73;
    info->timeDefined = 1;
    info->time.year = 256u * data[0] + data[1];
    info->time.month = data[2];
    info->time.day = data[3];
    info->time.hour = data[4];
    info->time.minute = data[5];
    info->time.second = data[6];
    return 0;
}

uint32_t readChunk_pHYs(PNGInfo* info, const uint8_t* data, uint32_t chunkLength)
{
    if (chunkLength != 9) return 74;
    info->physDefined = 1;
    info->physX = 16777216u * data[0] + 65536u * data[1] + 256u * data[2] + data[3];
    info->physY = 16777216u * data[4] + 65536u * data[5] + 256u * data[6] + data[7];
    info->physUnit = data[8];
    return 0;
}

uint32_t readChunk_gAMA(PNGInfo* info, const uint8_t* data, uint32_t chunkLength)
{
    if (chunkLength != 4) return 96;
    info->gamaDefined = 1;
    info->gamaGamma = 16777216u * data[0] + 65536u * data[1] + 256u * data[2] + data[3];
    return 0;
}

uint32_t readChunk_cHRM(PNGInfo* info, const uint8_t* data, uint32_t chunkLength)
{
    if (chunkLength != 32) return 97;
    info->chrmDefined = 1;
    info->chrmWhiteX = 16777216u * data[ 0] + 65536u * data[ 1] + 256u * data[ 2] + data[ 3];
    info->chrmWhiteY = 16777216u * data[ 4] + 65536u * data[ 5] + 256u * data[ 6] + data[ 7];
    info->chrmRedX   = 16777216u * data[ 8] + 65536u * data[ 9] + 256u * data[10] + data[11];
    info->chrmRedY   = 16777216u * data[12] + 65536u * data[13] + 256u * data[14] + data[15];
    info->chrmGreenX = 16777216u * data[16] + 65536u * data[17] + 256u * data[18] + data[19];
    info->chrmGreenY = 16777216u * data[20] + 65536u * data[21] + 256u * data[22] + data[23];
    info->chrmBlueX  = 16777216u * data[24] + 65536u * data[25] + 256u * data[26] + data[27];
    info->chrmBlueY  = 16777216u * data[28] + 65536u * data[29] + 256u * data[30] + data[31];
    return 0;
}

uint32_t readChunk_sRGB(PNGInfo* info, const uint8_t* data, uint32_t chunkLength)
{
    if (chunkLength != 1) return 98;
    info->srgbDefined = 1;
    info->srgbIntent = data[0];
    return 0;
}

uint32_t readChunk_iCCP(PNGInfo* info, const PNGDecoderSettings* decoder, const uint8_t* data, uint32_t chunkLength)
{
    uint32_t i;
    uint32_t error = 0;
    uint32_t size = 0;
    
    PNGDecompressSettings zlibSettings = decoder->zlibSettings;

    uint32_t length, string2Begin;

    info->iccpDefined = 1;
    if (info->iccpName) PNGClearICC(info);

    for (length = 0; length < chunkLength && data[length] != 0; ++length) ;
    if (length + 2 >= chunkLength) return 75;
    if (length < 1 || length > 79) return 89;

    info->iccpName = (char*)calloc(length + 1, 1);
    if (!info->iccpName) return 83;

    info->iccpName[length] = 0;
    for (i = 0; i != length; ++i) info->iccpName[i] = (char)data[i];

    if (data[length + 1] != 0) return 72;

    string2Begin = length + 2;
    if (string2Begin > chunkLength) return 75;

    length = chunkLength - string2Begin;
    zlibSettings.maxOutputSize = decoder->maxIccSize;
    error = zlibDecompress(&info->iccpProfile, &size, 0, &data[string2Begin], length, &zlibSettings);
    if (error && size > zlibSettings.maxOutputSize) error = 113;

    info->iccpProfileSize = size;
    if (!error && !info->iccpProfileSize) error = 100;
    return error;
}

uint32_t PNGInspectChunk(PNGState* state, uint32_t pos, const uint8_t* in, uint32_t insize)
{
    const uint8_t* chunk = in + pos;
    uint32_t chunkLength;
    const uint8_t* data;
    uint32_t unhandled = 0;
    uint32_t error = 0;

    if (pos + 4 > insize) return 30;
    chunkLength = PNGChunkLength(chunk);
    if (chunkLength > 2147483647) return 63;

    data = PNGChunkDataConst(chunk);
    if (data + chunkLength + 4 > in + insize) return 30;

    if (PNGChunkTypeEquals(chunk, "PLTE"))
    {
        error = readChunk_PLTE(&state->pngInfo.colorMode, data, chunkLength);
    }
    else if (PNGChunkTypeEquals(chunk, "tRNS"))
    {
        error = readChunk_tRNS(&state->pngInfo.colorMode, data, chunkLength);
    }
    else if (PNGChunkTypeEquals(chunk, "bKGD"))
    {
        error = readChunk_bKGD(&state->pngInfo, data, chunkLength);
    }
    else if (PNGChunkTypeEquals(chunk, "tEXt"))
    {
        error = readChunk_tEXt(&state->pngInfo, data, chunkLength);
    }
    else if (PNGChunkTypeEquals(chunk, "zTXt"))
    {
        error = readChunk_zTXt(&state->pngInfo, &state->decoder, data, chunkLength);
    }
    else if (PNGChunkTypeEquals(chunk, "iTXt"))
    {
        error = readChunk_iTXt(&state->pngInfo, &state->decoder, data, chunkLength);
    }
    else if (PNGChunkTypeEquals(chunk, "tIME"))
    {
        error = readChunk_tIME(&state->pngInfo, data, chunkLength);
    }
    else if (PNGChunkTypeEquals(chunk, "pHYs"))
    {
        error = readChunk_pHYs(&state->pngInfo, data, chunkLength);
    }
    else if (PNGChunkTypeEquals(chunk, "gAMA"))
    {
        error = readChunk_gAMA(&state->pngInfo, data, chunkLength);
    }
    else if (PNGChunkTypeEquals(chunk, "cHRM"))
    {
        error = readChunk_cHRM(&state->pngInfo, data, chunkLength);
    }
    else if (PNGChunkTypeEquals(chunk, "sRGB"))
    {
        error = readChunk_sRGB(&state->pngInfo, data, chunkLength);
    }
    else if (PNGChunkTypeEquals(chunk, "iCCP"))
    {
        error = readChunk_iCCP(&state->pngInfo, &state->decoder, data, chunkLength);
    }
    else
    {
        unhandled = 1;
    }

    if (!error && !unhandled && !state->decoder.ignoreCRC)
    {
        if (PNGChunkCheckCRC(chunk)) return 57;
    }

    return error;
}

void decodeGeneric(uint8_t** out, uint32_t* w, uint32_t* h, PNGState* state, const uint8_t* in, uint32_t insize)
{
    uint8_t IEND = 0;
    const uint8_t* chunk;
    uint8_t* idat;
    uint32_t idatSize = 0;
    uint8_t* scanlines = 0;
    uint32_t scanlinesSize = 0, expectedSize = 0;
    uint32_t outSize = 0;
    uint32_t unknown = 0;
    uint32_t criticalPos = 1;

    *out = 0;
    *w = *h = 0;

    state->error = PNGInspect(w, h, state, in, insize);
    if (state->error) return;

    if (PNGPixelOverflow(*w, *h, &state->pngInfo.colorMode, &state->rawInfo))
    {
        CERROR_RETURN(state->error, 92);
    }

    idat = (uint8_t*)calloc(insize, 1);
    if (!idat) CERROR_RETURN(state->error, 83);

    chunk = &in[33];

    while (!IEND && !state->error)
    {
        uint32_t chunkLength;
        const uint8_t* data;

        if (((chunk - in) + 12) > insize || chunk < in)
        {
            if (state->decoder.ignoreEnd) break;
            CERROR_BREAK(state->error, 30);
        }

        chunkLength = PNGChunkLength(chunk);
        if (chunkLength > 2147483647)
        {
            if (state->decoder.ignoreEnd) break;
            CERROR_BREAK(state->error, 63);
        }

        if (((chunk - in) + chunkLength + 12) > insize || (chunk + chunkLength + 12) < in)
        {
            CERROR_BREAK(state->error, 64);
        }

        data = PNGChunkDataConst(chunk);
        unknown = 0;

        if (PNGChunkTypeEquals(chunk, "IDAT"))
        {
            uint32_t newsize;
            if (PNGAddofl(idatSize, chunkLength, &newsize)) CERROR_BREAK(state->error, 95);
            if (newsize > insize) CERROR_BREAK(state->error, 95);
            memcpy(idat + idatSize, data, chunkLength);
            idatSize += chunkLength;
            criticalPos = 3;
        }
        else if (PNGChunkTypeEquals(chunk, "IEND"))
        {
            IEND = 1;
        }
        else if (PNGChunkTypeEquals(chunk, "PLTE"))
        {
            state->error = readChunk_PLTE(&state->pngInfo.colorMode, data, chunkLength);
            if (state->error) break;
            criticalPos = 2;
        }
        else if (PNGChunkTypeEquals(chunk, "tRNS"))
        {
            state->error = readChunk_tRNS(&state->pngInfo.colorMode, data, chunkLength);
            if (state->error) break;
        }
        else if (PNGChunkTypeEquals(chunk, "bKGD"))
        {
            state->error = readChunk_bKGD(&state->pngInfo, data, chunkLength);
            if (state->error) break;
        }
        else if (PNGChunkTypeEquals(chunk, "tEXt"))
        {
            if (state->decoder.readTextChunks)
            {
                state->error = readChunk_tEXt(&state->pngInfo, data, chunkLength);
                if (state->error) break;
            }
        }
        else if (PNGChunkTypeEquals(chunk, "zTXt"))
        {
            if (state->decoder.readTextChunks)
            {
                state->error = readChunk_zTXt(&state->pngInfo, &state->decoder, data, chunkLength);
                if (state->error) break;
            }
        }
        else if (PNGChunkTypeEquals(chunk, "iTXt"))
        {
            if (state->decoder.readTextChunks)
            {
                state->error = readChunk_iTXt(&state->pngInfo, &state->decoder, data, chunkLength);
                if (state->error) break;
            }
        }
        else if (PNGChunkTypeEquals(chunk, "tIME"))
        {
            state->error = readChunk_tIME(&state->pngInfo, data, chunkLength);
            if (state->error) break;
        }
        else if (PNGChunkTypeEquals(chunk, "pHYs"))
        {
            state->error = readChunk_pHYs(&state->pngInfo, data, chunkLength);
            if (state->error) break;
        }
        else if (PNGChunkTypeEquals(chunk, "gAMA"))
        {
            state->error = readChunk_gAMA(&state->pngInfo, data, chunkLength);
            if (state->error) break;
        }
        else if (PNGChunkTypeEquals(chunk, "cHRM"))
        {
            state->error = readChunk_cHRM(&state->pngInfo, data, chunkLength);
            if (state->error) break;
        }
        else if (PNGChunkTypeEquals(chunk, "sRGB"))
        {
            state->error = readChunk_sRGB(&state->pngInfo, data, chunkLength);
            if (state->error) break;
        }
        else if (PNGChunkTypeEquals(chunk, "iCCP"))
        {
            state->error = readChunk_iCCP(&state->pngInfo, &state->decoder, data, chunkLength);
            if (state->error) break;
        }
        else
        {
            if (!state->decoder.ignoreCritical && !PNGChunkAncillary(chunk))
            {
                CERROR_BREAK(state->error, 69);
            }
            unknown = 1;
            if (state->decoder.rememberUnknownChunks)
            {
                state->error = PNGChunkAppend(&state->pngInfo.unknownChunksData[criticalPos - 1], &state->pngInfo.unknownChunksSize[criticalPos - 1], chunk);
                if (state->error) break;
            }
        }

        if (!state->decoder.ignoreCRC && !unknown)
        {
            if (PNGChunkCheckCRC(chunk)) CERROR_BREAK(state->error, 57);
        }

        if (!IEND) chunk = PNGChunkNextConst(chunk, in + insize);
    }

    if (!state->error && state->pngInfo.colorMode.colorType == LCT_PALETTE && !state->pngInfo.colorMode.palette)
    {
        state->error = 106;
    }

    if (!state->error)
    {
        if (state->pngInfo.interlaceMethod == 0)
        {
            uint32_t bpp = PNGGetBpp(&state->pngInfo.colorMode);
            expectedSize = PNGGetRawSizeIDAT(*w, *h, bpp);
        }
        else
        {
            uint32_t bpp = PNGGetBpp(&state->pngInfo.colorMode);
            expectedSize = 0;
            expectedSize += PNGGetRawSizeIDAT((*w + 7) >> 3, (*h + 7) >> 3, bpp);
            if (*w > 4) expectedSize += PNGGetRawSizeIDAT((*w + 3) >> 3, (*h + 7) >> 3, bpp);
            expectedSize += PNGGetRawSizeIDAT((*w + 3) >> 2, (*h + 3) >> 3, bpp);
            if (*w > 2) expectedSize += PNGGetRawSizeIDAT((*w + 1) >> 2, (*h + 3) >> 2, bpp);
            expectedSize += PNGGetRawSizeIDAT((*w + 1) >> 1, (*h + 1) >> 2, bpp);
            if (*w > 1) expectedSize += PNGGetRawSizeIDAT((*w + 0) >> 1, (*h + 1) >> 1, bpp);
            expectedSize += PNGGetRawSizeIDAT((*w + 0), (*h + 0) >> 1, bpp);
        }

        state->error = zlibDecompress(&scanlines, &scanlinesSize, expectedSize, idat, idatSize, &state->decoder.zlibSettings);
    }

    if (!state->error && scanlinesSize != expectedSize) state->error = 91;
    free(idat);

    if (!state->error)
    {
        outSize = PNGGetRawSize(*w, *h, &state->pngInfo.colorMode);
        *out = (uint8_t*)calloc(outSize, 1);
        if (!*out) state->error = 83;
    }

    if (!state->error)
    {
        memset(*out, 0, outSize);
        state->error = postProcessScanlines(*out, scanlines, *w, *h, &state->pngInfo);
    }

    free(scanlines);
}

uint32_t PNGDecode(uint8_t** out, uint32_t* w, uint32_t* h, PNGState* state, const uint8_t* in, uint32_t insize)
{
    *out = 0;
    decodeGeneric(out, w, h, state, in, insize);
    if (state->error) return state->error;

    if (!state->decoder.colorConvert || PNGColorModeEqual(&state->rawInfo, &state->pngInfo.colorMode))
    {
        if (!state->decoder.colorConvert)
        {
            state->error = PNGColorModeCopy(&state->rawInfo, &state->pngInfo.colorMode);
            if (state->error) return state->error;
        }
    }
    else
    {
        uint8_t* data = *out;
        uint32_t outSize;

        if (!(state->rawInfo.colorType == LCT_RGB || state->rawInfo.colorType == LCT_RGBA) && !(state->rawInfo.bitDepth == 8))
        {
            return 56;
        }

        outSize = PNGGetRawSize(*w, *h, &state->rawInfo);
        *out = (uint8_t*)calloc(outSize, 1);
        if (!(*out))
        {
            state->error = 83;
        }
        else state->error = PNGConvert(*out, data, &state->rawInfo, &state->pngInfo.colorMode, *w, *h);
        free(data);
    }

    return state->error;
}

uint32_t writeSignature(ucVector* out)
{
    uint32_t pos = out->size;
    const uint8_t signature[] = {137, 80, 78, 71, 13, 10, 26, 10};
    if (!ucVectorResize(out, out->size + 8)) return 83;
    memcpy(out->data + pos, signature, 8);
    return 0;
}

uint32_t addChunk_IHDR(ucVector* out, uint32_t w, uint32_t h, PNGColorType colorType, uint32_t bitDepth, uint32_t interlaceMethod)
{
    uint8_t *chunk, *data;
    CERROR_TRY_RETURN(PNGChunkInit(&chunk, out, 13, "IHDR"));
    data = chunk + 8;

    PNGSet32bitInt(data + 0, w);
    PNGSet32bitInt(data + 4, h);
    data[8] = (uint8_t)bitDepth;
    data[9] = (uint8_t)colorType;
    data[10] = 0;
    data[11] = 0;
    data[12] = interlaceMethod;
    PNGChunkGenerateCRC(chunk);
    return 0;
}

uint32_t addChunk_PLTE(ucVector* out, const PNGColorMode* info)
{
    uint8_t* chunk;
    uint32_t i, j = 8;

    CERROR_TRY_RETURN(PNGChunkInit(&chunk, out, info->paletteSize * 3, "PLTE"));

    for (i = 0; i != info->paletteSize; ++i)
    {
        chunk[j++] = info->palette[i * 4 + 0];
        chunk[j++] = info->palette[i * 4 + 1];
        chunk[j++] = info->palette[i * 4 + 2];
    }

    PNGChunkGenerateCRC(chunk);
    return 0;
}

uint32_t addChunk_tRNS(ucVector* out, const PNGColorMode* info)
{
    uint8_t* chunk = 0;

    if (info->colorType == LCT_PALETTE)
    {
        uint32_t i, amount = info->paletteSize;
        for (i = info->paletteSize; i != 0; --i)
        {
            if (info->palette[4 * (i - 1) + 3] != 255) break;
            --amount;
        }

        if (amount)
        {
            CERROR_TRY_RETURN(PNGChunkInit(&chunk, out, amount, "tRNS"));
            for (i = 0; i != amount; ++i) chunk[8 + i] = info->palette[4 * i + 3];
        }
    }
    else if (info->colorType == LCT_GREY)
    {
        if (info->keyDefined)
        {
            CERROR_TRY_RETURN(PNGChunkInit(&chunk, out, 2, "tRNS"));
            chunk[8] = (uint8_t)(info->keyR >> 8);
            chunk[9] = (uint8_t)(info->keyR & 255);
        }
    }
    else if (info->colorType == LCT_RGB)
    {
        if (info->keyDefined)
        {
            CERROR_TRY_RETURN(PNGChunkInit(&chunk, out, 6, "tRNS"));
            chunk[8] = (uint8_t)(info->keyR >> 8);
            chunk[9] = (uint8_t)(info->keyR & 255);
            chunk[10] = (uint8_t)(info->keyG >> 8);
            chunk[11] = (uint8_t)(info->keyG & 255);
            chunk[12] = (uint8_t)(info->keyB >> 8);
            chunk[13] = (uint8_t)(info->keyB & 255);
        }
    }

    if (chunk) PNGChunkGenerateCRC(chunk);
    return 0;
}

uint32_t addChunk_IDAT(ucVector* out, const uint8_t* data, uint32_t dataSize, PNGCompressSettings* zlibSettings)
{
    uint32_t error = 0;
    uint8_t* zlib = 0;
    uint32_t zlibSize = 0;

    error = zlibCompress(&zlib, &zlibSize, data, dataSize, zlibSettings);
    if (!error)
    {
        error = PNGChunkCreatev(out, zlibSize, "IDAT", zlib);
    }

    free(zlib);
    return error;
}

uint32_t addChunk_IEND(ucVector* out)
{
    return PNGChunkCreatev(out, 0, "IEND", 0);
}

uint32_t addChunk_tEXt(ucVector* out, const char* keyword, const char* textString)
{
    uint8_t* chunk = 0;
    uint32_t keySize = strlen(keyword), textSize = strlen(textString);
    uint32_t size = keySize + 1 + textSize;
    if (keySize < 1 || keySize > 79) return 89;

    CERROR_TRY_RETURN(PNGChunkInit(&chunk, out, size, "tEXt"));
    memcpy(chunk + 8, keyword, keySize);
    chunk[8 + keySize] = 0;
    memcpy(chunk + 9 + keySize, textString, textSize);
    PNGChunkGenerateCRC(chunk);
    return 0;
}

uint32_t addChunk_zTXt(ucVector* out, const char* keyword, const char* textString, PNGCompressSettings* zlibSettings)
{
    uint32_t error = 0;
    uint8_t* chunk = 0;
    uint8_t* compressed = 0;
    uint32_t compressedSize = 0;
    uint32_t textSize = strlen(textString);
    uint32_t keySize = strlen(keyword);
    if (keySize < 1 || keySize > 79) return 89;

    error = zlibCompress(&compressed, &compressedSize, (const uint8_t*)textString, textSize, zlibSettings);
    if (!error)
    {
        uint32_t size = keySize + 2 + compressedSize;
        error = PNGChunkInit(&chunk, out, size, "zTXt");
    }

    if (!error)
    {
        memcpy(chunk + 8, keyword, keySize);
        chunk[8 + keySize] = 0;
        chunk[9 + keySize] = 0;
        memcpy(chunk + 10 + keySize, compressed, compressedSize);
        PNGChunkGenerateCRC(chunk);
    }

    free(compressed);
    return error;
}

uint32_t addChunk_iTXt(ucVector* out, uint32_t compress, const char* keyword, const char* langtag, const char* transkey, const char* textString, PNGCompressSettings* zlibSettings)
{
    uint32_t error = 0;
    uint8_t* chunk = 0;
    uint8_t* compressed = 0;
    uint32_t compressedSize = 0;
    uint32_t textSize = strlen(textString);
    uint32_t keySize = strlen(keyword), langsize = strlen(langtag), transsize = strlen(transkey);

    if (keySize < 1 || keySize > 79) return 89;

    if (compress)
    {
        error = zlibCompress(&compressed, &compressedSize, (const uint8_t*)textString, textSize, zlibSettings);
    }

    if (!error)
    {
        uint32_t size = keySize + 3 + langsize + 1 + transsize + 1 + (compress ? compressedSize : textSize);
        error = PNGChunkInit(&chunk, out, size, "iTXt");
    }

    if (!error)
    {
        uint32_t pos = 8;
        memcpy(chunk + pos, keyword, keySize);
        pos += keySize;
        chunk[pos++] = 0;
        chunk[pos++] = (compress ? 1 : 0);
        chunk[pos++] = 0;
        memcpy(chunk + pos, langtag, langsize);
        pos += langsize;
        chunk[pos++] = 0;
        memcpy(chunk + pos, transkey, transsize);
        pos += transsize;
        chunk[pos++] = 0;
        if (compress)
        {
            memcpy(chunk + pos, compressed, compressedSize);
        }
        else
        {
            memcpy(chunk + pos, textString, textSize);
        }
        PNGChunkGenerateCRC(chunk);
    }

    free(compressed);
    return error;
}

uint32_t addChunk_bKGD(ucVector* out, const PNGInfo* info)
{
    uint8_t* chunk = 0;
    if (info->colorMode.colorType == LCT_GREY || info->colorMode.colorType == LCT_GREY_ALPHA)
    {
        CERROR_TRY_RETURN(PNGChunkInit(&chunk, out, 2, "bKGD"));
        chunk[8] = (uint8_t)(info->backgroundR >> 8);
        chunk[9] = (uint8_t)(info->backgroundR & 255);
    }
    else if (info->colorMode.colorType == LCT_RGB || info->colorMode.colorType == LCT_RGBA)
    {
        CERROR_TRY_RETURN(PNGChunkInit(&chunk, out, 6, "bKGD"));
        chunk[8] = (uint8_t)(info->backgroundR >> 8);
        chunk[9] = (uint8_t)(info->backgroundR & 255);
        chunk[10] = (uint8_t)(info->backgroundG >> 8);
        chunk[11] = (uint8_t)(info->backgroundG & 255);
        chunk[12] = (uint8_t)(info->backgroundB >> 8);
        chunk[13] = (uint8_t)(info->backgroundB & 255);
    }
    else if (info->colorMode.colorType == LCT_PALETTE)
    {
        CERROR_TRY_RETURN(PNGChunkInit(&chunk, out, 1, "bKGD"));
        chunk[8] = (uint8_t)(info->backgroundR & 255);
    }

    if (chunk) PNGChunkGenerateCRC(chunk);
    return 0;
}

uint32_t addChunk_tIME(ucVector* out, const PNGTime* time)
{
    uint8_t* chunk;
    CERROR_TRY_RETURN(PNGChunkInit(&chunk, out, 7, "tIME"));
    chunk[8] = (uint8_t)(time->year >> 8);
    chunk[9] = (uint8_t)(time->year & 255);
    chunk[10] = (uint8_t)time->month;
    chunk[11] = (uint8_t)time->day;
    chunk[12] = (uint8_t)time->hour;
    chunk[13] = (uint8_t)time->minute;
    chunk[14] = (uint8_t)time->second;
    PNGChunkGenerateCRC(chunk);
    return 0;
}

uint32_t addChunk_pHYs(ucVector* out, const PNGInfo* info)
{
    uint8_t* chunk;
    CERROR_TRY_RETURN(PNGChunkInit(&chunk, out, 9, "pHYs"));
    PNGSet32bitInt(chunk + 8, info->physX);
    PNGSet32bitInt(chunk + 12, info->physY);
    chunk[16] = info->physUnit;
    PNGChunkGenerateCRC(chunk);
    return 0;
}

uint32_t addChunk_gAMA(ucVector* out, const PNGInfo* info)
{
    uint8_t* chunk;
    CERROR_TRY_RETURN(PNGChunkInit(&chunk, out, 4, "gAMA"));
    PNGSet32bitInt(chunk + 8, info->gamaGamma);
    PNGChunkGenerateCRC(chunk);
    return 0;
}

uint32_t addChunk_cHRM(ucVector* out, const PNGInfo* info)
{
    uint8_t* chunk;
    CERROR_TRY_RETURN(PNGChunkInit(&chunk, out, 32, "cHRM"));
    PNGSet32bitInt(chunk + 8, info->chrmWhiteX);
    PNGSet32bitInt(chunk + 12, info->chrmWhiteY);
    PNGSet32bitInt(chunk + 16, info->chrmRedX);
    PNGSet32bitInt(chunk + 20, info->chrmRedY);
    PNGSet32bitInt(chunk + 24, info->chrmGreenX);
    PNGSet32bitInt(chunk + 28, info->chrmGreenY);
    PNGSet32bitInt(chunk + 32, info->chrmBlueX);
    PNGSet32bitInt(chunk + 36, info->chrmBlueY);
    PNGChunkGenerateCRC(chunk);
    return 0;
}

uint32_t addChunk_sRGB(ucVector* out, const PNGInfo* info)
{
    uint8_t data = info->srgbIntent;
    return PNGChunkCreatev(out, 1, "sRGB", &data);
}

uint32_t addChunk_iCCP(ucVector* out, const PNGInfo* info, PNGCompressSettings* zlibSettings)
{
    uint32_t error = 0;
    uint8_t* chunk = 0;
    uint8_t* compressed = 0;
    uint32_t compressedSize = 0;
    uint32_t keySize = strlen(info->iccpName);

    if (keySize < 1 || keySize > 79) return 89;
    error = zlibCompress(&compressed, &compressedSize, info->iccpProfile, info->iccpProfileSize, zlibSettings);
    if (!error)
    {
        uint32_t size = keySize + 2 + compressedSize;
        error = PNGChunkInit(&chunk, out, size, "iCCP");
    }

    if (!error)
    {
        memcpy(chunk + 8, info->iccpName, keySize);
        chunk[8 + keySize] = 0;
        chunk[9 + keySize] = 0;
        memcpy(chunk + 10 + keySize, compressed, compressedSize);
        PNGChunkGenerateCRC(chunk);
    }

    free(compressed);
    return error;
}

void filterScanline(uint8_t* out, const uint8_t* scanline, const uint8_t* prevLine, uint32_t length, uint32_t byteWidth, uint8_t filterType)
{
    uint32_t i;
    switch (filterType)
    {
    case 0:
        for (i = 0; i != length; ++i) out[i] = scanline[i];
        break;

    case 1:
        for (i = 0; i != byteWidth; ++i) out[i] = scanline[i];
        for (i = byteWidth; i < length; ++i) out[i] = scanline[i] - scanline[i - byteWidth];
        break;

    case 2:
        if (prevLine)
        {
            for (i = 0; i != length; ++i) out[i] = scanline[i] - prevLine[i];
        }
        else
        {
            for (i = 0; i != length; ++i) out[i] = scanline[i];
        }
        break;

    case 3:
        if (prevLine)
        {
            for (i = 0; i != byteWidth; ++i) out[i] = scanline[i] - (prevLine[i] >> 1);
            for (i = byteWidth; i < length; ++i) out[i] = scanline[i] - ((scanline[i - byteWidth] + prevLine[i]) >> 1);
        }
        else
        {
            for (i = 0; i != byteWidth; ++i) out[i] = scanline[i];
            for (i = byteWidth; i < length; ++i) out[i] = scanline[i] - (scanline[i - byteWidth] >> 1);
        }
        break;

    case 4:
        if (prevLine)
        {
            for (i = 0; i != byteWidth; ++i) out[i] = (scanline[i] - prevLine[i]);
            for (i = byteWidth; i < length; ++i)
            {
                out[i] = (scanline[i] - paethPredictor(scanline[i - byteWidth], prevLine[i], prevLine[i - byteWidth]));
            }
        }
        else
        {
            for (i = 0; i != byteWidth; ++i) out[i] = scanline[i];
            for (i = byteWidth; i < length; ++i) out[i] = (scanline[i] - scanline[i - byteWidth]);
        }
        break;

    default:
        return;
    }
}

uint32_t ilog2(uint32_t i)
{
    uint32_t result = 0;
    if (i >= 65536) { result += 16; i >>= 16; }
    if (i >= 256) { result += 8; i >>= 8; }
    if (i >= 16) { result += 4; i >>= 4; }
    if (i >= 4) { result += 2; i >>= 2; }
    if (i >= 2) { result += 1;}
    return result;
}

uint32_t ilog2i(uint32_t i)
{
    uint32_t l;
    if (i == 0) return 0;
    l = ilog2(i);
    return i * l + ((i - (1u << l)) << 1u);
}

uint32_t filter(uint8_t* out, const uint8_t* in, uint32_t w, uint32_t h, const PNGColorMode* color, const PNGEncoderSettings* settings)
{
    uint32_t bpp = PNGGetBpp(color);
    uint32_t lineBytes = PNGGetRawSizeIDAT(w, 1, bpp) - 1u;

    uint32_t byteWidth = (bpp + 7u) / 8u;
    const uint8_t* prevLine = 0;
    uint32_t x, y;
    uint32_t error = 0;
    PNGFilterStrategy strategy = settings->fillterStrategy;

    if (settings->fillterPaletteZero && (color->colorType == LCT_PALETTE || color->bitDepth < 8)) strategy = LFS_ZERO;

    if (bpp == 0) return 31;

    if (strategy >= LFS_ZERO && strategy <= LFS_FOUR)
    {
        uint8_t type = (uint8_t)strategy;
        for (y = 0; y != h; ++y)
        {
            uint32_t outindex = (1 + lineBytes) * y;
            uint32_t inindex = lineBytes * y;
            out[outindex] = type;
            filterScanline(&out[outindex + 1], &in[inindex], prevLine, lineBytes, byteWidth, type);
            prevLine = &in[inindex];
        }
    }
    else if (strategy == LFS_MINSUM)
    {
        uint8_t* attempt[5];
        uint32_t smallest = 0;
        uint8_t type, bestType = 0;

        for (type = 0; type != 5; ++type)
        {
            attempt[type] = (uint8_t*)calloc(lineBytes, 1);
            if (!attempt[type]) error = 83;
        }

        if (!error)
        {
            for (y = 0; y != h; ++y)
            {
                for (type = 0; type != 5; ++type)
                {
                    uint32_t sum = 0;
                    filterScanline(attempt[type], &in[y * lineBytes], prevLine, lineBytes, byteWidth, type);

                    if (type == 0)
                    {
                        for (x = 0; x != lineBytes; ++x) sum += (uint8_t)(attempt[type][x]);
                    }
                    else
                    {
                        for (x = 0; x != lineBytes; ++x)
                        {
                            uint8_t s = attempt[type][x];
                            sum += s < 128 ? s : (255U - s);
                        }
                    }

                    if (type == 0 || sum < smallest)
                    {
                        bestType = type;
                        smallest = sum;
                    }
                }

                prevLine = &in[y * lineBytes];

                out[y * (lineBytes + 1)] = bestType;
                for (x = 0; x != lineBytes; ++x) out[y * (lineBytes + 1) + 1 + x] = attempt[bestType][x];
            }
        }

        for (type = 0; type != 5; ++type) free(attempt[type]);
    }
    else if (strategy == LFS_ENTROPY)
    {
        uint8_t* attempt[5];
        uint32_t bestSum = 0;
        uint32_t type, bestType = 0;
        uint32_t count[256];

        for (type = 0; type != 5; ++type)
        {
            attempt[type] = (uint8_t*)calloc(lineBytes, 1);
            if (!attempt[type]) error = 83;
        }

        if (!error)
        {
            for (y = 0; y != h; ++y)
            {
                for (type = 0; type != 5; ++type)
                {
                    uint32_t sum = 0;
                    filterScanline(attempt[type], &in[y * lineBytes], prevLine, lineBytes, byteWidth, type);
                    memset(count, 0, 256 * sizeof(*count));
                    for (x = 0; x != lineBytes; ++x) ++count[attempt[type][x]];
                    ++count[type];
                    for (x = 0; x != 256; ++x)
                    {
                        sum += ilog2i(count[x]);
                    }
                    
                    if (type == 0 || sum > bestSum)
                    {
                        bestType = type;
                        bestSum = sum;
                    }
                }

                prevLine = &in[y * lineBytes];

                out[y * (lineBytes + 1)] = bestType;
                for (x = 0; x != lineBytes; ++x) out[y * (lineBytes + 1) + 1 + x] = attempt[bestType][x];
            }
        }

        for (type = 0; type != 5; ++type) free(attempt[type]);
    }
    else if (strategy == LFS_PREDEFINED)
    {
        for (y = 0; y != h; ++y)
        {
            uint32_t outindex = (1 + lineBytes) * y;
            uint32_t inindex = lineBytes * y;
            uint8_t type = settings->predefinedFilters[y];
            out[outindex] = type;
            filterScanline(&out[outindex + 1], &in[inindex], prevLine, lineBytes, byteWidth, type);
            prevLine = &in[inindex];
        }
    }
    else if (strategy == LFS_BRUTE_FORCE)
    {
        uint32_t size[5];
        uint8_t* attempt[5];
        uint32_t smallest = 0;
        uint32_t type = 0, bestType = 0;
        uint8_t* dummy;
        PNGCompressSettings zlibSettings;
        memcpy(&zlibSettings, &settings->zlibSettings, sizeof(PNGCompressSettings));
        zlibSettings.btype = 1;
        zlibSettings.customZlib = 0;
        zlibSettings.customDeflate = 0;

        for (type = 0; type != 5; ++type)
        {
            attempt[type] = (uint8_t*)calloc(lineBytes, 1);
            if (!attempt[type]) error = 83;
        }

        if (!error)
        {
            for (y = 0; y != h; ++y)
            {
                for (type = 0; type != 5; ++type)
                {
                    uint32_t testsize = (uint32_t)lineBytes;
                    filterScanline(attempt[type], &in[y * lineBytes], prevLine, lineBytes, byteWidth, type);
                    size[type] = 0;
                    dummy = 0;
                    zlibCompress(&dummy, &size[type], attempt[type], testsize, &zlibSettings);
                    free(dummy);
                    if (type == 0 || size[type] < smallest)
                    {
                        bestType = type;
                        smallest = size[type];
                    }
                }

                prevLine = &in[y * lineBytes];
                out[y * (lineBytes + 1)] = bestType;
                for (x = 0; x != lineBytes; ++x) out[y * (lineBytes + 1) + 1 + x] = attempt[bestType][x];
            }
        }

        for (type = 0; type != 5; ++type) free(attempt[type]);
    }
    else return 88;
    return error;
}

void addPaddingBits(uint8_t* out, const uint8_t* in, uint32_t olineBits, uint32_t ilineBits, uint32_t h)
{
    uint32_t y;
    uint32_t diff = olineBits - ilineBits;
    uint32_t obp = 0, ibp = 0;

    for (y = 0; y != h; ++y)
    {
        uint32_t x;
        for (x = 0; x < ilineBits; ++x)
        {
            uint8_t bit = readBitFromReversedStream(&ibp, in);
            setBitOfReversedStream(&obp, out, bit);
        }

        for (x = 0; x != diff; ++x) setBitOfReversedStream(&obp, out, 0);
    }
}

void adam7Interlace(uint8_t* out, const uint8_t* in, uint32_t w, uint32_t h, uint32_t bpp)
{
    uint32_t passw[7], passh[7];
    uint32_t filterPassStart[8], paddedPassStart[8], passStart[8];
    uint32_t i;

    adam7GetPassValues(passw, passh, filterPassStart, paddedPassStart, passStart, w, h, bpp);

    if (bpp >= 8)
    {
        for (i = 0; i != 7; ++i)
        {
            uint32_t x, y, b;
            uint32_t byteWidth = bpp / 8u;

            for (y = 0; y < passh[i]; ++y)
            for (x = 0; x < passw[i]; ++x)
            {
                uint32_t pixelinstart = ((ADAM7_IY[i] + y * ADAM7_DY[i]) * w + ADAM7_IX[i] + x * ADAM7_DX[i]) * byteWidth;
                uint32_t pixeloutstart = passStart[i] + (y * passw[i] + x) * byteWidth;
                for (b = 0; b < byteWidth; ++b)
                {
                    out[pixeloutstart + b] = in[pixelinstart + b];
                }
            }
        }
    }
    else
    {
        for (i = 0; i != 7; ++i)
        {
            uint32_t x, y, b;
            uint32_t ilineBits = bpp * passw[i];
            uint32_t olineBits = bpp * w;
            uint32_t obp, ibp;

            for (y = 0; y < passh[i]; ++y)
            for (x = 0; x < passw[i]; ++x)
            {
                ibp = (ADAM7_IY[i] + y * ADAM7_DY[i]) * olineBits + (ADAM7_IX[i] + x * ADAM7_DX[i]) * bpp;
                obp = (8 * passStart[i]) + (y * ilineBits + x * bpp);
                for (b = 0; b < bpp; ++b)
                {
                    uint8_t bit = readBitFromReversedStream(&ibp, in);
                    setBitOfReversedStream(&obp, out, bit);
                }
            }
        }
    }
}

uint32_t preProcessScanlines(uint8_t** out, uint32_t* outSize, const uint8_t* in, uint32_t w, uint32_t h, const PNGInfo* pngInfo, const PNGEncoderSettings* settings)
{
    uint32_t error = 0;
    uint32_t bpp = PNGGetBpp(&pngInfo->colorMode);
    
    if (pngInfo->interlaceMethod == 0)
    {
        *outSize = h + (h * ((w * bpp + 7u) / 8u));
        *out = (uint8_t*)calloc(*outSize, 1);
        if (!(*out) && (*outSize)) error = 83;

        if (!error)
        {
            if (bpp < 8 && w * bpp != ((w * bpp + 7u) / 8u) * 8u)
            {
                uint8_t* padded = (uint8_t*)calloc(h * ((w * bpp + 7u) / 8u), 1);
                if (!padded) error = 83;
                if (!error)
                {
                    addPaddingBits(padded, in, ((w * bpp + 7u) / 8u) * 8u, w * bpp, h);
                    error = filter(*out, padded, w, h, &pngInfo->colorMode, settings);
                }
                free(padded);
            }
            else
            {
                error = filter(*out, in, w, h, &pngInfo->colorMode, settings);
            }
        }
    }
    else
    {
        uint8_t* adam7;
        uint32_t passw[7], passh[7];
        uint32_t filterPassStart[8], paddedPassStart[8], passStart[8];
        
        adam7GetPassValues(passw, passh, filterPassStart, paddedPassStart, passStart, w, h, bpp);

        *outSize = filterPassStart[7];
        *out = (uint8_t*)calloc(*outSize, 1);
        if (!(*out)) error = 83;

        adam7 = (uint8_t*)calloc(passStart[7], 1);
        if (!adam7 && passStart[7]) error = 83;

        if (!error)
        {
            uint32_t i;
            adam7Interlace(adam7, in, w, h, bpp);

            for (i = 0; i != 7; ++i)
            {
                if (bpp < 8)
                {
                    uint8_t* padded = (uint8_t*)calloc(paddedPassStart[i + 1] - paddedPassStart[i], 1);
                    if (!padded) ERROR_BREAK(83);
                    addPaddingBits(padded, &adam7[passStart[i]], ((passw[i] * bpp + 7u) / 8u) * 8u, passw[i] * bpp, passh[i]);
                    error = filter(&(*out)[filterPassStart[i]], padded, passw[i], passh[i], &pngInfo->colorMode, settings);
                    free(padded);
                }
                else
                {
                    error = filter(&(*out)[filterPassStart[i]], &adam7[paddedPassStart[i]], passw[i], passh[i], &pngInfo->colorMode, settings);
                }

                if (error) break;
            }
        }

        free(adam7);
    }

    return error;
}

uint32_t addUnknownChunks(ucVector* out, uint8_t* data, uint32_t dataSize)
{
    uint8_t* inchunk = data;
    while ((uint32_t)(inchunk - data) < dataSize)
    {
        CERROR_TRY_RETURN(PNGChunkAppend(&out->data, &out->size, inchunk));
        out->allocSize = out->size;
        inchunk = PNGChunkNext(inchunk, data + dataSize);
    }
    return 0;
}

uint32_t isGrayICCProfile(const uint8_t* profile, uint32_t size)
{
    if (size < 20) return 0;
    return profile[16] == 'G' &&  profile[17] == 'R' &&  profile[18] == 'A' &&  profile[19] == 'Y';
}

uint32_t isRGBICCProfile(const uint8_t* profile, uint32_t size)
{
    if (size < 20) return 0;
    return profile[16] == 'R' &&  profile[17] == 'G' &&  profile[18] == 'B' &&  profile[19] == ' ';
}

uint32_t PNGEncode(uint8_t** out, uint32_t* outSize, const uint8_t* image, uint32_t w, uint32_t h, PNGState* state)
{
    uint32_t i = 0;
    uint8_t* data = NULL;
    uint32_t dataSize = 0;
    ucVector outv = ucVectorInit(NULL, 0);

    PNGInfo info;
    const PNGInfo* pngInfo = &state->pngInfo;

    PNGInfoInit(&info);

    *out = 0;
    *outSize = 0;
    state->error = 0;

    if ((pngInfo->colorMode.colorType == LCT_PALETTE || state->encoder.forcePalette) && (pngInfo->colorMode.paletteSize == 0 || pngInfo->colorMode.paletteSize > 256))
    {
        state->error = 68;
        goto cleanup;
    }

    if (state->encoder.zlibSettings.btype > 2)
    {
        state->error = 61;
        goto cleanup;
    }

    if (pngInfo->interlaceMethod > 1)
    {
        state->error = 71;
        goto cleanup;
    }

    state->error = checkColorValidity(pngInfo->colorMode.colorType, pngInfo->colorMode.bitDepth);
    if (state->error) goto cleanup;

    state->error = checkColorValidity(state->rawInfo.colorType, state->rawInfo.bitDepth);
    if (state->error) goto cleanup;

    PNGInfoCopy(&info, &state->pngInfo);

    if (state->encoder.autoConvert)
    {
        PNGColorStats stats;
        PNGColorStatsInit(&stats);

        if (pngInfo->iccpDefined && isGrayICCProfile(pngInfo->iccpProfile, pngInfo->iccpProfileSize))
        {
            stats.allowPalette = 0;
        }

        if (pngInfo->iccpDefined && isRGBICCProfile(pngInfo->iccpProfile, pngInfo->iccpProfileSize))
        {
            stats.allowGreyscale = 0;
        }

        state->error = PNGComputeColorStats(&stats, image, w, h, &state->rawInfo);
        if (state->error) goto cleanup;

        if (pngInfo->backgroundDefined)
        {
            uint32_t r = 0, g = 0, b = 0;
            PNGColorMode mode16 = PNGColorModeMake(LCT_RGB, 16);
            PNGConvertRGB(&r, &g, &b, pngInfo->backgroundR, pngInfo->backgroundG, pngInfo->backgroundB, &mode16, &pngInfo->colorMode);
            state->error = PNGColorStatsAdd(&stats, r, g, b, 65535);
            if (state->error) goto cleanup;
        }

        state->error = autoChooseColor(&info.colorMode, &state->rawInfo, &stats);
        if (state->error) goto cleanup;

        if (pngInfo->backgroundDefined)
        {
            if (PNGConvertRGB(&info.backgroundR, &info.backgroundG, &info.backgroundB, pngInfo->backgroundR, pngInfo->backgroundG, pngInfo->backgroundB, &info.colorMode, &pngInfo->colorMode))
            {
                state->error = 104;
                goto cleanup;
            }
        }
    }

    if (pngInfo->iccpDefined)
    {
        uint32_t grayicc = isGrayICCProfile(pngInfo->iccpProfile, pngInfo->iccpProfileSize);
        uint32_t rgbicc = isRGBICCProfile(pngInfo->iccpProfile, pngInfo->iccpProfileSize);
        uint32_t graypng = info.colorMode.colorType == LCT_GREY || info.colorMode.colorType == LCT_GREY_ALPHA;

        if (!grayicc && !rgbicc)
        {
            state->error = 100;
            goto cleanup;
        }

        if (grayicc != graypng)
        {
            state->error = state->encoder.autoConvert ? 102 : 101;
            goto cleanup;
        }
    }
    
    if (!PNGColorModeEqual(&state->rawInfo, &info.colorMode))
    {
        uint8_t* converted;
        uint32_t size = (w * h * PNGGetBpp(&info.colorMode) + 7u) / 8u;

        converted = (uint8_t*)calloc(size, 1);
        if (!converted && size) state->error = 83;

        if (!state->error)
        {
            state->error = PNGConvert(converted, image, &info.colorMode, &state->rawInfo, w, h);
        }

        if (!state->error)
        {
            state->error = preProcessScanlines(&data, &dataSize, converted, w, h, &info, &state->encoder);
        }

        free(converted);
        if (state->error) goto cleanup;
    }
    else
    {
        state->error = preProcessScanlines(&data, &dataSize, image, w, h, &info, &state->encoder);
        if (state->error) goto cleanup;
    }

    state->error = writeSignature(&outv);
    if (state->error) goto cleanup;

    state->error = addChunk_IHDR(&outv, w, h, info.colorMode.colorType, info.colorMode.bitDepth, info.interlaceMethod);
    if (state->error) goto cleanup;

    if (info.unknownChunksData[0])
    {
        state->error = addUnknownChunks(&outv, info.unknownChunksData[0], info.unknownChunksSize[0]);
        if (state->error) goto cleanup;
    }

    if (info.iccpDefined)
    {
        state->error = addChunk_iCCP(&outv, &info, &state->encoder.zlibSettings);
        if (state->error) goto cleanup;
    }

    if (info.srgbDefined)
    {
        state->error = addChunk_sRGB(&outv, &info);
        if (state->error) goto cleanup;
    }

    if (info.gamaDefined)
    {
        state->error = addChunk_gAMA(&outv, &info);
        if (state->error) goto cleanup;
    }

    if (info.chrmDefined)
    {
        state->error = addChunk_cHRM(&outv, &info);
        if (state->error) goto cleanup;
    }

    if (info.colorMode.colorType == LCT_PALETTE)
    {
        state->error = addChunk_PLTE(&outv, &info.colorMode);
        if (state->error) goto cleanup;
    }

    if (state->encoder.forcePalette && (info.colorMode.colorType == LCT_RGB || info.colorMode.colorType == LCT_RGBA))
    {
        state->error = addChunk_PLTE(&outv, &info.colorMode);
        if (state->error) goto cleanup;
    }

    state->error = addChunk_tRNS(&outv, &info.colorMode);
    if (state->error) goto cleanup;

    if (info.backgroundDefined)
    {
        state->error = addChunk_bKGD(&outv, &info);
        if (state->error) goto cleanup;
    }

    if (info.physDefined)
    {
        state->error = addChunk_pHYs(&outv, &info);
        if (state->error) goto cleanup;
    }

    if (info.unknownChunksData[1])
    {
        state->error = addUnknownChunks(&outv, info.unknownChunksData[1], info.unknownChunksSize[1]);
        if (state->error) goto cleanup;
    }

    state->error = addChunk_IDAT(&outv, data, dataSize, &state->encoder.zlibSettings);
    if (state->error) goto cleanup;

    if (info.timeDefined)
    {
        state->error = addChunk_tIME(&outv, &info.time);
        if (state->error) goto cleanup;
    }

    for (i = 0; i != info.textNum; ++i)
    {
        if (strlen(info.textKeys[i]) > 79)
        {
            state->error = 66;
            goto cleanup;
        }

        if (strlen(info.textKeys[i]) < 1)
        {
            state->error = 67;
            goto cleanup;
        }

        if (state->encoder.textCompression)
        {
            state->error = addChunk_zTXt(&outv, info.textKeys[i], info.textStrings[i], &state->encoder.zlibSettings);
            if (state->error) goto cleanup;
        }
        else
        {
            state->error = addChunk_tEXt(&outv, info.textKeys[i], info.textStrings[i]);
            if (state->error) goto cleanup;
        }
    }

    if (state->encoder.addID)
    {
        uint32_t alreadyAddedIdText = 0;
        for (i = 0; i != info.textNum; ++i)
        {
            const char* k = info.textKeys[i];
            if (k[0] == 'L' && k[1] == 'o' && k[2] == 'd' && k[3] == 'e' && k[4] == 'P' && k[5] == 'N' && k[6] == 'G' && k[7] == '\0')
            {
                alreadyAddedIdText = 1;
                break;
            }
        }

        if (alreadyAddedIdText == 0)
        {
            state->error = addChunk_tEXt(&outv, "PNG", PNG_VERSION_STRING);
            if (state->error) goto cleanup;
        }
    }

    for (i = 0; i != info.itextNum; ++i)
    {
        if (strlen(info.itextKeys[i]) > 79)
        {
            state->error = 66;
            goto cleanup;
        }

        if (strlen(info.itextKeys[i]) < 1)
        {
            state->error = 67;
            goto cleanup;
        }

        state->error = addChunk_iTXt(&outv, state->encoder.textCompression, info.itextKeys[i], info.itextLangTags[i], info.itextTransKeys[i], info.itextStrings[i], &state->encoder.zlibSettings);
        if (state->error) goto cleanup;
    }

    if (info.unknownChunksData[2])
    {
        state->error = addUnknownChunks(&outv, info.unknownChunksData[2], info.unknownChunksSize[2]);
        if (state->error) goto cleanup;
    }

    state->error = addChunk_IEND(&outv);
    if (state->error) goto cleanup;

    cleanup:
    PNGInfoCleanup(&info);
    free(data);

    *out = outv.data;
    *outSize = outv.size;

    return state->error;
}

void PNGEncoderSettingsInit(PNGEncoderSettings* settings)
{
    PNGCompressSettingsInit(&settings->zlibSettings);
    settings->fillterPaletteZero = 1;
    settings->fillterStrategy = LFS_MINSUM;
    settings->autoConvert = 1;
    settings->forcePalette = 0;
    settings->predefinedFilters = 0;
    settings->addID = 0;
    settings->textCompression = 1;
}

void PNGDecoderSettingsInit(PNGDecoderSettings* settings)
{
    settings->colorConvert = 1;
    settings->readTextChunks = 1;
    settings->rememberUnknownChunks = 0;
    settings->maxTextSize = 16777216;
    settings->maxIccSize = 16777216;
    settings->ignoreCRC = 0;
    settings->ignoreCritical = 0;
    settings->ignoreEnd = 0;
    PNGDecompressSettingsInit(&settings->zlibSettings);
}

void PNGStateInit(PNGState* state)
{
    PNGDecoderSettingsInit(&state->decoder);
    PNGEncoderSettingsInit(&state->encoder);
    PNGColorModeInit(&state->rawInfo);
    PNGInfoInit(&state->pngInfo);
    state->error = 1;
}

void PNGStateCleanup(PNGState* state)
{
    PNGColorModeCleanup(&state->rawInfo);
    PNGInfoCleanup(&state->pngInfo);
}

// Load PNG file as GFX_IMAGE texture
uint32_t loadPNG(const char *fname, GFX_IMAGE *img)
{
    FILE *fp;

    PNGState state;
    uint32_t error = 0;
    uint32_t size = 0;
    uint8_t *buffer = NULL;

    fp = fopen(fname, "rb");
    if (!fp) return 83;

    size = getFileSize(fp);
    if (!size) return 83;

    buffer = (uint8_t*)calloc(size, 1);
    if (!buffer) return 83;

    fread(buffer, 1, size, fp);
    fclose(fp);

    PNGStateInit(&state);
    state.rawInfo.colorType = LCT_RGBA;
    state.rawInfo.bitDepth = 8;
    state.decoder.readTextChunks = 0;
    state.decoder.rememberUnknownChunks = 0;
    error = PNGDecode(&img->mData, &img->mWidth, &img->mHeight, &state, buffer, size);
    PNGStateCleanup(&state);
    
    img->mPixels   = 4;
    img->mSize     = img->mWidth * img->mHeight * img->mPixels;
    img->mRowBytes = img->mWidth * img->mPixels;

    free(buffer);

    buffer = img->mData;
    size   = img->mWidth * img->mHeight;

    // Make image buffer as texture (swap r, b to match screen ARGB)
    __asm {
        mov     edi, buffer
        mov     ecx, size
    next:
        mov     al, [edi]
        xchg    al, [edi + 2]
        mov     [edi], al
        add     edi, 4
        dec     ecx
        jnz     next
    }

    return error;
}

uint32_t savePNG(const char* fname, GFX_IMAGE *img)
{
    FILE *fp;

    uint32_t error = 0;
    uint8_t* buffer = NULL;
    uint32_t size = 0;

    PNGState state;
    PNGColorType colorType = (img->mPixels == 4) ?  LCT_RGBA : LCT_RGB;

    PNGStateInit(&state);
    state.rawInfo.colorType = colorType;
    state.rawInfo.bitDepth = 8;
    state.pngInfo.colorMode.colorType = colorType;
    state.pngInfo.colorMode.bitDepth = 8;
    PNGEncode(&buffer, &size, img->mData, img->mWidth, img->mHeight, &state);
    error = state.error;
    PNGStateCleanup(&state);

    if (error)
    {
        free(buffer);
        return error;
    }

    if (!(fp = fopen(fname, "wb")))
    {
        free(buffer);
        return 83;
    }

    fwrite(buffer, 1, size, fp);
    fclose(fp);

    free(buffer);
    return error;
}

const char* PNGErrorText(uint32_t code)
{
    switch (code)
    {
    case 0:  return "no error, everything went ok";
    case 1:  return "nothing done yet";
    case 10: return "end of src memory reached without huffman end code";
    case 11: return "error in code tree made it jump outside of huffman tree";
    case 13: return "problem while processing dynamic Deflate block";
    case 14: return "problem while processing dynamic Deflate block";
    case 15: return "problem while processing dynamic Deflate block";
    case 16: return "unexisting code while processing dynamic Deflate block";
    case 17: return "end of out buffer memory reached while inflating";
    case 18: return "invalid distance code while inflating";
    case 19: return "end of out buffer memory reached while inflating";
    case 20: return "invalid Deflate block btype encountered while decoding";
    case 21: return "nlen is not ones complement of len in a Deflate block";
    case 22: return "end of out buffer memory reached while inflating";
    case 23: return "end of in buffer memory reached while inflating";
    case 24: return "invalid FCHECK in zlib header";
    case 25: return "invalid compression method in zlib header";
    case 26: return "FDICT encountered in zlib header while it's not used for PNG";
    case 27: return "PNG file is smaller than a PNG header";
    case 28: return "incorrect PNG signature, it's no PNG or corrupted";
    case 29: return "first chunk is not the header chunk";
    case 30: return "chunk len too large, chunk broken off at end of file";
    case 31: return "illegal PNG colorMode type or bpp";
    case 32: return "illegal PNG compression method";
    case 33: return "illegal PNG Filter method";
    case 34: return "illegal PNG interlace method";
    case 35: return "chunk len of a chunk is too large or the chunk too small";
    case 36: return "illegal PNG Filter type encountered";
    case 37: return "illegal bit depth for this colorMode type given";
    case 38: return "the palette is too big";
    case 39: return "more palette alpha values given in tRNS chunk than there are colors in the palette";
    case 40: return "tRNS chunk has wrong size for greyscale data";
    case 41: return "tRNS chunk has wrong size for RGB data";
    case 42: return "tRNS chunk appeared while it was not allowed for this colorMode type";
    case 43: return "bKGD chunk has wrong size for palette data";
    case 44: return "bKGD chunk has wrong size for greyscale data";
    case 45: return "bKGD chunk has wrong size for RGB data";
    case 48: return "empty src or file doesn't exist";
    case 49: return "jumped past memory while generating dynamic huffman tree";
    case 50: return "jumped past memory while generating dynamic huffman tree";
    case 51: return "jumped past memory while inflating huffman block";
    case 52: return "jumped past memory while inflating";
    case 53: return "size of zlib data too small";
    case 54: return "repeat symbol in tree while there was no value symbol yet";
    case 55: return "jumped past tree while generating huffman tree";
    case 56: return "given dst data colorType or bitDepth not supported for colorMode conversion";
    case 57: return "invalid CRC encountered (checking CRC can be disabled)";
    case 58: return "invalid adler32 encountered (checking adler32 can be disabled)";
    case 59: return "requested colormode conversion not supported";
    case 60: return "invalid window size given in the settings of the encoder (must be 0-32768)";
    case 61: return "invalid btype given in the settings of the encoder (only 0, 1 and 2 are allowed)";
    case 62: return "conversion from colorMode to greyscale not supported";
    case 63: return "len of a chunk too long, max allowed for PNG is 2147483647 bytes per chunk";
    case 64: return "the len of the END symbol 256 in the Huffman tree is 0";
    case 66: return "the len of a text chunk keyword given to the encoder is longer than the maximum of 79 bytes";
    case 67: return "the len of a text chunk keyword given to the encoder is smaller than the minimum of 1 byte";
    case 68: return "tried to encode a PLTE chunk with a palette that has less than 1 or more than 256 colors";
    case 69: return "unknown chunk type with 'critical' flag encountered by the decoder";
    case 71: return "unexisting interlace mode given to encoder (must be 0 or 1)";
    case 72: return "while decoding, unexisting compression method encountering in zTXt or iTXt chunk (it must be 0)";
    case 73: return "invalid tIME chunk size";
    case 74: return "invalid pHYs chunk size";
    case 75: return "no null termination char found while decoding text chunk";
    case 76: return "iTXt chunk too short to contain required bytes";
    case 77: return "integer overflow in buffer size";
    case 78: return "failed to open file for reading";
    case 79: return "failed to open file for writing";
    case 80: return "tried creating a tree of 0 symbols";
    case 81: return "lazy matching at pos 0 is impossible";
    case 82: return "colormode conversion to palette requested while a colorMode isn't in palette";
    case 83: return "memory allocation failed";
    case 84: return "given data too small to contain all pixels to be encoded";
    case 86: return "impossible offset in lz77 encoding (internal bug)";
    case 87: return "must provide custom zlib function pointer if LODEPNG_COMPILE_ZLIB is not defined";
    case 88: return "invalid filter strategy given for PNGEncoderSettings.fillterStrategy";
    case 89: return "text chunk keyword too short or long: must have size 1-79";
    case 90: return "windowSize must be a power of two";
    case 91: return "invalid decompressed idat size";
    case 92: return "integer overflow due to too many pixels";
    case 93: return "zero bmWidth or bmHeight is invalid";
    case 94: return "header chunk must have a size of 13 bytes";
    case 95: return "integer overflow with combined idat chunk size";
    case 96: return "invalid gAMA chunk size";
    case 97: return "invalid cHRM chunk size";
    case 98: return "invalid sRGB chunk size";
    case 99: return "invalid sRGB rendering intent";
    case 100: return "invalid ICC profile color type, the PNG specification only allows RGB or GRAY";
    case 101: return "PNG specification does not allow RGB ICC profile on gray color types and vice versa";
    case 102: return "not allowed to set grayscale ICC profile with colored pixels by PNG specification";
    case 103: return "invalid palette index in bKGD chunk. Maybe it came before PLTE chunk?";
    case 104: return "invalid bKGD color while encoding (e.g. palette index out of range)";
    case 105: return "integer overflow of bitsize";
    case 106: return "PNG file must have PLTE chunk if color type is palette";
    case 107: return "color convert from palette mode requested without setting the palette data in it";
    case 108: return "tried to add more than 256 values to a palette";
    case 109: return "tried to decompress zlib or deflate data larger than desired max_output_size";
    case 110: return "custom zlib or inflate decompression failed";
    case 111: return "custom zlib or deflate compression failed";
    case 112: return "compressed text unreasonably large";
    case 113: return "ICC profile unreasonably large";    
    }

    return "Unknown error code";
}

void ColorTypeString(PNGColorType type, char *buff)
{
    char *name;
    switch (type)
    {
    case LCT_GREY:       name = "Grey"; break;
    case LCT_RGB:        name = "RGB"; break;
    case LCT_PALETTE:    name = "Palette"; break;
    case LCT_GREY_ALPHA: name = "Grey With Alpha"; break;
    case LCT_RGBA:       name = "ARGB"; break;
    default:             name = "Invalid"; break;
    }

    sprintf(buff, "%d (%s)", type, name);
}

// Display general info about the PNG.
void PNGShowInfo(PNGInfo* info)
{
    uint32_t i;
    char ctype[128] = {0};
    PNGColorMode colorMode = info->colorMode;

    printf("Compression Method: %u\n", info->compressionMethod);
    printf("Filter Method: %u\n", info->filterMethod);

    ColorTypeString(colorMode.colorType, ctype);

    printf("Color Type: %s\n", ctype);
    printf("Bit Depth: %u\n", colorMode.bitDepth);

    printf("Bits per Pixel: %u\n", PNGGetBpp(&colorMode));
    printf("Channels per Pixel: %u\n", PNGGetChannels(&colorMode));
    printf("Is Greyscale Type: %d\n", PNGIsGreyscaleType(&colorMode));
    printf("Can Have Alpha: %d\n", PNGCanHaveAlpha(&colorMode));

    printf("Palette Size: %u\n", colorMode.paletteSize);
    printf("Has Color Mode Key: %d\n", colorMode.keyDefined);

    if (colorMode.keyDefined)
    {
        printf("Color Key R: %d\n", colorMode.keyR);
        printf("Color Key G: %d\n", colorMode.keyG);
        printf("Color Key B: %d\n", colorMode.keyB);
    }

    printf("Interlace Method: %d\n", info->interlaceMethod);
    printf("Texts: %d\n", info->textNum);

    for (i = 0; i != info->textNum; i++)
    { 
        printf("Text: %s: %s\n", info->textKeys[i], info->textStrings[i]);
    }

    printf("International Texts: %u\n", info->itextNum);

    for (i = 0; i != info->itextNum; i++)
    {
        printf("Text: %s, %s, %s, %s\n", info->itextKeys[i], info->itextLangTags[i], info->itextTransKeys[i], info->itextStrings[i]);
    }

    printf("Time Defined: %u\n", info->timeDefined);

    if (info->timeDefined)
    {
        PNGTime time = info->time;
        printf("Year: %u\n", time.year);
        printf("Month: %u\n", time.month);
        printf("Day: %u\n", time.day);
        printf("Hour: %u\n", time.hour);
        printf("Minute: %u\n", time.minute);
        printf("Second: %u\n", time.second);
    }

    printf("Physics Defined: %u\n", info->physDefined);
    if (info->physDefined)
    {
        printf("Physics X: %u\n", info->physX);
        printf("Physics Y: %u\n", info->physY);
        printf("Physics Unit: %u\n", info->physUnit);
    }
}

void PNGFileInfo(const char* fname)
{
    FILE *fp;
    PNGState state;

    uint32_t w = 0, h = 0, error = 0;
    uint32_t inSize = 0;
    uint8_t* in = NULL;
    uint8_t* out = NULL;

    if (!(fp = fopen(fname, "rb"))) return;
    if (!(inSize = getFileSize(fp))) return;
    if (!(in = (uint8_t*)calloc(inSize, 1))) return;

    fread(in, inSize, 1, fp);
    fclose(fp);

    PNGStateInit(&state);
    state.rawInfo.colorType = LCT_RGBA;
    state.rawInfo.bitDepth = 8;
    error = PNGDecode(&out, &w, &h, &state, in, inSize);
    if (error)
    {
        printf("PNGFileInfo: PNG decode error: %s\n", PNGErrorText(error));
        free(in);
        free(out);
        PNGStateCleanup(&state);
        return;
    }

    printf("File Size: %u\n", inSize);
    printf("Width: %d\n", w);
    printf("Height: %d\n", h);
    printf("Num Pixels: %d\n", w * h);

    PNGShowInfo(&state.pngInfo); 
    PNGStateCleanup(&state);

    free(out);
    free(in);
}

// load image from file (support BMP, PNG)
// this use file extension to determine image type
int32_t loadImage(const char *fname, GFX_IMAGE *im)
{
    char *pos = strrchr(fname, '.');
    if (!pos) return 0;

    // load and convert bitmap to image (screen texture)
    if (!strcmpi(pos, ".bmp"))
    {
        GFX_BITMAP bm;
        if (!loadBitmap(fname, &bm)) return 0;
        convertBitmap(&bm, im);
        closeBitmap(&bm);
        return 1;
    }
    else if (!strcmpi(pos, ".png"))
    {
        if (loadPNG(fname, im)) return 0;
        return 1;
    }
    else fatalError("loadImage: load image unknown type!\n");
    return 0;
}

// Render PNG file (24/32 bits colors)
void showPNG(const char *file)
{
    GFX_IMAGE img = {0};
    uint32_t error = 0;
    uint8_t *pngData = NULL;
    uint32_t pngWidth = 0, pngHeight = 0;

    // try to load png file
    error = loadPNG(file, &img);
    if (error) fatalError("showPNG: cannot load PNG file: %s\n", PNGErrorText(error));

    // make local use for asm code
    pngData   = img.mData;
    pngWidth  = img.mWidth;
    pngHeight = img.mHeight;

    // render to screen
    switch (bitsPerPixel)
    {
    case 32:
        __asm {
            mov     eax, pageOffset
            mov     ebx, bytesPerScanline
            mul     ebx
            mov     edi, lfbPtr
            add     edi, eax
            mov     esi, pngData
            mov     ecx, pngWidth
            mov     eax, lfbWidth
            sub     eax, pngWidth
            shl     eax, 2
            push    eax
        next:
            xor     edx, edx
            mov     eax, ecx
            shr     eax, 4
            mov     ebx, 2
            div     ebx
            push    edx
            xor     edx, edx
            mov     eax, pngHeight
            shr     eax, 4
            mov     ebx, 2
            div     ebx
            pop     ebx
            cmp     edx, ebx
            jne     setbk
            mov     ebx, 255
            jmp     quit
        setbk:
            mov     ebx, 191
        quit:
            mov     eax, 255
            sub     al, [esi + 3]
            mul     ebx
            mov     ebx, eax
            xor     eax, eax
            mov     al, [esi + 3]
            mul     byte ptr[esi]
            add     eax, ebx
            shr     eax, 8
            stosb
            mov     al, [esi + 3]
            mul     byte ptr[esi + 1]
            add     eax, ebx
            shr     eax, 8
            stosb
            mov     al, [esi + 3]
            mul     byte ptr[esi + 2]
            add     eax, ebx
            shr     eax, 8
            stosb
            inc     edi
            add     esi, 4
            dec     ecx
            jnz     next
            mov     ecx, pngWidth
            add     edi, [esp]
            dec     pngHeight
            jnz     next
            pop     eax
        }
        break;

    case 24:
        __asm {
            mov     eax, pageOffset
            mov     ebx, bytesPerScanline
            mul     ebx
            mov     edi, lfbPtr
            add     edi, eax
            mov     esi, pngData
            mov     ecx, pngWidth
            mov     eax, lfbWidth
            sub     eax, pngWidth
            mov     ebx, eax
            shl     eax, 1
            add     eax, ebx
            push    eax
        next:
            xor     edx, edx
            mov     ebx, 2
            mov     eax, ecx
            shr     eax, 4
            div     ebx
            push    edx
            xor     edx, edx
            mov     eax, pngHeight
            shr     eax, 4
            div     ebx
            pop     ebx
            cmp     edx, ebx
            jne     setbk
            mov     ebx, 255
            jmp     quit
        setbk:
            mov     ebx, 191
        quit:
            mov     eax, 255
            sub     al, [esi + 3]
            mul     ebx
            mov     ebx, eax
            mov     al, [esi + 3]
            mul     byte ptr[esi]
            add     eax, ebx
            shr     eax, 8
            stosb
            mov     al, [esi + 3]
            mul     byte ptr[esi + 1]
            add     eax, ebx
            shr     eax, 8
            stosb
            mov     al, [esi + 3]
            mul     byte ptr[esi + 2]
            add     eax, ebx
            shr     eax, 8
            stosb
            add     esi, 4
            dec     ecx
            jnz     next
            mov     ecx, pngWidth
            add     edi, [esp]
            dec     pngHeight
            jnz     next
            pop     eax
        }
        break;
    }
}

// Capture display screen and save to file
void saveScreen(const char *fname)
{
    // check for image type (use file extension)
    char *pos = strrchr(fname, '.');
    if (!pos) return;

    // Save BMP file
    if (!strcmpi(pos, ".bmp"))
    {
        GFX_BITMAP bmp = {0};
        bmp.bmWidth    = lfbWidth;
        bmp.bmHeight   = lfbHeight;
        bmp.bmData     = lfbPtr;
        bmp.bmPixels   = bytesPerPixel;
        bmp.bmRowBytes = bytesPerScanline;
        if (!saveBitmap(fname, &bmp)) fatalError("saveScreen: cannot create BMP file: %s\n", fname);
    }

    // Save PNG file
    else if (!strcmpi(pos, ".png"))
    {
        GFX_IMAGE img = {0};
        uint32_t error = 0;
        uint8_t *pngData = NULL;
        uint32_t pngWidth = 0, pngHeight = 0;

        // init bitmap data
        img.mWidth      = lfbWidth;
        img.mHeight     = lfbHeight;
        img.mPixels     = bytesPerPixel;
        img.mRowBytes   = bytesPerScanline;
        img.mSize       = lfbSize;
        img.mData       = (uint8_t*)calloc(lfbSize, 1);
        if (!img.mData) fatalError("saveScreen: cannot alloc PNG buffer.\n");

        // make local to use asm
        pngData   = img.mData;
        pngWidth  = img.mWidth;
        pngHeight = img.mHeight;

        // copy screen pixel to png data buffer
        switch(bitsPerPixel)
        {
        case 32:
            __asm {
                mov     eax, pageOffset
                mul     bytesPerScanline
                mov     esi, lfbPtr
                add     esi, eax
                mov     edi, pngData
            again:
                mov     ecx, pngWidth
            next:
                lodsd
                mov     [edi + 3], 255  // a
                mov     [edi + 2], al   // b
                mov     [edi + 1], ah   // g
                shr     eax, 16
                mov     [edi], al       // r
                add     edi, 4
                dec     ecx
                jnz     next
                dec     pngHeight
                jnz     again
            }
            break;

        case 24:
            __asm {
                mov     eax, pageOffset
                mul     bytesPerScanline
                mov     esi, lfbPtr
                add     esi, eax
                mov     edi, pngData
            again:
                mov     ecx, pngWidth
            next:
                lodsw
                mov     [edi + 2], al  // r
                mov     [edi + 1], ah  // g
                lodsb
                mov     [edi]    , al  // b
                add     edi, 3
                dec     ecx
                jnz     next
                dec     pngHeight
                jnz     again
            }
            break;
        }

        // encode buffer into file
        error = savePNG(fname, &img);
        free(img.mData);
        if (error) fatalError("saveScreen: PNG save error: %s\n", PNGErrorText(error));
    }
}

// Initialize mouse driver
int32_t initMouse()
{
    short state = 0;
    short button = 0;

    __asm {
        xor     ax, ax
        xor     bx, bx
        int     33h
        mov     state, ax
        mov     button, bx
    }
    return (state != 0) && (button > 0);
}

// Initalize mouse driver and bitmap mouse image
int32_t initMouseButton(GFX_MOUSE_IMAGE *mi)
{
    // initialize mouse first
    short state = 0;
    short button = 0;
    
    __asm {
        xor     ax, ax
        xor     bx, bx
        int     33h
        mov     state, ax
        mov     button, bx
    }

    // initialize mouse image value
    mi->msState    = state;
    mi->msNumBtn   = button;
    mi->msPosX     = centerX;
    mi->msPosY     = centerY;
    mi->msWidth    = 0;
    mi->msHeight   = 0;
    mi->msPixels   = 0;
    mi->msUnder    = NULL;
    mi->msBitmap   = NULL;
    return (state != 0) && (button > 0);
}

// mouse callback handler
#pragma aux mouseHandler parm [eax] [ebx] [ecx] [edx]
void __loadds __far mouseHandler(int32_t max, int32_t mbx, int32_t mcx, int32_t mdx)
{
    mcd.max = max;
    mcd.mbx = mbx;
    mcd.mcx = mcx;
    mcd.mdx = mdx;
}

// install hardware interrupt handler
void installMouseHandler()
{
    __asm {
        lea     edx, mouseHandler
        mov     ax, seg mouseHandler
        mov     es, ax
        mov     ax, 0Ch
        mov     cx, 0FFh
        int     33h
    }
}

// uninstall hardware interrupt hander
void unInstallMouseHandler()
{
    __asm {
        xor     edx, edx
        mov     ax, 0Ch
        xor     cx, cx
        mov     es, cx
        int     33h
    }
}

// set new mouse postion
void setMousePos(short posx, short posy)
{
    __asm {
        mov     ax, 04h
        mov     cx, posx
        mov     dx, posy
        int     33h
    }
}

// set mouse limit range
void setMouseRange(short x1, short y1, short x2, short y2)
{
    __asm {
        mov     ax, 07h
        mov     cx, x1
        mov     dx, x2
        int     33h
        inc     ax
        mov     cx, y1
        mov     dx, y2
        int     33h
    }
}

// set mouse sensity and drag speed
void setMouseSensitivity(short sx, short sy, short dspeed)
{
    if (sx > 100) sx = 100;
    if (sy > 100) sy = 100;
    if (dspeed > 100) dspeed = 100;

    __asm {
        mov     ax, 01Ah
        mov     bx, sx
        mov     cx, sy
        mov     dx, dspeed
        int     33h
    }
}

// draw mouse cursor
void drawMouseCursor(GFX_MOUSE_IMAGE *mi)
{
    int32_t mbWidth  = mi->msWidth;
    int32_t mbHeight = mi->msHeight;

    int32_t mx = mi->msPosX - mi->msBitmap->mbHotX;
    int32_t my = mi->msPosY - mi->msBitmap->mbHotY;

    uint8_t *msUnder = mi->msUnder;
    uint8_t *mbImage = mi->msBitmap->mbData;

    // check color channel
    if (bytesPerPixel != mi->msPixels) return;

    // check clip boundary
    if (mx < cminX) mx = cminX;
    if (mx > cmaxX) mx = cmaxX;
    if (my < cminY) my = cminY;
    if (my > cmaxY) my = cmaxY;

    // render mouse cursor to screen
    switch(bytesPerPixel)
    {
    case 1:
        __asm {
            mov     eax, my
            add     eax, pageOffset
            mul     lfbWidth
            add     eax, mx
            mov     esi, lfbPtr
            add     esi, eax
            mov     edi, msUnder
            mov     ebx, lfbWidth
            sub     ebx, mbWidth
            push    ebx
            xor     edx, edx
        next:
            xor     ecx, ecx
        step:
            // check mouse boundary
            mov     ebx, mx
            add     ebx, ecx
            cmp     ebx, cminX
            jb      skip
            cmp     ebx, cmaxX
            ja      skip
            mov     ebx, my
            add     ebx, edx
            cmp     ebx, cminY
            jb      skip
            cmp     ebx, cmaxY
            ja      skip

            // copy screen background to mouse under
            movsb   
            push    esi
            push    edi

            // render mouse cursor to screen
            mov     eax, edi
            sub     eax, msUnder
            mov     edi, esi
            dec     edi
            mov     esi, mbImage
            add     esi, eax
            dec     esi

            // don't render color key
            lodsb
            test    al, al
            jz      quit
            stosb
            jmp     quit
        skip:
            inc     esi
            inc     edi
            jmp     cycle
        quit:
            pop     edi
            pop     esi
        cycle:
            inc     ecx
            cmp     ecx, mbWidth
            jb      step
            add     esi, [esp]
            inc     edx
            cmp     edx, mbHeight
            jb      next
            pop     ebx
        }
        break;

    case 2:
        __asm {
            mov     eax, my
            add     eax, pageOffset
            mul     lfbWidth
            add     eax, mx
            shl     eax, 1
            mov     esi, lfbPtr
            add     esi, eax
            mov     edi, msUnder
            mov     ebx, lfbWidth
            sub     ebx, mbWidth
            shl     ebx, 1
            push    ebx
            xor     edx, edx
        next:
            xor     ecx, ecx
        step:
            // check mouse boundary
            mov     ebx, mx
            add     ebx, ecx
            cmp     ebx, cminX
            jb      skip
            cmp     ebx, cmaxX
            ja      skip
            mov     ebx, my
            add     ebx, edx
            cmp     ebx, cminY
            jb      skip
            cmp     ebx, cmaxY
            ja      skip

            // copy screen background to mouse under
            movsw
            push    esi
            push    edi

            // render mouse cursor to screen
            mov     eax, edi
            sub     eax, msUnder
            mov     edi, esi
            sub     edi, 2
            mov     esi, mbImage
            add     esi, eax
            sub     esi, 2

            // don't render color key
            lodsw
            test    ax, ax
            jz      quit
            stosw
            jmp     quit
        skip:
            add     esi, 2
            add     edi, 2
            jmp     cycle
        quit:
            pop     edi
            pop     esi
        cycle:
            inc     ecx
            cmp     ecx, mbWidth
            jb      step
            add     esi, [esp]
            inc     edx
            cmp     edx, mbHeight
            jb      next
            pop     ebx
        }
        break;

    case 3:
        __asm {
            mov     eax, my
            add     eax, pageOffset
            mul     lfbWidth
            add     eax, mx
            mov     ebx, eax
            shl     eax, 1
            add     eax, ebx
            mov     esi, lfbPtr
            add     esi, eax
            mov     edi, msUnder
            mov     ebx, lfbWidth
            sub     ebx, mbWidth
            mov     eax, ebx
            shl     ebx, 1
            add     ebx, eax
            push    ebx
            xor     edx, edx
        next:
            xor     ecx, ecx
        step:
            // check mouse boundary
            mov     ebx, mx
            add     ebx, ecx
            cmp     ebx, cminX
            jb      skip
            cmp     ebx, cmaxX
            ja      skip
            mov     ebx, my
            add     ebx, edx
            cmp     ebx, cminY
            jb      skip
            cmp     ebx, cmaxY
            ja      skip

            // copy screen background to mouse under
            movsw
            movsb
            push    esi
            push    edi

            // render mouse cursor to screen
            mov     eax, edi
            sub     eax, msUnder
            mov     edi, esi
            sub     edi, 3
            mov     esi, mbImage
            add     esi, eax
            sub     esi, 3

            // don't render color key
            lodsw
            mov     ebx, eax
            lodsb
            shl     eax, 16
            or      eax, ebx
            test    eax, eax
            jz      quit
            stosw
            shr     eax, 16
            stosb
            jmp     quit
        skip:
            add     esi, 3
            add     edi, 3
            jmp     cycle
        quit:
            pop     edi
            pop     esi
        cycle:
            inc     ecx
            cmp     ecx, mbWidth
            jb      step
            add     esi, [esp]
            inc     edx
            cmp     edx, mbHeight
            jb      next
            pop     ebx
        }
        break;

    case 4:
        __asm {
            mov     eax, my
            add     eax, pageOffset
            mul     lfbWidth
            add     eax, mx
            shl     eax, 2
            mov     esi, lfbPtr
            add     esi, eax
            mov     edi, msUnder
            mov     eax, lfbWidth
            sub     eax, mbWidth
            shl     eax, 2
            push    eax
            xor     edx, edx
        next:
            xor     ecx, ecx
        step:
            // check mouse boundary
            mov     ebx, mx
            add     ebx, ecx
            cmp     ebx, cminX
            jb      skip
            cmp     ebx, cmaxX
            ja      skip
            mov     ebx, my
            add     ebx, edx
            cmp     ebx, cminY
            jb      skip
            cmp     ebx, cmaxY
            ja      skip

            // copy screen background to mouse under
            movsd
            push    esi
            push    edi

            // render mouse cursor to screen
            mov     eax, edi
            sub     eax, msUnder
            mov     edi, esi
            sub     edi, 4
            mov     esi, mbImage
            add     esi, eax
            sub     esi, 4

            // don't render color key
            lodsd
            and     eax, 00FFFFFFh
            test    eax, eax
            jz      quit
            stosd
            jmp     quit
        skip:
            add     esi, 4
            add     edi, 4
            jmp     cycle
        quit:
            pop     edi
            pop     esi
        cycle:
            inc     ecx
            cmp     ecx, mbWidth
            jb      step
            add     esi, [esp]
            inc     edx
            cmp     edx, mbHeight
            jb      next
            pop     ebx
        }
        break;
    }
}

// hide mouse cursor
void clearMouseCursor(GFX_MOUSE_IMAGE *mi)
{ 
    int32_t mbWidth  = mi->msWidth;
    int32_t mbHeight = mi->msHeight;

    uint8_t *msUnder = mi->msUnder;
    int32_t mx = mi->msPosX - mi->msBitmap->mbHotX;
    int32_t my = mi->msPosY - mi->msBitmap->mbHotY;

    // check color channel
    if (bytesPerPixel != mi->msPixels) return;

    // check clip boundary
    if (mx < cminX) mx = cminX;
    if (mx > cmaxX) mx = cmaxX;
    if (my < cminY) my = cminY;
    if (my > cmaxY) my = cmaxY;

    // render mouse under to screen
    switch(bytesPerPixel)
    {
    case 1:
        // copy mouse under to screen
        __asm {
            mov     eax, my
            add     eax, pageOffset
            mul     lfbWidth
            add     eax, mx
            mov     edi, lfbPtr
            add     edi, eax
            mov     esi, msUnder
            mov     ebx, lfbWidth
            sub     ebx, mbWidth
            xor     edx, edx
        next:
            xor     ecx, ecx
        step:
            // check mouse boundary
            mov     eax, mx
            add     eax, ecx
            cmp     eax, cminX
            jb      skip
            cmp     eax, cmaxX
            ja      skip
            mov     eax, my
            add     eax, edx
            cmp     eax, cminY
            jb      skip
            cmp     eax, cmaxY
            ja      skip
            movsb
            jmp     quit
        skip:
            inc     edi
            inc     esi
        quit:
            inc     ecx
            cmp     ecx, mbWidth
            jb      step
            add     edi, ebx
            inc     edx
            cmp     edx, mbHeight
            jb      next
        }
        break;

    case 2:
        // copy mouse under to screen
        __asm {
            mov     eax, my
            add     eax, pageOffset
            mul     lfbWidth
            add     eax, mx
            shl     eax, 1
            mov     edi, lfbPtr
            add     edi, eax
            mov     esi, msUnder
            mov     ebx, lfbWidth
            sub     ebx, mbWidth
            shl     ebx, 1
            xor     edx, edx
        next:
            xor     ecx, ecx
        step:
            // check mouse boundary
            mov     eax, mx
            add     eax, ecx
            cmp     eax, cminX
            jb      skip
            cmp     eax, cmaxX
            ja      skip
            mov     eax, my
            add     eax, edx
            cmp     eax, cminY
            jb      skip
            cmp     eax, cmaxY
            ja      skip
            movsw
            jmp     quit
        skip:
            add     edi, 2
            add     esi, 2
        quit:
            inc     ecx
            cmp     ecx, mbWidth
            jb      step
            add     edi, ebx
            inc     edx
            cmp     edx, mbHeight
            jb      next
        }
        break;

    case 3:
        // copy mouse under to screen
        __asm {
            mov     eax, my
            add     eax, pageOffset
            mul     lfbWidth
            add     eax, mx
            mov     ebx, eax
            shl     eax, 1
            add     eax, ebx
            mov     edi, lfbPtr
            add     edi, eax
            mov     esi, msUnder
            mov     ebx, lfbWidth
            sub     ebx, mbWidth
            mov     eax, ebx
            shl     ebx, 1
            add     ebx, eax
            xor     edx, edx
        next:
            xor     ecx, ecx
        step:
            // check mouse boundary
            mov     eax, mx
            add     eax, ecx
            cmp     eax, cminX
            jb      skip
            cmp     eax, cmaxX
            ja      skip
            mov     eax, my
            add     eax, edx
            cmp     eax, cminY
            jb      skip
            cmp     eax, cmaxY
            ja      skip
            movsw
            movsb
            jmp     quit
        skip:
            add     edi, 3
            add     esi, 3
        quit:
            inc     ecx
            cmp     ecx, mbWidth
            jb      step
            add     edi, ebx
            inc     edx
            cmp     edx, mbHeight
            jb      next
        }
        break;

    case 4:
        __asm {
            mov     eax, my
            add     eax, pageOffset
            mul     lfbWidth
            add     eax, mx
            shl     eax, 2
            mov     edi, lfbPtr
            add     edi, eax
            mov     esi, msUnder
            mov     ebx, lfbWidth
            sub     ebx, mbWidth
            shl     ebx, 2
            xor     edx, edx
        next:
            xor     ecx, ecx
        step:
            // check mouse boundary
            mov     eax, mx
            add     eax, ecx
            cmp     eax, cminX
            jb      skip
            cmp     eax, cmaxX
            ja      skip
            mov     eax, my
            add     eax, edx
            cmp     eax, cminY
            jb      skip
            cmp     eax, cmaxY
            ja      skip
            movsd
            jmp     quit
        skip:
            add     edi, 4
            add     esi, 4
        quit:
            inc     ecx
            cmp     ecx, mbWidth
            jb      step
            add     edi, ebx
            inc     edx
            cmp     edx, mbHeight
            jb      next
        }
        break;
    }
}

// Draw bitmap button
void drawButton(GFX_BUTTON_IMAGE *btn)
{
    int32_t x1, y1, x2, y2, lwidth, lheight;
    int32_t lminX, lminY, lmaxX, lmaxY;
    
    // Initialize button data
    int32_t btnWidth  = btn->btWidth;
    int32_t btnHeight = btn->btHeight;
    void *btnData = btn->btData[btn->btState % BUTTON_BITMAPS];

    // check color channel
    if (bytesPerPixel != btn->btPixels) return;

    // Calculate coordinator
    x1 = btn->btPosX;
    y1 = btn->btPosY;
    x2 = x1 + (btnWidth - 1);
    y2 = y1 + (btnHeight - 1);

    // Clip button image to context boundaries
    lminX = (x1 >= cminX) ? x1 : cminX;
    lminY = (y1 >= cminY) ? y1 : cminY;
    lmaxX = (x2 <= cmaxX) ? x2 : cmaxX;
    lmaxY = (y2 <= cmaxY) ? y2 : cmaxY;

    // Validate boundaries
    if (lminX >= lmaxX) return;
    if (lminY >= lmaxY) return;

    // Initialize loop variables
    lwidth  = (lmaxX - lminX) + 1;
    lheight = (lmaxY - lminY) + 1;

    switch (bytesPerPixel)
    {
    case 1:
        __asm {
            mov     edi, lfbPtr
            mov     eax, lminY
            add     eax, pageOffset
            mul     lfbWidth
            add     eax, lminX
            add     edi, eax
            mov     esi, btnData
            mov     eax, lminY
            sub     eax, y1
            mul     btnWidth
            mov     ebx, lminX
            sub     ebx, x1
            add     eax, ebx
            add     esi, eax
            mov     ebx, lfbWidth
            sub     ebx, lwidth
            mov     edx, btnWidth
            sub     edx, lwidth
        next:
            mov     ecx, lwidth
        plot:
            lodsb
            test    al, al
            jz      skip
            stosb
            jmp     quit
        skip:
            inc     edi
        quit:
            loop    plot
            add     edi, ebx
            add     esi, edx
            dec     lheight
            jnz     next
        }
        break;

    case 2:
        __asm {
            mov     edi, lfbPtr
            mov     eax, lminY
            add     eax, pageOffset
            mul     lfbWidth
            add     eax, lminX
            shl     eax, 1
            add     edi, eax
            mov     esi, btnData
            mov     eax, lminY
            sub     eax, y1
            mul     btnWidth
            mov     ebx, lminX
            sub     ebx, x1
            add     eax, ebx
            shl     eax, 1
            add     esi, eax
            mov     ebx, lfbWidth
            sub     ebx, lwidth
            shl     ebx, 1
            mov     edx, btnWidth
            sub     edx, lwidth
            shl     edx, 1
        next:
            mov     ecx, lwidth
        plot:
            lodsw
            test    ax, ax
            je      skip
            stosw
            jmp     quit
        skip:
            add     edi, 2
        quit:
            loop    plot
            add     edi, ebx
            add     esi, edx
            dec     lheight
            jnz     next
        }
        break;

    case 3:
        __asm {
            mov     edi, lfbPtr
            mov     eax, lminY
            add     eax, pageOffset
            mul     lfbWidth
            add     eax, lminX
            mov     ebx, eax
            shl     eax, 1
            add     eax, ebx
            add     edi, eax
            mov     esi, btnData
            mov     eax, lminY
            sub     eax, y1
            mul     btnWidth
            mov     ebx, lminX
            sub     ebx, x1
            add     eax, ebx
            mov     ebx, eax
            shl     eax, 1
            add     eax, ebx
            add     esi, eax
            mov     ebx, lfbWidth
            mov     ebx, lwidth
            mov     eax, ebx
            shl     ebx, 1
            add     ebx, eax
            push    ebx
            mov     edx, btnWidth
            sub     edx, lwidth
            mov     eax, edx
            shl     edx, 1
            add     edx, eax
        next:
            mov     ecx, lwidth
        plot:
            lodsw
            mov     ebx, eax
            lodsb
            shl     eax, 16
            or      eax, ebx
            test    eax, eax
            jz      skip
            stosw
            shr     eax, 16
            stosb
            jmp     quit
        skip:
            add     edi, 3
        quit:
            loop    plot
            add     edi, [esp]
            add     esi, edx
            dec     lheight
            jnz     next
            pop     ebx
        }
        break;

    case 4:
        __asm {
            mov     edi, lfbPtr
            mov     eax, lminY
            add     eax, pageOffset
            mul     lfbWidth
            add     eax, lminX
            shl     eax, 2
            add     edi, eax
            mov     esi, btnData
            mov     eax, lminY
            sub     eax, y1
            mul     btnWidth
            mov     ebx, lminX
            sub     ebx, x1
            add     eax, ebx
            shl     eax, 2
            add     esi, eax
            mov     ebx, lfbWidth
            sub     ebx, lwidth
            shl     ebx, 2
            mov     edx, btnWidth
            sub     edx, lwidth
            shl     edx, 2
        next:
            mov     ecx, lwidth
        plot:
            lodsd
            and     eax, 00FFFFFFh
            test    eax, eax
            jz      skip
            stosd
            jmp     quit
        skip:
            add     edi, 4
        quit:
            loop    plot
            add     edi, ebx
            add     esi, edx
            dec     lheight
            jnz     next
        }
        break;
    }
}

// load the bitmap mouse button
int32_t loadMouseButton(const char *fname, GFX_MOUSE_IMAGE *mi, MOUSE_BITMAP *mbm, GFX_BUTTON_IMAGE *btn)
{
    GFX_BITMAP bmp;
    int32_t i, j, y;
    uint8_t *src, *dst;

    // load mouse bitmap and animation
    if (!loadBitmap(fname, &bmp)) return 0;

    // allocate memory for mouse under background
    if (!(mi->msUnder = (uint8_t*)calloc(MOUSE_SIZE, bmp.bmPixels))) return 0;

    // init mouse image width and height
    mi->msWidth  = MOUSE_WIDTH;
    mi->msHeight = MOUSE_HEIGHT;
    mi->msPixels = bmp.bmPixels;

    // copy mouse cursors
    for (i = 0; i != NUM_MOUSE_BITMAPS; i++)
    {
        mbm[i].mbData = (uint8_t*)calloc(MOUSE_SIZE, bmp.bmPixels);
        if (!mbm[i].mbData) return 0;
        mbm[i].mbHotX = 12;
        mbm[i].mbHotY = 12;
        mbm[i].mbNext = &mbm[i + 1];
        for (y = 0; y != MOUSE_HEIGHT; y++)
        {
            dst = &mbm[i].mbData[y * MOUSE_WIDTH * bmp.bmPixels];
            src = &bmp.bmData[(i * MOUSE_WIDTH + y * bmp.bmWidth) * bmp.bmPixels];
            memcpy(dst, src, MOUSE_WIDTH * bmp.bmPixels);
        }
    }

    // init current and next mouse animated
    mbm[0].mbHotX = 7;
    mbm[0].mbHotY = 2;
    mbm[0].mbNext = &mbm[0];
    mbm[8].mbNext = &mbm[1];

    // copy button bitmaps
    for (i = 0; i != NUM_BUTTONS; i++)
    {
        btn[i].btWidth = BUTTON_WIDTH;
        btn[i].btHeight = BUTTON_HEIGHT;
        btn[i].btPixels = bmp.bmPixels;
        for (j = 0; j != BUTTON_BITMAPS; j++)
        {
            btn[i].btData[j] = (uint8_t*)calloc(BUTTON_SIZE, bmp.bmPixels);
            if (!btn[i].btData[j]) return 0;
            for (y = 0; y != BUTTON_HEIGHT; y++)
            {
                dst = &btn[i].btData[j][y * BUTTON_WIDTH * bmp.bmPixels];
                src = &bmp.bmData[(i * (bmp.bmWidth >> 1) + j * BUTTON_WIDTH + (BUTTON_HEIGHT + y) * bmp.bmWidth) * bmp.bmPixels];
                memcpy(dst, src, BUTTON_WIDTH * bmp.bmPixels);
            }
        }
    }

    // init button 'click me'
    btn[0].btPosX  = centerX - BUTTON_WIDTH - 20;
    btn[0].btPosY  = centerY - (BUTTON_HEIGHT >> 1);
    btn[0].btState = STATE_NORM;

    // init button 'exit'
    btn[1].btPosX  = centerX + BUTTON_WIDTH + 10;
    btn[1].btPosY  = centerY - (BUTTON_HEIGHT >> 1);
    btn[1].btState = STATE_NORM;

    // set palette for 8bits bitmap
    if (bmp.bmPixels == 1) setPalette(bmp.bmExtra);
    closeBitmap(&bmp);
    return 1;
}

// release mouse button
void closeMouseButton(GFX_MOUSE_IMAGE *mi, MOUSE_BITMAP *mbm, GFX_BUTTON_IMAGE *btn)
{
    int32_t i, j;

    // cleanup mouse bitmap
    for (i = 0; i != NUM_MOUSE_BITMAPS; i++)
    {
        if (mbm[i].mbData)
        {
            free(mbm[i].mbData);
            mbm[i].mbData = NULL;
        }
    }

    // cleanup button bitmap
    for (i = 0; i != NUM_BUTTONS; i++)
    {
        for (j = 0; j != BUTTON_BITMAPS; j++)
        {
            if (btn[i].btData[j])
            {
                free(btn[i].btData[j]);
                btn[i].btData[j] = NULL;
            }
        }
    }

    // cleanup mouse underground
    clearMouseCursor(mi);
    if (mi->msUnder)
    {
        free(mi->msUnder);
        mi->msUnder = NULL;
    }
}

// automatic mouse event handler
void handleMouse(const char *fname)
{
    GFX_MOUSE_IMAGE mi;

    MOUSE_BITMAP *msNormal = NULL;
    MOUSE_BITMAP *msWait = NULL;
    MOUSE_BITMAP *msNew = NULL;

    GFX_BUTTON_IMAGE btn[NUM_BUTTONS] = {0};
    MOUSE_BITMAP mbm[NUM_MOUSE_BITMAPS] = {0};

    int32_t i, done = 0;
    int32_t lastx = 0, lasty = 0;
    uint32_t lastTime = 0;
    uint32_t needDraw = 0xFFFF;
    const char *bkg[] = {"assets/1lan8.bmp", "assets/1lan16.bmp", "assets/1lan24.bmp", "assets/1lan32.bmp"};

    // init and setup bitmap mouse and button
    if (!initMouseButton(&mi)) fatalError("handleMouse: cannot init mouse driver.\n");
    if (!loadMouseButton(fname, &mi, mbm, btn)) fatalError("handleMouse: cannot load mouse bitmap: %s\n", fname);

    // install user-define mouse handler
    installMouseHandler();
    setMousePos(centerX, centerY);
    setMouseRange(centerX - 100, centerY - 100, centerX + 110, centerY + 100);
    setMouseSensitivity(100, 100, 100);

    // init mouse normal and wait cursor bitmap
    msNormal    = &mbm[0];
    msWait      = &mbm[1];
    mi.msBitmap = msNormal;

    // setup screen background
    showBitmap(bkg[bytesPerPixel - 1]);
    drawMouseCursor(&mi);

    // update last mouse pos
    lastx = mcd.mcx = centerX;
    lasty = mcd.mdx = centerY;
    lastTime = GetTicks();

    // remove keyboard buffer
    while (kbhit()) getch();
    while (!kbhit() && !done)
    {
        // only draw if needed
        if (needDraw)
        {
            // clear old mouse position
            waitRetrace();
            clearMouseCursor(&mi);

            // draw buttons
            if (needDraw > 1)
            {
                for (i = 0; i != NUM_BUTTONS; i++)
                {
                    if (needDraw & (2 << i)) drawButton(&btn[i]);
                }
            }

            // update new button bitmap
            if (msNew) mi.msBitmap = msNew;

            // update mouse position and draw new mouse position
            mi.msPosX = mcd.mcx;
            mi.msPosY = mcd.mdx;
            drawMouseCursor(&mi);

            // update last ticks count and turn off drawable
            lastTime = GetTicks();
            needDraw = 0;
            msNew = NULL;
        }

        // check for draw new state button
        if (GetTicks() != lastTime)
        {
            if (mi.msBitmap != mi.msBitmap->mbNext)
            {
                needDraw = 1;
                mi.msBitmap = mi.msBitmap->mbNext;
            }
            else
            {
                lastTime = GetTicks();
            }
        }

        // update drawable when position changing
        if (lastx != mcd.mcx || lasty != mcd.mdx)
        {
            lastx = mcd.mcx;
            lasty = mcd.mdx;
            needDraw = 1;
        }

        // check for new button state
        for (i = 0; i != NUM_BUTTONS; i++)
        {
            // check if mouse inside the button region
            if (mcd.mcx >= btn[i].btPosX && mcd.mcx <= btn[i].btPosX + BUTTON_WIDTH && mcd.mdx >= btn[i].btPosY && mcd.mdx <= btn[i].btPosY + BUTTON_HEIGHT)
            {
                if (mcd.mbx == 0 && btn[i].btState == STATE_PRESSED)
                {
                    btn[i].btState = STATE_ACTIVE;
                    needDraw |= (2 << i);
                    if (i == 0)
                    {
                        if (mi.msBitmap == msNormal) msNew = msWait;
                        else msNew = msNormal;
                    }
                    else if (i == 1) done = 1;
                }
                else if (mcd.mbx == 1)
                {
                    btn[i].btState = STATE_PRESSED;
                    needDraw |= (2 << i);
                }
                else if (btn[i].btState == STATE_NORM && mcd.mbx == 0)
                {
                    btn[i].btState = STATE_ACTIVE;
                    needDraw |= (2 << i);
                }
                else if (btn[i].btState == STATE_WAITING)
                {
                    if (mcd.mbx == 1)
                    {
                        btn[i].btState = STATE_PRESSED;
                    }
                    else
                    {
                        btn[i].btState = STATE_ACTIVE;
                    }
                    needDraw |= (2 << i);
                }
            }
            else if (btn[i].btState == STATE_ACTIVE)
            {
                btn[i].btState = STATE_NORM;
                needDraw |= (2 << i);
            }
            else if (btn[i].btState == STATE_PRESSED && mcd.mbx == 1)
            {
                btn[i].btState = STATE_WAITING;
                needDraw |= (2 << i);
            }
            else if (btn[i].btState == STATE_WAITING && mcd.mbx == 0)
            {
                btn[i].btState = STATE_NORM;
                needDraw |= (2 << i);
            }
        }
    }

    // release mouse bitmap and callback handler
    closeMouseButton(&mi, mbm, btn);
    unInstallMouseHandler();
}

///////////////////// END OF GFXLIB.C ///////////////////////////////////
int initVBE3()
{
    VBE_FAR_CALL        fcall;
    VBE_DRIVER_INFO     drvInfo;
    VBE_PM_INFO_BLOCK   *pmInfo;

    uint32_t val = 0;
    uint32_t i = 0;
    uint8_t biosCheckSum = 0;

    uint8_t *biosCode;
    uint8_t *biosData;
    uint8_t *biosStack;
    uint8_t *biosPtr;

    uint16_t biosDataSel;
    uint16_t biosCodeSel;
    uint16_t a0000Sel;
    uint16_t b0000Sel;
    uint16_t b8000Sel;

    uint16_t biosInitSel;
    uint16_t biosStackSel;

    uint16_t vbeInfoSel;

    // copy ROM BIOS code from physical address 0xC0000 to RAM
    biosCode = (uint8_t*)calloc(VBE_CODE_SIZE, 1);
    if (!biosCode) return 1;
    memcpy(biosCode, (uint8_t*)0xC0000, VBE_CODE_SIZE);

    // find VESA 3.0 protected mode info block signature
    biosPtr = biosCode;
    while ((biosPtr <= biosCode + VBE_CODE_SIZE - sizeof(VBE_PM_INFO_BLOCK)) && memcmp(((VBE_PM_INFO_BLOCK*)biosPtr)->Signature, "PMID", 4)) biosPtr++;

    // check for correct signature
    pmInfo = (VBE_PM_INFO_BLOCK*)biosPtr;
    if (memcmp(pmInfo->Signature, "PMID", 4))
    {
        printf("VESA PMID not found!\n");
        return 1;
    }

    // calculate BIOS checksum
    for (i = 0; i < sizeof(VBE_PM_INFO_BLOCK); i++) biosCheckSum += *biosPtr++;
    if (biosCheckSum)
    {
        printf("VESA BIOS checksum error!\n");
        return 1;
    }

    // setup structure (provide selectors, map video mem, ...)
    biosData = (uint8_t*)calloc(VBE_DATA_SIZE, 1);
    if (!biosData) return 1;
    memset(biosData, 0, VBE_DATA_SIZE);

    // setup BIOS data selector
    biosDataSel = allocSelector();
    if (biosDataSel == 0 || biosDataSel == 0xFFFF) return 1;
    if (!setSelectorRights(biosDataSel, 0x8092)) return 1;
    if (!setSelectorBase(biosDataSel, (uint32_t)biosData)) return 1;
    if (!setSelectorLimit(biosDataSel, VBE_DATA_SIZE - 1)) return 1;
    pmInfo->BIOSDataSel = biosDataSel;

    // map video memory
    a0000Sel = allocSelector();
    if (a0000Sel == 0 || a0000Sel == 0xFFFF) return 1;
    if (!setSelectorRights(a0000Sel, 0x8092)) return 1;
    if (!setSelectorBase(a0000Sel, (uint32_t)0xA0000)) return 1;
    if (!setSelectorLimit(a0000Sel, 0xFFFF)) return 1;
    pmInfo->A0000Sel = a0000Sel;

    b0000Sel = allocSelector();
    if (b0000Sel == 0 || b0000Sel == 0xFFFF) return 1;
    if (!setSelectorRights(b0000Sel, 0x8092)) return 1;
    if (!setSelectorBase(b0000Sel, (uint32_t)0xB0000)) return 1;
    if (!setSelectorLimit(b0000Sel, 0xFFFF)) return 1;
    pmInfo->B0000Sel = b0000Sel;

    b8000Sel = allocSelector();
    if (b8000Sel == 0 || b8000Sel == 0xFFFF) return 1;
    if (!setSelectorRights(b8000Sel, 0x8092)) return 1;
    if (!setSelectorBase(b8000Sel, (uint32_t)0xB8000)) return 1;
    if (!setSelectorLimit(b8000Sel, 0x7FFF)) return 1;
    pmInfo->B8000Sel = b8000Sel;

    // setup BIOS code selector
    biosCodeSel = allocSelector();
    if (biosCodeSel == 0 || biosCodeSel == 0xFFFF) return 1;
    if (!setSelectorRights(biosCodeSel, 0x8092)) return 1;
    if (!setSelectorBase(biosCodeSel, (uint32_t)biosCode)) return 1;
    if (!setSelectorLimit(biosCodeSel, VBE_CODE_SIZE - 1)) return 1;
    pmInfo->CodeSegSel = biosCodeSel;

    // put BIOS code run in protect mode
    pmInfo->InProtectMode = 1;

    // alloc code segment selector for initialize function
    biosInitSel = allocSelector();
    if (biosInitSel == 0 || biosInitSel == 0xFFFF) return 1;
    if (!setSelectorRights(biosInitSel, 0x8092)) return 1;
    if (!setSelectorBase(biosInitSel, (uint32_t)biosCode)) return 1;
    if (!setSelectorLimit(biosInitSel, VBE_CODE_SIZE - 1)) return 1;

    // alloc stack selector
    biosStack = (uint8_t *)calloc(VBE_STACK_SIZE, 1);
    if (!biosStack) return 1;
    biosStackSel = allocSelector();
    if (biosStackSel == 0 || biosStackSel == 0xFFFF) return 1;
    if (!setSelectorRights(biosStackSel, 0x8092)) return 1;
    if (!setSelectorBase(biosStackSel, (uint32_t)biosStack)) return 1;
    if (!setSelectorLimit(biosStackSel, VBE_STACK_SIZE - 1)) return 1;

    // call initialize protect mode function first
    fcall.offset = pmInfo->PMInitialize;
    fcall.segment = biosInitSel;
    
    __asm {
        pusha
        mov    ax, biosStackSel
        mov    ss, ax
        mov    sp, 0
        lea    esi, fcall
        call   fword ptr [esi]
        popa
    }

    // call initialize VBE controller
    vbeInfoSel = allocSelector();
    if (vbeInfoSel == 0 || vbeInfoSel == 0xFFFF) return 1;
    if (!setSelectorRights(vbeInfoSel, 0x8092)) return 1;
    if (!setSelectorBase(vbeInfoSel, (uint32_t)&drvInfo)) return 1;
    if (!setSelectorLimit(vbeInfoSel, sizeof(VBE_DRIVER_INFO) - 1)) return 1;

    fcall.offset = pmInfo->EntryPoint;
    fcall.segment = pmInfo->CodeSegSel;
    
    __asm {
        mov    ax, vbeInfoSel
        mov    es, ax
        mov    eax, 0x4F00
        xor    edi, edi
        lea    esi, fcall
        call   fword ptr [esi]
        mov    val, eax
    }

    if (val == 0x004F && !memcmp(drvInfo.VBESignature, "VESA", 4) && drvInfo.VBEVersion >= 0x0200) printf("OK!\n");
    else printf("VESA 3.0 INIT FAILED!\n");
    return 0;
}
