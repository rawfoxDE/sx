#ifndef MAKE_ID
#define MAKE_ID(a,b,c,d) ((ULONG) (a)<<24 | (ULONG) (b)<<16 | (ULONG) (c)<<8 | (ULONG) (d))
#endif

/* Libraries */
#include <libraries/mui.h>
#include <libraries/gadtools.h> /* for Barlabel in MenuItem */
#include <exec/memory.h>

/* Prototypes */
#include <proto/muimaster.h>
#include <proto/exec.h>
#ifdef __GNUC__
#include <proto/alib.h>
#else
#include <clib/alib_protos.h>
#endif /* __GNUC__ */

#include "SX-MCPGUI.h"

struct ObjApp * CreateApp(void)
{
	struct ObjApp * ObjectApp;

	APTR	MCP-MainWIN, GR_Head, GR_Nodelist, LV_NodeList, GR_MainButt, GR_Buttons;
	APTR	GR_ButtRight, BT_CloseScr, BT_OpenScr, BT_Chat, CY_Sysop, BT_MCPConfig;
	APTR	GR_ButtLeft, BT_Launch, BT_Shut, BT_ShutAll, BT_Kick, BT_Config, GR_Info;
	APTR	CY_label_0, LV_label_2;

	if (!(ObjectApp = AllocVec(sizeof(struct ObjApp),MEMF_CLEAR)))
		return(NULL);

	ObjectApp->CY_SysopContent[0] = "Sysop Away";
	ObjectApp->CY_SysopContent[1] = "Sysop Here";
	ObjectApp->CY_SysopContent[2] = NULL;
	ObjectApp->CY_label_0Content[0] = "User Information";
	ObjectApp->CY_label_0Content[1] = "Last Callers";
	ObjectApp->CY_label_0Content[2] = "Last Uploads";
	ObjectApp->CY_label_0Content[3] = "Last Downloads";
	ObjectApp->CY_label_0Content[4] = "Userbutton Bank";
	ObjectApp->CY_label_0Content[5] = "About System-X";
	ObjectApp->CY_label_0Content[6] = NULL;

	ObjectApp->BT_N = TextObject,
		ButtonFrame,
		MUIA_Weight, 16,
		MUIA_Background, MUII_ButtonBack,
		MUIA_Text_Contents, "N",
		MUIA_Text_PreParse, "\033c",
		MUIA_HelpNode, "BT_N",
		MUIA_InputMode, MUIV_InputMode_RelVerify,
	End;

	ObjectApp->BT_Name = SimpleButton("Name / Handle");

	ObjectApp->BT_Location = SimpleButton("Location / Group");

	ObjectApp->BT_Action = SimpleButton("Action / Door");

	ObjectApp->BT_I = TextObject,
		ButtonFrame,
		MUIA_Weight, 16,
		MUIA_Background, MUII_ButtonBack,
		MUIA_Text_Contents, "I",
		MUIA_Text_PreParse, "\033c",
		MUIA_HelpNode, "BT_I",
		MUIA_InputMode, MUIV_InputMode_RelVerify,
	End;

	GR_Head = GroupObject,
		MUIA_Group_Horiz, TRUE,
		Child, ObjectApp->BT_N,
		Child, ObjectApp->BT_Name,
		Child, ObjectApp->BT_Location,
		Child, ObjectApp->BT_Action,
		Child, ObjectApp->BT_I,
	End;

	LV_NodeList = FloattextObject,
		MUIA_Frame, MUIV_Frame_InputList,
		MUIA_List_Active, MUIV_List_Active_Top,
	End;

	LV_NodeList = ListviewObject,
		MUIA_Listview_DoubleClick, TRUE,
		MUIA_Listview_List, LV_NodeList,
	End;

	GR_Nodelist = GroupObject,
		Child, LV_NodeList,
	End;

	BT_CloseScr = TextObject,
		ButtonFrame,
		MUIA_Weight, 20,
		MUIA_Background, MUII_ButtonBack,
		MUIA_Text_Contents, "Close Screen",
		MUIA_Text_PreParse, "\033c",
		MUIA_InputMode, MUIV_InputMode_RelVerify,
	End;

	BT_OpenScr = TextObject,
		ButtonFrame,
		MUIA_Weight, 20,
		MUIA_Background, MUII_ButtonBack,
		MUIA_Text_Contents, "Open Screen",
		MUIA_Text_PreParse, "\033c",
		MUIA_InputMode, MUIV_InputMode_RelVerify,
	End;

	BT_Chat = TextObject,
		ButtonFrame,
		MUIA_Weight, 20,
		MUIA_Background, MUII_ButtonBack,
		MUIA_Text_Contents, "Enter Chat",
		MUIA_Text_PreParse, "\033c",
		MUIA_InputMode, MUIV_InputMode_RelVerify,
	End;

	CY_Sysop = CycleObject,
		MUIA_Weight, 20,
		MUIA_Cycle_Entries, ObjectApp->CY_SysopContent,
	End;

	BT_MCPConfig = TextObject,
		ButtonFrame,
		MUIA_Weight, 20,
		MUIA_Background, MUII_ButtonBack,
		MUIA_Text_Contents, "MCP-Config",
		MUIA_Text_PreParse, "\033c",
		MUIA_InputMode, MUIV_InputMode_RelVerify,
	End;

	GR_ButtRight = GroupObject,
		MUIA_Weight, 35,
		MUIA_Group_SameWidth, TRUE,
		Child, BT_CloseScr,
		Child, BT_OpenScr,
		Child, BT_Chat,
		Child, CY_Sysop,
		Child, BT_MCPConfig,
	End;

	BT_Launch = TextObject,
		ButtonFrame,
		MUIA_Weight, 50,
		MUIA_Background, MUII_ButtonBack,
		MUIA_Text_Contents, "Launch Node",
		MUIA_Text_PreParse, "\033c",
		MUIA_InputMode, MUIV_InputMode_RelVerify,
	End;

	BT_Shut = SimpleButton("Shut Node");

	BT_ShutAll = SimpleButton("Shut down");

	BT_Kick = SimpleButton("Kick User");

	BT_Config = SimpleButton("SX-Config");

	GR_ButtLeft = GroupObject,
		MUIA_Weight, 35,
		MUIA_Group_SameWidth, TRUE,
		Child, BT_Launch,
		Child, BT_Shut,
		Child, BT_ShutAll,
		Child, BT_Kick,
		Child, BT_Config,
	End;

	GR_Buttons = GroupObject,
		MUIA_Weight, 35,
		MUIA_Group_Horiz, TRUE,
		MUIA_Group_SameWidth, TRUE,
		Child, GR_ButtRight,
		Child, GR_ButtLeft,
	End;

	CY_label_0 = CycleObject,
		MUIA_Cycle_Entries, ObjectApp->CY_label_0Content,
	End;

	LV_label_2 = ListObject,
		MUIA_FramePhantomHoriz, TRUE,
	End;

	LV_label_2 = ListviewObject,
		MUIA_Listview_Input, FALSE,
		MUIA_Listview_List, LV_label_2,
	End;

	GR_Info = GroupObject,
		MUIA_Weight, 30,
		MUIA_Background, MUII_FILLSHINE,
		Child, CY_label_0,
		Child, LV_label_2,
	End;

	GR_MainButt = GroupObject,
		MUIA_Group_Horiz, TRUE,
		MUIA_Group_SameHeight, TRUE,
		Child, GR_Buttons,
		Child, GR_Info,
	End;

	MCP-MainWIN = GroupObject,
		Child, GR_Head,
		Child, GR_Nodelist,
		Child, GR_MainButt,
	End;

	ObjectApp->WI_label_0 = WindowObject,
		MUIA_Window_Title, "window_title",
		MUIA_Window_ID, MAKE_ID('0', 'W', 'I', 'N'),
		MUIA_Window_AppWindow, TRUE,
		WindowContents, MCP-MainWIN,
	End;

	ObjectApp->App = ApplicationObject,
		MUIA_Application_Author, "[r]FoX",
		MUIA_Application_Base, "SX-MCP",
		MUIA_Application_Title, "SX-MCP",
		MUIA_Application_Version, "$VER: SX-MCP v1.1a",
		MUIA_Application_Copyright, "Digital Corruption",
		MUIA_Application_Description, "SystemX Master Control Program",
		MUIA_Application_HelpFile, "SX-MCP.guide",
		SubWindow, ObjectApp->WI_label_0,
	End;


	if (!ObjectApp->App)
	{
		FreeVec(ObjectApp);
		return(NULL);
	}

	DoMethod(ObjectApp->WI_label_0,
		MUIM_Window_SetCycleChain, ObjectApp->BT_N,
		ObjectApp->BT_Name,
		ObjectApp->BT_Location,
		ObjectApp->BT_Action,
		ObjectApp->BT_I,
		0
		);

	set(ObjectApp->WI_label_0,
		MUIA_Window_Open, TRUE
		);


	return(ObjectApp);
}

void DisposeApp(struct ObjApp * ObjectApp)
{
	MUI_DisposeObject(ObjectApp->App);
	FreeVec(ObjectApp);
}
