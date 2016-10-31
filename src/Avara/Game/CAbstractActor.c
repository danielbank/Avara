/*
    Copyright �1994-1996, Juri Munkki
    All rights reserved.

    File: CAbstractActor.c
    Created: Sunday, November 20, 1994, 19:17
    Modified: Monday, September 16, 1996, 18:50
*/

#include "CAbstractActor.h"
#include "CScaledBSP.h"
#include "CDepot.h"
#include "CSoundMixer.h"

void	CAbstractActor::LoadPart(
	short	ind,
	short	resId)
{
	if(partScale == FIX(1))
	{	partList[ind] = new CSmartPart;
		partList[ind]->ISmartPart(resId, this, ind);
	}
	else
	{	CScaledBSP	*part;
	
		part = new CScaledBSP;
		part->IScaledBSP(partScale, resId, this, ind);
		partList[ind] = part;
	}
	
	if(partYon != 0 && partList[ind])
	{	partList[ind]->usesPrivateYon = true;
		partList[ind]->yon = partYon;
	}
}

void	CAbstractActor::LoadPartWithColors(
	short	ind,
	short	resId)
{
	LoadPart(ind, resId);
	partList[ind]->ReplaceColor(kMarkerColor, GetPixelColor());
	partList[ind]->ReplaceColor(kOtherMarkerColor, GetOtherPixelColor());
}

void	CAbstractActor::InitLocationLinks()
{
	short	i;
	
	for(i=0;i<4;i++)
	{	locLinks[i].next = NULL;
		locLinks[i].prev = NULL;
		locLinks[i].me = this;
	}
	
	searchCount = gCurrentGame->searchCount;
}

void	CAbstractActor::UnlinkLocation()
{
	short			i;
	ActorLocator	*loc;
	
	loc = locLinks;

	i = 4;
	do
	{	if(loc->next)
		{	loc->prev->next = loc->next;
			loc->next->prev = loc->prev;
			loc->next = NULL;
		}
		else
			break;

		loc++;
	} while(--i);
}

#define	LOCATORTABLESCALE	(LOCATORRECTSCALE-2)

void	CAbstractActor::LinkBox(
register	Fixed	minX,
register	Fixed	minZ,
register	Fixed	maxX,
register	Fixed	maxZ)
{
	char			*linkTable;
	ActorLocator	*head;
	ActorLocator	*loc;
	long			mask = LOCCOORDMASK;
	
	minX = (minX & mask) >> (LOCATORTABLESCALE-LOCATORTABLEBITS);
	maxX = (maxX & mask) >> (LOCATORTABLESCALE-LOCATORTABLEBITS);
	minZ = (minZ & mask) >> LOCATORTABLESCALE;
	maxZ = (maxZ & mask) >> LOCATORTABLESCALE;
	
	loc = locLinks;
	if(loc->next)
	{	UnlinkLocation();
	}
	linkTable = (char *)gCurrentGame->locatorTable;
	
	head = (ActorLocator *)&linkTable[minX | minZ];
	loc->prev = head;
	loc->next = head->next;
	loc->next->prev = loc;
	head->next = loc++;
	
	if(minX != maxX)
	{	head = (ActorLocator *)&linkTable[maxX | minZ];
		loc->prev = head;
		loc->next = head->next;
		loc->next->prev = loc;
		head->next = loc++;
		
		if(minZ != maxZ)
		{	head = (ActorLocator *)&linkTable[minX | maxZ];
			loc->prev = head;
			loc->next = head->next;
			loc->next->prev = loc;
			head->next = loc++;
			
			head = (ActorLocator *)&linkTable[maxX | maxZ];
			loc->prev = head;
			loc->next = head->next;
			loc->next->prev = loc;
			head->next = loc;
		}
	}
	else
	if(minZ != maxZ)
	{	head = (ActorLocator *)&linkTable[minX | maxZ];
		loc->prev = head;
		loc->next = head->next;
		loc->next->prev = loc;
		head->next = loc;
	}
}

void	CAbstractActor::LinkSphere(
	Fixed	*origin,
	Fixed	range)
{
	LinkBox(origin[0]-range, origin[2]-range,
			origin[0]+range, origin[2]+range);
}

void	CAbstractActor::LinkPartBoxes()
{
	CSmartPart	**thePart;
	Vector		minPoint;
	Vector		maxPoint;
	
	minPoint[0] = MAXFIXED;
	minPoint[1] = MAXFIXED;	//	We don't really care about y coordinates though
	minPoint[2] = MAXFIXED;

	maxPoint[0] = MINFIXED;
	maxPoint[1] = MINFIXED;	//	y is still useless, but why not...
	maxPoint[2] = MINFIXED;
	
	for(thePart = partList; *thePart; thePart++)
	{	if((*thePart)->isTransparent == false)
			(*thePart)->MinMaxGlobalBox(minPoint, maxPoint);
	}
	
	LinkBox(minPoint[0], minPoint[2], maxPoint[0], maxPoint[2]);
}

void	CAbstractActor::LinkPartSpheres()
{
	CSmartPart	**thePart;
	Vector		minPoint;
	Vector		maxPoint;
	
	minPoint[0] = MAXFIXED;
	minPoint[1] = MAXFIXED;	//	We don't really care about y coordinates though
	minPoint[2] = MAXFIXED;

	maxPoint[0] = MINFIXED;
	maxPoint[1] = MINFIXED;	//	y is still useless, but why not...
	maxPoint[2] = MINFIXED;
	
	for(thePart = partList; *thePart; thePart++)
	{	if((*thePart)->isTransparent == false)
			(*thePart)->MinMaxGlobalSphere(minPoint, maxPoint);
	}
	
	LinkBox(minPoint[0], minPoint[2], maxPoint[0], maxPoint[2]);
}

void	CAbstractActor::BuildPartSpheresProximityList(
	MaskType	filterMask)
{
	CSmartPart	**thePart;
	Vector		minPoint;
	Vector		maxPoint;
	
	minPoint[0] = MAXFIXED;
	minPoint[1] = MAXFIXED;	//	We don't really care about y coordinates though
	minPoint[2] = MAXFIXED;

	maxPoint[0] = MINFIXED;
	maxPoint[1] = MINFIXED;	//	y is still useless, but why not...
	maxPoint[2] = MINFIXED;
	
	for(thePart = partList; *thePart; thePart++)
	{	if((*thePart)->isTransparent == false)
			(*thePart)->MinMaxGlobalSphere(minPoint, maxPoint);
	}

	maxPoint[0] = (maxPoint[0] - minPoint[0]) >> 1;	
	maxPoint[1] = (maxPoint[1] - minPoint[1]) >> 1;	
	maxPoint[2] = (maxPoint[2] - minPoint[2]) >> 1;	

	minPoint[0] += maxPoint[0];
	minPoint[1] += maxPoint[1];
	minPoint[2] += maxPoint[2];

	BuildPartProximityList(minPoint, FDistanceOverEstimate(maxPoint[0], maxPoint[1], maxPoint[2]), filterMask);
}

void	CAbstractActor::IAbstractActor()
{
	short	i;

	proximityList.p = NULL;
	partCount = 0;
	mass = 0;
	isInGame = false;
	maskBits = 0;
	itsGame = gCurrentGame;
	itsSoundLink = NULL;
	isActive = kIsInactive;
	sleepTimer = 0;
	teamColor = kNeutralTeam;
	ident = 0;
	identLink = NULL;
	attachmentList = NULL;

	hitSoundId = 210;
	hitSoundVolume = DEFAULTHITVOLUME;
	blastVolume = FIX(DEFAULTBLASTVOLUME);
	
	shields = -1;
	hitScore = 0;
	destructScore = 0;
	
	hitMessage = 0;
	destructMessage = 0;
	stepOnMessage = 0;
	
	postMortemLink = NULL;

	InitLocationLinks();

	for(i=0;i<=MAXPARTS;i++)
	{	partList[i] = NULL;
	}
	
	partScale = FIX(1);
	partYon = 0;
	
	for(i=0;i<kSliverSizes;i++)
	{	sliverCounts[i] = 0;
		sliverLives[i] = 12;
	}
	
	sliverCounts[kSmallSliver] = 12;
	blastPower = 0;
	nextActor = NULL;
	
	cachePart = NULL;
	
	traction = kDefaultTraction;
	friction = kDefaultFriction;
}

void	CAbstractActor::Dispose()
{
	short			i;
	ActorAttachment	*nextA;

	while(attachmentList)
	{	nextA = attachmentList->next;
		attachmentList->me->ReleaseAttachment();
		attachmentList = nextA;
	}
	
	if(itsGame)
		itsGame->MessageCleanup(this);

	if(ident)	gCurrentGame->RemoveIdent(ident);

	UnlinkLocation();

	if(isInGame)
		itsGame->RemoveActor(this);
	
	for(i=0;i<partCount;i++)
	{	partList[i]->Dispose();
	}
	
	if(itsSoundLink)
	{	gHub->ReleaseLinkAndKillSounds(itsSoundLink);
	}

	inherited::Dispose();
}

void	CAbstractActor::Shatter(
	short	firstSliverType,
	short	sizesCount,
	short	*sCounts,
	short	*sLives,
	Fixed	speedFactor)
{
	short		i;
	CSmartPart	**partInd, *thePart;
	long		totalPolys;
	Vector		noDirection = { 0, FIX(1), 0, 0};

	partInd = partList;
	totalPolys = 0;

	while(thePart = *partInd++)
	{	if(!thePart->isTransparent)
			totalPolys += thePart->header.polyCount;
	}
	
	if(totalPolys)
	{	short	sliverTemps[kSliverSizes];
		long	counter;

		for(i=0;i<sizesCount;i++)	sliverTemps[i] = 0;

		counter = 0;

		partInd--;
		while(partInd-- != partList)
		{	Fixed	blowSpeed;

			thePart = *partInd;
			blowSpeed = FMul(speedFactor, FRadArcTan2(FIX3(500),thePart->header.enclosureRadius)>>1);
			//FSqrt(thePart->header.enclosureRadius));

			if(!thePart->isTransparent)
			{	counter += thePart->header.polyCount;
				for(i=0;i<sizesCount;i++)
				{	if(sCounts[i] && sLives[i])
					{	short	share;
					
						share = sCounts[i] * counter / totalPolys - sliverTemps[i];
						if(share > 0)
						{	
							itsGame->itsDepot->FireSlivers(
										share,
										thePart->sphereGlobCenter, noDirection,
										thePart->header.enclosureRadius,
										blowSpeed,
										180, sLives[i], i+firstSliverType, thePart);
							sliverTemps[i] += share;
						}
					}
				}
			}
		}
	}
}

void	CAbstractActor::Blast()
{
	
	if(isInGame)
	{
		short		i;
		CSmartPart	*thePart, *maxPart;
		SoundLink	*blastLink;
		short		maxCount = 0;
	
		Shatter(0, kSliverSizes, sliverCounts, sliverLives, FIX3(500));

		for(i=0;i<partCount;i++)
		{	thePart = partList[i];
			if((!thePart->isTransparent) && thePart->header.polyCount > maxCount)
			{	maxCount = thePart->header.polyCount;
				maxPart = thePart;
			}
		}

		if(maxCount)
		{	DoSound(blastSound, maxPart->sphereGlobCenter, blastVolume, FIX(1));
		}
	}
}

void	CAbstractActor::BeginScript()
{
	short	i;
	
	ProgramLongVar(iTeam, teamColor);
	ProgramLongVar(iHitSound, ReadLongVar(iHitSoundDefault));	
	ProgramLongVar(iHitVolume, DEFAULTHITVOLUME);

	ProgramLongVar(iIsTarget, maskBits & kTargetBit);
	ProgramLongVar(iBlastSound, ReadLongVar(iBlastSoundDefault));	
	ProgramFixedVar(iBlastVolume, blastVolume);

	ProgramFixedVar(iPower, 0);
	ProgramFixedVar(iShieldEnergy, shields);

	ProgramLongVar(iDestructScore, destructScore);
	ProgramLongVar(iHitScore, hitScore);
	
	ProgramLongVar(iDestructMessage, destructMessage);
	ProgramLongVar(iHitMessage, hitMessage);
	ProgramLongVar(iStepOnMessage, stepOnMessage);
	
	ProgramFixedVar(iMass, mass);
	ProgramFixedVar(iScale, partScale);
	ProgramFixedVar(iYon, partYon);	

	for(i=0;i<3;i++)
	{	ProgramLongVar(iSlivers0+i, sliverCounts[i]);
		ProgramLongVar(iSliverL0+i, sliverLives[i]);
	}
	
	ProgramReference(iTraction, iDefaultTraction);
	ProgramReference(iFriction, iDefaultFriction);
	
	ProgramLongVar(iTargetGroup, 0);
}

CAbstractActor *CAbstractActor::EndScript()
{
	maskBits |= ReadLongVar(iTargetGroup) &
				(kTargetGroup1 + kTargetGroup2 + kTargetGroup3 +
				 kTargetGroup4 + kTargetGroup5 + kTargetGroup6);
	
	hitSoundId = ReadLongVar(iHitSound);
	hitSoundVolume = ReadLongVar(iHitVolume);

	if(ReadLongVar(iIsTarget))
		maskBits |= kTargetBit;
	else
		maskBits &= ~kTargetBit;
	blastSound = ReadLongVar(iBlastSound);
	blastVolume = ReadFixedVar(iBlastVolume);

	blastPower = ReadFixedVar(iPower);
	shields = ReadFixedVar(iShieldEnergy);

	destructScore = ReadLongVar(iDestructScore);
	hitScore = ReadLongVar(iHitScore);	

	destructMessage = ReadLongVar(iDestructMessage);
	hitMessage = ReadLongVar(iHitMessage);
	stepOnMessage = ReadLongVar(iStepOnMessage);
	
	mass = ReadFixedVar(iMass);

	teamColor = ReadLongVar(iTeam);
	teamMask = 1<<teamColor;

	gHub->PreLoadSample(blastSound);
	gHub->PreLoadSample(hitSoundId);
	
	partScale = ReadFixedVar(iScale);
	partYon = ReadFixedVar(iYon);
	
	traction = ReadFixedVar(iTraction);
	friction = ReadFixedVar(iFriction);
	
	stepSound = ReadLongVar(iStepSound);
	gHub->LoadSample(stepSound);
	
	return this;
}

void	CAbstractActor::AddToGame()
{
	itsGame = gCurrentGame;
	itsGame->AddActor(this);
}

void	CAbstractActor::FrameAction()
{
	//	Just sit there. Subclasses actually do something.
}

void	CAbstractActor::BuildPartProximityList(
	Fixed		*origin,
	Fixed		range,
	MaskType	filterMask)
{
	Fixed			x,z;
	Fixed			startX,endX,startZ,endZ;
	ActorLocator	*head;
	ActorLocator	**lists;
	CAbstractActor	*anActor;
	CSmartPart		**thePart;
	ActorAttachment	*parasites;
	long			coordMask = ((1L << LOCATORTABLEBITS)-1);

	startX =	LOCCOORDTOINDEX(origin[0] - range);
	endX =		LOCCOORDTOINDEX(origin[0] + range);

	startZ =	LOCCOORDTOINDEX(origin[2] - range);
	endZ =		LOCCOORDTOINDEX(origin[2] + range);


	lists = itsGame->locatorTable;
	searchCount = ++itsGame->searchCount;
	parasites = attachmentList;
	while(parasites)
	{	parasites->me->searchCount = searchCount;
		parasites = parasites->next;
	}

	proximityList.p = NULL;

	x = startX;
	do
	{	z = startZ;
		do
		{	head = lists[(x<<LOCATORTABLEBITS) + z];
		
			while(head->next)
			{	anActor = head->me;
				if(anActor->searchCount != searchCount)
				{	anActor->searchCount = searchCount;
					if(anActor->maskBits & filterMask)
					{	for(thePart = anActor->partList; *thePart; thePart++)
						{	if((*thePart)->IsInRange(origin, range))
							{	(*thePart)->nextTemp = proximityList.p;
								proximityList.p = *thePart;
							}
						}
					}
				}
				
				head = head->next;
			}
			
			if(z == endZ) break;
			
			z++;
			z &= coordMask;

		} while(true);

		if(x == endX) break;
		
		x++;
		x &= coordMask;

	} while(true);
}

void	CAbstractActor::BuildActorProximityList(
	Fixed	*origin,
	Fixed	range,
	MaskType	filterMask)
{
	Fixed			x,z;
	Fixed			startX,endX,startZ,endZ;
	ActorLocator	*head;
	ActorLocator	**lists;
	CAbstractActor	*anActor;
	long			coordMask = ((1L << LOCATORTABLEBITS)-1);
	ActorAttachment	*parasites;

	startX =	LOCCOORDTOINDEX(origin[0] - range);
	endX =		LOCCOORDTOINDEX(origin[0] + range);

	startZ =	LOCCOORDTOINDEX(origin[2] - range);
	endZ =		LOCCOORDTOINDEX(origin[2] + range);

	lists = itsGame->locatorTable;
	searchCount = ++itsGame->searchCount;
	parasites = attachmentList;
	while(parasites)
	{	parasites->me->searchCount = searchCount;
		parasites = parasites->next;
	}

	proximityList.a = NULL;

	x = startX;
	do
	{	z = startZ;
		do
		{	head = lists[(x<<LOCATORTABLEBITS) + z];
		
			while(head->next)
			{	anActor = head->me;
				if(anActor->searchCount != searchCount)
				{	anActor->searchCount = searchCount;
					if(anActor->maskBits & filterMask)
					{	anActor->proximityList.a = proximityList.a;
						proximityList.a = anActor;
					}
				}
				
				head = head->next;
			}
			
			if(z == endZ) break;
			
			z++;
			z &= coordMask;

		} while(true);

		if(x == endX) break;
		
		x++;
		x &= coordMask;

	} while(true);
}

/*
**	DoCollisionTest tests the part list against the proximity part list.
**	If a hit is found, the colliding part is moved to the front of the
**	list and returned. In addition, the colliding part is cached into a
**	variable and given priority the next time this method is called.
*/
CSmartPart	*CAbstractActor::DoCollisionTest(
	CSmartPart	**hitList)
{

	CSmartPart		**prevP;
	CSmartPart		*thePart;
	CSmartPart		*myPart;
	short			i;
	ActorAttachment	*paraList;

#if	DEBUG_AVARA
	if(*hitList == (CSmartPart *)-1)
		Debugger();
#endif

	/*	First, look for the cached part only.
	*/	
	for(i=0;i<partCount;i++)
	{	myPart = partList[i];
		if(cachePart == myPart && myPart->isTransparent == false)
		{	prevP = hitList;
			for(thePart = *hitList; thePart; thePart = *(prevP = (CSmartPart **)&thePart->nextTemp))
			{
				if(thePart->CollisionTest(myPart))
				{
					*prevP = (CSmartPart *)thePart->nextTemp;
					thePart->nextTemp = *hitList;
					*hitList = thePart;

					return thePart;
				}
			}
		}
	}

	/*	Next, look for the other parts.
	*/
	for(i=0;i<partCount;i++)
	{	myPart = partList[i];
		if(cachePart != myPart && myPart->isTransparent == false)
		{	prevP = hitList;
			for(thePart = *hitList; thePart; thePart = *(prevP = (CSmartPart **)&thePart->nextTemp))
			{
				if(thePart->CollisionTest(myPart))
				{
					*prevP = (CSmartPart *)thePart->nextTemp;
					thePart->nextTemp = *hitList;
					*hitList = thePart;
					cachePart = myPart;

					return thePart;
				}
			}
		}
	}

	paraList = attachmentList;
	while(paraList)
	{	thePart = paraList->me->DoCollisionTest(hitList);
		if(thePart)
			return thePart;

		paraList = paraList->next;
	}

	return NULL;
}

void	CAbstractActor::RadiateDamage(
	BlastHitRecord	*blastRecord)
{
	CAbstractActor	*anActor, *next;
	Fixed			radRange;
		
	radRange = FSqrt(blastRecord->blastPower<<4);
	
	BuildActorProximityList(blastRecord->blastPoint, radRange, kSolidBit);
	
	anActor = proximityList.a;

	while(anActor)
	{	next = anActor->proximityList.a;
		anActor->BlastHit(blastRecord);
		anActor = next;
	}
}

Boolean	CAbstractActor::GetActorCenter(
	Fixed	*loc)
{
	CSmartPart	**thePart;
	Vector		minPoint;
	Vector		maxPoint;
	Boolean		gotLocation = false;
	
	minPoint[0] = MAXFIXED;
	minPoint[1] = MAXFIXED;	//	We don't really care about y coordinates though
	minPoint[2] = MAXFIXED;

	maxPoint[0] = MINFIXED;
	maxPoint[1] = MINFIXED;	//	y is still useless, but why not...
	maxPoint[2] = MINFIXED;
	
	for(thePart = partList; *thePart; thePart++)
	{	if((*thePart)->isTransparent != true)
		{	gotLocation = true;
			(*thePart)->MinMaxGlobalSphere(minPoint, maxPoint);
		}
	}
	
	if(gotLocation)
	{	loc[0] = (minPoint[0] + maxPoint[0]) >> 1;
		loc[1] = (minPoint[1] + maxPoint[1]) >> 1;
		loc[2] = (minPoint[2] + maxPoint[2]) >> 1;
	}

	return gotLocation;
}

void	CAbstractActor::PostMortemBlast(
	short	scoreTeam,
	short	scoreId,
	Boolean	doDispose)
{
	BlastHitRecord		blastRecord;
	
	UnlinkLocation();
	
	if(blastPower && GetActorCenter(blastRecord.blastPoint))
	{	blastRecord.blastPower = blastPower;
		blastRecord.team = scoreTeam;
		blastRecord.playerId = scoreId;
		RadiateDamage(&blastRecord);
	}
	
	if(doDispose)	Dispose();
}

void	CAbstractActor::SecondaryDamage(
	short	scoreTeam,
	short	scoreColor)
{
	CAbstractActor	*blastToo;
	
#if DEBUG_AVARA
	proximityList.p = (CSmartPart *)-1;
#endif

	while(blastToo = gCurrentGame->postMortemList)
	{	gCurrentGame->postMortemList = blastToo->postMortemLink;
		if(blastToo != this)
		{	gCurrentGame->scoreReason = ksiSecondaryDamage;
		}
		blastToo->PostMortemBlast(scoreTeam, scoreColor, true);
	}
}

void	CAbstractActor::RayTestWithGround(
	RayHitRecord	*hitRec,
	MaskType		testMask)
{
	Fixed	negOrigin1;
	
	negOrigin1 = - hitRec->origin[1];
	if(	hitRec->direction[1] < 0 &&
		negOrigin1 < 0 &&
		FMul(hitRec->distance, hitRec->direction[1]) < negOrigin1)
	{	hitRec->distance = FDivNZ(negOrigin1, hitRec->direction[1]);
	}

	RayTest(hitRec, testMask);
}

void	CAbstractActor::RayTest(
register	RayHitRecord	*hitRec,
			MaskType		testMask)
{
	long			xs, zs, dxs, dzs;
	long			endIndex, currentIndex;

	long			dx, dz;
	unsigned long	xf, zf;
	long			error;
	unsigned long	err1, err2;
	
	ActorLocator	*head;
	ActorLocator	**lists;
	CAbstractActor	*anActor;
	CSmartPart		**thePartList;
	ActorAttachment	*parasites;
	
	dx = hitRec->direction[0];
	dz = hitRec->direction[2];
	xs = hitRec->origin[0];
	zs = hitRec->origin[2];

	endIndex = LOCTOTABLE(	xs+FMul(dx, hitRec->distance),
							zs+FMul(dz, hitRec->distance));

	xf = (unsigned short)(xs >> (LOCATORRECTSCALE-16));
	zf = (unsigned short)(zs >> (LOCATORRECTSCALE-16));
	
	if(dx > 0)
	{	dxs = LOCATORRECTSIZE;
		xf = 65536 - xf;
	}
	else
	{	dxs = -LOCATORRECTSIZE;
		dx = -dx;
		xf += 1;
	}
	
	if(dz > 0)
	{	dzs = LOCATORRECTSIZE;
		zf = 65536 - zf;
	}
	else
	{	dzs = -LOCATORRECTSIZE;
		dz = -dz;
		zf += 1;
	}
	
	if(dx > dz)
	{	err1 = dx * zf;
		err2 = dz * xf;
	}
	else
	{	err1 = dz * xf;
		err2 = dx * zf;
	}
	
	asm
	{	move.l	err1,D0
		sub.l	err2,D0
		roxr.l	#1,D0
		moveq.l	#15,D1
		asr.l	D1,D0
		move.l	D0,error
	}
	
	lists = itsGame->locatorTable;
	searchCount = ++itsGame->searchCount;
	parasites = attachmentList;
	while(parasites)
	{	parasites->me->searchCount = searchCount;
		parasites = parasites->next;
	}

	do
	{	currentIndex = LOCTOTABLE(xs,zs);
		for(head = lists[currentIndex]; head->next; head = head->next)
		{	anActor = head->me;
			if(anActor->searchCount != searchCount)
			{	anActor->searchCount = searchCount;
				if(anActor->maskBits & testMask)
				{	for(thePartList = anActor->partList; *thePartList; thePartList++)
					{	(*thePartList)->RayTest(hitRec);
					}
				}
			}
		}

		if(dx > dz)
		{	if(error < 0)
			{	zs += dzs;
				error += dx;
			}
			else
			{	xs += dxs;
				error -= dz;
			}
		}
		else
		{	if(error < 0)
			{	xs += dxs;
				error += dz;
			}
			else
			{	zs += dzs;
				error -= dx;
			}
		}
	} while(currentIndex != endIndex && hitRec->closestHit == NULL);
}

static	long	totSoundCount = 0;
static	long	disSoundCount = 0;

void	CAbstractActor::DoSound(
	short	resId,
	Fixed	*where,
	Fixed	volume,
	Fixed	rate)
{
	SoundLink	*statLink;
	CBasicSound	*dullSound;
	Fixed		*ear;
	Fixed		dist;
	Fixed		estim;
	
	totSoundCount++;
	
	if(volume > FIX3(300))
	{	if(volume < FIX(40))
		{	ear = gHub->itsMixer->motionLink->nLoc;
			dist = FDistanceEstimate(where[0] - ear[0], where[1] - ear[1], where[2] - ear[2]);
			if(dist > FIX(2))
			{	estim = FMulDiv(gHub->itsMixer->distanceToLevelOne, volume, dist);
				if(estim < FIX3(800))
				{	return;
				}
			}
		}
		statLink = gHub->GetSoundLink();
		PlaceSoundLink(statLink, where);
		
		dullSound = gHub->GetSoundSampler(hubRate, resId);
		dullSound->SetRate(rate);
		dullSound->SetVolume(volume);
		dullSound->SetSoundLink(statLink);
		gHub->ReleaseLink(statLink);
		dullSound->Start();
		disSoundCount++;
	}
}

short	CAbstractActor::GetActorScoringId()
{
	return -1;
}

void	CAbstractActor::WasDestroyed()
{
	UnlinkLocation();

	Blast();

	postMortemLink = itsGame->postMortemList;
	itsGame->postMortemList = this;
}

void	CAbstractActor::WasHit(
	RayHitRecord			*theHit,
	Fixed					hitEnergy)
{	
	if(shields < 0 || shields > hitEnergy)
	{	itsGame->Score(theHit->team, theHit->playerId,
						FMul(hitScore, hitEnergy), hitEnergy,
						teamColor, GetActorScoringId());
		DoSound(hitSoundId, theHit->origin, hitSoundVolume * hitEnergy, FIX(1));
		itsGame->FlagMessage(hitMessage);

		if(shields >= 0) shields -= hitEnergy;
	}
	else
	{	if(shields >= 0)
		{	ScoreInterfaceReasons	savedReason;
		
			itsGame->Score(theHit->team, theHit->playerId,
						FMul(hitScore, shields), shields,
						teamColor, GetActorScoringId());
						
			savedReason = itsGame->scoreReason;
			itsGame->scoreReason = ksiKillBonus;
			itsGame->Score(theHit->team, theHit->playerId,
						destructScore, 0,
						teamColor, GetActorScoringId());
			itsGame->scoreReason = savedReason;

			itsGame->FlagMessage(hitMessage);
			itsGame->FlagMessage(destructMessage);

			shields = 0;
			WasDestroyed();
		}
	}
}

void	CAbstractActor::GetBlastPoint(
	BlastHitRecord	*theHit,
	RayHitRecord	*rayHit)
{
	Fixed		theDistance;
	Fixed		shortestDistance;
	Vector		*hitPoint;
	CSmartPart	**pp, *thePart;
	
	shortestDistance = FIX(128);
	rayHit->closestHit = NULL;

	pp = partList;
	while(thePart = *pp++)
	{	if(!thePart->isTransparent)
		{	theDistance = FDistanceEstimate(
							theHit->blastPoint[0] - thePart->sphereGlobCenter[0],
							theHit->blastPoint[1] - thePart->sphereGlobCenter[1],
							theHit->blastPoint[2] - thePart->sphereGlobCenter[2])
							- thePart->header.enclosureRadius;
			if(theDistance < shortestDistance)
			{	shortestDistance = theDistance;
				rayHit->closestHit = thePart;
			}
		}
	}
}

void	CAbstractActor::BlastHit(
	BlastHitRecord	*theHit)
{
	RayHitRecord	blastRay;
	CSmartPart		*thePart;
	Fixed			energy;
	Fixed			distance;
	
	GetBlastPoint(theHit, &blastRay);
	
	thePart = blastRay.closestHit;
	if(thePart)
	{	blastRay.direction[0] = thePart->sphereGlobCenter[0] - theHit->blastPoint[0];
		blastRay.direction[1] = thePart->sphereGlobCenter[1] - theHit->blastPoint[1];
		blastRay.direction[2] = thePart->sphereGlobCenter[2] - theHit->blastPoint[2];
		
		distance = NormalizeVector(3, blastRay.direction) - thePart->header.enclosureRadius;
		if(distance < FIX(1))
		{	distance = FIX(1);
			blastRay.origin[0] = theHit->blastPoint[0];
			blastRay.origin[1] = theHit->blastPoint[1];
			blastRay.origin[2] = theHit->blastPoint[2];
		}
		else
		{	blastRay.origin[0] = theHit->blastPoint[0] + FMul(distance, blastRay.direction[0]);
			blastRay.origin[1] = theHit->blastPoint[1] + FMul(distance, blastRay.direction[1]);
			blastRay.origin[2] = theHit->blastPoint[2] + FMul(distance, blastRay.direction[2]);
		}
		
		energy = FDiv(theHit->blastPower, FMul(distance, distance));
		if(energy > (65536>>6))
		{	blastRay.distance = distance;
			blastRay.team = theHit->team;
			blastRay.playerId = theHit->playerId;

			WasHit(&blastRay, energy);
		}
	}
}

void	CAbstractActor::Slide(
	Fixed	*direction)
{
	//	Subclass responsibility (by default, objects do not move).
}

void	CAbstractActor::Push(
	Fixed	*direction)
{
	//	Subclass responsibility (by default, objects do not move).
}

void	CAbstractActor::Accelerate(
	Fixed	*direction)
{
	//	Subclass responsibility (by default, objects do not move).
}

void	CAbstractActor::GetSpeedEstimate(
	Fixed	*theSpeed)
{
	*theSpeed++ = 0;
	*theSpeed++ = 0;
	*theSpeed++ = 0;
}

void	CAbstractActor::ResumeLevel()
{
	//	Subclass responsibility.
}

void	CAbstractActor::PauseLevel()
{
	//	Subclass responsibility.
}

/*
**	This method is called before the level is reset.
*/
void	CAbstractActor::LevelReset()
{
	Dispose();
}

void	CAbstractActor::RegisterReceiver(
	MessageRecord	*theMsg,
	MsgType			messageNum)
{
	theMsg->owner = this;
	itsGame->RegisterReceiver(theMsg, messageNum);
}

void	CAbstractActor::ImmediateMessage()
{
	//	Default is to do nothing with immediate messages...Ufos use them.
	//	Others will just handle them later on their own turn.
}

void	CAbstractActor::OffsetParts(
	Fixed	*offset)
{
	CSmartPart		**partL;
	ActorAttachment	*parasites;
	
	partL = partList;
	while(*partL)
	{	(*partL)->OffsetPart(offset);
		partL++;
	}

	parasites = attachmentList;
	while(parasites)
	{	parasites->me->OffsetParts(offset);
		parasites = parasites->next;
	}
}

void	CAbstractActor::PlaceParts()
{
	ActorAttachment	*parasites;
	
	parasites = attachmentList;
	while(parasites)
	{	parasites->me->PlaceParts();
		parasites = parasites->next;
	}
}

long	CAbstractActor::Attach(ActorAttachment *a)
{
	a->next = attachmentList;
	attachmentList = a;
	
	return ident;
}
void	CAbstractActor::Detach(ActorAttachment *a)
{
	ActorAttachment	**paraBackLink;

	paraBackLink = &attachmentList;
	
	while(*paraBackLink)
	{	if(*paraBackLink == a)
		{	*paraBackLink = a->next;
			a->next = NULL;
			return;
		}
		else
		{	paraBackLink = &(*paraBackLink)->next;
		}
	}
}

void	CAbstractActor::ReleaseAttachment()
{
	//	uh, I don't know what to do...abstract actors do not attach like parasites.
	//	In other words: subclass responsibility.
}

void	CAbstractActor::ReleaseAllAttachments()
{
	ActorAttachment	*parasite;
	ActorAttachment	*nextPara;

	parasite = attachmentList;
	while(parasite)
	{	nextPara = parasite->next;
		parasite->me->ReleaseAttachment();
		parasite = nextPara;
	}
}

long	CAbstractActor::SignalAttachments(
	long	theSignal,
	long	miscData)
{
	ActorAttachment	*parasite;
	ActorAttachment	*nextPara;
	long			response = 0;
	long			value;

	parasite = attachmentList;
	while(parasite)
	{	nextPara = parasite->next;
		value = parasite->me->ReceiveSignal(theSignal, miscData);
		if(value)
			response = value;

		parasite = nextPara;
	}
	
	return response;
}

long	CAbstractActor::ReceiveSignal(
	long	theSignal,
	long	miscData)
{
	return 0;
}

Fixed	CAbstractActor::GetTotalMass()
{
	Fixed	total = mass;
	ActorAttachment	*parasites;
	
	parasites = attachmentList;
	while(parasites)
	{	total += parasites->me->GetTotalMass();
		parasites = parasites->next;
	}

	return total;
}

void	CAbstractActor::StandingOn(
	CAbstractActor	*who,		//	Who is touching me?
	Fixed			*where,		//	Location of touch in global coordinates
	Boolean			firstLeg)	//	True, when this is the only or first leg to touch this object
{
}

void	CAbstractActor::StandOn(
	CAbstractActor	*who,		//	Who is touching me?
	Fixed			*where,		//	Location of touch in global coordinates
	Boolean			firstLeg,	//	True, when this is the only or first leg to touch this object
	Boolean			firstTouch)	//	True when the other object first steps on this object
{
	if(firstTouch && firstLeg)
	{	itsGame->FlagMessage(stepOnMessage);
	}
}

void	CAbstractActor::GetFrictionTraction(
	Fixed	*tract,
	Fixed	*frict)
{
	*tract = traction;
	*frict = friction;
}

short	CAbstractActor::GetPlayerPosition()
{
	return -1;	//	Not a player, as far as we know, so return -1 (invalid position)
}

short	CAbstractActor::GetBallSnapPoint(
	long		theGroup,
	Fixed		*ballLocation,
	Fixed		*snapDest,
	Fixed		*delta,
	CSmartPart	**hostPart)
{
	return	kSnapNot;
}
