#include "TGenericDetector.h"
#include "TMnemonic.h"

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

TGenericDetector::TGenericDetector(const TGenericDetector& rhs) : TDetector()
{
   rhs.Copy(*this);
}

void TGenericDetector::AddFragment(const std::shared_ptr<const TFragment>& frag, TChannel* chan)
{
	
   /// This function creates TGenericDetectorHits for each fragment and stores them in separate front and back vectors
   if(frag == nullptr || chan == nullptr) {
      return;
   }

   TGRSIDetectorHit* dethit = new TGRSIDetectorHit(*frag);
   fHits.push_back(std::move(dethit));
}

void TGenericDetector::Print(Option_t*) const
{
   printf("%s\tnot yet written.\n", __PRETTY_FUNCTION__);
}
