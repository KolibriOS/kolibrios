/****************************  containers.h   ********************************
* Author:        Agner Fog
* Date created:  2006-07-15
* Last modified: 2007-02-01
* Project:       objconv
* Module:        containers.h
* Description:
* Header file for container classes and dynamic memory allocation
*
* Copyright 2006-2008 GNU General Public License http://www.gnu.org/licenses
*****************************************************************************/

/*****************************************************************************
This header file declares various container classes for dynamic allocation
of memory for files and other types of data with unpredictable sizes.
These classes have private access to the memory buffer in order to prevent 
memory leaks. It is important to use these classes for all dynamic memory
allocation.

The class CMemoryBuffer and its descendants are used for many purposes of
storage of data with a size that is not known in advance. CMemoryBuffer
allows the size of its data to grow when new data are appended with the
Push() member function.

The class CFileBuffer, which is derived from CMemoryBuffer, is used for 
reading, writing and storing object files and other files.

There are many different classes for different things you can do with
an object file. These classes, declared in converters.h, are all
descendants of CFileBuffer. It is possible to transfer a data buffer
from one object to another by the operator

       A >> B

where A and B are both objects of classes that descend from CFileBuffer.
The file buffer that was owned by A is transferred to B and A is left empty
after the A >> B operation. This makes sure that a memory buffer is always 
owned by one, and only one, object. The opposite operator B << A does the 
same thing.

The >> operator is used whenever we want to do something to a file buffer
that requires a specialized class. The file buffer is transferred from the
object that owns it to an object of the specialized class and transferred
back again to the original owner when the object of the specialized class 
has done its job.

You may say that the descendants of CFileBuffer have a chameleonic nature:
You can change the nature of a piece of data owned by an object by 
transferring it to an object of a different class. This couldn't be done
by traditional polymorphism because it is not possible to change the class
of an object after it is created, and there are too many different things 
you can do with object files for a single class to handle them all.

The container class CMemoryBuffer is useful for storing data of mixed types.
Data of arbitrary type can be accessed by Get<type>(offset) or by
Buf() + offset.

If all items in a dynamic array are of the same type then it is easier to
use one of the template classes CArrayBuf<> or CSList<>. These can be
used in the same way as normal arrays with the operator [].
CArrayBuf<> and CSList<> both have a member function SetNum() to allocate 
the size. The size of CArrayBuf<> can be set only once, while the size of 
CSList<> can be changed at any time. CSList<> also has a member function 
Push() that adds records sequentially. CSList can be sorted if operators 
< and == are defined for the record type.

Warning:
It is necessary to use CArrayBuf<> rather than CSList<> if the record type
has a constructor or destructor.

Warning: 
It is not safe to make pointers to data inside a dynamic array of type 
CMemoryBuffer or CSList<> because the buffer may be re-allocated when the 
size grows. Such pointers will only work if we are finished with all push 
operations. It is safer to address data inside the buffer by their index
or offset relative to the buffer.

*****************************************************************************/

#ifndef CONTAINERS_H
#define CONTAINERS_H

extern CErrorReporter err;                       // Defined in error.cpp

class CFileBuffer;                               // Declared below

void operator >> (CFileBuffer & a, CFileBuffer & b); // Transfer ownership of buffer and other properties

// Class CMemoryBuffer makes a dynamic array which can grow as new data are
// added. Used for storage of files, file sections, tables, etc.
class CMemoryBuffer {
public:
   CMemoryBuffer();                              // Constructor
   ~CMemoryBuffer();                             // Destructor
   void SetSize(uint32 size);                    // Allocate buffer of specified size
   void SetDataSize(uint32 size);                // Claim space as a data
   uint32 GetDataSize()  {return DataSize;};     // File data size
   uint32 GetBufferSize(){return BufferSize;};   // Buffer size
   uint32 GetNumEntries(){return NumEntries;};   // Get number of entries
   uint32 Push(void const * obj, uint32 size);   // Add object to buffer, return offset
   uint32 PushString(char const * s);            // Add ASCIIZ string to buffer, return offset
   uint32 GetLastIndex();                        // Index of last object pushed (zero-based)
   void Align(uint32 a);                         // Align next entry to address divisible by a
   int8 * Buf() {return buffer;};                // Access to buffer
   template <class TX> TX & Get(uint32 Offset) { // Get object of arbitrary type from buffer
      if (Offset >= DataSize) {err.submit(2016); Offset = 0;} // Offset out of range
      return *(TX*)(buffer + Offset);}
private:
   CMemoryBuffer(CMemoryBuffer&);                // Make private copy constructor to prevent copying
   int8 * buffer;                                // Buffer containing binary data. To be modified only by SetSize and operator >>
   uint32 BufferSize;                            // Size of allocated buffer ( > DataSize)
protected:
   uint32 NumEntries;                            // Number of objects pushed
   uint32 DataSize;                              // Size of data, offset to vacant space
   friend void operator >> (CFileBuffer & a, CFileBuffer & b); // Transfer ownership of buffer and other properties
};

static inline void operator << (CFileBuffer & b, CFileBuffer & a) {a >> b;} // Same as operator << above


// Class CFileBuffer is used for storage of input and output files
class CFileBuffer : public CMemoryBuffer {
public:
   CFileBuffer();                                // Default constructor
   CFileBuffer(char const * filename);           // Constructor
   void Read(int IgnoreError = 0);               // Read file into buffer
   void Write();                                 // Write buffer to file
   int  GetFileType();                           // Get file format type
   void SetFileType(int type);                   // Set file format type
   void Reset();                                 // Set all members to zero
   static char const * GetFileFormatName(int FileType); // Get name of file format type
   char const * FileName;                        // Name of input file
   char const * OutputFileName;                  // Output file name
   int WordSize;                                 // Segment word size (16, 32, 64)
   int FileType;                                 // Object file type
   int Executable;                               // File is executable
   char * SetFileNameExtension(const char * f);  // Set file name extension according to FileType
protected:
   void GetOMFWordSize();                        // Determine word size for OMF file
   void CheckOutputFileName();                   // Make output file name or check that requested name is valid
};


// Class CTextFileBuffer is used for building text files
class CTextFileBuffer : public CFileBuffer {
public:
   CTextFileBuffer();                            // Constructor
   void Put(const char * text);                  // Write text string to buffer
   void Put(const char character);               // Write single character to buffer
   void NewLine();                               // Add linefeed
   void Tabulate(uint32 i);                      // Insert spaces until column i
   int  LineType;                                // 0 = DOS/Windows linefeeds, 1 = UNIX linefeeds
   void PutDecimal(int32 x, int IsSigned = 0);   // Write decimal number to buffer
   void PutHex(uint8  x, int MasmForm = 0);      // Write hexadecimal number to buffer
   void PutHex(uint16 x, int MasmForm = 0);      // Write hexadecimal number to buffer
   void PutHex(uint32 x, int MasmForm = 0);      // Write hexadecimal number to buffer
   void PutHex(uint64 x, int MasmForm = 0);      // Write hexadecimal number to buffer
   void PutFloat(float x);                       // Write floating point number to buffer
   void PutFloat(double x);                      // Write floating point number to buffer
   uint32 GetColumn() {return column;}           // Get column number
protected:
   uint32 column;                                // Current column
private:
   uint32 PushString(char const * s){return 0;}; // Make PushString private to prevent using it
};


// Class CArrayBuf<RecordType> is used for dynamic arrays.
// The size of the array can be set only once.
// Use CArrayBuf rather than one of the other container classes if RecordType
// has a constructor or destructor.
template <class RecordType>
class CArrayBuf {
private:
   RecordType * buffer;                          // Dynamically allocated memory
   uint32 num;                                   // Number of entries in array
   CArrayBuf (CArrayBuf &);                      // Make private copy constructor to prevent copying
public:
   CArrayBuf() {                                 // Default constructor
      num = 0;
   }
   ~CArrayBuf() {                                // Destructor
      if (num) delete[] buffer;                  // Deallocate memory. Will call RecordType destructor if any
   }
   void SetNum(uint32 n) {                       // Set size of array. May be called only once!
      if (n <= num) return;                      // Already allocated
      if (num) {
         err.submit(9004);                       // Cannot resize because items may have destructors
      }
      else {
         buffer = new RecordType[n];             // Allocate memory. Will call RecordType constructor if any
         if (!buffer) {
            err.submit(9006);                    // Memory allocation failed
         }
         else {
            num = n;                             // Save size
            memset(buffer, 0, n*sizeof(RecordType));// Initialize to zero
         }
      }
   }
   uint32 GetNumEntries() {
      return num;                                // Read size
   }
   RecordType & operator[] (uint32 i) {          // Access array element [i]
      if (i >= num) {
         err.submit(9003);  i = 0;               // Error: index out of range
      }
      return buffer[i];
   }
   void SetZero() {                              // Set all items in array to 0
      memset(buffer, 0, num*sizeof(RecordType)); // Warning: overwrites all members of RecordType with 0
   }
};


// Class CSList<RecordType> is used for dynamic arrays where all records
// have the same type RecordType. The list can be sorted if desired.
//
// An array defined as 
//       CSList<RecordType> list; 
// can be used in several ways:
//
// 1. The size can be set with list.SetNum(n) where n is the maximum number of 
//    entries. New entries can then be added in random order with list[i] = x;
//    where i < n. Unused entries will be zero.
// 2. Entries can be added sequentially with 
//    list.Push(x);
//    The first entry will be list[0]
// 3. Entries added with method 1 or 2 can be sorted in ascending order by 
//    calling list.Sort();
// 4. The list can be kept sorted at all times if records are added with 
//    list.PushSort(x);
//    The list will be kept sorted in ascending order, provided that it
//    was sorted before the call to PushSort.
// 5. The list can be kept sorted at all times and without duplicates if 
//    records are added with list.PushUnique(x);
//    The list will be sorted and without duplicates after PushUnique if
//    it was so before the call to PushUnique.
// 6. Entries can be read at all times as x = list[i];
//    An error will be submitted if i >= list.GetNumEntries()
// 7. A sorted list can be searched for entry x by i = list.FindFirst(x);
//    or i = list.Exists(x);
//
// Requirements:
// RecordType can be a simple type, a structure or a class.
// If RecordType has a constructor or destructor then they will not be
// called properly. Use CArrayBuf instead of CSList if RecordType has
// a constructor or destructor.
// The operator < const must be defined for RecordType if any of the sorting 
// features are used, i.e. Sort(), PushSort(), FindFirst(), Exists().
//
// Example:
// struct S1 {                                   // Define RecordType
//    int Index;
//    int operator < (S1 const & x) const {      // Define operator <
//       return Index < x.Index;}
// };
// CSList<S1> list;                              // Make list
// S1 a;  a.Index = 5;                           // Make record
// list.PushUnique(a);                           // Put record into list

template <class RecordType> 
class CSList : private CMemoryBuffer {
public:
   void Push(RecordType const & x) {
      // Add member to list
      CMemoryBuffer::Push(&x, sizeof(x));
   }
   void PushZero() {
      // Add blank entry to list
      CMemoryBuffer::Push(0, sizeof(RecordType));
   }
   void SetNum(uint32 n) {
      // Reserve space for n entries. Fill with zeroes
      SetSize(n * sizeof(RecordType));
      NumEntries = n;  DataSize = n * sizeof(RecordType);
   }
   uint32 GetNumEntries() {
      // Get number of entries
      return NumEntries;
   }
   RecordType & operator[] (uint32 i) {
      // Get entries by operator [] as for an array
      if (i >= NumEntries) {
         err.submit(9003); i = 0;}               // Error: index out of range
      return *(RecordType*)(Buf() + i * sizeof(RecordType));
   }
   void Sort() {                                 
      // Sort list by ascending RecordType items
      // Operator < must be defined for RecordType
      // Simple Bubble sort:
      int32 i, j;
      RecordType temp, * p1, * p2;
      for (i = 0; i < (int32)NumEntries; i++) {
         for (j = 0; j < (int32)NumEntries - i - 1; j++) {
            p1 = (RecordType*)(Buf() + j * sizeof(RecordType));
            p2 = (RecordType*)(Buf() + (j+1) * sizeof(RecordType));
            if (*p2 < *p1) {                     
               // Swap records
               temp = *p1;  *p1 = *p2;  *p2 = temp;
            }
         }
      }
   }
   int32 FindFirst(RecordType const & x) {
      // Returns index to first record >= x.
      // Returns 0 if x is smaller than all entries.
      // Returns NumEntries if x is bigger than all entries. Note that this
      // is not a valid index into the list.
      // List must be sorted before calling FindFirst
      uint32 a = 0;                              // Start of search interval
      uint32 b = NumEntries;                     // End of search interval + 1
      uint32 c = 0;                              // Middle of search interval
      // Binary search loop:
      while (a < b) {
         c = (a + b) / 2;
         if ((*this)[c] < x) {
            a = c + 1;}
         else {
            b = c;}
      }
      return (int32)a;
   }
   int32 Exists(RecordType const & x) {
      // Returns the record number if a record equal to x exists in the list.
      // Returns -1 if not. The list must be sorted before calling Exists.
      // Two records a and b are assumed to be equal if !(a < b || b < a)
      uint32 i = FindFirst(x);
      if (i == NumEntries) return -1;
      if (x < (*this)[i]) return -1; else return i;
   }
   int32 PushSort(RecordType const & x) {
      // Add member to list and keep the list sorted.
      // If the list is sorted before calling PushSort then it will also be 
      // sorted after the call. If x is equal to an existing entry then x
      // will be inserted before the existing entry.
      // Operator < must be defined for RecordType.
      int32 i = FindFirst(x);                    // Find where to insert x
      int32 RecordsToMove = (int32)NumEntries-i; // Number of records to move
      SetNum(NumEntries + 1);                    // Make space for one more record
      // Move subsequent entries up one place
      if (RecordsToMove > 0) {
         memmove(Buf() + i * sizeof(RecordType) + sizeof(RecordType), 
            Buf() + i * sizeof(RecordType),
            RecordsToMove * sizeof(RecordType));
      }
      // Insert x at position i
      (*this)[i] = x;
      return i;
   }
   int32 PushUnique(RecordType const & x) {
      // Add member to list and keep the list sorted. Avoids duplicate entries.
      // PushUnique will insert x in the list and keep the list sorted.
      // If an entry equal to x already exists in the list then x is not 
      // inserted, and the return value will be the index to the existing entry.
      // If no entry equal to x existed then x is inserted and the return 
      // value is the index to the new entry.
      // This list must be sorted and without duplicates before calling 
      // PushUnique.
      // Operator < must be defined for RecordType.
      int32 i = FindFirst(x);                    // Find where to insert x
      if (i < (int32)NumEntries && !(x < (*this)[i])) {
         return i;                               // Duplicate found. Return index
      }
      int32 RecordsToMove = (int32)NumEntries-i; // Number of records to move
      SetNum(NumEntries + 1);                    // Make space for one more record
      // Move subsequent entries up one place
      if (RecordsToMove > 0) {
         memmove(Buf() + i * sizeof(RecordType) + sizeof(RecordType), 
            Buf() + i * sizeof(RecordType),
            RecordsToMove * sizeof(RecordType));
      }
      // Insert x at position i
      (*this)[i] = x;
      // Return index
      return i;
   }
   void Remove(uint32 index) {
      // Remove record with this index
      if (index >= NumEntries) return;           // Index out of range
      uint32 RecordsToMove = NumEntries - index - 1; // Number of records to move
      // Move subsequent entries down one place
      if (RecordsToMove > 0) {
         memmove(Buf() + index * sizeof(RecordType),
            Buf() + index * sizeof(RecordType) + sizeof(RecordType),
            RecordsToMove * sizeof(RecordType));
      }
      // Count down number of entries
      SetNum(NumEntries - 1);
   }
};

#endif // #ifndef CONTAINERS_H
