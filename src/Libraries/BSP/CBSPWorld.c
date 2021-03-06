/*
    Copyright �1994-1996, Juri Munkki
    All rights reserved.

    File: CBSPWorld.c
    Created: Saturday, July 16, 1994, 23:14
    Modified: Tuesday, February 20, 1996, 7:39
*/

#include "CBSPWorld.h"
#include "CBSPPart.h"
#include "CViewParameters.h"
#include "BSPPlug.h"
#include "PowerPlugs.h"

static	DrawPolygonsListPlug	*bspDrawPolygonsList = NULL;
static	PreProcessListPlug		*bspPreProcessList = NULL;

extern	Vector				**bspPointTemp;
extern	short				*bspIndexStack;


void	CBSPWorld::IBSPWorld(
	short	initialObjectSpace)
{
	partSpace = initialObjectSpace;
	partCount = 0;
	
	partList = (CBSPPart ***)NewHandle(sizeof(CBSPPart *) * (long) partSpace);
	visibleList = (CBSPPart ***)NewHandle(sizeof(CBSPPart *) * (long) partSpace);
	
	bspPreProcessList = GetPowerRoutine(NULL, PAKITPOWERPLUGID, kBSPPreProcessList);
	bspDrawPolygonsList = GetPowerRoutine(NULL, PAKITPOWERPLUGID, kBSPDrawPolygonsList);
}

void	CBSPWorld::DisposeParts()
{
	short	i;
	
	for(i=0;i<partCount;i++)
	{	(*partList)[i]->Dispose();
	}
	
	partCount = 0;
}

void	CBSPWorld::Dispose()
{
	DisposeParts();

	DisposeHandle((Handle) partList);
	DisposeHandle((Handle) visibleList);
	
	inherited::Dispose();
}

void	CBSPWorld::AddPart(
	CBSPPart	*thePart)
{
	if(partSpace <= partCount)
	{	partSpace = partCount+(partCount>>3)+1;

		SetHandleSize((Handle)partList, sizeof(CBSPPart *) * (long) partSpace);
		SetHandleSize((Handle)visibleList, sizeof(CBSPPart *) * (long) partSpace);
	}
	
	thePart->worldIndex = partCount;
	(*partList)[partCount++] = thePart;
}

void	CBSPWorld::RemovePart(
	CBSPPart	*thePart)
{
	short		i;
	CBSPPart	**p;
	
	p = *partList;
	
	i = thePart->worldIndex;
	if(i >= 0 && i < partCount && p[i] == thePart)
	{	partCount--;
		p[i] = p[partCount];
	}
	else
	{	for(i=0;i<partCount;i++)
		{	if(*p == thePart)
			{
				partCount--;
				p[0] = (*partList)[partCount];
				break;
			}
			else
				p++;
		}
	}
}

void	CBSPWorld::SortByZ()
{
	short		h;
	CBSPPart	**ip, **jp, *vp;
	CBSPPart	**lastP;
	CBSPPart	**hp;
	Fixed		vz;

	h = 1;
	lastP = visibleP + visibleCount;
	
	do
	{	h += h+h+1;
	} while(h <= visibleCount);
	
	do
	{	h /= 3;
		hp = visibleP + h;
		
		for(ip = hp; ip < lastP; ip++)
		{	vp = *ip;
			vz = vp->maxZ;

			jp = ip;

			while((jp[-h])->maxZ < vz)
			{	
				*jp = jp[-h];
				jp -= h;
				
				if(jp < hp) break;
			}
			
			*jp = vp;
		}
	} while(h != 1);
}

/*
**	This is the old version. It doesn't work correctly in all cases.
*/
void	CBSPWorld::ScoreAndSort(CBSPPart **firstPart, short overlapCount)
{
	CBSPPart	*compPart;
	CBSPPart	**endPart, **jPart, **iPart;

	endPart = firstPart+overlapCount;

	iPart = firstPart;

	while(iPart<endPart)
	{	(*iPart++)->worldFlag = false;
	}

	for(iPart = firstPart; iPart < endPart; iPart++)
	{	compPart = *iPart;

		if(!compPart->worldFlag)
		for(jPart = iPart+1; jPart < endPart; jPart++)
		{	if(compPart->Obscures(*jPart))
			{	CBSPPart	**kPart;

				compPart = *jPart;
				kPart = jPart;
				while(kPart > iPart)
				{	kPart[0] = kPart[-1];
					kPart--;
				}
				
				*iPart = compPart;
				compPart->worldFlag = true;
			}
		}
	}
}

/*
**	Visibility sort "overlapCount" objects. The number of objects
**	is guaranteed to be at least 2.
*/

#define	FASTOBSCURETEST(a,b)	(b->maxZ > a->minZ)
static	long	totalObsCount = 0;

void	CBSPWorld::VisibilitySort(CBSPPart **parts, short overlapCount)
{
				CBSPPart	*thePart;
				CBSPPart	*listStart;
				CBSPPart	**prevLink;

	listStart = NULL;

	do
	{	thePart = *parts++;
		thePart->worldFlag = true;
		thePart->nextTemp = listStart;

		listStart = thePart;
	} while(--overlapCount);

	do
	{	register	Fixed	maxZ;
newListStart:
		maxZ = listStart->maxZ;
		prevLink = &listStart->nextTemp;
		thePart = *prevLink;
	
		while(thePart)
		{	//	The clean way would be to call the Obscures method and do all the work
			//	there. Unfortunately this is very costly because the stack is involved...
			if(
				maxZ			> thePart->minZ &&
				thePart->worldFlag				&&
				listStart->maxX > thePart->minX &&
				listStart->minX < thePart->maxX &&
				listStart->maxY > thePart->minY &&
				listStart->minY < thePart->maxY &&
				thePart->Obscures(listStart))
			{	*prevLink = thePart->nextTemp;	//	Unlink current part
				thePart->nextTemp = listStart;
				listStart->worldFlag = false;
				listStart = thePart;
				goto newListStart;
			}
			else
			{	prevLink = &thePart->nextTemp;
				thePart = *prevLink;
			}
		}
		
		*--parts = listStart;
		listStart = listStart->nextTemp;
	} while(listStart);
}

void	CBSPWorld::SortVisibleParts()
{
	CBSPPart	**thisPart, **endPart;
	Fixed		minZ, maxZ;
	short		overlapCount = 0;
				
	SortByZ();

	endPart = visibleP + visibleCount;
	thisPart = visibleP;
	minZ = maxZ = currentView->yonBound;
	
	while(thisPart < endPart)
	{	Fixed	z;
	
		z = (*thisPart)->maxZ;
		
		if(z > minZ)
		{	overlapCount++;
			z = (*thisPart)->minZ;
			if(z < minZ)	minZ = z;
		}
		else
		{
			if(overlapCount > 1)
			{	VisibilitySort(thisPart-overlapCount, overlapCount);
			}
			maxZ = z;
			minZ = (*thisPart)->minZ;
			overlapCount = 1;
		}
	
		thisPart++;
	}

	if(overlapCount > 1)
	{	VisibilitySort(thisPart-overlapCount, overlapCount);
	}
}

void	CBSPWorld::Render(
	CViewParameters	*theView)
{
	short		i;
	CBSPPart	**sp, **sd;
	
	currentView = theView;
	theView->DoLighting();

	HLock((Handle) partList);
	HLock((Handle) visibleList);
	
	visibleP = *visibleList;
	sd = visibleP;
	sp = *partList;
	visibleCount = 0;

	for(i=0;i<partCount;i++)
	{	
		if((*sp)->PrepareForRenderOne(theView))
		{	*sd++ = *sp;
			visibleCount++;
		}
		
		sp++;
	}

	if(bspPreProcessList)
	{	PreProcessListParam		param;
		param.visibleObjectArray = visibleP;
		param.visibleObjectCount = visibleCount;

		bspPreProcessList(&param);

		visibleP = param.visibleObjectArray;
		visibleCount = param.visibleObjectCount;
	}
	else
	{	sd = visibleP;
		sp = sd;
		i = visibleCount;
		visibleCount = 0;
		while(i--)
		{	
			if((*sp)->PrepareForRenderTwo())
			{	*sd++ = *sp;
				visibleCount++;
			}
			
			sp++;
		}
	}

	SortVisibleParts();
	
	if(bspDrawPolygonsList)
	{	DrawPolygonsParam	param;

		param.polys = GetPolyWorld();
		param.bspIndexStack = bspIndexStack;
		param.bspPointTemp = bspPointTemp;
		param.visibleObjectArray = visibleP;
		param.visibleObjectCount = visibleCount;

		bspDrawPolygonsList(&param);
	}
	else
	{	sd = visibleP;
		for(i=0;i<visibleCount;i++)
		{	(*sd)->DrawPolygons();
			sd++;
		}
	}

	sd = visibleP;
	for(i=0;i<visibleCount;i++)
	{	(*sd)->PostRender();
		sd++;
	}
	
	HUnlock((Handle) partList);
	HUnlock((Handle) visibleList);
}

CBSPPart *
	CBSPWorld::GetIndPart(
		short	ind)
{
	if(ind >= 0 && ind < partCount)
		return (*partList)[ind];
	else
		return NULL;
}

short	CBSPWorld::GetPartCount()
{
	return partCount;
}