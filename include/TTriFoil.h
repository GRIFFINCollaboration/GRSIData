#ifndef TTRIFOIL_PHASE_H
#define TTRIFOIL_PHASE_H

/** \addtogroup Detectors
 *  @{
 */

#include <vector>
#include <iostream>
#include <cstdio>

#include "TDetector.h"
#include "TFragment.h"

class TTriFoil : public TDetector {
public:
   TTriFoil();
   TTriFoil(const TTriFoil& rhs);
   TTriFoil(TTriFoil&&) noexcept            = default;
   TTriFoil& operator=(const TTriFoil&)     = default;
   TTriFoil& operator=(TTriFoil&&) noexcept = default;
   ~TTriFoil() override                     = default;

   std::vector<Short_t> GetWave() { return fTfWave; }
   bool                 Beam() const { return fBeam; }
   int                  NTBeam() const { return fTBeam.size(); }
   // int TBeam() const { return TBeam(0); }
   int TBeam(unsigned int n = 0) const
   {
      if(n < fTBeam.size()) {
         return fTBeam.at(n);
      }
      return -1;
   }

   bool   HasWave() const { return !fTfWave.empty(); }
   time_t GetTimeStamp() const { return fTimestamp; }

#ifndef __CINT__
   void AddFragment(const std::shared_ptr<const TFragment>&, TChannel*) override;   //!<!
#endif
   void BuildHits() override {}   // no need to build any hits, everything already done in AddFragment

   void Clear(Option_t* opt = "") override;         //!<!
   void Print(Option_t* opt = "") const override;   //!<!
   void Print(std::ostream& out) const override;    //!<!
   void Copy(TObject& rhs) const override;

private:
   std::vector<Short_t> fTfWave;
   Long_t               fTimestamp{0};
   bool                 fBeam{false};
   std::vector<int>     fTBeam;

   /// \cond CLASSIMP
   ClassDefOverride(TTriFoil, 2)   // NOLINT(readability-else-after-return)
   /// \endcond
};
/*! @} */
#endif
