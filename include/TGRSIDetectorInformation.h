#ifndef TGRSIDETECTORINFORMATION_H
#define TGRSIDETECTORINFORMATION_H

/** \addtogroup Sorting
 *  @{
 */

/////////////////////////////////////////////////////////////////
///
/// \class TGRSIDetectorInformation
///
/// This Class is designed to store setup dependent
/// information. It is used to store existence of
/// detector systems, etc.
///
/////////////////////////////////////////////////////////////////

#include "Globals.h"

#include "TDetectorInformation.h"
#include "TChannel.h"

class TGRSIDetectorInformation : public TDetectorInformation {
public:
   TGRSIDetectorInformation();
   ~TGRSIDetectorInformation() override;
	
	TEventBuildingLoop::EBuildMode BuildMode() const override;

	void Print(Option_t* opt = "") const override;
	void Clear(Option_t* = "") override;
	void Set() override;

	inline void SetDescantAncillary(bool flag = true) { fDescantAncillary = flag; }
	inline bool DescantAncillary() const { return fDescantAncillary; }

   inline void SetTigress(bool flag = true)    { fTigress = flag; }
   inline void SetSharc(bool flag = true)      { fSharc = flag; }
   inline void SetTriFoil(bool flag = true)    { fTriFoil = flag; }
   inline void SetRF(bool flag = true)         { fRf = flag; }
   inline void SetCSM(bool flag = true)        { fCSM = flag; }
   inline void SetSpice(bool flag = true)      { fSpice = flag; }
   inline void SetS3(bool flag = true)         { fS3 = flag; }
   inline void SetGeneric(bool flag = true)    { fGeneric = flag; }
   inline void SetTip(bool flag = true)        { fTip = flag; }
   inline void SetBambino(bool flag = true)    { fBambino = flag; }

   inline void SetGriffin(bool flag = true)    { fGriffin = flag; }
   inline void SetSceptar(bool flag = true)    { fSceptar = flag; }
   inline void SetPaces(bool flag = true)      { fPaces = flag; }
   inline void SetDante(bool flag = true)      { fDante = flag; }
   inline void SetZeroDegree(bool flag = true) { fZeroDegree = flag; }
   inline void SetDescant(bool flag = true)    { fDescant = flag; }

   inline void SetBgo(bool flag = true)        { fBgo = flag; }

   inline bool Tigress()    const { return fTigress; }
   inline bool Sharc()      const { return fSharc; }
   inline bool TriFoil()    const { return fTriFoil; }
   inline bool RF()         const { return fRf; }
   inline bool CSM()        const { return fCSM; }
   inline bool Spice()      const { return fSpice; }
   inline bool Bambino()    const { return fBambino; }
   inline bool Tip()        const { return fTip; }
   inline bool S3()         const { return fS3; }
   inline bool Generic()    const { return fGeneric; }

   inline bool Griffin()    const { return fGriffin; }
   inline bool Sceptar()    const { return fSceptar; }
   inline bool Paces()      const { return fPaces; }
   inline bool Dante()      const { return fDante; }
   inline bool ZeroDegree() const { return fZeroDegree; }
   inline bool Descant()    const { return fDescant; }

   inline bool Bgo()        const { return fBgo; }

private:
   //  detector types to switch over in Set()
   //  for more info, see: https://www.triumf.info/wiki/tigwiki/index.php/Detector_Nomenclature

	bool fDescantAncillary{false};  // Descant is in the ancillary detector locations

   bool fTigress{false}; // flag for Tigress on/off
   bool fSharc{false};   // flag for Sharc on/off
   bool fTriFoil{false}; // flag for TriFoil on/off
   bool fRf{false};      // flag for RF on/off
   bool fCSM{false};     // flag for CSM on/off
   bool fSpice{false};   // flag for Spice on/off
   bool fTip{false};     // flag for Tip on/off
   bool fS3{false};      // flag for S3 on/off
   bool fGeneric{false}; // flag for Generic on/off
   bool fBambino{false}; // flag for Bambino on/off

   bool fGriffin{false};    // flag for Griffin on/off
   bool fSceptar{false};    // flag for Sceptar on/off
   bool fPaces{false};      // flag for Paces on/off
   bool fDante{false};      // flag for LaBr on/off
   bool fZeroDegree{false}; // flag for Zero Degree Scintillator on/off
   bool fDescant{false};    // flag for Descant on/off

   bool fBgo{false};        // flag for Bgo on/off

   /// \cond CLASSIMP
   ClassDefOverride(TGRSIDetectorInformation, 1); // Contains the run-dependent information.
   /// \endcond
};
/*! @} */
#endif
