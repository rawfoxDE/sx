
#include <stdlib.h>
#include <string.h>
#include <exec/memory.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <devices/serial.h>
#include "SXStructs.h"
#include "sx.h"

extern struct StructsStruct	*Structs;
extern struct AmigaStruct	Amiga;
extern struct StringsStruct	*stg;
extern struct VariablesStruct	var;

extern UBYTE buf[512], charbuf[128];

struct ComStruct
{
	char command[16];
	char string[48];
	char exearg[20];
	char mainarg[20];
	UWORD function;
	UWORD low;
	UWORD high;
	UWORD misc;
	ULONG extra;
	char res[32];
};
struct ComStruct *Com;

struct MenuHeadStruct
{
	UBYTE type;
	UBYTE flags;
	UBYTE maxchars;
	UBYTE misc;
	char menuansi[32];
	char activity[32];
	char res[64];
};
struct MenuHeadStruct *MenuHead;

long MenuPo(char * name);
void ParseCommand(char * cbuf);
void GotoMenu(char * menuname, BOOL promptonly, BOOL noprompt);
void InternalCmd(char *com);
void DoFunction(int id, char * string, int extra, int low, int high, char * mainarg, char * exearg);

char prompt[256], lastmenu[32], thismenu[32];
long cmenu;


void ViewFile(void)
{
	int a;

	SetActive(10, 0);
	if(stg->arg[0]==0)
	{
		PS(Structs->SXStr[79]);
		LineInput(40,0,0);
	} else
		strcpy(charbuf, stg->arg);

	PS("[0m\r\n");
	if(!var.CARRIER || charbuf[0]==0) return;
	if(a = FindArcPo(charbuf))
	{
		if(Structs->Arc[a]->viewcmd[0]!=0)
		{
			if(FindFileOnHD(charbuf, 0))
			{
				strcat(stg->path, charbuf);
				mysprintf(buf, Structs->Arc[a]->viewcmd, stg->path);
				FifoExecute(buf, 0);
			} else
				PS(Structs->SXStr[81]);
		} else
			PS(Structs->SXStr[80]);
	} else
		PS(Structs->SXStr[80]);
}

void NewFileScan(void)
{
	if(Structs->Doors.nfdoor[0]!=0)
	{
		stg->arg[0] = 0;
		strcpy(stg->cmd, "CS");
		RunDoor(Structs->Doors.nfdoor, Structs->Doors.nftype, 0);
	}
}

void NewFileScanLogin(void)
{
	if(Structs->User.Flags&(1<<4))		/* ask */
	{
		PS(Structs->SXStr[124]);
		if(YesNo(1, TRUE) == 1) { PS("\r\n"); NewFileScan(); HitReturn(); }
	} else {
		if(Structs->User.Flags&(1<<3)) { NewFileScan(); HitReturn(); } /* yes */
	}
}

void MainMenu(void)
{
	BOOL checkcmd, UNKNOWN, menuchange;
	UBYTE lastkey;
	UWORD j;

	SetActive(0, 0);
	var.menufin=FALSE;
	if(var.EXPERT) PSLen("\r\n", 2);
	strcpy(lastmenu, "Main_Menu");
	strcpy(thismenu, lastmenu);

	GotoMenu(thismenu, FALSE, FALSE);

	while(!var.menufin && var.CARRIER)
	{
		if(!var.EXPERT || MenuHead->type==1 || (MenuHead->flags&(1<<1))) ShowAnsi(MenuHead->menuansi,1,1,1,1,1,0);

		if(MenuHead->type==1)
		{
hot:
			MyHotKey();
			if(!var.CARRIER) return;
			if(charbuf[0]==13 || charbuf[0]==10)
				charbuf[0] = 0;
			else {
				charbuf[1] = 13;
				charbuf[2] = 10;
				PSLen(charbuf, 3);
				charbuf[1] = 0;
			}
		} else {
			if(prompt[0]!=0) ShowPrompt(prompt, TRUE);//MCIPS(prompt, strlen(prompt), 0);
			LineInput(MenuHead->maxchars, 0, 0);
			if(!var.CARRIER) return;
			PSLen("\r\n",2);
		}
		lastkey=charbuf[0];
		menuchange=FALSE;
		if(charbuf[0]!='\0' && var.menusize!=0)
		{
			ParseCommand(charbuf);

			checkcmd=TRUE;
			Com = (struct ComStruct *) (cmenu + sizeof(struct MenuHeadStruct));
			j=0;
			UNKNOWN=TRUE;
			while(checkcmd)
			{
				j++;
				if(j > var.menusize)
					checkcmd = FALSE;
				else {
					if(stricmp(Com->command, stg->cmd)==0)
					{
						switch(Com->function)
						{
						case 2:
						case 3:
						case 53:
						case 79:
							checkcmd=FALSE;
							menuchange=TRUE;
						}
						DoFunction(Com->function, Com->string, Com->extra, Com->low, Com->high, Com->mainarg, Com->exearg);
						UNKNOWN=FALSE;
					}
					if(checkcmd) Com = (struct ComStruct *) ((LONG)Com + sizeof(struct ComStruct));
				}
			}
			SetActive(0, 0);
			var.CARRIER = Carrier();
			if(!var.CARRIER) return;
			if(!menuchange)
			{
				if(UNKNOWN && MenuHead->type==1)
				{
					PSLen("[A \b", 5);
					goto hot;
				}
				if((MenuHead->flags&(1<<0)) && UNKNOWN) PS(Structs->SXStr[8]);
				if(MenuHead->type==1 && var.CARRIER && !var.menufin) HitReturn();
			}
		} else {
			if(MenuHead->flags&(1<<2))
			{
				menuchange=TRUE;
				strcpy(charbuf,lastmenu);
				GotoMenu(charbuf, FALSE, FALSE);
			}
		}
		if(MenuHead->type==0 && !menuchange)
		{
			if(!var.EXPERT && !var.menufin && var.CARRIER)
				HitReturn();
			else {
				if(lastkey!=0) PSLen("\r\n",2);
			}
		}
		if(menuchange && var.EXPERT && MenuHead->type!=1) PSLen("\r\n",2);
	}
}

long MenuPo(char * name)
{
	Structs->McpMsg.data1=(long)name;
	Servermsg(6);
	var.menusize = Structs->McpMsg.data3;
	return(Structs->McpMsg.data2);
}

void ParseCommand(char * cbuf)
{
	UWORD j = 0, i, z = 0;
	BOOL flag = FALSE;

	stg->arg[0] = 0;
	stg->cmd[0] = 0;

	if(strchr(cbuf, ' ') == 0)
		strcpy(stg->cmd, cbuf);
	else {
		i = strlen(cbuf);
		while(j != i)
		{
			if(cbuf[j]==' ' && flag==FALSE)
			{
				z = 0;
				flag = TRUE;
			} else {
				if(flag)
				{
					stg->arg[z] = cbuf[j];
					z++;
					stg->arg[z] = 0;
				} else {
					if(cbuf[j]!=' ')
					{
						stg->cmd[z] = cbuf[j];
						z++;
						stg->cmd[z] = 0;
					}
				}
			}
			j++;
		}
	}
}

void GotoMenu(char * menuname, BOOL promptonly, BOOL noprompt)
{
	BPTR fh;
	long size, x;

	if(!menuname || menuname[0]==0) return;

	if(promptonly)
	{
		strcpy(menuname, thismenu);
		goto prompt;
	}

	strcpy(lastmenu,thismenu);
	strcpy(thismenu,menuname);
	cmenu = MenuPo(menuname);
	MenuHead = (struct MenuHeadStruct *) cmenu;

	if(MenuHead->maxchars > 119) MenuHead->maxchars = 119;

	var.menusize = (var.menusize - sizeof(struct MenuHeadStruct)) / sizeof(struct ComStruct);
	if(noprompt) return;
prompt:
	prompt[0]=0;
	if(MenuHead->type!=1)
	{
		mysprintf(buf,"SX:Commands/%s.prompt.%s", menuname, Structs->Disp.ext);
		fh=Open(buf, MODE_OLDFILE);
		if(fh)
		{
			Seek(fh,0,1);
			size=Seek(fh,0,-1);
			if(size>255) size = 255;
			x=Read(fh,&prompt,size);
			Close(fh);
			prompt[x]=0;
			if(Structs->Disp.flags&(1<<0)) StripAnsi(prompt, x);
		}
	}
}

void InternalCmd(char *com)
{
	struct ComStruct *temp;
	long oldsize = var.menusize;
	UWORD j = 0;
	char oldcmd[32];

	strcpy(oldcmd, stg->cmd);

	temp = (struct ComStruct *) ( MenuPo("InternalMap") + sizeof(struct MenuHeadStruct));

	//if(strchr(com, ' ')) 
	ParseCommand(com); 
	//else strcpy(stg->cmd, com);

	while(1)
	{
		j++;
		if(j > var.menusize)
			break;
		else {
			if(stricmp(temp->command, stg->cmd)==0)
			{
				DoFunction(temp->function, temp->string, temp->extra, temp->low, temp->high, temp->mainarg, temp->exearg);
				break;
			} else
				temp = (struct ComStruct *) ((LONG)temp + sizeof(struct ComStruct));
		}
	}
	var.menusize = oldsize;
	strcpy(stg->cmd, oldcmd);
}

void DoFunction(int id, char * string, int extra, int low, int high, char * mainarg, char * exearg)
{
	if(Structs->User.Sec_Status < low  ||  Structs->User.Sec_Status > high) return;

	if(mainarg && mainarg[0]!=0)
	{
		if(strcmp(mainarg, "%u")==0) strcpy(mainarg, "%ld");
		mysprintf(stg->arg, mainarg, var.thisfilearea);
	}

	switch(id)
	{
	case 7:
		var.RL=TRUE;
	case 1:
		var.menufin=TRUE;
		break;
	case 2:
		GotoMenu(string, FALSE, 0);
		break;
	case 3:
		GotoMenu("Main_Menu", FALSE, 0);
		break;
	case 4:
		if(var.EXPERT)
		{
			PS(Structs->SXStr[7]);/*PS("\r\nExpert Mode OFF.\r\n");*/
			var.EXPERT=FALSE;
		} else {
			PS(Structs->SXStr[6]);/*PS("\r\nExpert Mode ON.\r\n");*/
			var.EXPERT=TRUE;
		}
		break;
	case 5:
		PageSysOp();
		break;
	case 6:
		Ver();
		break;
	case 8:
		Who();
		break;
	case 9:
		FreeStrings();
		DisMode();
		GotoMenu(thismenu, TRUE, 0);
		break;
	case 10:
		PSFmt(Structs->SXStr[24], mytimestr(TRUE), mydatestr());
		break;
	case 11:
		ShowAnsi(string,1,1,1,1,extra,0);
		break;
	case 12:
		ShowAnsi(string,0,0,0,0,extra,0);
		break;
	case 13:
		NodeMsg(extra, string, 0);
		break;
	case 14:
		PS(Structs->SXStr[extra]);
		break;
	case 15:
		//if(extra) UserList(FALSE, TRUE); else UserList(FALSE, FALSE);
		switch(extra)
          {
                case 0:
                    UserList(FALSE,FALSE);
                    break;
                case 1:
                    UserList(FALSE,TRUE);
                    break;
                case 2:
                    UserList(TRUE,FALSE);
                    break;
                case 3:            
                    UserList(TRUE,TRUE);
                    break;                   
          }
          break;
	case 16:
		if(var.EXPERT && MenuHead->type!=1)
		{
			ShowAnsi(MenuHead->menuansi,1,1,1,1,1,0);
			PSLen("[A",3);
		}
		break;
	case 17:
		InternalCmd(string);
		break;
	case 18:
		MAXsShowAnsi(string);
		break;

	case 30:
		if(extra!=0)
			JoinConf(extra, FALSE);
		else
			JoinConf(atoi(stg->arg), FALSE);
		break;
	case 31:
		JoinConf(var.thisconf+1, FALSE);
		break;
	case 32:
		JoinConf(var.thisconf-1, FALSE);
		break;

	case 50:
		if(Structs->Conf[var.thisconf]->fileareas>=extra && extra>0) var.thisfilearea=extra;
		break;
	case 51:
		if(string[0]!=0) strcpy(stg->arg, string);
		ViewFile();
		break;
	case 52:
		AlterFlags();
		break;
	case 53:
		if(Structs->Conf[var.thisconf]->fileareas>=extra && extra>0) var.thisfilearea=extra;
		GotoMenu(string, FALSE, 0);
		break;

	case 70:
		EnterMsg(string, 0, var.thismsgarea, var.thisconf);
		break;
	case 71:
		EnterMsg(Structs->Cfg->Sysop, 0, var.thismsgarea, var.thisconf);
		break;
	case 72:
		ReadMsgs();
		break;
	case 73:
		if(!extra) GotoMsgArea(atol(stg->arg)); else GotoMsgArea(extra);
		break;
	case 74:
		GotoMsgArea(var.thismsgarea+1);
		break;
	case 75:
		GotoMsgArea(var.thismsgarea-1);
		break;
	case 76:
		MailScan();
		break;
	case 77:
		MailScanLogin();
		break;
	case 79:
		if(!extra) GotoMsgArea(atol(stg->arg)); else GotoMsgArea(extra);
		GotoMenu(string, FALSE, 0);
		break;

	case 92: User_Pass(0); break;
	case 93: User_Location(0); break;
	case 94: User_Phone(0); break;
	case 95: User_Screen(0); break;
	case 96: User_Computer(0); break;
	case 97: User_Protocol(); break;
	case 98: User_Signature(0); break;
	case 99: User_BGCheck(); break;
	case 100: User_FSED(); break;
	case 105: User_FileScan(); break;
	case 106: User_MailScan(); break;

	case 120: RunDoor(string, 1, exearg);  break;
	case 121: RunDoor(string, 2, exearg);  break;
	case 122: RunDoor(string, 3, exearg);  break;
	case 123: RunDoor(string, 4, exearg);  break;
	case 124: RunDoor(string, 5, exearg);  break;
	case 125: FifoExecute(string, exearg); break;
	case 126: RunDoor(string, 7, exearg);  break;

	case 140: DownloadFile(); break;
	case 141: UploadFile(TRUE); break;
	case 142: SendFile(string); break;
	case 143: RawReceive(string, extra); break;
	case 144: UploadFile(FALSE); break;

	case 160: EditUser(TRUE); break;
	case 161: FifoExecute(0,0); break;
	case 163: Execute(string,0,0); break;
	case 165: SysopDownload(); break;
	case 166: TestDoor(); break;
	case 167: LineED(string); break;
	case 168: if(Structs->Doors.msgdoor[0]!=0)
		{
			strcpy(charbuf, string);
			RunDoor(Structs->Doors.msgdoor, Structs->Doors.msgtype, 0);
		}
		break;

	case 180: LoadFlags(); break;
	case 181: CheckLCFiles(); break;

	case 200: NewFileScanLogin(); break;
	}
}
