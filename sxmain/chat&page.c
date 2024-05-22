
#include <stdlib.h>
#include <string.h>
#include <proto/exec.h>
#include <proto/intuition.h>
#include <exec/ports.h>
#include <devices/serial.h>
#include <dos/dos.h>
#include "SXStructs.h"
#include "SX.h"

extern struct StructsStruct	*Structs;
extern struct AmigaStruct	Amiga;
extern struct StringsStruct	*stg;
extern struct VariablesStruct	var;

extern UBYTE buf[512], charbuf[128];

void ActivateWindow( struct Window *window );


void PageSysOp(void)
{
	BOOL pagefin=FALSE;
	UBYTE j=0;
	char reason[42];

	var.chat=FALSE;
	var.justchatted=FALSE;
	SetActive(19, 0);
	Servermsg(10);
	if(Structs->McpMsg.data1!=0)
	{
		PS(Structs->SXStr[85]);
		PS((char *)Structs->McpMsg.data1);
		PS(Structs->SXStr[86]);
		return;
	}
	PS(Structs->SXStr[16]);
	LineInput(40,0,0);
	if(!var.CARRIER) return;
	if(charbuf[0]==0) { PS("\r\n"); return; }
	strcpy(reason, charbuf);
	PS(Structs->SXStr[20]);

	while(!pagefin)
	{
		j++;
		var.TIME_OUT=1;
		MyHotKey();
		if(!var.CARRIER)
		{
			var.TIME_OUT=0;
			var.TIMEOUTFLAG=FALSE;
			return;
		}
		if(var.justchatted!=TRUE)
		{
			if(charbuf[0]=='')
			{
				PS(Structs->SXStr[22]);	/*PS("Aborted\r\n");*/
				pagefin = TRUE;
				if(!var.paged)
				{
					Structs->McpMsg.data1 = 1;
					Servermsg(9);
					var.paged = TRUE;
				}
				mysprintf(buf, "	Operator Paged (%s) (%s)\n", mytimestr(TRUE), reason);
				CallerLog(buf);
			} else {
				if(var.TIMEOUTFLAG==TRUE)PS(Structs->SXStr[21]);
				if(!var.CARRIER) pagefin=TRUE;
				if(j==20)
				{
					pagefin = TRUE;
					if(!var.paged)
					{
						var.paged = TRUE;
						Structs->McpMsg.data1 = 1;
						Servermsg(9);
					}
					PS(Structs->SXStr[28]);
					mysprintf(buf, "	Operator Paged (%s) (%s)\n", mytimestr(TRUE), reason);
					CallerLog(buf);
				}
			}
		} else {
			pagefin = TRUE;
			if(var.paged)
			{
				var.paged = FALSE;
				Structs->McpMsg.data1 = 0;
				Servermsg(9);
			}
		}
	}
	var.TIME_OUT=0;
	var.TIMEOUTFLAG=FALSE;
	UpdateUserInfo();
}

void Chat(void)
{
	BOOL finchat=FALSE;
	WORD sysop=2;

	if(var.chat) return;

	var.chat = TRUE;
	var.TIME_OUT = 0;
	var.TIMEOUTFLAG = FALSE;
	var.justchatted = TRUE;
	SetActive(17, 0);
	ShowAnsi("EnterChat", 1, 1, 1, 1, 0, 0);
	if(var.CON)ActivateWindow(Amiga.win);
	if(var.paged)
	{
		var.paged = FALSE;
		Structs->McpMsg.data1 = 0;
		Servermsg(9);
	}
	UpdateUserInfo();
	if(Structs->Doors.chatdoor[0]!=0)
	{
		RunDoor(Structs->Doors.chatdoor, Structs->Doors.chattype, 0);
		var.chat=FALSE;
		ShowAnsi("ExitChat", 1, 1, 1, 1, 0, 0);
		return;
	}
	while(!finchat && var.CARRIER && var.CON)
	{
		MyHotKey();
		if(var.origin==1)
		{
			if(sysop!=1 && !(Structs->Disp.flags&(1<<2))) PS("[35m");
			sysop=1;
			if(charbuf[0]==27)finchat=TRUE;
		} else {
			if(sysop!=0 && !(Structs->Disp.flags&(1<<2))) PS("[36m");
			sysop=0;
		}
		switch(charbuf[0])
		{
		case 8:
			strcpy(charbuf,"\b \b");
			break;
		case 10:
		case 13:
			charbuf[0]=13;
			charbuf[1]=10;
			charbuf[2]=0;
			break;
		}
		PS(charbuf);
	}
	var.chat = FALSE;
	ShowAnsi("ExitChat", 1, 1, 1, 1, 0, 0);
}
