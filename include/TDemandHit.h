#ifndef DEMANDHIT_H
#define DEMANDHIT_H

/** \addtogroup Detectors
 *  @{
 */

#include <cstdio>
#include <cmath>

#include "TVector3.h"

#include "TFragment.h"
#include "TChannel.h"

#include "TDetectorHit.h"

class TDemandHit : public TDetectorHit {
public:
   TDemandHit();
   TDemandHit(const TDemandHit&);
   TDemandHit(TDemandHit&&) noexcept            = default;
   TDemandHit& operator=(const TDemandHit&)     = default;
   TDemandHit& operator=(TDemandHit&&) noexcept = default;
   explicit TDemandHit(const TFragment& frag);
   ~TDemandHit() override = default;

private:
   Int_t fFilter{0};

public:
   /////////////////////////  Setters  /////////////////////////////////////
   inline void SetFilterPattern(const int& x) { fFilter = x; }   //!<!
   // void SetHit();

   /////////////////////////  Getters  /////////////////////////////////////
   inline Int_t GetFilterPattern() const { return fFilter; }   //!<!

   /////////////////////////  TChannel Helpers /////////////////////////////////////
   bool InFilter(Int_t);   //!<!

   void     Clear(Option_t* opt = "") override;         //!<!
   void     Print(Option_t* opt = "") const override;   //!<!
   void     Print(std::ostream& out) const override;    //!<!
   void     Copy(TObject&) const override;              //!<!
   void     Copy(TObject&, bool) const override;        //!<!
   TVector3 GetPosition() const override;               //!<!

private:
   Double_t GetDefaultDistance() const { return 0.0; }

   /// \cond CLASSIMP
   ClassDefOverride(TDemandHit, 4);   // NOLINT(readability-else-after-return)
   /// \endcond
};
/*! @} */
#endif