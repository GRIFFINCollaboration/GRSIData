#include "TDemand.h"
#include "TDemandHit.h"
#include "Globals.h"
#include "TClass.h"
#include "TGRSIOptions.h"

/// \cond CLASSIMP
//ClassImp(TDemandHit)
/// \endcond


TDemandHit::TDemandHit()
{
   Clear();
}

TDemandHit::TDemandHit(const TDemandHit& rhs) : TDetectorHit(rhs)
{
   Clear();
   rhs.Copy(*this);
}

TDemandHit::TDemandHit(const TFragment& frag)
{
   frag.Copy(*this);
   frag.CopyWave(*this);
   /*if(TDemandHit::SetWave()) {
      if(frag.GetWaveform()->empty()) {
         std::cout << "Warning, TDemandHit::SetWave() set, but data waveform size is zero!" << std::endl;
      }
      frag.CopyWave(*this);

      //if(!GetWaveform()->empty()) {
      //   AnalyzeWaveform();
      //}
   }*/
}

void TDemandHit::Copy(TObject& rhs) const
{
   TDetectorHit::Copy(rhs);
   TDetectorHit::CopyWave(rhs);
   static_cast<TDemandHit&>(rhs).fFilter = fFilter;
}

void TDemandHit::Copy(TObject& rhs, bool waveform) const
{
   Copy(rhs);
   if(waveform) {
      CopyWave(rhs);
   }
}

bool TDemandHit::InFilter(Int_t)
{
   // check if the desired filter is in wanted filter;
   // return the answer;
   return true;
}

void TDemandHit::Clear(Option_t* opt)
{
   TDetectorHit::Clear(opt);   // clears the base (address, position and waveform)
   fFilter = 0;
}

void TDemandHit::Print(Option_t*) const
{
   Print(std::cout);
}

void TDemandHit::Print(std::ostream& out) const
{
   std::ostringstream str;
   str << "Demand Detector: " << GetDetector() << std::endl;
   str << "Demand Energy:   " << GetEnergy() << std::endl;
   str << "Demand hit time: " << GetTime() << std::endl;
   out << str.str();
}

TVector3 TDemandHit::GetPosition(int DetNbr) const
{

   if (1 > DetNbr || 6 < DetNbr){ //if detector number is not 1,2,3,4,5,6, then return a 0 vector
      return TVector3(0,0,0);
   }

   else{
      return TVector3(fXPositions[DetNbr-1],fYPositions[DetNbr-1],fZPositions[DetNbr-1]); //-1 because we index the detectors at 1 but the vector at 0
   }
}

TVector3 TDemandHit::GetPosition() const
{
   return TDemand::GetPosition(*this);
}
