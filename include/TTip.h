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

   TTipHit* GetTipHit(const int& i) const { return static_cast<TTipHit*>(GetHit(i)); }

#ifndef __CINT__
   void AddFragment(const std::shared_ptr<const TFragment>&, TChannel*) override; //!<!
#endif

   TTip& operator=(const TTip&); //!<!

   void Print(Option_t* opt = "") const override;

   /// \cond CLASSIMP
   ClassDefOverride(TTip, 2);
   /// \endcond
};
/*! @} */
#endif
