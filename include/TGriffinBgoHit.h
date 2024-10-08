#ifndef TGRIFFINBGOHIT_H
#define TGRIFFINBGOHIT_H

/** \addtogroup Detectors
 *  @{
 */

#include <cstdio>
#include <cmath>
#if !defined(__CINT__) && !defined(__CLING__)
#include <tuple>
#endif

#include "TMath.h"
#include "TVector3.h"
#include "TClonesArray.h"

#include "TFragment.h"
#include "TChannel.h"
#include "TPulseAnalyzer.h"

#include "TBgoHit.h"

class TGriffinBgoHit : public TBgoHit {
public:
   TGriffinBgoHit();
   TGriffinBgoHit(const TGriffinBgoHit& hit) : TBgoHit(static_cast<const TBgoHit&>(hit)) {}
   TGriffinBgoHit(TGriffinBgoHit&&) noexcept            = default;
   TGriffinBgoHit& operator=(const TGriffinBgoHit&)     = default;
   TGriffinBgoHit& operator=(TGriffinBgoHit&&) noexcept = default;
   explicit TGriffinBgoHit(const TFragment& frag) : TBgoHit(frag) {}
   ~TGriffinBgoHit() override = default;

   /////////////////////////		/////////////////////////////////////
   UShort_t GetArrayNumber() const override { return (20 * (GetDetector() - 1) + 5 * GetCrystal() + GetSegment()); }   //!<!

   /// \cond CLASSIMP
   ClassDefOverride(TGriffinBgoHit, 2)   // NOLINT(readability-else-after-return)
   /// \endcond
};
/*! @} */
#endif
