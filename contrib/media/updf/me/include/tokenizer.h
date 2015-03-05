/** \file tokenizer.h
 * definitions of input output classes that read and write from string.
 */


#ifndef __tokenizer_h__
#define __tokenizer_h__

#include "yacasbase.h"
#include "lispstring.h"
#include "lispio.h"
#include "lisphash.h"
class LispTokenizer : public YacasBase
{
public:
  LispTokenizer() : iToken() {}
  /// NextToken returns a string representing the next token,
  /// or an empty list.
  virtual LispString * NextToken(LispInput& aInput,
                                  LispHashTable& aHashTable);
  virtual ~LispTokenizer(){}
protected:
  LispString iToken; //Can be used as a token container.
};

class CommonLispTokenizer : public LispTokenizer
{
public:
    virtual LispString * NextToken(LispInput& aInput,
                                    LispHashTable& aHashTable);
};


// utility functions
LispBoolean IsDigit(LispChar c);
LispBoolean IsAlpha(LispChar c);
LispBoolean IsAlNum(LispChar c);
LispBoolean IsSymbolic(LispChar c);



#endif


