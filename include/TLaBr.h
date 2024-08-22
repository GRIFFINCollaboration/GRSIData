#ifndef TLABR_H
#define TLABR_H

/** \addtogroup Detectors
 *  @{
 */

/////////////////////////////////////////////////////////////
///
/// \class TLaBr
///
/// The TLaBr class defines the observables and algorithms used
/// when analyzing LaBr data. It includes detector positions,
/// etc.
///
/////////////////////////////////////////////////////////////

#include <vector>
#include <cstdio>

#include "TVector3.h"

#include "Globals.h"
#include "TSuppressed.h"
#include "TTransientBits.h"
#include "TLaBrHit.h"

class TLaBr : public TSuppressed {
public:
   enum class ELaBrBits {
      kIsSuppressed = 1<<0,
      kBit1         = 1<<1,
      kBit2         = 1<<2,
      kBit3         = 1<<3,
      kBit4         = 1<<4,
      kBit5         = 1<<5,
      kBit6         = 1<<6,
      kBit7         = 1<<7
   };

   TLaBr();
   ~TLaBr() override;
   TLaBr(const TLaBr& rhs);

   void Copy(TObject& rhs) const override;
   TLaBrHit* GetLaBrHit(const int& i) const { return static_cast<TLaBrHit*>(GetHit(i)); }

#if !defined(__CINT__) && !defined(__CLING__)
   void SetSuppressionCriterion(std::function<bool(const TDetectorHit*, const TDetectorHit*)> criterion)
   {
      fSuppressionCriterion = std::move(criterion);
   }
   std::function<bool(const TDetectorHit*, const TDetectorHit*)> GetSuppressionCriterion() const { return fSuppressionCriterion; }

   bool SuppressionCriterion(const TDetectorHit* hit, const TDetectorHit* bgoHit) override { return fSuppressionCriterion(hit, bgoHit); }
#endif

   TLaBrHit* GetSuppressedHit(const int& i);                          //!<!
   Short_t GetSuppressedMultiplicity(const TBgo* fBgo);
   bool IsSuppressed() const;
	void SetSuppressed(const bool flag);
   void ResetSuppressed();

#if !defined(__CINT__) && !defined(__CLING__)
   void AddFragment(const std::shared_ptr<const TFragment>&, TChannel*) override; //!<!
#endif
	void BuildHits() override {} // no need to build any hits, everything already done in AddFragment

   static TVector3 GetPosition(int DetNbr) { return fPosition[DetNbr]; } //!<!

   TLaBr& operator=(const TLaBr&); //!<!

private:
#if !defined(__CINT__) && !defined(__CLING__)
   static std::function<bool(const TDetectorHit*, const TDetectorHit*)> fSuppressionCriterion;
#endif
   std::vector<TDetectorHit*> fSuppressedHits; //   The set of suppressed LaBr hits

   static std::array<TVector3, 9> fPosition; //!<!  Position of each detectir

   mutable TTransientBits<UChar_t> fLaBrBits;  // Transient member flags

   void ClearStatus() const { fLaBrBits = 0; } //!<!
   void SetBitNumber(const ELaBrBits bit, const bool set) const { fLaBrBits.SetBit(bit, set); }
   Bool_t TestBitNumber(const ELaBrBits bit) const { return fLaBrBits.TestBit(bit); }

public:
   void Clear(Option_t* opt = "") override;       //!<!
   void Print(Option_t* opt = "") const override; //!<!
	void Print(std::ostream& out) const override; //!<!

   /// \cond CLASSIMP
   ClassDefOverride(TLaBr, 1) // LaBr Physics structure
	/// \endcond
};
/*! @} */
#endif
