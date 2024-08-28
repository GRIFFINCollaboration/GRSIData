#ifndef TRIFICHIT_H
#define TRIFICHIT_H

/** \addtogroup Detectors
 *  @{
 */

#include <cstdio>
#include <cmath>

#include "TVector3.h"

#include "TFragment.h"
#include "TChannel.h"

#include "TDetectorHit.h"

class TTrificHit : public TDetectorHit {
public:
   TTrificHit();
   TTrificHit(const TTrificHit&);
   explicit TTrificHit(const TFragment& frag) { frag.Copy(*this); }
   ~TTrificHit() override;

   /////////////////////////  Setters	/////////////////////////////////////
   inline void SetFilterPattern(const int& x) { fFilter = x; } //!<!
   // void SetHit();

   /////////////////////////  Getters	/////////////////////////////////////
   inline Int_t GetFilterPattern() const { return fFilter; } //!<!

   /////////////////////////  TChannel Helpers /////////////////////////////////////
   bool InFilter(Int_t); //!<!

   void Clear(Option_t* opt = "") override;            //!<!
   void Print(Option_t* opt = "") const override;      //!<!
	void Print(std::ostream& out) const override; //!<!
   void Copy(TObject&) const override;             //!<!
   void Copy(TObject&, bool) const override;      //!<!
   //TVector3 GetPosition(Double_t dist) const override; //!<!
   TVector3 GetPosition() const override;  //!<!             
   //TVector3 GetPosition() const; //!<!

private:
   Double_t GetDefaultDistance() const { return 0.0; }

   Int_t fFilter{0};

   /// \cond CLASSIMP
   ClassDefOverride(TTrificHit, 3); // NOLINT(readability-else-after-return)
   /// \endcond
};
/*! @} */
#endif
