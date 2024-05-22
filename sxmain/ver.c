#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <exec/execbase.h>
#include <proto/exec.h>
#include <graphics/gfxbase.h>
#include "sx.h"

extern struct ExecBase *SysBase;
extern struct Library *GfxBase;

extern UBYTE buf[512];
extern char ver[];


void Ver(void)
{
	UWORD flags, chipsetflag;
	char buf[256],*wb, *cpu, *copro, *chipset;
	struct GfxBase *Gfx = (struct GfxBase *)GfxBase;

	wb = "2.0";
	if(GfxBase->lib_Version > 38) wb = "3.0";
	if(GfxBase->lib_Version > 39) wb = "3.1";

	flags = SysBase->AttnFlags;
	cpu = "00";
	if(flags & AFF_68010) cpu = "10";
	if(flags & AFF_68020) cpu = "20";
	if(flags & AFF_68030) cpu = "30";
	if(flags & AFF_68040) cpu = "40";
	if(flags & AFF_68060) cpu = "60";

	copro = "None";
	if(flags & AFF_68881) copro = "68881";
	if(flags & AFF_68882) copro = "68882";
	if(flags & AFF_FPU40) copro = "040FPU";

	chipsetflag = Gfx->ChipRevBits0;
	chipset = "OCS";
	if((chipsetflag&GFXF_HR_AGNUS) || (chipsetflag&GFXF_HR_DENISE)) chipset = "ECS";
	if((chipsetflag&GFXF_AA_ALICE) || (chipsetflag&GFXF_AA_LISA) || (chipsetflag&GFXF_AA_MLISA)) chipset = "AGA";

	PS(ver);
	PS("\n\r\n\r[0mSystem Specifications:\n\r");
          sprintf(buf,"\n\r[32m Processor: [36m680%-12s", cpu);
     PS(buf);
     sprintf(buf,"[32m Copro/FPU: [36m%s",copro);
     PS(buf);
     sprintf(buf,"\n\r[32m Frequency: [36m%-15ld",SysBase->VBlankFrequency);
     PS(buf);
     sprintf(buf,"[32m Chipset  : [36m%s", chipset);
     PS(buf);
     sprintf(buf,"\n\r[32m Free Mem : [36m%-15ld", AvailMem(0));
     PS(buf);
     sprintf(buf,"[32m Workbench: [36m%s (%ld.%ld)", wb, GfxBase->lib_Version, SysBase->SoftVer);
     PS(buf);

     PS("\n\r");
     
}
