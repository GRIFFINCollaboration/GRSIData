#ifndef TGRSIDATAPARSEREXCEPTION_H
#define TGRSIDATAPARSEREXCEPTION_H
/** \addtogroup Sorting
 *  @{
 */

/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////

#include <exception>
#include <string>

#include "TGRSIDataParser.h"

class TGRSIDataParserException : public std::exception {
public:
   TGRSIDataParserException(TGRSIDataParser::EDataParserState state, int failedWord, bool multipleErrors);
   TGRSIDataParserException(const TGRSIDataParserException&) = default;
	TGRSIDataParserException(TGRSIDataParserException&&) noexcept = default;
	TGRSIDataParserException& operator=(const TGRSIDataParserException&) = default;
	TGRSIDataParserException& operator=(TGRSIDataParserException&&) noexcept = default;
   ~TGRSIDataParserException() override = default;

   const char* what() const noexcept override;

   int                               GetFailedWord() const { return fFailedWord; }
   TGRSIDataParser::EDataParserState GetParserState() const { return fParserState; }
   bool                              GetMultipleErrors() const { return fMultipleErrors; }

private:
   TGRSIDataParser::EDataParserState fParserState;
   int                               fFailedWord;
   bool                              fMultipleErrors;
   std::string                       fMessage;
};
/*! @} */
#endif
