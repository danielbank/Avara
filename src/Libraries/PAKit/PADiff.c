/*
    Copyright �1992-1996, Juri Munkki
    All rights reserved.

    File: PADiff.c
    Created: Monday, November 23, 1992, 16:44
    Modified: Tuesday, January 2, 1996, 9:42
*/

#include "PAKit.h"

extern	PolyWorld		*thePolyWorld;

void	DiffPolyRegions68K(
	PolyWorld	*polys,
	short		*oldScreen,
	short		*newScreen,
	short		*diffScreen,
	short		rowCount)
{
	short	*emergencyStop;

	emergencyStop = diffScreen + polys->deltaRegionSize - rowCount - 2*ACTIVELISTSIZE;

asm	{
	movem.l	D3-D7/A2-A4,-(sp)
	move.l	oldScreen,A0
	move.l	newScreen,A1
	move.l	diffScreen,A2

	move.l	thePolyWorld,A3
	move.w	OFFSET(PolyWorld,bounds.right)(A3),D7

	move.l	emergencyStop,A4
	move.w	rowCount,D6
	subq.w	#1,D6

@rowLoop
	move.w	OFFSET(PolyWorld,bounds.left)(A3),D0
	moveq.l	#-1,D1			;	oldC = whatever

@inSynch
	move.w	(A1)+,D3		;	read ncol
	move.w	(A1)+,D2		;	read nx
	
	move.w	(A0)+,D5		;	read ocol
	move.w	(A0)+,D4		;	read ox

@mainLoop
	cmp.w	D3,D5
	bne.s	@isDifferent

@isSame
	tst.w	D1
	bmi.s	@readMore

	move.w	D0,(A2)+		;	WriteSpan	x2
	moveq.l	#-1,D1
	move.w	D1,(A2)+		;	WriteSpan	color

	cmp.w	D2,D4
	beq.s	@readBoth
	blt.s	@readOldWasSame

@stillSame
	move.w	D2,D0
	move.w	(A1)+,D3
	move.w	(A1)+,D2

@mainLoopWasSame
	cmp.w	D3,D5
	bne.s	@isDifferent
	cmp.w	D2,D4
	beq.s	@readBoth
	blt.s	@readOldWasSame
	bra.s	@stillSame

@readOldWasSame
	move.w	(A0)+,D5
	move.w	D4,D0
	move.w	(A0)+,D4
	bra.s	@mainLoopWasSame

@readOld
	move.w	(A0)+,D5
	move.w	D4,D0
	move.w	(A0)+,D4
	cmp.w	D3,D5
	beq.s	@isSame

@isDifferent
	cmp.w	D3,D1
	beq.s	@readMore

	move.w	D0,(A2)+		;	WriteSpan	x2
	move.w	D3,D1
	move.w	D3,(A2)+		;	WriteSpan	color

@readMore
	cmp.w	D2,D4
	beq.s	@readBoth
	blt.s	@readOld
@readNew
	move.w	D2,D0
	move.w	(A1)+,D3
	move.w	(A1)+,D2
	bra.s	@mainLoop
@readBoth
	move.w	D2,D0
	cmp.w	D7,D2
	blt.s	@inSynch

	move.w	D2,(A2)+
	
	cmp.l	A4,A2
	dbcc	D6,@rowLoop
	bcs.s	@stop

@outOfMem
	move.w	D7,(A2)+
	subq.w	#1,rowCount
	bne.s	@outOfMem
	
@stop	
	movem.l	(sp)+,D3-D7/A2-A4
	}
}
#if 0
static	long	minOldTime = 0x7fffFFFF;
static	long	maxOldTime;
static	long	oldTimer;
static	long	callTimes;

#define	THEDELAY	-60000000L

void	DiffPolyRegions68K(
	PolyWorld	*polys,
	short		*oldScreen,
	short		*newScreen,
	short		*diffScreen,
	short		rowCount)
{
	TMTask	miniTask;
	short	i;
	long	deltaTime;

	if(rowCount > 100)
		callTimes++;

	{	miniTask.tmAddr = NULL;
		miniTask.tmWakeUp = 0;
		miniTask.tmReserved = 0;
		InsXTime((QElemPtr)&miniTask);
		FlushDataCache();
		FlushInstructionCache();
		PrimeTime((QElemPtr)&miniTask, THEDELAY);
		DiffPolyRegions68KTest(polys, oldScreen, newScreen, diffScreen, rowCount);
		RmvTime((QElemPtr)&miniTask);
	
		if(rowCount > 100)
		{	deltaTime = miniTask.tmCount - THEDELAY;
	
			oldTimer += deltaTime;
	
			if(deltaTime < minOldTime)
			{	minOldTime = deltaTime;
			}
	
			if(deltaTime > maxOldTime)
			{	maxOldTime = deltaTime;
			}
		}
	}
}
#endif

short	*ExtractDiff(
	short	*source,
	short	*dest,
	short	skipLines,
	short	extractLines,
	short	left,
	short	right,
	short	oldRight)
{
asm	{
		movem.l	D3-D7,-(sp)
		move.l	source,A0
		move.l	dest,A1

		move.w	left,D0
		move.w	right,D1
		move.w	oldRight,D2

		move.w	skipLines,D3
		subq.w	#1,D3
		bmi.s	@noSkip
@skipper1
		move.w	(A0)+,D4
		cmp.w	D2,D4
		beq.s	@emptyLine
@skipper2					;	Skip lines before
		move.l	(A0)+,D4
		cmp.w	D2,D4
		bne.s	@skipper2
@emptyLine
		dbra	D3,@skipper1

@noSkip
		move.w	extractLines,D3
		subq.w	#1,D3
//		bmi.s	@noExtract	;	Should never happen...means there's no output (So I commented it out)
@forEachLine
		move.w	D0,D4		;	left edge of range
		moveq.l	#-1,D5
		move.w	(A0)+,D6	;	right edge of range (possibly)
		cmp.w	D1,D6		;	Was it the end?
		bge.s	@x2right	; Jump to end, if it was
@checkRange
		cmp.w	D6,D4		;	Is left > right
		blt.s	@okRange	;	If it is, then move on.
		move.w	(A0)+,D5	;	Read next color
		move.w	(A0)+,D6	;	Read next right
		bra.s	@checkRange	;	Check again
@okRange
		tst.w	D5
		bmi.s	@noOutput
@nextSpan
		move.w	D4,(A1)+
		move.w	D5,(A1)+

@noOutput
		cmp.w	D1,D6		;	Is right >= right boundary
		bge.s	@x2right	;	If it is, exit this loop

		move.w	D6,D4
		move.w	(A0)+,D5
		move.w	(A0)+,D6
		bra.s	@nextSpan

@clearingLoop
		move.l	(A0)+,D6
@x2right
		cmp.w	D6,D2
		bne.s	@clearingLoop		

@noClearing
		move.w	D1,(A1)+
		dbra	D3,@forEachLine
		
@noExtract
		move.l	A1,D0		;	Return value
		movem.l	(sp)+,D3-D7
	}	
}