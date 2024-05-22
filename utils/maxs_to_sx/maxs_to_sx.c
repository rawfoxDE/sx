static char __version[] ="\0$VER: MAXS_TO_SX 1.06 [eSSeXX] 68020 ("__DATE__")";
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <dos/dos.h>
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

struct ReqToolsBase *ReqToolsBase;

struct MaxsStruct
{
	char	name[41],
		sub[40],
		password[21],
		phone[21],
		computer[21],
		comment[100];
	long	left,
		access,
		limit,
		ratio,
		lmr,
		pagelen,
		calls,
		messages,
		uploads,
		downloads,
		laston_days,
		laston_mins,
		flags,
		timebank,
		protocol,
		maxbank;
} Maxs;

struct ReqToolsBase *ReqToolsBase;

void upcase(char * str)
{
	int j=0, i=strlen(str);
	while(j!=(i+1))
	{
		str[j]=toupper(str[j]);
		j++;
	}
}


int main(int argc, char *argv[])
{
	MD5_CTX ctx;
	WORD i;
	BPTR fh, ufh, ifh, sfh;
	BOOL readfin = FALSE;
	char buf[256];
	ULONG ll;

	if(!argv[1] || argv[1][0]==0 || !argc)
	{
		PutStr("	maxs_to_sx <maxsuserbase>\nexample:\n	maxs_to_sx BBS:UserFiles/User.DATA\n");
		exit(0);
	}

	if (!(ReqToolsBase = (struct ReqToolsBase *)OpenLibrary (REQTOOLSNAME, REQTOOLSVERSION))) exit(10);

	memset(&User, 0, sizeof(struct UserData));
	memset(&SXUser, 0, sizeof(struct SXUserStruct));

	fh = Open(argv[1], MODE_OLDFILE);
	if(fh)
	{
		ufh = Open("ram:User.Data", MODE_NEWFILE);
		ifh = Open("ram:User.Index", MODE_NEWFILE);
		sfh = Open("ram:User.SX", MODE_NEWFILE);

		Seek(fh, 66, OFFSET_BEGINING);

		while(!readfin)
		{
			if(Read(fh, &Maxs, sizeof(struct MaxsStruct)))
			{
				strcpy(User.Name, Maxs.name);
				strcpy(UserIndex.realname, Maxs.name);
				strcpy(UserIndex.handle, Maxs.name);
				strcpy(User.Location, Maxs.sub);
				strcpy(User.PhoneNumber, Maxs.phone);
				strcpy(SXUser.computer, Maxs.computer);

				if(Maxs.access > 255)
				{
badacc:
					sprintf(buf, "Access %ld is invalid, chose new:", Maxs.access);
					ll = 255;
					rtGetLong(&ll, buf, NULL, TAG_DONE);
					if(ll > 255) goto badacc;
					User.Sec_Status = ll;
				} else {
					User.Sec_Status = Maxs.access;
				}

				User.Time_Limit = Maxs.limit;
				User.LineLength = Maxs.pagelen;
				User.Times_Called = Maxs.calls;
				User.Messages_Posted = Maxs.messages;
				User.Protocol = 'Z';

				strcpy(SXUser.sentbyline, "-");
				strcat(SXUser.sentbyline, User.Name);

				upcase(Maxs.password);
				MD5Init(&ctx);
				MD5Update(&ctx, Maxs.password, strlen(Maxs.password));
				MD5Final(SXUser.password, &ctx);

				i = 0;
				while(i < 32)
				{
					SXUser.ConfAccess[0] ^= 1L<<i;
					i++;
				}

				Write(ufh, &User, sizeof(struct UserData));

				Write(ifh, &UserIndex, sizeof(struct UserIndexStruct));

				Write(sfh, &SXUser, sizeof(struct SXUserStruct));

			} else {
				readfin = TRUE;
			}
		}
		if(ifh) Close(ifh);
		if(ufh) Close(ufh);
		if(sfh) Close(sfh);
		Close(fh);
		PutStr("SX Userbase has been saved in ram:\n");
	} else {
		PutStr("Cannot open input file!\n");
	}

	CloseLibrary((struct Library *)ReqToolsBase);
}
