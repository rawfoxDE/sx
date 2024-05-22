/*

Primitive/Primary Routines

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <exec/memory.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/graphics.h>
#include <proto/intuition.h>
#include <intuition/intuition.h>
#include <dos/dos.h>
#include <devices/serial.h>
#include "SXStructs.h"
#include "sx.h"

extern struct StructsStruct	*Structs;
extern struct AmigaStruct	Amiga;
extern struct StringsStruct	*stg;
extern struct VariablesStruct	var;

extern UBYTE buf[512], charbuf[128];
extern long comdex;
extern char *history[30];

void LineInput(int limit,BOOL pass,char * def);
void HitReturn(void);
void MyHotKey (void);
void RawHotKey(void);
void CheckKey (void);
void PSLen(char * b, long l);
void PS(char * b);
void SerWrite (char * b);
void ConWrite (char * b, ULONG l);
void Local (char * b);
int YesNo(int def, BOOL show);
BOOL Carrier(void);
ULONG SXWait(void);
void SafePS(char * s);
void addcom(char *com);


void mysprintf(char *Buffer,char *ctl, ...)
{
	RawDoFmt(ctl, (long *)(&ctl + 1), (void (*))"\x16\xc0\x4e\x75", Buffer);
}

void PSFmt(char *ctl, ...)
{
	RawDoFmt(ctl, (long *)(&ctl + 1), (void (*))"\x16\xc0\x4e\x75", buf);
	PSLen(buf, strlen(buf));
}

void TextFmt(struct RastPort *rP, char *ctl, ...)
{
	RawDoFmt(ctl, (long *)(&ctl + 1), (void (*))"\x16\xc0\x4e\x75", buf);
	Text(rP, buf, strlen(buf));
}

void ShowPrompt(char *str, BOOL mci)
{
	strcpy(stg->promptstr, str);
	if(mci)
		MCIPS(stg->promptstr, strlen(stg->promptstr), 0, 0);
	else
		PSLen(stg->promptstr, strlen(stg->promptstr));
}

void LineInput(int limit, BOOL pass, char *def)
{
     BOOL lineinput = TRUE;
	  char enter[128];
     UWORD pos;          
     char alfa[2]; // check alphanumeric            
   
    	if(limit > 127) limit = 127;  // eingabe zeichen eingrenzen

	if(def && def[0])             // wenns was gibt, dann copy nach `enter`
	{
		strcpy(enter, def);
		pos = strlen(enter);     // `pos` = laenge von `enter`
		PSLen(enter, pos);
	} else pos = 0;

	while(lineinput)
	{
		MyHotKey();
		if(!var.CARRIER) return;

		switch(charbuf[0])
		{
		case 0:
		case 1: 
		case 2:  // left  
         	case 3:  // right 
               break;
          case 4:  // up    
               comdex--;                       
               if(comdex < 1) comdex = 29;
               //sprintf(alfa,"%ld", isalnum(history[comdex][0]));
               if(history[comdex][0] != 0);  // && alfa[0] == '0')
               {    
                    if(pos!=0) PSFmt("[%ldD[K", pos);                     
                    strcpy(enter, history[comdex]); 
                    pos = strlen(enter); 
                    PS(enter);              
               } 
               break;                            
          case 5:  // down  
               comdex++;                                 
               if(comdex > 29) comdex = 1;                         
               //sprintf(alfa,"%ld", isalnum(history[comdex][0]));
               if(history[comdex][0] != 0);  // && alfa[0] == '0')
               {
                    if(pos!=0) PSFmt("[%ldD[K", pos);  
                    strcpy(enter, history[comdex]);
                    pos = strlen(enter);
                    PS(enter);
               }
               break;               
          case 6:
		case 7:
		case 11:
		case 12:
		case 14:
		case 15:
		case 16:
		case 17:
		case 18:
		case 19:
		case 20:
		case 21:
		case 22:
		case 23:
		case 25:
		case 26:
		case 27:
		case 28:
		case 29:
		//case 30:
		//case 31:
               break;
          case 8:            // BACKSPACE
		case 127:          // DEL
			if(pos > 0)
			{
				pos--;
				PSLen("\b \b", 3);
			}
			break;
		case 9:            // TAB
			if(pos%5 == 0)
			{
				enter[pos] = ' ';
				pos++;
				PSLen(" ", 1);
			}
			if(pos < (limit-5))
			{
				while(pos%5)
				{
					enter[pos] = ' ';
					pos++;
					PSLen(" ", 1);
				}
			}
			break;
		case 10:
		case 13:           // RETURN
			enter[pos] = 0;
               lineinput = FALSE;
               if(pos) addcom(enter);                          
               break;
		case 24:           // CANCEL, CTRL-X
			if(pos) 
			{
				PSFmt("[%ldD[K", pos);
				pos = 0;
			}
			break;
          default:
			if(pos != limit)
			{
				enter[pos] = charbuf[0];
				pos++;
				if(pass)
					PSLen(Structs->SXStr[17], 1);
				else
					PSLen(charbuf, 1);
			}
		}
	} 
     strcpy(charbuf, enter);
	stg->promptstr[0] = 0;
               
}

void HitReturn(void)
{
	if(Structs->SXStr[5]) ShowPrompt(Structs->SXStr[5], FALSE);
	MyHotKey();
	stg->promptstr[0] = 0; 
     PS("[A\r\n[K");
}

void MyHotKey(void)
{
	UBYTE t;
start:
	RawHotKey();
	//if(!var.RAWARROW && charbuf[0]==27)
	if(charbuf[0]==27)

     {
		t = charbuf[0];
		RawHotKey();
		if(var.TIMEOUTFLAG)
		{
			charbuf[0] = t;
			var.TIMEOUTFLAG = FALSE;
			return;
		}
		if(charbuf[0]=='[')    
		{
			RawHotKey();
			switch(charbuf[0])
			{
			case 'A': charbuf[0]=4; break;
               case 'B': charbuf[0]=5; break;
               case 'D': charbuf[0]=2; break;	// LEFT/RIGHT BACKWARDS!!
			case 'C': charbuf[0]=3; break;
			
               }
		}
	}

	if(charbuf[0]==155 && var.origin==1)
	{
		RawHotKey();
		switch(charbuf[0])
		{
		case 'A': charbuf[0]=4; break;
       	case 'B': charbuf[0]=5; break;
       	case 'D': charbuf[0]=2; break;
          case 'C': charbuf[0]=3; break;	// LEFT/RIGHT BACKWARDS!!
          
		default:
			t = charbuf[0];
			RawHotKey();
			if(var.CALLER) CheckOnlineKey(t); else CheckLocalKey(t+32);
			goto start;
		}
	}
}

void RawHotKey(void)
{
	ULONG signals;
	struct Message *msg;

	var.origin = 0;
	charbuf[0] = 0;
	charbuf[1] = 0;

hotwait:
	signals = SXWait();

	if((signals&var.sersig) && !var.LOCAL && var.SER)
	{
		if (msg = GetMsg(Amiga.ReadSerPort))
		{
			if(msg == (struct Message *)&Amiga.serreadreq)
			{
				if(Amiga.serreadreq.IOSer.io_Actual > 0)
				{
					charbuf[0] = stg->serbuf[0];
					var.origin = 2;
					var.bytesrec++;
					var.TIMEOUTFLAG = FALSE;
				}
			}
			Amiga.serreadreq.IOSer.io_Length = 1;
			SendIO((struct IORequest *)&Amiga.serreadreq);
		}
	}
	var.CARRIER = Carrier();
	if(charbuf[0]) return;

	if(var.CON) //&& (signals&var.consig))
	{
		if (msg = GetMsg(Amiga.ReadConPort))
		{
			if(msg == (struct Message *)&Amiga.conreadreq)
			{
				if(Amiga.conreadreq.io_Actual > 0)
				{
					charbuf[0] = stg->conbuf[0];
					var.origin = 1;
					var.TIMEOUTFLAG = FALSE;
				}
			}
			Amiga.conreadreq.io_Command = CMD_READ;
			SendIO((struct IORequest *)&Amiga.conreadreq);
		}
	}
	if(charbuf[0]==0 && !var.TIMEOUTFLAG && var.CARRIER) goto hotwait;
}

void CheckKey (void)
{
	struct Message *msg;

	var.origin = 0;
	charbuf[0] = 0;
	charbuf[1] = 0;

	if(!var.LOCAL && var.SER && var.CALLER)
	{
		if (msg = GetMsg(Amiga.ReadSerPort))
		{
			if(msg == (struct Message *)&Amiga.serreadreq)
			{
				if(Amiga.serreadreq.IOSer.io_Actual > 0)
				{
					charbuf[0] = stg->serbuf[0];
					var.origin = 2;
					var.bytesrec++;
				}
			}
			Amiga.serreadreq.IOSer.io_Length = 1;
			SendIO((struct IORequest *)&Amiga.serreadreq);
		}
	}
	var.CARRIER = Carrier();
	if(charbuf[0]) return;

	if(var.CON)
	{
		if (msg = GetMsg(Amiga.ReadConPort))
		{
			if(msg == (struct Message *)&Amiga.conreadreq)
			{
				if(Amiga.conreadreq.io_Actual > 0)
				{
					charbuf[0] = stg->conbuf[0];
					var.origin = 1;
				}
			}
			Amiga.conreadreq.io_Command = CMD_READ;
			SendIO((struct IORequest *)&Amiga.conreadreq);
		}
	}
	//var.CARRIER = Carrier();
}

void PSLen(char * b, long l)
{
	if(var.CALLER && var.SER && !var.LOCAL)
	{
		Amiga.serwritereq.IOSer.io_Data = b;
		Amiga.serwritereq.IOSer.io_Length = l;
		SendIO((struct IORequest *)&Amiga.serwritereq);
		var.bytessent += l;
	}

	if(var.CON)
	{
		Amiga.conwritereq.io_Data = b;
		Amiga.conwritereq.io_Length = l;
		DoIO((struct IORequest *)&Amiga.conwritereq);
	}

	if(Amiga.capfh) Write(Amiga.capfh, b, l);

	if(var.CALLER && var.SER && !var.LOCAL) WaitIO(&Amiga.serwritereq);
}

void PS(char * b)
{
	PSLen(b, strlen(b));
}

void SerWrite(char * b)
{
	if(var.SER && !var.LOCAL)
	{
		register long len = strlen(b);
		Amiga.serwritereq.IOSer.io_Data = b;
		Amiga.serwritereq.IOSer.io_Length = len;
		DoIO((struct IORequest *)&Amiga.serwritereq);
		var.bytessent += len;
	}
}

void ConWrite(char * b, ULONG l)
{
	Amiga.conwritereq.io_Data = b;
	Amiga.conwritereq.io_Length = l;
	DoIO((struct IORequest *)&Amiga.conwritereq);
}

void Local(char * b)
{
	if(var.CON) ConWrite(b, strlen(b));
}

int YesNo(int def, BOOL show)
{
	/* defaults are
		0 = none
		1 = yes
		2 = no	*/
	while(1)
	{
		MyHotKey();
		if(!var.CARRIER) return(0);
		if(charbuf[0]==13 || charbuf[0]==10)
		{
			if(def==1) charbuf[0]='Y';
			if(def==2) charbuf[0]='N';
		}
		switch(charbuf[0])
		{
		case 'y':
		case 'Y':
			if(show && Structs->SXStr[9]) PS(Structs->SXStr[9]);
			return(1);
		case 'n':
		case 'N':
			if(show && Structs->SXStr[10]) PS(Structs->SXStr[10]);
			return(0);
		}
	}
}

BOOL Carrier(void)	/* check for a carrier */
{
	UWORD status;
	if(var.LOCAL)
		return(var.CARRIER);
	else {
		if(var.SER)
		{
			Amiga.serwritereq.IOSer.io_Command	= SDCMD_QUERY;
			DoIO((struct IORequest *)&Amiga.serwritereq);
			status = Amiga.serwritereq.io_Status;
			Amiga.serwritereq.IOSer.io_Command 	= CMD_WRITE;
			if(status&(1<<5)) return(FALSE); else return(TRUE);
		} else
			return(TRUE);
	}
}

ULONG SXWait(void)
{
	ULONG sign, nodesig = 1L<<Amiga.nodemp->mp_SigBit;
	long idle, lastmove;

	if(var.SER) var.sersig = 1L<<Amiga.ReadSerPort->mp_SigBit;
	var.timsig = 1L<<Amiga.TimerPort->mp_SigBit;

	lastmove = mytime();
rewait:
	var.TIMEOUTFLAG=FALSE;
	if(var.TIME_OUT!=0) SetTimer(var.TIME_OUT, 0); else SetTimer(2, 0);

	if(var.CON)
	{
		var.infowinsig=1L<<Amiga.infowin->UserPort->mp_SigBit;
		var.cwinsig=1L<<Amiga.cwin->UserPort->mp_SigBit;
		var.winsig=1L<<Amiga.win->UserPort->mp_SigBit;
		var.consig=1L<<Amiga.ReadConPort->mp_SigBit;

		if(var.SER)
			sign = Wait( var.timsig | var.sersig | nodesig | var.cwinsig | var.infowinsig | var.winsig | var.consig );
		else
			sign = Wait( var.timsig | nodesig | var.cwinsig | var.infowinsig | var.winsig | var.consig );

		if(sign & var.cwinsig) CheckWin(Amiga.cwin);
		if(var.CON) { if(sign & var.winsig) CheckWin(Amiga.win); }
		if(var.CON) { if(sign & var.infowinsig) CheckWin(Amiga.infowin); }
	} else {
		if(var.SER)
			sign = Wait( var.timsig | var.sersig | nodesig | SIGBREAKF_CTRL_F );
		else
			sign = Wait( var.timsig | nodesig | SIGBREAKF_CTRL_F );

		if(sign & SIGBREAKF_CTRL_F) OpenDisplay(TRUE);
	}

	if(sign & var.timsig)
	{
		if(!var.disabletimeout && var.CALLER)
		{
			idle = mytime() - lastmove;
			if(idle>Structs->Cfg->inac_timeout && Structs->Cfg->inac_timeout>0)
			{
				PS(Structs->SXStr[118]);
				DropDTR(TRUE);
				return(sign);
			} else {
				if(idle>Structs->Cfg->inac_warn && Structs->Cfg->inac_warn>0) PS(Structs->SXStr[119]);
			}
		}
		//if(var.TIME_OUT==0)
		//{
			if(var.CON)
			{
				Move(&Amiga.scr->RastPort, 497, 7);
				Text(&Amiga.scr->RastPort, mytimestr(FALSE), 5);
			}
			if(var.USER)
			{
				CalcTimeLeft();
				UpdateTimeLeft();
			}
		//} else
		if(var.TIME_OUT!=0) var.TIMEOUTFLAG = TRUE;
	} else {
		AbortIO(Amiga.TimerMsg);
		WaitIO(Amiga.TimerMsg);
	}

	if(sign & nodesig) HandleNodeMsg();

	if(var.SER)
	{
		if(var.CON)
		{
			if(!(sign&var.sersig) && !CheckIO(&Amiga.conreadreq) && !var.done && !var.TIMEOUTFLAG && Carrier()) goto rewait;
		} else {
			if(!(sign&var.sersig) && !var.done && !var.TIMEOUTFLAG && Carrier()) goto rewait;
		}
	} else {
		if(var.CON)
		{
			if(!CheckIO(&Amiga.conreadreq) && !var.done && !var.TIMEOUTFLAG && Carrier()) goto rewait;
		} else {
			if(!var.done && !var.TIMEOUTFLAG && Carrier()) goto rewait;
		}
	}

	return(sign);
}

void SafePS(char * s)
{
	int j=0, k=0, l=strlen(s);
	char *tbuf = AllocMem(l*2, MEMF_PUBLIC);

	if(!tbuf) return;

	while(j < (l+1))
	{
		tbuf[k]=s[j];
		if(s[j]==13 && s[j+1]!=10)
		{
			k++;
			tbuf[k]=10;
		}
		if(s[j]==10 && s[j-1]!=13)
		{
			tbuf[k]=13;
			k++;
			tbuf[k]=10;
		}
		k++;
		j++;
	}
	tbuf[k]='\0';
	PS(tbuf);
	FreeMem(tbuf, l*2);
}

ULONG SpaceFree(void)
{
	__aligned struct InfoData info;
	
     //getdfs(0, &info);
	
     BPTR lck = Lock("", SHARED_LOCK);
	if(lck)
	{
		Info(lck, &info);
	}
	return((ULONG)(info.id_NumBlocks - info.id_NumBlocksUsed) * info.id_BytesPerBlock);
}

void OutP(void)
{
	DrawImage(Amiga.rp, Amiga.inpImage, 604, 3);
}

void InP(void)
{
	DrawImage(Amiga.rp, Amiga.outpImage, 604, 3);
}

int rnd(int x)
{
	int i;
	if(x <= 0) return(0);
	do
	{
		i = rand()%x;
	} while(i > x);
	return(i);
}

void DeCrypt(char *str)
{
	UWORD i = 0, j = strlen(str);

	while(i < j)
	{
		str[i] ^= 0xFF;
		str[i]--;
		i++;
	}
}

void CallerLog(char * logstr)
{
	if(!var.LOCAL)
	{
		BPTR fh;
		char fname[32];

		mysprintf(fname, "SX:Node%ld/CallersLog", var.node);
		fh = Open(fname, MODE_READWRITE);
		if(fh)
		{
			Seek(fh, 0, OFFSET_END);
			Write(fh, logstr, strlen(logstr));
	 		Close(fh);
		}
	}
}

void addcom(char *com)
{                          
        comdex++;
 
        if(comdex > 29) comdex = 1;
        if(comdex < 1) comdex = 29;
	// what is this?????????????
        //history[comdex] = AllocMem(64, MEMF_PUBLIC|MEMF_CLEAR);
        strcpy(history[comdex], com);               
}


/*void UDLog(char * logstr)
{
	BPTR fh;
	char fname[30];

	if(var.LOCAL) return;
	mysprintf(fname,"SX:Node%ld/UDLog",var.node);
	fh=Open(fname,MODE_READWRITE);
	if(!fh) fh=Open(fname,MODE_NEWFILE); else Seek(fh,0,OFFSET_END);
	if(fh) {
		Write(fh,logstr,strlen(logstr));
 		Close(fh);
	}
}*/

