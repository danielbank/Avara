/*
    Copyright �1994-1996, Juri Munkki
    All rights reserved.

    File: CSoundMixer.h
    Created: Friday, December 23, 1994, 8:24
    Modified: Saturday, February 24, 1996, 9:13
*/

#pragma once
#include "CDirectObject.h"
#include "CExitHandler.h"
#include <Sound.h>
#include "SoundSystemDefines.h"
#include "FastMat.h"

#define	CYCLEMILLISECS	1000L
//(5/*hours*/ * 60/*minutes*/ * 60L/*seconds*/ * 1000L /*milli*/)

class	CBasicSound;
class	CSoundMixer;
class	CSoundHub;

typedef struct
{
	CBasicSound		*active;
	CBasicSound		*intro;
	CBasicSound		*introBackup;
	CBasicSound		*release;
	short			volume;
	short			isFree;
} MixerInfo;

typedef struct
{
	Vector			loc;		//	Position at t = 0
	Vector			speed;		//	Linear motion
} SoundMotionInfo;

class	CSoundMixer : public CDirectObject
{
public:
			long				globRegister;
			long				volumeMax;	
			unsigned long		frameCounter;

			SndDoubleBufferHeader	doubleHeader;
			SndDoubleBufferPtr		currentBuffer;
			SndDoubleBufferPtr		doubleBuffers[3];

			SndChannelPtr		itsChannel;
			ExitRecord			itsExit;

			WordSample			*mixTo[2];
			WordSample			*mixBuffers[2];
			SampleConvert		*volumeLookup;
			Ptr					scaleLookup;
			Ptr					scaleLookupZero;

			MixerInfo			dummyChannel;
			MixerInfo			*infoTable;
			MixerInfo			**sortSpace[2];

			ComponentInstance	timerInstance;
			Int64Bit			baseTime;
			
			long				frameTime;
			long				frameStartTime;
			long				frameEndTime;

			UnsignedFixed		samplingRate;
			UnsignedFixed		standardRate;
			Fixed				timeConversion;
			Fixed				soundSpeed;
			Fixed				distanceToSamples;
			Fixed				distanceAdjust;
			Fixed				distanceToLevelOne;
			Fixed				maxAdjustedVolume;

			CSoundHub			*motionHub;
			SoundMotionInfo		motion;
			SoundLink			*motionLink;
			SoundLink			*altLink;
			Boolean				useAltLink;

			char				newRightMeta;
			Vector				newRight;

			Vector				rightVector;

			short				minimumVolume;
			short				maxChannels;
			short				maxMix;
			
			short				soundBufferSize;
			short				soundBufferBits;
			
			Boolean				hushFlag;
			Boolean				stopNow;
			Boolean				sample16flag;
			Boolean				stereo;
			Boolean				strongStereo;
			
	virtual	void		ISoundMixer(Fixed sampRate, short maxChannelCount, short maxMixCount,
									Boolean stereoEnable, Boolean sample16Enable,
									Boolean	interpolateEnable);

	virtual	void		SetSoundEnvironment(Fixed speedOfSound, Fixed distanceToLevelOne, long timeUnit);
	virtual	void		SetStereoSeparation(Boolean strongFlag);

	virtual	void		StartTiming();
	virtual	long		ReadTime();
	
	virtual	void		UpdateRightVector(Fixed *right);
	
	virtual	void		PrepareScaleLookup();
	virtual	void		PrepareVolumeLookup();
	virtual	void		SilenceBuffers();
	virtual	void		UpdateMotion();
	virtual	void		DoubleBack(SndDoubleBufferPtr theBuffer);
	virtual	void		PostMix(SndDoubleBufferPtr theBuffer);

	virtual	void		Dispose();
	virtual	void		HouseKeep();
	virtual	void		AddSound(CBasicSound *theSound);
};

void	InitDopplerSoundSystem();
