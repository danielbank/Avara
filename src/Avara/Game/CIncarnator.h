/*
    Copyright �1995-1996, Juri Munkki
    All rights reserved.

    File: CIncarnator.h
    Created: Tuesday, May 23, 1995, 17:39
    Modified: Saturday, February 17, 1996, 4:18
*/

#pragma once
#include "CPlacedActors.h"

class	CIncarnator;

class	CIncarnator : public CPlacedActors
{
public:
			CIncarnator		*nextIncarnator;
			long			colorMask;
			long			useCount;
			MessageRecord		startMsg;
			MessageRecord		stopMsg;
			Boolean				enabled;
			
	virtual	void			BeginScript();
	virtual	CAbstractActor	*EndScript();
	virtual	void			FrameAction();
};