
/* ================================ */
/* == SX - Message Base Routines == */
/* ================================ */

#include <stdlib.h>
#include <string.h>
#include <exec/memory.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <devices/serial.h>
#include "SXStructs.h"
#include "sx.h"

char Attachment[80];

extern struct StructsStruct	*Structs;
extern struct AmigaStruct	Amiga;
extern struct StringsStruct	*stg;
extern struct VariablesStruct	var;

extern void MoveFile(char *fname, char *dest);

extern UBYTE buf[512], charbuf[128];

struct MsgBaseStruct
{
	char name[24];
	UWORD maxmsgs;
	UBYTE lowaccess;
	UBYTE highaccess;
	UWORD flags;
	char res[66],
	     fidoname[28],
	     fidoorigin[58];
	UWORD AddressID;
	char res2[8];
} *msgbase;


	struct MsgStruct
	{
		char to[32];
		char from[32];
		char subj[72];
		char pass[16];
		char file[32];
		UWORD number;
		UWORD replypo;
		UWORD replycount;
		UWORD thisreply;
		LONG msgtime;
		LONG rectime;
		ULONG flags;	/* BIT 3 = DELETE   BIT 4 = PRIVATE */
		UWORD fidonet_packet_origin_zone;
		UWORD fidonet_packet_origin_netid;
		UWORD fidonet_packet_origin_node;
		UWORD fidonet_packet_origin_point;
		UWORD fidonet_origin_zone;
		UWORD fidonet_origin_netid;
		UWORD fidonet_origin_node;
		UWORD fidonet_origin_point;
		LONG  fidonet_msgid;
		UWORD fidonet_dest_zone;
		UWORD fidonet_dest_netid;
		UWORD fidonet_dest_node;
		UWORD fidonet_dest_point;
		char res[32];
	};

BOOL EnterMsg(char *to, char *subj, UWORD msgarea, UWORD confnum);
void SaveMsg(struct MsgStruct *Msg, UWORD msgarea, UWORD confnum);
void GotoMsgArea(long area);
void ReadMsgs(void);
BOOL ShowMsgHead(struct MsgStruct *Msg, UWORD confnum, UWORD msgarea);
UWORD GetLRP(UWORD confnum, UWORD basenum, WORD which);
void PutLRP(UWORD confnum, UWORD basenum, WORD which, UWORD LRP);
void MailScan(void);
BOOL ScanMsgBase(UWORD confnum, UWORD basenum);
BOOL MsgEditor(char *fname, BOOL reply, struct MsgStruct *Msg, UWORD confnum, UWORD msgarea);

BOOL LineED(char * fname)
{
	BPTR fh;
	BOOL finedit=FALSE, ret;
	char *msgbuf[256];
	int edline=1,i=0;

	PS(Structs->SXStr[62]);

	while(!finedit)
	{
		PSFmt(Structs->SXStr[63], edline);
		LineInput(74, 0, 0);
		if(!var.CARRIER) return(0);
		PSLen("\r\n", 2);
		if(charbuf[0]==0)
		{
			PS(Structs->SXStr[64]);
			LineInput(20,0,0);
 			if(!var.CARRIER) return(0);
			PS("\r\n");
			if(stricmp(charbuf,"A")==0) { finedit=TRUE; ret=FALSE; }
			if(stricmp(charbuf,"S")==0)
			{
				PS(Structs->SXStr[65]);
				fh = Open(fname,MODE_NEWFILE);
				if(fh)
				{
					while(!finedit)
					{
						i++;
						if(i==edline) finedit=TRUE;
						else Write(fh,msgbuf[i],strlen(msgbuf[i]));
					}
					Close(fh);
				}
				PS(Structs->SXStr[66]);
				ret=TRUE;
			}
		} else {
			msgbuf[edline] = AllocMem(82,MEMF_PUBLIC);
			strcpy(msgbuf[edline],charbuf);
			strcat(msgbuf[edline],"\r\n");
			if(edline<255) edline++; else PS("");
		}
	}

	finedit=FALSE;
	i=0;
	while(!finedit)
	{
		i++;
		if(i==edline) finedit=TRUE; else FreeMem(msgbuf[i],82);
	}
	return(ret);
}

BOOL EnterMsg(char *to, char *subj, UWORD msgarea, UWORD confnum)
{
	struct MsgStruct Msg;
	char tbuf[128];
	BPTR fh;

	if(msgarea == 0)
	{
		PS(Structs->SXStr[103]);
		return(0);
	}

	SetActive(28, 0);

	PS(Structs->SXStr[67]);
	memset(&Msg, 0, sizeof(struct MsgStruct));

	if(to && to[0]!=0)
	{
		strcpy(Msg.to, to);
		PS(to);
	} else {
		LineInput(31,0,0);

		if(!var.CARRIER || charbuf[0]==0)
		{
			PSLen("\r\n", 2);
			return(0);
		}

		if(stricmp(charbuf, "SYSOP")!=0 && stricmp(charbuf, "ALL")!=0 && stricmp(charbuf, "EALL")!=0)
		{
			PS(Structs->SXStr[2]);
			if(!(FindAccount(charbuf)))
			{
				PS(Structs->SXStr[128]);
				HitReturn();
				return(FALSE);
			}
		}
		strcpy(Msg.to, charbuf);
	}
     // hier sollte man mit den netzadressen einsteigen
 	if(stricmp(charbuf, "EALL") == 0) PS("[4D.-> EALL <-.");
	if(stricmp(charbuf, "ALL") == 0) PS("[3D.-> ALL <-.");
	if(stricmp(Msg.to, "SYSOP") == 0)
	{
		PSFmt("[5D%s[K", Structs->Cfg->Sysop);
		strcpy(Msg.to, Structs->Cfg->Sysop);
	}

	PSLen("\r\n", 2);

	PS(Structs->SXStr[68]);
	if(subj && subj[0]!=0)
		LineInput(70,0,subj);
	else
		LineInput(70,0,0);
	PSLen("\r\n", 2);

	if(!var.CARRIER || charbuf[0]==0) return(0);

	strcpy(Msg.subj, charbuf);
	strcpy(Msg.from, Structs->UserIndex.handle);
	Msg.msgtime = mytime();

	mysprintf(buf, "%sMessages/Base%ld/Messages.DAT", Structs->Conf[confnum]->path, msgarea);
	fh = Open(buf, MODE_OLDFILE);
	if(fh)
	{
		Seek(fh,0,1);
		Msg.number = (Seek(fh,0,-1)/sizeof(struct MsgStruct)) + 1;
		Close(fh);
	} else
		Msg.number = 1;


	if(stricmp(Msg.to, "EALL") != 0  &&  stricmp(Msg.to, "ALL") != 0)
	{
		mysprintf(tbuf, "%sMsgBases.DAT", Structs->Conf[confnum]->path);
		fh = Open(tbuf, MODE_OLDFILE);
		if(fh)
		{
			struct MsgBaseStruct tbase;
			UWORD i = 0;
			while(Read(fh, &tbase, sizeof(struct MsgBaseStruct)))
			{
				i++;
				if(i == msgarea) break;	
			}
			Close(fh);
			if(tbase.flags&(1<<3))	// IF ASK-PRIVATE AREA
			{
				PS(Structs->SXStr[139]);
				if(YesNo(2, TRUE) == 1) Msg.flags |= 1<<4;
			}
		}
	}

	mysprintf(tbuf, "%sMessages/Base%ld/Msg%ld", Structs->Conf[confnum]->path, msgarea, Msg.number);

	if(subj && subj[0])
	{
		if(MsgEditor(tbuf, TRUE, &Msg, confnum, msgarea))
		{
			SaveMsg(&Msg, msgarea, confnum);
			Structs->User.Messages_Posted++;
			return(1);
		}
	} else {
		if(MsgEditor(tbuf, FALSE, &Msg, confnum, msgarea))
		{
			SaveMsg(&Msg, msgarea, confnum);
			Structs->User.Messages_Posted++;
			return(1);
		}
	}
}

void SaveMsg(struct MsgStruct *Msg, UWORD msgarea, UWORD confnum)
{
	BPTR fh;

	mysprintf(buf, "%sMessages/Base%ld/Messages.DAT", Structs->Conf[confnum]->path, msgarea);
	fh = Open(buf, MODE_READWRITE);
	if(fh)
	{
		Seek(fh, 0, OFFSET_END);
		Write(fh, Msg, sizeof(struct MsgStruct));
		Close(fh);
	}
}

void GotoMsgArea(long area)
{
	UWORD i=0;
	long x;

	if(var.thismsgarea==0) { PS(Structs->SXStr[103]); return; }

	if(area<1)
	{
		mysprintf(buf, "%sMsgBases", Structs->Conf[var.thisconf]->path);
		ShowAnsi(buf, 0, 1, 1, 0, 1, 0);
		LineInput(30, 0, 0);
		if(charbuf[0]==0) return;
		area=atol(charbuf);
	}

	msgbase=(struct MsgBaseStruct *)stg->msgbases;
	while(1)
	{
		i++;
		if(i==area)
		{
			var.thismsgarea=i;
			var.msgareapo=(long)msgbase;
			return;
		}
		if(i >= (var.msgbasessize/sizeof(struct MsgBaseStruct))) return;

		x = (long)msgbase;
		msgbase = (struct MsgBaseStruct *) (x + sizeof(struct MsgBaseStruct));
	}
}

void DoQuote(BPTR infh, BPTR outfh, struct MsgStruct *Msg)
{
	char *inbuf, *outbuf;
	long size, i = 0;

	Seek(infh, 0, OFFSET_END);
	size = Seek(infh, 0, OFFSET_BEGINING);

	inbuf = AllocMem(size, MEMF_PUBLIC);
	if(inbuf)
	{
		outbuf = AllocMem((size*3)+128, MEMF_PUBLIC);
		if(outbuf)
		{
			UWORD po = 0;

			myctime(Msg->msgtime, buf);
			mysprintf(outbuf, "On %s, %s wrote: \r\r> ", buf, Msg->from);
			Write(outfh, outbuf, strlen(outbuf));

			Read(infh, inbuf, size);

			while(i < size)
			{
				if(inbuf[i] == 13  ||  inbuf[i] == 10)
				{
					outbuf[po] = '\r';
					po++;
					outbuf[po] = '>';
					po++;
					outbuf[po] = ' ';
					po++;
				} else {
					outbuf[po] = inbuf[i];
					po++;
				}
				i++;
			}
			outbuf[po] = '\r';
			po++;
			Write(outfh, outbuf, po);
			FreeMem(outbuf, (size*3)+128);
		}
		FreeMem(inbuf, size);
	}
}

void ReadMsgs(void)
{
	struct MsgStruct Msg;
	BPTR fh, infh, outfh;
	BOOL msgfin=FALSE;
	long max, num, x;
	UWORD lrp, i;

	if(var.thismsgarea==0) { PS(Structs->SXStr[103]); return; }

	SetActive(4, 0);

	mysprintf(buf, "%sMessages/Base%ld/Messages.DAT", Structs->Conf[var.thisconf]->path, var.thismsgarea);
	fh = Open(buf, MODE_OLDFILE);
	if(!fh) { PS(Structs->SXStr[96]); return; }

	if(stg->arg[0])
	{
		lrp = atoi(stg->arg);
		Seek(fh, 0, OFFSET_END);
		max = Seek(fh, 0, OFFSET_BEGINING) / sizeof(struct MsgStruct);
		if(lrp > max  ||  lrp < 1)
		{
			PS(Structs->SXStr[142]);
			Close(fh);
			return;
		}
	} else {
		lrp = GetLRP(var.thisconf, var.thismsgarea, 1);
		Seek(fh, 0, 1);
		max = Seek(fh, (lrp-1)*sizeof(struct MsgStruct), OFFSET_BEGINING);
		max = max/sizeof(struct MsgStruct);
	}

	while(!msgfin)
	{
		Seek(fh, (lrp-1)*sizeof(struct MsgStruct), OFFSET_BEGINING);
		Read(fh, &Msg, sizeof(struct MsgStruct));
		if(Msg.flags&(1<<3))
		{
			lrp++;		/* deleted message */
			goto max;
		}
		msgbase = (struct MsgBaseStruct *)var.msgareapo;

		if(msgbase->flags&(1<<3))	// ASK-PRIVATE AREA
		{
			if(Msg.flags&(1<<4))	// PRIVATE MSG
			{
				if(stricmp(Msg.to, Structs->User.Name)!=0 && stricmp(Msg.to, Structs->UserIndex.realname)!=0 && stricmp(Msg.to, "ALL")!=0 && stricmp(Msg.to, "EALL")!=0 && Structs->User.Sec_Status<255)
				{
					lrp++;
					goto max;
				}
			}
		} else {
			if(msgbase->flags&(1<<1)) /* private area */
			{
				if(stricmp(Msg.to, Structs->User.Name)!=0 && stricmp(Msg.to, Structs->UserIndex.realname)!=0 && stricmp(Msg.to, "ALL")!=0 && stricmp(Msg.to, "EALL")!=0 && Structs->User.Sec_Status<255)
				{
					lrp++;
					goto max;
				}
			}
		}

		if(Msg.rectime==0)
		{
			Msg.rectime = mytime();
			Seek(fh, (lrp-1)*sizeof(struct MsgStruct), OFFSET_BEGINING);
			Write(fh, &Msg, sizeof(struct MsgStruct));
		}

		if(ShowMsgHead(&Msg, var.thisconf, var.thismsgarea))
		{
			Seek(fh, (lrp-1)*sizeof(struct MsgStruct), OFFSET_BEGINING);
			Write(fh, &Msg, sizeof(struct MsgStruct));
		}

		mysprintf(buf, "%sMessages/Base%ld/Msg%ld", Structs->Conf[var.thisconf]->path, var.thismsgarea, lrp);
		ShowAnsi(buf, 0, 0, 0, 0, 1, 7);
prompt:
		PSFmt(Structs->SXStr[19], lrp, max);
		LineInput(30, 0, 0);
		num=atoi(charbuf);
		if(num>0 && num<=max)
		{
			lrp=num;
		} else {
			switch(charbuf[0])
			{
			case 'A':
			case 'a':
				break;
			case 'B':
			case 'b':
				lrp--;
				break;
			case 'd':
			case 'D':
				if(stricmp(Msg.to, Structs->User.Name)!=0 && stricmp(Msg.to, Structs->UserIndex.realname)!=0 && Structs->User.Sec_Status<255)
					PS(Structs->SXStr[126]);
				else {
					if(!(Msg.flags&(1<<3)))
					{
						if(Msg.file[0] != 0)
						{
							mysprintf(buf, "%sMessages/Base%ld/%s", Structs->Conf[var.thisconf]->path, var.thismsgarea, Msg.file);
							DeleteFile(buf);
							Msg.file[0] = 0;
						}
						Msg.flags|=1<<3;
						Seek(fh, (lrp-1)*sizeof(struct MsgStruct), OFFSET_BEGINING);
						Write(fh, &Msg, sizeof(struct MsgStruct));
					}
					PS(Structs->SXStr[125]);
				}
				goto prompt;
			case 'r':
			case 'R':
				PS(Structs->SXStr[127]);
				switch(YesNo(1, TRUE))
				{
				case 1:
					mysprintf(buf, "%sMessages/Base%ld/Msg%ld", Structs->Conf[var.thisconf]->path, var.thismsgarea, lrp);
					infh = Open(buf, MODE_OLDFILE);
					if(infh)
					{
						x = Seek(fh, 0, OFFSET_END);
						i = (Seek(fh, x, OFFSET_BEGINING) / sizeof(struct MsgStruct)) + 1;
						mysprintf(buf, "%sMessages/Base%ld/Msg%ld", Structs->Conf[var.thisconf]->path, var.thismsgarea, i);
						outfh = Open(buf, MODE_NEWFILE);
						if(outfh)
						{
							DoQuote(infh, outfh, &Msg);
							Close(outfh);
						}
						Close(infh);
					}
					break;
				}

				if(EnterMsg(Msg.from, Msg.subj, var.thismsgarea, var.thisconf))
				{
					Msg.replycount++;
					Seek(fh, (lrp-1)*sizeof(struct MsgStruct), OFFSET_BEGINING);
					Write(fh, &Msg, sizeof(struct MsgStruct));
				}
				break;
			case 'Q':
			case 'q':
				msgfin=TRUE;
				break;
			case 0:
				lrp++;
				break;
			case '?':
				ShowAnsi("MsgHelp", 1, 1, 1, 1, 0, 0);
				goto prompt;
				break;
			default:
				goto prompt;
			}
		}
		if(Msg.flags&(1<<3))
		{
			mysprintf(buf, "%sMessages/Base%ld/Msg%ld", Structs->Conf[var.thisconf]->path, var.thismsgarea, lrp);
			DeleteFile(buf);
		}
max:
		if(lrp==0) lrp=1;
		if(lrp>max) { msgfin=TRUE; lrp=max; }
	}
	Close(fh);
	PSLen("\r\n", 2);
	PutLRP(var.thisconf, var.thismsgarea, 1, lrp);
}

BOOL ShowMsgHead(struct MsgStruct *Msg, UWORD confnum, UWORD msgarea)
{
	char tbuf[32];
show:
	PS(Structs->SXStr[88]);
	PS(Msg->from);

	PS(Structs->SXStr[89]);
	PS(Msg->to);

	PS(Structs->SXStr[90]);
	myctime(Msg->msgtime, tbuf);
	PS(tbuf);

	PS(Structs->SXStr[91]);
	mysprintf(tbuf, "%ld", Msg->replycount);
	PS(tbuf);

	PS(Structs->SXStr[92]);
	myctime(Msg->rectime, tbuf);
	if(strcmp(tbuf, "none")==0) strcpy(tbuf, "not received");
	PS(tbuf);

	PS(Structs->SXStr[93]);
	if(msgbase && msgbase->name[0]!=0) PS(msgbase->name); else PS("???");

	PS(Structs->SXStr[94]);
	PS(Msg->subj);
	PS(Structs->SXStr[95]);

	if(Msg->file[0] != 0)
	{
		BPTR lck;
		char file[172];

		mysprintf(file, "%sMessages/Base%ld/%s", Structs->Conf[confnum]->path, msgarea, Msg->file);
		lck = Lock(file, ACCESS_READ);
		if(lck)
		{
			char ttbuf[256];

			UnLock(lck);
			mysprintf(ttbuf, Structs->SXStr[132], Msg->file);
			PS(ttbuf);

			switch(YesNo(1, TRUE))
			{
			case 1:
				if(SendFile(file))
				{
					Msg->file[0] = 0;
					DeleteFile(file);
					return(TRUE);
				} else
					goto show;
				break;
			default:
				PS("\r\n\r\n");
			}
		}
	}
	return(FALSE);
}

UWORD GetLRP(UWORD confnum, UWORD basenum, WORD which)
{
	BPTR fh;
	UWORD LRP;
	void *freemem;

	mysprintf(buf, "%sMessages/Base%ld/LRP.DAT", Structs->Conf[confnum]->path, basenum);
	fh = Open(buf, MODE_OLDFILE);
	if(!fh)
	{
		fh = Open(buf, MODE_NEWFILE);
		if(fh)
		{
			freemem = AllocMem(8192, MEMF_PUBLIC|MEMF_CLEAR);
			if(!freemem) { Close(fh); return(1); }
			Write(fh, freemem, 8192);
			FreeMem(freemem, 8192);
		}
		Close(fh);
		if(which) return(1); else return(0);
	}
	if(which==0) Seek(fh, (Structs->User.Slot_Number-1)*4, OFFSET_BEGINING);
	else Seek(fh, ((Structs->User.Slot_Number-1)*4)+2, OFFSET_BEGINING);

	Read(fh, &LRP, 2);
	Close(fh);
	if(LRP==0 && which) LRP=1;
	return(LRP);
}

void PutLRP(UWORD confnum, UWORD basenum, WORD which, UWORD LRP)
{
	BPTR fh;

	mysprintf(buf, "%sMessages/Base%ld/LRP.DAT", Structs->Conf[confnum]->path, basenum);
	fh = Open(buf, MODE_OLDFILE);
	if(fh)
	{
		if(which==0)
			Seek(fh, (Structs->User.Slot_Number-1)*4, OFFSET_BEGINING);
		else
			Seek(fh, ((Structs->User.Slot_Number-1)*4)+2, OFFSET_BEGINING);

		Write(fh, &LRP, 2);
		Close(fh);
	}
}

void MailScan(void)
{
	UWORD cn=0, i;
	long size, x;
	void *checkbase;
	BOOL scanfin;
	BPTR fh;

	SetActive(23, 0);
	PS(Structs->SXStr[97]);

	while(cn < var.confs)
	{
		cn++;

		if(CheckConfAccess(&Structs->SXUser, cn))
		{
			PSFmt(Structs->SXStr[98], Structs->Conf[cn]->name);

			mysprintf(buf, "%sMsgBases.DAT", Structs->Conf[cn]->path);
			fh = Open(buf, MODE_OLDFILE);
			if(fh)
			{
				Seek(fh,0,1);
				size = Seek(fh,0,-1);
				checkbase = AllocMem(size, MEMF_PUBLIC);
				if(checkbase)
				{
					Read(fh, checkbase, size);
					i=0;
					msgbase=(struct MsgBaseStruct *)checkbase;
					scanfin=FALSE;
					while(!scanfin)
					{
						i++;

						PSFmt(Structs->SXStr[99], msgbase->name);

						if(ScanMsgBase(cn, i))
						{
							PS(Structs->SXStr[97]);
							PSFmt(Structs->SXStr[98], Structs->Conf[cn]->name);
						} else
							PS(Structs->SXStr[100]);
	
						if(i >= (size/sizeof(struct MsgBaseStruct)))
						{
							scanfin=TRUE;
						} else {
							x = (long)msgbase;
							msgbase = (struct MsgBaseStruct *) (x + sizeof(struct MsgBaseStruct));
						}
					}
					FreeMem(checkbase, size);
				}
				Close(fh);
			} else {
				PS(Structs->SXStr[102]);
			}
		}
	}
	PS(Structs->SXStr[101]);
}

BOOL ScanMsgBase(UWORD confnum, UWORD basenum)
{
	BPTR fh, infh, outfh;
	long size, x;
	UWORD max, lrp, i;
	BOOL ret=0, readfin=0, msgfin;
	struct MsgStruct Msg;

	mysprintf(buf, "%sMessages/Base%ld/Messages.DAT", Structs->Conf[confnum]->path, basenum);
	fh = Open(buf, MODE_OLDFILE);
	if(fh)
	{
		Seek(fh,0,1);
		size = Seek(fh,0,-1);
		max = size/sizeof(struct MsgStruct);
		lrp = GetLRP(confnum, basenum, 0);
		if(lrp < max)
		{
			Seek(fh, lrp*sizeof(struct MsgStruct), OFFSET_BEGINING);
			while(!readfin)
			{
				lrp++;
				x=Read(fh, &Msg, sizeof(struct MsgStruct));
				if(!x) readfin=TRUE; else
				{
					if(!(Msg.flags&(1<<3))) /* deleted ? */
					{
						if(stricmp(Structs->User.Name, Msg.to)==0 || stricmp(Structs->UserIndex.realname, Msg.to)==0 || stricmp("EALL", Msg.to)==0)
						{
							if(Msg.rectime == 0)
							{
								Msg.rectime = mytime();
								x = Seek(fh, 0, OFFSET_CURRENT);
								Seek(fh, x-sizeof(struct MsgStruct), OFFSET_BEGINING);
								Write(fh, &Msg, sizeof(struct MsgStruct));
							}

	msgfin=0;
	ret=1;
	while(!msgfin)
	{
		if(ShowMsgHead(&Msg, confnum, basenum))
		{
			x = Seek(fh, 0, OFFSET_CURRENT);
			Seek(fh, x-sizeof(struct MsgStruct), OFFSET_BEGINING);
			Write(fh, &Msg, sizeof(struct MsgStruct));
		}
		mysprintf(buf, "%sMessages/Base%ld/Msg%ld", Structs->Conf[confnum]->path, basenum, lrp);
		ShowAnsi(buf, 0, 0, 0, 0, 1, 7);
prompt:
		PSFmt(Structs->SXStr[19], lrp, max);
		LineInput(30, 0, 0);
		switch(charbuf[0])
		{
		case 'A':
		case 'a':
			break;
		case 'b':
		case 'B':
			PS(Structs->SXStr[120]);
			goto prompt;
		case 'd':
		case 'D':
			if(stricmp(Msg.to, Structs->User.Name)!=0 && stricmp(Msg.to, Structs->UserIndex.realname)!=0 && Structs->User.Sec_Status<255)
				PS(Structs->SXStr[126]);
			else {
				if(!(Msg.flags&(1<<3)))
				{
					if(Msg.file[0] != 0)
					{
						mysprintf(buf, "%sMessages/Base%ld/%s", Structs->Conf[confnum]->path, basenum, Msg.file);
						DeleteFile(buf);
						Msg.file[0] = 0;
					}
					Msg.flags|=1<<3;
					Seek(fh, (lrp-1)*sizeof(struct MsgStruct), OFFSET_BEGINING);
					Write(fh, &Msg, sizeof(struct MsgStruct));
				}
				mysprintf(buf, "%sMessages/Base%ld/Msg%ld", Structs->Conf[confnum]->path, basenum, lrp);
				DeleteFile(buf);
				PS(Structs->SXStr[125]);
			}
			break;
		case 'r':
		case 'R':
			PS(Structs->SXStr[127]);
			switch(YesNo(1, TRUE))
			{
			case 1:
				mysprintf(buf, "%sMessages/Base%ld/Msg%ld", Structs->Conf[confnum]->path, basenum, lrp);
				infh = Open(buf, MODE_OLDFILE);
				if(infh)
				{
					x = Seek(fh, 0, OFFSET_END);
					i = (Seek(fh, x, OFFSET_BEGINING) / sizeof(struct MsgStruct)) + 1;
					mysprintf(buf, "%sMessages/Base%ld/Msg%ld", Structs->Conf[confnum]->path, basenum, i);
					outfh = Open(buf, MODE_NEWFILE);
					if(outfh)
					{
						DoQuote(infh, outfh, &Msg);
						Close(outfh);
					}
					Close(infh);
				}
				break;
			}

			if(EnterMsg(Msg.from, Msg.subj, basenum, confnum))
			{
				Msg.replycount++;
				x = Seek(fh, 0, OFFSET_CURRENT);
				Seek(fh, x-sizeof(struct MsgStruct), OFFSET_BEGINING);
				Write(fh, &Msg, sizeof(struct MsgStruct));
			}
			break;
		case 'Q':
		case 'q':
			readfin=TRUE;
		case 0:
			msgfin=TRUE;
			break;
		case '?':
			ShowAnsi("MsgScanHelp", 1, 1, 1, 1, 0, 0);
			goto prompt;
			break;
		default:
			goto prompt;
		}
	}
	PSLen("\r\n", 2);

						}
					}
				}
			}
			PutLRP(confnum, basenum, 0, max);
		}
		Close(fh);
	}

	return(ret);
}

BOOL MsgEditor(char *fname, BOOL reply, struct MsgStruct *Msg, UWORD confnum, UWORD msgarea)
{
	BPTR fh;
	long size = 0, newsize;
	char tbuf[128];

	if(Structs->User.Flags&(1<<5) || Structs->Doors.msgdoor[0]==0)
		return(LineED(fname));
	else {
		Attachment[0] = 0;
		strcpy(charbuf, fname);

		if( msgbase->flags&(1<<2) ) /* disable attachments */
			mysprintf(tbuf, "%s NO", fname);
		else
			mysprintf(tbuf, "%s YES", fname);

		if(reply)
		{
			fh = Open(fname, MODE_OLDFILE);
			if(fh)
			{
				Seek(fh, 0, OFFSET_END);
				size = Seek(fh, 0, 0);
				Close(fh);
			}
			if(Structs->Doors.msgdoorreplytype == 6)
				RunDoor(Structs->Doors.msgdoorreply, Structs->Doors.msgdoorreplytype, 0);
			else
				RunDoor(Structs->Doors.msgdoorreply, Structs->Doors.msgdoorreplytype, tbuf);
		} else {
			if(Structs->Doors.msgdoorreplytype == 6)
				RunDoor(Structs->Doors.msgdoor, Structs->Doors.msgtype, 0);
			else
				RunDoor(Structs->Doors.msgdoor, Structs->Doors.msgtype, tbuf);
		}
		if(Attachment[0] != 0)
		{
			strcpy(Msg->file, FilePart(Attachment));
			//mysprintf(tbuf, "c:move %s %sMessages/Base%ld", Attachment, Structs->Conf[confnum]->path, msgarea);
			//SystemTags(tbuf, TAG_DONE);
			mysprintf(tbuf, "%sMessages/Base%ld", Structs->Conf[confnum]->path, msgarea);
			MoveFile(Attachment, tbuf);
		}
		if(reply)
		{
			fh = Open(fname, MODE_OLDFILE);
			if(fh)
			{
				Seek(fh, 0, OFFSET_END);
				newsize = Seek(fh, 0, 0);
				Close(fh);
				if(newsize != size) return(TRUE); else
				{
					DeleteFile(fname);
					return(FALSE);
				}
			}
			return(FALSE);
		} else {
			fh = Lock(fname, ACCESS_READ);
			if(fh)
			{
				UnLock(fh);
				return(TRUE);
			} else return(FALSE);
		}
	}
}

void MailScanLogin(void)
{
	if(Structs->User.Flags&(1<<2))		/* ask */
	{
		PS(Structs->SXStr[123]);
		if(YesNo(1, TRUE) == 1) { PS("\r\n"); MailScan(); HitReturn(); }
	} else {
		if(Structs->User.Flags&(1<<1)) { MailScan(); HitReturn(); } /* yes */
	}
}
