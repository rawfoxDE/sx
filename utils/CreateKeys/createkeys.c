static char __version[] ="\0$VER: SX_CreateKeys 1.06 [eSSeXX] 68020 ("__DATE__")";

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <dos/dos.h>
#include <devices/serial.h>
#include "sxstructs.h"

struct UserKeys 
{                 /* changes to UserKeys (RTS) */
 char    UserName[31];
 long    Number;
 UBYTE   New_User;
 UWORD   UPcps;              /* highest upload cps rate */
 UWORD   DNcps;              /* highest dnload cps rate */
 UWORD   Userflags;          /*                         */
 UWORD   baud_rate;          /* last online baud rate   */
 char    Pad[9];             /* ?? should be 15         */
};

void main()
{
	struct UserData User;
	struct UserKeys Keys;

	BPTR fh = Open("BBS:User.DATA", MODE_OLDFILE);

	if(fh)
	{
		long i = 0;
		BPTR ofh = Open("BBS:User.KEYS", MODE_NEWFILE);

		if(ofh)
		{
			PutStr("Creating Keys...\n");

			Keys.UPcps = 1600;
			Keys.DNcps = 1600;
			Keys.baud_rate = 19200;

			while(1)
			{
				i++;

				if(Read(fh, &User, sizeof(struct UserData)))
				{
					strcpy(Keys.UserName, User.Name);
					Keys.Number = i;
					Write(ofh, &Keys, sizeof(struct UserKeys));
				} else
					break;
			}
			Close(ofh);
		}
		Close(fh);
	}
}
