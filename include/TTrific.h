#ifndef TRIFIC_H
#define TRIFIC_H

/** \addtogroup Detectors
 *  @{
 */

#include <vector>
#include <cstdio>

#include "TBits.h"
#include "TVector3.h"

#include "Globals.h"
#include "TDetector.h"
#include "TTrificHit.h"

////////////////////////////////////////////////////////////
///
/// \class TTrific
///
/// The TTrific class defines the observables and algorithms used
/// when analyzing TIGRESS data.
///
////////////////////////////////////////////////////////////

class TTrificHit;

class TTrific : public TDetector {

	public:

		enum class ETrificBits {
			kPositionCalculated = BIT(0),
			kRangeCalculated = BIT(1),
			kBit2 = BIT(2),
			kBit3 = BIT(3),
			kBit4 = BIT(4),
			kBit5 = BIT(5),
			kBit6 = BIT(6),
			kBit7 = BIT(7),
		};

		TTrific();
		TTrific(const TTrific&);
		~TTrific() override;


	public:
		TTrificHit* GetTrificHit(const int& i) const {return static_cast<TTrificHit*>(GetHit(i));}
		TTrificHit* GetTrificXHit(const int& i) const {return fXFragments.at(i);}
		TTrificHit* GetTrificYHit(const int& i) const {return fYFragments.at(i);}
		TTrificHit* GetTrificSingHit(const int& i) const {return fSingFragments.at(i);}

#ifndef __CINT__
		void AddFragment(const std::shared_ptr<const TFragment>&, TChannel* chan) override;
#endif
		void BuildHits() override {} // no need to build any hits, everything already done in AddFragment

		void ClearTransients() override
   		{
      		fTrificBits = 0;
			TDetector::ClearTransients();
			fParticle.SetXYZ(0,0,0);
			fRange = 0;
   		}

		virtual Short_t GetMultiplicity() const override {return fHits.size();}
		virtual Short_t GetXMultiplicity() const {return fXFragments.size();}
		virtual Short_t GetYMultiplicity() const {return fYFragments.size();}
		virtual Short_t GetSingMultiplicity() const {return fSingFragments.size();}

		//used to set the target to window distance depending on which experiment is running
		static void SetSharc(Double_t distance = 650.57) {fTargetToWindowCart = distance;} //mm. 640.2mm from ideal target location to start of window housing, then 20.74mm window housing thickness
		static void SetTip(Double_t distance = 600.) {fTargetToWindowCart = distance;} //mm. THIS IS ONLY A *ROUGH* APPROXIMATION
		static void SetCustomTargetChamber(Double_t distance){fTargetToWindowCart = distance;}


		TVector3 fParticle = TVector3(0,0,0); //!<!

		Int_t fRange = 0; //!<!

		//TVector3 GetPosition(const TTrificHit& hit,Int_t detectorNumber); //!<!
		TVector3 GetPosition(Int_t detectorNumber); //!<!

		TVector3 GetPosition(); //!<!

		Int_t GetRange(); //!<!

		//TVector2 GetEdESimple(); //!<!
		
		void GetXYGrid(); //!<!
		
		TTrific& operator=(const TTrific&); //!<!

	private:
		static bool fSetCoreWave; //!<!  Flag for Waveforms ON/OFF
		
		std::vector<TTrificHit*> fXFragments; 
		std::vector<TTrificHit*> fYFragments; 
		std::vector<TTrificHit*> fSingFragments; 

		TTransientBits<UShort_t> fTrificBits;

	public:
		static bool SetCoreWave() { return fSetCoreWave; } //!<!

		void Copy(TObject&) const override;            //!<!
		void Print(Option_t* opt = "") const override; //!<!
		void Clear(Option_t* = "") override; //!<!
		
		static const double fSpacingCart; //!
		static const double fInitialSpacingCart; //!
		static double fTargetToWindowCart; //!

	private:
		//physical information on the grids

		static const double fXmm[12]; //!
		//static double xW[12]; //!
		static const double fYmm[12]; //!
		//static double yW[12]; //!
		
		//for use in determining the XY grids
		static Int_t fGridX; //!
		static Int_t fGridY; //!
		static const double fAngle; //!
		static const TVector3 fNormalGridVec; //!

		/// \cond CLASSIMP
		ClassDefOverride(TTrific, 4) // TRIFIC Physics structure
			/// \endcond
};
/*! @} */
#endif
