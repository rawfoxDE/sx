/*

----------------------------------------------------
 Programming doors for System-X using EXEC-MESSAGES
----------------------------------------------------

See the botton of this source!

*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include "sxstructs.h"

struct MsgPort *bbsport;

struct JHMessage
{
  struct Message Msg;
  char String[200];
  int Data;
  int Command;
};
struct JHMessage themsg;


void PS(char * str);
void XIMFunction(int func, long data, char * str);
void Door(void);

BOOL sx;


int main(int argc, char *argv[])
{
	char portname[16];

	if(argv[1][0]==0)
	{
		PutStr("This program requires System-X BBS Software\n");
	} else {
		sprintf(portname, "AEDoorPort%s", argv[1]);
		bbsport = FindPort(portname);
		if(bbsport)
		{
			XIMFunction(1, 0, 0); 	/* function 1 = register */

			/* find out if we are under SYSTEM-X or AmiExpress */

			if(strcmp(themsg.String,"SX")==0) sx=TRUE; else sx=FALSE;

			Door();

			XIMFunction(2, 0, 0); 	/* function 2 = shutdown */
		}
	}
}

void PS(char * str)
{
	if(sx) XIMFunction(1500, (long)str, 0); else XIMFunction(4, 0, str);
}

void XIMFunction(int func, long data, char * str)
{
	struct MsgPort *replyport;

	replyport = CreateMsgPort();
	if(replyport)
	{
		themsg.Msg.mn_Length	= sizeof(struct JHMessage);
		themsg.Msg.mn_ReplyPort	= replyport;
		themsg.Data 		= data;
		themsg.Command 		= func;
		if(str && str[0]!=0) strcpy(themsg.String, str);
		PutMsg(bbsport, (struct Message *)&themsg);
		WaitPort(replyport);
		DeleteMsgPort(replyport);
	}
}

/* ============ PUT YOUR DOOR IN HERE ============== */

void Door(void)
{
        char buf[256];
   

	PS("Hello! Welcome the BB_LOGONTYPE Checker :)\r\n");
	XIMFunction(517,0,0);
     //sprintf(buf,"\n\rDATA IS: %ld", themsg.Data);
     //PS(buf);
     switch(themsg.Data)
	{        
      case 1:
           sprintf(buf,"\n\r Data = 1 Sysop Login");
           PS(buf);
           break;
      case 2:
           sprintf(buf,"\n\r Data = 2 Local Login");
           PS(buf);
           break;
      case 3:
           sprintf(buf,"\n\r Data = 3 Remote Login");
           PS(buf);
           break;
     } 
          
	PS("\r\n\r\nExiting...\r\n\r\n");

}
