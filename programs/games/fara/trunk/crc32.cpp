#include "kosSyst.h"
#include "CRC32.h"



CCRC32::CCRC32()
{
        Dword i, mask, j, k;

        for ( i=0xEDB8; i >= 0x8320; i-- )
        {
                mask = i;
                for ( j=0; j<8; j++ )
                {
                        k = mask;
                        mask >>= 1;
                        if ( k & 1 )
                        {
                                mask ^= 0xEDB88320;
                        }
                        this->table[i & 0xFF] = mask;
                }
        }
}


Dword CCRC32::InitCRC32()
{
        return 0xFFFFFFFF;
}


Dword CCRC32::FinalizeCRC32(Dword d)
{
        return ~d;
}


void CCRC32::DoCRC32(Byte *buffer, Dword length, Dword *CRC32value)
{
        Dword result, i;
        Byte mask;

        result = *CRC32value;

        for ( i=0; i<length; i++ )
        {
                mask = (Byte)( ( result ^ buffer[i] ) & 0xFF );
                result = ( result >>= 8 ) ^ this->table[mask];
        }
        *CRC32value = result;
}


Dword CCRC32::GetCRC32(Byte *buffer, Dword length)
{
        Dword result;

        result = 0xFFFFFFFF;

        this->DoCRC32( buffer, length, &result );

        return ~result;
}
