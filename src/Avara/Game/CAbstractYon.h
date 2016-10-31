/*
    Copyright �1996, Juri Munkki
    All rights reserved.

    File: CAbstractYon.h
    Created: Thursday, August 22, 1996, 2:53
    Modified: Thursday, August 22, 1996, 5:00
*/

#pragma once
#include "CAbstractActor.h"

class	CAbstractYon : public CAbstractActor
{
public:
			Vector				minBounds;
			Vector				maxBounds;
			Vector				sphere;
	class	CAbstractYon		*nextYon;

			MessageRecord		startMsg;
			MessageRecord		stopMsg;
			short				status;

	virtual	void				BeginScript();
	virtual	CAbstractActor *	EndScript();
	virtual	void				SetYon();
	virtual	void				FrameAction();
	virtual	Fixed				AdjustYon(Fixed *location, Fixed curYon);
};