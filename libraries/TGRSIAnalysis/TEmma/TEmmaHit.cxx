#include "TEmma.h"
#include "TEmmaHit.h"
#include "Globals.h"

double TEmmaHit::fLdelay = 40;
double TEmmaHit::fRdelay = 20;
double TEmmaHit::fTdelay = 10;
double TEmmaHit::fBdelay = 20;

TEmmaHit::TEmmaHit()
{
#if ROOT_VERSION_CODE < ROOT_VERSION(6,0,0)
   Class()->IgnoreTObjectStreamer(kTRUE);
#endif
   Clear();
}

TEmmaHit::TEmmaHit(const TEmmaHit& rhs) : TDetectorHit(rhs)
{
#if ROOT_VERSION_CODE < ROOT_VERSION(6,0,0)
   Class()->IgnoreTObjectStreamer(kTRUE);
#endif
   Clear();
   rhs.Copy(*this);
}

void TEmmaHit::Copy(TObject& rhs) const
{
   TDetectorHit::Copy(rhs);
   static_cast<TEmmaHit&>(rhs).fFilter = fFilter;
   static_cast<TEmmaHit&>(rhs).fTdcNumber = fTdcNumber;
   static_cast<TEmmaHit&>(rhs).fLeft = fLeft;
   static_cast<TEmmaHit&>(rhs).fRight = fRight;
   static_cast<TEmmaHit&>(rhs).fTop = fTop;
   static_cast<TEmmaHit&>(rhs).fBottom = fBottom;
   static_cast<TEmmaHit&>(rhs).fAnodeTrigger = fAnodeTrigger;
   static_cast<TEmmaHit&>(rhs).fFail = fFail;
   static_cast<TEmmaHit&>(rhs).fICSum = fICSum;
}

void TEmmaHit::Copy(TObject& rhs, bool waveform) const
{
	Copy(rhs);
	if(waveform) {
		CopyWave(rhs);
	}
}

bool TEmmaHit::InFilter(Int_t)
{
   // check if the desired filter is in wanted filter;
   // return the answer;
   return true;
}

void TEmmaHit::Clear(Option_t* opt)
{
   TDetectorHit::Clear(opt); // clears the base (address, position and waveform)
   fFilter = 0;
}

void TEmmaHit::Print(Option_t*) const
{
	Print(std::cout);
}

void TEmmaHit::Print(std::ostream& out) const
{
	std::ostringstream str;
	str<<"================"<<std::endl;
	str<<"not yet written."<<std::endl;
	str<<"================"<<std::endl;
	out<<str.str();
}

TVector3 TEmmaHit::GetPosition() const
{
   return TEmma::GetPosition(GetLeft(),GetRight(),GetTop(),GetBottom(),fLdelay,fRdelay,fTdelay,fBdelay);
}

TVector3 TEmmaHit::GetPosition(double delayL, double delayR, double delayT, double delayB) const
{
   return TEmma::GetPosition(GetLeft(),GetRight(),GetTop(),GetBottom(),delayL,delayR,delayT,delayB);
}
