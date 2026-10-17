#ifdef HAVE_CONFIG_H
#include <../../config.h>
#endif
// =======================================================================
//
// Windows BMP format raster loader.
//
// 1.1
// 1997/03/28 17:36:38
//
// InterViews Port to the Windows 3.1/NT operating systems
// Copyright (c) 1993 Tim Prinzing
//
// Permission to use, copy, modify, distribute, and sell this software and 
// its documentation for any purpose is hereby granted without fee, provided
// that (i) the above copyright notice and this permission notice appear in
// all copies of the software and related documentation, and (ii) the name of
// Tim Prinzing may not be used in any advertising or publicity relating to 
// the software without the specific, prior written permission of Tim Prinzing.
//
// THE SOFTWARE IS PROVIDED "AS-IS" AND WITHOUT WARRANTY OF ANY KIND, 
// EXPRESS, IMPLIED OR OTHERWISE, INCLUDING WITHOUT LIMITATION, ANY 
// WARRANTY OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.  
//
// IN NO EVENT SHALL Tim Prinzing BE LIABLE FOR ANY SPECIAL, INCIDENTAL, 
// INDIRECT OR CONSEQUENTIAL DAMAGES OF ANY KIND, OR ANY DAMAGES WHATSOEVER 
// RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER OR NOT ADVISED OF THE 
// POSSIBILITY OF DAMAGE, AND ON ANY THEORY OF LIABILITY, ARISING OUT OF OR 
// IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
//
// =======================================================================


#include <InterViews/color.h>
#include <InterViews/raster.h>
#include <InterViews/winbmp.h>
#include <stdlib.h>
#include <stdio.h>

typedef long int dword;
typedef int word;
typedef short int byte;

#define readChar(in,data)  fread(data,1,1,in) == 1
#define readShort(in,data) fread(data,2,1,in) == 1
#define readLong(in,data)  fread(data,4,1,in) == 1


Raster* BMPRaster::load(const char *filename)
{
 char dummyChar;
 short dummyShort;
 long dummyLong;

 FILE *in = fopen(filename,"r");

 // reading BITMAPFILEHEADER
 long fileSize;
 readShort(in,&dummyShort);
 readLong(in,&fileSize);
 readShort(in,&dummyShort);
 readShort(in,&dummyShort);
 readLong(in,&dummyLong);


 // reading BITMAPINFOHEADER
 long infoHeaderSize, width, height;
 short planes, bitCount;
 long compression, sizeImage, xres, yres, colorUsed, colorImp;
 readLong(in,&infoHeaderSize);
 readLong(in,&width);
 readLong(in,&height);
 readShort(in,&planes);
 readShort(in,&bitCount);
 readLong(in,&compression);
 readLong(in,&sizeImage);
 readLong(in,&xres);
 readLong(in,&yres);
 readLong(in,&colorUsed);
 readLong(in,&colorImp);


 // read in color table
 unsigned char red[256], green[256], blue[256];
 int colorTable = (bitCount != 24) ? (1 << bitCount) : 0;
 for (int i=0; i<colorTable; i++)
  {
   readChar(in,&(blue[i]));
   readChar(in,&(green[i]));
   readChar(in,&(red[i]));
   readChar(in,&dummyChar);
//   printf("%d:  %d %d %d\n",i,red[i],green[i],blue[i]);
  }

  Raster *res = new Raster(width,height);

 // read in bitmap image data
 // assume no RLE compression
   for (int row=height-1; row>=0; row--)
    {
     unsigned char data[32];

     // scan lines must end on 4-byte boundaries
     int length = width * bitCount / 8;
     if (length % 4)
       length += 4 - (length % 4);
     for (int k=0; k<length; k++)
       readChar(in,&(data[k]));



     for (int col=0; col<width; col++)
      {
        long int color = 0;
        if (bitCount == 24)
          {
           int index = col * 3;
           color = data[index];
           color <<= 8;
           color |= data[index+1];
           color <<= 8;
           color |= data[index+2];
          }
        else if (bitCount == 8)
          color = data[col];
        else if (bitCount == 4)
          {
           unsigned char temp = data[col/2];
           if (col % 2)
            color = temp & 0x0f;
           else
            color = (temp & 0xf0) >> 4;
          }
        else if (bitCount == 1)
          {
            int whichByte = col / 8;
            int whichBit = col % 8;
            color = !!(data[whichByte] & (1 << (7 - whichBit)));
          }

        if (bitCount == 24 || bitCount == 4 || bitCount == 1)
          {
//            printf("%d %d: %d\n",row,col,color);
            int r = (bitCount == 24) ? (color >> 16)          : red[color];
            int g = (bitCount == 24) ? ((color >> 8) & 0xff)  : green[color];
            int b = (bitCount == 24) ? (color & 0xff)         : blue[color];
//            printf(" --> %d %d %d\n",r,g,b);
            res->poke(col,row,
                      float(r)/255.0f, float(g)/255.0f, float(b)/255.0f,
                      1.0f);
          }
      }
    }


 fclose(in);

// printf("file is of size %d\n",fileSize);
// printf("width and height are %d %d\n",width,height);
// printf("planes and bitCount are %d and %d\n",planes,bitCount);
// printf("sizeImage is %d\n",sizeImage);
// printf("colorUsed and colorImp are %d and %d\n",colorUsed,colorUsed);

 return res;
}
