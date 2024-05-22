
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <exec/memory.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <devices/serial.h>
#include "SXStructs.h"
#include "sx.h"

extern struct StructsStruct	*Structs;
extern struct AmigaStruct	Amiga;
extern struct StringsStruct	*stg;
extern struct VariablesStruct	var;

extern UBYTE buf[512], charbuf[128];

void MCIPS(char * ansbuf, long size, BOOL pause, UWORD startline);
int MCIFunction(char * mci, char * tempbuf);
void LoadStrings(void);
void FreeStrings(void);
ULONG StripAnsi(char * ans, ULONG len);
void myctime(long secs, char *outbuf);
int ShowAnsiRAW (char *f, BOOL path, BOOL lang, BOOL axs, BOOL pause, UWORD startline);


int ShowAnsi (char *f, BOOL path, BOOL lang, BOOL axs, BOOL conf, BOOL pause, UWORD startline)
{
	if(conf && path)
	{
		char tbuf[256];

		strcpy(tbuf, Structs->Conf[var.thisconf]->path);
		strcat(tbuf, f);

		if(ShowAnsiRAW(tbuf, FALSE, lang, axs, pause, startline))
			return(1);
		else
			return(ShowAnsiRAW(f, path, lang, axs, pause, startline));
	} else
		return(ShowAnsiRAW(f, path, lang, axs, pause, startline));
}

int ShowAnsiRAW (char *f, BOOL path, BOOL lang, BOOL axs, BOOL pause, UWORD startline)
{
	BPTR fh;
	char *ansbuf, fname[256], tbuf[8];
	ULONG size, t;

	if(!var.CARRIER) return(0);
goagain:
	if(path) strcpy(fname, Structs->Disp.path); else fname[0]=0;
	strcat(fname, f);
	if(lang)
	{
		strcat(fname, ".");
		if(axs)
		{
			mysprintf(tbuf, "%ld.", Structs->User.Sec_Status);
			strcat(fname, tbuf);
		}
		strcat(fname, Structs->Disp.ext);
	}

	fh = Open(fname, MODE_OLDFILE);
	if(fh)
	{
		Seek(fh, 0, OFFSET_END);
		size = Seek(fh, 0, OFFSET_BEGINNING);
		ansbuf = AllocMem(size+2, MEMF_PUBLIC|MEMF_CLEAR);
		if(ansbuf)
		{
			Read(fh,ansbuf,size);
			Close(fh);
			if(Structs->Disp.flags&(1<<0)) t=StripAnsi(ansbuf, size); else t=size;
			MCIPS(ansbuf, t, pause, startline);
			FreeMem(ansbuf,size+2);
		}
		return(1);
	} else {
		if(axs) { axs=FALSE; goto goagain; }
		return(0);
	}
}

void MCIPS(char * ansbuf, long size, BOOL pause, UWORD startline)
{
	BOOL mcidone;
	char *ansbuf2, *insert, numb[16], mci[128], tbuf[128];
	ULONG i=0, k=0;
	UWORD nc, j, lc, t;
	long x;

	lc = startline;

	ansbuf2 = AllocMem(size + 2048, MEMF_PUBLIC|MEMF_CLEAR);
	if(!ansbuf2) return;

	if(Structs->User.LineLength == 0)
		pause = FALSE;
	else {
		if(Structs->User.LineLength < 10) Structs->User.LineLength = 10;
	}

	while(i <= size)
	{
startloop:
		if(lc >= (Structs->User.LineLength-1)  &&  pause  &&  Structs->User.LineLength)
		{
			lc = 1;
			ansbuf2[k] = 0;
			PS(ansbuf2);
			k = 0;
			PS(Structs->SXStr[131]);
pause:
			MyHotKey();
			switch(charbuf[0])
			{
			case 'n':
			case 'N':
				PS(Structs->SXStr[10]);
				FreeMem(ansbuf2, size + 2048);
				PS("[A\r\n[K");
				return;
			case 'c':
			case 'C':
				PS("Continueous[A\r\n[K");
				pause = FALSE;
				break;
			case 13:
			case 'y':
			case 'Y':
				PS(Structs->SXStr[9]);
				PS("[A\r\n[K");
				break;
			default:
				goto pause;
			}
			goto startloop;
		}
		if(ansbuf[i]==27)
		{
			t = i;
			i += 2;
			while(i <= size  &&  ansbuf[i]>46  &&  ansbuf[i]<60) i++;
			switch(ansbuf[i])
			{
			case 'A':
			case 'B':
			case 'H':
				pause = FALSE;
				break;
			}
			i = t;
		}
		if(ansbuf[i]==13 && ansbuf[i+1]!=10)
		{
			ansbuf2[k]=13;
			k++;
			ansbuf2[k]=10;
			k++;
			i++;
			lc++;
			goto startloop;
		}
		if(ansbuf[i]==10 && ansbuf[i-1]!=13)
		{
			ansbuf2[k]=13;
			k++;
			lc++;
		}
		if(ansbuf[i]=='~')
		{
			mcidone=FALSE;
			j=0;
			nc=0;
			numb[0]=0;
			while(i < (size+1) && !mcidone)
			{
				i++;
				if(ansbuf[i]==' ' || ansbuf[i]=='|') mcidone=TRUE;
				else
				{
					if(ansbuf[i]>47 && ansbuf[i]<58 && j<4)
					{
						numb[nc]=ansbuf[i];
						nc++;
						numb[nc]=0;
					} else {
						mci[j]=ansbuf[i];
						j++;
						mci[j]=0;
					}
				}
			}
			//ansbuf2[k]=0;
			//PS(ansbuf2);
			insert=0;
			switch(mci[0])
			{
			case 'A': mysprintf(tbuf,"%ld",Structs->User.Sec_Status); insert=(char *)&tbuf; break;
			case 'B':
				switch(mci[1])
				{
				case 'A':
					if(Structs->SXUser.byteratio==0)
						insert="Disabled";
					else {
						mysprintf(tbuf,"%ld",Structs->SXUser.byteratio);
						insert=(char *)&tbuf;
					}
					break;
				case 'G':
					if(Structs->User.Flags&(1<<0))
						insert = "Yes";
					else
						insert = "No";
					break;
				case 'R': mysprintf(tbuf,"%ld",var.baud); insert=(char *)&tbuf; break;
				}
				break;
			case 'C':
				switch(mci[1])
				{
				case 'A': ConfAccess(&Structs->SXUser, tbuf, 16); insert=(char *)&tbuf; break;
				case 'N': insert=Structs->Conf[var.thisconf]->name; break;
				case 'M': mysprintf(tbuf,"%ld",var.thisconf); insert=(char *)&tbuf; break;
				case 'T': insert=(char *)mytimestr(TRUE); break;
				}
				break;
			case 'D':
				switch(mci[1])
				{
				case 'B': mysprintf(tbuf,"%ld",Structs->User.Bytes_Download); insert=(char *)&tbuf; break;
				case 'T': insert=(char *)mydatestr(); break;
				}
				break;
			case 'E':
				switch(mci[1])
				{
				case 'F':
					if(Structs->User.Flags&(1<<5))
						insert = "Line";
					else
						insert = "Screen";
					break;
				}
				break;
			case 'F':
				switch(mci[1])
				{
				case 'D': mysprintf(tbuf,"%ld",Structs->User.Downloads); insert=(char *)&tbuf; break;
				case 'F':
					if(Structs->User.Flags&(1<<4))
					{
						insert = "Ask";
					} else {
						if(Structs->User.Flags&(1<<3)) insert = "Yes"; else insert = "No ";
					}
					break;
				case 'R':
					if(Structs->SXUser.fileratio==0) strcpy(tbuf, "Disabled"); else mysprintf(tbuf,"%ld",Structs->SXUser.fileratio);
					insert=(char *)&tbuf;
					break;
				case 'U': mysprintf(tbuf,"%ld",Structs->User.Uploads); insert=(char *)&tbuf; break;
				}
				break;
			case 'H':
				switch(mci[1])
				{
				case 'W': insert=(char *)&Structs->SXUser.computer; break;
				}
				break;
			case 'L':
				switch(mci[1])
				{
				case 'B':
					x=BytesLeft();
					if(x==20000001) strcpy(tbuf, "Unlimited"); else mysprintf(tbuf,"%ld",x);
					insert=(char *)&tbuf;
					break;
				case 'C':myctime(Structs->User.Time_Last_On, tbuf); insert=(char *)&tbuf; break;
				case 'F':
					x=FilesLeft();
					if(x==20000001) strcpy(tbuf, "Unlimited"); else mysprintf(tbuf,"%ld",x);
					insert=(char *)&tbuf;
					break;
				case 'K':myctime(var.loginsecs, tbuf); insert=(char *)&tbuf; break;
				case 'G':
					mysprintf(tbuf, "%ld", var.node);
					insert = tbuf;
					break;
				}
				break;
			case 'M':
				switch(mci[1])
				{
				case 'B': insert = (char *)var.msgareapo; break;
				case 'F':
					if(Structs->User.Flags&(1<<2))
						insert = "Ask";
					else {
						if(Structs->User.Flags&(1<<1)) insert = "Yes"; else insert = "No ";
					}
					break;
				default:
					mysprintf(tbuf, "%ld", Structs->User.Messages_Posted);
					insert = tbuf;
				}
				break;
			case 'N':
				switch(mci[1])
				{
				case 'P': pause = FALSE; break;
				default: insert=(char *)&Structs->User.Name; break;
				}
				break;
			case 'P':
				switch(mci[1])
				{
				case 'N': insert=(char *)&Structs->Proto.name; break;
				case 'R': tbuf[0]=Structs->User.Protocol; tbuf[1]=0; insert=(char *)&tbuf; break;
				default:
					insert="ENCRYPTED"; break;
				}
				break;
			case 'R':
				switch(mci[1])
				{
				case 'N': insert=(char *)&Structs->UserIndex.realname; break;
				}
				break;
			case 'S':
				switch(mci[1])
				{
				case 0: mysprintf(tbuf, "%ld", Structs->User.Slot_Number); insert=(char *)&tbuf; break;
				case 'F':
					mysprintf(tbuf, "%ld", SpaceFree());
					insert = (char *)&tbuf;
					break;
				case 'P':
					ansbuf2[k] = 0;
					PS(ansbuf2);
					lc = 0;
					k = 0;
					HitReturn();
					break;
				case 'S':
					strmid(mci, tbuf, 4, strlen(mci)-3);
					ansbuf2[k] = 0;
					PS(ansbuf2);
					ShowAnsi(tbuf, 0, 0, 0, 0, 0, 0);
					lc = 0;
					k = 0;
					break;
				case 'R':
					mysprintf(tbuf, "%s%ld", &mci[3], rnd(atoi(numb))+1);
					ansbuf2[k] = 0;
					PS(ansbuf2);
					ShowAnsi(tbuf, 0, 1, 0, 0, 0, 0);
					lc = 0;
					k = 0;
					break;
				case 'L':
					mysprintf(tbuf,"%ld",Structs->User.LineLength); insert=(char *)&tbuf; break;
					break;
				case 'E':
					switch(mci[2])
					{
					case 'R':
						if(var.SER) insert=(char *)&("Active");
						else insert=(char *)&("Quiet");
						break;
					}
					break;
				case 'X':
					ansbuf2[k] = 0;
					PS(ansbuf2);
					if(MCIFunction(mci, tbuf)==0)
					{
						FreeMem(ansbuf2, size + 2048);
						return;
					}
					k = 0;
					break;
				}
				break;
			case 'T':
				switch(mci[1])
				{
				case 'C': mysprintf(tbuf,"%ld",Structs->User.Times_Called); insert=(char *)&tbuf; break;
				case 'R': mysprintf(tbuf,"%ld",(Structs->User.Time_Left/60)+1); insert=(char *)&tbuf; break;
				}
				break;
			case 'U':
				switch(mci[1])
				{
				case 'L': insert=(char *)&Structs->User.Location; break;
				case 'B': mysprintf(tbuf,"%ld",Structs->User.Bytes_Upload); insert=(char *)&tbuf; break;
				case 'S': insert=(char *)&Structs->SXUser.sentbyline; break;
				}
				break;
			case 'V':
				switch(mci[1])
				{
				case 'V': insert = "1.06"; break;
				}
				break;
			case '~': insert="~"; break;
			case '#': insert=(char *)&Structs->User.PhoneNumber; break;
			}
			if(insert)
			{
				if(numb[0])
				{
					x = atoi(numb);
					strmid(insert, tbuf, 1, x);
					strcpy(&ansbuf2[k], tbuf);
					k += x;
				} else {
					strcpy(&ansbuf2[k], insert);
					k += strlen(insert);
				}
			}
		} else {
			ansbuf2[k] = ansbuf[i];
			k++;
		}
		i++;
	}
	PS(ansbuf2);
	FreeMem(ansbuf2, size + 2048);
}

int MCIFunction(char * mci, char * tempbuf)
{
	int sx=2, sxf=0, bufco=0, l=strlen(mci), func, extra;
	char string[48];

	if(Carrier()==FALSE) return(0);

	while(sx < l)
	{
		sx++;
		if(mci[sx]=='\\')
		{
			sxf++;
			switch(sxf)
			{
			case 1: strcpy(string,tempbuf); break;
			case 2: func=atoi(tempbuf); break;
			case 3: extra=atol(tempbuf); break;
			}
			bufco=0;
		} else {
			tempbuf[bufco]=mci[sx];
			bufco++;
			tempbuf[bufco]=0;
		}
	}
	DoFunction(func, string, extra, 0, 255, 0, 0);
	return(1);
}

void LoadStrings(void)
{
	BPTR fh;
	ULONG size, i=0, j=0;
	char *tempbuf;

	var.SXStrCount=1;

	mysprintf(buf, "SX:TXT/Strings.%s", Structs->Disp.ext);
	if(fh=Open(buf, MODE_OLDFILE))
	{
		Seek(fh, 0, OFFSET_END);
		size = Seek(fh, 0, OFFSET_BEGINNING);
		tempbuf = AllocMem(size+4, MEMF_PUBLIC|MEMF_CLEAR);
		if(!tempbuf) { Close(fh); return; }
		Read(fh, tempbuf, size);
		Close(fh);

		if(strstr(tempbuf, "%u") || strstr(tempbuf, "%d"))
		{
			PS(	"[0;31m\r\nWarning: [0mYou have illegal control codes in your STRINGS file."
				"\r\n         This may cause System-X to crash at sometime.\r\n");
			HitReturn();
		}

		if(Structs->Disp.flags&(1<<1)) var.strbufsize=StripAnsi(tempbuf, size)*2; else var.strbufsize=size*2;

		stg->strbuf = AllocMem(var.strbufsize, MEMF_PUBLIC|MEMF_CLEAR);
		if(!stg->strbuf) { FreeMem(tempbuf, size+4); return; }

		while(i <= size)
		{
			stg->strbuf[j] = tempbuf[i];
			if(stg->strbuf[j]==10 && tempbuf[i+1]!='#' && tempbuf[i-1]!='#' && stg->strbuf[j-1]!=13)
			{
				stg->strbuf[j]=13;
				j++;
				stg->strbuf[j]=10;
			}
			i++;
			j++;
		}
		FreeMem(tempbuf, size+4);

		i=0;
		Structs->SXStr[1] = (char *)&stg->strbuf[0];
		while(i <= j)
		{
			if(stg->strbuf[i]==10)
			{
				if(stg->strbuf[i+1]=='#' && stg->strbuf[i+2]=='*' && stg->strbuf[i+3]=='#')
				{
					stg->strbuf[i]=0;
					i=i+5;
					var.SXStrCount++;
					Structs->SXStr[var.SXStrCount] = (char *)&stg->strbuf[i];
				}
			}
			i++;
		}
	}
}

void FreeStrings(void)
{
	if(stg->strbuf)
	{
		FreeMem(stg->strbuf, var.strbufsize);
		stg->strbuf = 0;
	}
}

ULONG StripAnsi(char * ans, ULONG len)
{
	ULONG i=0, j=0, t;
	char *sbuf;

	sbuf = AllocMem(len+2, MEMF_PUBLIC|MEMF_CLEAR);
	if(!sbuf) return(len);

	while(i <= len)
	{
		if(ans[i] == 27)
		{
			t = i;
			i += 2;
			while(i<len && ans[i]>46 && ans[i]<60) i++;
			if(ans[i]!='m') { i=t; goto add; } else i++;
		} else {
add:
			sbuf[j]=ans[i];
			i++;
			j++;
		}
	}
	strcpy(ans, sbuf);
	FreeMem(sbuf, len+2);
	return(j);
}

void myctime(long secs, char *outbuf)
{
	struct DateStamp *tp;
	struct DateTime tostr;

	if(secs==0) { strcpy(outbuf, "none"); return; }

	tp = __timecvt((time_t)secs);

	memcpy(&tostr, tp, 12);

	tostr.dat_Format  = FORMAT_USA;
	tostr.dat_StrDay  = "2222222222";
	tostr.dat_StrDate = "000000000";
	tostr.dat_StrTime = "111111111";
	tostr.dat_Flags   = 0;

	DateToStr(&tostr);
	mysprintf(outbuf, "%s %s", tostr.dat_StrDate, tostr.dat_StrTime);
}
