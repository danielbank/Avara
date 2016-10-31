/*
    Copyright �1995, Juri Munkki
    All rights reserved.

    File: CCompactTagBase.c
    Created: Thursday, June 1, 1995, 1:28
    Modified: Tuesday, November 7, 1995, 18:12
*/

#include "CCompactTagBase.h"
#include "CHuffmanHandler.h"

Handle	CCompactTagBase::ConvertToHandle()
{
	Handle			raw, compressed;
	CHuffmanHandler	*theCompressor;	
	
	compressed = NULL;
	raw = inherited::ConvertToHandle();
	if(raw)
	{
		theCompressor = new CHuffmanHandler;
		theCompressor->Open();
		compressed = theCompressor->Compress(raw);
		DisposeHandle(raw);
		theCompressor->Dispose();
		
	}

	return compressed;	
}

void	CCompactTagBase::ConvertFromHandle(
	Handle theHandle)
{
	Handle			raw;
	CHuffmanHandler	*theCompressor;	
	
	if(theHandle)
	{	theCompressor = new CHuffmanHandler;
		theCompressor->Open();
		raw = theCompressor->Uncompress(theHandle);
		theCompressor->Dispose();
		
		inherited::ConvertFromHandle(raw);
		
		DisposeHandle(raw);
	}
}