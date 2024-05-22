
/* paragon stuff */

#include <stdlib.h>
#include <string.h>
#include <exec/memory.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <devices/serial.h>

LONG PutStr( STRPTR str );

#include "SXStructs.h"
#include "SX.h"

#define PG_SHUTDOWN 20
#define PG_CONSOLE 2
#define PG_SHOWSTRING 1
#define PG_SERWRITE 3
#define PG_CHARCONSOLE 4
#define PG_CHAR 5
#define PG_PM 6
#define PG_HK 8
#define PG_DROP 9
#define PG_SG 19
#define PG_SF 10
#define PG_EF 12
#define PG_UD 13
#define PG_US 14
#define PG_PS 15
#define PG_CS 16
#define PG_RD 17
#define PG_CL 18
#define PG_TM 21
#define PG_SC 7
#define PG_FF 11
#define MAX_CHECKKEY 201
#define MAX_AUTOINSERT 203

extern struct StructsStruct	*Structs;
extern struct AmigaStruct	Amiga;
extern struct StringsStruct	*stg;
extern struct VariablesStruct	var;
extern struct TagItem		tags[];

extern UBYTE buf[512], charbuf[128];

void MAXsPS(char *ans, long size, BOOL noprint);
BOOL MAXsShowAnsi(char *fname);


void WaitForParagon(char *door)
{
struct MsgPort *mp;
struct DoorMsg
{
	struct Message Door_Msg;
	short command;
	short data;
	char string[80];
	short carrier;
} *doormsg;
char portname[16];

SetActive(105, 0);

mysprintf(portname, "DoorControl%ld", var.node);
mp = CreatePort(portname, 0L);
if(!mp) return;

SystemTagList(door, tags);

while(1)
{
	WaitPort((struct MsgPort *)mp);
	while(doormsg=(struct DoorMsg *)GetMsg((struct MsgPort *)mp))
	{
		if(var.CARRIER)doormsg->carrier=FALSE; else doormsg->carrier=TRUE;
		/*mysprintf(buf,"PARA: %ld  DATA: %ld  STR: %s\n",doormsg->command,doormsg->data,doormsg->string);
		PutStr(buf);*/
		switch(doormsg->command)
		{
		case PG_DROP:
			DropDTR(TRUE);
			break;
		case PG_SHUTDOWN:
			ReplyMsg((struct Message *)doormsg);
			DeletePort((struct MsgPort *)mp);
			PS("\r\n");
			return;
		case PG_SERWRITE:
			buf[0]=doormsg->data;
			buf[1]='\0';
			SerWrite(buf);
			break;
		case PG_CHARCONSOLE:
			buf[0]=doormsg->data;
			buf[1]='\0';
			Local(buf);
			break;
		case PG_CHAR:
			buf[0]=doormsg->data;
			if(buf[0]==8) strcpy(buf,"\b \b"); else buf[1]='\0';
			PS(buf);
			break;
		case PG_CONSOLE:
			Local(doormsg->string);
			if(doormsg->data==1) Local("\r\n");
			break;
		case PG_SHOWSTRING:
			MAXsPS(doormsg->string, strlen(doormsg->string), FALSE);
			if(doormsg->data==1) PS("\r\n");
			break;
		case PG_PM:
		case PG_SC:
			MAXsPS(doormsg->string, strlen(doormsg->string), FALSE);
			LineInput(doormsg->data, 0, 0);
			strcpy(doormsg->string, charbuf);
			break;
		case PG_HK:
			MAXsPS(doormsg->string, strlen(doormsg->string), FALSE);
			MyHotKey();
			doormsg->string[0]=charbuf[0];
			doormsg->string[1]=0;
			doormsg->data=var.origin-1;
			break;
		case PG_UD:
			switch(doormsg->data)
			{
			case 1: doormsg->data=Structs->User.Sec_Status;	break;
			case 2: doormsg->data=var.EXPERT;	break;
			case 4: doormsg->data=Structs->User.Times_Called;break;
			case 7: doormsg->data=Structs->User.Time_Left/60;break;
			case 8: doormsg->data=80;		break;
			case 9: doormsg->data=Structs->User.LineLength;	break;
			}
			break;
		case PG_US:
			switch(doormsg->data)
			{
			case 1: strcpy(doormsg->string, Structs->User.Name); break;
			case 2: strcpy(doormsg->string, Structs->SXUser.password); break;
			case 3:
			case 4:
			case 5:
			case 6: strcpy(doormsg->string, Structs->User.Location); break;
			case 7: strcpy(doormsg->string, "Doors:"); break;
			case 8: strcpy(doormsg->string, "SX:"); break;
			case 9: strcpy(doormsg->string, mydatestr()); break;
			case 10: strcpy(doormsg->string, mytimestr(TRUE)); break;
			}
			break;
		case PG_SF:
			MAXsShowAnsi(doormsg->string);
			break;
		case PG_TM:
			var.lastleft = var.lastleft + doormsg->data;
			break;
		case MAX_CHECKKEY:
			CheckKey();
			doormsg->string[0] = charbuf[0];
			doormsg->string[1] = 0;
			doormsg->data = var.origin-1;
			break;
		case MAX_AUTOINSERT:
			doormsg->string[0] = '%';
			doormsg->string[1] = doormsg->data;
			doormsg->string[2] = 0;
			MAXsPS((char *)&doormsg->string, 2, TRUE);
			strcpy(doormsg->string, buf);
			break;
		case 101:
			DoFunction(1, 0, 0, 0, 255, 0, 0);
			break;
		case 113:
			DoFunction(12, doormsg->string, 1, 0, 255, 0, 0);
			break;
		case 114:
			DoFunction(12, doormsg->string, 0, 0, 255, 0, 0);
			break;
		case 138:
			DoFunction(8, 0, 0, 0, 255, 0, 0);
			break;
		default:
			mysprintf(buf,"PARA: %ld  DATA: %ld\n",doormsg->command,doormsg->data);
			PutStr(buf);
		}
		ReplyMsg((struct Message *)doormsg);
	}
}
}

BOOL MAXsShowAnsi(char *fname)
{
	BPTR fh;
	long size;
	char *ansbuf;

	if(fh = Open(fname, MODE_OLDFILE))
	{
		Seek(fh, 0, OFFSET_END);
		size = Seek(fh, 0, OFFSET_BEGINNING);
		ansbuf = AllocMem(size, MEMF_PUBLIC|MEMF_CLEAR);
		if(ansbuf)
		{
			Read(fh, ansbuf, size);
			MAXsPS(ansbuf, size, FALSE);
			FreeMem(ansbuf, size);
		}
		Close(fh);
		return(TRUE);
	}
	return(FALSE);
}

void MAXsPS(char *ans, long size, BOOL noprint)
{
	long i = 0, j = 0, t;
	char *insert, tbuf[32], *outbuf = AllocMem(size*2, MEMF_PUBLIC|MEMF_CLEAR);
	if(!outbuf) return;

	while(i < size)
	{
		switch(ans[i])
		{
		case 27:
			if(Structs->Disp.flags&(1<<2))
			{
				t = i;
				i += 2;
				while(i<size && ans[i]>46 && ans[i]<60) i++;
				if(ans[i]!='m')
				{
					i = t;
					goto norm;
				}
			}
			goto norm;
		case 10:
			if(ans[i-1]!=13)
			{
				outbuf[j]=13;
				j++;
				outbuf[j]=10;
			}
			break;
		case 13:
			if(ans[i+1]!=10)
			{
				outbuf[j]=13;
				j++;
				outbuf[j]=10;
			}
			break;
		case '%':
			insert=0;
			i++;
			switch(ans[i])
			{
			case 'a':
				insert=Structs->Cfg->Sysop;
				break;
			case 'b':
				mysprintf(tbuf, "%s %s", mytimestr(TRUE), mydatestr());
				insert=(char *)&tbuf;
				break;
			case 'c':
				insert=(char *)mydatestr();
				break;
			case 'd':
				insert=(char *)mytimestr(TRUE);
				break;
			case 'f':
				insert=(char *)&Structs->User.Name;
				break;
			case 'g':
				insert=(char *)&Structs->User.Location;
				break;
			case 'k':
				mysprintf(tbuf, "%ld", Structs->User.Time_Left/60);
				insert=(char *)&tbuf;
				break;
			case 'S':
				mysprintf(tbuf, "%ld", var.node);
				insert=(char *)&tbuf;
				break;
			case 'w':
				mysprintf(tbuf, "%ld", var.baud);
				insert=(char *)&tbuf;
				break;
			case 'X':
				DropDTR(TRUE);
				break;
			case 'Y':
				PSLen(outbuf, j);
				j=-1;
				Delay(49);
				break;
			case 'Z':
				PSLen(outbuf, j);
				j=-1;
				HitReturn();
				break;
			default:
				i--;
				goto norm;
			}
			if(insert)
			{
				if(noprint)
				{
					strcpy(buf, insert);
					FreeMem(outbuf, size*2);
					return;
				}
				PSLen(outbuf, j);
				j=-1;
				PS(insert);
			}
			break;
		default:
norm:
			outbuf[j]=ans[i];
			break;
		}
		i++;
		j++;
	}
	if(j)PSLen(outbuf, j);

	FreeMem(outbuf, size*2);
}
