#include "TTip.h"
#include "TTipHit.h"
#include "TGRSIOptions.h"

////////////////////////////////////////////////////////////
//
// TTipHit
//
// The TTipHit class defines the observables and algorithms used
// when analyzing TIP hits. It includes detector positions, etc.
//
////////////////////////////////////////////////////////////

TTipHit::TTipHit()
{
#if ROOT_VERSION_CODE < ROOT_VERSION(6,0,0)
   Class()->IgnoreTObjectStreamer(kTRUE);
#endif
   Clear();
}

TTipHit::TTipHit(const TFragment& frag)
{
	frag.Copy(*this);
   // SetVariables(frag);
   if(TGRSIOptions::AnalysisOptions()->IsWaveformFitting() && !IsCsI()) {
      SetWavefit(frag);
   } else if(TGRSIOptions::AnalysisOptions()->IsWaveformFitting() && IsCsI()) {
      SetPID(frag);
   }
}

TTipHit::~TTipHit() = default;

TTipHit::TTipHit(const TTipHit& rhs) : TDetectorHit(rhs)
{
#if ROOT_VERSION_CODE < ROOT_VERSION(6,0,0)
   Class()->IgnoreTObjectStreamer(kTRUE);
#endif
   Clear();
   rhs.Copy(*this);
}

void TTipHit::Copy(TObject& rhs) const
{
   TDetectorHit::Copy(rhs);
   TDetectorHit::CopyWave(rhs); //waveforms are basically always needed (for TIP PID)
   static_cast<TTipHit&>(rhs).fFilter     = fFilter;
   static_cast<TTipHit&>(rhs).fPID        = fPID;
   static_cast<TTipHit&>(rhs).fTipChannel = fTipChannel;
   static_cast<TTipHit&>(rhs).fTimeFit    = fTimeFit;
   static_cast<TTipHit&>(rhs).fFitType    = fFitType;
   static_cast<TTipHit&>(rhs).fSig2Noise  = fSig2Noise;
   static_cast<TTipHit&>(rhs).fChiSq      = fChiSq;
}

void TTipHit::Copy(TObject& rhs, bool waveform) const
{
	Copy(rhs);
	if(waveform) {
		CopyWave(rhs);
	}
}

TVector3 TTipHit::GetPosition() const
{
   return TTip::GetPosition(*this);
}

bool TTipHit::InFilter(Int_t)
{
   // check if the desired filter is in wanted filter;
   // return the answer;
   return true;
}

void TTipHit::Clear(Option_t*)
{
   fFilter     = 0;
   fPID        = 0;
   fTipChannel = 0;
   fTimeFit    = 0;
}

void TTipHit::Print(Option_t*) const
{
	Print(std::cout);
}

void TTipHit::Print(std::ostream& out) const
{
	std::ostringstream str;
   str<<"Tip Detector: "<<GetDetector()<<std::endl;
   str<<"Tip hit energy: "<<GetEnergy()<<std::endl;
   str<<"Tip hit time:   "<<GetTime()<<std::endl;
	out<<str.str();
}

void TTipHit::SetWavefit(const TFragment& frag)
{
   TPulseAnalyzer pulse(frag);
   if(pulse.IsSet()) {
      fTimeFit   = pulse.fit_newT0();
      fSig2Noise = pulse.get_sig2noise();
   }
}

void TTipHit::SetPID(const TFragment& frag)
{
   TPulseAnalyzer pulse(frag);
   if(pulse.IsSet()) {
      fPID     = pulse.CsIPID();
      fTimeFit = pulse.CsIt0();
      fChiSq   = pulse.GetCsIChiSq();
      fFitType = pulse.GetCsIFitType();
   }
}
