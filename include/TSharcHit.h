#ifndef TSHARCHIT_H
#define TSHARCHIT_H

/** \addtogroup Detectors
 *  @{
 */

#include <cstdio>
#include <utility>

#include "TVector3.h"
#include "TObject.h"
#include "TRandom.h"
#include "Rtypes.h"
#include "TMath.h"

#include "Globals.h"
#include "TDetectorHit.h"
#include "TFragment.h"

/////////////////////////////////////////////////////////////////////////
//
//  \class TSharcHit, the leaner method.
//
//  This is the updated sharc-hit storage class, designed to better work with
//  the GRSISort Inheritied class method.  A lot has changed, but the main function
//  stays the same.  Since two physial wire hits are needed to make one sharc hit,
//  and the "front" side will always have better resolution/timing properties, we
//  will make the inherited charge/time derive from the front.  So we do not lose the
//  back information, the class additionally holds a TDetectorHit for the back and
//  another for the pad if it is present.
//
////////////////////////////////////////////////////////////////////////////

class TSharcHit : public TDetectorHit {
public:
   TSharcHit();
   TSharcHit(const TSharcHit&);
	TSharcHit(TSharcHit&&) noexcept = default;
	TSharcHit& operator=(const TSharcHit&) = default;
	TSharcHit& operator=(TSharcHit&&) noexcept = default;
   ~TSharcHit() override;

private:
   // UShort_t   fDetectorNumber;  //
   // UShort_t   fFrontStrip;     //
   // UShort_t   fBackStrip;      //

   TDetectorHit fBackHit; //
   TDetectorHit fPadHit;  //

public:
   void Copy(TObject&) const override;        //!<!
   void Copy(TObject&, bool) const override;           //!<!
   void Clear(Option_t* = "") override;       //!<!
   void Print(Option_t* = "") const override; //!<!
	void Print(std::ostream& out) const override; //!<!

   // inline UShort_t GetDetector()       const  { return TDetectorHit::GetDetector();  } //!<!
   UShort_t GetFrontStrip() const { return GetSegment(); } //!<!
   UShort_t GetBackStrip() const { return GetBack().GetSegment(); }          //!<!

   // TDetectorHit* GetFront() const { return this; }
   TDetectorHit GetBack() const { return fBackHit; }
   TDetectorHit GetPad() const { return fPadHit; }

   Double_t GetSomeTime() const
   {
      return GetCfd() - GetBack().GetCfd() + ((GetBackStrip() % 24) * 157.6) - (GetFrontStrip() * 157.6);
   }

   inline Double_t GetDeltaE() const { return TDetectorHit::GetEnergy(); } //!<!
   inline Double_t GetDeltaT() const { return TDetectorHit::GetTime(); }   //!<!

   inline Double_t GetDeltaFrontE() const { return TDetectorHit::GetEnergy(); } //!<!
   inline Double_t GetDeltaFrontT() const { return TDetectorHit::GetTime(); }   //!<!
   inline Double_t GetDeltaBackE() const { return GetBack().GetEnergy(); }          //!<!
   inline Double_t GetDeltaBackT() const { return GetBack().GetTime(); }            //!<!

   inline Int_t GetFrontAddress() const { return TDetectorHit::GetAddress(); } //!<!
   inline Int_t GetBackAddress() const { return GetBack().GetAddress(); }          //!<!
   inline Int_t GetPadAddress() const { return GetPad().GetAddress(); }            //!<!

   inline Double_t GetPadE() const { return GetPad().GetEnergy(); } //!<!
   inline Double_t GetPadT() const { return GetPad().GetTime(); }   //!<!

   // std::pair<int,int>  GetPixel()  { return std::make_pair(fFrontStrip,fBackStrip);  }  //!<!

   Float_t GetFrontCharge() const
   {
      return TDetectorHit::GetCharge();
   }                                                               //!<!  //Charge is now stored after integration.
   Float_t GetBackCharge() const { return GetBack().GetCharge(); } //!<!  //Charge is now stored after integration.
   Float_t GetPadCharge() const { return GetPad().GetCharge(); }   //!<!  //Charge is now stored after integration.

   inline Double_t GetEnergy(Option_t* = "") const override
   {
      if(GetPadAddress() != -1) {
         return TDetectorHit::GetEnergy() + GetPad().GetEnergy();
      }
      return TDetectorHit::GetEnergy();
   }

   Double_t GetThetaDeg(double Xoff = 0.0, double Yoff = 0.0, double Zoff = 0.0) const
   {
      return GetTheta(Xoff, Yoff, Zoff) * TMath::RadToDeg();
   };                                                                          //!<!
   Double_t GetTheta(double Xoff = 0.0, double Yoff = 0.0, double Zoff = 0.0) const; //!<!

   ///////////////////////////////////////////////////////////////////////
   ///////////////////////////////////////////////////////////////////////
   ///////////////////////////////////////////////////////////////////////
   // void SetDetectorNumber(const UShort_t& det) { fDetectorNumber = det;   }  //!<!
   // void SetFrontStrip(const UShort_t& strip)   { fFrontStrip    = strip; }  //!<!
   // void SetBackStrip(const UShort_t& strip)    { fBackStrip     = strip; }  //!<!

   void SetFront(const TFragment& frag); //!<!
   void SetBack(const TFragment& frag);  //!<!
   void SetPad(const TFragment& frag);   //!<!

   TVector3 GetPosition(Double_t dist) const override; //!<!
   TVector3 GetPosition() const override;              //!<!

private:
   Double_t GetDefaultDistance() const { return 0.; };

   /// \cond CLASSIMP
   ClassDefOverride(TSharcHit, 6) // NOLINT(readability-else-after-return)
   /// \endcond
};
/*! @} */
#endif
