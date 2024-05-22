
#include <stdlib.h>
#include <string.h>
#include <exec/types.h>
#include <exec/memory.h>
#include <clib/alib_protos.h>
#include <clib/exec_protos.h>
#include <proto/exec.h>
#include <proto/dos.h>
#define SX_VAR
#include "SXStructs.h"

static const char __version[]="\0$VER: SX_BGCheck 1.06 [eSSeXX] 68020 ("__DATE__")";

struct ArchiveStruct arc;

char buf[256];

struct BGStruct
{
	struct ConfStruct	*Conf;
	struct UserData		*User;
	struct SXUserStruct	*SXUser;
	long 			baud;
	UWORD			thisfilearea;
	UWORD			thisconf;
	UWORD			node;
} *BG;


unsigned char * mydatestr(void)
{
	struct DateTime stamp;
	DateStamp(&stamp);
	stamp.dat_Format  = FORMAT_USA;
	stamp.dat_StrDay  = "2222222222";
	stamp.dat_StrDate = "000000000";
	stamp.dat_StrTime = "111111111";
	stamp.dat_Flags   = 0;
	DateToStr(&stamp);
	return(stamp.dat_StrDate);
}

void mysprintf(char *Buffer,char *ctl, ...)
{
	RawDoFmt(ctl, (long *)(&ctl + 1), (void (*))"\x16\xc0\x4e\x75",Buffer);
}

long mytime(void)
{
	struct DateStamp stamp;
	DateStamp(&stamp);
	return( (stamp.ds_Days*24*60*60) + (stamp.ds_Minute*60) + (stamp.ds_Tick/50) + 252460800 );
}

void FixDir(char *dir)
{
	register UWORD l = strlen(dir) - 1;
	if(dir[l]!=':' && dir[l]!=47) strcat(dir, "/");
}

BOOL CopyFile(char *fname, char *dest)
{
	BPTR infh, outfh;
	char *fbuf;
	ULONG bufsize;
	register long red;
	struct FileInfoBlock *fib = AllocDosObject(DOS_FIB, 0);

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
		if(CopyFile(fname, dest)) DeleteFile(fname);
	}
}

void GetPath(char *outbuf, UWORD area)
{
	char tbuf[256];
	UWORD j = 0;
	BPTR fh;

	mysprintf(tbuf, "%sPaths", BG->Conf->path);
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
	strcpy(outbuf, BG->Conf->filepath);
}

BOOL FindArcPo(UWORD node, char *fname)
{
	BPTR fh;

	mysprintf(buf, "SX:Node%ld/Prefs/Archives.DAT", node);
	fh = Open(buf, MODE_OLDFILE);
	if(fh)
	{
		while(1)
		{
			if(Read(fh, &arc, sizeof(struct ArchiveStruct)))
			{
				ParsePattern(arc.patt, buf, 256);
				if(MatchPatternNoCase(buf, fname))
				{
					Close(fh);
					return(TRUE);
				}
			} else {
				Close(fh);
				return(FALSE);
			}
		}
	}
}

void CallerLog(char *logstr, UWORD node)
{
	BPTR fh;
	char fname[32];

	mysprintf(fname, "SX:Node%ld/CallersLog", node);
	fh = Open(fname, MODE_READWRITE);
	if(fh)
	{
		Seek(fh, 0, OFFSET_END);
		Write(fh, logstr, strlen(logstr));
 		Close(fh);
	}
}

void main(int argc, char *argv[])
{
	struct MsgPort *port = 0;
	struct FileInfoBlock *fib;
	char tbuf[256], charbuf[128], filename[32], *desc[22];
	UWORD usingarea, a, thisfilearea, i, node;
	BPTR lck, fh, nlock, olock;
	long size, cps;
	char *po;

	if(argc<3 || !argv[1] || !argv[2] || !argv[3])
	{
		PutStr("BGCheck <BGStruct> <filename> <cps>\n");
		return;
	}

	BG		= (struct BGStruct *)atol(argv[1]);
	strcpy(filename, argv[2]);
	cps		= atol(argv[3]);

	node		= BG->node;
	thisfilearea	= BG->thisfilearea;

	mysprintf(tbuf, "T:%ld", FindTask(0L));
	lck = Lock(tbuf, ACCESS_READ);
	if(!lck) lck = CreateDir(tbuf);
	if(lck) UnLock(lck);

	nlock = Lock(tbuf, ACCESS_READ);
	if(nlock)
	{
		olock = CurrentDir(nlock);
		//chdir(tbuf);

		fib = AllocMem(sizeof(struct FileInfoBlock), 0);
		for(a=0;a<21;a++) desc[a] = AllocMem(46, MEMF_PUBLIC|MEMF_CLEAR);

		if(lck = Lock(filename, SHARED_LOCK))
		{
			Examine(lck, fib);
			size = fib->fib_Size;
	 		UnLock(lck);

			i = 0;
			if(FindPort("SXBGCheck"))
			{
				while(FindPort("SXBGCheck"))
				{
					if(i == 150) break;
					i++;
					Delay(20);
				}
			}
			port = CreatePort("SXBGCheck", 0L);

			DeleteFile("FILE_ID.DIZ");
			if(FindArcPo(node, FilePart(filename)))
			{
				if(arc.testcmd[0]!=0)
				{
					mysprintf(tbuf, arc.testcmd, filename);
					if(!strstr(tbuf,">NIL:")) strcat(tbuf," >NIL:");
					system(tbuf);
				}
				if(arc.unpackcmd[0]!=0)
				{
					mysprintf(buf, arc.unpackcmd, filename);
					if(!strstr(buf, ">NIL:")) strcat(buf, " >NIL:");
					system(buf);
					if(fh = Open("FILE_ID.DIZ", MODE_OLDFILE))
					{
						i = 0;
						while(FGets(fh, buf, 510) && i<20)
						{
							i++;
							buf[strlen(buf)-1] = 0;
							if(strlen(buf) > 44)
							{
								strmid(buf, charbuf, 1, 44);
								strcpy(buf, charbuf);
							}
							po = strchr(buf, '\r');
							if(po) po[0] = 0;
							po = strchr(buf, '\n');
							if(po) po[0] = 0;
							strcpy(desc[i], buf);
						}
						Close(fh);
						DeleteFile("FILE_ID.DIZ");
					} else {
						goto endo;
					}
				} else {
					goto endo;
				}
			} else {
				goto endo;
			}

			/* === save to filelist === */

			if(BG->Conf->uploadarea == 0)
			{
				if(thisfilearea == 0)
					usingarea = BG->Conf->fileareas;
				else
					usingarea = thisfilearea;
			} else {
				usingarea = BG->Conf->uploadarea;
			}
			mysprintf(buf, "%sdir%ld", BG->Conf->path, usingarea);
			if(fh = Open(buf, MODE_READWRITE))
			{
				Seek(fh, 0, OFFSET_END);
				mysprintf(buf, "%-12s P%7ld  %s  %s\n", FilePart(filename), size, mydatestr(), desc[1]);
				Write(fh, buf, strlen(buf));
				a = 1;
				while(a != i)
				{
					a++;
					if(desc[a][0]!=0)
					{
						mysprintf(buf, "                                 %s\n", desc[a]);
						Write(fh, buf, strlen(buf));
					}
				}
/* sentbyline */		if((BG->Conf->flags&(1<<0)) && BG->SXUser->sentbyline[0]!=0)
				{
					mysprintf(buf, "                                 %s\n", BG->SXUser->sentbyline);
					Write(fh, buf, strlen(buf));
				}
				Close(fh);
			}

			/* === add to credits and log === */

			BG->User->Uploads++;
			BG->User->Bytes_Upload += size;
			mysprintf(buf, "	Uploading %-12s%7ld bytes\n	 1 file(s), %ldk bytes, 1 minute(s). 10 second(s), %ld cps, %ld %% efficiency.\n", FilePart(filename), size, size/1024, cps, cps*100/(BG->baud/10));
			CallerLog(buf, node);

			mysprintf(buf, "SX:Node%ld/BG.report", node);
			fh = Open(buf, MODE_READWRITE);
			if(fh)
			{
				Seek(fh, 0, OFFSET_END);
				mysprintf(buf, "%-20s %ld bytes\r\n", FilePart(filename), size);
				Write(fh, buf, strlen(buf));
				Close(fh);
			}

			/* === move the file to correct directory === */

			GetPath(tbuf, usingarea);
			MoveFile(filename, tbuf);
			//mysprintf(buf, "c:move %s %s", filename, tbuf);
			//system(buf);
		}

		CurrentDir(olock);
		UnLock(nlock);
	}
endo:
	if(port) DeletePort(port);

	FreeMem(fib, sizeof(struct FileInfoBlock));
	for(a=0;a<21;a++) FreeMem(desc[a], 46);

	mysprintf(tbuf, "T:%ld", FindTask(0L));
	DeleteFile(tbuf);
}
