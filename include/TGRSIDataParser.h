#ifndef TGRSIDARAPARSER_H
#define TGRSIDARAPARSER_H

/** \addtogroup Sorting
 *  @{
 */

/////////////////////////////////////////////////////////////////
///
/// \class TGRSIDataParser
///
/// The TGRSIDataParser is the DAQ dependent part of GRSISort.
/// It takes a "DAQ-dependent"-flavoured MIDAS file and
/// converts it into a generic TFragment that the rest of
/// GRSISort can deal with. This is where event word masks
/// are applied, and any changes to the event format must
/// be implemented.
///
/////////////////////////////////////////////////////////////////

#include "Globals.h"
#include <ctime>
#include <sstream>
#include <vector>
#include <map>
#include <limits>

#ifndef __CINT__
#include <memory>
#endif

#include "TDataParser.h"
#include "TChannel.h"
#include "TFragment.h"
#include "TPPG.h"
#include "TScaler.h"
#include "TFragmentMap.h"
#include "ThreadsafeQueue.h"
#include "TEpicsFrag.h"
#include "TGRSIOptions.h"
#include "TRawEvent.h"
#include "TMidasEvent.h"

class TGRSIDataParser : public TDataParser {
public:
   TGRSIDataParser();
   TGRSIDataParser(const TGRSIDataParser&) = default;
	TGRSIDataParser(TGRSIDataParser&&) noexcept = default;
	TGRSIDataParser& operator=(const TGRSIDataParser&) = delete;
	TGRSIDataParser& operator=(TGRSIDataParser&&) noexcept = delete;
   ~TGRSIDataParser() override = default;

   // ENUM(EBank, char, kWFDN,kGRF1,kGRF2,kGRF3,kFME0,kFME1,kFME2,kFME3);
   enum class EBank { kWFDN = 0, kGRF1 = 1, kGRF2 = 2, kGRF3 = 3, kGRF4 = 4, kFME0 = 5, kFME1 = 6, kFME2 = 7, kFME3 = 8, kMADC = 9, kEMMT = 10};

   enum class EDataParserState {
      kGood,
      kBadHeader,
      kMissingWords,
      kBadScalerLowTS,
      kBadScalerValue,
      kBadScalerHighTS,
      kBadScalerType,
      kBadTriggerId,
      kBadLowTS,
      kBadHighTS,
      kSecondHeader,
      kWrongNofWords,
      kNotSingleCfd,
      kSizeMismatch,
      kBadFooter,
      kFault,
      kMissingPsd,
      kMissingCfd,
      kMissingCharge,
      kBadBank,
      kBadModuleType,
      kEndOfData,
      kBadRFScalerWord,
      kUndefined
   };

#ifndef __CINT__
	int Process(std::shared_ptr<TRawEvent>) override;
	int ProcessGriffin(uint32_t* data, const int& size, const EBank& bank, std::shared_ptr<TMidasEvent>& event);
        int TigressDataToFragment(uint32_t* data, int size, std::shared_ptr<TMidasEvent>& event);
	int CaenPsdToFragment(uint32_t* data, int size, std::shared_ptr<TMidasEvent>& event);
	int CaenPhaToFragment(uint32_t* data, int size, std::shared_ptr<TMidasEvent>& event);
	int EmmaMadcDataToFragment(const uint32_t* data, int size, std::shared_ptr<TMidasEvent>& event);
	int EmmaTdcDataToFragment(uint32_t* data, int size, std::shared_ptr<TMidasEvent>& event);
	int EmmaRawDataToFragment(uint32_t* data, int size, std::shared_ptr<TMidasEvent>& event);
	int EmmaSumDataToFragment(uint32_t* data, int size, std::shared_ptr<TMidasEvent>& event);
#endif

   int GriffinDataToFragment(uint32_t* data, int size, EBank bank, unsigned int midasSerialNumber = 0, time_t midasTime = 0);
   int GriffinDataToPPGEvent(uint32_t* data, int size, unsigned int midasSerialNumber = 0, time_t midasTime = 0);
   int GriffinDataToScalerEvent(uint32_t* data, int address);

   int RFScalerToFragment(uint32_t* data, int size, const std::shared_ptr<TFragment>& frag);

   int EPIXToScalar(float* data, int size, unsigned int midasSerialNumber = 0, time_t midasTime = 0);
   int SCLRToScalar(uint32_t* data, int size, unsigned int midasSerialNumber = 0, time_t midasTime = 0);
   int EightPIDataToFragment(uint32_t stream, uint32_t* data, int size, unsigned int midasSerialNumber = 0, time_t midasTime = 0);

private:
	EDataParserState fState;
	bool fIgnoreMissingChannel; ///< flag that's set to TGRSIOptions::IgnoreMissingChannel
#ifndef __CINT__
   void SetTIGWave(uint32_t, const std::shared_ptr<TFragment>&);
   void SetTIGAddress(uint32_t, const std::shared_ptr<TFragment>&);
   void SetTIGCfd(uint32_t, const std::shared_ptr<TFragment>&);
   void SetTIGCharge(uint32_t, const std::shared_ptr<TFragment>&);
   void SetTIGLed(uint32_t, const std::shared_ptr<TFragment>&);

   bool SetTIGTriggerID(uint32_t, const std::shared_ptr<TFragment>&);
   bool SetTIGTimeStamp(uint32_t*, const std::shared_ptr<TFragment>&);

   bool SetGRIFHeader(uint32_t, const std::shared_ptr<TFragment>&, EBank);
   bool SetGRIFPrimaryFilterPattern(uint32_t, const std::shared_ptr<TFragment>&, EBank);
   bool SetGRIFPrimaryFilterId(uint32_t, const std::shared_ptr<TFragment>&);
   bool SetGRIFChannelTriggerId(uint32_t, const std::shared_ptr<TFragment>&);
   bool SetGRIFTimeStampLow(uint32_t, const std::shared_ptr<TFragment>&);
   bool SetGRIFNetworkPacket(uint32_t, const std::shared_ptr<TFragment>&);

   bool SetGRIFPsd(uint32_t, const std::shared_ptr<TFragment>&);
   bool SetGRIFCc(uint32_t, const std::shared_ptr<TFragment>&);

   bool SetGRIFWaveForm(uint32_t, const std::shared_ptr<TFragment>&);
   bool SetGRIFDeadTime(uint32_t, const std::shared_ptr<TFragment>&);
#endif

   bool SetNewPPGPattern(uint32_t, TPPGData*);
   bool SetOldPPGPattern(uint32_t, TPPGData*);
   bool SetPPGNetworkPacket(uint32_t, TPPGData*);
   bool SetPPGLowTimeStamp(uint32_t, TPPGData*);
   bool SetPPGHighTimeStamp(uint32_t, TPPGData*);
   bool SetScalerNetworkPacket(uint32_t, TScalerData*);
   bool SetScalerLowTimeStamp(uint32_t, TScalerData*);
   bool SetScalerHighTimeStamp(uint32_t, TScalerData*, int&);
   bool SetScalerValue(int, uint32_t, TScalerData*);
};
/*! @} */
#endif
