
#include <stdlib.h>
#include <string.h>
#include <exec/memory.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <intuition/intuition.h>
#include <devices/serial.h>
#include "SXStructs.h"
#include "sx.h"

extern struct StructsStruct	*Structs;
extern struct AmigaStruct	Amiga;
extern struct StringsStruct	*stg;
extern struct VariablesStruct	var;

extern UBYTE buf[512], charbuf[128];

void Servermsg(int command);
void HandleNodeMsg(void);


void SetActive(int id, char * a)
{
	var.action=id;
	if(id == -1)
		strcpy(stg->action, a);
	else {
		switch(id)
		{
     		case 0: strcpy(stg->action, "Main Menu"); break;
     		case 1: strcpy(stg->action, "Downloading"); break;
     		case 2: strcpy(stg->action, "Uploading"); break;
     		case 3: strcpy(stg->action, "External Door"); break;
     		case 4: strcpy(stg->action, "Reading Mail"); break;
     		case 5: strcpy(stg->action, "Reviewing Stats"); break;
     		case 6: strcpy(stg->action, "Account Editing"); break;
     		case 7: strcpy(stg->action, "Zooming Mail"); break;
     		case 8: strcpy(stg->action, "Listing Files"); break;
     		case 9: strcpy(stg->action, "Reading Bulletin"); break;
     		case 10:strcpy(stg->action, "Viewing a file"); break;
     		case 11:strcpy(stg->action, "New User"); break;
     		case 12:strcpy(stg->action, "Logging off"); break;
     		case 13:strcpy(stg->action, "Sysop Commands"); break;
     		case 14:strcpy(stg->action, "Dropped to Shell"); break;
     		//case 15:strcpy(stg->action, "Using Emacs"); break;
     		case 16:strcpy(stg->action, "Joining a Conf."); break;
     		case 17:strcpy(stg->action, "Chatting"); break;
     		case 18:strcpy(stg->action, "Reseting Node"); break;
     		case 19:strcpy(stg->action, "Paging SysOp"); break;
     		case 20:strcpy(stg->action, "Connecting"); break;
     		case 21:strcpy(stg->action, "Logging on"); break;
     		case 22:strcpy(stg->action, "Waiting For Call"); break;
     		case 23:strcpy(stg->action, "Scanning Mail"); break;
     		case 24:strcpy(stg->action, "Node Inactive"); break;
     		case 25:strcpy(stg->action, "MultiNode Chat"); break;
     		case 26:strcpy(stg->action, "BBS Suspended"); break;
     		case 27:strcpy(stg->action, "Reserve for User"); break;
		case 28:strcpy(stg->action, "Writing Mail"); break;
		case 101:strcpy(stg->action, "System Password"); break;
		case 102:strcpy(stg->action, "S-Modem Transfer"); break;
		case 103:strcpy(stg->action, "HydraCom Transfer"); break;
		case 104:strcpy(stg->action, "DayDream Door"); break;
		case 105:strcpy(stg->action, "Paragon Door"); break;
		case 106:strcpy(stg->action, "AmiExpress Door"); break;
		case 107:strcpy(stg->action, "Auto LogOff"); break;
		case 108:strcpy(stg->action, "Edit Flag List"); break;
		case 109:strcpy(stg->action, "New User Password"); break;
		case 110:strcpy(stg->action, "FAME Door"); break;
		case 111:strcpy(stg->action, "Terminal Mode"); break;
		}
	}
	/*if(var.CON) {
		mysprintf(buf,"%20s",stg->action);
		SetAPen(Amiga.rp,15);
		Move(Amiga.rp,450,17);
		Text(Amiga.rp,buf,20);
	}*/
	Structs->McpMsg.action = stg->action;
	Servermsg(5);
}


void Servermsg(int command)
{
	Structs->McpMsg.Msg.mn_Length = sizeof(struct MCPMessage);
	Structs->McpMsg.nodenum = var.node;
	Structs->McpMsg.command = command;
	switch(command)
	{
	case 3:
		Structs->McpMsg.data1 = var.baud;
		Structs->McpMsg.data2 = (long)(APTR)&Structs->User;
		Structs->McpMsg.data3 = (long)(APTR)&Structs->UserIndex;
		Structs->McpMsg.data4 = (long)(APTR)&Structs->SXUser;
		Structs->McpMsg.data5 = var.loginsecs;
		break;
	case 5:
		Structs->McpMsg.data1 = var.action;
		break;
	case 8:
		Structs->McpMsg.data1 = (long)&charbuf;
		Structs->McpMsg.data2 = (long)&buf;
		break;
	}
	//if(act && act[0]!=0) Structs->McpMsg.action = act;
	PutMsg(Amiga.serverport, (struct Message *)&Structs->McpMsg);
	//if(command!=11)
	//{
		WaitPort(Structs->McpMsg.Msg.mn_ReplyPort);
		GetMsg(Structs->McpMsg.Msg.mn_ReplyPort);
	//}
	if(command==1)
	{
		Structs->Cfg = (struct MainStruct *)Structs->McpMsg.data1;
		Amiga.sxbbsImage = (struct Image *)Structs->McpMsg.data2;
		Amiga.inpImage = (struct Image *)Structs->McpMsg.data3;
		Amiga.outpImage = (struct Image *)Structs->McpMsg.data4;
	}
}


void HandleNodeMsg(void)
{
	#define CLOSESCREEN	1
	#define OPENSCREEN	2
	#define SHUTDOWN	3
	#define KICKUSER	4
	#define CHAT		5
	#define SERIALQUERY	6
	#define LOADCONFIG	7
	#define TOFRONT		8
	#define SHOWFILE	9
	#define SENDFILE	10
	#define SHOWSTRING	11

	BOOL front=FALSE;
	struct NodeMessage
	{
		struct Message Msg;
		UWORD command;
		UWORD misc;
		long data1;
		long data2;
		long data3;
		long data4;
	};
	struct NodeMessage *msg;
	while(msg=(struct NodeMessage *)GetMsg((struct MsgPort *)Amiga.nodemp))
	{
		switch(msg->command)
		{
		case TOFRONT:
			ReplyMsg(msg);
			front = TRUE;
			break;
		case CLOSESCREEN:
			ReplyMsg(msg);
			CloseDisplay();
			break;
		case OPENSCREEN:
			ReplyMsg(msg);
			if(var.CON) front=TRUE; else
			{
				OpenDisplay(TRUE);
				if(!var.CALLER) WaitScreen();
			}
			break;
		case SHUTDOWN:
			ReplyMsg(msg);
			var.done = TRUE;
			var.menufin = TRUE;
		case KICKUSER:
			ReplyMsg(msg);
			DropDTR(TRUE);
			break;
		case CHAT:
			ReplyMsg(msg);
			if(var.CALLER) { OpenDisplay(TRUE); Chat(); }
			break;
		case SHOWFILE:
			strcpy(buf, (char *)msg->data1);
			ShowAnsi(buf, 0, 0, 0, 0, 1, 0);
			ReplyMsg(msg);
			break;
		case SENDFILE:
			strcpy(buf, (char *)msg->data1);
			if(var.USER) SendFile(buf);
			ReplyMsg(msg);
			break;
		case SHOWSTRING:
			PS((char *)msg->data1);
 			ReplyMsg(msg);
			break;
	/*	case SERIALQUERY:
			if(var.SER) {
				serwritereq.IOSer.io_Command	= SDCMD_QUERY;
				DoIO(&serwritereq);
				serwritereq.IOSer.io_Command 	= CMD_WRITE;
				msg->data1=serwritereq.io_Status;
			} else {
				msg->data1=0;
			}
			ReplyMsg(msg);
			break;
		case LOADCONFIG:
			ReplyMsg(msg);
			FreePrefs();
			LoadPrefs();
			LoadConfs();
			LoadArcs();
			break;*/
		}
	}
	if(front && var.CON) ScreenToFront(Amiga.scr);
}
