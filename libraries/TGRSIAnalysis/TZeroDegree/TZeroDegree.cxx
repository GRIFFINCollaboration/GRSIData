#include <iostream>

#include "TRandom.h"
#include "TMath.h"

#include "TZeroDegree.h"
#include "TGRSIOptions.h"

/// \cond CLASSIMP
ClassImp(TZeroDegree)
/// \endcond

bool TZeroDegree::fSetWave = false;

TZeroDegree::TZeroDegree()
{
// Default Constructor
#if ROOT_VERSION_CODE < ROOT_VERSION(6,0,0)
   Class()->IgnoreTObjectStreamer(kTRUE);
#endif
   Clear();
}

TZeroDegree::~TZeroDegree()
{
   // Default Destructor
}

TZeroDegree::TZeroDegree(const TZeroDegree& rhs) : TDetector()
{
// Copy Contructor
#if ROOT_VERSION_CODE < ROOT_VERSION(6,0,0)
   Class()->IgnoreTObjectStreamer(kTRUE);
#endif
   rhs.Copy(*this);
}

TZeroDegree& TZeroDegree::operator=(const TZeroDegree& rhs)
{
   rhs.Copy(*this);
   return *this;
}

void TZeroDegree::Print(Option_t*) const
{
   /// Prints out TZeroDegree multiplicity, currently does little.
	Print(std::cout);
}

void TZeroDegree::Print(std::ostream& out) const
{
	std::ostringstream str;
   str<<fHits.size()<<" fHits"<<std::endl;
	out<<str.str();
}

void TZeroDegree::AddFragment(const std::shared_ptr<const TFragment>& frag, TChannel* chan)
{
   /// Builds the ZDS Hits directly from the TFragment. Basically, loops through the data for an event and sets
   /// observables.
   if(frag == nullptr || chan == nullptr) {
      return;
   }

   TZeroDegreeHit* hit = new TZeroDegreeHit(*frag);
   fHits.push_back(hit);
}
