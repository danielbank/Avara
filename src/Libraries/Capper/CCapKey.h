/*
    Copyright �1994, Juri Munkki
    All rights reserved.

    File: CCapKey.h
    Created: Friday, November 11, 1994, 21:14
    Modified: Saturday, November 12, 1994, 19:21
*/

#pragma once

#include "CCapMaster.h"

#define	MAXLABELSPOTS	4
#define	MAXFUNCSPERKEY	8
#define	LABELMAXLEN		9

class	CCapKey : direct
{
public:
	class	CCapMaster	*itsMaster;
			short		virtualKeyCode;

			short		numLabelSpots;
			Point		labelSpots[MAXLABELSPOTS];
			RgnHandle	theKeyRegion;
			
			short		funcCount;
			char		funcList[MAXFUNCSPERKEY];
			
			char		label[LABELMAXLEN+1];
						
	virtual	void		ICapKey(short keyCode, CCapMaster *theMaster);
	virtual	void		ResetCap(void);
	virtual	void		SetKeyChar(char theChar);
	virtual	void		SetKeyString(StringPtr theStr);
	virtual	void		AddKeyArea(short x, short y, RgnHandle theRegion);
	virtual	void		OffsetKey(short x, short y);
	virtual	void		DrawKey();
	virtual	void		Dispose();
	virtual	void		AddFunc(CCapFunc *theFunc);
	virtual	void		RemoveFunc(CCapFunc *theFunc);
	virtual	void		DragFunc(CCapFunc *theFunc, Point where, RgnHandle dragRgn);
	virtual	void		ReadCaps(long *map);
	virtual	void		WriteCaps(long *map);
};