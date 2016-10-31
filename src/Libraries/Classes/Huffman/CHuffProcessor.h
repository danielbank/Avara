/*
    Copyright �1995, Juri Munkki
    All rights reserved.

    File: CHuffProcessor.h
    Created: Thursday, August 31, 1995, 2:10
    Modified: Thursday, August 31, 1995, 20:58
*/

#pragma once
#include "CAbstractHuffPipe.h"

typedef struct
{
	long	decodedSize;
	long	countBitmap[NUMSYMBOLS>>5];
} HuffDataHeader;

#define	HUFFHANDLELOOKUPBITS		10
#define	HUFFHANDLELOOKUPSIZE		(1<<HUFFHANDLELOOKUPBITS)

class	CHuffProcessor : public CAbstractHuffPipe
{
public:
			Ptr				outPointer;
			long			outCount;

			HuffTreeNode	**lookupBuf;
			Boolean			singleSymbolData;
			unsigned char	theSingleSymbol;
			long			codeStrings[NUMSYMBOLS];
			short			codeLengths[NUMSYMBOLS];

	virtual	void	CreateLookupBuffer();
	virtual	void	CreateSymbolTable();
	virtual	void	DecodeAll(unsigned char *source, unsigned char *dest);

	virtual	long	GetUncompressedLen(Ptr compressedData);
	virtual	long	GetCompressedLen(Ptr sourceData, long sourceLen);
	
	virtual	void	Compress(Ptr fromPtr, Ptr destPtr);
	virtual	void	Uncompress(Ptr fromPtr, Ptr destPtr);
};