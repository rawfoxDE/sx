/*

AmiExpress door interface

*/

#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <exec/memory.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <devices/serial.h>
#include "SXStructs.h"
#include "SX.h"

#define JH_LI 0
#define JH_REGISTER 1
#define JH_SHUTDOWN 2
#define JH_WRITE 3
#define JH_SM 4
#define JH_PM 5
#define JH_HK 6
#define JH_SG 7
#define JH_SF 8
#define JH_EF 9
#define JH_CO 10
#define JH_BBSName 11
#define JH_Sysop 12
#define JH_FLAGFILE 13
#define JH_SHOWFLAGS 14
#define JH_DL 15
#define JH_ExtHK 15
#define JH_SIGBIT 16
#define JH_FetchKey 17
#define JH_SO 18

#define DT_NAME           100
#define DT_PASSWORD       101
#define DT_LOCATION       102
#define DT_PHONENUMBER    103
#define DT_SLOTNUMBER     104
#define DT_SECSTATUS      105
#define DT_SECBOARD       106
#define DT_SECLIBRARY     107
#define DT_SECBULLETIN    108
#define DT_MESSAGESPOSTED 109
#define DT_UPLOADS        110
#define DT_DOWNLOADS      111
#define DT_TIMESCALLED    112
#define DT_TIMELASTON     113
#define DT_TIMEUSED       114
#define DT_TIMELIMIT      115
#define DT_TIMETOTAL      116
#define DT_BYTESUPLOAD    117
#define DT_BYTEDOWNLOAD   118
#define DT_DAILYBYTELIMIT 119
#define DT_DAILYBYTEDLD   120
#define DT_EXPERT         121
#define DT_LINELENGTH     122
#define ACTIVE_NODES      123
#define DT_DUMP           124
#define DT_TIMEOUT        125
#define BB_CONFNAME       126
#define BB_CONFLOCAL      127
#define BB_LOCAL          128
#define BB_STATUS         129
#define BB_COMMAND        130
#define BB_MAINLINE       131
#define NB_LOAD           132
#define DT_USERLOAD       133
#define BB_CONFIG         134
#define CHG_USER          135
#define RETURNCOMMAND     136
#define ZMODEMSEND        137
#define ZMODEMRECEIVE     138
#define SCREEN_ADDRESS    139
#define BB_TASKPRI        140
#define RAWSCREEN_ADDRESS 141
#define BB_CHATFLAG       142
#define DT_STAMP_LASTON   143
#define DT_STAMP_CTIME    144
#define DT_CURR_TIME      145
#define DT_CONFACCESS     146
#define BB_PCONFLOCAL     147
#define BB_PCONFNAME      148
#define BB_NODEID         149
#define BB_CALLERSLOG     150
#define BB_UDLOG          151
#define EXPRESS_VERSION   152
#define SV_UNICONIFY      153
#define BB_CHATSET        162
#define ENVSTAT           163
#define GETKEY            500
#define RAWARROWXIM       501
#define CHAIN             502
#define NODE_DEVICE       503
#define NODE_UNIT         504
#define NODE_BAUD         505
#define NODE_NUMBER       506
#define JH_MCI            507
#define PRV_COMMAND       508
#define PRV_GROUP         509
#define BB_CONFNUM        510
#define BB_DROPDTR        511
#define BB_GETTASK        512
#define BB_REMOVEPORT     513
#define BB_SOPT           514
#define NODE_BAUDRATE     516
#define BB_LOGONTYPE      517
#define BB_SCRLEFT        518
#define BB_SCRTOP         519
#define BB_SCRWIDTH       520
#define BB_SCRHEIGHT      521
#define BB_PURGELINE      522
#define BB_PURGELINESTART 523
#define BB_PURGELINEEND   524
#define BB_NONSTOPTEXT    525
#define BB_LINECOUNT      526
#define DT_LANGUAGE       527
#define DT_QUICKFLAG      528
#define DT_GOODFILE       529
#define DT_ANSICOLOR      530
#define MULTICOM          531
#define LOAD_ACCOUNT      532
#define SAVE_ACCOUNT      533
#define SAVE_CONFDB       534
#define LOAD_CONFDB       535
#define GET_CONFNUM       536
#define SEARCH_ACCOUNT    537
#define APPEND_ACCOUNT    538
#define LAST_ACCOUNTNUM   539
#define MOD_TYPE          540
#define DT_ISANSI         541
#define BATCHZMODEMSEND   542
#define DT_MSGCODE        543
#define ACP_COMMAND       544
#define DT_FILECODE       545
#define EDITOR_STRUCT     546
#define BYPASS_CSI_CHECK  547
#define SENTBY            548
#define SETOVERIDE        549
#define FULLEDIT          550
#define DT_ADDBIT         1000
#define DT_REMBIT         1001
#define DT_QUERYBIT       1002
#define SX_PS		1500
#define SX_USERPO	1501
#define SX_USERPO2	1502
#define SX_USERPO3	1503
#define SX_VER		1504
#define SX_NODES	1505
#define SX_FUNCTION	1506
#define SX_ARG		1507
#define SX_CONF		1508
#define SX_FILEATTACH	1509
#define SX_GETPATH	1510
#define SX_FINDFILE	1511
#define	SX_SHOWANSI	1512
#define SX_CONFACCESS	1513
#define SX_CONFNAME	1514
#define SX_HITRETURN	1515

extern struct StructsStruct	*Structs;
extern struct AmigaStruct	Amiga;
extern struct StringsStruct	*stg;
extern struct VariablesStruct	var;
extern struct TagItem		tags[];

extern void GetPath(char *outbuf, UWORD confnum, UWORD area);

extern UBYTE buf[512], charbuf[128];
extern char Attachment[80];

void WaitForXIM(char *door)
{

struct SXFuncStruct
{
	UWORD	id;
	char	*string;
	UWORD	extra;
	UWORD	low;
	UWORD	high;
	char	*mainarg;
	char	*execarg;
} *SXFunc;

struct JHMessage
{
  struct Message Msg;
  char String[200];
  long Data;
  long Command;
  long NodeID;
  long LineNum;
  unsigned long signal;
  struct Process *task;
  long Semi;
} *msg;
struct MsgPort *mp;
struct DateTime stamp;
char portname[16], year[4], date[4], day[4], returncmd[32];
BOOL rcmd=FALSE, spaceflag = FALSE, itsopen = FALSE;
BPTR fh;
UBYTE waitkey=0;
long Nodes=0, x;

SetActive(106, 0);

mysprintf(portname, "AEDoorPort%ld", var.node);

if(mp = FindPort(portname))
	itsopen = TRUE;
else
	mp = CreatePort(portname, 0L);

if(!mp) return;

SystemTagList(door, tags);

var.RAWARROW = TRUE;

while(1)
{
	WaitPort(mp);
	while((msg = (struct JHMessage *)GetMsg(mp)))
	{
		//mysprintf(buf,"cmd: %ld data: %ld str: |%s|\n",msg->Command,msg->Data,msg->String);
		//PutStr(buf);

		switch(msg->Command)
		{
		case JH_BBSName:
			strcpy(msg->String, Structs->Cfg->BBSName);
			break;
		case JH_Sysop:
			strcpy(msg->String, Structs->Cfg->Sysop);
			break;
		case JH_REGISTER:
            		Nodes++;
			msg->String[0] = 'S';
			msg->String[1] = 'X';
			msg->String[2] = 0;
			break;
		//case JH_WRITE:
		//	if(Structs->Disp.flags&(1<<2)) StripAnsi((char *)&msg->String, strlen(msg->String));
		//	PS(msg->String);
		//	break;
		case CHAIN:
			Nodes--;
			break;
		case 2456:
			Nodes = 1;
		case JH_SHUTDOWN:
			Nodes--;
			if(Nodes < 1)
			{
           	ReplyMsg((struct Message *)msg);
				if(!itsopen) DeletePort((struct MsgPort *)mp);
				var.TIME_OUT = 0;
				var.TIMEOUTFLAG = FALSE;
				var.RAWARROW = TRUE;
				if(rcmd)
					InternalCmd(returncmd);
				else
					PS("[A");
				return;
			}
			break;
		case JH_CO:
			Local(msg->String);
			if(msg->Data) Local("\r\n");
			break;
		case JH_SO:
			SerWrite(msg->String);
			if(msg->Data) SerWrite("\r\n");
			break;
		case JH_MCI:
			if(Structs->Disp.flags&(1<<2)) StripAnsi((char *)&msg->String, strlen(msg->String));
			MCIPS(msg->String, strlen(msg->String), 0, 0);
			break;
		case JH_WRITE:
		case JH_SM:
			if(spaceflag)
			{
				if(strcmp(msg->String, "\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b")==0)
					spaceflag = FALSE;
				else
					PS("                          ");
			} else {
				if(strcmp(msg->String, "                          ")==0)
					spaceflag = TRUE;
				else {
					spaceflag = FALSE;
					if(Structs->Disp.flags&(1<<2)) StripAnsi((char *)&msg->String, strlen(msg->String));
					if(msg->Data && msg->Command==JH_SM)
					{
						x = strlen(msg->String);
						msg->String[x]=13;
						x++;
						msg->String[x]=10;
						x++;
						msg->String[x]=0;
					}
					PS(msg->String);
				}
			}
			break;
		case JH_PM:
			if(Structs->Disp.flags&(1<<2)) StripAnsi((char *)&msg->String, strlen(msg->String));
			PS(msg->String);
			LineInput(msg->Data, FALSE, 0);
			PS("\r\n");
			strcpy(msg->String,charbuf);
			if(var.CARRIER) msg->Data=1; else msg->Data=-1;
			break;
		case JH_ExtHK:
		case JH_HK:
			if(strcmp("press <RETURN> to continue",msg->String)==0)
				spaceflag = TRUE;
			else {
				SafePS(msg->String);
				if(waitkey!=0)
					msg->String[0]=waitkey;
				else {
					MyHotKey();
					msg->String[0]=charbuf[0];
				}
				msg->String[1]='\0';
				msg->Command=var.origin;
			}
			if(var.CARRIER && !var.TIMEOUTFLAG) msg->Data=1; else msg->Data=-1;
			break;
		case GETKEY:
			CheckKey();
			if(charbuf[0]!='\0')
			{
				msg->String[0]='1';
				waitkey = charbuf[0];
			} else {
				msg->String[0]='0';
				waitkey = 0;
			}
			msg->String[1]='\0';
			if(var.CARRIER) msg->Data=1; else msg->Data=-1;
			break;
		case JH_FetchKey:
			CheckKey();
			msg->Command = charbuf[0];
			if(var.CARRIER) msg->Data=1; else msg->Data=-1;
			break;
		case JH_FLAGFILE:
			msg->Data = 0;
			switch(FlagFile(msg->String, 0, 0, 0))
			{
			case 1: msg->Data = 1; break;
			case 4: PS(Structs->SXStr[107]); PSLen("\r\n", 2); break;
			case 5: PS(Structs->SXStr[106]); PSLen("\r\n", 2); break;
			}
			break;
		case JH_LI:
			LineInput(msg->Data,0,msg->String);
			PS("\r\n");
			strcpy(msg->String,charbuf);
			if(var.CARRIER) msg->Data=1; else msg->Data=-1;
			break;
		case DT_DUMP:
			fh = Open(msg->String, MODE_NEWFILE);
			if(fh) {
				Write(fh, &Structs->User, sizeof(struct UserData));
				Close(fh);
			}
			break;
		case DT_STAMP_LASTON:
			memcpy(&stamp, __timecvt((time_t)Structs->User.Time_Last_On), 12);
			stamp.dat_Format  = FORMAT_DOS;
			stamp.dat_StrDay  = "2222222222";
			stamp.dat_StrDate = "000000000";
			stamp.dat_StrTime = "111111111";
			stamp.dat_Flags   = 0;
			DateToStr(&stamp);
			strmid(stamp.dat_StrDate,buf,4,3);
			strmid(stamp.dat_StrDate,date,1,2);
			strmid(stamp.dat_StrDate,year,8,2);
			strmid(stamp.dat_StrDay,day,1,3);
			mysprintf(msg->String,"%s %s %s %s 19%s\n",day,buf,date,stamp.dat_StrTime,year);
			break;
		case DT_CURR_TIME:
			DateStamp(&stamp);
			mysprintf(msg->String,"%ld",stamp.dat_Stamp.ds_Minute*60);
			break;
		case DT_STAMP_CTIME:
			DateStamp(&stamp);
			stamp.dat_Format  = FORMAT_DOS;
			stamp.dat_StrDay  = "2222222222";
			stamp.dat_StrDate = "000000000";
			stamp.dat_StrTime = "111111111";
			stamp.dat_Flags   = 0;
			DateToStr(&stamp);
			strmid(stamp.dat_StrDate,buf,4,3);
			strmid(stamp.dat_StrDate,date,1,2);
			strmid(stamp.dat_StrDate,year,8,2);
			strmid(stamp.dat_StrDay,day,1,3);
			mysprintf(msg->String,"%s %s %s %s 19%s\n",day,buf,date,stamp.dat_StrTime,year);
			break;
		case EXPRESS_VERSION:
			strcpy(msg->String, "v4.20"); break;
		case BB_DROPDTR:
			DropDTR(TRUE);
			break;
		case BB_LOCAL:
			strcpy(msg->String, "SX:"); break;
		case BB_CHATFLAG:
			Servermsg(10);
			if(Structs->McpMsg.data1)
				strcpy(msg->String, "OFF"); // AWAY
			else
				strcpy(msg->String, "ON"); // NOT AWAY
			break;
		case 177:
			SetActive(-1,msg->String);
			break;
		case ENVSTAT:
			if(msg->Data) mysprintf(msg->String,"%ld",var.action);
			else SetActive((msg->String[0]-48), 0);
			break;
       		case DT_NAME:
			if(msg->Data) strcpy(msg->String,Structs->User.Name);
			else mysprintf(Structs->User.Name,"%.31s",msg->String);
			break;
		case JH_SG:
			ShowAnsi(msg->String,0,1,0,0,1,0);
			break;
		case JH_SF:
			ShowAnsi(msg->String,0,0,0,0,1,0);
			break;
		case DT_LANGUAGE:
			if(msg->Data) strcpy(msg->String,"TXT");
			break;
		case BB_CONFNUM:
			if(msg->Data) mysprintf(msg->String, "%ld", var.thisconf-1);
			else var.thisconf = atoi(msg->String)+1;
			break;
      case  BB_CONFNAME:
			if(msg->Data) strcpy(msg->String,Structs->Conf[var.thisconf]->name);
			else strcpy(Structs->Conf[var.thisconf]->name,msg->String);
         break;
      case  BB_CONFLOCAL:
			if(msg->Data) strcpy(msg->String,Structs->Conf[var.thisconf]->path);
			else strcpy(Structs->Conf[var.thisconf]->path,msg->String);
         break;
      case BB_GETTASK:
         msg->task=(struct Process *)FindTask(0L);
         break;
      case BB_LOGONTYPE:
			if(Nodes > 0)
			{
				if(!var.done && !var.CALLER) msg->Data = 0;
            if(var.SYSOPLOGIN && var.CALLER) msg->Data = 1;           
            if(!var.SYSOPLOGIN && var.LOCAL && var.CALLER) msg->Data = 2;
            if(!var.LOCAL && var.CARRIER && var.CALLER)    msg->Data = 3;
         } else msg->Data=-1;
         break;
		case NODE_NUMBER:
			mysprintf(msg->String,"%ld",var.node);
			break;
      case NODE_BAUD:
			mysprintf(msg->String,"%ld",var.baud);
			break;
      case NODE_BAUDRATE:
			mysprintf(msg->String,"%ld",var.baud);
			break;
      case NODE_DEVICE:
         strcpy(msg->String,Structs->Serial.device); break;
      case NODE_UNIT:
         mysprintf(msg->String,"%ld",Structs->Serial.unit);
         break;

       case  DT_PASSWORD      :
             if(msg->Data) strcpy(msg->String,Structs->User.Pass);
             else mysprintf(Structs->User.Pass,"%.9s",msg->String);
             break;
       case  DT_LOCATION      :
             if(msg->Data) strcpy(msg->String,Structs->User.Location);
             else mysprintf(Structs->User.Location,"%.30s",msg->String);
             break;
       case  DT_PHONENUMBER   :
             if(msg->Data) strcpy(msg->String,Structs->User.PhoneNumber);
             else mysprintf(Structs->User.PhoneNumber,"%.13s",msg->String);
             break;
       case  DT_SLOTNUMBER    :
             if(msg->Data) mysprintf(msg->String,"%ld",Structs->User.Slot_Number);
             else Structs->User.Slot_Number=atoi(msg->String);
             break;
       case  DT_SECSTATUS     :
             if(msg->Data) mysprintf(msg->String,"%ld",Structs->User.Sec_Status);
             else Structs->User.Sec_Status=atoi(msg->String);
             break;
       case  DT_SECBOARD      :
             if(msg->Data) mysprintf(msg->String,"%ld",Structs->User.Sec_Board);
             else Structs->User.Sec_Board=atoi(msg->String);
             break;
       case  DT_SECLIBRARY    :
             if(msg->Data) mysprintf(msg->String,"%ld",Structs->User.Sec_Library);
             else Structs->User.Sec_Library=atoi(msg->String);
             break;
       case  DT_SECBULLETIN   :
             if(msg->Data) mysprintf(msg->String,"%ld",Structs->User.Sec_Bulletin);
             else Structs->User.Sec_Bulletin=atoi(msg->String);
             break;
       case  DT_MESSAGESPOSTED:
             if(msg->Data) mysprintf(msg->String,"%ld",Structs->User.Messages_Posted);
             else Structs->User.Messages_Posted=atoi(msg->String);
             break;
       case  DT_UPLOADS       :
             if(msg->Data) mysprintf(msg->String,"%ld",Structs->User.Uploads);
             else Structs->User.Uploads=atoi(msg->String);
             break;
       case  DT_DOWNLOADS     :
             if(msg->Data) mysprintf(msg->String,"%ld",Structs->User.Downloads);
             else Structs->User.Downloads=atoi(msg->String);
             break;
       case  DT_TIMESCALLED   :
             if(msg->Data) mysprintf(msg->String,"%ld",Structs->User.Times_Called);
             else Structs->User.Times_Called=atoi(msg->String);
             break;
       case  DT_TIMELASTON    :
             if(msg->Data) mysprintf(msg->String,"%ld",Structs->User.Time_Last_On);
             else Structs->User.Time_Last_On=atol(msg->String);
             break;
       case  DT_TIMEUSED      :
             if(msg->Data) mysprintf(msg->String,"%ld",Structs->User.Time_Used);
             else Structs->User.Time_Used=atol(msg->String);
             break;
       case  DT_TIMELIMIT     :
             if(msg->Data) mysprintf(msg->String,"%ld",Structs->User.Time_Limit);
             else Structs->User.Time_Limit=atol(msg->String);
             break;
       case  DT_TIMETOTAL     :
             if(msg->Data) mysprintf(msg->String,"%ld",Structs->User.Time_Left);
             else Structs->User.Time_Left=atol(msg->String);
             break;
       case  DT_BYTESUPLOAD   :
             if(msg->Data) mysprintf(msg->String,"%ld",Structs->User.Bytes_Upload);
             else Structs->User.Bytes_Upload=atol(msg->String);
             break;
       case  DT_BYTEDOWNLOAD  :
             if(msg->Data) mysprintf(msg->String,"%ld",Structs->User.Bytes_Download);
             else Structs->User.Bytes_Download=atol(msg->String);
             break;
       case  DT_DAILYBYTELIMIT:
             if(msg->Data) mysprintf(msg->String,"%ld",Structs->User.Daily_Bytes_Limit);
             else Structs->User.Daily_Bytes_Limit=atol(msg->String);
             break;
       case  DT_DAILYBYTEDLD  :
             if(msg->Data) mysprintf(msg->String,"%ld",Structs->User.Daily_Bytes_Dld);
             else Structs->User.Daily_Bytes_Dld=atol(msg->String);
             break;
       case  DT_EXPERT        :
             if(msg->Data) mysprintf(msg->String,"%lc",Structs->User.Expert);
             else Structs->User.Expert=msg->String[0];
             break;
       case  DT_LINELENGTH    :
             if(msg->Data) mysprintf(msg->String,"%ld",Structs->User.LineLength);
             else Structs->User.LineLength=atoi(msg->String);
             break;
      case  DT_TIMEOUT:
            if(msg->Data) mysprintf(msg->String,"%ld",var.TIME_OUT);
            else var.TIME_OUT=atol(msg->String);
            break;
      case  SCREEN_ADDRESS:
            mysprintf(msg->String,"%08lx",Amiga.scr);
            break;
      case  RAWSCREEN_ADDRESS:
            mysprintf(msg->String,"%ld",Amiga.scr);
            break;
       case BB_MAINLINE:
		if(stg->arg[0]==0)
			strcpy(msg->String, stg->cmd);
		else
			mysprintf(msg->String,"%s %s", stg->cmd, stg->arg);
		break;
	case BB_PURGELINE:
	case BB_NONSTOPTEXT:
		break;
		case RAWARROWXIM:
			if(var.RAWARROW) var.RAWARROW = FALSE; else var.RAWARROW = TRUE;
			break;
		case BB_CHATSET:
			if(msg->Data) mysprintf(msg->String,"%ld",var.justchatted);
			else var.justchatted = atoi(msg->String);
			break;
		case DT_CONFACCESS:
			strcpy(msg->String, Structs->AccSet.name);
			break;
		case BB_PCONFLOCAL:
			x = atol(msg->String);
			if(x > var.confs  ||  x < 1)
				msg->String[0] = 0;
			else
				strcpy(msg->String, Structs->Conf[ x ] -> path );
			break;
		case BB_CALLERSLOG:
			/*CallerLog(msg->String);*/
			break;
		case DT_ISANSI:
			if(Structs->Disp.flags&(1<<2)) msg->Data=0;
			else msg->Data=1;
			break;
		/*case MULTICOM:
			msg->Semi=0;
			break;*/
		case 628:
		case PRV_COMMAND:
			InternalCmd(msg->String);
			break;
		case RETURNCOMMAND:
			strcpy(returncmd, msg->String);
			rcmd=TRUE;
			break;
		case ZMODEMRECEIVE:
			RawReceive(msg->String, TRUE);
			msg->Data = 1;
			break;
		case ZMODEMSEND:
			msg->Data = SendFile(msg->String);
			if(!var.CARRIER) msg->Data = -2;
			break;
		case 614:	// CONF ACCESS
			if(CheckConfAccess(&Structs->SXUser, msg->Data+1))
				msg->Data = 1;
			else
				msg->Data = 0;
			break;
		case SX_PS:
			PS((char *)msg->Data);
			break;
		case SX_USERPO:
			msg->Data=(long)&Structs->User;
			break;
		case SX_USERPO2:
			msg->Data=(long)&Structs->SXUser;
			break;
		case SX_USERPO3:
			msg->Data=(long)&Structs->UserIndex;
			break;
		case SX_VER:
			strcpy(msg->String, "1.06");
			break;
		case SX_NODES:
			Servermsg(14);
			msg->Data = Structs->McpMsg.data1;
			break;
		case SX_FUNCTION:
			SXFunc = (struct SXFuncStruct *) msg -> Data;
			DoFunction(SXFunc->id, SXFunc->string, SXFunc->extra,
					SXFunc->low, SXFunc->high,
					SXFunc->mainarg, SXFunc->execarg);
			break;
		case SX_ARG:
			msg->Data = (long)((char *)&stg->arg);
			break;
		case SX_CONF:
			msg->Data = (long)Structs->Conf[var.thisconf];
			break;
		case SX_FILEATTACH:
			strcpy(Attachment, msg->String);
			break;
		case SX_GETPATH:
			GetPath(msg->String, atoi(msg->String), msg->Data);
			break;
		case SX_FINDFILE:
			if(FindFileOnHD(msg->String, FALSE))
				strcpy(msg->String, stg->path);
			else
				msg->String[0] = 0;
			break;
		case SX_SHOWANSI:
			ShowAnsi(msg->String,
					(msg->Data&(1L<<0)),
					(msg->Data&(1L<<1)),
					(msg->Data&(1L<<2)),
					(msg->Data&(1L<<3)),
					(msg->Data&(1L<<4)), 0);
			break;
		case SX_CONFACCESS:
			msg->Data = Structs->SXUser.ConfAccess[0];
			msg->Command = var.confs;
			break;
		case SX_CONFNAME:
			strcpy(msg->String, Structs->Conf[msg->Data]->name);
			break;
		case SX_HITRETURN:
			HitReturn();
			break;
		default:
			mysprintf(buf,"XIM: %ld  DATA: %ld\n", msg->Command, msg->Data);
			PutStr(buf);
			break;
		}
    		ReplyMsg((struct Message *)msg);

	}
}
}
