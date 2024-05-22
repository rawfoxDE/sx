/* ==================================================================== */
/* ================= System-X Master Control Program ================== */
/* ==================================================================== */


static const char __version[]="\0$VER: SX-MCP 1.06b [eSSeXX] 68020 ("__DATE__")";

/* msgs to nodes */

#define CLOSESCREEN	1
#define OPENSCREEN	2
#define SHUTDOWN	3
#define KICKUSER	4
#define CHAT		5
#define SERIALQUERY	6
#define TOFRONT		8

/* msgs to MCP */

#define REGO		1
#define UNREGO		2
#define USERLOGIN	3
#define USERLOGOUT	4
#define USERACTIVE	5
#define MENUPO		6
#define UPDATEUSER	7
#define PASSWORD	8
#define PAGEDFLAG	9
#define QUERYSYSOP	10
#define REPORTCPS	11
#define APPENDFILE	12
#define NODESPO		14
#define DOWNLOADFIN	15
#define UPLOADFIN	16
#define LOADMCPCFG	17
#define DDNODES		18
#define SXTODD		19

#include <stdlib.h>
#include <string.h>
#include <exec/memory.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
#include <proto/icon.h>
#include <proto/wb.h>
#include <proto/gadtools.h>
#include <intuition/intuition.h>
#include <intuition/screens.h>
#include <libraries/gadtools.h>
#include <dos/dos.h>
#include <dos/dostags.h>
#include <workbench/workbench.h>
#include <workbench/icon.h>
#include <libraries/reqtools.h>
#include <proto/reqtools.h>
#include "SX-MCP.h"

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

extern struct Image sysxbbsImage;
extern struct Image p1Image;
extern struct Image p2Image;

extern void SmoothInfoBox(void);
extern void Smooth(int left, int top);
extern void TickON(void);
extern void TickOFF(void);
extern void Pic(int left, int top);
extern void BevBox(char inv, int left, int top, int right, int bot, char * s, int textpen);
extern void FadeOut(void);

void HandleMsg(void);
void CallerLog(struct CallerLog *Caller);
void CheckRexx(void);
void UpdateDisplay(int i, char sel);
void MainConfig(void);
void AppendFile(char *file, char *buffer, long size);
void CheckWin(void);
void StartWindow(void);
void HandleGad(struct Gadget *gad, UWORD code);
void ShowAll(void);
void ShowNode(UWORD node);
void UserInfo(int cls);
void ShowList(UWORD which);
void About(void);
void StartIcon(void);
void LoadMenus(void);
void FlushMenus(void);
void MsgToNode(int cmd, int node);
UWORD ActiveNodes(void);
UWORD OnlineNodes(void);
void SXBase_To_DDBase(struct UserData *TUser, struct SXUserStruct *TSXUser, struct UserIndexStruct *TIndex, struct DayDream_User *DDUser);
void ShutAll(void);


struct McpStruct
{
	UWORD	leftedge,
		topedge,
		flags;		/* BITS: 0=iconify, 1=realnames */
	char	text1[16],
		cmd1[48],
		text2[16],
		cmd2[48];
	UBYTE	listsize,
		reserve;
	char	awayreason[74];
} mcp;

struct NodeMessage
{
	struct Message Msg;
	UWORD command;
	UWORD misc;
	long data1;
	long data2;
	long data3;
	long data4;
} themsg;

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
} ddnodes[34];

struct node_struct
{						/* offset DEC	*/
	APTR next;				/*	0	*/
	struct UserData *User;			/*	4	*/
	struct UserIndexStruct *UserIndex;	/*	8	*/
	struct SXUserStruct *SXUser;		/*	12	*/
	char *action;				/*	16	*/
	char *filename;				/*	20	*/
	long baud;				/*	24	*/
	long loginsecs;				/*	28	*/
	UWORD number;				/*	32	*/
	UBYTE actionnumber;			/*	34	*/
	UBYTE active;				/*	35	*/
	UBYTE useron;				/*	36	*/
	UBYTE paged;				/*	37	*/
} sxnodes[34];

struct MainStruct MainCfg;
struct UserData User;

struct MenusStruct
{
	char name[30];
	char *byso;
	long size;
} menus[202];

struct XferLog
{
	UWORD	user_slot;
	UBYTE	conf,
		filearea;
	char	filename[32];
	long	size,
		baud,
		cps,
		time;
	UBYTE	node;
	char	res[7];
} Xfer;

struct CallerLog
{
	UWORD	user_slot,
		node;
	long	time_login,
		time_logout,
		seconds_online,
		bytes_uploaded,
		bytes_downloaded;
	UWORD	files_uploaded,
		files_downloaded,
		messages;
	long	baud,
		flags;
	UBYTE	logout_mode; 	/*	0 = normal		*/
	char	res[25];	/*	1 = lost carrier	*/
};				/*	2 = auto logoff after download*/

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


extern struct ExecBase *SysBase;
struct IntuitionBase *IntuitionBase;
struct GfxBase *GfxBase;
struct Library *IconBase, *GadToolsBase, *WorkbenchBase;
struct ReqToolsBase *ReqToolsBase;
struct Screen *scr;
struct Gadget *glist, *gad;
struct NewGadget ng;
struct MCPMessage *msg;
struct Window *win;
struct RastPort *rp;
struct TextFont *topazfont;
struct TextAttr topaz = {"topaz.font",8,0,0};
struct MsgPort *mp, *RexxPort;

void *vi;
char done, ICONIFY, away, blank, RESTART, SHUTALL;
long WinTop, ptop, WinBot, mcpsign, rexsign;
ULONG lastsec, lasttic, sec, tic;
char buf[256], *po[6], *lastcall[7], *lastdown[7], *lastup[7];
WORD sel, max, top, shownodes, info, alts[4];

struct TagItem tags[]={{SYS_Input,0},{SYS_Output,0},{SYS_Asynch,TRUE},{NP_Priority,0},{TAG_DONE,0}};

#define GAD_ICONIFY	0
#define GAD_CYCLE	1
#define GAD_OPEN	2
#define GAD_CLOSE	3
#define GAD_LAUNCH	4
#define GAD_SHUT	5
#define GAD_CHAT	6
#define GAD_KICK	7
#define GAD_MCPCONFIG	8
#define GAD_SHUTALL	9
#define GAD_AWAY	10
#define GAD_CONFIG	11
#define GAD_CONE	12
#define GAD_CTWO	13


void mysprintf(char *Buffer,char *ctl, ...)
{
	RawDoFmt(ctl, (long *)(&ctl + 1), (void (*))"\x16\xc0\x4e\x75", Buffer);
}

void TextFmt(struct RastPort *rP, char *ctl, ...)
{
	RawDoFmt(ctl, (long *)(&ctl + 1), (void (*))"\x16\xc0\x4e\x75", buf);
	Text(rP, buf, strlen(buf));
}

void main(void)/*int argc, char *argv[])*/
{
	long sign, winsig;
	UWORD i, next;
	BPTR ufh, fh1, fh2, callfh;
	struct CallerLog Caller;

	if(FindPort("SX-MCP")) return;

	if(((struct Library *)SysBase)->lib_Version > 35)
	{
		ReqToolsBase = (struct ReqToolsBase *)OpenLibrary(REQTOOLSNAME, 0);
		if(!ReqToolsBase) return;
		IntuitionBase = ReqToolsBase -> IntuitionBase;
		GfxBase = ReqToolsBase -> GfxBase;
		GadToolsBase = ReqToolsBase -> GadToolsBase;
		IconBase = OpenLibrary("icon.library", 0);
		WorkbenchBase = OpenLibrary("workbench.library", 0);

		MainConfig();
		LoadMenus();

		mp = CreatePort("SX-MCP", 0L);
		RexxPort = CreatePort("SX-MCP-REXX", 0L);

		mcpsign	= 1L<<mp->mp_SigBit;
		rexsign = 1L<<RexxPort->mp_SigBit;

		topazfont = OpenFont(&topaz);

		ufh = Open("SX:User.Data", MODE_OLDFILE);
		fh1 = Open("SX:LogFiles/Download.LOG", MODE_OLDFILE);
		fh2 = Open("SX:LogFiles/Upload.LOG", MODE_OLDFILE);
		callfh = Open("SX:LogFiles/Callers.LOG", MODE_OLDFILE);

		if(fh1) Seek(fh1, -6*sizeof(struct XferLog), OFFSET_END);
		if(fh2) Seek(fh2, -6*sizeof(struct XferLog), OFFSET_END);
		if(callfh)
		{
			Read(callfh, &next, 2);
			for(i=0 ; i<6 ; i++)
			{
				next--;
				if(next == 0) next = 30;
			}
		}

		i = 6;
		while(i != 0)
		{
			lastcall[i] = AllocMem(52, MEMF_PUBLIC|MEMF_CLEAR);
			if(callfh)
			{
				Seek(callfh, 2 + ((next-1)*sizeof(struct CallerLog)), OFFSET_BEGINING);
				if( Read(callfh, &Caller, sizeof(struct CallerLog)) )
				{
					next++;
					if(next > 30) next = 1;
					if(ufh && Caller.user_slot > 0)
					{
						Seek(ufh, (Caller.user_slot-1)*sizeof(struct UserData), OFFSET_BEGINING);
						Read(ufh, &User, sizeof(struct UserData));
						mysprintf(lastcall[i], "#%02ld %-16.16s %-16.16s %6ld %3ld",
							Caller.node,
							User.Name,
							User.Location,
							Caller.baud,
							(Caller.seconds_online/60) + 1);
					}
				}
			}
			lastdown[i] = AllocMem(52, MEMF_PUBLIC|MEMF_CLEAR);
			if(fh1)
			{
				if(Read(fh1, &Xfer, sizeof(struct XferLog)))
				{
					if(ufh)
					{
						Seek(ufh, (Xfer.user_slot-1)*sizeof(struct UserData), OFFSET_BEGINING);
						Read(ufh, &User, sizeof(struct UserData));
					}
					mysprintf(lastdown[i],"#%02ld %-20.20s %-13.13s %9ld", Xfer.node, User.Name, Xfer.filename, Xfer.size);
				} else {
					Close(fh1);
					fh1 = 0;
				}
			}
			lastup[i] = AllocMem(52, MEMF_PUBLIC|MEMF_CLEAR);
			if(fh2)
			{
				if(Read(fh2, &Xfer, sizeof(struct XferLog)))
				{
					if(ufh)
					{
						Seek(ufh, (Xfer.user_slot-1)*sizeof(struct UserData), OFFSET_BEGINING);
						Read(ufh, &User, sizeof(struct UserData));
					}
					mysprintf(lastup[i],"#%02ld %-20.20s %-13.13s %9ld", Xfer.node, User.Name, Xfer.filename, Xfer.size);
				} else {
					Close(fh2);
					fh2 = 0;
				}
			}
			i--;
		}
		if(fh1) Close(fh1);
		if(fh2) Close(fh2);
		if(ufh) Close(ufh);
		if(callfh) Close(callfh);

		i = 0;
		while(i < 32)
		{
			i++;
			memset(&sxnodes[i], 0, sizeof(struct node_struct));
 			memset(&ddnodes[i], 0, sizeof(struct DDNodeStruct));

			if(i < MainCfg.nodes)
			{
				ddnodes[i].ns_Next = &ddnodes[i+1];
				sxnodes[i].next = &sxnodes[i+1];
			}
			if(i > 1) ddnodes[i].ns_Prev = &ddnodes[i-1];

			ddnodes[i].ns_Nodenumber = i;
			sxnodes[i].number = i;
		}
		if(mcp.flags&(1<<0)) goto iconify;
start:
		i = 1;
		sel = 1;
		top = 1;
		ICONIFY = FALSE;
		done = FALSE;

		StartWindow();
		SmoothInfoBox();

		UpdateDisplay(1, TRUE);
		while(i != shownodes)
		{
			i++;
			UpdateDisplay(i, FALSE);
		}

		winsig	= 1L<<win->UserPort->mp_SigBit;
		while(!done && !ICONIFY && !RESTART)
		{
			sign = Wait( mcpsign | winsig | rexsign );

			if(sign & mcpsign) HandleMsg();
			if(sign & rexsign) CheckRexx();
			if(sign & winsig ) CheckWin();
		}
		if(!RESTART) FadeOut();
		CloseWindow(win);
	 	FreeGadgets(glist);
       	 	FreeVisualInfo(vi);

		if(RESTART)
		{
			RESTART = FALSE;
			goto start;
		}

		if(ICONIFY)
		{
iconify:
			StartIcon();
			blank=FALSE;
			goto start;
		}
		DeletePort((struct MsgPort *)mp);
		DeletePort((struct MsgPort *)RexxPort);
		CloseFont(topazfont);
		FlushMenus();

		i = 0;
		while(i < 6)
		{
			i++;
			FreeMem(lastcall[i], 52);
			FreeMem(lastdown[i], 52);
			FreeMem(lastup[i], 52);
		}

		CloseLibrary((struct Library *)ReqToolsBase);
		if(WorkbenchBase) CloseLibrary(WorkbenchBase);
		CloseLibrary(IconBase);

		fh1 = Open("SX:Prefs/MCP.DAT", MODE_NEWFILE);
		if(fh1)
		{
			if(!away) mcp.awayreason[0] = 0;
			Write(fh1, &mcp, sizeof(struct McpStruct));
			Close(fh1);
		}
	}
}

void HandleMsg(void)
{
	UWORD j;
	MD5_CTX ctx;
	char *pnt;
	struct CallerLog Caller;

	while(msg = (struct MCPMessage *)GetMsg((struct MsgPort *)mp)) 
	{
		switch(msg->command)
		{
		case LOADMCPCFG:
			MainConfig();
			ReplyMsg((struct Message *)msg);
			RESTART = TRUE;
			break;
		case REGO:
			msg->data1 = (long)&MainCfg;
			msg->data2 = (long)&sysxbbsImage;
			msg->data3 = (long)&p1Image;
			msg->data4 = (long)&p2Image;
			ReplyMsg((struct Message *)msg);
			if(!ddnodes[msg->nodenum].ns_Flags&(1<<0)) ddnodes[msg->nodenum].ns_Flags |= 1<<0;
			sxnodes[msg->nodenum].active = TRUE;
			break;
		case UNREGO:
			ReplyMsg((struct Message *)msg);
			if(ddnodes[msg->nodenum].ns_Flags&(1<<0)) ddnodes[msg->nodenum].ns_Flags ^= 1<<0;
			sxnodes[msg->nodenum].active = FALSE;
			ShowNode(msg->nodenum);
			if(SHUTALL && !ActiveNodes()) done = TRUE;
			break;
		case USERLOGIN:
			ddnodes[msg->nodenum].ns_ConnectionRate	= msg->data1;
			ddnodes[msg->nodenum].ns_ub	= AllocMem(sizeof(struct DayDream_User), MEMF_PUBLIC|MEMF_CLEAR);
			SXBase_To_DDBase((struct UserData *)msg->data2, (struct SXUserStruct *)msg->data4, (struct UserIndexStruct *)msg->data3, ddnodes[msg->nodenum].ns_ub);

			sxnodes[msg->nodenum].useron	= TRUE;
			sxnodes[msg->nodenum].baud	= msg->data1;
			sxnodes[msg->nodenum].User	= (struct UserData *)msg->data2;
			sxnodes[msg->nodenum].UserIndex	= (struct UserIndexStruct *)msg->data3;
			sxnodes[msg->nodenum].SXUser	= (struct SXUserStruct *)msg->data4;
			sxnodes[msg->nodenum].loginsecs = msg->data5;
			if(msg->nodenum==sel && info==0 && !ICONIFY) UserInfo(1);
			ReplyMsg((struct Message *)msg);
			ShowNode(msg->nodenum);
			break;
		case USERLOGOUT:
			FreeMem(ddnodes[msg->nodenum].ns_ub, sizeof(struct DayDream_User));
			ddnodes[msg->nodenum].ns_ub = NULL;

			sxnodes[msg->nodenum].useron=FALSE;
			if(msg->nodenum==sel && info==0 && !ICONIFY) { EraseRect(rp,241,ptop+13,631,WinBot-1); SmoothInfoBox(); }
			if(msg->data1!=0 && sxnodes[msg->nodenum].User->Name && sxnodes[msg->nodenum].User->Name[0]!=0)
			{
				j=7;
				while(j > 2)
				{
					j--;
					strcpy(lastcall[j], lastcall[j-1]);
				}
				mysprintf(lastcall[1], "#%02ld %-16.16s %-16.16s %6ld %3ld",
					msg->nodenum,
					sxnodes[msg->nodenum].User->Name,
					sxnodes[msg->nodenum].User->Location,
					sxnodes[msg->nodenum].baud,
					((msg->data5-sxnodes[msg->nodenum].loginsecs)/60)+1);

				Caller.user_slot = sxnodes[msg->nodenum].User->Slot_Number;
				Caller.node = msg->nodenum;
				Caller.time_login = sxnodes[msg->nodenum].loginsecs;
				Caller.time_logout = msg->data5;
				Caller.seconds_online = msg->data5 - Caller.time_login;
				Caller.bytes_uploaded = 0;
				Caller.bytes_downloaded = 0;
				Caller.files_uploaded = 0;
				Caller.files_downloaded = 0;
				Caller.messages = 0;
				Caller.baud = sxnodes[msg->nodenum].baud;
				Caller.flags = 0;
				Caller.logout_mode = msg->data2;
				CallerLog(&Caller);
			}
			ReplyMsg((struct Message *)msg);
			if(!ICONIFY) ShowList(1);
			ShowNode(msg->nodenum);
			break;
		case DOWNLOADFIN:
			if(msg->data1)
			{
				j = 7;
				while(j > 2)
				{
					j--;
					strcpy(lastdown[j], lastdown[j-1]);
				}
				mysprintf(lastdown[1], "#%02ld %-20.20s %-13.13s %9ld", msg->nodenum, sxnodes[msg->nodenum].User->Name, (char *)msg->data1, msg->data2);
				Xfer.user_slot = sxnodes[msg->nodenum].User->Slot_Number;
				Xfer.conf = msg->data4;
				Xfer.filearea = msg->data5;
				strcpy(Xfer.filename, (char *)msg->data1);
				Xfer.size = msg->data2;
				Xfer.baud = sxnodes[msg->nodenum].baud;
				Xfer.cps = msg->data3;
				Xfer.time = (long) msg->action;
				Xfer.node = msg->nodenum;
				ReplyMsg((struct Message *)msg);
				if(!ICONIFY)ShowList(3);
				AppendFile("SX:LogFiles/Download.Log", (char *)&Xfer, sizeof(struct XferLog));
			} else
				ReplyMsg((struct Message *)msg);
			break;
		case UPLOADFIN:
			if(msg->data1)
			{
				j = 7;
				while(j > 2)
				{
					j--;
					strcpy(lastup[j], lastup[j-1]);
				}
				mysprintf(lastup[1], "#%02ld %-20.20s %-13.13s %9ld", msg->nodenum, sxnodes[msg->nodenum].User->Name, (char *)msg->data1, msg->data2);
				Xfer.user_slot = sxnodes[msg->nodenum].User->Slot_Number;
				Xfer.conf = msg->data4;
				Xfer.filearea = msg->data5;
				strcpy(Xfer.filename, (char *)msg->data1);
				Xfer.size = msg->data2;
				Xfer.baud = sxnodes[msg->nodenum].baud;
				Xfer.cps = msg->data3;
				Xfer.time = (long) msg->action;
				Xfer.node = msg->nodenum;
				ReplyMsg((struct Message *)msg);
				if(!ICONIFY) ShowList(2);
				AppendFile("SX:LogFiles/Upload.Log", (char *)&Xfer, sizeof(struct XferLog));
			} else
				ReplyMsg((struct Message *)msg);
			break;
		case USERACTIVE:
			ddnodes[msg->nodenum].ns_Activity = msg->action;
			sxnodes[msg->nodenum].action = msg->action;
			sxnodes[msg->nodenum].actionnumber = msg->data1;
			ReplyMsg((struct Message *)msg);
			ShowNode(msg->nodenum);
			break;
		case MENUPO:
			msg->data3=0;
			j=0;
			while(j != 200)
			{
				j++;
				if(stricmp(menus[j].name, (char *)msg->data1)==0)
				{
					msg->data2=(long)menus[j].byso;
					msg->data3=menus[j].size;
					j=200;
				}
			}
			ReplyMsg((struct Message *)msg);
			break;
		case UPDATEUSER:
			if(msg->nodenum==sel && info==0 && !ICONIFY) UserInfo(1);
			ShowNode(msg->nodenum);
			if(ddnodes[msg->nodenum].ns_ub)
				SXBase_To_DDBase(sxnodes[msg->nodenum].User,
						sxnodes[msg->nodenum].SXUser,
						sxnodes[msg->nodenum].UserIndex,
						ddnodes[msg->nodenum].ns_ub);
			ReplyMsg((struct Message *)msg);
			break;
		case PASSWORD:
			pnt = (char *)msg->data1;
			MD5Init(&ctx);
			MD5Update(&ctx, pnt, strlen(pnt));
			pnt = (char *)msg->data2;
			MD5Final(pnt, &ctx);
			ReplyMsg((struct Message *)msg);
			break;
		case PAGEDFLAG:
			sxnodes[msg->nodenum].paged = msg->data1;
			ReplyMsg((struct Message *)msg);
			break;
		case QUERYSYSOP:
			if(away) msg->data1 = (long)&mcp.awayreason; else msg->data1 = 0;
			ReplyMsg((struct Message *)msg);
			break;
		case APPENDFILE:
			AppendFile((char *)msg->data1, (char *)msg->data2, msg->data3);
			ReplyMsg((struct Message *)msg);
			break;
		case NODESPO:
			msg->data1 = (long)&sxnodes[1];
			ReplyMsg((struct Message *)msg);
			break;
		case REPORTCPS:
			if(msg->data2 == 1)
				mysprintf(sxnodes[msg->nodenum].action, "Download %ld cps", msg->data1);
			else
				mysprintf(sxnodes[msg->nodenum].action, "Upload %ld cps", msg->data1);
			ShowNode(msg->nodenum);
			ReplyMsg((struct Message *)msg);
			break;
		case DDNODES:
			msg->data1 = (long)&ddnodes[msg->data1];
			ReplyMsg((struct Message *)msg);
			break;
		case SXTODD:
			SXBase_To_DDBase((struct UserData *)msg->data1, (struct SXUserStruct *)msg->data2, (struct UserIndexStruct *)msg->data3, (struct DayDream_User *)msg->data4);
			ReplyMsg((struct Message *)msg);
		}
	}
}

void CallerLog(struct CallerLog *Caller)
{
	UWORD next, i;
	char *nullstruct;
	BPTR fh = Open("SX:LogFiles/Callers.LOG", MODE_OLDFILE);

	if(!fh)
	{
		fh = Open("SX:LogFiles/Callers.LOG", MODE_NEWFILE);
		if(!fh) return;
		next = 1;
		Write(fh, &next, 2);
		i = 0;
		nullstruct = AllocMem(sizeof(struct CallerLog), MEMF_CLEAR|MEMF_PUBLIC);
		if(!nullstruct)
		{
			Close(fh);
			DeleteFile("SX:LogFiles/Callers.LOG");
			return;
		}
		while(i < 30)
		{
			Write(fh, nullstruct, sizeof(struct CallerLog));
			i++;
		}
		FreeMem(nullstruct, sizeof(struct CallerLog));
		Seek(fh, 2, OFFSET_BEGINING);
	} else {
		Read(fh, &next, 2);
		Seek(fh, 2 + ((next-1)*sizeof(struct CallerLog)), OFFSET_BEGINING);
	}

	Write(fh, Caller, sizeof(struct CallerLog));
	Seek(fh, 0, OFFSET_BEGINING);
	next++;
	if(next > 30) next = 1;
	Write(fh, &next, 2);
	Close(fh);
}

void AppendFile(char *file, char *buffer, long size)
{
	BPTR fh = Open(file, MODE_READWRITE);
	if(fh)
	{
		Seek(fh, 0, OFFSET_END);
		Write(fh, buffer, size);
		Close(fh);
	}
}

#include <rexx/storage.h>

#pragma libcall RexxSysBase CreateArgstring 7e 0802
UBYTE *CreateArgstring( UBYTE *string, unsigned long length );

void ReturnString(UBYTE *string, struct RexxMsg *Rexxmessage)
{
	struct Library *RexxSysBase = OpenLibrary("rexxsyslib.library", 0);
	if(RexxSysBase)
	{
		Rexxmessage->rm_Result2 = (long) CreateArgstring(string, strlen(string));
		CloseLibrary(RexxSysBase);
	}
}

void CheckRexx(void)
{
	char *rexxpars;
	struct RexxMsg *Rexxmessage;

	while (Rexxmessage = (struct RexxMsg *)GetMsg(RexxPort))
	{
		Rexxmessage->rm_Result1 = 0;
		Rexxmessage->rm_Result2 = 0;

		rexxpars = strtok(ARG0(Rexxmessage), " ");

		if(strcmp(rexxpars, "TOTALNODES") == 0)
			Rexxmessage->rm_Result1 = ActiveNodes();

		if(strcmp(rexxpars, "ONLINENODES") == 0)
			Rexxmessage->rm_Result1 = OnlineNodes();

		if(strcmp(rexxpars, "NODEINFO") == 0)
		{
			UWORD node = atoi(strtok(ARG1(Rexxmessage), " "));

			switch(atoi(strtok(ARG2(Rexxmessage), " ")))
			{
			case 0:
				Rexxmessage->rm_Result1 = sxnodes[node].active;
				break;
			case 1:
				Rexxmessage->rm_Result1 = sxnodes[node].useron;
				break;
			case 2:
				ReturnString(sxnodes[node].UserIndex->handle, Rexxmessage);
				break;
			case 3:
				ReturnString(sxnodes[node].UserIndex->realname, Rexxmessage);
				break;
			case 4:
				ReturnString(sxnodes[node].User->Location, Rexxmessage);
				break;
			case 5:
				ReturnString(sxnodes[node].action, Rexxmessage);
				break;
			}
		}

		ReplyMsg((struct Message *)Rexxmessage);
	}
}

void UpdateDisplay(int i, char sel)
{
	UWORD y = WinTop+12, j;

	if(i<1) return;
	if(sel)
	{
		SetBPen(rp, 3);
		SetAPen(rp, 2);
	} else {
		SetAPen(rp, 1);
		if(scr->RastPort.BitMap->Depth > 2) SetBPen(rp, 5);
	}
	j = (i+1)-top;
	Move(rp, 8, y+(j*9));
	if(sxnodes[i].active == FALSE) sxnodes[i].action = "Shutdown";
	if(sxnodes[i].useron == TRUE)
	{
		if(mcp.flags&(1<<1))
			mysprintf(buf, "%-2ld %-24.24s %-24.24s %-25s", i, sxnodes[i].UserIndex->realname, sxnodes[i].User->Location, sxnodes[i].action);
		else
			mysprintf(buf, "%-2ld %-24.24s %-24.24s %-25s", i, sxnodes[i].User->Name, sxnodes[i].User->Location, sxnodes[i].action);
	} else {
		mysprintf(buf, "%-2ld                                                   %-25s", i, sxnodes[i].action);
	}
	Text(rp, buf, strlen(buf));
	SetBPen(rp, 0);
}

void MainConfig(void)
{
	long re;
	BPTR fh = Open("SX:Prefs/Main.dat", MODE_OLDFILE);
	if(fh)
	{
		MainCfg.minspace = 0;
		Read(fh, &MainCfg, sizeof(struct MainStruct));
		Close(fh);
	}
	if(MainCfg.nodes < 1) MainCfg.nodes = 3;
	if(MainCfg.nodes > 32) MainCfg.nodes = 32;

	fh = Open("SX:Prefs/MCP.DAT", MODE_OLDFILE);
	if(fh)
	{
		re = Read(fh, &mcp, sizeof(struct McpStruct));
		Close(fh);
		if(re < sizeof(struct McpStruct)) mcp.awayreason[0] = 0;
		if(mcp.awayreason[0]!=0) away = TRUE; else away = FALSE;
	}
	if(mcp.listsize < 2) mcp.listsize = 2;
	if(mcp.listsize > MainCfg.nodes)
		shownodes = MainCfg.nodes;
	else
		shownodes = mcp.listsize;
}

void WinRender(char showall)
{
	BevBox(TRUE,   6, WinTop+1,  21, WinTop+12, "N",2);
	BevBox(TRUE,  24, WinTop+1, 224, WinTop+12, "Name / Handle",2);
	BevBox(TRUE, 227, WinTop+1, 423, WinTop+12, "Location / Group",2);
	BevBox(TRUE, 426, WinTop+1, 614, WinTop+12, "Action / Door",2);

	BevBox(TRUE,   7, WinTop+14,632, WinTop+14+(shownodes*9),"",1); /* node list */

	if(scr->RastPort.BitMap->Depth > 2)
	{
		SetAPen(rp, 5);
		RectFill(rp, 8, WinTop+21, 631, WinTop+8+(shownodes*9));
	}

	BevBox(TRUE,240,ptop+12,632,WinBot,"",1);	/* info list */

	DrawBevelBox(rp,98,ptop+48,24,12,GT_VisualInfo,vi);

	if(away)
		TickON();
	else
		TickOFF();

	if(showall) ShowAll();
}

void StartWindow(void)
{
	scr = LockPubScreen(0L);
	WinTop = (scr->WBorTop)+(scr->Font->ta_YSize)+1;

	alts[0] = mcp.leftedge;
	alts[1] = mcp.topedge;
	alts[2] = 175;
	alts[3] = WinTop;

	glist = NULL;
	ptop = (shownodes*9)+16+WinTop;
	WinBot = WinTop + (shownodes*9) + 87;
	if ((vi = GetVisualInfo(scr, TAG_END)) != NULL)
	{
		gad = CreateContext(&glist);
        	ng.ng_TextAttr   = &topaz;
        	ng.ng_VisualInfo = vi;
        	ng.ng_Height     = 12;
        	ng.ng_LeftEdge   = 240;
        	ng.ng_TopEdge    = ptop;
        	ng.ng_Width      = 393;
        	ng.ng_GadgetText = "";
        	ng.ng_GadgetID   = GAD_CYCLE;
		ng.ng_Flags      = NG_HIGHLABEL;
		po[0] = "User Information";
		po[1] = "Last Callers";
		po[2] = "Last Uploads";
		po[3] = "Last Downloads";
		po[4] = "About System-X";
		po[5] = 0;
		gad = CreateGadget(CYCLE_KIND,  gad, &ng,
			GTCY_Labels,	po,
			TAG_END);

        	ng.ng_LeftEdge   = 616;
        	ng.ng_TopEdge    = WinTop+1;
        	ng.ng_Width      = 17;
        	ng.ng_GadgetText = "_I";
        	ng.ng_GadgetID   = GAD_ICONIFY;
		ng.ng_Flags      = 0;
        	gad = CreateGadget(BUTTON_KIND, gad, &ng,
 	                GT_Underscore, '_',
			TAG_END);

        	ng.ng_LeftEdge   = 6;
        	ng.ng_TopEdge    = ptop;
        	ng.ng_Width      = 116;
        	ng.ng_Height     = 12;
        	ng.ng_GadgetText = "_Open Screen";
        	ng.ng_GadgetID   = GAD_OPEN;
        	gad = CreateGadget(BUTTON_KIND, gad, &ng,
 	                GT_Underscore, '_',
			TAG_END);

        	ng.ng_LeftEdge   = 123;
        	ng.ng_GadgetText = "_Close Screen";
        	ng.ng_GadgetID   = GAD_CLOSE;
        	gad = CreateGadget(BUTTON_KIND, gad, &ng,
 	                GT_Underscore, '_',
			TAG_END);

        	ng.ng_LeftEdge   = 6;
        	ng.ng_TopEdge    = ptop+12;
        	ng.ng_GadgetText = "_Launch Node";
        	ng.ng_GadgetID   = GAD_LAUNCH;
        	gad = CreateGadget(BUTTON_KIND, gad, &ng,
 	                GT_Underscore, '_',
			TAG_END);

        	ng.ng_LeftEdge   = 123;
        	ng.ng_GadgetText = "_Shut down";
        	ng.ng_GadgetID   = GAD_SHUT;
        	gad = CreateGadget(BUTTON_KIND, gad, &ng,
 	                GT_Underscore, '_',
			TAG_END);

        	ng.ng_LeftEdge   = 6;
        	ng.ng_TopEdge    = ptop+24;
        	ng.ng_GadgetText = "_Enter Chat";
        	ng.ng_GadgetID   = GAD_CHAT;
        	gad = CreateGadget(BUTTON_KIND, gad, &ng,
 	                GT_Underscore, '_',
			TAG_END);

        	ng.ng_LeftEdge   = 123;
        	ng.ng_GadgetText = "_Kick User";
        	ng.ng_GadgetID   = GAD_KICK;
        	gad = CreateGadget(BUTTON_KIND, gad, &ng,
 	                GT_Underscore, '_',
			TAG_END);

        	ng.ng_LeftEdge   = 6;
        	ng.ng_TopEdge    = ptop+36;
        	ng.ng_GadgetText = "_MCP Config";
        	ng.ng_GadgetID   = GAD_MCPCONFIG;
        	gad = CreateGadget(BUTTON_KIND, gad, &ng,
 	                GT_Underscore, '_',
			TAG_END);

        	ng.ng_LeftEdge   = 123;
        	ng.ng_GadgetText = "S_hut Nodes";
        	ng.ng_GadgetID   = GAD_SHUTALL;
        	gad = CreateGadget(BUTTON_KIND, gad, &ng,
 	                GT_Underscore, '_',
			TAG_END);

        	ng.ng_LeftEdge   = 6;
        	ng.ng_TopEdge    = ptop+48;
        	ng.ng_Width      = 92;
        	ng.ng_GadgetText = "SysOp _Away";
        	ng.ng_GadgetID   = GAD_AWAY;
        	gad = CreateGadget(BUTTON_KIND, gad, &ng,
 	                GT_Underscore, '_',
			TAG_END);

        	ng.ng_LeftEdge   = 123;
        	ng.ng_Width      = 116;
        	ng.ng_GadgetText = "S_X-Config";
        	ng.ng_GadgetID   = GAD_CONFIG;
        	gad = CreateGadget(BUTTON_KIND, gad, &ng,
 	                GT_Underscore, '_',
			TAG_END);

        	ng.ng_LeftEdge   = 6;
        	ng.ng_TopEdge    = ptop+60;
        	ng.ng_GadgetText = mcp.text1;
        	ng.ng_GadgetID   = GAD_CONE;
        	gad = CreateGadget(BUTTON_KIND, gad, &ng,
 	                GT_Underscore, '_',
			TAG_END);

        	ng.ng_LeftEdge   = 123;
        	ng.ng_GadgetText = mcp.text2;
        	ng.ng_GadgetID   = GAD_CTWO;
        	CreateGadget(BUTTON_KIND, gad, &ng,
 	                GT_Underscore, '_',
			TAG_END);
	}

	win = OpenWindowTags(NULL,
		WA_Title,		"SX Master Control Program 1.06 (eSSeXX) "__DATE__"",
		WA_Left,		mcp.leftedge,
		WA_Top,			mcp.topedge,
		WA_Width,		640,
		WA_InnerHeight,		(shownodes*9)+89,
		WA_SizeGadget,		FALSE,
		WA_PubScreen,		scr,
		WA_Gadgets,		glist,
		WA_Zoom,		&alts,
		WA_IDCMP,		IDCMP_MOUSEBUTTONS |
					IDCMP_CLOSEWINDOW |
                            		IDCMP_VANILLAKEY |
					IDCMP_GADGETUP |
					IDCMP_NEWSIZE |
					IDCMP_RAWKEY,
		WA_Flags,		WFLG_NOCAREREFRESH |
					WFLG_ACTIVATE |
					WFLG_CLOSEGADGET |
					WFLG_DRAGBAR |
					WFLG_DEPTHGADGET |
					WFLG_SMART_REFRESH |
					WFLG_RMBTRAP,
		TAG_END);
	GT_RefreshWindow(win, NULL);
	UnlockPubScreen(0L, scr);
	rp = win->RPort;
	SetFont(rp, topazfont);
	WinRender(FALSE);
}

void CheckWin(void)
{
	struct IntuiMessage *message;
	struct Gadget *gad;
	UWORD code, mx, my, last;
	ULONG class;

	while (message = GT_GetIMsg(win->UserPort))
	{
	        gad   = (struct Gadget *)message->IAddress;
		code  = message->Code;
		class = message->Class;
		mx = message->MouseX;
		my = message->MouseY;
		GT_ReplyIMsg(message);

		switch (class)
		{
		case IDCMP_NEWSIZE:
			if(win->Width > 630)
			{
				SetWindowTitles(win, (UBYTE *)"SX Master Control Program 1.06 (eSSeXX) "__DATE__"", (UBYTE *)-1);
				WinRender(TRUE);
				switch(info)
				{
				case 0: SmoothInfoBox(); UserInfo(2); break;
				case 1:	ShowList(1); break;
				case 2: ShowList(2); break;
				case 3: ShowList(3); break;
				case 4:	About(); break;
				}
			} else
				SetWindowTitles(win, (UBYTE *)"SX-MCP 1.06", (UBYTE *)-1);
			break;

		case IDCMP_VANILLAKEY:
			switch(code)
			{
			case 'h':
			case 'H':
				ShutAll();
				break;
			case 'm':
			case 'M':
				SystemTagList("SX:SX-Config MCP", tags);
				break;
			case 'O':
			case 'o':
				MsgToNode(OPENSCREEN, sel);
				break;
			case 'C':
			case 'c':
				MsgToNode(CLOSESCREEN, sel);
				break;
			case 'L':
			case 'l':
				mysprintf(buf, "SX:SX %ld", sel);
				SystemTagList(buf, tags);
				break;
			case 'S':
			case 's':
				if(sxnodes[sel].useron == TRUE)
				{
					if(rtEZRequestTags("There is a user on that node.", "Kick User!|Cancel", NULL, NULL, TAG_END))
					{
						MsgToNode(KICKUSER, sel);
						//MsgToNode(SHUTDOWN, sel);
					}
				} else
					MsgToNode(SHUTDOWN, sel);
				break;
			case 'E':
			case 'e':
				MsgToNode(CHAT, sel);
				break;
			case 'K':
			case 'k':
				MsgToNode(KICKUSER, sel);
				break;
			case 'A':
			case 'a':
				strcpy(buf, mcp.awayreason);
				if(rtGetString(buf, 73, "Enter a reason for being away:", 0, 0)) strcpy(mcp.awayreason, buf);
				break;
			case 'X':
			case 'x':
	        		SystemTagList("SX:SX-Config", tags);
				break;
			}
			break;
		case IDCMP_CLOSEWINDOW:
			if(ActiveNodes())
			{
				if(rtEZRequestTags(
					"Shut down all nodes and Quit SX-MCP?", 
					"Shutdown|Cancel", NULL, NULL, TAG_END))
				{
					SHUTALL = TRUE;
					ShutAll();
				}
			} else
				done = TRUE;
			break;
		case IDCMP_GADGETUP:
			HandleGad(gad, code);
			break;
		case IDCMP_RAWKEY:
			if(code==77) /* down */
			{
				/*last=sel;*/
				sel++;
				if(sel > (top+shownodes-1) && sel < (MainCfg.nodes+1)) top++;
				if(sel > MainCfg.nodes) { sel=1; top=1; }
				ShowAll();
				if(info==0) UserInfo(1);
			}
			if(code==76) /* up */
			{
				/*last=sel;*/
				sel--;
				if(sel < top && sel!=0) top=sel;
				if(sel < 1) { sel=MainCfg.nodes; top=sel-shownodes+1; }
				ShowAll();
				if(info==0) UserInfo(1);
			}
			break;
		case IDCMP_MOUSEBUTTONS:
			if(code==104)	/* up = 232 */
			{
				if(mx > 5 && mx < 633)
				{
					if(my > WinTop+14 && my < WinTop+14+(shownodes*9))
					{
						last = sel;
						sel = (my-(WinTop+5))/9;
						if(sel > 0 && sel <= shownodes)
						{
							sel += top-1;
							UpdateDisplay(last,FALSE);
							UpdateDisplay(sel,TRUE);
							if(info==0) UserInfo(1);
							CurrentTime(&sec, &tic);
							if(DoubleClick(lastsec, lasttic, sec, tic) && last==sel)
							{
								if(sxnodes[sel].active)
                                        {
                                             MsgToNode(OPENSCREEN,sel);
							          MsgToNode(TOFRONT, sel);
								} else 
                                        {
								     mysprintf(buf, "SX:SX %ld", sel);
									SystemTagList(buf, tags);
                                             MsgToNode(OPENSCREEN,sel);
									MsgToNode(TOFRONT, sel);
								}
							} else {
								lastsec = sec;
								lasttic = tic;
							}
						} else
							sel = last;
					}
				}
				if(mx > 97 && mx < 124 && my > ptop+47 && my < ptop+61)
				{
					if(away)
					{
						away=FALSE;
						TickOFF();
					} else {
						away=TRUE;
						TickON();
					}
				}
			}
		}
	}
}

void ShowNode(UWORD node)
{
	if(!ICONIFY  &&  node >= top  &&  node < top+shownodes)
	{
		if(node == sel)
			UpdateDisplay(sel, TRUE);
		else
			UpdateDisplay(node, FALSE);
	}
}

void ShowAll(void)
{
	WORD j = top;
	while (j < (top+shownodes))
	{
		if(j!=sel) UpdateDisplay(j, FALSE);
		j++;
	}
	UpdateDisplay(sel, TRUE);
}

void HandleGad(struct Gadget *gad, UWORD code)
{
	switch(gad->GadgetID)
	{
	case GAD_SHUTALL:
		ShutAll();
		break;
	case GAD_MCPCONFIG:
		SystemTagList("SX:SX-Config MCP", tags);
		break;
	case GAD_ICONIFY:
		ICONIFY = TRUE;
		break;
	case GAD_CYCLE:
		EraseRect(rp, 241, ptop+13, 631, WinBot-1);
		info = code;
		switch(info)
		{
		case 0: UserInfo(2); break;
		case 1:	ShowList(1); blank = FALSE; break;
		case 2: ShowList(2); blank = FALSE; break;
		case 3: ShowList(3); blank = FALSE; break;
		case 4:	About(); blank = FALSE; break;
		}
		break;
	case GAD_CONFIG:
	        SystemTagList("SX:SX-Config", tags);
		break;
	case GAD_OPEN:
		MsgToNode(OPENSCREEN, sel);
		break;
	case GAD_CLOSE:
		MsgToNode(CLOSESCREEN, sel);
		break;
	case GAD_SHUT:
		if(sxnodes[sel].useron == TRUE)
		{
			if(rtEZRequestTags("There is a user on that node.", "Kick User!|Cancel", NULL, NULL, TAG_END))
			{
				MsgToNode(KICKUSER, sel);
				//MsgToNode(SHUTDOWN, sel);
			}
		} else
			MsgToNode(SHUTDOWN, sel);
		break;
	case GAD_KICK:
		if(sxnodes[sel].useron == TRUE)
		{
			if(rtEZRequestTags("Are you sure?", "Kick!|Cancel", NULL, NULL, TAG_END))
				MsgToNode(KICKUSER, sel);
		}
		break;
	case GAD_CHAT:
		MsgToNode(CHAT, sel);
		break;
	case GAD_LAUNCH:
		mysprintf(buf, "SX:SX %ld", sel);
		SystemTagList(buf, tags);
		break;
	case GAD_AWAY:
		strcpy(buf, mcp.awayreason);
		if(rtGetString(buf, 73, "Enter a reason for being away:", 0, 0))
			strcpy(mcp.awayreason, buf);
		break;
	case GAD_CONE:
		SystemTagList(mcp.cmd1, tags);
		break;
	case GAD_CTWO:
		SystemTagList(mcp.cmd2, tags);
		break;
	}
}

void UserInfo(int cls)
{
	if(!blank)
	{
		if(cls == 1) EraseRect(rp, 241, ptop+13, 631, WinBot-1);
		if(cls) SmoothInfoBox();
	}
 	if(sxnodes[sel].useron == FALSE) { blank=TRUE; return; }
	blank = FALSE;

	SetAPen(rp, 1);
	Move(rp, 294, ptop+21);
	if(sxnodes[sel].paged)
	{
		TextFmt(rp, "*%s*", sxnodes[sel].User->Name);
		//mysprintf(buf, "*%s*", sxnodes[sel].User->Name);
		//Text(rp, buf, strlen(buf));
	} else
		Text(rp, sxnodes[sel].User->Name, strlen(sxnodes[sel].User->Name));

	Move(rp, 294, ptop+30);
	TextFmt(rp, "%.18s", sxnodes[sel].User->Location);

	Move(rp, 490, ptop+21);
	TextFmt(rp, "%.17s", sxnodes[sel].UserIndex->realname);

	Move(rp, 490, ptop+30);
	TextFmt(rp, "%ld", sxnodes[sel].User->Sec_Status);

	Move(rp,294,ptop+39);
	TextFmt(rp, "%ld", sxnodes[sel].baud);

	Move(rp,490,ptop+39);
	Text(rp, sxnodes[sel].User->PhoneNumber, strlen(sxnodes[sel].User->PhoneNumber));

	Move(rp,294,ptop+48);
	TextFmt(rp, "%ld", sxnodes[sel].User->Slot_Number);

	Move(rp,490,ptop+48);
	TextFmt(rp, "%ld / %ld", sxnodes[sel].SXUser->fileratio, sxnodes[sel].SXUser->byteratio);

	Move(rp,294,ptop+57);
	Text(rp, sxnodes[sel].SXUser->computer, strlen(sxnodes[sel].SXUser->computer));

	Move(rp,490,ptop+57);
	TextFmt(rp, "%ld / %ld", sxnodes[sel].User->Time_Left/60, sxnodes[sel].User->Time_Limit/60);

	Move(rp,294,ptop+66);
	TextFmt(rp, "%ld/%ld", sxnodes[sel].User->Downloads, sxnodes[sel].User->Bytes_Download);

	Move(rp,490,ptop+66);
	TextFmt(rp, "%ld/%ld", sxnodes[sel].User->Uploads, sxnodes[sel].User->Bytes_Upload);

	SetAPen(rp,2);
	Move(rp,244,ptop+21);
	Text(rp,"User:",5);
	Move(rp,244,ptop+30);
	Text(rp,"City:",5);
	Move(rp,244,ptop+39);
	Text(rp,"Baud:",5);
	Move(rp,244,ptop+48);
	Text(rp,"Slot:",5);
	Move(rp,244,ptop+57);
	Text(rp,"Comp:",5);
	Move(rp,244,ptop+66);
	Text(rp,"Down:",5);

	Move(rp,440,ptop+21);
	Text(rp,"Real:",5);
	Move(rp,448,ptop+30);
	Text(rp,"Sec:",4);
	Move(rp,448,ptop+39);
	Text(rp,"Ph#:",4);
	Move(rp,424,ptop+48);
	Text(rp,"Ratios:",7);
	Move(rp,440,ptop+57);
	Text(rp,"Time:",5);
	Move(rp,448,ptop+66);
	Text(rp,"Ups:",4);
}

void ShowList(UWORD which)
{
	WORD j = 0;

	if(info != which) return;
	SetAPen(rp, 1);
	while(j < 6)
	{
		j++;
		Move(rp, 244, ptop+12+(j*9));
		switch(which)
		{
		case 1:
			Text(rp, lastcall[j], strlen(lastcall[j]));
			break;
		case 2:
			Text(rp, lastup[j], strlen(lastup[j]));
			break;
		case 3:
			Text(rp, lastdown[j], strlen(lastdown[j]));
			break;
		}
	}
}

void About(void)
{
	SetAPen(rp, 1);
	RectFill(rp, 241, ptop+13, 627, WinBot-1);
	Pic(405, ptop+14);
	RectFill(rp, 628, ptop+13, 631, WinBot-1);

	SetDrMd(rp, JAM1);

	SetAPen(rp, 6);
	Move(rp, 249, ptop+24);
	Text(rp, "SX 1.06b (eSSeXX)", 17);
	SetAPen(rp, 2);
	Move(rp, 250, ptop+25);
	Text(rp, "SX 1.06b (eSSeXX)", 17);


     SetAPen(rp, 6);
	Move(rp, 249, ptop+35);
	Text(rp, "dIGITALcORRUPTION", 17);
	SetAPen(rp, 2);
	Move(rp, 250, ptop+36);
	Text(rp, "dIGITALcORRUPTION", 17);


	SetAPen(rp, 3);
	Move(rp, 249, ptop+56);
	Text(rp, "Supported at :", 14);
	SetAPen(rp, 2);
	Move(rp, 250, ptop+57);
	Text(rp, "Supported at :", 14);

	SetAPen(rp, 3);
	Move(rp, 249, ptop+66);
	Text(rp, "www.luebeck.netsurf.de/~clasen", 30);
	SetAPen(rp, 2);
	Move(rp, 250, ptop+67);
	Text(rp, "www.luebeck.netsurf.de/~clasen", 30);

	SetDrMd(rp, JAM2);
}

void StartIcon(void)
{
	struct DiskObject *dobj;
	struct MsgPort *iconport;
	struct AppIcon *mcpicon;
	struct AppMessage *appmsg;
	char finicon = FALSE;
	long sign, iconsig;

	if(!WorkbenchBase) return;

	if(iconport = CreateMsgPort())
	{
		if(dobj = GetDiskObject("PROGDIR:SX-MCP"))
		{
			if(mcpicon = AddAppIcon(1, 0, "SX-MCP", iconport, 0, dobj, 0))
			{
				while(!finicon)
				{
					iconsig = 1L<<iconport->mp_SigBit;
					sign = Wait( mcpsign | iconsig | rexsign );
					if(sign & iconsig)
					{
						//appmsg = (struct AppMessage *)GetMsg(iconport);
                              appmsg = (struct AppMessage *)GetMsg(iconport);
						if(appmsg->am_NumArgs==0 && appmsg->am_ArgList==0) finicon = TRUE;
						ReplyMsg(appmsg);
					}
					if(sign & mcpsign) HandleMsg();
					if(sign & rexsign) CheckRexx();
				}
				RemoveAppIcon(mcpicon);
			}
			FreeDiskObject(dobj);
		}
		DeleteMsgPort(iconport);
	}
}

void LoadMenus(void)
{
	BPTR fh;
	UWORD j = 0;
	char rdone = FALSE;
	struct AnchorPath *Anch;

	Anch = AllocMem(sizeof(struct AnchorPath), MEMF_PUBLIC|MEMF_CLEAR);
	if(!Anch) return;
	MatchFirst("SX:Commands/#?.mnu", Anch);
	while(1)
	{
		j++;
		if(j < 201)
		{
			strmid(Anch->ap_Info.fib_FileName, menus[j].name, 1, strlen(Anch->ap_Info.fib_FileName)-4);
			menus[j].size = Anch->ap_Info.fib_Size;
		}
		if( MatchNext(Anch) != 0 ) break;
	}
	MatchEnd(Anch);
	FreeMem(Anch, sizeof(struct AnchorPath));
	max = j;
	if(max > 200) max = 200;

	j = 0;
	while(!rdone)
	{
		j++;
		if(j > max)
			rdone = TRUE;
		else {
			mysprintf(buf, "SX:Commands/%s.mnu", menus[j].name);
			fh = Open(buf, MODE_OLDFILE);
			if(fh)
			{
				menus[j].byso = AllocMem(menus[j].size, MEMF_PUBLIC|MEMF_CLEAR);
				if(menus[j].byso) Read(fh, menus[j].byso, menus[j].size);
				else menus[j].size=0;
				Close(fh);
			}
		}
	}
}

void FlushMenus(void)
{
	UWORD j = 0;

	while(j < max)
	{
		j++;
		if(menus[j].size && menus[j].byso) FreeMem(menus[j].byso, menus[j].size);
	}
}

void MsgToNode(int cmd, int node)
{
	struct MsgPort *nodeport;

	if(sxnodes[node].active)
	{
		mysprintf(buf, "SX-Node%ld", node);
		if(nodeport = FindPort(buf))
		{
			if(themsg.Msg.mn_ReplyPort = CreateMsgPort())
			{
				themsg.Msg.mn_Length = sizeof(struct NodeMessage);
				themsg.command = cmd;
				PutMsg(nodeport, (struct Message *)&themsg);
				WaitPort(themsg.Msg.mn_ReplyPort);
				GetMsg(themsg.Msg.mn_ReplyPort);
				DeleteMsgPort(themsg.Msg.mn_ReplyPort);
			}
		}
	}
}

UWORD ActiveNodes(void)
{
	UWORD j = 0, active = 0;

	while ( j < 32 )
	{
		j++;
		if(sxnodes[j].active) active++;
	}
	return(active);
}

UWORD OnlineNodes(void)
{
	UWORD j = 0, online = 0;

	while ( j < 32 )
	{
		j++;
		if(sxnodes[j].useron) online++;
	}
	return(online);
}

void SXBase_To_DDBase(struct UserData *TUser, struct SXUserStruct *TSXUser, struct UserIndexStruct *TIndex, struct DayDream_User *DDUser)
{
	strcpy(DDUser->user_handle,		TIndex->handle);
	strcpy(DDUser->user_realname,		TIndex->realname);
	strcpy(DDUser->user_organization,	TUser->Location);
	strcpy(DDUser->user_zipcity,		TUser->Location);
	strcpy(DDUser->user_voicephone,		TUser->PhoneNumber);
	strcpy(DDUser->user_computermodel,	TSXUser->computer);
	strcpy(DDUser->user_signature,		TSXUser->sentbyline);

	DDUser->user_freedlbytes	= TSXUser->freebytes;
	DDUser->user_freedlfiles	= TSXUser->freefiles;
	DDUser->user_securitylevel	= TUser->Sec_Status;
	DDUser->user_timeremaining	= TUser->Time_Left/60;
	DDUser->user_connections	= TUser->Times_Called;
	DDUser->user_fileratio		= TSXUser->fileratio;
	DDUser->user_byteratio		= TSXUser->byteratio;
	DDUser->user_ulbytes		= TUser->Bytes_Upload;
	DDUser->user_dlbytes		= TUser->Bytes_Download;
	DDUser->user_ulfiles		= TUser->Uploads;
	DDUser->user_dlfiles		= TUser->Downloads;
	DDUser->user_screenlength	= TUser->LineLength;
	DDUser->user_dailytimelimit	= TUser->Time_Limit/60;
	DDUser->user_firstcall		= TSXUser->firstcall - 252504000;
	DDUser->user_conferenceacc1	= TSXUser->ConfAccess[0];
	DDUser->user_conferenceacc2	= TSXUser->ConfAccess[1];
}

void ShutAll(void)
{
	UWORD j = 0;

	while ( j < 32 )
	{
		j++;
		if(sxnodes[j].useron)
		{
			mysprintf(buf, "There is a user on node %ld", j);
			rtEZRequestTags(buf, "Shit!", NULL, NULL, TAG_END);
			return;
		} else {
			if(sxnodes[j].active) MsgToNode(SHUTDOWN, j);
		}
	}
}
