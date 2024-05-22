

static char __version[] ="\0$VER: SX_ShowAnsi 1.06 [eSSeXX] 68020 ("__DATE__")";

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <exec/types.h>
#include <exec/io.h>
#include <exec/memory.h>
#include <exec/io.h>
#include <intuition/intuition.h>
#include <graphics/displayinfo.h>
#include <graphics/view.h>
#include <intuition/screens.h>
#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>
#include <proto/dos.h>
#include <proto/exec.h>

void handleIDCMP ( struct Window * );
void ShowAnsi (char *f);

struct Library *GfxBase ,*IntuitionBase;
struct IOStdReq ioreq;
long x;

int main(int argc, char *argv[])
{
struct Window *win,*cwin;
struct Screen *scr;
struct ViewPort vp;

UWORD pens[]  = { 1,4,1,1,4,6,1,0,5,4,1,4,65535 };
UWORD color[] = { 0x0000, 0x0FFF, 0x00F0, 0x0FF0, 0x000F, 0x0F0F, 0x00FF, 0x0F00, 65535 };
/*                 black,    red,  green, yellow,   blue, purple,   aqua,  white */

IntuitionBase = OpenLibrary("intuition.library",37);

if(!argv[1]) exit(0); 

if (IntuitionBase != NULL)
    {
	GfxBase = OpenLibrary("graphics.library", 37);
        scr = OpenScreenTags(NULL,
			SA_Width,	640,
			SA_Height,	256,
			SA_Title,	"   Show Ansi 1.06 (eSSeXX)",
			SA_ShowTitle,	TRUE,
			SA_Type,	CUSTOMSCREEN,
			SA_DisplayID,	PAL_MONITOR_ID|HIRES_KEY,
                        SA_Pens,	(ULONG)pens,
                        SA_Depth,	3,
                        TAG_END);
        if ( scr != NULL )
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
                        TAG_END);
	cwin = OpenWindowTags(NULL,
			WA_Left,		0,
			WA_Top,			0,
			WA_Width,		19,
			WA_Height,		10,
			WA_CloseGadget,		TRUE,
			WA_DragBar,		FALSE,
			WA_DepthGadget,		FALSE,
			WA_SizeGadget,		FALSE,
			WA_IDCMP,		IDCMP_CLOSEWINDOW,
			WA_CustomScreen,	scr,
			TAG_END);

            if ( win != NULL )
                {
		ioreq.io_Data = win;
		ioreq.io_Length = sizeof(struct Window);
		x = OpenDevice("ibmcon.device", 0, &ioreq, 0);
		if(x==0)
          {
               ShowAnsi(argv[1]);
			handleIDCMP(cwin);
			CloseDevice(&ioreq);
			CloseWindow(cwin);
               CloseWindow(win);
		}
		}
            CloseScreen(scr);
            }
    CloseLibrary(GfxBase);
    CloseLibrary(IntuitionBase);
    }
}

void handleIDCMP(struct Window *win)
{
BOOL done = FALSE;
struct IntuiMessage *message;
ULONG class;
ULONG signals;

while (!done)
    {
    signals = Wait(1L << win->UserPort->mp_SigBit);
    if (signals & (1L << win->UserPort->mp_SigBit))
        {
        while ((!done) &&
               (message = (struct IntuiMessage *)GetMsg(win->UserPort)))
            {
            class = message->Class;
            ReplyMsg((struct Message *)message);

            switch (class)
                {
                case IDCMP_CLOSEWINDOW:
                    done = TRUE;
                    break;
                }
            }
        }
    }
}

void ShowAnsi (char *f)
{
	BPTR fh;
	int n;
	char *ansbuf;

	ansbuf = AllocMem(8192, MEMF_PUBLIC|MEMF_CLEAR);
	if(ansbuf!=0)
	{
		fh = Open(f,MODE_OLDFILE);
		if(fh!=0)
		{
			while (FGets(fh, ansbuf, 8192))
			{
				n = strlen(ansbuf);
				
                    ansbuf[n] = 13; // return
				n++;
				ansbuf[n] = 10; // linefeed
				ansbuf[n+1] = 0;// newline
		
          		ioreq.io_Data = ansbuf;
				ioreq.io_Length = n;
				ioreq.io_Command = CMD_WRITE;
				DoIO((struct IOStdReq *) &ioreq);
			}
                              
			Close(fh);
		}
		FreeMem(ansbuf, 8192);
	}
}
