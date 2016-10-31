/*
    Copyright �1994-1996, Juri Munkki
    All rights reserved.

    File: CDoorActor.h
    Created: Sunday, December 4, 1994, 9:59
    Modified: Saturday, February 17, 1996, 2:47
*/

#pragma once
#include "CGlowActors.h"

#define	kDefaultDoorBSP	550
#define	kDoorClosed		0
#define	kDoorOpen		65536

#define	kDoorSpeed	FIX3(20)

enum	{	kDoorStopped, kDoorOpening, kDoorClosing	};

class	CDoorActor : public CGlowActors
{
public:
			Fixed			doorStatus;
			Fixed			openSpeed;
			Fixed			closeSpeed;
			
			Fixed			deltas[3];
			Fixed			twists[3];
			
			MessageRecord	openActivator;
			MessageRecord	closeActivator;
			
			MsgType			didOpen;
			MsgType			didClose;

			long			closeCounter;
			long			openCounter;

			long			openDelay;
			long			closeDelay;
			long			collisionGuardTime;
			
			short			action;

			short			openSoundId;
			short			closeSoundId;
			short			stopSoundId;
			Fixed			doorSoundVolume;
			
			Fixed			hitPower;
			
			Fixed			lastMovement[3];

	virtual	void			BeginScript();
	virtual	CAbstractActor	*EndScript();
	virtual	void			Dispose();
	
	virtual	CSmartPart		*CollisionTest();
	virtual	void			TouchDamage();
	virtual	void			PlaceParts();
	virtual	void			FrameAction();
	
	virtual	void			DoorSound();
	
	virtual	void			ProgramDoorVariables();
	virtual	void			ReadDoorVariables();

	virtual	void			StandingOn(CAbstractActor *who, Fixed *where, Boolean firstLeg);
};