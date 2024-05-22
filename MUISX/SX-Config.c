#include "sx-config.h"

#include "SXSTRUCTS.H"

#define ID_ABOUT  1
#define ID_NEWVOL 2
#define ID_NEWBRI 3
#define ID_MCFG   4
#define ID_EDMENU 5

/*
#define List(ftxt)               ListviewObject,\
                                 MUIA_Weight, 50,\
                                 MUIA_Listview_Input, FALSE,\
                                 MUIA_Listview_List, FloattextObject,\
                                 MUIA_Frame, MUIV_Frame_ReadList,\
                                 MUIA_Background, MUII_ReadListBack,\
                                 MUIA_Floattext_Text, ftxt,\
                                 MUIA_Floattext_TabSize, 4,\
                                 MUIA_Floattext_Justify, TRUE,\
                                 End,\
                                 End
*/                                 
#define DemoWindow(name,id,info) WindowObject, MUIA_Window_Title, name, MUIA_Window_ID, id, WindowContents, VGroup

#define Image(nr)                ImageObject, MUIA_Image_Spec, nr, End
#define ScaledImage(nr,s,x,y)    ImageObject, MUIA_Image_Spec, nr, MUIA_FixWidth, x, MUIA_FixHeight, y, MUIA_Image_FreeHoriz, TRUE, MUIA_Image_FreeVert, TRUE, MUIA_Image_State, s, End
#define HProp                    PropObject, PropFrame, MUIA_Prop_Horiz, TRUE, MUIA_FixHeight, 8, MUIA_Prop_Entries, 111, MUIA_Prop_Visible, 10, End
#define VProp                    PropObject, PropFrame, MUIA_Prop_Horiz, FALSE, MUIA_FixWidth , 8, MUIA_Prop_Entries, 111, MUIA_Prop_Visible, 10, End


//sx-config mainmenu
struct NewMenu Menu[] =
{
        
	{ NM_TITLE, "Project"    , 0 ,0,0,(APTR)0            },
	{ NM_ITEM , "About..."   ,"?",0,0,(APTR)ID_ABOUT     },
	{ NM_ITEM , NM_BARLABEL  , 0 ,0,0,(APTR)0            },
	{ NM_ITEM , "Quit"       ,"Q",0,0,(APTR)MUIV_Application_ReturnID_Quit },
	{ NM_TITLE, "Configs"    , 0 ,0,0,(APTR)0            },
        { NM_ITEM , "Main Config","M",0,0,(APTR)ID_MCFG      },
        { NM_END  , NULL         , 0 ,0,0,(APTR)0            },                   
};

//SX structs

struct MainStruct
{
	char BBSName[64];
	char BBSPath[64];
	char BBSLoc[64];
	char Sysop[64];
	char DNPath[64];
	char ULPath[64];
	long nodes;
	UWORD inac_timeout;
	UWORD inac_warn;
	ULONG minspace;
} Main;

struct NodeStruct
{
	char systempass[16];
	char newuserpass[16];
	char fcheck[42];
	long flags;
} nodeprefs;


//some globals
char buf[256],path[128];
char mBBSName[64],mSYSOP[64],mLOC[64],mDPATH[256],mUPATH[256],mPATH[64];
static ULONG mMINHD,mINACt,mINACw;
static long mNODES;
char MenuEntries[256][52];
int j,i;
LONG node=1;
BOOL MenuDone = FALSE;  
 
 
//Functions Prototypes here
void LoadMain(void);
void LoadCommands(void);


//                     
//Conferences Window
static APTR LV_CONFS_conf, LI_CONFS_conf;
static APTR BT_ADD_conf, BT_DEL_conf, BT_EDBASE_conf;
static APTR BT_SAVE_conf, BT_CANCEL_conf;
static APTR ST_CONFNAME_conf, ST_PATH_conf, ST_PASS_conf, ST_DEFPATH_conf;
static APTR NAME_conf, PATH_conf, PASS_conf, DEFPATH_conf, NUMAREA_conf, UPAREA_conf;
static APTR BGCHECK_conf, SENTBY_conf, FFCONF_conf, NOCREDZ_conf;
static const char *CYA_FLISTTYP_conf[] = { "AmiExpress" , NULL };
static APTR CY_FLISTTYP_conf;        
static APTR SL_NUMAREA_conf,SL_UPAREA_conf;
static APTR CM_BGCHECK_conf, CM_SENTBY_conf, CM_FFCONF_conf, CM_NOCREDZ_conf;


//Archive Types Window
static APTR LV_ARC_arc,LI_ARC_arc;
static APTR BT_ADD_arc, BT_DEL_arc, BT_CANCEL_arc, BT_SAVE_arc;
static APTR ST_ARCNAME_arc, ST_PAT_arc, ST_TCOM_arc, ST_UCOM_arc, ST_PCOM_arc, ST_VCOM_arc;
static APTR ST_COR1_arc, ST_COR2_arc, ST_COR3_arc;
static APTR ST_VAXX_arc;
static APTR NAME_arc, PAT_arc, TCOM_arc, UCOM_arc, PCOM_arc, VCOM_arc, COR1_arc, COR2_arc, COR3_arc,VAXX_arc,OTEST_arc,OPACK_arc; 
static APTR CM_OTEST,CM_OPACK;

//Transfer Protocols

static APTR LV_PROTO,LI_PROTO;
static APTR ST_NAME_pr, ST_LETTER_pr, ST_INIT_pr;
static APTR BT_DEL_pr, BT_ADD_pr, BT_Save_pr, BT_Cancel_pr;
static APTR NAME_pr, LETTER_pr, LIB_pr, INIT_pr, EFF_pr;
static APTR POP_LIB_pr;
static const char *CYA_PTYPE_pr[] = { "XPR Library","S-Modem External","Hydracom External",NULL };
static APTR CY_PTYPE;
static APTR SL_EFF;

//Serial Config window 
static APTR POP_SERDEV;
static APTR SERDEV_sm,SERUNIT_sm, MINBAUD_sm, DCE_sm, RTS7_sm, SHARED_sm, DSR_sm;
static APTR CONNECT_sm,RING_sm,ATA_sm,INIT_sm;
static APTR ST_SERUNIT, ST_MINBAUD,ST_DCE,ST_CONNECT,ST_RING,ST_ATA,ST_INIT;
static APTR CM_RTS7, CM_SHARED, CM_DSR;

//This Node window
static APTR CM_TELNET,CM_IDLE,CM_MULTI,CM_CLOSED;
static APTR SYSPW_tn,NUPE_tn,FCHECKDIR_tn,TELNET_tn,CLOSED_tn,MULTI_tn,IDLE_tn; 
static APTR ST_SYSPW_tn,ST_NUPE_tn,ST_FCHECKDIR_tn;
static APTR BT_Cancel_tn,BT_Save_tn;

//MainConfig window pointers
static APTR BBSobj,SYSOPobj,PATHobj,LOCobj,DLPATHobj,ULPATHobj,INAC_OUTobj,NODESobj;
static APTR INAC_WARNobj,MINHDobj;
static APTR BT_Main,BT_MenCom,BT_IntMap,BT_AxxSet,BT_NetSet,BT_LocDis,BT_DisMod,BT_Cron,BT_Confs;
static APTR BT_ArcTyp,BT_Protocol,BT_SerMod,BT_Doors,BT_This,BT_Quit;
static APTR BT_Save,BT_Cancel;
static APTR ST_BBS,ST_SYSOP,ST_PATH,ST_LOC,POP_UPATH,POP_DPATH;//,ST_INAC_OUT,ST_INAC_WARN;
static APTR SL_INAC_OUT,SL_INAC_WARN,SL_NODES,SL_MINHD;

//DoorsConfig window pointers
static const char *CYA_CHAT_door[] = { "/X XIM","/X AIM","Paragon","DayDream","DayDreamRexx","CLI/STDIO","FAME",NULL };
static const char *CYA_MSGED_door[] = { "/X XIM","/X AIM","Paragon","DayDream","DayDreamRexx","CLI/STDIO","FAME",NULL };
static const char *CYA_MSGRE_door[] = { "/X XIM","/X AIM","Paragon","DayDream","DayDreamRexx","CLI/STDIO","FAME",NULL };
static const char *CYA_FRONT_door[] = { "/X XIM","/X AIM","Paragon","DayDream","DayDreamRexx","CLI/STDIO","FAME",NULL };
static const char *CYA_NEWF_door[] = { "/X XIM","/X AIM","Paragon","DayDream","DayDreamRexx","CLI/STDIO","FAME",NULL };
static const char *CYA_NEWM_door[] = { "/X XIM","/X AIM","Paragon","DayDream","DayDreamRexx","CLI/STDIO","FAME",NULL };

static APTR ST_CHAT_door,ST_MSGED_door,ST_MSGRE_door,ST_FRONT_door,ST_NEWF_door,ST_NEWM_door;
static APTR CHAT_door,MSGED_door,MSGRE_door,FRONT_door,NEWF_door,NEWM_door;
static APTR CY_CHAT,CY_MSGED,CY_MSGRE,CY_FRONT,CY_NEWF,CY_NEWM;

//Select Menus to Edit Window
static APTR LV_MENUS,LI_MENUS;
static APTR BT_Edit,BT_Delete,BT_Add,BT_CancelCom;

//EditMenus Window
static APTR BT_ADD_mc,BT_DEL_mc,BT_CancelEDMen;
static APTR LV_COMMAND, LI_COMMAND;
static const char *CYA_MTYPE_mc[] = { "HotKey","Command",NULL };
static APTR CY_MTYPE;
static APTR POP_MANSI,POP_FUNCID;
static APTR MANSI_mc,MCHAR_mc,ACTSTR_mc,CREXIT_mc,NOEXP_mc,UCOMM_mc,COMMAND_mc,EXNUM_mc,LOAXX_mc,HIAXX_mc,DOORSTR_mc;
static APTR FUNCID_mc,MARGS_mc,EXARGS_mc;
static APTR ST_MCHAR,ST_ACTSTR,ST_COMMAND,ST_EXNUM,ST_DOORSTR,ST_MARGS,ST_EXARGS,ST_FUNCID;
static APTR CM_CREXIT,CM_NOEXP,CM_UCOMM;
static APTR SL_LOAXX,SL_HIAXX;

//SX-Config window pointers
APTR AP_Demo;
APTR WI_Master,WI_Main,WI_MenCom,WI_EditMenus,WI_Doors,WI_This,WI_Serial,WI_PCol, WI_Arc, WI_Confs;
static APTR SL_NODE;

//hook functions            
void SAVEDS ASM SaveMainFunc(void)
{
                
	BPTR fh = Open("SX:Prefs/Main.DAT", MODE_NEWFILE);
	if(fh)
        {                                                           
                sprintf(Main.BBSName,"%s" ,mBBSName);
		sprintf(Main.Sysop  ,"%s" ,mSYSOP);
                sprintf(Main.BBSLoc ,"%s" ,mLOC);
                sprintf(Main.BBSPath,"%s" ,mPATH);                
                mDPATH[64] = 0;
                sprintf(Main.DNPath ,"%s" ,mDPATH);                
                mUPATH[64] = 0;
                sprintf(Main.ULPath ,"%s" ,mUPATH);                
                Main.nodes = mNODES;
                Main.minspace = mMINHD*1000000;                
                Main.inac_timeout = mINACt*60;
                Main.inac_warn    = mINACw*60;
                                
                Write(fh, &Main, sizeof(struct MainStruct));
		Close(fh);
	}
        MUI_Request(AP_Demo, WI_Master, 0, NULL, "OK", "Config was successfull saved !");
	DoMethod(WI_Main,MUIM_Set,MUIA_Window_Open,FALSE);

}

void SAVEDS ASM LoadMenusFunc(void)
{
              
        BPTR fhm1;        
        
        if(MenuDone == TRUE) return;
        
        i=0;
                
        Execute("c:list SX:Commands/#?.mnu files lformat=%s >t:sxmenu.tmp",0,0);
               
        fhm1 = Open("t:sxmenu.tmp",MODE_OLDFILE);        
        while(FGets(fhm1, buf,52))
        {
                i++;                           
                strcpy(MenuEntries[i],buf);               
                DoMethod(LI_MENUS, MUIM_List_InsertSingle, MenuEntries[i] ,1,MUIV_List_Insert_Bottom);
                             
        }            
        Close(fhm1);  
        DoMethod(LI_MENUS, MUIM_List_Sort);
       
        MenuDone = TRUE;
                        
}

void SAVEDS ASM SetNodePathFunc(void)
{
        sprintf(path, "SX:Node%ld/Prefs/",node);
        LoadCommands();
}        

void SAVEDS ASM LoadNodeSetFunc(void)
{
        
        LoadCommands();
}


int main(int argc,char *argv[])
{
    static struct Hook SaveMainHook={ {NULL,NULL}, (void *)SaveMainFunc, NULL,NULL };
    static struct Hook LoadMenusHook={ {NULL,NULL}, (void *)LoadMenusFunc, NULL,NULL };
    static struct Hook SetNodePathHook={ {NULL,NULL}, (void *)SetNodePathFunc, NULL,NULL };
    static struct Hook LoadNodeSetHook={ {NULL,NULL}, (void *)LoadNodeSetFunc, NULL,NULL };
                
	
        init();         
        LoadMain();                
        sprintf(path, "SX:Node%ld/Prefs/", node);
        LoadCommands();
        
/* The complete                                      /|/| /|||\ | 
// Application Tree                                 /   |/-||| \| 
*/      
        //Main Config Window titles
        BBSobj = Label2("         _BBS Name");
      SYSOPobj = Label2("        Sysop Name");
        LOCobj = Label2("      BBS Location");
       PATHobj = Label2("        Users Path");  
     DLPATHobj = KeyLabel2("     Sysop DL Path",'d');
     ULPATHobj = KeyLabel2("     Sysop UL Path",'u');
   INAC_OUTobj = Label2("Inactivity Timeout");
  INAC_WARNobj = Label2("   Inactivity Warn");
      MINHDobj = Label2("   Min MB HD Space");
      NODESobj = Label2("     Nodes visible");
   
        //Doors Window titles    
       CHAT_door = Label2("     Chat Door");
      MSGED_door = Label2("Message Editor");
      MSGRE_door = Label2(" Msg RE Editor");
      FRONT_door = Label2("    Front Door");
       NEWF_door = Label2("New Files Door");
       NEWM_door = Label2("Mail Scan Door");

        //Edit Menus/Commands Window
        //MTYPE_mc = Label2("Menu Type");
        MANSI_mc = Label2("Menu Ansi");
        MCHAR_mc = Label2("Max Chars");
       ACTSTR_mc = Label2("Activity Str");
       CREXIT_mc = Label2("<CR> to Exit Menu");
        NOEXP_mc = Label2("Ignore Expert Mode");
        UCOMM_mc = Label2("Show 'Unknown Command'");
        
      COMMAND_mc = Label2("Command");
        EXNUM_mc = Label2("Extra Number");
        LOAXX_mc = Label2("Low Access");
        HIAXX_mc = Label2("High Access");
       FUNCID_mc = Label2("Function ID");
      DOORSTR_mc = Label2("Door/String/File");
        MARGS_mc = Label2("MainLine Arguments");
       EXARGS_mc = Label2("Execute Arguments");
               
        
        //Serial&Modem Window titles
       SERDEV_sm = KeyLabel2("Serial Device",'s');
      SERUNIT_sm = KeyLabel2("Unit",'u');
      MINBAUD_sm = KeyLabel2("Minimum Baud",'m');
          DCE_sm = KeyLabel2("DCE Rate",'r'); 
         RTS7_sm = Label2("RTS/CTS 7 WIRE");
       SHARED_sm = Label2("Shared Access");
          DSR_sm = Label2("No DSR Check");
      CONNECT_sm = Label2("Connect String");
         RING_sm = Label2("Ring String");
          ATA_sm = Label2("Answer String");
         INIT_sm = Label2("Init String");
        
       
        //This Node Window titles
        SYSPW_tn = Label2("System Password");
         NUPE_tn = Label2("New User Password");
    FCHECKDIR_tn = Label2("Filecheck Dir");
       TELNET_tn = Label2("Telnet Node");
       CLOSED_tn = Label2("Closed Start");
        MULTI_tn = Label2("Multi Logins");
         IDLE_tn = Label2("Inactivity Timeout");        
         
        //Transfer Protocols Window titles
         NAME_pr = Label2("Name");
       LETTER_pr = Label2("ID Letter");
          LIB_pr = Label2("Library");        
         INIT_pr = Label2("Init String");        
          EFF_pr = Label2("Efficiency");

        //Archive Types Window titles
        NAME_arc = Label2("Archive Name");
         PAT_arc = Label2("Pattern");
        TCOM_arc = Label2("Test Command");
        UCOM_arc = Label2("Unpack Command");
        PCOM_arc = Label2("Pack Command");
        VCOM_arc = Label2("View Command");
        COR1_arc = Label2("Corrup ID STR1");
        COR2_arc = Label2("Corrup ID STR2");
        COR3_arc = Label2("Corrup ID STR3");
        VAXX_arc = Label2("View Archive Access");
        OTEST_arc = Label2("Show Output of Test");
        OPACK_arc = Label2("Shou Output of Pack");
             
        //Conferences Window titles     
        NAME_conf = Label2("Name");
        PATH_conf = Label2("Path");
        PASS_conf = Label2("Conf Pass");
     DEFPATH_conf = Label2("Def Files Path");
     NUMAREA_conf = Label2("Number of Fileareas");
      UPAREA_conf = Label2("Current Upload Area");          
      BGCHECK_conf = Label2("Use Background Checker");
      SENTBY_conf = Label2("Allow Sent-By Lines");
      FFCONF_conf = Label2("Free Files Conference");
      NOCREDZ_conf = Label2("No Credits Conference");  
               
      
	 AP_Demo = ApplicationObject,
        
		MUIA_Application_Title         , "SX-Config",
		MUIA_Application_Version       , "$VER: SX-Config 1.07a (__DATE__)",
		MUIA_Application_Copyright     , "Copyright ©1998-2000 - Digital Corruption",
		MUIA_Application_Author        , "[r]FoX",
		MUIA_Application_Description   , "System-X [eSSeXX] Main Configuration.",
		MUIA_Application_Base          , "SX-CONFIG",
		MUIA_Application_Menustrip     , MUI_MakeObject(MUIO_MenustripNM,Menu,0),
   
                
                
                
                
                SubWindow,      //                            Conferences Window    
                    WI_Confs = DemoWindow("Conferences / File Areas / Message Bases",MAKE_ID('C','O','N','F'),NULL),
                        
                        Child, HGroup, //GroupFrameT(""),
                                
                                Child, VGroup, GroupFrameT("Conferences"),
                                    Child, LV_CONFS_conf = ListviewObject,
                                        MUIA_Weight, 10, // strlen(...
                                        MUIA_Listview_List, LI_CONFS_conf = ListObject,
                                        InputListFrame, MUIA_Background,
                                        End,
                                    End,                                                                               
                                    Child, HGroup, MUIA_Group_Columns,2,
                                        Child, BT_ADD_conf = SimpleButton("Add"),
                                        Child, BT_DEL_conf = SimpleButton("Del"),
                                        End, 
                                End,
                                
                                Child, ColGroup(2),
                                
                                    Child, NAME_conf,
                                    Child, ST_CONFNAME_conf = StringObject,
                                           MUIA_Frame, MUIV_Frame_Group,End,
                                           
                                    Child, PATH_conf,
                                    Child, ST_PATH_conf = StringObject,
                                           MUIA_Frame, MUIV_Frame_String, End,
                                    
                                    Child, PASS_conf,
                                    Child, ST_PASS_conf = StringObject,
                                           MUIA_Frame, MUIV_Frame_String,End,
                                           
                                    Child, DEFPATH_conf,
                                    Child, ST_DEFPATH_conf = StringObject,
                                           MUIA_Frame, MUIV_Frame_String,End,
                                                                        
                                    Child, KeyLabel1("Filelist Type",'t'),Child, CY_FLISTTYP_conf  = KeyCycle(CYA_FLISTTYP_conf,'t'),
                                                 
                                    Child, NUMAREA_conf,
                                    Child, SL_NUMAREA_conf = SliderObject, 
                                                  MUIA_Group_Horiz,TRUE,
                                                  MUIA_Slider_Min, 0,
                                                  MUIA_Slider_Max,255,
                                                  End,              
                                    Child, UPAREA_conf,
                                    Child, SL_UPAREA_conf = SliderObject, 
                                                  MUIA_Group_Horiz,TRUE,
                                                  MUIA_Slider_Min, 0,
                                                  MUIA_Slider_Max,255,
                                                  End, 
                                                  
                                    Child, BGCHECK_conf,
                                    Child, CM_BGCHECK_conf = CheckMark(FALSE),
                                    
                                    Child, SENTBY_conf,
                                    Child, CM_SENTBY_conf = CheckMark(FALSE),
                                    
                                    Child, FFCONF_conf,
                                    Child, CM_FFCONF_conf = CheckMark(FALSE),
                                    
                                    Child, NOCREDZ_conf,
                                    Child, CM_NOCREDZ_conf = CheckMark(FALSE),                                                                                                 
                                                                    
                                End,                           
                                                                                            
                        End, 
                        
                        Child, HGroup, MUIA_Group_Columns,3,
                                    Child, BT_SAVE_conf = SimpleButton("Save"),
                                    Child, BT_EDBASE_conf = SimpleButton("Edit Message Bases"),

                                    Child, BT_CANCEL_conf = SimpleButton("Cancel"),
                               End, 
                    
                    End,
                End,    
                
                SubWindow,    //                              Archive Types Window                  
                        WI_Arc = DemoWindow("SX-Config Archive Types",MAKE_ID('A','R','C','T'),NULL),
                        
                        Child, HGroup, GroupFrameT("Archive Types Setup"),
                                
                                Child, VGroup, //GroupFrameT("Arc Types"),
                                           Child, LV_ARC_arc = ListviewObject,
                                                MUIA_Weight, 10, // strlen(...
                                                MUIA_Listview_List, LI_ARC_arc = ListObject,
                                                InputListFrame, MUIA_Background,
                                                End,
                                           End,
                                
                                
                                                                                    
                                           Child, HGroup, MUIA_Group_Columns,2,
                                                Child, BT_ADD_arc = SimpleButton("Add"),
                                                Child, BT_DEL_arc = SimpleButton("Del"),
                                           End, 
                                End, 
                                               
                                Child, HGroup, //GroupFrameT("Arc Type Settings"),
                                 
                                        MUIA_Group_Columns, 4,
                                                                        
                                        Child, NAME_arc,
                                        Child, ST_ARCNAME_arc = StringObject,
                                               MUIA_Frame, MUIV_Frame_Text,End,
                                               
                                        Child, PAT_arc,
                                        Child, ST_PAT_arc = StringObject,
                                                MUIA_Frame, MUIV_Frame_String,End,
                                        
                                        Child, TCOM_arc,
                                        Child, ST_TCOM_arc = StringObject,
                                                MUIA_Frame, MUIV_Frame_String,End,

                                        Child, UCOM_arc,
                                        Child, ST_UCOM_arc = StringObject,
                                                MUIA_Frame, MUIV_Frame_String,End,

                                        Child, PCOM_arc,
                                        Child, ST_PCOM_arc = StringObject,
                                                MUIA_Frame, MUIV_Frame_String,End,

                                        Child, VCOM_arc,
                                        Child, ST_VCOM_arc = StringObject,
                                                MUIA_Frame, MUIV_Frame_String,End,

                                        Child, COR1_arc,
                                        Child, ST_COR1_arc = StringObject,
                                                MUIA_Frame, MUIV_Frame_String,End,

                                        Child, COR2_arc,
                                        Child, ST_COR2_arc = StringObject,
                                                MUIA_Frame, MUIV_Frame_String,End,

                                        Child, COR3_arc,
                                        Child, ST_COR3_arc = StringObject,
                                                MUIA_Frame, MUIV_Frame_String,End,
                                               
                                        Child, VAXX_arc,
                                        Child, ST_VAXX_arc = StringObject,
                                                MUIA_Frame, MUIV_Frame_String,End,

                                        Child, OTEST_arc,
                                        Child, CM_OTEST = CheckMark(FALSE),
                                        
                                        
                                        Child, OPACK_arc,
                                        Child, CM_OPACK = CheckMark(FALSE),

                                
                                End,                    
                            
                        End,
                        
                        
                        Child, HGroup,                   
                                Child, BT_SAVE_arc   = SimpleButton("_Save"),
                                Child, BT_CANCEL_arc = SimpleButton("_Cancel"),
                         
                                End, 
                        
                        
                        End,
                End,
                
                
                SubWindow,      //                   Transfer Protocols WINDOW
                        WI_PCol = DemoWindow("SX-Config Protocols",MAKE_ID('S','X','P','R'),NULL),                               
                             
                           Child, HGroup, GroupFrameT("Serial Transfer Protocols"),
                                        
                                        Child, VGroup, GroupFrameT("Protocols"),
                                           Child, LV_PROTO = ListviewObject,
                                                MUIA_Weight, 10, // strlen(...
                                                MUIA_Listview_List, LI_PROTO = ListObject,
                                                InputListFrame, MUIA_Background,
                                                End,
                                           End,
                                                                                    
                                           Child, HGroup, MUIA_Group_Columns,2,
                                                Child, BT_ADD_pr = SimpleButton("Add"),
                                                Child, BT_DEL_pr = SimpleButton("Del"),
                                           End,     
                                                       
                                        End,
                                       
                                
                                Child, VGroup,
                                  
                                        Child, HGroup, GroupFrameT("Protocol Setup"),
                                        MUIA_Group_Columns,2,
                                        
                                                Child, NAME_pr,
                                                Child, ST_NAME_pr = StringObject,
                                                MUIA_Frame, MUIV_Frame_Text,End,
                                        
                                                Child, LETTER_pr,
                                                Child, ST_LETTER_pr = StringObject,
                                                MUIA_Frame, MUIV_Frame_String,End,
                                        
                                                Child, LIB_pr,
                                                Child,POP_LIB_pr = PopaslObject,
                                                  MUIA_Popasl_Type, ASL_FileRequest, 
                                                  MUIA_Popstring_String, KeyString(0,0,0),//  Main.ULPath,256,'s'),
                                                  MUIA_Popstring_Button,PopButton(MUII_PopDrawer), 
                                                  ASLFR_TitleText,"Select Protocol Library...",
                                                  ASLFR_DrawersOnly, FALSE,
                                                  
                                                  
                                                  Child,EFF_pr,  
                                                  Child, SL_EFF = SliderObject, 
                                                  MUIA_Group_Horiz,TRUE,
                                                  MUIA_Slider_Min, 0,
                                                  MUIA_Slider_Max,100,
                                                  End,                                           
                                                End,
                                                
                                                Child, INIT_pr,
                                                Child, ST_INIT_pr = StringObject,
                                                MUIA_Frame, MUIV_Frame_String,End,
                                        
                                                                                                Child, KeyLabel1("Protocol Type",'t'),
                                                Child, CY_PTYPE  = KeyCycle(CYA_PTYPE_pr,'t'),
                                        End,        
                                End,                                                 
                           End,
                                    
                        
                        
                        Child, HGroup,                   
                                Child, BT_Save_pr   = SimpleButton("_Save"),
                                Child, BT_Cancel_pr = SimpleButton("_Cancel"),
                         
                                End,     
                        End,      

                End,
                
                SubWindow, //                              Serial & Modem WINDOW
			WI_Serial = DemoWindow("SX-Config Serial/Modem",MAKE_ID('S','E','R','M'),NULL), 
                           
                           Child, VGroup,     
                                Child, VGroup, GroupFrameT("Device"),
                                       
                                        Child, HGroup, MUIA_Group_Columns, 2,
                                        Child, SERDEV_sm,
                                        Child,POP_SERDEV = PopaslObject,
                                                  MUIA_Popasl_Type, ASL_FileRequest, 
                                                  MUIA_Popstring_String, KeyString(Main.ULPath,256,'s'),
                                                  MUIA_Popstring_Button,PopButton(MUII_PopDrawer), 
                                                  ASLFR_TitleText,"Select Node Serial.Device...",
                                                  ASLFR_DrawersOnly, FALSE,
                                                  End,
                                        End,                                             
			        
                                        Child, HGroup,MUIA_Group_Columns,6,   
                                                                      
                                        Child, SERUNIT_sm,
                                                Child, ST_SERUNIT = StringObject,
                                                MUIA_Frame, MUIV_Frame_String,End,
                                                                              
                                        Child, MINBAUD_sm,
                                                Child, ST_MINBAUD = StringObject,
                                                MUIA_Frame, MUIV_Frame_String,End,
                                        
                                        Child, DCE_sm,
                                                Child, ST_DCE = StringObject,
                                                MUIA_Frame, MUIV_Frame_String,End, 
                                                End,                                
                                        End,
                                End,
                                        
                                Child, ColGroup(2), GroupFrameT("Msg Strings"), 
                                                                                
                                        Child, CONNECT_sm,
                                                Child, ST_CONNECT = StringObject,
                                                MUIA_Frame, MUIV_Frame_String,
                                                End,
                                       
                                        Child, RING_sm,
                                                Child, ST_RING = StringObject,
                                                MUIA_Frame, MUIV_Frame_String,
                                                End,
                                        
                                        Child, ATA_sm,
                                                Child, ST_ATA = StringObject,
                                                MUIA_Frame, MUIV_Frame_String,
                                                End,
                                        
                                        Child, INIT_sm,
                                                Child, ST_INIT = StringObject,
                                                MUIA_Frame, MUIV_Frame_String,
                                                End,
                                End,  
                                
                                Child, ColGroup(8),GroupFrameT("Flags"),
                                                                            
                                     Child, RectangleObject, End,
                                        Child, RTS7_sm,
                                        Child, CM_RTS7 = CheckMark(FALSE),
                                       
                                        Child, SHARED_sm,
                                        Child, CM_SHARED = CheckMark(FALSE),
                                                                              
                                        Child, DSR_sm,
                                        Child, CM_DSR = CheckMark(FALSE),
                                     Child, RectangleObject, End,
                                End,                                                          
                                
                                Child, HGroup,                   
                                Child, BT_Save   = SimpleButton("_Save"),
                                Child, BT_Cancel = SimpleButton("_Cancel"),
			        End,                             
                           End,
                        End,


                SubWindow, //                                   This Node WiNDOW
                        WI_This = DemoWindow("SX-Config This Node",MAKE_ID('T','N','O','D'),NULL),
            
                          Child, VGroup,  
                                Child, ColGroup(2), GroupFrameT("Current Node"),
			                                                          
                                         Child, SYSPW_tn,
                                         Child, ST_SYSPW_tn   = StringObject,
                                         MUIA_Frame, MUIV_Frame_String, 
                                         MUIA_HelpNode, "SYSPW", End,
                                                                       
                                         Child, NUPE_tn,
                                         Child, ST_NUPE_tn   = StringObject,
                                         MUIA_Frame, MUIV_Frame_String, 
                                         MUIA_HelpNode, "NUPE",End, 
                                               
                                         Child, FCHECKDIR_tn,
                                         Child, ST_FCHECKDIR_tn   = StringObject,
                                         MUIA_Frame, MUIV_Frame_String, 
                                         MUIA_HelpNode, "FCHECK",End, 
                                         
                                End,                   
                                           
                                Child, ColGroup(6),GroupFrameT("Flags"),
                                  Child, RectangleObject, End, 
                                                                                                                                     
                                    Child, IDLE_tn, 
                                    Child, CM_IDLE   = CheckMark(FALSE),
                                                                                                         
                                    Child, CLOSED_tn,                      
                                    Child, CM_CLOSED = CheckMark(FALSE),
                                  Child, RectangleObject, End,                
                                  Child, RectangleObject, End,                                                           
                                    Child, MULTI_tn,
                                    Child, CM_MULTI  = CheckMark(FALSE),
                                                                                                            
                                    Child, TELNET_tn,
                                    Child, CM_TELNET = CheckMark(FALSE),
                                  
                                  Child, RectangleObject, End, 
                                End,                                        
                          End,
                          Child, HGroup,                                                   
                                Child, BT_Save_tn   = SimpleButton("_Save"),
                                Child, BT_Cancel_tn = SimpleButton("_Cancel"),
		    	        End,                                                                                                                           
                          End, 
                    
                        End,        
                        
                SubWindow, //                                   Doors config WINDOW
			WI_Doors = DemoWindow("SX-Config Doors",MAKE_ID('D','O','O','R'),NULL),
                                Child, ColGroup(4), GroupFrameT("Door Replacements"),
	                                                                                         
                                        Child,CHAT_door,
                                        Child, ST_CHAT_door = StringObject, MUIA_Frame, MUIV_Frame_String, End,
                                        Child, KeyLabel1("Door Type",'t'),Child, CY_CHAT  = KeyCycle(CYA_CHAT_door,'t'),
                                                                                                        
                                        Child,MSGED_door,
                                        Child, ST_MSGED_door = StringObject, MUIA_Frame, MUIV_Frame_String, End,
                                        Child, KeyLabel1("Door Type",'m'), Child, CY_MSGED  = KeyCycle(CYA_MSGED_door,'m'), 
                                                                     
                                        Child,MSGRE_door,
                                        Child, ST_MSGRE_door = StringObject, MUIA_Frame, MUIV_Frame_String, End,
                                        Child, KeyLabel1("Door Type",'r'), Child, CY_MSGRE  = KeyCycle(CYA_MSGRE_door,'r'),
                                                                          
                                        Child,FRONT_door,
                                        Child, ST_FRONT_door = StringObject, MUIA_Frame, MUIV_Frame_String, End,
                                        Child, KeyLabel1("Door Type",'f'), Child, CY_FRONT  = KeyCycle(CYA_FRONT_door,'f'),
                                                                             
                                        Child,NEWF_door,
                                        Child, ST_NEWF_door = StringObject, MUIA_Frame, MUIV_Frame_String, End,
                                        Child, KeyLabel1("Door Type",'n'), Child, CY_NEWF  = KeyCycle(CYA_NEWF_door,'n'),
                                                                             
                                        Child,NEWM_door,
                                        Child, ST_NEWM_door = StringObject, MUIA_Frame, MUIV_Frame_String, End,
                                        Child, KeyLabel1("Door Type",'s'),Child, CY_NEWM  = KeyCycle(CYA_NEWM_door,'s'),
                                                                          
                                End,
                                
                                Child, HGroup, 
                                        Child, BT_Save   = SimpleButton("_Save"),
                                        Child, BT_Cancel = SimpleButton("_Cancel"),
			  	        End,
                                                                                              
                                End,        
			End,
                
                      
           
		SubWindow,                                 // MENU / COMMAND WINDOW
			WI_MenCom = DemoWindow("SX-Config Edit Menus",MAKE_ID('C','O','M','A'),NULL),
				
                                Child, HGroup, GroupFrameT("Command Menus"),
				        
                                        Child, LV_MENUS = ListviewObject, 
                                                          MUIA_Listview_List, LI_MENUS = ListObject,               
                                                          InputListFrame, MUIA_Background,
                                                          MUII_ListBack,                                            
                                                          End,
                                               End, 
                                        End,                           
                                
                                        Child, HGroup, MUIA_Group_Columns,2, 
                                        
                                                Child, BT_Delete = SimpleButton("_Delete"),
                                                Child, BT_Edit   = SimpleButton("_Edit"),
                                        
                                                Child, BT_Add    = SimpleButton("_Add"),
                                                Child, BT_CancelCom = SimpleButton("_Cancel"),
			  	        End,
                                        
                                End,	        
			End,
                      
                SubWindow,                                //                          EditMenus WINDOW
                        WI_EditMenus = DemoWindow("SX-Config Menus/Commands",MAKE_ID('E','D','M','N'),NULL),      
                                
                                Child, HGroup, GroupFrameT("Menus & Commands"),
                                        
                                        Child, VGroup, GroupFrameT("Commands"),
                                           Child, LV_COMMAND = ListviewObject,
                                                MUIA_Listview_List, LI_COMMAND = ListObject,
                                                InputListFrame, MUIA_Background,
                                                End,
                                           End, 
                                           Child, HGroup, MUIA_Group_Columns,2,
                                                Child, BT_ADD_mc = SimpleButton("Add"),
                                                Child, BT_DEL_mc = SimpleButton("Del"),
                                           End,     
                                                       
                                        End,
                                    
                                    
                                    Child, VGroup,
                                        
                                        Child, VGroup, GroupFrameT("Menu Setup"), 
                                                                                    
                                            Child, HGroup,MUIA_Group_Columns,4,
                                                                                 
                                               Child, KeyLabel1("Menu Type",'m'),
                                               Child, CY_MTYPE  = KeyCycle(CYA_MTYPE_mc,'m'),
                                              
                                               Child, MANSI_mc,
                                               Child,POP_MANSI = PopaslObject, 
                                                  MUIA_Popstring_String,KeyString(0,0,0),//  KeyString(MenuHead.menuansi,32,'a'),
                                                  MUIA_Popstring_Button,PopButton(MUII_PopDrawer), 
                                                  ASLFR_TitleText,"Select the Menu Ansi...",
                                                  ASLFR_DrawersOnly, FALSE,
                                                  End,
                                                  
                                               Child, MCHAR_mc,
                                               Child, ST_MCHAR = StringObject, 
                                                  MUIA_Frame, MUIV_Frame_String, 
                                                  MUIA_HelpNode, "MCHAR",
                                                  End,      
                                                  
                                               Child, ACTSTR_mc,
                                               Child, ST_ACTSTR = StringObject, 
                                                  MUIA_Frame, MUIV_Frame_String, 
                                                  MUIA_HelpNode, "ACTSTR",
                                                  End,                                 
                                            End,
                                            
                                            Child, HGroup, MUIA_Group_Columns,6,
                                                                                                                    
                                                Child, RectangleObject, End,
                                                        Child, CREXIT_mc,
                                                        Child, CM_CREXIT = CheckMark(FALSE),
                                       
                                                        Child, NOEXP_mc,
                                                        Child, CM_NOEXP = CheckMark(FALSE),
                                                Child, RectangleObject, End,
                              
                                                Child, RectangleObject, End,
                                                        Child, UCOMM_mc,
                                                        Child, CM_UCOMM = CheckMark(FALSE),
                                                Child, RectangleObject, End,
                                            End,
                                        End, 
                                        
                                        Child, VGroup, GroupFrameT("Commands Setup"),
                                              
                                              Child, HGroup, MUIA_Group_Columns,4,
                                                
                                                Child, COMMAND_mc,
                                                Child, ST_COMMAND = StringObject, 
                                                  MUIA_Frame, MUIV_Frame_Text, 
                                                  MUIA_HelpNode, "COMMAND",
                                                  End, 
                                                
                                                Child, EXNUM_mc,
                                                Child, ST_EXNUM = StringObject, 
                                                  MUIA_Frame, MUIV_Frame_String, 
                                                  MUIA_HelpNode, "EXNUM",
                                                  End, 
                                        
                                                Child, LOAXX_mc,
                                                Child, SL_LOAXX = SliderObject, 
                                                  MUIA_Group_Horiz,TRUE,
                                                  MUIA_Slider_Min, 0,
                                                  MUIA_Slider_Max,255,
                                                  End,  
                                                
                                                Child, HIAXX_mc,
                                                Child, SL_HIAXX = SliderObject, 
                                                  MUIA_Group_Horiz,TRUE,
                                                  MUIA_Slider_Min, 0,
                                                  MUIA_Slider_Max,255,
                                                  End,  
                                              
                                              End,
                                              
                                              Child, HGroup, MUIA_Group_Columns,3,  
                                                Child, FUNCID_mc,
                                                Child,POP_FUNCID = PopaslObject, 
                                                  MUIA_Popstring_String,KeyString(0,0,0),//  KeyString(MenuHead.menuansi,32,'a'),
                                                  MUIA_Popstring_Button,PopButton(MUII_PopDrawer), 
                                                  ASLFR_TitleText,"Select the Menu Ansi...",
                                                  ASLFR_DrawersOnly, FALSE,
                                                  End,
                                                Child, ST_FUNCID = StringObject, 
                                                  MUIA_Frame, MUIV_Frame_String, 
                                                  MUIA_HelpNode, "MCHAR",
                                                  End, 
  
                                              End,
                                              
                                              Child, HGroup, MUIA_Group_Columns,2,
                                                
                                                Child, DOORSTR_mc,    
                                                Child, ST_DOORSTR = StringObject, 
                                                  MUIA_Frame, MUIV_Frame_String, 
                                                  MUIA_HelpNode, "DOORSTR",
                                                  End, 
                                                
                                                Child, MARGS_mc,
                                                Child, ST_MARGS = StringObject, 
                                                  MUIA_Frame, MUIV_Frame_String, 
                                                  MUIA_HelpNode, "MARGS",
                                                  End, 
                                        
                                                Child, EXARGS_mc,
                                                Child, ST_EXARGS = StringObject, 
                                                  MUIA_Frame, MUIV_Frame_String, 
                                                  MUIA_HelpNode, "EXARGS",
                                                  End, 
                                              End,                                  
                                        End,                                
                                    End,
                                End, 
                 
                                Child, HGroup,       
                                       Child, BT_Save   = SimpleButton("_Save"),
                                       Child, BT_CancelEDMen = SimpleButton("_Cancel"),
			  	       End,
                                End,
                        End,
                      
                      
                SubWindow,                                 // MAIN CONFIG WINDOW
		        WI_Main = DemoWindow("SX-Config Main",MAKE_ID('M','C','F','G'),FALSE),
			   
                           	Child, VGroup, GroupFrameT("Main Settings"),
                                MUIA_Group_Columns,2, 	                                                                                           
                                
                                Child, BBSobj,
                                Child, ST_BBS   = StringObject,
                                                  MUIA_Frame, MUIV_Frame_String, 
                                                  MUIA_HelpNode, "BBS", 
                                                  End,                                 
                                Child, SYSOPobj,
                                Child, ST_SYSOP = StringObject, 
                                                  MUIA_Frame, MUIV_Frame_String, 
                                                  MUIA_HelpNode, "SYSOP",
                                                  End,                                
                                Child, PATHobj,
                                Child, ST_PATH  = StringObject, 
                                                  MUIA_Frame,MUIV_Frame_String, 
                                                  MUIA_HelpNode, "PATH", 
                                                  End,                                 
                                Child, LOCobj,
                                Child, ST_LOC   = StringObject, 
                                                  MUIA_Frame,MUIV_Frame_String, 
                                                  MUIA_HelpNode, "LOC", 
                                                  End,                                
                                Child, DLPATHobj,                                
                                Child,POP_DPATH = PopaslObject, 
                                                  MUIA_Popstring_String,KeyString(Main.DNPath,64,'d'),
                                                  MUIA_Popstring_Button,PopButton(MUII_PopDrawer), 
                                                  ASLFR_TitleText,"Select your Download Dir...",
                                                  ASLFR_DrawersOnly, TRUE,
                                                  End,
                                                  
                                Child, ULPATHobj,
                                Child,POP_UPATH = PopaslObject, 
                                                  MUIA_Popstring_String, KeyString(Main.ULPath,64,'u'),
                                                  MUIA_Popstring_Button,PopButton(MUII_PopDrawer), 
                                                  ASLFR_TitleText,"Select your Upload Dir...",
                                                  ASLFR_DrawersOnly, TRUE,
                                                  End,
                                                                                                                 
                                Child, MINHDobj,
                                Child, SL_MINHD = SliderObject, 
                                                  MUIA_Group_Horiz,TRUE,
                                                  MUIA_Slider_Min, 1,
                                                  MUIA_Slider_Max,999,
                                                  End,                                                                            
                                Child, NODESobj,
                                Child, SL_NODES = SliderObject, 
                                                  MUIA_Group_Horiz,TRUE,
                                                  MUIA_Slider_Min, 1,
                                                  MUIA_Slider_Max,32,
                                                  End,
                                End,                                  
                                
                                Child, VGroup, GroupFrameT("Users Inactivity"),
                                MUIA_Group_Columns,2,
                                 
                                Child, INAC_OUTobj,
                                Child, SL_INAC_OUT = SliderObject,
                                                  MUIA_Group_Horiz,TRUE, 
                                                  MUIA_Slider_Min, 1, 
                                                  MUIA_Slider_Max,999, 
                                                  End, 
                                
                                Child, INAC_WARNobj,
                                Child, SL_INAC_WARN = SliderObject,
                                                  MUIA_Group_Horiz,TRUE,
                                                  MUIA_Slider_Min, 1,
                                                  MUIA_Slider_Max,999, 
                                                  End,                      
                                End, 
                                                                                
                                Child, HGroup, 
                                        Child, BT_Save   = SimpleButton("_Save"),
                                        Child, BT_Cancel = SimpleButton("_Cancel"),
			  	        End,
                                End,			
	 		End,

		
		SubWindow,                                 // SX-CONFIG WINDOW
			WI_Master = WindowObject,
			MUIA_Window_Title, "SX-Config",
			MUIA_Window_ID   , MAKE_ID('M','A','I','N'),
			WindowContents, VGroup,
				Child, TextObject, GroupFrame, MUIA_Background, MUII_FILL, MUIA_Text_Contents, "\033c\033bSX-Config\033n\n \33bM\33nain\33bC\33nonfiguration\33bI\33nnterface",  End,		
				Child, VGroup, GroupFrameT("Global Settings"),
					
			        Child, BT_Main      = SimpleButton("_Main"),
                    Child, BT_Confs     = SimpleButton("C_onferences"),
					Child, BT_MenCom    = SimpleButton("Menus/_Commands"),
					Child, BT_IntMap    = SimpleButton("_Internal Map"),
					Child, BT_AxxSet    = SimpleButton("_Access Sets"),
                    Child, BT_Cron      = SimpleButton("Cron _Events"),
					Child, BT_NetSet    = SimpleButton("Net_work Settings"),
					End,                                       
                                                
				Child, VGroup, GroupFrameT("Node Settings"),   

	                     	        Child, BT_LocDis    = SimpleButton("_Local Display"),
					Child, BT_DisMod    = SimpleButton("_Display Modes"),
					Child, BT_ArcTyp    = SimpleButton("_Archive Types"),
					Child, BT_Protocol  = SimpleButton("_Protocols"),
					Child, BT_SerMod    = SimpleButton("_Serial/Modem"),
					Child, BT_Doors     = SimpleButton("_Doors"),
					Child, BT_This      = SimpleButton("This _Node"),
				        Child, VGroup, GroupFrameT("Current Node"),
                                        Child, SL_NODE = SliderObject,
                                                MUIA_Group_Horiz,TRUE, 
                                                MUIA_Slider_Min, 1, 
                                                MUIA_Slider_Max,32, 
                                                MUIA_Slider_Level,1,
                                                End, 
                                        End,      
                                End,
                                                               
                                Child, VGroup,                                
                                        Child, BT_Quit  = SimpleButton("_Quit"),
                                        End,
                                End,        
                        End,
		End;
               
  
        if (!AP_Demo) fail(AP_Demo,"Failed to create application.");

//check all the buttons now
    
//SX-Config Mainwindow 
//----------------------------------------------------------------------sx-config 
      //Mainconfig Button
	    DoMethod(BT_Main     ,MUIM_Notify,MUIA_Pressed,FALSE,
        WI_Main     ,3,MUIM_Set,MUIA_Window_Open,TRUE);	
      //MenusCommands Button
        DoMethod(BT_MenCom   ,MUIM_Notify,MUIA_Pressed,TRUE,
                 WI_MenCom ,2, MUIM_CallHook, &LoadMenusHook);
        DoMethod(BT_MenCom   ,MUIM_Notify,MUIA_Pressed,FALSE,
                 WI_MenCom   ,3,MUIM_Set,MUIA_Window_Open,TRUE);
      //Conferences Button
        DoMethod(BT_Confs   ,MUIM_Notify,MUIA_Pressed,FALSE,
        WI_Confs   ,3,MUIM_Set,MUIA_Window_Open,TRUE);                               
      //Internal Map Button
        DoMethod(BT_IntMap   ,MUIM_Notify,MUIA_Pressed,FALSE,
        WI_EditMenus   ,3,MUIM_Set,MUIA_Window_Open,TRUE);
      //Archive Types Button
        DoMethod(BT_ArcTyp  ,MUIM_Notify,MUIA_Pressed,FALSE,
        WI_Arc   ,3,MUIM_Set,MUIA_Window_Open,TRUE);            
      //Protocols window Button
        DoMethod(BT_Protocol   ,MUIM_Notify,MUIA_Pressed,FALSE,
        WI_PCol   ,3,MUIM_Set,MUIA_Window_Open,TRUE);        
      //This Node window Button
        DoMethod(BT_This   ,MUIM_Notify,MUIA_Pressed,TRUE,
        WI_This   ,2,MUIM_CallHook, &LoadNodeSetHook);
        DoMethod(BT_This   ,MUIM_Notify,MUIA_Pressed,FALSE,
        WI_This   ,3,MUIM_Set,MUIA_Window_Open,TRUE);
      //Serial&Modem window Button
        DoMethod(BT_SerMod   ,MUIM_Notify,MUIA_Pressed,FALSE,
        WI_Serial   ,3,MUIM_Set,MUIA_Window_Open,TRUE);        
      //Doors window Button
        DoMethod(BT_Doors   ,MUIM_Notify,MUIA_Pressed,FALSE,
        WI_Doors   ,3,MUIM_Set,MUIA_Window_Open,TRUE);
      //Node Slider
        DoMethod(SL_NODE , MUIM_Notify, MUIA_Slider_Level, MUIV_EveryTime,
                 SL_NODE , 3, MUIM_WriteLong, MUIV_TriggerValue, &node);
        DoMethod(SL_NODE , MUIM_Notify, MUIA_Slider_Level ,MUIV_EveryTime,
                 SL_NODE , 2, MUIM_CallHook, &SetNodePathHook); 
      //Quit Button  
        DoMethod(BT_Quit     ,MUIM_Notify, MUIA_Pressed,FALSE,
        AP_Demo,2, MUIM_Application_ReturnID,MUIV_Application_ReturnID_Quit);


//Mainconfig Window 
//----------------------------------------------------------------------mainconfig
        //BBS Name String Gad       
   	    DoMethod(ST_BBS,MUIM_Notify, MUIA_String_Contents,MUIV_EveryTime,
                 ST_BBS,3,MUIM_WriteString,MUIV_TriggerValue,mBBSName);
        //Sysop Name Gadged         
        DoMethod(ST_SYSOP,MUIM_Notify, MUIA_String_Contents,MUIV_EveryTime,
                 ST_SYSOP,3,MUIM_WriteString,MUIV_TriggerValue,mSYSOP);
        //BBS Location Gadged
        DoMethod(ST_LOC,MUIM_Notify, MUIA_String_Contents,MUIV_EveryTime,
                 ST_LOC,3,MUIM_WriteString,MUIV_TriggerValue,mLOC);
        
        //Users Path
        DoMethod(ST_PATH,MUIM_Notify, MUIA_String_Contents,MUIV_EveryTime,
                 ST_PATH,3,MUIM_WriteString,MUIV_TriggerValue,mPATH);
        
        //No. of Nodes Slider
        DoMethod(SL_NODES,MUIM_Notify, MUIA_Slider_Level,MUIV_EveryTime, 
                 SL_NODES,3,MUIM_WriteLong,MUIV_TriggerValue, &mNODES);

        //Downloadpath ASL requester
        DoMethod(POP_DPATH,MUIM_Notify,MUIA_String_Acknowledge,MUIV_EveryTime,
                 MUIV_Notify_Self,3,MUIM_WriteString,MUIV_TriggerValue, mDPATH);

        //Uploadpath ASL requester
        DoMethod(POP_UPATH,MUIM_Notify,MUIA_String_Acknowledge,MUIV_EveryTime,
                 MUIV_Notify_Self,3,MUIM_WriteString,MUIV_TriggerValue, mUPATH);

        //Min HD Space Slider
        DoMethod(SL_MINHD,MUIM_Notify, MUIA_Slider_Level,MUIV_EveryTime, 
                 SL_MINHD,3,MUIM_WriteLong,MUIV_TriggerValue, &mMINHD);

        //Inactivity Timeout Slider
        DoMethod(SL_INAC_OUT,MUIM_Notify, MUIA_Slider_Level,MUIV_EveryTime, 
                 SL_INAC_OUT,3,MUIM_WriteLong,MUIV_TriggerValue, &mINACt);
        
        //Inactivity Warn Slider
        DoMethod(SL_INAC_WARN,MUIM_Notify, MUIA_Slider_Level,MUIV_EveryTime, 
                 SL_INAC_WARN,3,MUIM_WriteLong,MUIV_TriggerValue, &mINACw);
 
        //Cancel Button
	    DoMethod(BT_Cancel ,MUIM_Notify, MUIA_Pressed,FALSE,
                 WI_Main,2 ,MUIM_Set,MUIA_Window_Open,FALSE);
        //Save Button
        DoMethod(BT_Save   ,MUIM_Notify, MUIA_Pressed,FALSE,
                 BT_Save,2 ,MUIM_CallHook, &SaveMainHook);

//Menus Commands Window
//------------------------------------------------------------------Menus Commands 
         
   /*     //Delete Button
        DoMethod(BT_Delete   ,MUIM_Notify, MUIA_Pressed,FALSE,
                 BT_Delete,2 ,MUIM_CallHook, &DelMenComHook);                 
        //Edit Button
        DoMethod(BT_Edit   ,MUIM_Notify, MUIA_Pressed,FALSE,
                 BT_Edit,2 ,MUIM_CallHook, &EdMenComHook);        
        //Add Button
        DoMethod(BT_Add   ,MUIM_Notify, MUIA_Pressed,FALSE,
                 BT_Add,2 ,MUIM_CallHook, &AddMenComHook);     
   */     
        //Cancel Button
    	DoMethod(BT_CancelCom ,MUIM_Notify, MUIA_Pressed,FALSE,
                 WI_MenCom,2 ,MUIM_Set,MUIA_Window_Open,FALSE);
        //Select Menu from List LI_MENUS
        DoMethod(LV_MENUS, MUIM_Notify, MUIA_Listview_DoubleClick, TRUE,
                 AP_Demo, 2 , MUIM_Application_ReturnID, ID_EDMENU);  
    
//Edit Menus Window
//-------------------------------------------------------------------Edit Menus  
        //Cancel Button
	    DoMethod(BT_CancelEDMen ,MUIM_Notify, MUIA_Pressed,FALSE,
                 WI_EditMenus,2 ,MUIM_Set,MUIA_Window_Open,FALSE);
        //Save Button
        /*DoMethod(BT_Save   ,MUIM_Notify, MUIA_Pressed,FALSE,
                 BT_Save,2 ,MUIM_CallHook, &SaveMainHook);
        */

//Archive Types Window
//-------------------------------------------------------------------Archive Types  
        DoMethod(BT_CANCEL_arc ,MUIM_Notify, MUIA_Pressed,FALSE,
                 WI_Arc,2 ,MUIM_Set,MUIA_Window_Open,FALSE);

//ThisNode WINDOW
//-------------------------------------------------------------------This Node    
        DoMethod(BT_Cancel_tn ,MUIM_Notify, MUIA_Pressed,FALSE,
                 WI_This,2 ,MUIM_Set,MUIA_Window_Open,FALSE);
 
                   
// Set some start values for certain objects.
//-------------------------------------------
        //set the main-config startvalues here!
        set(ST_BBS   ,MUIA_String_Contents,Main.BBSName);
        set(ST_SYSOP ,MUIA_String_Contents,Main.Sysop);
        set(ST_PATH  ,MUIA_String_Contents,Main.BBSPath);
        set(ST_LOC   ,MUIA_String_Contents,Main.BBSLoc);
        set(SL_MINHD ,MUIA_Slider_Level, ( Main.minspace/1000000));        
        set(SL_INAC_OUT   ,MUIA_Slider_Level,(Main.inac_timeout/60));
        set(SL_INAC_WARN  ,MUIA_Slider_Level,(Main.inac_warn/60));
        set(SL_NODES      ,MUIA_Slider_Level,Main.nodes);
        
         
        //set ThisNode startvalues
         
//QUIT and REMOVE 
        // Automagically remove a window when the user hits the close gadget.
	    DoMethod(WI_Main     ,MUIM_Notify,MUIA_Window_CloseRequest,TRUE,WI_Main  ,3,MUIM_Set,MUIA_Window_Open,FALSE);
	    DoMethod(WI_Doors    ,MUIM_Notify,MUIA_Window_CloseRequest,TRUE,WI_Doors ,3,MUIM_Set,MUIA_Window_Open,FALSE);
        DoMethod(WI_This     ,MUIM_Notify,MUIA_Window_CloseRequest,TRUE,WI_This  ,3,MUIM_Set,MUIA_Window_Open,FALSE);
        DoMethod(WI_Serial   ,MUIM_Notify,MUIA_Window_CloseRequest,TRUE,WI_Serial,3,MUIM_Set,MUIA_Window_Open,FALSE);
        DoMethod(WI_MenCom   ,MUIM_Notify,MUIA_Window_CloseRequest,TRUE,WI_MenCom,3,MUIM_Set,MUIA_Window_Open,FALSE);
        DoMethod(WI_EditMenus,MUIM_Notify,MUIA_Window_CloseRequest,TRUE,WI_EditMenus,3,MUIM_Set,MUIA_Window_Open,FALSE);
        DoMethod(WI_PCol     ,MUIM_Notify,MUIA_Window_CloseRequest,TRUE,WI_PCol  ,3,MUIM_Set,MUIA_Window_Open,FALSE);
        DoMethod(WI_Arc      ,MUIM_Notify,MUIA_Window_CloseRequest,TRUE,WI_Arc   ,3,MUIM_Set,MUIA_Window_Open,FALSE);
        DoMethod(WI_Confs    ,MUIM_Notify,MUIA_Window_CloseRequest,TRUE,WI_Confs ,3,MUIM_Set,MUIA_Window_Open,FALSE);
        

        
        // Closing the master window forces a complete shutdown of the application.
	    DoMethod(WI_Master,MUIM_Notify,MUIA_Window_CloseRequest,TRUE,AP_Demo,2,
        MUIM_Application_ReturnID,MUIV_Application_ReturnID_Quit);
        
        //asl open ?...close
    
    
        // open the master window now.
       	set(WI_Master,MUIA_Window_Open,TRUE);
        
    //MainLoop
	{
		ULONG signal;
		BOOL running = TRUE;
		//char *buf;

		while (running)
		{
			switch (DoMethod(AP_Demo,MUIM_Application_Input,&signal))
			{
				case MUIV_Application_ReturnID_Quit:
					running = FALSE;
					break;

				case ID_NEWVOL:
					//DoMethod(LV_Volumes,MUIM_List_GetEntry,MUIV_List_GetEntry_Active,&buf);
					//set(LV_Directory,MUIA_Dirlist_Directory,buf);
					break;

				case ID_NEWBRI:
					//get(LV_Brian,MUIA_List_Active,&buf);
					//set(ST_Brian,MUIA_String_Contents,NULL);
					break;
                                        
                                case ID_EDMENU:
                                        DoMethod(WI_EditMenus, MUIM_Set, MUIA_Window_Open, TRUE);
                                        DoMethod(LV_MENUS, MUIM_List_GetEntry, MUIV_List_GetEntry_Active,&buf);
                                        
                                        //LoadCommands();
                                        
                                        
                                        break;
                                        
				case ID_ABOUT:
					MUI_Request(AP_Demo, WI_Master, 0, NULL, "OK", "SX-Config\n© 1999 by [r]FoX");
					break;
                               
                                case ID_MCFG:
                                        DoMethod(WI_Main ,MUIM_Set,MUIA_Window_Open,TRUE);
	                                break;
			}
			if (running && signal) Wait(signal);
		}
	}
        //Close and fuck off all
        
        Execute("c:delete >nil: t:sxmenu.tmp quit",0,0);   
        for(j=0; j<i; i--) { MenuEntries[i][0]=0; }
       
        fail(AP_Demo,NULL); //close all
        Execute("c:avail flush >nil:",0,0);

        
        
}

//load Main.CFG
void LoadMain(void)
{
	BPTR fh = Open("SX:Prefs/Main.DAT", MODE_OLDFILE);
	if(fh)
	{
		Main.minspace = 0;
		Read(fh, &Main, sizeof(struct MainStruct));
		Close(fh);
	}

}          
//load CommandMenus   
void LoadCommands(void)
{
    BPTR fhnod;

	sprintf(buf, "%sNodePrefs.DAT", path);
	fhnod = Open(buf, MODE_OLDFILE);
	
    if(fhnod)
	{
		Read(fhnod, &nodeprefs, sizeof(struct NodeStruct));
		Close(fhnod);
	} else 
    {
		nodeprefs.systempass[0]=0;
		nodeprefs.newuserpass[0]=0;
	}
    
    set(ST_SYSPW_tn , MUIA_String_Contents, nodeprefs.systempass);
    set(ST_NUPE_tn , MUIA_String_Contents, nodeprefs.newuserpass);
    set(ST_FCHECKDIR_tn , MUIA_String_Contents, nodeprefs.fcheck);
    //Telnet checkmark
    if(nodeprefs.flags&(1<<0))
    {
        nodeprefs.flags^=1<<0;     
        DoMethod(CM_TELNET, MUIA_Selected, selected);
    
    } else 
    {
        nodeprefs.flags|=1<<0;
        
        DoMethod(CM_TELNET, MUIA_Selected, FALSE);
                    
    }
}



