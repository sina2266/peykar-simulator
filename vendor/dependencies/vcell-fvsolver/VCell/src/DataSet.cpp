/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */

#include <stdlib.h>
#include <string.h>
#include <VCELL/DataSet.h>
#include <iostream>
using std::cout;
using std::endl;

/*
 * Little-endian operating systems:
 * Linux on x86, x64, MIPSEL, Alpha and Itanium
 * Mac OS X on x86, x64
 * OpenVMS on VAX, Alpha and Itanium
 * Solaris on x86, x64, PowerPC
 * Tru64 UNIX on Alpha
 * Windows on x86, x64 and Itanium
 * Microsoft Xbox 1
 *
 * Big-endian operating systems:
 * AIX on POWER
 * AmigaOS on PowerPC and 680x0
 * HP-UX on Itanium and PA-RISC
 * Linux on MIPS, SPARC, PA-RISC, POWER, PowerPC, 680x0, ESA/390, and z/Architecture
 * Mac OS on PowerPC and 680x0
 * Mac OS X on PowerPC
 * MVS and DOS/VSE on ESA/390, and z/VSE and z/OS on z/Architecture
 * Solaris on SPARC
 * Microsoft Xbox 360, PlayStation 3, Nintendo Wii
 *
 */
Endian DataSet::endian = endian_not_set;
bool DataSet::isBigEndian() {
	if (endian == endian_not_set) {
		union {
			uint32 i;
			char c[4];
		} testint = {0x01020304};
		endian = (testint.c[0] == 1) ? big_endian : little_endian;
		cout << "**This is a " << (endian == big_endian ? "big" : "little") << " endian machine.**" << endl;
	}
	return endian == big_endian;
}

uint32 reverseLong(uint32 along)
{
	uint32 newLong = ((along>>24)&0x000000ff)|((along>>8)&0x0000ff00)|((along<<8)&0x00ff0000)|((along<<24)&0xff000000);
	return newLong;
}

unsigned char *reverseDouble(double adouble)
{
	static union {
		double dbl;
		unsigned char array[8];
	} longDoubleUnion;

	longDoubleUnion.dbl = adouble;

	for (int i=0;i<4;i++){
		unsigned char temp = longDoubleUnion.array[i];
		longDoubleUnion.array[i] = longDoubleUnion.array[7-i];
		longDoubleUnion.array[7-i] = temp;
	}
	return longDoubleUnion.array;
}

double reverseDouble(unsigned char array[8])
{
	static union {
		double dbl;
		unsigned char array[8];
	} longDoubleUnion;

	memcpy(longDoubleUnion.array,array,8);

	for (int i=0;i<4;i++){
		unsigned char temp = longDoubleUnion.array[i];
		longDoubleUnion.array[i] = longDoubleUnion.array[7-i];
		longDoubleUnion.array[7-i] = temp;
	}
	return longDoubleUnion.dbl;
}

//struct FileHeader {
//   char   magicString[16];
//   char   versionString[8];
//   long   numBlocks;
//   long   firstBlockOffset;
//   long   sizeX;
//   long   sizeY;
//   long   sizeZ;
//};
//
void DataSet::readHeader(FILE *fp, FileHeader *header)
{
	if (isBigEndian()) {
		if (fread(header, sizeof(FileHeader), 1, fp)!=1){
			throw "DataSet::readHeader() - could not read header (big endian)";
		}
	} else {
		if (fread(header->magicString, sizeof(char), 16, fp)!=16){
			throw "DataSet::readHeader() - could not read header->magicString (little endian)";
		}
		if (fread(header->versionString, sizeof(char), 8, fp)!=8){
			throw "DataSet::readHeader() - could not read header->versionString (little endian)";
		}
		uint32 newLongs[5];
		if (fread(&newLongs, sizeof(int32), 5, fp)!=5){
			throw "DataSet::readHeader() - could not read header->offsets.. (little endian)";
		}
		header->numBlocks = reverseLong(newLongs[0]);
		header->firstBlockOffset = reverseLong(newLongs[1]);
		header->sizeX = reverseLong(newLongs[2]);
		header->sizeY = reverseLong(newLongs[3]);
		header->sizeZ = reverseLong(newLongs[4]);
	}
}

void DataSet::writeHeader(FILE *fp, FileHeader *header)
{
	if (isBigEndian()) {
		if (fwrite(header, sizeof(FileHeader), 1, fp)!=1){
			throw "DataSet::writeHeader() - could not write header (big endian)";
		}
	} else {
		if (fwrite(header->magicString, sizeof(char), 16, fp)!=16){
			throw "DataSet::writeHeader() - could not write header->magicString (little endian)";
		}
		if (fwrite(header->versionString, sizeof(char), 8, fp)!=8){
			throw "DataSet::writeHeader() - could not write header->versionString (little endian)";
		}
		uint32 newLongs[5];
		newLongs[0] = reverseLong(header->numBlocks);
		newLongs[1] = reverseLong(header->firstBlockOffset);
		newLongs[2] = reverseLong(header->sizeX);
		newLongs[3] = reverseLong(header->sizeY);
		newLongs[4] = reverseLong(header->sizeZ);
		if (fwrite(newLongs, sizeof(int32), 5, fp)!=5){
			throw "DataSet::writeHeader() - could not write header->longs (little endian)";
		}
	}
}

//struct DataBlock {
//   char   varName[DATABLOCK_STRING_SIZE];
//   long   size;
//   long   dataOffset;
//};
//
void DataSet::readDataBlock(FILE *fp, DataBlock *block)
{
	if (isBigEndian()) {
		if (fread(block, sizeof(DataBlock), 1, fp)!=1){
			throw "DataSet::read() - could not read dataBlock (big endian)";
		}
	} else {
		if (fread(block->varName, sizeof(char), DATABLOCK_STRING_SIZE, fp)!=DATABLOCK_STRING_SIZE){
			throw "DataSet::readDataBlock() - could not read block->varName (little endian)";
		}
		uint32 newLongs[3];
		if (fread(&newLongs, sizeof(int32), 3, fp)!=3){
			throw "DataSet::read() - could not read dataBlock longs... (little endian)";
		}
		block->varType = reverseLong(newLongs[0]);
		block->size = reverseLong(newLongs[1]);
		block->dataOffset = reverseLong(newLongs[2]);
	}
}

void DataSet::writeDataBlock(FILE *fp, DataBlock *block)
{
	if (isBigEndian()) {
		if (fwrite(block, sizeof(DataBlock), 1, fp)!=1){
			throw "DataSet::writeDataBlock() - error writing data block (big endian)";
		}
	} else {
		if (fwrite(block->varName, sizeof(char), DATABLOCK_STRING_SIZE, fp)!=DATABLOCK_STRING_SIZE){
			throw "DataSet::writeDataBlock() - could not write block->varName (little endian)";
		}
		uint32 newLongs[3];
		newLongs[0] = reverseLong(block->varType);
		newLongs[1] = reverseLong(block->size);
		newLongs[2] = reverseLong(block->dataOffset);
		if (fwrite(newLongs, sizeof(int32), 3, fp)!=3){
			throw "DataSet::writeHeader() - could not write dataBlock->longs (little endian)";
		}
	}
}

void DataSet::readDoubles(FILE *fp, double *data, int length)
{
	if (isBigEndian()) {
		if (fread(data, sizeof(double), length, fp)!=length){
			throw "DataSet::readDoubles() - error reading data (big endian)";
		}
	} else {
		unsigned char tempArray[8];
		for (int i=0;i<length;i++){
			if (fread(tempArray, sizeof(char), 8, fp)!=8){
				throw "DataSet::readDoubles() - could not read double value (little endian)";
			}
			data[i] = reverseDouble(tempArray);
		}
	}
}

void DataSet::writeDoubles(FILE *fp, double *data, int length)
{
	if (isBigEndian()) {
		if (fwrite(data, sizeof(double), length, fp)!=length){
			throw "DataSet::writeDoubles() - error writing data (big endian)";
		}
	} else {
		for (int i=0;i<length;i++){
			unsigned char *tempPtr = reverseDouble(data[i]);
			if (fwrite(tempPtr, sizeof(int32), 2, fp)!=2){
				throw "DataSet::writeDoubles() - could not write double value (little endian)";
			}
		}
	}
}
