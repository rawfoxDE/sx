
#include <stdlib.h>
#include <string.h>
#include <exec/memory.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <devices/serial.h>
#include "SXStructs.h"
#include "SX.h"

extern struct StructsStruct	*Structs;
extern struct AmigaStruct	Amiga;
extern struct StringsStruct	*stg;
extern struct VariablesStruct	var;

extern UBYTE buf[512], charbuf[128];

long x;

void UnFlagPattern(char *pat, BOOL show);
int UnFlagFile(char * f);
int FlagFile(char * f, BOOL free, BOOL inst, BOOL raw);
void ClearFlags(void);
struct Node * FindFlag(char * f);
LONG FindFileOnHD(char * f, BOOL raw);
long FilesLeft(void);
long BytesLeft(void);


BOOL ListFlags(void)
{
	struct Flag *worknode, *nextnode;
	ldiv_t result;

	ShowAnsi("EditHeader",1,1,1,1,1,0);
	if (Amiga.Flags->lh_TailPred == (struct Node *)Amiga.Flags)
	{
		PS(Structs->SXStr[37]);
		return(FALSE);
	} else {
		worknode = (struct Flag *)(Amiga.Flags->lh_Head); /* First node */
		while (nextnode = (struct Flag *)(worknode->flag_node.ln_Succ))
		{
			result = ldiv((worknode->size/(var.baud/10)),60);
			PSFmt(Structs->SXStr[36], worknode->file, worknode->size, result.quot, result.rem+1);
			worknode = nextnode;
		}
	}
	result = ldiv((var.byteflags/(var.baud/10)),60);
	PSFmt(Structs->SXStr[44], var.byteflags, result.quot, result.rem+1);
	if(Structs->User.Time_Left < var.byteflags/(var.baud/10)) return(TRUE);
	return(FALSE);
}

void AlterFlags(void)
{
	BOOL remove = FALSE;

	SetActive(108, 0);
list:
	ListFlags();
add:
	PS(Structs->SXStr[29]);
	LineInput(50, 0, 0);
	PS("\r\n");
	if(!var.CARRIER) charbuf[0]=0;
	if(stricmp(charbuf, "L")==0) goto list;
	if(stricmp(charbuf, "R")==0)
	{
		PS(Structs->SXStr[30]);
		LineInput(50,0,0);
		if(!var.CARRIER) return;
		PS("\r\n");
		remove=TRUE;
	}
	if(stricmp(charbuf, "C")==0)
	{
		UnFlagPattern("#?", FALSE);
		PS(Structs->SXStr[112]);
		goto add;
	}
	if(charbuf[0]!='\0')
	{
		if(remove)
		{
			UnFlagPattern(charbuf, TRUE);
			remove = FALSE;
		} else {
			if(strchr(charbuf, '/') || strchr(charbuf, ':') || strchr(charbuf, '\\'))
				PS(Structs->SXStr[87]);
			else {
				switch(FlagFile(charbuf,0,0,0))
				{
				case 0: PS(Structs->SXStr[34]); break;
				case 1: PSFmt(Structs->SXStr[35], charbuf, x); break;
				case 2: PS(Structs->SXStr[33]); break;
				case 3: PS(Structs->SXStr[31]); break;
				case 4: PS(Structs->SXStr[107]); break;
				case 5: PS(Structs->SXStr[106]); break;
				}
			}
		}
		goto add;
	}
}

void UnFlagPattern(char *pat, BOOL show)
{
	struct Flag *worknode,*nextnode;
	char parsepat[256];
	BOOL match=FALSE;

	if (Amiga.Flags->lh_TailPred != (struct Node *)Amiga.Flags)
	{
		ParsePatternNoCase(pat, parsepat, 256);
		worknode = (struct Flag *)(Amiga.Flags->lh_Head); /* First node */
		while (nextnode = (struct Flag *)(worknode->flag_node.ln_Succ))
		{
			if(MatchPatternNoCase(parsepat, worknode->file))
			{
				var.fileflags--;
				var.byteflags=var.byteflags-worknode->size;
				if(!(worknode->free&(1<<0)))
				{
					var.fileflagsfree--;
					var.byteflagsfree=var.byteflagsfree-worknode->size;
				}
				if(show) PSFmt(Structs->SXStr[32], worknode->file);

				match=TRUE;
				Remove(worknode);
				FreeMem(worknode, sizeof(struct Flag));
			}
			worknode = nextnode;
		}
	}
	if(!match && show) PS(Structs->SXStr[31]);
}

int FlagFile(char * f, BOOL free, BOOL inst, BOOL raw)
{
	struct Node *node;
	struct Flag *newflag;
	char tbuf[128], *po;
	long size;

	if(raw)
		node = FindFlag(FilePart(f));
	else {
		if((Structs->Conf[var.thisconf]->flags&(1<<2)) && inst == FALSE) free = TRUE;
		node = FindFlag(f);
	}

	if(node) return(2);

	size = FindFileOnHD(f, raw);
	if(size==0) return(3);

	if(!free)
	{
		if(BytesLeft() < size) return(4);
		if(FilesLeft() < 1) return(5);
	}

	newflag = AllocMem(sizeof(struct Flag), MEMF_PUBLIC|MEMF_CLEAR);
	if(!newflag) return(0);

	if(raw)
	{
		po = FilePart(f);
		strcpy(newflag->file, po);
		strcpy(tbuf, f);
		po = FilePart(tbuf);
		po[0] = 0;
		strcpy(newflag->path, tbuf);
	} else {
		strcpy(newflag->file, f);
		strcpy(newflag->path, stg->path);
	}
	newflag->flag_node.ln_Name = newflag->file;
	newflag->size = size;
	newflag->xfer = 0;
	newflag->conf = var.thisconf;
	newflag->free = 0;
	if(free) newflag->free|=1<<0;
	if(inst) newflag->free|=1<<1;

	var.fileflags++;
	var.byteflags += size;

	if(!free)
	{
		var.fileflagsfree++;
		var.byteflagsfree += size;
	}

	AddTail((struct List *)Amiga.Flags, (struct Node *)newflag);
	x = size;
	return(1);
}

int UnFlagFile(char * f)
{
	struct Node *node;
	struct Flag *flagpo;

	node = FindFlag(f);
	if(!node) return(0);
	flagpo=(struct Flag *)node;
	var.fileflags--;
	var.byteflags = var.byteflags-flagpo->size;
	if(!(flagpo->free&(1<<0)))
	{
		var.fileflagsfree--;
		var.byteflagsfree = var.byteflagsfree-flagpo->size;
	}
	Remove(node);
	FreeMem(node, sizeof(struct Flag));
	return(1);
}

void ClearFlags(void)
{
	struct Flag *worknode,*nextnode;

	if (Amiga.Flags->lh_TailPred == (struct Node *)Amiga.Flags) return;

	worknode = (struct Flag *)(Amiga.Flags->lh_Head); /* First node */
	if(!worknode) return;
	while (nextnode = (struct Flag *)(worknode->flag_node.ln_Succ))
	{
		FreeMem(worknode,sizeof(struct Flag));
		worknode = nextnode;
	}
	var.fileflags=0;
	var.byteflags=0;
	var.fileflagsfree=0;
	var.byteflagsfree=0;
}

struct Node * FindFlag(char * f)
{
	struct Node *node;

	if(node = FindName(Amiga.Flags,f)) return(node);
	return(0);
}

LONG FindFileOnHD(char * f,BOOL raw)
{
	struct FileInfoBlock *fib=AllocMem(sizeof(struct FileInfoBlock),0);
	BPTR lck,fh;
	LONG size=0;
	char path[72];

	if(!fib) return(0);
	path[0]=0;
	if(raw) strcpy(buf, f);
	else { strcpy(path, Structs->Conf[var.thisconf]->filepath); mysprintf(buf,"%s%s", path, f); }
	if(!(lck=Lock(buf, SHARED_LOCK)))
	{
		mysprintf(buf, "%sPaths", Structs->Conf[var.thisconf]->path);
		if(fh=Open(buf, MODE_OLDFILE))
		{
			while(FGets(fh, buf, 420))
			{
				buf[strlen(buf)-1]=0;
				strcpy(path, buf);
				strcat(buf, f);
				if(lck=Lock(buf, SHARED_LOCK))
				{
					Close(fh);
					goto locked;
				}
			}
			Close(fh);
		} else {
			return(0);
		}
	} else {
locked:
		Examine(lck, fib);
		size = fib->fib_Size;
		UnLock(lck);
	}
	FreeMem(fib, sizeof(struct FileInfoBlock));
	var.size = size;
	strcpy(stg->path, path);
	return(size);
}

long FilesLeft(void)
{
	long temp=((((Structs->User.Uploads*Structs->SXUser.fileratio)+Structs->SXUser.freefiles)-Structs->User.Downloads)-var.fileflagsfree);
	if(Structs->SXUser.fileratio==0) temp = 20000001;
	return(temp);
}

long BytesLeft(void)
{
	long temp=((((Structs->User.Bytes_Upload*Structs->SXUser.byteratio)+Structs->SXUser.freebytes)-Structs->User.Bytes_Download)-var.byteflagsfree);
	if(Structs->SXUser.byteratio==0) temp = 20000001;
	return(temp);
}

void SaveFlags(void)
{
	struct Flag *worknode, *nextnode;
	BPTR lck, fh;

	mysprintf(buf, "%s%ld", Structs->Cfg->USERPath, Structs->User.Slot_Number);
	lck = Lock(buf, ACCESS_READ);
	if(!lck)
	{
		lck = CreateDir(buf);
		if(lck) UnLock(lck);
		SetComment(buf, Structs->User.Name);
	} else
		UnLock(lck);

	mysprintf(buf, "%s%ld/Flags.DAT", Structs->Cfg->USERPath, Structs->User.Slot_Number);

	if (Amiga.Flags->lh_TailPred != (struct Node *)Amiga.Flags)
	{
		fh = Open(buf, MODE_NEWFILE);
		if(fh)
		{
			worknode = (struct Flag *)(Amiga.Flags->lh_Head); /* First node */
			while (nextnode = (struct Flag *)(worknode->flag_node.ln_Succ))
			{
				Write(fh, &worknode->path, 128);
				worknode = nextnode;
			}
			Close(fh);
		}
	} else {
		DeleteFile(buf);
	}
}

void LoadFlags(void)
{
	struct Flag *newflag;
	BPTR fh;
	BOOL readfin=FALSE;

	mysprintf(buf, "%s%ld/Flags.DAT", Structs->Cfg->USERPath, Structs->User.Slot_Number);
	fh = Open(buf, MODE_OLDFILE);
	if(!fh) return;

	PS(Structs->SXStr[108]);

	while(!readfin)
	{
		newflag = AllocMem(sizeof(struct Flag), MEMF_PUBLIC|MEMF_CLEAR);
		if(!newflag)
		{
			Close(fh);
			return;
		}

		if(!(Read(fh, &newflag->path, 128)))
		{
			readfin = TRUE;
			FreeMem(newflag, sizeof(struct Flag));
		} else {
			newflag->flag_node.ln_Name = newflag->file;
			var.fileflags++;
			var.byteflags=var.byteflags+newflag->size;
			if(!(newflag->free&(1<<0)))
			{
				var.fileflagsfree++;
				var.byteflagsfree=var.byteflagsfree+newflag->size;
			}
			PSFmt(Structs->SXStr[35], newflag->file, newflag->size);
			AddTail((struct List *)Amiga.Flags, (struct Node *)newflag);
		}
	}
	Close(fh);
	PS("\r\n");
	HitReturn();
}
