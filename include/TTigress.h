#ifndef TTIGRESS_H
#define TTIGRESS_H

/** \addtogroup Detectors
 *  @{
 */

#include <utility>
#include <vector>
#include <cstdio>
#include <functional>

#include "TVector3.h"

#include "TTigressHit.h"
#include "TSuppressed.h"
#include "TTransientBits.h"

class TTigress : public TSuppressed {
public:
   enum class ETigressBits : std::uint8_t {
      kIsAddbackSet           = 1 << 0,
      kSpare1                 = 1 << 1,
      kIsCrossTalkSet         = 1 << 2,
      kSpare3                 = 1 << 3,
      kIsSuppressed           = 1 << 4,
      kSpare5                 = 1 << 5,
      kIsSuppressedAddbackSet = 1 << 6,
      kSpare7                 = 1 << 7
   };

   enum class ETigressGlobalBits : std::uint8_t {
      kSpare0       = BIT(0),
      kSetCoreWave  = BIT(1),
      kSetSegWave   = BIT(2),
      kSpare3       = BIT(3),
      kSpare4       = BIT(4),
      kArrayBackPos = BIT(5),
      kVectorsBuilt = BIT(6)   // 110 or 145
   };

   TTigress();
   TTigress(const TTigress&);
   TTigress(TTigress&&) noexcept = default;
   TTigress& operator=(const TTigress&);   //!<!
   TTigress& operator=(TTigress&&) noexcept = default;
   ~TTigress() override;

   TTigressHit* GetTigressHit(const int& i);   //!<!

   static TVector3    GetPosition(int DetNbr, int CryNbr, int SegNbr, double dist = 110.0, bool smear = false);   //!<!
   static TVector3    GetPosition(const TTigressHit* hit, double dist = 110.0, bool smear = false);               //!<!
   static const char* GetColorFromNumber(int number);
#ifndef __CINT__
   void AddFragment(const std::shared_ptr<const TFragment>&, TChannel*) override;   //!<!
#endif
   void BuildHits() override;

   void ClearTransients() override
   {
      TDetector::ClearTransients();
      fTigressBits = 0;
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
   TTigressHit* GetAddbackHit(const int& i);
   bool         IsAddbackSet() const;
   void         ResetAddback();   //!<!
   UShort_t     GetNAddbackFrags(const size_t& idx);

#if !defined(__CINT__) && !defined(__CLING__)
   void SetSuppressionCriterion(std::function<bool(const TDetectorHit*, const TDetectorHit*)> criterion)
   {
      fSuppressionCriterion = std::move(criterion);
   }
   std::function<bool(const TDetectorHit*, const TDetectorHit*)> GetSuppressionCriterion() const { return fSuppressionCriterion; }

   bool SuppressionCriterion(const TDetectorHit* hit, const TDetectorHit* bgoHit) override { return fSuppressionCriterion(hit, bgoHit); }
#endif

   TTigressHit* GetSuppressedHit(const int& i);   //!<!
   Short_t      GetSuppressedMultiplicity(const TBgo* bgo);
   bool         IsSuppressed() const;
   void         ResetSuppressed();

   Short_t      GetSuppressedAddbackMultiplicity(const TBgo* bgo);
   TTigressHit* GetSuppressedAddbackHit(const int& i);
   bool         IsSuppressedAddbackSet() const;
   void         ResetSuppressedAddback();
   UShort_t     GetNSuppressedAddbackFrags(const size_t& idx);

   // Cross-Talk stuff
   static Double_t CTCorrectedEnergy(const TTigressHit* hit_to_correct, const TTigressHit* other_hit, bool time_constraint = true);
   Bool_t          IsCrossTalkSet() const;
   void            FixCrossTalk();

   static void SetTargetOffset(double offset)
   {
      fTargetOffset = offset;
      BuildVectors();
   }
   static void SetRadialOffset(double offset)
   {
      fRadialOffset = offset;
      BuildVectors();
   }

   static double GetFaceDistance()
   {
      if(TestGlobalBit(ETigressGlobalBits::kArrayBackPos)) {
         return 145;
      }
      return 110;
   }

private:
#if !defined(__CINT__) && !defined(__CLING__)
   static std::function<bool(const TDetectorHit*, const TDetectorHit*)> fAddbackCriterion;
   static std::function<bool(const TDetectorHit*, const TDetectorHit*)> fSuppressionCriterion;
#endif

   static double fTargetOffset;   //!<!
   static double fRadialOffset;   //!<!

   // Vectors constructed from segment array and manual adjustments once at start of sort
   static std::array<std::array<std::array<std::array<TVector3, 9>, 4>, 17>, 2> fPositionVectors;   //!<!

   static std::array<TVector3, 17>                fCloverRadial;   //!<! direction vector of each HPGe Clover
   static std::array<std::array<TVector3, 2>, 17> fCloverCross;    //!<!  clover perpendicular vectors, for smearing

   // These array contain the original data that is used
   static std::array<std::array<std::array<double, 3>, 9>, 17> fGeBluePosition;        //!<!  detector segment XYZ
   static std::array<std::array<std::array<double, 3>, 9>, 17> fGeGreenPosition;       //!<!
   static std::array<std::array<std::array<double, 3>, 9>, 17> fGeRedPosition;         //!<!
   static std::array<std::array<std::array<double, 3>, 9>, 17> fGeWhitePosition;       //!<!
   static std::array<std::array<std::array<double, 3>, 9>, 17> fGeBluePositionBack;    //!<!  detector segment XYZ
   static std::array<std::array<std::array<double, 3>, 9>, 17> fGeGreenPositionBack;   //!<!
   static std::array<std::array<std::array<double, 3>, 9>, 17> fGeRedPositionBack;     //!<!
   static std::array<std::array<std::array<double, 3>, 9>, 17> fGeWhitePositionBack;   //!<!

   static TTransientBits<uint8_t>  fGlobalTigressBits;   //!<!
   mutable TTransientBits<uint8_t> fTigressBits;         // Transient member flags

   mutable std::vector<TDetectorHit*> fAddbackHits;    //!<! Used to create addback hits on the fly
   mutable std::vector<UShort_t>      fAddbackFrags;   //!<! Number of crystals involved in creating in the addback hit

   std::vector<TDetectorHit*> fSuppressedHits;   //!<!  The set of suppressed crystal hits

   mutable std::vector<TDetectorHit*> fSuppressedAddbackHits;    //!<! Used to create suppressed addback hits on the fly
   mutable std::vector<UShort_t>      fSuppressedAddbackFrags;   //!<! Number of crystals involved in creating in the suppressed addback hit

   // This is where the general untouchable functions live.
   void          ClearStatus() const { fTigressBits = 0; }   //!<!
   void          SetBitNumber(ETigressBits bit, Bool_t set) const;
   Bool_t        TestBitNumber(ETigressBits bit) const { return fTigressBits.TestBit(bit); }
   static void   SetGlobalBit(ETigressGlobalBits bit, Bool_t set = true) { fGlobalTigressBits.SetBit(bit, set); }
   static Bool_t TestGlobalBit(ETigressGlobalBits bit) { return (fGlobalTigressBits.TestBit(bit)); }

   void SetAddback(bool flag = true) const;
   void SetSuppressed(bool flag = true) const;
   void SetSuppressedAddback(bool flag = true) const;

   void SetCrossTalk(bool flag = true) const;

   static void BuildVectors();

public:
   void Copy(TObject&) const override;              //!<!
   void Clear(Option_t* opt = "all") override;      //!<!
   void Print(Option_t* opt = "") const override;   //!<!
   void Print(std::ostream& out) const override;    //!<!

   /// \cond CLASSIMP
   ClassDefOverride(TTigress, 7)   // Tigress Physics structure // NOLINT(readability-else-after-return)
   /// \endcond
};
/*! @} */
#endif
