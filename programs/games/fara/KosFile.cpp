#include "kosSyst.h"
#include "kosfile.h"


CKosFile::CKosFile(char *fileName)
{
	//
	this->fileInfo.bufferPtr = new Byte[FILE_BUFFER_SIZE];
	//
	this->filePointer = 0;
	this->bufferPointer = 0;
	this->validBuffer = false;
	//
	strcpy( this->fileInfo.fileURL, fileName );
}


CKosFile::~CKosFile(void)
{
	//
	delete this->fileInfo.bufferPtr;
}


void CKosFile::ValidateBuffer()
{
	//
	if ( this->validBuffer )
	{
		//
		if ( this->filePointer < this->bufferPointer
			|| this->filePointer >= (this->bufferPointer + FILE_BUFFER_SIZE) )
		{
			//
			this->validBuffer = false;
		}
	}
}


void CKosFile::UpdateBuffer(void)
{
	//
	if ( ! this->validBuffer )
	{
		//
		this->fileInfo.OffsetLow = this->filePointer / OS_BLOCK_SIZE;
		this->fileInfo.OffsetHigh = 0;
		//
		this->bufferPointer = this->fileInfo.OffsetLow * OS_BLOCK_SIZE;
		//
		this->fileInfo.dataCount = FILE_BUFFER_BLOCKS;
		//
		this->fileInfo.rwMode = 0;
		//
		Dword rr = kos_FileSystemAccess( &(this->fileInfo) );
		this->validBuffer = ( rr == 0 ) || ( rr == 6 );
	}
}


int CKosFile::Seek(int seekFrom, int seekStep)
{
	//
	switch ( seekFrom )
	{
	//
	case SEEK_SET:
		//
		this->filePointer = seekStep;
		break;
	//
	case SEEK_CUR:
		//
		this->filePointer += seekStep;
		break;
	}
	//
	this->ValidateBuffer();
	//
	return this->filePointer;
}


int	CKosFile::Read(Byte *targetPtr, int readCount)
{
	int bufferLeast, result;

	//
	result = 0;
	//
	do
	{
		//
		this->UpdateBuffer();
		//
		if ( ! this->validBuffer ) return result;
		//
		bufferLeast = FILE_BUFFER_SIZE - (this->filePointer - this->bufferPointer);
		//
		if ( bufferLeast > readCount ) bufferLeast = readCount;
		//
		if ( bufferLeast )
		{
			//
			memcpy(
				targetPtr,
				this->fileInfo.bufferPtr + (this->filePointer - this->bufferPointer),
				bufferLeast
				);
			//
			targetPtr += bufferLeast;
			readCount -= bufferLeast;
			this->filePointer += bufferLeast;
			//
			result += bufferLeast;
		}
		//
		this->ValidateBuffer();
	}
	while ( readCount > 0 );
	//
	return result;
}


int CKosFile::Write(Byte *sourcePtr, int writeCount)
{
	return 0;
}

