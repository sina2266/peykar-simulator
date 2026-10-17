/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */
#ifndef IMAGE_H
#define IMAGE_H

enum DataType {
    DataTypeBit 	= 1,	/* single-bit */
    DataTypeUChar 	= 2,	/* unsigned character (byte) */
    DataTypeChar 	= 4,	/* signed character (byte) */
    DataTypeUShort	= 8,	/* unsigned short integer (nominally 16 bits) */
    DataTypeShort	= 16,	/* signed short integer */
    DataTypeULong	= 32,	/* unsigned long integer */
    DataTypeLong	= 64,	/* long integer */
    DataTypeFloat	= 128,	/* floating point */
    DataTypeDouble	= 256	/* double precision floating point */
};

enum ChannelOrder {
    Interleaved = 1,	/* store as RGBRGBRGBRGB... */
    Sequential  = 2,	/* store as RRR..GGG..BBB.. per line */
    Separate    = 4	/* channels are stored in separate tiles */
};


class Image
{
  public:
    Image();
    ~Image();
  
  
  protected:
    long size;
    long sizeX;
    long sizeY;
    long sizeZ;
    long sizeC;
    
    void         *data;
    ChannelOrder cOrder;
    DataType     dType; 
};
    
#endif
