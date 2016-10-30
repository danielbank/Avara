/*/
    Copyright �1994-1995, Juri Munkki
    All rights reserved.

    File: CSmartPart.h
    Created: Wednesday, November 23, 1994, 0:36
    Modified: Sunday, December 10, 1995, 13:02
/*/

#pragma once
#include "CBSPPart.h"
#include "RayHit.h"

class	CAbstractActor;


class	CSmartPart : public CBSPPart
{
public:
			CAbstractActor	*theOwner;
			short			partCode;
			long			rSquare[2];
						
	virtual	void		ISmartPart(short resId, CAbstractActor *anActor, short aPartCode);
	virtual	Boolean		CollisionTest(CSmartPart *other);	//	Test for overlap.
	virtual	Boolean		HitTestBoxes(CSmartPart *other);
	
	virtual	void		RayTest(RayHitRecord *hitRec);
	virtual	Boolean		IsInRange(Fixed *origin, Fixed range);
	
	virtual	void		MinMaxGlobalBox(Fixed *minPoint, Fixed *maxPoint);
	virtual	void		MinMaxGlobalSphere(Fixed *minPoint, Fixed *maxPoint);
	
	virtual	void		OffsetPart(Fixed *offset);
};
