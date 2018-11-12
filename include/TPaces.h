#ifndef TPACES_H
#define TPACES_H

/** \addtogroup Detectors
 *  @{
 */

#include <vector>
#include <cstdio>

#include "TBits.h"
#include "TVector3.h"

#include "Globals.h"
#include "TDetector.h"
#include "TPacesHit.h"

class TPaces : public TDetector {

public:
   TPaces();
   TPaces(const TPaces&);
   ~TPaces() override;

public:
   TPacesHit* GetPacesHit(const int& i) const { return static_cast<TPacesHit*>(GetHit(i)); }

#ifndef __CINT__
   void AddFragment(const std::shared_ptr<const TFragment>&, TChannel*) override;
#endif
	void BuildHits() override {} // no need to build any hits, everything already done in AddFragment

   static TVector3 GetPosition(int DetNbr); //!<!

   TPaces& operator=(const TPaces&); //!<!

private:
   static bool fSetCoreWave; //!<!  Flag for Waveforms ON/OFF

public:
   static bool SetCoreWave() { return fSetCoreWave; } //!<!

   void Copy(TObject&) const override;            //!<!
   void Print(Option_t* opt = "") const override; //!<!

   /// \cond CLASSIMP
   ClassDefOverride(TPaces, 4) // Paces Physics structure
   /// \endcond
};
/*! @} */
#endif
