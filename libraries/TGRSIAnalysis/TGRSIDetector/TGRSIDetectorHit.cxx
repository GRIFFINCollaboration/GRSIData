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
      return SetTime(10. * (static_cast<Double_t>((GetTimeStamp()) + gRandom->Uniform())));
   }
   switch(static_cast<EDigitizer>(channel->GetDigitizerType())) {
		Double_t dTime;
		case EDigitizer::kGRF16:
		dTime = (GetTimeStamp() & (~0x3ffff)) * 10. +
		channel->CalibrateCFD((GetCfd() + gRandom->Uniform()) / 1.6); // CFD is in 10/16th of a nanosecond
		return SetTime(dTime - 10. * (channel->GetTZero(GetEnergy())));
		case EDigitizer::kGRF4G:
		dTime = GetTimeStamp() * 10. + channel->CalibrateCFD((fCfd >> 22) + ((fCfd & 0x3fffff) + gRandom->Uniform()) / 256.);
		return SetTime(dTime - 10. * (channel->GetTZero(GetEnergy())));
		case EDigitizer::kTIG10:
		dTime = (GetTimeStamp() & (~0x7fffff)) * 10. +
		channel->CalibrateCFD((GetCfd() + gRandom->Uniform()) / 1.6); // CFD is in 10/16th of a nanosecond
		//channel->CalibrateCFD((GetCfd() & (~0xf) + gRandom->Uniform()) / 1.6); // PBender suggests this.
		return SetTime(dTime - 10. * (channel->GetTZero(GetEnergy())));
      case EDigitizer::kCaen:
      //10 bit CFD for 0-2ns => divide by 512
      dTime = GetTimeStamp() * 10. + channel->CalibrateCFD((GetCfd() + gRandom->Uniform()) / 512.);
      return SetTime(dTime - 10. * (channel->GetTZero(GetEnergy())));
		default:
		dTime = static_cast<Double_t>((GetTimeStamp()) + gRandom->Uniform());
		return SetTime(10. * (dTime - channel->GetTZero(GetEnergy())));
	}
   return 0.;
}
