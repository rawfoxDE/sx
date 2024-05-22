#ifndef GUI_FILE_H
#define GUI_FILE_H

/* Types */
#include <exec/types.h>
struct ObjApp
{
	APTR	App;
	APTR	WI_label_0;
	APTR	BT_N;
	APTR	BT_Name;
	APTR	BT_Location;
	APTR	BT_Action;
	APTR	BT_I;
	char *	CY_SysopContent[3];
	char *	CY_label_0Content[7];
};


extern struct ObjApp * CreateApp(void);
extern void DisposeApp(struct ObjApp *);

#endif
