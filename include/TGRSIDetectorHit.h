#ifndef TGRSIDETECTORHIT_H
#define TGRSIDETECTORHIT_H

/** \addtogroup Detectors
 *  @{
 */

#include "Globals.h"

#include "TDetectorHit.h"
#include "TFragment.h"

/////////////////////////////////////////////////////////////////
///
/// \class TGRSIDetectorHit
///
/// This class overloads the GetTime function of TDetectorHit
/// and uses the TGRSIMnemonic detectory type enumeration to
/// calculate the time.
///
/////////////////////////////////////////////////////////////////

class TGRSIDetectorHit : public TDetectorHit {

   //
public:
   TGRSIDetectorHit(const int& Address = 0xffffffff);
   TGRSIDetectorHit(const TGRSIDetectorHit&, bool copywave = true);
	TGRSIDetectorHit(const TFragment& frag) : TDetectorHit(frag) {}
   ~TGRSIDetectorHit() override;

   virtual Double_t GetTime(const ETimeFlag& correct_flag = ETimeFlag::kAll,
                            Option_t*     opt          = "") const override; ///< Returns a time value to the nearest nanosecond!
	virtual Int_t    GetTimeStampUnit() const override;
	
   /// \cond CLASSIMP
   ClassDefOverride(TGRSIDetectorHit, 11) // Stores the information for a detector hit
   /// \endcond
};
/*! @} */
#endif
