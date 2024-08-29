#ifndef TPACES_H
#define TPACES_H

/** \addtogroup Detectors
 *  @{
 */

#include <vector>
#include <cstdio>

#include "TBits.h"
#include "Math/Vector3D.h"

#include "Globals.h"
#include "TDetector.h"
#include "TPacesHit.h"

class TPaces : public TDetector {
public:
   TPaces();
   TPaces(const TPaces&);
	TPaces(TPaces&&) noexcept = default;
   TPaces& operator=(const TPaces&); //!<!
	TPaces& operator=(TPaces&&) noexcept = default;
   ~TPaces() override = default;

   TPacesHit* GetPacesHit(const int& i) const { return static_cast<TPacesHit*>(GetHit(i)); }

#ifndef __CINT__
   void AddFragment(const std::shared_ptr<const TFragment>&, TChannel*) override;
#endif
	void BuildHits() override {} // no need to build any hits, everything already done in AddFragment

   static ROOT::Math::Polar3DVector GetPosition(int DetNbr) { return fDetectorPosition[DetNbr]; } //!<!

private:
   static bool fSetCoreWave; //!<!  Flag for Waveforms ON/OFF
   static std::array<ROOT::Math::Polar3DVector, 6> fDetectorPosition; //!<!  Position of each detector (plus one default position)

public:
   static bool SetCoreWave() { return fSetCoreWave; } //!<!

   void Copy(TObject&) const override;            //!<!
   void Print(Option_t* opt = "") const override; //!<!
	void Print(std::ostream& out) const override; //!<!

   /// \cond CLASSIMP
   ClassDefOverride(TPaces, 4) // Paces Physics structure // NOLINT(readability-else-after-return)
   /// \endcond
};
/*! @} */
#endif
