#ifndef TDEMAND_H
#define TDEMAND_H

/** \addtogroup Detectors
 *  @{
 */

#include <vector>
#include <cstdio>

#include "TBits.h"
#include "Math/Vector3D.h"

#include "Globals.h"
#include "TDetector.h"
#include "TDemandHit.h"

class TDemand : public TDetector {
public:
   TDemand();
   TDemand(const TDemand&);
   TDemand(TDemand&&) noexcept = default;
   TDemand& operator=(const TDemand&);   //!<!
   TDemand& operator=(TDemand&&) noexcept = default;
   ~TDemand() override                   = default;

   TDemandHit* GetDemandHit(const int& i) const { return static_cast<TDemandHit*>(GetHit(i)); }

#ifndef __CINT__
   void AddFragment(const std::shared_ptr<const TFragment>&, TChannel*) override;
#endif
   void BuildHits() override {}   // no need to build any hits, everything already done in AddFragment

   static ROOT::Math::Polar3DVector GetPosition(int DetNbr) { return fDetectorPosition[DetNbr]; }   //!<!

private:
   static bool                                     fSetCoreWave;        //!<!  Flag for Waveforms ON/OFF
   static std::array<ROOT::Math::Polar3DVector, 6> fDetectorPosition;   //!<!  Position of each detector (plus one default position)

public:
   static bool SetCoreWave() { return fSetCoreWave; }   //!<!

   void Copy(TObject&) const override;              //!<!
   void Print(Option_t* opt = "") const override;   //!<!
   void Print(std::ostream& out) const override;    //!<!

   /// \cond CLASSIMP
   ClassDefOverride(TDemand, 4)   // Demand Physics structure // NOLINT(readability-else-after-return)
   /// \endcond
};
/*! @} */
#endif