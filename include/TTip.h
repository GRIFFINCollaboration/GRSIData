#ifndef TTIP_H
#define TTIP_H

/** \addtogroup Detectors
 *  @{
 */

/////////////////////////////////////////////////////////////
///
/// \class TTip
///
/// The TTip class defines the observables and algorithms used
/// when analyzing TIP data. It includes detector positions,
/// etc.
///
/////////////////////////////////////////////////////////////

#include "Globals.h"
#include "TDetector.h"
#include "TTipHit.h"

class TTip : public TDetector {
public:

   ~TTip() override;
   TTip();
   TTip(const TTip& rhs);

   static TVector3 GetPosition(int DetNbr); //!<!
   static TVector3 GetPosition(const TTipHit&);

   TTipHit* GetTipHit(const int& i) const { return static_cast<TTipHit*>(GetHit(i)); }

#ifndef __CINT__
   void AddFragment(const std::shared_ptr<const TFragment>&, TChannel*) override; //!<!
#endif
	void BuildHits() override {} // no need to build any hits, everything already done in AddFragment

   TTip& operator=(const TTip&); //!<!

private:

   // Position vectors for individual TIP detectors
   static std::array<TVector3, 128> fPositionVectors;     //!<!

public:

   void Print(Option_t* opt = "") const override;
	void Print(std::ostream& out) const override; //!<!

   /// \cond CLASSIMP
   ClassDefOverride(TTip, 2);
   /// \endcond
};
/*! @} */
#endif
