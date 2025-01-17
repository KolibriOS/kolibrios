// memman.cpp : Defines the entry point for the console application.
//

#include "kosSyst.h"
#include "mcsmemm.h"


void * __cdecl operator new ( size_t count, size_t element_size )
{
	return allocmem( (Dword)(count * element_size) );
}

void * __cdecl operator new [] ( size_t amount )
{
	return allocmem( (Dword)amount );
}

void * __cdecl operator new ( size_t amount )
{
	return allocmem( (Dword)amount );
}

void __cdecl operator delete ( void *pointer )
{
	if ( pointer != NULL ) freemem( pointer );
}

void __cdecl operator delete [] ( void *pointer )
{
	if ( pointer != NULL ) freemem( pointer );
}


//
Dword mmMutex = FALSE;
MemBlock *rootfree = NULL;
MemBlock *rootuser = NULL;
bool mmInitialized = false;
Byte *mmHeapTop = NULL;


//
Byte *allocmem( Dword reqsize )
{
  MemBlock *BlockForCheck;
  MemBlock *LastKnownGood;
  Dword tail;
  Byte *address;

  //подровняем размер
  if( ( tail = reqsize % SIZE_ALIGN ) != 0 )
  {
    reqsize += SIZE_ALIGN - tail;
  }

  LastKnownGood = NULL;

  // ждём освобождения мьютекса
  while ( rtlInterlockedExchange( &mmMutex, TRUE ) )
  {
	  //
	  kos_Pause( 1 );
  }

  //ищем подходящий свободный блок
  if( rootfree != NULL )
  {
    for ( BlockForCheck = rootfree; ; BlockForCheck = BlockForCheck->Next )
    {
      if ( BlockForCheck->Size >= reqsize )
      {
        //нашли
        if ( LastKnownGood != NULL )
        {
          if ( LastKnownGood->Size >= BlockForCheck->Size )
            LastKnownGood = BlockForCheck;
        }
        else
          LastKnownGood = BlockForCheck;
        if ( LastKnownGood->Size == reqsize )
          break;
      }
      if ( BlockForCheck->Next == NULL )
        break;
    }
  }

  if ( LastKnownGood != NULL )
  {
    //проверим найденный блок на возможность деления
    tail = LastKnownGood->Size - reqsize;
    if ( tail >= ( sizeof(MemBlock) + SIZE_ALIGN ) )
    {
      //будем разбивать
      BlockForCheck = (MemBlock *)( ( (Byte *)LastKnownGood ) + tail );
      BlockForCheck->Size = reqsize;
      //вставим занятый блок в начало списка занатых блоков
      if( rootuser != NULL )
      {
        BlockForCheck->Next = rootuser;
        rootuser->Previous = BlockForCheck;
        BlockForCheck->Previous = NULL;
        rootuser = BlockForCheck;
      }
      else
      {
        rootuser = BlockForCheck;
        BlockForCheck->Next = NULL;
        BlockForCheck->Previous = NULL;
      }

      //изменим размер оставшейся части
      LastKnownGood->Size = tail - sizeof(MemBlock);
      address = ( (Byte *)BlockForCheck ) + sizeof(MemBlock);

	  // отпустим мьютекс
      rtlInterlockedExchange( &mmMutex, FALSE );

      return address;
    }
    else
    {
      //перемести блок из очереди свободных в начало очереди занятых
      //сначала выкинем его из очереди свободных
      if ( LastKnownGood->Previous != NULL )
      {
        LastKnownGood->Previous->Next = LastKnownGood->Next;
      }
      else
      {
        //блок стоит в начале очереди
        rootfree = LastKnownGood->Next;
      }
      if( LastKnownGood->Next != NULL )
      {
        LastKnownGood->Next->Previous = LastKnownGood->Previous;
      }
      //теперь вставим его в очередь занятых
      if( rootuser != NULL )
      {
        LastKnownGood->Next = rootuser;
        rootuser->Previous = LastKnownGood;
        LastKnownGood->Previous = NULL;
        rootuser = LastKnownGood;
      }
      else
      {
        rootuser = LastKnownGood;
        LastKnownGood->Next = NULL;
        LastKnownGood->Previous = NULL;
      }
	  //
      address = ( (Byte *)LastKnownGood ) + sizeof(MemBlock);

	  // отпустим мьютекс
      rtlInterlockedExchange( &mmMutex, FALSE );

      return address;
    }
  }
  else
  {
	// надо получить ещё блок памяти
	LastKnownGood = (MemBlock *)kos_malloc(
		(reqsize > 0x10000 - sizeof(MemBlock)) ? (reqsize + sizeof(MemBlock)) : 0x10000);
	if (LastKnownGood != NULL)
	{
		LastKnownGood->Size = reqsize;
		// теперь вставим его в очередь занятых
		LastKnownGood->Next = rootuser;
		LastKnownGood->Previous = NULL;
		if (rootuser != NULL)
			rootuser->Previous = LastKnownGood;
		rootuser = LastKnownGood;
		// а также добавим хвост свежевыделенного большого блока в список свободных
		if (reqsize < 0x10000 - sizeof(MemBlock))
		{
			MemBlock* free = (MemBlock*)((Byte*)LastKnownGood + sizeof(MemBlock) + reqsize);
			free->Next = rootfree;
			free->Previous = NULL;
			if (rootfree != NULL)
				rootfree->Previous = free;
			rootfree = free;
		}
		address = (Byte*)LastKnownGood + sizeof(MemBlock);
		// отпустим мьютекс
		rtlInterlockedExchange(&mmMutex, FALSE);

		return address;
	}
  }

  // отпустим мьютекс
  rtlInterlockedExchange( &mmMutex, FALSE );

  //
  rtlDebugOutString( "allocmem failed." );
  kos_ExitApp();
  //
  return NULL;
}

//
Dword freemem( void *vaddress )
{
  Dword result;

  Byte *checknext, *address = (Byte *)vaddress;
                               
  // ждём освобождения мьютекса
  while ( rtlInterlockedExchange( &mmMutex, TRUE ) )
  {
	  //
	  kos_Pause( 1 );
  }

  MemBlock *released = (MemBlock *)( address - sizeof(MemBlock) );

  result = released->Size;

  //убираем блок из списка занятых
  if ( released->Previous != NULL )
  {
    released->Previous->Next = released->Next;
  }
  else
  {
    rootuser = released->Next;
  }
  if ( released->Next != NULL )
  {
    released->Next->Previous = released->Previous;
  }
  //закинем теперь этот блок в список свободных
  released->Next = rootfree;
  released->Previous = NULL;
  rootfree = released;
  if ( released->Next != NULL )
  {
    released->Next->Previous = released;
  }

  //теперь поищем смежные свободные блоки
  checknext = (Byte *)(rootfree) + ( rootfree->Size + sizeof(MemBlock) );
  //
  for ( released = rootfree->Next; released != NULL; released = released->Next )
  {
    if ( checknext == (Byte *)released )
    {
      //собираем блоки вместе
      //сначала выкинем из очереди свободных
      released->Previous->Next = released->Next;
      if( released->Next != NULL )
      {
        released->Next->Previous = released->Previous;
      }
      //теперь увеличим размер корневого блока
      rootfree->Size += released->Size + sizeof(MemBlock);
      break;
    }
  }
  //если надо, поищем блоки перед текщим.
  checknext = (Byte *)(rootfree);
  //
  if ( released == NULL )
  {
    for ( released = rootfree->Next; released != NULL; released = released->Next )
    {
      if ( checknext == (Byte *)released + ( released->Size + sizeof(MemBlock) ) )
      {
        //собираем блоки вместе
        //увеличим размер блока
        released->Size += rootfree->Size + sizeof(MemBlock);
        //теперь выкинем из очереди свободных
        released->Previous->Next = released->Next;
        if ( released->Next != NULL )
        {
          released->Next->Previous = released->Previous;
        }
        //и закинем его в начало очереди вместо присоединённого блока из корня списка
        if ( rootfree->Next != NULL )
        {
          rootfree->Next->Previous = released;
        }
        released->Next = rootfree->Next;
        released->Previous = NULL;
        rootfree = released;
        break;
      }
    }
  }

  // отпустим мьютекс
  rtlInterlockedExchange( &mmMutex, FALSE );

  return result;
}

