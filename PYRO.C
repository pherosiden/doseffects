char c1[]="PYRO - A Simulated Pyrotechnics Display",
	  c2[]="Version 2.03  08/08/1998.",
	  c3[]="(c) 1988 by Nguyen Ngoc Van.";

/*

This program is a major revision of PYRO, the program that won the 1987 "Your
Computer" Microsoft Computer Program Art Competition.  A number of features
have been added that could not be included in PYRO version 1.00 because of
the 199 line length limit imposed by the competition.

Additions include:

. An opening screen.

. A new effect - Catherine wheels.

. CGA and text-only displays are supported.

. The program detects processor speed and adjusts delay loops to compensate.
  It also synchronizes itself with the timer tick of the machine.  Both these
  actions are necessary to generate a smooth display and reasonable sound
  effects.  The number of points used per effect may also be reduced, from a
  maximum of 60 down as far as about 24 if necessary.

. The program has been optimized considerable including trade-offs of size
  for speed, and can now run on a much wider range of machines.  On very
  slow machines it switches from the standard script to a simplified one
  which limits the amount of simultaneous screen activity that can occur.
*/

/* Timing results (with synch=0 and tot_loop=0) - ratio of unrestrained
execution to desired execution time.  These are used to calculate delay
loop values which compensate for the differing speeds of various machines.
These values were obtained using version 2.00.

np      Display      NEC Powermate    Cleveland AT
                       31/01/88         01/02/88
60      EGA             0.911            0.888
30      CGA             0.543            0.541
30      TEXT            0.274            0.274
60      suppressed      0.309            0.308
30      suppressed      0.171            0.181

Clock calls/timer tick    339              330
*/

#if !(defined(MSC) || defined(QUICKC) || defined(TURBOC))
	 /* One only of the following should be non-zero */
	 #define MSC     0    /* Microsoft Optimizing C Compiler */
	 #define QUICKC  1    /* Microsoft Quick C */
	 #define TURBOC  0    /* Turbo C */
#endif

#define ref_ega 0.911
#define ref_cga 0.543
#define ref_text 0.274
#define ref_herc 0.543 /* A first guess */
#define ref_60 0.309
#define ref_30 0.171
#define ref_count 339

#if MSC || QUICKC
	 #include <stdlib.h>
	 #include <dos.h>
	 #include <math.h>
	 #include <string.h>
	 #include <stdio.h>
	 #include <conio.h>

#elif TURBOC
	 #include <stdlib.h>
	 #include <dos.h>
	 #include <math.h>
	 #include <string.h>
	 #include <stdio.h>
	 #include <conio.h>

	 //#define outpw outport
#endif

/* The following factors 'correct' the speed estimates to allow for the fact
that some compilers generate better optimized code in the speed estimation
routine than they do in the remainder of the program, and vice versa.  The
REPCOR values are reasonably accurate, but the CALCCOR values are still
only best guesses.*/
#if MSC
	 #define REPCOR 0.695
	 #define CALCCOR 0.7
#elif QUICKC
	 #define REPCOR 1.076
	 #define CALCCOR 0.65
#elif TURBOC
	 #define REPCOR 1.0
	 #define CALCCOR 1.0
#endif

#define NI 6 /*Number of items that may exist simultaneously*/
#define TP 400 /*Total number of points*/
#define UNKNOWN -1
#define VGA 0
#define EGA 1
#define CGA 2
#define MDA 3
#define HGC 4

#define COLOUR 0
#define MONO 1

#define EGM 0
#define CGM 1
#define TXTM 2
#define HGCM 3

#define STANDARD 0
#define SIMPLE 1

void (*show)(int, int, int, int, int);

union REGS inr,outr;

char far *bytepointer,far *egapointer,far *cgapointer,far *hgcpointer,far *cgapointer0,
    far *cgapointer1,far *txtpointer,far * hgcpointers[4];

unsigned int address_6845,far *wordpointer;

char inkey,txt[]="THE END",cpoint[]=" .+*@@@@@@@",restore_screen[4000],
    *ddesc[]={"VGA","EGA","CGA","MDA","HERCULES"},
    *sdesc[]={"COLOUR","MONOCHROME"},
    *mdesc[]={"EGA","CGA","TEXT","HERCULES"};

int xl[NI],xh[NI],yl[NI],yh[NI],vell[NI],velh[NI],angl[NI],angh[NI],alivel[NI],
    aliveh[NI],coll[NI],colh[NI],sizev[NI],fadev[NI],item_alive[NI],
    devicetype[NI],wait[NI],master[NI],xref[NI],yref[NI],wheelstep[NI],
    wheelrep[NI],pdormant[NI],
    g[TP],x[TP],y[TP],xvel[TP],yvel[TP],alive[TP],col[TP],size[TP],fade[TP],
    itemno[TP],next[TP],state[TP],rowaddress[350],col_table[16],
    adapter,screen,mode,sc_xmax,sc_ymax,margin,baseline,sp_vl,sp_vh,fl_vl,fl_vh,
    rk_vl,rk_vh,rktail_vl,rktail_vh,ro_vl,ro_vh,wh_vl,wh_vh,sh,shx,shy,address,
    oty_fac,ot_size,etx_off,etx_f1,etx_f2,ety_off,ety_fac,tr_l,tr_h,t_s,t_f,ro_rad,
    continuous,burstwait,fullspeed,testcapacity,
    spread,cga_palette,swap_flag,swaplevel,mincol,maxcol,endcol,count,report,quiet,
    hs,ms,ss,he,me,se,stime,etime,extime,tot_loop,indep,per_point,slow1,synch,
    np,im,jm,km,xmin,xmax,ymin,ymax,xend,yend,newpt,endpt,ci,state2,fl_mask,
    num_active,pfree,pactive,num_dormant,mastercol,wheelradius,capacity,est_capacity,
    wheelstepv,wheelreplim,wheelsteplim,wh_mask,excount,script,rep_tot_loop,
    spurt=1,flare=2,burst=3,rocket=4,roman=5,wheel=6,
    gv=2,burstlife=10,
    noise=0;

float sina[401],cosa[401],t_slope,t_intercept;

unsigned long ticks1,ticks2;

void gotoxy(int col, int row) /* Moves the cursor to a specific column and row */
{
    inr.h.ah=2;
    inr.h.bh=0;
    inr.x.dx=((row-1)<<8)+col-1;
    int86(0X10,&inr,&outr);
} /*End gotoxy*/

void put_string(char *str,int col,int x,int y) /* Put string on screen at x,y*/
{
    static int i,j,k;
    static char far *bytepointer;
    if (mode==HGCM)
    {

	bytepointer=(char far *)0xf0000000;
	for (i=0;i<=strlen(str);i++)
	{
	    if (str[i]!=' ')
		for (j=7;j>=0;j--)
		    for (k=0;k<=7;k++)
			if ((bytepointer[0xfa6e+j+8*str[i]] & (0x80>>k))>0)
			{
			    show((9*(x+i)+k)<<sh,(13*y+j)<<sh,1,1,1);

			}
	}

    }
    else
    {
	for (i=0;i<strlen(str);i++)
	{
	    gotoxy(x+i,y);
	    inr.h.ah=9;
	    inr.h.al=str[i];
	    inr.h.bh=0;
	    inr.x.cx=1;
	    inr.h.bl=col;
	    int86(0x10,&inr,&outr);
	}
    }
} /*End put_string*/

unsigned long get_ticks() /* Get current timer tick count*/
{
    inr.h.ah=0;
    int86(0x1a,&inr,&outr);
    return (unsigned long)outr.x.cx<<16 | outr.x.dx;
} /*End get_ticks*/

void terminate();

void setrange(int *d1,int *d2,int s1,int s2) /*Store limits information*/
{
    if (s1<=s2)
    {
        *d1=s1;
        *d2=s2;
    } else {
        *d1=s2;
        *d2=s1;
    }
} /*End setrange*/

void limits(int it,int lxl,int lxh,int lyl,int lyh,int lvell,int lvelh,int langl,int langh,int lalivel,int laliveh,
int lcoll,int lcolh,int lsize,int lfade) /*Set limits for point generation for item it*/
{
    setrange(&xl[it],&xh[it],lxl<<sh,lxh<<sh);
    setrange(&yl[it],&yh[it],lyl<<sh,lyh<<sh);
    setrange(&vell[it],&velh[it],lvell<<sh,lvelh<<sh);
    setrange(&angl[it],&angh[it],langl,langh);
    setrange(&alivel[it],&aliveh[it],lalivel,laliveh);
    setrange(&coll[it],&colh[it],lcoll,lcolh);
    sizev[it]=lsize;
    fadev[it]=lfade;
} /*End limits*/

int rnd(int low,int high) /*Returns a value between low and high (inclusive)*/
{
    return low+(((long)rand()*(high-low+1))>>15);
} /*End rnd*/

int setscreen(int n) /* Set screen mode n.  Function value returns the mode
  actually set.*/
{
    char far *bptr;
    static long i;
    if (n==-7)
    {
	/* Set up Hercules graphics mode.  Method as given in Programmer's
	guide to PC & PS/2 Video Systems, Richard Wilton, Microsoft Press.*/

	bptr=(char far *) 0x00400049; /* BIOS data area*/
	bptr[0]=7; /* Crt mode*/
	bptr[1]=0x80;  bptr[2]=0x00;  /* Crt cols*/
	bptr[3]=0x00;  bptr[4]=0x80;  /* Crt len*/
	bptr[5]=0x00;  bptr[6]=0x00;  /* Crt start*/
	bptr[7]=0x00;  bptr[8]=0x00;  /* Cursor posn*/
	bptr[9]=0x00;  bptr[10]=0x00; /* Cursor posn*/
	bptr[11]=0x00; bptr[12]=0x00; /* Cursor posn*/
	bptr[13]=0x00; bptr[14]=0x00; /* Cursor posn*/
	bptr[15]=0x00; bptr[16]=0x00; /* Cursor mode*/
	bptr[17]=0x00; bptr[18]=0x00; /* Active page*/
	bptr[19]=0xb4; bptr[20]=0x03; /* Addr 6845*/
	bptr[21]=0x0a; /* Crt mode set (value for port 0x3b8*/
	bptr[22]=0x00; /*Crt palette (unused)*/

	outp(0x3bf,1); /* Allow graphics mode*/
	outp(0x3b8,0); /* Blank screen while programming CRTC*/
	outpw(0x3b4,0x3500); /* Horizontal Total: 54 characters*/
	outpw(0x3b4,0x2d01); /* Horizontal Displayed: 45 characters*/
	outpw(0x3b4,0x2e02); /* Horizontal Synch Position: at 46th character*/
	outpw(0x3b4,0x0703); /* Horizontal Synch Width: 7 character clocks*/
	outpw(0x3b4,0x5b04); /* Vertical Total: 92 Characters (368 lines)*/
	outpw(0x3b4,0x0205); /* Vertical Adjust: 2 scan lines*/
	outpw(0x3b4,0x5706); /* Vertical Displayed: 87 character rows*/
	outpw(0x3b4,0x5707); /* Vertical Synch Position: after 87th char row*/
	outpw(0x3b4,0x0309); /* Max Scan Line: 4 scan lines per character*/
	/* Clear screen before re-enabling*/
	bptr=(char far *) 0xb0000000;
	for (i=0;i<0x8000;i++) bptr[i]=0;
	outpw(0x3b8,0x0a); /* Enable graphics mode and video*/
	return -7;
    }
    else
	 {
	inr.h.ah=0;
	inr.h.al=n;
	int86(0x10,&inr,&outr); /* Set screen mode*/
	inr.h.ah=15;
	int86(0x10,&inr,&outr); /* Get video state*/
	return outr.h.al;
    }
} /*End setscreen*/

void setup_ega() /* Set up the ega screen for fireworks graphics*/
{
    if (mode==EGM)
    {
	outp(0x3ce,3);
	outp(0x3cf,0x18);/*Select Xor operation*/
    }
} /*End setup_ega*/

void reset_ega() /* Reset ega screen to standard settings for graphics*/
{
    if (mode==EGM)
    {
	outp(0x3ce,3);
	outp(0x3cf,0);/*Cancel Xor*/
	outp(0x3ce,8);
	outp(0x3cf,0xff);/*No mask*/
	outp(0x3c4,2);
	outp(0x3c5,0xff);/*Enable all bit planes*/
    }
} /*End reset_ega*/

void save_background() /* Save the current screen background into restore_screen*/
{
    if (mode==TXTM)
    {
        for(address=0;address<4000;address++)
	{
	    restore_screen[address]=txtpointer[address];
        }
    }
} /*End save_background*/

void clearscreen() /* Clear the screen*/
{
    switch(mode)
    {
    case EGM:
        reset_ega();
	for (address=0;address<28000;address++)
	{
            egapointer[address]=0;
	}
        break;
    case CGM:
        for (address=0;address<8000;address++)
        {
            cgapointer0[address]=0;
            cgapointer1[address]=0;
        }
        break;
    case TXTM:
        for(address=0;address<4000;address++)
        {
            txtpointer[address]=0;
        }
        save_background();
        break;
    case HGCM:
	for (address=0;address<7830;address++)
	{
	    hgcpointers[0][address]=0;
	    hgcpointers[1][address]=0;
	    hgcpointers[2][address]=0;
	    hgcpointers[3][address]=0;
	}
	break;
    }
} /*End clearscreen*/

void swap_cga() /* Swaps to the other palette (only if in cga mode)*/
{
    if (mode==CGM)
    {
        cga_palette=1-cga_palette;
        inr.h.ah=11;
        inr.h.bh=1;
        inr.h.bl=cga_palette;
        int86(0x10,&inr,&outr);
    }
} /*End cga_palette*/

void showega(int x,int y,int col,int size,int on)
/*EGA - Show (or remove) point x,y if it is in the screen area*/
{
    static int i,x1,x2,dummy,lbit,rbit,xsh,sizem1,sizem1mi,rowaddr;
    if (y>=ymin) if (x>=xmin) if (x<=xmax)
    {
        xsh=x>>sh;
        sizem1=size-1;
        for (i=-sizem1; i<=sizem1; i++)
        {
            sizem1mi=sizem1-abs(i);
            rowaddr=rowaddress[(y>>sh)+i];
            x1=xsh-sizem1mi;
            x2=xsh+sizem1mi;
            while (x1<=x2)
            {
                lbit=x1 & 7;
                rbit=(x2>=(x1 | 7))? 7 : lbit+x2-x1;
	        outp(0x3ce,8);
		outp(0x3cf,(unsigned char)(0xff<<(7-rbit+lbit))>>lbit);/*Set mask*/
                dummy=egapointer[address=rowaddr+(x1>>3)];/*Load latches*/
                outp(0x3c4,2);
                outp(0x3c5,col);/*Select bit planes*/
                egapointer[address]=0xff;/*Set selected bits to 1*/
                x1=x1+rbit-lbit+1;
            }
        }
    }
} /*End showega*/

void showcga(int x,int y,int col,int size,int on)
/*CGA - Show (or remove) point x,y if it is in the screen area*/
{
    static int shift[4]={6,4,2,0},i,j,sx,sy,pair,xsh,ysh,sizem1,sizem1mi,rowaddr;
    if (y>=ymin) if (x>=xmin) if (x<=xmax)
    {
        xsh=x>>sh;
        ysh=y>>sh;
        sizem1=size-1;
        for (i=-sizem1; i<=sizem1; i++)
        {
            sizem1mi=sizem1-abs(i);
            sy=ysh+i;
            rowaddr=rowaddress[sy];
            cgapointer=(sy & 1)? cgapointer1 : cgapointer0;
            for (j=-sizem1mi; j<=sizem1mi; j++)
            {
                sx=xsh+j;
                address=rowaddr+sx>>2;
                pair=sx & 3;
		cgapointer[address]^=((col & 3)<<shift[pair]);
            }
        }
    }
} /*End showcga*/

void showhgc(int x,int y,int col,int size,int on)
/*Hercules - Show (or remove) point x,y if it is in the screen area*/

{
    static int i,j,sx,sy,xsh,ysh,sizem1,sizem1mi,rowaddr;
    if (y>=ymin) if (x>=xmin) if (x<=xmax)
    {
	xsh=x>>sh;
	ysh=y>>sh;
	sizem1=size-1;
	for (i=-sizem1; i<=sizem1; i++)
	{
	    sizem1mi=sizem1-abs(i);
	    sy=ysh+i;
	    rowaddr=rowaddress[sy];
	    hgcpointer=hgcpointers[sy & 3];
	    for (j=-sizem1mi; j<=sizem1mi; j++)
	    {
		sx=xsh+j;
		address=rowaddr+(sx>>3);
		hgcpointer[address]^=(1<<(7-(sx & 7)));
	    }
	}
    }
} /*End showhgc*/

void showtxt(int x,int y,int col,int size,int on)
/*Text - Show (or remove) point x,y if it is in the screen area*/
{
    if (y>=ymin) if (x>=xmin) if (x<=xmax)
    {
        address=(rowaddress[y>>shy]+(x>>shx))<<1;
        if (on)
        {
            txtpointer[address]=cpoint[size];
            txtpointer[address+1]=col;
        }
        else
        {
            txtpointer[address]=restore_screen[address];
            txtpointer[address+1]=restore_screen[address+1];
        }
    }
} /*End showtxt*/

void create(int it,int *pt,int *plist,int *num_inlist) /*Create a point for
  item it, add to list plist, increment count num_inlist*/
{
    static int vel,angle;
    if (pfree>=0)
        *pt=pfree;
    else
    {
        reset_ega();
        printf("Error: point data storage overflow");
        exit(0);
    }
    itemno[*pt]=it;
    g[*pt]=gv;
    fade[*pt]=fadev[it];
    state[*pt]=0;
    x[*pt]=rnd(xl[it],xh[it]);
    y[*pt]=rnd(yl[it],yh[it]);
    vel=rnd(vell[it],velh[it]);
    angle=rnd(angl[it],angh[it]);
    xvel[*pt]=vel*sina[180+angle];
    yvel[*pt]=-(vel*cosa[180+angle]);
    col[*pt]=col_table[rnd(coll[it],colh[it])];
    size[*pt]=sizev[it];
    alive[*pt]=rnd(alivel[it],aliveh[it]);
/* Remove from free list and add to indicated list*/
    pfree=next[*pt];
    next[*pt]=*plist;
    *plist=*pt;
    (*num_inlist)++;
} /*End create*/

void move_points() /*Move all active points through 1 step*/
{
    static int i,j,k,n,lastj,nextj;
/* The motion is smoothed by distributing most of the necessary time wasting
in a delay loop that is executed slow1 times each time a point is moved.  This
is also important for generating the best sound effects.  A number of
parameters enter into the calculation of slow1 and these are specified in terms
of a unit that represents the time required to execute the delay loop once.
Only a reasonable estimate of the delay is needed because the delay loop will
short-circuit itself if the next timer tick occurs before all points have been
moved, and on the other hand any excess time is soaked up by the tick synch
routine.
Parameters:  tot_loop  - time taken for one timer tick
             indep     - part of execution time that is independent of num_active
             per_point - extra execution time per point*/
    if (num_active>0)
    {
        slow1=(tot_loop-indep-num_active*per_point)/num_active;
        j=pactive;
        lastj=-1;
        n=num_active;
        for(k=1;k<=n;k++)
        {
            nextj=next[j];
	    if (noise>0) if (rand()<noise) outp(0x61,inp(0x61)|3);
            if (state[j]>0) show(x[j],y[j],col[j],size[j],0);
            else state[j]=state2;
            yvel[j]+=g[j];
            x[j]+=xvel[j];
            if ((y[j]+=yvel[j])>ymax) /* Don't allow to fall off the bottom*/
            {
                y[j]-=yvel[j];
                xvel[j]=0;
            }
            if (--alive[j]<=0)
            {
                if (size[j]>1) /* Fade point*/
                {
                    size[j]--;
                    alive[j]=fade[j];
                    show(x[j],y[j],col[j],size[j],1);
                }
                else /* Remove point*/
                {
                    if (lastj==-1) pactive=next[j];
                    else next[lastj]=next[j];
                    next[j]=pfree;
                    pfree=j;
                    j=lastj;
                    num_active--;
                }
            }
            else
                show(x[j],y[j],col[j],size[j],1);
	    if (noise>0) outp(0x61,inp(0x61)&0xfc);
            for (i=1;i<=slow1;i++) /* Delay loop*/
            {
                ticks2=get_ticks();
                if (ticks1!=ticks2) slow1=0; /* Enough delay already*/
            }
            lastj=j;
            j=nextj;
        }
    }
    if ((noise=abs(noise))>0) noise-=300;
/* Swap cga palette any time the screen goes quiet*/
    if (num_active<=swaplevel)
    {
        if (swap_flag)
        {
            swap_cga();
            swap_flag=0;
        }
    }
    else
        swap_flag=1;
    count++;
/* Speed synchronization on timer tick*/
    if (synch)
    {
        while (ticks1==ticks2) ticks2=get_ticks();
        ticks1=ticks2;
    }
} /*End move_points*/

void activate(int it) /*Activate all dormant points for item it*/
{
    static int j,n;
/* Insert into list of active points*/
        j=pdormant[it];
        n=1;
        while (next[j]>-1)
        {
            j=next[j];
            n++;
            if (n>np)
            {
                reset_ega();
                printf("Error: list structure corrupt");
                exit(0);
            }
        }
        next[j]=pactive;
        pactive=pdormant[it];
        num_active+=n;
        num_dormant-=n;
        pdormant[it]=-1;
} /*End activate*/

void process(int t,int waiting) /*Process currently set devices for t intervals*/
{
    static int it,i,timer;
    for (timer=1;timer<=t;timer++)
    {
        for (it=0;it<NI;it++)
        {
            if (item_alive[it]>0) item_alive[it]--;
            else devicetype[it]=0;
            switch(devicetype[it])
            {
            /*SPURT*/ case 1:   if (wait[it]>130) create(it,&newpt,&pactive,&num_active);
                else if (wait[it]==0)
                {
                    setrange(&coll[it],&colh[it],rnd(mincol,maxcol),rnd(mincol,maxcol));
                    wait[it]=np+130;
                }
                break;
            /*FLARE*/ case 2:   if (wait[it]&fl_mask) create(it,&newpt,&pactive,&num_active);
                if (wait[it]==0)
                {
                    if (++coll[it]>=maxcol) coll[it]=mincol;
                    colh[it]=coll[it]+1;
                    wait[it]=50;
                }
                break;
            /*BURST*/ case 3:   if (wait[it]>burstwait) create(it,&master[it],&pdormant[it],&num_dormant);
                else if (wait[it]==0)
                {
                    activate(it);
                    if (quiet==0) noise=-3000;
                }
                break;
            /*ROCKET*/ case 4:  if (wait[it]==0)
                {
                    activate(it);
                    item_alive[it]=alive[master[it]]+fade[master[it]]*(size[master[it]]-1);
                }
                else if (wait[it]<0)
                {
                    create(it,&newpt,&pactive,&num_active);
                    x[newpt]=x[master[it]]+rnd(-64,64);
                    y[newpt]=y[master[it]]+rnd(-64,64);
                }
                break;
            /*ROMAN*/ case 5:   if ((wait[it]<=9) && (wait[it]>0))
                {
                    create(it,&newpt,&pdormant[it],&num_dormant);
                    x[newpt]=x[master[it]]+ro_rad*sina[180+(360*(wait[it]-5)/9)];
                    y[newpt]=y[master[it]]-ro_rad*cosa[180+(360*(wait[it]-5)/9)];
                    xvel[newpt]=xvel[master[it]];
                    yvel[newpt]=yvel[master[it]];
                    alive[newpt]=alive[master[it]];
                    col[newpt]=col[master[it]];
                } else if (wait[it]==0) {
                    limits(it,0,0,0,0,0,0,0,0,5,10,col[master[it]],col[master[it]],1,0);
                    activate(it);
                    item_alive[it]=alive[master[it]]+fade[master[it]]*(size[master[it]]-1);
                }
                else if (wait[it]<0)
                {
                    create(it,&newpt,&pactive,&num_active);
                    x[newpt]=x[master[it]]+rnd(-256,256);
                    y[newpt]=y[master[it]]+rnd(-64,64);
                }
                break;
            /*WHEEL*/ case 6:    for(i=1;i<=wheelrep[it];i++)
                {
                    angl[it]+=wheelstep[it];
                    angh[it]+=wheelstep[it];
                    if (angl[it]>=180)
                    {
                        angl[it]-=360;
                        angh[it]-=360;
                        if ((wheelstep[it]+=2)>wheelsteplim) wheelstep[it]=wheelsteplim;
                        if (++wheelrep[it]>wheelreplim) wheelrep[it]=wheelreplim;
                    }
                    xl[it]=xref[it]+wheelradius*cosa[180+angl[it]];
                    xh[it]=xl[it];
                    yl[it]=yref[it]+wheelradius*sina[180+angl[it]];
                    yh[it]=yl[it];
                    if (i&wh_mask) create(it,&newpt,&pactive,&num_active);
                }
                break;
            } /*End switch*/
            if ((wait[it]>-1) && (it!=waiting)) wait[it]--;
        }
        if (kbhit()) terminate();
        move_points();
    }
} /*End process*/

void opening_screen() /*Display opening screen*/
{
    static int i,j,k,l,kk,npoints,moving,xp[150],yp[150],tlength,xoff,yoff,
        lastj,nextj;
    static char txt[]="PYRO";
    tlength=strlen(txt);
    xoff=(sc_xmax-tlength*2*etx_f1+4*etx_f2)/2;
    yoff=sc_ymax/3;
    gv=2;
    state2=1;
    swaplevel=-1;
    clearscreen();
    if (mode==CGM) put_string(c2,col_table[mincol],9,19);
    else put_string(c2,col_table[mincol],29,19);
    if (mode==CGM) put_string(c3,col_table[mincol],9,21);
    else put_string(c3,col_table[mincol],29,21);
    save_background();
    setup_ega();
    /* Store the point positions*/
    npoints=-1;
    moving=0;
    bytepointer=(char far *)0xf0000000;
    for (i=0;i<=tlength;i++)
    {
        if (txt[i]!=' ')
            for (j=7;j>=0;j--)
                for (k=0;k<=7;k++)
                {
                    if ((bytepointer[0xfa6e+j+8*txt[i]] & (0x80>>k))>0)
                    {
                        npoints++;
                        xp[npoints]=(xoff+2*etx_f1*i+2*etx_f2*k)<<sh;
                        yp[npoints]=(yoff+oty_fac*j)<<sh;
                    }
                }
    }
    /* Generate the points in random order*/
    kk=1;
    if (script==SIMPLE) kk=3;
    k=0;
    while (npoints>-1 || moving>0)
    {
        if (npoints>-1 && k==0)
        {
            i=rnd(0,npoints);
            limits(0,sc_xmax/2,sc_xmax/2,baseline,baseline,
              0,0,0,0,120,170,mincol,maxcol,2,0);
            create(0,&newpt,&pactive,&num_active);
            l=alive[newpt]-1;
            xvel[newpt]=(xp[i]-x[newpt])/l;
	    x[newpt]=xp[i]-xvel[newpt]*l;
            yvel[newpt]=-(gv*l)/2+(yp[i]-y[newpt])/l;
            y[newpt]=(long)yp[i]-((long)yvel[newpt]*(long)l+(long)gv*(long)l*(long)l/2);
            moving++;
            xp[i]=xp[npoints];
            yp[i]=yp[npoints];
            npoints--;
        }
	process(1,-1);
	setup_ega(); /* Quick C seems to need this here*/
        j=pactive;
        lastj=-1;
        while (j>-1) /* Stop each point at its destination, pre-empting move_points*/
        {
            nextj=next[j];
            if (alive[j]<=1)
            {
                show(x[j],y[j],col[j],size[j],0);
                show(x[j],y[j],col[j],ot_size,1);
                if (mode==TXTM)
                {
                    restore_screen[address]=txtpointer[address];
                    restore_screen[address+1]=txtpointer[address+1];
                }
                if (lastj==-1) pactive=next[j];
                else next[lastj]=next[j];
                next[j]=pfree;
                pfree=j;
                j=lastj;
                num_active--;
                moving--;
            }
            lastj=j;
            j=nextj;
        }
        k++;
        if (k>kk) k=0;
    }
    for (i=1;i<=4*18;i++) /* Pause with title on screen*/
    {
        while (ticks1==ticks2) ticks2=get_ticks();
        ticks1=ticks2;
    }
    clearscreen();
    setup_ega();
    gv=2;
    swaplevel=10;
} /*End opening_screen*/

void terminate() /*Terminate display*/
{
    static int i,j,k,l;
    if ((inkey=kbhit()? getch(): 0)!=27)
        for (j=1;(j<=800) && (num_active+num_dormant>0);j++) process(1,-1);
    inr.h.ah=0x2c;
    int86(0x21,&inr,&outr); /* Get end time*/
    he=outr.h.ch;
    me=outr.h.cl;
    se=outr.h.dh;
    excount=count;
    if (inkey!=27 && capacity==0)
    {
        gv=0;
        state2=1;
        bytepointer=(char far *)0xf0000000;
        clearscreen();
        setup_ega();
        for (i=0;i<=6;i++)
        {
            if (txt[i]!=' ')
                for (j=7;j>=0;j--)
                    for (k=0;k<=7;k++)
                        if ((bytepointer[0xfa6e+j+8*txt[i]] & (0x80>>k))>0)
                        {
                            limits(0,etx_off+etx_f1*i+etx_f2*k+tr_l,etx_off+etx_f1*i+etx_f2*k+tr_h,ety_off+ety_fac*j+tr_l,ety_off+ety_fac*j+tr_h,0,0,0,0,30,50,endcol,endcol,t_s,t_f);
                            create(0,&newpt,&pactive,&num_active);
                            for (l=1;l<=3000;l++);
                        }
                        process(4,-1);
        }
        for (i=1;(i<=800) && (num_active>0);i++)
        {
            process(1,-1);
            j=pactive;
            while (j>-1) /* Make titles start to fall*/
            {
                if (alive[j]<=10) g[j]=1;
                j=next[j];
            }
        }
    }
    clearscreen();
    setscreen(3);
    if (inkey==0)
    {
        if (he<hs) he=he+24;
        stime=60*ms+ss;
        etime=60*me+se;
        etime=etime+3600*(he-hs);
        extime=etime-stime;
        printf("Ideal  execution time: %d seconds\n",(int)(excount/18.2));
        printf("Actual execution time: %d seconds\n",extime);
	printf("Ratio: %f\n",extime/(excount/18.2));
    }
#if MSC
    printf("\nCompiled using Microsoft Optimising C Compiler\n");
#elif QUICKC
    printf("\nCompiled using Microsoft QuickC\n");
#elif TURBOC
    printf("\nCompiled using Turbo C\n");
#endif
    exit(0);
} /*End terminate*/

void calc_end() /*Calculate endpoint for burst*/
{
    endpt=np-wait[ci]+burstwait+1;
    if ((mastercol=col[master[ci]])==maxcol) mastercol=maxcol-1;
    alive[master[ci]]=endpt-(size[master[ci]]-1)*fade[master[ci]];
    xend=x[master[ci]]+endpt*xvel[master[ci]];
    yend=y[master[ci]]+endpt*yvel[master[ci]]+(gv*endpt*endpt)/2;
} /*End calc_end*/

void startup(int devtype,int xi,int yi,int life,int p1,int p2,int p3,int p4,int waiting) /*Initializes a device*/
{
    ci=0;
    while (item_alive[ci]>0)
    {
        process(1,waiting);
        if (++ci>=NI) ci=0;
    }
    devicetype[ci]=devtype;
    item_alive[ci]=life;
    switch(devicetype[ci])
    {
    /*SPURT*/ case 1:   limits(ci,xi-spread,xi+spread,baseline,baseline,sp_vl,sp_vh,-20,20,120,160,rnd(mincol,maxcol),rnd(mincol,maxcol),2,25);
        wait[ci]=np+130;
        break;
    /*FLARE*/ case 2:   coll[ci]=rnd(mincol,maxcol-1);
        limits(ci,xi-spread,xi+spread,baseline,baseline,fl_vl,fl_vh,-17,17,20,30,coll[ci],coll[ci]+1,2,2);
        wait[ci]=50;
        break;
    /*BURST*/ case 3:   if (waiting<0) coll[ci]=rnd(mincol,maxcol-1);
        else coll[ci]=mastercol;
        limits(ci,xi-spread,xi+spread,yi-spread,yi+spread,rnd(p1,p2),rnd(p3,p4),-180,180,burstlife,2*burstlife,coll[ci],coll[ci]+1,2,rnd(burstlife,3*burstlife));
        wait[ci]=np+burstwait;
        break;
    /*ROCKET*/ case 4:  limits(ci,xi,xi,baseline,baseline,rk_vl,rk_vh,p1,p2,100,100,mincol,maxcol,rnd(3,4),3);
        wait[ci]=20;
        create(ci,&master[ci],&pdormant[ci],&num_dormant);
        limits(ci,xi,xi,baseline,baseline,rktail_vl,rktail_vh,90,90,6,7,col[master[ci]],col[master[ci]],2,20);
        vell[ci]=vell[ci]>>1;
        velh[ci]=velh[ci]>>1;
        calc_end();
        break;
    /*ROMAN*/ case 5:   limits(ci,xi,xi,baseline,baseline,ro_vl,ro_vh,-10,10,100,100,mincol,maxcol,3,1);
        wait[ci]=30;
        create(ci,&master[ci],&pdormant[ci],&num_dormant);
        calc_end();
        break;
    /*WHEEL*/ case 6:   coll[ci]=rnd(mincol,maxcol-1);
        limits(ci,xi,xi,yi,yi,wh_vl,wh_vh,-1,1,15,20,coll[ci],coll[ci]+1,2,2);
        xref[ci]=xl[ci];
        yref[ci]=yl[ci];
        wheelstep[ci]=wheelstepv;
        wheelrep[ci]=1;
        wait[ci]=0;
        break;
    }
} /*End startup*/

void wait_for(int waitlevel) /* Waits until there are waitlevel or fewer points
  on the screen*/
{
    while (num_active>waitlevel) process(1,-1);
} /*End wait_quiet*/

void multiple(int typ,int n,int rep,int life,int gap,int pause,int waitlevel)
  /* Multiple item display*/
{
    static int i,j;
    for (j=1;j<=rep;j++)
    {
        for (i=1;i<=n;i++)
        {
            switch(typ)
            {
            /*ROCKET*/ case 4: startup(rocket,sc_xmax/4+i*(sc_xmax/(n+1))/2,baseline,100,-45,45,0,0,-1);
                if (rand()<25000) startup(burst,xend>>sh,yend>>sh,100,1,3,5,12,ci);
                break;
            /*ROMAN*/ case 5: startup(roman,i*(sc_xmax/(n+1)),baseline,100,0,0,0,0,-1);
                startup(burst,xend>>sh,yend>>sh,100,8,8,10,12,ci);
                break;
            default: if (typ==burst) startup(burst,rnd(sc_xmax/4,3*sc_xmax/4),rnd(sc_ymax/7,4*sc_ymax/7),life,1,3,5,12,-1);
                else if (typ==wheel) startup(typ,i*(sc_xmax/(n+1)),2*sc_ymax/3,life,0,0,0,0,-1);
                else startup(typ,i*(sc_xmax/(n+1)),baseline,life,0,0,0,0,-1);
            } /*End switch*/
            process(gap,-1);
        }
        process(pause,-1);
/* Create gaps to allow cga colour changes*/
        if (mode==CGM) wait_for(waitlevel);
    }
}/*End multiple*/

void test_capacity() /* Test the point capacity of the current processor*/
/* This function is used for timing tests.  The program should be run with
all slowdown features turned off when this is invoked.*/
{
    static int n;
    inr.h.ah=0x2c;
    int86(0x21,&inr,&outr); /* Get start time*/
    hs=outr.h.ch;
    ms=outr.h.cl;
    ss=outr.h.dh;
    np=tot_loop/3;
    n=tot_loop/30;
    if (n>6) n=6;
    multiple(spurt,n,1,np,0,0,10000);
    while (num_active>0)
    {
        /* Wait for a timer tick*/
        ticks1=get_ticks();
        ticks2=ticks1;
        while (ticks1==ticks2) ticks2=get_ticks();
        ticks1=ticks2;
        /* Move points then see if it happened within 1 tick*/
        process(1,-1);
        if (ticks2==ticks1 && num_active>capacity) capacity=num_active;
        if (num_active>TP+n)
        {
            capacity=TP;
            num_active=0;
        }
    }
    clearscreen();
    setscreen(3);
    printf("Capacity %d points at full speed",capacity);
}/*End test_capacity*/

void main(int argc,char **argv)
{
    static int ega_byte,val1,val2;
    static unsigned int count;
    report=0; /* If non-zero, prints debugging information*/
    quiet=0; /* If non-zero, suppresses sound effects*/
    adapter=UNKNOWN;
    screen=UNKNOWN;
    mode=UNKNOWN;
/* Decode any command line parameters*/
    continuous=0;
    fullspeed=0;
    testcapacity=0;
    for (im=1; im<argc; im++)
    {
        if ((strlen(argv[im])==1) && ((argv[im][0]=='c') || (argv[im][0]=='C'))) continuous=1;
        if ((strlen(argv[im])==1) && ((argv[im][0]=='r') || (argv[im][0]=='R'))) report=1;
        if ((strlen(argv[im])==1) && ((argv[im][0]=='q') || (argv[im][0]=='Q'))) quiet=1;
        if ((strlen(argv[im])==1) && ((argv[im][0]=='f') || (argv[im][0]=='F'))) fullspeed=1;
        if ((strlen(argv[im])==1) && ((argv[im][0]=='t') || (argv[im][0]=='T'))) testcapacity=1;
        if ((strlen(argv[im])>1) && ((argv[im][0]=='e') || (argv[im][0]=='E'))) mode=EGM;
        if ((strlen(argv[im])>1) && ((argv[im][0]=='c') || (argv[im][0]=='C'))) mode=CGM;
        if ((strlen(argv[im])>1) && ((argv[im][0]=='t') || (argv[im][0]=='T'))) mode=TXTM;
	if ((strlen(argv[im])>1) && ((argv[im][0]=='h') || (argv[im][0]=='H'))) mode=HGCM;
    }
/* Identify the display adapter and screen*/
    /* Check for VGA*/
    inr.h.ah=0x1a;
    inr.h.al=0;
    int86(0x10,&inr,&outr);
    if (outr.h.al!=0)
    {
        /* VGA present, test whether active, get screen type*/
        if (outr.h.bl==7)
        {
            adapter=VGA;
            screen=MONO;
        }
        else if(outr.h.bl==8)
        {
            adapter=VGA;
            screen=COLOUR;
        }
    }
    if (adapter==UNKNOWN)
    /* Check for EGA*/
    {
        inr.h.ah=0x12;
        inr.h.bl=0x10;
        int86(0x10,&inr,&outr);
        if (outr.h.bl!=0x10)
        {
            /* EGA present, test whether active, get screen type*/
            bytepointer=(char far *)0x400087;
            ega_byte=*bytepointer;
            if ((ega_byte & 8)==0)
            {
                adapter=EGA;
                if (outr.h.bh==0) screen=COLOUR;
                else screen=MONO;
            }
        }
    }
    if (adapter==UNKNOWN)
    /* Check for CGA*/
    {
        wordpointer=(unsigned int far *)0x400063;
        address_6845=*wordpointer;
        if ((address_6845 & 0x40)!=0)
        {
            adapter=CGA;
            screen=COLOUR;
        }
        else
	{
	    adapter=MDA;
	    screen=MONO;
	    /* Check for HGC by looking for a change in bit 7 of port 0x3ba*/
	    val1=inp(0x3ba) & 0x80;
	    for (count=0;count<32767;count++)
	    {
		val2=inp(0x3ba) & 0x80;
		if (val2!=val1)
		{
		    adapter=HGC;
		    count=32767;
		}
	    }
        }
    }
    if (report) printf("Detected %s adapter\n",ddesc[adapter]);
    if (report && ((adapter==VGA) || (adapter==EGA))) printf("Detected %s screen\n",sdesc[screen]);
/* Determine speed of machine*/
/* Wait for a tick*/
    ticks2=get_ticks();
    do
        ticks1=get_ticks();
    while (ticks2==ticks1);
/* Count the number of calls to the next tick*/
    tot_loop=0;
    do
    {
        ticks2=get_ticks();
        tot_loop++;
    }
    while (ticks2==ticks1);
    tot_loop=tot_loop;
    /* Adjust reported value for various compilers and optimizations*/
    rep_tot_loop=tot_loop*REPCOR;
    if (report) printf("Measured processor speed: %d\n",rep_tot_loop);
    if (report) printf("For comparison: 4.77 MHz XT = 81, 8MHz AT = 330\n");
    tot_loop=tot_loop*CALCCOR; /* Adjust by a different factor for internal use*/
    if (adapter==MDA) est_capacity=.6*tot_loop;
    else est_capacity=.4*tot_loop;
    if (report) printf("\nCan move approx. %d points without loss of speed\n",est_capacity);
    np=est_capacity/2;
    if (np>60) np=60;
    if (np>=25) script=STANDARD;
    else
    {
        script=SIMPLE;
        np=1.5*np;
        if (report) printf("Using simplified script\n");
    }
    if (report) printf("Using a limit of %d points per device\n",np);
/* If not specified on the command line, decide which mode to use*/
    if (mode==UNKNOWN)
    {
        if ((adapter==VGA) || (adapter==EGA)) mode=EGM;
	else if (adapter==CGA) mode=CGM;
	else if (adapter==HGC) mode=HGCM;
        else mode=TXTM;
        if ((mode==EGM) && (tot_loop<250))
        {
            mode=CGM;
            if (report) printf("This computer is too slow to run the EGA mode display\n");
        }
        if (report) printf("Display mode set to %s\n",mdesc[mode]);
    }
/* Parameters that relate to the graphics screen being used*/
    if (mode==EGM)
    {
	sh=5;
        sc_xmax=639;
        sc_ymax=349;
        margin=5;
        baseline=340;
        spread=5;
        sp_vl=3;
        sp_vh=8;
        fl_vl=4;
        fl_vh=8;
        fl_mask=0xffff;
        rk_vl=5;
        rk_vh=10;
        rktail_vl=-2;
        rktail_vh=2;
        ro_vl=6;
        ro_vh=11;
        wh_vl=4;
        wh_vh=7;
        wheelradius=10<<sh;
        wheelreplim=4;
        wh_mask=0xffff;
        ro_rad=3<<sh;
        oty_fac=16;
        ot_size=4;
        etx_off=68;
        etx_f1=72;
        etx_f2=8;
        ety_off=150;
        ety_fac=8;
        tr_l=-1;
        tr_h=2;
        t_s=3;
        t_f=65;
        mincol=9;
        maxcol=15;
        endcol=13;
    }
    else if (mode==CGM)
    {
	sh=5;
        sc_xmax=319;
        sc_ymax=199;
        margin=5;
        baseline=195;
        spread=2;
        sp_vl=3;
        sp_vh=6;
        fl_vl=3;
        fl_vh=5;
        fl_mask=1;
        rk_vl=3;
        rk_vh=7;
        rktail_vl=-1;
        rktail_vh=1;
        ro_vl=3;
        ro_vh=6;
        wh_vl=2;
        wh_vh=4;
        wheelradius=6<<sh;
        wheelreplim=4;
        wh_mask=1;
        ro_rad=2<<sh;
        oty_fac=8;
        ot_size=3;
        etx_off=34;
        etx_f1=36;
        etx_f2=4;
        ety_off=75;
        ety_fac=6;
        tr_l=0;
        tr_h=0;
        t_s=2;
        t_f=130;
        mincol=1;
        maxcol=3;
        endcol=1;
    }
    else if (mode==TXTM)
    {
        sh=5;
        shx=7;
        shy=8;
        sc_xmax=319;
        sc_ymax=199;
        margin=0;
        baseline=195;
        spread=2;
        sp_vl=3;
        sp_vh=6;
        fl_vl=3;
        fl_vh=5;
        fl_mask=1;
        rk_vl=3;
        rk_vh=7;
        rktail_vl=-1;
        rktail_vh=1;
        ro_vl=3;
        ro_vh=7;
        wh_vl=2;
        wh_vh=4;
        wheelradius=6<<sh;
        wheelreplim=4;
        wh_mask=1;
        ro_rad=2<<sh;
        oty_fac=8;
        ot_size=3;
        etx_off=34;
        etx_f1=36;
        etx_f2=4;
        ety_off=75;
        ety_fac=8;
        tr_l=0;
        tr_h=0;
        t_s=2;
        t_f=130;
        mincol=9;
        maxcol=15;
        endcol=13;
    }
    if (mode==HGCM)
    {
	sh=5;
        sc_xmax=719;
        sc_ymax=347;
        margin=5;
        baseline=338;
        spread=5;
        sp_vl=3;
        sp_vh=8;
        fl_vl=4;
        fl_vh=8;
        fl_mask=0xffff;
        rk_vl=5;
        rk_vh=10;
        rktail_vl=-2;
        rktail_vh=2;
        ro_vl=6;
        ro_vh=11;
        wh_vl=4;
        wh_vh=7;
        wheelradius=10<<sh;
        wheelreplim=4;
        wh_mask=0xffff;
        ro_rad=3<<sh;
        oty_fac=16;
        ot_size=4;
	etx_off=76;
	etx_f1=81;
	etx_f2=9;
        ety_off=150;
        ety_fac=8;
        tr_l=-1;
        tr_h=2;
        t_s=3;
        t_f=65;
        mincol=1;
        maxcol=1;
        endcol=1;
    }
/* Set up the colour table*/
    for (im=0;im<=15;im++) col_table[im]=im;
    if (adapter==MDA)
    {
        mincol=1;
        maxcol=2;
        col_table[1]=7;
        col_table[2]=15;
    }
/* Calculate speed control parameters*/
    t_slope=(ref_60-ref_30)/30;
    t_intercept=2*ref_30-ref_60;
    indep=ref_count*(t_intercept+np*t_slope);
    if (mode==EGM) per_point=(ref_count*(ref_ega-ref_60))/120;
    else if (mode==CGM) per_point=(ref_count*(ref_cga-ref_30))/60;
    else if (mode==TXTM) per_point=(ref_count*(ref_text-ref_30))/60;
    else per_point=(ref_count*(ref_herc-ref_30))/60;
/*    printf("tot_loop %d indep %d per_point %d \n",tot_loop,indep,per_point);*/
    if (report) printf("Press any key to continue");
    if (report) getch();
/* Initialization*/
    if (mode==EGM)
    {
        setscreen(0x10);
	show=showega;
        egapointer=(char far *)0xa0000000;
	for (im=0;im<=349;im++) rowaddress[im]=im*80;
    }
    else if (mode==CGM)
    {
        setscreen(4);
	show=showcga;
        cgapointer0=(char far *)0xb8000000;
        cgapointer1=(char far *)0xba000000;
	for (im=0;im<=199;im++) rowaddress[im]=(im>>1)*320;
    }
    else if (mode==TXTM)
    {
        im=setscreen(3);
        if (im==7)
        {
            /* Monochrome display adapter*/
            txtpointer=(char far *)0xb0000000;
        }
        else
        {
            /* Cga etc*/
            txtpointer=(char far *)0xb8000000;
        }
	show=showtxt;
    for (im=0;im<=24;im++) rowaddress[im]=im*80;
        inr.h.ah=1;
        inr.h.ch=0xf;
        inr.h.cl=0xf;
        int86(0x10,&inr,&outr); /* Hide cursor*/
    }
    else if (mode==HGCM)
    {
	setscreen(-7);
	show=showhgc;
	hgcpointers[0]=(char far *)0xb0000000;
	hgcpointers[1]=(char far *)0xb0002000;
	hgcpointers[2]=(char far *)0xb0004000;
	hgcpointers[3]=(char far *)0xb0006000;
	for (im=0;im<=347;im++) rowaddress[im]=(im>>2)*90;
    }
    inr.x.ax=0x2c00;
    int86(0x21,&inr,&outr);
    srand(outr.x.dx); /* Seed random number generator*/
    setrange(&xmin,&xmax,margin<<sh,(sc_xmax-margin)<<sh);
    setrange(&ymin,&ymax,margin<<sh,(sc_ymax-margin)<<sh);
    for (im=0;im<NI;item_alive[im]=0,im++) /* Initialize items*/
    for (jm=0;jm<TP;alive[jm]=0,next[jm]=jm+1,jm++);
/* List of free points*/
    pfree=0;
    next[TP-1]=-1; /* End of list*/
    num_active=0;
    pactive=-1; /* List of active points*/
    num_dormant=0;
    for (im=0;im<NI;im++) pdormant[im]=-1; /* Lists of dormant points*/
    for (im=0;im<=90;im++)
    {
        sina[180+im]=sin(im/57.29578);
        sina[180+-im]=-sina[180+im];
        sina[180+180-im]=sina[180+im];
        sina[180+im-180]=-sina[180+im];
        cosa[180+im]=cos(im/57.29578);
        cosa[180+-im]=cosa[180+im];
        cosa[180+180-im]=-cosa[180+im];
        cosa[180+im-180]=-cosa[180+im];
    }
    for (im=181;im<=220;im++) /*Extend angle table for wheel rotation*/
    {
        sina[180+im]=sina[180+im-360];
        cosa[180+im]=cosa[180+im-360];
    }
    swap_flag=0; /* Flag for swapping cga colours*/
    swaplevel=10;
    cga_palette=rnd(0,1);
    swap_cga();
    burstwait=60-np; /* Wait before releasing burst*/
    wheelstepv=3;
    wheelsteplim=20;
    state2=1;
    capacity=0;
    clearscreen();
    setup_ega();
    synch=1; /* If non-zero, invokes timer tick synch*/
    if (testcapacity) fullspeed=1;
    if (fullspeed)
    {
        synch=0;
        tot_loop=0;
    }
    if (synch) ticks1=get_ticks();
    if (testcapacity) test_capacity();

	 do
    {
        opening_screen();
        count=0;
        inr.h.ah=0x2c;
        int86(0x21,&inr,&outr); /* Get start time*/
        hs=outr.h.ch;
        ms=outr.h.cl;
        ss=outr.h.dh;
        if (script==STANDARD)
        {
            multiple(flare,3,1,200,20,150,999);
            multiple(spurt,1,1,450,20,320+3*np,swaplevel+30);
            multiple(wheel,2,1,270,20,240,swaplevel+30);
            multiple(burst,5,1,100,10,75,swaplevel+30);
            multiple(rocket,5,3,100,10,0,swaplevel+30);
            multiple(burst,5,1,100,10,35,swaplevel+10);
            multiple(spurt,2,1,450,20,500,swaplevel+10);
            multiple(flare,4,1,200,20,150,swaplevel+30);
            swaplevel=0; /* Delay colour change while flare still active*/
            multiple(burst,5,1,100,10,25,swaplevel+30);
            swaplevel=10;
            multiple(roman,5,3,100,10,60,swaplevel+30);
            swap_cga();
            /*Set up for finale*/
            state2=0;
            burstlife=20;
            swaplevel=-999;
            startup(burst,sc_xmax/4,4*sc_ymax/7,100,1,3,3,5,-1);
            process(30,-1);
            startup(burst,8*sc_xmax/10,2*sc_ymax/7,100,1,3,3,5,-1);
            process(50,-1);
            startup(burst,4*sc_xmax/10,sc_ymax/7,100,1,3,3,5,-1);
            process(180,-1);
            state2=1;
            burstlife=10;
            swaplevel=10;
            clearscreen(); /*End of finale*/
            setup_ega();
        }
        else
        {
            multiple(flare,3,1,200,20,150,999);
            multiple(spurt,1,1,450,20,320+3*np,swaplevel+30);
            multiple(wheel,1,1,270,20,240,swaplevel+30);
            multiple(burst,5,1,100,20,75,swaplevel+30);
            multiple(rocket,5,3,100,25,20,swaplevel+30);
            multiple(burst,5,1,100,20,35,swaplevel+10);
            multiple(spurt,2,1,450,20,500,swaplevel+10);
            multiple(flare,2,1,200,20,150,swaplevel+30);
            swaplevel=0; /* Delay colour change while flare still active*/
            multiple(burst,5,1,100,20,25,swaplevel+30);
            swaplevel=10;
            multiple(roman,3,3,100,25,50,swaplevel+30);
            swap_cga();
            /*Set up for finale*/
            state2=0;
            burstlife=20;
            swaplevel=-999;
            startup(burst,sc_xmax/4,4*sc_ymax/7,100,1,3,3,5,-1);
            process(30,-1);
            startup(burst,8*sc_xmax/10,2*sc_ymax/7,100,1,3,3,5,-1);
            process(50,-1);
            startup(burst,4*sc_xmax/10,sc_ymax/7,100,1,3,3,5,-1);
            process(180,-1);
            state2=1;
            burstlife=10;
            swaplevel=10;
            clearscreen(); /*End of finale*/
            setup_ega();
        }
    }
    while (continuous);
    terminate();
} /*End main*/
