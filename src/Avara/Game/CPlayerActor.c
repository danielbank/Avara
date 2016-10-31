/*
    Copyright �1994-1995, Juri Munkki
    All rights reserved.

    File: CPlayerActor.c
    Created: Tuesday, November 22, 1994, 5:50
    Modified: Tuesday, December 5, 1995, 6:07
*/

#include "CPlayerActor.h"
#include "CSmartPart.h"
#include "CBSPWorld.h"
#include "AvaraDefines.h"

#define	kSpiderBSP			200
#define	kGunBSP				201

void	CPlayerActor::LoadParts()
{
	partCount = 2;
	LoadPart(0, kSpiderBSP);
	partList[0]->usesPrivateHither = true;
	partList[0]->hither = FIX3(500);
	
	LoadPart(1, kGunBSP);
	partList[1]->usesPrivateHither = true;
	partList[1]->hither = FIX3(100);
	
	viewPortPart = partList[1];

	proximityRadius = 2*partList[0]->header.enclosureRadius;
}

void	CPlayerActor::StartSystems()
{
	inherited::StartSystems();
	
}

void	CPlayerActor::PlaceParts()
{
	partList[0]->Reset();
	InitialRotatePartY(partList[0], heading);
	TranslatePart(partList[0], location[0], location[1], location[2]);
	partList[0]->MoveDone();

	partList[1]->Reset();
	InitialRotatePartZ(partList[1], FMul(viewYaw, -30));
	partList[1]->RotateX(viewPitch);
	partList[1]->RotateY(viewYaw);
	TranslatePartZ(partList[1], FIX3(-750));
	partList[1]->RotateOneY(heading);
	TranslatePart(partList[1], location[0], location[1]+FIX3(1300), location[2]);
	partList[1]->MoveDone();

	inherited::PlaceParts();
}

void	CPlayerActor::AvoidBumping()
{
	enum	{	undoHeadTurn, undoTurn, undoCount	};

	short			undoStep;
	CSmartPart		*objHit, *objTest;

	PlaceParts();
	objHit = DoCollisionTest();
	
	if(objHit)
	{	location[0] -= speed[0];
		location[2] -= speed[2];

		motors[0] = FMul(motors[0], friction);
		motors[1] = FMul(motors[1], friction);
		
		undoStep = 0;

		do
		{	switch(undoStep)
			{	case undoHeadTurn:
					viewYaw = oldYaw;
					viewPitch = oldPitch;
					break;
						
				case undoTurn:
					heading -= headChange;
					break;	
			}
	
			PlaceParts();
			objTest = DoCollisionTest();
	
		} while(objTest && ++undoStep < undoCount);
	
		FindBestMovement(objHit);
	}
}
