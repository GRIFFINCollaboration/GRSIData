#ifndef TEMMA_H
#define TEMMA_H

/** \addtogroup Detectors
 *  @{
 */

#include <vector>
#include <cstdio>

#include "TBits.h"
#include "TVector3.h"

#include "Globals.h"
#include "TDetector.h"
#include "TEmmaHit.h"
#include "TChannel.h"

class TEmma : public TDetector {

public:
   TEmma();
   TEmma(const TEmma&);
   ~TEmma() override;

public:
   TEmmaHit* GetEmmaHit(const int& i) const { return static_cast<TEmmaHit*>(GetHit(i)); }
   TEmmaHit* GetICHit(const int& i);
   TEmmaHit* GetAnodeHit(const int& i);
   TEmmaHit* GetTdcHit(const int& i);
   TEmmaHit* GetSiHit(const int& i);
   TEmmaHit* GetSSBHit(const int& i);
   TEmmaHit* GetTriggerHit(const int& i);

#ifndef __CINT__
   void AddFragment(const std::shared_ptr<const TFragment>&, TChannel*) override;
#endif
	void BuildHits() override; 

   Short_t GetICMultiplicity() const { return fEmmaICHits.size(); }
   Short_t GetAnodeMultiplicity() const { return fEmmaAnodeHits.size(); }
   Short_t GetTdcMultiplicity() const { return fEmmaTdcHits.size(); }
   Short_t GetSiMultiplicity() const { return fEmmaSiHits.size(); }
   Short_t GetSSBMultiplicity() const { return fEmmaSSBHits.size(); }
   Short_t GetTriggerMultiplicity() const { return fEmmaTriggerHits.size(); }

  // static TVector3 GetPosition(double delayL, double delayR, double delayT, double delayB); //!<!
   static TVector3 GetPosition(double left, double right, double top, double bottom, double delayL, double delayR, double delayT, double delayB );
   TEmma& operator=(const TEmma&); //!<!

private:
   std::vector<TEmmaHit> fEmmaICHits;
   std::vector<TEmmaHit> fEmmaAnodeHits;
   std::vector<TEmmaHit> fEmmaTdcHits;
   std::vector<TEmmaHit> fEmmaSiHits;
   std::vector<TEmmaHit> fEmmaSSBHits;
   std::vector<TEmmaHit> fEmmaTriggerHits;

   static double fAnodeTrigger;   //!<!
   static double fICEnergy;   //!<!
   static double fXdiff;
   static double fXsum;
   static double fYdiff;
   static double fYsum;
   static double fXlength;
   static double fYlength;
   static short fFail;


public:

   void Copy(TObject&) const override;            //!<!
   void Clear(Option_t* opt = "all") override;    //!<!
   void Print(Option_t* opt = "") const override; //!<!

   /// \cond CLASSIMP
   ClassDefOverride(TEmma, 4) // Emma Physics structure
   /// \endcond
};
/*! @} */
#endif
