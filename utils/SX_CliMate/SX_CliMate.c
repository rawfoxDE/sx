/* =========================== */
/* ====( SX_CliMate 1.04 )==== */
/* =========================== */


static const char __version[]="\0$VER: SX_CliMate 1.06 [eSSeXX] 68020 ("__DATE__")";


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <dos/dos.h>


/* ==== Messages To Nodes ==== */

#define SHOWFILE	9
#define SENDFILE	10


void mysprintf(char *Buffer,char *ctl, ...)
{
	RawDoFmt(ctl, (long *)(&ctl + 1), (void (*))"\x16\xc0\x4e\x75",Buffer);
}

void main(int argc, char *argv[])
{
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
	struct MsgPort *nodeport;
	char buf[16];
	BPTR fh;

	if(argc < 3 || strcmp(argv[1], "?")==0)
	{
		PutStr(	"SX_CliMate 1.06 (eSSeXX)\n\n"
			"Arguments:\n\n"
			"	SX_CliMate <command> <node> <file/string>\n\n"
			"Commands:\n\n"
			"	SEND	- Send a file to the user.\n"
			"	TEXT	- Show a string of text to user.\n"
			"	ANSI	- Show a ansi file to the user.\n\n");
		exit(0);
	}

	themsg.command = 0;

	if(stricmp(argv[1], "SEND")==0)
	{
		themsg.data1 = (long) argv[3];
		themsg.command = SENDFILE;
	}

	if(stricmp(argv[1], "TEXT")==0)
	{
		fh = Open("T:SX_CliMate.tmp", MODE_NEWFILE);
		if(!fh) exit(20);
		Write(fh, "\r\n", 2);
		Write(fh, argv[3], strlen(argv[3]));
		Write(fh, "\r\n", 2);
		Close(fh);
		themsg.data1 = (long) "T:SX_CliMate.tmp";
		themsg.command = SHOWFILE;
	}

	if(stricmp(argv[1], "ANSI")==0)
	{
		themsg.data1 = (long) argv[3];
		themsg.command = SHOWFILE;
	}

	if(themsg.command == 0)
	{
		PutStr("Unknown Command.\n");
		exit(10);
	}

	mysprintf(buf, "SX-Node%ld", atol(argv[2]));
	if(nodeport = FindPort(buf))
	{
		if(themsg.Msg.mn_ReplyPort = CreateMsgPort())
		{
			themsg.Msg.mn_Length = sizeof(struct NodeMessage);
			PutMsg(nodeport, (struct Message *)&themsg);
			WaitPort(themsg.Msg.mn_ReplyPort);
			DeleteMsgPort(themsg.Msg.mn_ReplyPort);
		}
	} else {
		PutStr("That node is not active.\n");
		exit(10);
	}

	DeleteFile("T:SX_CliMate.tmp");
}
