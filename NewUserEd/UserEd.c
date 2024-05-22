#include "demo.h"
#include <exec/memory.h>
#include <workbench/workbench.h>
#include <clib/icon_protos.h>
#include "SXStructs.h"


APTR app;

static APTR WI_Main;
static APTR TX_INFO;
static APTR LV_Users, LI_Users;

long usernr = 0;
char UserList[512][32];


void count_tools_list(void);
void check_entries(void);

void SAVEDS ASM LoadUsersFunc(void)
{
        struct UserData User;
                
        char tbuf[32];
       
        
        BPTR fhu = Open("SX:User.Data", MODE_OLDFILE);
        
        BOOL listfin = FALSE;
        
        
        if(fhu)
        {
                while(!listfin)
                {
                  if(!(FRead(fhu, &User, sizeof(struct UserData),1))) listfin = TRUE;                     
                  usernr++;                           
                  sprintf(tbuf, "%s", User.Name);                  
                  strcpy(UserList[usernr], tbuf);             
                  DoMethod(LV_Users, MUIM_List_InsertSingle, UserList[usernr], 1, MUIV_List_Insert_Bottom);                         
                  tbuf[0]=0;                                                  
                }
                DoMethod(LV_Users, MUIM_List_Sort);
                Close(fhu);
                                                     
        }  
        check_entries();
        count_tools_list();              
                        
}


/*** main ***/

int main(int argc, char *argv[])
{
     static struct Hook LoadUsersHook={ {NULL,NULL}, (void *)LoadUsersFunc, NULL,NULL };
     //struct UserData *User;

     int i;
           
	/*** init ***/
	BOOL running = TRUE;
     
     ULONG signal;
     
     
	init();

     //MUI_Request(0,0,0,0,"OK","Main joined !", TAG_END);
      
	/*** create mui-application ***/
	app = ApplicationObject,
		MUIA_Application_Title,				"SX-UserEd",
		MUIA_Application_Version,			"$VER: SX-UserEd 1.1 ("__DATE__")",
		MUIA_Application_Copyright,			"© 1999 by Digital Corruption",
		MUIA_Application_Author,			"[r]FoX",
		MUIA_Application_Description,		"System-X BBS User Editor",
		MUIA_Application_Base,				"SXUED",
		MUIA_Application_SingleTask,		TRUE,
		

		SubWindow, WI_Main = WindowObject,
         	  MUIA_Window_ID, MAKE_ID('M','A','I','N'),
     	  MUIA_Window_Title, "SX-UserEd",
	       WindowContents, 
               VGroup,
		  	
   	               Child, VGroup, GroupFrameT("Userlist"),

			          Child, TX_INFO = TextObject,
				          TextFrame,
				          MUIA_Background, MUII_TextBack,
				          End,

	     	          Child, VGroup,
					GroupSpacing(0),

					     Child, LV_Users = ListviewObject,
   					          MUIA_Listview_List, LI_Users = ListObject,
                                   MUIA_Listview_DoubleClick, TRUE,
                                   MUIA_Listview_MultiSelect, MUIV_Listview_MultiSelect_None,
                                   
                                   //MUIA_List_Format, "BAR,",
							     InputListFrame, 
							//MUIA_List_ConstructHook, &LoadUsersHook,
							//MUIA_List_DestructHook, &tools_list_deshook,
							//MUIA_List_CompareHook, &tools_list_cmphook,
							//MUIA_List_DisplayHook, &tools_list_dsphook,
							End,
						End,
                           
			          End,
                      
                    End,
		     End,
		End,
	End;


	/*** application failed ? ***/
	if (!app)	fail(app, "Creating application failed !");

	/*** connections & cycle ***/
	
 	//DoMethod(WI_Main, MUIM_Notify, MUIA_Window_CloseRequest, TRUE, WI_Main, 3, MUIM_Set, MUIA_Window_Open, FALSE);
	DoMethod(WI_Main,MUIM_Notify,MUIA_Window_CloseRequest,TRUE,app,2,
        MUIM_Application_ReturnID,MUIV_Application_ReturnID_Quit);
         
    	/*** open window ***/
	set(WI_Main, MUIA_Window_Open, TRUE);
	
     set(WI_Main, MUIA_Window_DefaultObject,	LV_Users);
     
     DoMethod(WI_Main ,MUIM_CallHook, &LoadUsersHook);
     
     //DoMethod(LI_Users, MUIM_List_Redraw, MUIV_List_Redraw_All);
               

	/*** set cursor on listview & check disable gadget ***/
	//check_entries();

	/*** main-loop ***/
     
	while(running)
     {
          switch (DoMethod(app,MUIM_Application_Input,&signal))
		{
			case MUIV_Application_ReturnID_Quit:
				running = FALSE;
                    for(i=0;i==usernr;usernr--) UserList[usernr][0]=0; // kill userentries     
				break;
          }
              
     }      
     fail(app, NULL);
}

/*** count active & inactive tools ***/

void count_tools_list(void)
{
	ULONG i;
	UWORD nr_active = 0, nr_inactive = 0, nr_changed = 0;
	char *line;

	/*** count ***/
	for (i=0; ; i++)
	{
		DoMethod(LV_Users, MUIM_List_GetEntry, i, &line);

		/*** last line ? ***/
		if (!line) break;

		/*** count tools ***/
		if (*(line+2) == '8')
			nr_active++;
		else
			nr_inactive++;

		if (*(line+4) == 'b')
			nr_changed++;
	}

	{
		char *s_inactive = "s";

		/*** check if 'tool' or 'tools' ***/
		if (nr_inactive == 1)
			s_inactive = "";

		/*** generate info text ***/
		DoMethod(TX_INFO, MUIM_SetAsString, MUIA_Text_Contents,
			    "-> %ld <- User%s", nr_inactive, s_inactive);
	}
}




/*** disable gadgets & menus, show changes ***/

void check_entries(void)
{
	ULONG i, sel, c_pos;
	UWORD nr_active = 0, nr_inactive = 0, nr_selected = 0;
	char *line;

	BOOL disable_activate = TRUE, disable_toggle = TRUE;
	BOOL disable_deactivate = TRUE, disable_restore = TRUE;
	BOOL disable_perform = TRUE, disable_edit = TRUE;

	BOOL refresh_display = FALSE;


	/*** cursor position ***/
	get(LV_Users, MUIA_List_Active, &c_pos);

	/*** count ***/
	for (i=0;; i++)
	{
		DoMethod(LV_Users, MUIM_List_GetEntry, i, &line);

		/*** last line ? ***/
		if (!line)
			break;

		/*** count selected or at cursor-position ***/
		DoMethod(LV_Users, MUIM_List_Select, i, MUIV_List_Select_Ask, &sel);
		if (sel || i==c_pos)
		{
			/*** count status of selected tools ***/
			if (*(line+2)=='8')
				nr_active++;
			else
				nr_inactive++;

			nr_selected++;
		}

		/*** check difference between stored and new status ***/
		if (*(line+2) != *line)
		{
			/*** make entry bold ***/
			if (*(line+4)!='b')
				refresh_display = TRUE;
			*(line+4)='b';

			disable_perform = FALSE;
		}
		else
		{
			/*** now a normal entry ***/
			if (*(line+4)!='n')
				refresh_display = TRUE;
			*(line+4)='n';
		}
	}

	if (nr_selected > 0)
	{
		disable_toggle = FALSE;
		disable_restore = FALSE;

		if (nr_inactive)
			disable_activate = FALSE;

		if (nr_active)
			disable_deactivate = FALSE;
	}

	if (nr_selected == 1)
		disable_edit = FALSE;

	/*** refresh display ***/
	//if (refresh_display)
	DoMethod(LV_Users, MUIM_List_Redraw, MUIV_List_Redraw_All);
}


        
