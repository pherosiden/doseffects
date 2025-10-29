#include <dos.h>
#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define VBE_CODE_SIZE   0x8000      /* 32KB BIOS area copy */
#define VBE_DATA_SIZE   0x2000      /* scratch data area */
#define VBE_STACK_SIZE  0x2000      /* call stack size */

#pragma pack(push, 1)

/* VESA 3.0 PMI block */
typedef struct {
    uint8_t     Signature[4];       /* PM Info signature ('PMID') */
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
} VBE_PM_INFO;

/* VBE3 (48 bits pointer address) far call functions */
typedef struct {
    uint32_t offset;                /* 32 bits offset */
    uint16_t segment;               /* 16 bits selector */
} VBE_FAR_PTR;

/* Driver info (reduced) returned by VBE GetInfo (4F00h) */
typedef struct {
    uint8_t     VBESignature[4];    /* "VESA" */
    uint16_t    VBEVersion;         /* VBE version */
    uint32_t    OemStringPtr;       /* pointer to OEM string (seg:off) */
    uint32_t    Capabilities;       /* capabilities of graphics controller */
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

/* Global state filled by init vbe3 driver */
uint32_t g_lfb_size = 0;
uint32_t *g_lfb_ptr = NULL;

uint32_t g_bytes_per_scanline = 0;

uint8_t *g_bios_code_ptr = NULL;
uint8_t *g_bios_data_ptr = NULL;
uint8_t *g_bios_stack_ptr = NULL;

uint16_t g_vbe_info_sel = 0, g_a0000_sel = 0, g_b0000_sel = 0, g_b8000_sel = 0;
uint16_t g_bios_code_sel = 0, g_bios_data_sel = 0, g_bios_stack_sel = 0;

/*------------------------------------------------------------------*/
/*-------------------- DPMI wrappers functions -------------------- */
/*------------------------------------------------------------------*/

/* allocate dos selector */
uint16_t dpmi_alloc_selector() {
    __asm {
        xor     eax, eax
        mov     ecx, 1
        int     31h
        jnc     quit
        xor     ax, ax
    quit:
    }
}

/* free dos selector */
int32_t dpmi_free_selector(uint16_t sel) {
    __asm {
        mov     bx, sel
        mov     eax, 0001h
        int     31h
    }
}

/* set selector rights (access) - not all hosts implement this */
int32_t dpmi_set_selector_rights(uint16_t sel, uint16_t access) {
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

int32_t dpmi_set_selector_base(uint16_t sel, uint32_t base) {
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

int32_t dpmi_set_selector_limit(uint16_t sel, uint32_t limit) {
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
uint32_t dpmi_map_physical_address(uint32_t phys_base, uint32_t size) {
    __asm {
        mov     ebx, phys_base
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

void dpmi_unmap_physical_address(uint32_t* linear_base) {
    __asm {
        mov     edi, linear_base
        mov     bx, [edi + 2]
        mov     cx, [edi]
        mov     dword ptr [edi], 0
        mov     eax, 0801h
        int     31h
    }
}

// convert real pointer to linear pointer
uint32_t map_real_pointer(uint32_t rm_seg_ofs)
{
    __asm {
        mov     eax, rm_seg_ofs
        mov     edx, eax
        and     eax, 0FFFF0000h
        and     edx, 0000FFFFh
        shr     eax, 12
        add     eax, edx
    }
}

/* compute 8-bit PMI BIOS bios checksum */
int32_t vbe_bios_checksum(uint8_t *bios, uint32_t len) {
    uint32_t i, sum = 0;
    for (i = 0; i < len; i++) sum += bios[i];
    return (sum & 0xFF) == 0;
}

/* find PMID block in BIOS copy buffer */
VBE_PM_INFO *vbe_find_pm_block(uint8_t *bios, uint32_t size) {
    uint32_t i = 0;
    uint32_t limit = size - sizeof(VBE_PM_INFO);

    for (i = 0; i < limit; i++) {
        if (!memcmp(&bios[i], "PMID", 4)) {
            fprintf(stderr, "Found PMID block at offset 0x%lX\n", &bios[i]);
            if (vbe_bios_checksum(&bios[i], sizeof(VBE_PM_INFO))) return (VBE_PM_INFO*)&bios[i];
        }
    }

    return NULL;
}

/* create selector and set base/limit/access rights */
uint16_t create_selector_linear(void *linear_base, uint32_t len) {
    uint16_t sel = dpmi_alloc_selector();
    if (!sel) return 0;
    
    if (!dpmi_set_selector_base(sel, (uint32_t)linear_base)) {
        dpmi_free_selector(sel);
        return 0;
    }

    if (!dpmi_set_selector_limit(sel, len - 1)) {
        dpmi_free_selector(sel);
        return 0;
    }

    if (!dpmi_set_selector_rights(sel, 0x8092)) {
        dpmi_free_selector(sel);
        return 0;
    }

    return sel;
}

/* create selector for physical address */
uint16_t create_selector_physical(uint32_t phys_base, uint32_t len) {
    uint16_t sel = dpmi_alloc_selector();
    if (!sel) return 0;
    
    if (!dpmi_set_selector_base(sel, phys_base)) {
        dpmi_free_selector(sel);
        return 0;
    }

    if (!dpmi_set_selector_limit(sel, len - 1)) {
        dpmi_free_selector(sel);
        return 0;
    }

    if (!dpmi_set_selector_rights(sel, 0x8092)) {
        dpmi_free_selector(sel);
        return 0;
    }

    return sel;
}

/* build 48 bits farptr (offset32 + sel16) in memory and call it */
void vbe_call_farptr(uint32_t offs, uint16_t sel, uint16_t stack_sel) {
    VBE_FAR_PTR farptr;
    farptr.offset = offs;
    farptr.segment = sel;

    __asm {
        pusha
        mov     ax, stack_sel
        mov     ss, ax
        xor     sp, sp
        lea     esi, farptr
        call    fword ptr [esi]
        popa
    }
}

/* call VBE entry functions AX=0x4F0xx */
uint32_t vbe_call_entry(uint32_t offset, uint16_t sel, uint16_t inax, uint16_t inbx, uint16_t ines) {
    uint32_t val = 0;
    VBE_FAR_PTR farptr;
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
    if (g_bios_code_ptr) free(g_bios_code_ptr);
    if (g_bios_data_ptr) free(g_bios_data_ptr);
    if (g_bios_stack_ptr) free(g_bios_stack_ptr);
    if (g_a0000_sel) dpmi_free_selector(g_a0000_sel);
    if (g_b0000_sel) dpmi_free_selector(g_b0000_sel);
    if (g_b8000_sel) dpmi_free_selector(g_b8000_sel);
    if (g_vbe_info_sel) dpmi_free_selector(g_vbe_info_sel);
    if (g_bios_code_sel) dpmi_free_selector(g_bios_code_sel);
    if (g_bios_data_sel) dpmi_free_selector(g_bios_data_sel);
    if (g_bios_stack_sel) dpmi_free_selector(g_bios_stack_sel);
    if (g_lfb_ptr) dpmi_unmap_physical_address((uint32_t*)&g_lfb_ptr);

    g_lfb_size = 0;
    g_lfb_ptr = NULL;
}

/* initialize VBE 3.0 via PMI: 
 * 1. copy BIOS
 * 2. find PMID
 * 3. allocate selectors
 * 4. call PMInitialize + EntryPoint
 */
int32_t vbe_init_driver(VBE_DRIVER_INFO *drv_info_ptr, VBE_PM_INFO *pm_info_ptr) {
    VBE_PM_INFO *pm_ptr = NULL;

    /* 1. copy BIOS data from C0000..C7FFF (physical) to real-mode. */
    g_bios_code_ptr = (uint8_t*)malloc(VBE_CODE_SIZE);
    if (!g_bios_code_ptr) return 0;
    memcpy(g_bios_code_ptr, (uint8_t*)0xC0000000UL, VBE_CODE_SIZE);
    
    /* 2. find PMID block */
    pm_ptr = vbe_find_pm_block(g_bios_code_ptr, VBE_CODE_SIZE);
    if (!pm_ptr) {
        free(g_bios_code_ptr);
        fprintf(stderr, "PMID block not found! Card not support VESA 3.0\n");
        return 0;
    }

    /* 3. create selectors (code, data, stack, A0000/B0000/B8000) */
    g_bios_data_ptr = (uint8_t*)calloc(1, VBE_DATA_SIZE);
    if (!g_bios_data_ptr) {
        free(g_bios_code_ptr);
        return 0;
    }

    g_bios_data_sel = create_selector_linear(g_bios_data_ptr, VBE_DATA_SIZE);
    if (!g_bios_data_sel) goto error;

    g_bios_code_sel = create_selector_linear(g_bios_code_ptr, VBE_CODE_SIZE);
    if (!g_bios_code_sel) goto error;

    g_a0000_sel = create_selector_physical(0x000A0000UL, 0x10000);
    if (!g_a0000_sel) goto error;

    g_b0000_sel = create_selector_physical(0x000B0000UL, 0x10000);
    if (!g_b0000_sel) goto error;

    g_b8000_sel = create_selector_physical(0x000B8000UL, 0x8000);
    if (!g_b8000_sel) goto error;

    g_bios_stack_ptr = (uint8_t*)malloc(VBE_STACK_SIZE);
    if (!g_bios_stack_ptr) goto error;

    g_bios_stack_sel = create_selector_linear(g_bios_stack_ptr, VBE_STACK_SIZE);
    if (!g_bios_stack_sel) goto error;

    /* 4. patch PMI fields inside bios copy */
    pm_ptr->CodeSegSel = g_bios_code_sel;
    pm_ptr->BIOSDataSel = g_bios_data_sel;
    pm_ptr->A0000Sel = g_a0000_sel;
    pm_ptr->B0000Sel = g_b0000_sel;
    pm_ptr->B8000Sel = g_b8000_sel;
    pm_ptr->InProtectMode = 1;
    
    /* 5. call PMInitialize (if present) */
    if (pm_ptr->PMInitialize) {
        fprintf(stderr, "Calling PMInitialize at offset 0x%04X using selector 0x%04X\n", pm_ptr->PMInitialize, g_bios_code_sel);
        vbe_call_farptr(pm_ptr->PMInitialize, g_bios_code_sel, g_bios_stack_sel);
    }
    else {
        /* some BIOS do not have PMInitialize - try to call EntryPoint directly */
        fprintf(stderr, "WARNING: PMInitialize not present, calling EntryPoint directly\n");
    }

    /* 6. allocate selector for VBE driver info buffer and call EntryPoint AX=4F00 to get driver info */
    g_vbe_info_sel = create_selector_linear(drv_info_ptr, sizeof(VBE_DRIVER_INFO));
    if (!g_vbe_info_sel) goto error;

    fprintf(stderr, "Calling VBE3 EntryPoint (4F00) via PM entry...\n");
    if (!vbe_call_entry(pm_ptr->EntryPoint, pm_ptr->CodeSegSel, 0x4F00, 0, g_vbe_info_sel)) {
        fprintf(stderr, "VBE3 call entry point function failed!\n");
        goto error;
    }
    else {
        if (memcmp(drv_info_ptr->VBESignature, "VESA", 4)) {
            fprintf(stderr, "VBE3 call entry point did not return valid driver info signature\n");
            goto error;
        }

        /* copy PMI block info */
        memcpy(pm_info_ptr, pm_ptr, sizeof(VBE_PM_INFO));
        fprintf(stderr, "Initialize VBE3 success (signature: %c%c%c%c, version=%04X)\n", drv_info_ptr->VBESignature[0], drv_info_ptr->VBESignature[1], drv_info_ptr->VBESignature[2], drv_info_ptr->VBESignature[3], drv_info_ptr->VBESignature[4], drv_info_ptr->VBEVersion);
        return 1;
    }

error:
    cleanup();
    return 0;
}

/* set VBE3 mode with given resolution and bpp */
int32_t vbe_set_mode(int32_t xres, int32_t yres, int32_t bpp) {
    VBE_PM_INFO pm_info;
    VBE_MODE_INFO mode_info;
    VBE_DRIVER_INFO drv_info;

    uint16_t mode = 0;
    uint16_t mode_sel = 0;
    uint16_t *mode_list = NULL;

    /* initialize VBE3 driver */
    if (!vbe_init_driver(&drv_info, &pm_info)) {
        cleanup();
        fprintf(stderr, "initialize VBE3 failed.\n");
        return 0;
    }

    /* alloc selector for mode info */
    mode_sel = create_selector_linear(&mode_info, sizeof(mode_info));
    if (!mode_sel) {
        fprintf(stderr, "alloc create_selector_linear failed\n");
        return 0;
    }
    
    /* convert real pointer (seg:ofs) to linear pointer */
    mode_list = (uint16_t*)map_real_pointer(drv_info.VideoModePtr);
    if (!mode_list) {
        dpmi_free_selector(mode_sel);
        fprintf(stderr, "map_real_pointer failed for mode list\n");
        return 0;
    }
    
    /* find request mode with resolution and bits plan */
    while (*mode_list != 0xFFFF) {
        /* call vbe 0x4F01 function to get mode info */
        mode = *mode_list;
        if (vbe_call_entry(pm_info.EntryPoint, pm_info.CodeSegSel, 0x4F01, mode, mode_sel)) {
            if (mode_info.XResolution == xres && mode_info.YResolution == yres && mode_info.BitsPerPixel == bpp) break;
        }

        /* go to next mode */
        mode_list++;
    }

    dpmi_free_selector(mode_sel);

    /* request mode not found*/
    if (mode == 0xFFFF) {
        fprintf(stderr, "Requested mode %dx%d@%d not found!\n", xres, yres, bpp);
        return 0;
    }

    /* request LFB by setting bit 14 of mode */
    mode |= 0x4000;
    if (!vbe_call_entry(pm_info.EntryPoint, pm_info.CodeSegSel, 0x4F02, mode, 0)) {
        fprintf(stderr, "VBE3 call entry 0x4F02 failed!\n");
        return 0;
    }

    /* map physical lfb */
    g_bytes_per_scanline = mode_info.BytesPerScanline;
    g_lfb_size = mode_info.BytesPerScanline * mode_info.YResolution;
    g_lfb_ptr = (uint32_t*)dpmi_map_physical_address(mode_info.PhysBasePtr, g_lfb_size);
    if (!g_lfb_ptr) {
        fprintf(stderr, "dpmi_map_physical_address failed!\n");
        return 0;
    }

    return 1;
}

/* simple draw pixel for 32bpp */
void draw_pixel(int32_t x, int32_t y, uint32_t color) {
    uint32_t ofs = y * g_bytes_per_scanline + x;
    *(g_lfb_ptr + ofs) = color;
}

int main() {
    int32_t i = 0;

    /* try to set vbe3 mode */
    if (!vbe_set_mode(800, 600, 32)) {
        cleanup();
        fprintf(stderr, "VBE3 set mode 800x600x32 failed!\n");
        return 0;
    }

    /* draw diagonal red if 32bpp */
    for (i = 0; i < 600; i++) draw_pixel(i, i, 0x00FF0000U);
    getch();

    cleanup();
    return 1;
}
