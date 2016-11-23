

struct TCollection{
  int Count;//itemcunt
  int Delta;//increase buffer Delta
  int Items;//pointer to array of item pointers
  int Limit;//currently allocated size (in elements) of Items list

  TCollection( int ALimit,ADelta );
  ~TCollection( void );

  int  At( int Index );
  void AtDelete( int Index );
  void AtFree( int Index );
  void AtInsert( int Index, Item );
  void DeleteAll( void );
  void Free( int Item );
  void FreeAll( void );
  void FreeItem( int Item );
  void Insert( int Item );
  void SetLimit( int ALimit );
};

:int TCollection::At( int Index )
{
  return DSDWORD[ Index*sizeof(int)+Items ];
}

:void TCollection::AtDelete( int Index )
{
//  if(Index<0)||(Index>=Count){/*Error(coIndexError);*/return;}
  ECX = Count-Index-1;
  EDI = Index*sizeof(int)+Items; ESI = EDI+4; $cld; $rep;$movsd;
  Count --;
}

:void TCollection::AtFree( int Index )
{
  int Item = At( Index);
  AtDelete( Index );
  FreeItem( Item );
}

:void TCollection::AtInsert( int Index, Item )
{
  $pushfd
  IF( Count == Limit )
  {
    SetLimit( Count+Delta );
  }
  ECX = Count-Index;
  EDI = Count*sizeof(int)+Items;
  ESI = EDI-4; $std $rep $movsd $cld
  DSDWORD[ EDI ] = Item;
  Count ++;
  $popfd
}

:void TCollection::DeleteAll()
{
  Count=0;
}

:void TCollection::Free( int Item )
{
  Delete( Item );
  FreeItem( Item );
}

:void TCollection::FreeAll( void )
{
  int I;
  FOR( I = 0; I < Count; I ++ )FreeItem( At(I) );
  Count = 0;
}

:void TCollection::FreeItem( int Item )
{
  IF( Item )free( Item );//+++Dispose(PObject(Item), Done);
}

:void TCollection::Insert( int Item )
{
  AtInsert( Count, Item );
}

:void TCollection::SetLimit( int ALimit )
{
  int AItems;

  IF( ALimit < Count )ALimit = Count;
  IF( ALimit != Limit )
  {
    IF( !ALimit ) AItems = 0;
    ELSE
    {
      AItems = malloc(ALimit*sizeof(int) );
      IF( Count )&&( Items )
      {
        CopyMemory( AItems, Items, Limit*sizeof(int) );
      }
    }
    IF( Limit )free( Items );
    Items = AItems;
    Limit = ALimit;
  }
}

:TCollection::TCollection( int ALimit, ADelta )
{
  Items = 0;
  Count = 0;
  Limit = 0;
  Delta = ADelta;
  SetLimit( ALimit );
  return this;
}

:TCollection::~TCollection( void )
{
  FreeAll();
  SetLimit(0);
}

struct TSortedCollection:TCollection
{
  int  comparemethod;
  int  Duplicates;

  TSortedCollection( int ALimit, ADelta );

  int  Compare( int Key1, Key2 );
  void Insert( int Item );
  int  Search( int Key, Index );
};

:int TSortedCollection::Compare( int Key1, Key2 )
{
  strcmp( Key1, Key2 );
}

:TSortedCollection::TSortedCollection( int ALimit, ADelta )
:TCollection( ALimit, ADelta );
{
  comparemethod=#Compare;
  return this;
}

:void TSortedCollection::Insert( int Item )
{
  int i;
  IF( !Search(/*KeyOf(*/Item/*)*/,#i) ) || ( Duplicates )AtInsert( i, Item );
}


:int TSortedCollection::Search( int Key, Index )
{
  int L, H, I, C;
  int S;

  S = 0;
  L = 0;
  H = Count-1;
  WHILE( L <= H )
  {
    I = L+H >> 1;
    ECX = I*sizeof(int)+Items;
    comparemethod( this, /*KeyOf(*/DSDWORD[ECX]/*)*/, Key );
    C = EAX;
    IF( C < 0 ){ L = I+1; }
    ELSE
    {
      H = I-1;
      IF( !C ){
        S = 1;
        IF( !Duplicates )L = I;
      }
    }
  }
  DSDWORD[ Index ]=L;
  return S;
}
