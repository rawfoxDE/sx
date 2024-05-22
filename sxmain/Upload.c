
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <exec/memory.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/reqtools.h>
#include <intuition/intuition.h>
#include <xproto.h>
#include <libraries/reqtools.h>
#include <devices/serial.h>
#include <dos/dos.h>
#include "SXStructs.h"
#include "SX.h"

extern struct StructsStruct	*Structs;
extern struct AmigaStruct	Amiga;
extern struct StringsStruct	*stg;
extern struct VariablesStruct	var;
extern struct XPR_IO		xio;
extern struct Library		*XProtocolBase;
extern struct ReqToolsBase	*ReqToolsBase;

extern UBYTE buf[512], charbuf[128];

char *desc[22];


void GetPath(char *outbuf, UWORD confnum, UWORD area);
int FindArcPo(char *fname);
void LocalUpload(BOOL raw, char *recpath);
int EnterDesc(char * fname);
void CheckResume(void);
void StoreFile(char *fname);
void CheckLCFiles(void);
void RawReceive(char *recpath, BOOL freezeclock);
void FileCheckDoor(char *fname);
BOOL CopyFile(char *fname, char *dest);
void MoveFile(char *fname, char *dest);


void UploadFile(BOOL showansi)
{
	BPTR old, lck;

	SetActive(2, 0);
	if(Structs->Conf[var.thisconf]->fileareas == 0)
	{
		PS(Structs->SXStr[109]);
		return;
	}

	lck = Lock(Structs->NodePrefs.fcheck, SHARED_LOCK);
	if(lck)
	{
		old = CurrentDir(lck);
		//chdir(Structs->NodePrefs.fcheck);
		if(SpaceFree() < Structs->Cfg->minspace)
		{
			PS(Structs->SXStr[138]);
			goto xit2;
		}

		if(showansi)
		{
			ShowAnsi("Upload", 1, 1, 1, 1, 1, 0);
			CheckResume();
ask:
			PS(Structs->SXStr[105]);
			LineInput(20, 0, 0);
			PSLen("\r\n", 2);
			if(!var.CARRIER || stricmp(charbuf, "A")==0) goto xit2;
			if(stricmp(charbuf, "P")!=0 && charbuf[0]!=0) goto ask;
		}

		PSFmt(Structs->SXStr[40], Structs->Proto.name);

		if(var.LOCAL)
			LocalUpload(FALSE, Structs->NodePrefs.fcheck);
		else
			RawTransfer(2, FALSE, TRUE, Structs->NodePrefs.fcheck, 0);

		ShowAnsi("PostUpload", 1, 1, 1, 1, 1, 0);
xit:
		mysprintf(buf, "Delete %s#? >NIL:", Structs->NodePrefs.fcheck);
		//system(buf);
		SystemTagList(buf, 0);
xit2:
		CurrentDir(old);
		UnLock(lck);
	}
}

void AddFile(char * filename, ULONG cps, ULONG size)
{
	struct FileInfoBlock *fib = AllocMem(sizeof(struct FileInfoBlock),0);
	BPTR lck, fh;
	char tbuf[256];
	UWORD usingarea, a;
	int i;
	char *po, *descbuf;

	if(!fib) return;
	var.CARRIER = Carrier();
	if(!var.CARRIER)
	{
		FreeMem(fib, sizeof(struct FileInfoBlock));
		StoreFile(filename);
		return;
	}
	for(a=0;a<21;a++) desc[a] = AllocMem(46, MEMF_PUBLIC|MEMF_CLEAR);
	if(lck = Lock(filename, SHARED_LOCK))
	{
		if(size == 0)
		{
			Examine(lck, fib);
			size = fib->fib_Size;
		}
 		UnLock(lck);
		if(strlen(filename) > 12)
		{
			tbuf[0]=filename[0];
			strmid(filename, (char *)&tbuf[1], strlen(filename)-10, 11);
			Rename(filename, tbuf);
			strcpy(filename, tbuf);
		}
		DeleteFile("FILE_ID.DIZ");

		PSFmt(Structs->SXStr[69], filename, size);

		if(a = FindArcPo(filename))
		{
			if(Structs->Arc[a]->testcmd[0]!=0)
			{
				PS(Structs->SXStr[73]);
				mysprintf(tbuf, Structs->Arc[a]->testcmd, filename);
				if(Structs->Arc[a]->flags&(1<<0))
					FifoExecute(tbuf, 0);
				else {
					if(!strstr(tbuf,">NIL:")) strcat(tbuf," >NIL:");
					SystemTagList(tbuf, 0);
					//system(tbuf);
				}
			}
			if(Structs->Arc[a]->unpackcmd[0]!=0)
			{
				PS(Structs->SXStr[71]); /* "unpacking diz..." */
				mysprintf(buf, Structs->Arc[a]->unpackcmd, filename);
				if(!strstr(buf, ">NIL:")) strcat(buf, " >NIL:");
				//system(buf);
				SystemTagList(buf, 0);
				if(fh = Open("FILE_ID.DIZ", MODE_OLDFILE))
				{
					i=0;
					while(FGets(fh, buf, 510) && i<20)
					{
						i++;
						buf[strlen(buf)-1] = 0;
						if(strlen(buf) > 44)
						{
							strmid(buf, charbuf, 1, 44);
							strcpy(buf, charbuf);
						}
						po = strstr(buf, "\r");
						if(po) po[0] = 0;
						po = strstr(buf, "\n");
						if(po) po[0] = 0;
						strcpy(desc[i], buf);
						PSFmt(Structs->SXStr[70], i, desc[i]);
					}
					Close(fh);
				} else {
					i = EnterDesc(filename);
				}
			} else {
				i = EnterDesc(filename);
			}
		} else {
			i = EnterDesc(filename);
		}
		switch(i)
		{
		case 0:
			i=1;
			strcpy(desc[1], "No desc available");
			break;
		case -1:
			StoreFile(filename);
			goto endo;
		case -2:
			goto endo;
		}


		/* === save to filelist === */

		if(Structs->Conf[var.thisconf]->uploadarea == 0)
		{
			if(var.thisfilearea == 0)
				usingarea = Structs->Conf[var.thisconf]->fileareas;
			else
				usingarea = var.thisfilearea;
		} else {
			usingarea = Structs->Conf[var.thisconf]->uploadarea;
		}

		descbuf = AllocMem(2048, MEMF_PUBLIC);
		if(descbuf)
		{
			long off;

			mysprintf(descbuf, "%-12s P%7ld  %s  %s\n", filename, size, mydatestr(), desc[1]);
			a = 1;
			while(a != i)
			{
				a++;
				if(desc[a][0] != 0)
				{
					off = strlen(descbuf);
					mysprintf(&descbuf[off], "                                 %s\n", desc[a]);
				}
			}
			if((Structs->Conf[var.thisconf]->flags&(1<<0)) && Structs->SXUser.sentbyline[0]!=0)
			{
				off = strlen(descbuf);
				mysprintf(&descbuf[off], "                                 %s\n", Structs->SXUser.sentbyline);
			}

			mysprintf(buf, "%sdir%ld", Structs->Conf[var.thisconf]->path, usingarea);
			Structs->McpMsg.data1 = (long) &buf;
			Structs->McpMsg.data2 = (long) descbuf;
			Structs->McpMsg.data3 = (long) strlen(descbuf);
			Servermsg(12);

			FreeMem(descbuf, 2048);
		}

		/* === add to credits and log === */

		Structs->User.Uploads++;
		Structs->User.Bytes_Upload += size;
		mysprintf(buf, "	Uploading %-12s%7ld bytes\n	 1 file(s), %ldk bytes, 1 minute(s). 10 second(s), %ld cps, %ld %% efficiency.\n", filename, size, size/1024, cps, cps*100/(var.baud/10));
		CallerLog(buf);


		/* === move the file to correct directory === */

		GetPath(tbuf, var.thisconf, usingarea);
		MoveFile(filename, tbuf);
	}
endo:
	FreeMem(fib, sizeof(struct FileInfoBlock));
	for(a=0;a<21;a++) FreeMem(desc[a], 46);
	DeleteFile("FILE_ID.DIZ");
}

void GetPath(char *outbuf, UWORD confnum, UWORD area)
{
	char tbuf[256];
	UWORD j = 0;
	BPTR fh;

	mysprintf(tbuf, "%sPaths", Structs->Conf[confnum]->path);
	if( fh = Open(tbuf, MODE_OLDFILE) )
	{
		while( FGets(fh, tbuf, 256) )
		{
			j++;
			if(j == area)
			{
				Close(fh);
				tbuf[strlen(tbuf)-1] = 0;
				strcpy(outbuf, tbuf);
				return;
			}
		}
		Close(fh);
	}
	strcpy(outbuf, Structs->Conf[confnum]->filepath);
}

int FindArcPo(char *fname)
{
	int i = 0;
	while(1)
	{
		i++;
		ParsePattern(Structs->Arc[i]->patt, buf, 256);
		if(MatchPatternNoCase(buf, fname)) return(i);
		if(i==var.arcs) return(0);
	}
	return(0);
}

void LocalUpload(BOOL raw, char *recpath)
{
	struct rtFileRequester *filereq;
	struct rtFileList *flist, *tempflist;
	char fbuf[256];
	int l;

	if (filereq = rtAllocRequestA (RT_FILEREQ, NULL))
	{
		fbuf[0] = 0;
		if(!var.CON) Amiga.scr = 0;
		rtChangeReqAttr(filereq, RTFI_Dir, Structs->Cfg->ULPath, TAG_END);
		flist = rtFileRequest (filereq, fbuf, "Files to add...",
			RT_LeftOffset,	20,
			RT_TopOffset,	11,
			RT_Screen, 	Amiga.scr,
			RTFI_Height,	300,
			RTFI_OkText,	"Upload",
			RTFI_Flags,	FREQF_MULTISELECT,
			TAG_END);
		if(var.CON) FixTitle();
		if(flist)
		{
			if(!raw) PS("[0m\r\n");
			PS("\r\nDo what with the files? [M]ove, (C)opy, (A)bort: ");
			LineInput(10,0,0);
			PSLen("\r\n", 2);

			if(stricmp(charbuf, "C")==0) l=1; else l=0;
			if(stricmp(charbuf, "A")!=0 && var.CARRIER)
			{
				tempflist = flist;
				while(tempflist)
				{
					strcpy(fbuf, filereq->Dir);
					FixDir(fbuf);
					strcat(fbuf, tempflist->Name);

					if(l == 1)
						CopyFile(fbuf, recpath);
					else
						MoveFile(fbuf, recpath);

					if(!raw)
					{
						FileCheckDoor(tempflist->Name);
						AddFile(tempflist->Name, 0, 0);
					}
					tempflist = tempflist->Next;
				}
			}
			rtFreeFileList(flist);
		}
		rtFreeRequest(filereq);
	}
}

int EnterDesc(char * fname)
{
	int i = 0, j, k = 0;
	char tbuf[256];
	BOOL descfin = FALSE;
	BPTR fh;

	PS(Structs->SXStr[75]);
	while(!descfin)
	{
		i++;
enter:
		PSFmt(Structs->SXStr[76], i);
		LineInput(44, 0, 0);
		if(!var.CARRIER) return(-1);
		if(charbuf[0] == 0)
		{
			if(i == 1) { PS("[A"); goto enter; }
			descfin = TRUE;
			i--;
		} else {
			strcpy(desc[i], charbuf);
		}
	}
filehandle:
	PS(Structs->SXStr[77]);
	LineInput(20, 0, 0);
	PSLen("\r\n", 2);
	if(!var.CARRIER) return(-1);
	if(stricmp(charbuf, "D")==0)
	{
		DeleteFile(fname);
		return(-2);
	}
	if(stricmp(charbuf, "S")==0 || charbuf[0]==0)
	{
		PS(Structs->SXStr[78]);
		j = FindArcPo(fname);
		if(j!=0)
		{
			if(Structs->Arc[j]->packcmd[0]!=0)
			{
				fh = Open("FILE_ID.DIZ", MODE_NEWFILE);
				if(fh)
				{
					while(k < i)
					{
						k++;
						Write(fh, desc[k], strlen(desc[k]));
						Write(fh, "\n", 1);
					}
					Close(fh);
					mysprintf(tbuf, Structs->Arc[j]->packcmd, fname);
					if(Structs->Arc[j]->flags&(1<<1))
						FifoExecute(tbuf, 0);
					else {
						if(!strstr(tbuf,">NIL:")) strcat(tbuf," >NIL:");
						//system(tbuf);
						SystemTagList(tbuf, 0);
					}
				}
			}
		}
	} else {
		goto filehandle;
	}
	return(i);
}

void CheckResume(void)
{
	struct AnchorPath *Anch;
	char tbuf[128];
	BPTR lck;
	BOOL del = FALSE;

	mysprintf(buf, "%s%ld/SX_RESUME", Structs->Cfg->USERPath, Structs->User.Slot_Number);
	lck = Lock(buf, SHARED_LOCK);
	if(lck)
	{
		UnLock(lck);

		Anch = AllocMem(sizeof(struct AnchorPath), MEMF_PUBLIC|MEMF_CLEAR);
		if(Anch)
		{
			mysprintf(buf, "%s%ld/#?.BAD", Structs->Cfg->USERPath, Structs->User.Slot_Number);
			if(MatchFirst(buf, Anch) == 0)
			{
				while(1)
				{
					strmid(Anch->ap_Info.fib_FileName, tbuf, 1, strlen(Anch->ap_Info.fib_FileName)-4);
					PSFmt(Structs->SXStr[136], tbuf, Anch->ap_Info.fib_Size);
					switch(YesNo(1, TRUE))
					{
					case 1:
						PSLen("\r\n", 2);
						MatchEnd(Anch);
						//mysprintf(buf, "%s%ld/%s", Structs->Cfg->USERPath, Structs->User.Slot_Number, Anch->ap_Info.fib_FileName);
						//CopyFile(, buf);
						mysprintf(buf, "Copy %s%ld/%s %s%s", Structs->Cfg->USERPath, Structs->User.Slot_Number, Anch->ap_Info.fib_FileName, Structs->NodePrefs.fcheck, tbuf);
						//system(buf);
						SystemTagList(buf, 0);
						FreeMem(Anch, sizeof(struct AnchorPath));
						return;
					case 0:
						PS(Structs->SXStr[137]);
						switch(YesNo(1, TRUE))
						{
						case 1:
							mysprintf(tbuf, "%s%ld/%s", Structs->Cfg->USERPath, Structs->User.Slot_Number, Anch->ap_Info.fib_FileName);
							del = TRUE;
						}
						PSLen("\r\n", 2);
						goto mend;
					}

					if( MatchNext(Anch) != 0 ) break;
				}
mend:
				MatchEnd(Anch);
				if(del) DeleteFile(tbuf);
			} else {
				mysprintf(buf, "%s%ld/SX_RESUME", Structs->Cfg->USERPath, Structs->User.Slot_Number);
				DeleteFile(buf);
			}
			FreeMem(Anch, sizeof(struct AnchorPath));
		}
	}
}

void StoreResume(char *fname)
{
	register BPTR lck;

	mysprintf(buf, "%s%ld", Structs->Cfg->USERPath, Structs->User.Slot_Number);
	lck = Lock(buf, ACCESS_READ);
	if(!lck) lck = CreateDir(buf);
	if(lck) UnLock(lck);

	mysprintf(buf, "%s%ld/", Structs->Cfg->USERPath, Structs->User.Slot_Number);
	MoveFile(fname, buf);
	//mysprintf(buf, "c:move %s %s%ld/ >NIL:", fname, Structs->Cfg->USERPath, Structs->User.Slot_Number);
	//system(buf);

	mysprintf(charbuf, "%s%ld/%s", Structs->Cfg->USERPath, Structs->User.Slot_Number, fname);
	strcpy(buf, charbuf);
	strcat(buf, ".BAD");
	DeleteFile(buf);
	Rename(charbuf, buf);

	mysprintf(buf, "%s%ld/SX_RESUME", Structs->Cfg->USERPath, Structs->User.Slot_Number);
	lck = Open(buf, MODE_NEWFILE);
	if(lck) Close(lck);
}

void StoreFile(char *fname)
{
	register BPTR lck;

	mysprintf(buf, "%s%ld", Structs->Cfg->USERPath, Structs->User.Slot_Number);
	lck = Lock(buf, ACCESS_READ);
	if(!lck) lck = CreateDir(buf);
	if(lck) UnLock(lck);

	mysprintf(buf, "%s%ld/LCFiles", Structs->Cfg->USERPath, Structs->User.Slot_Number);
	lck = Lock(buf, ACCESS_READ);
	if(!lck) lck = CreateDir(buf);
	if(lck) UnLock(lck);

	mysprintf(buf, "%s%ld/LCFiles/", Structs->Cfg->USERPath, Structs->User.Slot_Number);
	MoveFile(fname, buf);
	//mysprintf(buf, "c:move %s %s%ld/LCFiles >NIL:", fname, Structs->Cfg->USERPath, Structs->User.Slot_Number);
	//system(buf);
}

void CheckLCFiles(void)
{
	struct AnchorPath *Anch;
	BOOL cdone = FALSE;
	ULONG size;
	char tbuf[128];
	BPTR old, lck;

	Anch = AllocMem(sizeof(struct AnchorPath), MEMF_PUBLIC|MEMF_CLEAR);
	if(!Anch) return;
	mysprintf(buf, "%s%ld/LCFiles/#?", Structs->Cfg->USERPath, Structs->User.Slot_Number);
	if( MatchFirst(buf, Anch) != 0 )
	{
		FreeMem(Anch, sizeof(struct AnchorPath));
		return;
	}
	PS(Structs->SXStr[111]);

	//chdir(Structs->NodePrefs.fcheck);
	lck = Lock(Structs->NodePrefs.fcheck, SHARED_LOCK);
	if(lck)
	{
		old = CurrentDir(lck);

		while(!cdone)
		{
			strcpy(tbuf, Anch->ap_Info.fib_FileName);
			size = Anch->ap_Info.fib_Size;

			if( MatchNext(Anch) != 0 ) cdone = TRUE;

			mysprintf(buf, "%s%ld/LCFiles/%s", Structs->Cfg->USERPath, Structs->User.Slot_Number, tbuf);
			MoveFile(buf, Structs->NodePrefs.fcheck);
			//mysprintf(buf, "c:move %s%ld/LCFiles/%s %s >NIL:", Structs->Cfg->USERPath, Structs->User.Slot_Number, tbuf, Structs->NodePrefs.fcheck);
			//system(buf);

			AddFile(tbuf, var.baud/10, size);
		}
		MatchEnd(Anch);
		FreeMem(Anch, sizeof(struct AnchorPath));
		SaveAccount();

		CurrentDir(old);
		UnLock(lck);
	}
}

void RawReceive(char *recpath, BOOL freezeclock)
{
	if(var.term)
	{
		if(!Carrier()) return;
		Amiga.Flags = AllocMem(sizeof(struct List), MEMF_PUBLIC|MEMF_CLEAR);
		NewList(Amiga.Flags);
		var.LOCAL = FALSE;
	}

	SetActive(2, 0);
	if(var.LOCAL)
		LocalUpload(TRUE, recpath);
	else
		RawTransfer(3, FALSE, freezeclock, recpath, 0);

	if(var.term) FreeMem(Amiga.Uploads, sizeof(struct List));
}

void AsciiSend(void)
{
	struct rtFileRequester *filereq;
	char fbuf[128];

	if (filereq = rtAllocRequestA (RT_FILEREQ, NULL))
	{
		fbuf[0] = 0;
		if(!var.CON) Amiga.scr=0;
		rtChangeReqAttr(filereq, RTFI_Dir, Structs->Cfg->ULPath, TAG_END);
		rtFileRequest (filereq, fbuf, "File to send...",
			RT_LeftOffset,	20,
			RT_TopOffset,	11,
			RT_Screen, 	Amiga.scr,
			RTFI_Height,	300,
			RTFI_OkText,	"Send",
			TAG_END);
		if(var.CON) FixTitle();
		strcpy(buf, filereq->Dir);
		rtFreeRequest(filereq);
		FixDir(buf);
		//if(buf[strlen(buf)-1]!=':' && buf[strlen(buf)-1]!='//') strcat(buf, "/");
		strcat(buf, fbuf);
		var.CARRIER = TRUE;
		ShowAnsi(buf, 0, 0, 0, 0, 0, 0);
		var.CARRIER = Carrier();
	}
}

void FileCheckDoor(char *fname)
{
	char door[128], *po;
	BPTR fh;

	mysprintf(buf, "%sFileCheck.DAT", Structs->Conf[var.thisconf]->path);
	fh = Open(buf, MODE_OLDFILE);
	if(fh)
	{
		BPTR lck;

		while(1)
		{
			if(!FGets(fh, buf, 128)) break;
			if(!FGets(fh, door, 128)) break;

			door[strlen(door)-1] = 0;
			po = strchr(door, '\r');
			if(po) po[0] = 0;
			po = strchr(door, '\n');
			if(po) po[0] = 0;

			lck = Lock(fname, SHARED_LOCK);
			if(lck)
			{
				UnLock(lck);
				RunDoor(door, atoi(buf), fname);
			}
		}
		Close(fh);
	}
}

BOOL CopyFile(char *fname, char *dest)
{
	BPTR infh, outfh;
	char *fbuf;
	ULONG bufsize;
	register long red;
	struct FileInfoBlock *fib = AllocDosObject(DOS_FIB, TAG_END);

 	if(fib)
	{
		infh = Open(fname, MODE_OLDFILE);
		if(infh)
		{
			ExamineFH(infh, fib);

			fbuf = AllocMem(fib->fib_Size, MEMF_PUBLIC);
			if(fbuf)
				bufsize = fib->fib_Size;
			else {
				fbuf = AllocMem(65536, MEMF_PUBLIC);
				bufsize = 65536;
				if(!fbuf)
				{
					fbuf = AllocMem(32768, MEMF_PUBLIC);
					bufsize = 32768;
				}
			}

			if(fbuf)
			{
				char tbuf[152];

				strcpy(tbuf, dest);
				FixDir(tbuf);
				strcat(tbuf, FilePart(fname));
				outfh = Open(tbuf, MODE_NEWFILE);
				if(outfh)
				{
					while(red = Read(infh, fbuf, bufsize))
					{
						Write(outfh, fbuf, red);
					}
					Close(outfh);
					FreeMem(fbuf, bufsize);
					Close(infh);
					FreeDosObject(DOS_FIB, fib);
					return(TRUE);
				}
				FreeMem(fbuf, bufsize);
			}

			Close(infh);
		}
		FreeDosObject(DOS_FIB, fib);
	}
	return(FALSE);
}

void MoveFile(char *fname, char *dest)
{
	char newname[128];

	strcpy(newname, dest);
	FixDir(newname);
	strcat(newname, FilePart(fname));

	if(!Rename(fname, newname))
	{
		if(CopyFile(fname, dest))
			DeleteFile(fname);
		else
			PS("[0;31m\r\nMove failed!\r\n");
	}
}

void FixDir(char *dir)
{
	register UWORD l = strlen(dir) - 1;
	if(dir[l]!=':' && dir[l]!=47) strcat(dir, "/");
}

/*void CleanDir(char *dir)
{
	char tbuf[128];
	BOOL cdone = FALSE;
	struct AnchorPath *Anch = AllocMem(sizeof(struct AnchorPath), MEMF_PUBLIC|MEMF_CLEAR);

	if(Anch)
	{
		if( MatchFirst(dir, Anch) == 0 )
		{
			while(!cdone)
			{
				strcpy(tbuf, Anch->ap_Info.fib_FileName);
				if( MatchNext(Anch) != 0 ) cdone = TRUE;
				DeleteFile(tbuf);
			}
			MatchEnd(Anch);
		}
		FreeMem(Anch, sizeof(struct AnchorPath));
	}
}*/
