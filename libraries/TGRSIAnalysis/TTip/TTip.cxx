#include "TTip.h"

#include <iostream>

#include "TRandom.h"
#include "TMath.h"
#include "TClass.h"

/// \cond CLASSIMP
ClassImp(TTip)
/// \endcond

TTip::TTip()
{
}

TTip::~TTip()
{
   // Default Destructor
}

TTip::TTip(const TTip& rhs) : TDetector()
{
#if ROOT_VERSION_CODE < ROOT_VERSION(6,0,0)
   Class()->IgnoreTObjectStreamer(kTRUE);
#endif
   rhs.Copy(*this);
}

TTip& TTip::operator=(const TTip& rhs)
{
   rhs.Copy(*this);
   return *this;
}

void TTip::AddFragment(const std::shared_ptr<const TFragment>& frag, TChannel* chan)
{
   if(frag == nullptr || chan == nullptr) {
      return;
   }

   TTipHit* hit = new TTipHit(*frag);
   hit->SetUpNumbering(chan);          // Think about moving this to ctor
   fHits.push_back(hit);
}

void TTip::Print(Option_t*) const
{
   /// Prints out TTip members, currently only prints the multiplicity.
	Print(std::cout);
}

void TTip::Print(std::ostream& out) const
{
	std::ostringstream str;
   str<<fHits.size()<<" fHits"<<std::endl;
	out<<str.str();
}
