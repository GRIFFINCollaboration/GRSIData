#ifndef TLABRBGOHIT_H
#define TLABRBGOHIT_H

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

class TLaBrBgoHit : public TBgoHit {
public:
   TLaBrBgoHit();
   TLaBrBgoHit(const TLaBrBgoHit& hit) : TBgoHit(static_cast<const TBgoHit&>(hit)) {}
	TLaBrBgoHit(TLaBrBgoHit&&) noexcept = default;
	TLaBrBgoHit& operator=(const TLaBrBgoHit&) = default;
	TLaBrBgoHit& operator=(TLaBrBgoHit&&) noexcept = default;
   explicit TLaBrBgoHit(const TFragment& frag) : TBgoHit(frag) {}
   ~TLaBrBgoHit() override = default;

   /////////////////////////		/////////////////////////////////////
   UShort_t GetArrayNumber() const override { return (3 * (GetDetector() - 1) + GetCrystal()); } //!<! the BGO of each detector has three segments

   /// \cond CLASSIMP
   ClassDefOverride(TLaBrBgoHit, 2) // NOLINT(readability-else-after-return)
   /// \endcond
};
/*! @} */
#endif
