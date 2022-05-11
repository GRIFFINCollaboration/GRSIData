#ifndef TTIP_H
#define TTIP_H

/** \addtogroup Detectors
 *  @{
 */

/////////////////////////////////////////////////////////////
///
/// \class TTip
///
/// The TTip class defines the observables and algorithms used
/// when analyzing TIP data. It includes detector positions,
/// etc.
///
/////////////////////////////////////////////////////////////

#include "Globals.h"
#include "TDetector.h"
#include "TTipHit.h"

class TTip : public TDetector {
public:

   enum class ETipGlobalBits {
      kVectorsBuilt   = BIT(0)
   };

   ~TTip() override;
   TTip();
   TTip(const TTip& rhs);

   static TVector3 GetPosition(int DetNbr); //!<!
   static TVector3 GetPosition(const TTipHit&);

   TTipHit* GetTipHit(const int& i) const { return static_cast<TTipHit*>(GetHit(i)); }

#ifndef __CINT__
   void AddFragment(const std::shared_ptr<const TFragment>&, TChannel*) override; //!<!
#endif
	void BuildHits() override {} // no need to build any hits, everything already done in AddFragment

   TTip& operator=(const TTip&); //!<!

private:

   static TTransientBits<UShort_t> fgTipBits; //!

   // Position vectors for individual TIP detectors
   static TVector3 fPositionVectors[128];     //!<!

   static void SetGlobalBit(ETipGlobalBits bit, Bool_t set = true) { fgTipBits.SetBit(bit, set); }
   static Bool_t TestGlobalBit(ETipGlobalBits bit) { return (fgTipBits.TestBit(bit)); }

   static void BuildVectors();  //!<!

public:

   void Print(Option_t* opt = "") const override;
	void Print(std::ostream& out) const override; //!<!

   static bool GetVectorsBuilt() { return TestGlobalBit(ETipGlobalBits::kVectorsBuilt); } //!<!

   /// \cond CLASSIMP
   ClassDefOverride(TTip, 2);
   /// \endcond
};
/*! @} */
#endif
