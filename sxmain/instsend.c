
#include <stdlib.h>
#include <string.h>
#include <exec/memory.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <intuition/intuition.h>
#include <libraries/reqtools.h>
#include <proto/reqtools.h>
#include <devices/serial.h>
#include <xproto.h>
#include "SXStructs.h"
#include "sx.h"

extern struct StructsStruct	*Structs;
extern struct AmigaStruct	Amiga;
extern struct StringsStruct	*stg;
extern struct VariablesStruct	var;
extern struct XPR_IO		xio;
extern struct Library		*XProtocolBase;
extern struct ReqToolsBase	*ReqToolsBase;

extern UBYTE buf[512], charbuf[128];

extern LONG __saveds __asm xpr_ffirst(register __a0 STRPTR buffer,register __a1 STRPTR pattern);


void InstSend(void)
{
	struct rtFileRequester *filereq;
	struct rtFileList *flist, *tempflist;
	char fbuf[256], tbuf[256];

	if(!Carrier()) return;

	if(var.term)
	{
		Amiga.Flags = AllocMem(sizeof(struct List), MEMF_PUBLIC|MEMF_CLEAR);
		NewList(Amiga.Flags);
	}

	if (filereq = rtAllocRequestA (RT_FILEREQ, NULL))
	{
		fbuf[0] = 0;
		rtChangeReqAttr(filereq, RTFI_Dir, Structs->Cfg->ULPath, TAG_END);
		PS(Structs->SXStr[110]);
		flist = rtFileRequest (filereq, fbuf, "Files to send...",
			RT_LeftOffset,	20,
			RT_TopOffset,	11,
			RT_Screen, 	Amiga.scr,
			RTFI_Height,	300,
			RTFI_OkText,	"Send",
			RTFI_Flags,	FREQF_MULTISELECT,
			TAG_END);
		FixTitle();
		if(flist)
		{
			strcpy(fbuf, filereq->Dir);
			FixDir(fbuf);
			tempflist = flist;

			while(tempflist)
			{
				strcpy(tbuf, fbuf);
				strcat(tbuf, tempflist->Name);
				FlagFile(tbuf, TRUE, TRUE, TRUE);
				tempflist = tempflist->Next;
			}
			rtFreeFileList(flist);

			var.instsend = TRUE;
			xpr_ffirst(tbuf, 0);

			RawTransfer(1, TRUE, FALSE, Structs->NodePrefs.fcheck, tbuf);

		}
		rtFreeRequest(filereq);
	}

	if(var.term)
	{
		ClearFlags();
		FreeMem(Amiga.Flags, sizeof(struct List));
	}
}
