#ifndef TARIES_H
#define TARIES_H

/** \addtogroup Detectors
 *  @{
 */

/////////////////////////////////////////////////////////////
///
/// \class TAries
///
/// The TAries class defines the observables and algorithms used
/// when analyzing GRIFFIN data. It includes detector positions,
/// etc.
///
/////////////////////////////////////////////////////////////

#include <vector>
#include <cstdio>

#include "Math/Vector3D.h"

#include "Globals.h"
#include "TDetector.h"
#include "TAriesHit.h"

class TAries : public TDetector {
public:
   TAries();
   TAries(const TAries& rhs);
   TAries(TAries&&) noexcept = default;
   TAries& operator=(const TAries&);   //!<!
   TAries& operator=(TAries&&) noexcept = default;
   ~TAries() override                   = default;

   TAriesHit* GetAriesHit(const int& i) const { return static_cast<TAriesHit*>(GetHit(i)); }
#ifndef __CINT__
   void AddFragment(const std::shared_ptr<const TFragment>&, TChannel*) override;   //!<!
#endif
   void BuildHits() override {}   // no need to build any hits, everything already done in AddFragment

   static ROOT::Math::Polar3DVector GetPosition(int DetNbr) { return fPaddlePosition[DetNbr]; }   //!<!

   static bool SetWave() { return fSetWave; }   //!<!

private:
   static bool                     fSetWave;          //  Flag for Waveforms ON/OFF
   static std::array<ROOT::Math::Polar3DVector, 77> fPaddlePosition;   //!<!  Position of each Paddle

public:
   void Print(Option_t* opt = "") const override;   //!<!
   void Print(std::ostream& out) const override;    //!<!

   /// \cond CLASSIMP
   ClassDefOverride(TAries, 2)   // Aries Physics structure // NOLINT(readability-else-after-return)
                                 /// \endcond
};
/*! @} */
#endif
