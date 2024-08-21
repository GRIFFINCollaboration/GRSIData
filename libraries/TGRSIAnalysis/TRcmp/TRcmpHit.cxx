#include <iostream>
#include <vector>
#include "TRcmp.h"
#include "TRcmpHit.h"

#include <cstdlib>
#include <cmath>
#include "TMath.h"
#include "Globals.h"

// function implementations are below

TRcmpHit::TRcmpHit()
{
/// this is the default constructor
#if ROOT_VERSION_CODE < ROOT_VERSION(6, 0, 0)
   Class()->IgnoreTObjectStreamer(kTRUE);
#endif
   Clear();
}

TRcmpHit::TRcmpHit(const TRcmpHit& rhs) : TDetectorHit(rhs)
{
/// this is the copy constructor for a single TRcmpHit parameter
#if ROOT_VERSION_CODE < ROOT_VERSION(6, 0, 0)
   Class()->IgnoreTObjectStreamer(kTRUE);
#endif
   Clear();
   rhs.Copy(*this);
}

TRcmpHit::TRcmpHit(const TFragment& frag)
{
   /// this is the constructor that builds RCMP hits out of a single fragment (either front or back)
   Clear();
   frag.Copy(*this);
   fStrip = frag.GetSegment(); // set the (front or back) strip number of the hit (note that this would be between 00-31 since there are 32 strips)
}

TRcmpHit::TRcmpHit(const TFragment& fragFront, const TFragment& fragBack)
{
   /// this is the constructor that builds RCMP hits out of a front and a back fragment (energy, time, etc. is taken from the front-strip)
   Clear();
   fragFront.Copy(*this);                // we want to use the information from the front (P-side) strip such as energy, time, etc.
   fFrontStrip = fragFront.GetSegment(); // set the front strip number of the hit (goes from 00-31)
   fBackStrip  = fragBack.GetSegment();  // set the back strip number of the hit  (goes from 00-31)
}

// this is the destructor
TRcmpHit::~TRcmpHit() = default;

void TRcmpHit::Copy(TObject& rhs) const
{
   /// Copy function
   TDetectorHit::Copy(rhs);

   // copy the strip number data members
   static_cast<TRcmpHit&>(rhs).fFrontStrip = fFrontStrip;
   static_cast<TRcmpHit&>(rhs).fBackStrip  = fBackStrip;
   static_cast<TRcmpHit&>(rhs).fStrip      = fStrip;
}

void TRcmpHit::Copy(TObject& rhs, bool waveform) const
{
   Copy(rhs);
   if(waveform) {
      CopyWave(rhs);
   }
}

void TRcmpHit::Clear(Option_t* opt)
{
   TDetectorHit::Clear(opt);   // clears the base (address, position and waveform)
   
   // set the data members to default values
   fFrontStrip = -1;
   fBackStrip  = -1;
   fStrip      = -1;
}

void TRcmpHit::Print(Option_t*) const
{
   Print(std::cout);
}

void TRcmpHit::Print(std::ostream& out) const
{
   std::ostringstream str;
   str << "Rcmp Detector: " << GetDetector() << std::endl;
   str << "Rcmp Energy:   " << GetEnergy() << std::endl;
   str << "Rcmp hit time: " << GetTime() << std::endl;
   str << "Front strip: " << fFrontStrip << std::endl;
   str << "Back strip: " << fBackStrip << std::endl;
   out << str.str();
}
