#ifndef TMIDASEVENT_H
#define TMIDASEVENT_H

/** \addtogroup Sorting
 *  @{
 */

#include "Globals.h"
#include "TMidasEventHeader.h"
#include "TDataParser.h"

#include "TRawEvent.h"

/////////////////////////////////////////////////////////////////
///
/// \class TMidasEvent
///
/// C++ class representing one midas event.
///
/// Objects of this class are created by reading
/// midas events from a file, by reading
/// them from a midas shared memory buffer or by
/// receiving them through the mserver
///
/// Contains the information within a Midas Event. This
/// usually includes a header, midas information such as timestamp
/// bank id, etc. And the bank data. The bank data is usually
/// the information supplied by either a scaler or the
/// experimental DAQ system.
///
/////////////////////////////////////////////////////////////////

/// MIDAS event

class TMidasEvent : public TRawEvent {
public:
#include "TMidasBanks.h"

   // houskeeping functions
   TMidasEvent();                     ///< default constructor
   TMidasEvent(const TMidasEvent&);   ///< copy constructor
   TMidasEvent(TMidasEvent&&) noexcept = default;
   TMidasEvent& operator=(const TMidasEvent&);   ///< assignement operator
   TMidasEvent& operator=(TMidasEvent&&) noexcept = default;
   ~TMidasEvent() override;                              ///< destructor
   void Clear(Option_t* opt = "") override;              ///< clear event for reuse
   void Copy(TObject&) const override;                   ///< copy helper
   void Print(const char* option = "") const override;   ///< show all event information

   // get event information

   uint16_t GetEventId() const;              ///< return the event id
   uint16_t GetTriggerMask() const;          ///< return the triger mask
   uint32_t GetSerialNumber() const;         ///< return the serial number
   uint32_t GetTimeStamp() const override;   ///< return the time stamp (unix time in seconds)
   uint32_t GetDataSize() const override;    ///< return the event size

   // get data banks

   const char* GetBankList() const;   ///< return a list of data banks
   int         FindBank(const char* name, int* bklen, int* bktype, void** pdata) const;
   int         LocateBank(const void* unused, const char* name, void** pdata) const;

   bool IsBank32() const;                                     ///< returns "true" if event uses 32-bit banks
   int  IterateBank(TMidas_BANK**, char** pdata) const;       ///< iterate through 16-bit data banks
   int  IterateBank32(TMidas_BANK32**, char** pdata) const;   ///< iterate through 32-bit data banks

   // helpers for event creation

   TMidas_EVENT_HEADER* GetEventHeader();     ///< return pointer to the event header
   char*                GetData() override;   ///< return pointer to the data buffer

   void AllocateData();                       ///< allocate data buffer using the existing event header
   void SetData(uint32_t size, char* data);   ///< set an externally allocated data buffer

   int  SetBankList();        ///< create the list of data banks, return number of banks
   bool IsGoodSize() const;   ///< validate the event length

   void SwapBytesEventHeader();     ///< convert event header between little-endian (Linux-x86) and big endian (MacOS-PPC)
   int  SwapBytes(bool) override;   ///< convert event data between little-endian (Linux-x86) and big endian (MacOS-PPC)

private:
   TMidas_EVENT_HEADER fEventHeader;     ///< event header
   char*               fData;            ///< event data buffer
   int                 fBanksN;          ///< number of banks in this event
   char*               fBankList;        ///< list of bank names in this event
   bool                fAllocatedByUs;   ///< "true" if we own the data buffer

   /// \cond CLASSIMP
   ClassDefOverride(TMidasEvent, 0)   // All of the data contained in a Midas Event // NOLINT(readability-else-after-return)
   /// \endcond
};
/*! @} */
#endif   // TMidasEvent.h
