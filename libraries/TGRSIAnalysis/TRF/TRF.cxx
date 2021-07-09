#include "TRF.h"

/// \cond CLASSIMP
ClassImp(TRF)
/// \endcond

Double_t TRF::fPeriod;

TRF::TRF()
{
   Clear();
}

void TRF::Copy(TObject& rhs) const
{
   TDetector::Copy(rhs);
   static_cast<TRF&>(rhs).fMidasTime = fMidasTime;
   static_cast<TRF&>(rhs).fTimeStamp = fTimeStamp;
   static_cast<TRF&>(rhs).fTime      = fTime;
}

TRF::TRF(const TRF& rhs) : TDetector()
{
   rhs.Copy(*this);
}

TRF::~TRF() = default;

void TRF::AddFragment(const std::shared_ptr<const TFragment>& frag, TChannel*)
{

   fMidasTime = frag->GetDaqTimeStamp();
   fTimeStamp = frag->GetTimeStamp();

   TPulseAnalyzer pulse(*frag);
   if(pulse.IsSet()) {
      fTime      = pulse.fit_rf(fPeriod * 0.2); // period taken in half ticks... for reasons
   }else{
      //special RF scaler format
      //no waveform, only fit parameters

      //the phase shift (in cfd units) is stored as the cfd
      //the period (in ns) is stored as the charge

      fTime = frag->GetCfd() / 1.6; //convert from cfd units to ns
      fPeriod = frag->GetCharge();

   }
}

void TRF::Clear(Option_t*)
{
   fMidasTime = 0.0;
   fTimeStamp = 0.0;
   fTime      = 0.0;

   fPeriod = 84.8417;
}

void TRF::Print(Option_t*) const
{
   printf("time = %f\n", fTime);
   printf("timestamp = %ld\n", fTimeStamp);
   printf("midastime = %ld\n", fMidasTime);
}
