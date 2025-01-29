#ifndef TGRIFFIN_H
#define TGRIFFIN_H

/** \addtogroup Detectors
 *  @{
 */

#include <utility>
#include <vector>
#include <cstdio>
#include <functional>
//#include <tuple>

#include "TBits.h"
#include "TVector3.h"

#include "Globals.h"
#include "TGriffinHit.h"
#include "TSuppressed.h"
#include "TTransientBits.h"
#include "TSpline.h"

class TGriffin : public TSuppressed {
public:
   enum class EGriffinBits {
      kIsAddbackSet           = 1 << 0,
      kSpare1                 = 1 << 1,
      kIsCrossTalkSet         = 1 << 2,
      kSpare3                 = 1 << 3,
      kIsSuppressed           = 1 << 4,
      kSpare5                 = 1 << 5,
      kIsSuppressedAddbackSet = 1 << 6,
      kSpare7                 = 1 << 7
   };

   TGriffin();
   TGriffin(const TGriffin&);
   TGriffin(TGriffin&&) noexcept = default;
   TGriffin& operator=(const TGriffin&);   //!<!
   TGriffin& operator=(TGriffin&&) noexcept = default;
   ~TGriffin() override;

   TGriffinHit* GetGriffinHit(const int& i);   //!<!

   static TVector3    GetPosition(int DetNbr, int CryNbr = 5, double dist = 110.0);   //!<!
   static TVector3    GetDetectorPosition(int DetNbr);                                //!<!
   static const char* GetColorFromNumber(int number);
#ifndef __CINT__
   void AddFragment(const std::shared_ptr<const TFragment>&, TChannel*) override;   //!<!
#endif
   void BuildHits() override {}   // no need to build any hits, everything already done in AddFragment

   void ClearTransients() override
   {
      TDetector::ClearTransients();
      fGriffinBits = 0;
   }
   void ResetFlags() const;

#if !defined(__CINT__) && !defined(__CLING__)
   void SetAddbackCriterion(std::function<bool(const TDetectorHit*, const TDetectorHit*)> criterion)
   {
      fAddbackCriterion = std::move(criterion);
   }
   std::function<bool(const TDetectorHit*, const TDetectorHit*)> GetAddbackCriterion() const { return fAddbackCriterion; }

   bool AddbackCriterion(const TDetectorHit* hit1, const TDetectorHit* hit2) override { return fAddbackCriterion(hit1, hit2); }
#endif

   Short_t      GetAddbackMultiplicity();
   TGriffinHit* GetAddbackHit(const int& i);
   bool         IsAddbackSet() const;
   void         ResetAddback();                                   //!<!
   UShort_t     GetNAddbackFrags(const size_t& idx);

#if !defined(__CINT__) && !defined(__CLING__)
   void SetSuppressionCriterion(std::function<bool(const TDetectorHit*, const TDetectorHit*)> criterion)
   {
      fSuppressionCriterion = std::move(criterion);
   }
   std::function<bool(const TDetectorHit*, const TDetectorHit*)> GetSuppressionCriterion() const { return fSuppressionCriterion; }

   bool SuppressionCriterion(const TDetectorHit* hit, const TDetectorHit* bgoHit) override { return fSuppressionCriterion(hit, bgoHit); }
#endif

   TGriffinHit* GetSuppressedHit(const int& i);   //!<!
   Short_t      GetSuppressedMultiplicity(const TBgo* bgo);
   bool         IsSuppressed() const;
   void         ResetSuppressed();

   Short_t      GetSuppressedAddbackMultiplicity(const TBgo* bgo);
   TGriffinHit* GetSuppressedAddbackHit(const int& i);
   bool         IsSuppressedAddbackSet() const;
   void         ResetSuppressedAddback();
   UShort_t     GetNSuppressedAddbackFrags(const size_t& idx);

private:
#if !defined(__CINT__) && !defined(__CLING__)
   static std::function<bool(const TDetectorHit*, const TDetectorHit*)> fAddbackCriterion;
   static std::function<bool(const TDetectorHit*, const TDetectorHit*)> fSuppressionCriterion;
#endif

   static bool fSetCoreWave;   //!<!  Flag for Waveforms ON/OFF

   int64_t                         fCycleStart;    //!<!  The start of the cycle
   mutable TTransientBits<UChar_t> fGriffinBits;   // Transient member flags

   mutable std::vector<TDetectorHit*> fAddbackHits;     //!<! Used to create addback hits on the fly
   mutable std::vector<UShort_t>      fAddbackFrags;    //!<! Number of crystals involved in creating in the addback hit

   std::vector<TDetectorHit*> fSuppressedHits;    //!<!  The set of suppressed crystal hits

   mutable std::vector<TDetectorHit*> fSuppressedAddbackHits;     //!<! Used to create suppressed addback hits on the fly
   mutable std::vector<UShort_t>      fSuppressedAddbackFrags;    //!<! Number of crystals involved in creating in the suppressed addback hit

public:
   static bool      SetCoreWave() { return fSetCoreWave; }   //!<!

private:
   static std::array<TVector3, 17> fCloverPosition;                            //!<! Position of each HPGe Clover
   void                            ClearStatus() const { fGriffinBits = 0; }   //!<!
   void                            SetBitNumber(EGriffinBits bit, Bool_t set) const;
   Bool_t                          TestBitNumber(EGriffinBits bit) const { return fGriffinBits.TestBit(bit); }

   // Cross-Talk stuff
public:
   static Double_t CTCorrectedEnergy(const TGriffinHit* hit_to_correct, const TGriffinHit* other_hit, bool time_constraint = true);
   Bool_t          IsCrossTalkSet() const;
   void            FixCrossTalk();

private:
   // This is where the general untouchable functions live.
   void SetAddback(bool flag = true) const;
   void SetSuppressed(bool flag = true) const;
   void SetSuppressedAddback(bool flag = true) const;

   void SetCrossTalk(bool flag = true) const;

public:
   void Copy(TObject&) const override;              //!<!
   void Clear(Option_t* opt = "all") override;      //!<!
   void Print(Option_t* opt = "") const override;   //!<!
   void Print(std::ostream& out) const override;    //!<!

   /// \cond CLASSIMP
   ClassDefOverride(TGriffin, 7)   // Griffin Physics structure // NOLINT(readability-else-after-return)
   /// \endcond
};
/*! @} */
#endif
