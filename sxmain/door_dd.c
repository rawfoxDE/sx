
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

extern struct Library *IntuitionBase,*GfxBase,*DiskfontBase,*GadToolsBase;
extern struct ReqToolsBase *ReqToolsBase;

extern UBYTE buf[512], charbuf[128];


struct DayDream_DisplayMode
{
  UBYTE DISPLAY_ID;
  char DISPLAY_PATH[9];
  UWORD DISPLAY_ATTRIBUTES;
  UBYTE DISPLAY_INCOMING_TABLEID;
  UBYTE DISPLAY_OUTGOING_TABLEID;
  char DISPLAY_FONT[20];
  UWORD DISPLAY_FONTSIZE;
  UBYTE DISPLAY_STRINGS;
  char DISPLAY_FREEBLOCK[63];
};

struct DayDream_User
{
  char user_realname[26];
  char user_handle[26];
  char user_organization[26];
  char user_zipcity[21];
  char user_voicephone[21];
  char user_password[16];
  UBYTE	user_screenlength;
  UBYTE	user_protocol;
  ULONG	user_toggles;
  char user_signature[45];
  UBYTE	freeslot1;
  ULONG	user_ulbytes;
  ULONG	user_dlbytes;
  UWORD	user_ulfiles;
  UWORD	user_dlfiles;
  UWORD	user_pubmessages;
  UWORD	user_pvtmessages;
  UWORD	user_connections;
  UBYTE	user_fileratio;
  UBYTE	user_byteratio;
  char user_computermodel[21];
  UBYTE	freeslot2;
  ULONG	user_freedlbytes;
  UBYTE	user_failedlogins;
  UBYTE	user_securitylevel;
  UBYTE	user_joinconference;
  UBYTE	freeslot3;
  ULONG	user_firstcall;
  ULONG	user_lastcall;
  ULONG	user_conferenceacc1;
  ULONG	user_conferenceacc2;
  UWORD	user_dailytimelimit;
  UWORD	user_account_id;
  UWORD	user_timeremaining;
  UWORD	user_freedlfiles;
  UWORD	user_fakedfiles;
  ULONG	user_fakedbytes;
  char  user_inetname[9];
  char 	user_freeblock[23];
};

struct DayDream_MainConfig
{
  char	CFG_BOARDNAME[26];
  char	CFG_SYSOPNAME[26];
  UBYTE CFG_CHATMODE;
  UBYTE CFG_LOCALSCREEN;
  ULONG CFG_DEFAULTS;
  char	CFG_SERIALCODE[5];
  char	CFG_DEFAULTASCPATH[21];
  char	CFG_CHATDLPATH[41];
  UBYTE	CFG_DISPLAYMODE;
  ULONG	CFG_COPYBUFFER;
  UBYTE	CFG_JOINIFAUTOJOINFAILS;
  char	CFG_COLORSYSOP[11];
  char	CFG_COLORUSER[11];
  UBYTE	CFG_LINEEDCHAR;
  char	CFG_SYSTEMPW[16];
  char	CFG_NEWUSERPW[16];
  char	CFG_OLUSEREDPW[16];
  ULONG	CFG_UNIQUEMSGNUMBER;
  UBYTE	CFG_DIRFILEBUFFERSIZE;
  UBYTE	CFG_NEWUSERPRESETID;
  ULONG	CFG_IDLETIMEOUT;
  ULONG	CFG_FREEHDDSPACE;
  ULONG	CFG_FLAGS;
  char	CFG_SHELLPW[16];
  char  CFG_ALIENS[40];
  char  CFG_FSEDCOMMAND[71];
  char  CFG_FREEDLLINE[100];
  char  CFG_OLMS[41];
  UBYTE CFG_COSYSOPLEVEL;
  UBYTE CFG_CONTROLCODE;
};


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

struct DDNodeStruct
{
	struct DDNodeStruct *ns_Prev;
	struct DDNodeStruct *ns_Next;
	struct DayDream_User *ns_ub;
	UWORD ns_Unused;
	UBYTE ns_Nodenumber;
	UBYTE ns_Flags;
	struct Screen *ns_Screen;
	APTR ns_Activity;
	ULONG ns_ConnectionRate;
	APTR ns_Pagereason;
	struct DayDream_Multinode *ns_Currnode;
};

void WaitForDD(char *door)
{
	struct MsgPort *mp;
	struct DDMsg
	{
		struct Message Door_Msg;
		WORD command;
		long data1;
		long data2;
		long data3;
		long po[21];
		char str[2];
	} *doormsg;
	char portname[16];
	void *nullstruct;
	struct DayDream_User *DDUser;
	struct DDNodeStruct *ddnode;
	struct DayDream_DisplayMode *dmode;
	struct DayDream_MainConfig *maindd;
	long x;

	SetActive(104, 0);

	mysprintf(portname, "DD_DoorPort%ld", var.node);
	mp = CreatePort(portname, 0L);
	if(!mp) return;

	SystemTagList(door, tags);

	nullstruct = AllocMem(1024, MEMF_PUBLIC|MEMF_CLEAR);

	dmode = AllocMem(sizeof(struct DayDream_DisplayMode), MEMF_PUBLIC|MEMF_CLEAR);
	if(dmode)
	{
		dmode -> DISPLAY_ID = var.cmode;
		if(Structs->Disp.flags&(1<<2))
			dmode -> DISPLAY_ATTRIBUTES = 0;
		else
			dmode -> DISPLAY_ATTRIBUTES |= 1<<0;
	}

	maindd = AllocMem(sizeof(struct DayDream_MainConfig), MEMF_PUBLIC|MEMF_CLEAR);
	if(maindd)
	{
		strcpy(maindd->CFG_BOARDNAME, Structs->Cfg->BBSName);
		strcpy(maindd->CFG_SYSOPNAME, Structs->Cfg->Sysop);
	}

	Structs->McpMsg.data1 = var.node;
	Servermsg(18);
	ddnode = (struct DDNodeStruct *) Structs->McpMsg.data1;
	DDUser = ddnode -> ns_ub;

	var.RAWARROW = FALSE;

	while(1)
	{
		WaitPort((struct MsgPort *)mp);
		while(doormsg=(struct DDMsg *)GetMsg((struct MsgPort *)mp))
		{
			/*mysprintf(buf, "DD: %ld  DATA1: %ld\n", doormsg->command, doormsg->data1);
			PutStr(buf);*/
			switch(doormsg->command)
			{
			case 17:
				var.RL = TRUE;
			case 0:
				ReplyMsg((struct Message *)doormsg);
				DeletePort((struct MsgPort *)mp);
				var.RAWARROW = TRUE;
				FreeMem(nullstruct, 1024);
				if(dmode) FreeMem(dmode, sizeof(struct DayDream_DisplayMode));
				if(maindd) FreeMem(maindd, sizeof(struct DayDream_MainConfig));
				if(DDUser) strcpy(Structs->SXUser.sentbyline, DDUser->user_signature);
				return;
			case 1:
				if(Structs->Disp.flags&(1<<2)) StripAnsi((char *)doormsg->data1, strlen((char *)doormsg->data1));
				SafePS((char *)doormsg->data1);
				doormsg->command = 1L;
				break;
			case 2:
				LineInput((doormsg->data2>>16), (doormsg->data2&(1<<0)), (char *)doormsg->data1);
				if(!(doormsg->data2&(1<<1))) PS("\r\n");
				strcpy((char *)doormsg->data1, charbuf);
				break;
			case 3:
				doormsg->data1=0;
				doormsg->data2=Structs->User.Time_Last_On - 252504000;
				/*memset(&doormsg->po[0], 0, 84);*/
				doormsg->po[0] = (long) DOSBase;
				doormsg->po[1] = (long) IntuitionBase;
				doormsg->po[2] = (long) GfxBase;
				doormsg->po[3] = (long) ReqToolsBase;
				doormsg->po[4] = (long) DiskfontBase;	//16
				doormsg->po[5] = (long) nullstruct;	//20
				doormsg->po[6] = (long) nullstruct;	//24
				doormsg->po[7] = (long) nullstruct;	//28
				doormsg->po[8] = (long) maindd;		//32
				doormsg->po[9] = (long) dmode; /* <-- wrong */
				doormsg->po[10]= (long) dmode;
				doormsg->po[11]=(long)nullstruct;
				doormsg->po[12]=(long)nullstruct;
				doormsg->po[13]=(long)nullstruct;
				doormsg->po[14]=(long)nullstruct;
				doormsg->po[15]=(long)nullstruct;
				doormsg->po[16]=(long)nullstruct;
				doormsg->po[17]=(long)DDUser;
				doormsg->po[18]=(long)nullstruct;
				doormsg->po[19]=(long)nullstruct;
				doormsg->po[20]=(long)&stg->arg;
				break;
			case 4:
				ShowAnsi((char *)doormsg->data1, (doormsg->data3&(1<<0)), (doormsg->data3&(1<<0)), (doormsg->data3&(1<<4)), (doormsg->data3&(1<<3)), 0, 0);
				break;
			case 5:
				if(doormsg->data1 == 0) doormsg->data1 = 2;
				doormsg->data1 = YesNo(doormsg->data1, 1);
				if(var.CARRIER) doormsg->command = 1L; else doormsg->command = NULL;
				break;
			case 6:
				x = FindAccount((char *)doormsg->data1);
				if(x)
				{
					doormsg->command = 1;
					doormsg->data2 = NULL;
					doormsg->data1 = x * sizeof(struct UserData);
				} else
					doormsg->command = NULL;
				break;
			case 7:
				doormsg->command=1L;
				break;
			case 8:
				if(doormsg->data2 == -1) doormsg->data2 = mytime();
				myctime(doormsg->data2, (char *)doormsg->data1);
				doormsg->data1 = doormsg->data2;
				break;
			case 9:
				if(doormsg->data2&(1<<0))
/* receive */				RawReceive((char *)doormsg->data1, TRUE);
				/*else {
				}*/
				doormsg->command = 1L;
			case 10:
				break;
			case 11:
				if(doormsg->data1&(1<<2))
					var.RAWARROW = FALSE;
				else
					var.RAWARROW = TRUE;

				MyHotKey();
				if(doormsg->data1&(1<<0))
				{
					PSLen(charbuf, 1);
					if(doormsg->data1&(1<<1)) PSLen("\r\n", 2);
				}
				switch(charbuf[0])
				{
				case 2: charbuf[0]=253; break;
				case 3: charbuf[0]=252; break;
				case 4: charbuf[0]=250; break;
				case 5: charbuf[0]=251; break;
				}
				memcpy(&doormsg->command, &charbuf[0], 1);
				if(var.origin==2) doormsg->data1=0; else doormsg->data1=var.origin;
				var.RAWARROW = FALSE;
				if(!var.CARRIER) doormsg->command = NULL;
				break;
			case 13:
				strcpy(buf, (char *)doormsg->data1);
				strcat(buf, (char *)doormsg->data2);
				switch(FlagFile(buf, (doormsg->data3&(1<<0)), FALSE, TRUE))
				{
				case 2: doormsg->command = 1; break;
				case 4: doormsg->command = 3; break;
				case 5: doormsg->command = 2; break;
				default: doormsg->command = 0;
				}
				break;
			case 14:
				DropDTR(TRUE);
				break;
			case 18:
				doormsg->data1=var.baud;
				memset(&doormsg->po[5], 0, 64);
				Structs->McpMsg.data1 = 1;
				Servermsg(18);
				doormsg->po[0] = (long)Structs->McpMsg.data1;
				doormsg->po[1] = (long)ddnode;
				doormsg->po[2] = (long)nullstruct;
				doormsg->po[3] = (long)nullstruct;
				doormsg->po[4] = (long)nullstruct;
				doormsg->po[5] = (long)nullstruct;
				doormsg->po[6] = (long)nullstruct;
				doormsg->po[7] = (long)nullstruct;
				doormsg->po[8] = (long)nullstruct;
				doormsg->po[9] = (long)nullstruct;
				doormsg->po[10] = (long)nullstruct;
				break;
			case 19:
				JoinConf(doormsg->data1, !(doormsg->data1&(1<<2)));
				break;
			case 20:
				GotoMsgArea(doormsg->data1);
				break;
			case 15:
			case 21:
				InternalCmd((char *)doormsg->data1);
				break;
			case 22:
				CheckKey();
				memcpy(&doormsg->command, &charbuf[0], 1);
				if(var.origin==2) doormsg->data1=0; else doormsg->data1=var.origin;
				break;
			case 23:
				LineED((char *)doormsg->data1);
				break;
			case 24:
				FifoExecute((char *)doormsg->data1, 0);
				break;
			case 27:
				if(doormsg->data1&(1<<2))
					var.RAWARROW = FALSE;
				else
					var.RAWARROW = TRUE;

				var.TIME_OUT = doormsg->data2;
				if(var.TIME_OUT>29) var.TIME_OUT = 0;
				MyHotKey();
				var.TIME_OUT = 0;
				if(var.TIMEOUTFLAG)
				{
					doormsg->command = -1;
					var.TIMEOUTFLAG = FALSE;
				} else {
					if(doormsg->data1&(1<<0))
					{
						PSLen(charbuf, 1);
						if(doormsg->data1&(1<<1)) PSLen("\r\n", 2);
					}
					switch(charbuf[0])
					{
					case 2: charbuf[0]=253; break;
					case 3: charbuf[0]=252; break;
					case 4: charbuf[0]=250; break;
					case 5: charbuf[0]=251; break;
					}
					memcpy(&doormsg->command, &charbuf[0], 1);
					if(var.origin==2) doormsg->data1=0; else doormsg->data1=1;
					if(!var.CARRIER) doormsg->command = NULL;
				}
				var.RAWARROW = FALSE;
				break;
			case 28:
				SetActive(-1, (char *)doormsg->data1);
				break;
			case 29:
/* query sysop */		Servermsg(10);
				doormsg->data1 = Structs->McpMsg.data1;
				break;
			case 30:
				NodeMsg(doormsg->data1, 0, (char *)doormsg->data2);
				break;
			/*case 32:
				if(doormsg->data1) var.SER = TRUE; else var.SER = FALSE;
				break;*/
			case 33:
				Chat();
				break;
			case 34:
				HitReturn();
				break;
			case 36:
				Structs->User.Time_Left = (doormsg->data1*60) + 1;
				var.lastleft = Structs->User.Time_Left;
				break;
			default:
				mysprintf(buf, "DD: %ld  DATA1: %ld\n", doormsg->command, doormsg->data1);
				PutStr(buf);
				break;
			}
			ReplyMsg((struct Message *)doormsg);
		}
	}
}
