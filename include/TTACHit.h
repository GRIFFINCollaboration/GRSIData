#ifndef TACHIT_H
#define TACHIT_H

/** \addtogroup Detectors
 *  @{
 */

/////////////////////////////////////////////////////////////////
///
/// \class TTACHit
///
/// This is class that contains the information about a TAC
/// hit. This class is used to find energy, time, etc.
///
/////////////////////////////////////////////////////////////////

#include <cstdio>
#include <cmath>

#include "TFragment.h"
#include "TChannel.h"
#include "TGraph.h"

#include "TVector3.h"

#include "TDetectorHit.h"

class TTACHit : public TDetectorHit {
public:
   TTACHit();
   TTACHit(const TTACHit&);
   TTACHit(TTACHit&&) noexcept            = default;
   TTACHit& operator=(const TTACHit&)     = default;
   TTACHit& operator=(TTACHit&&) noexcept = default;
   explicit TTACHit(const TFragment& frag) { frag.Copy(*this); }
   ~TTACHit() override = default;

   /////////////////////////		/////////////////////////////////////
   inline void SetFilterPattern(const int& x) { fFilter = x; }   //!<!

   /////////////////////////		/////////////////////////////////////
   inline Int_t GetFilterPattern() const { return fFilter; }   //!<!

   bool InFilter(Int_t);   //!<!

   Double_t GetTempCorrectedCharge(TGraph* correction_graph) const;
   Double_t TempCorrectedCharge(TGraph* correction_graph) const;
   Double_t GetTempCorrectedEnergy(TGraph* correction_graph) const;

   void Clear(Option_t* opt = "") override;         //!<!
   void Print(Option_t* opt = "") const override;   //!<!
   void Print(std::ostream& out) const override;    //!<!
   void Copy(TObject&) const override;              //!<!
   void Copy(TObject&, bool) const override;        //!<!

private:
   Int_t fFilter{0};

   /// \cond CLASSIMP
   ClassDefOverride(TTACHit, 2)   // Stores the information for a TACrHit // NOLINT(readability-else-after-return)
   /// \endcond
};
/*! @} */
#endif
