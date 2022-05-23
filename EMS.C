/*--------------------------------------*/
/*          EMS core Functions          */
/* Author : Nguyen Ngoc Van             */
/* Create : 25/04/1998                  */
/* Email  : siden@codedemo.net          */
/* Website: http://www.codedemo.net     */
/*--------------------------------------*/

const char *EMSErrorMsg[] = {
    "EMS driver error (EMM trouble)",
    "EMS hardware error",
    "EMM is busy",
    "Illegal EMM handle",
    "Called EMS function does not exist",
    "No more free EMS handles available",
    "Error while saving or restoring mapping",
    "More pages requested than are actually available",
    "More pages requested than are free",
    "No pages requested",
    "Logical page does not belong to handle",
    "Illegal physical page number",
    "Mapping memory range is full",
    "Map save has already been done",
    "Mapping must be saved before it can be restored",
    "Parameter of subfunction in AL does not exist"
};

uint16_t EMSVers = 0;
uint8_t EMSErrorCode = 0;
char EMSCode[] = "EMSXXXX0";

uint8_t EMSDetect()
{
    __asm {
        mov     ax, 0x3567
        int     0x21
        lea     si, EMSCode
        mov     di, 10
        mov     cx, 4
        repnz   cmpsw
        db      0x0F, 0x94, 0xC0
        shr     ax, 8
    }
}

void EMSGetStatus()
{
    __asm {
        mov     ah, 0x40
        int     0x67
        mov     EMSErrorCode, ah
    }
}

uint16_t EMSGetFrameSeg()
{
    __asm {
        mov     ah, 0x41
        int     0x67
        mov     EMSErrorCode, ah
        mov     ax, bx
    }
}

uint16_t EMSGetFreePagesNum()
{
    __asm {
        mov     ah, 0x42
        int     0x67
        mov     EMSErrorCode, ah
        mov     ax, bx
    }
}

uint16_t EMSGetAllPagesNum()
{
    __asm {
        mov     ah, 0x42
        int     0x67
        mov     EMSErrorCode, ah
        mov     ax, dx
    }
}

uint16_t EMSNew(uint16_t pages)
{
    __asm {
        mov     ah, 0x43
        mov     bx, pages
        int     0x67
        mov     EMSErrorCode, ah
        mov     ax, dx
    }
}

uint8_t EMSMap(uint16_t h, uint16_t page, uint8_t ram)
{
    __asm {
        mov     ah, 0x44
        mov     al, ram
        mov     bx, page
        mov     dx, h
        int     0x67
        shr     ax, 8
    }
}

void EMSFree(uint16_t h)
{
    __asm {
        mov     ah, 0x45
        mov     dx, h
        int     0x67
        mov     EMSErrorCode, ah
    }
}

void EMSGetVersion()
{
    __asm {
        mov     ah, 0x46
        int     0x67
        mov     EMSErrorCode, ah
        mov     ah, al
        shr     al, 4
        and     ah, 0x0F
        mov     EMSVers, ax
    }
}

uint16_t EMSGetHandlesNum()
{
    __asm {
        mov     ah, 0x4B
        int     0x67
        mov     EMSErrorCode, ah
        mov     ax, bx
    }
}

uint16_t EMSGetPagesNum(uint16_t h)
{
    __asm {
        mov     ah, 0x4C
        mov     dx, h
        int     0x67
        mov     EMSErrorCode, ah
        mov     ax, bx
    }
}

uint16_t EMSGetHandleStruc(uint16_t fseg, uint16_t fofs)
{
    __asm {
        mov     ah, 0x4D
        mov     es, fseg
        mov     di, fofs
        int     0x67
        mov     EMSErrorCode, ah
        mov     ax, bx
    }
}
