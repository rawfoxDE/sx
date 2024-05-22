
/* ================================================================== */
/* ======================== CONFIG LOADING ========================== */
/* ================================================================== */

#include <stdlib.h>
#include <string.h>
#include <exec/memory.h>
#include <exec/types.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/graphics.h>
#include <graphics/text.h>
#include <devices/serial.h>
#include "SXStructs.h"
#include "SX.h"

extern struct StructsStruct	*Structs;
extern struct TextAttr		ansia;
extern struct AmigaStruct	Amiga;
extern struct StringsStruct	*stg;
extern struct VariablesStruct	var;

extern UWORD color[17], pens[];
extern UBYTE buf[512];


void LoadPrefs(void)
{
	BPTR fh;

	mysprintf(buf, "SX:Node%ld/Prefs/Serial.DAT", var.node);
	if(fh = Open(buf, MODE_OLDFILE))
	{
		Read(fh, &Structs->Serial, sizeof(struct SerialStruct));
		strcat(Structs->Serial.answer, "\r");
		strcat(Structs->Serial.initstr, "\r");
		Close(fh);
		if(Structs->Serial.dcerate == 0) Structs->Serial.dcerate = 19200;
	} else {
		strcpy(Structs->Serial.device, "serial.device");
		Structs->Serial.unit=0;
		Structs->Serial.dcerate=19200;
	}

	mysprintf(buf, "SX:Node%ld/Prefs/Doors.DAT", var.node);
	fh = Open(buf, MODE_OLDFILE);
	if(fh)
	{
		Read(fh, &Structs->Doors, sizeof(struct DoorsStruct));
		Close(fh);
	}

	mysprintf(buf, "SX:Node%ld/Prefs/NodePrefs.DAT", var.node);
	fh = Open(buf, MODE_OLDFILE);
	if(fh)
	{
		Read(fh, &Structs->NodePrefs, sizeof(struct NodeStruct));
		Close(fh);
	} else {
		Structs->NodePrefs.systempass[0]=0;
		Structs->NodePrefs.newuserpass[0]=0;
	}
}

void LoadLocalDis(void)
{
	BPTR fh;

	mysprintf(buf,"SX:Node%ld/Prefs/LocalDis.DAT",var.node);
	if(fh = Open(buf,MODE_OLDFILE)) {
		Read(fh,&Structs->ScreenMode,sizeof(struct ScreenModeStruct));
		Close(fh);
	}
	ansia.ta_Name = (STRPTR)&Structs->ScreenMode.fontname;
	ansia.ta_YSize = Structs->ScreenMode.fontsize;

	mysprintf(buf, "SX:Node%ld/Prefs/LocalDisPal.DAT", var.node);
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

	if(var.CON)
	{
		if(DisModeCompare(&Structs->Disp.ScreenMode, &Structs->ScreenMode))
		{
			CloseDisplay();
			OpenDisplay(TRUE);
		}
	}
	Structs->Disp.flags = 0;
	var.cmode = 0;
}

void LoadConfs(void)
{
	int i;
	BPTR fh = Open("SX:Prefs/Confs.DAT" ,MODE_OLDFILE);
	if(fh)
	{
		i=0;
		while(i!=200)
		{
			i++;
			Structs->Conf[i] = AllocMem(sizeof(struct ConfStruct), MEMF_PUBLIC|MEMF_CLEAR);
			if(!Read(fh, Structs->Conf[i], sizeof(struct ConfStruct)))
			{
				var.confs=i-1;
				i=200;
				FreeMem(Structs->Conf[i], sizeof(struct ConfStruct));
			}
		}
		Close(fh);
	}
}

void LoadArcs(void)
{
	BPTR fh;
	int i = 0;

	mysprintf(buf, "SX:Node%ld/Prefs/Archives.DAT", var.node);
	fh = Open(buf, MODE_OLDFILE);
	if(fh)
	{
		while(i!=200)
		{
			i++;
			Structs->Arc[i] = AllocMem(sizeof(struct ArchiveStruct), MEMF_PUBLIC|MEMF_CLEAR);
			if(!Read(fh, Structs->Arc[i], sizeof(struct ArchiveStruct)))
			{
				var.arcs = i-1;
				FreeMem(Structs->Arc[i], sizeof(struct ArchiveStruct));
				i=200;
			}
		}
		Close(fh);
	}
}

void FreePrefs(void)
{
	UWORD i = 0;

	while(i != var.confs)
	{
		i++;
		FreeMem(Structs->Conf[i], sizeof(struct ConfStruct));
	}

	i = 0;
	while(i != var.arcs)
	{
		i++;
		FreeMem(Structs->Arc[i], sizeof(struct ArchiveStruct));
	}
}
