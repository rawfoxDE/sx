
static const char __version[]="\0$VER: SX-UserED 1.06b [eSSeXX] 68020 ("__DATE__")";

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <exec/types.h>
#include <exec/memory.h>
#include <exec/nodes.h>
#include <exec/lists.h>
#include <intuition/intuition.h>
#include <intuition/screens.h>
#include <intuition/gadgetclass.h>
#include <libraries/gadtools.h>
#include <clib/alib_protos.h>
#include <clib/gadtools_protos.h>
#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>
#include <proto/dos.h>
#include "UserEd_gui.h"
#include "newscan_gui.h"
#include <devices/serial.h>
#include "sxstructs.h"
#include <libraries/reqtools.h>
#include <proto/reqtools.h>


/* typedef a 32-bit type */
#ifdef __alpha
typedef unsigned int UINT4;
#else
typedef unsigned long int UINT4;
#endif

/* Data structure for MD5 (Message-Digest) computation */
typedef struct {
  UINT4 buf[4];                                    /* scratch buffer */
  UINT4 i[2];                   /* number of _bits_ handled mod 2^64 */
  unsigned char in[64];                              /* input buffer */
} MD5_CTX;

extern void MD5Init(MD5_CTX *mdContext);
extern void MD5Update(MD5_CTX *mdContext, unsigned char *bug, unsigned int len);
extern void MD5Final(unsigned char digest[16], MD5_CTX *mdContext);
extern void Transform(UINT4 *buf, UINT4 *in);


struct Library *IntuitionBase, *GfxBase, *GadToolsBase;
struct ReqToolsBase *ReqToolsBase;
struct UserIndexStruct Index;
struct UserData User;
struct SXUserStruct SXUser;
struct AccessStruct AccSet;

char buf[128], firstcall[32], lastcall[32], proto[2], search[32] = "\0";
BOOL done = FALSE;
BPTR ifh, ufh, sfh;
UWORD slot = 1, highest, newuser;


void mysprintf(char *Buffer,char *ctl, ...)
{
	RawDoFmt(ctl, (long *)(&ctl + 1), (void (*))"\x16\xc0\x4e\x75",Buffer);
}

long SearchUser(char *pat)
{
	struct UserIndexStruct *TempIndex;
	char *sbuf, parsepat[256];
	BOOL sfin = FALSE;
	long red, j, curslot = 0, slot = 0;

	ParsePatternNoCase(pat, parsepat, 256);

	sbuf = AllocMem(sizeof(struct UserIndexStruct)*6, MEMF_CLEAR|MEMF_PUBLIC);
	if(sbuf)
	{
		Seek(ifh, 0, OFFSET_BEGINING);
		while(!sfin)
		{
			red = Read(ifh, sbuf, sizeof(struct UserIndexStruct)*6) / sizeof(struct UserIndexStruct);
			j = 0;
			while(j < red)
			{
				j++;
				TempIndex = (struct UserIndexStruct *) ( (long)sbuf + ((j-1)*sizeof(struct UserIndexStruct)) );
				if(!(TempIndex->flags&(1<<6)))
				{
					if(MatchPatternNoCase(parsepat, TempIndex->handle)  ||  MatchPatternNoCase(parsepat, TempIndex->realname))
					{
						slot = curslot + j;
						sfin = TRUE;
						j = red;
					}
				}
			}
			if(red < 6) sfin = TRUE; else curslot += red;
		}
		FreeMem(sbuf, sizeof(struct UserIndexStruct)*6);
	}
	return(slot);
}

void myctime(long secs, char *outbuf)
{
	struct DateStamp *tp;
	struct DateTime tostr;

	tp = __timecvt((time_t)secs);

	memcpy(&tostr, tp, 12);

	tostr.dat_Format  = FORMAT_DOS;
	tostr.dat_StrDay  = "22222%s %s";
	tostr.dat_StrDate = "000000000";
	tostr.dat_StrTime = "!!Unknown";
	tostr.dat_Flags   = 0;

	DateToStr(&tostr);
	if(tostr.dat_StrDate[7] == '8'  ||  secs == 0)
		strcpy(outbuf, "Unknown");
	else
		mysprintf(outbuf, "%s %s", tostr.dat_StrDate, tostr.dat_StrTime);
}

void SaveUser(UWORD workslot)
{
	Seek(ifh, (sizeof(struct UserIndexStruct)*(workslot-1)), OFFSET_BEGINNING);
	Write(ifh, &Index, sizeof(struct UserIndexStruct));

	Seek(ufh, (sizeof(struct UserData)*(workslot-1)), OFFSET_BEGINNING);
	Write(ufh, &User, sizeof(struct UserData));

	Seek(sfh, (sizeof(struct SXUserStruct)*(workslot-1)), OFFSET_BEGINNING);
	Write(sfh, &SXUser, sizeof(struct SXUserStruct));
}

void ShowUser(UWORD workslot, BOOL load)
{
	if(load)
	{
		Seek(ifh, (sizeof(struct UserIndexStruct)*(workslot-1)), OFFSET_BEGINNING);
		Read(ifh, &Index, sizeof(struct UserIndexStruct));

		Seek(ufh, (sizeof(struct UserData)*(workslot-1)), OFFSET_BEGINNING);
		Read(ufh, &User, sizeof(struct UserData));

		Seek(sfh, (sizeof(struct SXUserStruct)*(workslot-1)), OFFSET_BEGINNING);
		Read(sfh, &SXUser, sizeof(struct SXUserStruct));
	}

	GT_SetGadgetAttrs(Project0Gadgets[GD_REAL],Project0Wnd,0,GTST_String,Index.realname,TAG_END);
       	GT_SetGadgetAttrs(Project0Gadgets[GD_ALIAS],Project0Wnd,0,GTST_String,Index.handle,TAG_END);
       	GT_SetGadgetAttrs(Project0Gadgets[GD_LOC],Project0Wnd,0,GTST_String,User.Location,TAG_END);
       	GT_SetGadgetAttrs(Project0Gadgets[GD_PHONE],Project0Wnd,0,GTST_String,User.PhoneNumber,TAG_END);
       	GT_SetGadgetAttrs(Project0Gadgets[GD_SBL],Project0Wnd,0,GTST_String,SXUser.sentbyline,TAG_END);
       	GT_SetGadgetAttrs(Project0Gadgets[GD_COMP],Project0Wnd,0,GTST_String,SXUser.computer,TAG_END);
       	GT_SetGadgetAttrs(Project0Gadgets[GD_SEC],Project0Wnd,0,GTIN_Number,User.Sec_Status,TAG_END);
	proto[0] = User.Protocol;
       	GT_SetGadgetAttrs(Project0Gadgets[GD_PROTO],Project0Wnd,0,GTST_String,proto,TAG_END);
       	GT_SetGadgetAttrs(Project0Gadgets[GD_TIMEUSED],Project0Wnd,0,GTIN_Number,User.Time_Used,TAG_END);
       	GT_SetGadgetAttrs(Project0Gadgets[GD_TIMELIMIT],Project0Wnd,0,GTIN_Number,User.Time_Limit,TAG_END);
       	GT_SetGadgetAttrs(Project0Gadgets[GD_BYTESDOWN],Project0Wnd,0,GTIN_Number,User.Bytes_Download,TAG_END);
       	GT_SetGadgetAttrs(Project0Gadgets[GD_BYTESUP],Project0Wnd,0,GTIN_Number,User.Bytes_Upload,TAG_END);
       	GT_SetGadgetAttrs(Project0Gadgets[GD_FILESDOWN],Project0Wnd,0,GTIN_Number,User.Downloads,TAG_END);
       	GT_SetGadgetAttrs(Project0Gadgets[GD_FILESUP],Project0Wnd,0,GTIN_Number,User.Uploads,TAG_END);
       	GT_SetGadgetAttrs(Project0Gadgets[GD_FREEBYTES],Project0Wnd,0,GTIN_Number,SXUser.freebytes,TAG_END);
       	GT_SetGadgetAttrs(Project0Gadgets[GD_FREEFILES],Project0Wnd,0,GTIN_Number,SXUser.freefiles,TAG_END);
	myctime(SXUser.firstcall, firstcall);
       	GT_SetGadgetAttrs(Project0Gadgets[GD_FIRSTCALL],Project0Wnd,0,GTTX_Text,firstcall,TAG_END);
	myctime(User.Time_Last_On, lastcall);
       	GT_SetGadgetAttrs(Project0Gadgets[GD_LASTCALL],Project0Wnd,0,GTTX_Text,lastcall,TAG_END);
       	GT_SetGadgetAttrs(Project0Gadgets[GD_FILERATIO],Project0Wnd,0,GTIN_Number,SXUser.fileratio,TAG_END);
       	GT_SetGadgetAttrs(Project0Gadgets[GD_BYTERATIO],Project0Wnd,0,GTIN_Number,SXUser.byteratio,TAG_END);
}

void PutAccessSet(void)
{
	User.Time_Left = AccSet.Time_Limit;
	User.Time_Limit = AccSet.Time_Limit;
	User.Sec_Status = AccSet.access;
	SXUser.freefiles = AccSet.freefiles;
	SXUser.freebytes = AccSet.freebytes;
	SXUser.fileratio = AccSet.fileratio;
	SXUser.byteratio = AccSet.byteratio;
	memcpy(&SXUser.ConfAccess[0], &AccSet.ConfAccess[0], 40);
}

BOOL GetAccessSet(UWORD access)
{
	BPTR fh = Open("SX:Prefs/Access.DAT", MODE_OLDFILE);
	if(fh)
	{
		while(1)
		{
			if(Read(fh, &AccSet, sizeof(struct AccessStruct)) == 0)
			{
				Close(fh);
				return(FALSE);
			}
			if((AccSet.access == access))
			{
				Close(fh);
				return(TRUE);
			}
		}
	}
	return(FALSE);
}

void Presets(struct SXUserStruct *TempSXUser, struct UserData *TempUser)
{
	BPTR fh;
	UWORD i = 0;
	ULONG out, set = TempUser->Sec_Status;

	fh = Open("SX:Prefs/Access.DAT", MODE_OLDFILE);
	if(fh)
	{
		out = Open("con:0/0/300/200/Access_Sets", MODE_NEWFILE);
		if(out)
		{
			while(Read(fh, &AccSet, sizeof(struct AccessStruct)))
			{
				i++;
				mysprintf(buf, "	%003ld) %s\n", AccSet.access, AccSet.name);
				Write(out, buf, strlen(buf));
			}
			Close(fh);
			fh = 0;
			if(rtGetLong(&set, "Select Access Set:", NULL, TAG_END))
			{
				if(GetAccessSet(set)) PutAccessSet();
			}
			Close(out);
		}
		if(fh) Close(fh);
	}
}

void upcase(char * str)
{
	int j=0, i=strlen(str);
	while(j!=(i+1))
	{
		str[j]=toupper(str[j]);
		j++;
	}
}

void Password(void)
{
	MD5_CTX ctx;
	char pass[32];

	mysprintf(buf, "Enter new password for %s:", User.Name);
	if(rtGetString(pass, 32, buf, 0, 0))
	{
		upcase(pass);
		MD5Init(&ctx);
		MD5Update(&ctx, pass, strlen(pass));
		MD5Final(SXUser.password, &ctx);
	}
}

void LongToStr(ULONG num, char *str)
{
	int i=0;
	while(i<32)
	{
		if(num&(1L<<i)) str[i]='X'; else str[i]='_';
		i++;
	}
	str[i]=0;
}

ULONG StrToLongg(char *str)
{
	int i=0;
	ULONG num=0;

	while(i<32)
	{
		if(str[i]=='X' || str[i]=='x') num^=1L<<i;
		i++;
	}
	return(num);
}

void ConfAccess(void)
{
	long ret = rtEZRequestTags("Edit which range?", "01-32|33-64|65-96|Cancel", NULL, NULL, TAG_DONE);

	switch(ret)
	{
	case 0:
		return;
	case 1:
		LongToStr(SXUser.ConfAccess[0], buf);
		break;
	case 2:
		LongToStr(SXUser.ConfAccess[1], buf);
		break;
	case 3:
		LongToStr(SXUser.ConfAccess[2], buf);
	}

	if(rtGetString(buf, 32, "Edit Conference Access:", 0, 0))
	{
		switch(ret)
		{
		case 1:
			SXUser.ConfAccess[0] = StrToLongg(buf);
			break;
		case 2:
			SXUser.ConfAccess[1] = StrToLongg(buf);
			break;
		case 3:
			SXUser.ConfAccess[2] = StrToLongg(buf);
		}
	}
}

void NextNew(void)
{
	UWORD tempslot;

	if(!Project1Wnd) return;

	rtSetWaitPointer(Project1Wnd);
	rtSetWaitPointer(Project0Wnd);

	tempslot = slot+1;
	Seek(ufh, (sizeof(struct UserData)*(tempslot-1)), OFFSET_BEGINNING);

	while(1)
	{
		if(Read(ufh, &User, sizeof(struct UserData)))
		{
			if(User.Sec_Status == newuser)
			{
				ClearPointer(Project1Wnd);
				ClearPointer(Project0Wnd);
				slot = tempslot;
				ShowUser(slot, TRUE);
				return;
			}
		} else {
			rtEZRequestTags("No new users left.", "Ok", NULL, NULL, TAG_DONE);
			ClearPointer(Project1Wnd);
			ClearPointer(Project0Wnd);
			return;
		}
		tempslot++;
	}
}

void StartNewScan(void)
{
	BPTR fh;

	if(Project1Wnd)
	{
		CloseProject1Window();
		Project1Wnd = 0;
	} else {
		OpenProject1Window();
		fh = Open("SX:Prefs/Access.DAT", MODE_OLDFILE);
		if(fh)
		{
			Read(fh, &AccSet, sizeof(struct AccessStruct));
			Close(fh);
			newuser = AccSet.access;
		} else {
			CloseProject1Window();
			Project1Wnd = 0;
		}
	}
}

void DeleteUser(void)
{
	if(Index.flags&(1<<6))
	{
	       	GT_SetGadgetAttrs(Project0Gadgets[GD_LOC],Project0Wnd,0,GTST_String,User.Location,TAG_END);
		if(User.Flags&(1<<6)) User.Flags ^= 1<<6;
		if(Index.flags&(1<<6)) Index.flags ^= 1<<6;
		rtEZRequestTags("User has been undeleted.", "Ok", NULL, NULL, TAG_DONE);
	} else {

		mysprintf(buf, "Delete %s/%s ?", Index.realname, Index.handle);

		if(rtEZRequestTags(buf, "Delete|Cancel", NULL, NULL, TAG_DONE))
		{
	       		GT_SetGadgetAttrs(Project0Gadgets[GD_LOC],Project0Wnd,0,GTST_String,"-*- DELETED -*-",TAG_END);
			User.Flags |= 1<<6;
			Index.flags |= 1<<6;
			rtEZRequestTags("Deleted!\n\nClick 'Save Current Account'\nto make changes perminent.", "Ok", NULL, NULL, TAG_DONE);
		}
	}
}

void GetWinMsg(void)
{
	struct IntuiMessage *message;
	struct Gadget *gad;
	UWORD code;
	BOOL finscan = FALSE;

	if(Project1Wnd)
	{
		while (message = GT_GetIMsg(Project1Wnd->UserPort))
		{
			gad   = (struct Gadget *)message->IAddress;
			code  = message->Code;
			GT_ReplyIMsg(message);
			switch (message->Class)
			{
			case IDCMP_GADGETUP:
				switch(gad->GadgetID)
				{
				case GD_FINDNEXT:
					NextNew();
					break;
				case GD_QUITNEWSCAN:
					finscan = TRUE;
				}
				break;
			case IDCMP_CLOSEWINDOW:
				finscan = TRUE;
			}
		}
	}
	if(finscan)
	{
		CloseWindow(Project1Wnd);
		Project1Wnd = 0;
	}

	while (message = GT_GetIMsg(Project0Wnd->UserPort))
	{
	        gad   = (struct Gadget *)message->IAddress;
		code  = message->Code;
		GT_ReplyIMsg(message);
		switch (message->Class)
		{
		case IDCMP_RAWKEY:
			switch(code)
			{
			case 207:
				if(slot > 1)
				{
					slot--;
					ShowUser(slot, TRUE);
				} else DisplayBeep(Scr);
				break;
			case 206:
				if(slot < highest)
				{
					slot++;
					ShowUser(slot, TRUE);
				} else DisplayBeep(Scr);
			}
			break;
		case IDCMP_GADGETUP:
			switch(gad->GadgetID)
			{
			case GD_PREV:
				if(slot > 1)
				{
					slot--;
					ShowUser(slot, TRUE);
				} else DisplayBeep(Scr);
				break;
			case GD_NEXT:
				if(slot < highest)
				{
					slot++;
					ShowUser(slot, TRUE);
				} else DisplayBeep(Scr);
				break;
			case GD_SEARCH:
				if(rtGetString(search, 32, "Enter Search Pattern:", 0, 0))
				{
					code = SearchUser(search);
					if(code)
					{
						slot = code;
						ShowUser(slot, TRUE);
					} else {
						rtEZRequestTags("No match found.", "Ok", NULL, NULL, TAG_DONE);
					}
				}
				break;
			case GD_SAVE:
				SaveUser(slot);
				break;
			case GD_QUIT:
				done = TRUE;
				break;
			case GD_SETS:
				Presets(&SXUser, &User);
				ShowUser(slot, FALSE);
				break;
			case GD_REAL:
				strcpy(Index.realname, GetString(gad));
				break;
			case GD_ALIAS:
				strcpy(Index.handle, GetString(gad));
				strcpy(User.Name, Index.handle);
				break;
			case GD_PHONE:
				strcpy(User.PhoneNumber, GetString(gad));
				break;
			case GD_LOC:
				strcpy(User.Location, GetString(gad));
				break;
			case GD_COMP:
				strcpy(SXUser.computer, GetString(gad));
				break;
			case GD_SBL:
				strcpy(SXUser.sentbyline, GetString(gad));
				break;
			case GD_SEC:
				User.Sec_Status = GetNumber(gad);
				break;
			case GD_TIMELIMIT:
				User.Time_Limit = GetNumber(gad);
				break;
			case GD_TIMEUSED:
				User.Time_Used = GetNumber(gad);
				break;
			case GD_PROTO:
				User.Protocol = (((struct StringInfo * )gad->SpecialInfo)->Buffer[0]);
				break;
			case GD_BYTESDOWN:
				User.Bytes_Download = GetNumber(gad);
				break;
			case GD_BYTESUP:
				User.Bytes_Upload = GetNumber(gad);
				break;
			case GD_FILESDOWN:
				User.Downloads = GetNumber(gad);
				break;
			case GD_FILESUP:
				User.Uploads = GetNumber(gad);
				break;
			case GD_FREEBYTES:
				SXUser.freebytes = GetNumber(gad);
				break;
			case GD_FREEFILES:
				SXUser.freefiles = GetNumber(gad);
				break;
			case GD_FILERATIO:
				SXUser.fileratio = GetNumber(gad);
				break;
			case GD_BYTERATIO:
				SXUser.byteratio = GetNumber(gad);
				break;
			case GD_PASS:
				Password();
				break;
			case GD_CONFACCESS:
				ConfAccess();
				break;
			case GD_NEWSCAN:
				StartNewScan();
				break;
			case GD_DEL:
				DeleteUser();
				break;
			}
			break;
		case IDCMP_CLOSEWINDOW:
			done = TRUE;
		}
	}
}

void main(void)
{
	IntuitionBase = OpenLibrary("intuition.library", 37);
	if(IntuitionBase)
	{
		GfxBase = OpenLibrary("graphics.library", 37);
		GadToolsBase = OpenLibrary("gadtools.library", 37);

 		ReqToolsBase = (struct ReqToolsBase *)OpenLibrary (REQTOOLSNAME, REQTOOLSVERSION);
		if(ReqToolsBase)
		{
			if(SetupScreen()==0)
			{
				if(OpenProject0Window()==0)
				{
					ifh = Open("SX:User.Index", MODE_OLDFILE);
					if(ifh)
					{
						Seek(ifh, 0, OFFSET_END);
						highest = Seek(ifh, 0, OFFSET_BEGINING) / sizeof(struct UserIndexStruct);
						ufh = Open("SX:User.Data", MODE_OLDFILE);
						if(ufh)
						{
							sfh = Open("SX:User.SX", MODE_OLDFILE);
							if(sfh)
							{
								proto[1] = 0;
								ShowUser(slot, TRUE);
								while(!done)
								{
									if(Project1Wnd)
										Wait(1L<<Project1Wnd->UserPort->mp_SigBit | 1L<<Project0Wnd->UserPort->mp_SigBit);
									else
										WaitPort(Project0Wnd->UserPort);
									GetWinMsg();
								}
								Close(sfh);
							}
							Close(ufh);
						}
						Close(ifh);
					}
					if(Project1Wnd) CloseProject1Window();
					CloseProject0Window();
				}
				CloseDownScreen();
			}
			CloseLibrary((struct Library *)ReqToolsBase);
		}

		CloseLibrary(GadToolsBase);
		CloseLibrary(GfxBase);
		CloseLibrary(IntuitionBase);
	}
}
