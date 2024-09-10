#ifndef ZERODEGREEHIT_H
#define ZERODEGREEHIT_H

/** \addtogroup Detectors
 *  @{
 */

/////////////////////////////////////////////////////////////////
///
/// TZeroDegreeHit
///
/// This is class that contains the information about a ZDS
/// hit. This class is used to find energy, time, etc.
///
/////////////////////////////////////////////////////////////////

#include <cstdio>
#include <cmath>

#include "TFragment.h"
#include "TChannel.h"

#include "TVector3.h"

#include "TDetectorHit.h"

class TZeroDegreeHit : public TDetectorHit {
public:
   TZeroDegreeHit();
   TZeroDegreeHit(const TZeroDegreeHit&);
   TZeroDegreeHit(TZeroDegreeHit&&) noexcept            = default;
   TZeroDegreeHit& operator=(const TZeroDegreeHit&)     = default;
   TZeroDegreeHit& operator=(TZeroDegreeHit&&) noexcept = default;
   explicit TZeroDegreeHit(const TFragment& frag);
   ~TZeroDegreeHit() override = default;

private:
   Int_t                fFilter{0};
   std::vector<int16_t> fCfdMonitor;
   std::vector<int>     fPartialSum;

public:
   /////////////////////////		/////////////////////////////////////
   inline void SetFilterPattern(const int& x) { fFilter = x; }   //!<!

   /////////////////////////		/////////////////////////////////////
   inline Int_t                 GetFilterPattern() const { return fFilter; }   //!<!
   inline std::vector<int16_t>& GetCfdMonitor() { return fCfdMonitor; }
   inline std::vector<int>&     GetPartialSum() { return fPartialSum; }

   Float_t GetCfd() const override;
   Int_t   GetRemainder() const;

   Int_t                CalculateCfd(double attenuation, unsigned int delay, int halfsmoothingwindow,
                                     unsigned int interpolationSteps);   //!<!
   Int_t                CalculateCfdAndMonitor(double attenuation, unsigned int delay, int halfsmoothingwindow,
                                               unsigned int interpolationSteps, std::vector<Short_t>& monitor);   //!<!
   std::vector<Short_t> CalculateCfdMonitor(double attenuation, int delay, int halfsmoothingwindow);              //!<!
   std::vector<Short_t> CalculateSmoothedWaveform(unsigned int halfsmoothingwindow);                              //!<!
   std::vector<Int_t>   CalculatePartialSum();                                                                    //!<!

   bool InFilter(Int_t);   //!<!

   bool AnalyzeWaveform();   //!<!

   void Clear(Option_t* opt = "") override;         //!<!
   void Print(Option_t* opt = "") const override;   //!<!
   void Print(std::ostream& out) const override;    //!<!
   void Copy(TObject&) const override;              //!<!
   void Copy(TObject&, bool) const override;        //!<!

   // Position Not written for ZeroDegree Yet

   /// \cond CLASSIMP
   ClassDefOverride(TZeroDegreeHit, 4)   // Stores the information for a ZeroDegreeHit // NOLINT(readability-else-after-return)
   /// \endcond
};
/*! @} */
#endif
