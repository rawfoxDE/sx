
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <exec/memory.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <dos/dos.h>
#include <devices/serial.h>
#include <time.h>
#include "SXStructs.h"
#include "sx.h"

extern struct StructsStruct	*Structs;
extern struct AmigaStruct	Amiga;
extern struct StringsStruct	*stg;
extern struct VariablesStruct	var;

extern UBYTE buf[512], charbuf[128];

int FindAccount(char * searchstr);
int FindAccess(UWORD access);
void SaveAccount(void);
void LoadAccount(int i, BOOL index);
void ConfAccessEdit(struct SXUserStruct *TempSXUser);
ULONG StrToLongg(char *str);
void Presets(struct SXUserStruct *TempSXUser, struct UserData *TempUser);

void EditUser(BOOL remote)
{
	struct UserIndexStruct TempUserIndex;
	struct UserData TempUser;
	struct SXUserStruct TempSXUser;
	int slot;
	BPTR ifh;
	BOOL editdone = FALSE, OLDSER = var.SER;
	var.edit = TRUE;
	SetActive(6, 0);

	if(!remote && var.CARRIER) { PS(Structs->SXStr[41]); var.SER=FALSE; }
startedit:
	ShowAnsi("SX:TXT/OnlineEdit.TXT", 0, 0, 0, 0, 0, 0);
	if(var.USER)SaveAccount();
	LineInput(10,0,0);
	PS("\r\n");
	if(charbuf[0]==0) goto xit;

	slot = Structs->User.Slot_Number;

	if(stricmp(charbuf,"S")==0)
	{
		PS("\r\n\r\nEnter name to find: ");
		LineInput(40,0,0);
		PS("\r\n");
		slot = FindAccount(charbuf);
		if(slot==0)
		{
			PS("\r\n\r\nNo match found.\r\n");
			goto xit;
		}
	}
	if(stricmp(charbuf,"#")==0)
	{
		PS("\r\n\r\nEnter slot number: ");
		LineInput(40,0,0);
		PS("\r\n");
		if(charbuf[0]==0) goto xit;
		slot = atoi(charbuf);
	}
	if(stricmp(charbuf, "A")==0)
	{
		PS("\r\n\r\nEnter access level: ");
		LineInput(10,0,0);
		PS("\r\n");
		if(charbuf[0]==0) goto xit;
		slot = FindAccess(atoi(charbuf));
	}

	if(ifh = Open("SX:User.Index",MODE_OLDFILE)) {
		Seek(ifh,(sizeof(struct UserIndexStruct)*(slot-1)),OFFSET_BEGINNING);
		Read(ifh,&TempUserIndex,sizeof(struct UserIndexStruct));
		Close(ifh);
	}
	if(ifh = Open("SX:User.Data",MODE_OLDFILE)) {
		Seek(ifh,(sizeof(struct UserData)*(slot-1)),OFFSET_BEGINNING);
		Read(ifh,&TempUser,sizeof(struct UserData));
		Close(ifh);
	}
	if(ifh = Open("SX:User.SX",MODE_OLDFILE)) {
		Seek(ifh,(sizeof(struct SXUserStruct)*(slot-1)),OFFSET_BEGINNING);
		Read(ifh,&TempSXUser,sizeof(struct SXUserStruct));
		Close(ifh);
	}
redraw:

	if(TempUser.Flags&(1<<6) || TempUserIndex.flags&(1<<6))
		PSFmt("  [31mDELETED [0m[%ld]\r\n",TempUser.Slot_Number);
	else
		PSFmt("  [33mACTIVE [0m[%ld]\r\n",TempUser.Slot_Number);

	PSFmt(	"[33mA> [32mName[36m: [0m%-29s[33mB> [32mReal[36m: [0m%-30s\r\n"
		"[33mC> [32mLoc.[36m: [0m%-29s[33mD> [32mPass ...[36m: [0mENCRYPTED\r\n",TempUser.Name,TempUserIndex.realname,TempUser.Location);
	PSFmt(	"[33mE> [32mPhn#[36m: [0m%-29s[33mF> [32mSec Lvl.[36m: [0m%ld\r\n"
		"[33mG> [32mComp[36m: [0m%-29s[33mH> [32mCalls...[36m: [0m%ld\r\n",TempUser.PhoneNumber,TempUser.Sec_Status,TempSXUser.computer,TempUser.Times_Called);
	PSFmt(	"[33mI> [32mByte Ratio[36m: [0m%-23ld[33mJ> [32mFile Ratio[36m: [0m%ld\r\n"
		"[33mK> [32mDownloads.[36m: [0m%-23ld[33mL> [32mBytes Down[36m: [0m%ld\r\n",TempSXUser.byteratio,TempSXUser.fileratio,TempUser.Downloads,TempUser.Bytes_Download);
	PSFmt(	"[33mM> [32mUploads...[36m: [0m%-23ld[33mN> [32mBytes Up..[36m: [0m%ld\r\n"
		"[33mO> [32mByte Limit[36m: [0m%-23ld[33mQ> [32mDown Today[36m: [0m%ld\r\n",TempUser.Uploads,TempUser.Bytes_Upload,TempUser.Daily_Bytes_Limit,TempUser.Daily_Bytes_Dld);
	PSFmt(	"[33mR> [32mAutoReJoin[36m: [0m%-23ld[33mS> [32mMessages..[36m: [0m%ld\r\n"
		"[33mT> [32mTime Limit[36m: [[0m%-8ld[36m][0m mins        [33mU> [32mFree Bytes[36m: [0m%ld\r\n",TempUser.ConfRJoin,TempUser.Messages_Posted,TempUser.Time_Limit/60, TempSXUser.freebytes);
	PSFmt("[33mV> [32mTime Left.[36m: [[0m%-8ld[36m][0m mins        [33mW> [32mFree Files[36m: [0m%ld",TempUser.Time_Left/60, TempSXUser.freefiles);
	PS(	"\r\n\r\n[33mX  [36m=[0mEXIT-NOSAVE [33m~[36m=[0mSAVE  [33mP[36m=[0mPRESETS  [33m9[36m=[0mUN-DELETE"
		"\r\n[33mTAB[36m=[0mCONT [33m@[36m=[0mCONFERENCE ACCESS  [33mDEL[36m=[0mDELETE\r\n");
	while(!editdone)
	{
		PS("[17H");
		MyHotKey();
		if(!var.CARRIER) goto xit;
		switch(charbuf[0])
		{
		case '9':
			if(TempUser.Flags&(1<<6)) TempUser.Flags ^= 1<<6;
			if(TempUserIndex.flags&(1<<6)) TempUserIndex.flags ^= 1<<6;
			goto redraw;
		case 127:
			PS("\r\nDelete this user (y/N)? ");
			switch(YesNo(2, FALSE))
			{
			case 0:
				PS("No");
				goto redraw;
			case 1:
				TempUser.Flags |= 1<<6;
				TempUserIndex.flags |= 1<<6;
				PS("Yes\r\n\r\n[0;31mUser Marked as Deleted![0m Select SAVE to make change perminent.\r\n\r\n<Hit Return>");
				MyHotKey();
				goto redraw;
			}
			break;
		case 'P':
		case 'p':
			Presets(&TempSXUser, &TempUser);
			goto redraw;
		case '@':
			ConfAccessEdit(&TempSXUser);
			goto redraw;
		case 9:
			goto startedit;
		case 'x':
		case 'X':
			editdone=TRUE;
			PS("\r\n");
			break;
		case '~':
			editdone=TRUE;
			PS("\r\nSaving...");
			if(ifh = Open("SX:User.Index",MODE_OLDFILE)) {
				Seek(ifh,(sizeof(struct UserIndexStruct)*(slot-1)),OFFSET_BEGINNING);
				Write(ifh,&TempUserIndex,sizeof(struct UserIndexStruct));
				Close(ifh);
			}
			if(ifh = Open("SX:User.Data",MODE_OLDFILE)) {
				Seek(ifh,(sizeof(struct UserData)*(slot-1)),OFFSET_BEGINNING);
				Write(ifh,&TempUser,sizeof(struct UserData));
				Close(ifh);
			}
			if(ifh = Open("SX:User.SX",MODE_OLDFILE)) {
				Seek(ifh,(sizeof(struct SXUserStruct)*(slot-1)),OFFSET_BEGINNING);
				Write(ifh,&TempSXUser,sizeof(struct SXUserStruct));
				Close(ifh);
			}
			if(slot==Structs->User.Slot_Number) LoadAccount(slot, TRUE);
			if(var.CALLER && var.USER)
			{
				CalcTimeLeft();
				UpdateUserInfo();
				UpdateTimeLeft();
			}
			break;
		case 'A':
		case 'a':
			PS("[2;10H");
			LineInput(29,0,TempUser.Name);
			strcpy(TempUser.Name,charbuf);
			strcpy(TempUserIndex.handle,charbuf);
			break;
		case 'B':
		case 'b':
			PS("[2;48H");
			LineInput(30,0,TempUserIndex.realname);
			strcpy(TempUserIndex.realname,charbuf);
			break;
		case 'C':
		case 'c':
			PS("[3;10H");
			LineInput(29,0,TempUser.Location);
			strcpy(TempUser.Location,charbuf);
			break;
		case 'D':
		case 'd':
			PS("[3;52H[K");
			LineInput(24,0,0);
			if(charbuf[0]!=0)
			{
				upcase(charbuf);
				Servermsg(8);
				memcpy(TempSXUser.password,buf,16);
			} else {
				PS("[3;52HENCRYPTED");
			}
			break;
		case 'E':
		case 'e':
			PS("[4;10H");
			LineInput(13,0,TempUser.PhoneNumber);
			strcpy(TempUser.PhoneNumber,charbuf);
			break;
		case 'F':
		case 'f':
			PS("[4;52H");
			mysprintf(buf,"%ld",TempUser.Sec_Status);
			LineInput(5,0,buf);
			TempUser.Sec_Status=atoi(charbuf);
			break;
		case 'G':
		case 'g':
			PS("[5;10H");
			LineInput(13,0,TempSXUser.computer);
			strcpy(TempSXUser.computer,charbuf);
			break;
		case 'H':
		case 'h':
			PS("[5;52H");
			mysprintf(buf,"%ld",TempUser.Times_Called);
			LineInput(16,0,buf);
			TempUser.Times_Called=atoi(charbuf);
			break;
		case 'I':
		case 'i':
			PS("[6;16H");
			mysprintf(buf,"%ld",TempSXUser.byteratio);
			LineInput(5,0,buf);
			TempSXUser.byteratio=atoi(charbuf);
			break;
		case 'J':
		case 'j':
			PS("[6;54H");
			mysprintf(buf,"%ld",TempSXUser.fileratio);
			LineInput(5,0,buf);
			TempSXUser.fileratio=atoi(charbuf);
			break;
		case 'k':
		case 'K':
			PS("[7;16H");
			mysprintf(buf,"%ld",TempUser.Downloads);
			LineInput(12,0,buf);
			TempUser.Downloads=atoi(charbuf);
			break;
		case 'l':
		case 'L':
			PS("[7;54H");
			mysprintf(buf,"%ld",TempUser.Bytes_Download);
			LineInput(16,0,buf);
			TempUser.Bytes_Download=atoi(charbuf);
			break;
		case 'M':
		case 'm':
			PS("[8;16H");
			mysprintf(buf,"%ld",TempUser.Uploads);
			LineInput(12,0,buf);
			TempUser.Uploads=atoi(charbuf);
			break;
		case 'N':
		case 'n':
			PS("[8;54H");
			mysprintf(buf,"%ld",TempUser.Bytes_Upload);
			LineInput(16,0,buf);
			TempUser.Bytes_Upload=atoi(charbuf);
			break;
		case 'o':
		case 'O':
			PS("[9;16H");
			mysprintf(buf,"%ld",TempUser.Daily_Bytes_Limit);
			LineInput(16,0,buf);
			TempUser.Daily_Bytes_Limit=atoi(charbuf);
			break;
		case 'Q':
		case 'q':
			PS("[9;54H");
			mysprintf(buf,"%ld",TempUser.Daily_Bytes_Dld);
			LineInput(16,0,buf);
			TempUser.Daily_Bytes_Dld=atoi(charbuf);
			break;
		case 'R':
		case 'r':
			PS("[10;16H");
			mysprintf(buf,"%ld",TempUser.ConfRJoin);
			LineInput(16,0,buf);
			TempUser.ConfRJoin=atoi(charbuf);
			break;
		case 'S':
		case 's':
			PS("[10;54H");
			mysprintf(buf,"%ld",TempUser.Messages_Posted);
			LineInput(16,0,buf);
			TempUser.Messages_Posted=atoi(charbuf);
			break;
		case 't':
		case 'T':
			PS("[11;17H");
			mysprintf(buf,"%ld",TempUser.Time_Limit/60);
			LineInput(8,0,buf);
			TempUser.Time_Limit=atoi(charbuf)*60;
			break;
		case 'u':
		case 'U':
			PS("[11;54H");
			mysprintf(buf,"%ld",TempSXUser.freebytes);
			LineInput(16,0,buf);
			TempSXUser.freebytes=atol(charbuf);
			break;
		case 'V':
		case 'v':
			PS("[12;17H");
			mysprintf(buf,"%ld",TempUser.Time_Left/60);
			LineInput(8,0,buf);
			TempUser.Time_Left=(atoi(charbuf)*60)+1;
			var.lastleft = TempUser.Time_Left;
			break;
		case 'W':
		case 'w':
			PS("[12;54H");
			mysprintf(buf,"%ld",TempSXUser.freefiles);
			LineInput(16,0,buf);
			TempSXUser.freefiles=atol(charbuf);
			break;
		}
	}
xit:
	var.SER = OLDSER;
	var.edit = FALSE;
	WaitBuf();
	if(!remote) PS(Structs->SXStr[42]);
}

int FindAccount(char * searchstr)
{
	long x, i = 0;
	struct UserIndexStruct TempUserIndex;
	BOOL fin = FALSE;
	BPTR fh = Open("SX:User.Index", MODE_OLDFILE);
	if(!fh) return(0);

	while(!fin)
	{
		i++;
		x = Read(fh, &TempUserIndex, sizeof(struct UserIndexStruct));
		if(!x)
			fin = TRUE;
		else {
			if(!(TempUserIndex.flags&(1<<6))) /* not deleted? */
			{
				if(stricmp(TempUserIndex.handle,searchstr)==0 || stricmp(TempUserIndex.realname,searchstr)==0)
				{
					Close(fh);
					return(i);
				}
			}
		}
	}
	Close(fh);
	return(0);
}

int FindAccess(UWORD access)
{
	long x, i = 0;
	BOOL fin=FALSE;
	BPTR fh = Open("SX:User.Data", MODE_OLDFILE);
	struct UserData TempUser;

	if(fh)
	{
		while(!fin)
		{
			i++;
			x=Read(fh, &TempUser, sizeof(struct UserData));
			if(!x)fin=TRUE;
			if(TempUser.Sec_Status == access)
			{
				Close(fh);
				return(i);
			}
		}
		Close(fh);
	}
	return(0);
}

void SaveAccount(void)
{
	BPTR fh;
	if(var.EXPERT)Structs->User.Expert=1; else Structs->User.Expert=0;

	fh = Open("SX:User.Data",MODE_READWRITE);
	Seek(fh,(sizeof(struct UserData)*(Structs->User.Slot_Number-1)),OFFSET_BEGINNING);
	Write(fh,&Structs->User,sizeof(struct UserData));
	Close(fh);

	fh = Open("SX:User.Index",MODE_READWRITE);
	Seek(fh,(sizeof(struct UserIndexStruct)*(Structs->User.Slot_Number-1)),OFFSET_BEGINNING);
	Write(fh,&Structs->UserIndex,sizeof(struct UserIndexStruct));
	Close(fh);

	fh = Open("SX:User.SX",MODE_READWRITE);
	Seek(fh,(sizeof(struct SXUserStruct)*(Structs->User.Slot_Number-1)),OFFSET_BEGINNING);
	Write(fh,&Structs->SXUser,sizeof(struct SXUserStruct));
	Close(fh);
}

void LoadAccount(int i, BOOL index)
{
	BPTR ifh;
	if(ifh = Open("SX:User.Data", MODE_OLDFILE)) {
		Seek(ifh,(sizeof(struct UserData)*(i-1)),OFFSET_BEGINNING);
		Read(ifh,&Structs->User,sizeof(struct UserData));
		Close(ifh);
	}
	if(ifh = Open("SX:User.SX", MODE_OLDFILE)) {
		Seek(ifh,(sizeof(struct SXUserStruct)*(i-1)),OFFSET_BEGINNING);
		Read(ifh,&Structs->SXUser,sizeof(struct SXUserStruct));
		Close(ifh);
	}
	if(index)
	{
		if(ifh = Open("SX:User.Index", MODE_OLDFILE)) {
			Seek(ifh, (sizeof(struct UserIndexStruct)*(i-1)), OFFSET_BEGINNING);
			Read(ifh, &Structs->UserIndex, sizeof(struct UserIndexStruct));
			Close(ifh);
		}
	}
}

void ConfAccessEdit(struct SXUserStruct *TempSXUser)
{
redraw:
	PS("[0m\r\n[33m1>[32m Access 01-32[36m: [0m");
	ConfAccess(TempSXUser, buf, 32);
	PS(buf);

	PS("\r\n[33m2>[32m Access 33-64[36m: [0m");
	ConfAccess(TempSXUser, buf, 64);
	PS((char *)&buf[32]);

	PS("\r\n\r\n[33mU[36m = [0mUSE    [33mC[36m = [0mCANCEL\r\n\r\n");

	while(1)
	{
		MyHotKey();
		if(!var.CARRIER) return;
		switch(charbuf[0])
		{
		case 'u':
		case 'U':
			return;
		case 'C':
		case 'c':
			return;
		case '1':
			PS("[2;18H");
			LineInput(32, 0, 0);
			TempSXUser->ConfAccess[0] = StrToLongg(charbuf);
			PS("[2;18H");
			ConfAccess(TempSXUser, buf, 32);
			PS(buf);
			PS("[7H");
			break;
		case '2':
			PS("[3;18H");
			LineInput(32, 0, 0);
			TempSXUser->ConfAccess[1] = StrToLongg(charbuf);
			PS("[3;18H");
			ConfAccess(TempSXUser, buf, 64);
			PS((char *)&buf[32]);
			PS("[7H");
			break;
		default:
			goto redraw;
		}
	}
}

ULONG StrToLongg(char *str)
{
	UWORD i=0;
	ULONG num=0;

	while(i<32)
	{
		if(str[i]=='X' || str[i]=='x') num^=1L<<i;
		i++;
	}
	return(num);
}

void Presets(struct SXUserStruct *TempSXUser, struct UserData *TempUser)
{
	BPTR fh = Open("SX:Prefs/Access.DAT", MODE_OLDFILE);
	if(fh)
	{
		PS("[0m\r\n");
		while(Read(fh, &Structs->AccSet, sizeof(struct AccessStruct)))
		{
			PSFmt("[33m%-3ld[32m) [36m%s\n\r", Structs->AccSet.access, Structs->AccSet.name);
		}
		Close(fh);
		PS("\r\nWhich set: ");
		LineInput(30, 0, 0);
		if(charbuf[0]==0 || !var.CARRIER) return;
		if(GetAccessSet(atoi(charbuf))) PutAccessSet(TempSXUser, TempUser);
	}
}

void UserList(BOOL alpha, BOOL realname)
{
	struct UserData TempUser;
	struct UserIndexStruct TempIndex;
	BPTR ifh, ufh = Open("SX:User.Data", MODE_OLDFILE);
	BOOL listfin = FALSE, contin = FALSE;
	UWORD lco = 3, users = 0, total;
	char tbuf[128],laston[18];
     
     if(alpha)	PS(Structs->SXStr[142]);
	else PS(Structs->SXStr[116]);
     
     if(ufh)
	{
		ifh = Open("SX:User.Index", MODE_OLDFILE);
		if(ifh)
		{
			Seek(ifh, 0, OFFSET_END);
			total = Seek(ifh, 0, OFFSET_BEGINING) / sizeof(struct UserIndexStruct);
			while(!listfin)
			{
				if(!(FRead(ufh, &TempUser, sizeof(struct UserData), 1)))
				{
					listfin = TRUE;
				} else {
					if(!(FRead(ifh, &TempIndex, sizeof(struct UserIndexStruct), 1)))
					{
						listfin = TRUE;
					} else {
						if(!(TempUser.Flags&(1<<6)))
						{				
                                   if(realname)
                                   {
                                        if(alpha) // extra nr. 2
                                        {
                                                myctime(TempUser.Time_Last_On, laston);                                    
								        mysprintf(tbuf, Structs->SXStr[143], TempIndex.realname, TempUser.Sec_Status, TempUser.Location, TempUser.Times_Called, laston);
                                        } else
                                        {
                                                mysprintf(tbuf, Structs->SXStr[117], TempIndex.realname, TempUser.Location, TempUser.Times_Called);
                                        }
                                                
                                   
                                   } else
                                   {
                                        if(alpha)
                                        {
                                                myctime(TempUser.Time_Last_On, laston); 
								        mysprintf(tbuf, Structs->SXStr[143], TempIndex.handle, TempUser.Sec_Status, TempUser.Location, TempUser.Times_Called, laston);
                                        } else
                                        {
                                                mysprintf(tbuf, Structs->SXStr[117], TempIndex.handle, TempUser.Location, TempUser.Times_Called);
                                        }
                                   
                                   }
                                   lco++;
							users++;
							if(contin)
							{
								CheckKey();
								if(charbuf[0] == 3) listfin = TRUE;
							}
							if(lco >= (Structs->User.LineLength-1)  &&  !contin)
							{
askcont:
								PS(Structs->SXStr[121]);
								MyHotKey();
								switch(charbuf[0])
								{
								case 'y':	
								case 'Y':
								case 10:
								case 13:
									PS(Structs->SXStr[9]);
									PS("[A\r\n[K");
									break;
								case 'n':
								case 'N':
									PS(Structs->SXStr[10]);
									PS("\r\n");
									listfin = TRUE;
									break;
								case 'c':
								case 'C':
									PS("C\r\n");
									contin = TRUE;
									break;
								default:
									PS("[A\r\n");
									goto askcont;
								}
								lco = 1;
							}
							if(!listfin) PS(tbuf);
						}
					}
				}
			}
			PSFmt(Structs->SXStr[122], users, total);
			Close(ifh);
		}
		Close(ufh);
	}
}
