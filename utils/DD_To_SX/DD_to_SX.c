static char __version[] ="\0$VER: DD_TO_SX 1.06 [eSSeXX] 68020 ("__DATE__")";
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <dos/dos.h>

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
struct DayDream_User DDUser;

struct SXUserStruct
{
	UWORD byteratio;
	UWORD fileratio;
	ULONG flags;
	UWORD freefiles;
	ULONG freebytes;
	ULONG ConfAccess[10];
	UWORD lastfilearea;
	char computer[24];
	char sentbyline[46];
	char password[16];
	long firstcall;
	char reserved[110];
};
struct SXUserStruct SXUser;

struct UserData {
	char	Name[31],
		Pass[9],
		Location[30],
		PhoneNumber[13];
	USHORT	Slot_Number;
	USHORT	Sec_Status,
		Sec_Board,		/* File or Byte Ratio */
		Sec_Library,		/* Ratio              */
		Sec_Bulletin,		/* Computer Type      */
		Messages_Posted;
	ULONG	NewSinceDate,
		ConfRead1,
		ConfRead2,
		ConfRead3,
		ConfRead4,
		ConfRead5,
		ConfRead6,
		ConfRead7,
		ConfRead8,
		ConfRead9;
	char	Conference_Access[10];
	USHORT	Uploads,
		Downloads,
		ConfRJoin,
		Times_Called;
	long	Time_Last_On,
		Time_Used,
		Time_Limit,
		Time_Left;
	ULONG	Bytes_Download,
		Bytes_Upload,
		Daily_Bytes_Limit,
		Daily_Bytes_Dld;
	char	Expert;
	ULONG	ConfYM1,
		ConfYM2,
		ConfYM3,
		ConfYM4,
		ConfYM5,
		ConfYM6,
		ConfYM7,
		ConfYM8,
		ConfYM9;
	long    BeginLogCall;
	UBYTE	Protocol,
		UUCPA,
		LineLength,
		New_User;
	};
struct UserData User;

struct UserIndexStruct
{
	char handle[31];
	char realname[31];
	UWORD misc;
};
struct UserIndexStruct UserIndex;

void main(void) /*int argc, char *argv[])*/
{
	BPTR readfh, userfh, indexfh, sxfh;
	BOOL readfin=FALSE;
	long i=0, ret;

	memset(&User, 0, sizeof(struct UserData));

	readfh = Open("daydream:data/userbase.dat", MODE_OLDFILE);
	if(readfh)
	{
		userfh = Open("ram:User.Data", MODE_NEWFILE);
		indexfh = Open("ram:User.Index", MODE_NEWFILE);
		sxfh = Open("ram:User.SX", MODE_NEWFILE);
		while(!readfin)
		{
			ret=Read(readfh, &DDUser, sizeof(struct DayDream_User));
			if(ret)
			{
				i++;
				strcpy(UserIndex.realname,DDUser.user_realname);
				strcpy(UserIndex.handle,DDUser.user_handle);
				strcpy(User.Name,DDUser.user_handle);
				strcpy(User.Location,DDUser.user_organization);
				strcpy(User.PhoneNumber,DDUser.user_voicephone);
				memcpy(SXUser.password, DDUser.user_password, 16);
				User.LineLength=DDUser.user_screenlength;
				User.Protocol=DDUser.user_protocol;
				strcpy(SXUser.sentbyline,DDUser.user_signature);
				User.Bytes_Upload=DDUser.user_ulbytes;
				User.Bytes_Download=DDUser.user_dlbytes;
				User.Downloads=DDUser.user_dlfiles;
				User.Uploads=DDUser.user_ulfiles;
				User.Messages_Posted=DDUser.user_pvtmessages+DDUser.user_pubmessages;
				User.Times_Called=DDUser.user_connections;
				SXUser.fileratio=DDUser.user_fileratio;
				SXUser.byteratio=DDUser.user_byteratio;
				strcpy(SXUser.computer,DDUser.user_computermodel);
				SXUser.freebytes=DDUser.user_freedlbytes;
				User.Sec_Status=DDUser.user_securitylevel;
				User.ConfRJoin=DDUser.user_joinconference;
				SXUser.firstcall=DDUser.user_firstcall;
				User.Time_Last_On=DDUser.user_lastcall;
				User.Time_Limit=DDUser.user_dailytimelimit*60;
				User.Time_Left=DDUser.user_dailytimelimit*60;
				User.Time_Used=0;
				SXUser.freefiles=DDUser.user_freedlfiles;
				User.Slot_Number=i;
				SXUser.ConfAccess[0]=DDUser.user_conferenceacc1;
				SXUser.ConfAccess[1]=DDUser.user_conferenceacc2;
				Write(userfh,&User,sizeof(struct UserData));
				Write(indexfh,&UserIndex,sizeof(struct UserIndexStruct));
				Write(sxfh,&SXUser,sizeof(struct SXUserStruct));
			} else {
				readfin=TRUE;
			}
		}
		Close(readfh);
		Close(userfh);
		Close(indexfh);
		Close(sxfh);
		PutStr("SX Userbase has been saved in Ram:\n");
	} else {
		PutStr("Cannot open daydream:data/userbase.data!\n");
	}
}
