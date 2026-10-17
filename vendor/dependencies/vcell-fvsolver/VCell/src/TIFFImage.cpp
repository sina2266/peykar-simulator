/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */
#ifdef WIN32
#define _HAS_STD_BYTE 0
#include <Windows.h>
#else
#include <UnixDefs.h>
#endif

#include <stdio.h>
#include <string.h>
#include <assert.h>
//#include <il/ilFileImage.h>
//#include <il/ilGenericImageFile.h>
//#include <il/ilMemoryImage.h>
//#include <il/ilSize.h>
#include <VCELL/TIFFImage.h>

#define MAX_TYPE_IDS  13
#define MAX_TAG_IDS  81
#define DEFAULT_STRIP_OFFSET 8
#define max(a,b) (((a)>(b))?(a):(b))
#define min(a,b) (((a)<(b))?(a):(b))

TypeID typeID[MAX_TYPE_IDS] = {
    "NoType         ", TIFF_NOTYPE,
    "Byte           ", TIFF_BYTE,
    "ASCII          ", TIFF_ASCII,
    "Short          ", TIFF_SHORT,
    "Long           ", TIFF_LONG,
    "Rational       ", TIFF_RATIONAL,
    "Signed Char    ", TIFF_SBYTE,
    "Undefined 8-bit", TIFF_UNDEFINED,
    "Signed Short   ", TIFF_SSHORT,
    "Signed Long    ", TIFF_SLONG,
    "Signed Rational", TIFF_SRATIONAL,
    "Float          ", TIFF_FLOAT,
    "Double         ", TIFF_DOUBLE,
};

TagID tagID[MAX_TAG_IDS] = {
    "NewSubfileType           ",  TIFFTAG_SUBFILETYPE,
    "SubfileType              ",  TIFFTAG_OSUBFILETYPE,
    "ImageWidth               ",  TIFFTAG_IMAGEWIDTH,
    "ImageLength              ",  TIFFTAG_IMAGELENGTH,
    "BitsPerSample            ",  TIFFTAG_BITSPERSAMPLE,
    "Compression              ",  TIFFTAG_COMPRESSION,
    "PhotometricInterpretation",  TIFFTAG_PHOTOMETRIC,
    "Thresholding             ",  TIFFTAG_THRESHHOLDING,
    "CellWidth                ",  TIFFTAG_CELLWIDTH,
    "CellLength               ",  TIFFTAG_CELLLENGTH,
    "FillOrder                ",  TIFFTAG_FILLORDER, 
    "DocumentName             ",  TIFFTAG_DOCUMENTNAME,
    "ImageDescription         ",  TIFFTAG_IMAGEDESCRIPTION,
    "Make                     ",  TIFFTAG_MAKE,
    "Model                    ",  TIFFTAG_MODEL,
    "StripOffsets             ",  TIFFTAG_STRIPOFFSETS,
    "Orientation              ",  TIFFTAG_ORIENTATION,
    "SamplesPerPixel          ",  TIFFTAG_SAMPLESPERPIXEL,
    "RowsPerStrip             ",  TIFFTAG_ROWSPERSTRIP,
    "StripByteCounts          ",  TIFFTAG_STRIPBYTECOUNTS,
    "MinSampleValue           ",  TIFFTAG_MINSAMPLEVALUE,
    "MaxSampleValue           ",  TIFFTAG_MAXSAMPLEVALUE,
    "XResolution              ",  TIFFTAG_XRESOLUTION,
    "YResolution              ",  TIFFTAG_YRESOLUTION,
    "PlanarConfiguration      ",  TIFFTAG_PLANARCONFIG,
    "PageName                 ",  TIFFTAG_PAGENAME,
    "XPosition                ",  TIFFTAG_XPOSITION,
    "YPosition                ",  TIFFTAG_YPOSITION,
    "FreeOffsets              ",  TIFFTAG_FREEOFFSETS,
    "FreeByteCounts           ",  TIFFTAG_FREEBYTECOUNTS,
    "GrayResponseUnit         ",  TIFFTAG_GRAYRESPONSEUNIT,
    "GrayResponseCurve        ",  TIFFTAG_GRAYRESPONSECURVE,
    "Group3options            ",  TIFFTAG_GROUP3OPTIONS,
    "Group4options            ",  TIFFTAG_GROUP4OPTIONS,
    "ResolutionUnit           ",  TIFFTAG_RESOLUTIONUNIT,
    "PageNumber               ",  TIFFTAG_PAGENUMBER,
    "ColorResponseCurves      ",  TIFFTAG_COLORRESPONSEUNIT,
    "TransferFunction         ",  TIFFTAG_TRANSFERFUNCTION, 
    "Software                 ",  TIFFTAG_SOFTWARE,
    "DateTime                 ",  TIFFTAG_DATETIME,
    "Artist                   ",  TIFFTAG_ARTIST,
    "Hostcomputer             ",  TIFFTAG_HOSTCOMPUTER,
    "Predictor                ",  TIFFTAG_PREDICTOR,
    "Whitepoint               ",  TIFFTAG_WHITEPOINT,
    "PrimaryChromaticities    ",  TIFFTAG_PRIMARYCHROMATICITIES,
    "Colormap                 ",  TIFFTAG_COLORMAP,
    "HalftoneHints            ",  TIFFTAG_HALFTONEHINTS, 
    "TileWidth                ",  TIFFTAG_TILEWIDTH, 
    "TileLength               ",  TIFFTAG_TILELENGTH, 
    "TileOffsets              ",  TIFFTAG_TILEOFFSETS, 
    "TileByteCounts           ",  TIFFTAG_TILEBYTECOUNTS, 
    "BadFaxLines              ",  TIFFTAG_BADFAXLINES, 
    "CleanFaxData             ",  TIFFTAG_CLEANFAXDATA, 
    "ConsecutiveBadFaxLines   ",  TIFFTAG_CONSECUTIVEBADFAXLINES, 
    "BadFaxLines              ",  TIFFTAG_BADFAXLINES, 
    "InkSet                   ",  TIFFTAG_INKSET, 
    "InkNames                 ",  TIFFTAG_INKNAMES, 
    "DotRange                 ",  TIFFTAG_DOTRANGE, 
    "TargetPrinter            ",  TIFFTAG_TARGETPRINTER, 
    "ExtraSamples             ",  TIFFTAG_EXTRASAMPLES, 
    "SampleFormat             ",  TIFFTAG_SAMPLEFORMAT, 
    "SMinSampleValue          ",  TIFFTAG_SMINSAMPLEVALUE, 
    "SMaxSampleValue          ",  TIFFTAG_SMAXSAMPLEVALUE, 
    "JpegProc                 ",  TIFFTAG_JPEGPROC, 
    "JpegIfOffset             ",  TIFFTAG_JPEGIFOFFSET, 
    "JpegIfbytecount          ",  TIFFTAG_JPEGIFBYTECOUNT, 
    "JpegRestartInterval      ",  TIFFTAG_JPEGRESTARTINTERVAL, 
    "JpegLosslessPredictors   ",  TIFFTAG_JPEGLOSSLESSPREDICTORS, 
    "JpegPointtransform       ",  TIFFTAG_JPEGPOINTTRANSFORM, 
    "JpegQTables              ",  TIFFTAG_JPEGQTABLES, 
    "JpegDCTables             ",  TIFFTAG_JPEGDCTABLES, 
    "JpegACTables             ",  TIFFTAG_JPEGACTABLES, 
    "YcbcrCoefficients        ",  TIFFTAG_YCBCRCOEFFICIENTS, 
    "YcbcrSubSampling         ",  TIFFTAG_YCBCRSUBSAMPLING, 
    "YcbcrPositioning         ",  TIFFTAG_YCBCRPOSITIONING, 
    "ReferenceBlackWhite      ",  TIFFTAG_REFERENCEBLACKWHITE, 
    "Matteing                 ",  TIFFTAG_MATTEING, 
    "Datatype                 ",  TIFFTAG_DATATYPE, 
    "ImageDepth               ",  TIFFTAG_IMAGEDEPTH, 
    "TileDepth                ",  TIFFTAG_TILEDEPTH, 
};
//------------------------------------------------------------------------
//
//  class TIFFImage
//
//------------------------------------------------------------------------
TIFFImage::TIFFImage()
{
	_pixels=NULL;
	_tagHead=NULL;
	   
	_width=0;
	_height=0;
	_depth=1;
	_widthTile=0;
	_heightTile=0;
	_depthTile=1;
	_bitspersample=0;
	   
	_pStripByteCounts=0;
	_pStripOffsets=0;
	_numStrips=0;
	_pTileByteCounts=0;
	_pTileOffsets=0;
	_numTiles=0;
}


TIFFImage::~TIFFImage()
{
	if (_pixels) delete[] _pixels; 
	if (_pStripByteCounts) delete _pStripByteCounts;
	if (_pStripOffsets)    delete _pStripOffsets;
	if (_pTileByteCounts)  delete _pTileByteCounts;
	if (_pTileOffsets)     delete _pTileOffsets;
    
	//   if (_tagHead) delete _tagHead;  
}


boolean TIFFImage::copyData(unsigned char *data, long size)
{
	if (_bitspersample != 8){
		printf("image data is not 8 bit\n");
		return FALSE;
	}
	if (size != _width*_height*_depth){
		printf("image size mismatch\n");
		return FALSE;
	}
	memcpy(data, _pixels, size);
	return TRUE;
}


boolean TIFFImage::read(char *filename)
{
	char byteOrder[3]={0};
	unsigned short  version;
	unsigned long offsetIFD;

	if ((_fp=fopen(filename, "r"))==NULL){
		printf("could not open %s\n", filename);
		return FALSE;
	}
	if (fread(byteOrder, sizeof(char), 2, _fp)!=2){
		printf("could not read byte order\n");
		return FALSE;
	}
	if (!strcmp(byteOrder, "II")){
		_byteOrder = ByteOrderPC;
		printf("PC byte order\n");
	}else if (!strcmp(byteOrder, "MM")){
		_byteOrder = ByteOrderUnix;
		printf("Unix/MAC byte order\n");
	}else{
		printf("undefined byte order %s\n", byteOrder);
	}

	if (!read(&version)){
		printf("could not read version\n");
		return FALSE;
	}
	if (version!=42){
		printf("version is %d should be 42, invalid TIFF format\n", (int)version);
	//      return FALSE;
	}

	if (!read(&offsetIFD)){
		printf("could not read IFD offset\n");
		return FALSE;
	}
   
	printf("reading Image File Directory at %ld\n", offsetIFD);

	while ((offsetIFD=readIFD(offsetIFD))!=(long)NULL){
		printf("reading Image File Directory at %ld\n", offsetIFD);
	}
	   
	if (!parseTags()){
		printf("error parsing tags\n");
		return FALSE;
	}
	   
	if (!readStrips()){
		if (!readTiles()){
			printf("error reading pixels\n");
			return FALSE; 
		}
	}
	   
	return TRUE;
}


long TIFFImage::readIFD(long offset)
{  
	unsigned short entryCount=0;

	if (fseek(_fp, offset, SEEK_SET)){
		printf("could not find IFD offset\n");
		return 0;
	}

	if (!read(&entryCount)){
		printf("could not read IFD entry count\n");
		return 0;
	}
	   
	if (entryCount<=0){
		printf("readIFD(), entryCount=%d\n", (int)entryCount);
		return 0;
	}
	printf("IFD has %d entries\n", entryCount);
	for (int i=0;i<entryCount;i++){
		Tag *pTag = new Tag(this);
		if (!pTag->read(_fp)){
			printf("error reading entry\n");
			return 0;
		}
		pTag->_next = _tagHead; 
		_tagHead=pTag;
	}
	   
	if (!read((unsigned long *)&offset)){
		printf("could not read IFD offset\n");
		return 0;
	}
	   
	return offset;  
}

Tag *TIFFImage::getTag(int tagID)
{
	Tag *ptr = _tagHead;

	while (ptr){
		if (ptr->getTagID()==tagID){
			return ptr;
		}
		ptr = ptr->_next;
	}
	return NULL;
}

boolean TIFFImage::parseTags()
{  
	Tag *tag;

	if (tag=getTag(TIFFTAG_IMAGEWIDTH)){
		_width = tag->getFirstValue();
	}

	if (tag=getTag(TIFFTAG_IMAGELENGTH)){
		_height = tag->getFirstValue();
	}

	if (tag=getTag(TIFFTAG_IMAGEDEPTH)){
		_depth = tag->getFirstValue();
	}
	   
	if (tag=getTag(TIFFTAG_BITSPERSAMPLE)){
		_bitspersample = tag->getFirstValue();
	}
	   
	if (tag=getTag(TIFFTAG_STRIPOFFSETS)){
		tag->getValue(&_pStripOffsets);
		_numStrips = tag->getLength();
	}
	   
	if (tag=getTag(TIFFTAG_STRIPBYTECOUNTS)){
		tag->getValue(&_pStripByteCounts);
	}
	   
	if (tag=getTag(TIFFTAG_TILEOFFSETS)){
		tag->getValue(&_pTileOffsets);
		_numTiles = tag->getLength();
	}
	   
	if (tag=getTag(TIFFTAG_TILEBYTECOUNTS)){
		tag->getValue(&_pTileByteCounts);
	}
   
	if (tag=getTag(TIFFTAG_TILEWIDTH)){
		_widthTile = tag->getFirstValue();
	}
	   
	if (tag=getTag(TIFFTAG_TILELENGTH)){
		_heightTile = tag->getFirstValue();
	}
	   
	if (tag=getTag(TIFFTAG_TILEDEPTH)){
		_depthTile = tag->getFirstValue();
	}
	   
	if (!_pStripByteCounts && !_pTileByteCounts){
		_pStripByteCounts = new unsigned long;
		*_pStripByteCounts = _width*_height*_depth*(_bitspersample/8);
	}
	if (!_pStripOffsets && !_pTileOffsets){
		_pStripOffsets = new unsigned long;
		*_pStripOffsets = DEFAULT_STRIP_OFFSET;
		_numStrips = 1;
	}
	   
	if (_numStrips){
		printf("TIFFImage::parseTags(), numStrips=%d\n", (int)_numStrips);
		printf("TIFFImage::parseTags(), stripByteCounts[0]=%d\n", (int)(_pStripByteCounts[0]));
		printf("TIFFImage::parseTags(), stripOffsets[0]=%d\n", (int)(_pStripOffsets[0]));
	}
	if (_numTiles){
		printf("TIFFImage::parseTags(), numTiles=%d\n", (int)_numTiles);
		printf("TIFFImage::parseTags(), tileByteCounts[0]=%d\n", (int)(_pTileByteCounts[0]));
		printf("TIFFImage::parseTags(), tileOffsets[0]=%d\n", (int)(_pTileOffsets[0]));
	}
	printf("TIFFImage::parseTags(), bitsPerSample=%d\n", (int)_bitspersample);
	printf("TIFFImage::parseTags(), width=%d\n", (int)_width);
	printf("TIFFImage::parseTags(), height=%d\n", (int)_height);
	if (verifyDepth()){
		printf("TIFFImage::parseTags(), depth=%d\n", (int)_depth);
	}

	return TRUE;
}

boolean TIFFImage::verifyDepth()
{
	if (_depth>1) return TRUE;
	//
	// determine number of bytes in image
	//
	unsigned long numBytes = 0;
	if (_numStrips){  
		for (int k=0;k<_numStrips;k++){
			numBytes+=_pStripByteCounts[k];
		}
	}else if (_numTiles){
		for (int k=0;k<_numTiles;k++){
			numBytes+=_pTileByteCounts[k];
		}
	}else{
		printf("TIFFImage::verifyDepth() - can't determine # data bytes\n");
		return FALSE;
	}
   
	//
	// check number of bytes against calculated from W*H for data size
	//
	if (numBytes != _width*_height*(_bitspersample/8)){
		printf("warning, # bytes = %ld, Width X Height X size = %ld", 
				numBytes, _width*_height*(_bitspersample/8));
		_depth = ((double)numBytes)/(_width*_height*(_bitspersample/8));
		if (_depth*_width*_height*(_bitspersample/8) == numBytes){
			printf(" ... resolved depth=%d\n", (int)_depth);
			return TRUE;
		}else{
			printf(" ... could not resolve depth\n");
			return FALSE;
		}
	}
	return TRUE;
}

boolean TIFFImage::readStrips()
{  
	unsigned short ushort;
	unsigned char  uchar;

	if (_numStrips==0) return FALSE;

	switch (_bitspersample){
		case 8:{
			_pixels = new char[_width*_height*_depth];
			char *pPixels = (char *)_pixels;
			for (int i=0;i<_numStrips;i++){
				if (fseek(_fp, _pStripOffsets[i], SEEK_SET)){
					printf("could not find strip offset (%ld)\n", _pStripOffsets[i]);
					return FALSE;
				}
				if (fread(pPixels, sizeof(char), _pStripByteCounts[i], _fp)!=_pStripByteCounts[i]){
				printf("could not read (%d) bytes from offset (%d)\n", 
						(int)_pStripByteCounts[i], (int)_pStripOffsets[i]);
					return FALSE;
				}
				pPixels += _pStripByteCounts[i];
			}
			break;
       }
		case 16:{
			_pixels = new unsigned short[_width*_height*_depth];
			unsigned short *pPixels = (unsigned short *)_pixels;
			for (int i=0;i<_numStrips;i++){
				if (fseek(_fp, _pStripOffsets[i], SEEK_SET)){
					printf("could not find strip offset (%ld)\n", _pStripOffsets[i]);
					return FALSE;
				}
				for (int j=0;j<(_pStripByteCounts[i]/2);j++){
					if (!read(pPixels)){
						return FALSE;
					}
					pPixels++;
				}
			}
			break;
       }
       case 32:{
			_pixels = new unsigned long[_width*_height*_depth];
			unsigned long *pPixels = (unsigned long *)_pixels;
			for (int i=0;i<_numStrips;i++){
				if (fseek(_fp, _pStripOffsets[i], SEEK_SET)){
					printf("could not find strip offset (%ld)\n", _pStripOffsets[i]);
					return FALSE;
				}
				for (int j=0;j<(_pStripByteCounts[i]/4);j++){
					if (!read(pPixels, &ushort, &uchar)){
						return FALSE;
					}
					pPixels++;
				}
			}
			break;
       }
       default:
			printf("cannot handle (%d) bits per sample\n", _bitspersample);
			return FALSE;
			//break;
	}
	return TRUE;
}

boolean TIFFImage::readTiles()
{  
	unsigned short ushort;
	unsigned char  uchar;
	long     tileX=0;
	long     tileY=0;
	long     tileZ=0;
	long     x, y, z;

	if (_numTiles==0) return FALSE;
   
	switch (_bitspersample){
		case 8:{
			_pixels = new char[_width*_height*_depth];
			char *pPixels = (char *)_pixels;
			char *tile = new char[_widthTile*_heightTile*_depthTile];
			char *tilePtr;
			for (int i=0;i<_numTiles;i++){
				if (fseek(_fp, _pTileOffsets[i], SEEK_SET)){
					printf("could not find tile offset (%ld)\n", _pTileOffsets[i]);
					return FALSE;
				}
				if (fread(tile, sizeof(char), _pTileByteCounts[i], _fp)!=_pTileByteCounts[i]){
					printf("could not read (%d) bytes from offset (%d)\n", 
						(int)_pTileByteCounts[i], (int)_pTileOffsets[i]);
					return FALSE;
				}
				for (z=tileZ;z<min(_depth,_depthTile+tileZ);z++){
					for (y=tileY;y<min(_height,_heightTile+tileY);y++){
						for (x=tileX;x<min(_width,_widthTile+tileX);x++){
							tilePtr = tile + ((z-tileZ)*_heightTile+(y-tileY))*_widthTile+(x-tileX);
							*(pPixels+(z*_height+y)*_width+x) = *tilePtr;
						} 
					}
				}
				//
				// test for continuation in X
				//
				if (_widthTile<_width){
					tileX+=_widthTile;
					if (tileX < _width){
						continue;
					}
					tileX=0;
				}
				//
				// test for continuation in Y
				//
				if (_heightTile<_height){
					tileY+=_heightTile;
					if (tileY < _height){
						continue;
					}
					tileY=0;
				}
				//
				// test for continuation in Z
				//
				if (_depthTile<_depth){
					tileZ+=_depthTile;
					if (tileZ < _depth){
						continue;
					}
					tileZ=0;
				}
			}
			break;
		}
		case 16:{
			_pixels = new unsigned short[_width*_height*_depth];
			unsigned short *pPixels = (unsigned short *)_pixels;
			for (int i=0;i<_numTiles;i++){
					if (fseek(_fp, _pTileOffsets[i], SEEK_SET)){
						printf("could not find tile offset (%ld)\n", _pTileOffsets[i]);
						return FALSE;
					}
					unsigned short temp;
					for (z=tileZ;z<_depthTile+tileZ;z++){
						for (y=tileY;y<_heightTile+tileY;y++){
							for (x=tileX;x<_widthTile+tileX;x++){
								if (!read(&temp)){
								return FALSE;
							}
							if (x>=_width || y>=_height || z>=_depth) continue;
							*(pPixels+(z*_height+y)*_width+x) = temp;
						} 
					}
				}
				//
				// test for continuation in X
				//
				if (_widthTile<_width){
					tileX+=_widthTile;
					if (tileX < _width){
						continue;
					}
					tileX=0;
				}
				//
				// test for continuation in Y
				//
				if (_heightTile<_height){
					tileY+=_heightTile;
					if (tileY < _height){
						continue;
					}
					tileY=0;
				}
				//
				// test for continuation in Z
				//
				if (_depthTile<_depth){
					tileZ+=_depthTile;
					if (tileZ < _depth){
						continue;
					}
					tileZ=0;
				}
			}
			break;
		}
		case 32:{
			_pixels = new unsigned long[_widthTile*_heightTile*_depthTile];
			unsigned long *pPixels = (unsigned long *)_pixels;
			for (int i=0;i<_numTiles;i++){
				if (fseek(_fp, _pTileOffsets[i], SEEK_SET)){
					printf("could not find tile offset (%ld)\n", _pTileOffsets[i]);
					return FALSE;
				}
				for (int j=0;j<(_pTileByteCounts[i]/4);j++){
					if (!read(pPixels, &ushort, &uchar)){
						return FALSE;
					}
					pPixels++;
				}
			}
			break;
		}
		default:
			printf("cannot handle (%d) bits per sample\n", _bitspersample);
			return FALSE;
			//break;
	}
	return TRUE;
}

boolean TIFFImage::read(unsigned short *ushort)
{
	unsigned char bytes[2];
	if (fread(bytes, sizeof(char), 2, _fp)!=2){
		printf("could not read short\n");
		return FALSE;
	}else{
		//printf("short=[0x%x 0x%x]=", (int)bytes[0],(int)bytes[1]);   
		if (_byteOrder==ByteOrderPC){
			*ushort = bytes[0] + (bytes[1]<<8);
		}else{
			*ushort = *((unsigned short *)bytes);
		}
		//printf("%d\n",(int) *ushort);
		return TRUE;
	}
}

boolean TIFFImage::read(unsigned long *ulong, unsigned short *ushort, unsigned char *uchar)
{
	unsigned char bytes[4];

	assert(sizeof(long)==(2*sizeof(short)));
	if (fread(bytes, sizeof(char), 4, _fp)!=4){
		printf("could not read long\n");
		return FALSE;
	}
	if (ushort!=NULL){
		if (_byteOrder==ByteOrderPC){
			*ushort = bytes[0] + (bytes[1]<<8);
			*(ushort+1)= bytes[2] + (bytes[3]<<8);
		}else{
			*ushort = *((unsigned short *)bytes);
			*(ushort+1) = *((unsigned short *)(bytes+2));
		}
	}
	if (uchar!=NULL){
		memcpy(uchar, bytes, 4);
	}
	//printf("long=[0x%x 0x%x 0x%x 0x%x]=", (int)bytes[0],(int)bytes[1],
	//                                      (int)bytes[2],(int)bytes[3]);   
	if (_byteOrder==ByteOrderPC){
		*ulong = (bytes[3]<<24) + (bytes[2]<<16) + (bytes[1]<<8) + bytes[0];
	}else{
		*ulong = *((unsigned long *)bytes);
	}
	//printf("%ld\n", *ulong);
	return TRUE;
}

/*
boolean TIFFImage::write(char *filename)
{
   ilMemoryImage *memImage;
   ilSize size;
   size.x=_width;
   size.y=_height;
   size.z=_depth;
   size.c=1;
   ilType type;
   switch (_bitspersample){
       case 8:
          type = ilUChar;
          break;
       case 16:
          type = ilUShort;
          break;
       case 32:
          type = ilULong;
          break;
   }
   ilOrder order = ilSeparate;
   
   memImage = new ilMemoryImage(_pixels, size, type, order);
   
   ilFileImage *fileImage = ilCreateImageFile(filename, size, type, order,
                                        ilTIFF_IMG,NULL);
   if (!fileImage) {
      printf("TIFFImage::write() error writing %s\n",filename);
      delete memImage;
      return FALSE;
   }

   *fileImage << *memImage;
   delete fileImage;
   delete memImage;
   return TRUE;
}
*/

//------------------------------------------------------------------------
//
//  class Tag
//
//------------------------------------------------------------------------
Tag::Tag(TIFFImage *tiffImage)
{
	_length=0;
	_pLong=NULL;
	_pShort=NULL;
	_pChar=NULL; 
	_tiffImage=tiffImage;
	_next=NULL;
}

Tag::~Tag()
{
	if (_pLong) delete[] _pLong;
	if (_pShort) delete[] _pShort;
	if (_pChar) delete[] _pChar;
}

boolean Tag::read(FILE *fp)
{
	long  savePosition;

	_entryOffset = ftell(fp);
	   
	if (!_tiffImage->read(&_tag)){
		printf("could not read entry tag\n");
		return FALSE;
	}   

	if (!_tiffImage->read(&_entryType)){
		printf("could not read entry type\n");
		return FALSE;
	}   
	   
	if (!_tiffImage->read(&_length)){
		printf("could not read entry _length\n");
		return FALSE;
	}   
	unsigned short actualShort[2];
	unsigned char actualChar[4];
	if (!_tiffImage->read(&_valueOffset, actualShort, actualChar)){
		printf("could not read entry offset\n");
		return FALSE;
	}   
   

	savePosition = ftell(fp);
	//
	// read value
	//
	_dataType=TIFF_NOTYPE;
	parseTagType(_tag);
	parseDataType(_entryType);
   
	switch(_dataType){
		//
		// 8 bit data types
		//
		case TIFF_UNDEFINED:
		case TIFF_SBYTE:
		case TIFF_ASCII:
		case TIFF_BYTE:
		if (_length<=0) break;
		_pChar = new char[_length];
		if (_length<=(sizeof(long)/sizeof(char))){
			memcpy(_pChar, &actualChar, sizeof(char)*_length);
		}else{
			fseek(fp, _valueOffset, SEEK_SET);
			if (fread(_pChar, sizeof(char), _length, fp)!=_length){
				printf("error reading BYTE value\n");
				return FALSE;
			}
		}
		break;
		//
		// 16 bit data types
		//
		case TIFF_SHORT:
		case TIFF_SSHORT:{
			if (_length<=0) {
				printf("error, length<0\n");
				return FALSE;
			}
			_pShort = new unsigned short[_length];
			if (_length<=(sizeof(long)/sizeof(short))){
				memcpy(_pShort, actualShort, sizeof(short)*_length);
			}else{
				fseek(fp, _valueOffset, SEEK_SET);
				for (int i=0;i<_length;i++){
					if (!_tiffImage->read(&_pShort[i])){
						printf("error reading SHORT buffer\n");
						return FALSE;
					}
				}
			}
        break;
		}
		//
		// 32 bit data types
		//
		case TIFF_LONG:
		case TIFF_SLONG:{
			if (_length<=0) break;
			_pLong = new unsigned long[_length];
			if (_length<=1){
				*_pLong = _valueOffset;
			}else{
				fseek(fp, _valueOffset, SEEK_SET);
				for (int i=0;i<_length;i++){
					if (!_tiffImage->read(&_pLong[i])){
						printf("error reading LONG buffer\n");
						return FALSE;
					}
				}
			}
			break;
		}
		case TIFF_FLOAT:
			if (_length<=0) break;
				_pFloat = new float[_length];
				if (_length<=1){
				*_pFloat = _valueOffset;
				}else{
					fseek(fp, _valueOffset, SEEK_SET);
					if (fread(_pFloat, sizeof(float), _length, fp)!=_length){
						printf("error reading float buffer\n");
						return FALSE;
					}
				}
			break;
		//
		// 64 bit data types
		//
		case TIFF_RATIONAL:{
			if (_length<=0) break;
			_pRational = new unsigned long[_length*2];
			fseek(fp, _valueOffset, SEEK_SET);
			for (int i=0;i<_length;i++){
				if (!_tiffImage->read(&_pRational[2*i])){
					printf("error reading LONG buffer\n");
					return FALSE;
				}
				if (!_tiffImage->read(&_pRational[2*i+1])){
					printf("error reading LONG buffer\n");
					return FALSE;
				}
			}
			break;
		}
		case TIFF_DOUBLE:
			if (_length<=0) break;
			_pDouble = new double[_length];
			fseek(fp, _valueOffset, SEEK_SET);
			if (fread(_pDouble, sizeof(double), _length, fp)!=_length){
				printf("error reading DOUBLE buffer\n");
				return FALSE;
			}
			break;
		case TIFF_NOTYPE:
		default:
			printf("unknown data format <%d> in Tag::read()\n", _entryType);
			return FALSE;
			//break;
	}
	//   printf("offset=%-10ld\t", (long)_valueOffset);
	printf("%25s (%10s) = %s\n",_tagName, _typeName, getValueString());
	fseek(fp, savePosition, SEEK_SET);
   
	return TRUE;
}

double Tag::getFirstValue()
{
	if (_pChar)
		return *_pChar;
	if (_pShort)
		return *_pShort;
	if (_pLong)
		return *_pLong;
	if (_pFloat)
		return *_pFloat;
	if (_pDouble)
		return *_pDouble;
	if (_pRational)
		return ((double)(_pRational[0]))/_pRational[1];
	return -1;
}

int Tag::getValue(char **value)
{
	if (getDataType()==TIFF_BYTE || 
		getDataType()==TIFF_SBYTE || 
		getDataType()==TIFF_UNDEFINED || 
		getDataType()==TIFF_ASCII){
		*value = _pChar;
		return _length;
	}else{
		return 0;
	}
}

int Tag::getValue(unsigned short **value)
{
	if (getDataType()==TIFF_SHORT ||
		getDataType()==TIFF_SSHORT){
		*value = _pShort;
		return _length;
	}else{
		return 0;
	}
}

int Tag::getValue(unsigned long **value)
{
	if (getDataType()==TIFF_LONG || getDataType()==TIFF_SLONG){
		*value = _pLong;
		return _length;
	}else{
		return 0;
	}
}

char *Tag::getValueString()
{
	static char string[1000];
	char buffer[200];

	strcpy(string, "Unknown Data Type");
	switch(_dataType){
		case TIFF_ASCII:
			sprintf(string, "'%s'", _pChar);
			break;
		case TIFF_BYTE:
		case TIFF_SBYTE:
		case TIFF_UNDEFINED:{
			strcpy(string, "'");
			for (int i=0;i<_length;i++){
				sprintf(buffer, "%c",(char)(_pChar[i]));
				strcat(string, buffer);
			}
			strcat(string, "'");
			break;
		}
		case TIFF_SSHORT:
		case TIFF_SHORT:{
			string[0]=NULL;
			for (int i=0;i<_length;i++){
				sprintf(buffer, "%d, ", (int)(_pShort[i]));
				strcat(string, buffer);
			}
			break;
		}
		case TIFF_LONG:{
			string[0]=NULL;
			for (int i=0;i<_length;i++){
				sprintf(buffer, "%ld, ", (long)(_pLong[i]));
				strcat(string, buffer);
			}
			break;
		}
		case TIFF_FLOAT:{
			string[0]=NULL;
			for (int i=0;i<_length;i++){
				sprintf(buffer, "%f, ", (float)(_pFloat[i]));
				strcat(string, buffer);
			}
			break;
		}
		case TIFF_DOUBLE:{
			string[0]=NULL;
			for (int i=0;i<_length;i++){
				sprintf(buffer, "%lf, ", (double)(_pDouble[i]));
				strcat(string, buffer);
			}
			break;
		}
		case TIFF_RATIONAL:{
			string[0]=NULL;
			for (int i=0;i<_length;i++){
				//	    sprintf(buffer, "%ld/%ld, ", (long)_pRational[0],(long)_pRational[1]);
				sprintf(buffer, "%lg, ", ((double)_pRational[0])/((double)_pRational[1]));
				strcat(string, buffer);
			}
			break;
		}
	}
	return string;
}

void Tag::parseDataType(int Type)
{
	_dataType = TIFF_UNDEFINED;
	_typeName[0] = NULL;
	   
	for (int i=0;i<MAX_TYPE_IDS;i++){
		if (typeID[i].id==Type){
			_dataType = (TIFFDataType) Type;
			strcpy(_typeName,typeID[i].name);
			return;
		}
	}
	sprintf(_typeName, "Unknown Data Type (%d)", Type);
}

void Tag::parseTagType(int tag)
{
	_tagType = -1;
	_tagName[0] = NULL;
	   
	for (int i=0;i<MAX_TAG_IDS;i++){
		if (tagID[i].id==tag){
			_tagType = tag;
			strcpy(_tagName,tagID[i].name);
			return;
		}
	}
	sprintf(_tagName, "Unknown Tag (%d)", tag);
}
