/*---------------------------------------------------------*/
/* Demo link code c++ with asm code (see circlea.asm)      */
/* Compile: OpenWatcom C/C++                               */
/*          wasm -zq -fp3 -mc -zcm=tasm circlea.asm        */
/*          wcl -zq -ox -fp3 -3 -mc circlec.c circlea.obj  */
/*---------------------------------------------------------*/

#include <dos.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <conio.h>

float costab[400] = {0};
float sintab[400] = {0};

uint8_t *vbuff1, *vbuff2, *vbuff3;
uint8_t *tmem = (uint8_t *)0xB8000000L;
uint8_t *vmem = (uint8_t *)0xA0000000L;

extern int16_t  _cdecl  mouseDetect();
extern void     _cdecl  getMousePos(int16_t*, int16_t*, int16_t*);
extern void     _cdecl  setMouseWindow(int16_t, int16_t, int16_t, int16_t);
extern void     _cdecl  setMousePos(int16_t, int16_t);
extern void     _cdecl  flipScreen(uint8_t*, uint8_t*);
extern void     _cdecl  putPixel(int16_t, int16_t, uint8_t);
extern uint8_t  _cdecl  getPixel(int16_t, int16_t);
extern void     _cdecl  clearTextMem();
extern void     _cdecl  clearBuffer(uint8_t*);
extern void     _cdecl  writeString(int16_t, int16_t, uint8_t, char*);
extern void     _cdecl  waitPressKey(int16_t);
extern void     _cdecl  setVideoMode(int16_t);
extern void     _cdecl  closeMouse();
extern void     _cdecl  setPalette(uint8_t*);

void preCalc()
{
    int16_t i;
    float deg = 0.0;
    
    for (i = 0; i < 400; i++)
    {
        costab[i] = cos(deg);
        sintab[i] = sin(deg);
        deg += 0.05;
    }
}

inline int16_t roundf(float x)
{
    if (x > 0.0) return x + 0.5;
    return x - 0.5;
}

void drawCircle(int16_t ox, int16_t oy, int16_t rad)
{
    int16_t i;
    int16_t dx, dy;

    for (i = 0; i < 400; i++)
    {
        dx = roundf(rad * costab[i]);
        dy = roundf(rad * sintab[i]);
        putPixel(ox + dx, oy + dy, getPixel(ox + dx, oy + dy));
    }
}

void main()
{
    FILE *fp;

    int16_t i;
    int16_t x, y, btn;
    uint8_t pal[768] = {0};

    clearTextMem();
    writeString(1, 1, 0x0F, "CircleC - (c) 1998 by Nguyen Ngoc Van");
    writeString(1, 2, 0x07, "Use mouse to move - Any key/Left Button to exit");
    writeString(1, 3, 0x07, "Press space bar to start demo");
    waitPressKey(0x39);

    if (!mouseDetect()) return;

    vbuff1 = (uint8_t*)malloc(64000);
    vbuff2 = (uint8_t*)malloc(64000);
    vbuff3 = (uint8_t*)malloc(64000);

    if (!vbuff1) return;
    if (!vbuff2) return;
    if (!vbuff3) return;

    clearBuffer(vbuff1);
    clearBuffer(vbuff2);
    clearBuffer(vbuff3);

    preCalc();

    fp = fopen("image.cel", "rb");
    if (!fp) return;

    fseek(fp, 32, SEEK_SET);
    fread(pal, 1, 768, fp);
    fread(vbuff1, 1, 64000, fp);
    fclose(fp);

    fp = fopen("wall.cel", "rb");
    if (!fp) return;
    
    fseek(fp, 800, SEEK_SET);
    fread(vbuff3, 1, 64000, fp);
    fclose(fp);

    setVideoMode(0x13);
    setPalette(pal);
    setMouseWindow(38, 38, 319 - 38, 199 - 38);
    setMousePos(40, 70);
    while(kbhit()) getch();
    
    do {
        getMousePos(&x, &y, &btn);
        flipScreen(vbuff3, vbuff2);
        for (i = 0; i < 38; i++) drawCircle(x, y, i);
        flipScreen(vbuff2, vmem);
    } while(!kbhit() && btn != 1);

    setVideoMode(0x03);
    closeMouse();

    free(vbuff1);
    free(vbuff2);
    free(vbuff3);
}
