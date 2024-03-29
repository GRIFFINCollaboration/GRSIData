#ifndef TTIGRESS_H
#define TTIGRESS_H

/** \addtogroup Detectors
 *  @{
 */

#include <utility>
#include <vector>
#include <iostream>
#include <set>
#include <cstdio>
#include <functional>

#include "TMath.h"
#include "TVector3.h"
#include "TObject.h"
#include "TClonesArray.h"
#include "TRandom2.h"

#include "TDetector.h"
#include "TTigressHit.h"
#include "TBgoHit.h"

class TTigress : public TDetector {
public:
   enum class ETigressBits {
      kAddbackSet  = BIT(0),
      kSuppression = BIT(1),
      kBit2        = BIT(2),
      kBit3        = BIT(3),
      kBit4        = BIT(4),
      kBit5        = BIT(5),
      kBit6        = BIT(6),
      kBit7        = BIT(7)
   };

   enum class ETigressGlobalBits {
      kSetBGOWave   = BIT(0),
      kSetCoreWave  = BIT(1),
      kSetSegWave   = BIT(2),
      kSetBGOHits   = BIT(3),
      kForceCrystal = BIT(4),
      kArrayBackPos = BIT(5),
      kVectorsBuilt = BIT(6) // 110 or 145
   };

#ifndef __CINT__
   std::vector<std::vector<std::shared_ptr<const TFragment>>> SegmentFragments;
#endif

   TTigress();
   TTigress(const TTigress&);
   ~TTigress() override;

   // Dont know why these were changes to return by reference rather than pointer
   // The tigress group prefer them the old way
   TTigressHit* GetTigressHit(const int& i) const { return static_cast<TTigressHit*>(GetHit(i)); }
   static TVector3         GetPosition(int DetNbr, int CryNbr, int SegNbr, double dist = 0., bool smear = false); //!<!
   static TVector3         GetPosition(const TTigressHit&, double dist = 0., bool smear = false);                 //!<!

   std::vector<TBgoHit> fBgos;
   void AddBGO(TBgoHit& bgo) { fBgos.push_back(bgo); }                      //!<!
   Short_t                  GetBGOMultiplicity() const { return fBgos.size(); } //!<!
   int                  GetNBGOs() const { return fBgos.size(); }           //!<!
   TBgoHit GetBGO(int& i) const { return fBgos.at(i); }                     //!<!
   TBgoHit& GetBGO(int& i) { return fBgos.at(i); }                          //!<!

   Int_t        GetAddbackMultiplicity();
   TTigressHit* GetAddbackHit(const int&);
   void         ResetAddback(); //!<!
   UShort_t GetNAddbackFrags(size_t idx) const;

#ifndef __CINT__
   void AddFragment(const std::shared_ptr<const TFragment>&, TChannel*) override; //!<!
#endif
   void BuildHits() override;

   void ClearTransients() override
   {
      fTigressBits = 0;
		TDetector::ClearTransients();
   }

   TTigress& operator=(const TTigress&); //!<!

#if !defined(__CINT__) && !defined(__CLING__)
   void SetAddbackCriterion(std::function<bool(TDetectorHit*, TDetectorHit*)> criterion)
   {
      fAddbackCriterion = std::move(criterion);
   }

   std::function<bool(TDetectorHit*, TDetectorHit*)> GetAddbackCriterion() const { return fAddbackCriterion; }
   void SetSuppressionCriterion(std::function<bool(TDetectorHit*, TBgoHit&)> criterion)
   {
      fSuppressionCriterion = std::move(criterion);
   }
   std::function<bool(TDetectorHit*, TBgoHit&)> GetSuppressionCriterion() const { return fSuppressionCriterion; }
#endif

private:
#if !defined(__CINT__) && !defined(__CLING__)
   static std::function<bool(TDetectorHit*, TDetectorHit*)> fAddbackCriterion;
   static std::function<bool(TDetectorHit*, TBgoHit&)>      fSuppressionCriterion;
#endif
   static TTransientBits<UShort_t> fgTigressBits; //!
   TTransientBits<UShort_t>        fTigressBits;

   static double fTargetOffset; //!<!
   static double fRadialOffset; //!<!
   
   // Vectors constructed from segment array and manual adjustments once at start of sort
   static TVector3 fPositionVectors[2][17][4][9];     //!<!
   
   static TVector3 fCloverRadial[17];      	//!<!  clover direction vectors
   static TVector3 fCloverCross[17][2];      	//!<!  clover perpendicular vectors, for smearing

   // These array contain the original data that is used
   static double GeBluePosition[17][9][3];      //!<!  detector segment XYZ
   static double GeGreenPosition[17][9][3];     //!<!
   static double GeRedPosition[17][9][3];       //!<!
   static double GeWhitePosition[17][9][3];     //!<!
   static double GeBluePositionBack[17][9][3];  //!<!  detector segment XYZ
   static double GeGreenPositionBack[17][9][3]; //!<!
   static double GeRedPositionBack[17][9][3];   //!<!
   static double GeWhitePositionBack[17][9][3]; //!<!

   //    void ClearStatus();                      // WARNING: this will change the building behavior!
   //		void ClearGlobalStatus() { fTigressBits = 0; }
   static void SetGlobalBit(ETigressGlobalBits bit, Bool_t set = true) { fgTigressBits.SetBit(bit, set); }
   static Bool_t TestGlobalBit(ETigressGlobalBits bit) { return (fgTigressBits.TestBit(bit)); }

   std::vector<TDetectorHit*> fAddbackHits;  //!<! Used to create addback hits on the fly
   std::vector<UShort_t>      fAddbackFrags; //!<! Number of crystals involved in creating in the addback hit

   static void BuildVectors();  //!<!
   
public:
   // Naming convention was off, couldnt find anything that used them in grsisort
   // Left them as return bool to not break external code
   static bool SetCoreWave(bool set = true)
   {
      SetGlobalBit(ETigressGlobalBits::kSetCoreWave, set);
      return set;
   } //!<!
   static bool SetSegmentWave(bool set = true)
   {
      SetGlobalBit(ETigressGlobalBits::kSetSegWave, set);
      return set;
   } //!<!
   static bool SetBGOWave(bool set = true)
   {
      SetGlobalBit(ETigressGlobalBits::kSetBGOWave, set);
      return set;
   } //!<!
   static bool SetForceCrystal(bool set = true)
   {
      SetGlobalBit(ETigressGlobalBits::kForceCrystal, set);
      return set;
   } //!<!
   static bool SetArrayBackPos(bool set = true)
   {
      SetGlobalBit(ETigressGlobalBits::kArrayBackPos, set);
      BuildVectors();
      return set;
   } //!<!

   static bool GetCoreWave() { return TestGlobalBit(ETigressGlobalBits::kSetCoreWave); }      //!<!
   static bool GetSegmentWave() { return TestGlobalBit(ETigressGlobalBits::kSetSegWave); }    //!<!
   static bool GetBGOWave() { return TestGlobalBit(ETigressGlobalBits::kSetBGOWave); }        //!<!
   static bool GetForceCrystal() { return TestGlobalBit(ETigressGlobalBits::kForceCrystal); } //!<!
   static bool GetArrayBackPos() { return TestGlobalBit(ETigressGlobalBits::kArrayBackPos); } //!<!
   static bool GetVectorsBuilt() { return TestGlobalBit(ETigressGlobalBits::kVectorsBuilt); } //!<!

   static bool BGOSuppression[4][4][5]; //!<!

   static void SetTargetOffset(double offset) {
	   fTargetOffset = offset; 
	   BuildVectors();
   } //!<!
   static void SetRadialOffset(double offset) { 
	   fRadialOffset = offset;
	   BuildVectors();
   } //!<!

   static double GetFaceDistance()
   {
      if(GetArrayBackPos()) {
         return 145;
      }
      return 110;
   }

public:
   void Clear(Option_t* opt = "") override;       //!<!
   void Print(Option_t* opt = "") const override; //!<!
	void Print(std::ostream& out) const override; //!<!
   void Copy(TObject&) const override;            //!<!

   /// \cond CLASSIMP
   ClassDefOverride(TTigress, 7) // Tigress Physics structure
   /// \endcond
};

std::underlying_type<TTigress::ETigressGlobalBits>::type operator |(TTigress::ETigressGlobalBits lhs, TTigress::ETigressGlobalBits rhs);

/*! @} */
#endif
