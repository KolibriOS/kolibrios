#pragma once

#define SEEK_SET			0
#define SEEK_CUR			1

#define FILE_BUFFER_SIZE	512
#define OS_BLOCK_SIZE		1
#define FILE_BUFFER_BLOCKS	(FILE_BUFFER_SIZE / OS_BLOCK_SIZE)


class CKosFile
{
public:
	CKosFile(char *fileName);
	virtual ~CKosFile(void);
	virtual int	Read(Byte *targetPtr, int readCount);
	virtual int Write(Byte *sourcePtr, int writeCount);
	virtual int Seek(int seekFrom, int seekStep);
protected:
	int filePointer;
	int bufferPointer;
	bool validBuffer;
	kosFileInfo fileInfo;
	virtual void ValidateBuffer(void);
	virtual void UpdateBuffer(void);
};
