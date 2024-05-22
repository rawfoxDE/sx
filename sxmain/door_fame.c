/*
 *   =========================================
 *      System-X BBS  -  FAME Door Routines
 *   =========================================
 */

#include <stdlib.h>
#include <string.h>
#include <exec/memory.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <devices/serial.h>
#include "SXStructs.h"
#include "SX.h"

LONG PutStr( STRPTR str );

extern struct StructsStruct	*Structs;
//extern struct AmigaStruct	Amiga;
extern struct StringsStruct	*stg;
extern struct VariablesStruct	var;
extern struct TagItem		tags[];

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

#include "FAMEDoorCommands.h"

void WaitForFAME(char *door)
{
	struct  FAMEDoorMsg
	{
		struct  Message	fdom_Msg;           /* MessagePort Structure (System)    */
		char    fdom_IOString[202];         /* In/OutPut String                  */
		STRPTR  fdom_StringPtr;             /* String pointer                    */
		long    fdom_Command,               /* Command Node<->Door               */
		        fdom_Data1,                 /* Data 1 to transfer data           */
		        fdom_Data2;                 /* Data 2 to transfer data           */
		ULONG   fdom_Data3;                 /* Data 3 to transfer data           */
		long    fdom_ReturnCode,            /* Returncode                        */
		        fdom_Node;                  /* NodeNumber                        */
		ULONG   fdom_InternalBits;          /* FAME internal data bit flags      */
		APTR    fdom_StructDummy1,          /* APTR 1 for SystemStructs          */
		        fdom_StructDummy2,          /* APTR 2 for SystemStructs          */
		        fdom_StructDummy3;          /* APTR 3 for SystemStructs          */
		STRPTR  fdom_StringPtr2;            /* A second string pointer           */
		ULONG   fdom_Data4,                 /* Data 4 to transfer data (future)  */
		        fdom_BitFlags;              /* A bit mask to give the door infos */
		struct  MsgPort *fdom_ExternalPort; /* External AbortIO port             */
	} *famemsg;
	UWORD doors = 0;
	struct MsgPort *mp;
	char portname[18];
	//long x;

	SetActive(110, 0);

	mysprintf(portname, "FAMEDoorPort%ld", var.node);
	mp = CreatePort(portname, 0L);
	if(!mp) return;

	SystemTagList(door, tags);

	var.RAWARROW = FALSE;

	while(1)
	{
		WaitPort((struct MsgPort *)mp);
		while(famemsg = (struct FAMEDoorMsg *)GetMsg((struct MsgPort *)mp))
		{
			/*mysprintf(buf, "FAME: %ld  DATA1: %ld\n", famemsg->fdom_Command, famemsg->fdom_Data1);
			PutStr(buf);*/
			switch(famemsg -> fdom_Command)
			{
			case MC_DoorStart:
				doors++;
				break;

			case MC_ShutDownLastWords:
				SafePS(famemsg -> fdom_IOString);

			case MC_ShutDown:
				doors--;
				if(doors < 1)
				{
					ReplyMsg((struct Message *)famemsg);
					DeletePort((struct MsgPort *)mp);
					var.RAWARROW = TRUE;
					return;
				}
				break;

			case NR_SendStr:
				SafePS(famemsg -> fdom_IOString);
				break;

			case NR_SendStrCRLF:
				SafePS(famemsg -> fdom_IOString);
				PSLen("\r\n", 2);
				break;

			case NR_SendStrCon:
				Local(famemsg -> fdom_IOString);
				break;

			case NR_SendStrSer:
				SerWrite(famemsg -> fdom_IOString);
				break;

			case NR_GetEditString:
			case NR_PromptChars:
				SafePS(famemsg -> fdom_IOString);
				LineInput(famemsg -> fdom_Data1, famemsg -> fdom_Data2&(1<<4), NULL);
				strcpy(famemsg -> fdom_IOString, charbuf);
				break;

			case NR_HotKey:
				CheckKey();
				famemsg -> fdom_Data2 = charbuf[0];
				famemsg -> fdom_Data3 = var.origin - 1;
				break;

			case NR_BBSName:
				strcpy(famemsg -> fdom_IOString, Structs->Cfg -> BBSName);
				break;

			case NR_SysOp:
				strcpy(famemsg -> fdom_IOString, Structs->Cfg->Sysop);
				break;

			case NR_MainLine:
                		if(stg->arg[0]==0)
					strcpy(famemsg -> fdom_IOString, stg->cmd);
				else
					mysprintf(famemsg -> fdom_IOString, "%s %s", stg->cmd, stg->arg);
				break;

			case NR_NodeID:
				famemsg -> fdom_Data2 = var.node;
				break;

			case NR_GetConfNum:
				famemsg -> fdom_Data2 = var.thisconf;
				break;

			case NR_StampTime:
				strcpy(famemsg -> fdom_IOString, mytimestr(TRUE));
				break;

			case NR_CurrTime:
				famemsg -> fdom_Data2 = mytime();
				break;

			case NR_GetFullArg:
                		strcpy(famemsg -> fdom_IOString, stg->arg);
				break;

			case NR_ThisConfAccess:
				famemsg -> fdom_Data2 = CheckConfAccess(&Structs->SXUser, famemsg -> fdom_Data1);
				break;

			case NR_Name:
				strcpy(famemsg -> fdom_IOString, Structs->User.Name);
				break;

			case NR_Location:
			case NR_From:
				strcpy(famemsg -> fdom_IOString, Structs->User.Location);
				break;

			case NR_PhoneNumber:
				strcpy(famemsg -> fdom_IOString, Structs->User.PhoneNumber);
				break;

			case NR_SlotNumber:
				famemsg -> fdom_Data2 = Structs->User.Slot_Number;
				break;

			case NR_AccessLevel:
				famemsg -> fdom_Data2 = Structs->User.Sec_Status;
				break;

			case NR_CompType:
				famemsg -> fdom_Data2 = 1;
				strcpy(famemsg -> fdom_IOString, Structs->SXUser.computer);
				break;

			case NR_MessagePosted:
				famemsg -> fdom_Data3 = Structs->User.Messages_Posted;
				break;

			case NR_NoCalls:
				famemsg -> fdom_Data3 = Structs->User.Times_Called;
				break;

			case NR_TimeLastOn:
				famemsg -> fdom_Data2 = Structs->User.Time_Last_On;
				break;

			case NR_TimeUsed:
				famemsg -> fdom_Data2 = Structs->User.Time_Used;
				break;

			case NR_TimeLimit:
				famemsg -> fdom_Data2 = Structs->User.Time_Limit;
				break;

			case NR_TimeRemain:
				famemsg -> fdom_Data2 = Structs->User.Time_Left;
				break;

			case NR_XferProt:
				famemsg -> fdom_Data2 = Structs->User.Protocol;
				strcpy(famemsg -> fdom_IOString, Structs->Proto.name);
				break;

			case NR_NumLines:
				famemsg -> fdom_Data2 = Structs->User.LineLength;
				break;

			case NR_SentBy:
				strcpy(famemsg -> fdom_IOString, Structs->SXUser.sentbyline);
				break;

			case NR_WaitChar:
			case AR_WaitRAWChar:
				MyHotKey();
				famemsg -> fdom_Data2 = charbuf[0];
				famemsg -> fdom_Data3 = var.origin - 1;
				break;

			case CF_ShowText:
				ShowAnsi(famemsg -> fdom_IOString, 0, 0, 0, 0, TRUE, 0);
				break;

			case CF_ShowTextSuffix:
				ShowAnsi(famemsg -> fdom_IOString, 0, TRUE, 0, 0, TRUE, 0);
				break;

			case CF_ShowTextSufLvl:
				ShowAnsi(famemsg -> fdom_IOString, 0, TRUE, TRUE, 0, TRUE, 0);
				break;

			case CF_ExecuteCommand:
			case CF_InternalCmd:
				InternalCmd(famemsg -> fdom_IOString);
				break;

			case CF_ShTxtSufLvlCyc:
				strcpy(buf, famemsg -> fdom_IOString);
				FixDir(buf);
				strcat(buf, famemsg -> fdom_StringPtr);
				ShowAnsi(buf, FALSE, TRUE, TRUE, FALSE, TRUE, 0);
				break;

			case CF_SysOpChat:
				Chat();
				break;

			case SR_FAMEDataFileVers:
			case SR_FAMEVersion:
				famemsg -> fdom_Data2 = 1;
				famemsg -> fdom_Data3 = 23;
				strcpy(famemsg -> fdom_IOString, "1.23");
				break;

			case SR_MCI:
				MCIPS(famemsg -> fdom_IOString, strlen(famemsg -> fdom_IOString), TRUE, 0);
				break;

			case SR_ChatSet:
				Servermsg(10);
                                famemsg -> fdom_Data2 = !Structs->McpMsg.data1;
				break;

			case AR_GetKey:
				CheckKey();
				if(charbuf[0]) famemsg -> fdom_Data2 = 1;
				famemsg -> fdom_Data3 = var.origin - 1;
				break;

			case AR_SendStr:
				SafePS(famemsg -> fdom_StringPtr);
				if(famemsg -> fdom_Data1 != 0) PSLen("\r\n", 2);
				break;

			case AR_SendStrCon:
				Local(famemsg -> fdom_StringPtr);
				break;

			case AR_SendStrSer:
				SerWrite(famemsg -> fdom_StringPtr);
				break;

			case AC_ServerAction:
				SetActive(-1, famemsg -> fdom_IOString);
				break;

			default:
				mysprintf(buf, "FAME: %ld  DATA1: %ld\n", famemsg->fdom_Command, famemsg->fdom_Data1);
				PutStr(buf);
				break;
			}

			if(var.CARRIER)
				famemsg -> fdom_ReturnCode = 0;
			else
				famemsg -> fdom_ReturnCode = -2;

			ReplyMsg((struct Message *)famemsg);
		}
	}
}
