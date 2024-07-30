/// \author Dhruval Shah <dsi491@uregina.ca>, University of Regina

#ifndef RCMPHIT_H
#define RCMPHIT_H

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

class TRcmpHit : public TDetectorHit {
public:
   // constructors and destructors
   TRcmpHit();                  ///< default constructor
   TRcmpHit(const TRcmpHit&);   ///< copy constructor
   // TRcmpHit(const TRcmpHit& front, const TRcmpHit& back);          ///< dual parameter constructor for front and back hits
   TRcmpHit(const TFragment& frag);                                   ///< single fragment constructor (either front or back)
   TRcmpHit(const TFragment& fragFront, const TFragment& fragBack);   ///< dual parameter constructor for front and back fragments
   ~TRcmpHit() override;                                              ///< destructor

   void Clear(Option_t* opt = "") override;         //!<!
   void Print(Option_t* opt = "") const override;   //!<!
   void Print(std::ostream& out) const override;    //!<!
   void Copy(TObject&) const override;              //!<!
   void Copy(TObject&, bool) const override;        //!<!

   inline Short_t GetFrontStripNumber() const { return fFrontStrip; }   ///< RcmpHit vector elements can access this
   inline Short_t GetBackStripNumber() const  { return fBackStrip; }    ///< RcmpHit vector elements can access this
   inline Short_t GetStripNumber() const      { return fStrip; }        ///< RcmpFrontHit or RcmpBackHit vector elements can access this
   inline UShort_t GetArrayNumber() const override { return ( 32 * (GetDetector() - 1) + GetSegment() ); }   //!<! //function to plot all front strips or back strips for energy vs channel histograms

private:
   Double_t GetDefaultDistance() const { return 0.0; }

   // data members for front and back strips, the values for these will be set in the constructors
   Short_t fFrontStrip = -1;   ///< this corresponds to the front strip number for a Pixel Hit
   Short_t fBackStrip  = -1;   ///< this corresponds to the back strip number for a Pixel Hit
   Short_t fStrip      = -1;   ///< this corresponds to the front strip number for a front hit and the back strip number for a back hit

   /// \cond CLASSIMP
   ClassDefOverride(TRcmpHit, 1); // NOLINT
   /// \endcond
};

/*! @} */
#endif
