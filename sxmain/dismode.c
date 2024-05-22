
#include <stdlib.h>
#include <string.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <devices/serial.h>
#include "SXStructs.h"
#include "sx.h"

extern struct StructsStruct	*Structs;
extern struct AmigaStruct	Amiga;
extern struct StringsStruct	*stg;
extern struct VariablesStruct	var;

extern UBYTE buf[512], charbuf[128];
extern UWORD color[17], pens[];

extern void Colors(UWORD *ccolor);


int DisModeCompare(struct ScreenModeStruct *dmone, struct ScreenModeStruct *dmtwo)
{
	if(dmone->DisplayID != dmtwo->DisplayID) return(1);
	if(dmone->DisplayWidth != dmtwo->DisplayWidth) return(2);
	if(dmone->DisplayHeight != dmtwo->DisplayHeight) return(3);
	if(dmone->DisplayDepth != dmtwo->DisplayDepth) return(4);
	if(dmone->unit != dmtwo->unit) return(1);

	if(stricmp(dmone->device, dmtwo->device)!=0) return(5);
	if(stricmp(dmone->fontname, dmtwo->fontname)!=0) return(6);

	return(0);
}

int DisMode(void)
{
	BPTR fh;
	long size;

	mysprintf(buf, "SX:Node%ld/Prefs/DisplayModes.DAT", var.node);
	if(!(fh = Open(buf, MODE_OLDFILE))) return(0);
	Seek(fh, 0, 1);
	size = Seek(fh, 0, -1)/sizeof(struct DisplayStruct);

start:
	if(stg->arg && stg->arg[0]!=0)
	{
		var.cmode = atoi(stg->arg);
		if(var.cmode>0 && var.cmode<=size) goto doit;
	}
	ShowAnsi("SX:TXT/DisplayMode.TXT", 0, 0, 0, 0, 0, 0);
	while(1)
	{
		LineInput(24, 0, 0);
		if(!var.CARRIER) { Close(fh); return(0); }

		if(charbuf[0]=='?') goto start;
		var.cmode = atoi(charbuf);
		if(charbuf[0]==0) var.cmode = 1;
		if(var.cmode>0 && var.cmode<=size) break;
		PSFmt("[%ldD[K", strlen(charbuf));
	}
doit:
	PS("\r\n");
	Seek(fh, (var.cmode-1)*sizeof(struct DisplayStruct), OFFSET_BEGINNING);
	Read(fh, &Structs->Disp, sizeof(struct DisplayStruct));
	Close(fh);

	mysprintf(buf, "SX:Node%ld/Prefs/Mode%ldPalette.DAT", var.node, var.cmode);
	if(fh = Open(buf, MODE_OLDFILE))
	{
		Read(fh, &color, 32);
		Read(fh, &pens, 24);
		Close(fh);
	} else {
		color[0] = 0x0000;
		color[8] = 0x0555;
		color[9] = 0x0F00;
		color[10] = 0x00F0;
		color[11] = 0x0FF0;
		color[12] = 0x000F;
		color[13] = 0x0F0F;
		color[14] = 0x00FF;
		color[15] = 0x0FFF;
		switch(Structs->ScreenMode.DisplayDepth)
		{
		case 1:
			color[1] = 0x0FFF;
			break;
		case 4:
			color[1] = 0x0D00;
			color[2] = 0x00D0;
			color[3] = 0x0DD0;
			color[4] = 0x000D;
			color[5] = 0x0D0D;
			color[6] = 0x00DD;
			color[7] = 0x0DDD;
			break;
		default:
			color[1] = 0x0F00;
			color[2] = 0x00F0;
			color[3] = 0x0FF0;
			color[4] = 0x000F;
			color[5] = 0x0F0F;
			color[6] = 0x00FF;
			color[7] = 0x0FFF;
			break;
		}
		fh = Open(buf, MODE_NEWFILE);
		if(fh)
		{
			Write(fh, &color, 32);
			Write(fh, &pens, 24);
			Close(fh);
		}
	}

	LoadStrings();
	if(DisModeCompare(&Structs->Disp.ScreenMode, &Structs->ScreenMode))
	{
		memcpy(&Structs->ScreenMode, &Structs->Disp.ScreenMode, sizeof(struct ScreenModeStruct));
		if(var.CON)
		{
			CloseDisplay();
			OpenDisplay(TRUE);
		}
	} else {
		if(var.CON) Colors((UWORD *)&color);
	}

	return(1);
}
