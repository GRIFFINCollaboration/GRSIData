#ifndef TTIGRESSBGOHIT_H
#define TTIGRESSBGOHIT_H

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

class TTigressBgoHit : public TBgoHit {
public:
   TTigressBgoHit();
   TTigressBgoHit(const TTigressBgoHit& hit) : TBgoHit(static_cast<const TBgoHit&>(hit)) {}
   TTigressBgoHit(TTigressBgoHit&&) noexcept            = default;
   TTigressBgoHit& operator=(const TTigressBgoHit&)     = default;
   TTigressBgoHit& operator=(TTigressBgoHit&&) noexcept = default;
   explicit TTigressBgoHit(const TFragment& frag) : TBgoHit(frag) {}
   ~TTigressBgoHit() override = default;

   /////////////////////////		/////////////////////////////////////
   UShort_t GetArrayNumber() const override { return (20 * (GetDetector() - 1) + 5 * GetCrystal() + GetSegment()); }   //!<!

   /// \cond CLASSIMP
   ClassDefOverride(TTigressBgoHit, 2)   // NOLINT(readability-else-after-return)
   /// \endcond
};
/*! @} */
#endif
