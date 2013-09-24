//-------------------------------------------------------------------
// AEDIT.H - Include file for AEDIT.C
//
// Copyright 1993 - Douglas Boling
//-------------------------------------------------------------------
//
// Edit state structure
//
typedef struct {
	LONG	lScrRow;       //Screen Row
	UINT	usScrCol;      //Screen Col
	UINT	usCurRow;      //Cursor Row
	UINT	usCurCol;      //Cursor Col
	LONG	lSMarkRow;     //Starting Mark Row
	UINT 	usSMarkCol;    //Starting Mark Col
	LONG	lEMarkRow;     //Ending Mark Row
	UINT 	usEMarkCol;    //Ending Mark Col

	INT 	sUndoType;     //Undo operation
	LONG	lSUndoRow;     //Starting Undo Row
	UINT 	usSUndoCol;    //Starting Undo Col
	LONG	lEUndoRow;     //Ending Undo Row
	UINT 	usEUndoCol;    //Ending Undo Col

	UINT	selPBuff;      //Line Ptr buff Selector
	UINT	selEBuff;      //Edit buff Selector
	HPSZ	lpEEnd;        //Edit buff 1st avail byte
	HPSZ	lpESize;       //Edit buff end buff ptr
	HPSZ	lpUndoBuff;    //Ptr to last deleted text 
	HPSZ	lpCutBuff;     //Ptr to cut buffer
	LONG	lEBuffSize;    //Edit buff size
	LONG  lNumPtrs;      //Num Line ptrs available
	HPSZ	lpGCollectPtr; //Ptr for buff compress routine

	UINT	usTab;         //Tab expansion value
	BYTE	fFlags;        //Edit flags

	HFILE	hHandle;       //File handle
	LONG	lSize;         //Size of file in bytes
	LONG	lLines;        //Num of lines in file
	char	szName[MAXFNAMELEN]; //File name
} EDITSTATE;
typedef EDITSTATE *PEDITSTATE;

typedef struct {
	UCHAR	chChar;
	UCHAR	chScan;
	UINT	fsState;
} KEYST;
typedef KEYST *PKEYST;

typedef	BOOL (*EDITFUNC) (PKEYST);
typedef	EDITFUNC *PEDITFUNC;

typedef struct {
	UINT  	Key;
	EDITFUNC Fxn;
} DECODEKEY; 

typedef struct {
	EDITFUNC Fxn;
	UINT		usStatus;
	UINT		Key;
	char  	*szName;
} MENUITEM;
typedef MENUITEM *PMENUITEM;

typedef struct {
	INT	sSize;
	MENUITEM mMenu[5];
} MYMENU;
typedef MYMENU *PMYMENU;
//
// Function Prototypes
//
BOOL	DoNew (PKEYST);
BOOL	DoOpen (PKEYST);
BOOL	DoSave (PKEYST);
BOOL	DoSaveAs (PKEYST);
BOOL	DoExit (PKEYST);
BOOL	DoUndo (PKEYST);
BOOL	DoCut (PKEYST);
BOOL	DoCopy (PKEYST);
BOOL	DoPaste (PKEYST);
BOOL	DoSearch (PKEYST);
BOOL	DoRepSearch (PKEYST);
BOOL	DoReplace (PKEYST);
BOOL	DoGoto (PKEYST);
BOOL	DoTabs (PKEYST);
BOOL	DoToggleCase (PKEYST);
BOOL	DoKeyHelp (PKEYST);
BOOL	DoAbout (PKEYST);

BOOL	DoEnter (PKEYST);
BOOL	DoReturn (PKEYST);
BOOL	DoSplitLine (PKEYST);
BOOL	DoBackspace (PKEYST);
BOOL	DoDelete (PKEYST);
BOOL	DoDeleteEOL (PKEYST);
BOOL	DoToggleInsert (PKEYST);
BOOL	GoNextWord (PKEYST);
BOOL	GoPrevWord (PKEYST);
BOOL	GoFileEnd (PKEYST);
BOOL	GoFileTop (PKEYST);
BOOL	GoScreenTop (PKEYST);
BOOL	GoScreenBottom (PKEYST);
BOOL	DoPageDown (PKEYST);
BOOL	DoPageUp (PKEYST);
BOOL	GoLineStart (PKEYST);
BOOL	GoLineEnd (PKEYST);
BOOL	DoCurDown (PKEYST);
BOOL	DoCurUp (PKEYST);
BOOL	DoCurLeft (PKEYST);
BOOL	DoCurRight (PKEYST);
BOOL	DoCompress (PKEYST);
BOOL	DoNothing (PKEYST);
BOOL	DefaultKeyProc (PKEYST);

INT	NewFile (PEDITSTATE);
INT	LoadFile (PEDITSTATE);
INT	StoreFile (PEDITSTATE);

void	DrawMenu (PMYMENU, INT, UINT);
BOOL	DrawHelp (char **, INT);
void	DrawStatus (PEDITSTATE, char *, BOOL);
void	DrawScreenLine (PEDITSTATE, UINT);
void	DrawScreen (PEDITSTATE);
INT	QueryUser (char *, char *, UINT, INT);

BOOL	FindString (PEDITSTATE, char *, LONG *, UINT *, BOOL);
void	GetMarkedArea (PEDITSTATE, LONG *, UINT *, LONG *, UINT *);
INT	JoinLine (PEDITSTATE, LONG);
void	SaveUndo (PEDITSTATE, INT, LONG, UINT, LONG, UINT);
INT	PasteBuff (PEDITSTATE, HPSZ);
INT	CutString (PEDITSTATE, LONG, UINT, UINT);
INT	CutCopy (PEDITSTATE, HPSZ *, BOOL);
HPSZ	CutCopyLine (PEDITSTATE, LONG, UINT, UINT, BOOL);
UINT	GetLineLen (HPSZ);
UINT	GetColPos (HPSZ, HPSZ);
void	EraseLine (PEDITSTATE, HPSZ);
HPSZ	CopyToClip (PEDITSTATE, HPSZ, UINT);
HPSZ	MoveToEnd (PEDITSTATE, LONG);
HPSZ	CopyToEnd (PEDITSTATE, HPSZ);
void	MoveTo (PEDITSTATE, LONG, UINT, BOOL);
HPSZ	MakeSpace (PEDITSTATE, UINT, UINT, LONG, INT *);
void	CompressBuff (PEDITSTATE);
HPSZ	FreeBuffSpace (PEDITSTATE, HPSZ);
INT	AddFileLine (PEDITSTATE, LONG, HPSZ, LONG);
HPSZ	DelFileLine (PEDITSTATE, LONG, LONG);
HPSZ	GetScreenLine (PEDITSTATE, UINT);
HPSZ	GetScreenChar (PEDITSTATE, UINT, UINT);
HPSZ	GetFileChar (PEDITSTATE, HPSZ, UINT);
HPSZ	GetFileLine (PEDITSTATE, LONG);
HPSZ  far *GetLinePtr (PEDITSTATE, LONG);
void  SetLinePtr (PEDITSTATE, LONG, HPSZ);


void	GetScreenInfo (void);
void	ClearScreen (BYTE);
BOOL	GetKey (PKEYST, UINT, BOOL);
void	InitKeyboard (UINT);
void	Yield (void);
BOOL	ProcessKey (PKEYST, BOOL);
INT	ReadIniFile (char *);
INT 	ParseCmdLine (INT, char **, char *, LONG *, UINT *);
void	WriteString (HPSZ, UINT, BYTE, UINT, UINT);
void	WriteLine (HPSZ, BYTE, UINT);
void	MyWriteChar (BYTE, BYTE, UINT, UINT);
void	PrintCopyright (INT);

UINT	AllocMem (LONG, LONG);
UINT	ReallocMem (UINT, LONG);
void	FreeMem (UINT);

INT	OpenFile (char *, HFILE *, UINT);
INT	InitFile (PEDITSTATE);
INT	CloseFile (PEDITSTATE);
INT	DeleteFile (char *);
INT	ReadFile (PEDITSTATE, HPSZ);
INT	WriteFile (PEDITSTATE, HPSZ, UINT);
LONG	MoveFilePtr (PEDITSTATE, LONG, INT);

void	SetCursor (UINT, UINT);
void	GetCursor (UINT *, UINT *);
void	SaveScreen (UINT *);
void	RestoreScreen (UINT);
void	ScrollUp (INT);
void	ScrollScrUp (SHORT, SHORT, SHORT, BYTE);
void	ScrollScrDown (SHORT, SHORT, SHORT, BYTE);

//
// Keyboard equates
//
#define ESC        0x0001
#define KEY_1      0x0002
#define KEY_2      0x0003
#define KEY_3      0x0004
#define KEY_4      0x0005
#define KEY_5      0x0006
#define KEY_6      0x0007
#define KEY_7      0x0008
#define KEY_8      0x0009
#define KEY_9      0x000A
#define KEY_0      0x000B
#define MINUS      0x000C
#define EQUAL      0x000D
#define BKSP       0x000E
#define TAB        0x000F
#define KEY_Q      0x0010
#define KEY_W      0x0011
#define KEY_E      0x0012
#define KEY_R      0x0013
#define KEY_T      0x0014
#define KEY_Y      0x0015
#define KEY_U      0x0016
#define KEY_I      0x0017
#define KEY_O      0x0018
#define KEY_P      0x0019
#define LBRKT      0x001A
#define RBRKT      0x001B
#define ENTER      0x001C
#define KEY_A      0x001E
#define KEY_S      0x001F
#define KEY_D      0x0020
#define KEY_F      0x0021
#define KEY_G      0x0022
#define KEY_H      0x0023
#define KEY_J      0x0024
#define KEY_K      0x0025
#define KEY_L      0x0026
#define SCOLON     0x0027
#define QUOTE      0x0028
#define BQUOTE     0x0029
#define BSLASH     0x002b
#define KEY_Z      0x002c
#define KEY_X      0x002d
#define KEY_C      0x002e
#define KEY_V      0x002f
#define KEY_B      0x0030
#define KEY_N      0x0031
#define KEY_M      0x0032
#define COMMA      0x0033
#define DOT        0x0034
#define FSLASH     0x0035
#define SCAN_36    0x0036
#define ASTR       0x0037
#define SCAN_38    0x0038
#define SPACE      0x0039
#define SCAN_3A    0x003A
#define KEY_F01    0x003b
#define KEY_F02    0x003c
#define KEY_F03    0x003d
#define KEY_F04    0x003e
#define KEY_F05    0x003f
#define KEY_F06    0x0040
#define KEY_F07    0x0041
#define KEY_F08    0x0042
#define KEY_F09    0x0043
#define KEY_F10    0x0044
#define SCAN_45    0x0045
#define SCAN_46    0x0046				        
#define HOME       0x0047
#define UP         0x0048
#define PGUP       0x0049
#define GRAYM      0x004a
#define LEFT       0x004b
#define CENTER     0x004c
#define RIGHT      0x004d
#define GRAYP      0x004E
#define END        0x004F
#define DOWN       0x0050
#define PGDN       0x0051
#define INS        0x0052
#define DEL        0x0053
#define SFT_F01    0x0054
#define SFT_F02    0x0055
#define SFT_F03    0x0056
#define SFT_F04    0x0057
#define SFT_F05    0x0058
#define SFT_F06    0x0059
#define SFT_F07    0x005A
#define SFT_F08    0x005B
#define SFT_F09    0x005C
#define SFT_F10    0x005D
#define CTL_F01    0x045E
#define CTL_F02    0x045F
#define CTL_F03    0x0460
#define CTL_F04    0x0461
#define CTL_F05    0x0462
#define CTL_F06    0x0463
#define CTL_F07    0x0464
#define CTL_F08    0x0465
#define CTL_F09    0x0466
#define CTL_F10    0x0467
#define ALT_F01    0x0868
#define ALT_F02    0x0869
#define ALT_F03    0x086A
#define ALT_F04    0x086B
#define ALT_F05    0x086C
#define ALT_F06    0x086D
#define ALT_F07    0x086E
#define ALT_F08    0x086F
#define ALT_F09    0x0870
#define ALT_F10    0x0871
#define SCAN_72    0x0072
#define CTL_LEFT   0x0473
#define CTL_RIGHT  0x0474
#define CTL_END    0x0475
#define CTL_PGDN   0x0476
#define CTL_HOME   0x0477
#define ALT_1      0x0878
#define ALT_2      0x0879
#define ALT_3      0x087A
#define ALT_4      0x087B
#define ALT_5      0x087C
#define ALT_6      0x087D
#define ALT_7      0x087E
#define ALT_8      0x087F
#define ALT_9      0x0880
#define ALT_0      0x0881
#define ALT_MINUS  0x0882
#define ALT_EQUAL  0x0883
#define CTL_PGUP   0x0484
#define F11        0x0085
#define F12        0x0086
#define SFT_F11    0x0387
#define SFT_F12    0x0388
#define CTL_F11    0x0489
#define CTL_F12    0x048A
#define ALT_F11    0x088B
#define ALT_F12    0x088C
#define CTL_UP     0x048D
#define CTL_GRAYM  0x048E
#define CTL_CENTER 0x048F
#define CTL_GRAYP  0x0490
#define CTL_DOWN   0x0491
#define CTL_INS    0x0492
#define CTL_DEL    0x0493
#define CTL_TAB    0x0494
#define CTL_FSLASH 0x0495
#define ALT_ASTRSK 0x0896
#define ALT_HOME   0x0897
#define ALT_UP     0x0898
#define ALT_PGUP   0x0899
#define ALT_GRAYM  0x084a
#define ALT_LEFT   0x089b
#define ALT_CENTER 0x084c
#define ALT_RIGHT  0x089d
#define ALT_GRAYP  0x084E
#define ALT_END    0x089F
#define ALT_DOWN   0x08A0
#define ALT_PGDN   0x08A1
#define ALT_INS    0x08A2
#define ALT_DEL    0x08A3
#define ALT_FSLASH 0x08A4
#define ALT_TAB    0x08A5
#define ALT_ENTER  0x08A6

