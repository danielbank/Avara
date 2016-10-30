/*/
    Copyright �1994-1996, Juri Munkki
    All rights reserved.

    File: CAvaraGame.h
    Created: Sunday, November 20, 1994, 19:01
    Modified: Sunday, September 15, 1996, 21:09
/*/

#pragma once
#include "CGameTimer.h"
#include "PAKit.h"
#include "AvaraTypes.h"
#include "AvaraDefines.h"
#include "AvaraScoreInterface.h"

#define	IDENTTABLESIZE			512

#define	MESSAGEHASH				128
#define	LOCATORTABLEBITS		6
#define	LOCATORTABLESIZE		(1L << (2*LOCATORTABLEBITS))
#define	LOCATORRECTSCALE		(16+5)	/*	Each locator square is 32 x 32 metres */
#define	LOCATORRECTSIZE			(1L<<LOCATORRECTSCALE)
#define	LOCCOORDMASK			(((1L << LOCATORTABLEBITS)-1)<<LOCATORRECTSCALE)
#define	LOCCOORDTOINDEX(coord)	(((coord) & LOCCOORDMASK) >> LOCATORRECTSCALE)
#define	LOCTOTABLE(x,z)			((((x) & LOCCOORDMASK) >> (LOCATORRECTSCALE-LOCATORTABLEBITS)) \
								| (((z) & LOCCOORDMASK) >> LOCATORRECTSCALE))

#define	kJoystickMode			1
#define	kFlipAxis				2
#define	kSimulChatMode			4

#define	kAmbientSoundToggle		1
#define	kTuijaToggle			2
#define	kInterpolateToggle		4
#define	kMissileLoopToggle		8
#define	kFootStepToggle			16
#define	kMusicToggle			32

enum	{	kPlayingStatus, kAbortStatus, kReadyStatus, kPauseStatus, kNoVehicleStatus, kWinStatus, kLoseStatus	};

class	CAbstractActor;
class	CAbstractPlayer;
class	CPlayerManager;

class	CBSPWorld;
class	CBSPPart;
class	CViewParameters;

class	CStatusDisplay;
class	CInfoPanel;
class	CDepot;

class	CAvaraApp;

class	CSoundHub;
class	CNetManager;
class	CIncarnator;
class	CWorldShader;
class	CScoreKeeper;
class	CAbstractYon;

class	CAvaraGame : public CGameTimer
{
public:
	OSType			loadedTag;
	OSType			loadedDirectory;
	Str63			loadedLevel;
	Str255			loadedDesigner;
	Str255			loadedInfo;
	long			loadedTimeLimit;
	long			timeInSeconds;
	long			frameNumber;
	
	long			topSentFrame;
	
	long			frameTime;		//	In milliseconds.
	short			gameStatus;
	short			statusRequest;
	short			pausePlayer;
	
	CAbstractActor	*actorList;
	CAbstractActor	*nextActor;

	CAbstractPlayer	*playerList;
	CAbstractPlayer	*nextPlayer;
	long			playersStanding;
	short			teamsStandingMask;
	short			teamsStanding;

	CIncarnator		*incarnatorList;
	CAbstractPlayer	*freshPlayerList;
	
	CAbstractActor	*postMortemList;
	
	ActorLocator	**locatorTable;
	ActorLocator	locatorListEnd;
	unsigned long	searchCount;
	
	Fixed			friendlyHitMultiplier;
	long			scores[kMaxAvaraPlayers];

	long			curIdent;
	CAbstractActor	*identTable[IDENTTABLESIZE];

	CAvaraApp		*itsApp;
	WindowPtr		itsWindow;
	PolyWorld		itsPolyWorld;
	CBSPWorld		*itsWorld;
	CBSPWorld		*hudWorld;
	CViewParameters	*itsView;
	CAbstractYon	*yonList;
	
	Rect			gameRect;
	
	MessageRecord	*messageBoard[MESSAGEHASH];
	Fixed			*baseLocation;
	
	Fixed			gravityRatio;

	Fixed			groundTraction;
	Fixed			groundFriction;
	
	//	Object oriented submanagers:
	
	CInfoPanel		*infoPanel;
	CDepot			*itsDepot;			//	Storage maintenance for ammo
	CSoundHub		*soundHub;			//	Sound playback and control hub
	CNetManager		*itsNet;			//	Networking management
	CWorldShader	*worldShader;		//	Manages ground and sky colors.
	CScoreKeeper	*scoreKeeper;

	//	Sound related variables:
	long			soundTime;
	short			soundOutputStyle;	//	Mono, speakers stereo, headphones stereo
	short			sound16BitStyle;	//	true = try 16 bit, false = 8 bit 
	short			soundSwitches;		//	kAmbientSoundToggle & kTuijaToggle
	short			groundStepSound;

	//	Networking & user control related stuff:
	Handle			mapRes;						//	Keyboard mapping resource handle.

	short			moJoOptions;				//	Mouse and Joystick options.
	short			sensitivity;

	long			latencyTolerance;
	long			waitCount;

	ScoreInterfaceReasons	scoreReason;
	ScoreInterfaceReasons	lastReason;

	long			nextScheduledFrame;
	Boolean			canPreSend;

	Boolean			didWait;
	Boolean			longWait;
	Boolean			veryLongWait;
	Boolean			allowBackgroundProcessing;
	Boolean			simpleExplosions;

	Boolean			resetMouse;

	//	Methods:
	virtual	void	IAvaraGame(WindowPtr theWindow, CAvaraApp *theApp);
	
	virtual	void	InitLocatorTable();

	virtual	CAbstractActor *
					FindIdent(long ident);
	virtual	void	GetIdent(CAbstractActor *theActor);
	virtual	void	RemoveIdent(long ident);
	
	virtual	CAbstractPlayer	*
					GetLocalPlayer();

	virtual	void	AddActor(CAbstractActor *theActor);
	virtual	void	RemoveActor(CAbstractActor *theActor);

	virtual	void	AmbientHologramControl();

	virtual	void	ResumeActors();
	virtual	void	PauseActors();
	virtual	void	RunFrameActions();
	
	virtual	void	Score(short team, short player, long points, Fixed energy, short hitTeam, short hitPlayer);

	virtual	void	ChangeDirectoryFile();
	virtual	void	LevelReset(Boolean clearReset);
	virtual	void	EndScript();
	
	virtual	void	ReadGamePrefs();
	
	virtual	void	SendStartCommand();
	virtual	void	ResumeGame();
	virtual	void	GameLoop();
	virtual	void	Dispose();
	
	virtual	void	ResetView();
	
	virtual	void	RegisterReceiver(MessageRecord *theMsg, MsgType messageNum);
	virtual	void	RemoveReceiver(MessageRecord *theMsg);
	virtual	void	FlagMessage(MsgType messageNum);
	virtual	void	FlagImmediateMessage(MsgType messageNum);
	virtual	void	MessageCleanup(CAbstractActor *deadActor);
	
	virtual	void	CalcGameRect();
	virtual	void	StopGame();
	virtual	void	RefreshWindow();
	virtual	void	DrawGameWindowContents();
	
	virtual	void	InitMixer(Boolean silentFlag);
	virtual	void	GameOptionCommand(long theCommand);
	
	virtual	CPlayerManager *
					GetPlayerManager(CAbstractPlayer *thePlayer);
	
	virtual	void	GotEvent();
};

#ifndef MAINAVARAGAME
extern	CAvaraGame	*gCurrentGame;
extern	CSoundHub	*gHub;
#else
CAvaraGame	*gCurrentGame = NULL;
CSoundHub	*gHub = NULL;
#endif
