/*
    Copyright �1994-1996, Juri Munkki
    All rights reserved.

    File: KeyFuncs.h
    Created: Tuesday, November 22, 1994, 5:14
    Modified: Friday, February 2, 1996, 10:41
*/

#pragma once

typedef struct
{
	long	down[2];
	long	up[2];
	long	held[2];
	
	Point	mouseDelta;
	Boolean	buttonStatus;
	char	msgChar;
} FunctionTable;

typedef struct
{
	FunctionTable	ft;
	long			validFrame;
}	FrameFunction;

//	Flags for "p1" of packet.
#define	kbuIsDown		1
#define	kbuWentDown		2
#define	kbuWentUp		4
#define	kNoDownData		0x80
#define	kNoUpData		0x40
#define	kNoHeldData		0x20
#define	kNoMouseV		0x10
#define	kNoMouseH		0x08
#define	kDataFlagMask	(kNoDownData | kNoUpData | kNoHeldData | kNoMouseH | kNoMouseV)

#define	FUNMAPID	8000
#define	FUNMAPTYPE	'FMAP'

#ifdef MORETHAN32FUNCS
#define	TESTFUNC(n, map)	(0 != ((n < 32) ? (map[0] & (0x80000000 >> n)) : (map[1] & (1 << (63-n)))))
#else
#define	TESTFUNC(n, map)	(0 != (map[0] & (0x80000000 >> n)))
#endif

enum
{
	kfuForward,
	kfuReverse,
	kfuLeft,
	kfuRight,
/*
	kfuLeftForward,
	kfuRightForward,
	kfuLeftReverse,
	kfuRightReverse,
*/
	kfuVerticalMotion,

	kfuFireWeapon,
	kfuLoadGrenade,
	kfuLoadMissile,
	kfuBoostEnergy,
	kfuTypeText,

	kfuScanDist,

	kfuZoomIn,
	kfuZoomOut,

	kfuScoutView,
	kfuScoutControl,
	
	kfuPauseGame,
	kfuAbortGame,
	
	kfuDebug1,
	kfuDebug2,

	kfuJump,
	kfuLookLeft,
	kfuLookRight,
	kfuAimForward,
	
	kKeyFuncCount
};

extern	Point			MTemp			: 0x828;
extern	Point			RawMouse		: 0x82c;
extern	Point			Mouse			: 0x830;
extern	short			CrsrNewCouple	: 0x8ce;
extern	Byte			CrsrNew			: 0x8ce;
extern	Byte			CrsrCouple		: 0x8cf;
extern	Byte			CrsrScale		: 0x8D3;