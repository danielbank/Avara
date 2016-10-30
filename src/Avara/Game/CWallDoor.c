/*/
    Copyright �1995-1996, Juri Munkki
    All rights reserved.

    File: CWallDoor.c
    Created: Thursday, December 7, 1995, 9:18
    Modified: Saturday, August 10, 1996, 18:30
/*/

#include "CWallDoor.h"
#include "CWallActor.h"
#include "CSmartPart.h"
#include "CBSPWorld.h"

extern	CWallActor	*lastWallActor;

void	CWallDoor::LoadPart(
	short	ind,
	short	resId)
{
	if(lastWallActor)
	{	CSmartPart	*thePart;
	
		thePart = lastWallActor->partList[0];
		thePart->theOwner = this;

		partList[ind] = thePart;
	
		TranslatePartY(thePart, ReadLongVar(iHeight));
		VECTORCOPY(location, thePart->itsTransform[3]); 
		itsGame->itsWorld->RemovePart(thePart);

		heading = 0;
		lastWallActor->partList[0] = NULL;
		lastWallActor->partCount = 0;
		lastWallActor->Dispose();
		lastWallActor = NULL;
	}
	else
	{	inherited::LoadPart(ind, resId);
	}

}