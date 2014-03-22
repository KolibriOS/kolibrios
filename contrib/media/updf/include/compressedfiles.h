
#ifndef __compressedfiles_h__
#define __compressedfiles_h__

#include "lisptype.h"
#include "lispassert.h"

class CompressedFiles
{
public:
  CompressedFiles(unsigned char * aBuffer, LispInt aFullSize, LispInt aCompressed);
  ~CompressedFiles();
  LispInt FindFile(LispChar * aName);
  LispChar * Name(LispInt aIndex);
  LispChar * Contents(LispInt aIndex);
  inline LispInt NrFiles() const {return iNrFiles;}
  void Sizes(LispInt& aOriginalSize, LispInt& aCompressedSize, LispInt aIndex);
  inline LispInt IsValid() const {return iIsValid;}
protected:
  LispInt GetInt(unsigned char*&indptr);

private:
  // copy constructor not implemented yet, so an assert is in order
  CompressedFiles(const CompressedFiles& aOther)
    : iFullBuffer(NULL),iCompressed(0),iFullSize(0),iIndex(NULL),iNrFiles(0),iIndexSize(0),iIsValid(LispFalse)
  {
    LISPASSERT(0);
  }
  inline CompressedFiles& operator=(const CompressedFiles& aOther)
  {
    iFullBuffer = NULL;
    iCompressed = 0;
    iFullSize = 0;
    iIndex = NULL;
    iNrFiles = 0;
    iIndexSize = 0;
    iIsValid = LispFalse;
    LISPASSERT(0);
    return *this;
  }
private:
  unsigned char * iFullBuffer;
  LispInt iCompressed;
  LispInt iFullSize;
  unsigned char * *iIndex;
  LispInt iNrFiles;
  LispInt iIndexSize;
  LispInt iIsValid;
};

#endif // __compressedfiles_h__
