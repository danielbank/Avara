/*
    Copyright �1995-1996, Juri Munkki
    All rights reserved.

    File: CLogicTimer.c
    Created: Wednesday, November 22, 1995, 8:13
    Modified: Saturday, January 27, 1996, 12:04
*/

#include "CLogicTimer.h"

void	CLogicTimer::BeginScript()
{
	inherited::BeginScript();
	ProgramLongVar(iTimer, 16);
}

CAbstractActor *
		CLogicTimer::EndScript()
{
	if(inherited::EndScript())
	{
		theDelay = ReadLongVar(iTimer);
		if(theDelay < 0)
		{	theDelay = -theDelay;
			sleepTimer = theDelay+1;
		}
	
		if(theDelay)
			return this;
	}

	Dispose();
	return NULL;
}


void	CLogicTimer::FrameAction()
{
	short	i;
	
	inherited::FrameAction();
	
	if(isActive & kHasMessage)
	{	isActive &= ~kHasMessage;
		
		for(i=0;i<LOGIC_IN;i++)
		{	if(in[i].triggerCount)
			{	if(enabled)
				{	sleepTimer = theDelay;
					whenFrame = itsGame->frameNumber + sleepTimer;
				}
				in[i].triggerCount = 0;
			}
		}
	}
	else if(whenFrame == itsGame->frameNumber)
	{	//	Triggered!

		Trigger();
			
		if(!restart)
		{	Dispose();
			return;
		}
	}
}