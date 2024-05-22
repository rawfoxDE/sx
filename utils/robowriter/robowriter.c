
static char __version[] ="\0$VER: SX_RoboWriter 1.06 [eSSeXX] 68020 ("__DATE__")";

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <exec/types.h>
#include <exec/memory.h>
#include <devices/serial.h>
#include <clib/exec_protos.h>
#include <proto/dos.h>
#include "sxstructs.h"


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
		ULONG flags;
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


#define TEMPLATE "CONF/N/A,BASE/N/A,MESSAGE=MSG/K/A,FROM/K/A,TO/K/A,SUBJECT/K/A,PRIVATE/S,DATE/N"

void SaveMsg(char *file, struct MsgStruct *Msg, UWORD msgarea, UWORD confnum)
{
	char buf[256];
	BPTR fh = Open("SX:Prefs/Confs.DAT", MODE_OLDFILE);
	if(fh)
	{
		struct ConfStruct conf;
		UWORD i = 0;

		while(1)
		{
			if(Read(fh, &conf, sizeof(struct ConfStruct)))
			{
				i++;
				if(confnum == i) break;
			} else {
				Close(fh);
				return;
			}
		}
		Close(fh);

		sprintf(buf, "%sMessages/Base%u/Messages.DAT", conf.path, msgarea);
		fh = Open(buf, MODE_READWRITE);
		if(fh)
		{
			Seek(fh, 0, OFFSET_END);

			Msg->number = (Seek(fh,0,-1)/sizeof(struct MsgStruct)) + 1;

			sprintf(buf, "copy %s %sMessages/Base%u/Msgs%u", file, conf.path, msgarea, Msg->number);
			system(buf);

			Write(fh, Msg, sizeof(struct MsgStruct));
			Close(fh);
		}
	}
}

void main(void)
{
	LONG args[10];
	struct RDArgs *rargs;
	struct RDArgs *myrdargs;
	struct MsgStruct Msg;
	UWORD conf, base;
	char file[72];
	BOOL private;

	myrdargs = AllocDosObjectTags(DOS_RDARGS, NULL);
	if(!myrdargs)
	{
		PrintFault(IoErr(), NULL);
		exit(5);
	}

	rargs = (struct RDArgs *)ReadArgs(TEMPLATE, &args[0], myrdargs);
	if(!rargs)
	{
		PrintFault(IoErr(), NULL);
		FreeDosObject(DOS_RDARGS, myrdargs);
		exit(5);
	}

	if(args[0])	conf = *((LONG *)args[0]); else goto xit;
	if(args[1])	base = *((LONG *)args[1]); else goto xit;
	if(args[2])	strcpy(file,     strdup((STRPTR)args[2])); else goto xit;

	if(args[3])	strcpy(Msg.from, strdup((STRPTR)args[3])); else goto xit;
	if(args[4])	strcpy(Msg.to,   strdup((STRPTR)args[4])); else goto xit;
	if(args[5])	strcpy(Msg.subj, strdup((STRPTR)args[5])); else goto xit;

	if(args[6])	private = TRUE; else goto xit;

	if(args[7])	Msg.msgtime = *((LONG *)args[7]); else goto xit;

	SaveMsg(file, &Msg, base, conf);
xit:
	FreeArgs(rargs);
	FreeDosObject(DOS_RDARGS, myrdargs);
}
