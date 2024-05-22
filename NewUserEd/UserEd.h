/*************************************************************/
/* Includes and other common stuff for the MUI demo programs */
/*************************************************************/

/* MUI */
#include <libraries/mui.h>

/* System */
#include <dos/dos.h>
#include <graphics/gfxmacros.h>
#include <workbench/workbench.h>
#include <exec/memory.h>

/* Prototypes */
#include <clib/alib_protos.h>
#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/icon_protos.h>
#include <clib/graphics_protos.h>
#include <clib/intuition_protos.h>
#include <clib/gadtools_protos.h>
#include <clib/utility_protos.h>
#include <clib/asl_protos.h>

#ifndef __GNUC__
#include <clib/muimaster_protos.h>
#else
#include <inline/muimaster.h>
#endif

/* ANSI C */
#include <stdlib.h>
#include <string.h>
#include <stdio.h>


/* Compiler specific stuff */


#define REG(x) register __ ## x
#define ASM    __asm
#define SAVEDS __saveds

#include <pragmas/exec_sysbase_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/icon_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/gadtools_pragmas.h>
#include <pragmas/utility_pragmas.h>
#include <pragmas/asl_pragmas.h>
#include <pragmas/muimaster_pragmas.h>

extern struct Library *SysBase,*IntuitionBase,*UtilityBase,*GfxBase,*DOSBase,*IconBase;
struct Library *MUIMasterBase;


/*************************/
/* Init & Fail Functions */
/*************************/

static VOID fail(APTR app,char *str)
{
        if (app)
                MUI_DisposeObject(app);

#ifndef _DCC
        if (MUIMasterBase)
                CloseLibrary(MUIMasterBase);
#endif

        if (str)
        {
                puts(str);
                exit(20);
        }
        exit(0);
}


#ifdef _DCC

int brkfunc(void) { return(0); }

int wbmain(struct WBStartup *wb_startup)
{
        extern int main(int argc, char *argv[]);
        return (main(0, (char **)wb_startup));
}

#endif


#ifdef __SASC
int CXBRK(void) { return(0); }
int _CXBRK(void) { return(0); }
void chkabort(void) {}
#endif


static VOID init(VOID)
{
#ifdef _DCC
        onbreak(brkfunc);
#endif

#ifndef _DCC
        if (!(MUIMasterBase = OpenLibrary(MUIMASTER_NAME,MUIMASTER_VMIN)))
                fail(NULL,"Failed to open "MUIMASTER_NAME".");
#endif
}


#ifndef __SASC
static VOID stccpy(char *dest,char *source,int len)
{
        strncpy(dest,source,len);
        dest[len-1]='\0';
}
#endif


#ifndef MAKE_ID
#define MAKE_ID(a,b,c,d) ((ULONG) (a)<<24 | (ULONG) (b)<<16 | (ULONG) (c)<<8 | (ULONG) (d))
#endif


LONG __stack = 8192;
