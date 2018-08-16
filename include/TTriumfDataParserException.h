#ifndef TTRIUMFDATAPARSEREXCEPTION_H
#define TTRIUMFDATAPARSEREXCEPTION_H
/** \addtogroup Sorting
 *  @{
 */

/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////

#include <exception>
#include <string>

#include "TTriumfDataParser.h"

class TTriumfDataParserException : public std::exception {
public:
   TTriumfDataParserException(TTriumfDataParser::EDataParserState state, int failedWord, bool multipleErrors);
   ~TTriumfDataParserException() override;

   const char* what() const noexcept override;

   int                                 GetFailedWord() { return fFailedWord; }
   TTriumfDataParser::EDataParserState GetParserState() { return fParserState; }
   bool                                GetMultipleErrors() { return fMultipleErrors; }

private:
   TTriumfDataParser::EDataParserState fParserState;
   int                                 fFailedWord;
   bool                                fMultipleErrors;
   std::string                         fMessage;
};
/*! @} */
#endif
