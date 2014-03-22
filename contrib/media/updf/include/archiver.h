
#ifndef __archiver_h__
#define __archiver_h__

#include "yacasbase.h"
#include "compressedfiles.h"

class CCompressedArchive : public YacasBase
{
public:
    CCompressedArchive(unsigned char * aBuffer, LispInt aFullSize, LispInt aCompressed);
    CompressedFiles iFiles;
};

#endif // __archiver_h__

