#ifndef TTAC_H
#define TTAC_H

/** \addtogroup Detectors
 *  @{
 */

/////////////////////////////////////////////////////////////
///
/// \class TTAC
///
/// The TTAC class defines the observables and algorithms used
/// when analyzing TAC data. It includes detector positions,
/// etc.
///
/////////////////////////////////////////////////////////////

#include <vector>
#include <cstdio>

#include "TVector3.h"

#include "Globals.h"
#include "TDetector.h"
#include "TTACHit.h"

class TTAC : public TDetector {
public:
   TTAC();
   ~TTAC() override;
   TTAC(const TTAC& rhs);

   TTACHit* GetTACHit(const int& i) const { return static_cast<TTACHit*>(GetHit(i)); }

#ifndef __CINT__
   void AddFragment(const std::shared_ptr<const TFragment>&, TChannel*) override; //!<!
#endif
	void BuildHits() override {} // no need to build any hits, everything already done in AddFragment

   TTAC& operator=(const TTAC&); //!<!

   void Print(Option_t* opt = "") const override; //!<!
	void Print(std::ostream& out) const override; //!<!

   /// \cond CLASSIMP
   ClassDefOverride(TTAC, 1) // TAC Physics structure
   /// \endcond
};
/*! @} */
#endif
