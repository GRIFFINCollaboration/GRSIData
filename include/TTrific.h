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

class TTrific : public TDetector {

	public:
		TTrific();
		TTrific(const TTrific&);
		~TTrific() override;

	public:
		TTrificHit* GetTrificHit(const int& i) const { return static_cast<TTrificHit*>(GetHit(i)); }

#ifndef __CINT__
		void AddFragment(const std::shared_ptr<const TFragment>&, TChannel* chan) override;
#endif
		void BuildHits() override {} // no need to build any hits, everything already done in AddFragment


		//TVector3 GetPosition(const TTrificHit& hit,Int_t detectorNumber); //!<!
		TVector3 GetPosition(Int_t detectorNumber); //!<!

		TVector3 GetPosition(); //!<!


		Int_t GetRange(); //!<!
		
		void GetXYGrid(); //!<!


		TTrific& operator=(const TTrific&); //!<!

	private:
		static bool fSetCoreWave; //!<!  Flag for Waveforms ON/OFF
		
		std::vector<TDetectorHit*> fXFragments; 
		std::vector<TDetectorHit*> fYFragments; 
		std::vector<TDetectorHit*> fSingFragments; 

	public:
		static bool SetCoreWave() { return fSetCoreWave; } //!<!

		void Copy(TObject&) const override;            //!<!
		void Print(Option_t* opt = "") const override; //!<!
		void Clear(Option_t* = "") override; //!<!

	private:
		//physical information on the grids

		static const double xmm[12]; //!
		//static double xW[12]; //!
		static const double ymm[12]; //!
		//static double yW[12]; //!
		static const double spacingCart; //!
		static const double initialSpacingCart; //!
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
