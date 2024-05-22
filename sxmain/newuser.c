
#include <stdlib.h>
#include <string.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <devices/serial.h>
#include "SXStructs.h"
#include "sx.h"

extern struct StructsStruct	*Structs;
extern struct AmigaStruct	Amiga;
extern struct StringsStruct	*stg;
extern struct VariablesStruct	var;

extern UBYTE buf[512], charbuf[128];


void ConfAccess(struct SXUserStruct *TempSXUser, char *str, UWORD max);
BOOL CheckConfAccess(struct SXUserStruct *TempSXUser, UWORD conf);
void PutAccessSet(struct SXUserStruct *TempSXUser, struct UserData *TempUser);
BOOL GetAccessSet(UWORD access);
int YesNoAsk(int def);
void User_Location(BOOL force);
void User_Phone(BOOL force);
void User_Computer(BOOL force);
void User_Pass(BOOL force);
void User_Screen(BOOL force);
void User_Signature(BOOL force);
void User_Protocol(void);
void User_MailScan(void);
void User_FileScan(void);

void NewUser(void)
{
	BPTR fh;
	long x = 0;

	if(Structs->NodePrefs.newuserpass[0]!=0)
	{
		SetActive(109, 0);
newpass:
		ShowAnsi("NewUserPass", 1, 1, 0, 0, 1, 0);
		LineInput(17, 1, 0);
		PSLen("\r\n", 2);
		if(stricmp(charbuf, Structs->NodePrefs.newuserpass) != 0)
		{
			x++;
			if(x == 3)
			{
				PS(Structs->SXStr[26]);
				DropDTR(TRUE);
				return;
			}
			PS(Structs->SXStr[25]);
			goto newpass;
		}
	}

	SetActive(11, 0);

	memset(&Structs->User, 0, sizeof(struct UserData));
	memset(&Structs->SXUser, 0, sizeof(struct SXUserStruct));
	Structs->UserIndex.flags = 0;
	Structs->UserIndex.unused = 0;

	fh = Open("SX:User.Index", MODE_OLDFILE);
	if(fh)
	{
		Seek(fh, 0, OFFSET_END);
		Structs->User.Slot_Number = (Seek(fh,0,0)/sizeof(struct UserIndexStruct))+1;
		Close(fh);
	}
	ShowAnsi("NewUser", 1, 1, 1, 1, 1, 0);

handle:
	PS(Structs->SXStr[47]);
	LineInput(30, 0, 0);
	if(!var.CARRIER) return;
	if(charbuf[0]==0) goto handle;
	PS(Structs->SXStr[46]);
	if(FindAccount(charbuf)) { PS(Structs->SXStr[45]); goto handle; }
	strcpy(Structs->User.Name, charbuf);
	strcpy(Structs->UserIndex.handle, charbuf);

realname:
	PS(Structs->SXStr[48]);
	LineInput(30,0,0);
	if(!var.CARRIER) return;
	if(charbuf[0]==0) goto realname;
	PS(Structs->SXStr[46]);
	if(FindAccount(charbuf)) { PS(Structs->SXStr[45]); goto realname; }
	strcpy(Structs->UserIndex.realname,charbuf);

	User_Location(1);
	if(!var.CARRIER) return;

	User_Phone(1);
	if(!var.CARRIER) return;

	User_Computer(1);
	if(!var.CARRIER) return;

	User_Pass(1);
	if(!var.CARRIER) return;

	User_Screen(1);
	if(!var.CARRIER) return;

	mysprintf(Structs->SXUser.sentbyline, "-%s-", Structs->User.Name);

	User_MailScan();
	if(!var.CARRIER) return;

	User_FileScan();
	if(!var.CARRIER) return;

	GetAccessSet(0);
	PutAccessSet(&Structs->SXUser, &Structs->User);

	Structs->SXUser.firstcall = mytime();
	Structs->User.Protocol = 'Z';

	fh = Open("SX:User.Index", MODE_OLDFILE);
	if(fh)
	{
		Seek(fh, 0, OFFSET_END);
		Structs->User.Slot_Number = (Seek(fh,0,0)/sizeof(struct UserIndexStruct))+1;
		Close(fh);
	}
	SaveAccount();
	ShowAnsi("PostNewUser", 1, 1, 1, 1, 1, 0);
}

void ConfAccess(struct SXUserStruct *TempSXUser, char *str, UWORD max)
{
	UWORD i = 0;

	while(i < max)
	{
		if(CheckConfAccess(TempSXUser, i+1)) str[i]='X'; else str[i]='_';
		i++;
	}
	str[i]=0;
}

BOOL CheckConfAccess(struct SXUserStruct *TempSXUser, UWORD conf)
{
	if(conf < 33)
	{
		if( TempSXUser->ConfAccess[0] & (1L<<(conf-1)) ) return(TRUE); else return(FALSE);
	}
	if(conf < 65)
	{
		if( TempSXUser->ConfAccess[1] & (1L<<(conf-33)) ) return(TRUE); else return(FALSE);
	}
}

void PutAccessSet(struct SXUserStruct *TempSXUser, struct UserData *TempUser)
{
	TempUser->Time_Left = Structs->AccSet.Time_Limit;
	TempUser->Time_Limit = Structs->AccSet.Time_Limit;
	TempUser->Sec_Status = Structs->AccSet.access;
	TempSXUser->freefiles = Structs->AccSet.freefiles;
	TempSXUser->freebytes = Structs->AccSet.freebytes;
	TempSXUser->fileratio = Structs->AccSet.fileratio;
	TempSXUser->byteratio = Structs->AccSet.byteratio;
	memcpy(&TempSXUser->ConfAccess[0], &Structs->AccSet.ConfAccess[0], 40);

	if(TempUser == &Structs->User)
	{
		var.lastleft = Structs->User.Time_Limit;
		Structs->User.Time_Left = Structs->User.Time_Limit;
	}
}

BOOL GetAccessSet(UWORD access)
{
	BPTR fh = Open("SX:Prefs/Access.DAT", MODE_OLDFILE);
	if(fh)
	{
		while(1)
		{
			if( Read(fh, &Structs->AccSet, sizeof(struct AccessStruct)) == 0)
			{
				Close(fh);
				return(FALSE);
			}
			if((Structs->AccSet.access == access) || (access == 0))
			{
				Close(fh);
				return(TRUE);
			}
		}
	} else {
		return(FALSE);
	}
}

int YesNoAsk(int def)
{
	while(1)
	{
		MyHotKey();
		if(charbuf[0]==13 || charbuf[0]==10)
		{
			switch(def)
			{
			case 1: charbuf[0]='Y'; break;
			case 2: charbuf[0]='N'; break;
			case 3: charbuf[0]='A'; break;
			}
		}
		switch(charbuf[0])
		{
		case 'y':
		case 'Y':
			PS(Structs->SXStr[9]);
			return(1);
		case 'n':
		case 'N':
			PS(Structs->SXStr[10]);
			return(0);
		case 'a':
		case 'A':
			PS(Structs->SXStr[23]);
			return(2);
		}
	}
}

void User_Location(BOOL force)
{
	while(1)
	{
		PS(Structs->SXStr[49]);
		if(force) LineInput(29,0,0); else LineInput(29,0,Structs->User.Location);
		if(!var.CARRIER) return;
		if(charbuf[0]==0)
		{
			if(!force) break;
		} else {
			strcpy(Structs->User.Location,charbuf);
			break;
		}
	}
	UpdateUserInfo();
}

void User_Phone(BOOL force)
{
	while(1)
	{
		PS(Structs->SXStr[50]);
		if(force) LineInput(12,0,0); else LineInput(12,0,Structs->User.PhoneNumber);
		if(!var.CARRIER) return;
		if(charbuf[0]==0)
		{
			if(!force) break;
		} else {
			strcpy(Structs->User.PhoneNumber,charbuf);
			break;
		}
	}
	UpdateUserInfo();
}

void User_Computer(BOOL force)
{
	BOOL fin=FALSE;
	while(!fin)
	{
		PS(Structs->SXStr[51]);
		if(force) LineInput(23,0,0); else LineInput(23,0,Structs->SXUser.computer);
		if(!var.CARRIER) return;
		if(charbuf[0]==0)
		{
			if(!force) fin=TRUE;
		} else {
			strcpy(Structs->SXUser.computer,charbuf);
			fin=TRUE;
		}
	}
	UpdateUserInfo();
}

void User_Pass(BOOL force)
{
	char tbuf[32];
pass1:
	PS(Structs->SXStr[52]);
	LineInput(24,1,0);
	if(!var.CARRIER) return;
	if(charbuf[0]==0 && force) goto pass1;
	if(charbuf[0]==0) return;
	strcpy(tbuf,charbuf);
pass2:
	PS(Structs->SXStr[53]);
	LineInput(24,1,0);
	if(!var.CARRIER) return;
	if(charbuf[0]==0 && force) goto pass2;
	if(charbuf[0]==0) return;
	if(stricmp(tbuf, charbuf)!=0 && force) { PS(Structs->SXStr[54]); goto pass1; }
	if(stricmp(tbuf, charbuf)!=0) { PS(Structs->SXStr[54]); return; }
	upcase(charbuf);
	Servermsg(8);
	memcpy(Structs->SXUser.password, buf, 16);
}

void User_Screen(BOOL force)
{
screen:
	if(Structs->User.LineLength==0) force=TRUE;
	PS(Structs->SXStr[55]);
	LineInput(10,0,0);
	if(!var.CARRIER) return;
	if(charbuf[0]==0 && force) goto screen;
	if(charbuf[0]==0) return;
	if(stricmp(charbuf,"T")==0) { ShowAnsi("ScreenLength", 1, 1, 0, 0, 0, 0); goto screen; }
	Structs->User.LineLength=atoi(charbuf);
	if(Structs->User.LineLength==0) goto screen;
}

void User_Signature(BOOL force)
{
	while(1)
	{
		PS(Structs->SXStr[49]);
		if(force) LineInput(29,0,0); else LineInput(29,0,Structs->SXUser.sentbyline);
		if(!var.CARRIER) return;
		if(charbuf[0]==0)
		{
			if(!force) break;
		} else {
			strcpy(Structs->SXUser.sentbyline, charbuf);
			break;
		}
	}
}

void User_Protocol(void)
{
	while(1)
	{
		ShowAnsi("Protocols", 1, 1, 1, 1, 1, 0);
		LineInput(20, 0, 0);
		if(!var.CARRIER) return;
		if(charbuf[0]==0)
		{
			if(LoadProtocol(Structs->User.Protocol)) return;
		} else {
			if(LoadProtocol(charbuf[0]))
			{
				Structs->User.Protocol = charbuf[0];
				return;
			}
		}
	}
}

void User_MailScan(void)
{
	PS(Structs->SXStr[56]);
	switch(YesNoAsk(0))	/* mail scan */
	{
	case 1:
		Structs->User.Flags|=1<<1;	/* yes */
		if(Structs->User.Flags&(1<<2)) Structs->User.Flags^=1<<2;
		break;
	case 2:
		Structs->User.Flags|=1<<2;	/* ask */
		break;
	default:
		if(Structs->User.Flags&(1<<1)) Structs->User.Flags^=1<<1;
		if(Structs->User.Flags&(1<<2)) Structs->User.Flags^=1<<2;
	}
}

void User_FileScan(void)
{
	PS(Structs->SXStr[57]);
	switch(YesNoAsk(0))	/* file scan */
	{
	case 1:
		Structs->User.Flags|=1<<3;	/* yes */
		if(Structs->User.Flags&(1<<4)) Structs->User.Flags^=1<<4;
		break;
	case 2:
		Structs->User.Flags|=1<<4;	/* ask */
		break;
	default:
		if(Structs->User.Flags&(1<<3)) Structs->User.Flags^=1<<3;
		if(Structs->User.Flags&(1<<4)) Structs->User.Flags^=1<<4;
	}
}

void User_BGCheck(void)
{
	PS(Structs->SXStr[133]);
	switch(YesNo(0, TRUE))			/* BG Upload Checker */
	{
	case 1:
		Structs->User.Flags|=1<<0;			/* yes */
		break;
	case 0:
		if(Structs->User.Flags&(1<<0)) Structs->User.Flags^=1<<0;		/* no */
		break;
	}
}

void User_FSED(void)
{
	PS(Structs->SXStr[134]);
	switch(YesNo(0, TRUE))			/* Full Screen Editor */
	{
	case 1:
		if(Structs->User.Flags&(1<<5)) Structs->User.Flags^=1<<5;		/* yes */
		break;
	case 0:
		Structs->User.Flags|=1<<5;	/* BIT 5 = LINE EDITOR */
		break;
	}
}
