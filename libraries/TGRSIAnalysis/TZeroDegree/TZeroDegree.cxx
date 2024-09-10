#include <iostream>

#include "TRandom.h"
#include "TMath.h"

#include "TZeroDegree.h"
#include "TGRSIOptions.h"

bool TZeroDegree::fSetWave = false;

TZeroDegree::TZeroDegree()
{
// Default Constructor
   Clear();
}

TZeroDegree::TZeroDegree(const TZeroDegree& rhs) : TDetector(rhs)
{
// Copy Contructor
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
	str<<"TZeroDegree contains "<<GetMultiplicity()<<" hits"<<std::endl;
	out<<str.str();
}

void TZeroDegree::AddFragment(const std::shared_ptr<const TFragment>& frag, TChannel* chan)
{
   /// Builds the ZDS Hits directly from the TFragment. Basically, loops through the data for an event and sets
   /// observables.
   if(frag == nullptr || chan == nullptr) {
      return;
   }

   auto* hit = new TZeroDegreeHit(*frag);
   AddHit(hit);
}
