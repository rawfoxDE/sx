
/* ================================================================== */
/* =================== EXTERNAL PROTO TYPES ========================= */
/* ================================================================== */

extern struct DosLibrary *DOSBase ;

extern void FixDir(char *dir);
extern ULONG SpaceFree(void);
extern void CheckUploads(void);
extern void CheckFlags(void);
extern int FindArcPo(char *fname);
extern LONG FindFileOnHD(char * f,BOOL raw);
extern void CheckOnlineKey(UBYTE code);
extern void CheckLocalKey(UWORD code);
extern void WaitScreen(void);
extern void User_Location(BOOL force);
extern void User_Phone(BOOL force);
extern void User_Computer(BOOL force);
extern void User_Pass(BOOL force);
extern void User_Screen(BOOL force);
extern void User_Signature(BOOL force);
extern void User_Protocol(void);
extern void User_FileScan(void);
extern void User_MailScan(void);
extern void User_BGCheck(void);
extern void User_FSED(void);
extern void upcase(char * str);
extern void PageSysOp(void);
extern void ShowPrompt(char *str, BOOL mci);
extern void Who(void);
extern void Ver(void);
extern BOOL EnterMsg(char *to, char *subj, UWORD msgarea, UWORD confnum);
extern void ViewFile(void);
extern void EditUser(BOOL remote);
extern ULONG StripAnsi(char * ans, ULONG len);
extern void Servermsg(int command);
extern void GotoMsgArea(long area);
extern void ReadMsgs(void);
extern void MailScan(void);
extern void LoadFlags(void);
extern void CheckLCFiles(void);
extern void RawReceive(char *recpath, BOOL freezeclock);
extern BOOL SendFile(char *fname);
extern void UserList(BOOL alpha, BOOL realname);
extern void MailScanLogin(void);
extern BOOL MAXsShowAnsi(char *fname);
extern void HandleNodeMsg(void);
extern void CalcTimeLeft(void);
extern void CheckWin(struct Window *win);
extern void SetTimer(ULONG secs, ULONG micros);
extern int rnd(int x);
extern long FilesLeft(void);
extern long BytesLeft(void);
extern void ConfAccess(struct SXUserStruct *TempUser, char *str, UWORD max);
extern BOOL ListFlags(void);
extern void RawTransfer(UWORD xtype, BOOL isend, BOOL freeze, char *recpath, char *fname);
extern void NodeMsg(WORD msgnode, char *fname, char *string);
extern BOOL LineED(char * fname);
extern void JoinConf(UWORD confnum, BOOL hitreturn);
extern void myctime(long secs, char *outbuf);
extern int FindAccount(char * searchstr);
extern void DoFunction(int id, char * string, int extra, int low, int high, char * mainarg, char * exearg);
extern BOOL SendFile(char *fname);
extern void CheckKey(void);
extern ULONG StripAnsi(char * ans, ULONG len);
extern void RunDoor(char *door, UWORD type, char *exearg);
extern void SModem(char *recpath);
extern void HydraCom(char *recpath);
extern void AlterFlags(void);
extern void UnFlagPattern(char *pat, BOOL show);
extern int UnFlagFile(char * f);
extern int FlagFile(char * f, BOOL free, BOOL inst, BOOL raw);
extern struct Node * FindFlag(char * f);
extern void AddFile(char * filename, ULONG cps, ULONG size);
extern void DropDTR(BOOL sendio);
extern long mytime(void);
extern unsigned char * mytimestr(BOOL sec);
extern unsigned char * mydatestr(void);
extern void WaitBuf(void);
extern void ClearFlags(void);
extern void WaitForXIM(char *door);
extern void WaitForParagon(char *door);
extern void WaitForDD(char *door);
extern void WaitForFAME(char *door);
extern void FifoExecute(char * exe, char *exearg);
extern int ShowAnsi (char *f, BOOL path, BOOL lang, BOOL axs, BOOL conf, BOOL pause, UWORD startline);
extern void CallerLog(char * logstr);
extern void MainMenu(void);
extern void EditUser(BOOL remote);
extern void NewUser(void);
extern void SaveAccount(void);
extern void LoadAccount(int i, BOOL index);
extern void SetActive(int id, char * a);
extern void LoadStrings(void);
extern void FreeStrings(void);
extern void LineInput(int limit,BOOL pass,char * def);
extern void HitReturn(void);
extern void MyHotKey (void);
extern void PS(char * b);
extern void SerWrite (char * b);
extern void ConWrite (char * b, ULONG l);
extern void Local (char * b);
extern int YesNo(int def, BOOL show);
extern BOOL Carrier(void);
extern ULONG SXWait(void);
extern void SafePS(char * s);
extern void OutP(void);
extern void InP(void);
extern int DisMode(void);
extern int DisModeCompare(struct ScreenModeStruct *dmone, struct ScreenModeStruct *dmtwo);
extern void InternalCmd(char *com);
extern void PSLen(char * b, long l);
extern void GotoMsgArea(long area);
extern void DeCrypt(char *str);
extern BOOL CheckConfAccess(struct SXUserStruct *TempUser, UWORD conf);
extern BOOL GetAccessSet(UWORD access);
extern void SaveFlags(void);
extern void InstSend(void);
extern void RawReceive(char *recpath, BOOL freezeclock);
extern void AsciiSend(void);
extern BOOL LoadProtocol(char idletter);
extern void PutAccessSet(struct SXUserStruct *TempSXUser, struct UserData *TempUser);
extern void LoadPrefs(void);
extern void LoadConfs(void);
extern void LoadArcs(void);
extern void LoadLocalDis(void);
extern void Chat(void);
extern int OpenDisplay(BOOL sendio);
extern void CloseDisplay(void);
extern void GrabColors(UWORD *ccolor);
extern void InfoWinBorder(BOOL i);
extern void FixTitle(void);
extern UWORD FindWhite(void);
extern void UpdateUserInfo(void);
extern void UpdateTimeLeft(void);
extern BOOL Pens(void);
extern BOOL ScreenModeReq(void);
extern void Palette(void);
extern void SavePalette(void);
extern void FreePrefs(void);
extern void PanelPrompts(void);
extern void MCIPS(char * ansbuf, long size, BOOL pause, UWORD startline);
extern void mysprintf(char *Buffer,char *ctl, ...);
extern void PSFmt(char *ctl, ...);
extern void TextFmt(struct RastPort *rP, char *ctl, ...);
extern void TestDoor(void);
extern void SysopDownload(void);
extern void DownloadFile(void);
extern void UploadFile(BOOL showansi);
