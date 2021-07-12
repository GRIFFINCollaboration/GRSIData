#ifndef TRF_PHASE_H
#define TRF_PHASE_H

/** \addtogroup Detectors
 *  @{
 */

#include <vector>
#include <iostream>
#include <cstdio>

#include "TMath.h"

#include "TFragment.h"
#include "TPulseAnalyzer.h"
#include "TDetector.h"

class TRF : public TDetector {
public:
   TRF();
   TRF(const TRF&);
   ~TRF() override;

   Double_t Phase() const 
   {
      if(fPeriod > 0.0f) {
         return (fTime / fPeriod) * TMath::TwoPi(); 
      } else {
         return -10.0; //negative value indicates failed RF fit
      }
   }
   Double_t Time() const { return fTime; } //in ns, not tstamp 10ns
   Double_t Period() const { return fPeriod; } //in ns
   Long_t   TimeStamp() const { return fTimeStamp; }
   time_t   MidasTime() const { return fMidasTime; }

   Double_t GetTimeFitCfd() const
   {
      if(fTime != 0 && fTime < 1000 && fTime > -1000) {
         return GetTimestampCfd() + fTime * 1.6; // ns->cfdunits
      }
      return 0;
   }

   Double_t GetTimeFitns() const
   {
      if(fTime != 0 && fTime < 1000 && fTime > -1000) {
         return TimeStamp() + fTime; // 
      }
      return 0;
   }
   
   Double_t GetTimestampCfd() const
   { // ticks ->cfdunits
      long ts =
         TimeStamp()<<4 & 0x07ffffff; // bit shift by 4 (x16) then knock off the highest bit which is absent from cfd
      return ts;
   }

#ifndef __CINT__
   void AddFragment(const std::shared_ptr<const TFragment>&, TChannel*) override; //!<!
#endif
   void BuildHits() override {} // no need to build any hits, everything already done in AddFragment

   void Copy(TObject&) const override;
   void Clear(Option_t* opt = "") override;       //!<!
   void Print(Option_t* opt = "") const override; //!<!

private:
   time_t fMidasTime;
   Long_t fTimeStamp;
   double fTime; //RF time offset from timestamp, like a CFD value
   double fPeriod;

   /// \cond CLASSIMP
   ClassDefOverride(TRF, 4)
   /// \endcond
};
/*! @} */
#endif
