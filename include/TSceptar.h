#ifndef TSCEPTAR_H
#define TSCEPTAR_H

/** \addtogroup Detectors
 *  @{
 */

/////////////////////////////////////////////////////////////
///
/// \class TSceptar
///
/// The TSceptar class defines the observables and algorithms used
/// when analyzing GRIFFIN data. It includes detector positions,
/// etc.
///
/////////////////////////////////////////////////////////////

#include <vector>
#include <cstdio>

#include "TVector3.h"

#include "Globals.h"
#include "TDetector.h"
#include "TSceptarHit.h"

class TSceptar : public TDetector {
public:
   TSceptar();
   TSceptar(const TSceptar& rhs);
   TSceptar(TSceptar&&) noexcept = default;
   TSceptar& operator=(const TSceptar&);   //!<!
   TSceptar& operator=(TSceptar&&) noexcept = default;
   ~TSceptar() override                     = default;

   TSceptarHit* GetSceptarHit(const int& i) const { return static_cast<TSceptarHit*>(GetHit(i)); }
#ifndef __CINT__
   void AddFragment(const std::shared_ptr<const TFragment>&, TChannel*) override;   //!<!
#endif
	void BuildHits() override {} // no need to build any hits, everything already done in AddFragment

   static TVector3 GetPosition(int DetNbr) { return fPaddlePosition[DetNbr]; }   //!<!

   static bool SetWave() { return fSetWave; }   //!<!

private:
   static bool                     fSetWave;          //  Flag for Waveforms ON/OFF
   static std::array<TVector3, 21> fPaddlePosition;   //!<!  Position of each Paddle

public:
   void Print(Option_t* opt = "") const override;   //!<!
   void Print(std::ostream& out) const override;    //!<!

   /// \cond CLASSIMP
   ClassDefOverride(TSceptar, 2)   // Sceptar Physics structure // NOLINT(readability-else-after-return)
                                   /// \endcond
};
/*! @} */
#endif
