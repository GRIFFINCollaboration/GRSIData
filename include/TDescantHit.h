#ifndef DESCANTHIT_H
#define DESCANTHIT_H

/** \addtogroup Detectors
 *  @{
 */

#include <cstdio>
#include <cmath>
#include <vector>

#include "TVector3.h"

#include "TFragment.h"
#include "TChannel.h"

#include "TDetectorHit.h"

class TDescantHit : public TDetectorHit {
public:
   TDescantHit();
   TDescantHit(const TDescantHit&);
   TDescantHit(TDescantHit&&) noexcept            = default;
   TDescantHit& operator=(const TDescantHit&)     = default;
   TDescantHit& operator=(TDescantHit&&) noexcept = default;
   explicit TDescantHit(const TFragment& frag);
   ~TDescantHit() override = default;

private:
   Int_t                fFilter;
   Int_t                fPsd;
   Int_t                fZc;
   Int_t                fCcShort;
   Int_t                fCcLong;
   std::vector<int16_t> fCfdMonitor;
   std::vector<int>     fPartialSum;

public:
   /////////////////////////		/////////////////////////////////////
   inline void SetFilterPattern(const int& x) { fFilter = x; }   //!<!
   inline void SetPsd(const int& x) { fPsd = x; }                //!<!
   inline void SetZc(const int& x) { fZc = x; }                  //!<!
   inline void SetCcShort(const int& x) { fCcShort = x; }        //!<!
   inline void SetCcLong(const int& x) { fCcLong = x; }          //!<!

   // 4G 26bit CFD word: 4 bit timestamp remainder, 22 bit actual cfd word
   // inline void SetCfd(const Int_t& x)           { fCfd    = (fCfd & 0x70000000) | (x & 0x1fffffff); } //!<!

   /////////////////////////		/////////////////////////////////////
   inline Int_t                 GetFilterPattern() const { return fFilter; }   //!<!
   inline Int_t                 GetPsd() const { return fPsd; }                //!<!
   inline Int_t                 GetZc() const { return fZc; }                  //!<!
   inline Int_t                 GetCcShort() const { return fCcShort; }        //!<!
   inline Int_t                 GetCcLong() const { return fCcLong; }          //!<!
   inline std::vector<int16_t>& GetCfdMonitor() { return fCfdMonitor; }
   inline std::vector<int>&     GetPartialSum() { return fPartialSum; }

   Float_t GetCfd() const override;
   Int_t   GetRemainder() const;

   Int_t                CalculateCfd(double attenuation, unsigned int delay, int halfsmoothingwindow,
                                     unsigned int interpolation_steps);   //!<!
   Int_t                CalculateCfdAndMonitor(double attenuation, unsigned int delay, int halfSmoothingWindow,
                                               unsigned int interpolationSteps, std::vector<Short_t>& monitor);   //!<!
   std::vector<Short_t> CalculateCfdMonitor(double attenuation, unsigned int delay,
                                            unsigned int halfSmoothingWindow);            //!<!
   std::vector<Short_t> CalculateSmoothedWaveform(unsigned int halfSmoothingWindow);      //!<!
   std::vector<Int_t>   CalculatePartialSum();                                            //!<!
   Int_t                CalculatePsd(double fraction, unsigned int interpolationSteps);   //!<!
   Int_t                CalculatePsdAndPartialSums(double fraction, unsigned int interpolationSteps,
                                                   std::vector<Int_t>& partialSums);   //!<!

   bool InFilter(Int_t);   //!<!

   bool AnalyzeWaveform();   //!<!

   TVector3 GetPosition(Double_t dist) const override;   //!<!
   TVector3 GetPosition() const override;                //!<!

   void Copy(TObject&) const override;              //!<!
   void Copy(TObject&, bool) const override;        //!<!
   void Clear(Option_t* opt = "") override;         //!<!
   void Print(Option_t* opt = "") const override;   //!<!
   void Print(std::ostream& out) const override;    //!<!

private:
   Double_t GetDefaultDistance() const { return 222.; }

   /// \cond CLASSIMP
   ClassDefOverride(TDescantHit, 7)   // NOLINT(readability-else-after-return)
   /// \endcond
};
/*! @} */
#endif
