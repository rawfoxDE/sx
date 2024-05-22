
static const char __version[]="\0$VER: SX-CleanList 1.06 [eSSeXX] 68020 ("__DATE__")";

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <proto/exec.h>
#include <exec/memory.h>
#include <proto/dos.h>
#include <dos/dos.h>
#include <dos.h>

struct ConfStruct
{
	char name[45];
	char path[55];
	char pass[16];
	char filepath[52];
	UWORD fileareas;
	UWORD uploadarea;
	UBYTE flf;
	UBYTE flags;
	char reserve[82];
} Conf;

void mysprintf(char *Buffer,char *ctl, ...)
{
	RawDoFmt(ctl, (long *)(&ctl + 1), (void (*))"\x16\xc0\x4e\x75",Buffer);
}

char buf[256];
char *path[512];

UWORD highpath;


void CleanList(char *fname);
BOOL CheckFile(char *fname);


void main(int argc, char *argv[])
{
	UWORD i = 0;
	UWORD conf = atoi(argv[1]);
	UWORD dir = atoi(argv[2]);
	BPTR fh;
	BOOL readfin = FALSE;
	long x;
	char tbuf[256];

	if(argc < 2 || conf < 1 || dir < 1)
	{
		PutStr(	"SX_CleanList <Conf> <Dir>\n\n"
			"›32meg: ›mSX_CleanList 7 1   ›32m<-- would scan conference 7, filedir 1.›m\n\n");
		exit(0);
	}

	fh = Open("SX:Prefs/Confs.DAT", MODE_OLDFILE);
	if(fh)
	{
		while(!readfin)
		{
			i++;
			x = Read(fh, &Conf, sizeof(struct ConfStruct));
			if(x)
			{
				if(i == conf) readfin=TRUE;
			} else {
				readfin = TRUE;
			}
		}
		Close(fh);
		if(!x)
		{
			PutStr("Cannot find that conference.\n");
			exit(10);
		}
		mysprintf(buf, "›m\nScanning Conference ›32m%ld:%s›m directory ›32m%ld›m\n\n", conf, Conf.name, dir);
		PutStr(buf);

		mysprintf(buf, "%sPaths", Conf.path);
		fh = Open(buf, MODE_OLDFILE);
		i=0;
		if(fh)
		{

			while(FGets(fh, buf, 128))
			{
				i++;
				path[i] = AllocMem(128, MEMF_PUBLIC);
				x = strlen(buf);
				buf[x-1] = 0;
				strcpy(path[i], buf);
			}
			Close(fh);
		}
		highpath=i;

		mysprintf(tbuf, "%sDir%ld", Conf.path, dir);
		CleanList(tbuf);

		i=0;
		while(i < highpath)
		{
			i++;
			FreeMem(path[i], 128);
		}
	}	
}

void CleanList(char *fname)
{
	char listbuf[130];
	FILE *fp = fopen(fname, "r");
	BPTR outfh;
	UWORD i;

	if(fp)
	{
		outfh = Open("T:SXCLEAN.TMP", MODE_NEWFILE);
		if(outfh)
		{
			while(fgets(listbuf, 128, fp))
			{
				if(strncmp(listbuf, "      ", 6))
				{
jump:
					i=0;
					while(i <= strlen(listbuf))
					{
						if(listbuf[i]==' ')
						{
							buf[i]=0;
							i=130;
						} else buf[i]=listbuf[i];
						i++;
					}
					if(CheckFile(buf))
					{
						Write(outfh, listbuf, strlen(listbuf));
					} else {
						mysprintf(listbuf, "Removing: ›32m%s›m\n", buf);
						PutStr(listbuf);
						while(fgets(listbuf, 128, fp))
						{
							if(strncmp(listbuf, "      ", 6)) goto jump;
						}
					}
				} else {
					Write(outfh, listbuf, strlen(listbuf));
				}
			}
			Close(outfh);
			fclose(fp);
			mysprintf(buf, "Copy T:SXCLEAN.TMP %s QUIET", fname);
			Execute(buf, 0, 0);
			DeleteFile("T:SXCLEAN.TMP");
		}
	}
}

BOOL CheckFile(char *fname)
{
	UWORD i = 0;
	char tbuf[256];
	BPTR lck;

	if(highpath == 0)
	{
		mysprintf(tbuf, "%s%s", Conf.filepath, fname);
		lck = Lock(tbuf, ACCESS_READ);
		if(!lck) return(0);
		UnLock(lck);
		return(1);
	}

	while(i < highpath)
	{
		i++;
		mysprintf(tbuf, "%s%s", path[i], fname);
		lck = Lock(tbuf, ACCESS_READ);
		if(lck)
		{
			UnLock(lck);
			return(1);
		}
	}

	return(0);
}
