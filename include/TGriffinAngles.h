#ifndef TGRIFFINANGLES_H
#define TGRIFFINANGLES_H

/** \addtogroup Analysis
 *  @{
 */

////////////////////////////////////////////////////////////////////////////////
///
/// \class TGriffinAngles
///
/// This class provides the number of angles, their values, and the index a
/// specific angle is at, as well as how many combinations contribute to this
/// angle.
/// It is meant to be used for the angular correlation analysis.
///
////////////////////////////////////////////////////////////////////////////////

#include <set>
#include <map>
#include <functional>

#include "TCollection.h"
#include "TNamed.h"
#include "TGraphErrors.h"

#include "Globals.h"

class TGriffinAngles : public TNamed {
public:
   explicit TGriffinAngles(double distance = 145., bool folding = false, bool grouping = false, bool addback = true);
   TGriffinAngles(const TGriffinAngles&)                = default;
   TGriffinAngles(TGriffinAngles&&) noexcept            = default;
   TGriffinAngles& operator=(const TGriffinAngles&)     = default;
   TGriffinAngles& operator=(TGriffinAngles&&) noexcept = default;
   ~TGriffinAngles()                                    = default;

   double Distance() const { return fDistance; }
   bool   Folding() const { return fFolding; }
   bool   Grouping() const { return fGrouping; }
   bool   Addback() const { return fAddback; }

   int    Index(double angle);
   int    NumberOfAngles() const { return fAngles.size(); }
   double Angle(int index) const
   {
      auto it = fAngles.begin();
      std::advance(it, index);
      return *it;
   }
   double AverageAngle(int index) const;
   int    Count(double angle)
   {
      /// If the angle is in our map, report how often it exists, otherwise return zero.
      if(fAngleCount.count(static_cast<int>(std::round(angle / fRounding))) == 1) { return fAngleCount.at(static_cast<int>(std::round(angle / fRounding))); }
      return 0;
   }

   void                       FoldOrGroup(TGraphErrors* z0, TGraphErrors* z2, TGraphErrors* z4, bool verbose = false) const;
   std::set<double>::iterator begin() const { return fAngles.begin(); }
   std::set<double>::iterator end() const { return fAngles.end(); }

   bool ExcludeDetector(int detector) const;
   bool ExcludeCrystal(int detector, int crystal) const;

   void Print(Option_t* = "") const override;

   Long64_t Merge(TCollection* list)
   {
      for(auto* griffinAngles : *list) { Add(static_cast<TGriffinAngles*>(griffinAngles)); }
      return 0;
   }

   static double Rounding() { return fRounding; }
   static void   Rounding(const double& val) { fRounding = val; }

   static EVerbosity Verbosity() { return fVerbosity; }
   static void       Verbosity(const EVerbosity& val) { fVerbosity = val; }

private:
   void Add(TGriffinAngles* griffinAngles);

   static EVerbosity     fVerbosity;           ///< verbosity level
   double                fDistance{145.};      ///< distance of detector from center of array in mmm
   bool                  fFolding{false};      ///< flag indicating whether we fold our distribution around 90 degree
   bool                  fGrouping{false};     ///< flag indicating whether we group close angles together
   bool                  fAddback{true};       ///< flag indicating whether we use addback
   static double         fRounding;            ///< we consider any angles whose difference is less than this to be equal
   std::vector<int>      fExcludedDetectors;   ///< list of detectors that are excluded in calculating the angles
   std::vector<int>      fExcludedCrystals;    ///< list of crystals that are excluded in calculating the angles, the crystals are numbered as 4*(det-1)+cry, so start at 0 and go up to 63
   std::vector<int>      fCustomGrouping;      ///< list of custom groups
   std::set<double>      fAngles;              ///< set of unique angles, when grouping is used, the largest angle of the group is used!
   std::map<double, int> fAngleMap;            ///< Maps angles to indices. This is fairly straight forward without grouping, but if grouping is used multiple angles can be mapped to the same index.
   std::map<int, int>    fAngleCount;          ///< Maps angles (divided by rounding and cast to integers) to number of combinations contributing to it.

   /// \cond CLASSIMP
   ClassDefOverride(TGriffinAngles, 5)   // NOLINT(readability-else-after-return)
   /// \endcond
};
/*! @} */
#endif
