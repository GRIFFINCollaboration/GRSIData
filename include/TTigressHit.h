#ifndef TIGRESSHIT_H
#define TIGRESSHIT_H

/** \addtogroup Detectors
 *  @{
 */

#include <cstdio>
#include <cmath>
#if !defined(__CINT__) && !defined(__CLING__)
#include <tuple>
#endif

#include "TMath.h"
#include "TVector3.h"

#include "TFragment.h"
#include "TChannel.h"
#include "TPulseAnalyzer.h"

#include "TDetectorHit.h"

class TTigressHit : public TDetectorHit {
public:
   TTigressHit();
   TTigressHit(const TTigressHit&);
   explicit TTigressHit(const TFragment& frag);
   void CopyFragment(const TFragment& frag);
   ~TTigressHit() override;

private:
   // UShort_t fFirstSegment;
   // Float_t  fFirstSegmentCharge; //!<!

   std::vector<TDetectorHit> fSegments;

	bool    fCoreSet{false};
   bool    fBgoFired{false};
   Float_t fTimeFit{0.};
   Float_t fSig2Noise{0.};

   // need to do sudo tracking to build addback. do not remove.  pcb. */
   // TVector3 fLastHit;                //!   <! */

public:
   void SetHit() {}
   /////////////////////////    /////////////////////////////////////
   void SetCore(const TTigressHit& core) { core.Copy(*this); fCoreSet = true; }                //!<!
   void AddSegment(const TDetectorHit& seg) { fSegments.push_back(seg); } //!<!
   //    void AddBGO(const TDetectorHit& bgo)        { fBgos.push_back(bgo);  }     //!<!
   // void SetInitalHit(const int &i)     { fFirstSegment = i; }        //!<!

   /////////////////////////    /////////////////////////////////////
   // int GetCrystal()   const;           //{  return crystal;      }    //!<!
   // int GetInitialHit() const           {  return fFirstSegment;  }      //!<!

   void     SetWavefit(const TFragment&);
   void     SetWavefit();
   Double_t GetSignalToNoise() const { return fSig2Noise; } //!<!
   Double_t GetFitTime() const { return fTimeFit; }         //!<!

   UShort_t GetArrayNumber() const override { return 4 * (GetDetector() - 1) + GetCrystal(); }

   inline double GetDoppler(double beta, TVector3* vec = nullptr)
   {
      if(vec == nullptr) {
         vec = GetBeamDirection();
      }
      double tmp   = 0;
      double gamma = 1 / (sqrt(1 - pow(beta, 2)));
      tmp          = this->GetEnergy() * gamma * (1 - beta * TMath::Cos(GetPosition().Angle(*vec)));
      return tmp;
   }

   bool CoreSet() const { return fCoreSet; }
   void CoreSet(bool val) { fCoreSet = val; }
   bool BGOFired() const { return fBgoFired; }
   void SetBGOFired(bool fired) { fBgoFired = fired; }

   int GetTimeToTrigger() { return (GetTimeStamp() & 0x7fffff) - (static_cast<Int_t>(GetCfd()) >> 4); }

   int GetSegmentMultiplicity() const { return fSegments.size(); } //!<!
   int GetNSegments() const { return fSegments.size(); }           //!<!
   /* int GetBGOMultiplicity()            const { return fBgos.size();     }  //!<! */
   /* int GetNBGOs()                      const { return fBgos.size();     }  //!<! */

   const TDetectorHit& GetSegmentHit(int i) const { return fSegments.at(i); } //!<!
   /* const TDetectorHit& GetBGO(int i)     const { return fBgos.at(i);      }  //!<! */
   const TDetectorHit& GetCore() const { return *this; } //!<!

   const std::vector<TDetectorHit>& GetSegmentVec() const { return fSegments; }
   /* const std::vector<TDetectorHit>& GetBGOVec()     const { return fBgos; } */

   /* modified by Momiyama and Niikura on Aug. 23, 2016 */
   /* int GetFirstSeg() const { if(fSegments.size()>0) return fSegments.front().GetSegment(); return -1; } */
   /* int GetLastSeg()  const { if(fSegments.size()>0) return fSegments.back().GetSegment(); return -1; } */
   int GetFirstSeg() const
   {
      if(!fSegments.empty()) {
         return fSegments.front().GetSegment();
      }
      return 0;
   }
   int GetLastSeg() const
   {
      if(!fSegments.empty()) {
         return fSegments.back().GetSegment();
      }
      return 0;
   }

   static bool Compare(const TTigressHit& lhs, const TTigressHit& rhs);       //!<!
   static bool CompareEnergy(const TTigressHit& lhs, const TTigressHit& rhs); //!<!

   void SumHit(TTigressHit*); //!<!

   TVector3 GetPosition(Double_t dist = 0.) const override;
   TVector3 GetLastPosition(Double_t dist = 0.) const;

   void Clear(Option_t* opt = "") override;       //!<!
   void Copy(TObject&) const override;            //!<!
   void Copy(TObject&, bool) const override;      //!<!
   void Print(Option_t* opt = "") const override; //!<!
	void Print(std::ostream& out) const override; //!<!

   void SortSegments() { std::sort(fSegments.begin(), fSegments.end()); } //!<!

   /// \cond CLASSIMP
   ClassDefOverride(TTigressHit, 4)
   /// \endcond
};
/*! @} */
#endif
