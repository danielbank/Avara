/*
    Copyright �1994-1995, Juri Munkki
    All rights reserved.

    File: LevelLoader.h
    Created: Sunday, November 20, 1994, 19:48
    Modified: Friday, September 8, 1995, 9:24
*/

#pragma once

enum	{
			iDesignerName,
			iLevelInformation,
			iTimeLimit,
			
			iGravity,
			iCustomGravity,

			iShape,
			iAltShape,
			iScale,
			iYon,
			iWallYon,
			iHasFloor,
			
			iWallTemplateResource,
			iFloorTemplateResource,

			iHeight,
			iMask,
			iTeam,
			iWallHeight,
			iWallAltitude,
			iBaseHeight,
			iPixelToThickness,
			iMass,
			iVisible,
			iThickness,
			iWinTeam,
			
			iTargetGroup,

			//	Sounds:
			iHitSound,
			iHitVolume,
			iHitSoundDefault,
			iShieldHitSoundDefault,
			iPlayerHitSoundDefault,
			iBlastSound,
			iBlastVolume,
			iBlastSoundDefault,
			
			iStepSound,
			iGroundStepSound,
			
			iIsTarget,

			iSound,
			iOpenSound,
			iCloseSound,
			iStopSound,
			iVolume,
			
			iDefaultLives,
			iLives,
			
			iIncarnateSound,
			iIncarnateVolume,
			
			iWinSound,
			iWinVolume,
			
			iLoseSound,
			iLoseVolume,
			
			//	Scoring
			iDestructScore,
			iHitScore,
			iFriendlyHitMultiplier,
			
			//	Explosions and damage:
			iSlivers0, iSlivers1, iSlivers2,
			iSliverL0, iSliverL1, iSliverL2,
			iCanGlow,

			//	Messages sent when hit/destroyed.
			iDestructMessage,
			iHitMessage,
			
			//	Other messages.
			iStepOnMessage,

			//	Motion
			iAccelerate,
			
			//	Toggle switches
			iTogglePower,
			iBlastToggle,
			
			//	Doors & such
			iOpenMsg,
			iCloseMsg,
			iDidOpen,
			iDidClose,
			iOpenDelay,
			iCloseDelay,
			iGuardDelay,
			iStatus,
			iOpenSpeed,
			iCloseSpeed,
			
			iPitch, iYaw, iRoll,
			iDeltaX, iDeltaY, iDeltaZ,

				//	Door2 specific:			
				iMidState,
				iMidPitch, iMidYaw, iMidRoll,
				iMidX, iMidY, iMidZ,

			// Misc
			iPower,
			iMaxPower,
			iDrain,

			//	Guns
			iFireMsg,
			iTrackMsg,
			iStopTrackMsg,
			iSpeed,
			iShotPower,
			
			//  Ufos
			iCheckPeriod,
			iAttack,
			iDefense,
			iVisionScore,
			iHideScore,
			iMotionRange,
			iPitchRange,
			iVerticalRangeMin,
			iVerticalRangeMax,
			iBurstLength,
			iBurstSpeed,
			iBurstCharge,
			
			iHomeSick,
			iHomeRange,
			iHomeBase,
			
			//	Mines
			iShieldEnergy,
			iActivateEnergy,
			iRange,
			iPhase,
			iActiveTimer,
			iBoom,

			iIdleShapeTimer,
			iIdleAltShapeTimer,
			iActiveShapeTimer,
			iActiveAltShapeTimer,

			iActivateSound,
			iActivateVolume,

			//	Teleporters:
			iGroup,
			iDestGroup,
			iSpinFlag,
			iFragmentFlag,
			iWin,
			iDeadRange,
			iShowAlways,
						
			//	Activators: (areas, etc.)
			iWatchMask,
			iFrequency,
			iEnter,
			iExit,
			
			//	Text
			iText,
			iShowEveryone,
			iAlignment,
			
			//	Goody
			iGrenades,
			iMissiles,
			iBoosters,
			iBoostTime,
			
			//	Ball & goal
			iGoalMsg,
			iGoalAction,
			iGoalScore,

			iEjectPitch,
			iEjectPower,
			iEjectSound,
			iEjectVolume,
			
			iShieldChargeRate,
			iMaxShield,
			iShootShield,
			iGrabShield,

			iCarryScore,			
			iDropEnergy,
			iChangeHolderPower,
			iChangeOwnerTime,
			
			iChangeOwnerSound,
			iChangeOwnerVolume,

			iSnapSound,
			iSnapVolume,
			
			//	Sound objects:
			iIsMusic,
			iIsAmbient,
			iIsPlaced,
			iRate,
			iLoopCount,
			iVolume0,
			iVolume1,
			iStartMsg,
			iStopMsg,
			iKillMsg,

			//	Logic:
			iInVar,
			iFirstIn,
			iLastIn = iFirstIn + 9,
			
			iOutVar,
			iFirstOut,
			iLastOut = iFirstOut + 9,
			
			iRestartFlag,
			iCount,
			iTimer,

			//	Lights
			iAmbient,
			iLightsTable,
			iLightsTableEnd = iLightsTable+11,	//	a 3 x 4 table
			
			// Advanced weapons
			iGrenadePower,
			iMissilePower,
			iMissileTurnRate,
			iMissileAcceleration,
			
			iMaxStartGrenades,
			iMaxStartMissiles,
			iMaxStartBoosts,
			
			//	Hulls
			iFirstHull,
			iSecondHull,
			iThirdHull,

			//	Traction/friction control:
			iDefaultTraction,
			iDefaultFriction,
			
			iWallTraction,
			iWallFriction,
			
			iTraction,
			iFriction,
			
			iWallShields,
			iWallPower,

			iVarInternalVariableCount
		};

double	ReadVariable(short index);
Fixed	ReadFixedVar(short index);
long	ReadLongVar(short index);
long	ReadColorVar(short index);
void	ReadStringVar(short index,StringPtr dest);

void	ProgramVariable(short index, double value);
void	ProgramFixedVar(short index, Fixed value);
void	ProgramLongVar(short index, long value);
void	ProgramReference(short index, short ref);
void	ProgramOffsetAdd(short index, short ref, long addValue);
void	ProgramOffsetMultiply(short index, short ref, long addValue);
void	ProgramMessage(short index, long value);

enum
{
	TextBegin =		150,	//	6	TTxtPicRec	Begin text function
	TextEnd	=		151,	//	0	NIL	End text function
	StringBegin	=	152,	//	0	NIL	Begin string delimitation
	StringEnd =		153		//	0	NIL	End string delimitation	
};

void	ConvertFileToLevelMap(StringPtr fileName);
void	ConvertResToLevelMap(short resId);
void	ConvertToLevelMap(PicHandle thePicture);
void	GetLastArcLocation(Fixed *theLoc);
Fixed	GetLastArcDirection();
Fixed	GetDome(Fixed *theLoc, Fixed *startAngle, Fixed *spanAngle);
long	GetPixelColor();
long	GetOtherPixelColor();
long	GetDecimalColor();

Fixed	GetLastOval(Fixed *theLoc);
