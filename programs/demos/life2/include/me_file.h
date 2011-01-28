#ifndef __MENUET_FILE_H_INCLUDED_
#define __MENUET_FILE_H_INCLUDED_

#include <menuet.h>
#include <me_heap.h>

// Menuet file interface.

namespace Menuet   // All menuet functions, types and data are nested in the (Menuet) namespace.
{
	struct _FileDataStruct;
	typedef _FileDataStruct *TFileData;

	TFileData FileOpen(const char *name, unsigned int buffer_length = 1024);
	int FileClose(TFileData file_data);
	bool FileEof(TFileData file_data);
	unsigned int FileGetPosition(TFileData file_data);
	void FileSetPosition(TFileData file_data, unsigned int pos);
	void FileReset(TFileData file_data);
	unsigned int FileGetLength(TFileData file_data);
	int FileTestRead(TFileData file_data);
	int FileRead(TFileData file_data, void *mem, int size);
}

#ifdef __MENUET__

namespace Menuet
{
// Define the file data structure.

	struct _FileDataStruct
	{
		unsigned int length;
		unsigned int position;
		unsigned int *buffer;
		unsigned int access_param[5];

		enum {PosName = (unsigned int)(((_FileDataStruct*)0)->access_param + 5)};
	};

// Inline functions.

	inline bool FileEof(TFileData file_data)
	{
		return file_data && file_data->position >= file_data->length;
	}

	inline unsigned int FileGetPosition(TFileData file_data)
	{
		return file_data ? file_data->position : 0;
	}

	inline void FileReset(TFileData file_data)
	{
		if (!file_data) return;
		file_data->length = -1;
		file_data->position = 0;
		if (file_data->buffer) file_data->buffer[1] = 0;
	}

// Functions.

	int _FileAccess(void *file_access_param);

	TFileData FileOpen(const char *name, unsigned int buffer_length)
	{
		if (!name || !name[0]) return 0;
		unsigned int name_len = StrLen(name) + 1;
		unsigned int data_len = (_FileDataStruct::PosName + name_len + 3) & ~3;
		buffer_length = (buffer_length / MENUET_FILE_BLOCK_SIZE) * MENUET_FILE_BLOCK_SIZE;
		if (buffer_length) data_len += buffer_length + 2*sizeof(unsigned int);
		TFileData file = (TFileData)Alloc(_FileDataStruct::PosName + data_len);
		if (!file) return 0;
		file->length = -1;
		file->position = 0;
		if (buffer_length)
		{
			file->buffer = (unsigned int*)((char*)file + data_len) - 2;
			file->buffer[0] = buffer_length;
			file->buffer[1] = 0;
		}
		MemCopy(file->access_param + 5, name, name_len);
		unsigned int attr[40/4];
		file->access_param[0] = 5;
		file->access_param[1] = 0;
		file->access_param[2] = 0;
		file->access_param[3] = 0;
		file->access_param[4] = (int)attr;
		_FileAccess(file->access_param);
		file->length = attr[32/4];
		return file;
	}

	int FileClose(TFileData file_data)
	{
		if (!file_data) return -1;
		Free(file_data);
		return 0;
	}

	void FileSetPosition(TFileData file_data, unsigned int pos)
	{
		if (!file_data) return;
		if (file_data->buffer && file_data->buffer[1])
		{
			if (pos >= file_data->position && pos < file_data->position + file_data->buffer[1])
			{
				file_data->buffer[1] -= pos - file_data->position;
			}
			else file_data->buffer[1] = 0;
		}
		file_data->position = pos;
	}

	int _FileReadBuffer(TFileData file_data, void *mem, int size, void *temp_mem = 0)
	{
		unsigned int *buffer;
		if (!file_data || !mem || size <= 0) return -1;
		if (file_data->buffer) buffer = file_data->buffer;
		else if (temp_mem)
		{
			buffer = (unsigned int*)((char*)temp_mem + MENUET_FILE_BLOCK_SIZE);
		}
		else return 0;
		if (!buffer[1]) return 0;
		if (file_data->position >= file_data->length)
		{
			buffer[1] = 0;
			return 0;
		}
		unsigned int buf_size = file_data->length - file_data->position;
		if (buf_size > buffer[1]) buf_size = buffer[1];
		if ((unsigned int)size >= buf_size) size = buf_size;
		MemCopy(mem, (char*)buffer - buffer[1], size);
		file_data->position += size;
		if ((unsigned int)size >= buf_size) buffer[1] = 0;
		else buffer[1] -= size;
		return size;
	}

	int _FileReadSystem(TFileData file_data, void *mem, int size)
	{
		int res;
		unsigned int len0, len1;
		size /= MENUET_FILE_BLOCK_SIZE;
		if (!file_data || !mem || size <= 0) return -1;
		file_data->access_param[0] = 0;
		file_data->access_param[1] = (file_data->position / MENUET_FILE_BLOCK_SIZE) * MENUET_FILE_BLOCK_SIZE;
		file_data->access_param[2] = 0;
		file_data->access_param[3] = size * MENUET_FILE_BLOCK_SIZE;
		file_data->access_param[4] = (unsigned int)mem;
		res = _FileAccess(file_data->access_param);
		if (res != 0 && res != 6) return (res & 255) - 1024;
		if (file_data->length <= file_data->position) return 0;
		len0 = file_data->length - file_data->position;
		len1 = size * MENUET_FILE_BLOCK_SIZE - (file_data->position % MENUET_FILE_BLOCK_SIZE);
		return (len0 <= len1) ? len0 : len1;
	}

	int _FileBufferSystem(TFileData file_data, void *&temp_mem)
	{
		int res;
		unsigned int *buffer;
		if (!file_data) return -1;
		if (file_data->buffer) buffer = file_data->buffer;
		else
		{
			if (!temp_mem)
			{
				temp_mem = Alloc(MENUET_FILE_BLOCK_SIZE + 2*sizeof(unsigned int));
				if (!temp_mem) return -10;
			}
			buffer = (unsigned int*)((char*)temp_mem + MENUET_FILE_BLOCK_SIZE);
			buffer[0] = MENUET_FILE_BLOCK_SIZE;
		}
		buffer[1] = buffer[0];
		res = _FileReadSystem(file_data, (char*)buffer - buffer[1], buffer[1]);
		if (res < 0) buffer[1] = 0;
		else buffer[1] -= file_data->position % MENUET_FILE_BLOCK_SIZE;
		return res;
	}

	int FileTestRead(TFileData file_data)
	{
		int res;
		void *temp_mem = 0;
		if (!file_data) return -1;
		if (file_data->buffer && file_data->buffer[1]) return 0;
		res = _FileBufferSystem(file_data, temp_mem);
		if (temp_mem) Free(temp_mem);
		return (res < 0) ? res : 0;
	}

	int FileRead(TFileData file_data, void *mem, int size)
	{
		int tlen, res, read_len;
		void *temp_mem = 0;
		res = _FileReadBuffer(file_data, mem, size);
		if (res < 0 || res >= size) return res;
		read_len = res;
		mem = (char*)mem + res;
		size -= res;
		tlen = file_data->position % MENUET_FILE_BLOCK_SIZE;
		if (tlen)
		{
			res = _FileBufferSystem(file_data, temp_mem);
			if (res < 0)
			{
				if (temp_mem) Free(temp_mem);
				return read_len ? read_len : res;
			}
			res = _FileReadBuffer(file_data, mem, size);
			read_len += res;
			if (res >= size || file_data->length <= file_data->position ||
				file_data->length - file_data->position <= res)
			{
				if (temp_mem) Free(temp_mem);
				return read_len;
			}
			mem = (char*)mem + res;
			size -= res;
		}
		if (size >= (file_data->buffer ? file_data->buffer[0] : MENUET_FILE_BLOCK_SIZE))
		{
			res = _FileReadSystem(file_data, mem, size);
			if (res < 0)
			{
				if (temp_mem) Free(temp_mem);
				return read_len ? read_len : res;
			}
			file_data->position += res;
			read_len += res;
			if (res < (size / MENUET_FILE_BLOCK_SIZE) * MENUET_FILE_BLOCK_SIZE)
			{
				if (temp_mem) Free(temp_mem);
				return read_len;
			}
			mem = (char*)mem + res;
			size -= res;
		}
		if (size)
		{
			res = _FileBufferSystem(file_data, temp_mem);
			if (res < 0)
			{
				if (temp_mem) Free(temp_mem);
				return read_len ? read_len : res;
			}
			read_len += _FileReadBuffer(file_data, mem, size, temp_mem);
		}
		if (temp_mem) Free(temp_mem);
		return read_len;
	}
	
// Inline functions.

	inline unsigned int FileGetLength(TFileData file_data)
	{
		if (!file_data) return -1;
		if (file_data->length == -1) FileTestRead(file_data);
		return file_data->length;
	}
}

#else   // def  __MENUET__

namespace Menuet
{
	struct _FileDataStruct
	{
		unsigned int data;
	};
}

#endif  // else: def  __MENUET__

#endif  // ndef __MENUET_FILE_H_INCLUDED_

