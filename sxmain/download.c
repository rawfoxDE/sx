
#include <stdlib.h>
#include <string.h>
#include <exec/memory.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <xproto.h>
#include <devices/serial.h>
#include "sxstructs.h"
#include "SX.h"

extern struct StructsStruct	*Structs;
extern struct Library		*XProtocolBase;
extern struct AmigaStruct	Amiga;
extern struct StringsStruct	*stg;
extern struct VariablesStruct	var;

extern UBYTE buf[512], charbuf[128];
extern long x;

extern long __saveds __asm xpr_ffirst(register __a0 STRPTR buffer, register __a1 STRPTR pattern);
extern BOOL CopyFile(char *fname, char *dest);

void SysopDownload(void);
int ActualDownload(void);
BOOL LoadProtocol(char idletter);
void FlagArgs(BOOL sysop);

void DownloadFile(void)
{
	BOOL addfin = FALSE;

	SetActive(1, 0);
	if(Structs->Conf[var.thisconf]->fileareas == 0)
	{
		PS(Structs->SXStr[109]);
		return;
	}
	ShowAnsi("DownLoad",1,1,1,1,1,0);

	if(stg->arg[0] != 0) FlagArgs(FALSE);

	while(!addfin)
	{
		PSFmt(Structs->SXStr[38], var.fileflags, var.byteflags);
		LineInput(50, 0, 0);
		PS("\r\n");
		if(charbuf[0]==0 || !var.CARRIER) addfin=TRUE; else
		{
			if(strchr(charbuf, '/') || strchr(charbuf, ':') || strchr(charbuf, '\\'))
				PS(Structs->SXStr[87]);
			else {
				switch(FlagFile(charbuf,0,0,0))
				{
				case 0: PS(Structs->SXStr[34]); break;
				case 1: PSFmt(Structs->SXStr[35], charbuf, x); break;
				case 2: PS(Structs->SXStr[33]); break;
				case 3: PS(Structs->SXStr[31]); break;
				case 4: PS(Structs->SXStr[107]); break;
				case 5: PS(Structs->SXStr[106]); break;
				}
			}
		}
	}
	if(ActualDownload()) ShowAnsi("PostDownLoad",1,1,1,1,1,0);
}

int ActualDownload(void)
{
	BOOL discon = FALSE;
	UWORD i;
	char tbuf[128];

	if(!var.CARRIER || var.fileflags==0 || var.byteflags==0) return(0);
	if(ListFlags()) ShowAnsi("TimeWarn", 1, 1, 1, 1, 1, 0);
dlprompt:
	PS(Structs->SXStr[43]);
	LineInput(4,0,0);
	if(!var.CARRIER) return(0);
	PSLen("\r\n",2);
	switch(charbuf[0])
	{
	case 'A':
	case 'a':
		return(0);
	case 'c':
	case 'C':
		User_Protocol();
		goto dlprompt;
	case 'd':
	case 'D':
		discon=TRUE;
		break;
	case 'e':
	case 'E':
		AlterFlags();
		goto dlprompt;
	}

	PSFmt(Structs->SXStr[39], Structs->Proto.name);

	if(var.LOCAL)
	{
		struct Flag *worknode, *nextnode;
	
		if (Amiga.Flags->lh_TailPred != (struct Node *)Amiga.Flags)
		{
			worknode = (struct Flag *)(Amiga.Flags->lh_Head); /* First node */
			if(!worknode) return(0);
			while (nextnode = (struct Flag *)(worknode->flag_node.ln_Succ))
			{
				PSFmt(Structs->SXStr[141], worknode->file, Structs->Cfg->DNPath);
				strcpy(buf, worknode->path);
				strcat(buf, worknode->file);
				CopyFile(buf, Structs->Cfg->DNPath);
				Remove(worknode);
				FreeMem(worknode, sizeof(struct Flag));
				worknode = nextnode;
			}
			var.fileflags=0;
		        var.byteflags=0;
		        var.fileflagsfree=0;
		        var.byteflagsfree=0;
		}
		return(0);
	}

	xpr_ffirst(tbuf, 0);
	RawTransfer(1, FALSE, FALSE, Structs->NodePrefs.fcheck, tbuf);

	if(discon && var.CARRIER)
	{
		SetActive(107, 0);
		PS("\r\n[K\r\n[K");
		i=10;
		while(i!=0)
		{
			PSFmt(Structs->SXStr[61], i);
			var.TIME_OUT = 1;
			MyHotKey();
			if(!var.CARRIER) return(0);
			if(charbuf[0]==''){
				var.TIME_OUT=0;
				var.TIMEOUTFLAG=FALSE;
				return(1);
			}
			i--;
		}
		DropDTR(TRUE);
		var.offtype=2;
		var.TIME_OUT=0;
		var.TIMEOUTFLAG=FALSE;
		return(0);
	}
	return(1);
}

void SysopDownload(void)
{
	BOOL addfin = FALSE;

	SetActive(1, 0);
	ShowAnsi("SysOpDownLoad",1,1,1,1,1,0);

	if(stg->arg[0] != 0) FlagArgs(TRUE);

	while(!addfin)
	{
		PSFmt(Structs->SXStr[38], var.fileflags, var.byteflags);
		LineInput(50,0,0);
		PS("\r\n");
		if(charbuf[0]==0 || !var.CARRIER) addfin=TRUE; else
		{
			switch(FlagFile(charbuf,TRUE,0,TRUE))
			{
			case 0: PS(Structs->SXStr[34]); break;
			case 1: PSFmt(Structs->SXStr[35], charbuf, x); break;
			case 2: PS(Structs->SXStr[33]); break;
			case 3: PS(Structs->SXStr[31]); break;
			}
		}
	}
	if(ActualDownload()) ShowAnsi("PostSysOpDownLoad",1,1,1,1,1,0);
}

BOOL LoadProtocol(char idletter)
{
	BPTR fh;
	BOOL readfin = FALSE;

	if(idletter == 0) idletter = 'Z';

	mysprintf(buf, "SX:Node%ld/Prefs/Protocols.DAT", var.node);
	fh = Open(buf, MODE_OLDFILE);
	if(fh)
	{
		while(!readfin)
		{
			x = Read(fh, &Structs->Proto, sizeof(struct ProtoStruct));
			if(!x)
				readfin = TRUE;
			else {
				if(Structs->Proto.letter == idletter) readfin = TRUE;
			}
		}
		Close(fh);
		if(Structs->Proto.letter != idletter) goto internal;
		return(TRUE);
	} else {
internal:
		strcpy(Structs->Proto.name, "Z-Modem");
		Structs->Proto.letter = 'Z';
		Structs->Proto.type = 0;
		Structs->Proto.efficiency = 95;
		strcpy(Structs->Proto.library, "xprzmodem.library");
		strcpy(Structs->Proto.initstr, "TC,OR,B16,FO,AN,DN,KY,SN,RN");
		return(FALSE);
	}
}

void FlagArgs(BOOL sysop)
{
	BOOL flagdone = FALSE;
	UWORD i = 0, last = 0;

	while(!flagdone)
	{
		if(stg->arg[i] == ' '  ||  stg->arg[i] == 0)
		{
			if(stg->arg[i] == 0) flagdone = TRUE;
			stg->arg[i] = 0;
			strcpy(charbuf, &stg->arg[last]);
			last = i + 1;

			PSFmt(Structs->SXStr[38], var.fileflags, var.byteflags);
			PSFmt("%s\r\n", charbuf);

			if( (strchr(charbuf, '/') || strchr(charbuf, ':') || strchr(charbuf, '\\')) && !sysop )
				PS(Structs->SXStr[87]);
			else {
				switch(FlagFile(charbuf, sysop, 0, sysop))
				{
				case 0: PS(Structs->SXStr[34]); break;
				case 1: PSFmt(Structs->SXStr[35], charbuf, x); break;
				case 2: PS(Structs->SXStr[33]); break;
				case 3: PS(Structs->SXStr[31]); break;
				case 4: PS(Structs->SXStr[107]); break;
				case 5: PS(Structs->SXStr[106]); break;
				}
			}
		}
		i++;
	}
}
