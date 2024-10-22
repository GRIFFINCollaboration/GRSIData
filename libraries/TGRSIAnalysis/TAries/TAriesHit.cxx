#include "TAriesHit.h"

#include <iostream>
#include <algorithm>
#include <climits>

#include "Globals.h"
#include "TAries.h"
#include "TGRSIOptions.h"

TAriesHit::TAriesHit()
{
   // Default Constructor
   Clear();
}

TAriesHit::TAriesHit(const TAriesHit& rhs) : TDetectorHit(rhs)
{
   // Copy Constructor
   Clear();
   rhs.Copy(*this);
}

TAriesHit::TAriesHit(const TFragment& frag)
{
   frag.Copy(*this);
   if(TAries::SetWave()) {
      if(frag.GetWaveform()->empty()) {
         std::cout << "Warning, TAries::SetWave() set, but data waveform size is zero!" << std::endl;
      }
      frag.CopyWave(*this);
   }
}

void TAriesHit::Copy(TObject& rhs) const
{
   // Copies a TAriesHit
   TDetectorHit::Copy(rhs);
}

void TAriesHit::Copy(TObject& obj, bool waveform) const
{
   Copy(obj);
   if(waveform) {
      CopyWave(obj);
   }
}

TVector3 TAriesHit::GetPosition(Double_t) const
{
   // Gets the position of the current TAriesHit
   return TAries::GetPosition(GetDetector());
}

TVector3 TAriesHit::GetPosition() const
{
   // Gets the position of the current TAriesHit
   return GetPosition(GetDefaultDistance());
}

void TAriesHit::Clear(Option_t*)
{
   // Clears the AriesHit
   TDetectorHit::Clear();
}

void TAriesHit::Print(Option_t*) const
{
   /// Prints the AriesHit. Returns:
   /// Detector
   /// Energy
   /// Time
   Print(std::cout);
}

void TAriesHit::Print(std::ostream& out) const
{
   std::ostringstream str;
   str << "Aries Detector:   " << GetDetector() << std::endl;
   str << "Aries hit energy: " << GetEnergy() << std::endl;
   str << "Aries hit time:   " << GetTime() << std::endl;
   out << str.str();
}

