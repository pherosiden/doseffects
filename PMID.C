/*
 *  Simple test program --  test for VESA Bios Extension (VBE) Protected Mode
 *                          Interface structure existance in the graphics
 *                          BIOS
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#define TRUE   1
#define FALSE  0

#pragma pack(push, 1)
struct VBE_PMInfoBlock {
    uint8_t   Signature[4];          /*  PM Info Block Signature */
    uint16_t  EntryPoint;            /*  Offset of PM entry point within BIOS */
    uint16_t  PMInitialize;          /*  Offset of PM initialization entry point */
    uint16_t  BIOSDataSel;           /*  Selector to BIOS data area emulation block */
    uint16_t  A0000Sel;              /*  Selector to access A0000h physical mem */
    uint16_t  B0000Sel;              /*  Selector to access B0000h physical mem */
    uint16_t  B8000Sel;              /*  Selector to access B8000h physical mem */
    uint16_t  CodeSegSel;            /*  Selector to access code segment as data */
    uint8_t   InProtectMode;         /*  Set to 1 when in protected mode */
    uint8_t   Checksum;              /*  Checksum byte for structure */
}; 
#pragma pack(pop)

int main(int argc, char const *argv[])
{
        uint8_t  *biosStart = (uint8_t *)0xC0000;
        //uint32_t  biosLimit = 0xFFFF;
        uint32_t  pmidLimit = 0x8000; /* according to VBE spec PMID is located withing first 32kB of the BIOS */
        uint32_t  i, structIter;
//        uint8_t   fastIndex;
        uint8_t   PMIDSignature[] = "PMID";
        uint8_t   sum;
        struct VBE_PMInfoBlock* pmib;
        uint16_t  numCandidates = 0;
        uint8_t   VGABIOShasNoProtectedMode = 0;

        printf("\n*** VBE PMInfoBlock struc - search ***\n");
        for(i=0;i<=(pmidLimit-sizeof(struct VBE_PMInfoBlock));i++){
            if(*(uint32_t*)(biosStart+i) == *(uint32_t*)(PMIDSignature)){
                numCandidates += 1;
                printf("PMID candidate: %p\n", (biosStart+i));
                sum = 0;
                for(structIter=0;structIter<sizeof(struct VBE_PMInfoBlock);structIter++){
                    sum += *(uint8_t *)(biosStart+i+structIter);
                }
                pmib = (struct VBE_PMInfoBlock*)(biosStart+i);
                //printf("\nSignature:    ");       /*  PM Info Block Signature */
                //for(fastIndex=0;fastIndex<sizeof(pmib->Signature);fastIndex++)printf("%c", pmib->Signature[fastIndex]);
                printf("\nEntryPoint:   %6d\t0x%x",  pmib->EntryPoint   ,  pmib->EntryPoint   );       /*  Offset of PM entry point within BIOS */
                printf("\nPMInitialize: %6d\t0x%x",  pmib->PMInitialize ,  pmib->PMInitialize );       /*  Offset of PM initialization entry point */
                printf("\nBIOSDataSel:  %6d\t0x%x",  pmib->BIOSDataSel  ,  pmib->BIOSDataSel  );       /*  Selector to BIOS data area emulation block */
                printf("\nA0000Sel:     %6d\t0x%x",  pmib->A0000Sel     ,  pmib->A0000Sel     );       /*  Selector to access A0000h physical mem */
                printf("\nB0000Sel:     %6d\t0x%x",  pmib->B0000Sel     ,  pmib->B0000Sel     );       /*  Selector to access B0000h physical mem */
                printf("\nB8000Sel:     %6d\t0x%x",  pmib->B8000Sel     ,  pmib->B8000Sel     );       /*  Selector to access B8000h physical mem */
                printf("\nCodeSegSel:   %6d\t0x%x",  pmib->CodeSegSel   ,  pmib->CodeSegSel   );       /*  Selector to access code segment as data */
                printf("\nInProtectMode:%6d\t0x%x",  pmib->InProtectMode,  pmib->InProtectMode);       /*  Set to 1 when in protected mode */
                printf("\nChecksum:     %6d\t0x%x",  pmib->Checksum     ,  pmib->Checksum     );       /*  Checksum byte for structure */
                
                printf("\n\n");
                if(sum == 0){
                    printf("checksum ... OK\n");
                    break;
                }
                else {
                    printf("checksum ... FAILED\n");
                    if(pmib->A0000Sel == 0xA000 && pmib->B0000Sel == 0xB000 && pmib->CodeSegSel == 0xC000){

                        printf("looks like PMID anyway\n");
                        break; // the true PMID struc could be somewhere later in the ROM
                    }
                    else
                        continue;
                }
            }
        }
        if(numCandidates == 0){
            VGABIOShasNoProtectedMode = 1;
        }
        printf("\n*** END OF VBE PMInfoBlock struc ***\n");
    return 1;
}

/* end of file */
