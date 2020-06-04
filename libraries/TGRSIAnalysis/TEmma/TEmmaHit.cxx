#include "TEmma.h"
#include "TEmmaHit.h"
#include "Globals.h"

/// \cond CLASSIMP
ClassImp(TEmmaHit)
/// \endcond

double TEmmaHit::fLdelay = 40;
double TEmmaHit::fRdelay = 20;
double TEmmaHit::fTdelay = 10;
double TEmmaHit::fBdelay = 20;

TEmmaHit::TEmmaHit()
   : TGRSIDetectorHit()
{
#if MAJOR_ROOT_VERSION < 6
   Class()->IgnoreTObjectStreamer(kTRUE);
#endif
   Clear();
}

TEmmaHit::TEmmaHit(const TEmmaHit& rhs) : TGRSIDetectorHit()
{
#if MAJOR_ROOT_VERSION < 6
   Class()->IgnoreTObjectStreamer(kTRUE);
#endif
   Clear();
   rhs.Copy(*this);
}

TEmmaHit::~TEmmaHit() = default;

void TEmmaHit::Copy(TObject& rhs) const
{
   TGRSIDetectorHit::Copy(rhs);
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

bool TEmmaHit::InFilter(Int_t)
{
   // check if the desired filter is in wanted filter;
   // return the answer;
   return true;
}

void TEmmaHit::Clear(Option_t* opt)
{
   TGRSIDetectorHit::Clear(opt); // clears the base (address, position and waveform)
   fFilter = 0;
}

void TEmmaHit::Print(Option_t*) const
{
	printf("================\n");
	printf("not yet written.\n");
	printf("================\n");
}

TVector3 TEmmaHit::GetPosition() const
{
   return TEmma::GetPosition(GetLeft(),GetRight(),GetTop(),GetBottom(),fLdelay,fRdelay,fTdelay,fBdelay);
}

TVector3 TEmmaHit::GetPosition(double delayL, double delayR, double delayT, double delayB) const
{
   return TEmma::GetPosition(GetLeft(),GetRight(),GetTop(),GetBottom(),delayL,delayR,delayT,delayB);
}
