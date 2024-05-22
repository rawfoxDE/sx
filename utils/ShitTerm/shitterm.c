/*

shitterm

*/

static const char __version[]="\0$VER: ShitTerm 1.1 ("__DATE__")";

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <exec/types.h>
#include <exec/memory.h>
#include <exec/io.h>
#include <intuition/intuition.h>
#include <graphics/displayinfo.h>
#include <intuition/screens.h>
#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>
#include <proto/dos.h>
#include <devices/serial.h>
#include <clib/alib_protos.h>


void FixTitle(struct Screen *scr);
VOID handleIDCMP ( struct Window * );
void OpenSerial(void);
void SerWrite(APTR data, ULONG size);
void ConWrite(APTR data, ULONG size);

struct Library *IntuitionBase, *GfxBase;
struct MsgPort *ReadSerPort, *WriteSerPort, *ReadConPort, *WriteConPort;
struct IOStdReq conreadreq, conwritereq;
struct IOExtSer serreadreq, serwritereq;

char serdev[32], conbuf[16], serbuf[2048];

UWORD serunit;


int main(int argc, char *argv[])
{
	struct Window *win, *cwin;
	struct Screen *scr;
	struct ViewPort vp;
	struct TextAttr topaz = {"topaz.font",8,0,0};
	long x;

	UWORD pens[]  = { 7,4,7,7,6,4,7,0,5,4,7,4,65535 };
	UWORD color[] = { 0x0000, 0x0F00, 0x00F0, 0x0FF0, 0x000F, 0x0F0F, 0x00FF, 0x0FFF, 65535 };
	/*                 black,    red,  green, yellow,   blue, purple,   aqua,  white */

	strcpy(serdev, argv[1]);
	serunit = atoi(argv[2]);

	if(argc<2)
	{
		PutStr("ShitTerm <serialdevice> <serialunit>\n");
		exit(0);
	}

	OpenSerial();

	IntuitionBase = OpenLibrary("intuition.library", 36);
	if (IntuitionBase)
	{
		GfxBase = OpenLibrary("graphics.library", 36);
		scr = OpenScreenTags(NULL,
			SA_Width,	640,
			SA_Height,	256,
			SA_Title,	"   Shit Term 1.1",
			SA_ShowTitle,	TRUE,
			SA_Type,	CUSTOMSCREEN,
			SA_DisplayID,	PAL_MONITOR_ID|HIRES_KEY,
                        SA_Pens,	(ULONG)pens,
                        SA_Depth,	3,
			SA_Font,	&topaz,
                        TAG_END);
		if (scr)
		{
			vp = scr->ViewPort;
			LoadRGB4(&vp, (UWORD *)&color, 8);
			win = OpenWindowTags(NULL,
				WA_Left,		0,
				WA_Top,			12,
				WA_Width,		640,
				WA_Height,		245,
				WA_Borderless,		TRUE,
				WA_CloseGadget,		FALSE,
				WA_DragBar,		FALSE,
				WA_DepthGadget,		FALSE,
				WA_SizeGadget,		FALSE,
        	                WA_CustomScreen, 	scr,
				WA_Activate,		TRUE,
				WA_RMBTrap,		TRUE,
        	                TAG_END);
			cwin = OpenWindowTags(NULL,
				WA_Left,		0,
				WA_Top,			0,
				WA_Width,		19,
				WA_Height,		11,
				WA_CloseGadget,		TRUE,
				WA_DragBar,		FALSE,
				WA_DepthGadget,		FALSE,
				WA_SizeGadget,		FALSE,
				WA_IDCMP,		IDCMP_CLOSEWINDOW,
				WA_CustomScreen,	scr,
				WA_RMBTrap,		TRUE,
				TAG_END);
			if (win && cwin)
			{
				FixTitle(scr);
				RefreshWindowFrame(cwin);

				ReadConPort = CreateMsgPort();
				WriteConPort = CreateMsgPort();

				conreadreq.io_Message.mn_ReplyPort=ReadConPort;
				conreadreq.io_Data = win;
				conreadreq.io_Length = sizeof(struct Window);
				conreadreq.io_Command = CMD_READ;
				OpenDevice("console.device", 0, &conreadreq, 0);

				conwritereq.io_Message.mn_ReplyPort=WriteConPort;
				conwritereq.io_Data = win;
				conwritereq.io_Length = sizeof(struct Window);
				conwritereq.io_Command = CMD_WRITE;
				x = OpenDevice("ibmcon.device", 0, &conwritereq, 0);
				if(x!=0) PutStr("This program requires ibmcon.device\n");

#define CREDITS "\r\n›33;44m Shit Term 1.1 By Zed of Digital Corruption © 1997. ›m\r\n\r\n"

				ConWrite(CREDITS, strlen(CREDITS));

				conreadreq.io_Data = (APTR)&conbuf;
				conreadreq.io_Length = 16;
				SendIO(&conreadreq);

				handleIDCMP(cwin);

				CloseDevice(&conwritereq);
				CloseDevice(&conreadreq);
				DeleteMsgPort(WriteConPort);
				DeleteMsgPort(ReadConPort);

				CloseWindow(cwin);
				CloseWindow(win);

				AbortIO(&serreadreq);
				WaitIO(&serreadreq);
				CloseDevice(&serreadreq);
				DeleteMsgPort(ReadSerPort);
				DeleteMsgPort(WriteSerPort);
			}
			CloseScreen(scr);
    	        }
    		CloseLibrary(GfxBase);
    		CloseLibrary(IntuitionBase);
	}
}

void FixTitle(struct Screen *scr)
{
	SetAPen(&scr->RastPort, 15);
	RectFill(&scr->RastPort, 0, 0, 617, 9);
	SetAPen(&scr->RastPort, 14);
	RectFill(&scr->RastPort, 0, 10, 617, 10);
	SetAPen(&scr->RastPort, 12);
	SetBPen(&scr->RastPort, 15);
	Move(&scr->RastPort, 25, 7);

#define title "ShitTerm 1.1 By Zed/DC"

	Text(&scr->RastPort, title, strlen(title));
	SetAPen(&scr->RastPort, 12);
	SetBPen(&scr->RastPort, 15);
}

VOID handleIDCMP(struct Window *win)
{
	BOOL done = FALSE;
	struct IntuiMessage *message;
	struct Message *msg;
	ULONG sign;
	ULONG closesig = 1L << win->UserPort->mp_SigBit;
	ULONG sersig   = 1L << ReadSerPort->mp_SigBit;
	ULONG consig   = 1L << ReadConPort->mp_SigBit;

	while (!done)
	{
		sign = Wait( closesig | sersig | consig );
		if(sign & closesig)
		{
			while ((message = (struct IntuiMessage *)GetMsg(win->UserPort)))
			{
				if(message->Class == IDCMP_CLOSEWINDOW) done = TRUE;
            			ReplyMsg((struct Message *)message);
			}
            	}
		if(sign & sersig)
		{
			if (msg = GetMsg(ReadSerPort))
			{
				if(msg == (struct Message *)&serreadreq)
				{
					if(serreadreq.IOSer.io_Actual > 0) ConWrite(serbuf, serreadreq.IOSer.io_Actual);
				}
				serreadreq.IOSer.io_Command = SDCMD_QUERY;
				DoIO(&serreadreq);
				serreadreq.IOSer.io_Command = CMD_READ;
				serreadreq.IOSer.io_Length = serreadreq.IOSer.io_Actual;
				if(serreadreq.IOSer.io_Length>2048) serreadreq.IOSer.io_Length=2048;
				if(serreadreq.IOSer.io_Length<1) serreadreq.IOSer.io_Length=1;
				SendIO(&serreadreq);
			}
		}
		if(sign & consig)
		{
			if (msg = GetMsg(ReadConPort))
			{
				if(msg == (struct Message *)&conreadreq)
				{
					if(conreadreq.io_Actual > 0) SerWrite(conbuf, conreadreq.io_Actual);
				}
				SendIO(&conreadreq);
			}
		}
	}
}

void OpenSerial(void)
{
	long x;

	ReadSerPort = CreateMsgPort();
	WriteSerPort = CreateMsgPort();

	serreadreq.IOSer.io_Message.mn_ReplyPort=ReadSerPort;

	x = OpenDevice(serdev, serunit, (struct IORequest *)&serreadreq, 0);

	if(x!=0)
	{
		DeleteMsgPort(ReadSerPort);
		DeleteMsgPort(WriteSerPort);
		PutStr("Cannot open serial device!\n");
		exit(10);
	}

	serwritereq=serreadreq;
	serwritereq.IOSer.io_Message.mn_ReplyPort=WriteSerPort;
	serwritereq.IOSer.io_Command	= SDCMD_SETPARAMS;
	serwritereq.io_Baud	 	= 19200;
	serwritereq.io_SerFlags 	= SERF_SHARED|SERF_7WIRE|SERF_RAD_BOOGIE|SERF_XDISABLED;
	serwritereq.io_RBufLen		= 16384L;
	serwritereq.io_CtlChar		= 0x11130000L;
	DoIO(&serwritereq);

	serwritereq.IOSer.io_Command 	= CMD_WRITE;
	serwritereq.IOSer.io_Flags	= IOF_QUICK;

	serreadreq.IOSer.io_Command 	= CMD_READ;
	serreadreq.IOSer.io_Data 	= serbuf;
	serreadreq.IOSer.io_Length 	= 1;
	SendIO(&serreadreq);
}

void SerWrite(APTR data, ULONG size)
{
	serwritereq.IOSer.io_Data = data;
	serwritereq.IOSer.io_Length = size;
	DoIO(&serwritereq);
}

void ConWrite(APTR data, ULONG size)
{
	conwritereq.io_Data = data;
	conwritereq.io_Length = size;
	DoIO(&conwritereq);
}
