
#include <stdlib.h>
#include <string.h>
#include <exec/ports.h>
#include <proto/exec.h>
#include <devices/serial.h>
#include <sx/SXStructs.h>
#include "sx.h"

extern struct StructsStruct	*Structs;
extern struct AmigaStruct	Amiga;
extern struct StringsStruct	*stg;
extern struct VariablesStruct	var;

extern UBYTE buf[512], charbuf[128];

struct node_struct
{					/* offset DEC	*/
	APTR next;			/*	0	*/
	struct UserData *User;		/*	4	*/
	struct UserIndexStruct *UserIndex;/*	8	*/
	struct SXUserStruct *SXUser;	/*	12	*/
	char *action;			/*	16	*/
	char *filename;			/*	20	*/
	long baud;			/*	24	*/
	long loginsecs;			/*	28	*/
	UWORD number;			/*	32	*/
	UBYTE actionnumber;		/*	34	*/
	UBYTE active;			/*	35	*/
	UBYTE useron;			/*	36	*/
	UBYTE misc;			/*	37	*/
};


BOOL CheckMulti(void)
{
	struct node_struct *node;

	Servermsg(14);
	node = (struct node_struct *)Structs->McpMsg.data1;
	while(1)
	{
		if(node->number != var.node && node->useron && node->User->Slot_Number == Structs->User.Slot_Number) return(TRUE);

		if(!node->next) return(FALSE); else node = node->next;
	}
}

void Who(void)
{
	struct node_struct *node;
	char rate[16];
	BOOL whofin=FALSE;
	UWORD i=0;

	PS(Structs->SXStr[82]);
	Servermsg(14);
	node = (struct node_struct *)Structs->McpMsg.data1;
	while(!whofin)
	{
		i++;
		if(node->baud==-1) strcpy(rate, "TELNET"); else mysprintf(rate, "%ld", node->baud);
		if(node->useron)
			PSFmt(Structs->SXStr[83], i, node->User->Name, node->User->Location, node->action, rate);
		else
			PSFmt(Structs->SXStr[83], i, "", "", node->action, "");

		if(!node->next) whofin=TRUE; else node = node->next;
	}
	PS(Structs->SXStr[84]);
}

void NodeMsg(WORD msgnode, char *fname, char *string)
{
	struct MsgPort *nodeport;
	struct NodeMessage
	{
		struct Message Msg;
		UWORD command;
		UWORD misc;
		long data1;
		long data2;
		long data3;
		long data4;
	} nodemsg;
	BOOL file;

	if(string) file = FALSE; else file = TRUE;

	if(msgnode == var.node) return;

	if(msgnode == 0)
	{
		if(stg->arg[0])
		{
			msgnode = atoi(stg->arg);
			if(msgnode == var.node) return;
			if(msgnode > 0 && msgnode < 256) goto cont;
		}

		file = FALSE;
ask:
		PS(Structs->SXStr[113]);
		LineInput(8, 0, 0);
		PS("\r\n");
		if(!var.CARRIER || charbuf[0]==0) return;
		if(stricmp(charbuf, "V")==0)
		{
			Who();
			goto ask;
		}
		msgnode = atoi(charbuf);
		if(msgnode == var.node) return;
cont:
		PS(Structs->SXStr[114]);
		LineInput(78, 0, 0);
		PS("\r\n");
		if(!var.CARRIER || charbuf[0]==0) return;
		mysprintf(buf, Structs->SXStr[115], Structs->User.Name, var.node, charbuf);
	}

	mysprintf(charbuf, "SX-Node%ld", msgnode);
	if(nodeport = FindPort(charbuf))
	{
		if(nodemsg.Msg.mn_ReplyPort = CreateMsgPort())
		{
			nodemsg.Msg.mn_Length = sizeof(struct NodeMessage);
			if(file)
			{
				nodemsg.command = 9;
				nodemsg.data1 = (long) fname;
			} else {
				nodemsg.command = 11;
				nodemsg.data1 = (long) &buf;
			}
			PutMsg(nodeport, (struct Message *)&nodemsg);
			WaitPort(nodemsg.Msg.mn_ReplyPort);
			GetMsg(nodemsg.Msg.mn_ReplyPort);
			DeleteMsgPort(nodemsg.Msg.mn_ReplyPort);
		}
	}
}
