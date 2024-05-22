static char __version[] ="\0$VER: SX_MakeAC 1.06 [eSSeXX] 68020 ("__DATE__")";
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


void upcase(char * str)
{
	int j=0, i=strlen(str);
	while(j!=(i+1))
	{
		str[j]=toupper(str[j]);
		j++;
	}
}


void main(void)
{
	MD5_CTX ctx;
	WORD i = 0;
	BPTR fh;
	char pass[16] = "\0";

	if (!(ReqToolsBase = (struct ReqToolsBase *)OpenLibrary (REQTOOLSNAME, REQTOOLSVERSION))) exit(10);

	memset(&User, 0, sizeof(struct UserData));
	memset(&SXUser, 0, sizeof(struct SXUserStruct));

	fh = Open("SX:User.Data", MODE_OLDFILE);
	if(fh)
	{
		Close(fh);
		if(!(rtEZRequestTags("Userbase already exists, overwrite?\n",
                "OK|CANCEL", NULL, NULL, TAG_DONE)))
		{
			CloseLibrary((struct Library *)ReqToolsBase);
			exit(0);
		}
	}

	fh = Open("SX:User.Data", MODE_NEWFILE);
	if(fh)
	{
		UserIndex.misc = 0;

		while(! (rtGetString(User.Name,30,"Enter your alias/handle:",0,0)) );
		strcpy(UserIndex.handle, User.Name);

		while(! (rtGetString(UserIndex.realname,30,"Enter your realname:",0,0)) );

		while(! (rtGetString(User.Location,29,"Enter your location:",0,0)) );

		while(! (rtGetString(pass,16,"Enter password for account:",0,0)) );

		upcase(pass);
		MD5Init(&ctx);
		MD5Update(&ctx, pass, strlen(pass));
		MD5Final(SXUser.password, &ctx);

		while(i < 32)
		{
			SXUser.ConfAccess[0] ^= 1L<<i;
			i++;
		}
		strcpy(SXUser.computer, "Amiga");
		strcpy(SXUser.sentbyline, "-");
		strcat(SXUser.sentbyline, User.Name);
		strcpy(User.PhoneNumber, "555-SX!");
		User.Slot_Number = 1;
		User.Sec_Status = 255;
		User.ConfRJoin = 1;
		User.Time_Limit = 5000 * 60;
		User.Time_Left = 5000 * 60;
		User.Time_Used = 0;
		User.Protocol = 'Z';
		User.LineLength = 27;

		Write(fh, &User, sizeof(struct UserData));
		Close(fh);

		fh = Open("SX:User.Index", MODE_NEWFILE);
		if(fh)
		{
			Write(fh, &UserIndex, sizeof(struct UserIndexStruct));
			Close(fh);
		}

		fh = Open("SX:User.SX", MODE_NEWFILE);
		if(fh)
		{
			Write(fh, &SXUser, sizeof(struct SXUserStruct));
			Close(fh);
		}

	}

	CloseLibrary((struct Library *)ReqToolsBase);
}
