#ifndef TGENERICDET_H
#define TGENERICDET_H

/** \addtogroup Detectors
 *  @{
 */

#include <iostream>

#include "TDetector.h"
#include "TChannel.h"
#include "TGRSIDetectorHit.h"

class TGenericDetector : public TDetector {
public:

   TGenericDetector();
   TGenericDetector(const TGenericDetector&);
   ~TGenericDetector() override;

#ifndef __CINT__
   void AddFragment(const std::shared_ptr<const TFragment>&, TChannel*) override; //!<!
#endif
	void BuildHits() override {} // no need to build any hits, everything already done in AddFragment

   TGRSIDetectorHit* GetGenericDetectorHit(const int& i) const { return static_cast<TGRSIDetectorHit*>(GetHit(i)); }

   TGenericDetector& operator=(const TGenericDetector&);                    //
   void Print(Option_t* opt = "") const override; //!<!

private:
   void  ClearStatus() {  }

   /// \cond CLASSIMP
   ClassDefOverride(TGenericDetector, 1)
   /// \endcond
};
/*! @} */
#endif
