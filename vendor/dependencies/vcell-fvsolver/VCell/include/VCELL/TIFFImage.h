/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */
#ifndef TIFFIMAGE_H
#define TIFFIMAGE_H

#include <VCELL/ilTIFF.h>

typedef enum {
	ByteOrderPC, 
	ByteOrderUnix
} ByteOrder;

typedef struct {
	char name[50];
	int  id;
} TagID;

typedef struct {
	char name[50];
	int  id;
} TypeID;

class TIFFImage;

class Tag
{
public:
	Tag(TIFFImage *tiffImage);
	~Tag();
	boolean   read(FILE *fp);
	
	char         *getTagName() { return _tagName; }
	int           getTagID()   { return _tagType; }
	char         *getDataName() { return _typeName; }
	TIFFDataType  getDataType() { return _dataType; }
	
	int           getLength() { return _length; }
	double        getFirstValue();
	
	char         *getValueString();
	int           getValue(char **value);
	int           getValue(unsigned short **value);
	int           getValue(unsigned long **value);
	Tag           *_next;
protected:
	void          parseTagType(int tag);
	void          parseDataType(int Type);

	unsigned short _tag;
	unsigned short _entryType;
	unsigned long  _entryOffset;
	unsigned long  _valueOffset;

	TIFFImage        *_tiffImage;
	char            _tagName[200];
	char            _typeName[200];
	int             _tagType;
	TIFFDataType    _dataType;
	unsigned long   _length;
	unsigned long  *_pRational;
	double         *_pDouble;
	float          *_pFloat;
	unsigned long  *_pLong;
	unsigned short *_pShort;
	char           *_pChar;
};

class TIFFImage 
{
public:
	TIFFImage();
	~TIFFImage();
	boolean read(char *filename);
	boolean write(char *filename=NULL);  

	long getSizeX() { return _width; }
	long getSizeY() { return _height; }
	long getSizeZ() { return _depth; }
	boolean copyData(unsigned char *data, long size);
    
protected:
	boolean read(unsigned short *ushort);
	boolean read(unsigned long *ulong, 
					unsigned short *ushort=NULL, 
			unsigned char *uchar=NULL);
	long    readIFD(long offset);
	Tag     *getTag(int tagID);
	boolean parseTags();
	boolean verifyDepth();
	boolean readStrips();
	boolean readTiles();

	Tag                *_tagHead;

	FILE               *_fp;
	void               *_pixels;
	ByteOrder           _byteOrder;
	unsigned long      *_pStripByteCounts;
	unsigned long      *_pStripOffsets;
	long                _numStrips;
	unsigned long      *_pTileByteCounts;
	unsigned long      *_pTileOffsets;
	long                _numTiles;
	long                _width;
	long                _height;
	long                _depth;
	long                _widthTile;
	long                _heightTile;
	long                _depthTile;
	long                _bitspersample;
	long                _compression;
	// unsigned long       _rowsPerStrip;
	// unsigned long       _stripOffsets[];

	friend class Tag;
};

#endif
