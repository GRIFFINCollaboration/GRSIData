#include "TRF.h"

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
   static_cast<TRF&>(rhs).fPeriod    = fPeriod;
}

TRF::TRF(const TRF& rhs) : TDetector(rhs)
{
   rhs.Copy(*this);
}

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

      //the phase shift (in cfd units) is stored in the fragment as the cfd
      //the period (in ns) is stored in the fragment as the charge

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
	Print(std::cout);
}

void TRF::Print(std::ostream& out) const
{
	std::ostringstream str;
   str<<"time =      "<<fTime<<std::endl;
   str<<"timestamp = "<<fTimeStamp<<std::endl;
   str<<"midastime = "<<fMidasTime<<std::endl;
	out<<str.str();
}
