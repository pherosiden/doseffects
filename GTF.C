/* gtf.c  Generate mode timings using the GTF Timing Standard
 *
 * gcc gtf.c -o gtf -lm -Wall
 *
 * Copyright (c) 2001, Andy Ritger  aritger@nvidia.com
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 
 * o Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * o Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer
 *   in the documentation and/or other materials provided with the
 *   distribution.
 * o Neither the name of NVIDIA nor the names of its contributors
 *   may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT
 * NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
 * THE REGENTS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * 
 *
 * This program is based on the Generalized Timing Formula(GTF TM)
 * Standard Version: 1.0, Revision: 1.0
 *
 * The GTF Document contains the following Copyright information:
 *
 * Copyright (c) 1994, 1995, 1996 - Video Electronics Standards
 * Association. Duplication of this document within VESA member
 * companies for review purposes is permitted. All other rights
 * reserved.
 *
 * While every precaution has been taken in the preparation
 * of this standard, the Video Electronics Standards Association and
 * its contributors assume no responsibility for errors or omissions,
 * and make no warranties, expressed or implied, of functionality
 * of suitability for any purpose. The sample code contained within
 * this standard may be used without restriction.
 *
 * 
 *
 * The GTF EXCEL(TM) SPREADSHEET, a sample (and the definitive)
 * implementation of the GTF Timing Standard, is available at:
 *
 * ftp://ftp.vesa.org/pub/GTF/GTF_V1R1.xls
 *
 *
 *
 * This program takes a desired resolution and vertical refresh rate,
 * and computes mode timings according to the GTF Timing Standard.
 * These mode timings can then be formatted as an XFree86 modeline
 * or a mode description for use by fbset(8).
 *
 *
 *
 * NOTES:
 *
 * The GTF allows for computation of "margins" (the visible border
 * surrounding the addressable video); on most non-overscan type
 * systems, the margin period is zero.  I've implemented the margin
 * computations but not enabled it because 1) I don't really have
 * any experience with this, and 2) neither XFree86 modelines nor
 * fbset fb.modes provide an obvious way for margin timings to be
 * included in their mode descriptions (needs more investigation).
 * 
 * The GTF provides for computation of interlaced mode timings;
 * I've implemented the computations but not enabled them, yet.
 * I should probably enable and test this at some point.
 *
 * 
 *
 * TODO:
 *
 * o Add support for interlaced modes.
 *
 * o Implement the other portions of the GTF: compute mode timings
 *   given either the desired pixel clock or the desired horizontal
 *   frequency.
 *
 * o It would be nice if this were more general purpose to do things
 *   outside the scope of the GTF: like generate double scan mode
 *   timings, for example.
 *   
 * o Printing digits to the right of the decimal point when the
 *   digits are 0 annoys me.
 *
 * o Error checking.
 *
 */


#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#define MARGIN_PERCENT    1.8   /* % of active vertical image                */
#define CELL_GRAN         8.0   /* assumed character cell granularity        */
#define MIN_PORCH         1     /* minimum front porch                       */
#define V_SYNC_RQD        3     /* width of vsync in lines                   */
#define H_SYNC_PERCENT    8.0   /* width of hsync as % of total line         */
#define MIN_VSYNC_PLUS_BP 550.0 /* min time of vsync + back porch (microsec) */
#define M                 600.0 /* blanking formula gradient                 */
#define C                 40.0  /* blanking formula offset                   */
#define K                 128.0 /* blanking formula scaling factor           */
#define J                 20.0  /* blanking formula scaling factor           */

/* C' and M' are part of the Blanking Duty Cycle computation */
#define C_PRIME           (((C - J) * K / 256.0) + J)
#define M_PRIME           (K / 256.0 * M)

/*
 * print_fb_mode() - print a mode description in fbset(8) format;
 * see the fb.modes(8) manpage.  The timing description used in
 * this is rather odd; they use "left and right margin" to refer
 * to the portion of the hblank before and after the sync pulse
 * by conceptually wrapping the portion of the blank after the pulse
 * to infront of the visible region; ie:
 * 
 *
 * Timing description I'm accustomed to:
 *
 *
 *
 *     <--------1--------> <--2--> <--3--> <--4-->
 *                                _________
 *    |-------------------|_______|       |_______
 *
 *                        R       SS      SE     FL
 *       
 * 1: visible image
 * 2: blank before sync (aka front porch)
 * 3: sync pulse
 * 4: blank after sync (aka back porch)
 * R: Resolution
 * SS: Sync Start
 * SE: Sync End
 * FL: Frame Length
 *
 *
 * But the fb.modes format is:
 *
 *
 *    <--4--> <--------1--------> <--2--> <--3--> 
 *                                       _________
 *    _______|-------------------|_______|       |
 *  
 * The fb.modes(8) manpage refers to <4> and <2> as the left and
 * right "margin" (as well as upper and lower margin in the vertical
 * direction) -- note that this has nothing to do with the term
 * "margin" used in the GTF Timing Standard.
 *
 * XXX always prints the 32 bit mode -- should I provide a command
 * line option to specify the bpp?  It's simple enough for a user
 * to edit the mode description after it's generated.
 */

inline int roundf(float x)
{
    if (x > 0) return (x + 0.5);
    return (x - 0.5);
} 

/*
 * vert_refresh() - as defined by the GTF Timing Standard, compute the
 * Stage 1 Parameters using the vertical refresh frequency.  In other
 * words: input a desired resolution and desired refresh rate, and
 * output the GTF mode timings.
 *
 * XXX All the code is in place to compute interlaced modes, but I don't
 * feel like testing it right now.
 *
 * XXX margin computations are implemented but not tested (nor used by
 * XFree86 of fbset mode descriptions, from what I can tell).
 */

// CRTC timing value
#define CRTC_DOUBLE_SCANLINE (1UL << 0)
#define CRTC_INTERLACED      (1UL << 1)
#define CRTC_HSYNC_NEGATIVE  (1UL << 2)
#define CRTC_VSYNC_NEGATIVE  (1UL << 3)

// VESA 3.0 CRTC timings structure
typedef struct
{
    unsigned short HorizontalTotal;
    unsigned short HorizontalSyncStart;
    unsigned short HorizontalSyncEnd;
    unsigned short VerticalTotal;
    unsigned short VerticalSyncStart;
    unsigned short VerticalSyncEnd;
    unsigned char  Flags;
    unsigned long  PixelClock;  // units of Hz
    unsigned short RefreshRate; // units of 0.01 Hz
    unsigned char  Reserved[40];
} VBE_CRTC_INFO_BLOCK;

void calcCrtcTimingGTF(VBE_CRTC_INFO_BLOCK *crtc, int horizPixels, int vertLines, int freq, int interlaced, int margins)
{
    float horizPixelsRnd;
    float vertLinesRnd;
    float vertFieldRateRqd;
    float topMargin;
    float bottonMargin;
    float interlace;
    float horizPeriodEst;
    float vertSyncPlusBP;
    float vertBackPorch;
    float totalVertLine;
    float vertFieldRateEst;
    float horizPeriod;
    float vertFieldRate;
    float vertFrameRate;
    float leftMargin;
    float rightMargin;
    float totalActivePixels;
    float idealDutyCycle;
    float horizBlank;
    float totalPixels;
    float pixelFreq;
    float horizFreq;
    float horizSync;
    float horizFrontPorch;
    float vertOddFrontPorchLines;
    
    // 0. Check double scanline
    if (vertLines < 400) vertLines *= 2;

    // 1. In order to give correct results, the number of horizontal
    // pixels requested is first processed to ensure that it is divisible
    // by the character size, by rounding it to the nearest character
    // cell boundary:
    horizPixelsRnd = roundf(horizPixels / CELL_GRAN) * CELL_GRAN;

    // 2. If interlace is requested, the number of vertical lines assumed
    // by the calculation must be halved, as the computation calculates
    // the number of vertical lines per field. In either case, the
    // number of lines is rounded to the nearest integer.
    vertLinesRnd = interlaced ? roundf(vertLines / 2.0) : roundf(vertLines);

    // 3. Find the frame rate required:
    vertFieldRateRqd = interlaced ? freq * 2.0 : freq;

    // 4. Find number of lines in Top margin:
    topMargin = margins ? roundf(MARGIN_PERCENT / 100.0 * vertLinesRnd) : 0.0;

    // 5. Find number of lines in Bottom margin:
    bottonMargin = margins ? roundf(MARGIN_PERCENT / 100.0 * vertLinesRnd) : 0.0;

    // 6. If interlace is required, then set variable [INTERLACE]=0.5:
    interlace = interlaced ? 0.5 : 0.0;

    // 7. Estimate the Horizontal period
    horizPeriodEst = (((1.0 / vertFieldRateRqd) - (MIN_VSYNC_PLUS_BP / 1000000.0)) / (vertLinesRnd + (2 * topMargin) + MIN_PORCH + interlace) * 1000000.0);

    // 8. Find the number of lines in V sync + back porch:
    vertSyncPlusBP = roundf(MIN_VSYNC_PLUS_BP / horizPeriodEst);

    // 9. Find the number of lines in V back porch alone:
    vertBackPorch = vertSyncPlusBP - V_SYNC_RQD;

    // 10. Find the total number of lines in Vertical field period:
    totalVertLine = vertLinesRnd + topMargin + bottonMargin + vertSyncPlusBP + interlace + MIN_PORCH;

    // 11. Estimate the Vertical field frequency:
    vertFieldRateEst = 1.0 / horizPeriodEst / totalVertLine * 1000000.0;

    // 12. Find the actual horizontal period:
    horizPeriod = horizPeriodEst / (vertFieldRateRqd / vertFieldRateEst);

    // 13. Find the actual Vertical field frequency:
    vertFieldRate = 1.0 / horizPeriod / totalVertLine * 1000000.0;

    // 14. Find the Vertical frame frequency:
    vertFrameRate = interlaced ? vertFieldRate / 2.0 : vertFieldRate;

    // 15. Find number of pixels in left margin:
    leftMargin = margins ? roundf(horizPixelsRnd * MARGIN_PERCENT / 100.0 / CELL_GRAN) * CELL_GRAN : 0.0;

    // 16. Find number of pixels in right margin:
    rightMargin = margins ? roundf(horizPixelsRnd * MARGIN_PERCENT / 100.0 / CELL_GRAN) * CELL_GRAN : 0.0;

    // 17. Find total number of active pixels in image and left and right margins:
    totalActivePixels = horizPixelsRnd + leftMargin + rightMargin;

    // 18. Find the ideal blanking duty cycle from the blanking duty cycle
    idealDutyCycle = C_PRIME - (M_PRIME * horizPeriod / 1000.0);

    // 19. Find the number of pixels in the blanking time to the nearest double character cell:
    horizBlank = roundf(totalActivePixels * idealDutyCycle / (100.0 - idealDutyCycle) / (2.0 * CELL_GRAN)) * (2.0 * CELL_GRAN);

    // 20. Find total number of pixels:
    totalPixels = totalActivePixels + horizBlank;

    // 21. Find pixel clock frequency:
    pixelFreq = totalPixels / horizPeriod;

    // 22. Find horizontal frequency:
    horizFreq = 1000.0 / horizPeriod;

    // 17. Find the number of pixels in the horizontal sync period:
    horizSync = roundf(H_SYNC_PERCENT / 100.0 * totalPixels / CELL_GRAN) * CELL_GRAN;

    // 18. Find the number of pixels in the horizontal front porch period:
    horizFrontPorch = (horizBlank / 2.0) - horizSync;

    // 36. Find the number of lines in the odd front porch period:
    vertOddFrontPorchLines = MIN_PORCH + interlace;

    // finally, pack the results in the mode struct
    crtc->HorizontalSyncStart = horizPixelsRnd + horizFrontPorch;
    crtc->HorizontalSyncEnd = horizPixelsRnd + horizFrontPorch + horizSync;
    crtc->HorizontalTotal = totalPixels;
    crtc->VerticalSyncStart = vertLinesRnd + vertOddFrontPorchLines;
    crtc->VerticalSyncEnd = vertLinesRnd + vertOddFrontPorchLines + V_SYNC_RQD;
    crtc->VerticalTotal = totalVertLine;
    crtc->PixelClock = pixelFreq * 1000.0 * 1000.0;
    crtc->RefreshRate = freq * 100;
    crtc->Flags = CRTC_HSYNC_NEGATIVE | CRTC_VSYNC_NEGATIVE;
    if (interlaced) crtc->Flags |= CRTC_INTERLACED;
    if (vertLines < 400) crtc->Flags |= CRTC_DOUBLE_SCANLINE;

}

int main(int argc, char *argv[])
{
    VBE_CRTC_INFO_BLOCK crtc;
    calcCrtcTimingGTF(&crtc, 800, 600, 85, 0, 0);
    printf("HorizontalTotal:     %u\n", crtc.HorizontalTotal);
    printf("HorizontalSyncStart: %u\n", crtc.HorizontalSyncStart);
    printf("HorizontalSyncEnd:   %u\n", crtc.HorizontalSyncEnd);
    printf("VerticalTotal:       %u\n", crtc.VerticalTotal);
    printf("VerticalSyncStart:   %u\n", crtc.VerticalSyncStart);
    printf("VerticalSyncEnd:     %u\n", crtc.VerticalSyncEnd);
    printf("PixelClock:          %u\n", crtc.PixelClock);
    printf("RefreshRate:         %u\n", crtc.RefreshRate);
    printf("Flags:               0x%02x\n", crtc.Flags);
    return 0;
}
