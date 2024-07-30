/// \author Dhruval Shah <dsi491@uregina.ca>, University of Regina

#ifndef TRCMP_H
#define TRCMP_H

/** \addtogroup Detectors
 *  @{
 */

#include <vector>
#include <iostream>
#include <cstdlib>
#include <map>
#include <utility>
#include <set>

#include "TDetector.h"
#include "TChannel.h"
#include "TRcmpHit.h"
#include "TMath.h"
#include "TVector3.h"
#include "Globals.h"
#include "TFragment.h"
#include "TGRSIMnemonic.h"

////////////////////////////////////////////////////////////
///
/// \class TRcmp
///
/// The TRcmp class defines the observables and algorithms
/// used when analyzing RCMP data.
///
////////////////////////////////////////////////////////////

class TRcmp : public TDetector {
public:
   // these "bits" represent flags that will be used to clear the transient vectors
   // UChar_t (character) has 8 bits, and it is the smallest data type we can have
   // thus, the bits that are not used in the class will just be at their default value
   enum class ERcmpBits {
      kFrontSet  = BIT(0), // flag for the front multiplicity transient vector
      kBackSet   = BIT(1), // flag for the back  multiplicity transient vector
      kPixelSet  = BIT(2), // flag for the pixel multiplicity transient vector
      kBit3      = BIT(3),
      kBit4      = BIT(4),
      kBit5      = BIT(5),
      kBit6      = BIT(6),
      kBit7      = BIT(7)
   };

   // constructors and destructors
   TRcmp();               // default constructor
   TRcmp(const TRcmp&);   // copy constructor
   ~TRcmp() override;     // destructor

#ifndef __CINT__
   void AddFragment(const std::shared_ptr<const TFragment>&, TChannel*) override;   //!<! //adds front and back fragments to their respective fragment vectors
#endif
   void BuildHits() override {} // even though we are not using this function, we still need to override it to avoid warning during the sorting process

   // functions to access the individual fragments
   inline TFragment GetRcmpFrontFrag(const int& i) const { return fFrontPFragVector.at(i); } // returns the front fragment stored at the index
   inline TFragment GetRcmpBackFrag(const int& i)  const { return fBackNFragVector.at(i); }  // returns the back  fragment stored at the index

   // functions to access the individual hits
   inline TRcmpHit* GetRcmpFrontHit(const int& i) const { return fFrontPMulVector.at(i); }   // returns a front strip hit in RCMP (this hit can be used to fill histograms when looping over front multiplicity)
   inline TRcmpHit* GetRcmpBackHit(const int& i)  const { return fBackNMulVector.at(i); }    // returns a back strip hit in RCMP  (this hit can be used to fill histograms when looping over back  multiplicity)
   inline TRcmpHit* GetRcmpHit(const int& i)      const { return fPixelMulVector.at(i); }    // returns a pixel hit in RCMP       (this hit can be used to fill histograms when looping over pixel multiplicity)

   TRcmp& operator=(const TRcmp&);
   void   Copy(TObject&) const override;              //!<!
   void   Clear(Option_t* opt = "all") override;      //!<!
   void   Print(Option_t* opt = "") const override;   //!<!
   void   Print(std::ostream& out) const override;    //!<!

   // multiplicity functions
   size_t  GetFrontFragmentMultiplicity() const { return fFrontPFragVector.size(); }  // returns the size of the vector that contains all the front fragments (within a build window)
   size_t  GetBackFragmentMultiplicity()  const { return fBackNFragVector.size(); }   // returns the size of the vector that contains all the back  fragments (within a build window)
   Short_t GetFrontMultiplicity();                                                    // returns the size of the vector that contains all the front hits (within a build window)
   Short_t GetBackMultiplicity();                                                     // returns the size of the vector that contains all the back  hits (within a build window)
   Short_t GetPixelMultiplicity();                                                    // returns the size of the vector that contains all the pixel hits (within a build window)

   inline Int_t GetZeroMultiplicity() const { return fZeroMultiplicity; }  // returns the zero multiplicty counter

   // note that the coincidence time condition is not implemented in the pixel multiplicity function as of now
   // it can be set in the GRSIFrame helper if needed
   inline Int_t GetCoincidenceTime() const { return fCoincidenceTime; }                     // returns the coincidence time between front and back strip of a detector in nanoseconds
   inline void  SetCoincidenceTime(const int coincTime) { fCoincidenceTime = coincTime; }   // sets the coincidence time between front and back strip of a detector in nanoseconds

private:
   // vectors that contain front and back fragments
   // note: P-junction (front) and N-ohmic (back)
   std::vector<TFragment> fFrontPFragVector;   ///< stores all front strip fragments
   std::vector<TFragment> fBackNFragVector;    ///< stores all back  strip fragments

   // vectors that contain front, back, and pixel hits
   std::vector<TRcmpHit*> fFrontPMulVector;      //!<! transient vector that stores hits for the front strips (i.e., front multiplicity)
   std::vector<TRcmpHit*> fBackNMulVector;       //!<! transient vector that stores hits for the back  strips (i.e., back  multiplicity)
   std::vector<TRcmpHit*> fPixelMulVector;       //!<! transient vector that stores hits for front+back       (i.e., pixel multiplicity)

   TTransientBits<UChar_t> fRcmpBits;                                              ///< flags for transient members
   void                    ClearStatus() { fRcmpBits = 0; }
   void SetBitNumber(ERcmpBits bit, Bool_t set = true);                            ///< second argument shows that set is true by default
   Bool_t TestBitNumber(ERcmpBits bit) const { return (fRcmpBits.TestBit(bit)); }  ///< returns true if the bit is set

   Int_t fZeroMultiplicity = 0;   ///< keep track of zero multiplicty with this counter (it is incremented every time the Pixel Multiplicity loops are entered, but the hits are not built)

   // this would be the default coincidence time unless changed using the SetCoincidenceTime function
   Int_t fCoincidenceTime = 50;   ///< GetTime for TDetectorHit is in nanoseconds, so this is in nanoseconds, too!

   /// \cond CLASSIMP
   ClassDefOverride(TRcmp, 2)
   /// \endcond
};

/*! @} */
#endif
