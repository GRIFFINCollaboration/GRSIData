#ifndef EMMAHIT_H
#define EMMAHIT_H

/** \addtogroup Detectors
 *  @{
 */

#include <cstdio>
#include <cmath>

#include "TVector3.h"

#include "TFragment.h"
#include "TChannel.h"

#include "TGRSIDetectorHit.h"

class TEmmaHit : public TGRSIDetectorHit {
public:
   TEmmaHit();
   TEmmaHit(const TEmmaHit&);
   TEmmaHit(const TFragment& frag) : TGRSIDetectorHit(frag) {}
   ~TEmmaHit() override;

private:
   Int_t fFilter{0};

public:
   /////////////////////////  Setters	/////////////////////////////////////
   inline void SetFilterPattern(const int& x) { fFilter = x; } //!<!
   // void SetHit();
   void SetLeft(double q) { fLeft = q; }
   void SetRight(double q) { fRight = q; }
   void SetTop(double q) { fTop = q; }
   void SetBottom(double q) { fBottom = q; }
   void SetAnodeTrigger(double q) { fAnodeTrigger = q; }
   void SetTdcNumber(int n) { fTdcNumber = n; }
   void SetFailedFill(int n) { fFail = n; }

   /////////////////////////  Getters	/////////////////////////////////////
   inline Int_t GetFilterPattern() const { return fFilter; } //!<!

   TVector3 GetPosition() const override;              //!<!
   TVector3 GetPosition(double delayL, double delayR, double delayT, double delayB) const;              //!<!
   Double_t GetLeft() const {return fLeft; }
   Double_t GetRight() const {return fRight; }
   Double_t GetTop() const {return fTop; }
   Double_t GetBottom() const {return fBottom; }
   Short_t GetTdcNumber() const { return fTdcNumber; }

   Short_t GetFailedFill() const { return fFail; }

   /////////////////////////  TChannel Helpers /////////////////////////////////////
   bool InFilter(Int_t); //!<!

public:
   void Clear(Option_t* opt = "") override;            //!<!
   void Print(Option_t* opt = "") const override;      //!<!
   void     Copy(TObject&) const override;             //!<!

private:
   Double_t fLeft{0};             // Left Cathode
   Double_t fRight{0};            // Right Cathode
   Double_t fTop{0};              // Top Cathode
   Double_t fBottom{0};           // Bottom Cathode
   Double_t fAnodeTrigger{0};     // Trigger Anode
   Short_t fTdcNumber{0};         // Used in BuildHits
   Double_t fICSum{0};		  // IC Sum
   Short_t fFail{0};              // Check BuildHits is working
   Double_t fSiE{0};              // Si at focal plane
   static double fLdelay;
   static double fRdelay;
   static double fTdelay;
   static double fBdelay;

   /// \cond CLASSIMP
   ClassDefOverride(TEmmaHit, 3);
   /// \endcond
};
/*! @} */
#endif
