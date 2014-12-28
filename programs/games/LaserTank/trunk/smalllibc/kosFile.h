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
	~CKosFile(void);
	int	Read(Byte *targetPtr, int readCount);
	int Write(Byte *sourcePtr, int writeCount);
	int Seek(int seekFrom, int seekStep);
	void LoadTex(Byte *targetPtr, Byte Size, int width, int height);
protected:
	int filePointer;
	int bufferPointer;
	bool validBuffer;
	kosFileInfo fileInfo;
	void ValidateBuffer(void);
	void UpdateBuffer(void);
};
