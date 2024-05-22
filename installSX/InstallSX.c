static char __version[] ="\0$VER: SX 1.06 [eSSeXX] Installer v1.o ("__DATE__")";

#include <exec/types.h>
#include <exec/memory.h>
#include <exec/execbase.h>
#include <libraries/dos.h>
#include <intuition/intuition.h>
#include <proto/exec.h>
#include <proto/dos.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <libraries/reqtools.h>
#include <proto/reqtools.h>

extern struct ExecBase *SysBase;
struct ReqToolsBase *ReqToolsBase;

ULONG undertag[] = { RT_Underscore, '_', TAG_END };

void CheckPath(char *path);
void EdStartup(char *line, USHORT batch);

void myputs (char *str)
{
	Write (Output(), str, strlen(str));
}

void main (char *line)
{		
     struct rtFileRequester *filereq; 
	char buffer[128], filename[128], exestr[128], dirname[4]="SX";
	ULONG ret;
     ULONG tags[] = { RTEZ_ReqTitle, (ULONG)"SX 1.06 (eSSeXX) Installer", TAG_END };
     USHORT batch;
     

	if (!(ReqToolsBase = (struct ReqToolsBase *)OpenLibrary (REQTOOLSNAME, REQTOOLSVERSION))) 
     {
		myputs ("You need reqtools.library V38 or higher!\n"
		        "Please install it in your Libs: directory.\n"
                  "Have a look at the Libs in this archive.");
		exit (0);
     }

	ret = rtEZRequest ("      System-X 1.06 (eSSeXX) Installer      \n"
	                   "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n\n"
			         "The Installer will copy all related Files to\n"
                        "  your HD and does all other needed changes \n",
                        " Install SX | Abort ", NULL, (struct TagItem *)tags);
     if(ret==FALSE) exit(0);

newpath:

     if (filereq = rtAllocRequestA (RT_FILEREQ, NULL)) 
     {
		if (rtFileRequest (filereq, filename, "Were to create the SX directory ?", RTFI_Flags, FREQF_NOFILES, TAG_END));  
		else
          {
			ret = rtEZRequest ("No directory selected. Select again ?", " YES | No ", NULL, (struct TagItem *)tags);
               if(ret==FALSE) exit(0);
               else goto newpath;
          }
		rtFreeRequest (filereq);
	}
	else
		rtEZRequest ("Out of memory!", "Oh boy!", 0, (struct TagItem *)tags);
  
   strcpy(buffer,filereq->Dir);
   CheckPath(buffer);
   if(buffer[0]==0)
   {
        goto newpath;
               
   } else
   {
         ret = rtEZRequest("Install System-X 1.06 (eSSeXX) ==>> %s%s ?"," Install | Abort ",0,(struct TagItem *)tags,buffer,dirname);
         if(ret==FALSE) exit(0);
         
         sprintf(exestr,"c:makedir %s%s",buffer, dirname); // make target dir
         Execute(exestr,0,0);

         sprintf(exestr,"c:assign SX: %s%s", buffer, dirname); // assign sx:
         Execute(exestr,0,0);

         ret = rtEZRequest("Now copy the C: commands to your C: directory"," OK | Abort ",0,(struct TagItem *)tags);
         if(ret==FALSE) exit(0);
         sprintf(exestr,"c:copy c to sys:c all", buffer, dirname);
         Execute(exestr,0,0);         

         ret = rtEZRequest("Now copy the DEVS: commands to your DEVS: directory"," OK | Abort ",0,(struct TagItem *)tags);
         if(ret==FALSE) exit(0);
         sprintf(exestr,"c:copy devs to sys:devs all", buffer, dirname);
         Execute(exestr,0,0);    
         
         ret = rtEZRequest("Now copy the L: commands to your L: directory"," OK | Abort ",0,(struct TagItem *)tags);
         if(ret==FALSE) exit(0);
         sprintf(exestr,"c:copy l to sys:l all", buffer, dirname);
         Execute(exestr,0,0);    

         ret = rtEZRequest("Now copy the LIBS: commands to your LIBS: directory"," OK | Abort ",0,(struct TagItem *)tags);
         if(ret==FALSE) exit(0);
         sprintf(exestr,"c:copy libs to sys:libs all", buffer, dirname);
         Execute(exestr,0,0);    
         
         ret = rtEZRequest("Now copy the S: commands to your S: directory"," OK | Abort ",0,(struct TagItem *)tags);
         if(ret==FALSE) exit(0);
         sprintf(exestr,"c:copy s to sys:s all", buffer, dirname);
         Execute(exestr,0,0);    
         
         ret = rtEZRequest("Now copy System-X to your SX: directory.\n"
                           "This operation could take some seconds.."," OK | Abort " ,0,(struct TagItem *)tags);
         if(ret==FALSE) exit(0);
         sprintf(exestr,"c:copy sx to %s%s/ all", buffer, dirname);
         Execute(exestr,0,0);    
         sprintf(exestr,"c:copy sx.info to %s", buffer, dirname);
         Execute(exestr,0,0);   
         
         // add the assigns now                                               
         ret = rtEZRequest("Like to add the needed assigns to your\n"
                           "UserStartup or to SX:Startbatch?","UserStartup|Startbatch",0,(struct TagItem *)tags);
         if(ret==FALSE) batch=1;
         else 
              EdStartup(";START SYSTEM-X",batch);                   
              sprintf(exestr,"\nC:assign >NIL: SX: %s%s", buffer, dirname);
              EdStartup(exestr,batch);
              sprintf(exestr,"\nC:assign >NIL: BBS: %s%s", buffer, dirname);
              EdStartup(exestr,batch);
              sprintf(exestr,"\nC:assign >NIL: Doors: sx:doors");
              EdStartup(exestr,batch);
              sprintf(exestr,"\nC:assign >NIL: Files: sx:confs");
              EdStartup(exestr,batch);
              sprintf(exestr,"\nC:assign >NIL: DD: %s%s", buffer, dirname);
              EdStartup(exestr,batch);
              sprintf(exestr,"\npath SX:utils c: add %s%s", buffer, dirname);
              EdStartup(exestr,batch);
                       
         if(batch==1) // = startbatch 
         {
                sprintf(exestr,"\nC:run >NIL: <NIL: L:Fifo-Handler ; fifo-handler for usage of STDIO doors");
                EdStartup(exestr, batch);
                
                sprintf(exestr,"\nC:run >NIL: SX:SX-MCP            ; Start SX-Master Control Program");
                EdStartup(exestr,batch);
                     EdStartup("\nWait 2                           ; wait a bit for MCP",batch);
                     
                sprintf(exestr,"\nC:run >NIL: SX:SX 1              ; Start Node #1 i.e. for Serial connects");
                EdStartup(exestr,batch);
               
                sprintf(exestr,"\nC:run >NIL: SX:SX 3              ; Start Node #3 i.e. for Console logins");
                EdStartup(exestr,batch);

               
                EdStartup("\n;END SYSTEM-X",batch);
                            
                sprintf(exestr,"c:rename sx:StartBatchExample.info to sx:StartBatch.info");
                Execute(exestr,0,0);

                sprintf(exestr,"C:protect SX:StartBatch +s");
                Execute(exestr,0,0);              
         
         } else // batch = 0 = user-startup
         {
                ret = rtEZRequest("Run SX (eSSeXX) while computer startup ?"," Yes | No ",0,(struct TagItem *)tags);
                if(ret==TRUE)
                {
                        sprintf(exestr,"\nC:run >NIL: <NIL: L:Fifo-Handler ; fifo-handler for usage of STDIO doors");
                        EdStartup(exestr, batch);
                        sprintf(exestr,"\nC:run >NIL: SX:SX-MCP");
                        EdStartup(exestr,batch);
                        sprintf(exestr,"\nC:run >NIL: SX:SX 1");
                        EdStartup(exestr,batch);
                        EdStartup("\n;END SYSTEM-X (eSSeXX)",batch);

                }else
                {
                        EdStartup("\n;END SYSTEM-X (eSSeXX)",batch);
                }                              
         }            
             
         ret = rtEZRequest("Now we set up the SYSOP ACCOUNT.\n"
                           "Please follow the questions by the\n"
                           "Requesters..."," OK | Abort ",0,(struct TagItem *)tags);
         if(ret==FALSE) exit(0);
         
         sprintf(exestr,"sx:utils/makeac");  // make a new useraccount here to define the sysop
         Execute(exestr,0,0);
    
         sprintf(exestr,"C:run C:amigaguide sx:doc/SX.Guide");  // launch the guide for more infoz
         Execute(exestr,0,0);              
   }                                 
   Delay(75);   
   rtEZRequest("That's it, installation is complete!\n"
	          "  and a new UserBase is created.    \n"
               "   SX 1.06 (eSSeXX) is complete     \n"
               "    installed to your HD-Path.      \n"
               " Read the Guide now, for additional \n"
               "informations about the BBS softawre.\n",
               "NICE",0,(struct TagItem *)tags);

   CloseLibrary ((struct Library *)ReqToolsBase);
   exit (0);
}

void CheckPath(char *path)
{
	UWORD l = strlen(path)-1;
	if(path[l]!='/' && path[l]!=':')
   {
      rtEZRequest("                   WARNING!                   \n"
                  "       Must be `/` or `:` at the End!         \n"
                  "Remeber: directory `SX` will be created for you!","OK",0,0,0);
      path[0]=0;
   }
}

void EdStartup(char *line, USHORT batch)
{
        BPTR fh;
       
        char buf[128];
        
        if(batch!=1) sprintf(buf,"S:User-Startup");
        else sprintf(buf,"SX:Startbatch");
        fh = Open(buf, MODE_OLDFILE);       
        if(!fh) fh = Open(buf, MODE_NEWFILE);
        if(fh)
        {
                Seek(fh,0,OFFSET_END);
                Write(fh, line, strlen(line));
         
        }
        if(fh) Close(fh);
}        

