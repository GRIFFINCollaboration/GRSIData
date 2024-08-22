#include "TGenericDetector.h"

#include "TClass.h"
#include <cmath>
#include "TMath.h"

#include "TGRSIOptions.h"

/// \cond CLASSIMP
ClassImp(TGenericDetector)
/// \endcond


TGenericDetector::TGenericDetector()
{
   Clear();
}

TGenericDetector::~TGenericDetector() = default;

TGenericDetector& TGenericDetector::operator=(const TGenericDetector& rhs)
{
   rhs.Copy(*this);
   return *this;
}

TGenericDetector::TGenericDetector(const TGenericDetector& rhs) : TDetector(rhs)
{
   rhs.Copy(*this);
}

void TGenericDetector::AddFragment(const std::shared_ptr<const TFragment>& frag, TChannel* chan)
{
	
   /// This function creates TGenericDetectorHits for each fragment and stores them in separate front and back vectors
   if(frag == nullptr || chan == nullptr) {
      return;
   }

   TDetectorHit* hit = new TDetectorHit(*frag);
   AddHit(hit);
}

void TGenericDetector::Print(Option_t*) const
{
	Print(std::cout);
}

void TGenericDetector::Print(std::ostream& out) const
{
	std::ostringstream str;
   str<<__PRETTY_FUNCTION__<<"\tnot yet written."<<std::endl; // NOLINT(cppcoreguidelines-pro-bounds-array-to-pointer-decay)
	out<<str.str();
}
