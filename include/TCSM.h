#ifndef TCSM_H
#define TCSM_H

/** \addtogroup Detectors
 *  @{
 */

/////////////////////////////////////////////////////////////
///
/// \class TCSM
///
/////////////////////////////////////////////////////////////

#include <vector>
#include <cstdio>
#include <map>
#include <iostream>
#if !defined(__CINT__) && !defined(__CLING__)
#include <tuple>
#include <iterator>
#include <algorithm>
#endif
#include <utility>
#include <set>

#include "TMath.h"
#include "TVector3.h"

#include "Globals.h"
#include "TFragment.h"
#include "TChannel.h"
#include "TDetector.h"
#include "TCSMHit.h"
#include "TGRSIMnemonic.h"

class TCSM : public TDetector {
public:
   TCSM();
   ~TCSM() override;

   TCSMHit* GetCSMHit(const int& i) const { return static_cast<TCSMHit*>(GetHit(i)); }

   static TVector3 GetPosition(int detector, char pos, int horizontalstrip, int verticalstrip, double X = 0.00,
                               double Y = 0.00, double Z = 0.00);

#ifndef __CINT__
   void AddFragment(const std::shared_ptr<const TFragment>&, TChannel*) override; //!<!
#endif
   void BuildHits() override;

private:
   std::map<int16_t, std::vector<std::vector<std::vector<std::pair<TFragment, TGRSIMnemonic>>>>> fFragments; //!<!
   double               fAlmostEqualWindow;

   static int fCfdBuildDiff; //!<! largest acceptable time difference between events (clock ticks)  (50 ns)

   void BuildVH(std::vector<std::vector<std::pair<TFragment, TGRSIMnemonic>>>&, std::vector<TDetectorHit*>&);
   void BuilddEE(std::vector<std::vector<TDetectorHit*>>&, std::vector<TDetectorHit*>&);
   void OldBuilddEE(std::vector<TDetectorHit*>&, std::vector<TDetectorHit*>&, std::vector<TDetectorHit*>&);
   void MakedEE(std::vector<TDetectorHit*>& DHitVec, std::vector<TDetectorHit*>& EHitVec, std::vector<TDetectorHit*>& BuiltHits);
   TCSMHit* MakeHit(std::pair<TFragment, TGRSIMnemonic>&, std::pair<TFragment, TGRSIMnemonic>&);
   TCSMHit* MakeHit(std::vector<std::pair<TFragment, TGRSIMnemonic>>&, std::vector<std::pair<TFragment, TGRSIMnemonic>>&);
   TCSMHit* CombineHits(TDetectorHit*, TDetectorHit*);
   void    RecoverHit(char, std::pair<TFragment, TGRSIMnemonic>&, std::vector<TDetectorHit*>&);
   bool    AlmostEqual(int, int) const;
   bool    AlmostEqual(double, double) const;

   /// \cond CLASSIMP
   ClassDefOverride(TCSM, 5) // NOLINT(readability-else-after-return)
   /// \endcond
};
/*! @} */
#endif
