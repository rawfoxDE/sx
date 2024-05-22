/*

Remote Shell (FIFO) routines

*/

#include <stdlib.h>
#include <string.h>
#include <exec/memory.h>
#include <exec/io.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
#include <proto/diskfont.h>
#include <proto/reqtools.h>
#include <proto/gadtools.h>
#include <intuition/intuition.h>
#include <devices/serial.h>
#include <devices/timer.h>
#include <fifo.h>
#include <fifopr.h>
#include "SXStructs.h"
#include "sx.h"

extern struct StructsStruct	*Structs;
extern struct AmigaStruct	Amiga;
//extern struct StringsStruct	*stg;
extern struct VariablesStruct	var;

extern UBYTE buf[512], charbuf[128];


void WaitMsg(struct Message *msg);
void CheckInsert(char *old, char *new);


void FifoExecute(char * exe, char *exearg)
{
	char commandline[80], FifoSlav[16], FifoMast[16], *ptr, newexe[164];
	struct Library *FifoBase;
	struct Message RMsg,*msg;
	struct MsgPort *IoSink;
	void *FifoR, *FifoW;
	LONG pmask, FifrIP=1, n;
	ULONG sign, nodesig=1L<<Amiga.nodemp->mp_SigBit;
	BOOL fifodone=FALSE;
	BPTR fh;

	mysprintf(FifoSlav, "SX%ldfifo_s", var.node);
	mysprintf(FifoMast, "SX%ldfifo_m", var.node);

	mysprintf(commandline, "NewShell FIFO:SX%ldfifo/rwkecs", var.node);

	if(exe && exe[0]!=0)
	{
		if(strstr(exe, "%")) CheckInsert(exe, newexe); else strcpy(newexe, exe);
		mysprintf(buf, "t:sxshell%ld.tmp", var.node);
		strcat(commandline, " FROM ");
		strcat(commandline, buf);
		if(fh = Open(buf, MODE_NEWFILE))
		{
			if(exearg && exearg[0]!=0)
				mysprintf(buf, "FailAt 100000\n%s %s\nEndCLI\n", newexe, exearg);
			else
				mysprintf(buf, "FailAt 100000\n%s\nEndCLI\n", newexe);

			Write(fh, buf, strlen(buf));
			Close(fh);
		}
	} else {
		SetActive(14, 0);
		strcat(commandline, " FROM S:SXShell-Startup");
		ShowAnsi("Shell", 1, 1, 1, 1, 1, 0);
	}
	FifoBase = OpenLibrary(FIFONAME, 0);
	if(!FifoBase) return;

	SystemTags(commandline, TAG_DONE);
	//system(commandline);

	FifoW = OpenFifo(FifoMast, 1024, FIFOF_WRITE | FIFOF_NORMAL | FIFOF_NBIO);
	if(FifoW == NULL)
	{
		CloseLibrary(FifoBase);
		return;
	}
	FifoR = OpenFifo(FifoSlav, 1024, FIFOF_READ  | FIFOF_NORMAL | FIFOF_NBIO);
	if (FifoR == NULL)
	{
		if(FifoW) CloseFifo(FifoW, FIFOF_EOF);
		CloseLibrary(FifoBase);
		return;
	}
	IoSink = CreatePort(NULL, 0);
	RMsg.mn_ReplyPort = IoSink;
	pmask = 1 << IoSink->mp_SigBit;
	RequestFifo(FifoR, &RMsg, FREQ_RPEND);
	while(!fifodone)
	{
		SetTimer(2, 0);
		if(var.CON) {
			sign = Wait(var.timsig | pmask | var.infowinsig | var.cwinsig | var.winsig | var.consig | var.sersig | nodesig);
			if(sign & var.cwinsig)CheckWin(Amiga.cwin);
			if(var.CON) { if(sign & var.winsig)CheckWin(Amiga.win); }
			if(var.CON) { if(sign & var.infowinsig)CheckWin(Amiga.infowin); }
			//if(sign & var.consig)
			//{
				while (msg = GetMsg(Amiga.ReadConPort))
				{
					if(msg == (struct Message *)&Amiga.conreadreq)
					{
						if (Amiga.conreadreq.io_Actual > 0) WriteFifo(FifoW,Amiga.conreadreq.io_Data,1);
					}
					SendIO(&Amiga.conreadreq);
				}
			//}
		} else {
			sign = Wait(var.timsig | pmask | var.sersig | nodesig | SIGBREAKF_CTRL_F);
			if(sign & SIGBREAKF_CTRL_F) OpenDisplay(TRUE);
		}
		if(sign & var.timsig)
		{
			if(var.CON) {
				Move(&Amiga.scr->RastPort, 497, 7);
				Text(&Amiga.scr->RastPort, mytimestr(FALSE), 5);
			}
			if(var.USER) {
				CalcTimeLeft();
				UpdateTimeLeft();
			}
		} else {
			AbortIO(Amiga.TimerMsg);
			WaitIO(Amiga.TimerMsg);
		}
		if(var.SER)
		{	
			if(sign & var.sersig)
			{
				while (msg = GetMsg(Amiga.ReadSerPort))
				{
					if(msg == (struct Message *)&Amiga.serreadreq)
					{
						if(Amiga.serreadreq.IOSer.io_Actual > 0) WriteFifo(FifoW,Amiga.serreadreq.IOSer.io_Data,1);
					}
					Amiga.serreadreq.IOSer.io_Length = 1;
					SendIO(&Amiga.serreadreq);
				}
			}
		}
		if(sign & nodesig) HandleNodeMsg();
		if(sign & pmask)
		{
			while(msg = (struct Message *)GetMsg(IoSink))
			{
				if(msg == (struct Message *)&RMsg)
				{
					FifrIP = 0;
					if((n = ReadFifo(FifoR, &ptr,0)) > 0)
					{
						if(n > 256) n = 256;
						PSLen(ptr,n);
						n = ReadFifo(FifoR, &ptr, n);
					}
					if(n<0) fifodone=TRUE;
					else {
						RequestFifo(FifoR, &RMsg, FREQ_RPEND);
						FifrIP = 1;
					}
				}
			}
		}
		if(Carrier()==FALSE) fifodone=TRUE;
	}
    	if (FifrIP) {
		RequestFifo(FifoR, &RMsg, FREQ_ABORT);
		WaitMsg(&RMsg);
	}
	if(FifoR) CloseFifo(FifoR, FIFOF_EOF);
	if(FifoW) CloseFifo(FifoW, FIFOF_EOF);
	CloseLibrary(FifoBase);
	if(exe && exe[0]!=0)
	{
		mysprintf(buf,"t:sxshell%ld.tmp",var.node);
		DeleteFile(buf);
	} else
		ShowAnsi("PostShell", 1, 1, 1, 1, 1, 0);

	if(IoSink) DeletePort(IoSink);
}

void WaitMsg(struct Message *msg)
{
    while (msg->mn_Node.ln_Type == NT_MESSAGE)
	Wait(1 << msg->mn_ReplyPort->mp_SigBit);
    Forbid();
    Remove(&msg->mn_Node);
    Permit();
}

void CheckInsert(char *old, char *new)
{
	UWORD len = strlen(old), i = 0, j = 0;
	char tbuf[164];

	while(i < len)
	{
		if(old[i]=='%')
		{
			i++;
			tbuf[0] = 0;
			switch(old[i])
			{
			case 'l':
				mysprintf(tbuf, "%ld", Structs->User.LineLength);
				break;
			case 'f':
				strcpy(tbuf, charbuf);
				break;
			case 'h':
				strcpy(tbuf, Structs->UserIndex.handle);
				break;
			case 'n':
				strcpy(tbuf, Structs->UserIndex.realname);
				break;
			case 's':
				mysprintf(tbuf, "%ld", Structs->User.Slot_Number);
				break;
			case 't':
				mysprintf(tbuf, "%ld", Structs->User.Time_Left / 60);
				break;
			case 'a':
				mysprintf(tbuf, "%ld", Structs->User.Sec_Status);
				break;
			}
			if(tbuf[0] != 0)
			{
				new[j] = 0;
				strcat(new, tbuf);
				j = strlen(new);
			}
		} else {
			new[j] = old[i];
			j++;
			if(j==163) { new[j] = 0; return; }
		}
		i++;
	}
	new[j] = 0;
}

/*
	%l	screen length
	%f	filename to edit (in msgs)
	%s	user slot number
	%h      user handle/alias
	%n      user realname
	%t	timeleft mins
	%a	access level
*/
