
#include <devices/serial.h>
#include <clib/dos_protos.h>
#include <dos.h>
#include "sxstructs.h"

extern struct VariablesStruct var;

extern void mysprintf(char *Buffer,char *ctl, ...);
extern size_t strlen(const char *);

void CallerLog(char * logstr)
{
	BPTR fh;
	char fname[32];

	if(var.LOCAL) return;
	mysprintf(fname, "SX:Node%ld/CallersLog", var.node);
	fh = Open(fname, MODE_READWRITE);
	if(fh)
	{
		Seek(fh, 0, OFFSET_END);
		Write(fh, logstr, strlen(logstr));
 		Close(fh);
	}
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
