#ifndef TDESCANT_H
#define TDESCANT_H

/** \addtogroup Detectors
 *  @{
 */

/////////////////////////////////////////////////////////////
///
/// \class TDescant
///
/// The TDescant class defines the observables and algorithms used
/// when analyzing GRIFFIN data. It includes detector positions,
/// etc.
///
/////////////////////////////////////////////////////////////

#include <vector>
#include <cstdio>

#include "TVector3.h"

#include "Globals.h"
#include "TDetector.h"
#include "TDescantHit.h"

class TDescant : public TDetector {
public:
   TDescant();
   TDescant(const TDescant&);
	TDescant(TDescant&&) noexcept = default;
   TDescant& operator=(const TDescant&); //
	TDescant& operator=(TDescant&&) noexcept = default;
   ~TDescant() override = default;

   TDescantHit* GetDescantHit(const Int_t& i = 0) const { return static_cast<TDescantHit*>(GetHit(i)); }

   static TVector3 GetPosition(int DetNbr, double dist = 222); //!<!

#ifndef __CINT__
   void              AddFragment(const std::shared_ptr<const TFragment>&, TChannel*) override; //!<!
#endif
	void BuildHits() override {} // no need to build any hits, everything already done in AddFragment

public:
   static bool SetWave() { return fSetWave; }         //!<!
   void SetWave(bool flag) { fSetWave = flag; }       //!<!
   bool              Hit() const { return fHitFlag; }       //!<!
   void SetHit(bool flag = true) { fHitFlag = flag; } //!<!
private:
   static bool              fSetWave;     ///<  Flag for Waveforms ON/OFF
   bool                     fHitFlag;   ///<   Is there a Descant hit?

   static std::array<TVector3, 71> fPosition;          //!<!
   static std::array<TVector3, 9>  fAncillaryPosition; //!<!
public:
   void Copy(TObject&) const override;            //!<!
   void Clear(Option_t* opt = "") override;       //!<!
   void Print(Option_t* opt = "") const override; //!<!
	void Print(std::ostream& out) const override; //!<!

   /// \cond CLASSIMP
   ClassDefOverride(TDescant, 1) // NOLINT(readability-else-after-return)
	/// \endcond
};
/*! @} */
#endif
