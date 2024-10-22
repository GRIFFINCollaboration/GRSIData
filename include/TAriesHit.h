#ifndef ARIESHIT_H
#define ARIESHIT_H

/** \addtogroup Detectors
 *  @{
 */

/////////////////////////////////////////////////////////////////
///
/// \class TAriesHit
///
/// This is class that contains the information about a aries
/// hit. This class is used to find energy, time, etc.
///
/////////////////////////////////////////////////////////////////

#include <cstdio>
#include <cmath>
#include <vector>

#include "TFragment.h"
#include "TChannel.h"

#include "TVector3.h"

#include "TDetectorHit.h"

class TAriesHit : public TDetectorHit {
public:
   TAriesHit();
   TAriesHit(const TAriesHit&);
   TAriesHit(TAriesHit&&) noexcept            = default;
   TAriesHit& operator=(const TAriesHit&)     = default;
   TAriesHit& operator=(TAriesHit&&) noexcept = default;
   explicit TAriesHit(const TFragment& frag);
   ~TAriesHit() override = default;

   TVector3 GetPosition(Double_t dist) const override;   //!<!
   TVector3 GetPosition() const override;                //!<!

   void Clear(Option_t* opt = "") override;         //!<!
   void Print(Option_t* opt = "") const override;   //!<!
   void Print(std::ostream& out) const override;    //!<!
   void Copy(TObject&) const override;              //!<!
   void Copy(TObject&, bool) const override;        //!<!

private:
   Double_t GetDefaultDistance() const { return 0.0; }

   /// \cond CLASSIMP
   ClassDefOverride(TAriesHit, 4)   // Stores the information for a AriesHit // NOLINT(readability-else-after-return)
   /// \endcond
};
/*! @} */
#endif
