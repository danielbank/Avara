/*
*/

#define	OPTIONSUTIL
#include "OptionsUtil.h"
#include "CEventHandler.h"

pascal
Boolean	OptionsFilter(
	DialogPtr	theDialog,
	EventRecord	*theEvent,
	short		*itemHit)
{
	Rect	iRect;
	Handle	iHandle;
	short	iType;
	GrafPtr	saved;
	Boolean	didHandle = false;
	short	doHilite = 0;

	GetPort(&saved);
	SetPort(theDialog);

	switch(theEvent->what)
	{	case updateEvt:
			if(theDialog == (DialogPtr)theEvent->message)
			{	GetDItem(theDialog, defaultButton, &iType, &iHandle, &iRect);
				PenSize(3,3);
				InsetRect(&iRect, -4, -4);
				FrameRoundRect(&iRect, 16, 16);
				PenSize(1,1);
			}
			else
			{	gApplication->ExternalEvent(theEvent);
			}
			break;
		case keyDown:
			{	char	theChar;
			
				theChar = theEvent->message;
				if(theChar == 13 || theChar == 3)
				{	*itemHit = defaultButton;
					didHandle = true;
					doHilite = defaultButton;
				}
				else if(theChar == 27 ||
					 (theChar == '.' && (theEvent->modifiers & cmdKey)))
				{	*itemHit = optCancel;
					doHilite = optCancel;
					didHandle = true;
				}
			}
			break;
	}
	
	if(doHilite)
	{	ControlHandle	theControl;
		long			finalTick;
	
		GetDItem(theDialog, doHilite, &iType, (Handle *)&theControl, &iRect);
		HiliteControl(theControl, 1);
		Delay(3, &finalTick);
		HiliteControl(theControl, 0);
	}

	gApplication->GotEvent();

	SetPort(saved);
	return didHandle;
}

void	SetOptionValue(
	short	ind,
	short	value)
{
	ControlHandle	theControl;
	short			iType;
	Rect			iRect;
	
	if(value)	value = true;

	GetDItem(optsDialog, ind, &iType, (Handle *)&theControl, &iRect);
	if(GetCtlValue(theControl) != value)
	{	SetCtlValue(theControl, value);
	}
}

void	DimOptionControl(
	short	ind)
{
	ControlHandle	theControl;
	short			iType;
	Rect			iRect;
	
	GetDItem(optsDialog, ind, &iType, (Handle *)&theControl, &iRect);
	HiliteControl(theControl, 255);
}
