#include "TLaBrHit.h"

#include <iostream>
#include <climits>

#include "TLaBr.h"

TLaBrHit::TLaBrHit()
{
   // Default Constructor
   Clear();
}

TLaBrHit::TLaBrHit(const TLaBrHit& rhs) : TDetectorHit(rhs)
{
   // Copy Constructor
   Clear();
   rhs.Copy(*this);
}

void TLaBrHit::Copy(TObject& rhs) const
{
   // Copies a TLaBrHit
   TDetectorHit::Copy(rhs);
   static_cast<TLaBrHit&>(rhs).fFilter = fFilter;
}

void TLaBrHit::Copy(TObject& obj, bool waveform) const
{
   Copy(obj);
   if(waveform) {
      CopyWave(obj);
   }
}

TVector3 TLaBrHit::GetPosition(Double_t) const
{
   // Gets the position of the current TLaBrHit
   return TLaBr::GetPosition(GetDetector());
}

TVector3 TLaBrHit::GetPosition() const
{
   // Gets the position of the current TLaBrHit
   return GetPosition(GetDefaultDistance());
}

bool TLaBrHit::InFilter(Int_t)
{
   // check if the desired filter is in wanted filter;
   // return the answer;
   // currently does nothing
   return true;
}

void TLaBrHit::Clear(Option_t*)
{
   // Clears the LaBrHit
   fFilter = 0;
   TDetectorHit::Clear();
}

void TLaBrHit::Print(Option_t*) const
{
   /// Prints the LaBrHit. Returns:
   /// Detector
   /// Energy
   /// Time
   Print(std::cout);
}

void TLaBrHit::Print(std::ostream& out) const
{
   std::ostringstream str;
   str << "LaBr Detector:   " << GetDetector() << std::endl;
   str << "LaBr hit energy: " << GetEnergy() << std::endl;
   str << "LaBr hit time:   " << GetTime() << std::endl;
   out << str.str();
}
