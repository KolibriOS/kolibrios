#if !defined(AFX_CRC32_H__1F375B73_5C72_4711_B464_7212EFC14AAE__INCLUDED_)
#define AFX_CRC32_H__1F375B73_5C72_4711_B464_7212EFC14AAE__INCLUDED_

class CCRC32
{
private:
        Dword table[256];


public:
        Dword GetCRC32( Byte *buffer, Dword length );
        void DoCRC32( Byte *buffer, Dword length, Dword *CRC32value );
        Dword FinalizeCRC32( Dword );
        Dword InitCRC32();
        CCRC32();
};

#endif // !defined(AFX_CRC32_H__1F375B73_5C72_4711_B464_7212EFC14AAE__INCLUDED_)
