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
   ~TDemand() override                    = default;

   TDemandHit*     GetDemandHit(const int& i) const { return static_cast<TDemandHit*>(GetHit(i)); }
   static TVector3 GetPosition(int DetNbr);          //!<!
   static TVector3 GetPosition(const TDemandHit&);   //!<!

#ifndef __CINT__
   void AddFragment(const std::shared_ptr<const TFragment>&, TChannel*) override;
#endif
   void BuildHits() override {}   // no need to build any hits, everything already done in AddFragment


private:
   static bool fSetCoreWave;   //!<!  Flag for Waveforms ON/OFF

   //physical information
   static const std::array<double, 6> fXPositions;   //!
   static const std::array<double, 6> fYPositions;   //!
   static const std::array<double, 6> fZPositions;   //!

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
