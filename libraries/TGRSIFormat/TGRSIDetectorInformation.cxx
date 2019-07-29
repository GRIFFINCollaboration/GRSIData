#include "TGRSIDetectorInformation.h"

#include <iostream>
#include <unordered_map>

#include "TROOT.h"

#include "TGRSIOptions.h"
#include "TGRSIMnemonic.h"

/// \cond CLASSIMP
ClassImp(TGRSIDetectorInformation)
/// \endcond

TGRSIDetectorInformation::TGRSIDetectorInformation() : TDetectorInformation()
{
   /// Default ctor for TGRSIDetectorInformation.
   Clear();
	Set();
}

TGRSIDetectorInformation::~TGRSIDetectorInformation() = default;

TEventBuildingLoop::EBuildMode TGRSIDetectorInformation::BuildMode() const
{
	if(fSortByTriggerId) {
		return TEventBuildingLoop::EBuildMode::kTriggerId;
	}
	if(TGRSIOptions::Get()->AnalysisOptions()->BuildEventsByTimeStamp()) {
		return TEventBuildingLoop::EBuildMode::kTimestamp;
	}
	return TEventBuildingLoop::EBuildMode::kTime;
}

void TGRSIDetectorInformation::Print(Option_t* opt) const
{
   // Prints the TGRSIDetectorInformation. Options:
   // a: Print out more details.
   if(strchr(opt, 'a') != nullptr) {
      printf(DBLUE "\t\tDESCANT in ancillary positions = " DRED "%s" RESET_COLOR "\n",
             DescantAncillary() ? "TRUE" : "FALSE");
      printf("\n");
      printf("\t\tTIGRESS:            %s\n", Tigress() ? "true" : "false");
      printf("\t\tSHARC:              %s\n", Sharc() ? "true" : "false");
      printf("\t\tTRIFOIL:            %s\n", TriFoil() ? "true" : "false");
      printf("\t\tTIP:                %s\n", Tip() ? "true" : "false");
      printf("\t\tCSM:                %s\n", CSM() ? "true" : "false");
      printf("\t\tSPICE:              %s\n", Spice() ? "true" : "false");
      printf("\t\tGENERIC:            %s\n", Generic() ? "true" : "false");
      printf("\t\tS3:                 %s\n", S3() ? "true" : "false");
      printf("\t\tBAMBINO:            %s\n", Bambino() ? "true" : "false");
      printf("\t\tRF:                 %s\n", RF() ? "true" : "false");
      printf("\t\tGRIFFIN:            %s\n", Griffin() ? "true" : "false");
      printf("\t\tSCEPTAR:            %s\n", Sceptar() ? "true" : "false");
      printf("\t\tPACES:              %s\n", Paces() ? "true" : "false");
      printf("\t\tDESCANT:            %s\n", Descant() ? "true" : "false");
      printf("\t\tZDS:                %s\n", ZeroDegree() ? "true" : "false");
      printf("\t\tDANTE:              %s\n", Dante() ? "true" : "false");
      printf("\t\tBGO:                %s\n", Bgo() ? "true" : "false");
      printf("\t\tEMMA:                %s\n", Emma() ? "true" : "false");
      printf("\n");
   }
}

void TGRSIDetectorInformation::Clear(Option_t*)
{
   // Clears the TGRSIDetectorInformation. Currently, there are no available
   // options.

	TDetectorInformation::Clear();

   fTigress = false;
   fSharc   = false;
   fTriFoil = false;
   fRf      = false;
   fCSM     = false;
   fSpice   = false;
   fTip     = false;
   fS3      = false;
   fGeneric = false;
   fBambino = false;
   fEmma        = false;

   fGriffin    = false;
   fSceptar    = false;
   fPaces      = false;
   fDante      = false;
   fZeroDegree = false;
   fDescant    = false;
   fBgo        = false;

   fDescantAncillary = false;
}

void TGRSIDetectorInformation::Set()
{
   /// Sets the run info. This figures out what systems are available.
   std::unordered_map<unsigned int, TChannel*>::iterator iter;
   for(iter = TChannel::GetChannelMap()->begin(); iter != TChannel::GetChannelMap()->end(); iter++) {
      std::string channelname = iter->second->GetName();

		// check if we have an old TIG digitizer, in that case sort by trigger ID (instead of time stamp)
		// don't need to check again if we already found one, so include check for fSortByTriggerId being true
		if(!fSortByTriggerId && (iter->second->GetDigitizerType() == EDigitizer::kTIG10 || iter->second->GetDigitizerType() == EDigitizer::kTIG64)) {
			fSortByTriggerId = true;
		}
      //  detector system type.
      //  for more info, see: https://www.triumf.info/wiki/tigwiki/index.php/Detector_Nomenclature
      switch(static_cast<const TGRSIMnemonic*>(iter->second->GetMnemonic())->System()) {
			case TGRSIMnemonic::ESystem::kTigress:
				SetTigress();
				break;
			case TGRSIMnemonic::ESystem::kSharc:
				SetSharc();
				break;
			case TGRSIMnemonic::ESystem::kTriFoil:
				SetTriFoil();
				break;
			case TGRSIMnemonic::ESystem::kRF:
				SetRF();
				break;
			case TGRSIMnemonic::ESystem::kCSM:
				SetCSM();
				break;
			case TGRSIMnemonic::ESystem::kTip:
				SetTip();
				break;
			case TGRSIMnemonic::ESystem::kGriffin:
				SetGriffin();
				break;
			case TGRSIMnemonic::ESystem::kSceptar:
				SetSceptar();
				break;
			case TGRSIMnemonic::ESystem::kPaces:
				SetPaces();
				break;
			case TGRSIMnemonic::ESystem::kLaBr:
				SetDante();
				break;
			case TGRSIMnemonic::ESystem::kZeroDegree:
				SetZeroDegree();
				break;
			case TGRSIMnemonic::ESystem::kDescant:
				SetDescant();
				break;
			case TGRSIMnemonic::ESystem::kBgo:
				SetBgo();
				break;
			case TGRSIMnemonic::ESystem::kEmma:
				SetEmma();
				break;
			case TGRSIMnemonic::ESystem::kGeneric:
				SetGeneric();
				break;
			default:
				std::string system = iter->second->GetMnemonic()->SystemString();
				if(system.compare("SP") == 0) {
					SetSpice();
					SetS3();
				} else if(system.compare("BA") == 0) {
					SetBambino();
				}
		};
	}
}

