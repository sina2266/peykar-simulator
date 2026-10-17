/*
Copyright libCellML Contributors

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include "zlib.h"

#include "compress.h"

typedef unsigned char BYTE;
#ifdef WIN32
  typedef unsigned long uLongPlatform;
#else
  typedef size_t uLongPlatform;
#endif

int main()
{

    FILE * readFile = fopen(INPUT_FILE, "rb");
    if( readFile == NULL )
    {
        printf("Couldn't open input file for reading\n");
        exit(1);    //quit
    }

    // get size of file
    fseek( readFile, 0, SEEK_END );
    size_t fileLength = ftell( readFile );
    rewind( readFile );

    BYTE * dataReadInOriginal = (BYTE*)malloc( fileLength );

    // read in entire file
    fread( dataReadInOriginal, fileLength, 1, readFile );

    fclose( readFile );
    readFile = NULL;

    uLongPlatform sizeDataCompressed  = static_cast<uLongPlatform>(fileLength * 1.1 + 12);

    BYTE * dataCompressed = (BYTE*)malloc( sizeDataCompressed );

    int z_result = compress(dataCompressed, &sizeDataCompressed, dataReadInOriginal, fileLength) ;

    switch( z_result )
    {
    case Z_OK:
        printf("***** SUCCESS! *****\n");
        break;

    case Z_MEM_ERROR:
        printf("out of memory\n");
        exit(1);    // quit.
        break;

    case Z_BUF_ERROR:
        printf("output buffer wasn't large enough!\n");
        exit(1);    // quit.
        break;
    }

    FILE * out = fopen( "compressed.dat", "wb" );
    if( out == NULL )
    {
        printf("Couldn't open output file for writing\n");
        exit(1);    //quit
    }
    fwrite( dataCompressed, sizeDataCompressed, 1, out );
    fclose( out );
    out = NULL;

    free(dataReadInOriginal);
    free(dataCompressed);
    return 0;
};
