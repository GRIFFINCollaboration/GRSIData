#ifndef TIGRESSHIT_H
#define TIGRESSHIT_H

/** \addtogroup Detectors
 *  @{
 */

#include <cstdio>
#include <cmath>

#include "TVector3.h"

#include "TFragment.h"

#include "TDetectorHit.h"

class TTigressHit : public TDetectorHit {
public:
   enum class ETigressHitBits : std::uint8_t {
      kTotalPU1    = 1 << 0,
      kTotalPU2    = 1 << 1,
      kPUHitOffset = kTotalPU2,
      kPUHit1      = 1 << 2,
      kPUHit2      = 1 << 3,
      kBit4        = 1 << 4,
      kBit5        = 1 << 5,
      kBit6        = 1 << 6,
      kBit7        = 1 << 7
   };

   TTigressHit();
   TTigressHit(const TTigressHit&);
   TTigressHit(TTigressHit&&) noexcept            = default;
   TTigressHit& operator=(const TTigressHit&)     = default;
   TTigressHit& operator=(TTigressHit&&) noexcept = default;
   explicit TTigressHit(const TFragment&);
   ~TTigressHit() override = default;

   void CopyFragment(const TFragment&);

private:
   Int_t                   fFilter{0};        ///<  The Filter Word
   TTransientBits<UChar_t> fTigressHitBits;   ///<  Transient Member Flags

   // copied from old TTigressHit
   std::vector<TDetectorHit> fSegments;

   bool    fCoreSet{false};
   bool    fBgoFired{false};
   Float_t fTimeFit{0.};
   Float_t fSig2Noise{0.};

public:
   // copied from old TTigressHit
   void SetCore(const TTigressHit& core)
   {
      core.Copy(*this);
      fCoreSet = true;
   }
   void    AddSegment(const TDetectorHit& seg) { fSegments.push_back(seg); }   //!<!
   void    SetWavefit(const TFragment&);
   void    SetWavefit();
   Float_t GetSignalToNoise() const { return fSig2Noise; }   //!<!
   Float_t GetFitTime() const { return fTimeFit; }           //!<!

   double GetDoppler(double beta, TVector3* vec = nullptr)
   {
      if(vec == nullptr) {
         vec = GetBeamDirection();
      }
      double result = 0;
      double gamma  = 1 / (sqrt(1 - pow(beta, 2)));
      result        = GetEnergy() * gamma * (1 - beta * TMath::Cos(GetPosition().Angle(*vec)));
      return result;
   }

   bool CoreSet() const { return fCoreSet; }
   void CoreSet(bool val) { fCoreSet = val; }
   bool BGOFired() const { return fBgoFired; }
   void SetBGOFired(bool fired) { fBgoFired = fired; }

   int GetTimeToTrigger() { return (GetTimeStamp() & 0x7fffff) - (static_cast<Int_t>(GetCfd()) >> 4); }

   int GetSegmentMultiplicity() const { return fSegments.size(); }   //!<!
   int GetNSegments() const { return fSegments.size(); }             //!<!

   const TDetectorHit&              GetSegmentHit(int i) const { return fSegments.at(i); }   //!<!
   const std::vector<TDetectorHit>& GetSegments() const { return fSegments; }
   int                              GetFirstSegment() const
   {
      if(!fSegments.empty()) {
         return fSegments.front().GetSegment();
      }
      return 0;
   }
   int GetLastSegment() const
   {
      if(!fSegments.empty()) {
         return fSegments.back().GetSegment();
      }
      return 0;
   }

   void SortSegments() { std::sort(fSegments.begin(), fSegments.end()); }   //!<!

   /////////////////////////  Setters	/////////////////////////////////////
   void SetFilterPattern(const int& x) { fFilter = x; }   //!<!

   /////////////////////////  Getters	/////////////////////////////////////
   Int_t    GetFilterPattern() const { return fFilter; }   //!<!
   Double_t GetNoCTEnergy(Option_t* opt = "") const;

   /////////////////////////  Recommended Functions/////////////////////////

   /////////////////////////  TChannel Helpers /////////////////////////////////////
   UShort_t NPileUps() const;
   UShort_t PUHit() const;
   void     SetNPileUps(UChar_t npileups);
   void     SetPUHit(UChar_t puhit);

   /////////////////////////		/////////////////////////////////////

   UShort_t GetArrayNumber() const override { return (4 * (GetDetector() - 1) + (GetCrystal() + 1)); }   //!<!
   // returns a number 1-64 ( 1 = Detector 1 blue;  64 =  Detector 16 white; )

   bool InFilter(Int_t);   //!<!

   static bool Compare(const TTigressHit*, const TTigressHit*);         //!<!
   static bool CompareEnergy(const TTigressHit*, const TTigressHit*);   //!<!
   void        Add(const TDetectorHit*) override;                       //!<!

   void Clear(Option_t* opt = "") override;         //!<!
   void Print(Option_t* opt = "") const override;   //!<!
   void Print(std::ostream& out) const override;    //!<!
   void Copy(TObject&) const override;              //!<!
   void Copy(TObject&, bool) const override;        //!<!

   TVector3 GetPosition(double dist) const override;   //!<!
   TVector3 GetLastPosition(double dist) const;        //!<!
   TVector3 GetPosition() const override;

private:
   void     SetTigressFlag(ETigressHitBits, Bool_t set);
   Double_t GetDefaultDistance() const { return 110.; }

   /// \cond CLASSIMP
   ClassDefOverride(TTigressHit, 8);   // Information about a GRIFFIN Hit // NOLINT(readability-else-after-return)
   /// \endcond
};
/*! @} */
#endif
