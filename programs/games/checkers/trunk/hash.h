#if !defined(HASH_TABLE_H)
#define HASH_TABLE_H

template <class T, class TypeString, class HashFunction>
class THashTable
{
public:
   THashTable(int _m = -1) {MemInit(_m);}
   ~THashTable() {null();}

   int hash1(const TypeString &x) const {return func1(x);}
   int hash2(const TypeString &x) const {return 2*func2(x) + 1;}

   void null();
   int IsNull() const {return M >= 0;}
   int NumElem() const {return n;}
   int MaxNumElem() const {return stack_size;}
   int GetM() const {return M;}
   int TableSize() const {return 1 << M;}

   void resize(int _m);
   void push(const T &x);
   T *find(const TypeString &x);
   void pop();

   T &last() {return stack[n-1];}
   const T &last() const {return stack[n-1];}
   T &first() {return stack[0];}
   const T &first() const {return stack[0];}
   T &operator[](int i) {return stack[i];}
   const T &operator[](int i) const {return stack[i];}
   T *operator()() {return stack;}
   const T *operator()() const {return stack;}
protected:
   void _push(const T &x);
   int _find_pointer(const T *p) const;
   int _find(const TypeString &x) const;
   void MemInit(int _m);
protected:
   int M;
   int stack_size;
   int n;
   T **table;
   T *stack;
protected:
   HashFunction func1, func2;
};

template <class T, class TypeString, class HashFunction>
void THashTable<T, TypeString, HashFunction>::null()
{
  if (table) delete[] table;
  if (stack) delete[] stack;
  M = -1;
  table = 0;
  stack = 0;
  stack_size = 0;
  n = 0;
  func1.init(-1);
  func2.init(-1);
}

template <class T, class TypeString, class HashFunction>
void THashTable<T, TypeString, HashFunction>::resize(int _m)
{
  delete[] table;
  T *stp = stack;
  int np = n;
  MemInit(_m);
  for (int i = 0; i < np && n < stack_size; i++) _push(stp[i]);
  if (stp) delete[] stp;
}

template <class T, class TypeString, class HashFunction>
inline void THashTable<T, TypeString, HashFunction>::push(const T &x)
{
  if (n == stack_size) resize(M + 1);
  _push(x);
}

template <class T, class TypeString, class HashFunction>
inline T *THashTable<T, TypeString, HashFunction>::find(const TypeString &x)
{
  int i = _find(x);
  if (i >= 0) return table[i];
  else return 0;
}

template <class T, class TypeString, class HashFunction>
inline void THashTable<T, TypeString, HashFunction>::pop()
{
  if (n > 0)
  {
    n--;
    int i = _find_pointer(stack + n);
    if (i >= 0) table[i] = NULL;
  }
}

template <class T, class TypeString, class HashFunction>
void THashTable<T, TypeString, HashFunction>::_push(const T &x)
{
  int h1 = hash1(x);
  int h2 = hash2(x);
  int i = h1;
  stack[n] = x;
  do
  {
    if (table[i] == NULL)
    {
      table[i] = stack + n;
      break;
    }
    i = (i + h2) & ((1 << M) - 1);
  }
  while (i != h1);
  n++;
}

template <class T, class TypeString, class HashFunction>
int THashTable<T, TypeString, HashFunction>::_find_pointer(const T *p) const
{
  if (n > 0)
  {
    int h1 = hash1(*p);
    int h2 = hash2(*p);
    int i = h1;
    do
    {
      if (table[i] == NULL) break;
      if (table[i] == p) return i;
      i = (i + h2) & ((1 << M) - 1);
    }
    while (i != h1);
  }
  return -1;
}

template <class T, class TypeString, class HashFunction>
int THashTable<T, TypeString, HashFunction>::_find(const TypeString &x) const
{
  if (n > 0)
  {
    int h1 = hash1(x);
    int h2 = hash2(x);
    int i = h1;
    do
    {
      if (table[i] == NULL) break;
      if ((*table[i]) == x) return i;
      i = (i + h2) & ((1 << M) - 1);
    }
    while (i != h1);
  }
  return -1;
}

template <class T, class TypeString, class HashFunction>
void THashTable<T, TypeString, HashFunction>::MemInit(int _m)
{
  M = _m;
  if (M < 0)
  {
    M = -1;
    stack_size = 0;
    table = 0;
    stack = 0;
    n = 0;
    func1.init(-1);
    func2.init(-1);
  }
  else
  {
    if (M < 3) M = 3;
    stack_size = (1 << M) / 3;
    table = new T*[1 << M];
    for (int i = 0; i < (1 << M); i++) table[i] = NULL;
    stack = new T[stack_size];
    n = 0;
    func1.init(M);
    func2.init(M-1);
  }
}

#endif  // HASH_TABLE_H
