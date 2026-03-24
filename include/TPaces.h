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
   TPaces& operator=(const TPaces&);   //!<!
   TPaces& operator=(TPaces&&) noexcept = default;
   ~TPaces() override                   = default;

   TPacesHit* GetPacesHit(const int& i) const { return static_cast<TPacesHit*>(GetHit(i)); }

#ifndef __CINT__
   void AddFragment(const std::shared_ptr<TFragment>&, TChannel*) override;
#endif
   void BuildHits() override {}   // no need to build any hits, everything already done in AddFragment

   static ROOT::Math::Polar3DVector GetPosition(int DetNbr) { return fDetectorPosition[DetNbr]; }   //!<!

   // overrides of basic TObject/TDetector functions
   void Copy(TObject&) const override;              //!<!
   void Print(Option_t* opt = "") const override;   //!<!
   void Print(std::ostream& out) const override;    //!<!

private:
   static std::array<ROOT::Math::Polar3DVector, 6> fDetectorPosition;   //!<!  Position of each detector (plus one default position)

   /// \cond CLASSIMP
   ClassDefOverride(TPaces, 4)   // Paces Physics structure // NOLINT(readability-else-after-return)
   /// \endcond
};
/*! @} */
#endif
