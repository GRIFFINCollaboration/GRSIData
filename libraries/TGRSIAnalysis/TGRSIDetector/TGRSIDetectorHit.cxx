#include "TGRSIDetectorHit.h"

#include "TChannel.h"
#include "TGRSIMnemonic.h"

/// \cond CLASSIMP
ClassImp(TGRSIDetectorHit)
/// \endcond

TGRSIDetectorHit::TGRSIDetectorHit(const int& Address) : TDetectorHit()
{
   /// Default constructor
   Clear();
   fAddress = Address;

#if MAJOR_ROOT_VERSION < 6
   Class()->IgnoreTObjectStreamer(kTRUE);
#endif
}

TGRSIDetectorHit::TGRSIDetectorHit(const TGRSIDetectorHit& rhs, bool copywave) : TDetectorHit(rhs)
{
   /// Default Copy constructor
   rhs.Copy(*this);
   if(copywave) {
      rhs.CopyWave(*this);
   }
   ClearTransients();

#if MAJOR_ROOT_VERSION < 6
   Class()->IgnoreTObjectStreamer(kTRUE);
#endif
}

TGRSIDetectorHit::~TGRSIDetectorHit()
{
   // Default destructor
}

Double_t TGRSIDetectorHit::GetTime(const ETimeFlag&, Option_t*) const
{
   if(IsTimeSet()) {
      return fTime;
   }

   TChannel* channel = GetChannel();
   if(channel == nullptr) {
      Error("GetTime", "No TChannel exists for address 0x%08x", GetAddress());
		return SetTime(static_cast<Double_t>(((GetTimeStamp()) + gRandom->Uniform()) * GetTimeStampUnit()));
   }
	
   switch(static_cast<EDigitizer>(channel->GetDigitizerType())) {
		Double_t dTime;
		case EDigitizer::kGRF16:
			// we need to zero the lowest 18 bits of the timestamp as those are included in the CFD value
			// TODO: what happens close to the wrap-around of those 18 bits??? This only happens every 2^18 * 10e-8 so 2.5 ms so 400 Hz
			dTime = (GetTimeStamp() & (~0x3ffff)) * GetTimeStampUnit() + channel->CalibrateCFD((GetCfd() + gRandom->Uniform()) / 1.6); // CFD is in 10/16th of a nanosecond
			return SetTime(dTime - channel->GetTZero(GetEnergy()) - channel->GetTimeOffset());
		case EDigitizer::kGRF4G:
			dTime = GetTimeStamp()*GetTimeStampUnit() + channel->CalibrateCFD((static_cast<Int_t>(fCfd) >> 22) + ((static_cast<Int_t>(fCfd) & 0x3fffff) + gRandom->Uniform()) / 256.);
			return SetTime(dTime - channel->GetTZero(GetEnergy()) - channel->GetTimeOffset());
		case EDigitizer::kTIG10:
			dTime = (GetTimeStamp() & (~0x7fffff))*GetTimeStampUnit() +	channel->CalibrateCFD((GetCfd() + gRandom->Uniform()) / 1.6); // CFD is in 10/16th of a nanosecond
			//channel->CalibrateCFD((GetCfd() & (~0xf) + gRandom->Uniform()) / 1.6); // PBender suggests this.
			return SetTime(dTime - channel->GetTZero(GetEnergy()) - channel->GetTimeOffset());
      case EDigitizer::kCaen:
			//10 bit CFD for 0-2ns => divide by 512
			dTime = GetTimeStamp()*GetTimeStampUnit() + channel->CalibrateCFD((GetCfd() + gRandom->Uniform()) / 512.);
			return SetTime(dTime - channel->GetTZero(GetEnergy()) - channel->GetTimeOffset());
		default:
			dTime = static_cast<Double_t>(((GetTimeStamp()) + gRandom->Uniform())*GetTimeStampUnit());
			return SetTime(dTime - channel->GetTZero(GetEnergy()) - channel->GetTimeOffset());
	}
   return 0.;
}

