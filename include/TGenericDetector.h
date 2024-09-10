#ifndef TGENERICDET_H
#define TGENERICDET_H

/** \addtogroup Detectors
 *  @{
 */

#include <iostream>

#include "TDetector.h"
#include "TChannel.h"
#include "TDetectorHit.h"

class TGenericDetector : public TDetector {
public:
   TGenericDetector();
   TGenericDetector(const TGenericDetector&);
   TGenericDetector(TGenericDetector&&) noexcept = default;
   TGenericDetector& operator=(const TGenericDetector&);
   TGenericDetector& operator=(TGenericDetector&&) noexcept = default;
   ~TGenericDetector() override                             = default;

#ifndef __CINT__
   void AddFragment(const std::shared_ptr<const TFragment>&, TChannel*) override;   //!<!
#endif
	void BuildHits() override {} // no need to build any hits, everything already done in AddFragment

   TDetectorHit* GetGenericDetectorHit(const int& i) const { return GetHit(i); }

   void Print(Option_t* opt = "") const override;   //!<!
   void Print(std::ostream& out) const override;    //!<!

private:
   void ClearStatus() {}

   /// \cond CLASSIMP
   ClassDefOverride(TGenericDetector, 1)   // NOLINT(readability-else-after-return)
   /// \endcond
};
/*! @} */
#endif
