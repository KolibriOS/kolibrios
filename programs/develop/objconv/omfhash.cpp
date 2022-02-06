/****************************  omfhash.cpp  **********************************
* Author:        Agner Fog
* Date created:  2007-02-14
* Last modified: 2007-02-14
* Project:       objconv
* Module:        omfhash.cpp
* Description:
* This module contains code for searching and making hash tables for OMF
* libraries.
*
* Copyright 2007-2008 GNU General Public License http://www.gnu.org/licenses
*****************************************************************************/

#include "stdafx.h"

void COMFHashTable::Init(SOMFHashBlock * blocks, uint32 NumBlocks) {
   // Initialize
   this->blocks = blocks;                        // Pointer to blocks
   this->NumBlocks = NumBlocks;                  // Number of blocks
   String = 0;
   StringLength = 0;
}

// Rotate right 16-bit word
uint16 RotR(uint16 x, uint16 bits) {
   return (x >> bits) | (x << (16 - bits));
}

// Rotate left 16-bit word
uint16 RotL(uint16 x, uint16 bits) {
   return (x << bits) | (x >> (16 - bits));
}

void COMFHashTable::MakeHash(int8 * name) {
   // Compute hash according to the official algorithm
   uint8 * pb;                                   // Pointer for forward scan through string
   uint8 * pe;                                   // Pointer for backwards scan through string
   uint16 c;                                     // Current character converted to lower case
   uint16 BlockX;                                // Calculate block hash
   uint16 BucketX;                               // Calculate block hash
   String = (uint8*)name;                        // Type cast string to unsigned char *
   StringLength = (uint32)strlen(name);
   if (StringLength > 255 || StringLength == 0) {
      // String too long
      err.submit(1204, name);                    // Warning: truncating
      StringLength = 255;
      String[StringLength] = 0;                  // Truncation modifies string source!
   }
   String = (uint8*)name;                        // Type cast to unsigned characters
   pb = String;                                  // Initialize pointer for forward scan
   pe = String + StringLength;                   // Initialize pointer for backward scan
   BlockX = BucketD = StringLength | 0x20;       // Initialize left-to-right scan
   BucketX = BlockD = 0;                         // Initialize right-to-left scan

   // Scan loop
   while (1) {
      c = *(--pe) | 0x20;                        // Read character for backward scan, make lower case
      BucketX = RotR(BucketX, 2) ^ c;            // Rotate, XOR
      BlockD  = RotL(BlockD,  2) ^ c;            // Rotate, XOR
      if (pe == String) break;                   // Stop loop when backward scan finished
      c = *(pb++) | 0x20;                        // Read character for forward scan, make lower case
      BlockX  = RotL(BlockX,  2) ^ c;            // Rotate, XOR
      BucketD = RotR(BucketD, 2) ^ c;            // Rotate, XOR
   }
   // Make values modulo number of blocks / buckets
   BlockX = BlockX % NumBlocks;
   BlockD = BlockD % NumBlocks;
   if (BlockD == 0) BlockD = 1;
   BucketX = BucketX % OMFNumBuckets;
   BucketD = BucketD % OMFNumBuckets;
   if (BucketD == 0) BucketD = 1;
   StartBlock  = BlockX;
   StartBucket = BucketX;
}


int  COMFHashTable::FindString(uint32 & ModulePage, uint32 & Conflicts) {
   // Search for String. 
   // Returns number of occurrences of String
   // Module receives the module page for the first occurrence
   // Conflicts receives the number of conflicting entries encountered before the match
   uint32 Num = 0;                               // Number of occurrences of string found
   uint16 Block;                                 // Block number
   uint16 Bucket;                                // Bucket number
   uint32 StringIndex;                           // Index to string
   Conflicts = 0;                                // Start counting Conflicts

   Block = StartBlock;
   Bucket = StartBucket;

   // Loop through blocks
   do {

      // Loop through buckets
      do {

         // String index of current bucket
         StringIndex = blocks[Block].b.Buckets[Bucket];
         if (StringIndex == 0) {
            if (blocks[Block].b.FreeSpace < 0xff) {
               // Empty bucket found. End of search
               return Num;
            }
            else {
               // Block is full. Search next block

               // Note: It would be logical to set StartBucket = Bucket
               // here in order to allow all buckets in the next block
               // to be tried, but the official algorithm doesn't seem
               // to do so!?
               // StartBucket = Bucket; 

               break;
            }
         }
         // Bucket contains a string. Is it the same string?
         if (blocks[Block].Strings[StringIndex*2] == StringLength
         && strncmp((int8*)&blocks[Block].Strings[StringIndex*2+1], (int8*)String, StringLength) == 0) {
            // Matching string found
            Num++;
            if (Num == 1) {
               // First occurrence. Save module number
               ModulePage = *(uint16*)&blocks[Block].Strings[StringIndex*2+1+StringLength];
            }
         }
         else {
            // Conflicting string found
            Conflicts++;
         }
         // Next bucket
         Bucket = (Bucket + BucketD) % OMFNumBuckets;
      } while (Bucket != StartBucket);

      // Next block
      Block = (Block + BlockD) % NumBlocks;
   }  while (Block != StartBlock);
   // Finished searching all blocks and buckets
   return Num;
}

int COMFHashTable::InsertString(uint16 & ModulePage) {
   // Insert string in hash table.
   // Parameter:
   // ModulePage = module address / page size
   // Return value:
   // 0 if success, 
   // 1 if identical string allready exists in the table. New string will not be entered.
   //   ModulePage will receive the module page of the existing string in this case.
   // 2 if table is full,
   uint16 Block;                                 // Block number
   uint16 Bucket;                                // Bucket number
   uint32 StringIndex;                           // Index to string space
   uint32 StringOffset;                          // Offset to string from begin of block
   uint32 SpaceRequired;                         // Space required to store string

   SpaceRequired = StringLength + 3;             // Space for string + stringlength + module index
   SpaceRequired = (SpaceRequired + 1) & uint32(-2);// Round up to nearest even

   Block = StartBlock;
   Bucket = StartBucket;

   // Loop through blocks
   do {

      // Loop through buckets
      do {

         // String index of current bucket
         StringIndex = blocks[Block].b.Buckets[Bucket];
         if (StringIndex == 0) {
            // Found empty bucket. Check if block has enough free space
            if (uint32(OMFBlockSize) - blocks[Block].b.FreeSpace * 2 < SpaceRequired) {
               // Not enough space in block.
               // Continue with same bucket in next block.

               // Note: It would be logical to set StartBucket = Bucket
               // here in order to allow all buckets in the next block
               // to be tried, but the official algorithm doesn't seem
               // to do so!?
               // StartBucket = Bucket; 
               break;
            }
            // Enough space found. Enter string in bucket
            StringIndex = blocks[Block].b.FreeSpace;
            blocks[Block].b.Buckets[Bucket] = StringIndex;
            // Address to store string
            StringOffset = StringIndex * 2;
            // Store string length
            blocks[Block].Strings[StringOffset] = (uint8)StringLength;
            // Copy string
            memcpy(blocks[Block].Strings + StringOffset + 1, String, StringLength);
            // Insert module page number
            *(uint16*)(blocks[Block].Strings + StringOffset + 1 + StringLength) = ModulePage;
            // Update free space
            blocks[Block].b.FreeSpace += (uint8)(SpaceRequired / 2);
            // Check if overflow
            if (blocks[Block].b.FreeSpace == 0) blocks[Block].b.FreeSpace = 0xFF;
            // Indicate success
            return 0;
         }
         else {
            // Bucket contains a string. Check if it is the same string
            if (blocks[Block].Strings[StringIndex*2] == StringLength
            && strncmp((int8*)(blocks[Block].Strings+StringIndex*2+1), (int8*)String, StringLength) == 0) {
               // Identical string found. Return module index for existing string entry
               ModulePage = *(uint16*)(blocks[Block].Strings+StringIndex*2+1+StringLength);
               // Indicate failure
               return 1;
            }
         }
         // Bucket was full. Go to next bucket
         Bucket = (Bucket + BucketD) % OMFNumBuckets;
      } while (Bucket != StartBucket);

      // If we got here, we have found no empty bucket in the block or 
      // there was not enough string space in the block.
      // We need to mark the block as full to tell the linker to
      // continue in next block when searching for this string
      // Whether the block has any empty buckets or not
      blocks[Block].b.FreeSpace = 0xFF;

      // Go to next block
      Block = (Block + BlockD) % NumBlocks;
   }  while (Block != StartBlock);

   // Finished searching all blocks and buckets
   // No empty space found. Indicate failure:
   return 2;
}


// Table of prime numbers
// You may add more prime numbers if very big library files are needed
static const uint32 PrimeNumbers[] = {
   1, 2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71,
   73, 79, 83, 89, 97, 101, 103, 107, 109, 113, 127, 131, 137, 139, 149, 151, 157,
   163, 167, 173, 179, 181, 191, 193, 197, 199, 211, 223, 227, 229, 233, 239, 241,
   251, 257, 263, 269, 271, 277, 281, 283, 293, 307, 311, 313, 317, 331, 337, 347,
   349, 353, 359, 367, 373, 379, 383, 389, 397, 401, 409, 419, 421, 431, 433, 439,
   443, 449, 457, 461, 463, 467, 479, 487, 491, 499, 503, 509, 521, 523, 541, 547,
   557, 563, 569, 571, 577, 587, 593, 599, 601, 607, 613, 617, 619, 631, 641, 643,
   647, 653, 659, 661, 673, 677, 683, 691, 701, 709, 719, 727, 733, 739, 743, 751,
   757, 761, 769, 773, 787, 797, 809, 811, 821, 823, 827, 829, 839, 853, 857, 859,
   863, 877, 881, 883, 887, 907, 911, 919, 929, 937, 941, 947, 953, 967, 971, 977,
   983, 991, 997, 1009, 1013, 1019, 1021, 1031, 1033, 1039, 1049, 1051, 1061, 1063,
   1069, 1087, 1091, 1093, 1097, 1103, 1109, 1117, 1123, 1129, 1151, 1153, 1163,
   1171, 1181, 1187, 1193, 1201, 1213, 1217, 1223, 1229, 1231, 1237, 1249, 1259,
   1277, 1279, 1283, 1289, 1291, 1297, 1301, 1303, 1307, 1319, 1321, 1327, 1361,
   1367, 1373, 1381, 1399, 1409, 1423, 1427, 1429, 1433, 1439, 1447, 1451, 1453,
   1459, 1471, 1481, 1483, 1487, 1489, 1493, 1499, 1511, 1523, 1531, 1543, 1549,
   1553, 1559, 1567, 1571, 1579, 1583, 1597, 1601, 1607, 1609, 1613, 1619, 1621,
   1627, 1637, 1657, 1663, 1667, 1669, 1693, 1697, 1699, 1709, 1721, 1723, 1733,
   1741, 1747, 1753, 1759, 1777, 1783, 1787, 1789, 1801, 1811, 1823, 1831, 1847,
   1861, 1867, 1871, 1873, 1877, 1879, 1889, 1901, 1907, 1913, 1931, 1933, 1949,
   1951, 1973, 1979, 1987, 1993, 1997, 1999, 2003, 2011, 2017, 2027, 2029, 2039,
   2053, 2063, 2069, 2081, 2083, 2087, 2089, 2099, 2111, 2113, 2129, 2131, 2137,
   2141, 2143, 2153, 2161, 2179, 2203, 2207, 2213, 2221, 2237, 2239, 2243, 2251,
   2267, 2269, 2273, 2281, 2287, 2293, 2297, 2309, 2311, 2333, 2339, 2341, 2347,
   2351, 2357, 2371, 2377, 2381, 2383, 2389, 2393, 2399, 2411, 2417, 2423, 2437,
   2441, 2447, 2459, 2467, 2473, 2477, 2503, 2521, 2531, 2539, 2543, 2549, 2551,
   2557, 2579, 2591, 2593, 2609, 2617, 2621, 2633, 2647, 2657, 2659, 2663, 2671,
   2677, 2683, 2687, 2689, 2693, 2699, 2707, 2711, 2713, 2719, 2729, 2731, 2741,
   2749, 2753, 2767, 2777, 2789, 2791, 2797, 2801, 2803, 2819, 2833, 2837, 2843,
   2851, 2857, 2861, 2879, 2887, 2897, 2903, 2909, 2917, 2927, 2939, 2953, 2957,
   2963, 2969, 2971, 2999, 3001, 3011, 3019, 3023, 3037, 3041, 3049, 3061, 3067,
   3079, 3083, 3089, 3109, 3119, 3121, 3137, 3163, 3167, 3169, 3181, 3187, 3191,
   3203, 3209, 3217, 3221, 3229, 3251, 3253, 3257, 3259, 3271, 3299, 3301, 3307,
   3313, 3319, 3323, 3329, 3331, 3343, 3347, 3359, 3361, 3371, 3373, 3389, 3391,
   3407, 3413, 3433, 3449, 3457, 3461, 3463, 3467, 3469, 3491, 3499, 3511, 3517,
   3527, 3529, 3533, 3539, 3541, 3547, 3557, 3559, 3571, 3581, 3583, 3593, 3607,
   3613, 3617, 3623, 3631, 3637, 3643, 3659, 3671, 3673, 3677, 3691, 3697, 3701,
   3709, 3719, 3727, 3733, 3739, 3761, 3767, 3769, 3779, 3793, 3797, 3803, 3821,
   3823, 3833, 3847, 3851, 3853, 3863, 3877, 3881, 3889, 3907, 3911, 3917, 3919,
   3923, 3929, 3931, 3943, 3947, 3967, 3989, 4001, 4003, 4007, 4013, 4019, 4021,
   4027, 4049, 4051, 4057, 4073, 4079, 4091, 4093, 4099, 4111, 4127, 4129, 4133,
   4139, 4153, 4157, 4159, 4177, 4201, 4211, 4217, 4219, 4229, 4231, 4241, 4243,
   4253, 4259, 4261, 4271, 4273, 4283, 4289, 4297, 4327, 4337, 4339, 4349, 4357,
   4363, 4373, 4391, 4397, 4409, 4421, 4423, 4441, 4447, 4451, 4457, 4463, 4481,
   4483, 4493, 4507, 4513, 4517, 4519, 4523, 4547, 4549, 4561, 4567, 4583, 4591,
   4597, 4603, 4621, 4637, 4639, 4643, 4649, 4651, 4657, 4663, 4673, 4679, 4691,
   4703, 4721, 4723, 4729, 4733, 4751, 4759, 4783, 4787, 4789, 4793, 4799, 4801,
   4813, 4817, 4831, 4861, 4871, 4877, 4889, 4903, 4909, 4919, 4931, 4933, 4937,
   4943, 4951, 4957, 4967, 4969, 4973, 4987, 4993, 4999, 5003, 5009, 5011, 5021
};

// Length of table
static const uint32 PrimeNumbersLen = sizeof(PrimeNumbers)/sizeof(PrimeNumbers[0]);


void COMFHashTable::MakeHashTable(CSList<SStringEntry> & StringEntries, 
CMemoryBuffer & StringBuffer, CMemoryBuffer & OutFile, CLibrary * Library) {
   // Make hash table. Parameters:
   // StringEntries[].String = name of each public symbol as offset into StringBuffer
   // StringEntries[].Member = page address of member = offset / page size
   // StringBuffer = contains all strings
   // OutFile will receive the output hash table

   CSList<SOMFHashBlock> HashTable;              // Hash table
   COMFHashTable TableHandler;                   // Hash table handler
   uint32 NumBlocksI;                            // Number of blocks as index into prime number table
   uint32 BlockI;                                // Block index
   uint32 SymI;                                  // Symbol index
   int8 * String;                                // Symbol name
   uint16 Module1, Module2;                      // Module page = offset / page size
   int    Result;                                // 0 = success

   // Estimate required number of blocks
   NumBlocks = (StringEntries.GetNumEntries() * 8 + StringBuffer.GetDataSize()) / 256;
   // Find nearest prime number >= NumBlocks, but stay within the range from 2 to 251.
   // The minimum NumBlocks is 1, but some systems use 2 as the minimum.
   // The maximum is 251, but some linkers may allow a higher number

   for (NumBlocksI = 1; NumBlocksI < 55; NumBlocksI++) {
      if (PrimeNumbers[NumBlocksI] >= NumBlocks) break;
   }

   // Try if this number of blocks is sufficient
   while (NumBlocksI < PrimeNumbersLen) {

      // Get number of blocks from prime numbers table
      NumBlocks = PrimeNumbers[NumBlocksI];

      // Check if <= 251
      if (NumBlocks > 255) err.submit(1215); // Number of blocks exceeds official limit. May still work with some linkers

      // Allocate space for hash table
      HashTable.SetNum(NumBlocks);
      memset(&HashTable[0], 0, NumBlocks * OMFBlockSize);

      // Initialize hash table handler
      TableHandler.Init(&HashTable[0], NumBlocks);

      // Set free space pointers
      for (BlockI = 0; BlockI < NumBlocks; BlockI++) {
         TableHandler.blocks[BlockI].b.FreeSpace = 19;
      }
      Result = 0;

      // Insert symbols
      // Loop through symbols
      for (SymI = 0; SymI < StringEntries.GetNumEntries(); SymI++) {

         // Symbol name
         String = StringBuffer.Buf() + StringEntries[SymI].String;

         // Module page
         Module1 = Module2 = StringEntries[SymI].Member;

         // Insert name in table
         TableHandler.MakeHash(String);
         Result = TableHandler.InsertString(Module2);

         if (Result == 1) {
            // String already exists
            // Compose error string "Modulename1 and Modulename2"
            char ErrorModuleNames[64];
            strcpy(ErrorModuleNames, Library->GetModuleName(Module1));
            strcpy(ErrorModuleNames + strlen(ErrorModuleNames), " and ");
            strcpy(ErrorModuleNames + strlen(ErrorModuleNames), Library->GetModuleName(Module2));
            // submit error message
            err.submit(1214, String, ErrorModuleNames);
         }
         if (Result == 2) {
            // Table is full. Stop and repeat with a higher NumBlocks
            break;
         }
      } // End of loop through symbols

      if (Result < 2) {
         // Finished with success
         // Store hash table
         OutFile.Push(&HashTable[0], HashTable.GetNumEntries() * OMFBlockSize);
         return;
      }

      // Table is full. Try again with a higher number of blocks
      NumBlocksI++;
   }

   // End of loop through PrimeNumbers table
   err.submit(2605);                             // Failed to make table
}
