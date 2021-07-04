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
		void AddFragment(const std::shared_ptr<const TFragment>&, TChannel*) override;
#endif
		void BuildHits() override {} // no need to build any hits, everything already done in AddFragment

		Int_t GetXYGrid(char grid, const TTrific& event); //!<!

		static TVector3 GetPosition(Int_t DetectorNumber); //!<!

		TVector3 GetPosition(); //!<!


		Int_t GetRange(); //!<!


		TTrific& operator=(const TTrific&); //!<!

	private:
		static bool fSetCoreWave; //!<!  Flag for Waveforms ON/OFF

		//static double xmm; //!<!
		//static double xW; //!<!
		//static double ymm; //!<!
		//static double yW; //!<!
		//static double spacing; //!<!
		//static double initialSpacing; //!<!
		//static Int_t gridX; //!<!
		//static Int_t gridY; //!<!


	public:
		static bool SetCoreWave() { return fSetCoreWave; } //!<!

		void Copy(TObject&) const override;            //!<!
		void Print(Option_t* opt = "") const override; //!<!

		/// \cond CLASSIMP
		ClassDefOverride(TTrific, 4) // TRIFIC Physics structure
			/// \endcond
};
/*! @} */
#endif
