//
//  TMidasEvent.cxx.
//
//  $Id: TMidasEvent.cxx 91 2012-04-12 18:36:17Z olchansk $
//

#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <cassert>
#include <sstream>

#include "TMidasEvent.h"
#include "TGRSIOptions.h"
#include "TRunInfo.h"
#include "TXMLOdb.h"

/// \cond CLASSIMP
ClassImp(TMidasEvent)
/// \endcond

TMidasEvent::TMidasEvent()
	: fData(nullptr), fBanksN(0), fBankList(nullptr), fAllocatedByUs(false)
{
   // Default constructor
   fEventHeader.fEventId      = 0;
   fEventHeader.fTriggerMask  = 0;
   fEventHeader.fSerialNumber = 0;
   fEventHeader.fTimeStamp    = 0;
   fEventHeader.fDataSize     = 0;
}

void TMidasEvent::Copy(TObject& rhs) const
{
   // Copies the entire TMidasEvent. This includes the bank information.
   static_cast<TMidasEvent&>(rhs).fEventHeader = fEventHeader;

   static_cast<TMidasEvent&>(rhs).fData = static_cast<char*>(malloc(static_cast<TMidasEvent&>(rhs).fEventHeader.fDataSize)); // NOLINT(cppcoreguidelines-no-malloc)
   assert(static_cast<TMidasEvent&>(rhs).fData);
   memcpy(static_cast<TMidasEvent&>(rhs).fData, fData, static_cast<TMidasEvent&>(rhs).fEventHeader.fDataSize);
   static_cast<TMidasEvent&>(rhs).fAllocatedByUs = true;

   static_cast<TMidasEvent&>(rhs).fBanksN   = fBanksN;
   static_cast<TMidasEvent&>(rhs).fBankList = nullptr;
   // if(fBankList) static_cast<TMidasEvent&>(rhs).fBankList    = strdup(fBankList);
   // assert(static_cast<TMidasEvent&>(rhs).fBankList);
}

TMidasEvent::TMidasEvent(const TMidasEvent& rhs) : TRawEvent()
{
   // Copy ctor.
   rhs.Copy(*this);
}

TMidasEvent::~TMidasEvent()
{
   Clear();
}

TMidasEvent& TMidasEvent::operator=(const TMidasEvent& rhs)
{
   if(&rhs != this) {
      Clear();
   }

   rhs.Copy(*this);
   return *this;
}

void TMidasEvent::Clear(Option_t*)
{
   // Clears the TMidasEvent.
	delete fBankList;
   fBankList = nullptr;

   if(fAllocatedByUs) {
      delete fData;
   }
   fData = nullptr;

   fAllocatedByUs = false;
   fBanksN        = 0;

   fEventHeader.fEventId      = 0;
   fEventHeader.fTriggerMask  = 0;
   fEventHeader.fSerialNumber = 0;
   fEventHeader.fTimeStamp    = 0;
   fEventHeader.fDataSize     = 0;

	TRawEvent::Clear();
}

void TMidasEvent::SetData(uint32_t size, char* data)
{
   // Sets the data in the TMidasEvent as the data argument passed into
   // this function.
   fEventHeader.fDataSize = size;
   assert(!fAllocatedByUs);
   assert(IsGoodSize());
   fData          = data;
   fAllocatedByUs = false;
   SwapBytes(false);
}

uint16_t TMidasEvent::GetEventId() const
{
   return fEventHeader.fEventId;
}

uint16_t TMidasEvent::GetTriggerMask() const
{
   return fEventHeader.fTriggerMask;
}

uint32_t TMidasEvent::GetSerialNumber() const
{
   return fEventHeader.fSerialNumber;
}

uint32_t TMidasEvent::GetTimeStamp() const
{
   return fEventHeader.fTimeStamp;
}

uint32_t TMidasEvent::GetDataSize() const
{
   return fEventHeader.fDataSize;
}

char* TMidasEvent::GetData()
{
   // Allocates the data if it has not been already, and then
   // returns the allocated data.
   if(fData == nullptr) {
      AllocateData();
   }
   return fData;
}

TMidas_EVENT_HEADER* TMidasEvent::GetEventHeader()
{
   return &fEventHeader;
}

bool TMidasEvent::IsGoodSize() const
{
   return fEventHeader.fDataSize > 0 && fEventHeader.fDataSize <= 500 * 1024 * 1024;
}

bool TMidasEvent::IsBank32() const
{
   return ((reinterpret_cast<TMidas_BANK_HEADER*>(fData))->fFlags & (1<<4)) != 0u;
}

int TMidasEvent::LocateBank(const void*, const char* name, void** pdata) const
{
   /// See FindBank()

   int bktype = 0;
	int bklen = 0;

   int status = FindBank(name, &bklen, &bktype, pdata);

   if(status == 0) {
      *pdata = nullptr;
      return 0;
   }

   return bklen;
}

/// Find a data bank.
/// \param [in] name Name of the data bank to look for.
/// \param [out] bklen Number of array elements in this bank.
/// \param [out] bktype Bank data type (MIDAS TID_xxx).
/// \param [out] pdata Pointer to bank data, Returns nullptr if bank not found.
/// \returns 1 if bank found, 0 otherwise.
///
int TMidasEvent::FindBank(const char* name, int* bklen, int* bktype, void** pdata) const
{
   auto*        pbkh = reinterpret_cast<TMidas_BANK_HEADER*>(fData);
   TMidas_BANK* pbk  = nullptr;

   std::array<unsigned, 17> TID_SIZE = {0, 1, 1, 1, 2, 2, 4, 4, 4, 4, 8, 1, 0, 0, 0, 0, 0};

   if(((pbkh->fFlags & (1<<4)) > 0)) {
      TMidas_BANK32* pbk32 = nullptr;

      while(true) {
         IterateBank32(&pbk32, reinterpret_cast<char**>(pdata));
         if(pbk32 == nullptr) {
            break;
         }

         if(name[0] == pbk32->fName[0] && name[1] == pbk32->fName[1] && name[2] == pbk32->fName[2] &&
            name[3] == pbk32->fName[3]) {

            if(TID_SIZE[pbk32->fType & 0xFF] == 0) {
               *bklen = pbk32->fDataSize;
            } else {
               *bklen = pbk32->fDataSize / TID_SIZE[pbk32->fType & 0xFF];
            }

            *bktype = pbk32->fType;
            return 1;
         }
      }
   } else {
      pbk = reinterpret_cast<TMidas_BANK*>(pbkh + 1);
      do {
         if(name[0] == pbk->fName[0] && name[1] == pbk->fName[1] && name[2] == pbk->fName[2] &&
            name[3] == pbk->fName[3]) {
            *pdata = pbk + 1;
            if(TID_SIZE[pbk->fType & 0xFF] == 0) {
               *bklen = pbk->fDataSize;
            } else {
               *bklen = pbk->fDataSize / TID_SIZE[pbk->fType & 0xFF];
            }

            *bktype = pbk->fType;
            return 1;
         }
         pbk = reinterpret_cast<TMidas_BANK*>(reinterpret_cast<char*>(pbk + 1) + (((pbk->fDataSize) + 7) & ~7));
      } while(reinterpret_cast<char*>(pbk) < reinterpret_cast<char*>(pbkh) + pbkh->fDataSize + sizeof(TMidas_BANK_HEADER));
   }
   //
   // bank not found
   //
   *pdata = nullptr;
   return 0;
}

void TMidasEvent::Print(const char* option) const
{
   /// Print data held in this class.
   /// \param [in] option If 'a' (for "all") then the raw data will be
   /// printed out too.
   ///

   auto t = static_cast<time_t>(fEventHeader.fTimeStamp);

	std::cout<<"Event start:"<<std::endl;
   std::cout<<"  event id:       "<<hex(fEventHeader.fEventId,4)<<std::endl;
   std::cout<<"  trigger mask:   "<<hex(fEventHeader.fTriggerMask,4)<<std::endl;
   std::cout<<"  serial number:  "<<fEventHeader.fSerialNumber<<std::endl;
   std::cout<<"  time stamp:     "<<fEventHeader.fTimeStamp<<", "<<ctime(&t)<<std::endl;
   std::cout<<"  data size:      "<<std::setw(8)<<fEventHeader.fDataSize<<std::endl;
   // const_cast<TMidasEvent*>(this)->SetBankList(); // moved here to get event information in case SetBankList crashes
   if((fEventHeader.fEventId & 0xffff) == 0x8000) {
		std::cout<<"Begin of run "<<fEventHeader.fSerialNumber<<std::endl;
   } else if((fEventHeader.fEventId & 0xffff) == 0x8001) {
		std::cout<<"End of run "<<fEventHeader.fSerialNumber<<std::endl;
   } else if((fEventHeader.fEventId & 0xffff) == 0x8002) {
		std::cout<<"Message event \""<<fData<<"\""<<std::endl;
   } else if(fBanksN <= 0) {
		std::cout<<"TMidasEvent::Print: Use SetBankList() before Print() to print bank data"<<std::endl;
   } else {
		std::cout<<"Banks: "<<fBankList<<std::endl;

      for(int i = 0; i < fBanksN * 4; i += 4) {
         int   bankLength = 0;
         int   bankType   = 0;
         void* pdata      = nullptr;
         int   found      = FindBank(&fBankList[i], &bankLength, &bankType, &pdata);

			std::cout<<"Bank "<<fBankList[i]<<fBankList[i+1]<<fBankList[i+2]<<fBankList[i+3]<<", length "<<std::setw(6)<<bankLength<<", type "<<bankType<<std::endl;

         int highlight = -1;
         if(strlen(option) > 1) {
            highlight = atoi(option + 1);
         }

         if(option[0] == 'a' && (found != 0)) {
            switch(bankType) {
            case 4: // TID_WORD
               for(int j = 0; j < bankLength; j++) {
                  if(j == highlight) {
							std::cout<<ALERTTEXT<<hex((reinterpret_cast<uint16_t*>(pdata))[j],4)<<RESET_COLOR<<((j % 10 == 9) ? '\n' : ' ');
                  } else {
							std::cout<<hex((reinterpret_cast<uint16_t*>(pdata))[j],4)<<((j % 10 == 9) ? '\n' : ' ');
                  }
               }
					std::cout<<std::endl;
               break;
            case 6: // TID_DWORD
               for(int j = 0; j < bankLength; j++) {
                  if(j == highlight) {
							std::cout<<ALERTTEXT<<hex((reinterpret_cast<uint32_t*>(pdata))[j],8)<<RESET_COLOR<<((j % 10 == 9) ? '\n' : ' ');
                  } else {
							std::cout<<hex((reinterpret_cast<uint32_t*>(pdata))[j],8)<<((j % 10 == 9) ? '\n' : ' ');
                  }
               }
					std::cout<<std::endl;
               break;
            case 7: // TID_nd280 (like a DWORD?)
               for(int j = 0; j < bankLength; j++) {
                  if(j == highlight) {
							std::cout<<ALERTTEXT<<hex((reinterpret_cast<uint32_t*>(pdata))[j],8)<<RESET_COLOR<<((j % 10 == 9) ? '\n' : ' ');
                  } else {
							std::cout<<hex((reinterpret_cast<uint32_t*>(pdata))[j],8)<<((j % 10 == 9) ? '\n' : ' ');
                  }
               }
					std::cout<<std::endl;
               break;
            case 9: // TID_FLOAT
               for(int j = 0; j < bankLength; j++) {
                  if(j == highlight) {
							std::cout<<ALERTTEXT<<(reinterpret_cast<float*>(pdata))[j]<<RESET_COLOR<<((j % 10 == 9) ? '\n' : ' ');
                  } else {
							std::cout<<(reinterpret_cast<float*>(pdata))[j]<<((j % 10 == 9) ? '\n' : ' ');
                  }
               }
					std::cout<<std::endl;
               break;
            case 10: // TID_DOUBLE
               for(int j = 0; j < bankLength; j++) {
                  if(j == highlight) {
							std::cout<<ALERTTEXT<<(reinterpret_cast<double*>(pdata))[j]<<RESET_COLOR<<((j % 10 == 9) ? '\n' : ' ');
                  } else {
							std::cout<<(reinterpret_cast<double*>(pdata))[j]<<((j % 10 == 9) ? '\n' : ' ');
                  }
               }
					std::cout<<std::endl;
               break;
            default:
					std::cout<<"TMidasEvent::Print: Do not know how to print bank of type "<<bankType<<std::endl;
					break;
            }
         }
      }
   }
}

void TMidasEvent::AllocateData()
{
   // Allocates space for the data from the event header if it is a good size
   assert(!fAllocatedByUs);
   assert(IsGoodSize());
   fData = reinterpret_cast<char*>(malloc(fEventHeader.fDataSize)); // NOLINT(cppcoreguidelines-no-malloc)
   assert(fData);
   fAllocatedByUs = true;
}

const char* TMidasEvent::GetBankList() const
{
   return fBankList;
}

int TMidasEvent::SetBankList()
{
   // Sets the bank list by Iterating of the banks.
   // See IterateBank32 and IterateBank
   if(fEventHeader.fEventId <= 0) {
      return 0;
   }

   if(fBankList != nullptr) {
      return fBanksN;
   }

   int listSize = 0;

   fBanksN = 0;

   TMidas_BANK32* pmbk32 = nullptr;
   TMidas_BANK*   pmbk   = nullptr;
   char*          pdata  = nullptr;

   while(true) {
      if(fBanksN * 4 >= listSize) {
         listSize += 400;
         fBankList = reinterpret_cast<char*>(realloc(fBankList, listSize)); // NOLINT(cppcoreguidelines-no-malloc)
      }

      if(IsBank32()) {
         IterateBank32(&pmbk32, &pdata);
         if(pmbk32 == nullptr) {
            break;
         }
         memcpy(fBankList + fBanksN * 4, pmbk32->fName, 4); // NOLINT(cppcoreguidelines-pro-bounds-array-to-pointer-decay)
         fBanksN++;
      } else {
         IterateBank(&pmbk, &pdata);
         if(pmbk == nullptr) {
            break;
         }
         memcpy(fBankList + fBanksN * 4, pmbk->fName, 4); // NOLINT(cppcoreguidelines-pro-bounds-array-to-pointer-decay)
         fBanksN++;
      }
   }

   fBankList[fBanksN * 4] = 0;

   return fBanksN;
}

int TMidasEvent::IterateBank(TMidas_BANK** pbk, char** pdata) const
{
   /// Iterates through banks inside an event. The function can be used
   /// to enumerate all banks of an event.
   /// \param [in] pbk Pointer to the bank header, must be nullptr for the
   /// first call to this function. Returns nullptr if no more banks
   /// \param [in] pdata Pointer to data area of bank. Returns nullptr if no more banks
   /// \returns Size of bank in bytes or 0 if no more banks.
   ///
   auto* event = reinterpret_cast<TMidas_BANK_HEADER*>(fData);

   if(*pbk == nullptr) {
      *pbk = reinterpret_cast<TMidas_BANK*>(event + 1);
   } else {
      *pbk = reinterpret_cast<TMidas_BANK*>(reinterpret_cast<char*>(*pbk + 1) + ((((*pbk)->fDataSize) + 7) & ~7));
   }

   *pdata = reinterpret_cast<char*>((*pbk) + 1);

   if(reinterpret_cast<char*>(*pbk) >=
      reinterpret_cast<char*>(event) + event->fDataSize + sizeof(TMidas_BANK_HEADER)) {
      *pbk   = nullptr;
      *pdata = nullptr;
      return 0;
   }

   return (*pbk)->fDataSize;
}

int TMidasEvent::IterateBank32(TMidas_BANK32** pbk, char** pdata) const
{
   /// See IterateBank()

   auto* event = reinterpret_cast<TMidas_BANK_HEADER*>(fData);
   if(*pbk == nullptr) {
      *pbk = reinterpret_cast<TMidas_BANK32*>(event + 1);
   } else {
      uint32_t length          = (*pbk)->fDataSize;
      uint32_t length_adjusted = (length + 7) & ~7;
      *pbk = reinterpret_cast<TMidas_BANK32*>(reinterpret_cast<char*>(*pbk + 1) + length_adjusted);
   }

   auto* bk4 = reinterpret_cast<TMidas_BANK32*>((reinterpret_cast<char*>(*pbk)) + 4);

   if((*pbk)->fType > 17) {// bad - unknown bank type - it's invalid MIDAS file?
      if(bk4->fType <= 17) {// okey, this is a malformed T2K/ND280 data file
         *pbk = bk4;
      } else {
         // truncate invalid data
         *pbk   = nullptr;
         *pdata = nullptr;
         return 0;
      }
   }

   *pdata = reinterpret_cast<char*>((*pbk) + 1);

   if(reinterpret_cast<char*>(*pbk) >= reinterpret_cast<char*>(event) + event->fDataSize + sizeof(TMidas_BANK_HEADER)) {
      *pbk   = nullptr;
      *pdata = nullptr;
      return 0;
   }

   return (*pbk)->fDataSize;
}

using BYTE = uint8_t;

/// Byte swapping routine.
///
#define QWORD_SWAP(x)                            \
   {                                             \
      BYTE _tmp;                                 \
      _tmp                = *((BYTE*)(x));       \
      *((BYTE*)(x))       = *(((BYTE*)(x)) + 7); \
      *(((BYTE*)(x)) + 7) = _tmp;                \
      _tmp                = *(((BYTE*)(x)) + 1); \
      *(((BYTE*)(x)) + 1) = *(((BYTE*)(x)) + 6); \
      *(((BYTE*)(x)) + 6) = _tmp;                \
      _tmp                = *(((BYTE*)(x)) + 2); \
      *(((BYTE*)(x)) + 2) = *(((BYTE*)(x)) + 5); \
      *(((BYTE*)(x)) + 5) = _tmp;                \
      _tmp                = *(((BYTE*)(x)) + 3); \
      *(((BYTE*)(x)) + 3) = *(((BYTE*)(x)) + 4); \
      *(((BYTE*)(x)) + 4) = _tmp;                \
   }

/// Byte swapping routine.
///
#define DWORD_SWAP(x)                            \
   {                                             \
      BYTE _tmp;                                 \
      _tmp                = *((BYTE*)(x));       \
      *((BYTE*)(x))       = *(((BYTE*)(x)) + 3); \
      *(((BYTE*)(x)) + 3) = _tmp;                \
      _tmp                = *(((BYTE*)(x)) + 1); \
      *(((BYTE*)(x)) + 1) = *(((BYTE*)(x)) + 2); \
      *(((BYTE*)(x)) + 2) = _tmp;                \
   }

/// Byte swapping routine.
///
#define WORD_SWAP(x)                             \
   {                                             \
      BYTE _tmp;                                 \
      _tmp                = *((BYTE*)(x));       \
      *((BYTE*)(x))       = *(((BYTE*)(x)) + 1); \
      *(((BYTE*)(x)) + 1) = _tmp;                \
   }

void TMidasEvent::SwapBytesEventHeader()
{
   // Swaps bytes in the header for endian-ness reasons
   WORD_SWAP(&fEventHeader.fEventId);
   WORD_SWAP(&fEventHeader.fTriggerMask);
   DWORD_SWAP(&fEventHeader.fSerialNumber);
   DWORD_SWAP(&fEventHeader.fTimeStamp);
   DWORD_SWAP(&fEventHeader.fDataSize);
}

int TMidasEvent::SwapBytes(bool force)
{
   // Swaps bytes for endian-ness reasons
   TMidas_BANK_HEADER* pbh = nullptr;
   TMidas_BANK*        pbk = nullptr;
   TMidas_BANK32*      pbk32 = nullptr;
   void*               pdata = nullptr;
   uint16_t            type = 0;

   pbh = reinterpret_cast<TMidas_BANK_HEADER*>(fData);

   uint32_t dssw = pbh->fDataSize;

   DWORD_SWAP(&dssw);

   // only swap if flags in high 16-bit
   //
   if(pbh->fFlags < 0x10000 && !force) {
      return 0;
   }

   if(pbh->fDataSize == 0x6d783f3c) { // string "<xml..." in wrong-endian format
      return 1;
   }

   if(pbh->fDataSize == 0x3c3f786d) { // string "<xml..."
      return 1;
   }

   if(dssw > fEventHeader.fDataSize + 100) { // swapped data size looks wrong. do not swap.
      return 1;
   }

   //
   // swap bank header
   //
   DWORD_SWAP(&pbh->fDataSize);
   DWORD_SWAP(&pbh->fFlags);
   //
   // check for 32-bit banks
   //
   bool b32 = IsBank32();

   pbk   = reinterpret_cast<TMidas_BANK*>(pbh + 1);
   pbk32 = reinterpret_cast<TMidas_BANK32*>(pbk);
   //
   // scan event
   //
   while(reinterpret_cast<char*>(pbk) < reinterpret_cast<char*>(pbh) + pbh->fDataSize + sizeof(TMidas_BANK_HEADER)) {
      //
      // swap bank header
      //
      if(b32) {
         DWORD_SWAP(&pbk32->fType);
         DWORD_SWAP(&pbk32->fDataSize);
         pdata = pbk32 + 1;
         type  = static_cast<uint16_t>(pbk32->fType);
      } else {
         WORD_SWAP(&pbk->fType);
         WORD_SWAP(&pbk->fDataSize);
         pdata = pbk + 1;
         type  = pbk->fType;
      }
      //
      // pbk points to next bank
      //
      if(b32) {
         assert(pbk32->fDataSize < fEventHeader.fDataSize + 100);
         pbk32 = reinterpret_cast<TMidas_BANK32*>(reinterpret_cast<char*>(pbk32 + 1) + (((pbk32->fDataSize) + 7) & ~7));
         pbk   = reinterpret_cast<TMidas_BANK*>(pbk32);
      } else {
         assert(pbk->fDataSize < fEventHeader.fDataSize + 100);
         pbk   = reinterpret_cast<TMidas_BANK*>(reinterpret_cast<char*>(pbk + 1) + (((pbk->fDataSize) + 7) & ~7));
         pbk32 = reinterpret_cast<TMidas_BANK32*>(pbk);
      }

      switch(type) {
      case 4:
      case 5:
         while(pdata < pbk) {
            WORD_SWAP(pdata);
            pdata = (reinterpret_cast<char*>(pdata)) + 2;
         }
         break;
      case 6:
      case 7:
      case 8:
      case 9:
         while(pdata < pbk) {
            DWORD_SWAP(pdata);
            pdata = (reinterpret_cast<char*>(pdata)) + 4;
         }
         break;
      case 10:
         while(pdata < pbk) {
            QWORD_SWAP(pdata);
            pdata = (reinterpret_cast<char*>(pdata)) + 8;
         }
         break;
      }
   }
   return 1;
}

