
/* FAME Door Commands
 *
 * $VER:FAMEDoorCommands.h v1.2
 *
 * Used tab size: 2
 */

#ifndef FAME_FAMEDOORCOMMANDS_H
#define FAME_FAMEDOORCOMMANDS_H

/*
 *                               GENERAL INFO's
 *                               --------------
 *
 * -General use of Commands:
 *  A Door normaly has to give values (numbers like confnumbers/usernumbers/
 *  flags etc..) into Data1 of the struct FAMEDoorMsg to the DoorPort.
 *  If more values are needed the next Data will be used: Data2 and then
 *  Data3. Values requested from a Door normaly can be found in Data2.
 *  Only if a value needs a ULONG Data3 must be used.
 *
 * -The Door have to check the ReturnCode of struct FAMEDoorMsg!!!
 *  See FAMEGlobalBBSStructs.h for defination of the ReturnCode.
 *
 * -Note that all structure elements of the struct FAMEDoorMsg are beginning
 *  with fdom_ !
 *
 *  If you see the elements below like here:
 *
 *  // Data1
 *  // Data2
 *  // Data3
 *  // IOString
 *
 *  They have to be used like this:
 *
 *  MyFAMEDoorMsg -> fdom_Data1
 *  MyFAMEDoorMsg -> fdom_Data2
 *  MyFAMEDoorMsg -> fdom_Data3
 *  MyFAMEDoorMsg -> fdom_IOString
 *
 *  The same for all other structure elements!
 */


/* FIM FAME Interface Module */


/* FAME DOORCOMMANDS */

/*
 kEYwORD | mEANS | dESCRIPTION
-----------------------------------
		UNUSED  <->  Not used yet!
				MC  <->  Must Command! Every Door must call this Command once!
				NR  <->  Normal Command! Most String operations (Retrieve/Send datas)
				NC  <->  Normal Command! Most String operations (Change datas)
				CF  <->  Call function! BBS Functions like Editor,Comp.Modem.NumLine
				SR  <->  System Commands! (Retrieve/Send datas)
				SC  <->  System Commands! (Change datas)
				AR  <->  Additional Command! (Retrieve/Send datas)
				AC  <->  Additional Command! (Change datas)
				RD  <->  Reserved! Do not us it !!!!!!!!!

define arrays:

MC      1 -     9

NR     10 -   199
NC    200 -   399
CF    400 -   599
SR    600 -   699
SC    700 -   799
AR    800 -   899
AC    900 -   999
RD - 1000 -  9999       - means not secret but reserved.
RD +10000 - 99999       + means secret and can't be used on the normal way.

*/


/*   --------------- Description of special symboles: ---------------------


     <- means you have to or can set this variable.

     -> means this variable has something for you! U'r request and more.


*/

/* No longer used and free command numbers:

#define CF_UnUsed1         416
#define SC_UnUsed1         700
#define SC_UnUsed2         701

 */

/*   ##################### COMMAND DEFINES BEGIN ##########################
 */

/*   ------------------------- NR commands --------------------------------
 */

#define MC_DoorStart         1
/* Register a new Door, counter increases
 *
 * U HAVE TO START THIS FIRST!!!
 */

#define MC_ShutDown          2
/* Tells the node that the door is shutting down, counter decreases
 *
 * U HAVE TO USE THIS ON ENDING A DOOR! ALSO ON ENDING BECAUSE OF AN ERROR!
 *
 * U ALSO CAN USE ALTERNATIVE MC_ShutDownLastWords!
 */

#define MC_ShutDownLastWords 3
/* Tells the node that the door is shutting down also write some last words,
 * counter decreases
 *
 * Data1
 * Data2
 * Data3
 * IOString <- The string to be send to the user.
 *
 * U HAVE TO USE THIS ON ENDING A DOOR! ALSO ON ENDING BECAUSE OF AN ERROR!
 *
 * U ALSO CAN USE ALTERNATIVE MC_ShutDown!
 */

#define NR_SendStr          10
/* Sends a String to the User
 *
 * Data1
 * Data2
 * Data3
 * IOString <- The string to be send to the user.
 */

#define NR_SendStrCRLF      11
/* Sends a String to the User with CR/LF
 *
 * Data1
 * Data2
 * Data3
 * IOString <- The string to be send to the user.
 */

#define NR_SendStrCon       12
/* Sends a String to the Console only
 *
 * Data1
 * Data2
 * Data3
 * IOString <- The string to be send to the console.
 */

#define NR_SendStrSer       13
/* Sends a String to the Serial only
 *
 * Data1
 * Data2
 * Data3
 * IOString <- The string to be send to the serial.
 */

#define NR_PromptChars      14
/* Prompt the user for a specified number of chars
 *
 * Data1    <- Max. chars can be typed.
 * Data2    <- Flag for different modes.
 * Data3    -> Returncode from internal procedure (usable for Data2 flag 3).
 * IOString <- String will be used in prompt.
 * IOString -> Filled string with chars typed by the user.
 *
 * Data2 flag description:
 *
 * 0 Simple stringingput without edit and other features.
 * 1 Stringingput for chatmode. (uses wordwrapping. Do not use this to get a
 *   complete string, because after a wordwrap the string changes).
 * 2 Stringingput for MsgEditor. (You can't use this flag! It can cause
 *   damage, because it uses an internal list. Flag denied.
 * 3 Stringingput for bulletinviewer. (It returns cursorkeys left and right
 *   up and down for the bullview to switch -(x)-BullHelp.txt texts.
 *   Data3 contains: Up: 65 Down: 66 Rigth: 67 Left: 68.
 * 4 The same as 0, but all chars will be displayed as *. Like: ****** ,
 *   but they musn't be displayed as * on console! If the flag "Display
 *   Passwords to SysOp" is set the chars will be displayed as real chars
 *   on console.
 * 5 Unused.
 * 6 A real feature! This is the best flag u can use! U let the user totaly
 *   edit his string. The user can walk into the string, delete chars with
 *   delete and backspace and the user can insert at every position new chars.
 * 7 The same as 4, but here will definality no single char be typed on the
 *   screen. No single char means also NO stars (*). Only exception is if
 *   the flag "Display Passwords to SysOp" is set the chars will be displayed
 *   as real chars, but only on console.
 * 8 Numeric mode only. Only alphanumeric chars can be typed and no
 *   alphabetical ones.
 *
 * Higher flags are reserved and can't be used.
 * Normaly u can only use flag 0 and 6 !!!
 *
 * If IOString contains data on calling this command, the user will get this
 * data into the prompt, and can edit it (flag 6).
 */

#define NR_HotKey           15
/* Gets a char without waiting for it
 *
 * Data1
 * Data2    -> The char typed by the user.
 * Data3    -> 0 = Console char, 1 = Serial char.
 * IOString
 *
 * Note: This command checks for a available char. If there is a char
 *       available you will get it immediatly, else you will get 0 in
 *       Data2.
 *
 * Note: NR_HotKey is the same as AR_GetKey, but on AR_GetKey you wont
 *       get the char, only a notify that a char was pressed.
 *       Here on NR_HotKey you will get this pressed key.
 */

#define NR_BBSName          16
/* Retrieve the BBS Name
 *
 * Data1
 * Data2
 * Data3
 * IOString -> The BBS Name.
 */

#define NR_SysOp            17
/* Retrieve the Sysop Name
 *
 * Data1
 * Data2
 * Data3
 * IOString -> The SysOp Name.
 */

#define NR_SetFlagFile      18
/* Add Files to the Flaglist
 *
 * Data1    <- The conferencenumber in which you want to flag files.
 * Data2
 * Data3    -> Errormsg 0 on success.

 * IOString <- The file(s) to be flagged, you can flag more files at the same
 *             time, but they must have spaces beetween each other flag.
 *             maximum of 12 chars per flag and overall 201 chars including
 *             the spaces. Note that the BBS filters double flags.
 *
 * Note: If Data1 is lower than 1 or higher than the last conference
 *       available, Data1 will be set to the current conference number!
 *
 * Note: Data3 is errormsg:
 *       0  means successfull flagged.
 *       1  means file allready flagged.
 *       2  means length of IOString is lower than  byte, maybe not true here,
 *                because of more files to be flagged.
 *       -1 means list element memory allocation faild.
 *       -2 means given conference not found, normaly not happens here,
 *                because of Data1 check.
 *       Think of that here may be flagged more than one file, this may have
 *       to result that 1, 2 and -1 musn't be correct!
 *       But it must be correct if IOString contains only one file.
 */

#define NR_GetFlagFile      19
/* Get File from the Flaglist
 *
 * Data1    <- The conferencenumber from which you want to get a flagged file.
 * Data2
 * Data3
 * IOString -> The filename of the flagged file.
 * StructDummy1 -> The struct FileList.
 *
 * Note: This command counts the flagnumbers itself! That means if you got
 *       a flag, you will get the next flag if you call this commandd again!
 *       This happens till no more flags are available!
 *       To reset this you have to use the command: NR_ResetFlagFile!
 *       But if you use NR_ResetFlagFile you get also on that command the FIRST
 *       flagged file! That means first use NR_ResetFlagFile and get the
 *       first fileflag, from now on use NR_GetFlagFile to get the next
 *       flagged files... if the IOString is empty no more flagged files
 *       are available.
 *
 * Note: If Data1 is lower than 1 or higher than the last conference
 *       available, Data1 will be set to the current conference number!
 */

#define NR_ActiveNode       20
/* Check for a specific Node exists
 *
 * Data1    <- The Node you want to check if it exists or not.
 * Data2    -> 1 if Node exists, 0 if it doesn't.
 * Data3
 * IOString
 */

#define NR_ActiveNodes      21
/* Retrieve a string of active&inactive nodes
 *
 * Data1
 * Data2
 * Data3
 * IOString -> A String contains on every charposition an X if the relative
 *             Node exists. Byte 0 stands for Node0 !
 *             A maximum of 200 Nodes will be checked.
 */

#define NR_TimeOut          22
/* Retrieve the door timeout limit
 *
 * Data1
 * Data2    -> Contains the door timeout limit.
 * Data3
 * IOString
 */

#define NR_MainLine         23
/* Retrieve the menu prompt arguments prior to the door being entered
 *
 * Data1
 * Data2
 * Data3
 * IOString -> Contains the MainLine, the full argumentstring which was
 *             used on starting this door!
 *             Note that FAME makes no difference on typing arguments!
 *             You can have so much spaces you are willing to on the menu
 *             command between arguments, FAME will interpret it!
 *             This means it could be possible that the String you get
 *             can look like this one: " who       u            force ".
 *             This means you should interpret it also!
 *             If this is to much work for you, you should use instead the
 *             commands: NR_GetFullArg, NR_GetArgument1 ,NR_GetArgument2,
 *             NR_GetArgument3 and NR_GetArgument4. The first has sorted
 *             the first 4 arguments, all other arguments are like they are
 *             typed in. NR_GetArgument(1-4) is every single argument.
 *             But on those commands you get ONLY the arguments, not the
 *             commandname!
 */

#define NR_NodeID           24
/* Retrieve the Node number for  the current node
 *
 * Data1
 * Data2    -> The current Nodenumber.
 * Data3
 * IOString
 */

#define NR_MinUpCPS         25
/* Retreive Min Upload CPS
 *
 * Data1
 * Data2    -> Users min. Upload CPS.
 * Data3
 * IOString
 */

#define NR_GetConfNum       26
/* Retrieve the current Conf number
 *
 * Data1
 * Data2    -> Cotains the actual Conferencenumber.
 * Data3
 * IOString
 */

#define NR_SearchAccount    27
/* Looks for Account in Userdatas
 *
 * Data1    <- The usernumber you are searching for.
 * Data2
 * Data3
 * IOString
 * StructDummy1 -> The userkey structure of the user.
 *                 You have to check the ReturnCode! If it's non zero the
                   StructDummy1 isn't valid!!!
 */

#define NR_StampTime        28
/* Retrieve the current time string
 *
 * Data1
 * Data2
 * Data3
 * IOString -> Contains the current timestring.
 */

#define NR_CurrTime         29
/* Retrieve the current time in seconds since January
 *
 * Data1
 * Data2    -> Contains the current timevalue.
 * Data3
 * IOString
 */

#define NR_ThisConfAccess   30
/* Retrieve the users conference access of selected Conf
 *
 * Data1    <- The Confnumber you are checking the access of the online user.
 * Data2    -> 1 if the user has access to the conference, else 0.
 * Data3
 * IOString
 *
 * Note: If Data1 is lower than 1 or higher than the last conference
 *       available, Data1 will be set to the current conference number!
 */

#define NR_Name             31
/* Retrieve users name/handle
 *
 * Data1
 * Data2
 * Data3
 * IOString -> Contains the UserName.
 */

#define NR_Password         32
/* Retrieve users password
 *
 * Data1
 * Data2
 * Data3
 * IOString -> Contains the Userpassword.
 */

#define NR_Location         33
/* Retrieve users location
 *
 * Data1
 * Data2
 * Data3
 * IOString -> Contains the User's location.
 */

#define NR_From             34
/* Retrieve users from
 *
 * Data1
 * Data2
 * Data3
 * IOString -> Contains the User's origin.
 */

#define NR_PhoneNumber      35
/* Retrieve users phone number
 *
 * Data1
 * Data2
 * Data3
 * IOString -> Contains the User's phonenumber.
 */

#define NR_SlotNumber       36
/* Retrieve users slot number
 *
 * Data1
 * Data2    -> Contains the User's slotnumber.
 * Data3
 * IOString
 */

#define NR_AccessLevel      37
/* Retrieve users access level
 *
 * Data1
 * Data2    -> Contains the User's level.
 * Data3
 * IOString
 */

#define NR_RatioType        38
/* Retrieve users ratiotype
 *
 * Data1
 * Data2    -> Contains the User's ratiotype.
 * Data3
 * IOString
 */

#define NR_Ratio            39
/* Retrieve users ratio
 *
 * Data1
 * Data2    -> Contains the User's ratio.
 * Data3
 * IOString
 */

#define NR_CompType         40
/* Retrieve users computertype code
 *
 * Data1
 * Data2    -> Contains the User's computertype code.
 * Data3
 * IOString -> Contains the User's computer.
 */

#define NR_ModemType        41
/* Retrieve users modemtype code
 *
 * Data1
 * Data2    -> Contains the User's modemtype code.
 * Data3
 * IOString -> Contains the User's modem.
 */

#define NR_MessagePosted    42
/* Retrieve users messagesposted
 *
 * Data1
 * Data2
 * Data3    -> Contains the User's message posted.
 * IOString
 */

#define NR_MessageRead      43
/* Retrieve the number of readen Msg's
 *
 * Data1
 * Data2
 * Data3    -> Contains the User's message read.
 * IOString
 */

#define NR_NoCalls          44
/* Retrieve number of usercalls
 *
 * Data1
 * Data2
 * Data3    -> Contains the User's calls.
 * IOString
 */

#define NR_TimeLastOn       45
/* Retrieve time user last called
 *
 * Data1
 * Data2    -> Contains the User's last time called.
 * Data3
 * IOString
 */

#define NR_TimeUsed         46
/* Retrieve timeused today
 *
 * Data1
 * Data2    -> Contains the User's time used.
 * Data3
 * IOString
 */

#define NR_TimeLimit        47
/* Retrieve timeallowed for a user
 *
 * Data1
 * Data2    -> Contains the User's time limit.
 * Data3
 * IOString
 */

#define NR_TimeRemain       48
/* Retrieve total time remaining
 *
 * Data1
 * Data2    -> Contains the User's total time remain.
 * Data3
 * IOString
 */

#define NR_StampLastOn      49
/* Retrieve the date string containing the date when the user last logged on
 *
 * Data1
 * Data2
 * Data3
 * IOString -> Contains the User's last time called as an timestring.
 */

#define NR_ConfAccess       50
/* Retrieve the users conference access
 *
 * Data1
 * Data2
 * Data3
 * IOString -> Contains the User's conference access alias.
 */

#define NR_Uploads          51
/* Retrieve number of useruploads
 *
 * Data1
 * Data2
 * Data3    -> Contains the User's number of uploads.
 * IOString
 */

#define NR_Downloads        52
/* Retrieve number of userdownloads
 *
 * Data1
 * Data2
 * Data3    -> Contains the User's number of downloads.
 * IOString
 */

#define NR_BytesUpload      53
/* Retrieve bytes uploads per user
 *
 * Data1
 * Data2
 * Data3    -> Contains the User's bytes uploaded.
 * IOString
 */

#define NR_BytesDownload    54
/* Retrieve bytes downloaded per user
 *
 * Data1
 * Data2
 * Data3    -> Contains the User's bytes downloaded.
 * IOString
 */

#define NR_DailyByteLimit   55
/* Retrieve a users daily byte
 *
 * Data1
 * Data2
 * Data3    -> Contains the User's daily byte limit.
 * IOString
 */

#define NR_DailyFileLimit   56
/* Retrieve a users daily files
 *
 * Data1
 * Data2
 * Data3    -> Contains the User's daily file limit.
 * IOString
 */

#define NR_DailyByteDld     57
/* Retrieve daily bytes downloaded
 *
 * Data1
 * Data2
 * Data3    -> Contains the User's daily bytes downloaded.
 * IOString
 */

#define NR_DailyFileDld     58
/* Retrieve daily files downloaded
 *
 * Data1
 * Data2
 * Data3    -> Contains the User's daily files downloaded.
 * IOString
 */

#define NR_DailyByteBonus   59
/* Retrieve daily byte bonus from user
 *
 * Data1
 * Data2
 * Data3    -> Contains the User's daily byte bonus.
 * IOString
 */

#define NR_DailyFileBonus   60
/* Retrieve daily file bonus from user
 *
 * Data1
 * Data2
 * Data3    -> Contains the User's daily file bonus.
 * IOString
 */

#define NR_Expert           61
/* Retrieve expert mode
 *
 * Data1
 * Data2    -> 1 means Expertmode is set, else 0 means it's not.
 * Data3
 * IOString
 */

#define NR_NumLines         62
/* Retrieve user number of lines
 *
 * Data1
 * Data2    -> Contains the User's number of lines.
 * Data3
 * IOString
 *
 * Note: The number of lines is in real 1 line more than u will get here,
 *       because of scrolling texts u must have 1 line in reserve.
 */

#define NR_Birthday         63
/* Retrieve the users Birthday
 *
 * Data1
 * Data2
 * Data3
 * IOString -> Contains the User's birthday.
 */

#define NR_MenuPrompt       64
/* Retrieve the users Menuprompt
 *
 * Data1
 * Data2
 * Data3
 * IOString -> Contains the User's menuprompt.
 */

#define NR_EditorType       65
/* Retrieve the users Editor type
 *
 * Data1
 * Data2    -> Contains the User's editor type.
 * Data3
 * IOString
 */

#define NR_XferProt         66
/* Retrieve the User's (last) used transfer protocol
 *
 * Data1
 * Data2    -> Contains the User's (last) used transfer protocol value.
 * Data3
 * IOString -> Contains the User's (last) used transfer protocol name.
 */

#define NR_LostCarrier      67
/* Retrieve the number of Los Carriers
 *
 * Data1
 * Data2    -> Contains the User's numbers of loss carriers.
 * Data3
 * IOString
 */

#define NR_Zoom             68
/* Retrieve the ZOOM type
 *
 * Data1
 * Data2    -> Contains the User's Zoom type.
 * Data3
 * IOString
 */

#define NR_SysLanguage      69
/* Retrieve the current System language specifications
 *
 * Data1
 * Data2
 * Data3
 * IOString -> Contains the User's tetx language specification.
 */

#define NR_Language         70
/* Retrieve the current language specifications
 *
 * Data1
 * Data2    -> Contains the User's current language setting.
 * Data3
 * IOString
 */

#define NR_LineCount        71
/* Retrieve the current number of lines viewed
 *
 * Data1
 * Data2    -> Contains the current number of lines viewed.
 * Data3
 * IOString
 */

#define NR_AnsiColor        72
/* Retrieve the ANSI flag
 *
 * Data1
 * Data2    -> Contains the User's Ansiflag.
 * Data3
 * IOString
 */

#define NR_SentBy           73
/* Retrieve the SentBy Line
 *
 * Data1    <- The conferencenumber in which u want to change the SentByLine
 * Data2
 * Data3
 * IOString -> Contains the User's SentBy line.
 */

#define NR_AutoFileID       74
/* Retrieve the Auto FILE_ID.DIZ
 *
 * Data1
 * Data2    -> Contains the User's fileid flag.
 * Data3
 * IOString
 */

#define NR_NewMessage       75
/* Retrieve the NewMessage Status
 *
 * Data1
 * Data2    -> Contains the User's newscan flag.
 * Data3
 * IOString
 */

#define NR_Goodbye          76
/* Retrieve the Goodbye Flag
 *
 * Data1
 * Data2    -> Contains the User's (fast)goodbye flag.
 * Data3
 * IOString
 */

#define NR_ViewFlag         77
/* Retrieve the View Flag
 *
 * Data1
 * Data2    -> Contains the User's view flag.
 * Data3
 * IOString
 */

#define NR_ZippyFlag        78
/* Retrieve the Zippy Flag
 *
 * Data1
 * Data2    -> Contains the User's zippy flag.
 * Data3
 * IOString
 */

#define NR_ReplyMSGFlag     79
/* Retrieve the ReplyMS Flag
 *
 * Data1    <- The conferencenumber in which u want to change the SentByLine
 * Data2    -> Contains the User's reply msg flag
 * Data3
 * IOString
 */

#define NR_NukedFiles       80
/* Retrieve the NukedFiles
 *
 * Data1
 * Data2
 * Data3    -> Contains the User's nuked files.
 * IOString
 */

#define NR_NukedBytes       81
/* Retrieve the NukedBytes
 *
 * Data1
 * Data2
 * Data3    -> Contains the User's nuked bytes.
 * IOString
 */

#define NR_MinDownCPS       82
/* Retrieve the Min Downlaod CPS
 *
 * Data1
 * Data2    -> Contains the User's min. download CPS.
 * Data3
 * IOString
 */

#define NR_GetEditString    83
/* Prompt the user for a specified number of chars and edits String
 *
 * Data1    <- Max. chars can be typed.
 * Data2    <- Flag for different modes.
 * Data3    -> Returncode from internal procedure (usable for Data2 flag 3).
 * IOString <- String will be used in prompt.
 * IOString -> Filled string with chars typed by the user.
 *
 * Data2 flag description:
 *
 * 0 Simple stringingput without edit and other features.
 * 1 Stringingput for chatmode. (uses wordwrapping. Do not use this to get a
 *   complete string, because after a wordwrap the string changes).
 * 2 Stringingput for MsgEditor. (You musn't use this flag! It can cause
 *   damage, because it uses an internal list.
 * 3 Stringingput for bulletinviewer. (It returns cursorkeys left and right
 *   up and down for the bullview to switch -(x)-BullHelp.txt texts.
 *   Data3 contains: Up: 65 Down: 66 Rigth: 67 Left: 68.
 * 4 The same as 0, but all chars will be displayed as *. Like: ****** ,
 *   but they musn't be displayed as * on console! If the flag "Display
 *   Passwords to SysOp" is set the chars will be displayed as real chars
 *   on console.
 * 5 Unused.
 * 6 A real feature! This is the best flag u can use! U let the user totaly
 *   edit his string. The user can walk into the string, delete chars with
 *   delete and backspace and the user can insert at every position new chars.
 * 7 The same as 4, but here will definality no single char be typed on the
 *   screen. No single char means also NO stars (*). Only exception is if
 *   the flag "Display Passwords to SysOp" is set the chars will be displayed
 *   as real chars, but only on console.
 * 8 Numeric mode only. Only alphanumeric chars can be typed and no
 *   alphabetical ones.
 *
 * Higher flags are reserved and can't be used.
 * Normaly u can only use flag 0 and 6 !!!
 *
 * If IOString contains data on calling this command, the user will get this
 * data into the prompt, and can edit it (flag 6).
 *
 * Note: NR_GetEditString is nearly the same as NR_PromptChars. In fact it
 *       is the same, but if you use NR_GetEditString no Data2 (mode flag)
 *       check will be done. Here you can use undocumented, obsolete and non
 *       existing modes. Beware of what you're doing it can result in crashes
 *       if you use for example flag number 2.
 */

#define NR_ResetFlagFile    84
/* Reset and get the first file from the Flaglist
 *
 * Data1    <- The conferencenumber from which you want to get a flagged file.
 * Data2
 * Data3
 * IOString -> The filename of the first flagged file.
 * StructDummy1 -> The struct FileList.
 *
 * Note: Use NR_ResetFlagFile at first! You will reset the internal counter
 *       and get the first flagged file! That means first use NR_ResetFlagFile
 *       and get the first fileflag, from now on use NR_GetFlagFile to get
 *       the next flagged files...
 *       If IOString is empty no flagged files are available.
 *
 * Note: If Data1 is lower than 1 or higher than the last conference
 *       available, Data1 will be set to the current conference number!
 */

#define NR_DeleteFlagFile   85
/* Delete File of the Flaglist
 *
 * Data1    <- The conferencenumber in which u want to delete a flagged file.
 * Data2    <- If true the file will not be deleted if the FFFL_NODELETE flag
 *             of the file flag is set. If false flag will be deleted in any
 *             case.
 * Data3
 * IOString <- The file or match to delete file(s).
 *
 * Note: U can use a full filename and also a match like abc* or abc+
 *       or * or + and so on to delete files.
 *
 * Note: If Data1 is lower than 1 or higher than the last conference
 *       available, Data1 will be set to the current conference number!
 */

#define NR_DelFlagFileNum   86
/* Delete File of the Flaglist with the fileflagnumber
 *
 * Data1    <- The conferencenumber in which u want to delete a flagged file.
 * Data2    <- The number of a flag u want to delete.
 * Data3
 * IOString
 *
 * Note: If Data1 is lower than 1 or higher than the last conference
 *       available, Data1 will be set to the current conference number!
 */

#define NR_GetFullArg       87
/* Retrieve the full Argumentstring
 *
 * Data1
 * Data2
 * Data3
 * IOString -> Will be the string of all arguments.
 *
 * The first 4 arguments will automaticly be sorted by FAME, arguments after
 * argument with a single space between each other, but all other orguments
 * behind 4 will be like they are typed it.
 */

#define NR_GetArgument1     88
/* Get the first argument
 *
 * Data1
 * Data2
 * Data3
 * IOString -> Will be the first argument seperatly.
 */

#define NR_GetArgument2     89
/* Get the second argument
 *
 * Data1
 * Data2
 * Data3
 * IOString -> Will be the second argument seperatly.
 */

#define NR_GetArgument3     90
/* Get the third argument
 *
 * Data1
 * Data2
 * Data3
 * IOString -> Will be the third argument seperatly.
 */

#define NR_GetArgument4     91
/* Get all other arguments
 *
 * Data1
 * Data2
 * Data3
 * IOString -> Will be the fourth and all other arguments.
 */

#define NR_WaitChar         92
/* Get a char with waiting for it
 *
 * Data1
 * Data2    -> The char typed be the user.
 * Data3    -> 0 = Console char, 1 = Serial char.
 * IOString <- String will be displayed to the user.
 *
 * Note: You get all ascii codes also RAW codes, but only the CSI codes and
 *       escape-sequences will be filtered, because the BBS has to check for
 *       FKeys and more.
 *       You will get the cursorkeys as the following values:
 *       4 for UP, 5 for DOWN, 3 for RIGHT and 2 for LEFT.
 */

#define NR_GetConFontSize   93
/* Get the Console Font height and width
 *
 * Data1
 * Data2    -> height.
 * Data3    -> width.
 * IOString
 */

#define NR_ResetANSI        94
/* Reset console and serial attributes to standart values
 *
 * Data1
 * Data2
 * Data3
 * IOString
 *
 * No Data needed.
 */

#define NR_ResetANSIOnExit  95
/* Set a flag to let the node reset console and serial attributes to
 * standart values on MC_ShutDown / MC_ShutDownLastWord
 *
 * Data1
 * Data2
 * Data3
 * IOString
 *
 * No Data needed.
 */

#define NR_CONNumLines      96
/* Retrieve the number of lines from the console
 *
 * Data1
 * Data2    -> Contains the number of lines.
 * Data3
 * IOString
 *
 * This function is important if you must create a Screen "View" and the
 * user has more lines on his screen than the sysop on the user's node
 * screen. You may handle the sysop's display separately from the user view !
 *
 * Note: The number of lines is in real 1 line more than u will get here,
 *       because of scrolling texts u must have 1 line in reserve.
 */

#define NR_NumberOfChats    97
/* Retrieve the number of chats
 *
 * Data1
 * Data2
 * Data3    -> Contains the number of chats.
 * IOString
 */

#define NR_NumberOfPages    98
/* Retrieve the number of pages
 *
 * Data1
 * Data2    -> Contains the number of pages.
 * Data3
 * IOString
 *
 */

#define NR_NumberOfDayPages 99
/* Retrieve the number of day pages
 *
 * Data1
 * Data2    -> Contains the number of day pages.
 * Data3
 * IOString
 */

#define NR_NumOfPagesAllowed 100
/* Retrieve the number of pages allowed per day
 *
 * Data1
 * Data2    -> Contains the number of pages per day.
 * Data3
 * IOString
 */

#define NR_NumberOfDayRelogs 101
/* Retrieve the number of day relogins
 *
 * Data1
 * Data2    -> Contains the number of day relogins.
 * Data3
 * IOString
 */

#define NR_NumOfRelogsAllowed 102
/* Retrieve the number of relogins allowed per day
 *
 * Data1
 * Data2    -> Contains the number of relogins per day.
 * Data3
 * IOString
 */

#define NR_DoorHelp         103
/* Try to load and display online user help
 *
 * Data1
 * Data2
 * Data3
 * IOString <- Minimum Version.Revision string. Eample: "1.23"
 * StringPtr <- Help file name without any paths.
 *
 * Note: Nearly all Arguments are invalid after usage!
 *       Especialy NR_MainLine and NR_GetArgument1 command arguments!
 */

#define NR_SetDoorReturnCode 104
/* Set DoorReturnCode
 *
 * Data1    <- The new DoorReturnCode.
 * Data2
 * Data3
 * IOString
 */

#define NR_GetDoorReturnCode 105
/* Retrieve DoorReturnCode
 *
 * Data1
 * Data2    -> Contains the DoorReturnCode.
 * Data3
 * IOString
 */

#define NR_ClrFileFlgLst     106
/* Delete all file flags in a single confernece
 *
 * Data1    <- The conference number.
 * Data2
 * Data3
 * IOString
 *
 * Note: If Data1 is lower than 1 or higher than the last conference
 *       available, Data1 will be set to the current conference number!
 */

#define NR_ClrFileFlgLsts    107
/* Delete all file flags in all conferneces
 *
 * Data1
 * Data2
 * Data3
 * IOString
 */

#define NR_GetRelativeStatus 108
/* Retrieve the relative status. Has this node relative conferences?
 *
 * Data1
 * Data2    -> The relative status, if non 0 relative is on.
 * Data3
 * IOString
 */

#define NR_AbsToRel          109
/* Convert an absolute confernece number (real) to the relative one.
 *
 * Data1    <- Your absolute conference number.
 * Data2    -> The relative conference number.
 * Data3
 * IOString
 *
 * Note: If Data2 is 0 the user has no access to this conference.
 *
 * Note: If Data1 is lower than 1 or higher than the last conference
 *       available, Data1 will be set to the current conference number!
 */

#define NR_RelToAbs          110
/* Convert a relative confernece number to the absolute (real) one.
 *
 * Data1    <- Your relative conference number.
 * Data2    -> The absolute conference number.
 * Data3
 * IOString
 *
 * Note: If Data1 is lower than 1 or higher than the last conference
 *       available, Data1 will be set to the current relative conference
 *       number!
 */

#define NR_GetAbortIOPort    111
/* Get the AbortIO MsgPort
 *
 * Data1
 * Data2
 * Data3
 * IOString
 * StructDummy1 -> The AbortIO MsgPort structure
 */

#define NR_GetDoorDatas      112
/* Get datas of your door like the path
 *
 * Data1
 * Data2
 * Data3
 * IOString -> The path from where FAME has started your door.
 * StringPtr -> The complete path + door file name.
 * StructDummy1 -> Pointer to struct dOORS (Developer only).
 */

#define NR_GetDoorCallName   113
/* Get the DoorCallName.
 *
 * Data1
 * Data2
 * Data3
 * IOString -> The DoorCallName.
 */


#define NR_GetUserLevelFlags 114
/* Get the level flags for the actual user.
 *
 * Data1
 * Data2    -> MaxPages
 * Data3    -> MaxRelogins
 * IOString -> Array of level flags.
 *
 * Note: You will get an Array (IOString) where you can use the level defines
 *       to see if a level flag is set.
 *       True / (1) = (flag set), false / 0 = (flag not set).
 *       If in the position you check the value is 0 the level flag
 *       is not set and the user isn't able to do this. If the value is
 *       <> 0 or != 0 (not 0) it's set and the user is able to do this.
 *
 * Note: For the level defines used in the array see FAMEDefine.h
 *       FLVL_<Level flag name> ( FLVL_ ) defines.
 *
 * Example 1:
 *
 * PutCommand("",0,0,0,NR_GetUserLevelFlags);
 *
 * if(MyFAMEDoorMsg -> fdom_IOString[ FLVL_CommenttoSysOp ])
 *   PutString("\r\nComment to SysOp allowed!\r\n",1);
 * else
 *   PutString("\r\nNo comment to SysOp allowed!\r\n",1);
 *
 * Example 2:
 *
 * PutCommand("",0,0,0,NR_GetUserLevelFlags);
 *
 * if(MyFAMEDoorMsg -> fdom_IOString[ FLVL_CommenttoSysOp ] == 0)
 *   PutString("\r\nNo comment to SysOp allowed!\r\n",1);
 * else
 *   PutString("\r\nComment to SysOp allowed!\r\n",1);
 *
 */

/*   ------------------------- NC commands --------------------------------
 */

#define NC_TimeOut         200
/* Change the door timeout limit
 *
 * Data1    <- The new doortimeout.
 * Data2
 * Data3
 * IOString
 */

#define NC_Name            201
/* Change users name/handle
 *
 * Data1
 * Data2
 * Data3
 * IOString <- The new username of the online user (max. 30 chars).
 *
 * Note: If ReturnCode=1 the username allready exists! Nothing changed!
 */

#define NC_Password        202
/* Change users password
 *
 * Data1
 * Data2
 * Data3
 * IOString <- The new password of the online user (max. 20 chars).
 */

#define NC_Location        203
/* Change users location
 *
 * Data1
 * Data2
 * Data3
 * IOString <- The new location of the online user (max. 30 chars).
 */

#define NC_From            204
/* Change users from
 *
 * Data1
 * Data2
 * Data3
 * IOString <- The new origin of the online user (max. 30 chars).
 */

#define NC_PhoneNumber     205
/* Change users phone number
 *
 * Data1
 * Data2
 * Data3
 * IOString <- The new phonenumber of the online user (max. 15 chars).
 */

#define NC_AccessLevel     206
/* Change users access level
 *
 * Data1    <- The new level of the online user.
 * Data2
 * Data3
 * IOString
 */

#define NC_RatioType       207
/* Change users ratiotype
 *
 * Data1    <- The new ratiotype of the online user.
 * Data2
 * Data3
 * IOString
 */

#define NC_Ratio           208
/* Change users ratio
 *
 * Data1    <- The new ratio of the online user.
 * Data2
 * Data3
 * IOString
 */

#define NC_CompType        209
/* Change users computertype code
 *
 * Data1    <- The new computertype of the online user.
 * Data2
 * Data3
 * IOString <- The new computer.
 */

#define NC_ModemType       210
/* Change users modemtype code
 *
 * Data1    <- The new modemtype of the online user.
 * Data2
 * Data3
 * IOString <- The new modem.
 */

#define NC_MessagePosted   211
/* Change users messagesposted
 *
 * Data1
 * Data2
 * Data3    <- The new messagesposted of the online user.
 * IOString
 */

#define NC_MessageRead     212
/* Change the number of readen Msg's
 *
 * Data1
 * Data2
 * Data3    <- The new messageread of the online user.
 * IOString
 */

#define NC_NoCalls         213
/* Change number of usercalls
 *
 * Data1
 * Data2
 * Data3    <- The new number of calls of the online user.
 * IOString
 */

#define NC_TimeLastOn      214
/* Change time user last called
 *
 * Data1    <- The new last time called of the online user.
 * Data2
 * Data3
 * IOString
 */

#define NC_TimeUsed        215
/* Change timeused today
 *
 * Data1    <- The new timeused today of the online user.
 * Data2
 * Data3
 * IOString
 */

#define NC_TimeLimit       216
/* Change timeallowed for a user
 *
 * Data1    <- The new timeallowed of the online user.
 * Data2
 * Data3
 * IOString
 */

#define NC_TimeRemain      217
/* Change total time remaining
 *
 * Data1    <- The new total time remaining of the online user.
 * Data2
 * Data3
 * IOString
 */

#define NC_Uploads         218
/* Change number of useruploads
 *
 * Data1
 * Data2
 * Data3    <- The new number of useruploads of the online user.
 * IOString
 */

#define NC_Downloads       219
/* Change number of userdownloads
 *
 * Data1
 * Data2
 * Data3    <- The new number of userdownloads of the online user.
 * IOString
 */

#define NC_BytesUpload     220
/* Change bytes uploads per user
 *
 * Data1
 * Data2
 * Data3    <- The new bytes uploaded of the online user.
 * IOString
 */

#define NC_BytesDownload   221
/* Change bytes downloaded per user
 *
 * Data1
 * Data2
 * Data3    <- The new bytes downloaded of the online user.
 * IOString
 */

#define NC_DailyByteLimit  222
/* Change a users daily byte
 *
 * Data1
 * Data2
 * Data3    <- The new users daily byte of the online user.
 * IOString
 */

#define NC_DailyFileLimit  223
/* Change a users daily files
 *
 * Data1
 * Data2
 * Data3    <- The new users daily files of the online user.
 * IOString
 */

#define NC_DailyByteDld    224
/* Change daily bytes downloaded
 *
 * Data1
 * Data2
 * Data3    <- The new daily bytes downloaded of the online user.
 * IOString
 */

#define NC_DailyFileDld    225
/* Change daily files downloaded
 *
 * Data1
 * Data2
 * Data3    <- The new daily files downloaded of the online user.
 * IOString
 */

#define NC_DailyByteBonus  226
/* Change daily byte bonus from user
 *
 * Data1
 * Data2
 * Data3    <- The new daily byte bonus of the online user.
 * IOString
 */

#define NC_DailyFileBonus  227
/* Change daily file bonus from user
 *
 * Data1
 * Data2
 * Data3    <- The new daily file bonus of the online user.
 * IOString
 */

#define NC_Expert          228
/* Change expert mode
 *
 * Data1    <- The new expert mode of the online user.
 * Data2
 * Data3
 * IOString
 */

#define NC_NumLines        229
/* Change user number of lines
 *
 * Data1    <- The new number of lines of the online user.
 * Data2
 * Data3
 * IOString
 *
 * Note: You have to set Data1 one line less than the full number of lines of
 *       the screen, because of scrolling texts u must have 1 line in reserve.
 */

#define NC_Birthday        230
/* Change the users Birthday
 *
 * Data1
 * Data2
 * Data3
 * IOString <- The new Birthday of the online user (max. 10 chars).
 */

#define NC_MenuPrompt      231
/* Change the users Menuprompt
 *
 * Data1
 * Data2
 * Data3
 * IOString <- The new Menuprompt of the online user (max. 200 chars).
 */

#define NC_EditorType      232
/* Change the users Editor type
 *
 * Data1    <- The new Editor type of the online user.
 * Data2
 * Data3
 * IOString
 */

#define NC_XferProt        233
/* obsolete
 *
 * Data1   
 * Data2
 * Data3
 * IOString
 */

#define NC_Zoom            234
/* Change the ZOOM type
 *
 * Data1    <- The new ZOOM type of the online user.
 * Data2
 * Data3
 * IOString
 */

#define NC_SysLanguage     235
/* Change the current System language specifications
 *
 * Data1
 * Data2
 * Data3
 * IOString <- The new System language of the online user (max. 30 chars).
 */

#define NC_Language        236
/* Change the current language specifications
 *
 * Data1    <- The new language of the online user.
 * Data2
 * Data3
 * IOString
 */

#define NC_LineCount       237
/* Change the user's current number of lines viewed
 *
 * Data1    <- The new number of lines viewed.
 * Data2
 * Data3
 * IOString
 */

#define NC_AnsiColor       238
/* Change the ANSI flag
 *
 * Data1    <- The new ANSI flag of the online user.
 * Data2
 * Data3
 * IOString
 */

#define NC_SentBy          239
/* Change the SentBy Line
 *
 * Data1    <- The conferencenumber in which u want to change the SentByLine
 * Data2
 * Data3
 * IOString <- The new SentBy Line of the online user (max 45 chars).
 *
 * Note: If Data1 is lower than 1 or higher than the last conference
 *       available, Data1 will be set to the current conference number!
 */

#define NC_AutoFileID      240
/* Change the Auto FILE_ID.DIZ
 *
 * Data1    <- The new Auto FILE_ID.DIZ of the online user.
 * Data2
 * Data3
 * IOString
 */

#define NC_NewMessage      241
/* Change the NewMessage Status
 *
 * Data1    <- The new NewMessage Status of the online user.
 * Data2
 * Data3
 * IOString
 */

#define NC_Goodbye         242
/* Change the Goodbye Flag
 *
 * Data1    <- The new Goodbye Flag of the online user.
 * Data2
 * Data3
 * IOString
 */

#define NC_ViewFlag        243
/* Change the View Flag
 *
 * Data1    <- The new View Flag of the online user.
 * Data2
 * Data3
 * IOString
 */

#define NC_ZippyFlag       244
/* Change the Zippy Flag
 *
 * Data1    <- The new Zippy Flag of the online user.
 * Data2
 * Data3
 * IOString
 */

#define NC_ReplyMSGFlag    245
/* Change the ReplyMSG Flag
 *
 * Data1    <- The conferencenumber you are going to change the ReplyMSG Flag.
 * Data2    <- The new ReplyMSG Flag of the online user.
 * Data3
 * IOString
 *
 * Note: If Data1 is lower than 1 or higher than the last conference
 *       available, Data1 will be set to the current conference number!
 */

#define NC_NukedFiles      246
/* Change the NukedFiles
 *
 * Data1
 * Data2
 * Data3    <- The new NukedFiles of the online user.
 * IOString
 */

#define NC_NukedBytes      247
/* Change the NukedBytes
 *
 * Data1
 * Data2
 * Data3    <- The new NukedBytes of the online user.
 * IOString
 */

#define NC_MinUpCPS        248
/* Change the Min Upload CPS
 *
 * Data1    <- The new Min Upload CPS of the online user.
 * Data2
 * Data3
 * IOString
 */

#define NC_MinDownCPS      249
/* Change the Min Download CPS
 *
 * Data1    <- The new Min Download CPS of the online user.
 * Data2
 * Data3
 * IOString
 */

#define NC_LostCarrier     250
/* Change the number of Los Carriers
 *
 * Data1    <- The new number of Los Carriers of the online user.
 * Data2
 * Data3
 * IOString
 */

#define NC_NumberOfChats   251
/* Change the number of chats
 *
 * Data1
 * Data2
 * Data3    <- The new number of chats.
 * IOString
 *
 */

#define NC_NumberOfPages   252
/* Change the number of pages
 *
 * Data1    <- The new number of pages.
 * Data2
 * Data3
 * IOString
 *
 */

#define NC_NumberOfDayPages 253
/* Change the number of day pages
 *
 * Data1    <- The new number of day pages.
 * Data2
 * Data3
 * IOString
 *
 */

#define NC_NumberOfDayRelogs 254
/* Retrieve the number of day relogins
 *
 * Data1    <- The new number of day relogins.
 * Data2
 * Data3
 * IOString
 */

#define NC_Nuked             255
/* Called from a Nuker to update the nuked bytes and files
 *
 * Data1    <- The number of files nuked.
 * Data2
 * Data3    <- The number of bytes nuked.
 * IOString
 *
 * Note: Data1 and Data3 are files and bytes nuked by the nuker and they
 *       will be added to the user's datas.
 *
 * Note: This command will also call all doors found under SYSCMD
 *       NUKED and NUKED<x>
 */

#define NC_NukedAfter        256
/* Called from a Nuker to update the nuked bytes and files
 *
 * Data1    <- The number of files nuked.
 * Data2
 * Data3    <- The number of bytes nuked.
 * IOString
 *
 * Note: Data1 and Data3 are files and bytes nuked by the nuker and they
 *       will be added to the user's datas.
 *
 * Note: This command will also call all doors found under SYSCMD
 *       NUKED and NUKED<x>, but not before the Nuker has shut down.
 */

/*   ------------------------- CF commands --------------------------------
 */

#define CF_ShowText        400
/* Shows a Textfile without Suffix
 *
 * Data1
 * Data2
 * Data3
 * IOString <- the complete path and filename of the text to be viewed.
 *
 * Note: You have to give the complete path and filename incl. the suffix!
 *       BBS:Bull.txt
 */

#define CF_ShowTextSuffix  401
/* Shows a Textfile with full Suffix
 *
 * Data1
 * Data2
 * Data3
 * IOString <- the complete path and filename of the text to be viewed.
 *
 * Note: You have to give the complete path and filename, but without suffix!
 *       BBS:Bull can be BBS:Bull.txt or BBS:Bull.ger ...
 */

#define CF_ShowTextSufLvl  402
/* Shows a Textfile with full Suffix & Level
 *
 * Data1
 * Data2
 * Data3
 * IOString <- the complete path and filename of the text to be viewed.
 *
 * Note: U have to give the complete path and filename, but without suffix!
 *       The Level will automaticly be searched and inserted!
 *       BBS:Bull can be BBS:Bull100.txt or BBS:Bull255.txt or BBS:Bull10.ger
 */

#define CF_ExecuteCommand  403
/* Execute a menu command including doors if available
 *
 * Data1
 * Data2
 * Data3
 * IOString <- The command to be executed.
 */

#define CF_InternalCmd     404
/* Execute an internal menu command
 *
 * Data1
 * Data2
 * Data3
 * IOString <- The internal menu command to be executed.
 */

#define CF_ZModemSend      405
/* Send file to the user via Zmodem protocol
 *
 * Data1
 * Data2    -> Returns the status.
 *             1  file has been transfered.
 *             0  file has been copied local to sysops download directory.
 *             -1 means xprzmodem.library not open.
 * Data3
 * IOString <- The File to be sended to the user.
 */

#define CF_ZModemReceive   406
/* Receive files via Zmodem protocol
 *
 * Data1
 * Data2    -> Returns the status. 1 means all seems to be going right.
 *             0 means xprzmodem.library not open.
 * Data3
 * IOString <- The path for the file to be sended to.
 *
 * Note: You have to give a path! Else the transfer won't begin!
 */

#define CF_ZModemSendLst   407
/* Send files to the user via Zmodem protocol from Flaglist
 *
 * Data1
 * Data2    -> Returns the status.
 *             1  file has been transfered.
 *             0  file has been copied local to sysops download directory.
 *             -1 means xprzmodem.library not open.
 * Data3
 * IOString
 * StringPtr <- A Stringpointer of nearly unlimited filenames incl. path.
 *              Between every single path/filename has to be a space.
 *              Maximum filelength of path+filename is 100 chars.
 *              Example: "RAM:File1.123 RAM:File2.123 RAM:ENV/TIME"
 */

#define CF_ReturnCommand   408
/* Execute a command/door.
 *
 * Data1
 * Data2
 * Data3
 * IOString <- The command to be executed.
 *
 * Note: To delete a previous ReturnCommand Request overwrite it with a
 *       new request, or use "" to delete it really.
 */

#define CF_SetFlagFile     409
/* Add File to the Door own Flaglist !NOT IMPLEMENTED YET!
 *
 * Data1
 * Data2
 * Data3
 * IOString
 */

#define CF_GetFlagFile     410
/* Get File from the Door own Flaglist !NOT IMPLEMENTED YET!
 *
 * Data1
 * Data2
 * Data3
 * IOString
 */

#define CF_CallersLog      411
/* add a line of text to the callers.log
 *
 * Data1
 * Data2
 * Data3
 * IOString <- The string added to the callers.log.
 */

#define CF_UDLog           412
/* add a line of text to the ud.log
 *
 * Data1
 * Data2
 * Data3
 * IOString <- The string added to the ud.log.
 */

#define CF_DoorLog         413
/* add a line of text to the Door.log
 *
 * Data1
 * Data2
 * Data3
 * IOString <- The string added to the Door.log.
 */

#define CF_CompType        414
/* Computertype change with build in selector
 *
 * Data1    <- Old or default computertype to load, 0 to let user select.
 * Data2
 * Data3
 * IOString
 */

#define CF_ModemType       415
/* Modemtype change with build in selector
 *
 * Data1    <- Old or default modemtype to load, 0 to let user select.
 * Data2
 * Data3
 * IOString
 */

#define CF_UnUsed1         416

#define CF_Language        417
/* SystemTextLanguage/TextLanguage (Suffix) changing with build in selector
 *
 * Data1    <- Old or default language to load, 0 to let user select.
 * Data2
 * Data3
 * IOString
 */

#define CF_NumLines        418
/* number of lines changing with build in selector
 *
 * Data1    <- 0 if selector should appear with prompt for numbers
 *             or higer 0 to switch to the given value without prompt.
 * Data2
 * Data3
 * IOString
 */

#define CF_ShTxtSufLvlCyc  419
/* Shows a Textfile with full Suffix & Level & Cycle
 *
 * Data1    <- 1 means use suffix .txt only. 0 means use users sys language.
 * Data2
 * Data3
 * IOString <- The path where the files are located.
 * StringPtr <- The filename itself, without path!
 * IOString -> The complete filepath and name with cyclenumber and more...
 *
 * Example: IOString<-"BBS:Screens" StringPtr<-"Bull" Data1<-"1"
 *          is trying to view a file with cycle.
 *          IOString->"BBS:Screens/1-Bull.ger" can be the result.
 *          If no cycle texts can be found the BBS tries to use:
 *          "BBS:Screens/Bull.ger" if also no Bull.ger exists the BBS tries
 *          to use "BBS:Screens/Bull.txt".
 */

#define CF_ShowTextSetable 420
/* Shows a Textfile free configurable
 *
 * Data1    <- 0 means don't cycle, else cycle.
 * Data2    <- 1 means use users language, 0 means u have to set the
 *             suffix your self!
 * Data3    <- 1 means use suffix .txt only. 0 means use users sys language.
 *             Only available if Data1 isn't 0!
 * IOString <- If Data1 isn't 0 IOString is the path where the files are
 *             located, else IOString ist the full path+filename.
 * StringPtr <- The filename itself, without path!
 *             Only available if Data1 isn't 0!
 * IOString -> The complete filepath and name (with cyclenumber and more...)
 * StructDummy1 <- "-1" means only users suffix will be added as suffix.
 *                 "-2" means .GR or if .GR not exists GR1 will be added
 *                 as suffix! This is used for Paragon Door texts.
 *                 If the value of StructDummy1 is higer than 0 (Atol()),
 *                 for example it's 10, than it will be used as the Level
 *                 like: Menu10.txt
 *                 If the value of StructDummy1 id 0 (Atol()), than the
 *                 next lower level text will be used
 *                 like: Menu300.txt or Menu255.txt. It counts down from
 *                 1000 to 0, but not every single Level! Only in 5 steps.
 *                 Note: StructDummy1 is only available if Data2 is 1!
 */

#define CF_InternReturnCmd 421
/* Execute an internal command.
 *
 * Data1
 * Data2
 * Data3
 * IOString <- The command to be executed.
 *
 * Note: To delete a previous ReturnCommand Request overwrite it with a
 *       new request, or use "" to delete it really.
 */

#define CF_DoCallersLog    422
/* Write to any <x>.log
 *
 * Data1    <- phase (type).
 * Data2    <- <x>.log type.
 * Data3
 * IOString <- The string added to the <x>.log.
 *
 * Log types <x>:
 *
 * 0 = Callers.log only.
 * 1 = UDLog
 * 2 = StartLog
 * 3 = UDLog
 *
 * Note: Callers.log will be written in ervery case!
 */

#define CF_SaveWherePhase  423
/* Save whereiam phase temporär.
 *
 * Data1
 * Data2
 * Data3
 * IOString
 */

#define CF_RestWherePhase  424
/* Restore temporär saved whereiam phase.
 *
 * Data1
 * Data2
 * Data3
 * IOString
 */

#define CF_FileCopyMove    425
/* Copy or move a file.
 *
 * Data1    <- if 0 then copy, else move file.
 * Data2    -> Returncode. If false then there was an error.
 * Data3
 * IOString <- Source file.
 * StringPtr -> Destination file.
 */

#define CF_SysOpChat       426
/* Jump into SysOp-Chat.
 *
 * Data1    <- if 0 last line will be restored.
 * Data2    <- if TRUE shifted chat.
 * Data3    <- if TRUE no screen to front will be done, else it will.
 * IOString
 * StringPtr
 *
 * Note: Data1 indicates (if 0) that the last displayed string on screen
 *       before the chat will be called is buffered and shall be re-
 *       displayed after chatting.
 *       Often useful for buffering input prompts.
 *
 * Note: Data2 emulates (if TRUE) a chat like with SHIFT-F1 called.
 *       If FALSE it will be a chat like with F1 alone.
 *
 * Note: If the user is already in SysOp-Chat it's not possible to go
 *       in chat again. It's only posible if no SysOp-Chat runs.
 *
 *       If it was not possible to go into SysOp-Chat because the user
 *       is already in chat the command will return with:
 *       ReturnCode = 1 (Comand not successfull executed).
 */

#define CF_SpecialCmd      427
/* Execute a special door command (BBSCMD, SYSCMD or CONF<X>CMD).
 *
 * Data1    <- Command type.
 * Data2
 * Data3
 * IOString <- The door command to be executed.
 * StringPtr <- The arguments for the door.
 *
 * Note: Data1 can have the following numbers:
 *
 *       - If Data1 > 0 it's CONF<Data1>CMD .
 *       - If Data1 = -1 it's BBSCMD .
 *       - If Data1 = -2 it's SYSCMD .
 */

#define CF_GetUserConfXS   428
/* Get a string containing the conference access of a special user.
 *
 * Data1    <- The usernumber of the user you want to get the conf xs from.
 * Data2
 * Data3
 * IOString
 * StringPtr -> A buffer filled with the conference access.
 *
 * Note: Check fdom_ReturnCode for a failure!
 *
 * Note: StringPtr will be an allocated buffer which will be freed from
 *       MainPart if you door exists.
 *
 * Note: StringPtr contains the conference access of you user in the
 *       following form: "X__X__X_X_"... The first char (byte 0) will be
 *       Conference 1 the second (byte 1) Conference 2 and so on.
 *       There is *NO* relative Conference here! They are physical from
 *       1 till the last Conference available. You have to check yourself
 *       if the Conference is in the relative list if you need this.
 *       For this you can use NR_GetRelativeStatus to see if relative
 *       Conferences is activated and NR_AbsToRel to get the releative
 *       number of an absolute (real/physical) one.
 *
 * Note: X means that the user has access to this Conference.
 *       _ means that the user hasn't access to this Conference.
 *       Please check always for X and not _ because X is the relevant one
 *       and everything which is not X is _ and means no access.
 */

/*   ------------------------- SR commands --------------------------------
 */

#define SR_ConfName        600
/* Retrieve the conference name
 *
 * Data1    <- The conference number.
 * Data2
 * Data3
 * IOString -> The requested conferencename.
 *
 * Note: If Data1 is lower than 1 or higher than the last conference
 *       available, Data1 will be set to the current conference number!
 */

#define SR_ConfLocation    601
/* Retrieve the conference location
 *
 * Data1    <- The conference number.
 * Data2
 * Data3
 * IOString -> The requested conferencelocation.
 *
 * Note: If Data1 is lower than 1 or higher than the last conference
 *       available, Data1 will be set to the current conference number!
 */

#define SR_ConfNum         602
/* Retrieve the current conference number
 *
 * Data1
 * Data2    -> The actual conferencenumber.
 * Data3    -> The relative conferencenumber.
 * IOString
 *
 * Note: Normaly if the relative conferencenumber is 0 the user has no access
 *       to this conference, but this can't happen here.
 */

#define SR_BBSLocation     603
/* Retrieve the current BBS location
 *
 * Data1
 * Data2
 * Data3
 * IOString -> The BBS Location.
 */

#define SR_Status          604
/* Retrieve the current status of the node
 *
 * Data1
 * Data2
 * Data3
 * IOString -> Contains the status.
 */

#define SR_ScreenAdress    605
/* Retrieve the screen address
 *
 * Data1
 * Data2
 * Data3
 * IOString -> Contains the hex screenadress.
 */

#define SR_TaskPri         606
/* Retrieve the priority the node is running at
 *
 * Data1
 * Data2    -> Contains the Nodepriority.
 * Data3
 * IOString
 */

#define SR_RawScreenAdress 607
/* Retrieve the screen address of the node
 *
 * Data1
 * Data2    -> Contains the raw screenadress.
 * Data3
 * IOString
 */

#define SR_FAMEVersion     608
/* Retrieve the current version string of FAME
 *
 * Data1
 * Data2    -> Contains the versionnumber of FAME.
 * Data3    -> Contains the revsionnumber of FAME.
 * IOString -> Contains the versionstring of FAME.
 */

#define SR_ChatSet         609
/* Retrieve the chat status
 *
 * Data1
 * Data2    -> Contains the Chat flag.
 * Data3
 * IOString
 */

#define SR_ENVStat         610
/* Retrieve the current env stat variable code
 *
 * Data1
 * Data2    -> Contains the env stat variable code.
 * Data3
 * IOString
 */

#define SR_NodeDevice      611
/* Retrieve the node device name
 *
 * Data1
 * Data2
 * Data3
 * IOString -> Contains the devicename.
 */

#define SR_NodeUnit        612
/* Retrieve the node unit number
 *
 * Data1
 * Data2    -> Contains the unit number of the used device.
 * Data3
 * IOString
 */

#define SR_NodeBaud        613
/* Retrieve the initialized baud rate of the node
 *
 * Data1
 * Data2    -> Contains the initial baud rate of the node.
 * Data3
 * IOString
 */

#define SR_NodeNumber      614
/* Retrieve the node nummber
 *
 * Data1
 * Data2    -> Contains the nodenumber of this node.
 * Data3
 * IOString
 */

#define SR_MCI             615
/* Send mci text to FAME
 *
 * Data1    <- If non zero a return will be done at the end.
 * Data2    -> The Returnvalue. 0 everything is ok, non zero means
 *             there was an error.
 * Data3
 * IOString <- The MCI-string to be interpreted.
 */

#define SR_GetTask         616
/* Finds the current nodes task address
 *
 * Data1
 * Data2
 * Data3
 * IOString
 * StructDummy1 -> Is the (struct Process *) of this Node.
 */

#define SR_NodeBaudRate    617
/* Retrieve the current users connect rate
 *
 * Data1
 * Data2    -> Contains the connect rate.
 * Data3
 * IOString
 */

#define SR_LogonType       618
/* Retrieve the LOGONTYPE
 *
 * Data1
 * Data2    -> Contains the logontype.
 * Data3
 * IOString
 */

#define SR_ScrLeft         619
/* Retrieve the screen coordinates
 *
 * Data1
 * Data2    -> Contains the screenleft.
 * Data3
 * IOString
 */

#define SR_ScrTop          620
/* Retrieve the screen coordinates
 *
 * Data1
 * Data2    -> Contains the screentop.
 * Data3
 * IOString
 */

#define SR_ScrWidth        621
/* Retrieve the screen coordinates
 *
 * Data1
 * Data2    -> Contains the screenwidth.
 * Data3
 * IOString
 */

#define SR_ScrHeight       622
/* Retrieve the screen coordinates
 *
 * Data1
 * Data2    -> Contains the screenheight.
 * Data3
 * IOString
 */

#define SR_PurgeLine       623
/* Abort serial input
 *
 * Data1
 * Data2
 * Data3
 * IOString
 *
 * No Data needed.
 */

#define SR_NonStopText     624
/* Retrieve the NONSTOP text scrolling flag
 *
 * Data1
 * Data2    -> Contains the nonstop flag.
 * Data3
 * IOString
 */

#define SR_GoodFile        625
/* Retrieve the results of a tested file after upload
 *
 * Data1
 * Data2    -> Contains the filestatus.
 * Data3
 * IOString
 */

#define SR_LastAccountNum  626
/* Retrieve the Last Account number
 *
 * Data1
 * Data2    -> Contains the number of the last available account.
 * Data3
 * IOString
 */

#define SR_PurgeLineStart  627
/* Begin serial input Abort
 *
 * Data1
 * Data2
 * Data3
 * IOString
 *
 * No Data needed.
 */

#define SR_PurgeLineEnd    628
/* End serial input Abort
 *
 * Data1
 * Data2
 * Data3
 * IOString
 *
 * No Data needed.
 */

#define SR_BBSOrigin       629
/* Retrieve the BBS Origin
 *
 * Data1
 * Data2
 * Data3
 * IOString -> Contains the BBS Origin.
 */

#define SR_DefNumLines     630
/* Retrieve the default text number of lines
 *
 * Data1
 * Data2    -> Contains the default text number of lines.
 * Data3
 * IOString
 */

#define SR_NumberOfNodes   631
/* Retrieve the number of Nodes
 *
 * Data1
 * Data2    -> Contains the number of Nodes.
 * Data3
 * IOString
 */

#define SR_FileDescUndLine 632
/* Retrieve the File Decsription underline
 *
 * Data1
 * Data2
 * Data3
 * IOString -> Contains the Decsription underline.
 */

#define SR_NumberOfConfs   633
/* Retrieve the number of Conferences
 *
 * Data1
 * Data2    -> Contains the number of Conferences.
 * Data3
 * IOString
 */

#define SR_ConfNameLoc     634
/* Retrieve the conference name and conference location
 *
 * Data1    <- The conference number.
 * Data2
 * Data3
 * IOString -> The requested conferencename.
 * StringPtr -> The requested conferencelocation.
 * StructDummy1 -> The requested conferencename.
 * StructDummy2 -> The requested conferencelocation.
 *
 * Note: If Data1 is lower than 1 or higher than the last conference
 *       available, Data1 will be set to the current conference number!
 *
 * Note: You have to allocate memory for StructDummy1 and StructDummy2.
 *       For conferencename (StructDummy1) you need 32 bytes and for
 *       conferencelocation (StructDummy2) you need 102 bytes of memory
 *       (zero-bytes are already included).
 *       The easyest way to do this is if you have two strings long enough
 *       to which you can let point StructDummy1 and StructDummy1 before
 *       you call this command. If you don't do this, you damage foreign
 *       memory and this will normaly cause a guru.
 */

#define SR_FAMEDataFileVers 635
/* Retrieve the current Datafile version string of FAME
 *
 * Data1
 * Data2    -> Contains the versionnumber used to check for fileheaders.
 * Data3    -> Contains the revsionnumber used to check for fileheaders.
 * IOString -> Contains the Datafile version string of FAME.
 */

#define SR_ConnectString    636
/* Retrieve the current users connect string
 *
 * Data1
 * Data2
 * Data3
 * IOString -> Contains the connect string.
 */

/*   ------------------------- SC commands --------------------------------
 */

#define SC_UnUsed1         700

#define SC_UnUsed2         701

#define SC_ChatSet         702
/* Change the chat status
 *
 * Data1    <- 0 means no chat, else chat.
 * Data2
 * Data3
 * IOString
 */

#define SC_ENVStat         703
/* Change the current env stat variable code
 *
 * Data1    <- the new env stat variable code.
 * Data2
 * Data3
 * IOString
 */

#define SC_NonStopText     704
/* Change the NONSTOP text scrolling flag
 *
 * Data1    <- 0 means no NONSTOP text, else NONSTOP text.
 * Data2
 * Data3
 * IOString
 */

#define SC_DefNumLines     705
/* Change the default text number of lines
 *
 * Data1    <- The new default text number of lines.
 * Data2
 * Data3
 * IOString
 */

#define SC_FileDescUndLine 706
/* Change the File Decsription underline
 *
 * Data1
 * Data2
 * Data3
 * IOString <- The new File Decsription underline (max. 45 chars).
 */

#define SC_GoodFile        707
/* Set the results of a tested file after upload
 *
 * Data1    <- The new filestatus.
 * Data2
 * Data3
 * IOString
 *
 * -1 means the file failed the filetest.
 *  0 means the file passed the filetest.
 *  1 means the file was not tested.
 */


/*   ------------------------- AR commands --------------------------------
 */

#define AR_GetKey          800
/* Look for a keypress without waiting for it
 *
 * Data1
 * Data2    -> 0 means nothing happens, 1 means a key was pressed!
 * Data3    -> 0 = Console char, 1 = Serial char.
 * IOString
 *
 * Note: AR_GetKey is the same as NR_HotKey, but on NR_HotKey you will
 *       get the char, while AR_GetKey notifies only that a char was
 *       present. Usable for a simple "Press a key" prompt.
 */

#define AR_WaitRAWChar     801
/* Get a char with waiting for it, also get ALL REAL RAW commands!
 *
 * Data1
 * Data2    -> The char typed be the user.
 * Data3    -> 0 = Console char, 1 = Serial char.
 * IOString <- String will be displayed to the user.
 *
 * Note: U have to check your self if the Rawkeycode is a FKey/Shift-FKey
 *       or the Helpkey. If such a key appears, you have to use the next
 *       command: AR_EmulateFKeyHelp
 */

#define AR_EmulateFKeyHelp 802
/* Emulates a pressed FKey/Shift-FKey/Help on Con
 *
 * Data1    <- Thy keytype.
 * Data2
 * Data3
 * IOString
 *
 * Note: Data1 can have the following values:
 *       1-10 means F1-F10, 11-20 means ShiftF1-ShiftF10, 21 means Help.
 *
 * Note: If ReturnCode is 1, the node is iconified and this command
 *       won't be executed. This is because of more security, because
 *       it's only an emulation of keys pressed by the SysOp and not for
 *       other things.
 */

#define AR_GetCharHex      803
/* Retrieve the Char pressed in HEX  !NOT IMPLEMENTED YET!
 *
 * Data1
 * Data2
 * Data3
 * IOString
 */

#define AR_EditFile        804
/* Edit every File with the internal Texteditor
 *
 * Data1
 * Data2    -> Returncode: 0 successful, -2 aborted by user.
 * Data3
 * IOString <- The file to be edit.
 */

#define AR_Dump            805
/* dump the user's data structure to a specified file
 *
 * Data1
 * Data2
 * Data3
 * IOString <- The file where the user data structure should be generated.
 */

#define AR_UserStatus      806
/* Retrieve the Userstatus (Aktiv/Deleted/InAktive)
 *
 * Data1
 * Data2    -> The user status flag.
 * Data3
 * IOString
 */

#define AR_NewScan         807
/* Retrieve the NewScan Flag
 *
 * Data1    <- The conferencenumber from which u want to get the NewScan flag.
 * Data2    -> 1 if NewScan is on, 0 if off.
 * Data3
 * IOString
 *
 * Note: If Data1 is lower than 1 or higher than the last conference
 *       available, Data1 will be set to the current conference number!
 */

#define AR_Hacks           808
/* Retrieve the Hackattemps
 *
 * Data1
 * Data2    -> Contains the User's number of hacks.
 * Data3
 * IOString
 */

#define AR_LastConf        809
/* Retrieve the Last Conf which was joined
 *
 * Data1
 * Data2
 * Data3    -> Contains the User's last conference on.
 * IOString
 */

#define AR_ConfReJoin      810
/* Retrieve the Conf which should every time be joined on logon
 *
 * Data1
 * Data2
 * Data3    -> Contains the User's number of confjoins.
 * IOString
 */

#define AR_HighUpCPS       811
/* Retrieve the highest CPS on Uploads
 *
 * Data1
 * Data2    -> Contains the User's highest CPS on Uploads.
 * Data3
 * IOString
 */

#define AR_HighDownCPS     812
/* Retrieve the highest CPS on Downloads
 *
 * Data1
 * Data2    -> Contains the User's highest CPS on Downloads.
 * Data3
 * IOString
 */

#define AR_Baud            813
/* Retrieve the Baudrate at the last Logon
 *
 * Data1
 * Data2    -> Contains the User's Baudrate at the last Logon.
 * Data3
 * IOString
 */

#define AR_MsgCLS          814
/* Retrieve the ClearScreen on Msg's flag
 *
 * Data1
 * Data2    -> Contains the User's ClearScreen on Msg's flag.
 * Data3
 * IOString
 */

#define AR_FileCLS         815
/* Retrieve the ClearScreen on Filelists flag
 *
 * Data1
 * Data2    -> Contains the User's ClearScreen on Filelists flag.
 * Data3
 * IOString
 */

#define AR_UGlobal         816
/* Retrieve the global upload flag
 *
 * Data1
 * Data2    -> Contains the User's global upload flag.
 * Data3
 * IOString
 */

#define AR_DGlobal         817
/* Retrieve the global download flag
 *
 * Data1
 * Data2    -> Contains the User's global download flag.
 * Data3
 * IOString
 */

#define AR_FileBase        818
/* Retrieve the access to the User File Base
 *
 * Data1
 * Data2    -> Contains the User's access to the User File Base.
 * Data3
 * IOString
 */

#define AR_Hide            819
/* Retrieve the hide flag
 *
 * Data1
 * Data2    -> Contains the User's hide flag.
 * Data3
 * IOString
 */

#define AR_MsgRooming      820
/* Retrieve the Msgrooming flag
 *
 * Data1
 * Data2    -> Contains the User's Msgrooming flag.
 * Data3
 * IOString
 */

#define AR_StringEdit      821
/* Retrieve the StringEdit flag
 *
 * Data1
 * Data2    -> Contains the User's StringEdit flag.
 * Data3
 * IOString
 */

#define AR_CryptPW         822
/* Retrieve the CryptPW
 *
 * Data1
 * Data2
 * Data3
 * IOString -> Contains the User's CryptPW.
 */

#define AR_CryptFlag       823
/* Retrieve the CryptFlag
 *
 * Data1
 * Data2    -> Contains the User's Crypt flag.
 * Data3
 * IOString
 */

#define AR_UserFileBPW     824
/* Retrieve the UserFileBasePW
 *
 * Data1
 * Data2
 * Data3
 * IOString -> Contains the User's UserFileBasePW.
 */

#define AR_DropDTR         825
/* Drop carrier on a user
 *
 * Data1
 * Data2
 * Data3
 * IOString
 *
 * No data needed.
 */

#define AR_Userlimit       826
/* Retrieve the Userlimit
 *
 * Data1
 * Data2
 * Data3    -> Contains the BBS Userlimit.
 * IOString
 */

#define AR_MaxNameFailure  827
/* Retrieve the MaxNameFailure
 *
 * Data1
 * Data2    -> Contains the BBS MaxNameFailure.
 * Data3
 * IOString
 */

#define AR_MaxUserPWFail   828
/* Retrieve the MaxUserPWFail
 *
 * Data1
 * Data2    -> Contains the BBS MaxUserPWFail.
 * Data3
 * IOString
 */

#define AR_MaxSysPWFailure 829
/* Retrieve the MaxSysPWFailure
 *
 * Data1
 * Data2    -> Contains the BBS MaxSysPWFailure.
 * Data3
 * IOString
 */

#define AR_MaxNUPFailure   830
/* Retrieve the MaxNUPFailure
 *
 * Data1
 * Data2    -> Contains the BBS MaxNUPFailure.
 * Data3
 * IOString
 */

#define AR_MaxUserHacks    831
/* Retrieve the MaxUserHacks
 *
 * Data1
 * Data2    -> Contains the BBS MaxUserHacks.
 * Data3
 * IOString
 */

#define AR_ScreensPath     832
/* Retrieve the ScreensPath
 *
 * Data1
 * Data2
 * Data3
 * IOString -> Contains the BBS ScreensPath.
 */

#define AR_SysPWPrompt     833
/* Retrieve the SysPWPrompt
 *
 * Data1
 * Data2
 * Data3
 * IOString -> Contains the BBS SysPWPrompt.
 */

#define AR_NewUserPWPrompt 834
/* Retrieve the NewUserPWPrompt
 *
 * Data1
 * Data2
 * Data3
 * IOString -> Contains the BBS NewUserPWPrompt.
 */

#define AR_UserNamePrompt  835
/* Retrieve the UserNamePrompt
 *
 * Data1
 * Data2
 * Data3
 * IOString -> Contains the BBS UserNamePrompt.
 */

#define AR_UserPWPrompt    836
/* Retrieve the UserPWPrompt
 *
 * Data1
 * Data2
 * Data3
 * IOString -> Contains the BBS UserPWPrompt.
 */

#define AR_PausePrompt     837
/* Retrieve the PausePrompt
 *
 * Data1
 * Data2
 * Data3
 * IOString -> Contains the BBS PausePrompt.
 */

#define AR_SysOpChatColor  838
/* Retrieve the SysOpChatColor
 *
 * Data1
 * Data2
 * Data3
 * IOString -> Contains the BBS SysOpChatColor.
 */

#define AR_UserChatColor   839
/* Retrieve the UserChatColor
 *
 * Data1
 * Data2
 * Data3
 * IOString -> Contains the BBS UserChatColor.
 */

#define AR_UploadPathI     840
/* Retrieve the UploadPathI
 *
 * Data1
 * Data2
 * Data3
 * IOString -> Contains the UploadPathI of the actual conference.
 */

#define AR_DownloadPathI   841
/* Retrieve the DownloadPathI
 *
 * Data1
 * Data2
 * Data3
 * IOString -> Contains the DownloadPathI of the actual conference.
 */

#define AR_AdditioUlPaths  842
/* Retrieve the AdditioUlPaths
 *
 * Data1
 * Data2
 * Data3
 * IOString -> Contains the AdditioUlPaths of the actual conference.
 */

#define AR_AdditioDlPaths  843
/* Retrieve the AdditioDlPaths
 *
 * Data1
 * Data2
 * Data3
 * IOString -> Contains the AdditioDlPaths of the actual conference.
 */

#define AR_NumberofDirs    844
/* Retrieve the NumberofDirs
 *
 * Data1    <- The conference number.
 * Data2    -> Contains the number of dirs of the actual conference.
 * Data3
 * IOString
 *
 * Note: If Data1 is lower than 1 or higher than the last conference
 *       available, Data1 will be set to the current conference number!
 */

#define AR_GiveUlBytes     845
/* Retrieve the GiveUlBytes
 *
 * Data1
 * Data2    -> Contains the number of Ul bytes given of the actual conference.
 * Data3
 * IOString
 */

#define AR_GiveDlBytes     846
/* Retrieve the GiveDlBytes
 *
 * Data1
 * Data2    -> Contains the number of Ul bytes given of the actual conference.
 * Data3
 * IOString
 */

#define AR_FlagFilePath    847
/* Add a file on a Conffileflaglist with or without path
 *
 * Data1    <- The conference number.
 * Data2    <- FreeDl Flag. If non 0 it will be a FreeDownloadfile.
 * Data3    -> Errormsg 0 on success.
 * IOString <- The Filename
 * StringPtr <- The Path for the file, "" if no path is known and the BBS
 *              should find a path for the file.
 *
 * Note: U can flag here also filenames longer than 12 chars.
 *       Filenames can be long up to 31 chars.
 *       The Filepath can be long up to 101 chars.
 *
 * Note: If Data1 is lower than 1 or higher than the last conference
 *       available, Data1 will be set to the current conference number!
 *
 * Note: Data3 is errormsg:
 *       0  means successfull flagged.
 *       1  means file allready flagged.
 *       2  means length of IOString is lower than  byte.
 *       -1 means list element memory allocation faild.
 *       -2 means given conference not found, normaly not happens here,
 *                because of Data1 check.
 */

#define AR_StringToNode    848
/* Send a Textstring to a Node (dependent of users settings "W 18")
 *
 * Data1    <- The Node to which the String should go.
 * Data2
 * Data3
 * IOString <- The String displayed at the Node.
 *
 * Note: If ReturnCode=1 the String can't be send to this Node,
 *       because this Node doesn't exists.
 *       Use NR_ActiveNode to check if the Node exists.
 *       AR_StringToNode dependent of users settings "W 18", that means
 *       only on some positions in the Node the Message can be directly
 *       received. If the User don't want to get Messages at some actions
 *       in the BBS, the Message will be added to a list which will be
 *       displayed to the User when he comes to a position where he will or
 *       must receive the messages. Every user *MUST* receive them on idle
 *       mode (Menuprompt).
 */

#define AR_GetNodeEnv      849
/* Retrieve the status of a Node
 *
 * Data1    <- The Node u mean.
 * Data2    -> The status of the Node. Use this one to get a real result!
 * Data3    -> The full last Node to Server Command, better u doesn't use it!
 * IOString -> The Actionstring displayed at the Server.
 */

#define AR_FullEdStruct    850
/* Retrieve the (filled) struct for external MsgEd
 *
 * Data1
 * Data2
 * Data3
 * IOString
 * StructDummy1 -> Pointer to struct ExternEditor.
 */

#define AR_SendStr         851
/* Sends a non length limited String to the User
 *
 * Data1    <- if not 0 a "\r\n" combination will be send after the string
 *             has been send.
 * Data2
 * Data3
 * IOString
 * StringPtr <- The string to be send to the user.
 */

#define AR_SendStrCon      852
/* Sends a non length limited String to the Console only
 *
 * Data1    <- if not 0 a "\r\n" combination will be send after the string
 *             has been send.
 * Data2
 * Data3
 * IOString
 * StringPtr <- The string to be send to the console.
 */

#define AR_SendStrSer      853
/* Sends a non length limited String to the Serial only
 *
 * Data1    <- if not 0 a "\r\n" combination will be send after the string
 *             has been send.
 * Data2
 * Data3
 * IOString
 * StringPtr <- The string to be send to the serial.
 */

#define AR_ResetNumFlags   854
/* Reset NumFlagList
 *
 * Data1
 * Data2
 * Data3   -> Returncode.
 * IOString
 *
 * Note: Data3 must be true or an internal error has been detected.
 *       In this case you musn't use AR_SetNumFlag and AR_GetNumFlag!!!
 */

#define AR_SetNumFlag      855
/* Flag an entry into the NumFlagList
 *
 * Data1    <- The number of the new entry. It's on you to count it correctly!
 * Data2
 * Data3    -> Returncode.
 * IOString <- The entry (FileName).
 *
 * Note: Data3 is the following returncode:
 *        0 means success.
 *       -1 means allocation failed.
 *       -2 means Base not initialized.
 */

#define AR_GetNumFlag      856
/* Get an entry from the NumFlagList
 *
 * Data1    <- The number of the entry you want to have.
 * Data2
 * Data3    -> Returncode.
 * IOString -> The entry (FileName).
 *
 * Note: Data3 is the following returncode:
 *        0 means success.
 *       -1 means entry number not found.
 *       -2 means Base not initialized.
 */

#define AR_NodeVersion     857
/* Get the version and revision numbers of a specific Node.
 *
 * Data1    <- The Node number
 * Data2    -> The version of this Node.
 * Data3    -> The revision of this Node.
 * IOString
 */

#define AR_NodeStartTime   858
/* Get the start time of a specific Node.
 *
 * Data1    <- The Node number
 * Data2    -> The start time/date (unix) of this Node.
 * Data3
 * IOString -> The start time/date (string) of this Node.
 */

#define AR_ServerVersion   859
/* Get the version and revision numbers of the Server.
 *
 * Data1
 * Data2    -> The version of the Server.
 * Data3    -> The revision of the Server.
 * IOString
 */

#define AR_ServerStartTime 860
/* Get the start time of the Server.
 *
 * Data1
 * Data2    -> The start time/date (unix) of the Server.
 * Data3
 * IOString -> The start time/date (string) of the Server.
 */

#define AR_HotKey          861
/* Gets a char without waiting for it, get also self definable cursor keys.
 *
 * Data1    <- Cursor key begining return code. (If 0 they begin with 2)
 * Data2    -> The char typed by the user.
 * Data3    -> 0 = Console char, 1 = Serial char.
 * IOString
 *
 * Note: AR_HotKey is the same as NR_HotKey, but like NR_WaitChar it
 *       returns also cursor keys. Beware, the cursor key returns
 *       are maybe others than with NR_WaitChar!!!
 *
 * Note: You get all ascii codes also RAW codes, but only the CSI codes and
 *       escape-sequences will be filtered, because the BBS has to check for
 *       FKeys and more.
 *       You will get the cursorkeys as the following values:
 *       Data1+2 for UP, Data1+3 for DOWN, Data1+1 for RIGHT and Data1 for LEFT.
 *
 *       Defination: LEFT  = 0
 *                   RIGHT = 1
 *                   UP    = 3
 *                   DOWN  = 2
 *
 *       You now have to add the defines above to the returncode to see
 *       if the result is a cursor key.
 *
 *       If you specify that LEFT (first cursor key) begins with 160
 *       (set Data1 to 160) the cursorkeys will be the following ones:
 *
 *       CURSOR LEFT  = 160
 *       CURSOR RIGHT = 161
 *       CURSOR UP    = 163
 *       CURSOR DOWN  = 162
 */

#define AR_DropDtrOnNode   862
/* Drop the carrier on a Node.
 *
 * Data1    <- The Node where the carrier shall be droped.
 * Data2
 * Data3
 * IOString
 */

/*   ------------------------- AC commands --------------------------------
 */

#define AC_UserStatus      900
/* Cange the Userstatus (Aktiv/Deleted/InAktive)
 *
 * Data1    <- The new Userstatus.
 * Data2
 * Data3
 * IOString
 */

#define AC_NewScan         901
/* Change the NewScan Flag
 *
 * Data1    <- The conferencenumber from which u want to get the NewScan flag.
 * Data2    <- 1 if NewScan should be set to on, 0 for off.
 * Data2
 * Data3
 * IOString
 */

#define AC_HackCount       902
/* Resets the Hackcounter
 *
 * Data1    <- The new hackcounter.
 * Data2
 * Data3
 * IOString
 */

#define AC_LastConf        903
/* Change the Last Conf which was joined
 *
 * Data1
 * Data2
 * Data3    <- The new last conf joined.
 * IOString
 */

#define AC_ConfReJoin      904
/* Change the Conf which should every time be joined on logon
 *
 * Data1
 * Data2
 * Data3    <- The new last conf which should every time be joined on logon.
 * IOString
 */

#define AC_HighUpCPS       905
/* Change the highest CPS on Uploads
 *
 * Data1    <- The new highest CPS on Uploads.
 * Data2
 * Data3
 * IOString
 */

#define AC_HighDownCPS     906
/* Change the highest CPS on Downloads
 *
 * Data1    <- The new highest CPS on Downloads.
 * Data2
 * Data3
 * IOString
 */

#define AC_Baud            907
/* Change the Baudrate at the last Logon
 *
 * Data1    <- The new Baudrate at the last Logon.
 * Data2
 * Data3
 * IOString
 */

#define AC_MsgCLS          908
/* Change the ClearScreen on Msg's flag
 *
 * Data1    <- The new ClearScreen on Msg's flag.
 * Data2
 * Data3
 * IOString
 */

#define AC_FileCLS         909
/* Change the ClearScreen on Filelists flag
 *
 * Data1    <- The new ClearScreen on Filelists flag.
 * Data2
 * Data3
 * IOString
 */

#define AC_UGlobal         910
/* Change the global upload flag
 *
 * Data1    <- The new global upload flag.
 * Data2
 * Data3
 * IOString
 */

#define AC_DGlobal         911
/* Change the global download flag
 *
 * Data1    <- The new global download flag.
 * Data2
 * Data3
 * IOString
 */

#define AC_FileBase        912
/* Change the access to the User File Base
 *
 * Data1    <- The new access to the User File Base.
 * Data2
 * Data3
 * IOString
 */

#define AC_Hide            913
/* Change the hide flag
 *
 * Data1    <- The new hide flag.
 * Data2
 * Data3
 * IOString
 */

#define AC_MsgRooming      914
/* Change the Msgrooming flag
 *
 * Data1    <- The new Msgrooming flag.
 * Data2
 * Data3
 * IOString
 */

#define AC_StringEdit      915
/* Change the StringEdit flag
 *
 * Data1    <- The new StringEdit flag.
 * Data2
 * Data3
 * IOString
 */

#define AC_CryptPW         916
/* Change the CryptPW
 *
 * Data1
 * Data2
 * Data3
 * IOString <- The new CryptPW (max 11 chars).
 */

#define AC_CryptFlag       917
/* Change the CryptFlag
 *
 * Data1    <- The new CryptFlag.
 * Data2
 * Data3
 * IOString
 */

#define AC_UserFileBPW     918
/* Change the UserFileBasePW
 *
 * Data1
 * Data2
 * Data3
 * IOString <- The new UserFileBasePW.
 */

#define AC_Userlimit       919
/* Change the Userlimit
 *
 * Data1
 * Data2
 * Data3    <- The new BBS Userlimit.
 * IOString
 */

#define AC_MaxNameFailure  920
/* Change the MaxNameFailure
 *
 * Data1    <- The new BBS MaxNameFailure.
 * Data2
 * Data3
 * IOString
 */

#define AC_MaxUserPWFail   921
/* Change the MaxSysPWFailure
 *
 * Data1    <- The new BBS MaxSysPWFailure.
 * Data2
 * Data3
 * IOString
 */

#define AC_MaxSysPWFailure 922
/* Change the MaxNUPFailure
 *
 * Data1    <- The new BBS MaxNUPFailure.
 * Data2
 * Data3
 * IOString
 */

#define AC_MaxNUPFailure   923
/* Change the MaxNUPFailure
 *
 * Data1    <- The new BBS MaxNUPFailure.
 * Data2
 * Data3
 * IOString
 */

#define AC_MaxUserHacks    924
/* Change the MaxUserHacks
 *
 * Data1    <- The new BBS MaxUserHacks.
 * Data2
 * Data3
 * IOString
 */

#define AC_ScreensPath     925
/* Change the ScreensPath
 *
 * Data1
 * Data2
 * Data3
 * IOString <- The new BBS ScreensPath.
 */

#define AC_SysPWPrompt     926
/* Change the SysPWPrompt
 *
 * Data1
 * Data2
 * Data3
 * IOString <- The new BBS SysPWPrompt.
 */

#define AC_NewUserPWPrompt 927
/* Change the NewUserPWPrompt
 *
 * Data1
 * Data2
 * Data3
 * IOString <- The new BBS NewUserPWPrompt.
 */

#define AC_UserNamePrompt  928
/* Change the UserNamePrompt
 *
 * Data1
 * Data2
 * Data3
 * IOString <- The new BBS UserNamePrompt.
 */

#define AC_UserPWPrompt    929
/* Change the UserPWPrompt
 *
 * Data1
 * Data2
 * Data3
 * IOString <- The new BBS UserPWPrompt.
 */

#define AC_PausePrompt     930
/* Change the PausePrompt
 *
 * Data1
 * Data2
 * Data3
 * IOString <- The new BBS PausePrompt.
 */

#define AC_SysOpChatColor  931
/* Change the SysOpChatColor
 *
 * Data1
 * Data2
 * Data3
 * IOString <- The new BBS SysOpChatColor.
 */

#define AC_UserChatColor   932
/* Change the UserChatColor
 *
 * Data1
 * Data2
 * Data3
 * IOString <- The new BBS UserChatColor.
 */

#define AC_NumberofDirs    933
/* Change the NumberofDirs
 *
 * Data1    <- The new BBS NumberofDirs.
 * Data2
 * Data3
 * IOString
 */

#define AC_GiveUlBytes     934
/* Change the GiveUlBytes
 *
 * Data1    <- The new BBS givven Ul bytes.
 * Data2
 * Data3
 * IOString
 */

#define AC_GiveDlBytes     935
/* Change the GiveDlBytes
 *
 * Data1    <- The new BBS given Dl bytes.
 * Data2
 * Data3
 * IOString
 */

#define AC_ServerAction    936
/* Write action string to the server nodeline.
 *
 * Data1
 * Data2
 * Data3
 * IOString <- The string to the server.
 */

#define AC_ServerActionCPS 937
/* Write action string and Baud/CPS to the server nodeline.
 *
 * Data1    <- The Baud/CPS to the server.
 * Data2
 * Data3
 * IOString <- The string to the server.
 */


/*   ------------------------- RD commands --------------------------------
 */

#define RD_ShowFlags       1000
/* Shows the Flaglist of the current Conf  !NOT IMPLEMENTED YET!
 *
 * Data1
 * Data2
 * Data3
 * IOString
 */

#define RD_ShowAllFlags    1001
/* Shows the Flaglists of all Conf's  !NOT IMPLEMENTED YET!
 *
 * Data1
 * Data2
 * Data3
 * IOString
 */

#define RD_ConfigLoad      1002
/* Reload all Configdatas  !NOT IMPLEMENTED YET!
 *
 * Data1
 * Data2
 * Data3
 * IOString
 */

#define RD_IconifyIs       1003
/* Retrieve Iconify Status
 *
 * Data1
 * Data2    -> The iconify status flag.
 * Data3
 * IOString
 */

#define RD_Iconify         1004
/* Change to Iconify or UnIconify
 *
 * Data1
 * Data2
 * Data3
 * IOString
 *
 * No datas needed, because this only toggles.
 */

#define RD_Spy             1005
/* Retrieve or toggles the Spy flag  !NOT IMPLEMENTED YET!
 *
 * Data1
 * Data2
 * Data3
 * IOString
 */

#define RD_NewUser         1006
/* Retrieve or toggles the NewUser flag
 *
 * Data1    <- On 0 retrieve, else set the NewUser flag.
 * Data2    -> If Data1=0 the NewUser flag.
 * Data3    <- If Data1=1 the new NewUser flag.
 * IOString
 */

#define RD_LoadAccount     1007
/* Load Userdatas
 *
 * Data1    <- The usernumber of the user you want to load.
 * Data2    -> An internal error code.
 * Data3
 * IOString
 * StructDummy1 -> The Userdata structure.
 * StructDummy2 -> The Userkey structure.
 */

#define RD_SaveAccount     1008
/* Write Userdatas back
 *
 * Data1    <- The usernumber of the user you want to save.
 * Data2
 * Data3
 * IOString
 * StructDummy1 <- The Userdata structure.
 * StructDummy2 <- The Userkey structure.
 */

#define RD_LoadConfDat     1009
/* Write UserCnf.data back
 *
 * Data1    <- The usernumber of the user from whom you want to load the aktual
 *             conf's Userconfdata structure.
 * Data2    <- The selected conference.
 * Data3
 * IOString
 * StructDummy1 -> The Userconfdata structure.
 *
 * Note: If Data2 is lower than 1 or higher than the last conference
 *       available, Data2 will be set to the current conference number!
 */

#define RD_SaveConfDat     1010
/* Load UserCnf.data
 *
 * Data1    <- The usernumber of the user to whom you want to save the aktual
 *             conf's Userconfdata structure.
 * Data2    <- The selected conference.
 * Data3
 * IOString
 * StructDummy1 <- The Userconfdata structure.
 *
 * Note: If Data2 is lower than 1 or higher than the last conference
 *       available, Data2 will be set to the current conference number!
 */

#define RD_AppendAccount   1011
/* Add a new Account
 *
 * Data1
 * Data2
 * Data3
 * IOString
 * StructDummy1 <- The filled Userdata structure.
 * StructDummy2 <- The filled Userkey structure.
 *
 * Note: The Userconfdata structures will be generated automaticly and set to
 *       default values.
 */

#define RD_DoOnMaxUserHack 1012
/* Not used now
 *
 * Data1
 * Data2
 * Data3
 * IOString
 */

#define RD_ServerCommand   1013
/* Send ACP command
 *
 * Data1    <- Data1 of the Server Msg.
 * Data2    <- Data2 of the Server Msg.
 * Data3
 * IOString <- String (if needed) for a command.
 */

#define RD_GetMciFlag      1014
/* Retrieve the MCI Flag  !NOT IMPLEMENTED YET!
 *
 * Data1
 * Data2
 * Data3
 * IOString
 */

#define RD_MciFlag         1015
/* Change the MCI Flag  !NOT IMPLEMENTED YET!
 *
 * Data1
 * Data2
 * Data3
 * IOString
 */

#define RD_LoadAccountMisc 1016
/* Load Miscdatas
 *
 * Data1    <- The usernumber of the user you want to load.
 * Data2    <- The selected conference.
 * Data3    -> An internal error code.
 * IOString
 * StructDummy1 -> The Userdata structure.
 * StructDummy2 -> The Userdata structure.
 * StructDummy3 -> The Userconfdata structure.
 *
 * Note: If Data2 is lower than 1 or higher than the last conference
 *       available, Data2 will be set to the current conference number!
 */

#define RD_SaveAccountMisc 1017
/* Write Miscdatas back
 *
 * Data1    <- The usernumber of the user you want to save.
 * Data2    <- The selected conference.
 * Data3
 * IOString
 * StructDummy1 <- The Userdata structure.
 * StructDummy2 <- The Userdata structure.
 * StructDummy3 <- The Userconfdata structure.
 *
 * Note: If Data2 is lower than 1 or higher than the last conference
 *       available, Data2 will be set to the current conference number!
 */

#define RD_ASLLocalUlPath  1018
/* Retrieve or change ASLLocalUlPath
 *
 * Data1    <- 0 means retrieve, else change ASLLocalUlPath.
 * Data2
 * Data3
 * IOString <-> If Data1=0 it contains ASLLocalUlPath,
 *              else this will be the new one (max 21 chars).
 */

#define RD_ASLTextViewPath 1019
/* Retrieve or change ASLTextViewPath
 *
 * Data1    <- 0 means retrieve, else change ASLTextViewPath.
 * Data2
 * Data3
 * IOString <-> If Data1=0 it contains ASLTextViewPath,
 *              else this will be the new one (max 21 chars).
 */

#define RD_ASLSendFilePath 1020
/* Retrieve or change ASLSendFilePath
 *
 * Data1    <- 0 means retrieve, else change ASLSendFilePath.
 * Data2
 * Data3
 * IOString <-> If Data1=0 it contains ASLSendFilePath,
 *              else this will be the new one (max 21 chars).
 */

#define RD_StartNodeCmd    1021
/* Start a command (Menu/Door) at another Node
 *
 * Data1    <- The Node u mean.
 * Data2
 * Data3
 * IOString <- The command incl. Arguments.
 */

#define RD_BeADoorOnNode   1022
/* Let us be a Door of another Node. This means in detail, this command
 * emulates on the given Node that you are a Door started on this Node.
 * The Node opens, if not exists, the own FAMEDoorPort(x).
 * Now you can access this port like a normal Door (which you already are)
 * like as you has been started from there. This also means you *HAVE*
 * to use the Mustcommands: MC_DoorStart and MC_ShutDown !!!
 *
 * Data1    <- The Node you mean.
 * Data2
 * Data3
 * IOString <- Arguments given on doorstart (only a joke 8)...but true!)
 */

#define RD_OpenDoorPort    1023
/* Opens a DoorPort an a Node
 *
 * Data1    <- The Node u mean.
 * Data2    <- The DoorPorttype.
 * Data3
 * IOString
 *
 * DoorPorttype: 0 FIM, 1 XIM, 2 TIM.
 */

#define RD_CloseDoorPort   1024
/* Closes a DoorPort an a Node
 *
 * Data1    <- The Node u mean.
 * Data2    <- The DoorPorttype.
 * Data3
 * IOString
 *
 * DoorPorttype: 0 FIM, 1 XIM, 2 TIM.
 */

#define RD_GetServerList   1025
/* Get the internal Serverliststamm
 *
 * Data1
 * Data2
 * Data3
 * IOString
 * StructDummy1 -> The internal Serverliststamm (struct NodeListStamm).
 */

#define RD_StringToNode    1026
/* Send a Textstring to a Node
 *
 * Data1    <- The Node to which the String should go.
 * Data2
 * Data3
 * IOString <- The String displayed at the Node.
 *
 * Note: If ReturnCode=1 the String can't be send to this Node.
 *       Maybe the Node doesn't exists.
 *       Use NR_ActiveNode to check if the Node exists and AR_GetNodeEnv
 *       to retrieve the status of the node. The best way is to use the
 *       internal ServerNodelist.
 *       This String appears on every position/action on the Node.
 *
 */

#define RD_StringToNodes   1027
/* Send a Textstring to a list of Nodes
 *
 * Data1
 * Data2
 * Data3
 * IOString <- The String displayed at the Nodes.
 * StructDummy1 <- The list of Nodes from type struct FAMEDestNodes.
 *
 * Note: Use NR_ActiveNode to check if the Nodes exist and AR_GetNodeEnv
 *       to retrieve the status of the nodes. The best way is to use the
 *       internal ServerNodelist.
 *       This String appears on every position/action on the Node.
 *
 */

#define RD_CONStatus       1028
/* Get or set the CON Output flag
 *
 * Data1    <- 1 means set, else get.
 * Data2    <-> On Data1 you set this value, else u will get it.
 * Data3
 * IOString
 */

#define RD_SERStatus       1029
/* Get or set the SER Output flag
 *
 * Data1    <- 1 means set, else get.
 * Data2    <-> On Data1 you set this value, else u will get it.
 * Data3
 * IOString
 */

#define RD_SaveMsgFile     1030
/* Save a Msg from
 *
 * Data1    <- Post to Conference.
 * Data2    -> Result.
 * Data3    <- File to message flag.
 * IOString <- File and path.
 * StructDummy1 <- Allocated and filled struct MailHeader
 * StringPtr    <- File or directory contends to message.
 *
 * Results are: 0 = Successfull.
 *              1 = Your given file doesn't exists.
 *              2 = Can't get conference datas from your given conf number.
 *              3 = MsgBase lock failed.
 *              4 = MsgStatus.dat can't be opened.
 *              5 = MsgHeader.dat can't be opened.
 *              6 = Filecopy of message failed.
 *              7 = Your struct MailHeader isn't valid.
 *
 * Data3 file to message flags: 0 - Nothing to attach.
 *                              1 - One single file to attach.
 *                              2 - All files of a directory to attach.
 *
 * Note: Minimum filled elements in struct MailHeader:
 *       - fmah_ToName
 *       - fmah_FromName
 *       - fmah_Subject
 *       - fmah_MsgStatus <- *MUST* be set to TRUE!
 *       - fmah_Private
 *
 * Note: If StringPtr is a valid path to a file it will be copied to
 *       the mail and a flag in the struct MailHeader will be set
 *       to mark that there is a attached file.
 */

#define RD_SaveMsgList     1031
/*
 *
 * Data1    <- Post to Conference.
 * Data2
 * Data3
 * IOString
 * StructDummy1 <- Filled struct ExternEditor.
 *
 * Note: Get the pointer to struct ExternEditor with AR_FullEdStruct!
 *       Also call RD_FreeMsgListFEd before AR_FullEdStruct and get sure
 *       that no msg is on writing on the node till you use this !!!
 */

#define RD_FreeMsgListFEd  1032
/*
 *
 * Data1
 * Data2
 * Data3
 * IOString
 *
 * Note: Free the Internal Msglist. No data needed.
 */

#define RD_ActASLULPath    1033
/* Retrieve or change Actual ASL Upload Path
 *
 * Data1    <- 0 means retrieve, else change ActASLULPath.
 * Data2
 * Data3
 * IOString <-> If Data1=0 it contains ActASLULPath,
 *              else this will be the new one (max 100 chars).
 */

#define RD_ActASLTextViewP 1034
/* Retrieve or change Actual ASL TextView Path
 *
 * Data1    <- 0 means retrieve, else change ActASLTextViewPath.
 * Data2
 * Data3
 * IOString <-> If Data1=0 it contains ActASLTextViewPath,
 *              else this will be the new one (max 100 chars).
 */

#define RD_ActASLDlPath    1035
/* Retrieve or change Actual ASL Download Path
 *
 * Data1    <- 0 means retrieve, else change ActASLDlPath.
 * Data2
 * Data3
 * IOString <-> If Data1=0 it contains ActASLDlPath,
 *              else this will be the new one (max 100 chars).
 */

#define RD_NodeScrToFront  1036
/* Brings Node Screen to Front
 *
 * Data1
 * Data2
 * Data3
 * IOString
 *
 * No datas needed.
 * If the screen is iconified, it will be uniconified.
 */

#define RD_SetServerActCol 1037
/* Change display colour at the Serveraction
 *
 * Data1    <- Colour
 * Data2    <- Node
 * Data3
 * IOString
 */

#define RD_InitNumFlag     1038
/* Initialize your own NumFlagList
 *
 * Data1
 * Data2
 * Data3
 * IOString
 * StructDummy1 <- Your own not initilized struct NumFlagList NumFlagListBase
 *                 pointer.
 *
 * Note: Don't forget to set your NumFlagListBase=NULL;
 *
 * Note: If StructDummy1 is FALSE an internal error has been detected.
 *       In this case you musn't use RD_RemoveNumFlag, RD_ResetNumFlags,
 *       RD_SetNumFlag and RD_GetNumFlag!!!
 *
 * Here now the struct NumFlagList:
 *
 * struct NumFlagList {
 * struct NumFlagList *Next,
 *                    *Prev;
 * char               FileName[102];
 * long               FileNumber;
 * };
 *
 * Note: You can have as many NumFlagListBases as you want to!
 *       There is no limit on FAME side.
 *       Your only thing to do is to take another NumFlagListBase
 *       like here:
 *
 *       struct NumFlagList *NumFlagListBase1,
 *                          *NumFlagListBase2,
 *                          *NumFlagListBase3;
 *
 *       So you easily can have as many simple lists as you want to have.
 *       Normaly such a list is often usable. FAME MainPart uses
 *       8 of them. One for restricted file checkings, one for the
 *       NamesNotAllowed list, one for the NodeToNode comunications,
 *       one for the FlagFromFile/Number function of the menu command "A",
 *       one for file transfering, one for fileskippings, one for
 *       ASL-Requester lists and one for the additional download paths list.
 *       Every of theese 8 different bases needs only only 4 bytes, because
 *       it's just a pointer! Use lists whereever you can! For this reason
 *       i give you those simple list functions, if you don't know how to do
 *       this, also it saves multiple list handling functions.
 *       In future i will give you more komplex list functions, like the
 *       list functions of the internal lineeditor, there are additional
 *       functions for insert, sorting and many more.
 *       If you write own lists in your doors, think about the usage
 *       of the FAME.library internal memory pool functions, which are
 *       compatible to the original AmigaOS3.x pool functions!
 */

#define RD_RemoveNumFlag   1039
/* Remove your own NumFlagList
 *
 * Data1
 * Data2
 * Data3
 * IOString
 * StructDummy1 <- Your own initilized struct NumFlagList NumFlagListBase
 *                 pointer.
 *
 * Note: Don't forget to set your NumFlagListBase=NULL; if you use it agin
 *       with RD_InitNumFlag!
 */

#define RD_ResetNumFlags   1040
/* Reset your own NumFlagList
 *
 * Data1
 * Data2
 * Data3
 * IOString
 * StructDummy1 <- Your own initilized struct NumFlagList NumFlagListBase
 *                 pointer.
 *
 * Note: This is the combination of RD_RemoveNumFlag and RD_InitNumFlag.
 *
 * Note: If StructDummy1 is FALSE an internal error has been detected.
 *       In this case you musn't use RD_RemoveNumFlag, RD_ResetNumFlags,
 *       RD_SetNumFlag and RD_GetNumFlag!!!
 */

#define RD_SetNumFlag      1041
/* Flag an entry into your own NumFlagList
 *
 * Data1    <- The number of the new entry. It's on you to count it correctly!
 * Data2
 * Data3    -> Returncode.
 * IOString <- The entry (FileName).
 * StructDummy1 <- Your own initilized struct NumFlagList NumFlagListBase
 *                 pointer.
 *
 * Note: Data3 is the following returncode:
 *        0 means success.
 *       -1 means allocation failed.
 *       -2 means Base not initialized.
 */

#define RD_GetNumFlag      1042
/* Get an entry from your own NumFlagList
 *
 * Data1    <- The number of the entry you want to have.
 * Data2
 * Data3    -> Returncode.
 * IOString -> The entry (FileName).
 * StructDummy1 <- Your own initilized struct NumFlagList NumFlagListBase
 *                 pointer.
 *
 * Note: Data3 is the following returncode:
 *        0 means success.
 *       -1 means entry number not found.
 *       -2 means Base not initialized.
 */

#define RD_GetUserDataLoc  1043
/* Get the user datas location path
 *
 * Data1
 * Data2
 * Data3
 * IOString -> 'User.data' name
 * StringPtr -> Locationpath of all user datas.
 */

#define RD_ConfName        1044
/* Change the conference name
 *
 * Data1    <- The conference number.
 * Data2
 * Data3
 * IOString <- The new conference name.
 *
 * Note: If Data1 is lower than 1 or higher than the last conference
 *       available, Data1 will be set to the current conference number!
 */

#define RD_ConfLocation    1045
/* Change the conference location
 *
 * Data1    <- The conference number.
 * Data2
 * Data3
 * IOString <- The new conference location.
 *
 * Note: If Data1 is lower than 1 or higher than the last conference
 *       available, Data1 will be set to the current conference number!
 */

#define RD_FGetNextConf    1046
/* Update your conf pointer to the next available conference
 *
 * Data1
 * Data2    -> The new conference number. If relative is on, this value is
 *             also relative, else it's the real conference number.
 * Data3
 * IOString
 * StructDummy1 <-> Your struct ConfList pointer.
 *
 * Note: If no next conference is available for the user, this pointer
 *       leaves unchanged, else it will be set to the new conference pointer.
 * Note: If Data2 has a negative value no next conference is available
 *       for the user.
 *
 * Note: I've said above your pointer leaves unchanged in some cases.
 *       This is true, but not if your pointer was NULL. If your pointer
 *       was NULL it now will be the first available conference of the user.
 */

#define RD_FGetPrevConf    1047
/* Update your conf pointer to the previous available conference
 *
 * Data1
 * Data2    -> The new conference number. If relative is on, this value is
 *             also relative, else it's the real conference number.
 * Data3
 * IOString
 * StructDummy1 <-> Your struct ConfList pointer.
 *
 * Note: If no previous conference is available for the user, this pointer
 *       leaves unchanged, else it will be set to the new conference pointer.
 * Note: If Data2 has a negative value no previous conference is available
 *       for the user.
 *
 * Note: I've said above your pointer leaves unchanged in some cases.
 *       This is true, but not if your pointer was NULL. If your pointer
 *       was NULL it now will be the first available conference of the user.
 */

#define RD_FGetConfNum     1048
/* Get a pointer to a given conference (relative)
 *
 * Data1    <- The conference number from which you want to get the ptr.
 * Data2    -> The new conference number. If relative is on, this value is
 *             also relative, else it's the real conference number.
 * Data3
 * IOString
 * StructDummy1 -> Your new struct ConfList pointer.
 *
 * Note: This function checks for relative flag, if relative is on, the
 *       number used for conference search (Data1) is handled as relative
 *       conference number, else it's used to search for a real conference
 *       number.
 * Note: If Data2 is -2 your given conference number is not available.
 *       If Data2 is -3 your given conference number is available, but
 *       the user has no access to this conference.
 *
 * Note: Your pointer leaves normaly unchanged if Data2 is -3, but not if
 *       your pointer was NULL. If your pointer was NULL it now will be the
 *       first available conference of the user.
 *
 * Note: If Data1 is lower than 1 or higher than the last conference
 *       available, Data1 will be set to the current conference number!
 */

#define RD_FGetRealConfNum 1049
/* Get a pointer to a given conference (not relative)
 *
 * Data1    <- The conference number from which you want to get the ptr.
 * Data2    -> The new conference number, it's ever real!
 * Data3
 * IOString
 * StructDummy1 -> Your new struct ConfList pointer.
 *
 * Note: If Data2 is negative your given conference number is not available.
 *
 * Note: Your pointer leaves normaly unchanged if Data2 is negative, but not
 *       if your pointer was NULL. If your pointer was NULL it now will be
 *       the first available conference of the user.
 *
 * Note: If Data1 is lower than 1 or higher than the last conference
 *       available, Data1 will be set to the current conference number!
 */

#define RD_FGetConfBase    1050
/* Get the conference base pointer.
 *
 * Data1
 * Data2
 * Data3
 * IOString
 * StructDummy1 -> The conference base pointer.
 *
 * Note: You can only use the element RelativeNext to get the first
 *       available conference of the user.
 *
 * Example:
 *
 * struct ConfList *MyConfPtr = DoorMsg -> StructDummy1 -> RelativeNext;
 *
 * while(MyConfPtr)
 * {
 *   ...
 *   ...
 *   MyConfPtr = MyConfPtr -> RelativeNext;
 * }
 */

#define RD_FGetAktConf     1051
/* Get the current conference pointer.
 *
 * Data1
 * Data2
 * Data3
 * IOString
 * StructDummy1 -> The current conference pointer.
 *
 * Note: You can use this pointer to check for following conferences
 *       (RD_FGetNextConf) or previous conferences (RD_FGetPrevConf).
 *
 * Example:
 *
 * while(DoorMsg -> StructDummy1)
 * {
 *   ...
 *   ...
 *   DoorMsg -> StructDummy1 = DoorMsg -> StructDummy1 -> RelativeNext;
 * }
 */

#define RD_GiveNumFromConf 1052
/* Get the relative/non relative number from a conference pointer.
 *
 * Data1
 * Data2    -> The conference number. If relative is on, this value is
 *             also relative, else it's the real conference number.
 * Data3
 * IOString
 * StructDummy1 <- Your struct ConfList pointer.
 */

#define RD_SaveUFlagList   1053
/* Forbid upload procedure to clear the upload flag list after transfer
 *
 * Data1    <- 0 = clear, else don't clear.
 * Data2
 * Data3
 * IOString
 *
 * Note: If Data1 is non zero from now on the upload procedure won't any
 *       longer clear the flag list of uploaded files and the door can use it
 *       else if Data1 is zero you tell the upload procedure it now can
 *       clear it again.
 *       Normaly the upload procedure clears after finished filechecking
 *       and fileid procedure at last this list to free the memory to
 *       the system. But it's not realy a must, because if the next
 *       upload is started this list will be cleared also before the
 *       real transfer begins. It's a question of memory usage. It's
 *       better to clear it after upload, because we don't know how
 *       may files are in the list and when the next upload will be
 *       started.
 *       If a door want's to know which files were uploaded after
 *       upload procedure it's needed to get this list. To get a filled
 *       list we must use RD_SaveUFlagList with Data1 = TRUE.
 *       Then we can get the base pointer to this list with RD_GetUFlagList.
 *       After we have finished reading from this list, beware it's read
 *       only!!!, we should use RD_ClearUFlagList to clear the list now.
 *       If our door no longer needs information of the list, because
 *       no more upload procedures will be started we should use
 *       RD_SaveUFlagList again with Data1 = FALSE (0) to set the
 *       upload procedure to the origin state. It's not realy bad if you
 *       forget this, because if your door shuts down this flag will be set
 *       automaticly back.
 */

#define RD_ClearUFlagList  1054
/* Clear the upload flag list now
 *
 * Data1
 * Data2
 * Data3
 * IOString
 */

#define RD_GetUFlagList    1055
/* Retrieve the basis of the upload flag list
 *
 * Data1
 * Data2
 * Data3
 * IOString
 * StructDummy1 -> The basis pointer of the upload flag list.
 */

#define RD_GetMailHeader   1056
/* Retrieve the MailHeader pointer (SYSCMD: MAILHEADER)
 *
 * Data1
 * Data2
 * Data3
 * IOString
 * StructDummy1 -> The MailHeader pointer.
 */

#define RD_ShowMailHeader  1057
/* Inform the Node to show MailHeader (SYSCMD: MAILHEADER)
 *
 * Data1        <- True to show MailHeader, false not.
 * Data2
 * Data3
 * IOString
 */

#define RD_GetDldFileStamm 1058
/* Retreive the DldFilesStamm list root pointer.
 *
 * Data1
 * Data2
 * Data3
 * IOString
 * StructDummy1 -> The struct NumFlagList *DldFilesStamm pointer.
 *
 * Note: Use RD_GetDldFileStamm to retreive the beginning of the downloaded
 *       file list. In this list you will find all files downloaded by
 *       the currently online user of the last download if there was one
 *       this time. DldFilesStamm is empty if no download has been done yet.
 *       DldFilesStamm will be reinitialized if the user starts a new
 *       download or the user logges off.
 *
 * Note: After using RD_RIDldFileStamm you have to use RD_GetDldFileStamm
 *       again if you want to continue work with the DldFilesStamm pointer!
 */

#define RD_RIDldFileStamm  1059
/* Reinitialize the DldFilesStamm list.
 *
 * Data1
 * Data2
 * Data3
 * IOString
 *
 * Note: RD_RIDldFileStamm reinitializes the DldFilesStamm you will get
 *       with RD_GetDldFileStamm. This means all list entries will be freed
 *       and the list pointer reinitialized.
 *
 * Note: After using RD_RIDldFileStamm you have to use RD_GetDldFileStamm
 *       again if you want to work with the DldFilesStamm pointer!
 */

#define RD_PutDldFileStamm 1060
/* Add a file to the DldFilesStamm list.
 *
 * Data1    <- The Conference number depending to the file.
 * Data2
 * Data3
 * IOString <- The filename.
 *
 * Note: The filed FileNumber of the struct NumFlagList will be used for
 *       the Conference number. This number indicates the Conference
 *       (real/physical number) where the file has been downloaded from.
 *       If you don't know the Conference number set it to 0 as default.
 */

/*   ###################### COMMAND DEFINES END ###########################
 */

#endif  /* FAME_FAMEDOORCOMMANDS_H */
