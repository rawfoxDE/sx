
struct MsgBaseStruct			// <confpath>/MsgBases.DAT
{
	char name[24];
	UWORD maxmsgs;
	UBYTE lowaccess;
	UBYTE highaccess;
	UWORD flags;
	char res[66],
	     fidoname[28],
	     fidoorigin[58];
	UWORD AddressID;
	char res2[8];
};


	struct MsgStruct		// baseX/messages.DAT
	{
		char to[32];
		char from[32];
		char subj[72];
		char pass[16];
		char file[32];
		UWORD number;
		UWORD replypo;
		UWORD replycount;
		UWORD thisreply;
		LONG msgtime;
		LONG rectime;
		ULONG flags;	/* BIT 3 = DELETE   BIT 4 = PRIVATE */
		UWORD fidonet_packet_origin_zone;
		UWORD fidonet_packet_origin_netid;
		UWORD fidonet_packet_origin_node;
		UWORD fidonet_packet_origin_point;
		UWORD fidonet_origin_zone;
		UWORD fidonet_origin_netid;
		UWORD fidonet_origin_node;
		UWORD fidonet_origin_point;
		LONG  fidonet_msgid;
		UWORD fidonet_dest_zone;
		UWORD fidonet_dest_netid;
		UWORD fidonet_dest_node;
		UWORD fidonet_dest_point;
		char res[32];
	};
