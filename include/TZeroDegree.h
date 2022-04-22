#ifndef TZERODEGREE_H
#define TZERODEGREE_H

/** \addtogroup Detectors
 *  @{
 */

/////////////////////////////////////////////////////////////
///
/// \class TZeroDegree
///
/// The TZeroDegree class defines the observables and algorithms used
/// when analyzing GRIFFIN data. It includes detector positions,
/// etc.
///
/////////////////////////////////////////////////////////////

#include <vector>
#include <cstdio>

#include "TVector3.h"

#include "Globals.h"
#include "TDetector.h"
#include "TZeroDegreeHit.h"

class TZeroDegree : public TDetector {
public:
   TZeroDegree();
   ~TZeroDegree() override;
   TZeroDegree(const TZeroDegree& rhs);

public:
   TZeroDegreeHit* GetZeroDegreeHit(const int& i) const { return static_cast<TZeroDegreeHit*>(GetHit(i)); }

   static TVector3 GetPosition(double dist) { return TVector3(0, 0, dist); } //!<!

#ifndef __CINT__
   void AddFragment(const std::shared_ptr<const TFragment>&, TChannel*) override; //!<!
#endif
	void BuildHits() override {} // no need to build any hits, everything already done in AddFragment

   TZeroDegree& operator=(const TZeroDegree&); //!<!

private:
   static bool fSetWave; ///<  Flag for Waveforms ON/OFF

public:
   static bool SetWave() { return fSetWave; } //!<!

public:
   void Print(Option_t* opt = "") const override; //!<!
	void Print(std::ostream& out) const override; //!<!

   /// \cond CLASSIMP
   ClassDefOverride(TZeroDegree, 2) // ZeroDegree Physics structure
                                    /// \endcond
};
/*! @} */
#endif
