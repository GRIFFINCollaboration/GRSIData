#include "TTigressHit.h"

#include "TClass.h"

#include "TTigress.h"

/// \cond CLASSIMP
ClassImp(TTigressHit)
/// \endcond

TTigressHit::TTigressHit()
{
   Clear();
}

TTigressHit::~TTigressHit() = default;

TTigressHit::TTigressHit(const TTigressHit& rhs) : TDetectorHit()
{
   rhs.Copy(*this);
}

TTigressHit::TTigressHit(const TFragment& frag) : TDetectorHit(frag)
{
}

void TTigressHit::CopyFragment(const TFragment& frag)
{
   frag.Copy(*this);
}

void TTigressHit::Clear(Option_t* opt)
{
   TDetectorHit::Clear(opt);
   fTimeFit   = 0.0;
   fSig2Noise = 0.0;

	fCoreSet  = false;
   fBgoFired = false;
   fSegments.clear();
}

void TTigressHit::Copy(TObject& rhs) const
{
   TDetectorHit::Copy(rhs);
   static_cast<TTigressHit&>(rhs).fTimeFit  = fTimeFit;
   static_cast<TTigressHit&>(rhs).fSegments = fSegments;
   static_cast<TTigressHit&>(rhs).fCoreSet  = fCoreSet;
   static_cast<TTigressHit&>(rhs).fBgoFired = fBgoFired;
}

TVector3 TTigressHit::GetPosition(Double_t dist) const
{
   return TTigress::GetPosition(*this, dist);
}

TVector3 TTigressHit::GetLastPosition(Double_t dist) const
{
   const TDetectorHit* seg;
   if(GetNSegments() > 0) {
      seg = &GetSegmentHit(GetNSegments() - 1); // returns the last segment in the segment vector.
   } else {
      seg = this; // if no segments, use the core. pcb.
   }

   return TTigress::GetPosition(seg->GetDetector(), seg->GetCrystal(), seg->GetSegment(), dist);
}

void TTigressHit::Print(Option_t* opt) const
{
   TString sopt(opt);
   printf("==== TigressHit @ 0x%p\n ====", (void*)this);
   printf("\t%s\n", GetName());
   printf("\tCharge: %f\n", GetCharge());
   printf("\tEnergy: %.2f\n", GetEnergy());
   printf("\tTime:   %.2f\n", GetTime());
   printf("\tCore set: %s\n", CoreSet() ? "true" : "false");
   printf("\tBGO Fired: %s\n", BGOFired() ? "true" : "false");
   std::cout<<"\tTime:   "<<GetTimeStamp()<<"\n";
   printf("\thit contains %i segments.\n", GetNSegments());
   if(sopt.Contains("all")) {
      printf("Name           Charge\n");
      for(int x = 0; x < GetNSegments(); x++) {
         printf("\t\t%s  |   %f\n", GetSegmentHit(x).GetName(), GetSegmentHit(x).GetCharge());
      }
      GetPosition().Print();
   }
   printf("============================\n");
}

bool TTigressHit::Compare(const TTigressHit& lhs, const TTigressHit& rhs)
{
   if(lhs.GetDetector() == rhs.GetDetector()) {
      return (lhs.GetCrystal() < rhs.GetCrystal());
   }
   return (lhs.GetDetector() < rhs.GetDetector());
}

bool TTigressHit::CompareEnergy(const TTigressHit& lhs, const TTigressHit& rhs)
{
   return (lhs.GetEnergy()) > rhs.GetEnergy();
}

void TTigressHit::SumHit(TTigressHit* hit)
{
   if(this != hit) {

      // Should always be true when called by addback construction due to energy ordering during detector construction
      if(GetEnergy() > hit->GetEnergy()) {
         SetTime(GetTime()); // Needs to be call before energy sum to ensure and kIsTimeSet using original energy
                             // for any adjustment
         for(int x = 0; x < hit->GetNSegments(); x++) {
            AddSegment((hit->fSegments[x]));
         }
      } else {
         SetTime(hit->GetTime());
         SetAddress(hit->GetAddress());
         SetCfd(hit->GetCfd());

         // Maybe overkill, but consistent
         std::vector<TDetectorHit> fSegmentHold = hit->fSegments;
         for(int x = 0; x < GetNSegments(); x++) {
            fSegmentHold.push_back(fSegments[x]);
         }
         fSegments = fSegmentHold;
      }

      SetEnergy(GetEnergy() + hit->GetEnergy());

      if(hit->BGOFired()) {
         SetBGOFired(true);
      }
   }
}

void TTigressHit::SetWavefit(const TFragment& frag)
{
   TPulseAnalyzer pulse(frag);
   if(pulse.IsSet()) {
      fTimeFit   = pulse.fit_newT0();
      fSig2Noise = pulse.get_sig2noise();
   }
}

void TTigressHit::SetWavefit()
{
   TPulseAnalyzer pulse(*GetWaveform(), 0, GetName());
   if(pulse.IsSet()) {
      fTimeFit   = pulse.fit_newT0();
      fSig2Noise = pulse.get_sig2noise();
   }
}
