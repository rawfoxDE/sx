
#include "sx-config.h"


#define MyWindow(name,id,info) WindowObject, MUIA_Window_Title, name, MUIA_Window_ID, id, WindowContents, VGroup



static APTR WI_MAIN;

/* A few test items */
STRPTR TestEntries[] =
{
	"Smoke City",
	"Tori Amos",
	"Fluke",
	"Garbage",
	"Depeche Mode",
	"Madonna",
	"Eclipse",
	"TIP",
	"Hole",
	"Nine Inch Nails",
	"Lamb",
	"Moloco",
	"Sorten Muld",
	"L7",
	"Morcheeba",
	"Portishead",
	NULL
};

VOID main (void)
{
                
	ULONG sigs;
        Object *app, *listview, *list;

        init();

	app = ApplicationObject,
		MUIA_Application_Author,		"RawFox",
		MUIA_Application_Base,			"ListInputTest",
		MUIA_Application_Copyright,	"®1999 Digital Corruption",
		
                SubWindow,
                        WI_MAIN = MyWindow("Listview Testwindow",MAKE_ID('M','A','Y','W'),NULL),
			
                                Child, HGroup,
				    Child, listview = ListviewObject,
					MUIA_Listview_List, list = ListObject,
						InputListFrame,
						MUIA_Background, MUII_ListBack,
						/* Initial contents...*/
						MUIA_List_SourceArray, TestEntries,
						End,
					End,
                                    End,
				End,
			End,
		End;

	if(app)
	{
		DoMethod(WI_MAIN, MUIM_Notify, MUIA_Window_CloseRequest, TRUE, MUIV_Notify_Application, 2, MUIM_Application_ReturnID, MUIV_Application_ReturnID_Quit);
		DoMethod(list, MUIM_List_Sort);
		SetAttrs(WI_MAIN,
			MUIA_Window_DefaultObject, listview,
			MUIA_Window_Open, TRUE,
			TAG_DONE);

		/* Add an extra item... */
		DoMethod(list, MUIM_List_InsertSingle, "Sneaker Pimps", MUIV_List_Insert_Sorted);

		sigs = 0;
		while(DoMethod(app, MUIM_Application_NewInput, &sigs) != MUIV_Application_ReturnID_Quit)
		{
			if(sigs)
			{
				if((sigs = Wait(sigs | SIGBREAKF_CTRL_C)) & SIGBREAKF_CTRL_C)
					break;
			}
		}
		MUI_DisposeObject(app);
	}
}
