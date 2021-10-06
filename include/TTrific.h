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
		//TTrificHit* GetTrificHit(const int& i) const { return static_cast<TTrificHit*>(GetHit(i)); } //current does nothing because we have moved all fragments into an X, Y, or Sing fragment
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
			particle.SetXYZ(0,0,0);
			range = 0;
   		}

		//since we've eliminated fHits from TTrific, we need to redefine GetMultiplicity()
		virtual Short_t GetMultiplicity() const override {return fXFragments.size()+fYFragments.size()+fSingFragments.size();} //Note: you cannot iterate over just the multiplicity, since there is no one vector will all hits in it
		virtual Short_t GetXMultiplicity() const {return fXFragments.size();}
		virtual Short_t GetYMultiplicity() const {return fYFragments.size();}
		virtual Short_t GetSingMultiplicity() const {return fSingFragments.size();}

		static TVector3 particle; //!<!

		static Int_t range; //!<!

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
		
		static const double spacingCart; //!
		static const double initialSpacingCart; //!
		static double targetToWindowCart; //!

	private:
		//physical information on the grids

		static const double xmm[12]; //!
		//static double xW[12]; //!
		static const double ymm[12]; //!
		//static double yW[12]; //!
		
		//for use in determining the XY grids
		static Int_t gridX; //!
		static Int_t gridY; //!
		static const double angle; //!
		static const TVector3 normalGridVec; //!

		/// \cond CLASSIMP
		ClassDefOverride(TTrific, 4) // TRIFIC Physics structure
			/// \endcond
};
/*! @} */
#endif
