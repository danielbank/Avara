/*
    Copyright �1995, Juri Munkki
    All rights reserved.

    File: CRosterWindow.h
    Created: Thursday, May 25, 1995, 22:53
    Modified: Friday, June 9, 1995, 8:00
*/

#pragma once
#include "CWindowCommander.h"

#define	kRosterStringListID	130
#define	kNetInfoTimeOut		120		// 2 seconds

#define	kChatMessageWidth	336
#define	kMaxChatLines		13

enum
{	kTopBox,
	kCardBox,
	kCardContentBox,
	kTabBox,
	
	kChatBufferBox,
	kChatBufferLineBox,
	kChatEntryBox,
	kChatDivisorBox,
	
	kOnePlayerBox,
	kFullMapBox,
	kSmallMapBox,
	kMugShotBox,

	kMapInfoBox,	//	Location coordinates and maybe more
	kNetInfoBox,	//	Information on networking stuff
	kOneColorBox,
	kOneNameBox,	//	Player name and status
	kOneScoreBox,	//	Latest player result
	kOneMessageBox,	//	
	
	kResultsBox,
	kCustomResultsBox,
	
	kScoreInformationBox,
	
	kBottomBox,
	kBottomLeftBox,
	kBottomRightBox,
	kWaterMarkBox,
	kRosterRowBox,
	kNumberBox,
	kUserBox,
	kUserBoxTopLine,
	kUserBoxBottomLine,
	kRealNameBox
};

class	CRosterWindow : public CWindowCommander
{
public:
			PicHandle			waterMarkPic;
			Rect				waterMarkFrame;

			PicHandle			mapPic;
	class	CTabCards			*tabs;
			Str255				chatLine;
			StringPtr			chatBuffers[kMaxChatLines];
			Str255				chatSpace[kMaxChatLines];
			
			Handle				leftNetInfo;
			Handle				rightNetInfo;
			short				tabNetInfo;
			short				slotInfo;
			long				infoStamp;
			
			void				TruncatePlayerName(StringPtr theName);

	virtual	void				IWindowCommander(CCommander *theCommander,
												CWindowCommander **theList);
	virtual	void				Dispose();

	virtual	void				GetMapPicture();

	virtual	CloseRequestResult	CloseRequest(Boolean isQuitting);
	
	virtual	void				PointToAnchor(Point *thePoint);
	virtual	void				ChatClick(Point where);
	virtual	void				ContentClick(EventRecord *theEvent, Point where);

	virtual	Boolean				CheckNetInfo(short slot);

	virtual	void				GotEvent();

	virtual	void				DrawAngleString(short angle, short positive, short negative);

	virtual	void				DrawChatPage();
	virtual	void				DrawPlayersPage();
	virtual	void				DrawOnePlayerPage();
	virtual	void				DrawPlayerPicts();
	virtual	void				DrawResultsPage();
	virtual	void				DrawCustomPage();

	virtual	void				DrawContents();
		
	//	CCommander methods:
	virtual	void				AdjustMenus(CEventHandler *master);
	virtual	void				DoCommand(long theCommand);

	virtual	void				KeyEvent(EventRecord *theEvent);

	virtual	Boolean				DoEvent(CEventHandler *master, EventRecord *theEvent);

	//	Utility:
	virtual	Boolean				GetRosterRect(Rect *theRect, short part, short index);
	virtual	void				DragUserBox(EventRecord *theEvent, Point where, short index);
	virtual	void				ClickUserColor(EventRecord *theEvent, Point where, short index);
	virtual	void				InvalidateColorBox(short ind);
	virtual	void				InvalidateArea(short areaCode, short i);
	
	virtual	void				SetChatLine(StringPtr theLine);
	virtual	void				NewChatLine(StringPtr theLine);

	virtual	Boolean				AvoidReopen(FSSpec *theFile, Boolean doSelect);
};