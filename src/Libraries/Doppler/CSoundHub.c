/*
    Copyright �1994-1996, Juri Munkki
    All rights reserved.

    File: CSoundHub.c
    Created: Sunday, December 25, 1994, 11:06
    Modified: Tuesday, February 20, 1996, 8:59
*/

#include "CSoundHub.h"
#include "CRateSound.h"
#include "CSoundMixer.h"
#include "CBasicSound.h"
#include "CHuffProcessor.h"

void	CSoundHub::CreateSound(
	short	kind)
{
	CBasicSound	*aSound;

	switch(kind)
	{	case hubBasic:
			aSound = new CBasicSound;
			break;
		case hubRate:
			aSound = new CRateSound;
			break;	
		default:
			aSound = NULL;
	}
	
	if(aSound)
	{	aSound->nextSound = soundList[kind];
		aSound->itsHub = this;
		aSound->hubId = kind;
		soundList[kind] = aSound;
	}
}

void	CSoundHub::Restock(
	CBasicSound	*aSound)
{
	SampleHeaderHandle	aSample;

	aSound->nextSound = soundList[aSound->hubId];
	soundList[aSound->hubId] = aSound;

	if(aSound->itsSamples)
	{	aSample = aSound->itsSamples;
		if(--(*aSample)->refCount == 0)
		{	HUnlock((Handle)aSample);
		}
		
		aSound->itsSamples = NULL;
	}
}

CBasicSound *CSoundHub::Aquire(
	short	kind)
{
	CBasicSound	*aSound;
	
	if(soundList[kind] == NULL)
		CreateSound(kind);
	
	aSound = soundList[kind];
	if(aSound)
	{	soundList[kind] = aSound->nextSound;
		aSound->Reset();
	}
	
	return aSound;
}

void	CSoundHub::ISoundHub(
	short	numOfEachKind,
	short	initialLinks)
{
	short	i,j;
	
	itsMixer = NULL;

	for(j=0;j<hubSoundKinds;j++)
	{	soundList[j] = NULL;

		for(i=0;i<numOfEachKind;i++)
		{	CreateSound(j);
		}
	}
	
	muteFlag = false;
	sampleList = NULL;

	itsCompressor = new CHuffProcessor;
	itsCompressor->Open();
	
	soundLinkStorage = NULL;
	firstFreeLink = NULL;

	if(initialLinks > 0)
		CreateSoundLinks(initialLinks);
}

void	CSoundHub::AttachMixer(
	CSoundMixer *aMixer)
{
	itsMixer = aMixer;
	muteFlag = itsMixer->maxChannels == 0;
}

SampleHeaderHandle	CSoundHub::LoadSample(
	short	resId)
{
	SampleHeaderHandle	aSample;
	SampleHeaderPtr		sampP;
	
	aSample = sampleList;
	
	while(aSample)
	{	sampP = *aSample;
		if(sampP->resId == resId)
		{	sampP->flags = 0;
			break;
		}
		aSample = sampP->nextSample;
	}
	
	if(!aSample)
	{	Handle	compressedData;
		Handle	soundInfo;
	
		compressedData = GetResource(HSOUNDRESTYPE, resId);
		if(compressedData)
		{	long		len;
			short		tryCount;
			Ptr			soundData;
			HSNDRecord	*ir;

			MoveHHi(compressedData);
			HLock(compressedData);

			ir = (HSNDRecord *)*compressedData;

			soundData = ir->dataOffset + *compressedData;
			len = itsCompressor->GetUncompressedLen(soundData);
			
			aSample = (SampleHeaderHandle) NewHandle(sizeof(SampleHeader) + len);
			if(!aSample)
			{	FreeOldSamples();
				aSample = (SampleHeaderHandle) NewHandle(sizeof(SampleHeader) + len);
				
				if(!aSample)
				{	FreeUnusedSamples();
					aSample = (SampleHeaderHandle) NewHandle(sizeof(SampleHeader) + len);
				}
			}
			
			if(aSample)
			{	unsigned char	value;
				unsigned char	*p;
				long			i;

				sampP = *aSample;
				sampP->resId = resId;
				sampP->refCount = 0;
				sampP->flags = 0;
				sampP->len = len;
				sampP->loopStart = ir->loopStart;
				sampP->loopEnd = ir->loopEnd;
				sampP->loopCount = ir->loopCount;
   				sampP->nextSample = sampleList;
   				
   				if(ir->versNum < 2)
   				{	sampP->baseRate = FIX(1);
   				}
   				else
   				{	sampP->baseRate = ir->baseRate;   					
   				}

   				sampleList = aSample;
   				
   				HLock((Handle)aSample);
   				p = sizeof(SampleHeader) + (unsigned char *) sampP;
	   			itsCompressor->Uncompress(soundData, (Ptr) p);
   				HUnlock((Handle)aSample);

				value = 128 >> (8-BITSPERSAMPLE);
				for(i=0;i<len;i++)
				{	value += *p;
					*p++ = value & (0xFF >> (8-BITSPERSAMPLE));
				}
			}
			
			ReleaseResource(compressedData);
		}
	}
	
	return aSample;
}

SampleHeaderHandle	CSoundHub::PreLoadSample(
	short	resId)
{
	if(muteFlag)	return NULL;
	else			return LoadSample(resId);
}

SampleHeaderHandle	CSoundHub::RequestSample(
	short	resId)
{
	SampleHeaderHandle	aSample;
	SampleHeaderPtr		p;
	
	aSample = LoadSample(resId);
	if(aSample)
	{	p = *aSample;
		if(p->refCount++ == 0)
		{	HLock((Handle)aSample);
		}
	}

	return aSample;
}

void	CSoundHub::FreeUnusedSamples()
{
	SampleHeaderHandle	aSample, nextSample, *prevP;
	
	prevP = &sampleList;
	
	aSample = sampleList;
	while(aSample)
	{
		nextSample = (*aSample)->nextSample;
		if((*aSample)->refCount == 0)
		{	GetHandleSize((Handle)aSample);
			DisposeHandle((Handle)aSample);
			*prevP = nextSample;
		}
		else
		{	prevP = &(*aSample)->nextSample;
		}
		aSample = nextSample;
	}
}

void	CSoundHub::FreeOldSamples()
{
	SampleHeaderHandle	aSample, nextSample, *prevP;
	
	prevP = &sampleList;
	
	aSample = sampleList;
	while(aSample)
	{	nextSample = (*aSample)->nextSample;
		if((*aSample)->refCount == 0 && ((*aSample)->flags & kOldSampleFlag))
		{	GetHandleSize((Handle)aSample);
			DisposeHandle((Handle)aSample);
			*prevP = nextSample;
		}
		else
		{	prevP = &(*aSample)->nextSample;
		}
		aSample = nextSample;
	}
}
void	CSoundHub::FlagOldSamples()
{
	SampleHeaderHandle	aSample;
	
	aSample = sampleList;
	while(aSample)
	{	(*aSample)->flags |= kOldSampleFlag;
		aSample = (*aSample)->nextSample;
	}
}

void	CSoundHub::Dispose()
{
	CBasicSound			*aSound, *nextSound;
	SampleHeaderHandle	aSample, nextSample;
	short			i;

	if(itsMixer)
	{	itsMixer->Dispose();
	}
	
	for(i=0;i<hubSoundKinds;i++)
	{	aSound = soundList[i];
		while(aSound)
		{	nextSound = aSound->nextSound;
			aSound->Dispose();
			aSound = nextSound;
		}
	}

	aSample = sampleList;
	while(aSample)
	{	nextSample = (*aSample)->nextSample;
		DisposeHandle((Handle)aSample);
		aSample = nextSample;
	}

	itsCompressor->Dispose();
	DisposeSoundLinks();
	inherited::Dispose();
}


CBasicSound *CSoundHub::GetSoundSampler(
	short	kind,
	short	resId)
{
	CBasicSound			*aSound;
	SampleHeaderHandle	theSamples;
	
	aSound = Aquire(kind);
	if(aSound)
	{	aSound->itsMixer = itsMixer;
		
		if(muteFlag)	theSamples = NULL;
		else			theSamples = RequestSample(resId);
		aSound->UseSamples(theSamples);
		aSound->SetRate(FIX(1));
	}
	
	return aSound;
}

void	CSoundHub::CreateSoundLinks(
	short	n)
{
	Ptr			newStorage;
	SoundLink	*linkP;

	newStorage = NewPtr(sizeof(Ptr) + sizeof(SoundLink) * n);
	if(newStorage)
	{	*(Ptr *)newStorage = soundLinkStorage;
		soundLinkStorage = newStorage;
		linkP = (SoundLink *)(newStorage+sizeof(Ptr));

		while(n--)
		{	*(SoundLink **)linkP = firstFreeLink;
			firstFreeLink = linkP;
			linkP++;
		}
	}
}

void	CSoundHub::DisposeSoundLinks()
{
	while(soundLinkStorage)
	{	Ptr		temp;
	
		temp = *(Ptr *)soundLinkStorage;
		DisposePtr(soundLinkStorage);
		soundLinkStorage = temp;
	}
}

long	CSoundHub::ReadTime()
{
	return itsMixer->ReadTime();
}

void	CSoundHub::SetMixerLink(
	SoundLink	*newLink)
{
	SoundLink	*oldLink;	
	
	oldLink = itsMixer->motionLink;

	if(newLink != oldLink)
	{
		if(newLink)	newLink->refCount++;

		itsMixer->altLink = newLink;
		itsMixer->useAltLink = true;

		itsMixer->motionLink = newLink;
		if(oldLink)	ReleaseLink(oldLink);
		itsMixer->useAltLink = false;
		itsMixer->motionHub = this;
	}
}

SoundLink *	CSoundHub::UpdateRightVector(
	Fixed	*right)
{
	itsMixer->UpdateRightVector(right);
	
	return itsMixer->motionLink;
}

SoundLink *	CSoundHub::GetSoundLink()
{
	SoundLink	*theLink;
	
	theLink = firstFreeLink;
	if(theLink == 0)
	{	CreateSoundLinks(EXTRASOUNDLINKCOUNT);
		theLink = firstFreeLink;
	}
	
	if(theLink)
	{	firstFreeLink = *(SoundLink **)theLink;
		theLink->refCount = 1;
		theLink->meta = metaNoData;
	}
	
	return theLink;
}

void	CSoundHub::ReleaseLink(SoundLink *linkPtr)
{
	linkPtr->refCount--;
	if(linkPtr->refCount == 0)
	{	*(SoundLink **)linkPtr = firstFreeLink;
		firstFreeLink = linkPtr;
	}
}

void	CSoundHub::ReleaseLinkAndKillSounds(
	SoundLink *linkPtr)
{
	linkPtr->refCount--;
	if(linkPtr->refCount > 0)
	{	linkPtr->meta = metaKillNow;
	}
	else
	{	*(SoundLink **)linkPtr = firstFreeLink;
		firstFreeLink = linkPtr;
	}
}

void	CSoundHub::HouseKeep()
{
	itsMixer->HouseKeep();
}

void	UpdateSoundLink(
register	SoundLink		*theLink,
register	Fixed			*s,
register	Fixed			*v,
	unsigned long	t)
{
	theLink->meta = metaNoData;

	theLink->nLoc[0] = *s++;
	theLink->nLoc[1] = *s++;
	theLink->nLoc[2] = *s++;

	theLink->nSpeed[0] = *v++;
	theLink->nSpeed[1] = *v++;
	theLink->nSpeed[2] = *v++;
	
	theLink->t = t;
	
	theLink->meta = metaNewData;
}

void	ZeroSoundLink(
register	SoundLink		*theLink)
{
	theLink->meta = metaNoData;

	theLink->nLoc[0] = 0;
	theLink->nLoc[1] = 0;
	theLink->nLoc[2] = 0;

	theLink->nSpeed[0] = 0;
	theLink->nSpeed[1] = 0;
	theLink->nSpeed[2] = 0;
	
	theLink->t = 0;
	
	theLink->meta = metaNewData;
}

void	PlaceSoundLink(
register	SoundLink	*theLink,
register	Fixed		*s)
{
	theLink->meta = metaNoData;

	theLink->nLoc[0] = *s++;
	theLink->nLoc[1] = *s++;
	theLink->nLoc[2] = *s++;

	theLink->nSpeed[0] = 0;
	theLink->nSpeed[1] = 0;
	theLink->nSpeed[2] = 0;
	
	theLink->t = 0;
	
	theLink->meta = metaNewData;
}