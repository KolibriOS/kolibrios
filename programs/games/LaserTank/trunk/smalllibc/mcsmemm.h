//

struct MemBlock
{
  Dword Size;
  Dword Addr;
  MemBlock *Next;
  MemBlock *Previous;
};


#define INITIALQUEUESIZE  (32 * 4)

#define FALSE   0
#define TRUE    -1

#define MB_FREE   0
#define MB_USER   1

#define SIZE_ALIGN  4



Byte * __fastcall allocmem( Dword reqsize );
void __fastcall freemem( void *vaddress );



