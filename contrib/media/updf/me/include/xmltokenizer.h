
#ifndef _xmltokenizer_h_
#define _xmltokenizer_h_

#include "yacasbase.h"
#include "tokenizer.h"

class XmlTokenizer : public LispTokenizer
{
public:
  XmlTokenizer() {}
  /// NextToken returns a string representing the next token,
  /// or an empty list.
  virtual LispString * NextToken(LispInput& aInput,
                                  LispHashTable& aHashTable);
  virtual ~XmlTokenizer();
};

#endif

