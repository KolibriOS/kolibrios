#define AR_CHUNK_SIZE	64

//
template<class TYPE>
class MCArray
{
protected:
	TYPE * _dataPtr;
	int _elementsCount;
	int _capacity;

public:
	MCArray();
	~MCArray();
	virtual int Add( const TYPE &element );
	TYPE & GetAt( int Ndx );
	TYPE & operator [] ( int Ndx );
	int Find( int startNdx, TYPE & element );
	int RemoveAt( int Ndx );
	void Clear(void);
	int GetCount(void);
};

//


//
template<class TYPE>
MCArray<TYPE>::MCArray()
{
	// устанавливаем переменные
	this->_dataPtr = NULL;
	this->_capacity = 0;
	this->_elementsCount = 0;
}


//
template<class TYPE>
MCArray<TYPE>::~MCArray()
{
	//
	this->_capacity = 0;
	this->_elementsCount = 0;
	//
	if ( this->_dataPtr != NULL )
	{
		delete this->_dataPtr;
	}
}


//
template<class TYPE>
int MCArray<TYPE>::Add( const TYPE &element )
{
	TYPE * dPtr;

	// есть ли место?
	if ( this->_elementsCount >= this->_capacity )
	{
		// занимаем ещё памяти
		dPtr = new TYPE [this->_capacity + AR_CHUNK_SIZE];
		// проверка
		if ( dPtr == NULL )
		{
			//
			return -1;
		}

		if ( this->_capacity > 0 )
		{
			// скопируем существующие данные на новое место
			memcpy( dPtr, this->_dataPtr, sizeof(TYPE) * this->_capacity );
			// удалим старую копию данных
			delete this->_dataPtr;
		}
		// скорректируем размер
		this->_capacity += AR_CHUNK_SIZE;
		// скорректируем указатель на данные
		this->_dataPtr = dPtr;
	}

	// копируем элемент в массив
	memcpy( this->_dataPtr + this->_elementsCount, &element, sizeof(TYPE) );

	// увеличиваем счётчик элементов
	return 	++this->_elementsCount;
}


//
template<class TYPE>
TYPE & MCArray<TYPE>::GetAt( int Ndx )
{
	//assert( Ndx >= 0 && Ndx < this->_elementsCount );
	return this->_dataPtr[Ndx];
}


//
template<class TYPE>
TYPE & MCArray<TYPE>::operator [] ( int Ndx )
{
	return this->GetAt( Ndx );
}


//
template<class TYPE>
int MCArray<TYPE>::Find( int startNdx, TYPE & element )
{
	int i;

	if ( startNdx < 0 || startNdx >= this->_elementsCount )
	{
		return -1;
	}

	for ( i = startNdx; i < this->_elementsCount; i++ )
	{
		if ( element == this->_dataPtr[i] )
		{
			return i;
		}
	}

	return -1;
}


//
template<class TYPE>
int MCArray<TYPE>::RemoveAt( int Ndx )
{
	int mn;

	if ( Ndx < 0 || Ndx >= this->_elementsCount )
	{
		return 0;
	}

	mn = this->_elementsCount - Ndx;

	if ( mn != 1 )
	{
		memcpy( this->_dataPtr + Ndx, this->_dataPtr + Ndx + 1, sizeof(TYPE) * ( mn - 1 ) );
	}

	this->_elementsCount--;
	return 1;
}


//
template<class TYPE>
void MCArray<TYPE>::Clear()
{
	this->_elementsCount = 0;
}

//
template<class TYPE>
int MCArray<TYPE>::GetCount()
{
	return this->_elementsCount;
}
