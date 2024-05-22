
#include <time.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <exec/types.h>
#include <exec/memory.h>
#include <exec/io.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <dos/dostags.h>
#include <devices/serial.h>
#include <devices/timer.h>
#include "SXStructs.h"
#include "sx.h"

extern struct StructsStruct	*Structs;
extern struct AmigaStruct	Amiga;
extern struct StringsStruct	*stg;
extern struct VariablesStruct	var;

extern UBYTE buf[512], charbuf[128];

void SModem(char *recpath);
void HydraCom(char *recpath);
void FixFileName(char *fname);


void DumpFlags(char *fname)
{
	struct Flag *worknode, *nextnode;
	BPTR fh;

	if (Amiga.Flags->lh_TailPred != (struct Node *)Amiga.Flags)
	{
		fh = Open(fname, MODE_NEWFILE);
		if(!fh) return;
		worknode = (struct Flag *)(Amiga.Flags->lh_Head); /* First node */
		while (nextnode = (struct Flag *)(worknode->flag_node.ln_Succ))
		{
			mysprintf(buf, "%s%s\n", worknode->path, worknode->file);
			if(var.instsend)
			{
				if(worknode->free&(1<<1)) Write(fh, buf, strlen(buf));
			} else
				Write(fh, buf, strlen(buf));
			worknode = nextnode;
		}
		Close(fh);
	}
}

void SModem(char *recpath)
{
	long x;
	BPTR fh;
	char *po;
	struct Flag *worknode;

	SetActive(102, 0);

	mysprintf(buf, "T:SM_LEECH%ld.TMP", var.node);
	DumpFlags(buf);

	mysprintf(buf,
	"SModem DEVICE %s UNIT %ld BAUD %ld ULLIST T:SM_LEECH%ld.TMP DLPATH %s DSZLOG T:DSZ%ld.LOG CHKCARRIER CTSRTS FORCECAP AUTOEXIT",
		Structs->Serial.device,
		Structs->Serial.unit,
		Structs->Serial.dcerate,
		var.node,
		recpath,
		var.node);
	SystemTagList(buf, 0);
	//system(buf);

	mysprintf(buf, "T:DSZ%ld.LOG", var.node);
	fh = Open(buf, MODE_OLDFILE);
	if(fh)
	{
		while(FGets(fh, buf, 256))
		{
			buf[strlen(buf)-3] = 0;
			po = (char *)((long)buf + 50);
			po = FilePart(po);
			x = (long)buf + 1;
			switch(buf[0])
			{
			case 's':
				worknode = (struct Flag *) FindFlag(po);
				if(worknode)
				{
					worknode->xfer = atol((char *)x);
					po = (char *)((long)buf + 18);
					worknode->cps = atol(po);
				}
				break;
			case 'S':
				Amiga.uploadnode = AllocMem(sizeof(struct Flag), MEMF_PUBLIC|MEMF_CLEAR);
				strcpy(Amiga.uploadnode->file, po);
				Amiga.uploadnode->flag_node.ln_Name = Amiga.uploadnode->file;
				Amiga.uploadnode->size = atol((char *)x);
				Amiga.uploadnode->xfer = Amiga.uploadnode->size;
				po = (char *)((long)buf + 18);
				Amiga.uploadnode->cps = atol(po);
				Amiga.uploadnode->conf = var.thisconf;
				AddTail((struct List *)Amiga.Uploads, (struct Node *)Amiga.uploadnode);
				break;
			}
		}
		Close(fh);
		mysprintf(buf, "T:DSZ%ld.LOG", var.node);
		DeleteFile(buf);
	}
	mysprintf(buf, "T:SM_LEECH%ld.TMP", var.node);
	DeleteFile(buf);
	
	CheckUploads();

	CheckFlags();
}

void HydraCom(char *recpath)
{
	BPTR fh;
	long cps;
	char *po, recname[128], sendname[128];
	struct Flag *worknode;

	SetActive(103, 0);

	mysprintf(buf, "T:SM_LEECH%ld.TMP", var.node);
	DumpFlags(buf);

	mysprintf(buf,
	"HydraCom port %ld speed %ld device %s log T:SXHYDRA%ld.LOG level 2 receive %s send @T:SM_LEECH%ld.TMP",
		Structs->Serial.unit,
		Structs->Serial.dcerate,
		Structs->Serial.device,
		var.node,
		recpath,
		var.node);
	SystemTagList(buf, 0);
	//system(buf);

	mysprintf(buf, "T:SXHYDRA%ld.LOG", var.node);
	fh = Open(buf, MODE_OLDFILE);
	if(fh)
	{
		while(FGets(fh, buf, 256))
		{
			buf[strlen(buf)-3] = 0;
			po = (char *)((long)buf + 23);

			if(strncmp("HSEND", po, 5)==0)
			{
				po = (char *)((long)po + 7);
				FixFileName(po);
				po = FilePart(po);
				strcpy(sendname, po);
			}

			if(strncmp("HRECV", po, 5)==0)
			{
				po = (char *)((long)po + 7);
				FixFileName(recname);
				po = FilePart(po);
				strcpy(recname, po);
			}

			if(strncmp("Sent-H", po, 6)==0)
			{
				worknode = (struct Flag *) FindFlag(sendname);
				if(worknode)
				{
					worknode->cps = atol((char *)((long)po + 11));
					worknode->xfer = worknode->size;
				}
			}

			if(strncmp("Rcvd-H", po, 6)==0)
			{
				cps = atol((char *)((long)po + 11));
				Amiga.uploadnode = AllocMem(sizeof(struct Flag), MEMF_PUBLIC|MEMF_CLEAR);
				strcpy(Amiga.uploadnode->file, recname);
				Amiga.uploadnode->flag_node.ln_Name = Amiga.uploadnode->file;
				Amiga.uploadnode->size = 1;
				Amiga.uploadnode->xfer = 1;
				Amiga.uploadnode->cps = cps;
				Amiga.uploadnode->conf = var.thisconf;
				AddTail((struct List *)Amiga.Uploads, (struct Node *)Amiga.uploadnode);
			}

		}
		Close(fh);
		mysprintf(buf, "T:SXHYDRA%ld.LOG", var.node);
		DeleteFile(buf);
	}
	mysprintf(buf, "T:SM_LEECH%ld.TMP", var.node);
	DeleteFile(buf);

	CheckUploads();

	CheckFlags();
}

void FixFileName(char *fname)
{
/*	UWORD i = 0, len = strlen(fname);

	while(i < len)
	{
		if(fname[i]==' ') { fname[i]=0; return; }
		i++;
	}*/

	char *po = strchr(fname, ' ');
	if(po) po[0] = 0;
}
