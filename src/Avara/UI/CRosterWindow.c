/*
    Copyright �1995, Juri Munkki
    All rights reserved.

    File: CRosterWindow.c
    Created: Thursday, May 25, 1995, 22:58
    Modified: Monday, June 12, 1995, 6:46
*/

#include "CRosterWindow.h"
#include "CTabCards.h"
#include "CAvaraApp.h"
#include "CAvaraGame.h"
#include "CNetManager.h"
#include "CPlayerManager.h"
#include "CommandList.h"
#include "AvaraDefines.h"
#include "Palettes.h"
#include "CCommManager.h"
#include "CInfoPanel.h"
#include "JAMUtil.h"
#include "Folders.h"
#include "CScoreKeeper.h"
#include "CommDefs.h"

#define	tabBoxHeight			16
#define	tabAreaHeight			(tabBoxHeight)
#define	topAreaHeight			(193+tabAreaHeight)
#define	rosterInset				6
#define	rosterRowHeight			30
#define	rosterBottomLeftMargin	85
#define	playerNumBoxWidth		32
#define	kMaxMessageLen			50

enum	{	kPlayersTab = 1,
			kChatTab,
			kFirstPlayerTab,
			kLastPlayerTab = kFirstPlayerTab + kMaxAvaraPlayers - 1,
			kCustomTab,
			kResultsTab
		};

CloseRequestResult	CRosterWindow::CloseRequest(Boolean isQuitting)
{
	if(isQuitting)
	{	SaveIntoPreferences(kRosterWindowRectTag, kRosterWindowVisTag);
	}

	HideWindow(itsWindow);
	
	return kDidHideDontDispose;
}

void	CRosterWindow::IWindowCommander(
	CCommander			*theCommander,
	CWindowCommander	**theList)
{
	short			i;
	PaletteHandle	pal;
	Rect			tabRect;
	
	inherited::IWindowCommander(theCommander, theList);

	wantsOnWindowsMenu = false;
	hasGrowBox = false;
	itsWindow = GetNewCWindow(130, 0, FrontWindow());
	MoveWindow(itsWindow,
				(*GetMainDevice())->gdRect.right - itsWindow->portRect.right - rosterInset,
				45, false);
	pal = GetNewPalette(128);
	SetPalette(itsWindow, pal, true);
	
	RestoreFromPreferences(kRosterWindowRectTag, kRosterWindowVisTag, true, true);

	AddToList();
	
	GetRosterRect(&tabRect, kTabBox, 0);
	tabs = new CTabCards;
	tabs->ITabCards(itsWindow, 134, &tabRect, geneva, 10, kStatusTextAndBorderColor, kAvaraBackColor);
	
	waterMarkPic = GetPicture(129);
	waterMarkFrame = (*waterMarkPic)->picFrame;
	GetMapPicture();
	
	chatLine[0] = 0;
	for(i=0;i<kMaxChatLines;i++)
	{	chatBuffers[i] = chatSpace[i];
		chatSpace[i][0] = 0;
	}
	
	leftNetInfo = NewHandle(0);
	rightNetInfo = NewHandle(0);
	tabNetInfo = 0;
	slotInfo = -99;
}

void	CRosterWindow::Dispose()
{
	tabs->Dispose();
	tabs = NULL;
	
	if(mapPic)
	{	DisposeHandle((Handle)mapPic);
		mapPic = NULL;
	}
	
	if(leftNetInfo)		DisposeHandle(leftNetInfo);
	leftNetInfo = NULL;

	if(rightNetInfo)	DisposeHandle(rightNetInfo);
	rightNetInfo = NULL;

	inherited::Dispose();
}

Boolean	CRosterWindow::GetRosterRect(
	Rect	*theRect,
	short	part,
	short	index)
{
	Boolean		result = true;

	switch(part)
	{	case kTopBox:
			*theRect = itsWindow->portRect;
			theRect->bottom = topAreaHeight;
			InsetRect(theRect, rosterInset, rosterInset);
			break;
		case kCardBox:
			GetRosterRect(theRect, kTopBox, index);
			theRect->bottom = theRect->top + kMaxAvaraPlayers * rosterRowHeight + 1;
			break;
		case kCardContentBox:
			GetRosterRect(theRect, kCardBox, index);
			InsetRect(theRect, 1, 1);
			break;
		case kTabBox:
			GetRosterRect(theRect, kCardBox, index);
			theRect->top = theRect->bottom;
			theRect->bottom += tabBoxHeight;
			break;
		case kChatBufferBox:
			if(result = (tabs->activeTab == kChatTab))
			{	GetRosterRect(theRect, kCardContentBox, index);
				theRect->bottom -= 15;
			}
			break;
		case kChatBufferLineBox:
			if(result = GetRosterRect(theRect, kChatBufferBox, 0))
			{	theRect->top += 3 + 12 * index;
				theRect->bottom = theRect->top + 12;
			}
			break;
		case kChatEntryBox:
			if(result = (tabs->activeTab == kChatTab))
			{	GetRosterRect(theRect, kCardContentBox, index);
				theRect->top = theRect->bottom - 14;
				theRect->bottom -= 2;
			}
			break;

		case kResultsBox:
			if(result = (tabs->activeTab == kResultsTab))
			{	GetRosterRect(theRect, kCardContentBox, index);
			}
			break;

		case kCustomResultsBox:
			if(result = (tabs->activeTab == kCustomTab))
			{	GetRosterRect(theRect, kCardContentBox, index);
			}
			break;
		
		case kChatDivisorBox:
			if(result = (tabs->activeTab == kChatTab))
			{	GetRosterRect(theRect, kCardContentBox, index);
				theRect->top = theRect->bottom - 19;
				theRect->bottom = theRect->top + 2;
			}
			break;

		case kOnePlayerBox:
			if(tabs->activeTab == index + kFirstPlayerTab)
					result = GetRosterRect(theRect, kCardContentBox, index);
			else	result = false;	

			break;
		case kFullMapBox:
			if( result = GetRosterRect(theRect, kOnePlayerBox, index))
			{	theRect->left += 4;
				theRect->top += 4;
				theRect->bottom = theRect->top + 66;
				theRect->right = theRect->left + 146;
			}
			break;
		case kSmallMapBox:
			if( result = GetRosterRect(theRect, kFullMapBox, index))
			{	theRect->right = theRect->left + 76;
			}
			break;
		case kMugShotBox:
			if( result = GetRosterRect(theRect, kFullMapBox, index))
			{	theRect->left = theRect->right - 66;
			}
			break;
			
		case kMapInfoBox:
			if( result = GetRosterRect(theRect, kFullMapBox, index))
			{	theRect->top = theRect->bottom + 2;
				theRect->bottom = theRect->top + 24;
			}
			break;

		case kNetInfoBox:
			if( result = GetRosterRect(theRect, kOnePlayerBox, index))
			{	Rect	tempRect;
			
				GetRosterRect(&tempRect, kFullMapBox, index);
				theRect->left = tempRect.right + 4;
				theRect->right -= 4;
				GetRosterRect(&tempRect, kMapInfoBox, index);
				theRect->top += 4;
				theRect->bottom = tempRect.bottom;
			}
			break;

		case kOneColorBox:
			if( result = GetRosterRect(theRect, kOnePlayerBox, index))
			{	Rect	tempRect;
			
				GetRosterRect(&tempRect, kMapInfoBox, index);
				theRect->left += 4;
				theRect->right = theRect->left + 48;
				theRect->top = tempRect.bottom + 4;
				theRect->bottom = theRect->top + 48;
			}
			break;

		case kRealNameBox:
			if( result = GetRosterRect(theRect, kOnePlayerBox, index))
			{	Rect	tempRect;
			
				GetRosterRect(&tempRect, kOneColorBox, index);
				theRect->left = tempRect.right + 6;
				theRect->right -= 4;
				theRect->top = tempRect.top;
				theRect->bottom = theRect->top + 16;
			}
			break;

		case kOneNameBox:
			if( result = GetRosterRect(theRect, kRealNameBox, index))
			{	theRect->top = theRect->bottom;
				theRect->bottom += 16;
			}
			break;
						
		
		case kOneScoreBox:
			if( result = GetRosterRect(theRect, kOneNameBox, index))
			{	theRect->top = theRect->bottom;
				theRect->bottom += 16;
			}
			break;

		case kOneMessageBox:
			if( result = GetRosterRect(theRect, kOnePlayerBox, index))
			{	Rect	tempRect;
			
				theRect->left += 4;
				theRect->right -= 4;
				
				GetRosterRect(&tempRect, kOneColorBox, index);
				theRect->top = tempRect.bottom + 4;
				theRect->bottom -= 4;
			}
			break;
		
		case kScoreInformationBox:
			if(result = (tabs->activeTab == kResultsTab))
			{	GetRosterRect(theRect, kCardContentBox, index);
			}
			else if(tabs->activeTab >= kFirstPlayerTab && tabs->activeTab <= kLastPlayerTab)
			{	result = GetRosterRect(theRect, kOneScoreBox, tabs->activeTab - kFirstPlayerTab);
			}
			break;

		case kBottomBox:
			*theRect = itsWindow->portRect;
			theRect->top = topAreaHeight;
			InsetRect(theRect, rosterInset, rosterInset);
			break;
		case kBottomLeftBox:
			GetRosterRect(theRect, kBottomBox, 0);
			theRect->right = theRect->left + rosterBottomLeftMargin;
			break;
		case kBottomRightBox:
			GetRosterRect(theRect, kBottomBox, 0);
			theRect->left += rosterBottomLeftMargin + rosterInset;
			break;
		case kWaterMarkBox:
			{	Rect	tempRect;
			
				*theRect = waterMarkFrame;	//	(*waterMarkPic)->picFrame;
				OffsetRect(theRect, (- theRect->left - theRect->right)>>1, (-theRect->top - theRect->bottom)>>1);
				GetRosterRect(&tempRect, kBottomBox, 0);
				OffsetRect(theRect, (tempRect.left+tempRect.right)>>1, (tempRect.top+tempRect.bottom)>>1);
			}
			break;
		case kRosterRowBox:
			if(tabs->activeTab == kPlayersTab)
			{	GetRosterRect(theRect, kCardBox, 0);
				theRect->top += 1 + index * rosterRowHeight;
				theRect->bottom = theRect->top + rosterRowHeight - 1;
			}
			else
			{	result = false;
			}
			break;
		case kNumberBox:
			if(tabs->activeTab == kPlayersTab)
			{	GetRosterRect(theRect, kRosterRowBox, index);
				theRect->right = theRect->left + playerNumBoxWidth - 1;
				theRect->left += 1;
			}
			else
			{	result = GetRosterRect(theRect, kOneColorBox, index);
			}
			break;
		case kUserBox:
			if(result = GetRosterRect(theRect, kRosterRowBox, index))
			{	theRect->left = theRect->left + playerNumBoxWidth;
				theRect->top += 1;
				theRect->right -= 1;
			}
			break;
		case kUserBoxTopLine:
			if(result = GetRosterRect(theRect, kUserBox, index))
			{	theRect->bottom = (theRect->top + theRect->bottom) >> 1;
			}
			else
				result = GetRosterRect(theRect, kOneNameBox, index);
			break;
		case kUserBoxBottomLine:
			if(result = GetRosterRect(theRect, kUserBox, index))
			{	theRect->top = (theRect->top + theRect->bottom) >> 1;
			}
			else
				result = GetRosterRect(theRect, kOneMessageBox, index);
			break;
	}
	
	if(!result)
	{	theRect->top = 10000;
		theRect->left = 10000;
		theRect->bottom = 10000;
		theRect->right = 10000;
	}

	return result;
}

void	CRosterWindow::InvalidateArea(
	short	areaCode,
	short	i)
{
	GrafPtr		saved;
	Rect		rosterRect;
	CInfoPanel	*theInfoPanel = ((CAvaraApp *)gApplication)->theInfoPanel;
	
	GetPort(&saved);
	if(theInfoPanel)
		theInfoPanel->InvalidateRosterData(i, areaCode);

	if(GetRosterRect(&rosterRect, areaCode, i))
	{	SetPort(itsWindow);
		InvalRect(&rosterRect);
	}
	
	SetPort(saved);
}

#define	kMapFileType	'cdev'
#define	kMapFileCreator	'wmap'

void	CRosterWindow::GetMapPicture()
{
	short	foundVRefNum;
	long	foundDirID;
	OSErr	err;
	
	mapPic = GetPicture(400);
	if(mapPic)	DetachResource((Handle)mapPic);

	if(mapPic == NULL)
	{	err = FindFolder(kOnSystemDisk, kControlPanelFolderType, false, &foundVRefNum, &foundDirID);
		if(err == noErr)
		{	HFileParam		theParam;
			Str255			fileName;
			short			i;
		
			i = 1;
			do
			{	theParam.ioCompletion = NULL;
				theParam.ioVRefNum = foundVRefNum;
				theParam.ioFVersNum = 0;
				theParam.ioDirID = foundDirID;
				theParam.ioNamePtr = fileName;
				fileName[0] = 0;
				theParam.ioFDirIndex = i++;		
	
				err = PBHGetFInfo((HParmBlkPtr)&theParam, false);
				if(theParam.ioFlFndrInfo.fdType == kMapFileType &&
					theParam.ioFlFndrInfo.fdCreator == kMapFileCreator)
				{	short	fRefNum;
				
					fRefNum = HOpenResFile(foundVRefNum, foundDirID, fileName, fsRdPerm);
					if(fRefNum != -1)
					{	mapPic = GetPicture(-4064);
						if(mapPic)	DetachResource((Handle)mapPic);
						CloseResFile(fRefNum);
					}
				}
			} while(err == noErr && !mapPic);
		}
	}
}

void	CRosterWindow::DragUserBox(
	EventRecord	*theEvent,
	Point		where,
	short		index)
{
	Rect		legalArea;
	Rect		toDrag;
	Rect		rosterRect;
	RgnHandle	toDragRgn;
	short		i;
	short		release;
	CNetManager	*theNet = ((CAvaraApp *)gApplication)->gameNet;
	
	GetRosterRect(&toDrag, kUserBox, index);
	GetRosterRect(&legalArea, kCardBox, 0);

	rosterRect = toDrag;
	InsetRect(&rosterRect, 2,2);

	toDragRgn = NewRgn();
	OpenRgn();
	FrameRect(&toDrag);
	FrameRect(&rosterRect);	
	CloseRgn(toDragRgn);

	release = DragGrayRgn(toDragRgn, where, &legalArea, &legalArea, vAxisOnly, NULL) >> 16;

	DisposeRgn(toDragRgn);

	if(	release == 0x8000)
	{	if(theNet->PermissionQuery(kAllowKickBit, index))
		{	short	theClient;
		
			theClient = theNet->positionToSlot[index];
			if(theEvent->modifiers & cmdKey)
			{	//	Hard kill...can not be prevented by a "hacked" Avara
				theNet->HandleDisconnect(theNet->positionToSlot[index], 0);
			}
			else
			{	theNet->itsCommManager->SendPacket(1<<theClient, kpKickClient, 0,0,0, 0, NULL);
			}
		}
	}
	else
	{	where.v += release;
		for(i=0;i<kMaxAvaraPlayers;i++)
		{	GetRosterRect(&rosterRect, kUserBox, i);
			if(PtInRect(where, &rosterRect))
			{	CNetManager	*theNet = ((CAvaraApp *)gApplication)->gameNet;
	
				if(i != index)
				{	theNet->SwapPositions(i, index);
				}
			}
		}
	}
}

void	CRosterWindow::ClickUserColor(
	EventRecord	*theEvent,
	Point		where,
	short		index)
{
	CNetManager	*theNet = ((CAvaraApp *)gApplication)->gameNet;
	char		origColor;
	char		shown;
	char		*theColor;
	Rect		theRect;
	Rect		firstPop;
	Rect		otherRect;
	short		i;
	
#define	popSize		22
#define	popOffset	(popSize - 1)
	
	GetRosterRect(&theRect, kNumberBox, index);
	theColor = &theNet->teamColors[index];
	origColor = *theColor;

	DoUpdate();

	firstPop = theRect;
	firstPop.top = firstPop.bottom+2;
	firstPop.bottom = firstPop.top + popSize;
	firstPop.left = firstPop.left + popSize;
	firstPop.right = firstPop.left + 3 * popSize;

	otherRect = firstPop;
	for(i=0;i<kMaxTeamColors;i++)
	{	if(i != origColor)
		{	PmForeColor(kFirstPlayerColor + i);
			PaintRect(&otherRect);
			PmForeColor(1);
			FrameRect(&otherRect);
			MoveTo(otherRect.right, otherRect.top+1);
			LineTo(otherRect.right, otherRect.bottom);
			LineTo(otherRect.left+2, otherRect.bottom);
			OffsetRect(&otherRect, 0, popOffset);
		}
	}

	do
	{	shown = *theColor;

		GetNextEvent(mUpMask+mDownMask, theEvent);
		where = theEvent->where;
		GlobalToLocal(&where);
		
		otherRect = firstPop;
		InsetRect(&otherRect, 1,1);
		*theColor = origColor;
		for(i=0;i<kMaxTeamColors;i++)
		{	if(i != origColor)
			{	if(PtInRect(where, &otherRect))
				{	*theColor = i;
					if(i != shown)
					{	PmForeColor(1);
						FrameRect(&otherRect);
					}
				}
				else
				{	if(i == shown)
					{	PmForeColor(kFirstPlayerColor + i);
						PaintRect(&otherRect);
					}
				}

				OffsetRect(&otherRect, 0, popOffset);
			}
		}
		
		if(shown != *theColor)
		{	InvalRect(&theRect);
			DoUpdate();
		}

	} while(theEvent->what != mouseUp);

	otherRect = firstPop;
	otherRect.right++;
	otherRect.bottom++;
	for(i=0;i<kMaxTeamColors;i++)
	{	if(i != origColor)
		{	InvalRect(&otherRect);
			OffsetRect(&otherRect, 0, popOffset);
		}
	}
	
	if(*theColor != origColor)
	{
		CInfoPanel	*theInfoPanel = ((CAvaraApp *)gApplication)->theInfoPanel;

		theNet->SendColorChange();
		
		if(theInfoPanel)
			theInfoPanel->InvalidateRosterData(index, kNumberBox);
	}
}

void	CRosterWindow::InvalidateColorBox(
	short	ind)
{
	InvalidateArea(kNumberBox, 	ind);
}

void	CRosterWindow::PointToAnchor(
	Point	*thePoint)
{
	Boolean		leftSide;
	StringPtr	theRow;
	short		offsetCode;
	short		i;
	
	for(i=0;i<kMaxChatLines;i++)
	{	Rect	theRect;
	
		GetRosterRect(&theRect, kChatBufferLineBox, i);
		theRect.left += 4;
		
		if(thePoint->v < theRect.bottom)
		{	if(thePoint->v < theRect.top)
			{	thePoint->h = 0;
				thePoint->v = i;
			}
			else
			{	theRow = chatBuffers[i];
				thePoint->v = i;
				thePoint->h = Pixel2Char((Ptr)theRow + 1, theRow[0], 0, thePoint->h - theRect.left, &leftSide);
			}
			return;
		}
	}
	
	thePoint->v = kMaxChatLines-1;
	thePoint->h = chatBuffers[kMaxChatLines-1][0];
}

#define		HILITEMODE	LMSetHiliteMode(127 & LMGetHiliteMode())

void	CRosterWindow::ChatClick(
	Point	where)
{
	RgnHandle		hiliteRgn;
	RgnHandle		freshRgn;
	RgnHandle		tempRgn;
	EventRecord		theEvent;
	Point			textAnchor;
	Point			textMover;
	TextSettings	sText;
	Point			aPoint;
	Point			bPoint;
	short			i;
			
	DoUpdate();
	GetSetTextSettings(&sText, geneva, 0, 9, srcOr);
	hiliteRgn = NewRgn();
	freshRgn = NewRgn();
	tempRgn = NewRgn();
	
	textAnchor = where;
	PointToAnchor(&textAnchor);
	
	do
	{	Rect	theRect;
		StringPtr	theRow;
	
		GetNextEvent(mUpMask + mDownMask, &theEvent);
		
		textMover = theEvent.where;
		GlobalToLocal(&textMover);
		PointToAnchor(&textMover);
		
		if(*(long *)&textAnchor > *(long *)&textMover)
		{	aPoint = textMover;
			bPoint = textAnchor;
		}
		else
		{	bPoint = textMover;
			aPoint = textAnchor;
		}
		
		GetRosterRect(&theRect, kChatBufferLineBox, aPoint.v);

		theRow = chatBuffers[aPoint.v];
		theRect.left += 4 + TextWidth(theRow + 1, 0, aPoint.h);
		theRect.right -= 4;
		OpenRgn();
		FrameRect(&theRect);
		
		GetRosterRect(&theRect, kChatBufferLineBox, bPoint.v);
		theRow = chatBuffers[bPoint.v];
		theRect.left += 4+TextWidth(theRow + 1, 0, bPoint.h);
		theRect.right -= 4;
		FrameRect(&theRect);

		for(i=aPoint.v+1; i<=bPoint.v; i++)
		{	GetRosterRect(&theRect, kChatBufferLineBox, i);
			theRect.left+=4;
			theRect.right -= 4;
			FrameRect(&theRect);
		}

		CloseRgn(freshRgn);

		XorRgn(freshRgn, hiliteRgn, tempRgn);
		CopyRgn(freshRgn, hiliteRgn);
		HILITEMODE;
		InvertRgn(tempRgn);
		
	} while(theEvent.what != mouseUp);
	
	HILITEMODE;
	InvertRgn(hiliteRgn);

	DisposeRgn(hiliteRgn);
	DisposeRgn(freshRgn);
	DisposeRgn(tempRgn);
	RestoreTextSettings(&sText);
	
	if(*(long *)&textAnchor != *(long *)&textMover)
	{	short	totalLen;
		Ptr		theText;
		Ptr		p;
	
		totalLen = 0;
		for(i=aPoint.v;i<bPoint.v;i++)
		{	totalLen += 1 + chatBuffers[i][0];
		}
		
		totalLen -= aPoint.h;
		totalLen += bPoint.h;
		theText = NewPtr(totalLen);
		p = theText;
		
		while(*(long *)&aPoint != *(long *)&bPoint)
		{	if(aPoint.h < chatBuffers[aPoint.v][0])
			{	*p++ = chatBuffers[aPoint.v][++aPoint.h];
			}
			else
			{	*p++ = 13;
				aPoint.h = 0;
				aPoint.v++;
			}
		}
		
		ZeroScrap();
		PutScrap(totalLen, 'TEXT', theText);
		DisposePtr(theText);
	}
}

void	CRosterWindow::ContentClick(
	EventRecord	*theEvent,
	Point		where)
{
	Rect		rosterRect;
	short		i;
	CNetManager	*theNet = ((CAvaraApp *)gApplication)->gameNet;
	
	GetRosterRect(&rosterRect, kCardBox, 0);
	
	if(PtInRect(where, &rosterRect))
	{			switch(tabs->activeTab)
		{	case kPlayersTab:
				for(i=0;i<kMaxAvaraPlayers;i++)
				{	if(	theNet->PermissionQuery(kAllowPositionBit, 0)
						&&	GetRosterRect(&rosterRect, kUserBox, i)
						&&	PtInRect(where, &rosterRect))
					{	DragUserBox(theEvent, where, i);
						break;
					}
					
					if(	theNet->PermissionQuery(kAllowOwnColorBit, i)
						&&	GetRosterRect(&rosterRect, kNumberBox, i)
						&&	PtInRect(where, &rosterRect))
					{	ClickUserColor(theEvent, where, i);
						break;
					}
				}
				break;
			case kResultsTab:
				GetRosterRect(&rosterRect, kResultsBox, 0);
				if(PtInRect(where, &rosterRect))
				{	CScoreKeeper	*keeper = ((CAvaraApp *)gApplication)->itsGame->scoreKeeper;
				
					keeper->RegularClick(theEvent, &rosterRect);
				}
			case kCustomTab:
				GetRosterRect(&rosterRect, kCustomResultsBox, 0);
				if(PtInRect(where, &rosterRect))
				{	CScoreKeeper	*keeper = ((CAvaraApp *)gApplication)->itsGame->scoreKeeper;
				
					keeper->Click(theEvent, &rosterRect);
				}
				break;
			case kChatTab:
				GetRosterRect(&rosterRect, kChatTab, 0);
				if(PtInRect(where, &rosterRect))
				{	ChatClick(where);
				}
				break;
			default:
				if(tabs->activeTab >= kFirstPlayerTab && tabs->activeTab <= kLastPlayerTab)
				{	i = tabs->activeTab - kFirstPlayerTab;
					if(	theNet->PermissionQuery(kAllowOwnColorBit, i)
						&&	GetRosterRect(&rosterRect, kNumberBox, i)
						&&	PtInRect(where, &rosterRect))
					{	ClickUserColor(theEvent, where, i);
						break;
					}

					if(	GetRosterRect(&rosterRect, kNetInfoBox, i) &&
						PtInRect(where, &rosterRect))
					{	theNet->itsCommManager->SendPacket(1 << theNet->positionToSlot[i], kpPing, 0,0, 32, 0, NULL);
						break;
					}
				}
		}

	}
	else
	{	if(		GetRosterRect(&rosterRect, kTabBox, 0)
			&&	PtInRect(where, &rosterRect))
		{	short		oldTab = tabs->activeTab;
		
			if(tabs->Click(theEvent, where))
			{	InvalidateArea(kCardContentBox, 0);
				if(tabs->activeTab == kCustomTab ||
					oldTab == kCustomTab)
				{	CScoreKeeper	*keeper = ((CAvaraApp *)gApplication)->itsGame->scoreKeeper;
				
					keeper->HideShow(oldTab == kCustomTab);
				}
			}
		}
	}
}

static
pascal
void	NullRectProc(
	GrafVerb verb,
	Rect *r)
{

}

Boolean	CRosterWindow::CheckNetInfo(
	short	slot)
{
	long		thisTime;

	thisTime = TickCount();

	if(slot == slotInfo && infoStamp - thisTime > 0)
	{	return false;
	}
	else
	{	CNetManager	*theNet = ((CAvaraApp *)gApplication)->gameNet;
		short		newTab;
		Handle		newLeft, newRight;

		infoStamp = thisTime + kNetInfoTimeOut;
		newLeft = NewHandle(0);
		newRight = NewHandle(0);

		newTab = theNet->itsCommManager->GetStatusInfo(slot, newLeft, newRight);
		if(		newTab == tabNetInfo
			&&	GetHandleSize(newLeft) == GetHandleSize(leftNetInfo)
			&&	GetHandleSize(newRight) == GetHandleSize(rightNetInfo))
		{	Ptr		np, op;
			long	len;
		
			len = GetHandleSize(newLeft);
			np = *newLeft;
			op = *leftNetInfo;
			while(len--)
			{	if(*np++ != *op++)
				{	len = 0;
					break;
				}
			}
			
			if(len)
			{	len = GetHandleSize(newRight);
				np = *newRight;
				op = *rightNetInfo;
				while(len--)
				{	if(*np++ != *op++)
					{	len = 0;
						break;
					}
				}
				
				if(len)
				{	DisposeHandle(newLeft);
					DisposeHandle(newRight);
					return false;
				}
			}
		}
		
		//	New and old are different.
		
		DisposeHandle(leftNetInfo);
		DisposeHandle(rightNetInfo);
		tabNetInfo = newTab;
		leftNetInfo = newLeft;
		rightNetInfo = newRight;
		slotInfo = slot;
		
		InvalidateArea(kNetInfoBox, theNet->slotToPosition[slot]);
		
		return true;	//	There was a change!
	}
}

void	CRosterWindow::GotEvent()
{
	if(	tabs->activeTab >= kFirstPlayerTab &&
		tabs->activeTab <= kLastPlayerTab)
	{	CNetManager	*theNet = ((CAvaraApp *)gApplication)->gameNet;
	
		CheckNetInfo(theNet->positionToSlot[tabs->activeTab - kFirstPlayerTab]);
	}
}

void	CRosterWindow::DrawAngleString(
	short	angle,
	short	positive,
	short	negative)
{
	Str32	dirString;
	Str32	numString;
	short	degrees;
	short	minutes;
	
	if(angle < 0)
	{	GetIndString(dirString, kRosterStringListID, negative);
		angle = -angle;
	}
	else
	{	GetIndString(dirString, kRosterStringListID, positive);
	}
	
	minutes = (((angle * 21600L) + 0x8000) >> 16);
	degrees = minutes / 60;
	minutes -= degrees * 60;

	NumToString(degrees, numString);
	if(numString[0] == 2)
	{	numString[3] = numString[2];
		numString[2] = numString[1];
		numString[1] = 202;
		numString[0] = 3;
	}
	else
	if(numString[0] == 1)
	{	numString[3] = numString[1];
		numString[2] = 202;
		numString[1] = 202;
		numString[0] = 3;
	}
	numString[++numString[0]] = '�';
	numString[++numString[0]] = ' ';
	DrawString(numString);

	NumToString(minutes, numString);
	if(numString[0] == 1)
	{	numString[2] = numString[1];
		numString[1] = 202;	//	Option space
		numString[0] = 2;
	}
	numString[++numString[0]] = '�';
	numString[++numString[0]] = ' ';
	DrawString(numString);
	
	DrawString(dirString);
}

void	CRosterWindow::DrawChatPage()
{
	Rect			theRect;
	short			i;
	TextSettings	sText;
	
	GetSetTextSettings(&sText, geneva, 0, 9, srcOr);
	
	PmForeColor(kStatusTextAndBorderColor);

	if(GetRosterRect(&theRect, kChatEntryBox, 0))
	{
		MoveTo(theRect.left + 4,
					theRect.bottom - 3);
		DrawString(chatLine);
	}

	for(i=0;i<kMaxChatLines;i++)
	{	if(GetRosterRect(&theRect, kChatBufferLineBox, i))
		{	MoveTo(theRect.left + 4,
					theRect.bottom - 3);
			DrawString(chatBuffers[i]);
		}
	}
	
	if(GetRosterRect(&theRect, kChatDivisorBox, 0))
	{	PmForeColor(kShadowGrayColor);
		theRect.bottom--;
		PaintRect(&theRect);

		PmForeColor(0);
		theRect.top++;
		theRect.bottom++;
		PaintRect(&theRect);
		PmForeColor(kStatusTextAndBorderColor);
	}
	
	RestoreTextSettings(&sText);
}

#if 0
void	DrawStringNoTrailingSpace(StringPtr theString);

void	DrawStringNoTrailingSpace(
	StringPtr	theString)
{
	short	len;
	
	len = theString[0];
	
	while(len && theString[len] == 32)
	{	len--;
	}
	
	DrawText(theString+1, 0, len);
}
#endif

void	CRosterWindow::DrawPlayersPage()
{
	Rect		tempRect, rosterRect;
	char		*colorInd;
	short		i,y;
	CNetManager	*theNet = ((CAvaraApp *)gApplication)->gameNet;

	PmForeColor(1);
	GetRosterRect(&rosterRect, kCardContentBox, 0);
	GetRosterRect(&tempRect, kNumberBox, 0);
	rosterRect.left = tempRect.right;
	rosterRect.right = tempRect.right + 1;
	PaintRect(&rosterRect);

	TextSize(9);
	TextFont(monaco);
	
	for(i=0;i<kMaxAvaraPlayers;i++)
	{	CPlayerManager	*thePlayer;
		Str255			tempStr;
		short			slot;

		GetRosterRect(&tempRect, kUserBox, i);
		y = tempRect.top + 10;

		slot = theNet->positionToSlot[i];
		thePlayer = theNet->playerTable[slot];
		
		if(slot == 0)	TextFace(underline);
		MoveTo(tempRect.left + rosterInset, y);
		DrawString(thePlayer->playerName);
		if(slot == 0)	TextFace(0);
		
		if(thePlayer->loadingStatus)
		{	thePlayer->GetLoadingStatusString(tempStr);
			MoveTo(tempRect.right - rosterInset - StringWidth(tempStr), y);
			DrawString(tempStr);
		}
		
		MoveTo(tempRect.left + rosterInset, y+14);
		if(thePlayer->message[0] <= kMaxMessageLen)
		{	DrawString(thePlayer->message);
		}
		else
		{	DrawText(thePlayer->message, thePlayer->message[0]+1-kMaxMessageLen, kMaxMessageLen);
		}
	}

	PmForeColor(0);
	for(i=0;i<kMaxAvaraPlayers;i++)
	{	if(GetRosterRect(&tempRect, kUserBox, i))
		{	MoveTo(tempRect.left, tempRect.top - 1);
			LineTo(tempRect.right-1, tempRect.top - 1);
		}
	}

	PmForeColor(1);
	for(i=0;i<kMaxAvaraPlayers-1;i++)
	{	GetRosterRect(&tempRect, kRosterRowBox, i);
		MoveTo(tempRect.left, tempRect.bottom);
		LineTo(tempRect.right-1, tempRect.bottom);
	}

	TextMode(srcOr);
	TextFont(geneva);
	TextSize(24);
	TextFace(0);
	colorInd = theNet->teamColors;
	for(i=0;i<kMaxAvaraPlayers;i++)
	{	PmForeColor(kFirstPlayerColor+*colorInd++);
		GetRosterRect(&tempRect, kNumberBox, i);
		PaintRect(&tempRect);
		PmForeColor(1);
		MoveTo((tempRect.left + tempRect.right - CharWidth('1'+i))/2, tempRect.bottom - 5);
		DrawChar('1'+i);
	}
}

void	CRosterWindow::DrawOnePlayerPage()
{
	CNetManager		*theNet = ((CAvaraApp *)gApplication)->gameNet;
	CPlayerManager	*thePlayer;
	short			position;
	Rect			theRect;
	RgnHandle		theClip;
	TextSettings	sText;
	TextSettings	nText;
	
	position = tabs->activeTab - kFirstPlayerTab;
	thePlayer = theNet->playerTable[theNet->positionToSlot[position]];

	if(thePlayer->loadingStatus != kLNotConnected)
	{
		Str255	tempStr;
		short	charFit;

		theClip = NewRgn();
		GetClip(theClip);

		GetRosterRect(&theRect, kOneNameBox, position);
		ClipRect(&theRect);

		theRect.top += GetSetTextSettings(&sText, geneva, 0, 12, srcOr);
		if(RectInRgn(&theRect, itsWindow->visRgn))
		{
			if(thePlayer->slot == 0)
				TextFace(underline);
			MoveTo(theRect.left, theRect.top);
			DrawString(thePlayer->playerName);
			
			TextFace(0);
	
			thePlayer->GetLoadingStatusString(tempStr);
			MoveTo(theRect.right - StringWidth(tempStr), theRect.top);
			DrawString(tempStr);
	
		}

		GetRosterRect(&theRect, kRealNameBox, position);
		ClipRect(&theRect);

		theRect.top += GetSetTextSettings(&sText, geneva, 0, 12, srcOr);
		if(RectInRgn(&theRect, itsWindow->visRgn))
		{
			MoveTo(theRect.left, theRect.top);
			DrawString(thePlayer->playerRegName);	
		}

		SetClip(theClip);
	
		GetRosterRect(&theRect, kMapInfoBox, position);
		if(RectInRgn(&theRect, itsWindow->visRgn))
		{	
			theRect.top += GetSetTextSettings(&nText, geneva, 0, 9, srcOr);
			MoveTo(theRect.left, theRect.top);
			DrawAngleString(thePlayer->globalLocation.v, kStringNorth, kStringSouth);
			
			theRect.top += nText.nextLine;
			MoveTo(theRect.left, theRect.top);
			DrawAngleString(thePlayer->globalLocation.h, kStringEast, kStringWest);
		}

		if(GetRosterRect(&theRect, kOneScoreBox, position))
		{	CScoreKeeper	*keeper = ((CAvaraApp *)gApplication)->itsGame->scoreKeeper;
		
			keeper->DrawOnePlayerResults(theNet->positionToSlot[position], &theRect);
		}

		if(		GetRosterRect(&theRect, kNetInfoBox, position)
			&&	RectInRgn(&theRect, itsWindow->visRgn)
			&&	!CheckNetInfo(theNet->positionToSlot[position]))
		{	long		len;
			CQDProcs	myQDProcs;
			CQDProcs	*savedProcs;


			GetSetTextSettings(&nText, geneva, 0, 9, srcOr);
			len = GetHandleSize(leftNetInfo);

			SetStdCProcs(&myQDProcs);
			myQDProcs.rectProc = (void *)NullRectProc;
			savedProcs = (void *)itsWindow->grafProcs;
			itsWindow->grafProcs = (void *)&myQDProcs;

			if(len)
			{	HLock(leftNetInfo);
				TextBox(*leftNetInfo, len, &theRect, teForceLeft);
				HUnlock(leftNetInfo);
			}
			
			len = GetHandleSize(rightNetInfo);
			if(len)
			{	theRect.left += tabNetInfo;
				HLock(rightNetInfo);
				TextBox(*rightNetInfo, len, &theRect, teForceLeft);
				HUnlock(rightNetInfo);
			}

			itsWindow->grafProcs = (void *)savedProcs;
		}

		GetRosterRect(&theRect, kOneMessageBox, position);
		if(RectInRgn(&theRect, itsWindow->visRgn))
		{	StringPtr	messString;
			short		len;
		
			ClipRect(&theRect);
		
			theRect.top += GetSetTextSettings(&nText, monaco, 0, 9, srcOr);
			charFit = (theRect.right - theRect.left) / nText.newInfo.widMax;

			MoveTo(theRect.left, theRect.top);
			
			messString = thePlayer->message;
			len = *messString++;

			if(len >= 2 * charFit)
			{	messString += len;
				DrawText(messString, -2 * charFit, charFit);
				MoveTo(theRect.left, theRect.top + nText.nextLine);
				DrawText(messString, -1 * charFit, charFit);
			}
			else if(len > charFit)
			{	DrawText(messString, 0, charFit);
				MoveTo(theRect.left, theRect.top + nText.nextLine);
				DrawText(messString, charFit, len - charFit);
			}
			else
			{	DrawText(messString, 0, len);
			}

			SetClip(theClip);
		}
	
		if(GetRosterRect(&theRect, kOneColorBox, position) &&
			RectInRgn(&theRect, itsWindow->visRgn))
		{	FrameRect(&theRect);
			InsetRect(&theRect, 1, 1);
			PmForeColor(kFirstPlayerColor+theNet->teamColors[position]);
			PaintRect(&theRect);
			PmForeColor(kStatusTextAndBorderColor);
		}

		DisposeRgn(theClip);
		
		RestoreTextSettings(&sText);
	}
	
}

void	CRosterWindow::DrawPlayerPicts()
{
	CNetManager		*theNet = ((CAvaraApp *)gApplication)->gameNet;
	CPlayerManager	*thePlayer;
	short			position;
	Rect			theRect;
	Rect			mapRect;
	Handle			theShot;
	RgnHandle		theClip;
	
	SetPort(itsWindow);
	PmForeColor(kStatusTextAndBorderColor);
	PmBackColor(kAvaraBackColor);

	theClip = NewRgn();
	GetClip(theClip);
	
	position = tabs->activeTab - kFirstPlayerTab;
	thePlayer = theNet->playerTable[theNet->positionToSlot[position]];
	
	theShot = thePlayer->GetMugShot();

	if(theShot)
	{	GetRosterRect(&mapRect, kSmallMapBox, position);
	
		GetRosterRect(&theRect, kMugShotBox, position);
		if(RectInRgn(&theRect, itsWindow->visRgn))
		{	Rect	tempRect;
		
			FrameRect(&theRect);
			InsetRect(&theRect, 1, 1);

			tempRect = (*(PicHandle)theShot)->picFrame;
			OffsetRect(&tempRect, (- tempRect.left - tempRect.right)>>1, (-tempRect.top - tempRect.bottom)>>1);
			OffsetRect(&tempRect, (theRect.left+theRect.right)>>1, (theRect.top+theRect.bottom)>>1);

			ClipRect(&theRect);
			DrawPicture((PicHandle) theShot, &tempRect);
			SetClip(theClip);
		}
	}
	else
	{	GetRosterRect(&mapRect, kFullMapBox, position);
	}

	if(RectInRgn(&mapRect, itsWindow->visRgn)
	    && mapPic && thePlayer->loadingStatus != kLNotConnected)
	{
		Point		degLocation;
		Rect		picToRect;
		RGBColor	theOp = { 0x6000, 0x6000, 0x6000 };
		short		i;
		short		picWidth;
		short		vertAdjust;

		FrameRect(&mapRect);
		InsetRect(&mapRect, 1, 1);
		ClipRect(&mapRect);
		
		degLocation.h = (((0x10000L + thePlayer->globalLocation.h) * 360) >> 16) - 360;
		degLocation.v = (((0x10000L + thePlayer->globalLocation.v) * 360) >> 16) - 360;
		
		picToRect = (*mapPic)->picFrame;
		picToRect.right -= picToRect.left;
		picToRect.bottom -= picToRect.top;

		picWidth = picToRect.right;
		
		picToRect.left = 0;
		picToRect.top = 0;
		
		OffsetRect(&picToRect, 	((mapRect.right + mapRect.left) >> 1)
								- degLocation.h - (picToRect.right >> 1)
								- picWidth,
								((mapRect.top + mapRect.bottom) >> 1)
								+ degLocation.v - (picToRect.bottom >> 1));
		
		if(picToRect.top > mapRect.top)
		{	vertAdjust = mapRect.top - picToRect.top;
		}
		else
		if(picToRect.bottom < mapRect.bottom)
		{	vertAdjust = mapRect.bottom - picToRect.bottom;
		}
		else
			vertAdjust = 0;
		
		picToRect.top += vertAdjust;
		picToRect.bottom += vertAdjust;
		
		for(i=0;i<3;i++)
		{	Rect	dummyRect;
		
			if(SectRect(&picToRect, &mapRect, &dummyRect))
				DrawPicture(mapPic, &picToRect);
				
			OffsetRect(&picToRect, picWidth, 0);
		} 
		
		PenMode(blend);
		OpColor(&theOp);
		PmForeColor(1);
		MoveTo((mapRect.right + mapRect.left) >> 1, mapRect.top);
		Line(0, 500);

		vertAdjust += ((mapRect.top + mapRect.bottom) >> 1);
		MoveTo(mapRect.left, vertAdjust);
		Line(500, 0);

		PmForeColor(0);
		MoveTo(((mapRect.right + mapRect.left) >> 1) + 1, mapRect.top);
		Line(0, 500);

		MoveTo(mapRect.left, vertAdjust + 1);
		Line(500, 0);
		
		PenMode(patCopy);
		
		PmForeColor(kStatusTextAndBorderColor);
		SetClip(theClip);
	}
	
	DisposeRgn(theClip);

}

void	CRosterWindow::DrawResultsPage()
{
	CScoreKeeper	*keeper = ((CAvaraApp *)gApplication)->itsGame->scoreKeeper;
	Rect			theRect;

	GetRosterRect(&theRect, kResultsBox, 0);
	keeper->DrawResultsSummary(&theRect);
}
void	CRosterWindow::DrawCustomPage()
{
	Rect			theRect;
	long			len;
	CQDProcs		myQDProcs;
	CQDProcs		*savedProcs;
	TextSettings	sText;
	Handle			text;
	CScoreKeeper	*keeper = ((CAvaraApp *)gApplication)->itsGame->scoreKeeper;

	GetSetTextSettings(&sText, monaco, 0, 9, srcOr);

	text = keeper->GetCustomText();
	if(text)
	{	len = GetHandleSize(text);

		GetRosterRect(&theRect, kCustomResultsBox, 0);
		if(len)
		{	SetStdCProcs(&myQDProcs);
			myQDProcs.rectProc = (void *)NullRectProc;
			savedProcs = (void *)itsWindow->grafProcs;
			itsWindow->grafProcs = (void *)&myQDProcs;
		
			if(len)
			{	char	state;
				Rect	inRect;
			
				inRect = theRect;
				InsetRect(&inRect, 2, 2);
				state = HGetState(text);
				HLock(text);
				TextBox(*text, len, &inRect, teForceLeft);
				HSetState(text, state);
			}
		
			itsWindow->grafProcs = (void *)savedProcs;
		}

		keeper->DrawCustomWindow(&theRect);
	}
	else
	{	InvalidateArea(kCustomResultsBox, 0);
	}
	
	RestoreTextSettings(&sText);
}

void	CRosterWindow::DrawContents()
{
	Rect		portRect;
	Rect		rosterRect;
	Rect		tempRect;
	short		i,y;
	
	portRect = itsWindow->portRect;
	PmBackColor(kAvaraBackColor);
	PmForeColor(kStatusTextAndBorderColor);
	EraseRect(&portRect);
	MoveTo(portRect.left, topAreaHeight);
	LineTo(portRect.right, topAreaHeight);

	GetRosterRect(&rosterRect, kCardBox, 0);
	FrameRect(&rosterRect);

	switch(tabs->activeTab)
	{	case kPlayersTab:
			DrawPlayersPage();
			break;
		case kChatTab:
			DrawChatPage();
			break;
		case kResultsTab:
			DrawResultsPage();
			break;
		case kCustomTab:
			DrawCustomPage();
			break;
		default:
			if(	tabs->activeTab >= kFirstPlayerTab &&
				tabs->activeTab <= kLastPlayerTab)
			{	DrawOnePlayerPage();
			}
	}

	tabs->Draw();

#define	INFOLINEHEIGHT	12

	if(gCurrentGame && gCurrentGame->loadedLevel[0])
	{	GetRosterRect(&tempRect, kBottomBox, 0);
		if(RectInRgn(&tempRect, itsWindow->visRgn))
		{	Str63		tempString;
			CQDProcs	myQDProcs, *savedProcs;
			long		theTime;
			short		width;
		
			TextFont(geneva);
			TextSize(9);
			TextMode(srcOr);
			TextFace(0);
			
			GetRosterRect(&tempRect, kBottomLeftBox, 0);
			tempRect.top -= 2;
			for(i=1;i<3;i++)
			{	tempRect.top += INFOLINEHEIGHT;
				GetIndString(tempString, 133, i);
				MoveTo(tempRect.right - StringWidth(tempString), tempRect.top);
				DrawString(tempString);
			}
			
			TextFace(0);
			GetRosterRect(&tempRect, kBottomRightBox, 0);
			tempRect.top -= 2;
			tempRect.top += INFOLINEHEIGHT;
			MoveTo(tempRect.left, tempRect.top);
			DrawString(gCurrentGame->loadedLevel);

			theTime = gCurrentGame->loadedTimeLimit;
			if(theTime >= 0)
			{	long	hours;
				long	minutes;
				long	seconds;
			
				hours = theTime / (60 * 60);
				theTime -= hours * (60 * 60);
				minutes = theTime / 60;
				theTime -= minutes * 60;
				seconds = theTime;
				
				if(hours)
				{	NumToString(hours, tempString);
					tempString[++(tempString[0])] = ':';
					tempString[++(tempString[0])] = '0' + (minutes / 10);
					tempString[++(tempString[0])] = '0' + (minutes % 10);
				}
				else
				{	NumToString(minutes, tempString);
				}
				tempString[++(tempString[0])] = ':';
				tempString[++(tempString[0])] = '0' + (seconds / 10);
				tempString[++(tempString[0])] = '0' + (seconds % 10);
			}
			else
			{	GetIndString(tempString, 133, 4);
			}

			width = StringWidth(tempString);
			MoveTo(tempRect.right - width, tempRect.top);
			DrawString(tempString);
			
			GetIndString(tempString, 133, 3);
			width += rosterInset + StringWidth(tempString);
			MoveTo(tempRect.right - width, tempRect.top);
			DrawString(tempString);

			tempRect.top += INFOLINEHEIGHT;
			MoveTo(tempRect.left, tempRect.top);
			DrawString(gCurrentGame->loadedDesigner);
			
			tempRect.top += INFOLINEHEIGHT-5;

			SetStdCProcs(&myQDProcs);
			myQDProcs.rectProc = (void *)NullRectProc;
			savedProcs = (void *)itsWindow->grafProcs;
			itsWindow->grafProcs = (void *)&myQDProcs;

			TextBox(gCurrentGame->loadedInfo+1, gCurrentGame->loadedInfo[0], &tempRect, teJustLeft);
			itsWindow->grafProcs = (void *)savedProcs;
		}
	}
	else
	{	GetRosterRect(&tempRect, kWaterMarkBox, 0);
		if(RectInRgn(&tempRect, itsWindow->visRgn))
		{	ClipRect(&tempRect);
			waterMarkPic = GetPicture(129);
			HLock((Handle)waterMarkPic);
			DrawPicture(waterMarkPic, &tempRect);
			HUnlock((Handle)waterMarkPic);
			ClipRect(&itsWindow->portRect);
		}
	}

	if(	tabs->activeTab >= kFirstPlayerTab &&
		tabs->activeTab <= kLastPlayerTab)
	{	DrawPlayerPicts();
	}

	ClipRect(&itsWindow->portRect);
}

void	CRosterWindow::AdjustMenus(
	CEventHandler *master)
{
	long	scrapOffset;

	if(GetScrap(NULL, 'TEXT', &scrapOffset) > 0 ||
		GetScrap(NULL, 'PICT', &scrapOffset))
	{	master->EnableCommand(kPasteCmd);
	}

	inherited::AdjustMenus(master);
}

void	CRosterWindow::DoCommand(
	long theCommand)
{
	switch(theCommand)
	{	case kPasteCmd:
			{	Handle	theText;
				long	count;
				long	scrapOffset;
			
				theText = NewHandle(0);
				count = GetScrap(theText, 'TEXT', &scrapOffset);
				if(count > 0)
				{	CNetManager		*theNet = ((CAvaraApp *)gApplication)->gameNet;
					
					HLock(theText);
					
					if(count > PACKETDATABUFFERSIZE)
					{	
						theNet->SendRosterMessage(PACKETDATABUFFERSIZE,
											count - PACKETDATABUFFERSIZE + *theText);
					}
					else
					{	theNet->SendRosterMessage(count, *theText);
					}
				}
				else
				{	count = GetScrap(NULL, 'PICT', &scrapOffset);
					if(count > 0)
					{	gApplication->BroadcastCommand(kPastePlayerPictCmd);
					}
				}
				DisposeHandle(theText);
			}
			break;
		default:
			inherited::DoCommand(theCommand);
			break;
	}
}

void	CRosterWindow::KeyEvent(
	EventRecord		*theEvent)
{
	char			theChar = theEvent->message;
	CNetManager		*theNet = ((CAvaraApp *)gApplication)->gameNet;
	
	if(theChar == 0x1C)
	{	short	newTab = tabs->activeTab - 1;
		
		if(newTab == 0 || (theEvent->modifiers & optionKey))
		{	newTab = 1;
		}
		
		if(newTab != tabs->activeTab)
		{	tabs->SelectTab(newTab);
			InvalidateArea(kCardContentBox, 0);
		}
	}

	else if(theChar == 0x1D)
	{	short	newTab = tabs->activeTab + 1;
		
		if(newTab > tabs->listSize || (theEvent->modifiers & optionKey))
		{	newTab = tabs->listSize;
		}
		
		if(newTab != tabs->activeTab)
		{	tabs->SelectTab(newTab);
			InvalidateArea(kCardContentBox, 0);
		}
	}
	else
		theNet->SendRosterMessage(1, &theChar);
}

void	CRosterWindow::SetChatLine(
	StringPtr	theLine)
{
	BlockMove(theLine, chatLine, theLine[0] + 1);
	chatLine[++chatLine[0]] = '�';
	InvalidateArea(kChatEntryBox, 0);
}

void	CRosterWindow::NewChatLine(
	StringPtr	theLine)
{
	short		i;
	StringPtr	nextBuffer = chatBuffers[0];
	
	for(i=0;i<kMaxChatLines-1;i++)
	{	chatBuffers[i] = chatBuffers[i+1];
	}
	
	chatBuffers[kMaxChatLines-1] = nextBuffer;
	BlockMove(theLine, nextBuffer, theLine[0] + 1);
	
	InvalidateArea(kChatBufferBox, 0);
}

Boolean	CRosterWindow::AvoidReopen(
	FSSpec	*theFile,
	Boolean	doSelect)
{
	FInfo		theInfo;
	OSErr		theErr;
	Boolean		result = false;
	
	theErr = FSpGetFInfo(theFile, &theInfo);

	if(theErr == noErr && theInfo.fdType == kScoreInterfaceFileType)
	{	result = (noErr == ((CAvaraApp *)gApplication)->itsGame->scoreKeeper->OpenPlugIn(theFile));
	
		if(result && doSelect)
		{	Select();
		}
	}

	return result;
}

void	CRosterWindow::TruncatePlayerName(
	StringPtr		theName)
{
	Rect			rosterRect;
	short			savedTab;
	GrafPtr			saved;
	TextSettings	sText;

	GetPort(&saved);
	SetPort(itsWindow);
	savedTab = tabs->activeTab;
	tabs->activeTab = kFirstPlayerTab;
	GetRosterRect(&rosterRect, kRealNameBox, 0);
	GetSetTextSettings(&sText, geneva, 0, 12, srcOr);
	TruncString(rosterRect.right - rosterRect.left, theName, smTruncMiddle);
	RestoreTextSettings(&sText);
	tabs->activeTab = savedTab;
	SetPort(saved);
}

Boolean	CRosterWindow::DoEvent(CEventHandler *master, EventRecord *theEvent)
{
	Point	thePoint;
	GrafPtr	saved;

	inherited::DoEvent(master, theEvent);
	
	if(theEvent->what == nullEvent)
	{	Rect	theRect;
	
		GetPort(&saved);
		SetPort(itsWindow);
		thePoint = theEvent->where;
		GlobalToLocal(&thePoint);
		
		GetRosterRect(&theRect, kChatBufferBox, 0);
		if(PtInRect(thePoint, &theRect))
		{	SetCursor(&gIBeamCursor);
		}
		else
		{	SetCursor(&qd.arrow);
		}
		
		SetPort(saved);
	}
}
