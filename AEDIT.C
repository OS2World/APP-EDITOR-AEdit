//===================================================================
// AEDIT  A FAPI Editor
// Copyright 1993 Douglas Boling
//
// Revision History:
//
// 1.0   Initial Release           PC Magazine Issue 20 Vol 12
//
//===================================================================
#define dim(x) (sizeof(x) / sizeof(x[0]))   // Returns no. of elements

#define HPSZ  char _huge *

#define MAXFNAMELEN      256
#define MAXMENULEN       15
#define MAXKEYS          64

#define EDITBUFFSTART    65536
#define EDITBUFFEXTRA    2048
#define NUMPTRS          16384
#define MAXNUMPTRS       262144

#define SCROVERLAP       5

#define GFLAG_INSERT     0x01
#define GFLAG_CHKCASE    0x02
#define GFLAG_CONTMARK   0x04
#define GFLAG_SCRLOCK    0x10
#define GFLAG_NUMLOCK    0x20
#define GFLAG_CAPLOCK    0x40

#define FLAG_DIRTY       0x01
#define FLAG_MARKFLAG    0x02
#define FLAG_BINFILE     0x04
#define FLAG_NEWFILE     0x08

#define UNDO_NOTHING     0
#define UNDO_CUT         1
#define UNDO_COPY        2
#define UNDO_PASTE       3
#define UNDO_DEL         4

#define CR               0x0d
#define LF               0x0a
#define FILLCHAR         0x20

#define INCL_VIO
#define INCL_KBD
#define INCL_NOPM
#define INCL_DOS

#include <os2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ERR_HELP         1000
#define ERR_SYNTAX       1
#define ERR_DISKFULL     2
#define ERR_DOSERROR     3
#define ERR_OUTOFMEM     11
#define ERR_OPENERROR    12
#define ERR_TOOMANYLINES 13
#define ERR_CANCELED     14
#define ERR_NOMARKED     15
#define ERR_NOUNDO       16
#define ERR_COLTOOBIG    17
#define ERR_TABTOOBIG    18
#define ERR_LINETOOLONG  19
#define ERR_FILEEXISTS   20
#define ERR_FILENOTSAVED 21

#include "aedit.h"
//
// Global Data
//
char	szBinFile[] = "Null characters in file";
char	szNewFile[] = "File not found";
char	szFileExists[] = "File exists. Replace? (y,n)";
char	szFileNotSaved[] = "File not saved. Save now? (y,n)";
char	szFileSaved[] = "File Saved";
char	szNamePrompt[] = "Filename:";
char	szCut[] = "Area Cut to buffer";
char	szCopied[] = "Area Copied to buffer";
char	szPasted[] = "Buffer inserted";
char	szDeleted[] = "Area deleted";
char	szUndone[] = "Undone";
char	szSearchPrompt[] = "String:";
char	szRepPrompt[] = "Replace with:";
char	szRep1Prompt[] = "Replace? (y,n,a,q):";
char	szNotFound[] = "Not found";
char	szLinePrompt[] = "Line:";
char	szTabPrompt[] = "New Tabstop:";
char	szCmdDone[] = "Done";
char	szStoring[] = "Saving file...";
char	szSearching[] = "Searching...";

char	szNothingInBuff[] = "Nothing in buffer";
char	szCompressing[] = "Compressing Buffer";

char	*szSyntaxText[] = {
	"Syntax AEDIT [/Tn] [/Ln] [/Cn] [Filename]",
	"",
	" Filename - Name of file to edit",
	"",
	" /T - Set tab stops to n",
	" /L - Set initial cursor position to line n",
	" /C - Set initial cursor position to column n",
};
char	*szAboutMsg[] = {
	"",
	"AEDIT 1.0 Copyright 1993 Douglas Boling",
	"First published in PC Magazine November 23, 1993"
};
char	*szHelpMsgs[] = {
	"Key\tFunction",
	"",
	"Ctrl-C\tCopy selected text to buffer",
	"Ctrl-X\tCut selected text to buffer",
	"Ctrl-P\tPaste buffer into text",
	"Ctrl-E\tDelete text to end of line",
	"Ctrl-Z\tUndo last Cut, Copy, Paste or Delete",
	"Ctrl-S\tSearch for text",
	"Ctrl-R\tRepeat last search",
	"Ctrl-T\tSearch and replace text",
	"Ctrl-A\tToggle search respect case flag",
	"Ctrl-G\tGoto line",
	"Alt-X, Alt-F4\tExit program",
	"",
	"Ctrl-Left,Right\tGoto start of previous/next word",
	"Ctrl-PageUp,PageDn\tGoto top/bottom line of screen",
	"Ctrl-Home,End\tGoto start/end of file",
	"Home, End \tGoto start/end of line",
	"Page Up, Page Down\tGoto previous/next screen",
	"Up,Down,Left,Right\tMove cursor",
	"",
	"Text is selected by holding down the shift key while",
	"moving the cursor."
};

char	*szErrMsgs[] = {
	"",
	"Syntax error.  Type AEDIT /? for help",
	"Disk Full",
	"DOS Error",
	"Bad Function",
	"File not found",
	"Path not found",
	"Out of handles",
	"Access denied",
	"Invalid handle",
	"Arena trashed",
	"Out of Memory",
	"Can't open file",
	"Too many lines",
	"Command Canceled",
	"No Marked Area",
	"Nothing to undo",
	"Column too big",
	"Bad tab value",
	"Line too long",
	"File already exists",
	"File not saved",
}; 
//
// Menu Templates
//
MYMENU mFile = { 5,
	{{DoNew,     0, KEY_N, "&New"},
	 {DoOpen,    0, KEY_O, "&Open"},
	 {DoSave,    0, KEY_S, "&Save"},
	 {DoSaveAs,  0, KEY_A, "Save &As"},
	 {DoExit,    0, KEY_X, "E&xit"}}
};
MYMENU mEdit = { 4,
	{{DoUndo,    0, KEY_U, "&Undo"},
	 {DoCut,     0, KEY_T, "Cu&t"},
	 {DoCopy,    0, KEY_C, "&Copy"},
	 {DoPaste,   0, KEY_P, "&Paste"}}
};
MYMENU mSearch = { 4,
	{{DoSearch,    0, KEY_F, "&Find"},
	 {DoRepSearch, 0, KEY_R, "&Repeat"},
	 {DoReplace,   0, KEY_C, "&Change"},
	 {DoGoto,      0, KEY_G, "&Goto"}}
};
MYMENU mConfig = { 2,
	{{DoTabs,      0, KEY_T, "&Tabs"},
	 {DoToggleCase,0, KEY_C, "Respect &Case"}}
};
MYMENU mHelp = { 2,
	{{DoKeyHelp, 0, KEY_K, "&Keys"},
	 {DoAbout,   0, KEY_A, "&About"}}
};
MYMENU mMain = { 5,
	{{(EDITFUNC) &mFile,   8, KEY_F, "&File"},
	 {(EDITFUNC) &mEdit,   8, KEY_E, "&Edit"},
	 {(EDITFUNC) &mSearch, 8, KEY_S, "&Search"},
	 {(EDITFUNC) &mConfig, 8, KEY_C, "&Configure"},
	 {(EDITFUNC) &mHelp,   8, KEY_H, "&Help"}}
};
//
// Default keyboard assignments for editor
//
#define S_CTL 0x0400
#define S_ALT 0x0800

DECODEKEY KeyProcs[] = {
	DOWN,            DoCurDown,
	UP,              DoCurUp,
	LEFT,            DoCurLeft,
	RIGHT,           DoCurRight,
	PGDN,            DoPageDown,
	PGUP,            DoPageUp,
	HOME,            GoLineStart,
	END,             GoLineEnd,
	ENTER,           DoEnter,
	CTL_PGUP,        GoScreenTop,
	CTL_PGDN,        GoScreenBottom,
	CTL_HOME,        GoFileTop,
	CTL_END,         GoFileEnd,
	CTL_RIGHT,       GoNextWord,
	CTL_LEFT,        GoPrevWord,
	INS,             DoToggleInsert,
	DEL,             DoDelete,
	BKSP,            DoBackspace,
	(S_CTL | KEY_A), DoToggleCase,
	(S_CTL | KEY_C), DoCopy,
	(S_CTL | KEY_E), DoDeleteEOL,
	(S_CTL | KEY_G), DoGoto,
	(S_CTL | KEY_S), DoSearch,
	(S_CTL | KEY_R), DoRepSearch,
	(S_CTL | KEY_T), DoReplace,
	(S_CTL | KEY_V), DoPaste,
	(S_CTL | KEY_X), DoCut,
	(S_CTL | KEY_Z), DoUndo,
	(S_ALT | KEY_Z), DoCompress,
	(S_ALT | KEY_X), DoExit,
	ALT_F04,         DoExit,
	ESC,             DoNothing
};
//
// Screen information
//
UINT	usRows;     //Screen Rows
UINT	usCols;     //Screen Cols
UINT	usEWinRows; //Edit window rows
UINT	usEWinStrt; //Edit window starting row
UINT	usMenuRow;  //Menu row
UINT	usStatRow;  //Status line row
BYTE	ucAtNorm;   //Normal attribute
BYTE	ucAtMark;   //Marked text attribute
BYTE  ucAtMenu;   //Menu attribute
BYTE  ucAtMenuHot;//Menu hotkey attribute
BYTE  ucAtStat;   //Status line attribute

EDITSTATE estate;                //Edit state information
PEDITSTATE pe;                   //Ptr to active edit state
UINT	usDosVer;
UINT	fGlobalFlags = 0;
char	szIniFileName[MAXFNAMELEN] = ""; //Name of INI file
char	szSearchStr[50] = "";
char	szReplaceStr[50] = "";
//Menu pointer
INT		sMenuSel = 0;
PMYMENU	pmCurrMenu;

//===================================================================
// Program Entry Point
//===================================================================
main (int argc, char *argv[]) {

	KEYST	ksKey;
	BOOL	fContinue, fMenu;
	INT	rc;
	UINT	usSaveRow, usSaveCol, selScreen, usSCol;
	LONG	lSRow;
	// 	
	//Program initialization processing
	//
	pe = &estate;
	DosGetVersion (&usDosVer);        
	InitKeyboard (0);

	GetScreenInfo ();
	//Alloc buffer for screen save
	selScreen = AllocMem (usRows * usCols * 2, 0);
	//
	// Init edit state structure.  Most fields default to 0.
	//
	memset (pe, 0, sizeof (EDITSTATE));
	pe->usTab = 8;
	pe->sUndoType = UNDO_NOTHING;
	fGlobalFlags = GFLAG_INSERT;
	pmCurrMenu = &mMain;
	//
	// Parse the command line.  Exit if error
	//
	lSRow = 0;
	usSCol = 0;
	rc = ParseCmdLine (argc, argv, pe->szName, &lSRow, &usSCol);
	if (rc) {
		PrintCopyright(rc);
		DosExit (1, rc);
	}
	//
	// If file name on command line, open file.
	//
	if (strlen (pe->szName))
		rc = LoadFile (pe);
	else
		rc = NewFile (pe);
	if (rc) {
		PrintCopyright(rc);
		DosExit (1, rc);
	} 
	//
	// Std setup. Save screen and cursor pos.  Move cursor to
	// 0 or specified location, then draw screen.  Init
	// buffer garbage collect routine and clear key state.
	//
	SaveScreen (&selScreen);
	GetCursor (&usSaveRow, &usSaveCol);
	MoveTo (pe, lSRow, usSCol, FALSE);
	DrawScreen (pe);
	pe->lpGCollectPtr = FreeBuffSpace (pe, 0);
	ksKey.fsState = 0;
	if (pe->fFlags & FLAG_BINFILE)
		DrawStatus (pe, szBinFile, TRUE);
	else
		DrawStatus (pe, 0, TRUE);
	//
	// Process loop
	//
	fContinue = TRUE;
	while (fContinue) {
		fMenu = (ksKey.fsState & ALT) || (pmCurrMenu != &mMain);
		DrawMenu (pmCurrMenu, sMenuSel, fMenu);
	   if (GetKey (&ksKey, 0, TRUE)) {
		   fContinue = ProcessKey (&ksKey, fMenu);
			//
			// After processing key, see if marking still enabled. If not
			// redraw the screen w/o mark.
			//
			if (pe->fFlags & FLAG_MARKFLAG) {
				if (!(fGlobalFlags & GFLAG_CONTMARK)) {
					pe->fFlags &= ~FLAG_MARKFLAG;
					DrawScreen (pe);
				}				
				fGlobalFlags &= ~GFLAG_CONTMARK;
			}
 		} else
			pe->lpGCollectPtr = FreeBuffSpace (pe, pe->lpGCollectPtr);

		fGlobalFlags = (ksKey.fsState & 0x00f0) | (fGlobalFlags & 0xff0f);
		DrawStatus (pe, 0, FALSE);
	}
	//
	// Clean up and exit
	//
	if (pe->hHandle != 0) {
		CloseFile (pe);
		if (pe->fFlags & FLAG_NEWFILE)
			DeleteFile (pe->szName);
	}
	//
	// Restore screen and cursor pos
	//
	if (selScreen) {
		SetCursor (usSaveRow, usSaveCol);
		RestoreScreen (selScreen);
	} else {
		ClearScreen (ucAtNorm);
		SetCursor (0, 0);
	}
	PrintCopyright(0);
	DosExit (1, 0);
	return 0;
}
//===================================================================
// Editor processing
//===================================================================
//-------------------------------------------------------------------
// ProcessKey - Acts on keys entered
//-------------------------------------------------------------------
BOOL ProcessKey (PKEYST pksKey, BOOL fMenu) {
   INT i;
	UINT usKeyCode;
	PMYMENU pmMenu;

	usKeyCode = ((pksKey->fsState & 0x000c) << 8) | pksKey->chScan;
	//
	// See if accel key
	//
	for (i = 0; i < dim (KeyProcs); i++) {
	   if (KeyProcs[i].Key == usKeyCode) {
			pmCurrMenu = &mMain;
	      return (*KeyProcs[i].Fxn)(pksKey);
		}
	}
	//
	// See if Menu key
	//
	if (fMenu) {
		for (i = 0; i < pmCurrMenu->sSize; i++) {
			if (pmCurrMenu->mMenu[i].Key == pksKey->chScan)
				if (pmCurrMenu->mMenu[i].usStatus & 0x08) {
					pmCurrMenu = (PMYMENU) pmCurrMenu->mMenu[i].Fxn;
					fGlobalFlags |= GFLAG_CONTMARK;
					return TRUE;
				} else {
					pmMenu = pmCurrMenu;
					pmCurrMenu = &mMain;
					return (*pmMenu->mMenu[i].Fxn)(pksKey);
				}
		}
		if (pksKey->chScan) {
			pmCurrMenu = &mMain;
		}
	} else	   	
		return DefaultKeyProc (pksKey);
}
//-------------------------------------------------------------------
// Start of edit functions
//-------------------------------------------------------------------
//-------------------------------------------------------------------
// DefaultKeyProc - Default processing for a key
//-------------------------------------------------------------------
BOOL DefaultKeyProc (PKEYST pksKey) {
	HPSZ pszChar;
	UINT usRow;
	INT i, rc, sLen;

	if ((pksKey->chChar) && (pksKey->chChar != 0xe0)) {
		usRow = pe->usCurRow;

		pszChar = GetScreenChar (pe, usRow, pe->usCurCol);

		if (fGlobalFlags & GFLAG_INSERT)
			pszChar = MakeSpace (pe, usRow, pe->usCurCol, 1, &rc);
		else if (*pszChar == '\0')
				pszChar = MakeSpace (pe, usRow, pe->usCurCol, 1, &rc);

		if (pszChar) {
			pksKey->fsState &= 0xfffc;
			*pszChar = pksKey->chChar;
			DrawScreenLine (pe, usRow);
			if (pksKey->chChar == '\t') {
				sLen = (INT) (pe->usTab - 
				      ((pe->usScrCol + pe->usCurCol) % pe->usTab));
				for (i = 0; i < sLen; i++)
					DoCurRight (pksKey);
			} else
				DoCurRight (pksKey);
		} else
			DrawStatus (pe, szErrMsgs[rc], TRUE);
		pe->fFlags |= FLAG_DIRTY;
	}
	pmCurrMenu = &mMain;
   return TRUE;
}
//-------------------------------------------------------------------
// DoNothing - Does nothing
//-------------------------------------------------------------------
BOOL DoNothing (PKEYST pksKey) {

	pmCurrMenu = &mMain;
   return TRUE;
}
//-------------------------------------------------------------------
// DoCompress - Compresses the edit buffer
//-------------------------------------------------------------------
BOOL DoCompress (PKEYST pksKey) {

	DrawStatus (pe, szCompressing, TRUE);
	CompressBuff (pe);
	DrawStatus (pe, szCmdDone, TRUE);
	return TRUE;
}
//-------------------------------------------------------------------
// DoNew - Open an uninitalized file
//-------------------------------------------------------------------
BOOL DoNew (PKEYST pksKey) {
	char szOut[4];
	INT rc;

	if (pe->fFlags & FLAG_DIRTY) {
		QueryUser (szFileNotSaved, szOut, (UINT) "yn", 2);
		if (szOut[0] == 'y') {
			rc = StoreFile (pe);
			if (rc) {
				DrawStatus (pe, szErrMsgs[rc], TRUE);
			  	return TRUE;
			}
			DrawStatus (pe, szFileSaved, TRUE);
		}
		pe->fFlags &= ~FLAG_DIRTY;
		pe->fFlags &= ~FLAG_NEWFILE;
	}
	rc = NewFile (pe);
	pe->szName[0] = '\0';
	MoveTo (pe, 0, 0, FALSE);
	DrawScreen (pe);
	DrawStatus (pe, 0, TRUE);
  	return TRUE;
}
//-------------------------------------------------------------------
// DoOpen - Open a new file
//-------------------------------------------------------------------
BOOL DoOpen (PKEYST pksKey) {
	char szName[MAXFNAMELEN];
	INT rc;

	if (pe->fFlags & FLAG_DIRTY) {
		QueryUser (szFileNotSaved, szName, (UINT)"yn", 2);
		if (szName[0] == 'y') {
			rc = StoreFile (pe);
			if (rc) {
				DrawStatus (pe, szErrMsgs[rc], TRUE);
			  	return TRUE;
			}
		}
	}
	if (QueryUser (szNamePrompt, szName, sizeof (szName), 1)) {
		_fullpath (pe->szName, szName, MAXFNAMELEN);
		strupr (pe->szName);
		rc = LoadFile (pe);
		if (rc) 
			DrawStatus (pe, szErrMsgs[rc], TRUE);
		else {
			DrawScreen (pe);
			MoveTo (pe, 0, 0, FALSE);
			DrawStatus (pe, 0, TRUE);
		}
	} else
		DrawStatus (pe, szErrMsgs[ERR_CANCELED], TRUE);
  	return TRUE;
}
//-------------------------------------------------------------------
// DoSave - Save the file
//-------------------------------------------------------------------
BOOL DoSave (PKEYST pksKey) {
	INT rc;

	rc = StoreFile (pe);
	if (rc)
		DrawStatus (pe, szErrMsgs[rc], TRUE);
	else {
		pe->fFlags &= ~FLAG_DIRTY;
		DrawStatus (pe, szFileSaved, TRUE);
	}
  	return TRUE;
}
//-------------------------------------------------------------------
// DoSaveAs - Rename and save 
//-------------------------------------------------------------------
BOOL DoSaveAs (PKEYST pksKey) {
	INT rc;
	char chFirst;

	chFirst = pe->szName[0];
	pe->szName[0] = '\0';
	rc = StoreFile (pe);
	if (rc) {
		if (rc == ERR_FILENOTSAVED)
			pe->szName[0] = chFirst;
		DrawStatus (pe, szErrMsgs[rc], TRUE);
	} else {
		pe->fFlags &= ~FLAG_DIRTY;
		DrawStatus (pe, szFileSaved, TRUE);
	}
	return TRUE;
}
//-------------------------------------------------------------------
// DoExit - Terminate the editor
//-------------------------------------------------------------------
BOOL DoExit (PKEYST pksKey) {
	char szOut[2];
	INT rc = ERR_CANCELED;

	if ((pe->fFlags & FLAG_DIRTY) == 0)
		return FALSE;

	if (QueryUser (szFileNotSaved, szOut, (UINT)"yn", 2)) {
		if (szOut[0] == 'y') {
			rc = StoreFile (pe);
			if (rc == 0)
				return FALSE;
		} else 
			return FALSE;
	}
	DrawStatus (pe, szErrMsgs[rc], TRUE);
	return TRUE;
}
//-------------------------------------------------------------------
// DoUndo - Undo prev cut, copy, paste or delete.
//-------------------------------------------------------------------
BOOL DoUndo (PKEYST pksKey) {
	HPSZ pszTemp;
	INT rc;

	if (pe->sUndoType) {
		pe->lSMarkRow = pe->lSUndoRow;
		pe->usSMarkCol = pe->usSUndoCol;
		pe->lEMarkRow = pe->lEUndoRow; 
		pe->usEMarkCol = pe->usEUndoCol;

		switch (pe->sUndoType) {

			case UNDO_DEL:
				MoveTo (pe, pe->lSUndoRow, pe->usSUndoCol, FALSE);
				rc = PasteBuff (pe, pe->lpUndoBuff);
				break;

			case UNDO_CUT:
				MoveTo (pe, pe->lSUndoRow, pe->usSUndoCol, FALSE);
				rc = PasteBuff (pe, pe->lpCutBuff);
				break;

			case UNDO_COPY:
				pszTemp = pe->lpUndoBuff;
				pe->lpUndoBuff = pe->lpCutBuff;
				pe->lpCutBuff = pszTemp;
				break;

			case UNDO_PASTE:
				EraseLine (pe, pe->lpUndoBuff);
				MoveTo (pe, pe->lSUndoRow, pe->usSUndoCol, FALSE);
				rc = CutCopy (pe, &pe->lpUndoBuff, TRUE);
				break;
		}
		DrawScreen (pe);
		pe->sUndoType = UNDO_NOTHING;
		pe->fFlags |= FLAG_DIRTY;
	} else
		rc = ERR_NOUNDO;
	if (rc)
		DrawStatus (pe, szErrMsgs[rc], TRUE);
	else
		DrawStatus (pe, szUndone, TRUE);
  	return TRUE;
}
//-------------------------------------------------------------------
// DoCut - Cuts the marked area to the clipboard
//-------------------------------------------------------------------
BOOL DoCut (PKEYST pksKey) {
	INT rc;
	UINT usStartCol, usEndCol;
	LONG lStartRow, lEndRow;

	if (pe->fFlags & FLAG_MARKFLAG) {
		EraseLine (pe, pe->lpUndoBuff);
		pe->lpUndoBuff = pe->lpCutBuff;

		rc = CutCopy (pe, &pe->lpCutBuff, TRUE);
		if (rc)
			pe->lpCutBuff = pe->lpUndoBuff;
		else {
			GetMarkedArea (pe, &lStartRow, &usStartCol, &lEndRow, &usEndCol);
			MoveTo (pe, lStartRow, usStartCol, FALSE);
			DrawStatus (pe, szCut, TRUE);
			SaveUndo (pe, UNDO_CUT, lStartRow, usStartCol, 0, 0);
			DrawScreen (pe);
		}
		pe->fFlags |= FLAG_DIRTY;
	} else
		rc = ERR_NOMARKED;
	if (rc)
		DrawStatus (pe, szErrMsgs[rc], TRUE);
	return TRUE;
}
//-------------------------------------------------------------------
// DoCopy - Copies the marked area to the clipboard
//-------------------------------------------------------------------
BOOL DoCopy (PKEYST pksKey) {
	INT rc;

	if (pe->fFlags & FLAG_MARKFLAG) {
		EraseLine (pe, pe->lpUndoBuff);
		pe->lpUndoBuff = pe->lpCutBuff;

		rc = CutCopy (pe, &pe->lpCutBuff, FALSE);
		if (rc)
			pe->lpCutBuff = pe->lpUndoBuff;
		else {
			DrawStatus (pe, szCopied, TRUE);
			SaveUndo (pe, UNDO_COPY, 0, 0, 0, 0);
		}
	} else
		rc = ERR_NOMARKED;
	if (rc)
		DrawStatus (pe, szErrMsgs[rc], TRUE);
	return TRUE;
}
//-------------------------------------------------------------------
// DoPaste - Pastes the clipboard to the current entry point
//-------------------------------------------------------------------
BOOL DoPaste (PKEYST pksKey) {
	INT rc;
	UINT usStartCol, usEndCol;
	LONG lStartRow, lEndRow;

	if (pe->lpCutBuff == 0) {
		DrawStatus (pe, szNothingInBuff, TRUE);
		return TRUE;
	}
	rc = PasteBuff (pe, pe->lpCutBuff);
	if (rc)
		DrawStatus (pe, szErrMsgs[rc], TRUE);
	else {
		DrawStatus (pe, szPasted, TRUE);
		GetMarkedArea (pe, &lStartRow, &usStartCol, &lEndRow, &usEndCol);
		SaveUndo (pe, UNDO_PASTE, 
		          pe->lScrRow + pe->usCurRow,
		          pe->usScrCol + pe->usCurCol,
		          pe->lScrRow + pe->usCurRow + (lEndRow - lStartRow),
		          pe->usScrCol + pe->usCurCol + (usEndCol - usStartCol));
	}
	DrawScreen (pe);
	MoveTo (pe, pe->lScrRow + pe->usCurRow, 
	        pe->usScrCol + pe->usCurCol, FALSE);
	pe->fFlags |= FLAG_DIRTY;
	return TRUE;
}
//-------------------------------------------------------------------
// DoSearch - Search for a string
//-------------------------------------------------------------------
BOOL DoSearch (PKEYST pksKey) {

	szSearchStr[0] = '\0';
	return DoRepSearch (pksKey);
}
//-------------------------------------------------------------------
// DoRepSearch - Repeat last search for a string
//-------------------------------------------------------------------
BOOL DoRepSearch (PKEYST pksKey) {
	LONG lLine;
	UINT usCol;
	BOOL fFound;

	if (szSearchStr[0] == '\0')
		if (!QueryUser (szSearchPrompt, szSearchStr, sizeof (szSearchStr),1)) {
			DrawStatus (pe, szErrMsgs[ERR_CANCELED], TRUE);
			return TRUE;
		}
	lLine = pe->lScrRow + pe->usCurRow;
	usCol = pe->usScrCol + pe->usCurCol;
	DrawStatus (pe, szSearching, TRUE);
	fFound = FindString (pe, szSearchStr, &lLine, &usCol, 
	                     fGlobalFlags & GFLAG_CHKCASE);
	if (fFound) {
		MoveTo (pe, lLine, usCol, FALSE);
		DrawStatus (pe, 0, TRUE);
	} else
		DrawStatus (pe, szNotFound, TRUE);

	return TRUE;
}
//-------------------------------------------------------------------
// MarkArea - Marks an area on a line
//-------------------------------------------------------------------
void MarkArea (PEDITSTATE pe, LONG lLine, UINT usCol, UINT usLen) {
	HPSZ pszLine;
	HPSZ pszChar;

	pszLine = GetFileLine (pe, lLine);
	pszChar = GetFileChar (pe, pszLine, usCol);

	pe->lSMarkRow = lLine;
	pe->usSMarkCol = GetColPos (pszLine, pszChar);
	pe->lEMarkRow = lLine;
	pe->usEMarkCol = GetColPos (pszLine, pszChar + usLen);
	pe->fFlags |= FLAG_MARKFLAG;
	DrawScreenLine (pe, (UINT)(lLine - pe->lScrRow));
	return;
}
//-------------------------------------------------------------------
// DoReplace - Search and replace string.
//-------------------------------------------------------------------
BOOL DoReplace (PKEYST pksKey) {
	LONG lLine, lSSRow;
	UINT usCol, usLen, usRepLen, usNum = 0;
	UINT usSCRow, usSSCol, usSCCol;
	char szAns[40];
	BOOL fFound = TRUE, fGlobal = FALSE;
	INT rc;

	if (!QueryUser (szSearchPrompt, szSearchStr, sizeof (szSearchStr),1)) {
		DrawStatus (pe, szErrMsgs[ERR_CANCELED], TRUE);
		return TRUE;
	}
	if (!QueryUser (szRepPrompt, szReplaceStr, sizeof (szReplaceStr),1)) {
		DrawStatus (pe, szErrMsgs[ERR_CANCELED], TRUE);
		return TRUE;
	}
	usLen = strlen (szSearchStr);
	usRepLen = strlen (szReplaceStr);
	lLine = pe->lScrRow + pe->usCurRow;
	usCol = pe->usScrCol + pe->usCurCol;
	rc = 0;
	while (fFound && (rc == 0)) {
		DrawStatus (pe, szSearching, TRUE);
		fFound = FindString (pe, szSearchStr, &lLine, &usCol, 
		                     fGlobalFlags & GFLAG_CHKCASE);
		if (fFound) {
			if (!fGlobal) {
				MoveTo (pe, lLine, usCol, FALSE);
				MarkArea (pe, lLine, usCol, usLen);
				if (QueryUser (szRep1Prompt, szAns, (UINT) "ynaq", 2)) {
					if (szAns[0] == 'a') {
						szAns[0] = 'y';
						fGlobal = TRUE;

						lSSRow = pe->lScrRow;
						usSCRow = pe->usCurRow;
						usSSCol = pe->usScrCol;
						usSCCol = pe->usCurCol;
					} else if (szAns[0] == 'q') 
						rc = ERR_CANCELED;
				} else
					rc = ERR_CANCELED;

				pe->fFlags &= ~FLAG_MARKFLAG;
				DrawScreenLine (pe, (UINT)(lLine - pe->lScrRow));
			} else {
				pe->lScrRow = lLine;
				pe->usCurRow = 0;
				pe->usScrCol = usCol;
				pe->usCurCol = 0;
			}
			if (szAns[0] == 'y') {
				rc = CutString (pe, lLine, usCol, usLen);
				if (rc == 0)
					rc = PasteBuff (pe, szReplaceStr);
				pe->fFlags |= FLAG_DIRTY;
				if (!fGlobal)
					DrawScreenLine (pe, (UINT)(lLine - pe->lScrRow));
				usNum++;
				usCol += usRepLen;
			} else
				usCol += usLen;
	 	}
	}
	if (rc)
		DrawStatus (pe, szErrMsgs[rc], TRUE);
	else if (fGlobal) {
		pe->lScrRow = lSSRow;
		pe->usCurRow = usSCRow;
		pe->usScrCol = usSSCol;
		pe->usCurCol = usSCCol;
		DrawScreen (pe);
		itoa (usNum, szAns, 10);
		strcat (szAns, " instance(s) changed.");
		DrawStatus (pe, szAns, TRUE);
	} else
		DrawStatus (pe, szNotFound, TRUE);
	return TRUE;
}
//-------------------------------------------------------------------
// DoGoto - Goto a line number
//-------------------------------------------------------------------
BOOL DoGoto (PKEYST pksKey) {
	char szNum[10];
	LONG lLine;

	if (QueryUser (szLinePrompt, szNum, sizeof (szNum), 3)) {
		lLine = atol (szNum) - 1;
		MoveTo (pe, lLine, pe->usScrCol + pe->usCurCol, FALSE);
		DrawStatus (pe, 0, TRUE);
	} else
		DrawStatus (pe, szErrMsgs[ERR_CANCELED], TRUE);

	return TRUE;
}
//-------------------------------------------------------------------
// DoTabs - Get a new tabstop value
//-------------------------------------------------------------------
BOOL DoTabs (PKEYST pksKey) {
	char szNum[4];
	UINT usOldTab;

	usOldTab = pe->usTab;
	if (QueryUser (szTabPrompt, szNum, sizeof (szNum), 3)) {
		pe->usTab = atoi (szNum);
		if (pe->usTab == 0)
			pe->usTab = 1;
		if (usOldTab != pe->usTab)
			DrawScreen(pe);
		DrawStatus (pe, 0, TRUE);
	} else
		DrawStatus (pe, szErrMsgs[ERR_CANCELED], TRUE);
	return TRUE;
}
//-------------------------------------------------------------------
// DoToggleCase - Toggle search respect case flag
//-------------------------------------------------------------------
BOOL DoToggleCase (PKEYST pksKey) {

	if (fGlobalFlags & GFLAG_CHKCASE) {
		strcpy (mConfig.mMenu[1].szName, "Respect &Case");
		fGlobalFlags &= ~GFLAG_CHKCASE;
	} else {
		strcpy (mConfig.mMenu[1].szName, "Ignore &Case");
		fGlobalFlags |= GFLAG_CHKCASE;
	}
	DrawStatus (pe, 0, TRUE);
  	return TRUE;
}
//-------------------------------------------------------------------
// DoKeyHelp - Display Help screen
//-------------------------------------------------------------------
BOOL DoKeyHelp (PKEYST pksKey) {

	return DrawHelp (szHelpMsgs, dim (szHelpMsgs));
}
//-------------------------------------------------------------------
// DoAbout - Display About string on status line
//-------------------------------------------------------------------
BOOL DoAbout (PKEYST pksKey) {

	return DrawHelp (szAboutMsg, dim (szAboutMsg));
}
//-------------------------------------------------------------------
// DoEnter - Enter key processing
//-------------------------------------------------------------------
BOOL DoEnter (PKEYST pksKey) {

	if (fGlobalFlags & GFLAG_INSERT)
		return DoSplitLine (pksKey);
	else
		return DoReturn (pksKey);
	return TRUE;
}
//-------------------------------------------------------------------
// DoReturn - Move cursor to start of next line
//-------------------------------------------------------------------
BOOL DoReturn (PKEYST pksKey) {
	MoveTo (pe, pe->lScrRow + pe->usCurRow + 1, 0, pksKey->fsState & 3);
	return TRUE;
}
//-------------------------------------------------------------------
// DoSplitLine - Splits the line at the cursor
//-------------------------------------------------------------------
BOOL DoSplitLine (PKEYST pksKey) {
	HPSZ pszChar;
	INT rc;

	pszChar = GetScreenChar (pe, pe->usCurRow, pe->usCurCol);
	if (*pszChar == '\0') {
		rc = AddFileLine (pe, pe->lScrRow + pe->usCurRow + 1, 0, 1);
		if (rc) {
			DrawStatus (pe, szErrMsgs[rc], TRUE);
			return TRUE;
		}
	} else {		
		pszChar = MakeSpace (pe, pe->usCurRow, pe->usCurCol, 1, &rc);
		if (pszChar != 0)
			rc = AddFileLine (pe, pe->lScrRow + pe->usCurRow + 1, pszChar+1, 1);
		if (rc) 
			DrawStatus (pe, szErrMsgs[rc], TRUE);
		else
			*pszChar = '\0';
	}
	ScrollScrDown (usEWinStrt + pe->usCurRow + 1, 
	               usEWinRows - pe->usCurRow - 2, 1, ucAtNorm);
	DrawScreenLine (pe, pe->usCurRow);
	DrawScreenLine (pe, pe->usCurRow + 1);
	MoveTo (pe, pe->lScrRow + pe->usCurRow + 1, 0, pksKey->fsState & 3);

	pe->fFlags |= FLAG_DIRTY;
	return TRUE;
}
//-------------------------------------------------------------------
// DoJoinLine - Appends the next line to the current line.
//-------------------------------------------------------------------
BOOL DoJoinLine (PKEYST pksKey) {
	INT rc;

	rc = JoinLine (pe, pe->lScrRow + pe->usCurRow);
	if (rc) {
		DrawStatus (pe, szErrMsgs[rc], TRUE);
		return TRUE;
	}
	ScrollScrUp (usEWinStrt + pe->usCurRow + 1, 
	             usEWinRows - pe->usCurRow - 2, 1, ucAtNorm);
	DrawScreenLine (pe, pe->usCurRow);
	DrawScreenLine (pe, usEWinRows-usEWinStrt);
	pe->fFlags |= FLAG_DIRTY;
	return TRUE;
}
//-------------------------------------------------------------------
// DoBackspace - Del prev char and backup
//-------------------------------------------------------------------
BOOL DoBackspace (PKEYST pksKey) {

	HPSZ pszLine;
	HPSZ pszChar;
	HPSZ pszPtr;

	pszLine = GetScreenLine (pe, pe->usCurRow);
	pszChar = GetScreenChar (pe, pe->usCurRow, pe->usCurCol);
	pszPtr = pszChar;

	if (pszLine != pszChar) {
		do {
			*(pszChar-1) = *pszChar;
		} while (*pszChar++ != '\0');
		pszLine = GetScreenLine (pe, pe->usCurRow);
		if (*pszLine == '\0')
			SetLinePtr (pe, pe->lScrRow + pe->usCurRow, 
			            MAKEP (pe->selEBuff, 0));
		DrawScreenLine (pe, pe->usCurRow);
		MoveTo (pe, pe->lScrRow + pe->usCurRow, 
		        GetColPos (pszLine, pszPtr-1), FALSE);
		pe->fFlags |= FLAG_DIRTY;
	}
	return TRUE;
}
//-------------------------------------------------------------------
// SaveUndo - Saves undo information in the edit state structure.
//-------------------------------------------------------------------
void SaveUndo (PEDITSTATE pe, INT sType, LONG lSRow, UINT usSCol, 
               LONG lERow, UINT usECol) {

	pe->sUndoType = sType;
	pe->lSUndoRow = lSRow;
	pe->usSUndoCol = usSCol;
	pe->lEUndoRow = lERow;
	pe->usEUndoCol = usECol;
	return;
}
//-------------------------------------------------------------------
// DoDeleteEOL - Deletes to the end of line.
//-------------------------------------------------------------------
BOOL DoDeleteEOL (PKEYST pksKey) {
	INT rc;
	HPSZ pszLine;

	pszLine = GetScreenChar (pe, pe->usCurRow, pe->usCurCol);
	if (*pszLine == 0)
		return TRUE;

	rc = CutString (pe, pe->lScrRow + pe->usCurRow, 
	                pe->usScrCol + pe->usCurCol, GetLineLen (pszLine));
	if (rc)
		DrawStatus (pe, szErrMsgs[rc], TRUE);
	else {
		SaveUndo (pe, UNDO_DEL, pe->lScrRow + pe->usCurRow, 
		          pe->usScrCol + pe->usCurCol, 0, 0);
		DrawStatus (pe, szDeleted, TRUE);
	}
	DrawScreenLine (pe, pe->usCurRow);
	pe->fFlags |= FLAG_DIRTY;
  	return TRUE;
}
//-------------------------------------------------------------------
// DoDelete - Delete character or marked area.
//-------------------------------------------------------------------
BOOL DoDelete (PKEYST pksKey) {
	INT rc;
	HPSZ pszChar;
	UINT usStartCol, usEndCol;
	LONG lStartRow, lEndRow;

	if (pe->fFlags & FLAG_MARKFLAG) {
		EraseLine (pe, pe->lpUndoBuff);
		rc = CutCopy (pe, &pe->lpUndoBuff, TRUE);
		if (rc)
			DrawStatus (pe, szErrMsgs[rc], TRUE);
		else {
			GetMarkedArea (pe, &lStartRow, &usStartCol, &lEndRow, &usEndCol);
			SaveUndo (pe, UNDO_DEL, lStartRow, usStartCol, 0, 0);
			MoveTo (pe, lStartRow, usStartCol, FALSE);
			DrawStatus (pe, szDeleted, TRUE);
		}
		DrawScreen (pe);
	} else {
		pszChar = GetScreenChar (pe, pe->usCurRow, pe->usCurCol);
		if (*pszChar == '\0') {
			pszChar = MakeSpace (pe, pe->usCurRow, pe->usCurCol, 1, &rc);
			*pszChar = '\0';
			return DoJoinLine (pksKey);
		}

		while (*pszChar != '\0') {
			*pszChar = *(pszChar+1);
			pszChar++;
		}
		pszChar = GetScreenLine (pe, pe->usCurRow);
		if (*pszChar == '\0')
			SetLinePtr (pe, pe->lScrRow + pe->usCurRow, 
			            MAKEP (pe->selEBuff, 0));
		DrawScreenLine (pe, pe->usCurRow);
	}
	pe->fFlags |= FLAG_DIRTY;
  	return TRUE;
}
//-------------------------------------------------------------------
// DoToggleInsert - Toggle Insert flag
//-------------------------------------------------------------------
BOOL DoToggleInsert (PKEYST pksKey) {

	if (fGlobalFlags & GFLAG_INSERT) 
		fGlobalFlags &= ~GFLAG_INSERT;
	else
		fGlobalFlags |= GFLAG_INSERT;
  	return TRUE;
}
//-------------------------------------------------------------------
// GoNextWord - Move cursor to the start of the next word
//-------------------------------------------------------------------
BOOL GoNextWord (PKEYST pksKey) {
	HPSZ pszChar;
	HPSZ pszLine;

	pszLine = GetScreenLine (pe, pe->usCurRow);
	pszChar = GetScreenChar (pe, pe->usCurRow, pe->usCurCol);

	while ((*pszChar != '\0') && (*pszChar > ' '))
		pszChar++;

	while (pe->lScrRow + pe->usCurRow < pe->lLines) {

		while ((*pszChar != '\0') && (*pszChar <= ' '))
			pszChar++;
		if (*pszChar > ' ')
			break;

		while ((*pszChar == '\0') && 
		       (pe->lScrRow + pe->usCurRow < pe->lLines)){
			MoveTo (pe, pe->lScrRow + pe->usCurRow + 1, 0, 
			        pksKey->fsState & 3);
			pszLine = GetScreenLine (pe, pe->usCurRow);
			pszChar = pszLine;
		}

	}
	MoveTo (pe, pe->lScrRow + pe->usCurRow, 
	        GetColPos (pszLine, pszChar), pksKey->fsState & 3);

  	return TRUE;
}
//-------------------------------------------------------------------
// GoPrevWord - Move cursor to the start of the prev word
//-------------------------------------------------------------------
BOOL GoPrevWord (PKEYST pksKey) {
	HPSZ pszChar;
	HPSZ pszLine;

	pszLine = GetScreenLine (pe, pe->usCurRow);
	pszChar = GetScreenChar (pe, pe->usCurRow, pe->usCurCol);
	if ((pszLine == pszChar) && (pe->lScrRow + pe->usCurRow == 0))
		return TRUE;

	do {
		while ((pszChar == pszLine) && (pe->lScrRow + pe->usCurRow)) {
			MoveTo (pe, pe->lScrRow + pe->usCurRow - 1, 0, pksKey->fsState & 3);
			pszLine = GetScreenLine (pe, pe->usCurRow);
			pszChar = GetScreenChar (pe, pe->usCurRow, GetLineLen (pszLine));
		}
		pszChar--;		
	} while (*pszChar <= ' ');

	while (pszChar > pszLine) {
		if (*(pszChar-1) <= ' ')
			break;
		pszChar--;
	}
	MoveTo (pe, pe->lScrRow + pe->usCurRow, 
	        GetColPos (pszLine, pszChar), pksKey->fsState & 3);

  	return TRUE;
}
//-------------------------------------------------------------------
// GoScreenTop - Move cursor to top of screen
//-------------------------------------------------------------------
BOOL GoScreenTop (PKEYST pksKey) {
	
	MoveTo (pe, pe->lScrRow, pe->usScrCol + pe->usCurCol, 
	        pksKey->fsState & 3);
	return TRUE;
}
//-------------------------------------------------------------------
// GoScreenBottom - Move cursor to bottom of screen
//-------------------------------------------------------------------
BOOL GoScreenBottom (PKEYST pksKey) {
	
	MoveTo (pe, pe->lScrRow + usEWinRows-usEWinStrt, 
	        pe->usScrCol + pe->usCurCol, pksKey->fsState & 3);
	return TRUE;
}
//-------------------------------------------------------------------
// DoPageUp - Scroll the screen up
//-------------------------------------------------------------------
BOOL DoPageUp (PKEYST pksKey) {

	MoveTo (pe, pe->lScrRow + pe->usCurRow - usEWinRows,
   	         pe->usScrCol + pe->usCurCol, pksKey->fsState & 3);
	return TRUE;
}
//-------------------------------------------------------------------
// DoPageDown - Scroll the screen down
//-------------------------------------------------------------------
BOOL DoPageDown (PKEYST pksKey) {

	MoveTo (pe, pe->lScrRow + pe->usCurRow + usEWinRows,
   	         pe->usScrCol + pe->usCurCol, pksKey->fsState & 3);
	return TRUE;
}
//-------------------------------------------------------------------
// DoCurDown - Move the cursor down one line  >>>
//-------------------------------------------------------------------
BOOL DoCurDown (PKEYST pksKey) {

	MoveTo (pe, pe->lScrRow + pe->usCurRow + 1, 
	        pe->usScrCol + pe->usCurCol, pksKey->fsState & 3);
	return TRUE;
}
//-------------------------------------------------------------------
// DoCurUp - Move the cursor up one line
//-------------------------------------------------------------------
BOOL DoCurUp (PKEYST pksKey) {

	MoveTo (pe, pe->lScrRow + pe->usCurRow - 1, 
	        pe->usScrCol + pe->usCurCol, pksKey->fsState & 3);
	return TRUE;
}
//-------------------------------------------------------------------
// DoCurLeft - Move the cursor left one column
//-------------------------------------------------------------------
BOOL DoCurLeft (PKEYST pksKey) {

	if (pe->usScrCol + pe->usCurCol != 0)
		MoveTo (pe, pe->lScrRow + pe->usCurRow, 
		            pe->usScrCol + pe->usCurCol - 1, pksKey->fsState & 3);
	return TRUE;
}
//-------------------------------------------------------------------
// DoCurRight - Move the cursor right one column
//-------------------------------------------------------------------
BOOL DoCurRight (PKEYST pksKey) {

	if (pe->usScrCol + pe->usCurCol != 0xffff)
		MoveTo (pe, pe->lScrRow + pe->usCurRow, 
		            pe->usScrCol + pe->usCurCol + 1, pksKey->fsState & 3);
	return TRUE;
}
//-------------------------------------------------------------------
// GoFileTop - Move cursor to top of file
//-------------------------------------------------------------------
BOOL GoFileTop (PKEYST pksKey) {

	MoveTo (pe, 0, 0, pksKey->fsState & 3);
	return TRUE;
}
//-------------------------------------------------------------------
// GoFileEnd - Move cursor to end of file
//-------------------------------------------------------------------
BOOL GoFileEnd (PKEYST pksKey) {

	MoveTo (pe, pe->lLines-1, 
	        GetLineLen (GetFileLine (pe, pe->lLines-1)), 
			  pksKey->fsState & 3);
	return TRUE;
}
//-------------------------------------------------------------------
// GoLineStart - Move cursor to left most column
//-------------------------------------------------------------------
BOOL GoLineStart (PKEYST pksKey) {
	
	MoveTo (pe, pe->lScrRow + pe->usCurRow, 0, pksKey->fsState & 3);
	return TRUE;
}
//-------------------------------------------------------------------
// GoLineEnd - Move cursor to end of line
//-------------------------------------------------------------------
BOOL GoLineEnd (PKEYST pksKey) {

	MoveTo (pe, pe->lScrRow + pe->usCurRow, 
	        GetLineLen (GetScreenLine (pe, pe->usCurRow)), 
			  pksKey->fsState & 3);
	return TRUE;
}
//-------------------------------------------------------------------
// Start of support functons
//-------------------------------------------------------------------
//-------------------------------------------------------------------
// NewFile - Resets selectors for a new file
//-------------------------------------------------------------------
INT NewFile (PEDITSTATE peb) {

	LONG _huge *lpPtrs;
	HPSZ	lpData;

	LONG i;

	if (peb->selEBuff)
		FreeMem (peb->selEBuff);
		
	if (peb->selPBuff)
		FreeMem (peb->selPBuff);

	peb->selPBuff = AllocMem ((LONG)NUMPTRS * sizeof (ULONG), 
	                     MAXNUMPTRS * sizeof (ULONG));
	peb->lNumPtrs = NUMPTRS;
	peb->selEBuff = AllocMem (EDITBUFFSTART, EDITBUFFSTART*4);
	
	if ((peb->selEBuff == 0) || (peb->selPBuff == 0))
		return ERR_OUTOFMEM;

	peb->lEBuffSize = EDITBUFFSTART;
	peb->lpEEnd = MAKEP (peb->selEBuff, 2);
	peb->lpESize = MAKEP (peb->selEBuff, EDITBUFFSTART);

	lpPtrs = MAKEP (peb->selPBuff, 0);
	lpData = (HPSZ) MAKEP (peb->selEBuff, 1);

	for (i = 0; i < peb->lNumPtrs; i++)
		SetLinePtr (pe, i, lpData);

	lpPtrs = MAKEP (peb->selEBuff, 0);
	*(PULONG)lpPtrs = 0;

	peb->hHandle = 0;
	peb->lSize = 0;
	peb->lLines = 0;
	peb->fFlags = 0;
	peb->lpGCollectPtr = 0;
	return 0;
} 
//-------------------------------------------------------------------
// LoadFile - Loads a file into memory
//-------------------------------------------------------------------
INT LoadFile (PEDITSTATE peb) {
	HPSZ lpData;
	LONG lLine, j;
	INT rc;

	rc = InitFile (peb);
	if (rc)
		return rc;
		
	if (peb->selPBuff)
		FreeMem (peb->selPBuff);

	if (peb->selEBuff)
		FreeMem (peb->selEBuff);

	peb->selPBuff = AllocMem ((LONG)NUMPTRS * sizeof (ULONG), 
	                     MAXNUMPTRS * sizeof (ULONG));
	peb->lNumPtrs = NUMPTRS;

	peb->selEBuff = AllocMem (peb->lSize+EDITBUFFEXTRA+15, 
	                          peb->lSize+(EDITBUFFEXTRA*4)+15);
	if (peb->selEBuff == 0) {
		CloseFile (peb);
		return ERR_OUTOFMEM;
	}

	peb->fFlags &= ~FLAG_MARKFLAG;
	peb->fFlags &= ~FLAG_BINFILE;
	peb->lEBuffSize = peb->lSize + EDITBUFFEXTRA;

	peb->lpESize = MAKEP (peb->selEBuff, 0);
	peb->lpESize += peb->lEBuffSize;

	lpData = MAKEP (peb->selEBuff, 0);
	*lpData++ = '\0';
	*lpData++ = '\0';
	rc = ReadFile (peb, lpData);
	if (rc) {
		CloseFile (peb);
		return rc;
	}
	j = 0;
	lLine = 0;
	while (j < peb->lSize) {
		if (*lpData != CR) {
			SetLinePtr (peb, lLine, lpData);
			while ((*lpData != CR) && (j < peb->lSize)) {
				if (*lpData == '\0') {
					*lpData = FILLCHAR;
					peb->fFlags |= FLAG_BINFILE;
				}
				lpData++;
				j++;
			}
		} else
			SetLinePtr (peb, lLine, MAKEP (peb->selEBuff, 0));
		*lpData++ = '\0';
		j++;
		lLine++;
		if (j >= peb->lSize)
			break;
		if (*lpData == LF) {
			*lpData++ = '\0';
			j++;
		}
		if (lLine >= peb->lNumPtrs-1) {
			rc = ReallocMem (peb->selPBuff, 
			                 (peb->lNumPtrs+1024) * sizeof (ULONG));
			if (rc)
				return ERR_TOOMANYLINES;
			peb->lNumPtrs += 1024;				
		}
	}
	*lpData = '\0';
	peb->lpEEnd = lpData;
	peb->lLines = lLine;
	lpData = MAKEP (peb->selEBuff, 1);
	while (lLine < peb->lNumPtrs) 
		SetLinePtr (peb, lLine++, lpData);

	peb->lpGCollectPtr = 0;
	return 0;
} 
//-------------------------------------------------------------------
// StoreFile - Writes a file to disk
//-------------------------------------------------------------------
INT StoreFile (PEDITSTATE peb) {

	HPSZ lpData;
	HPSZ lpEnd;
	char szName[MAXFNAMELEN];
	HFILE hHandle;
	char szLineEnd[2];
	INT rc;
	LONG i;
	UINT usLen;

	if (peb->szName[0] == '\0') {
		if (!QueryUser (szNamePrompt, szName, sizeof (szName), 1)) {
			return ERR_CANCELED;
		}
		rc = OpenFile (szName, &hHandle, FILE_CREATE);
		if (rc) {
			if (rc == ERR_FILEEXISTS) {

				QueryUser (szFileExists, szLineEnd, (UINT) "yn", 2);
				if (szLineEnd[0] != 'y')
					return ERR_FILENOTSAVED;

				rc = OpenFile (szName, &hHandle, 
				               FILE_OPEN | FILE_CREATE);
			}
		}
		if (rc)
			return rc;
		else {
			CloseFile (peb);
			peb->hHandle = hHandle;
			strcpy (peb->szName, szName);
		}
	}
	DrawStatus (pe, szStoring, TRUE);
	szLineEnd[0] = CR;
	szLineEnd[1] = LF;
	lpEnd = MAKEP (peb->selEBuff, 1);

	CloseFile (peb);
	rc = OpenFile (peb->szName, &peb->hHandle, FILE_TRUNCATE);
	if (rc)
		return rc;

	for (i = 0; (i < peb->lNumPtrs) && (rc == 0); i++) {
		lpData = GetFileLine (peb, i);
		if (lpData == lpEnd)
			break;
		for (usLen = 0; *(lpData+usLen) != '\0' && usLen < 0xffff; usLen++)
			;
		if (usLen) 
			rc = WriteFile (peb, lpData, usLen);
		if (!rc)
			rc = WriteFile (peb, szLineEnd, 2);
	}
	pe->fFlags &= ~FLAG_NEWFILE;
	return rc;
} 
//-------------------------------------------------------------------
// GetMarkedArea - Returns the bounds of a marked area
//-------------------------------------------------------------------
void GetMarkedArea (PEDITSTATE peb, LONG *lStartRow, UINT *usStartCol,
                    LONG *lEndRow, UINT *usEndCol) {

	if (peb->lSMarkRow < peb->lEMarkRow) {
		*lStartRow = peb->lSMarkRow;
		*usStartCol = peb->usSMarkCol;
		*lEndRow = peb->lEMarkRow;
		*usEndCol = peb->usEMarkCol;
	} else {
		*lStartRow = peb->lEMarkRow;
		*lEndRow = peb->lSMarkRow;
		if ((peb->lSMarkRow == peb->lEMarkRow) &&
		    (peb->usSMarkCol < peb->usEMarkCol)) {
			*usStartCol = peb->usSMarkCol;
			*usEndCol = peb->usEMarkCol;
		} else {
			*usStartCol = peb->usEMarkCol;
			*usEndCol = peb->usSMarkCol;
		}
	}
	return;
} 
//-------------------------------------------------------------------
// DrawScreenLine - Draws a line from the file in the edit window
//-------------------------------------------------------------------
void DrawScreenLine (PEDITSTATE peb, UINT usRow) {
	PSZ lpszData;
	LONG lFileRow;
	UINT usDrawRow, usStartCol, usEndCol;
	UINT usProgCol, usProgEnd;
	LONG lStartRow, lEndRow;

	lpszData = GetScreenChar (peb, usRow, 0);
	lFileRow = peb->lScrRow + usRow;
	usDrawRow = usRow + usEWinStrt;
	if (usDrawRow > usEWinRows)
		return;
	//
	// If no marked area, just draw line.
	//
	if (!(peb->fFlags & FLAG_MARKFLAG)) {
		WriteLine (lpszData, ucAtNorm, usDrawRow);
		return;
	}
	// Get boundry of marked area
	GetMarkedArea (peb, &lStartRow, &usStartCol, &lEndRow, &usEndCol);
	//
	// If a middle line in a block, just write entire line as marked.
	// If line not in marked block, just write a normal line.
	//
	if ((lFileRow > lStartRow) && (lFileRow < lEndRow)) {
		WriteLine (lpszData, ucAtMark, usDrawRow);
		return;
	} else if ((lFileRow < lStartRow) || (lFileRow > lEndRow)) {
		WriteLine (lpszData, ucAtNorm, usDrawRow);
		return;
	}
	//
	// Draw unmarked start of line if starting line
	//
	usProgCol = 0;
	if (lFileRow == lStartRow) {
			 
		if (usStartCol > pe->usScrCol) {
			usProgCol = usStartCol - pe->usScrCol;
			WriteString (lpszData, usProgCol, ucAtNorm, usDrawRow, 0);
			
			lpszData = GetFileChar (peb, lpszData, usProgCol);
		}
	}
	//
	// Draw Marked part of line
	//
	if ((lFileRow == lEndRow) && (usEndCol < pe->usScrCol + usCols))
		usProgEnd = min (usEndCol - pe->usScrCol, usCols);
	else
		usProgEnd = usCols;

	WriteString (lpszData, usProgEnd - usProgCol, ucAtMark, usDrawRow, 
	             usProgCol);
	//
	// Draw unmarked end of line if necessary
	//
	if (usProgEnd < usCols) {
		lpszData = GetFileChar (peb, lpszData, usProgEnd - usProgCol);

		WriteString (lpszData, usCols-usProgEnd, ucAtNorm, usDrawRow, 
		             usProgEnd);
	}
	return;
}
//-------------------------------------------------------------------
// DrawScreen - Draws the edit window
//-------------------------------------------------------------------
void DrawScreen (PEDITSTATE peb) {
	UINT i;

	for (i = 0; i < usEWinRows; i++) {
		DrawScreenLine (peb, i);
	}
	return;
}
//-------------------------------------------------------------------
// DrawMenu - Draws the menu bar on the screen
//-------------------------------------------------------------------
void DrawMenu (PMYMENU pmMenu, INT sActive, UINT fHigh) {
	static PMYMENU pmOld = 0;
	static INT sOldAct = 0;
	static BOOL fOldHigh = 0;
	char	*pszName;
	INT	i, j, sCol;

	if ((pmMenu == pmOld) && (sOldAct == sActive) &&
	    (fOldHigh == fHigh))
		return;
	sCol = 0;	
	for (i = 0; i < pmMenu->sSize; i++) {
		j = 1;
	 	MyWriteChar (' ', ucAtMenu, usMenuRow, sCol++);
		pszName = pmMenu->mMenu[i].szName;
		while ((*pszName != '\0') && (j < MAXMENULEN)) {
			if (*pszName == '&') {
				pszName++;
				if ((sActive > 0) || (fHigh))
					MyWriteChar (*pszName++, ucAtMenuHot, usMenuRow, sCol++);
				else
					MyWriteChar (*pszName++, ucAtMenu, usMenuRow, sCol++);
			} else
				MyWriteChar (*pszName++, ucAtMenu, usMenuRow, sCol++);
			j++;	
		}
	 	MyWriteChar (' ', ucAtMenu, usMenuRow, sCol++);
		j++;	
	}
	for (; j < (INT) usCols; j++)
		MyWriteChar (' ', ucAtMenu, usMenuRow, sCol++);
	pmOld = pmMenu;
	sOldAct = sActive;
	fOldHigh = fHigh;
	return;
}
//-------------------------------------------------------------------
// DrawHelp - Display Help screen
//-------------------------------------------------------------------
BOOL DrawHelp (char **ppszStrArray, INT sNumLines) {
	KEYST	ksKey;
	INT i;
	UINT usOldTab, usSaveRow, usSaveCol;

	GetCursor (&usSaveRow, &usSaveCol);
	SetCursor (usStatRow, 26);
	usOldTab = pe->usTab;
	pe->usTab = 20;
	ClearScreen (ucAtNorm);
	for (i = 0; i < sNumLines; i++) {
		WriteLine (ppszStrArray[i], ucAtNorm, i);
	}

	DrawStatus (pe, "Press any key to continue", TRUE);
	GetKey (&ksKey, 0, FALSE);

	pe->usTab = usOldTab;
	SetCursor (usSaveRow, usSaveCol);
	DrawScreen (pe);
	DrawStatus (pe, 0, TRUE);
  	return TRUE;
}
//-------------------------------------------------------------------
// DrawStatus - Draws the status bar on the screen
//-------------------------------------------------------------------
void DrawStatus (PEDITSTATE peb, char *szMsg, BOOL fForce) {

#define LINESIZE        13
#define COLSIZE         12

	char	szStr[40];
	UINT usColPtr;

	static LONG lOldRow;
	static UINT usOldCol; 
	static UINT fOldFlags; 

	if (usCols < 80)
		usColPtr = usCols;
	else
		usColPtr = usCols - 30;
	if (szMsg || fForce) {
		if (szMsg)
			WriteString (szMsg, usColPtr, ucAtStat, usStatRow, 0);
		else if (peb->hHandle)
			WriteString (peb->szName, usColPtr, ucAtStat, usStatRow, 0);
		else
			WriteString (szAboutMsg[1], usColPtr, ucAtStat, usStatRow, 0);
	}
	if (usCols < 80)
		return;

	if ((lOldRow != peb->lScrRow + peb->usCurRow + 1) || fForce) {
		lOldRow = peb->lScrRow + peb->usCurRow + 1;
		strcpy (szStr, "  Line: ");
		ltoa (lOldRow, &szStr[8], 10);
		WriteString (szStr, LINESIZE, ucAtStat, usStatRow, usColPtr);
	}
	usColPtr += LINESIZE;
	if ((usOldCol != peb->usScrCol + peb->usCurCol + 1) || fForce) {
		usOldCol = peb->usScrCol + peb->usCurCol + 1;
		strcpy (szStr, "  Col: ");
		ltoa (usOldCol, &szStr[7], 10);
		WriteString (szStr, COLSIZE, ucAtStat, usStatRow, usColPtr);
	}
	usColPtr += COLSIZE;
	if ((fOldFlags != fGlobalFlags) || fForce) {
		strcpy (szStr, "        ");
		if (fGlobalFlags & GFLAG_CAPLOCK)
			szStr[0] = 24;
		if (fGlobalFlags & GFLAG_INSERT)
			szStr[1] = 'I';
		else
			szStr[1] = 'O';
		if (fGlobalFlags & GFLAG_NUMLOCK)
			szStr[2] = '#';
		if (fGlobalFlags & GFLAG_CHKCASE)
			szStr[3] = 'S';
		else
			szStr[3] = 's';

		WriteString (szStr, usCols - usColPtr, ucAtStat, usStatRow, usColPtr);
		fOldFlags = fGlobalFlags;
	}
   return;
}
//-------------------------------------------------------------------
// QueryUser - Asks a question of the user.
//	modes   1 - Get string, wParam = Max chars
//         2 - Get letter, wParam points to allowable chars
//         3 - Get number, wParam = Max chars
//
// Returns 0 - Esc pressed   1 - Enter pressed
//-------------------------------------------------------------------
INT QueryUser (char *pszQuestion, char *pszOut, UINT wParam, INT sMode) { 
	KEYST	ksQKey;
	UINT usOldRow, usOldCol, usBaseCol;
	char *pszPtr;
	char *pszPtr1;
	INT sState, i, sLen, sTab = 8;

	GetCursor (&usOldRow, &usOldCol);

	WriteLine (pszQuestion, ucAtStat, usStatRow);
	usBaseCol = strlen (pszQuestion)+1;
	SetCursor (usStatRow, usBaseCol);
	sState = 2;

	pszPtr = pszOut;
	*pszPtr = '\0';
	while (sState == 2) {
	   GetKey (&ksQKey, 0, FALSE);
		switch (ksQKey.chScan) {
			case ENTER:
				sState = 1;
				break;
			case ESC:
				sState = 0;
				break;
			case HOME:
				pszPtr = pszOut;
				break;
			case END:
				while (*pszPtr != '\0') 
					pszPtr++;
				break;
			case RIGHT:
				if (*pszPtr != '\0') 
					pszPtr++;
				break;
			case DEL:
				pszPtr1 = pszPtr;
				while (*pszPtr1 != '\0') {
					*pszPtr1 = *(pszPtr1+1);
					pszPtr1++;
				}
				break;
			case BKSP:
				if (pszPtr > pszOut) {
					pszPtr1 = pszPtr;
					do {
						*(pszPtr1-1) = *pszPtr1;
					} while (*pszPtr1++ != '\0');
				}
				//Backspace code falls through here, don't move this.
			case LEFT:
				if (pszPtr > pszOut)
					pszPtr--;
				break;
			case TAB:
				sLen = sTab - ((pszPtr-pszOut) % sTab);
				for (i=0; i < sLen && (pszPtr - pszOut < (INT)wParam-1); i++) {
					if (*pszPtr == '\0') 
						*(pszPtr+1) = '\0';
					*pszPtr++ = ' ';
				}
				break;
			default:
				if ((ksQKey.chChar) && (ksQKey.chChar != 0xe0)) {
					if (*pszPtr == '\0') 
						*(pszPtr+1) = '\0';
					switch (sMode) {
						case 1:
							if (pszPtr < pszOut + wParam - 1)
								*pszPtr++ = ksQKey.chChar;
							break;
						case 2:
							if ((ksQKey.chChar >= 'A') &&
							    (ksQKey.chChar <= 'Z'))
								ksQKey.chChar |= 0x20;
							if (strchr ((char *)wParam, ksQKey.chChar)) {
								*pszPtr++ = ksQKey.chChar;
								sState = 1;
							}
							break;
						case 3:
							if (pszPtr < pszOut + wParam - 1)
								if ((ksQKey.chChar >= '0') && 
								    (ksQKey.chChar <= '9'))
								*pszPtr++ = ksQKey.chChar;
							break;
					}
				} 
				break;
		}
		WriteString (pszQuestion, usBaseCol-1, ucAtStat, usStatRow, 0);
		WriteString (pszOut, usCols - usBaseCol, ucAtStat, 
		             usStatRow, usBaseCol);
		SetCursor (usStatRow, usBaseCol + pszPtr - pszOut);
	}
	SetCursor (usOldRow, usOldCol);
	return sState;
}
//-------------------------------------------------------------------
// GetLineLen - Returns the column length of a line
//-------------------------------------------------------------------
UINT GetLineLen (HPSZ pszStr) {

	return GetColPos (pszStr, 0);
}
//-------------------------------------------------------------------
// GetColPos - Returns the screen column pos for a character
//-------------------------------------------------------------------
UINT GetColPos (HPSZ pszLine, HPSZ pszChar) {

	UINT usCol;

	usCol = 0;
	while ((*pszLine != '\0') && (pszLine != pszChar)) {
		if (*pszLine == '\t') {
			usCol += pe->usTab - (usCol % pe->usTab);
		} else
			usCol++;
		pszLine++;
	}
	return usCol;
}
//-------------------------------------------------------------------
// MoveTo - Moves the cursor (and screen) to a specific line and
// column.
//-------------------------------------------------------------------
void MoveTo (PEDITSTATE pe, LONG lLine, UINT usCol, BOOL fMark) {
	UINT i, usOldRow;

	if (lLine < 0)
		lLine = 0;
	else if (lLine >= pe->lNumPtrs)
		lLine = pe->lNumPtrs-1;

	if (fMark) {
		if (!(pe->fFlags & FLAG_MARKFLAG)) {
			pe->lSMarkRow = pe->lScrRow + pe->usCurRow;
			pe->usSMarkCol = pe->usScrCol + pe->usCurCol;
		}
		pe->lEMarkRow = lLine;
		pe->usEMarkCol = usCol;
		pe->fFlags |= FLAG_MARKFLAG;
		fGlobalFlags |= GFLAG_CONTMARK;
	}
	if ((lLine >= pe->lScrRow) && (lLine < pe->lScrRow + usEWinRows) &&
	    (usCol >= pe->usScrCol) && (usCol < pe->usScrCol + usCols)) {

		usOldRow = pe->usCurRow;
		pe->usCurRow = (UINT) (lLine - pe->lScrRow);
		pe->usCurCol = usCol - pe->usScrCol;

		if (pe->fFlags & FLAG_MARKFLAG) {
			if (usOldRow >= pe->usCurRow)
				for (i = pe->usCurRow; i <= usOldRow; i++)
					DrawScreenLine (pe, i);
			else
				for (i = usOldRow; i <= pe->usCurRow; i++)
					DrawScreenLine (pe, i);
		}
	} else if ((usCol >= pe->usScrCol) && (usCol < pe->usScrCol + usCols)) {


		if (pe->lScrRow + pe->usCurRow + 1 == lLine) {
			ScrollScrUp (usEWinStrt, usEWinRows-usEWinStrt, 1, ucAtNorm);
			pe->lScrRow++;
			DrawScreenLine (pe, usEWinRows-usEWinStrt-1);
			DrawScreenLine (pe, usEWinRows-usEWinStrt);
		} else if (pe->lScrRow + pe->usCurRow - 1 == lLine) {
			pe->lScrRow--;
			ScrollScrDown (usEWinStrt, usEWinRows-usEWinStrt, 1, ucAtNorm);
			DrawScreenLine (pe, 0);
			DrawScreenLine (pe, 1);
		} else {
			pe->usCurCol = usCol - pe->usScrCol;
			pe->lScrRow = lLine - pe->usCurRow;
			if (pe->lScrRow < 0) {
				pe->lScrRow = 0;
				pe->usCurRow = (UINT) lLine; 
			}
			DrawScreen (pe);
		}
	} else {
		if (usCol < pe->usScrCol) {
			if (usCol < usCols) {
				pe->usScrCol = 0;
				pe->usCurCol = usCol;
			} else {
				pe->usScrCol = usCol - usCols + 5;
				pe->usCurCol = usCols - 5;
			}	
		} else if (usCol >= pe->usScrCol + usCols) {
			pe->usScrCol = usCol - usCols + 5;
			pe->usCurCol = usCols - 5;
		} else
			pe->usCurCol = usCol - pe->usScrCol;

		pe->lScrRow = lLine - pe->usCurRow;
		if (pe->lScrRow < 0) {
			pe->lScrRow = 0;
			pe->usCurRow = (UINT) lLine; 
		}
		DrawScreen (pe);
	}
	SetCursor (pe->usCurRow + usEWinStrt, pe->usCurCol);
	return;
}
//-------------------------------------------------------------------
// MakeSpace - Inserts a blank char in a line then returns a 
// pointer to that blank char.  Insert point passed in
// screen coordinates.
//-------------------------------------------------------------------
HPSZ MakeSpace (PEDITSTATE peb, UINT usRow, UINT usCol, LONG lIns, 
               INT *psRC) {
	HPSZ pszStart;
	HPSZ pszChar;
	HPSZ pszEnd;
	LONG i;
	UINT usPad, j;

	*psRC = 0;
	pszStart = GetScreenLine (peb, usRow);
	//
	// If new line past end of file, add blank lines to fill in
	//
	if (pszStart == MAKEP (peb->selEBuff,1)) {

		for (i = pe->lLines; i <= pe->lScrRow + usRow; i++) {
			SetLinePtr (peb, i, MAKEP (peb->selEBuff,0));
			peb->lLines++;
		}
		pszStart = GetScreenLine (peb, usRow);
	}			
	//
	// If blank line, point to end of buffer.
	//
	if (pszStart == MAKEP (peb->selEBuff, 0)) {
		peb->lpEEnd++;
		SetLinePtr (peb, peb->lScrRow + usRow, peb->lpEEnd);
		pszStart = peb->lpEEnd;
		*peb->lpEEnd = '\0';
	}		
	//
	// Find insert point in current line
	//
	pszChar = GetScreenChar (peb, usRow, usCol);
	pszEnd = pszChar;
	while (*pszEnd != '\0')
		pszEnd++;
	//
	// See if we need to pad the end of the line.
	//
	usPad = 0;
	if (*pszChar == '\0')
		usPad = pe->usScrCol + usCol - GetColPos (pszStart, pszChar);
	//
	// Limit check line length
	//
	i = pszEnd - pszStart;
	if (lIns + usPad + i > 0xFFFF) {
		*psRC = ERR_LINETOOLONG;
		return 0;
	}
	//
	// If line not at end of buff, move it there.
	//
	if (pszEnd != peb->lpEEnd) {
		i = pszChar - pszStart;
		pszStart = MoveToEnd (peb, pe->lScrRow + usRow);
		if (pszStart == 0) {
			*psRC = ERR_OUTOFMEM;
			return 0;
		}
		pszChar = pszStart + (UINT) i;
		pszEnd = peb->lpEEnd;
	}
	//
	// Make room for extra characters
	//
	if (pszEnd + usPad + lIns > peb->lpESize) { 
		i = max (lIns + usPad, EDITBUFFEXTRA);
		if (ReallocMem (peb->selEBuff, peb->lEBuffSize + i)) {
			*psRC = ERR_OUTOFMEM;
			return 0;
		}
		peb->lEBuffSize += i;
		peb->lpESize += i;
	}
	//
	// If necessary, pad line to start of insert point.
	//
	if (usPad) {
		for (j = 0; j < usPad; j++)
			*pszChar++ = FILLCHAR;
		peb->lpEEnd = pszChar;
	}
	//
	// Slide line to make enough room for insert characters.
	//
	pszEnd = peb->lpEEnd;
	while (pszEnd >= pszChar) {
		*(pszEnd + lIns) = *pszEnd;
	 	pszEnd--;
	}
	peb->lpEEnd += lIns;
	*peb->lpEEnd = '\0';
	return pszChar;	
}
//-------------------------------------------------------------------
// JoinLine - Appends the next line to the current line.
//-------------------------------------------------------------------
INT JoinLine (PEDITSTATE pe, LONG lRow) {
	HPSZ pszLine;

	if (MoveToEnd (pe, lRow++) == 0)
		return ERR_OUTOFMEM;

	pe->lpEEnd--;                           //Backup over term zero.

	pszLine = GetFileLine (pe, lRow);
	if (CopyToEnd (pe, pszLine) == 0)
		return ERR_OUTOFMEM;

	DelFileLine (pe, lRow, 1);
	EraseLine (pe, pszLine);
	return 0;
}
//-------------------------------------------------------------------
// FindString - Search for string
//-------------------------------------------------------------------
BOOL FindString (PEDITSTATE pe, char *pszFindStr, LONG *lLine, 
                 UINT *usCol, BOOL fChkCase) {
	HPSZ pszEnd;
	HPSZ pszChar;
	HPSZ pszCmp;
	HPSZ pszLine;
	INT i, sLen;
	BOOL fFound = FALSE;
	char szStr[50], ch;

	strcpy (szStr, pszFindStr);
	if (!fChkCase)
		strlwr (szStr);
	sLen = strlen (szStr);
	pszEnd = MAKEP (pe->selEBuff, 1);

	pszLine = GetFileLine (pe, *lLine);
	pszChar = GetFileChar (pe, pszLine, *usCol);
	do {
		while (*pszChar != '\0') {
			pszCmp = pszChar;
			if (fChkCase) {
				for (i = 0; i < sLen; i++) 
					if (szStr[i] != *pszCmp++)
						break;
			} else {
				for (i = 0; i < sLen; i++) {
					ch = *pszCmp++;
					if ((ch >= 'A') && (ch <= 'Z'))
						ch |= (BYTE) 0x20;
					if (szStr[i] != ch)
						break;
				}
			}
			if (i == sLen) {
				fFound = TRUE;
				break;
			}
			pszChar++;
		}
		if (!fFound) {
			(*lLine)++;
			pszLine = GetFileLine (pe, *lLine);
			pszChar = pszLine;
		}
	} while (*lLine < pe->lLines && !fFound);
	if (fFound)
		*usCol = GetColPos (pszLine, pszChar);
	return fFound;
}
//-------------------------------------------------------------------
// PasteBuff - Pastes a buffer to the current insert point
//-------------------------------------------------------------------
INT PasteBuff (PEDITSTATE pe, HPSZ pszBuff) {
	INT rc;
	HPSZ pszChar;
	LONG lLines, lChars;

	// Determine number of characters and lines in paste buff
	pszChar = pszBuff;
	lLines = lChars = 0;
	while (*pszChar != '\0') {
		if (*pszChar == CR)
			lLines++;
		lChars++;
		pszChar++;
	}
	//Make room for new characters
	pszChar = MakeSpace (pe, pe->usCurRow, pe->usCurCol, lChars, &rc);
	if (pszChar == 0)
		return rc;

	//Make room for the new lines
	if (pe->usScrCol + pe->usCurCol)
		rc = AddFileLine (pe, pe->lScrRow + pe->usCurRow, 
		                  GetScreenLine (pe, pe->usCurRow), lLines);
	else
		rc = AddFileLine (pe, pe->lScrRow + pe->usCurRow, pszChar, lLines);
	if (rc)
		return rc;

	//Copy the buff into the line.  Add line pointers every CR
	lLines = pe->lScrRow + pe->usCurRow;
	while (*pszBuff != '\0') {
		if (*pszBuff == CR) {
			if (*(pszChar-1) == '\0')
				SetLinePtr (pe, lLines, MAKEP (pe->selEBuff, 0));
			lLines++;
			*pszChar++ = '\0';
			pszBuff++;
			SetLinePtr (pe, lLines, pszChar);
		} else
			*pszChar++ = *pszBuff++;
	}
	pszBuff = GetFileLine (pe, lLines);
	if ((*pszBuff == 0) && (pszBuff != MAKEP (pe->selEBuff, 0)))
		SetLinePtr (pe, lLines, MAKEP (pe->selEBuff, 0));
	return rc;
}

//-------------------------------------------------------------------
// CutString - Cuts a string from the text
//-------------------------------------------------------------------
INT CutString (PEDITSTATE pe, LONG lLine, UINT usCol, UINT usLen) {
	HPSZ pszLine;

	pszLine = GetFileChar (pe, GetFileLine (pe, lLine), usCol);
	pe->lSMarkRow = lLine;
	pe->usSMarkCol = usCol;
	pe->lEMarkRow = lLine;
	pe->usEMarkCol = usCol + usLen;

	EraseLine (pe, pe->lpUndoBuff);
	return CutCopy (pe, &pe->lpUndoBuff, TRUE);
}
//-------------------------------------------------------------------
// CutCopy - Cuts or copies the marked area to the clipboard
//-------------------------------------------------------------------
INT CutCopy (PEDITSTATE pe, HPSZ *lpCutBuffPtr, BOOL bCut) {
	LONG i, lStartRow, lEndRow;
	UINT usStartCol, usEndCol;
	INT rc = 0;

	GetMarkedArea (pe, &lStartRow, &usStartCol, &lEndRow, &usEndCol);
	if (lStartRow == lEndRow) {
		*lpCutBuffPtr = CutCopyLine (pe, lStartRow, usStartCol,
		                             usEndCol, bCut);
		if (*lpCutBuffPtr == 0)
			return ERR_OUTOFMEM;

		*pe->lpEEnd = '\0';
		return 0;
	}
	*lpCutBuffPtr = CutCopyLine (pe, lStartRow, usStartCol,
	                             0xffff, bCut);
	if (*lpCutBuffPtr == 0)
		return ERR_OUTOFMEM;

	for (i = lStartRow + 1; i < lEndRow && rc == 0; i++)
		if (CutCopyLine (pe, i, 0, 0xffff, bCut) == 0) 
			rc = ERR_OUTOFMEM;
	if (rc == 0)
		if (CutCopyLine (pe, lEndRow, 0, usEndCol, bCut) == 0)
			rc = ERR_OUTOFMEM;

	//Terminate cutbuffer data with zero;
	*pe->lpEEnd = '\0';
	if (rc)
		return rc;	
	if (bCut) {

		i = lStartRow;
		if (*GetLinePtr (pe, i) != (HPSZ)-1)
			i++;	
		if (*GetLinePtr (pe, i) == (HPSZ)-1) {
			if (*GetLinePtr (pe, lEndRow) != (HPSZ)-1)
				DelFileLine (pe, i, lEndRow - i);
			else
				DelFileLine (pe, i, lEndRow - i + 1);
		}
		if ((lStartRow != lEndRow) && (usStartCol)) {
			JoinLine (pe, lStartRow);

		}
	}
	return rc;
}
//-------------------------------------------------------------------
// CutCopyLine - Copies a line to the clipboard then deletes it
// if necessary.
//-------------------------------------------------------------------
HPSZ CutCopyLine (PEDITSTATE pe, LONG lRow, UINT usStartCol,
                  UINT usEndCol, BOOL bCut) {
	HPSZ pszClip;
	HPSZ pszEnd;
	HPSZ pszChar;

	if (usStartCol)
		pszChar = GetFileChar (pe, GetFileLine (pe, lRow), usStartCol);
	else
		pszChar = GetFileLine (pe, lRow);

	pszClip = CopyToClip (pe, pszChar, usEndCol - usStartCol);
	if (pszClip == 0) {
		CompressBuff (pe);
		if (usStartCol)
			pszChar = GetFileChar (pe, GetFileLine (pe, lRow), usStartCol);
		else
			pszChar = GetFileLine (pe, lRow);

		pszEnd = GetFileChar (pe, GetFileLine (pe, lRow), usEndCol);
		pszClip = CopyToClip (pe, pszChar, (UINT)(pszEnd - pszChar));
	}
	if (bCut) {
		pszEnd = GetFileChar (pe, GetFileLine (pe, lRow), usEndCol);
		//If complete line, delete it
		if (*pszEnd == '\0') {
			if (usStartCol == 0) 
				SetLinePtr (pe, lRow, (HPSZ)-1);
		} else
			//Copy end of line over cut characters
			while (*pszEnd != '\0')    
				*pszChar++ = *pszEnd++;

		EraseLine (pe, pszChar);	//Erase remainder of line
	}
	return pszClip;
}
//-------------------------------------------------------------------
// CopyToClip - Copies a line to the end of the edit buffer.  The
// terminating zero is replaced by a CR.
//-------------------------------------------------------------------
HPSZ CopyToClip (PEDITSTATE peb, HPSZ pszChar, UINT usLen) {
	HPSZ lpStart;

	if (usLen == 0) 
		lpStart = CopyToEnd (peb, "");
	else
		lpStart = CopyToEnd (peb, pszChar);
	if (lpStart == 0)
		return 0;

	if (usLen != 0xffff) 
	if (peb->lpEEnd > lpStart + usLen)
		peb->lpEEnd = lpStart + usLen;
	*peb->lpEEnd = CR;
	return lpStart;
}
//-------------------------------------------------------------------
// MoveToEnd - Moves a line to the end of the Edit buffer. 
//-------------------------------------------------------------------
HPSZ MoveToEnd (PEDITSTATE peb, LONG lRow) {
	HPSZ pszLine;
	HPSZ pszNew;

	pszLine = GetFileLine (peb, lRow);
	pszNew = CopyToEnd (peb, pszLine);
	if (pszNew == 0)
		return 0;
	SetLinePtr (peb, lRow, pszNew);
	EraseLine (peb, pszLine);
	return pszNew;
}
//-------------------------------------------------------------------
// CopyToEnd - Copies a line to the end of the Edit buffer. 
//-------------------------------------------------------------------
HPSZ CopyToEnd (PEDITSTATE peb, HPSZ pszLine) {
	HPSZ lpStart;
	LONG i, j;

	for (i = 0; *(pszLine+i) != '\0'; i++)
		;

	i++;
	if (peb->lpEEnd + i > peb->lpESize) {
		j = max (i, EDITBUFFEXTRA);
		if (ReallocMem (peb->selEBuff, peb->lEBuffSize + j)) {
			j = i;
			if (ReallocMem (peb->selEBuff, peb->lEBuffSize + j))
				return 0;
		}
		peb->lEBuffSize += j;
		peb->lpESize += j;
	}
	peb->lpEEnd++;
	lpStart = peb->lpEEnd;
	while (*pszLine != '\0')
		*peb->lpEEnd++ = *pszLine++;
	*peb->lpEEnd = '\0';
	return lpStart;
}
//-------------------------------------------------------------------
// EraseLine - Zeros out a line in the edit buffer.
//-------------------------------------------------------------------
void EraseLine (PEDITSTATE peb, HPSZ pszChar) {
	HPSZ pszStart;

	if (pszChar == 0)
		return;	
	pszStart = pszChar;
	while ((*pszChar != '\0') && (pszChar < peb->lpEEnd))
		*pszChar++ = '\0';

	if (pszChar == peb->lpEEnd)
		peb->lpEEnd = pszStart;
	return; 
}
//-------------------------------------------------------------------
// CompressBuff - Compresses the edit buffer.
//-------------------------------------------------------------------
void CompressBuff (PEDITSTATE pe) {
	HPSZ lpPtr;
	HPSZ lpPtrOld;

	lpPtr = FreeBuffSpace (pe, 0);	
	while (lpPtr > lpPtrOld) {
		lpPtrOld = lpPtr; 
		lpPtr = FreeBuffSpace (pe, lpPtrOld);
	}
	pe->lpGCollectPtr = 0;
	return;
}
//-------------------------------------------------------------------
// FreeBuffSpace - Moves free edit buffer space to the end of the
// buffer.
//-------------------------------------------------------------------
HPSZ FreeBuffSpace (PEDITSTATE pe, HPSZ lpszStart) {
	HPSZ lpszNext;
	LONG i;
	static LONG lLastLine;
	BOOL fFound = FALSE;
char szTemp [80];
HPSZ lpszTemp;

	if (pe->lLines == 0)
		return 0;
	if (lpszStart == 0) {
		lpszStart = (HPSZ) MAKEP (pe->selEBuff, 2);
		lLastLine = 0;
	}
	//
	//Find next free space in buffer.  If at end, return ptr to
	// start of buffer for next pass.
	//
	while ((*lpszStart != '\0') && (lpszStart < pe->lpEEnd))
		lpszStart++;
	
	if (lpszStart >= pe->lpEEnd) 
		return (HPSZ) MAKEP (pe->selEBuff, 2);
	//
	//Point to next char, scan to find next string. If at end,
	// adjust end ptr to last occupied space. If no gap, return
	// with updated starting pointer.
	//
	lpszStart++;
	lpszNext = lpszStart;
	while ((*lpszNext == '\0') && (lpszNext < pe->lpEEnd)) {
		lpszNext++;
	}
	if (lpszNext >= pe->lpEEnd) {              //If at end, move end
		pe->lpEEnd = lpszStart-1;               //ptr to last valid char.
		return (HPSZ) MAKEP (pe->selEBuff, 2);
	}
	if (lpszNext == lpszStart)                 //If no gap, return.
		return lpszStart;
	//
	// Find line in line ptr buff. 
	//
	for (i = lLastLine; i < pe->lLines; i++)
		if (GetFileLine (pe, i) == lpszNext) {
			fFound = TRUE;
			break;
		}
	if (!fFound)
		for (i = 0; i < pe->lLines; i++)
			if (GetFileLine (pe, i) == lpszNext) {
				fFound = TRUE;
				break;
			}
	if (!fFound) {
		if (pe->lpCutBuff == lpszNext)
			pe->lpCutBuff = lpszStart;
		else if (pe->lpUndoBuff == lpszNext)
			pe->lpUndoBuff = lpszStart;
//
//Debug code
//
		else {
			strcpy (szTemp, "*** Lost string ***   ");
			lpszTemp = lpszNext;
			for (i = 20; (i < 79) && (*lpszTemp != 0); i++);
				szTemp[i] = *lpszTemp++;
			szTemp[i] = 0;
			DrawStatus (pe, szTemp, TRUE);
		}		
//
//
//
	} else {
		SetLinePtr (pe, i, lpszStart);
		lLastLine = i;
	}
	//
	//Move line into gap in buffer.
	//
	while (*lpszNext != '\0') {
		*lpszStart++ = *lpszNext;
 		*lpszNext++ = '\0';
	}
	*lpszStart = '\0';
	return lpszStart;
}
//-------------------------------------------------------------------
// GetScreenChar - Returns a pointer to a character on the screen
//-------------------------------------------------------------------
HPSZ GetScreenChar (PEDITSTATE peb, UINT usRow, UINT usCol) {
	HPSZ pszLine;

	pszLine = GetFileLine (peb, peb->lScrRow + usRow);
	return GetFileChar (peb, pszLine, peb->usScrCol+usCol);
}
//-------------------------------------------------------------------
// GetScreenLine - Returns a pointer to a line on the screen
//-------------------------------------------------------------------
HPSZ GetScreenLine (PEDITSTATE peb, UINT usRow) {

	return GetFileLine (peb, peb->lScrRow + usRow);
}
//-------------------------------------------------------------------
// GetFileChar - Returns a pointer to a character in the file
//-------------------------------------------------------------------
HPSZ GetFileChar (PEDITSTATE peb, HPSZ pszLine, UINT usCol) {
	UINT i;

	for (i = 0; *pszLine != '\0' && i < usCol; i++) {
		if (*pszLine == '\t')
			i += (pe->usTab - i % pe->usTab - 1);
		if (i >= usCol)
 			break;
 		pszLine++;
	}
	return pszLine;
}
//-------------------------------------------------------------------
// GetFileLine - Returns a pointer to a line from the file
//-------------------------------------------------------------------
HPSZ GetFileLine (PEDITSTATE peb, LONG lLine) {

	return (HPSZ) *GetLinePtr (peb, lLine);
}
//-------------------------------------------------------------------
// AddFileLine - Adds (lRows) to the file after (lLine)
//-------------------------------------------------------------------
INT AddFileLine (PEDITSTATE pe, LONG lLine, HPSZ lpStr, LONG lRows) {
	LONG i;

	if (lRows == 0)
		return 0;
	if (pe->lLines + lRows < pe->lNumPtrs) {

		for (i = pe->lLines; i >= lLine; i--)
			SetLinePtr (pe, i+lRows, *GetLinePtr (pe, i));

		if (lpStr)
			SetLinePtr (pe, lLine, lpStr);
		else
			SetLinePtr (pe, lLine, MAKEP (pe->selEBuff, 0));

		pe->lLines += lRows;
		pe->lpGCollectPtr = 0;          //Reset garbage collect routine
	} else
		return ERR_TOOMANYLINES;
	return 0;
}
//-------------------------------------------------------------------
// DelFileLine - Deletes a line from the file
//-------------------------------------------------------------------
HPSZ DelFileLine (PEDITSTATE pe, LONG lLine, LONG lNum) {
	LONG i;
	HPSZ lpszStr;

	lpszStr = *GetLinePtr (pe, lLine);
	if (lLine < pe->lLines) {
		for (i = lLine; i < pe->lLines; i++)
			SetLinePtr (pe, i, *GetLinePtr (pe, i+lNum));
		pe->lLines -= lNum;
	}
	return lpszStr;
}
//-------------------------------------------------------------------
// GetLinePtr - Returns a pointer to a line pointer
//-------------------------------------------------------------------
HPSZ far *GetLinePtr (PEDITSTATE peb, LONG lLine) {
	HPSZ _huge *lpPtrs;
	static HPSZ lpBeyond;

	if (lLine < peb->lNumPtrs) {
		lpPtrs = MAKEP (peb->selPBuff, 0);
		return (lpPtrs + lLine);
	} else {
		lpBeyond = MAKEP (pe->selEBuff, 1);
		return &lpBeyond;
	}
}
//-------------------------------------------------------------------
// SetLinePtr - Sets a line pointer
//-------------------------------------------------------------------
void SetLinePtr (PEDITSTATE peb, LONG lLine, HPSZ lpPtr) {
	HPSZ _huge *lpPtrs;

	if (lLine < peb->lNumPtrs) {
		lpPtrs = MAKEP (peb->selPBuff, 0);
		*(lpPtrs + lLine) = lpPtr;
	}
	return;
}
//-------------------------------------------------------------------
// Clear screen - Clears the screen with the desired attribute
//-------------------------------------------------------------------
void ClearScreen (BYTE ucAttr) {
	return ScrollScrUp (0, -1, -1, ucAttr);
}
//-------------------------------------------------------------------
// GetParm - Returns the number after a cmd line switch
//-------------------------------------------------------------------
LONG GetParam (char **argv, INT *sNum, INT sNumLeft) {
	char *pszStr;

	if (*(argv[*sNum]+2) != '\0')				//See if number immediately
		pszStr = argv[*sNum]+2;					//after switch or if space
	else {											//in between.
		if (sNumLeft == 1)
			return 0;
		(*sNum)++;
		pszStr = argv[*sNum];
	}		
	return atol (pszStr);
}
	
//-------------------------------------------------------------------
// ParseCmdLine - Parses the command line.
//-------------------------------------------------------------------
INT ParseCmdLine (INT argc, char **argv, char *pszFilename,
                  LONG *lSRow, UINT * usSCol) {
	BOOL	fFirst = TRUE; 
	INT	i, j;
	LONG	lTemp;

	char	szFName[MAXFNAMELEN];

	*pszFilename = '\0';
	for (i = 1; i < argc; i++) {
		if (*argv[i] == '/' || *argv[i] == '-') {
			switch (*(argv[i]+1)) {
				case '?':
					return ERR_HELP;

				case 'l':
				case 'L':
					*lSRow = GetParam (argv, &i, argc - i);
					if (*lSRow != 0)
						(*lSRow)--;
					break;

				case 'c':
				case 'C':
					lTemp = GetParam (argv, &i, argc - i);
					if (lTemp > 0xFFFF)
						return ERR_COLTOOBIG;
					if (lTemp != 0)
						lTemp--;
					*usSCol = (UINT) lTemp;
					break;

				case 't':
				case 'T':
					lTemp = GetParam (argv, &i, argc - i);
					if ((lTemp > 255) || (lTemp == 0))
						return ERR_TABTOOBIG;
					pe->usTab = (UINT) lTemp;
					break;

				default:
					return ERR_SYNTAX;
			}
		} else {
			if (!fFirst)
				return ERR_SYNTAX;
			fFirst = FALSE;
			for (j = 0; (j < MAXFNAMELEN) && (argv[i][j] > ' ') &&				
				         !strchr ("?*-+/<>", argv[i][j]); j++) {

				szFName[j] = argv[i][j];
			}
			szFName[j] = '\0';
			_fullpath (pszFilename, szFName, MAXFNAMELEN);
		}
	}
	strupr (pszFilename);
	return 0;
}
//-------------------------------------------------------------------
// WriteLine - Writes a complete line on the screen.	$$$$
//-------------------------------------------------------------------
void WriteLine (HPSZ pszString, BYTE chAttr, UINT usRow) {

	return WriteString (pszString, usCols, chAttr, usRow, 0);
}
//-------------------------------------------------------------------
// PrintCopyright - Prints the copyright message
//-------------------------------------------------------------------
void PrintCopyright (INT rc) {
	INT i;

	for (i = 0; i < 3; i++) {
		printf (szAboutMsg[i]);
		printf ("\n");
	}
	if (rc) {
		printf ("\n");
		if (rc == ERR_HELP) {
			for (i = 0; i < dim (szSyntaxText); i++) {
				printf ("\n");
				printf (szSyntaxText[i]);
			}
		} else
			printf (szErrMsgs[rc]);

		printf ("\n\n");
	}
	return;
}
//===================================================================
// Routines below this point make operating system calls
//===================================================================
//-------------------------------------------------------------------
// AllocMem - Allocates a memory selector
//-------------------------------------------------------------------
UINT AllocMem (LONG lSize, LONG lMaxSize) {
	UINT usSel, rc, usMaxNum;

	if (lSize > lMaxSize)
		lMaxSize = lSize;

	usMaxNum = (LOUSHORT (lMaxSize)) ? HIUSHORT (lMaxSize) + 1 : 
	                                   HIUSHORT (lMaxSize);

	rc = DosAllocHuge (HIUSHORT (lSize), LOUSHORT (lSize), 
	                   &usSel, usMaxNum, 0);
	if (rc)
		return 0;
	else
		return usSel;
}
//-------------------------------------------------------------------
// ReallocMem - Resizes a memory selector
//-------------------------------------------------------------------
UINT ReallocMem (UINT usSel, LONG lNewSize) {

	return DosReallocHuge (HIUSHORT (lNewSize), LOUSHORT (lNewSize), 
	                       usSel);

}
//-------------------------------------------------------------------
// FreeMem - Free memory selector
//-------------------------------------------------------------------
void FreeMem (UINT usSel) {
	DosFreeSeg (usSel);
	return;
}
//-------------------------------------------------------------------
// InitFile - Opens a file and initializes the necessary structures
//-------------------------------------------------------------------
INT InitFile (PEDITSTATE pfile) {
   UINT	rc;
   LONG	lTemp;

	rc = OpenFile (pfile->szName, &pfile->hHandle, FILE_OPEN);
	if (rc) {
		pfile->fFlags |= FLAG_NEWFILE;
		rc = OpenFile (pfile->szName, &pfile->hHandle, FILE_CREATE);
	}
	if (rc == 0) {
	   pfile->fFlags &= ~FLAG_DIRTY;
	   pfile->lLines = 0;
	   DosChgFilePtr (pfile->hHandle, 0, FILE_END, &pfile->lSize);
	   DosChgFilePtr (pfile->hHandle, 0, FILE_BEGIN, &lTemp);
	}
	return rc;
}	                
//-------------------------------------------------------------------
// OpenFile - Opens a file for editing
//-------------------------------------------------------------------
INT OpenFile (char *pszName, HFILE *phHandle, UINT usOpenType) {
   UINT	usAction, rc;

	rc = DosOpen (pszName, phHandle, &usAction, 0, 0, 
	               usOpenType, OPEN_SHARE_DENYREADWRITE | 
					   OPEN_ACCESS_READWRITE, 0L);
	if (rc) {
		if (rc == 0x6E)
			rc= ERR_FILEEXISTS;
		else if (rc > 8)
			rc = ERR_OPENERROR;
		else
			rc += ERR_DOSERROR;
	}
	return rc;
}	                
//-------------------------------------------------------------------
// ReadFile - Read data from a file
//-------------------------------------------------------------------
INT ReadFile (PEDITSTATE pfile, HPSZ lpData) {
   UINT usBytesRead = 0, usReadBytes, rc = 0;
	LONG lSize = 0;

	while ((rc == 0) && (lSize < pfile->lSize)) {
		if (pfile->lSize > 32768)
	      usReadBytes = 32768;
		else
			usReadBytes = (UINT) pfile->lSize;
		//
		//Don't read across segment boundry
		//
		if ((LONG)usReadBytes + (LONG) OFFSETOF (lpData) > 0x10000)
			usReadBytes = (UINT)(0x10000 - OFFSETOF (lpData));

		rc = DosRead (pfile->hHandle, lpData, usReadBytes, &usBytesRead);
		lpData += usBytesRead;
		lSize += usBytesRead;
		if (usBytesRead < usReadBytes)
			break;
	}
	return rc;
}	                
//-------------------------------------------------------------------
// WriteFile - Write data to a file
//-------------------------------------------------------------------
INT WriteFile (PEDITSTATE peb, HPSZ lpData, UINT usLen) {

   UINT usBytesWritten, rc = 0;
   LONG	lSize = 0;

	if (usLen) {
		rc = DosWrite (peb->hHandle, lpData, usLen, &usBytesWritten);
		if (rc)
			rc = ERR_DOSERROR;
		else if (usBytesWritten < usLen)
			rc = ERR_DISKFULL;
	}
	return rc;
}	                
//-------------------------------------------------------------------
// MoveFilePtr - Moves the read/write pointer for a file
//-------------------------------------------------------------------
LONG MoveFilePtr (PEDITSTATE peb, LONG lPtr, INT sBase) {
	LONG lNewPtr;

	DosChgFilePtr (peb->hHandle, lPtr, sBase, &lNewPtr);
	return lNewPtr;
}	                
//-------------------------------------------------------------------
// CloseFile - Closes a file
//-------------------------------------------------------------------
INT CloseFile (PEDITSTATE pfile) {

	return DosClose (pfile->hHandle);
}	                
//-------------------------------------------------------------------
// DeleteFile - Deletes a file
//-------------------------------------------------------------------
INT DeleteFile (char *szName) {

	return DosDelete (szName, 0);
}	                
//-------------------------------------------------------------------
// Yield - Yields ctrl
//
// Need to fix because stupid BIND makers didn't do an Int 28!
//
//-------------------------------------------------------------------
void Yield (void) {

	DosSleep (0);
	return;
}
//-------------------------------------------------------------------
// GetKey - Reads a key from the keyboard
// Need GetStatus because stupid BIND makers didn't report shift
// status on peek.
//-------------------------------------------------------------------
BOOL GetKey (PKEYST pksKey, UINT usKbdHdl, BOOL fPeek) {
   KBDKEYINFO	kbiInKey;
   KBDINFO	kbiStatus;
	
	memset (&kbiInKey, 0, sizeof (kbiInKey));
	if (fPeek)
	   KbdCharIn (&kbiInKey, 1, usKbdHdl);
	else 
	   KbdCharIn (&kbiInKey, 0, usKbdHdl);

	kbiStatus.cb = 10;
   KbdGetStatus (&kbiStatus, usKbdHdl);
	
	pksKey->fsState = kbiStatus.fsState;
	if (pksKey->fsState & 1)               //Combine shift key flags
		pksKey->fsState |= 3;
	else if (pksKey->fsState & 2)
		pksKey->fsState |= 3;
	pksKey->chChar = kbiInKey.chChar;
	pksKey->chScan = kbiInKey.chScan;
	if (kbiInKey.chScan == 0xe0)
		switch (kbiInKey.chChar) {
			case 0x0a:
			case 0x0d:
				pksKey->chScan = 0x1c;
				break;
			case 0x2f:
				pksKey->chScan = 0x35;
				break;
		}				

	if (fPeek) {
//		Yield ();
		if (!(kbiInKey.fbStatus & 0x40)) 
			return FALSE;
		else
			return TRUE;
	}
	return TRUE;
}
//-------------------------------------------------------------------
// InitKeyboard - Sets the keyboard mode
//-------------------------------------------------------------------
void InitKeyboard (UINT usKbdHdl) {
   KBDINFO	kbiStatus;

	kbiStatus.cb = 10;
   KbdGetStatus (&kbiStatus, usKbdHdl);
	
	kbiStatus.fsMask = KEYBOARD_ECHO_OFF | KEYBOARD_BINARY_MODE;
   KbdSetStatus (&kbiStatus, usKbdHdl);
	return;
}

//-------------------------------------------------------------------
// MyWriteChar - Writes a character to the screen
//------------------------------------------------------------------
void MyWriteChar (BYTE chChar, BYTE chAttr, UINT usRow, UINT usCol) {

	BYTE	chCell[2];
	
	chCell[0] = chChar;
	chCell[1] = chAttr;
	VioWrtNCell (chCell, 1, usRow, usCol, 0);   
	return;
}
//-------------------------------------------------------------------
// WriteString - Writes a string to the screen
//-------------------------------------------------------------------
void WriteString (HPSZ pszIn, UINT usLen, BYTE chAttr, 
                  UINT usRow, UINT usSCol) {

	BYTE	chCell[2];
	UINT usSL, usLoc;
	HPSZ pszStr;

	chCell[0] = ' ';
	chCell[1] = chAttr;
	usLen = min (usLen, usCols - usSCol);
	
	usSL = 0;
	usLoc = usSCol;
	pszStr = pszIn;
	while ((*pszStr != '\0') && (usLoc + usSL < usLen + usSCol)) {
		if (*pszStr == '\t') {
			*pszStr = 0;
			VioWrtCharStrAtt (pszIn, usSL, usRow, usLoc, &chAttr, 0);
			*pszStr = '\t';
			usLoc += usSL;
			usSL = min (pe->usTab - (usLoc % pe->usTab), usCols);
			VioWrtNCell (chCell, usSL, usRow, usLoc, 0);   
			usLoc += usSL;
			pszIn = pszStr+1;
			usSL = 0;
		} else
			usSL++;
		pszStr++;
	}
	VioWrtCharStrAtt (pszIn, usSL, usRow, usLoc, &chAttr, 0);
	usLoc += usSL;
	if (usLoc < usSCol + usLen)
		VioWrtNCell (chCell, (usSCol + usLen) - usLoc, usRow, usLoc, 0);   
	return; 
}
//-------------------------------------------------------------------
// Scroll Scr Up - Scrolls the screen up x rows
//-------------------------------------------------------------------
void ScrollScrUp (SHORT sTop, SHORT sNum, SHORT sRows, BYTE ucAttr) {
	UINT	wCell;

	wCell = ((UINT) ucAttr << 8) | 0x20;
	VioScrollUp (sTop, 0, sTop+sNum, -1, sRows, (PBYTE) &wCell, 0);
}
//-------------------------------------------------------------------
// Scroll Scr Down - Scrolls the screen down x rows
//-------------------------------------------------------------------
void ScrollScrDown (SHORT sTop, SHORT sNum, SHORT sRows, BYTE ucAttr) {
	UINT	wCell;

	wCell = ((UINT) ucAttr << 8) | 0x20;
	VioScrollDn (sTop, 0, sTop+sNum, -1, sRows, (PBYTE) &wCell, 0);
}
//-------------------------------------------------------------------
// SetCursor - Moves cursor to specified location on screen.
//-------------------------------------------------------------------
void SetCursor (UINT usRow, UINT usCol) {

	VioSetCurPos (usRow, usCol, 0);
	return;
}
//-------------------------------------------------------------------
// GetCursor - Returns the cursor position
//-------------------------------------------------------------------
void GetCursor (UINT *usRow, UINT *usCol) {

	VioGetCurPos (usRow, usCol, 0);
	return;
}
//-------------------------------------------------------------------
// SaveScreen - Saves the screen 
//-------------------------------------------------------------------
void SaveScreen (UINT *pselScr) {
	UINT i, usLen;
	PCH lpBuff;

 	if (*pselScr == 0)
		return;
	lpBuff = MAKEP (*pselScr, 0);

	for (i = 0; i < usRows; i++) {
		usLen = usCols * 2;
		VioReadCellStr (lpBuff, &usLen, i, 0, 0);
		lpBuff += (usCols * 2);
 	}
	return;
}
//-------------------------------------------------------------------
// RestoreScreen - Restores the screen 
//-------------------------------------------------------------------
void RestoreScreen (UINT selScr) {
	UINT i;
	PCH lpBuff;

	if (selScr == 0)
		return;

	lpBuff = MAKEP (selScr, 0);

	for (i = 0; i < usRows; i++) {
		VioWrtCellStr (lpBuff, usCols * 2, i, 0, 0);
		lpBuff += (usCols * 2);
 	}
	FreeMem (selScr);
	return;
}
//-------------------------------------------------------------------
// GetScreenInfo - Query the system for screen info
//-------------------------------------------------------------------
void GetScreenInfo (void) {

	VIOMODEINFO	vmiData;

	vmiData.cb = sizeof (vmiData);			//Set buffer size
	VioGetMode (&vmiData, 0);              //Get info

	usRows = vmiData.row;
	usCols = vmiData.col;
	usEWinStrt = 1;
	usEWinRows = vmiData.row - usEWinStrt - 1;
	usMenuRow = 0;
	usStatRow = vmiData.row - 1;
	if (vmiData.color >= 4) {
		ucAtNorm = 0x17;
		ucAtMark = 0x71;
		ucAtMenu = 0x70;
		ucAtMenuHot = 0x7F;
		ucAtStat = 0x71;
	} else {
		ucAtNorm = 0x07;
		ucAtMark = 0x70;
		ucAtMenu = 0x70;
		ucAtMenuHot = 0x7F;
		ucAtStat = 0x70;
	}	
	return;

}

