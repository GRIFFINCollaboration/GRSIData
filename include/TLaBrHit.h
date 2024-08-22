#ifndef LABRHIT_H
#define LABRHIT_H

/** \addtogroup Detectors
 *  @{
 */

/////////////////////////////////////////////////////////////////
///
/// \class TLaBrHit
///
/// This is class that contains the information about a LaBr
/// hit. This class is used to find energy, time, etc.
///
/////////////////////////////////////////////////////////////////

#include <cstdio>
#include <cmath>

#include "TFragment.h"
#include "TChannel.h"

#include "TVector3.h"

#include "TDetectorHit.h"

class TLaBrHit : public TDetectorHit {
public:
   TLaBrHit();
   ~TLaBrHit() override;
   TLaBrHit(const TLaBrHit&);
   explicit TLaBrHit(const TFragment& frag) : TDetectorHit(frag) {}

   /////////////////////////		/////////////////////////////////////
   inline void SetFilterPattern(const int& x) { fFilter = x; } //!<!

   /////////////////////////		/////////////////////////////////////
   inline Int_t GetFilterPattern() const { return fFilter; } //!<!

   bool InFilter(Int_t); //!<!

   void Clear(Option_t* opt = "") override;       //!<!
   void Print(Option_t* opt = "") const override; //!<!
	void Print(std::ostream& out) const override; //!<!
   void Copy(TObject&) const override;            //!<!
   void Copy(TObject&, bool) const override;      //!<!
   TVector3 GetPosition(Double_t dist) const override;
   TVector3 GetPosition() const override;

private:
   Int_t fFilter{0};

   Double_t GetDefaultDistance() const { return 0.; } // This needs to be updated

   /// \cond CLASSIMP
   ClassDefOverride(TLaBrHit, 3) // Stores the information for a LaBrrHit
   /// \endcond
};
/*! @} */
#endif
