
/* ================================================================== */
/* ======================= SCREEN AND MENUS ========================= */
/* ================================================================== */

#include <stdlib.h>
#include <string.h>
#include <exec/memory.h>
#include <exec/io.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
#include <proto/diskfont.h>
#include <proto/reqtools.h>
#include <proto/gadtools.h>
#include <intuition/intuition.h>
#include <graphics/displayinfo.h>
#include <clib/keymap_protos.h>
#include <libraries/reqtools.h>
#include <proto/reqtools.h>
#include <libraries/gadtools.h>
#include <devices/serial.h>
#include "sxstructs.h"
#include "sx.h"

extern struct StructsStruct	*Structs;
extern struct AmigaStruct	Amiga;
extern struct StringsStruct	*stg;
extern struct VariablesStruct	var;
extern struct TextAttr		ansia, topaz;
extern struct TextFont		*ansifont;

extern UBYTE buf[512], charbuf[128];
extern UWORD pens[], color[17];

struct NewMenu mynewmenu[] =
    {
        { NM_TITLE, "System-X",		 0 , 0, 0, 0,},
        {  NM_ITEM, "About",		"A", 0, 0, 0,},
        {  NM_ITEM, NM_BARLABEL,	 0 , 0, 0, 0,},
        {  NM_ITEM, "Quit...",		 0 , 0, 0, 0,},
        {   NM_SUB, "REALLY!",		 0 , 0, 0, 0,},

        { NM_TITLE, "Online",		 0 , 0, 0, 0,},
        {  NM_ITEM, "Send Files",       "S", 0, 0, 0,},
        {  NM_ITEM, "Receive Files",	"R", 0, 0, 0,},
        {  NM_ITEM, NM_BARLABEL,	 0 , 0, 0, 0,},
        {  NM_ITEM, "Add 1 Minute",	"+", 0, 0, 0,},
        {  NM_ITEM, "Sub 1 Minute",	"-", 0, 0, 0,},
        {  NM_ITEM, NM_BARLABEL,	 0 , 0, 0, 0,},
        {  NM_ITEM, "Hangup Modem",	 0 , 0, 0, 0,},
        {   NM_SUB, "DO IT!",		 0 , 0, 0, 0,},
        {  NM_ITEM, NM_BARLABEL,	 0 , 0, 0, 0,},
        {  NM_ITEM, "Enter Chat",	"T", 0, 0, 0,},
	{  NM_ITEM, "Disable Idle",	"B", HIGHCOMP|CHECKIT|MENUTOGGLE, 0, 0,},

        { NM_TITLE, "Offline",		 0 , 0, 0, 0,},
        {  NM_ITEM, "Local Login",	"L", 0, 0, 0,},
        {  NM_ITEM, "SysOp Login",	"Y", 0, 0, 0,},
        {  NM_ITEM, "Remote Login",	"G", 0, 0, 0,},
        {  NM_ITEM, NM_BARLABEL,	 0 , 0, 0, 0,},
        {  NM_ITEM, "Init Modem",	"I", 0, 0, 0,},
        {  NM_ITEM, "Reserve Node",	"N", 0, 0, 0,},
        {  NM_ITEM, "Local Shell",	"O", 0, 0, 0,},
        {  NM_ITEM, "Terminal Mode",	"W", HIGHCOMP|CHECKIT|MENUTOGGLE, 0, 0,},

        { NM_TITLE, "Misc",		 0 , 0, 0, 0,},
        {  NM_ITEM, "UserEd (Local)",	"E", 0, 0, 0,},
        {  NM_ITEM, "UserEd (Remote)",	"U", 0, 0, 0,},
        {  NM_ITEM, "ASCII Send",	"D", 0, 0, 0,},
        {  NM_ITEM, "Close Screen",	"\\",0, 0, 0,},
        {  NM_ITEM, "Ansi Capture",	"F", HIGHCOMP|CHECKIT|MENUTOGGLE, 0, 0,},

        { NM_TITLE, "Screen",		 0 , 0, 0, 0,},
        {  NM_ITEM, "Screen Pens",	"X", 0, 0, 0,},
        {  NM_ITEM, "Screen Mode",	"M", 0, 0, 0,},
        {  NM_ITEM, "Screen Palette",	"P", 0, 0, 0,},

        {   NM_END, NULL,		 0 , 0, 0, 0,},
    };

void GrabColors(UWORD *ccolor);
void FixTitle(void);
UWORD FindWhite(void);



void SavePalette(void)
{
	BPTR fh;

	if(var.cmode)
		mysprintf(buf, "SX:Node%ld/Prefs/Mode%ldPalette.DAT", var.node, var.cmode);
	else
		mysprintf(buf, "SX:Node%ld/Prefs/LocalDisPal.DAT", var.node);

	fh = Open(buf, MODE_NEWFILE);
	if(fh)
	{
		Write(fh, &color, 32);
		Write(fh, &pens, 24);
		Close(fh);
	}
}

void Palette(void)
{
	APTR reqinfo = rtAllocRequest(RT_REQINFO, NULL);

	if(reqinfo)
	{
		if(!var.cmode)
			strcpy(buf, "Palette: Local Display");
		else
			mysprintf(buf, "Palette for: '%s'", Structs->Disp.name);

		if(rtPaletteRequest(buf, reqinfo, RT_Window, Amiga.win, TAG_DONE) != -1)
		{
			GrabColors((UWORD *)&color);

			var.white = FindWhite();

			SavePalette();
		}
		rtFreeRequest(reqinfo);
	}
}

BOOL ScreenModeReq(void)
{
	struct rtScreenModeRequester *scrmodereq;

	if(scrmodereq = rtAllocRequestA (RT_SCREENMODEREQ, NULL))
	{
		scrmodereq->DisplayID = Structs->ScreenMode.DisplayID;
		scrmodereq->DisplayDepth = Structs->ScreenMode.DisplayDepth;
		scrmodereq->DisplayWidth = Structs->ScreenMode.DisplayWidth;
		scrmodereq->DisplayHeight = Structs->ScreenMode.DisplayHeight;

		if(!var.cmode)
			strcpy(buf, "Screen for Local Display");
		else
			mysprintf(buf, "Screen for: '%s'", Structs->Disp.name);

		if (rtScreenModeRequest (scrmodereq, buf,
			RT_Window,	Amiga.win,
			RTSC_Flags,	SCREQF_DEPTHGAD|SCREQF_SIZEGADS|SCREQF_GUIMODES,
			RTSC_MaxDepth,	4,
			TAG_END))
		{
			Structs->ScreenMode.DisplayID = scrmodereq->DisplayID;
			Structs->ScreenMode.DisplayWidth = scrmodereq->DisplayWidth;
			Structs->ScreenMode.DisplayHeight = scrmodereq->DisplayHeight;
			Structs->ScreenMode.DisplayDepth = scrmodereq->DisplayDepth;
			rtFreeRequest (scrmodereq);
			return(TRUE);
		}
		rtFreeRequest (scrmodereq);
	}
	return(FALSE);
}

BOOL Pens(void)
{
	char tit[128];
	UWORD i = 0;

	mysprintf(buf, "%ld%ld%ld%ld%ld%ld%ld%ld%ld%ld%ld%ld",
		pens[0],
		pens[1],
		pens[2],
		pens[3],
		pens[4],
		pens[5],
		pens[6],
		pens[7],
		pens[8],
		pens[9],
		pens[10],
		pens[11]);

	if(!var.cmode)
		strcpy(tit, "Pens for: 'Local Display'");
	else
		mysprintf(tit, "Pens for: '%s'", Structs->Disp.name);

	if(rtGetString(buf, 12, tit, 0, RT_Window, Amiga.win, TAG_DONE))
	{
		while(i < 12)
		{
			pens[i] = buf[i] - 48;
			i++;
		}
		SavePalette();
		FixTitle();
		return(TRUE);
	}
	FixTitle();
	return(FALSE);
}

void UpdateUserInfo(void)
{
	if(var.CON && !var.panel)
	{
		SetAPen(Amiga.rp, 14);

		Move(Amiga.rp,383,9);
		Text(Amiga.rp,"Slot:",5);

		Move(Amiga.rp,383,17);
		Text(Amiga.rp,"Accs:",5);

		Move(Amiga.rp,468,9);
		Text(Amiga.rp,"Ups:",4);

		Move(Amiga.rp,468,17);
		Text(Amiga.rp,"Dns:",4);

		SetAPen(Amiga.rp,var.white);

		Move(Amiga.rp,506,9);
		TextFmt(Amiga.rp, "%ld/%ldk", Structs->User.Uploads, Structs->User.Bytes_Upload/1024);

		Move(Amiga.rp,506,17);
		TextFmt(Amiga.rp, "%ld/%ldk", Structs->User.Downloads, Structs->User.Bytes_Download/1024);

		Move(Amiga.rp,429,9);
		TextFmt(Amiga.rp, "%ld", Structs->User.Slot_Number);

		Move(Amiga.rp,429,17);
		TextFmt(Amiga.rp, "%ld", Structs->User.Sec_Status);

		Move(Amiga.rp, 5, 9);
		if(var.paged)
		{
			SetAPen(Amiga.rp, 9);
			TextFmt(Amiga.rp, "*%-20.20s*", Structs->UserIndex.handle);
			SetAPen(Amiga.rp, var.white);
		} else
			TextFmt(Amiga.rp, "%-22.22s", Structs->UserIndex.handle);

		Move(Amiga.rp, 5, 17);
		TextFmt(Amiga.rp, "%-22.22s", Structs->UserIndex.realname);

		Move(Amiga.rp,191,9);
		TextFmt(Amiga.rp, "%-23.23s", Structs->User.Location);

		Move(Amiga.rp,191,17);
		TextFmt(Amiga.rp, "%-23.23s", Structs->SXUser.computer);
	}
	Servermsg(7);
}

void UpdateTimeLeft(void)
{
	if(var.CON)
	{
		if(var.panel)
		{
			Move(Amiga.rp, 346, 13);
			TextFmt(Amiga.rp, "%-9ld", var.bytesrec);

			Move(Amiga.rp, 515, 13);
			TextFmt(Amiga.rp, "%-11ld", var.bytessent);
		}
		Move(&Amiga.scr->RastPort, 420, 7);
		TextFmt(&Amiga.scr->RastPort, "%02ld:%02ld:%02ld", Structs->User.Time_Left/3600, (Structs->User.Time_Left/60)%60, Structs->User.Time_Left%60);
	}
}

void PanelPrompts(void)
{
	if(var.CALLER && var.CON)
	{
		EraseRect(Amiga.rp, 2, 2, 603, 18);
		if(var.USER)
		{
			Move(Amiga.rp, 53, 13);
			if(var.paged)
			{
				SetAPen(Amiga.rp, 9);
				TextFmt(Amiga.rp, "*%-19.19s*", Structs->UserIndex.handle);
			} else
				TextFmt(Amiga.rp, "%-21.21s", Structs->UserIndex.handle);
		}

		SetAPen(Amiga.rp, 14);

		Move(Amiga.rp, 8, 13);
		Text(Amiga.rp, "User:", 5);

		Move(Amiga.rp, 220, 13);
		Text(Amiga.rp, "Bytes Received:", 15);

		Move(Amiga.rp, 420, 13);
		Text(Amiga.rp, "Bytes Sent:", 11);

		SetAPen(Amiga.rp, var.white);
	}
}

UWORD FindWhite(void)	/* find the brightest color */
{
	struct ColorMap *cm=Amiga.scr->ViewPort.ColorMap;
	UWORD whit, j, col, ocol = 0;

	for(j=0;j<17;j++)
	{
		col = GetRGB4(cm,j);
		if(col > ocol)
		{
			ocol = col;
			whit = j;
		}
	}
	return(whit);
}

void FixTitle(void)
{
	WORD i = 0, x;

	SetAPen(&Amiga.scr->RastPort, var.white);
	RectFill(&Amiga.scr->RastPort, 20, 0, 617, 9);

	SetAPen(&Amiga.scr->RastPort, 14);
	RectFill(&Amiga.scr->RastPort, 0, 10, 617, 10);

	SetAPen(&Amiga.scr->RastPort, 12);
	SetBPen(&Amiga.scr->RastPort, var.white);
	Move(&Amiga.scr->RastPort, 25, 7);
	Text(&Amiga.scr->RastPort, stg->title, strlen(stg->title));
	/*RefreshWindowFrame(Amiga.cwin);*/

	if(var.cols < 8) return;
	if(var.cols == 8) x = 287; else x = 232;
	while(i < var.cols)
	{
		x += 10;
		if(var.cols==16) x += 10;
		if(var.cols==16 && i==8) x -= 150;
		SetAPen(&Amiga.scr->RastPort, i);
		RectFill(&Amiga.scr->RastPort, x, 1, x+8, 8);
		i++;
	}
	SetAPen(&Amiga.scr->RastPort, 12);
	/*SetBPen(&Amiga.scr->RastPort, var.white);*/
}

void InfoWinBorder(BOOL i)
{
	if(i) SetAPen(Amiga.rp, var.white); else SetAPen(Amiga.rp, 13);
	Move(Amiga.rp,1,1);
	Draw(Amiga.rp,639,1);
	Move(Amiga.rp,1,1);
	Draw(Amiga.rp,1,19);
	Move(Amiga.rp,1,19);
	if(i) SetAPen(Amiga.rp, 13); else SetAPen(Amiga.rp, var.white);
	Draw(Amiga.rp,639,19);
	Move(Amiga.rp,639,19);
	Draw(Amiga.rp,639,1);
	if(i) SetAPen(Amiga.rp, var.white);
}

void GrabColors(UWORD *ccolor)
{
	UWORD i = 0;

	while(i < var.cols)
	{
		ccolor[i] = GetRGB4(Amiga.scr->ViewPort.ColorMap, i);
		i++;
	}
}

void Colors(UWORD *ccolor)
{
	switch(Structs->ScreenMode.DisplayDepth)
	{
	case 1:
		var.cols = 2;
		break;
	case 2:
		var.cols = 4;
		break;
	case 4:
		var.cols = 16;
		break;
	default:
		var.cols = 8;
		break;
	}
	LoadRGB4(&Amiga.scr->ViewPort, ccolor, var.cols);
}

int OpenDisplay(BOOL sendio)
{
	UWORD winheight;

	if(var.CON) return(1);
	ansifont = OpenDiskFont(&ansia);
	if(!ansifont)
	{
		strcpy(ansia.ta_Name, "topaz.font");
		ansia.ta_YSize = 8;
		ansifont = OpenFont(&ansia);
	}
        Amiga.scr = OpenScreenTags(NULL,
		SA_Title,	"   System-X",
		SA_Width,	Structs->ScreenMode.DisplayWidth,
		SA_Height,	Structs->ScreenMode.DisplayHeight,
		SA_ShowTitle,	TRUE,
		SA_Type,	CUSTOMSCREEN,
		SA_DisplayID,	Structs->ScreenMode.DisplayID,
		SA_Pens,	(ULONG)pens,
		SA_Depth,	Structs->ScreenMode.DisplayDepth,
		SA_Font,	&topaz,
                TAG_END);
        if ( Amiga.scr != NULL )
	{
		Colors((UWORD *)&color);

		var.white = FindWhite();

		FixTitle();

		winheight = Structs->ScreenMode.DisplayHeight-33;

		Amiga.cwin = OpenWindowTags(NULL,
			WA_Left,		0,
			WA_Top,			0,
			WA_Width,		19,
			WA_Height,		11,
			WA_CloseGadget,		TRUE,
			WA_DragBar,		FALSE,
			WA_DepthGadget,		FALSE,
			WA_SizeGadget,		FALSE,
			WA_IDCMP,		IDCMP_CLOSEWINDOW | IDCMP_MENUPICK,
			WA_CustomScreen,	Amiga.scr,
			TAG_END);
		Amiga.infowin = OpenWindowTags(NULL,
			WA_Left,		0,
			WA_Top,			winheight+15,
			WA_Width,		Structs->ScreenMode.DisplayWidth,
			WA_Height,		21,
			WA_Borderless,		TRUE,
			WA_CloseGadget,		FALSE,
			WA_DragBar,		FALSE,
			WA_DepthGadget,		FALSE,
			WA_SizeGadget,		FALSE,
                        WA_CustomScreen, 	Amiga.scr,
			WA_IDCMP,		IDCMP_RAWKEY|IDCMP_MOUSEBUTTONS|IDCMP_ACTIVEWINDOW|IDCMP_INACTIVEWINDOW|IDCMP_MENUPICK,
                        TAG_END);
		Amiga.rp = Amiga.infowin->RPort;
		InfoWinBorder(1);

		Amiga.win = OpenWindowTags(NULL,
			WA_Left,		0,
			WA_Top,			12,
			WA_Width,		Structs->ScreenMode.DisplayWidth,
			WA_Height,		winheight,
			WA_CloseGadget,		FALSE,
			WA_DragBar,		FALSE,
			WA_DepthGadget,		FALSE,
			WA_SizeGadget,		FALSE,
                        WA_CustomScreen, 	Amiga.scr,
			WA_IDCMP,		IDCMP_MENUPICK,
			WA_Flags,		WFLG_BORDERLESS |
						WFLG_ACTIVATE,
                        TAG_END);

		SetFont(Amiga.win->RPort, ansifont);

		if(var.panel) { InP(); PanelPrompts(); } else OutP();

		Amiga.conreadreq.io_Message.mn_ReplyPort=Amiga.ReadConPort;
		Amiga.conreadreq.io_Data = Amiga.win;
		Amiga.conreadreq.io_Length = sizeof(struct Window);
		OpenDevice("console.device", 0, &Amiga.conreadreq, 0);

		Amiga.conwritereq.io_Data = Amiga.win;
		Amiga.conwritereq.io_Length = sizeof(struct Window);

		if(strcmp(Structs->ScreenMode.device, "console.device")==0)
		{
usecon:			var.usedconsole = TRUE;
			Amiga.conwritereq = Amiga.conreadreq;
		} else {
			if(OpenDevice(Structs->ScreenMode.device, Structs->ScreenMode.unit, &Amiga.conwritereq, 0) != 0)
			{
				strcpy(Structs->ScreenMode.device, "console.device");
				goto usecon;
			}
			var.usedconsole = FALSE;
		}

		Amiga.conwritereq.io_Message.mn_ReplyPort=Amiga.WriteConPort;
		Amiga.conwritereq.io_Command = CMD_WRITE;

		Amiga.conreadreq.io_Command = CMD_READ;
		Amiga.conreadreq.io_Data = &stg->conbuf;
		Amiga.conreadreq.io_Length = 1;

		if(sendio) SendIO(&Amiga.conreadreq);

		var.CON = TRUE;
		if(var.USER)
		{
			UpdateUserInfo();
			UpdateTimeLeft();
		} else
			DrawImage(Amiga.rp, Amiga.sxbbsImage, (Structs->ScreenMode.DisplayWidth-236)/2, 5);

		Amiga.menuStrip = 0;
		if(Amiga.vi = GetVisualInfoA(Amiga.scr, 0))
		{
			if(var.term)
				mynewmenu[25].nm_Flags |= CHECKED;
			else
				mynewmenu[25].nm_Flags = HIGHCOMP|CHECKIT|MENUTOGGLE;

			if(Amiga.capfh)
				mynewmenu[31].nm_Flags |= CHECKED;
			else
				mynewmenu[31].nm_Flags = HIGHCOMP|CHECKIT|MENUTOGGLE;

			if(Amiga.menuStrip = CreateMenusA(mynewmenu, 0))
			{
				LayoutMenusA(Amiga.menuStrip, Amiga.vi, 0);
				SetMenuStrip(Amiga.win, Amiga.menuStrip);
				ResetMenuStrip(Amiga.infowin, Amiga.menuStrip);
				ResetMenuStrip(Amiga.cwin, Amiga.menuStrip);

				if(var.term)
				{
					OnMenu(Amiga.win, FULLMENUNUM(1, -1, 0));
					OffMenu(Amiga.win, FULLMENUNUM(1, 3, 0));
					OffMenu(Amiga.win, FULLMENUNUM(1, 4, 0));
					OffMenu(Amiga.win, FULLMENUNUM(1, 8, 0));
					OffMenu(Amiga.win, FULLMENUNUM(1, 9, 0));

					OffMenu(Amiga.win, FULLMENUNUM(2, 0, 0));
					OffMenu(Amiga.win, FULLMENUNUM(2, 1, 0));
					OffMenu(Amiga.win, FULLMENUNUM(2, 2, 0));
					OffMenu(Amiga.win, FULLMENUNUM(2, 4, 0));
					OffMenu(Amiga.win, FULLMENUNUM(2, 5, 0));
					OffMenu(Amiga.win, FULLMENUNUM(2, 6, 0));

					OffMenu(Amiga.win, FULLMENUNUM(3, 0, 0));
					OffMenu(Amiga.win, FULLMENUNUM(3, 1, 0));
					OffMenu(Amiga.win, FULLMENUNUM(3, 2, 0));
				} else {
					if(var.CALLER)
					{
						OffMenu(Amiga.win, FULLMENUNUM(2, -1, 0));
						OnMenu(Amiga.win, FULLMENUNUM(1, -1, 0));
					} else {
						OnMenu(Amiga.win, FULLMENUNUM(2, -1, 0));
						OffMenu(Amiga.win, FULLMENUNUM(1, -1, 0));
					}
				}
			} else
				FreeVisualInfo(Amiga.vi);
		}
	} else {
		CloseFont(ansifont);

		rtEZRequestTags("Screen failed to open.",
				"OK", NULL, NULL, RTEZ_ReqTitle, "Error",
				TAG_DONE);

		return(0);
	}
	return(1);
}

void CloseDisplay(void)
{
	if(!var.CON) return;

	AbortIO(&Amiga.conreadreq);
	WaitIO(&Amiga.conreadreq);		/* close console */
	CloseDevice(&Amiga.conreadreq);
	if(!var.usedconsole) CloseDevice(&Amiga.conwritereq);

	if(Amiga.menuStrip)
	{
		ClearMenuStrip(Amiga.win);
		ClearMenuStrip(Amiga.cwin);
		ClearMenuStrip(Amiga.infowin);
		FreeMenus(Amiga.menuStrip);
		FreeVisualInfo(Amiga.vi);
	}

	if(Amiga.xferwin)CloseWindow(Amiga.xferwin);
        if(Amiga.win)CloseWindow(Amiga.win);
	if(Amiga.infowin)CloseWindow(Amiga.infowin);	/* close windows */
	if(Amiga.cwin)CloseWindow(Amiga.cwin);
	if(Amiga.scr)CloseScreen(Amiga.scr);
	if(ansifont)CloseFont(ansifont);
	var.CON = FALSE;
}
