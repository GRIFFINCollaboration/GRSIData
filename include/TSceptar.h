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
   ~TSceptar() override;
   TSceptar(const TSceptar& rhs);

   TSceptarHit* GetSceptarHit(const int& i) const { return static_cast<TSceptarHit*>(GetHit(i)); }
#ifndef __CINT__
   void AddFragment(const std::shared_ptr<const TFragment>&, TChannel*) override; //!<!
#endif
	void BuildHits() override {} // no need to build any hits, everything already done in AddFragment

   static TVector3 GetPosition(int DetNbr) { return gPaddlePosition[DetNbr]; } //!<!

   TSceptar& operator=(const TSceptar&); //!<!

   static bool SetWave() { return fSetWave; } //!<!

private:
   static bool fSetWave; //  Flag for Waveforms ON/OFF
   static TVector3 gPaddlePosition[21]; //!<!  Position of each Paddle

public:
   void Print(Option_t* opt = "") const override; //!<!
	void Print(std::ostream& out) const override; //!<!

   /// \cond CLASSIMP
   ClassDefOverride(TSceptar, 2) // Sceptar Physics structure
	/// \endcond
};
/*! @} */
#endif
