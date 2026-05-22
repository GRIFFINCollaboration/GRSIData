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

#include "TROOT.h"
#include "RtypesCore.h"

#include "Globals.h"

#include "TDetectorInformation.h"
#include "TChannel.h"

class TGRSIDetectorInformation : public TDetectorInformation {
public:
   TGRSIDetectorInformation();
   TGRSIDetectorInformation(const TGRSIDetectorInformation&)                = default;
   TGRSIDetectorInformation(TGRSIDetectorInformation&&) noexcept            = default;
   TGRSIDetectorInformation& operator=(const TGRSIDetectorInformation&)     = default;
   TGRSIDetectorInformation& operator=(TGRSIDetectorInformation&&) noexcept = default;
   ~TGRSIDetectorInformation() override                                     = default;

   TEventBuildingLoop::EBuildMode BuildMode() const override;

   void Print(Option_t* opt = "") const override;
   void Clear(Option_t* = "") override;
   void Set() override;

   void SetDescantAncillary(bool flag = true) { fDescantAncillary = flag; }
   bool DescantAncillary() const { return fDescantAncillary; }
   void SetDescantWall(bool flag = true) { fDescantWall = flag; }
   bool DescantWall() const { return fDescantWall; }

   void SetTigress(bool flag = true) { fTigress = flag; }
   void SetSharc(bool flag = true) { fSharc = flag; }
   void SetTriFoil(bool flag = true) { fTriFoil = flag; }
   void SetRF(bool flag = true) { fRf = flag; }
   void SetCSM(bool flag = true) { fCSM = flag; }
   void SetSpice(bool flag = true) { fSpice = flag; }
   void SetS3(bool flag = true) { fS3 = flag; }
   void SetGeneric(bool flag = true) { fGeneric = flag; }
   void SetTip(bool flag = true) { fTip = flag; }
   void SetBambino(bool flag = true) { fBambino = flag; }
   void SetEmma(bool flag = true) { fEmma = flag; }
   void SetTrific(bool flag = true) { fTrific = flag; }
   void SetSharc2(bool flag = true) { fSharc2 = flag; }

   void SetGriffin(bool flag = true) { fGriffin = flag; }
   void SetSceptar(bool flag = true) { fSceptar = flag; }
   void SetPaces(bool flag = true) { fPaces = flag; }
   void SetDante(bool flag = true) { fDante = flag; }
   void SetZeroDegree(bool flag = true) { fZeroDegree = flag; }
   void SetDescant(bool flag = true) { fDescant = flag; }
   void SetRcmp(bool flag = true) { fRcmp = flag; }
   void SetAries(bool flag = true) { fAries = flag; }
   void SetDemand(bool flag = true) { fDemand = flag; }

   void SetBgo(bool flag = true) { fBgo = flag; }

   bool Tigress() const { return fTigress; }
   bool Sharc() const { return fSharc; }
   bool TriFoil() const { return fTriFoil; }
   bool RF() const { return fRf; }
   bool CSM() const { return fCSM; }
   bool Spice() const { return fSpice; }
   bool Bambino() const { return fBambino; }
   bool Tip() const { return fTip; }
   bool S3() const { return fS3; }
   bool Emma() const { return fEmma; }
   bool Trific() const { return fTrific; }
   bool Sharc2() const { return fSharc2; }
   bool Generic() const { return fGeneric; }

   bool Griffin() const { return fGriffin; }
   bool Sceptar() const { return fSceptar; }
   bool Paces() const { return fPaces; }
   bool Dante() const { return fDante; }
   bool ZeroDegree() const { return fZeroDegree; }
   bool Descant() const { return fDescant; }
   bool Rcmp() const { return fRcmp; }
   bool Aries() const { return fAries; }
   bool Demand() const { return fDemand; }

   bool Bgo() const { return fBgo; }

private:
   //  detector types to switch over in Set()
   //  for more info, see: https://www.triumf.info/wiki/tigwiki/index.php/Detector_Nomenclature

   bool fDescantAncillary{false};   ///< Descant is in the ancillary detector locations
   bool fDescantWall{false};        ///< Descant is in the wall detector locations

   bool fTigress{false};   ///< flag for Tigress on/off
   bool fSharc{false};     ///< flag for Sharc on/off
   bool fTriFoil{false};   ///< flag for TriFoil on/off
   bool fRf{false};        ///< flag for RF on/off
   bool fCSM{false};       ///< flag for CSM on/off
   bool fSpice{false};     ///< flag for Spice on/off
   bool fTip{false};       ///< flag for Tip on/off
   bool fS3{false};        ///< flag for S3 on/off
   bool fGeneric{false};   ///< flag for Generic on/off
   bool fBambino{false};   ///< flag for Bambino on/off
   bool fEmma{false};      ///< flag for Emma on/off
   bool fTrific{false};    ///< flag for TRIFIC on/off
   bool fSharc2{false};    ///< flag for SHARC 2 on/off

   bool fGriffin{false};      ///< flag for Griffin on/off
   bool fSceptar{false};      ///< flag for Sceptar on/off
   bool fPaces{false};        ///< flag for Paces on/off
   bool fDante{false};        ///< flag for LaBr on/off
   bool fZeroDegree{false};   ///< flag for Zero Degree Scintillator on/off
   bool fDescant{false};      ///< flag for Descant on/off
   bool fRcmp{false};         ///< flag for Rcmp on/off
   bool fAries{false};        ///< flag for Aries on/off
   bool fDemand{false};       ///< flag for Demand on/off

   bool fBgo{false};   ///< flag for Bgo on/off

   bool fSortByTriggerId{false};   ///< flag to sort by trigger ID instead of time stamp

   /// \cond CLASSIMP
   ClassDefOverride(TGRSIDetectorInformation, 2);   // NOLINT(readability-else-after-return)
   /// \endcond
};
/*! @} */
#endif
