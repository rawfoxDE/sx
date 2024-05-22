
struct DoorsStruct
{
	char	chatdoor[52];
	UBYTE	chattype;
	char	msgdoor[52];
	UBYTE	msgtype;
	char	frontdoor[52];
	UBYTE	fronttype;
	char	msgdoorreply[52];
	UBYTE	msgdoorreplytype;
	char	nfdoor[52];
	UBYTE	nftype;
};

struct AccessStruct
{
	char name[32];
	UBYTE access;
	UBYTE flags;
	ULONG freefiles;
	ULONG freebytes;
	UWORD byteratio;
	UWORD fileratio;
	long Time_Limit;
	ULONG ConfAccess[10];
	char res[38];
};

struct MCPMessage
{
	struct Message Msg;
	UWORD command;
	UWORD nodenum;
	long data1;
	long data2;
	long data3;
	long data4;
	long data5;
	char *action;
};

struct Flag
{
	struct Node flag_node;
	char path[72];
	char file[34];
	ULONG size;
	ULONG xfer;
	ULONG cps;
	UWORD secs;
	UWORD mins;
	UWORD hours;
	UBYTE conf;
	UBYTE area;
	UBYTE free;
	UBYTE misc;
};

struct MainStruct
{
	char BBSName[64];
	char USERPath[64];
	char BBSLoc[64];
	char Sysop[64];
	char DNPath[64];
	char ULPath[64];
	long nodes;
	UWORD inac_timeout;
	UWORD inac_warn;
};

struct SerialStruct
{
	char device[32];
	WORD unit;
	WORD misc;
	long minrate;
	long dcerate;
	UBYTE sevenwire;
	UBYTE shared;
	char ring[16];
	char connect[24];
	char answer[16];
	char initstr[48];
};

struct ConfStruct
{
	char name[45];
	char path[55];
	char pass[16];
	char filepath[52];
	UWORD fileareas;
	UWORD uploadarea;
	UBYTE flf;
	UBYTE flags;
	char reserve[82];
};

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
		Flags; 	/* BIT 0 = NEW USER		*/
			/*     1 = MAIL SCAN		*/
			/*     2 = MAIL SCAN ASK	*/
			/*     3 = FILE SCAN		*/
			/*     4 = FILE SCAN ASK	*/
			/*     5 = LINE MSG EDITOR	*/
			/*     6 = DELETED!		*/
	};

struct UserIndexStruct
{
	char handle[31];
	char realname[31];
	UBYTE flags; /* BIT 6 = DELETED!	*/
	UBYTE unused;
};

struct ArchiveStruct
{
	char name[24];
	char patt[24];
	char testcmd[72];
	char unpackcmd[72];
	char packcmd[72];
	char viewcmd[72];
	char corrupt1[16];
	char corrupt2[16];
	char corrupt3[16];
	UBYTE flags;
	UBYTE viewaccess;
	char reserve[30];
};

struct ScreenModeStruct
{
	ULONG DisplayID;
	UWORD DisplayWidth;
	UWORD DisplayHeight;
	UWORD DisplayDepth;
	ULONG reserved;
	char fontname[32];
	UWORD fontsize;
	char device[32];
	WORD unit;
	ULONG flags;
};

struct DisplayStruct
{
	char name[16];
	char path[48];
	char ext[4];
	struct ScreenModeStruct ScreenMode;
	ULONG flags;
	char res[40];
};

struct ProtoStruct
{
	char name[24];
	char letter;
	char null;
	char type;
	char efficiency;
	char library[30];
	char initstr[30];
	char res[64];
};

struct NodeStruct
{
	char systempass[16];
	char newuserpass[16];
	char fcheck[42];
	long flags;
};

struct MsgBaseStruct			// <confpath>/MsgBases.DAT
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
};


struct MsgStruct		// baseX/messages.DAT
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
