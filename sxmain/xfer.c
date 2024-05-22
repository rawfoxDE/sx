/*

SYSTEM-X:	TRANSFER ROUTINES

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <exec/memory.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
#include <proto/diskfont.h>
#include <proto/reqtools.h>
#include <proto/gadtools.h>
#include <intuition/intuition.h>
#include <graphics/text.h>
#include <devices/serial.h>
#include <devices/timer.h>
#include <xproto.h>
#include "SXStructs.h"
#include "SX.h"

extern struct StructsStruct	*Structs;
extern struct AmigaStruct	Amiga;
extern struct StringsStruct	*stg;
extern struct VariablesStruct	var;
extern struct TagItem		tags[];
extern struct Library		*XProtocolBase;

extern UBYTE buf[512], charbuf[128];

extern void StoreResume(char *fname);
extern void FileCheckDoor(char *fname);
void XferWindow(void);
long init_xpr(struct XPR_IO *IO);
void CheckFlags(void);
void CheckUploads(void);
BOOL SendFile(char *fname);
void RawTransfer(UWORD xtype, BOOL isend, BOOL freeze, char *recpath, char *fname);

struct timerequest treq;
struct MsgPort *tport;
struct XPR_IO xio;
UWORD timeropen=0, rc, lastgs;

struct BGStruct
{
	struct ConfStruct	*Conf;
	struct UserData		*User;
	struct SXUserStruct	*SXUser;
	long 			baud;
	UWORD			thisfilearea;
	UWORD			thisconf;
	UWORD			node;
} BG;


long opentimer(void)
{
    tport=CreatePort(0,0);
    if(!tport) return(-1);
    treq.tr_node.io_Message.mn_ReplyPort=tport;
    if(OpenDevice("timer.device",UNIT_VBLANK,(struct IORequest *)&treq,0)) return(-1);
    timeropen=1;
    return(0);
}
void closetimer(void)
{
    if(timeropen) CloseDevice((struct IORequest *)&treq);
    if(tport) DeletePort(tport);
}
void qtimer(long micros)
{
    int secs=0;
    if (micros > 1000000) {
	 secs	= micros / 1000000;
	 micros = micros % 1000000;
    }
    treq.tr_time.tv_micro=micros;
    treq.tr_time.tv_secs=secs;
    treq.tr_node.io_Command=TR_ADDREQUEST;
    SendIO((struct IORequest *)&treq);
}

int ProtoStart(void)
{
	XProtocolBase = OpenLibrary(Structs->Proto.library, 0);
	if(!XProtocolBase)
	{
		PS("\r\n[0;31mERROR: [0mCould not open transfer library.\r\n");
		return(0);
	}
	init_xpr(&xio);
	xio.xpr_filename = Structs->Proto.initstr;
	XProtocolSetup(&xio);

	AbortIO(&Amiga.serreadreq);
	WaitIO(&Amiga.serreadreq);
	Amiga.serwritereq.IOSer.io_Command 	= CMD_FLUSH;
	DoIO((struct IORequest *)&Amiga.serwritereq);
	Amiga.serwritereq.IOSer.io_Command 	= CMD_CLEAR;
	DoIO((struct IORequest *)&Amiga.serwritereq);
	Amiga.serwritereq.IOSer.io_Command 	= CMD_WRITE;

	if(var.CON)
	{
		AbortIO(&Amiga.conreadreq);
		WaitIO(&Amiga.conreadreq);
	}

	return(1);
}

void ProtoClean(void)
{
	if(var.CON && Amiga.xferwin)
	{
		CloseWindow(Amiga.xferwin);
		Amiga.xferwin = 0;
		FixTitle();
	}
	if(var.xfertype == 2) SetActive(2, 0); else SetActive(1, 0);

	closetimer();
	XProtocolCleanup(&xio);
	CloseLibrary(XProtocolBase);

	if(var.SER)
	{
		Amiga.serwritereq.IOSer.io_Command	= CMD_CLEAR;
		DoIO((struct IORequest *)&Amiga.serwritereq);

		Amiga.serwritereq.IOSer.io_Command 	= CMD_WRITE;
		Amiga.serwritereq.IOSer.io_Message.mn_ReplyPort = Amiga.WriteSerPort;

		Amiga.serreadreq.IOSer.io_Command 	= CMD_READ;
		Amiga.serreadreq.IOSer.io_Data		= &stg->serbuf;
		Amiga.serreadreq.IOSer.io_Length 	= 1;
	}

	if(var.term)
		SetActive(111, 0);
	else
		UpdateUserInfo();

	var.CARRIER = Carrier();

	if(var.CARRIER && !var.term) Delay(150);
	if(var.CON) SendIO((struct IORequest *)&Amiga.conreadreq);
	if(var.SER) SendIO((struct IORequest *)&Amiga.serreadreq);
	WaitBuf();
	if(var.xfertype == 1) CheckFlags(); else CheckUploads();

	if(!var.term) UpdateUserInfo();
}
LONG __saveds __asm
xpr_finfo(register __a0 STRPTR name,register __d0 LONG type)
{
    struct FileInfoBlock *fib=AllocMem(sizeof(struct FileInfoBlock), 0);
    BPTR lck;
    long result=0;

    if(!fib) return(0);
    if(!(lck=Lock(name, SHARED_LOCK))) goto xit;
    Examine(lck, fib);
    UnLock(lck);
    result=fib->fib_Size;
    if(type==2) result=1;
    else if(type!=1) result=0;
xit:
    FreeMem(fib, sizeof(struct FileInfoBlock));
    return(result);
}

LONG __saveds __asm
xpr_swrite(register __a0 UBYTE *buffer, register __d0 LONG size)
{
    Amiga.serwritereq.IOSer.io_Length=size;
    Amiga.serwritereq.IOSer.io_Data=buffer;
    Amiga.serwritereq.IOSer.io_Command=CMD_WRITE;
    DoIO((struct IORequest *)&Amiga.serwritereq);
    return((long)Amiga.serwritereq.IOSer.io_Error);
}

LONG __saveds __asm
xpr_sread(register __a0 APTR buffer,register __d0 ULONG size,register __d1 ULONG timeout)
{
	long len, nflag, flag;

	flag=1<<Amiga.serwritereq.IOSer.io_Message.mn_ReplyPort->mp_SigBit;

	if(Carrier()==FALSE) return(-1L);
	if(var.CON && Amiga.xferwin) flag|=1<<Amiga.xferwin->UserPort->mp_SigBit;
    SetSignal(0,flag);
    if(timeout) {
	flag|=1<<tport->mp_SigBit;
	qtimer(timeout);
    }
    else {
	Amiga.serwritereq.IOSer.io_Command=SDCMD_QUERY;
	DoIO((struct IORequest *)&Amiga.serwritereq);
	len=Amiga.serwritereq.IOSer.io_Actual;
	if(!len) return(0);
	else {
	    if(len>size) len=size;
	    Amiga.serwritereq.IOSer.io_Command=CMD_READ;
	    Amiga.serwritereq.IOSer.io_Data=buffer;
	    Amiga.serwritereq.IOSer.io_Length=len;
	    DoIO((struct IORequest *)&Amiga.serwritereq);
	    return(len);
	}
    }
    SetSignal(0,flag);
    Amiga.serwritereq.IOSer.io_Command=CMD_READ;
    Amiga.serwritereq.IOSer.io_Data=buffer;
    Amiga.serwritereq.IOSer.io_Length=size;
    SendIO((struct IORequest *)&Amiga.serwritereq);
    nflag=Wait(flag);
	if(var.CON && Amiga.xferwin)
	{
		if(nflag&(1<<Amiga.xferwin->UserPort->mp_SigBit))
		{
			AbortIO(&treq);
			AbortIO(&Amiga.serwritereq);
			WaitIO(&treq);
			WaitIO(&Amiga.serwritereq);
			return(-1);
		}
	}
	if(nflag&(1<<Amiga.serwritereq.IOSer.io_Message.mn_ReplyPort->mp_SigBit))
	{
		AbortIO(&treq);
		WaitIO(&treq);
		return((long)Amiga.serwritereq.IOSer.io_Actual);
	}
	if(nflag&(1<<tport->mp_SigBit))
	{
		AbortIO(&Amiga.serwritereq);
		WaitIO(&Amiga.serwritereq);
		return((long)Amiga.serwritereq.IOSer.io_Actual);
	}
	return(0);
}

/* sflush - flushes serial port */
LONG __saveds __asm
xpr_sflush(void)
{
	Amiga.serwritereq.IOSer.io_Command=CMD_FLUSH;
	DoIO((struct IORequest *)&Amiga.serwritereq);
	return((long)Amiga.serwritereq.IOSer.io_Error);
}

LONG __saveds __asm
xpr_ffirst(register __a0 STRPTR buffer,register __a1 STRPTR pattern)
{
	Amiga.currentnode = (struct Flag *)(Amiga.Flags->lh_Head);
	if(var.instsend && !(Amiga.currentnode->free&1<<1))
	{
redo:
		Amiga.currentnode = (struct Flag *)(Amiga.currentnode->flag_node.ln_Succ);
		if(!Amiga.currentnode) return(0);
		if(Amiga.currentnode->free&1<<1) { } else { goto redo; }
	}
	strcpy(buffer,Amiga.currentnode->path);
	strcat(buffer,Amiga.currentnode->file);
	return(1);
}

LONG __saveds __asm
xpr_fnext(register __d0 LONG oc,register __a0 STRPTR buffer,register __a1 STRPTR pattern)
{
	long eff;

	if(Amiga.currentnode->xfer >= Amiga.currentnode->size)
	{
		if(!(Amiga.currentnode->free&1<<0))
		{
			Structs->User.Downloads++;
			Structs->User.Bytes_Download = Structs->User.Bytes_Download + Amiga.currentnode->size;
			Structs->User.Daily_Bytes_Dld = Structs->User.Daily_Bytes_Dld + Amiga.currentnode->size;
			Amiga.currentnode->free|=1<<0;
		}
		if(Amiga.currentnode->size!=0 && !var.term)
		{
			Amiga.currentnode->free|=1<<2;
			eff = (Amiga.currentnode->cps*100)/(var.baud/10);
			mysprintf(buf,"	Downloading %s %ld bytes\n	 %ld files, %ldk bytes, 1 minutes 10 seconds %ld cps, %ld %% efficiency at %ld\n", Amiga.currentnode->file, Amiga.currentnode->size, 1, Amiga.currentnode->size/1024, Amiga.currentnode->cps, eff, var.baud);
			CallerLog(buf);
		}
		if(!var.term)
		{
			Structs->McpMsg.data1 = (long) Amiga.currentnode->file;
			Structs->McpMsg.data2 = (long) Amiga.currentnode->size;
			Structs->McpMsg.data3 = (long) Amiga.currentnode->cps;
			Structs->McpMsg.data4 = (long) Amiga.currentnode->conf;
			Structs->McpMsg.data5 = (long) Amiga.currentnode->area;
			Structs->McpMsg.action = (char *) mytime();
			Servermsg(15);
		}
	}
	if(Amiga.currentnode->flag_node.ln_Succ)
	{
redo:
		Amiga.currentnode=(struct Flag *)(Amiga.currentnode->flag_node.ln_Succ);
		if(!(Amiga.currentnode->flag_node.ln_Succ)) return(0);
		if(var.instsend)
		{
			if(!Amiga.currentnode) return(0);
			if(!(Amiga.currentnode->free&1<<1)) goto redo;
		}
		if(!Amiga.currentnode->file || Amiga.currentnode->file[0]==0) return(0);
		strcpy(buffer, Amiga.currentnode->path);
		strcat(buffer, Amiga.currentnode->file);
		return(oc);
	} else {
		return(0);
	}
}

LONG __saveds __asm
xpr_gets(register __a0 STRPTR Prompt,register __a1 STRPTR Buffer)
{
    return(0);
}

LONG __saveds __asm
xpr_fopen(register __a0 STRPTR FileName,register __a1 STRPTR AccessMode)
{
	long h, fh;

	if(var.CON && Amiga.xferwin) { EraseRect(Amiga.xrp,21,91,499,100); lastgs=0; }

	switch(*AccessMode)
	{
	case 'r':
		return(Open(FileName, MODE_OLDFILE));

	case 'w':
		strcpy(buf, Structs->Conf[var.thisconf]->filepath);
		strcat(buf, FileName);
		if(h=Lock(buf, ACCESS_READ))
		{
			UnLock(h);
			return(0);
		}
		mysprintf(buf, "%sPaths", Structs->Conf[var.thisconf]->path);
		if(fh=Open(buf, MODE_OLDFILE))
		{
			while(FGets(fh, buf, 500))
			{
				buf[strlen(buf)-1]=0;
				strcat(buf, FileName);
				if(h=Lock(buf, ACCESS_READ))
				{
					UnLock(h);
					Close(fh);
					return(0);
				}
			}
			Close(fh);
		}
		return(Open(FileName, MODE_NEWFILE));

	case 'a':
		if(h=Open(FileName, MODE_OLDFILE))
		{
			Seek((BPTR)h, 0, OFFSET_END);
			return(h);
		}
		return(Open(FileName, MODE_NEWFILE));
	}
	return(0);
}

LONG __saveds __asm
xpr_fclose(register __a0 BPTR File)
{
    if(File) Close(File);
    return(0);
}

LONG __saveds __asm
xpr_fread(register __a0 APTR Buffer,register __d0 LONG Size,register __d1 LONG Count,register __a1 BPTR File)
{
    if(Size==0 || Count==0) return(0);
    return(Read(File,Buffer,Size*Count));
}

LONG __saveds __asm
xpr_fwrite(register __a0 APTR Buffer,register __d0 LONG Size,register __d1 LONG Count,register __a1 BPTR File)
{
    if(Size==0 || Count==0) return(0);
    return(Write(File,Buffer,Size*Count));
}

LONG __saveds __asm
xpr_fseek(register __a0 BPTR File,register __d0 LONG Offset,register __d1 LONG Origin)
{
    long h;
    switch(Origin) {
	case 0: h=OFFSET_BEGINNING; break;
	case 1: h=OFFSET_CURRENT; break;
	case 2: h=OFFSET_END; break;
	default: return(-1);
    }
    return((Seek(File,Offset,h)!=-1)?0:-1);
}

LONG __saveds __asm
xpr_unlink(register __a0 STRPTR FileName)
{
	return(DeleteFile(FileName));
}

void BGCheck(void)
{
	char tbuf[128];

	if(Structs->User.Flags&(1<<0) && Structs->Conf[var.thisconf]->flags&(1<<1) && Amiga.uploadnode && Amiga.uploadnode->file[0]!=0 && Amiga.uploadnode->xfer>=Amiga.uploadnode->size)
	{
		BG.Conf = Structs->Conf[var.thisconf];
		BG.User = &Structs->User;
		BG.SXUser = &Structs->SXUser;
		BG.baud = var.baud;
		BG.thisfilearea = var.thisfilearea;
		BG.thisconf = var.thisconf;
		BG.node = var.node;
		if(strlen(Amiga.uploadnode->file) > 12)
		{
			tbuf[0]=Amiga.uploadnode->file[0];
			strmid(Amiga.uploadnode->file, (char *)&tbuf[1], strlen(Amiga.uploadnode->file)-10, 11);
			Rename(Amiga.uploadnode->file, tbuf);
			strcpy(Amiga.uploadnode->file, tbuf);
		}
		//mysprintf(buf, "run <>NIL: SX:Utils/BGCheck %ld %s%s %ld", (long)&BG, Structs->NodePrefs.fcheck, Amiga.uploadnode->file, Amiga.uploadnode->cps);
		//system(buf);
		mysprintf(buf, "SX:Utils/BGCheck %ld %s%s %ld", (long)&BG, Structs->NodePrefs.fcheck, Amiga.uploadnode->file, Amiga.uploadnode->cps);
		SystemTagList(buf, tags);
	}
}

LONG __saveds __asm
xpr_update(register __a0 struct XPR_UPDATE *xu)
{
	long ud = xu->xpru_updatemask;
	UWORD gs;

	if(!var.CON)
	{
		if(ud&XPRU_DATARATE)
		{
			rc++;
			if(rc == 7)
			{
				rc = 0;
				Structs->McpMsg.data1 = xu->xpru_datarate;
				Structs->McpMsg.data2 = var.xfertype;
				Servermsg(11);
			}
		}
		if(ud&XPRU_FILENAME)
		{
			if(!xu->xpru_filename || xu->xpru_filename[0]==0) return(0);
			if(var.xfertype != 1)
			{
				if(!var.term) BGCheck();
				Amiga.uploadnode = AllocMem(sizeof(struct Flag), MEMF_PUBLIC|MEMF_CLEAR);
				strcpy(Amiga.uploadnode->file, xu->xpru_filename);
				Amiga.uploadnode->flag_node.ln_Name = Amiga.uploadnode->file;
				Amiga.uploadnode->size = xu->xpru_filesize;
				Amiga.uploadnode->xfer = 0;
				AddTail((struct List *)Amiga.Uploads, (struct Node *)Amiga.uploadnode);
			}
		}
		if(ud&XPRU_FILESIZE)
		{
			if(var.xfertype != 1) Amiga.uploadnode->size = xu->xpru_filesize;
		}
		if(ud&XPRU_BYTES)
		{
			if(var.xfertype == 1) Amiga.currentnode->xfer = xu->xpru_bytes;
			else Amiga.uploadnode->xfer = xu->xpru_bytes;
			if(xu->xpru_bytes >= xu->xpru_filesize)
			{
				if(var.xfertype == 1) Amiga.currentnode->cps = xu->xpru_datarate;
				else Amiga.uploadnode->cps = xu->xpru_datarate;
			}
		}
		return(0);
	}
	if(ud&XPRU_PROTOCOL)
	{
		Move(Amiga.xrp,99,22);
		Text(Amiga.xrp,xu->xpru_protocol,strlen(xu->xpru_protocol));
	}
	if(ud&XPRU_FILENAME)
	{
		if(!xu->xpru_filename || xu->xpru_filename[0]==0) return(0);
		if(var.xfertype == 1)
		{
			Move(Amiga.xrp,99,31);
			TextFmt(Amiga.xrp, "%-42s", Amiga.currentnode->path);

			Move(Amiga.xrp,99,40);
			TextFmt(Amiga.xrp, "%-30s", Amiga.currentnode->file);
		} else {
			register BPTR lck = CurrentDir(0);
			NameFromLock(lck, buf, 100);
			CurrentDir(lck);
			//getcwd(buf, 510);
			Move(Amiga.xrp,99,31);
			Text(Amiga.xrp,buf,strlen(buf));

			Move(Amiga.xrp, 99, 40);
			TextFmt(Amiga.xrp, "%-30s", xu->xpru_filename);

			if(!var.term) BGCheck();
			Amiga.uploadnode = AllocMem(sizeof(struct Flag), MEMF_PUBLIC|MEMF_CLEAR);
			strcpy(Amiga.uploadnode->file,xu->xpru_filename);
			Amiga.uploadnode->flag_node.ln_Name = Amiga.uploadnode->file;
			Amiga.uploadnode->size = xu->xpru_filesize;
			Amiga.uploadnode->xfer = 0;
			AddTail((struct List *)Amiga.Uploads, (struct Node *)Amiga.uploadnode);
		}
	}
	if(ud&XPRU_FILESIZE)
	{
		Move(Amiga.xrp, 133, 49);
		TextFmt(Amiga.xrp, "%-10ld", xu->xpru_filesize);
		if(var.xfertype != 1) Amiga.uploadnode->size = xu->xpru_filesize;
	}
	if(ud&XPRU_BYTES)
	{
		Move(Amiga.xrp, 133, 58);
		TextFmt(Amiga.xrp, "%-10ld", xu->xpru_bytes);

		Move(Amiga.xrp, 373, 67);
		TextFmt(Amiga.xrp, "%ld%%  ", (xu->xpru_bytes*100)/xu->xpru_filesize);

		gs = (xu->xpru_bytes*478)/xu->xpru_filesize;
		if(gs > lastgs)
		{
			SetAPen(Amiga.xrp, 12);
			RectFill(Amiga.xrp, 21+lastgs, 91, 21+gs, 100);
			SetAPen(Amiga.xrp, var.white);
			lastgs = gs;
		}
		if(var.xfertype == 1)
			Amiga.currentnode->xfer = xu->xpru_bytes;
		else
			Amiga.uploadnode->xfer = xu->xpru_bytes;

		if(xu->xpru_bytes >= xu->xpru_filesize)
		{
			if(var.xfertype == 1)
				Amiga.currentnode->cps = xu->xpru_datarate;
			else
				Amiga.uploadnode->cps = xu->xpru_datarate;
		}
	}
	if(ud&XPRU_BLOCKCHECK)
	{
		Move(Amiga.xrp,133,67);
		TextFmt(Amiga.xrp,"%-10s",xu->xpru_blockcheck);
	}
	if(ud&XPRU_ERRORS)
	{
		Move(Amiga.xrp,133,76);
		TextFmt(Amiga.xrp,"%-10ld",xu->xpru_errors);
	}
	if(ud&XPRU_MSG)
	{
		Move(Amiga.xrp,133,85);
		TextFmt(Amiga.xrp,"%-40s",xu->xpru_msg);
	}

/* row 2 */

	if(ud&XPRU_ELAPSEDTIME)
	{
		Move(Amiga.xrp,373,49);
		TextFmt(Amiga.xrp,"%-16s",xu->xpru_elapsedtime);
	}
	if(ud&XPRU_EXPECTTIME)
	{
		Move(Amiga.xrp,373,58);
		TextFmt(Amiga.xrp,"%-16s",xu->xpru_expecttime);
	}
	if(ud&XPRU_DATARATE)
	{
		Move(Amiga.xrp, 373, 76);
		TextFmt(Amiga.xrp, "%-10ld", xu->xpru_datarate);

		rc++;
		if(rc == 7)
		{
			rc = 0;
			Structs->McpMsg.data1 = xu->xpru_datarate;
			Structs->McpMsg.data2 = var.xfertype;
			Servermsg(11);
			Move(&Amiga.scr->RastPort, 497, 7);
			Text(&Amiga.scr->RastPort, mytimestr(FALSE), 5);
		}
	}
	return(0);
}

LONG __saveds __asm
xpr_chkabort(void)
{
	struct NodeMessage
	{
		struct Message Msg;
		UWORD command;
		UWORD misc;
		long data1;
		long data2;
		long data3;
		long data4;
	} *msg;
	struct IntuiMessage *im;
	long ret = 0;
	BOOL close = FALSE;
	UWORD command;

	if(var.CON && Amiga.xferwin)
	{
		if(im=(struct IntuiMessage *)GetMsg(Amiga.xferwin->UserPort))
		{
			ReplyMsg(im);
			SerWrite("");
			return(-1);
		}

		while(im=(struct IntuiMessage *)GetMsg(Amiga.cwin->UserPort))
		{
			if(im->Class == IDCMP_CLOSEWINDOW) close = TRUE;
			ReplyMsg((struct Message *)im);
		}
	}

	#define CLOSESCREEN	1
	#define OPENSCREEN	2
	#define SHUTDOWN	3
	#define KICKUSER	4
	#define TOFRONT		8
	#define SHOWFILE	9
	#define SENDFILE	10

	while(msg=(struct NodeMessage *)GetMsg((struct MsgPort *)Amiga.nodemp))
	{
		command = msg->command;
		ReplyMsg(msg);

		switch(command)
		{
		case TOFRONT:
			if(var.CON) ScreenToFront(Amiga.scr);
			break;
		case CLOSESCREEN:
			close = TRUE;
			break;
		case OPENSCREEN:
			if(var.CON)
				ScreenToFront(Amiga.scr);
			else {
				OpenDisplay(FALSE);
				XferWindow();
				lastgs=0;
			}
			break;
		case SHUTDOWN:
			//SerWrite("");
			var.done = TRUE;
			var.menufin = TRUE;
			ret = -1;
			DropDTR(FALSE);
			break;
		case KICKUSER:
			DropDTR(FALSE);
			ret=-1;
			break;
		case SHOWFILE:
		case SENDFILE:
			break;
		}
	}

	if(close)
	{
		CloseWindow(Amiga.xferwin);
		Amiga.xferwin=0;
		CloseDisplay();
	}

	if(Carrier()==FALSE) ret=-1; else
	{
		if(var.xfertype == 1  &&  rc == 6  &&  !var.term) /* if a download */
		{
			Structs->User.Time_Left = var.lastleft - (mytime() - var.loginsecs);
			Structs->User.Time_Used = Structs->User.Time_Limit - Structs->User.Time_Left;
			if(Structs->User.Time_Left < 1)
			{
				SerWrite("");
				Structs->User.Time_Left = 0;
				var.lastleft = 0;
				//DropDTR(FALSE);
				ret=-1;
			}
			UpdateTimeLeft();
		}
	}

	return(ret);
}

LONG __saveds __asm
xpr_squery(void)
{
	Amiga.serwritereq.IOSer.io_Command 	= SDCMD_QUERY;
	DoIO((struct IORequest *)&Amiga.serwritereq);
	return((long)Amiga.serwritereq.IOSer.io_Actual);
}

/* init the xprIO and build everything */
long init_xpr(struct XPR_IO *IO)
{
   IO->xpr_fopen     = xpr_fopen;
   IO->xpr_fclose    = xpr_fclose;
   IO->xpr_fread     = xpr_fread;
   IO->xpr_fwrite    = xpr_fwrite;
   IO->xpr_sread     = xpr_sread;
   IO->xpr_swrite    = xpr_swrite;
   IO->xpr_sflush    = xpr_sflush;
   IO->xpr_update    = xpr_update;
   IO->xpr_chkabort  = xpr_chkabort;
   IO->xpr_ffirst    = xpr_ffirst;
   IO->xpr_fnext     = xpr_fnext;
   IO->xpr_finfo     = xpr_finfo;
   IO->xpr_fseek     = xpr_fseek;
   IO->xpr_gets      = xpr_gets;
   IO->xpr_unlink    = xpr_unlink;
   IO->xpr_squery    = xpr_squery;
   IO->xpr_extension = 1L;
	if(var.CON) XferWindow();
   return(opentimer());
}

void XferWindow(void)
{
	UWORD red;

	if(var.white == 1  ||  var.white == 9) red = 15; else red = 9;

	Amiga.xferwin = OpenWindowTags(NULL,
		WA_Title,		"Transfer in Progress...",
		WA_Left,		60,
		WA_Top,			65,
		WA_Width,		520,
		WA_Height,		108,
		WA_CloseGadget,		TRUE,
		WA_DragBar,		TRUE,
		WA_Activate,		TRUE,
		WA_DepthGadget,		TRUE,
		WA_RMBTrap,		TRUE,
		WA_SizeGadget,		FALSE,
		WA_CustomScreen,	Amiga.scr,
		WA_IDCMP,		IDCMP_CLOSEWINDOW,
		TAG_END);
	Amiga.xrp = Amiga.xferwin->RPort;
	SetAPen(Amiga.xrp, red);
	Move(Amiga.xrp, 20, 22);
	Text(Amiga.xrp, "Protocol:", 9);
	Move(Amiga.xrp, 52, 31);
	Text(Amiga.xrp, "Path:", 5);
	Move(Amiga.xrp, 52, 40);
	Text(Amiga.xrp, "Name:", 5);
	Move(Amiga.xrp, 44, 49);
	Text(Amiga.xrp, "File size:", 10);
	Move(Amiga.xrp, 20, 58);
	Text(Amiga.xrp, "Bytes xfer'd:", 13);
	Move(Amiga.xrp, 36, 67);
	Text(Amiga.xrp, "Block type:", 11);
	Move(Amiga.xrp, 68, 76);
	Text(Amiga.xrp, "Errors:", 7);
	Move(Amiga.xrp, 20, 85);
	Text(Amiga.xrp, "Status/Error:", 13);

	Move(Amiga.xrp, 304, 49);
	Text(Amiga.xrp, "Actual:", 7);
	Move(Amiga.xrp, 288, 58);
	Text(Amiga.xrp, "Expected:", 9);
	Move(Amiga.xrp, 280, 67);
	Text(Amiga.xrp, "%  xfer'd:", 10);
	Move(Amiga.xrp, 280, 76);
	Text(Amiga.xrp, "Chars/sec:", 10);

	SetAPen(Amiga.xrp, var.white);
	Move(Amiga.xrp, 20, 90);
	Draw(Amiga.xrp, 500, 90);
	Move(Amiga.xrp, 500, 90);
	SetAPen(Amiga.xrp, 10);
	Draw(Amiga.xrp, 500, 101);
	Move(Amiga.xrp, 500, 101);
	Draw(Amiga.xrp, 20, 101);
	SetAPen(Amiga.xrp, var.white);
	Move(Amiga.xrp, 20, 101);
	Draw(Amiga.xrp, 20, 90);
	SetAPen(Amiga.xrp, var.white);
}

void CheckFlags(void)
{
	struct Flag *worknode, *nextnode;
	long eff;

	if(var.term) return;

	if (Amiga.Flags->lh_TailPred == (struct Node *)Amiga.Flags) return;

	worknode = (struct Flag *)(Amiga.Flags->lh_Head); /* First node */
	if(!worknode) return;
	while (nextnode = (struct Flag *)(worknode->flag_node.ln_Succ))
	{
		if((worknode->xfer+1) > worknode->size)
		{
			if(!(worknode->free&1<<0))
			{
				Structs->User.Downloads++;
				Structs->User.Bytes_Download = Structs->User.Bytes_Download + worknode->size;
				Structs->User.Daily_Bytes_Dld = Structs->User.Daily_Bytes_Dld + worknode->size;
			}
			if(!(worknode->free&1<<2)  &&  worknode->size!=0)
			{
				eff = (Amiga.currentnode->cps*100)/(var.baud/10);
				mysprintf(buf, "	Downloading %s %ld bytes\n	 %ld files, %ldk bytes, 1 minutes 10 seconds %ld cps, %ld %% efficiency at %ld\n", worknode->file, worknode->size, 1, worknode->size/1024, worknode->cps, eff, var.baud);
				CallerLog(buf);
			}
			var.fileflags--;
			var.byteflags = var.byteflags-worknode->size;
			Remove(worknode);
			FreeMem(worknode, sizeof(struct Flag));
		}
		worknode = nextnode;
	}
	SaveAccount();
}

void CheckUploads(void)
{
	char lastfile[32];
	struct Flag *worknode, *nextnode;
	ULONG bytes = 0;
	UWORD files = 0, i = 0;
	BPTR fh;

	if(!var.term) PS("[0m\r\n");

	if (Amiga.Uploads->lh_TailPred == (struct Node *)Amiga.Uploads) return;

	worknode = (struct Flag *)(Amiga.Uploads->lh_Head); /* First node */
	if(!worknode) return;

	if(var.xfertype != 3)
	{
		if(FindPort("SXBGCheck"))
		{
			PS(Structs->SXStr[129]);
			while(FindPort("SXBGCheck"))
			{
				if(i == 120) break;
				i++;
				Delay(25);
			}
		}

		if(var.CARRIER)
		{
			mysprintf(buf, "SX:Node%ld/BG.report", var.node);
			fh = Lock(buf, SHARED_LOCK);
			if(fh)
			{
				UnLock(fh);
				PS(Structs->SXStr[130]);
				ShowAnsi(buf, 0, 0, 0, 0, 1, 0);
				DeleteFile(buf);
			}
		}
	}

	lastfile[0] = 0;

	while (nextnode = (struct Flag *)(worknode->flag_node.ln_Succ))
	{
		if(strcmp(worknode->file, lastfile) == 0) // DEBUG - HOPEFULLY STOPS INFINATE LOOP
		{
			PS("\r\nAn extreme error has occured.\r\n");
			FreeMem(Amiga.Uploads, sizeof(struct List));
			Amiga.Uploads = AllocMem(sizeof(struct List), MEMF_PUBLIC|MEMF_CLEAR);
			NewList(Amiga.Uploads);
			Amiga.uploadnode = 0;
			return;
		}

		if(var.xfertype != 3)
		{
			if(worknode->size > 0)
			{
				if(worknode->xfer >= worknode->size)
				{
					Structs->McpMsg.data1 = (long) worknode->file;
					Structs->McpMsg.data2 = (long) worknode->size;
					Structs->McpMsg.data3 = (long) worknode->cps;
					Structs->McpMsg.data4 = (long) worknode->conf;
					Structs->McpMsg.data5 = (long) worknode->area;
					Structs->McpMsg.action = (char *) mytime();
					Servermsg(16);

					FileCheckDoor(worknode->file);

					AddFile(worknode->file, worknode->cps, worknode->size);

					if(var.CARRIER)
					{
						mysprintf(buf, "%s%ld/%s.BAD", Structs->Cfg->USERPath, Structs->User.Slot_Number, worknode->file);
						fh = Lock(buf, SHARED_LOCK);
						if(fh)
						{
							UnLock(fh);
							DeleteFile(buf);
						}
					}

					files++;
					bytes+=worknode->size;
				} else {
					StoreResume(worknode->file);
					if(var.CARRIER) PSFmt(Structs->SXStr[135], worknode->file, worknode->size, worknode->xfer);
				}
			} else
				DeleteFile(worknode->file);

		} else {
			if(worknode -> xfer  <  worknode -> size)
				SetComment(worknode -> file, "N");
		}
		strcpy(lastfile, worknode->file);
		//Remove(worknode);
		FreeMem(worknode, sizeof(struct Flag));
		worknode = nextnode;
	}

	if(!var.term)
	{
		SaveAccount();
		if(bytes == 0) files = 0;
		if(var.xfertype != 3  &&  var.CARRIER  &&  files) PSFmt(Structs->SXStr[74], files, bytes);
	}

	Amiga.uploadnode = 0;
	FreeMem(Amiga.Uploads, sizeof(struct List));
	Amiga.Uploads = AllocMem(sizeof(struct List), MEMF_PUBLIC|MEMF_CLEAR);
	NewList(Amiga.Uploads);
}

BOOL SendFile(char *fname)
{
	FlagFile(fname, TRUE, TRUE, TRUE);

	RawTransfer(1, TRUE, FALSE, Structs->NodePrefs.fcheck, fname);

	if(FindFlag(FilePart(fname)))
	{
		UnFlagFile(FilePart(fname));
		return(FALSE);
	}
	return(TRUE);
}

void RawTransfer(UWORD xtype, BOOL isend, BOOL freeze, char *recpath, char *fname)
{
	long preuploadtime;

	if(var.LOCAL)
	/*{
		if(xtype == 1)
		{
		}*/
		return;
	/*}*/

	var.xfertype = xtype;
	var.instsend = isend;

	switch(Structs->Proto.type)
	{
	case 0:
		if(ProtoStart())
		{
			BPTR old, lck;

			if(freeze) preuploadtime = mytime();
			xio.xpr_filename = fname;
			switch(var.xfertype)
			{
			case 1:
				XProtocolSend(&xio);
				break;
			case 2:
			case 3:
				lck = Lock(recpath, SHARED_LOCK);
				if(lck)
				{
					old = CurrentDir(lck);
					//chdir(recpath);
					XProtocolReceive(&xio);
					CurrentDir(old);
					UnLock(lck);
				}
				break;
			}
			if(freeze) var.lastleft = var.lastleft + (mytime() - preuploadtime) + 15;
			ProtoClean();
		}
		break;
	case 1:
		SModem(recpath);
		break;
	case 2:
		HydraCom(recpath);
		break;
	}

	var.instsend = FALSE;
}
