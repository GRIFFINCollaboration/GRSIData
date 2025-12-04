#include "TTigressHit.h"

#include <cmath>
#include <iostream>

#include "TTigress.h"
#include "TPulseAnalyzer.h"

TTigressHit::TTigressHit()
{
   /// Default Ctor. Ignores TObject Streamer in ROOT < 6.
   Clear();
}

TTigressHit::TTigressHit(const TTigressHit& rhs) : TDetectorHit(rhs)
{
   /// Copy Ctor. Ignores TObject Streamer in ROOT < 6.
   Clear();
   rhs.Copy(*this);
}

TTigressHit::TTigressHit(const TFragment& frag)
{
   frag.Copy(*this);
   SetNPileUps(frag.GetNumberOfPileups());
}

void TTigressHit::CopyFragment(const TFragment& frag)
{
   frag.Copy(*this);
   SetNPileUps(frag.GetNumberOfPileups());
}

void TTigressHit::Copy(TObject& rhs) const
{
   TDetectorHit::Copy(rhs);
   static_cast<TTigressHit&>(rhs).fFilter = fFilter;
   // We should copy over a 0 and let the hit recalculate, this is safest
   static_cast<TTigressHit&>(rhs).fTigressHitBits = 0;
   static_cast<TTigressHit&>(rhs).fTimeFit        = fTimeFit;
   static_cast<TTigressHit&>(rhs).fSegments       = fSegments;
   static_cast<TTigressHit&>(rhs).fCoreSet        = fCoreSet;
   static_cast<TTigressHit&>(rhs).fBgoFired       = fBgoFired;
}

void TTigressHit::Copy(TObject& obj, bool waveform) const
{
   Copy(obj);
   if(waveform) {
      CopyWave(obj);
   }
}

bool TTigressHit::InFilter(Int_t)
{
   // check if the desired filter is in wanted filter;
   // return the answer;
   return true;
}

void TTigressHit::Clear(Option_t* opt)
{
   // Clears the information stored in the TTigressHit.
   TDetectorHit::Clear(opt);   // clears the base (address, position and waveform)
   fFilter         = 0;
   fTigressHitBits = 0;

   fTimeFit   = 0.0;
   fSig2Noise = 0.0;

   fCoreSet  = false;
   fBgoFired = false;
   fSegments.clear();
}

void TTigressHit::Print(Option_t*) const
{
   // Prints the Detector Number, Crystal Number, Energy, Time and Angle.
   Print(std::cout);
}

void TTigressHit::Print(std::ostream& out) const
{
   std::ostringstream str;
   str << "==== TigressHit @ " << this << " ====" << std::endl
       << "\t" << GetName() << std::endl
       << "\tCharge:    " << GetCharge() << std::endl
       << "\tEnergy:    " << GetEnergy() << std::endl
       << "\tTime:      " << GetTime() << std::endl
       << "\tCore set:  " << (CoreSet() ? "true" : "false") << std::endl
       << "\tBGO Fired: " << (BGOFired() ? "true" : "false") << std::endl
       << "\tTime:      " << GetTimeStamp() << std::endl
       << "\thit contains " << GetNSegments() << " segments:" << std::endl;
   str << "Name           Charge" << std::endl;
   for(int x = 0; x < GetNSegments(); x++) {
      str << "\t\t" << GetSegmentHit(x).GetName() << "  |   " << GetSegmentHit(x).GetCharge() << std::endl;
   }
   auto p = GetPosition();
   str << p.GetName() << " " << p.GetTitle() << " (x,y,z)=(" << p.X() << "," << p.Y() << "," << p.Z() << ") (rho,theta,phi)=(" << p.Mag() << "," << p.Theta() << "," << p.Phi() << ")" << std::endl;
   str << "============================" << std::endl;

   out << str.str();
}

TVector3 TTigressHit::GetPosition(double dist) const
{
   return TTigress::GetPosition(GetDetector(), GetCrystal(), GetSegment(), dist);
}

TVector3 TTigressHit::GetLastPosition(double dist) const
{
   const TDetectorHit* seg = nullptr;
   if(GetNSegments() > 0) {
      seg = &fSegments.back();   // last segment in the segment vector.
   } else {
      seg = this;   // if no segments, use the core.
   }

   return TTigress::GetPosition(seg->GetDetector(), seg->GetCrystal(), seg->GetSegment(), dist);
}

TVector3 TTigressHit::GetPosition() const
{
   return GetPosition(GetDefaultDistance());
}

bool TTigressHit::CompareEnergy(const TTigressHit* lhs, const TTigressHit* rhs)
{
   return lhs->GetEnergy() > rhs->GetEnergy();
}

bool TTigressHit::Compare(const TTigressHit* lhs, const TTigressHit* rhs)
{
   if(lhs->GetDetector() == rhs->GetDetector()) {
      return lhs->GetCrystal() < rhs->GetCrystal();
   }
   return lhs->GetDetector() < rhs->GetDetector();
}

void TTigressHit::Add(const TDetectorHit* hit)
{
   if(hit == this) { return; }

   const auto* tigressHit = dynamic_cast<const TTigressHit*>(hit);
   if(tigressHit == nullptr) {
      throw std::runtime_error("trying to add non-griffin hit to griffin hit!");
   }
   // add another griffin hit to this one (for addback),
   // using the time and position information of the one with the higher energy
   if(!CompareEnergy(this, tigressHit)) {
      SetCfd(tigressHit->GetCfd());
      SetTime(tigressHit->GetTime());
      SetAddress(tigressHit->GetAddress());
   } else {
      SetTime(GetTime());
   }
   SetEnergy(GetEnergy() + tigressHit->GetEnergy());
   // this has to be done at the very end, otherwise GetEnergy() might not work
   SetCharge(0);
   // Add all of the pileups.This should be changed when the max number of pileups changes
   if((NPileUps() + tigressHit->NPileUps()) < 4) {
      SetNPileUps(NPileUps() + tigressHit->NPileUps());
   } else {
      SetNPileUps(3);
   }
   if((PUHit() + tigressHit->PUHit()) < 4) {
      SetPUHit(PUHit() + tigressHit->PUHit());
   } else {
      SetPUHit(3);
   }
   // KValue is somewhat meaningless in addback, so I am using it as an indicator that a piledup griffinHit was added-back RD
   if(GetKValue() > tigressHit->GetKValue()) {
      SetKValue(tigressHit->GetKValue());
   }
}

void TTigressHit::SetTigressFlag(enum ETigressHitBits flag, Bool_t set)
{
   fTigressHitBits.SetBit(flag, set);
}

UShort_t TTigressHit::NPileUps() const
{
   // The pluralized test bits returns the actual value of the fBits masked. Not just a bool.
   return static_cast<UShort_t>(fTigressHitBits.TestBits(ETigressHitBits::kTotalPU1) + fTigressHitBits.TestBits(ETigressHitBits::kTotalPU2));
}

UShort_t TTigressHit::PUHit() const
{
   // The pluralized test bits returns the actual value of the fBits masked. Not just a bool.
   return static_cast<UShort_t>(fTigressHitBits.TestBits(ETigressHitBits::kPUHit1) +
                                (fTigressHitBits.TestBits(ETigressHitBits::kPUHit2) >> static_cast<std::underlying_type<ETigressHitBits>::type>(ETigressHitBits::kPUHitOffset)));
}

void TTigressHit::SetNPileUps(UChar_t npileups)
{
   SetTigressFlag(ETigressHitBits::kTotalPU1, (npileups & static_cast<std::underlying_type<ETigressHitBits>::type>(ETigressHitBits::kTotalPU1)) != 0);
   SetTigressFlag(ETigressHitBits::kTotalPU2, (npileups & static_cast<std::underlying_type<ETigressHitBits>::type>(ETigressHitBits::kTotalPU2)) != 0);
}

void TTigressHit::SetPUHit(UChar_t puhit)
{
   if(puhit > 2) {
      puhit = 3;
   }
   // The pluralized test bits returns the actual value of the fBits masked. Not just a bool.

   SetTigressFlag(ETigressHitBits::kPUHit1, ((puhit << static_cast<std::underlying_type<ETigressHitBits>::type>(ETigressHitBits::kPUHitOffset)) & static_cast<std::underlying_type<ETigressHitBits>::type>(ETigressHitBits::kPUHit1)) != 0);
   SetTigressFlag(ETigressHitBits::kPUHit2, ((puhit << static_cast<std::underlying_type<ETigressHitBits>::type>(ETigressHitBits::kPUHitOffset)) & static_cast<std::underlying_type<ETigressHitBits>::type>(ETigressHitBits::kPUHit2)) != 0);
}

Double_t TTigressHit::GetNoCTEnergy(Option_t*) const
{
   TChannel* chan = GetChannel();
   if(chan == nullptr) {
      Error("GetEnergy", "No TChannel exists for address 0x%08x", GetAddress());
      return 0.;
   }
   return chan->CalibrateENG(Charge(), GetKValue());
}

void TTigressHit::SetWavefit(const TFragment& frag)
{
   TPulseAnalyzer pulse(frag);
   if(pulse.IsSet()) {
      fTimeFit   = static_cast<Float_t>(pulse.fit_newT0());
      fSig2Noise = static_cast<Float_t>(pulse.get_sig2noise());
   }
}

void TTigressHit::SetWavefit()
{
   TPulseAnalyzer pulse(*GetWaveform(), 0, GetName());
   if(pulse.IsSet()) {
      fTimeFit   = static_cast<Float_t>(pulse.fit_newT0());
      fSig2Noise = static_cast<Float_t>(pulse.get_sig2noise());
   }
}
