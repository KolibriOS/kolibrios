/** \file lispparser.h
 *  parsing and printing in the old-fashioned lisp style
 *
 */

#ifndef __lispparser_h__
#define __lispparser_h__

#include "yacasbase.h"
#include "lispobject.h"
#include "tokenizer.h"
#include "lispio.h"
#include "lisphash.h"
#include "evalfunc.h"
class LispParser : public YacasBase
{
public:
    LispParser(LispTokenizer& aTokenizer, LispInput& aInput,
               LispEnvironment& aEnvironment);
    virtual ~LispParser();
    virtual void Parse(LispPtr& aResult );
protected:
    void ParseList(LispPtr& aResult);
    void ParseAtom(LispPtr& aResult,LispString * aToken);

public:
    LispTokenizer& iTokenizer;
    LispInput& iInput;
    LispEnvironment& iEnvironment;
    LispInt iListed;
};




class LispPrinter : public YacasBase
{
public:
    virtual ~LispPrinter();
    virtual void Print(LispPtr& aExpression, LispOutput& aOutput, LispEnvironment& aEnvironment);
    virtual void RememberLastChar(LispChar aChar);
private:
    void PrintExpression(LispPtr& aExpression, LispOutput& aOutput,
                       LispEnvironment& aEnvironment,
       LispInt aDepth=0);

    void Indent(LispOutput& aOutput, LispInt aDepth);
};


#endif
