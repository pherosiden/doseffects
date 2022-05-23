/*--------------------------------------------*/
/*         SVGA Graphics Library v2.0         */
/*  (Support 8, 15, 16, 24, 32 bits colors)   */
/*--------------------------------------------*/
/* Environment : Open Watcom C++ 1.9          */
/* Author      : Nguyen Ngoc Van              */
/* Email       : pherosiden@gmail.com         */
/* Memory model: all                          */
/* Heap Size   : 640K                         */
/* Home Page   : http://www.codedemo.net      */
/* Create      : 10/11/1998                   */
/* Last Update : 27/10/2002                   */
/*--------------------------------------------*/
/* WARNING: This library is freeware, it mean */
/* you using it freely with any purpose. But  */
/* you must place this header at the top of   */
/* your program. Have fun :), Thank you!      */
/* ONLY WORK WELL ON REAL-DOS MODEL           */
/*--------------------------------------------*/

#include <dos.h>
#include <mem.h>
#include <math.h>
#include <time.h>
#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define RIGHT       0x01
#define LEFT        0x02
#define LOW         0x04
#define HIGH        0x08
#define ECHE        0.75
#define M_PI        3.141592f

// Rountine functions
#define Swap(a, b)  {int16_t t = a; a = b; b = t;}

#pragma pack(push, 1)

// VESA information block
typedef struct tagVESAInfo {
    // Mandatory information for all VBE revisions
    char        VESASignature[4];       // 'VESA' VBE Signature
    uint16_t    VESAVersion;            // VBE Version. BCD number 0x0102 => Version 1.2
    char*       OEMStringPtr;           // Pointer to OEM String
    uint32_t    Capabilities;           // Capabilities of graphics controller
    uint16_t*   VideoModePtr;           // Pointer to VideoModeList
    uint16_t    TotalMemory;            // Number of 64kb memory blocks

    // Added for VBE 2.0+
    uint16_t    OEMSoftwareRev;         // VBE implementation Software revision
    char*       OEMVendorNamePtr;       // Pointer to Vendor Name String
    char*       OEMProductNamePtr;      // Pointer to Product Name String
    char*       OEMProductRevPtr;       // Pointer to Product Revision String
    uint8_t     Reserved[222];          // Reserved for VBE implementation scratch area
    uint8_t     OEMData[256];           // Data Area for OEM Strings
} VESAINFO;

// VESA video card capabilities block
typedef struct tagVESAMode {
    // Mandatory information for all VBE revisions
    uint16_t    ModeAttributes;         // Mode attributes
    uint8_t     WinAAttributes;         // Window A attributes
    uint8_t     WinBAttributes;         // Window B attributes
    uint16_t    WinGranularity;         // Window granularity
    uint16_t    WinSize;                // Window size
    uint16_t    WinASegment;            // Window A start segment
    uint16_t    WinBSegment;            // Window B start segment
    void*       WinFunctionPtr;         // Pointer to window function
    uint16_t    bytesPerScanLine;       // Bytes per scan line

    // Mandatory information for VBE 1.2 and above
    uint16_t    XResolution;            // Horizontal resolution in pixels or characters
    uint16_t    YResolution;            // Vertical resolution in pixels or characters
    uint8_t     XCharSize;              // Character cell width in pixels
    uint8_t     YCharSize;              // Character cell height in pixels
    uint8_t     NumberOfPlanes;         // Number of memory planes
    uint8_t     bitsPerPixel;           // Bits per pixel
    uint8_t     NumberOfBanks;          // Number of banks
    uint8_t     MemoryModel;            // Memory model type
    uint8_t     BankSize;               // Bank size in KB
    uint8_t     NumberOfImagePages;	    // Number of images
    uint8_t     Reserved1;              // Reserved for page function

    // Direct Color fields (required for direct/6 and YUV/7 memory models)
    uint8_t     RedMaskSize;            // Size of direct color red mask in bits
    uint8_t     RedFieldPosition;       // Bit position of lsb of red mask
    uint8_t     GreenMaskSize;          // Size of direct color green mask in bits
    uint8_t     GreenFieldPosition;     // Bit position of lsb of green mask
    uint8_t     BlueMaskSize;           // Size of direct color blue mask in bits
    uint8_t     BlueFieldPosition;      // Bit position of lsb of blue mask
    uint8_t     RsvdMaskSize;           // Size of direct color reserved mask in bits
    uint8_t     RsvdFieldPosition;      // Bit position of lsb of reserved mask
    uint8_t     DirectColorModeInfo;    // Direct color mode attributes

    // Mandatory information for VBE 2.0+
    void*       PhysicalBasePtr;        // Physical address for flat memory frame buffer
    void*       OffScreenMemOffset;     // Pointer to start of off screen memory
    uint16_t    OffScreenMemSize;       // Amount of off screen memory in 1k units

    // Mandatory information for VBE 3.0+
    uint16_t    LinBytesPerScanLine;    // Bytes per scan line for linear modes
    uint8_t     BnkNumberOfImagePages;  // Number of images for banked modes
    uint8_t     LinNumberOfImagePages;  // Number of images for linear modes
    uint8_t     LinRedMaskSize;         // Size of direct color red mask (linear modes)
    uint8_t     LinRedFieldPosition;    // Bit posotion of lsb of red mask (linear modes)
    uint8_t     LinGreenMaskSize;       // Size of direct color green mask (linear modes)
    uint8_t     LinGreenFieldPosition;  // Bit posotion of lsb of green mask (linear modes)
    uint8_t     LinBlueMaskSize;        // Size of direct color blue mask (linear modes)
    uint8_t     LinBlueFieldPosition;   // Bit posotion of lsb of blue mask (linear modes)
    uint8_t     LinRsvdMaskSize;        // Size of direct color reserved mask (linear modes)
    uint8_t     LinRsvdFieldPosition;   // Bit posotion of lsb of reserved mask (linear modes)
    uint32_t    MaxPixelClock;          // Maximun pixel clock (in Hz) for graphics modes
    uint8_t     Reserved2[190];         // Remainder of ModeInfoBlock
} VESAMODE;

// CHR font header
typedef struct tagCHRHeader {
    char        Sign[4];        // CHR signature
    char        Version[0x54];  // font version
    uint8_t     Rev;            // Revert not use
    uint16_t    HeaderSize;     // Sizeof font header
    char        FontName[4];    // Font family name
    char        Padding[33];    // Padding bytes
} HEADER;

// CHR font information
typedef struct tagCHRInfo {
    char        Sign;           // font signature
    uint16_t    numChar;        // Number of characters in font
    uint8_t     Dump;           // Dummy
    char        charStart;      // First char in font table
    uint16_t    StartOffset;    // Start font offset
    char        Padding[9];     // Padding struct
} FONTINFO;

// Information of each CHR character
typedef struct tagStroke {
    uint8_t     x;
    uint8_t     y;
} STROKE;

// RGB for palette 8/15/16/24 bits colors
typedef struct tagRGB {
    uint8_t     r;
    uint8_t     g;
    uint8_t     b;
} RGB;

// RGBA for palette 32 bits colors
typedef struct tagRGBA {
    uint8_t     r;
    uint8_t     g;
    uint8_t     b;
    uint8_t     a;
} RGBA;

// Tag point 2D
typedef struct tagPoint {
    double      x;
    double      y;
} POINT2D;

// Tag Stack
typedef struct tagStack {
    uint16_t    count;          // Current elements
    int16_t	    data[2000];     // data of element (MAX 2000)
} STACK;

#pragma pack(pop)

// Rountine for VESA Functions
void        putPixel320(uint16_t, uint16_t, uint32_t);
void        putPixel8(uint16_t, uint16_t, uint32_t);
void        putPixel15(uint16_t, uint16_t, uint32_t);
void        putPixel16(uint16_t, uint16_t, uint32_t);
void        putPixel24(uint16_t, uint16_t, uint32_t);
void        putPixel32(uint16_t, uint16_t, uint32_t);

uint32_t    getPixel320(uint16_t, uint16_t);
uint32_t    getPixel8(uint16_t, uint16_t);
uint32_t    getPixel15(uint16_t, uint16_t);
uint32_t    getPixel16(uint16_t, uint16_t);
uint32_t    getPixel24(uint16_t, uint16_t);
uint32_t    getPixel32(uint16_t, uint16_t);

uint32_t    fromRGB15(uint8_t, uint8_t, uint8_t);
uint32_t    fromRGB16(uint8_t, uint8_t, uint8_t);
uint32_t    fromRGB24(uint8_t, uint8_t, uint8_t);
uint32_t    fromRGB32(uint8_t, uint8_t, uint8_t);

uint8_t     getVesaInfo();
uint8_t     getVesaModeInfo(uint16_t);
uint8_t     setVesaMode(uint16_t);
uint8_t     initGraph(uint16_t, uint16_t, uint8_t);
uint16_t    findVesaMode(uint16_t, uint16_t, uint8_t);
uint16_t    getVesaMode();

void        waitRetrace();
void        closeGraph();
void        displayVesaInfo();
void        setPalette(RGB*, uint8_t, uint8_t);
void        getPalette(RGB*, uint8_t, uint8_t);
void        copyPalette(RGB*, RGB*, uint8_t, uint8_t);
void        movePalette(RGB*, RGB*, uint16_t);
void        clearPalette(RGB*, uint8_t, uint8_t);
void        linearPalette(RGB*);
void        rainbowPalette(RGB*);
void        rotatePalette(RGB*, uint8_t, uint8_t, uint8_t, uint16_t);
void        fadeIn(RGB*, RGB*, uint8_t, uint8_t);
void        fadeOut(RGB*, RGB*, uint8_t, uint8_t);
void        fadeMax(RGB*, uint8_t, uint8_t);
void        fadeMin(RGB*, uint8_t, uint8_t);
void        waitKeyPressed(uint16_t);

// Routine for font
uint8_t     getASCII(char);
uint16_t    getFileSize(FILE*);
int16_t     strPos(char*, char*);
char*       chr2Str(char, char);
void        fontVNI(char*);
void        schRepl(char*, char* , char);
void        strDelete(char*, int16_t, int16_t);
void        insertChar(char*, char, int16_t);
void        loadFontVNI(char*, uint8_t, uint8_t, uint8_t);
void        loadFontCHR(char*);
void        writeStr(uint16_t, uint16_t, uint8_t, char*);
void        writeCharVNI(uint16_t, uint16_t, char, uint32_t);
void        writeStrVNI(uint16_t, uint16_t, char*, uint32_t, uint8_t);
void        writeCharCHR(uint16_t, uint16_t, uint8_t, uint8_t, uint32_t);
void        writeStrCHR(uint16_t, uint16_t, char*, uint8_t, uint32_t, uint8_t);

// Basic drawing shape
uint16_t    createCode(int16_t, int16_t);
uint8_t     isEmpty(STACK*);
uint32_t*   getImageSize(int16_t, int16_t, int16_t, int16_t);

int16_t     scanLeft(int16_t, int16_t, uint32_t, uint32_t);
int16_t     scanRight(int16_t, int16_t, uint32_t, uint32_t);
void        moveTo(int16_t, int16_t);
void        lineTo(int16_t, int16_t, uint32_t);
void        drawLine(int16_t, int16_t, int16_t, int16_t, uint32_t);
void        horzLine(int16_t, int16_t, int16_t, uint32_t);
void        vertLine(int16_t, int16_t, int16_t, uint32_t);
void        clearScreen(int16_t, int16_t, int16_t, int16_t, uint32_t);
void        clearTextScreen();
void        drawCircle(int16_t, int16_t, int16_t, uint32_t);
void        drawEllipse(int16_t, int16_t, int16_t, int16_t, uint32_t);
void        drawRect(int16_t, int16_t, int16_t, int16_t, uint32_t);
void        drawBox(int16_t, int16_t, int16_t, int16_t, int16_t, int16_t, uint32_t);
void        drawPoly(POINT2D*, uint16_t, uint32_t);
void        floodFill(int16_t, int16_t, uint32_t, uint32_t);
void        initStack(STACK*);
void        push(STACK*, int16_t);
int16_t     pop(STACK*);
void        regionFill(int16_t, int16_t, uint32_t, uint32_t);
void        fillRect(int16_t, int16_t, int16_t, int16_t, uint32_t);
void        fillEllipse(int16_t, int16_t, int16_t, int16_t, uint32_t, uint32_t);
void        fillCircle(int16_t, int16_t, int16_t, uint32_t, uint32_t);
void        setViewPort(int16_t, int16_t, int16_t, int16_t);
void        clipLine(int16_t, int16_t, int16_t, int16_t, uint8_t);
void        getImage(int16_t, int16_t, int16_t, int16_t, uint32_t*);
void        putImage(int16_t, int16_t, uint32_t*);

// 3D transform
void        initProjection();
void        projette(double, double, double);
void        traceVers(double, double, double, uint32_t);
void        deplaceEn(double, double, double);

// Pointer to functions
void*       setBank;
void        (*putPixel)(uint16_t, uint16_t, uint32_t);
uint32_t    (*getPixel)(uint16_t, uint16_t);
uint32_t    (*fromRGB)(uint8_t, uint8_t, uint8_t);

// Global variable
VESAMODE    vesaMode;
VESAINFO    vesaInfo;

HEADER      CHRHeader;
FONTINFO    CHRInfo;
STROKE      strokes[1000] = {0};

uint8_t     bitsPerPixel;
uint16_t    bytesPerScanLine;
uint16_t    bankShifter = 0;
uint16_t    currReadBank = 0;
uint16_t    currWriteBank = 0;
uint16_t    cmaxX, cmaxY, centerX, centerY;
uint16_t    xUL, xLR, yUL, yLR;
uint8_t     charWidth, charHeight, charStart;
uint16_t    currX, currY;
uint8_t*    fontTable = NULL;
uint8_t*    videoMem = (uint8_t*)0xA0000000L;
uint8_t*    textMem = (uint8_t*)0xB8000000L;

// 3D projection
enum        projectionType {perspective, parallele};
double      DE, rho, theta, phi, aux1, aux2, aux3, aux4, aux5;
double      aux6, aux7, aux8, obsX, obsY, obsZ, projX, projY;

int16_t     projection;

uint16_t    cranXE, cranYE;
uint16_t    offsets[256] = {0};
uint8_t     widths[256] = {0};

char*       fileCHR;
FILE*       fpCHR;

// Implementation for VESA routine
uint8_t getVesaInfo()
{
    uint8_t val = 0;

    memcpy(vesaInfo.VESASignature, "VBE2", 4);
    
    __asm {
        mov     ax, seg vesaInfo
        mov     es, ax
        mov     di, offset vesaInfo
        mov     ax, 0x4F00
        int     0x10
        mov     val, al
    }

    return (val == 0x4F);
}

uint8_t getVesaModeInfo(uint16_t Mode)
{
    uint8_t val = 0;

    __asm {
        mov     ax, seg vesaMode
        mov     es, ax
        mov     di, offset vesaMode
        mov     ax, 0x4F01
        mov     cx, Mode
        int     0x10
        mov     val, al
    }

    if (val != 0x4F) return 0;

    bankShifter = vesaMode.WinGranularity ? 64 / vesaMode.WinGranularity : 1;
    
    return 1;
}

uint16_t findVesaMode(uint16_t XRes, uint16_t YRes, uint8_t bitsPerPixel)
{
    uint16_t *modePtr = vesaInfo.VideoModePtr;
    if (!modePtr) return 0;

    while (*modePtr != 0xFFFF)
    {
        if (!getVesaModeInfo(*modePtr)) break;
        if ((vesaMode.XResolution == XRes) && (vesaMode.YResolution == YRes) && (vesaMode.bitsPerPixel == bitsPerPixel)) return *modePtr;
        modePtr++;
    }

    return 0;
}

uint8_t setVesaMode(uint16_t mode)
{
    uint8_t val = 0;

    __asm {
        mov     ax, 0x4F02
        mov     bx, mode
        int     0x10
        mov     val, al
    }

    if (val != 0x4F) return 0;

    switch(vesaMode.bitsPerPixel)
    {
    case 8:
        putPixel = putPixel8;
        getPixel = getPixel8;
        fromRGB = NULL;
        break;

    case 15:
        putPixel = putPixel15;
        getPixel = getPixel15;
        fromRGB = fromRGB15;
        break;

    case 16:
        putPixel = putPixel16;
        getPixel = getPixel16;
        fromRGB = fromRGB16;
        break;

    case 24:
        putPixel = putPixel24;
        getPixel = getPixel24;
        fromRGB = fromRGB24;
        break;

    case 32:
        putPixel = putPixel32;
        getPixel = getPixel32;
        fromRGB = fromRGB32;
        break;
    }

    setBank = vesaMode.WinFunctionPtr;
    bitsPerPixel = vesaMode.bitsPerPixel;
    bytesPerScanLine = vesaMode.bytesPerScanLine;

    cmaxX = vesaMode.XResolution;
    cmaxY = vesaMode.YResolution;
    
    centerX = vesaMode.XResolution >> 1;
    centerY = vesaMode.YResolution >> 1;

    return 1;
}

uint16_t getVesaMode()
{
    uint16_t val;

    __asm {
        mov     ax, 0x4F03
        int     0x10
        mov     val, bx
    }

    return val;
}

uint8_t initGraph(uint16_t x, uint16_t y, uint8_t bitPerPixels)
{
    uint16_t Mode = 0;

    if (!getVesaInfo()) return 0;

    Mode = findVesaMode(x, y, bitPerPixels);

    if (Mode > 0) return setVesaMode(Mode);

    return 0;
}

void closeGraph()
{
    __asm {
        mov     ax, 0x03
        int     0x10
    }

    if (fontTable) free(fontTable);
}

void putPixel320(uint16_t x, uint16_t y, uint32_t color)
{
    __asm {
        les     di, videoMem
        mov     bx, y
        shl     bx, 6
        add     bh, byte ptr y
        add     bx, x
        add     di, bx
        mov     al, byte ptr [color]
        stosb
    }
}

void putPixel8(uint16_t x, uint16_t y, uint32_t color)
{
    __asm {
        mov     ax, y
        mov     cx, bytesPerScanLine
        mul     cx
        add     ax, x
        adc     dx, 0
        push  	ax
        cmp     dx, currWriteBank
        jz    	plot
        mov   	currWriteBank, dx
        mov     ax, dx
        mov     cx, bankShifter
        mul     cx
        mov     dx, ax
        mov     ax, 0x4F05
        xor     bx, bx
        call    setBank
    plot:
        les     di, videoMem
        pop     di
        mov     al, byte ptr [color]
        stosb
    }
}

void putPixel15(uint16_t x, uint16_t y, uint32_t color)
{
    __asm {
        mov     ax, y
        mov     cx, bytesPerScanLine
        mul     cx
        mov     bx, x
        shl     bx, 1
        add     ax, bx
        adc     dx, 0
        push  	ax
        cmp     dx, currWriteBank
        jz    	plot
        mov   	currWriteBank, dx
        mov     ax, dx
        mov     cx, bankShifter
        mul     cx
        mov     dx, ax
        mov     ax, 0x4F05
        xor     bx, bx
        call    setBank
    plot:
        les     di, videoMem
        pop     di
        mov     ax, word ptr color
        stosw
    }
}

void putPixel16(uint16_t x, uint16_t y, uint32_t color)
{
    __asm {
        mov     ax, y
        mov     cx, bytesPerScanLine
        mul     cx
        mov     bx, x
        shl     bx, 1
        add     ax, bx
        adc     dx, 0
        push  	ax
        cmp     dx, currWriteBank
        jz    	plot
        mov   	currWriteBank, dx
        mov     ax, dx
        mov     cx, bankShifter
        mul     cx
        mov     dx, ax
        mov     ax, 0x4F05
        xor     bx, bx
        call    setBank
    plot:
        les     di, videoMem
        pop     di
        mov     ax, word ptr color
        stosw
    }
}

void putPixel24(uint16_t x, uint16_t y, uint32_t color)
{
    uint8_t CheckCarry = 0;

    __asm {
        mov     ax, y
        mov     cx, bytesPerScanLine
        mul     cx
        mov     bx, x
        shl     bx, 1
        add     bx, x
        add     ax, bx
        adc     dx, 0
        push    ax
        cmp     dx, currWriteBank
        jz      plot
        mov     currWriteBank, dx
        mov     ax, dx
        mov     cx, bankShifter
        mul     cx
        mov     dx, ax
        mov     ax, 0x4F05
        xor     bx, bx
        call    setBank
    plot:
        les     di, videoMem
        pop     di
        mov     al, byte ptr [color]
        stosb
        jnc     nocarry1
        jmp     switchbank
    nocarry1:
        mov     CheckCarry, 1
        mov     al, byte ptr [color + 1]
        stosb
        jnc     nocarry2
        jmp     switchbank
    nocarry2:
        mov     al, byte ptr [color + 2]
        stosb
        jmp     done
    switchbank:
        mov     ax, currWriteBank
        inc     ax
        mov     currWriteBank, ax
        mov     cx, bankShifter
        mul     cx
        mov     dx, ax
        mov     ax, 0x4F05
        xor     bx, bx
        call    setBank
        cmp     CheckCarry, 0
        jz      nocarry1
        cmp     CheckCarry, 1
        je      nocarry2
    done:
    }
}

void putPixel32(uint16_t x, uint16_t y, uint32_t color)
{
    __asm {
        mov     ax, y
        mov     cx, bytesPerScanLine
        mul     cx
        mov     bx, x
        shl     bx, 2
        add     ax, bx
        adc     dx, 0
        push    ax
        cmp     dx, currWriteBank
        jz      plot
        mov     currWriteBank, dx
        mov     ax, dx
        mov     cx, bankShifter
        mul     cx
        mov     dx, ax
        mov     ax, 0x4F05
        xor     bx, bx
        call    setBank
    plot:
        les     di, videoMem
        pop     di
        mov     al, byte ptr [color]
        stosb
        mov     al, byte ptr [color + 1]
        stosb
        mov     al, byte ptr [color + 2]
        stosb
    }
}

uint32_t getPixel320(uint16_t x, uint16_t y)
{
    uint8_t val = 0;

    __asm {
        push    ds
        lds     si, videoMem
        mov     bx, y
        shl     bx, 6
        add     bh, byte ptr y
        add     bx, x
        add     si, bx
        lodsb
        mov     val, al
        pop     ds
    }

    return val;
}

uint32_t getPixel8(uint16_t x, uint16_t y)
{
    uint8_t val = 0;

    __asm {
        push    ds
        mov     ax, y
        mov     cx, bytesPerScanLine
        mul     cx
        add     ax, x
        adc     dx, 0
        push    ax
        cmp     dx, currReadBank
        jz      plot
        mov     currReadBank, dx
        mov     ax, dx
        mov     cx, bankShifter
        mul     cx
        mov     dx, ax
        mov     ax, 0x4F05
        xor     bx, bx
        call    setBank
    plot:
        lds     si, videoMem
        pop     si
        lodsb
        mov     val, al
        pop     ds
    }

    return val;
}

uint32_t getPixel15(uint16_t x, uint16_t y)
{
    uint16_t val = 0;

    __asm {
        push    ds
        mov     ax, y
        mov     cx, bytesPerScanLine
        mul     cx
        mov     bx, x
        shl     bx, 1
        add     ax, bx
        adc     dx, 0
        push    ax
        cmp     dx, currReadBank
        jz      plot
        mov     currReadBank, dx
        mov     ax, dx
        mov     cx, bankShifter
        mul     cx
        mov     dx, ax
        mov     ax, 0x4F05
        xor     bx, bx
        call    setBank
    plot:
        lds     si, videoMem
        pop     si
        lodsw
        mov     val, ax
        pop     ds
    }

    return val;
}

uint32_t getPixel16(uint16_t x, uint16_t y)
{
    uint16_t val = 0;

    __asm {
        push    ds
        mov     ax, y
        mov     cx, bytesPerScanLine
        mul     cx
        mov     bx, x
        shl     bx, 1
        add     ax, bx
        adc     dx, 0
        push    ax
        cmp     dx, currReadBank
        jz      plot
        mov     currReadBank, dx
        mov     ax, dx
        mov     cx, bankShifter
        mul     cx
        mov     dx, ax
        mov     ax, 0x4F05
        xor     bx, bx
        call    setBank
    plot:
        lds     si, videoMem
        pop     si
        lodsw
        mov     val, ax
        pop     ds
    }

    return val;
}

uint32_t getPixel24(uint16_t x, uint16_t y)
{
    uint32_t color = 0;
    uint8_t checkCarry = 0;
    
    __asm {
        push    ds
        mov     ax, y
        mov     cx, bytesPerScanLine
        mul     cx
        mov     bx, x
        shl     bx, 1
        add     bx, x
        add     ax, bx
        adc     dx, 0
        push    ax
        cmp     dx, currReadBank
        jz      plot
        mov     currReadBank, dx
        mov     ax, dx
        mov     cx, bankShifter
        mul     cx
        mov     dx, ax
        mov     ax, 0x4F05
        xor     bx, bx
        call    setBank
    plot:
        lds     si, videoMem
        pop     si
        lodsb
        mov     byte ptr [color], al
        jnc     nocarry1
        jmp     switchbank
    nocarry1:
        mov     checkCarry, 1
        lodsb
        mov     byte ptr [color + 1], al
        jnc     nocarry2
        jmp     switchbank
    nocarry2:
        lodsb
        mov     byte ptr [color + 2], al
        jmp     done
    switchbank:
        mov     ax, currReadBank
        inc     ax
        mov     currReadBank, ax
        mov     cx, bankShifter
        mul     cx
        mov     dx, ax
        mov     ax, 0x4F05
        xor     bx, bx
        call    setBank
        cmp     checkCarry, 0
        jz      nocarry1
        cmp     checkCarry, 1
        je      nocarry2
    done:
        pop     ds
    }

    return color;
}

uint32_t getPixel32(uint16_t x, uint16_t y)
{
    uint32_t color = 0;

    __asm {
        push    ds
        mov     ax, y
        mov     cx, bytesPerScanLine
        mul     cx
        mov     bx, x
        shl     bx, 2
        add     ax, bx
        adc     dx, 0
        push    ax
        cmp     dx, currReadBank
        jz      plot
        mov     currReadBank, dx
        mov     ax, dx
        mov     cx, bankShifter
        mul     cx
        mov     dx, ax
        mov     ax, 0x4F05
        xor     bx, bx
        call    setBank
    plot:
        lds     si, videoMem
        pop     si
        lodsb
        mov     byte ptr [color], al
        lodsb
        mov     byte ptr [color + 1], al
        lodsb
        mov     byte ptr [color + 2], al
        pop     ds
    }

    return color;
}

uint32_t fromRGB15(uint8_t R, uint8_t G, uint8_t B)
{
    uint16_t val = 0;

    __asm {
        xor     ah, ah
        mov     al, R
        shr     al, 3
        shl     ax, 10
        mov     bx, ax
        xor     ah, ah
        mov     al, G
        shr     al, 3
        shl     ax, 5
        add     bx, ax
        xor     ah, ah
        mov     al, B
        shr     al, 3
        add     bx, ax
        mov     val, bx
    }

    return val;
}

uint32_t fromRGB16(uint8_t R, uint8_t G, uint8_t B)
{
    uint16_t val = 0;

    __asm {
        xor     ah, ah
        mov     al, R
        shr     al, 3
        shl     ax, 11
        mov     bx, ax
        xor     ah, ah
        mov     al, G
        shr     al, 2
        shl     ax, 5
        add     bx, ax
        xor     ah, ah
        mov     al, B
        shr     al, 3
        add     bx, ax
        mov     val, bx
    }

    return val;
}

uint32_t fromRGB24(uint8_t R, uint8_t G, uint8_t B)
{
    uint32_t val = 0;

    __asm {
        mov     al, R
        mov     byte ptr [val + 2], al
        mov     al, G
        mov     byte ptr [val + 1], al
        mov     al, [B]
        mov     byte ptr [val], al
    }

    return val;
}

uint32_t fromRGB32(uint8_t R, uint8_t G, uint8_t B)
{
    uint32_t val = 0;

    __asm {	
        mov     al, R
        mov     byte ptr [val + 2], al
        mov     al, G
        mov     byte ptr [val + 1], al
        mov     al, B
        mov     byte ptr [val], al
    }

    return val;
}

void clearTextScreen()
{
    __asm {
        les     di, textMem
        xor     ax, ax
        mov     cx, 1000
        rep     stosd
    }
}

void waitKeyPressed(uint16_t key)
{
    __asm {
        mov     dx, 0x60
        xor     ah, ah
    next:
        in      al, dx
        cmp     ax, key
        jne     next
    }
}

void writeStr(uint16_t X, uint16_t Y, uint8_t col, char *msg)
{
    __asm {
        push    ds
        les     di, textMem
        lds     si, msg
        add     di, X
        shl     di, 1
        mov     bx, Y
        shl     bx, 5
        add     di, bx
        shl     bx, 2
        add     di, bx
        mov     ah, col
    next:
        lodsb
        test    al, al
        jz      quit
        stosw
        jmp     next
    quit:
        pop     ds
    }
}

void setPalette(RGB *pal, uint8_t start, uint8_t count)
{
    __asm {
        push    ds
        mov     dx, 0x03C8
        mov     al, start
        out     dx, al
        inc     dx
        mov     bl, 3
        mul     bl
        lds     si, pal
        add     si, ax
        mov     al, 3
        mov     cl, count
        mul     cl
        mov     cx, ax
        rep     outsb
        pop     ds
    }
}

void getPalette(RGB *pal, uint8_t start, uint8_t count)
{
    __asm {
        mov     dx, 0x3C7
        mov     al, start
        out     dx, al
        add     dx, 2
        mov     bl, 3
        mul     bl
        les     di, pal
        add     di, ax
        mov     al, 3
        mov     cl, count
        mul     cl
        mov     cx, ax
        rep     insb
    }
}

void movePalette(RGB *dst, RGB *src, uint16_t len)
{
    __asm {
        push    ds
        les     di, dst
        lds     si, src
        mov     cx, len
        mov     ax, cx
        shr     cx, 2
        rep     movsd
        mov     cl, al
        and     cl, 0x03
        rep     movsb
        pop     ds
    }
}

void copyPalette(RGB *pal1, RGB *pal2, uint8_t start, uint8_t count)
{
    __asm {
        push    ds
        lds     si, pal2
        les     di, pal1
        mov     al, 3
        mov     bl, start
        mul     bl
        add     si, ax
        add     di, ax
        mov     al, 3
        mov     cl, count
        mul     cl
        mov     cx, ax
        shr     cx, 2
        rep     movsd
        mov     cl, al
        and     cl, 0x03
        rep     movsb
        pop     ds
    }
}

void clearPalette(RGB *pal, uint8_t start, uint8_t count)
{
    __asm {
        les     di, pal
        mov     al, start
        mov     bl, 3
        mul     bl
        add     di, ax
        mov     al, 3
        mov     cl, count
        mul     cl
        mov     cx, ax
        mov     bx, ax
        xor     ax, ax
        shr     cx, 2
        rep     stosd
        mov     cl, bl
        and     cl, 0x03
        rep     stosb
    }
}

void waitRetrace()
{
    __asm {
        mov     dx, 0x03DA
    waitH:
        in      al, dx
        test    al, 0x08
        jnz     waitH
    waitV:
        in      al, dx
        test    al, 0x08
        jz      waitV
    }
}

void fadeIn(RGB *pal1, RGB *pal2, uint8_t start, uint8_t count)
{
    __asm {
        push    ds
        mov     cx, 64
        mov     dx, 64
        mov     al, 3
        mov     bl, start
        mul     bl
        lds     si, pal2
        les     di, pal1
        add     si, ax
        add     di, ax
    next:
        push    cx
        push    si
        push    di
        dec     dx
        mov     al, 3
        mov     cl, count
        mul     cl
        mov     cx, ax
        xor     bx, bx
    cont:
        lodsb
        inc     bx
        cmp     ax, dx
        jbe     quit
        inc     byte ptr es:[di + bx - 1]
    quit:
        loop    cont
        push    dx
        mov     dx, 0x03DA
    waitH:
        in      al, dx
        test    al, 0x08
        jnz     waitH
    waitV:
        in      al, dx
        test    al, 0x08
        jz      waitV
        mov     dx, 0x03C8
        mov     al, start
        out     dx, al
        inc     dx
        mov     bl, 3
        mul     bl
        lds     si, pal1
        add     si, ax
        mov     al, 3
        mov     cl, count
        mul     cl
        mov     cx, ax
        rep     outsb
        pop     dx
        pop     di
        pop     si
        pop     cx
        loop    next
        pop     ds
    }
}

void fadeOut(RGB *pal1, RGB *pal2, uint8_t start, uint8_t count)
{
    __asm {
        push    ds
        mov     cx, 64
        mov     dx, 64
        mov     al, 3
        mov     bl, start
        mul     bl
        lds     si, pal2
        les     di, pal1
        add     si, ax
        add     di, ax
    next:
        push    cx
        push    si
        push    di
        dec     dx
        mov     al, 3
        mov     cl, count
        mul     cl
        mov     cx, ax
        xor     bx, bx
    cont:
        lodsb
        inc     bx
        cmp     ax, dx
        jae     quit
        dec     byte ptr es:[di + bx - 1]
    quit:
        loop    cont
        push    dx
        mov     dx, 0x03DA
    waitH:
        in      al, dx
        test    al, 0x08
        jnz     waitH
    waitV:
        in      al, dx
        test    al, 0x08
        jz      waitV
        mov     dx, 0x03C8
        mov     al, start
        out     dx, al
        inc     dx
        mov     bl, 3
        mul     bl
        lds     si, pal1
        add     si, ax
        mov     al, 3
        mov     cl, count
        mul     cl
        mov     cx, ax
        rep     outsb
        pop     dx
        pop     di
        pop     si
        pop     cx
        loop    next
        pop     ds
    }
}

void fadeMax(RGB *pal, uint8_t start, uint8_t count)
{
    __asm {
        push    ds
        lds     si, pal
        mov     al, 3
        mov     bl, start
        mul     bl
        add     si, ax
        mov     cx, 64
    next:
        push    cx
        push    si
        mov     al, 3
        mov     cl, count
        mul     cl
        mov     cx, ax
    cont:
        lodsb
        cmp     al, 63
        jae     quit
        inc     byte ptr ds:[si - 1]
    quit:
        loop    cont
        mov     dx, 0x03DA
    waitH:
        in      al, dx
        test    al, 0x08
        jnz     waitH
    waitV:
        in      al, dx
        test    al, 0x08
        jz      waitV
        mov     dx, 0x03C8
        mov     al, start
        out     dx, al
        inc     dx
        mov     bl, 3
        mul     bl
        lds     si, pal
        add     si, ax
        mov     al, 3
        mov     cl, count
        mul     cl
        mov     cx, ax
        rep     outsb
        pop     si
        pop     cx
        loop    next
        pop     ds
    }
}

void fadeMin(RGB *pal, uint8_t start, uint8_t count)
{
    __asm {
        push    ds
        lds     si, pal
        mov     al, 3
        mov     bl, start
        mul     bl
        add     si, ax
        mov     cx, 64
    next:
        push    cx
        push    si
        mov     al, 3
        mov     cl, count
        mul     cl
        mov     cx, ax
    cont:
        lodsb
        test    al, al
        jz      quit
        dec     byte ptr ds:[si - 1]
    quit:
        loop  	cont
        mov     dx, 0x03DA
    waitH:
        in      al, dx
        test    al, 0x08
        jnz     waitH
    waitV:
        in      al, dx
        test    al, 0x08
        jz      waitV
        mov     dx, 0x03C8
        mov     al, start
        out     dx, al
        inc     dx
        mov     bl, 3
        mul     bl
        lds     si, pal
        add     si, ax
        mov     al, 3
        mov     cl, count
        mul     cl
        mov     cx, ax
        rep     outsb
        pop     si
        pop     cx
        loop    next
        pop     ds
    }
}

void DisplayVesaInfo()
{
    int16_t i = 0, j = 1;

    if (!getVesaInfo())
    {
        printf("Sorry, VESA does not installed on your card!");
        exit(1);
    }

    printf("\t\t\tVideo BIOS Extention Information");
    printf("\nSignature: %s", vesaInfo.VESASignature);
    printf("\nGPU type: %s", vesaInfo.OEMStringPtr);
    printf("\nVESA version: %u.%u", vesaInfo.VESAVersion >> 8, vesaInfo.VESAVersion & 0xFF);
    printf("\nRAM video: %uMB\n\n", vesaInfo.TotalMemory >> 4);
    printf("Mode      Active  Resolution  Bits/Pixel  Colors\n");

    while (vesaInfo.VideoModePtr[i] != 0xFFFF)
    {
        getVesaModeInfo(vesaInfo.VideoModePtr[i]) && (vesaMode.ModeAttributes & 0x01) ?
        printf("%3Xh     Present   ", vesaInfo.VideoModePtr[i]) :
        printf("%3Xh NOT Present   ", vesaInfo.VideoModePtr[i]);
        printf("%4ux%4u%8u bpp  ", vesaMode.XResolution, vesaMode.YResolution, vesaMode.bitsPerPixel);

        switch (vesaMode.MemoryModel)
        {
            case 0x00: (vesaMode.ModeAttributes & 0x08) ? printf("Color Text") : printf("Black & White Text"); break;
            case 0x01: printf("CGA graphics"); break;
            case 0x02: printf("HGC graphics"); break;
            case 0x03: printf("16 colors graphics"); break;
            case 0x04: printf("256 colors Packed Pixel graphics"); break;
            case 0x05: printf("Non-Chain 4, 256 colors graphics"); break;
            case 0x06: printf("%lu direct Color Graphics", (2UL << vesaMode.bitsPerPixel - 1) - 1); break;
            case 0x07: printf("YUV"); break;
            case 0x08:
            case 0x09:
            case 0x0A:
            case 0x0B:
            case 0x0C:
            case 0x0D:
            case 0x0E:
            case 0x0F: printf("Reserved for VESA"); break;
            default	: printf("OEM memory model");
        }

        i++;
        j++;

        if (!(j % 18))
        {
            printf("\nPress any key to continue...");
            getch();
        }

        printf("\n");
    }
}

void rainbowPalette(RGB *pal)
{
    int16_t i;
    for (i = 0; i != 32; i++)
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
}

void linearPalette(RGB *pal)
{
    int16_t i, j;

    for (i = 0; i < 32; i++)
    {
        j =  16 + i;
        pal[j].r = 63;
        pal[j].g = 0;
        pal[j].b = 63 - (i << 1);

        j =  48 + i;
        pal[j].r = 63;
        pal[j].g = (i << 1);
        pal[j].b = 0;

        j =  80 + i;
        pal[j].r = 63 - (i << 1);
        pal[j].g = 63;
        pal[j].b = 0;

        j = 112 + i;
        pal[j].r = 0;
        pal[j].g = 63;
        pal[j].b = (i << 1);

        j = 144 + i;
        pal[j].r = 0;
        pal[j].g = 63 - (i << 1);
        pal[j].b = 63;

        j = 176 + i;
        pal[j].r = (i << 1);
        pal[j].g = 0;
        pal[j].b = 63;
    }   
}

void rotatePalette(RGB *pal, uint8_t start, uint8_t count, uint8_t times, uint16_t rept)
{
    RGB tmp;

    if (!rept) while (!kbhit())
    {
        tmp = pal[start];
        movePalette(&pal[start], &pal[start + 1], (count - 1) * sizeof(RGB));
        pal[count + start - 1] = tmp;
        waitRetrace();
        setPalette(pal, start, count);
        if (times) delay(times);
    }
    else while (rept--)
    {
        tmp = pal[start];
        movePalette(&pal[start], &pal[start + 1], (count - 1) * sizeof(RGB));
        pal[count + start - 1] = tmp;
        waitRetrace();
        setPalette(pal, start, count);
        if (times) delay(times);
    }
}

void horzLine(int16_t x1, int16_t x2, int16_t y, uint32_t color)
{
    int16_t i;
    for (i = x1; i <= x2; i++) putPixel(i, y, color);
}

void vertLine(int16_t x, int16_t y1, int16_t y2, uint32_t color)
{
    int16_t i;
    for (i = y1; i <= y2; i++) putPixel(x, i, color);
}

void drawLine(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint32_t color)
{
    int16_t d, sign;
    int16_t x, y, dx, dy, dt, ds;
    
    if (abs(x2 - x1) < abs(y2 - y1))
    {

        if (y1 > y2)
        {
            Swap(x1, x2);
            Swap(y1, y2);
        }

        x = x1;
        y = y1;

        dx = abs(x2 - x1);
        dy = y2 - y1;

        dt = (dx - dy) << 1;
        ds = dx << 1;

        d = (dx << 1) - dy;
        sign = (x2 > x1) ? 1 : -1;

        putPixel(x, y, color);

        for (y = y1 + 1; y <= y2; y++)
        {
            if (d >= 0)
            {
                x += sign;
                d += dt;
            } else d += ds;

            putPixel(x, y, color);
        }
    }
    else
    {
        if (x1 > x2)
        {
            Swap(x1, x2);
            Swap(y1, y2);
        }

        x = x1;
        y = y1;

        dx = x2 - x1;
        dy = abs(y2 - y1);

        dt = (dy - dx) << 1;
        ds = dy << 1;

        d = (dy << 1) - dx;
        sign = (y2 > y1) ? 1 : -1;

        putPixel(x, y, color);

        for (x = x1 + 1; x <= x2; x++)
        {
            if (d >= 0)
            {
                y += sign;
                d += dt;
            } else d += ds;

            putPixel(x, y, color);
        }
    }
}

void moveTo(int16_t x, int16_t y)
{
    currX = x;
    currY = y;
}

void lineTo(int16_t x, int16_t y, uint32_t color)
{
    drawLine(currX, currY, x, y, color);
    moveTo(x, y);
}

void clearScreen(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint32_t color)
{
    int16_t i;

    if (x1 > x2) Swap(x1, x2);
    if (y1 > y2) Swap(y1, y2);

    for (i = y1; i <= y2; i++) horzLine(x1, x2, i, color);
}

void drawCircle(int16_t xc, int16_t yc, int16_t r, uint32_t color)
{
    int16_t x = 0, y = r;
    int16_t p = 1 - r;

    while (x <= y)
    {
        putPixel(xc + x, yc + y, color);
        putPixel(xc - x, yc + y, color);
        putPixel(xc + x, yc - y, color);
        putPixel(xc - x, yc - y, color);

        putPixel(xc + y, yc + x, color);
        putPixel(xc - y, yc + x, color);
        putPixel(xc + y, yc - x, color);
        putPixel(xc - y, yc - x, color);

        if (p < 0) p += (x << 1) + 3;
        else
        {
            p += ((x - y) << 1) + 5;
            y--;
        }

        x++;
    }
}

void drawEllipse(int16_t xc, int16_t yc, int16_t a, int16_t b, uint32_t color)
{
    int16_t x = 0, y = b;
    double aa = a * a, bb = b * b, aa2 = 2 * aa, bb2 = 2 * bb;
    double fx = 0.0, fy = aa2 * b, p = bb - aa * b + 0.25 * aa;

    while (fx < fy)
    {
        putPixel(xc + x, yc + y, color);
        putPixel(xc - x, yc - y, color);
        putPixel(xc - x, yc + y, color);
        putPixel(xc + x, yc - y, color);

        x++;
        fx += bb2;

        if (p < 0) p += fx + bb;
        else
        {
            y--;
            fy -= aa2;
            p = p + fx + bb - fy;
        }
    }

    putPixel(xc + x, yc + y, color);
    putPixel(xc - x, yc - y, color);
    putPixel(xc + x, yc - y, color);
    putPixel(xc - x, yc + y, color);

    p = bb * (x + 0.5) * (x + 0.5) + aa * (y -1) * (y - 1) - aa * bb;

    while (y > 0)
    {
        y--;
        fy -= aa2;

        if (p >= 0) p = p - fy + aa;
        else
        {
            x++;
            fx += bb2;
            p = p - fy + aa + fx;
        }

        putPixel(xc + x, yc + y, color);
        putPixel(xc - x, yc - y, color);
        putPixel(xc + x, yc - y, color);
        putPixel(xc - x, yc + y, color);
    }
}

void drawRect(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint32_t color)
{
    if (x1 > x2) Swap(x1, x2);
    if (y1 > y2) Swap(y1, y2);

    horzLine(x1, x2, y1, color);
    vertLine(x2, y1, y2, color);

    horzLine(x1, x2, y2, color);
    vertLine(x1, y1, y2, color);
}

void drawBox(int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t dx, int16_t dy, uint32_t color)
{
    int16_t x11, y11;
    int16_t x22, y22;

    x11 = x1 + dx;
    y11 = y1 - dy;

    x22 = x2 + dx;
    y22 = y2 - dy;

    drawRect(x1, y1, x2, y2, color);
    drawRect(x11, y11, x22, y22, color);

    drawLine(x1, y1, x11, y11, color);
    drawLine(x2, y1, x22, y11, color);

    drawLine(x2, y2, x22, y22, color);
    drawLine(x1, y2, x11, y22, color);
}

void drawPoly(POINT2D *points, uint16_t num, uint32_t color)
{
    int16_t i;

    if (num <= 2) return;

    for (i = 0; i < num; i++)
    {
        drawLine(points[i].x, points[i].y, points[(i + 1) % num].x, points[(i + 1) % num].y, color);
    }
}

void floodFill(int16_t seedX, int16_t seedY, uint32_t borderColor, uint32_t fillColor)
{
    uint32_t currColor = getPixel(seedX, seedY);

    if (currColor != borderColor && currColor != fillColor)
    {
        putPixel(seedX, seedY, fillColor);
        floodFill(seedX - 1, seedY    , borderColor, fillColor);
        floodFill(seedX + 1, seedY    , borderColor, fillColor);
        floodFill(seedX    , seedY + 1, borderColor, fillColor);
        floodFill(seedX    , seedY - 1, borderColor, fillColor);
    }
}

uint32_t getTicks()
{
    uint32_t ticks = 0;
    __asm {
        xor ax, ax
        mov es, ax
        mov bx, 46Ch
        mov eax, dword ptr es:[bx]
        mov ticks, eax
    }
    return ticks;
}

void initStack(STACK *stack)
{
    int16_t i;

    for (i = 0; i < 2000; i++) stack->data[i] = -1;
    stack->count = 0;
}

void push(STACK *stack, int16_t item)
{
    stack->data[stack->count++] = item;
    if (stack->count > 2000) exit(1);
}

int16_t pop(STACK *stack)
{
    if (stack->count == 0) exit(1);
    return stack->data[--stack->count];
}

uint8_t isEmpty(STACK *stack)
{
    return (stack->count == 0);
}

int16_t scanLeft(int16_t sx, int16_t sy, uint32_t fillColor, uint32_t borderColor)
{
    uint32_t currColor;

    do {
        sx--;
        currColor = getPixel(sx, sy);
    } while (currColor != borderColor && currColor != fillColor && sx > 0);

    return (!sx) ? -1 : ++sx;
}

int16_t scanRight(int16_t sx, int16_t sy, uint32_t fillColor, uint32_t borderColor)
{
     uint32_t currColor;

     do {
        sx++;
        currColor = getPixel(sx, sy);
     } while (currColor != borderColor && currColor != fillColor && sx < 5000);

     return (sx == 5000) ? -1 : --sx;
}

void regionFill(int16_t seedX, int16_t seedY, uint32_t borderColor, uint32_t fillColor)
{
    STACK stack;
    int16_t sx, sy;
    int16_t x, y;
    int16_t xl, xll;
    int16_t xr, xrr;

    initStack(&stack);
    push(&stack, seedX);
    push(&stack, seedY);

    while (!isEmpty(&stack))
    {
        sy = pop(&stack);
        sx = pop(&stack);

        xl = scanLeft(sx, sy, fillColor, borderColor);
        xr = scanRight(sx, sy, fillColor, borderColor);

        for (x = xl; x <= xr; x++) putPixel(x, sy, fillColor);
        sy++;

        xll = scanLeft((xl + xr) / 2, sy, fillColor, borderColor);
        xrr = scanRight((xl + xr) / 2, sy, fillColor, borderColor);

        if (xll == -1 || xrr == 5000) break;

        if (xll < xrr)
        {
            push(&stack, (xll + xrr) / 2);
            push(&stack, sy);
         }
     }

     initStack(&stack);
     push(&stack, seedX);
     push(&stack, seedY - 1);

     while (!isEmpty(&stack))
     {
         sy = pop(&stack);
         sx = pop(&stack);

         xl = scanLeft(sx, sy, fillColor, borderColor);
         xr = scanRight(sx, sy, fillColor, borderColor);

         for (x = xl; x <= xr; x++) putPixel(x, sy, fillColor);
         sy--;

         xll = scanLeft((xl + xr) / 2, sy, fillColor, borderColor);
         xrr = scanRight((xl + xr) / 2, sy, fillColor, borderColor);

         if (xll == -1 || xrr == 5000) break;

         if (xll < xrr)
         {
            push(&stack, (xll + xrr) / 2);
            push(&stack, sy);
         }
     }
}

void fillRect(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint32_t fillColor)
{
    int16_t i;

    if (x1 > x2) Swap(x1, x2);
    if (y1 > y2) Swap(y1, y2);

    for (i = y1; i <= y2; i++) horzLine(x1, x2, i, fillColor);
}

void fillEllipse(int16_t xc, int16_t yc, int16_t a, int16_t b, uint32_t bcol, uint32_t fcol)
{
  int16_t limit, flag, i, j;
  int16_t xm, ym, xmx, ymx;

  drawEllipse(xc, yc, a, b, bcol);

  xm = xc - a;
  ym = yc - b;

  xmx = xc + a;
  ymx = yc + b;

  for (i = ym + 1; i < ymx; i++)
  {
      limit = 0;
      flag = 1;

      for (j = xm; j < xmx; j++)
      {
            if (getPixel(j, i) == bcol)
            {
                if (flag)
                {
                    limit++;
                    flag = 0;
                }
            }
            else if (limit)
            {
                putPixel(j, i, fcol);
                flag = 1;
            }

            if (limit > 1) break;
      }
  }
}

void fillCircle(int16_t xc, int16_t yc, int16_t r, uint32_t bcol, uint32_t fcol)
{
  int16_t limit, flag, i, j;
  int16_t xm, ym, xmx, ymx;

  drawCircle(xc, yc, r, bcol);

  xm = xc - r;
  ym = yc - r;

  xmx = xc + r;
  ymx = yc + r;

  for (i = ym + 1; i < ymx; i++)
  {
      limit = 0; flag = 1;
      for (j = xm; j < xmx; j++)
      {
            if (getPixel(j, i) == bcol)
            {
                if (flag)
                {
                    limit++;
                    flag = 0;
                }
            }
            else if (limit)
            {
                putPixel(j, i, fcol);
                flag = 1;
            }

            if (limit > 1) break;
      }
  }
}

void setViewPort(int16_t x1, int16_t y1, int16_t x2, int16_t y2)
{
    xUL = x1;
    yUL = y1;

    xLR = x2;
    yLR = y2;
}

uint16_t createCode(int16_t x, int16_t y)
{
    int16_t code = 0;

    if (x < xUL)      	code = LEFT;
    else if (x > xLR)   code = RIGHT;

    if (y < yUL)      	code |= HIGH;
    else if (y > yLR)   code |= LOW;

    return code;
}

void clipLine(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint8_t clipOn)
{
    int16_t x, y;
    int16_t code, code1, code2;
    
    if (clipOn)
    {
        code1 = createCode(x1, y1);
        code2 = createCode(x2, y2);

        while (code1 || code2)
        {
            if (code1 & code2) break;

            code = (!code1) ? code2 : code1;

            if (code & LEFT)
            {
                x = xUL;
                y = y1 + (y2 - y1) * (xUL - x1) / (x2 - x1);
            }
            else if (code & RIGHT)
            {
                x = xLR;
                y = y1 + (y2 - y1) * (xLR - x1) / (x2 - x1);
            }

            if (code & LOW)
            {
                y = yLR; x = x1 + (x2 - x1) * (yUL - y1) / (y2 - y1);
            }

            if (code == code1)
            {
                x1 = x;
                y1 = y;
                code1 = createCode(x, y);
            }
        }
    }
}

void getImage(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint32_t *buffer)
{
    int16_t i, j;

    buffer[0] = y2 - y1;
    buffer[1] = x2 - x1;

    for (i = 0; i < y2 - y1; i++)
        for (j = 0; j < x2 - x1; j++)
            buffer[i * (y2 - y1) + j + 2] = getPixel(x1 + j, y1 + i);
}

uint32_t *newImage(int16_t x1, int16_t y1, int16_t x2, int16_t y2)
{
    uint32_t *Block;
    int16_t dy = y2 - y1;
    int16_t dx = x2 - x1;

    Block = (uint32_t*)malloc(dy * dx * sizeof(int32_t) + 2);
    return Block;
}

void putImage(int16_t x, int16_t y, uint32_t *buffer)
{
    int16_t i, j;

    for (i = 1; i < buffer[0]; i++)
        for (j = 0; j < buffer[1]; j++)
            putPixel(x + j, y + i, buffer[i * buffer[1] + j]);
}

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

void projette(double X, double Y, double Z)
{
    obsX = -X * aux1 + Y * aux3;
    obsY = -X * aux5 - Y * aux6 + Z * aux4;

    if (projection == perspective)
    {
        obsZ = -X * aux7 - Y * aux8 - Z * aux2 + rho;
        projX = DE * obsX / obsZ;
        projY = DE * obsY / obsZ;
    }
    else
    {
        projX = DE * obsX;
        projY = DE * obsY;
    }
}

void deplaceEn(double X, double Y, double Z)
{
    projette(X, Y, Z);

    cranXE = projX * ECHE + (cmaxX >> 1);
    cranYE = (cmaxY >> 1) - projY;

    moveTo(cranXE, cranYE);
}

void traceVers(double X, double Y, double Z, uint32_t Colour)
{
    projette(X, Y, Z);

    cranXE = projX * ECHE + (cmaxX >> 1);
    cranYE = (cmaxY >> 1) - projY;

    lineTo(cranXE, cranYE, Colour);
}

// Implementation for font routine
int16_t strPos(char *str, char *szSubstr)
{
    char *ptr = strstr(str, szSubstr);
    return (!ptr) ? -1 : (ptr - str);
}

void insertChar(char *str, char chr, int16_t iPos)
{
    if (iPos < 0 || iPos > strlen(str)) return;
    *(str + iPos) = chr;
}

void strDelete(char *str, int16_t i, int16_t numChar)
{
    if (i < 0 || i > strlen(str)) return;
    memmove((str + i + 1), (str + i + numChar), (strlen(str) - i - 1));
}

void schRepl(char *str, char *sch, char repl)
{
    int16_t pos;

    do {
        pos = strPos(str, sch);
        if (pos >= 0)
        {
            strDelete(str, pos, strlen(sch));
            insertChar(str, repl, pos);
        }
    } while(pos >= 0);
}

char *chr2Str(char chr, char n)
{
    char *szString = (char*)malloc(3);

    *szString = chr;
    *(szString + 1) = n;
    *(szString + 2) = 0;

    return szString;
}

void fontVNI(char *szPrmpt)
{
    schRepl(szPrmpt, "a8", 128);
    schRepl(szPrmpt, chr2Str(128, '1'), 129);
    schRepl(szPrmpt, chr2Str(128, '2'), 130);
    schRepl(szPrmpt, chr2Str(128, '3'), 131);
    schRepl(szPrmpt, chr2Str(128, '4'), 132);
    schRepl(szPrmpt, chr2Str(128, '5'), 133);

    schRepl(szPrmpt, "a6", 134);
    schRepl(szPrmpt, chr2Str(134, '1'), 135);
    schRepl(szPrmpt, chr2Str(134, '2'), 136);
    schRepl(szPrmpt, chr2Str(134, '3'), 137);
    schRepl(szPrmpt, chr2Str(134, '4'), 138);
    schRepl(szPrmpt, chr2Str(134, '5'), 139);

    schRepl(szPrmpt, "e6", 140);
    schRepl(szPrmpt, chr2Str(140, '1'), 141);
    schRepl(szPrmpt, chr2Str(140, '2'), 142);
    schRepl(szPrmpt, chr2Str(140, '3'), 143);
    schRepl(szPrmpt, chr2Str(140, '4'), 144);
    schRepl(szPrmpt, chr2Str(140, '5'), 145);

    schRepl(szPrmpt, "o7", 146);
    schRepl(szPrmpt, chr2Str(146, '1'), 147);
    schRepl(szPrmpt, chr2Str(146, '2'), 148);
    schRepl(szPrmpt, chr2Str(146, '3'), 149);
    schRepl(szPrmpt, chr2Str(146, '4'), 150);
    schRepl(szPrmpt, chr2Str(146, '5'), 151);

    schRepl(szPrmpt, "o6", 152);
    schRepl(szPrmpt, chr2Str(152, '1'), 153);
    schRepl(szPrmpt, chr2Str(152, '2'), 154);
    schRepl(szPrmpt, chr2Str(152, '3'), 155);
    schRepl(szPrmpt, chr2Str(152, '4'), 156);
    schRepl(szPrmpt, chr2Str(152, '5'), 157);

    schRepl(szPrmpt, "u7", 158);
    schRepl(szPrmpt, chr2Str(158, '1'), 159);
    schRepl(szPrmpt, chr2Str(158, '2'), 160);
    schRepl(szPrmpt, chr2Str(158, '3'), 161);
    schRepl(szPrmpt, chr2Str(158, '4'), 162);
    schRepl(szPrmpt, chr2Str(158, '5'), 163);

    schRepl(szPrmpt, "a1", 164);
    schRepl(szPrmpt, "a2", 165);
    schRepl(szPrmpt, "a3", 166);
    schRepl(szPrmpt, "a4", 167);
    schRepl(szPrmpt, "a5", 168);

    schRepl(szPrmpt, "e1", 169);
    schRepl(szPrmpt, "e2", 170);
    schRepl(szPrmpt, "e3", 171);
    schRepl(szPrmpt, "e4", 172);
    schRepl(szPrmpt, "e5", 173);

    schRepl(szPrmpt, "i1", 174);
    schRepl(szPrmpt, "i2", 175);
    schRepl(szPrmpt, "i3", 181);
    schRepl(szPrmpt, "i4", 182);
    schRepl(szPrmpt, "i5", 183);

    schRepl(szPrmpt, "o1", 184);
    schRepl(szPrmpt, "o2", 190);
    schRepl(szPrmpt, "o3", 198);
    schRepl(szPrmpt, "o4", 199);
    schRepl(szPrmpt, "o5", 208);

    schRepl(szPrmpt, "u1", 210);
    schRepl(szPrmpt, "u2", 211);
    schRepl(szPrmpt, "u3", 212);
    schRepl(szPrmpt, "u4", 213);
    schRepl(szPrmpt, "u5", 214);

    schRepl(szPrmpt, "y1", 215);
    schRepl(szPrmpt, "y2", 216);
    schRepl(szPrmpt, "y3", 221);
    schRepl(szPrmpt, "y4", 222);
    schRepl(szPrmpt, "y5", 248);

    schRepl(szPrmpt, "d9", 249);
    schRepl(szPrmpt, "D9", 250);
}

uint8_t getASCII(char chr)
{
    return ((int16_t)chr < 128) ? chr : (chr + 128);
}

uint16_t getFileSize(FILE *filePtr)
{
    uint16_t curPos, length;

    curPos = ftell(filePtr);
    fseek(filePtr, 0L, SEEK_END);

    length = ftell(filePtr);
    fseek(filePtr, curPos, SEEK_SET);

    return length;
}

void loadFontVNI(char *fileName, uint8_t height, uint8_t width, uint8_t startChr)
{
    uint16_t size;
    FILE *filePtr;

    filePtr  = fopen(fileName, "rb");
    if (!filePtr)
    {
        closeGraph();
        printf("Files %s does not exist", fileName);
        exit(1);
    }

    size = getFileSize(filePtr);

    fontTable = (uint8_t *)malloc(size);
    if (!fontTable)
    { 
        closeGraph();
        printf("Not enough memory to allocation");
        exit(1);
    }

    fread(fontTable, size, 1, filePtr);
    fclose(filePtr);

    charWidth = width;
    charHeight = height;
    charStart = startChr;
}

void writeCharVNI(uint16_t x, uint16_t y, char ch, uint32_t color)
{
    int16_t i, j;
    uint8_t currLine;
    uint8_t currChar;
    
    if (ch == 32) return;

    currChar = getASCII(ch) - charStart;

    for (i = 0; i < charHeight; i++)
    {
        currLine = fontTable[currChar * charHeight + i];
        for (j = charWidth; j > 0 ; j--)
        {
            if (currLine & 0x01) putPixel(x + j, y + i, color);
            currLine >>= 1;
        }
    }
}

void writeStrVNI(uint16_t x, uint16_t y, char *msg, uint32_t color, uint8_t type)
{
    if (!type) while(*msg)
    {
        writeCharVNI(x, y, *msg++, color++);
        x += charWidth;
    }
    else while(*msg)
    {
        writeCharVNI(x, y, *msg++, color);
        x += charWidth;
    }
}

void writeCharCHR(uint16_t x, uint16_t y, uint8_t code, uint8_t size, uint32_t color)
{
    int16_t i;
    int16_t codeX, codeY, px, py;
    uint8_t numStroke = 0;

    int16_t pos = offsets[code - CHRInfo.charStart] + sizeof(HEADER) + CHRInfo.StartOffset;

    if (!size) size = 1;
    if (size > 8) size = 8;

    fpCHR = fopen(fileCHR, "rb");
    fseek(fpCHR, pos, SEEK_SET);

    do {
        fread(&strokes[numStroke], sizeof(STROKE), 1, fpCHR);
        codeX = (strokes[numStroke].x & 0x80) >> 7;
        codeY = (strokes[numStroke].y & 0x80) >> 7;
        numStroke++;
    } while(codeX || codeY);

    for (i = 0; i < numStroke; i++)
    {
        px = (int16_t)(strokes[i].x & 0x7F);
        if (px > 0x3F) px -= 0x80;

        py = (int16_t)(strokes[i].y & 0x7F);
        if (py > 0x3F) py -= 0x80;

        codeX = (strokes[i].x & 0x80) >> 7;
        codeY = (strokes[i].y & 0x80) >> 7;

        if (codeX && !codeY) moveTo(x + px * size, y + 30 - py * size);
        else if (codeX && codeY) lineTo(x + px * size, y + 30 - py * size , color);
    }
    fclose(fpCHR);
}

void loadFontCHR(char *fileName)
{
    fileCHR = fileName;

    fpCHR = fopen(fileCHR, "rb");
    if (!fpCHR)
    {
        closeGraph();
        printf("File %s does not exist", fileName);
        exit(1);
    }

    fread(&CHRHeader, sizeof(HEADER), 1, fpCHR);

    if (CHRHeader.Sign[0] != 'P' && CHRHeader.Sign[1] != 'K' && CHRHeader.Sign[2] != 8 && CHRHeader.Sign[3] != 8)
    {
        fclose(fpCHR);
        closeGraph();
        printf("Bad format font CHR");
        exit(1);
    }

    fread(&CHRInfo, sizeof(FONTINFO), 1, fpCHR);
    fread(&offsets, sizeof(uint16_t), CHRInfo.numChar, fpCHR);
    fread(&widths, sizeof(uint8_t), CHRInfo.numChar, fpCHR);

    fclose(fpCHR);
}

void writeStrCHR(uint16_t x, uint16_t y, char *str, uint8_t size, uint32_t color, uint8_t type)
{
    if (!size) size = 1;
    if (size > 4) size = 4;

    if (!type) while(*str)
    {
        writeCharCHR(x, y, *str, size, color++);
        x += widths[*str++ - CHRInfo.charStart] * size;
    }
    else while(*str)
    {
        writeCharCHR(x, y, *str, size, color);
        x += widths[*str++ - CHRInfo.charStart] * size;
    }
}
/*---------------------- END OF FILE SVGA.C --------------------------*/

void _main()
{
    initGraph(800, 600, 24);
    drawLine(0, 0, cmaxX, cmaxY, fromRGB(255, 255, 0));
    getch();
    closeGraph();
}
