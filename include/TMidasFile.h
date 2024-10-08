#ifndef TMIDASFILE_H
#define TMIDASFILE_H

/** \addtogroup Sorting
 *  @{
 */

/////////////////////////////////////////////////////////////////
///
/// \class TMidasFile
///
/// This Class is used to read and write MIDAS files in the
/// root framework. It reads and writes TMidasEvents.
///
/////////////////////////////////////////////////////////////////

#include <string>

#ifdef __APPLE__
#include <_types/_uint32_t.h>
#else
#include <cstdint>
#endif

#include "TRawFile.h"

#include "TXMLOdb.h"
#include "TMidasEvent.h"

/// Reader for MIDAS .mid files

class TMidasFile : public TRawFile {
public:
   TMidasFile();   ///< default constructor
   explicit TMidasFile(const char* filename, TRawFile::EOpenType open_type = TRawFile::EOpenType::kRead);
   TMidasFile(const TMidasFile&)                = default;
   TMidasFile(TMidasFile&&) noexcept            = default;
   TMidasFile& operator=(const TMidasFile&)     = default;
   TMidasFile& operator=(TMidasFile&&) noexcept = default;
   ~TMidasFile() override;   ///< destructor

   bool Open(const char* filename) override;   ///< Open input file
   bool OutOpen(const char* filename);         ///< Open output file

   void Close() override;   ///< Close input file
   void OutClose();         ///< Close output file

   using TObject::Read;
   using TObject::Write;
#ifndef __CINT__
   int  Read(std::shared_ptr<TRawEvent> event) override;   ///< Read one event from the file
   bool Write(const std::shared_ptr<TMidasEvent>& midasEvent,
              Option_t*                           opt = "");   ///< Write one event to the output file
#endif
   void        Skip(size_t nofEvents) override;   ///< Skip nofEvents from the file
   std::string Status(bool long_file_description = true) override;

#ifndef __CINT__
   void FillBuffer(const std::shared_ptr<TMidasEvent>& midasEvent,
                   Option_t*                           opt = "");   // Fill buffer to write out chunks of data
#endif
   bool WriteBuffer();
   // int GetBufferSize() const { return fWriteBuffer.size(); }

   int         GetLastErrno() const { return fLastErrno; }           ///< Get error value for the last file error
   const char* GetLastError() const { return fLastError.c_str(); }   ///< Get error text for the last file error

#ifndef __CINT__
   std::shared_ptr<TRawEvent> GetOdbEvent() override
   {
      return std::static_pointer_cast<TRawEvent>(fOdbEvent);
   }
#endif

   int GetRunNumber() override;
   int GetSubRunNumber() override;

   void SetMaxBufferSize(int maxsize);

#ifndef __CINT__
   std::shared_ptr<TRawEvent> NewEvent() override
   {
      return std::make_shared<TMidasEvent>();
   }
#endif

private:
   void ReadMoreBytes(size_t bytes);

   void SetFileOdb();
   void SetRunInfo(uint32_t time);
   void SetEPICSOdb();
   void SetTIGOdb();
   void SetGRIFFOdb();
   void SetTIGDAQOdb();

#ifndef __CINT__
   std::shared_ptr<TMidasEvent> fOdbEvent;
#endif

#ifdef HAS_XML
   TXMLOdb* fOdb;
#endif

   std::string fOutFilename;   ///< name of the currently open file

   std::vector<char> fWriteBuffer;
   uint32_t          fCurrentBufferSize{0};
   uint32_t          fMaxBufferSize{1000000};

   int         fLastErrno{0};   ///< errno from the last operation
   std::string fLastError;      ///< error string from last errno
   int         fCurrentEventNumber{0};

   bool fDoByteSwap{false};   ///< "true" if file has to be byteswapped

   int   fFile{-1};             ///< open input file descriptor
   void* fGzFile{nullptr};      ///< zlib compressed input file reader
   void* fPoFile{nullptr};      ///< popen() input file reader
   int   fOutFile{-1};          ///< open output file descriptor
   void* fOutGzFile{nullptr};   ///< zlib compressed output file reader

   /// \cond CLASSIMP
   ClassDefOverride(TMidasFile, 0)   // Used to open and write Midas Files // NOLINT(readability-else-after-return)
   /// \endcond
};
/*! @} */
#endif   // TMidasFile.h
