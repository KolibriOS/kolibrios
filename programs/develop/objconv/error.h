/****************************   error.h   ************************************
* Author:        Agner Fog
* Date created:  2006-07-15
* Last modified: 2006-07-15
* Project:       objconv
* Module:        error.h
* Description:
* Header file for error handler error.cpp
*
* Copyright 2006-2008 GNU General Public License http://www.gnu.org/licenses
*****************************************************************************/
#ifndef OBJCONV_ERROR_H
#define OBJCONV_ERROR_H

// Structure for defining error message texts
struct SErrorText {
   int  ErrorNumber;    // Error number
   int  Status;         // bit 0-3 = severity: 0 = ignore, 1 = warning, 2 = error, 9 = abort
                        // bit 8   = error number not found
   char const * Text;   // Error text
};

// General error routine for reporting warning and error messages to STDERR output
class CErrorReporter {
public:
   CErrorReporter();    // Default constructor
   static SErrorText * FindError(int ErrorNumber); // Search for error in ErrorTexts
   void submit(int ErrorNumber); // Print error message
   void submit(int ErrorNumber, int extra); // Print error message with extra info
   void submit(int ErrorNumber, int, int);  // Print error message with two extra numbers inserted
   void submit(int ErrorNumber, char const * extra); // Print error message with extra info
   void submit(int ErrorNumber, char const *, char const *); // Print error message with two extra text fields inserted
   void submit(int ErrorNumber, int, char const *); // Print error message with two extra text fields inserted
   int Number();        // Get number of errors
   int GetWorstError(); // Get highest warning or error number encountered
   void ClearError(int ErrorNumber); // Ignore further occurrences of this error
protected:
   int NumErrors;       // Number of errors detected
   int NumWarnings;     // Number of warnings detected
   int WorstError;      // Highest error number encountered
   int MaxWarnings;     // Max number of warning messages to pring
   int MaxErrors;       // Max number of error messages to print
   void HandleError(SErrorText * err, char const * text); // Used by submit function
};

#ifndef OBJCONV_ERROR_CPP
extern CErrorReporter err;  // Error handling object is in error.cpp
extern SErrorText ErrorTexts[]; // List of error texts
#endif

#endif // #ifndef OBJCONV_ERROR_H
