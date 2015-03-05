
#ifndef __numbers_h__
#define __numbers_h__

#include "lispenvironment.h"
#include "yacasbase.h"



/// Whether the numeric library supports 1.0E-10 and such.
LispInt NumericSupportForMantissa();

LispObject* GcdInteger(LispObject* int1, LispObject* int2, LispEnvironment& aEnvironment);

LispObject* SinFloat(LispObject* int1, LispEnvironment& aEnvironment,LispInt aPrecision);
LispObject* CosFloat(LispObject* int1, LispEnvironment& aEnvironment,LispInt aPrecision);
LispObject* TanFloat(LispObject* int1, LispEnvironment& aEnvironment,LispInt aPrecision);
LispObject* ArcSinFloat(LispObject* int1, LispEnvironment& aEnvironment,LispInt aPrecision);
LispObject* ExpFloat(LispObject* int1, LispEnvironment& aEnvironment,LispInt aPrecision);
LispObject* LnFloat(LispObject* int1, LispEnvironment& aEnvironment,LispInt aPrecision);

LispObject* SqrtFloat(LispObject* int1, LispEnvironment& aEnvironment,LispInt aPrecision);
LispObject* ModFloat( LispObject* int1, LispObject* int2, LispEnvironment& aEnvironment,
                        LispInt aPrecision);

LispObject* PowerFloat(LispObject* int1, LispObject* int2,
                         LispEnvironment& aEnvironment,LispInt aPrecision);
LispObject* ShiftLeft( LispObject* int1, LispObject* int2, LispEnvironment& aEnvironment,LispInt aPrecision);
LispObject* ShiftRight( LispObject* int1, LispObject* int2, LispEnvironment& aEnvironment,LispInt aPrecision);
LispObject* LispFactorial(LispObject* int1, LispEnvironment& aEnvironment,LispInt aPrecision);



// methods generally useful for all numeric libraries
const unsigned GUARD_BITS = 8;  // we leave this many guard bits untruncated in various situations when we need to truncate precision by hand

template<class T> inline T MAX(T x, T y) { if (x<y) return y; else return x; }
template<class T> inline T MIN(T x, T y) { if (x>y) return y; else return x; }

const long DIST_BITS = 1;  // at least this many bits of difference - used in precision tracking

/// DIST(x, y) returns 1 if abs(x-y) >= DIST_BITS. See documentation for precision tracking.
template<class T> inline T DIST(T x, T y) { return (x>=y+DIST_BITS || y>=x+DIST_BITS) ? 0 : 1; }


/** Base number class.
 */
 

class ANumber;


 
/// Main class for multiple-precision arithmetic.
/// All calculations are done at given precision. Integers grow as needed, floats don't grow beyond given precision.
class BigNumber : public YacasBase
{
public: //constructors
  BigNumber(const LispChar * aString,LispInt aPrecision,LispInt aBase=10);
/// copy constructor
  BigNumber(const BigNumber& aOther);
  // no constructors from int or double to avoid automatic conversions
  BigNumber(LispInt aPrecision = 20);
  ~BigNumber();
  // assign from another number
  void SetTo(const BigNumber& aOther);
  // assign from string, precision in base digits
  void SetTo(const LispChar * aString,LispInt aPrecision,LispInt aBase=10);
    // assign from a platform type
  void SetTo(long value);
  inline void SetTo(LispInt value) { SetTo(long(value)); };
  void SetTo(double value);
public: // Convert back to other types
  /// ToString : return string representation of number in aResult to given precision (base digits)
  void ToString(LispString& aResult, LispInt aPrecision, LispInt aBase=10) const;
  /// Give approximate representation as a double number
  double Double() const;

public://basic object manipulation
  LispBoolean Equals(const BigNumber& aOther) const;
  LispBoolean IsInt() const;
  LispBoolean IsIntValue() const;
  LispBoolean IsSmall() const;
  void BecomeInt();
  void BecomeFloat(LispInt aPrecision=0);
  LispBoolean LessThan(const BigNumber& aOther) const;
public://arithmetic
  /// Multiply two numbers at given precision and put result in *this
  void Multiply(const BigNumber& aX, const BigNumber& aY, LispInt aPrecision);
  /** Multiply two numbers, and add to *this (this is useful and generally efficient to implement).
   * This is most likely going to be used by internal functions only, using aResult as an accumulator.
   */
  void MultiplyAdd(const BigNumber& aX, const BigNumber& aY, LispInt aPrecision);
  /// Add two numbers at given precision and return result in *this
  void Add(const BigNumber& aX, const BigNumber& aY, LispInt aPrecision);
  /// Negate the given number, return result in *this
  void Negate(const BigNumber& aX);
  /// Divide two numbers and return result in *this. Note: if the two arguments are integer, it should return an integer result!
  void Divide(const BigNumber& aX, const BigNumber& aY, LispInt aPrecision);

  /// integer operation: *this = y mod z
  void Mod(const BigNumber& aY, const BigNumber& aZ);

  /// For debugging purposes, dump internal state of this object into a string
  void DumpDebugInfo();

public:
  /// assign self to Floor(aX) if possible
  void Floor(const BigNumber& aX);
  /// set precision (in bits)
  void Precision(LispInt aPrecision);

public:/// Bitwise operations, return result in *this.
  void ShiftLeft( const BigNumber& aX, LispInt aNrToShift);
  void ShiftRight( const BigNumber& aX, LispInt aNrToShift);
  void BitAnd(const BigNumber& aX, const BigNumber& aY);
  void BitOr(const BigNumber& aX, const BigNumber& aY);
  void BitXor(const BigNumber& aX, const BigNumber& aY);
  void BitNot(const BigNumber& aX);
  /// Bit count operation: return the number of significant bits if integer, return the binary exponent if float (shortcut for binary logarithm)
  /// give bit count as a platform integer
  signed long BitCount() const;
 
  /// Give sign (-1, 0, 1)
  LispInt Sign() const;

public:
  inline LispInt GetPrecision() const {return iPrecision;};

private:
  BigNumber& operator=(const BigNumber& aOther)
  {
    // copy constructor not written yet, hence the assert
    LISPASSERT(0);
    return *this;
  }
public:
  ReferenceCount iReferenceCount;
private:
  LispInt iPrecision;

public:
  /// Internal library wrapper starts here.
    inline void SetIsInteger(LispBoolean aIsInteger) {iType = (aIsInteger ? KInt : KFloat);}
    enum ENumType
    {
      KInt = 0,
      KFloat
    };
    ENumType iType;
    ANumber* iNumber;
  /// Internal library wrapper ends here.
};

/// bits_to_digits and digits_to_bits, utility functions
/// to convert the number of digits in some base (usually 10) to bits and back

// lookup table for Ln(n)/Ln(2). This works whether or not we have math.h.
// table range is from 2 to this value:
unsigned log2_table_range();
// convert the number of digits in given base to the number of bits, and back.
// need to round the number of digits.
// These functions only work for aBase inside the allowed table range.
unsigned long digits_to_bits(unsigned long aDigits, unsigned aBase);
unsigned long bits_to_digits(unsigned long abits, unsigned aBase);


#endif













