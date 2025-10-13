#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <string.h>
#include <dos.h>
#include <stdint.h>

#define VBE_CODE_SIZE   0x8000      /* 32KB BIOS area copy */
#define VBE_DATA_SIZE   0x2000      /* scratch data area */
#define VBE_STACK_SIZE  0x2000      /* call stack size */

#pragma pack(push, 1)

/* VESA 3.0 PMI block layout */
typedef struct {
    uint8_t     Signature[4];       /* PM Info Block signature ('PMID') */
    uint16_t    EntryPoint;         /* offset of PM entry points within BIOS */
    uint16_t    PMInitialize;       /* offset of PM initialization entry points */
    uint16_t    BIOSDataSel;        /* selector to BIOS data area emulation block */
    uint16_t    A0000Sel;           /* selector to 0xa0000 */
    uint16_t    B0000Sel;           /* selector to 0xb0000 */
    uint16_t    B8000Sel;           /* selector to 0xb8000 */
    uint16_t    CodeSegSel;         /* selector to access code segment as data */
    uint8_t     InProtectMode;      /* true if in protected mode */
    uint8_t     Checksum;           /* sum of all bytes in this struct must match 0 */
    uint8_t     Reserved[6];        /* reserved - must be zero */
} VBE_PM_INFO_BLOCK;

/* VESA 3.0 far call wrapper structure (48 bits address) */
typedef struct {
    uint32_t offset;                /* 32 bits offset */
    uint16_t segment;               /* 16 bits selector */
} VBE_FAR_CALL;

/* Driver info (reduced) returned by VBE GetInfo (4F00h) */
typedef struct {
    uint8_t     VBESignature[4];    /* "VESA" */
    uint16_t    VBEVersion;
    uint32_t    OemStringPtr;
    uint32_t    VideoModePtr;       /* pointer to mode list (seg:off) */
    uint16_t    TotalMemory;        /* 64K blocks */

    /* VBE 2.0 extensions */
    uint16_t    OemSoftwareRev;     /* VBE implementation software revision */
    uint32_t    OemVendorNamePtr;   /* vendor name */
    uint32_t    OemProductNamePtr;  /* product name */
    uint32_t    OemProductRevPtr;   /* product revision */
    uint8_t     Reserved[222];      /* VBE implementation scratch area */
    uint8_t     OemData[256];       /* data area for OEM strings */
} VBE_DRIVER_INFO;

/* Mode info (reduced for our needs) */
typedef struct {
    /* for all VBE revisions */
    uint16_t    ModeAttributes;
    uint8_t     WinAAttributes;
    uint8_t     WinBAttributes;
    uint16_t    WinGranularity;
    uint16_t    WinSize;
    uint16_t    WinASegment;
    uint16_t    WinBSegment;
    uint32_t    WinFuncPtr;
    uint16_t    BytesPerScanline;

    /* VBE 1.2+ */
    uint16_t    XResolution;
    uint16_t    YResolution;
    uint8_t     XCharSize;
    uint8_t     YCharSize;
    uint8_t     NumberOfPlanes;
    uint8_t     BitsPerPixel;
    uint8_t     NumberOfBanks;
    uint8_t     MemoryModel;
    uint8_t     BankSize;
    uint8_t     NumberOfImagePages;
    uint8_t     Reserved1;

    /* VBE 1.2+ Direct color fields (required for direct/6 and YUV/7 memory models) */
    uint8_t     RedMaskSize, RedFieldPos;
    uint8_t     GreenMaskSize, GreenFieldPos;
    uint8_t     BlueMaskSize, BlueFieldPos;
    uint8_t     RsvdMaskSize, RsvdFieldPos;
    uint8_t     DirectColorModeInfo;

    /* VBE 2.0+ */
    uint32_t    PhysBasePtr;        /* physical address for access linear frame buffer */
    uint32_t    OffScreenMemOffset;
    uint16_t    OffScreenMemSize;

    /* VBE 3.0+ */
    uint16_t    LinBytesPerScanline;
    uint8_t     BnkNumberOfImagePages;
    uint8_t     LinNumberOfImagePages;
    uint8_t     LinRedMaskSize;
    uint8_t     LinRedFieldPosition;
    uint8_t     LinGreenMaskSize;
    uint8_t     LinGreenFieldPosition;
    uint8_t     LinBlueMaskSize;
    uint8_t     LinBlueFieldPosition;
    uint8_t     LinRsvdMaskSize;
    uint8_t     LinRsvdFieldPosition;
    uint32_t    MaxPixelClock;
    uint8_t     Reserved2[189];
} VBE_MODE_INFO;
#pragma pack(pop)

/* Global state filled by initVBE3 */
VBE_DRIVER_INFO g_drvInfo;
VBE_PM_INFO_BLOCK *g_pmInfo = NULL;

uint32_t g_lfbSize = 0;
uint32_t g_bytesPerScanline = 0;
uint32_t *g_lfbLinear = NULL;

uint8_t *g_biosCode = NULL;
uint8_t *g_biosData = NULL;
uint8_t *g_biosStack = NULL;

uint16_t g_vbeInfoSel = 0, g_a0000Sel = 0, g_b0000Sel = 0, g_b8000Sel = 0;
uint16_t g_biosCodeSel = 0, g_biosDataSel = 0, g_biosStackSel = 0;

/* -------------------- DPMI wrappers (basic) -------------------- */
/* These use int 0x31 DPMI services. Adjust if your host differs. */

uint16_t allocSelector() {
    __asm {
        xor     eax, eax
        mov     ecx, 1
        int     31h
        jnc     quit
        xor     ax, ax
    quit:
    }
}

int32_t freeSelector(uint16_t sel) {
    __asm {
        mov     bx, sel
        mov     eax, 0001h
        int     31h
    }
}

/* set selector rights (access) - not all hosts implement this; routine left for completeness */
int32_t setSelectorRights(uint16_t sel, uint16_t access) {
    __asm {
        mov     bx, sel
        mov     cx, access
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

int32_t setSelectorBase(uint16_t sel, uint32_t base) {
    __asm {
        mov     bx, sel
        mov     ecx, base
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

int32_t setSelectorLimit(uint16_t sel, uint32_t limit) {
    __asm {
        mov     bx, sel
        mov     ecx, limit
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

/* Map physical memory to linear address using DPMI Map Physical Address (0x0800). */
uint32_t mapPhysicalAddress(uint32_t physBase, uint32_t size) {
    __asm {
        mov     ebx, physBase
        mov     esi, size
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

void unmapPhysicalAddress(uint32_t* linearAddr) {
    __asm {
        mov     edi, linearAddr
        mov     bx, [edi + 2]
        mov     cx, [edi]
        mov     dword ptr [edi], 0
        mov     eax, 0801h
        int     31h
    }
}

/* compute 8-bit PMI BIOS checksum */
int32_t PMChecksum(uint8_t *bios, uint32_t len) {
    uint32_t i, sum = 0;
    for (i = 0; i < len; i++) sum += bios[i];
    return (sum & 0xFF) == 0;
}

/* find PMID block in BIOS copy buffer */
VBE_PM_INFO_BLOCK *PMFindBlock(uint8_t *bios, uint32_t size) {
    uint32_t i = 0;
    uint32_t limit = size - sizeof(VBE_PM_INFO_BLOCK);

    for (i = 0; i < limit; i++) {
        if (bios[i] == 'P' && bios[i + 1] == 'M' && bios[i + 2] == 'I' && bios[i + 3] == 'D') {
            if (PMChecksum(&bios[i], sizeof(VBE_PM_INFO_BLOCK))) return (VBE_PM_INFO_BLOCK*)&bios[i];
        }
    }

    return NULL;
}

/* create selector and set base/limit/access rights */
uint16_t createSelectorLinear(void *linearBase, uint32_t len) {
    uint16_t sel = allocSelector();
    if (!sel) return 0;
    
    if (!setSelectorBase(sel, (uint32_t)linearBase)) {
        freeSelector(sel);
        return 0;
    }

    if (!setSelectorLimit(sel, len - 1)) {
        freeSelector(sel);
        return 0;
    }

    if (!setSelectorRights(sel, 0x8092)) {
        freeSelector(sel);
        return 0;
    }

    return sel;
}

/* create selector for physical address */
uint16_t createSelectorPhysical(uint32_t physBase, uint32_t len) {
    uint16_t sel = allocSelector();
    if (!sel) return 0;
    
    if (!setSelectorBase(sel, physBase)) {
        freeSelector(sel);
        return 0;
    }

    if (!setSelectorLimit(sel, len - 1)) {
        freeSelector(sel);
        return 0;
    }

    if (!setSelectorRights(sel, 0x8092)) {
        freeSelector(sel);
        return 0;
    }

    return sel;
}

/* build 48 bits farptr (offset32 + sel16) in memory and call it */
void callFarPtr48(uint32_t offset, uint16_t sel, uint16_t stackSel) {
    VBE_FAR_CALL farptr;
    farptr.offset = offset;
    farptr.segment = sel;

    __asm {
        pusha
        mov     ax, stackSel
        mov     ss, ax
        xor     sp, sp
        lea     esi, farptr
        call    fword ptr [esi]
        popa
    }
}

/* call VBE entry functions AX=0x4F0xx */
uint32_t callEntry(uint32_t offset, uint16_t sel, uint16_t inax, uint16_t inbx, uint16_t ines) {
    uint32_t val = 0;
    VBE_FAR_CALL farptr;
    farptr.offset = offset;
    farptr.segment = sel;

    __asm {
        pusha
        mov     ax, inax
        mov     bx, inbx
        mov     es, ines
        xor     di, di
        lea     esi, farptr
        call    fword ptr [esi]
        mov     [val], eax
        popa
    }

    return (val & 0xFFFF) != 0x004F ? 0 : 1;
}

void cleanup() {
    if (g_biosCode) free(g_biosCode);
    if (g_biosData) free(g_biosData);
    if (g_biosStack) free(g_biosStack);
    if (g_a0000Sel) freeSelector(g_a0000Sel);
    if (g_b0000Sel) freeSelector(g_b0000Sel);
    if (g_b8000Sel) freeSelector(g_b8000Sel);
    if (g_vbeInfoSel) freeSelector(g_vbeInfoSel);
    if (g_biosCodeSel) freeSelector(g_biosCodeSel);
    if (g_biosDataSel) freeSelector(g_biosDataSel);
    if (g_biosStackSel) freeSelector(g_biosStackSel);
    if (g_lfbLinear) unmapPhysicalAddress((uint32_t*)&g_lfbLinear);

    g_pmInfo = NULL;
    g_lfbSize = 0;
    g_lfbLinear = NULL;
}

/* initialize VBE 3.0 via PMI: 
 * 1. copy BIOS
 * 2. find PMID
 * 3. allocate selectors
 * 4. call PMInitialize + EntryPoint
 */
int32_t initVBE3() {
    /* 1. copy BIOS data from C0000..C7FFF (physical) to real-mode. */
    g_biosCode = (uint8_t*)malloc(VBE_CODE_SIZE);
    if (!g_biosCode) return 0;
    _fmemcpy(g_biosCode, (uint8_t far*)MK_FP(0xC000, 0x0000), VBE_CODE_SIZE);

    /* 2. find PMID block */
    g_pmInfo = PMFindBlock(g_biosCode, VBE_CODE_SIZE);
    if (!g_pmInfo) {
        free(g_biosCode);
        fprintf(stderr, "VESA PMID not found!\n");
        return 0;
    }
   
    /* 3. create selectors:
        - code selector mapping bios_copy
        - data selector for BIOSData (0x600 bytes)
        - selectors for A0000/B0000/B8000 physical areas
        - stack selector
    */

    /* allocate bios data area in linear memory and selector for it */
    g_biosData = (uint8_t*)calloc(1, VBE_DATA_SIZE);
    if (!g_biosData) {
        free(g_biosCode);
        fprintf(stderr, "calloc VBE_DATA_SIZE error!\n");
        return 0;
    }

    /* map BIOS code and data to selector */
    g_biosDataSel = createSelectorLinear(g_biosData, VBE_DATA_SIZE);
    if (!g_biosDataSel) goto cleanup;

    g_biosCodeSel = createSelectorLinear(g_biosCode, VBE_CODE_SIZE);
    if (!g_biosCodeSel) goto cleanup;

    /* map video memory segments A0000/B0000/B8000 to selectors */
    g_a0000Sel = createSelectorPhysical(0x000A0000UL, 0x10000);
    if (!g_a0000Sel) goto cleanup;

    g_b0000Sel = createSelectorPhysical(0x000B0000UL, 0x10000);
    if (!g_b0000Sel) goto cleanup;

    g_b8000Sel = createSelectorPhysical(0x000B8000UL, 0x8000);
    if (!g_b8000Sel) goto cleanup;

    g_biosStack = (uint8_t*)malloc(VBE_STACK_SIZE);
    if (!g_biosStack) goto cleanup;

    g_biosStackSel = createSelectorLinear(g_biosStack, VBE_STACK_SIZE);
    if (!g_biosStackSel) goto cleanup;

    /* 4. patch PMI fields inside bios_copy:
        - set CodeSegSel = sel_code
        - set BIOSDataSel = sel_data
        - set A0000Sel/B0000Sel/B8000Sel
        - set InProtectMode = 1
    */

    g_pmInfo->CodeSegSel = g_biosCodeSel;
    g_pmInfo->BIOSDataSel = g_biosDataSel;
    g_pmInfo->A0000Sel = g_a0000Sel;
    g_pmInfo->B0000Sel = g_b0000Sel;
    g_pmInfo->B8000Sel = g_b8000Sel;
    g_pmInfo->InProtectMode = 1;
    
    /* 5. call PMInitialize (if present) */
    if (g_pmInfo->PMInitialize) {
        fprintf(stderr, "Calling PMInitialize at offset 0x%04X using selector 0x%04X\n", g_pmInfo->PMInitialize, g_biosCodeSel);
        callFarPtr48(g_pmInfo->PMInitialize, g_biosCodeSel, g_biosStackSel);
    }
    else {
        /* some BIOSes do not have PMInitialize - try to call EntryPoint directly */
        fprintf(stderr, "WARNING: PMInitialize not present, calling EntryPoint directly\n");
    }

    /* 6. allocate selector for VBE driver info buffer and call EntryPoint AX=4F00 to get driver info */
    g_vbeInfoSel = createSelectorLinear(&g_drvInfo, sizeof(VBE_DRIVER_INFO));
    if (!g_vbeInfoSel) goto cleanup;

    fprintf(stderr, "Calling VBE3 EntryPoint (4F00) via PM entry...\n");
    if (!callEntry(g_pmInfo->EntryPoint, g_pmInfo->CodeSegSel, 0x4F00, 0, g_vbeInfoSel)) {
        fprintf(stderr, "VBE3 EntryPoint call failed!\n");
        goto cleanup;
    }
    else {
        if (memcmp(g_drvInfo.VBESignature, "VESA", 4) != 0) {
            fprintf(stderr, "VBE3 EntryPoint did not return valid driver info signature\n");
            goto cleanup;
        }
        fprintf(stderr, "Initialize VBE3 success (signature: %.4s version=%04X)\n", g_drvInfo.VBESignature, g_drvInfo.VBEVersion);
        return 1;
    }

cleanup:
    cleanup();
    return 0;
}

int32_t setModeInfo(int32_t xres, int32_t yres, int32_t bpp) {
    VBE_MODE_INFO minfo;
    uint16_t mode = 0;
    uint16_t minfSel = 0;
    uint16_t *modeList = NULL;

    if (!g_pmInfo) return 0;

    /* alloc selector for mode info */
    minfSel = createSelectorLinear(&minfo, sizeof(minfo));
    if (!minfSel) {
        fprintf(stderr, "alloc createSelectorLinear failed\n");
        return 0;
    }
    
    /* find request mode with resolution and bits plan */
    modeList = (uint16_t*)MK_FP(g_drvInfo.VideoModePtr >> 16, g_drvInfo.VideoModePtr & 0xFFFF);
    while (*modeList != 0xFFFF) {
        /* call vbe 0x4F01 function to get mode info */
        mode = *modeList;
        if (callEntry(g_pmInfo->EntryPoint, g_pmInfo->CodeSegSel, 0x4F01, mode, minfSel)) {
            if (minfo.XResolution == xres && minfo.YResolution == yres && minfo.BitsPerPixel == bpp) break;
        }

        /* go to next mode */
        modeList++;
    }

    freeSelector(minfSel);

    /* not found coresponse mode*/
    if (mode == 0xFFFF) {
        fprintf(stderr, "Requested mode %dx%d@%d not found\n", xres, yres, bpp);
        return 0;
    }

    /* request LFB by setting bit 14 of BX */
    mode |= 0x4000;
    if (!callEntry(g_pmInfo->EntryPoint, g_pmInfo->CodeSegSel, 0x4F02, mode, 0)) {
        fprintf(stderr, "Set VBE3 mode failed\n");
        return 0;
    }

    /* map physical lfb */
    g_bytesPerScanline = minfo.BytesPerScanline;
    g_lfbSize = minfo.BytesPerScanline * minfo.YResolution;
    g_lfbLinear = (uint32_t*)mapPhysicalAddress(minfo.PhysBasePtr, g_lfbSize);
    if (!g_lfbLinear) {
        fprintf(stderr, "mapPhysicalAddress failed\n");
        return 0;
    }

    return 1;
}

/* simple draw pixel for 32bpp (assumes linear mapped and 32bpp) */
void drawPixel(int x, int y, uint32_t color) {
    uint32_t *base = g_lfbLinear;
    uint32_t ofs = y * g_bytesPerScanline + x;
    *((uint32_t*)(base + ofs)) = color;
}

int32_t main(void) {
    int32_t i = 0;

    /* initialize VBE3 */
    if (!initVBE3()) {
        cleanup();
        fprintf(stderr, "initVBE3 failed.\n");
        return 0;
    }

    /* find a mode and set it (mode hard-coded or take from driver info) */
    /* example: 1024x768 (depends on card) */
    if (!setModeInfo(1024, 768, 32)) {
        cleanup();
        fprintf(stderr, "VBE3 set mode failed\n");
        return 1;
    }

    /* draw diagonal red if 32bpp */
    for (i = 0; i < 768; i++) drawPixel(i, i, 0x00FF0000U);
    getch();

    cleanup();
    return 0;
}
