#include <iostream>
#include <fstream>
#include <cstdio>
#include <cstring>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cerrno>
#include <cassert>
#include <cstdlib>

#ifdef HAVE_ZLIB
#include <zlib.h>
#endif

#include "TString.h"

#include "TMidasFile.h"
#include "TMidasEvent.h"
#include "TRunInfo.h"
#include "TGRSIDetectorInformation.h"
#include "TGRSIMnemonic.h"
#include "GRSIDataVersion.h"

TMidasFile::TMidasFile()
#ifdef HAS_XML
   : fOdb(nullptr)
#endif
{
   // Default Constructor
   uint32_t endian = 0x12345678;
   fDoByteSwap     = *reinterpret_cast<char*>(&endian) != 0x78;
}

TMidasFile::TMidasFile(const char* filename, TRawFile::EOpenType open_type) : TMidasFile()
{
   bool status = false;
   switch(open_type) {
   case TRawFile::EOpenType::kRead: status = Open(filename); break;
   case TRawFile::EOpenType::kWrite: status = OutOpen(filename); break;
   }
   if(!status) {
      std::stringstream str;
      str << RED << "Failed to create midas file \"" << filename << "\": " << fLastError << RESET_COLOR << std::endl;
      throw std::runtime_error(str.str());
   }
}

TMidasFile::~TMidasFile()
{
   // Default dtor. It closes the read in midas file as well as the output midas file.
   Close();
   OutClose();
}

std::string TMidasFile::Status(bool)
{
   std::stringstream str;
   str << HIDE_CURSOR << " Processing event " << fCurrentEventNumber << " have processed " << std::setprecision(2) << static_cast<double>(BytesRead()) / 1000000. << "MB/" << static_cast<double>(FileSize()) / 1000000. << " MB               " << SHOW_CURSOR << "\r";
   return str.str();
}

static int hasSuffix(const char* name, const char* suffix)
{
   // Checks to see if midas file has suffix.
   const char* s = strstr(name, suffix);
   if(s == nullptr) {
      return 0;
   }

   return static_cast<int>((s - name) + strlen(suffix) == strlen(name));
}

/// Open a midas .mid file with given file name.
///
/// Remote files can be accessed using these special file names:
/// - pipein://command - read data produced by given command, see examples below
/// - ssh://username\@hostname/path/file.mid - read remote file through an ssh pipe
/// - ssh://username\@hostname/path/file.mid.gz and file.mid.bz2 - same for compressed files
/// - dccp://path/file.mid (also file.mid.gz and file.mid.bz2) - read data from dcache, requires dccp in the PATH
///
/// Examples:
/// - ./event_dump.exe /ladd/data9/t2km11/data/run02696.mid.gz - read normal compressed file
/// - ./event_dump.exe ssh://ladd09//ladd/data9/t2km11/data/run02696.mid.gz - read compressed file through ssh to ladd09
/// (note double "/")
/// - ./event_dump.exe pipein://"cat /ladd/data9/t2km11/data/run02696.mid.gz | gzip -dc" - read data piped from a
/// command or script (note quotes)
/// - ./event_dump.exe pipein://"gzip -dc /ladd/data9/t2km11/data/run02696.mid.gz" - another way to read compressed
/// files
/// - ./event_dump.exe dccp:///pnfs/triumf.ca/data/t2km11/aug2008/run02837.mid.gz - read file directly from a dcache
/// pool (note triple "/")
///
/// \param[in] filename The file to open.
/// \returns "true" for succes, "false" for error, use GetLastError() to see why
bool TMidasFile::Open(const char* filename)
{
   if(fFile > 0) {
      Close();
   }

   Filename(filename);

   std::string pipe;

   std::ifstream in(GetFilename(), std::ifstream::in | std::ifstream::binary);
   in.seekg(0, std::ifstream::end);
   FileSize(in.tellg());
   in.close();

   // Do we need these?
   // signal(SIGPIPE,SIG_IGN); // crash if reading from closed pipe
   // signal(SIGXFSZ,SIG_IGN); // crash if reading from file >2GB without O_LARGEFILE

   if(strncmp(filename, "ssh://", 6) == 0) {
      const char* name = filename + 6;
      const char* s    = strstr(name, "/");

      if(s == nullptr) {
         fLastErrno = -1;
         fLastError.assign("TMidasFile::Open: Invalid ssh:// URI. Should be: ssh://user@host/file/path/...");
         return false;
      }

      const char* remoteFile = s + 1;

      std::string remoteHost;
      for(s = name; *s != '/'; s++) {
         remoteHost += *s;
      }

      pipe = "ssh -e none -T -x -n ";
      pipe += remoteHost;
      pipe += " dd if=";
      pipe += remoteFile;
      pipe += " bs=1024k";

      if(hasSuffix(remoteFile, ".gz") != 0) {
         pipe += " | gzip -dc";
      } else if(hasSuffix(remoteFile, ".bz2") != 0) {
         pipe += " | bzip2 -dc";
      }
   } else if(strncmp(filename, "dccp://", 7) == 0) {
      const char* name = filename + 7;

      pipe = "dccp ";
      pipe += name;
      pipe += " /dev/fd/1";

      if(hasSuffix(filename, ".gz") != 0) {
         pipe += " | gzip -dc";
      } else if(hasSuffix(filename, ".bz2") != 0) {
         pipe += " | bzip2 -dc";
      }
   } else if(strncmp(filename, "pipein://", 9) == 0) {
      pipe = filename + 9;
#if 0   // read compressed files using the zlib library
	} else if(hasSuffix(filename, ".gz")) {
		pipe = "gzip -dc ";
		pipe += filename;
#endif
   } else if(hasSuffix(filename, ".bz2") != 0) {
      pipe = "bzip2 -dc ";
      pipe += filename;
   }
   // Note: We cannot use "cat" in a similar way to offload, and must open it directly.
   //       "cat" ends immediately on end-of-file, making live histograms impossible.
   //       "tail -fn +1" has the opposite problem, and will never end, stalling in read().

   if(pipe.length() > 0) {
      fprintf(stderr, "TMidasFile::Open: Reading from pipe: %s\n", pipe.c_str());
      fPoFile = popen(pipe.c_str(), "r");

      if(fPoFile == nullptr) {
         fLastErrno = errno;
         fLastError.assign(std::strerror(errno));
         return false;
      }

      fFile = fileno(reinterpret_cast<FILE*>(fPoFile));
   } else {
#ifndef O_LARGEFILE
#define O_LARGEFILE 0
#endif

      fFile = open(filename, O_RDONLY | O_LARGEFILE);

      if(fFile <= 0) {
         fLastErrno = errno;
         fLastError.assign(std::strerror(errno));
         return false;
      }

      if(hasSuffix(filename, ".gz") != 0) {
// this is a compressed file
#ifdef HAVE_ZLIB
         fGzFile             = new gzFile;
         (*(gzFile*)fGzFile) = gzdopen(fFile, "rb");
         if((*(gzFile*)fGzFile) == nullptr) {
            fLastErrno = -1;
            fLastError.assign("zlib gzdopen() error");
            return false;
         }
#else
         fLastErrno = -1;
         fLastError.assign("Do not know how to read compressed MIDAS files");
         return false;
#endif
      }
   }

   // setup TChannel to use our mnemonics
   TChannel::SetMnemonicClass(TGRSIMnemonic::Class());

   // read ODB from file
   if(fOdbEvent == nullptr) { fOdbEvent = std::make_shared<TMidasEvent>(); }
   Read(fOdbEvent);

   SetFileOdb();
   TRunInfo::SetRunInfo(GetRunNumber(), GetSubRunNumber());
   TRunInfo::ClearLibraryVersion();
   TRunInfo::SetLibraryVersion(GRSIDATA_RELEASE);

   auto* detInfo = new TGRSIDetectorInformation();
   TRunInfo::SetDetectorInformation(detInfo);

   return true;
}

bool TMidasFile::OutOpen(const char* filename)
{
   /// Open a midas .mid file for OUTPUT with given file name.
   ///
   /// Remote files not yet implemented
   ///
   /// \param [in] filename The file to open.
   /// \returns "true" for succes, "false" for error, use GetLastError() to see why

   if(fOutFile > 0) {
      OutClose();
   }

   fOutFilename = filename;

   std::cout << "Attempting normal open of file " << filename << std::endl;
   // fOutFile = open(filename, O_CREAT |  O_WRONLY | O_LARGEFILE , S_IRUSR| S_IWUSR | S_IRGRP | S_IROTH );
   // fOutFile = open(filename, O_WRONLY | O_CREAT | O_TRUNC | O_BINARY | O_LARGEFILE, 0644);
   fOutFile = open(filename, O_WRONLY | O_CREAT | O_TRUNC | O_LARGEFILE, 0644);

   if(fOutFile <= 0) {
      fLastErrno = errno;
      fLastError.assign(std::strerror(errno));
      return false;
   }

   std::cout << "Opened output file " << filename << "; return fOutFile is " << fOutFile << std::endl;

   if(hasSuffix(filename, ".gz") != 0) {
      // this is a compressed file
#ifdef HAVE_ZLIB
      fOutGzFile           = new gzFile;
      *(gzFile*)fOutGzFile = gzdopen(fOutFile, "wb");
      if((*(gzFile*)fOutGzFile) == nullptr) {
         fLastErrno = -1;
         fLastError.assign("zlib gzdopen() error");
         return false;
      }
      std::cout << "Opened gz file successfully" << std::endl;
      if(true) {
         if(gzsetparams(*(gzFile*)fOutGzFile, 1, Z_DEFAULT_STRATEGY) != Z_OK) {
            std::cout << "Cannot set gzparams" << std::endl;
            fLastErrno = -1;
            fLastError.assign("zlib gzsetparams() error");
            return false;
         }
         std::cout << "setparams for gz file successfully" << std::endl;
      }
#else
      fLastErrno = -1;
      fLastError.assign("Do not know how to write compressed MIDAS files");
      return false;
#endif
   }
   return true;
}

static int readpipe(int fd, char* buf, int length)
{
   int count = 0;
   while(length > 0) {
      int rd = read(fd, buf, length);
      if(rd > 0) {
         buf += rd;
         length -= rd;
         count += rd;
      } else if(rd == 0) {
         return count;
      } else {
         return -1;
      }
   }
   return count;
}

/// \param [in] event shared Pointer to an empty TMidasEvent
/// \returns "true" for success, "false" for failure, see GetLastError() to see why
///
///  EDITED FROM THE ORIGINAL TO RETURN TOTAL SUCESSFULLY BYTES READ INSTEAD OF TRUE/FALSE,  PCB
///
int TMidasFile::Read(std::shared_ptr<TRawEvent> event)
{
   if(event == nullptr) {
      return -1;
   }
   std::shared_ptr<TMidasEvent> midasEvent = std::static_pointer_cast<TMidasEvent>(event);
   if(BufferSize() < sizeof(TMidas_EVENT_HEADER)) {
      ReadMoreBytes(sizeof(TMidas_EVENT_HEADER) - BufferSize());
   }

   if(BufferSize() < sizeof(TMidas_EVENT_HEADER)) {
      return 0;
   }

   midasEvent->Clear();
   memcpy(reinterpret_cast<char*>(midasEvent->GetEventHeader()), BufferData(), sizeof(TMidas_EVENT_HEADER));
   if(fDoByteSwap) {
      std::cout << "Swapping bytes" << std::endl;
      midasEvent->SwapBytesEventHeader();
   }
   if(!midasEvent->IsGoodSize()) {
      fLastErrno = -1;
      fLastError.assign("Invalid event size");
      return 0;
   }

   size_t event_size = midasEvent->GetDataSize();
   size_t total_size = sizeof(TMidas_EVENT_HEADER) + event_size;

   if(BufferSize() < total_size) {
      ReadMoreBytes(total_size - BufferSize());
   }

   if(BufferSize() < total_size) {
      return 0;
   }

   memcpy(midasEvent->GetData(), BufferData() + sizeof(TMidas_EVENT_HEADER), event_size);
   midasEvent->SwapBytes(false);

   size_t bytesRead = BufferSize();
   IncrementBytesRead(bytesRead);
   fCurrentEventNumber++;
   ClearBuffer();

   return bytesRead;
}

void TMidasFile::Skip(size_t nofEvents)
{
   TMidasEvent ev;
   for(size_t i = 0; i < nofEvents; ++i) {
      // if we don't have enough data left for a header, we try and read more
      if(BufferSize() < sizeof(TMidas_EVENT_HEADER)) {
         ReadMoreBytes(sizeof(TMidas_EVENT_HEADER) - BufferSize());
      }

      // if we don't have enough data to read the header we are done
      if(BufferSize() < sizeof(TMidas_EVENT_HEADER)) {
         return;
      }

      ev.Clear();
      // copy the header
      memcpy(reinterpret_cast<char*>(ev.GetEventHeader()), BufferData(), sizeof(TMidas_EVENT_HEADER));
      if(fDoByteSwap) {
         std::cout << "Swapping bytes" << std::endl;
         ev.SwapBytesEventHeader();
      }
      if(!ev.IsGoodSize()) {
         fLastErrno = -1;
         fLastError.assign("Invalid event size");
         return;
      }

      // check if this event is the end-of-run event
      if((ev.GetEventHeader()->fEventId & 0xffff) == 0x8001) {
         // we simply return here so that the next event read is the end-of-run event
         return;
      }

      size_t event_size = ev.GetDataSize();
      size_t total_size = sizeof(TMidas_EVENT_HEADER) + event_size;

      // try and read the total event if we don't already have it in the buffer
      if(BufferSize() < total_size) {
         ReadMoreBytes(total_size - BufferSize());
      }

      // if we don't have enough data to read the whole event we are done
      if(BufferSize() < total_size) {
         return;
      }

      //increment our counters and clear the buffer
      size_t bytesRead = BufferSize();
      IncrementBytesRead(bytesRead);
      fCurrentEventNumber++;
      ClearBuffer();
   }
}

void TMidasFile::ReadMoreBytes(size_t bytes)
{
   size_t initial_size = BufferSize();
   ResizeBuffer(initial_size + bytes);
   size_t rd = 0;
   if(fGzFile != nullptr) {
#ifdef HAVE_ZLIB
      rd = gzread(*(gzFile*)fGzFile, BufferData() + initial_size, bytes);
#else
      assert(!"Cannot get here");
#endif
   } else {
      rd = readpipe(fFile, BufferData() + initial_size, bytes);
   }

   ResizeBuffer(initial_size + rd);

   if(rd == 0) {
      fLastErrno = 0;
      fLastError.assign("EOF");
   } else if(rd != bytes) {
      fLastErrno = errno;
      fLastError.assign(std::strerror(errno));
   }
}

void TMidasFile::FillBuffer(const std::shared_ptr<TMidasEvent>& midasEvent, Option_t*)
{
   // Fills a buffer to be written to a midas file.

   // Not the prettiest way to do this but it works.
   // It seems to be filling in the wrong order of bits, but this does it correctly
   // There is a byte swap happening at some point in this process. Might have to put something
   // in here that protects against "Endian-ness"
   fWriteBuffer.push_back(static_cast<char>(midasEvent->GetEventId() & 0xFF));
   fWriteBuffer.push_back(static_cast<char>(midasEvent->GetEventId() >> 8));

   fWriteBuffer.push_back(static_cast<char>(midasEvent->GetTriggerMask() & 0xFF));
   fWriteBuffer.push_back(static_cast<char>(midasEvent->GetTriggerMask() >> 8));

   fWriteBuffer.push_back(static_cast<char>(midasEvent->GetSerialNumber() & 0xFF));
   fWriteBuffer.push_back(static_cast<char>((midasEvent->GetSerialNumber() >> 8) & 0xFF));
   fWriteBuffer.push_back(static_cast<char>((midasEvent->GetSerialNumber() >> 16) & 0xFF));
   fWriteBuffer.push_back(static_cast<char>(midasEvent->GetSerialNumber() >> 24));

   fWriteBuffer.push_back(static_cast<char>(midasEvent->GetTimeStamp() & 0xFF));
   fWriteBuffer.push_back(static_cast<char>((midasEvent->GetTimeStamp() >> 8) & 0xFF));
   fWriteBuffer.push_back(static_cast<char>((midasEvent->GetTimeStamp() >> 16) & 0xFF));
   fWriteBuffer.push_back(static_cast<char>(midasEvent->GetTimeStamp() >> 24));

   fWriteBuffer.push_back(static_cast<char>(midasEvent->GetDataSize() & 0xFF));
   fWriteBuffer.push_back(static_cast<char>((midasEvent->GetDataSize() >> 8) & 0xFF));
   fWriteBuffer.push_back(static_cast<char>((midasEvent->GetDataSize() >> 16) & 0xFF));
   fWriteBuffer.push_back(static_cast<char>(midasEvent->GetDataSize() >> 24));

   for(size_t i = 0; i < midasEvent->GetDataSize(); i++) {
      fWriteBuffer.push_back(midasEvent->GetData()[i]);
   }

   fCurrentBufferSize += midasEvent->GetDataSize() + sizeof(TMidas_EVENT_HEADER);

   if(fWriteBuffer.size() > fMaxBufferSize) {
      WriteBuffer();
   }
}

bool TMidasFile::WriteBuffer()
{
   // Writes a buffer of TMidasEvents to the output file.
   int wr = -2;

   if(fOutGzFile != nullptr) {
#ifdef HAVE_ZLIB
      wr = gzwrite(*(gzFile*)fOutGzFile, fWriteBuffer.data(), fCurrentBufferSize);
#else
      assert(!"Cannot get here");
#endif
   } else {
      wr = write(fOutFile, fWriteBuffer.data(), fCurrentBufferSize);
   }
   fCurrentBufferSize = 0;
   fWriteBuffer.clear();

   return wr != 0;
}

bool TMidasFile::Write(const std::shared_ptr<TMidasEvent>& midasEvent, Option_t* opt)
{
   // Writes an individual TMidasEvent to the output TMidasFile. This will
   // write to a zipped file if the output file is defined as a zipped file.
   int wr = -2;

   if(fOutGzFile != nullptr) {
#ifdef HAVE_ZLIB
      wr = gzwrite(*(gzFile*)fOutGzFile, (char*)midasEvent->GetEventHeader(), sizeof(TMidas_EVENT_HEADER));
#else
      assert(!"Cannot get here");
#endif
   } else {
      wr = write(fOutFile, reinterpret_cast<char*>(midasEvent->GetEventHeader()), sizeof(TMidas_EVENT_HEADER));
   }

   if(wr != sizeof(TMidas_EVENT_HEADER)) {
      std::cout << "TMidasFile: error on write event header, return " << wr << ", size requested " << sizeof(TMidas_EVENT_HEADER) << std::endl;
      return false;
   }

   if(strncmp(opt, "q", 1) != 0) {
      std::cout << "Written event header to outfile , return is " << wr << std::endl;
   }

   if(fOutGzFile != nullptr) {
#ifdef HAVE_ZLIB
      wr = gzwrite(*(gzFile*)fOutGzFile, (char*)midasEvent->GetData(), midasEvent->GetDataSize());
#else
      assert(!"Cannot get here");
#endif
   } else {
      wr = write(fOutFile, midasEvent->GetData(), midasEvent->GetDataSize());
   }

   if(strncmp(opt, "q", 1) != 0) {
      std::cout << "Written event to outfile , return is " << wr << std::endl;
   }

   return wr != 0;
}

void TMidasFile::SetMaxBufferSize(int maxsize)
{
   // Sets the maximum buffer size for the TMidasEvents to be written to
   // an output TMidasFile.
   fMaxBufferSize = maxsize;
}

void TMidasFile::Close()
{
   // Closes the input midas file. Use OutClose() to close the output
   // Midas File.
   if(fPoFile != nullptr) {
      pclose(reinterpret_cast<FILE*>(fPoFile));
   }
   fPoFile = nullptr;
#ifdef HAVE_ZLIB
   if(fGzFile) gzclose(*(gzFile*)fGzFile);
   fGzFile = nullptr;
#endif
   if(fFile > 0) {
      close(fFile);
   }

   fFile = -1;
   Filename("");
}

void TMidasFile::OutClose()
{
   // Closes the output midas file. Use Close() to close the read-in midas file

   if(static_cast<unsigned int>(!fWriteBuffer.empty()) != 0u) {
      WriteBuffer();
   }
#ifdef HAVE_ZLIB
   if(fOutGzFile) {
      gzflush(*(gzFile*)fOutGzFile, Z_FULL_FLUSH);
      gzclose(*(gzFile*)fOutGzFile);
   }
   fOutGzFile = nullptr;
#endif
   if(fOutFile > 0) {
      close(fOutFile);
   }
   fOutFile     = -1;
   fOutFilename = "";
}

int TMidasFile::GetRunNumber()
{
   // Parse the run number from the current TMidasFile. This assumes a format of
   // run#####_###.mid or run#####.mid.
   if(Filename().length() == 0) {
      return 0;
   }
   std::size_t foundslash = Filename().rfind('/');
   std::size_t found      = Filename().rfind(".mid");
   if(found == std::string::npos) {
      return 0;
   }
   std::size_t found2 = Filename().rfind('-');
   if((found2 < foundslash && foundslash != std::string::npos) || found2 == std::string::npos) {
      found2 = Filename().rfind('_');
   }
   if(found2 < foundslash && foundslash != std::string::npos) {
      found2 = std::string::npos;
   }
   std::string temp;
   if(found2 == std::string::npos || Filename().compare(found2 + 4, 4, ".mid") != 0) {
      temp = Filename().substr(found - 5, 5);
   } else {
      temp = Filename().substr(found - 9, 5);
   }
   return atoi(temp.c_str());
}

int TMidasFile::GetSubRunNumber()
{
   // Parse the sub run number from the current TMidasFile. This assumes a format of
   // run#####_###.mid or run#####.mid.
   if(Filename().empty()) {
      return -1;
   }
   std::size_t foundslash = Filename().rfind('/');
   std::size_t found      = Filename().rfind('-');
   if((found < foundslash && foundslash != std::string::npos) || found == std::string::npos) {
      found = Filename().rfind('_');
   }
   if(found < foundslash && foundslash != std::string::npos) {
      found = std::string::npos;
   }
   if(found != std::string::npos) {
      std::string temp = Filename().substr(found + 1, 3);
      return atoi(temp.c_str());
   }
   return -1;
}

void TMidasFile::SetFileOdb()
{
#ifdef HAS_XML
   // check if we have already set the TChannels....
   //
   delete fOdb;
   fOdb = nullptr;

   if(TGRSIOptions::Get()->IgnoreFileOdb()) {
      std::cout << DYELLOW << "\tskipping odb information stored in file." << RESET_COLOR << std::endl;
      return;
   }

   try {
      fOdb = new TXMLOdb(fOdbEvent->GetData(), fOdbEvent->GetDataSize());
   } catch(std::exception& e) {
      std::cout << "Got exception '" << e.what() << "' trying to read " << fOdbEvent->GetDataSize() << " bytes (or words?) from:" << std::endl;
      std::cout << fOdbEvent->GetData() << std::endl;
      throw e;
   }
   TChannel::DeleteAllChannels();

   SetRunInfo(fOdbEvent->GetTimeStamp());

   // Check for EPICS variables
   SetEPICSOdb();

   if(TGRSIOptions::Get()->IgnoreOdbChannels()) {
      std::cout << DYELLOW << "\tskipping odb channel information stored in file." << RESET_COLOR << std::endl;
      return;
   }

   // Check to see if we are running a GRIFFIN or TIGRESS experiment
   TXMLNode* node = fOdb->FindPath("/Experiment");
   if(!node->HasChildren()) {
      return;
   }
   node = node->GetChildren();
   std::string expt;
   while(true) {
      std::string key = fOdb->GetNodeName(node);
      if(key == "Name") {
         expt = node->GetText();
         break;
      }
      if(!node->HasNextNode()) {
         break;
      }
      node = node->GetNextNode();
   }
   if(expt == "tigress") {
      SetTIGOdb();
   } else if(expt.find("grif") != std::string::npos) {
      // for GRIFFIN the experiment name might be griffin, grifstor, grifalt, etc.
      SetGRIFFOdb();
   } else if(expt == "tigdaq") {   //New TIGRESS DAQ
      SetTIGDAQOdb();
   } else {
      std::cerr << RED << "Unknown experiment name \"" << expt << "\", ODB won't be read!" << RESET_COLOR << std::endl;
   }
#endif
}

void TMidasFile::SetRunInfo(uint32_t time)
{
#ifdef HAS_XML
   TXMLNode* node = fOdb->FindPath("/Runinfo/Start time binary");
   if(node != nullptr) {
      std::stringstream str(node->GetText());
      unsigned int      odbTime = 0;
      str >> odbTime;
      if(TRunInfo::SubRunNumber() == 0 && time != odbTime) {
         std::cout << "Warning, ODB start time of first subrun (" << odbTime << ") does not match midas time of first event in this subrun (" << time << ")!" << std::endl;
      }
      TRunInfo::SetRunStart(time);
   }

   node = fOdb->FindPath("/Experiment/Run parameters/Run Title");
   if(node != nullptr) {
      TRunInfo::SetRunTitle(node->GetText());
      std::cout << "Title: " << DBLUE << node->GetText() << RESET_COLOR << std::endl;
   }

   if(node != nullptr) {
      node = fOdb->FindPath("/Experiment/Run parameters/Comment");
      TRunInfo::SetRunComment(node->GetText());
      std::cout << "Comment: " << DBLUE << node->GetText() << RESET_COLOR << std::endl;
   }
#endif
}

void TMidasFile::SetEPICSOdb()
{
#ifdef HAS_XML
   TXMLNode*                node  = fOdb->FindPath("/Equipment/Epics/Settings/Names");
   std::vector<std::string> names = fOdb->ReadStringArray(node);
   TEpicsFrag::SetEpicsNameList(names);
#endif
}

void TMidasFile::SetGRIFFOdb()
{
#ifdef HAS_XML
   // get calibrations
   // check if we can find new /DAQ/PSC path, otherwise default back to old /DAQ/MSC path
   std::string path = "/DAQ/PSC";
   std::string temp;
   if(fOdb->FindPath(path.c_str()) != nullptr) {
      std::cout << "using GRIFFIN path to analyzer info: " << path << "..." << std::endl;

      temp = path;
      temp.append("/PSC");
   } else {
      path = "/DAQ/MSC";
      std::cout << "using GRIFFIN path to analyzer info: " << path << "..." << std::endl;

      temp = path;
      temp.append("/MSC");
   }
   TXMLNode*        node    = fOdb->FindPath(temp.c_str());
   std::vector<int> address = fOdb->ReadIntArray(node);

   temp = path;
   temp.append("/chan");
   node                           = fOdb->FindPath(temp.c_str());
   std::vector<std::string> names = fOdb->ReadStringArray(node);

   temp = path;
   temp.append("/datatype");
   node                  = fOdb->FindPath(temp.c_str());
   std::vector<int> type = fOdb->ReadIntArray(node);
   if(type.empty()) {
      // failed to read array from /DAQ/<P/M>SC/datatype, so try to read /DAQ/<P/M>SC/DetType
      temp = path;
      temp.append("/DetType");
      node = fOdb->FindPath(temp.c_str());
      type = fOdb->ReadIntArray(node);
      std::cout << "failed to find ODB path " << path << "/datatype, using " << type.size() << " entries from " << path << "/DetType instead" << std::endl;
   }

   temp = path;
   temp.append("/gain");
   node                      = fOdb->FindPath(temp.c_str());
   std::vector<double> gains = fOdb->ReadDoubleArray(node);

   temp = path;
   temp.append("/offset");
   node                        = fOdb->FindPath(temp.c_str());
   std::vector<double> offsets = fOdb->ReadDoubleArray(node);

   // DAQ has quadratic terms in ODB from November 2019
   temp = path;
   temp.append("/quadratic");
   node                      = fOdb->FindPath(temp.c_str());
   std::vector<double> quads = fOdb->ReadDoubleArray(node);

   temp = path;
   temp.append("/digitizer");
   node                               = fOdb->FindPath(temp.c_str());
   std::vector<std::string> digitizer = fOdb->ReadStringArray(node);

   if((address.size() == names.size()) && (names.size() == gains.size()) && (gains.size() == offsets.size()) &&
      (offsets.size() == type.size())) {
      // all good. We ignore the digitizer size as this information is not present for all data
      // Also ignoring quad terms as this will only be present in future data - S. Gillespie
      for(size_t x = 0; x < address.size(); x++) {
         TChannel* tempChan = TChannel::GetChannel(address.at(x), false);   // names.at(x).c_str());
         if(tempChan == nullptr) {
            tempChan = new TChannel();
         }
         tempChan->SetName(names.at(x).c_str());
         tempChan->SetAddress(address.at(x));
         tempChan->SetNumber(TPriorityValue<int>(x, EPriority::kRootFile));

         tempChan->AddENGCoefficient(static_cast<Float_t>(offsets.at(x)));
         tempChan->AddENGCoefficient(static_cast<Float_t>(gains.at(x)));
         if(x < quads.size()) {
            tempChan->AddENGCoefficient(static_cast<Float_t>(quads.at(x)));   //Assuming this means quad terms won't be added if not there.
         }
         if(x < digitizer.size()) {
            tempChan->SetDigitizerType(TPriorityValue<std::string>(digitizer.at(x), EPriority::kRootFile));
         }
         // TChannel::UpdateChannel(tempChan);
         TChannel::AddChannel(tempChan, "overwrite");
      }
      std::cout << TChannel::GetNumberOfChannels() << "\t TChannels created." << std::endl;
   } else {
      std::cout << BG_WHITE DRED << "problem parsing odb data, arrays are different sizes, channels not set." << RESET_COLOR << std::endl;
   }

   // get cycle information
   // "/Experiment/Edit on start/PPG Cycle" is a link to the PPG cycle used (always "/PPG/Current"???)
   // "/PPG/Current" gives the current PPG cycle used, e.g. 146Cs_S1468
   // "/PPG/Cycles/146Cs_S1468" then has four PPGcodes and four durations
   node = fOdb->FindPath("/PPG/Current");
   if(node == nullptr) {
      std::cerr << R"(Failed to find "/PPG/Current" in ODB!)" << std::endl;
      return;
   }

   if(!node->HasChildren()) {
      std::cout << "Node has no children, can't read ODB cycle" << std::endl;
      return;
   }
   std::string currentCycle = "/PPG/Cycles/";
   currentCycle.append(node->GetChildren()->GetContent());
   temp = currentCycle;
   temp.append("/PPGcodes");
   node = fOdb->FindPath(temp.c_str());
   if(node == nullptr) {
      std::cerr << R"(Failed to find ")" << temp << R"(" in ODB!)" << std::endl;
      return;
   }
   std::vector<int> tmpCodes = fOdb->ReadIntArray(node);
   // the codes are 32bit with the 16 high bits being the same as the 16 low bits
   // we check this and only keep the low 16 bits
   std::vector<int16_t> ppgCodes;
   for(auto& code : tmpCodes) {
      if(((code >> 16) & 0xffff) != (code & 0xffff)) {
         std::cout << DRED << "Found ppg code in the ODB with high bits (0x" << std::hex << (code >> 16)
                   << ") != low bits (" << (code & 0xffff) << std::dec << ")" << RESET_COLOR << std::endl;
      }
      ppgCodes.push_back(code & 0xffff);
   }
   temp = currentCycle;
   temp.append("/durations");
   node = fOdb->FindPath(temp.c_str());
   if(node == nullptr) {
      std::cerr << R"(Failed to find ")" << temp << R"(" in ODB!)" << std::endl;
      return;
   }
   std::vector<int> durations = fOdb->ReadIntArray(node);

   if(durations.size() != ppgCodes.size()) {
      std::cerr << "Mismatching sizes of ppg codes (" << ppgCodes.size() << ") and duration (" << durations.size() << ")" << std::endl;
      return;
   }

   TPPG::Get()->SetOdbCycle(ppgCodes, durations);
#endif
}

void TMidasFile::SetTIGOdb()
{
#ifdef HAS_XML
   std::string                                        typepath = "/Equipment/Trigger/settings/Detector Settings";
   std::map<int, std::pair<std::string, std::string>> typemap;
   TXMLNode*                                          typenode    = fOdb->FindPath(typepath.c_str());
   int                                                typecounter = 0;
   if(typenode->HasChildren()) {
      TXMLNode* typechild = typenode->GetChildren();
      while(true) {
         std::string tname = fOdb->GetNodeName(typechild);
         if(tname.length() > 0 && typechild->HasChildren()) {
            typecounter++;
            TXMLNode* grandchild = typechild->GetChildren();
            while(true) {
               std::string grandchildname = fOdb->GetNodeName(grandchild);
               if(grandchildname.compare(0, 7, "Digitis") == 0) {
                  std::string dname    = grandchild->GetText();
                  typemap[typecounter] = std::make_pair(tname, dname);
                  break;
               }
               if(!grandchild->HasNextNode()) {
                  break;
               }
               grandchild = grandchild->GetNextNode();
            }
         }
         if(!typechild->HasNextNode()) {
            break;
         }
         typechild = typechild->GetNextNode();
      }
   }

   std::string path = "/Analyzer/Shared Parameters/Config";
   TXMLNode*   test = fOdb->FindPath(path.c_str());
   if(test == nullptr) {
      path.assign("/Analyzer/Parameters/Cathode/Config");   // the old path to the useful odb info.
   }
   std::cout << "using TIGRESS path to analyzer info: " << path << "..." << std::endl;

   std::string temp = path;
   temp.append("/FSCP");
   TXMLNode*        node    = fOdb->FindPath(temp.c_str());
   std::vector<int> address = fOdb->ReadIntArray(node);

   temp = path;
   temp.append("/Name");
   node                           = fOdb->FindPath(temp.c_str());
   std::vector<std::string> names = fOdb->ReadStringArray(node);

   temp = path;
   temp.append("/Type");
   node                  = fOdb->FindPath(temp.c_str());
   std::vector<int> type = fOdb->ReadIntArray(node);

   temp = path;
   temp.append("/g");
   node                      = fOdb->FindPath(temp.c_str());
   std::vector<double> gains = fOdb->ReadDoubleArray(node);

   temp = path;
   temp.append("/o");
   node                        = fOdb->FindPath(temp.c_str());
   std::vector<double> offsets = fOdb->ReadDoubleArray(node);

   //   if( (address.size() == names.size()) && (names.size() == gains.size()) && (gains.size() == offsets.size()) &&
   //   offsets.size() == type.size() ) {
   if((address.size() == gains.size()) && (gains.size() == offsets.size()) && offsets.size() == type.size()) {
      // all good.
   } else {
      std::cout << BG_WHITE DRED << "problem parsing odb data, arrays are different sizes, channels not set." << RESET_COLOR << std::endl;
      std::cout << DRED << "\taddress.size() = " << address.size() << RESET_COLOR << std::endl;
      std::cout << DRED << "\tnames.size()   = " << names.size() << RESET_COLOR << std::endl;
      std::cout << DRED << "\tgains.size()   = " << gains.size() << RESET_COLOR << std::endl;
      std::cout << DRED << "\toffsets.size() = " << offsets.size() << RESET_COLOR << std::endl;
      std::cout << DRED << "\ttype.size()    = " << type.size() << RESET_COLOR << std::endl;
      return;
   }

   for(size_t x = 0; x < address.size(); x++) {
      TChannel* tempChan = TChannel::GetChannel(address.at(x), false);   // names.at(x).c_str());
      if(tempChan == nullptr) {
         tempChan = new TChannel();
      }
      if(x < names.size()) {
         tempChan->SetName(names.at(x).c_str());
      }
      tempChan->SetAddress(address.at(x));
      tempChan->SetNumber(TPriorityValue<int>(x, EPriority::kRootFile));
      int temp_integration = 0;
      if(type.at(x) != 0) {
         tempChan->SetDigitizerType(TPriorityValue<std::string>(typemap[type.at(x)].second, EPriority::kRootFile));
         if(strcmp(tempChan->GetDigitizerTypeString(), "Tig64") ==
            0) {   // TODO: maybe use enumerations via GetDigitizerType()
            temp_integration = 25;
         } else if(strcmp(tempChan->GetDigitizerTypeString(), "Tig10") == 0) {
            temp_integration = 125;
         }
      }
      tempChan->SetIntegration(TPriorityValue<int>(temp_integration, EPriority::kRootFile));
      tempChan->AddENGCoefficient(static_cast<Float_t>(offsets.at(x)));
      tempChan->AddENGCoefficient(static_cast<Float_t>(gains.at(x)));

      TChannel::AddChannel(tempChan, "overwrite");
   }
   std::cout << TChannel::GetNumberOfChannels() << "\t TChannels created." << std::endl;
#endif
}

void TMidasFile::SetTIGDAQOdb()   // Basically a copy of the GRIFFIN one without the PPG (as we don't have one) and digitizer <P/M>SC key which is not in TIGDAQ
{
#ifdef HAS_XML
   // get calibrations
   // check if we can find new /DAQ/PSC path, otherwise default back to old /DAQ/MSC path
   std::string path = "/DAQ/PSC";
   std::string temp;
   if(fOdb->FindPath(path.c_str()) != nullptr) {
      std::cout << "using TIGRESS path to analyzer info: " << path << "..." << std::endl;

      temp = path;
      temp.append("/PSC");
   } else {
      path = "/DAQ/MSC";
      std::cout << "using TIGRESS path to analyzer info: " << path << "..." << std::endl;

      temp = path;
      temp.append("/MSC");
   }
   TXMLNode*        node    = fOdb->FindPath(temp.c_str());
   std::vector<int> address = fOdb->ReadIntArray(node);

   temp = path;
   temp.append("/chan");
   node                           = fOdb->FindPath(temp.c_str());
   std::vector<std::string> names = fOdb->ReadStringArray(node);

   temp = path;
   temp.append("/datatype");
   node                  = fOdb->FindPath(temp.c_str());
   std::vector<int> type = fOdb->ReadIntArray(node);
   if(type.empty()) {
      // failed to read array from <path>/datatype, so try to read <path>/DetType
      temp = path;
      temp.append("/DetType");
      node = fOdb->FindPath(temp.c_str());
      type = fOdb->ReadIntArray(node);
      std::cout << "failed to find ODB path " << path << "/datatype, using " << type.size() << " entries from " << path << "/DetType instead" << std::endl;
   }

   temp = path;
   temp.append("/gain");
   node                      = fOdb->FindPath(temp.c_str());
   std::vector<double> gains = fOdb->ReadDoubleArray(node);

   temp = path;
   temp.append("/offset");
   node                        = fOdb->FindPath(temp.c_str());
   std::vector<double> offsets = fOdb->ReadDoubleArray(node);

   // DAQ has quadratic terms in ODB from November 2019
   temp = path;
   temp.append("/quadratic");
   node                      = fOdb->FindPath(temp.c_str());
   std::vector<double> quads = fOdb->ReadDoubleArray(node);

   temp = path;
   temp.append("/digitizer");
   node                               = fOdb->FindPath(temp.c_str());
   std::vector<std::string> digitizer = fOdb->ReadStringArray(node);

   if((address.size() == names.size()) && (names.size() == gains.size()) && (gains.size() == offsets.size()) && (gains.size() == offsets.size()) &&
      (offsets.size() == type.size())) {
      // Only data without quad terms is mine so they will always be present - S. Gillespie
      for(size_t x = 0; x < address.size(); x++) {
         TChannel* tempChan = TChannel::GetChannel(address.at(x), false);   // names.at(x).c_str());
         if(tempChan == nullptr) {
            tempChan = new TChannel();
         }
         tempChan->SetName(names.at(x).c_str());
         tempChan->SetAddress(address.at(x));
         tempChan->SetNumber(TPriorityValue<int>(x, EPriority::kRootFile));

         tempChan->AddENGCoefficient(static_cast<Float_t>(offsets.at(x)));
         tempChan->AddENGCoefficient(static_cast<Float_t>(gains.at(x)));
         if(x < quads.size()) { tempChan->AddENGCoefficient(static_cast<Float_t>(quads.at(x))); }   //Assuming this means quad terms won't be added if not there.
      }
      std::cout << TChannel::GetNumberOfChannels() << "\t TChannels created." << std::endl;
   } else {
      std::cout << BG_WHITE DRED << "problem parsing odb data, arrays are different sizes, channels not set." << RESET_COLOR << std::endl;
   }
#endif
}

// end
