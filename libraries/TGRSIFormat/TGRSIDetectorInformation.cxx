#include "TGRSIDetectorInformation.h"

#include <iostream>
#include <unordered_map>

#include "TROOT.h"

#include "TGRSIOptions.h"
#include "TGRSIMnemonic.h"

TGRSIDetectorInformation::TGRSIDetectorInformation()
{
   /// Default ctor for TGRSIDetectorInformation.
   Clear();
   Set();
}

TEventBuildingLoop::EBuildMode TGRSIDetectorInformation::BuildMode() const
{
   if(fSortByTriggerId) {
      return TEventBuildingLoop::EBuildMode::kTriggerId;
   }
   if(TGRSIOptions::AnalysisOptions()->BuildEventsByTimeStamp()) {
      return TEventBuildingLoop::EBuildMode::kTimestamp;
   }
   return TEventBuildingLoop::EBuildMode::kTime;
}

void TGRSIDetectorInformation::Print(Option_t* opt) const
{
   // Prints the TGRSIDetectorInformation. Options:
   // a: Print out more details.
   if(strchr(opt, 'a') != nullptr) {
      std::ostringstream str;
      str << DBLUE << "\t\tDESCANT in ancillary positions = " << DRED << (DescantAncillary() ? "TRUE" : "FALSE") << RESET_COLOR << std::endl;
      str << DBLUE << "\t\tDESCANT in wall configuration = " << DRED << (DescantWall() ? "TRUE" : "FALSE") << RESET_COLOR << std::endl;
      str << std::endl;
      str << "\t\tTIGRESS:            " << (Tigress() ? "true" : "false") << std::endl;
      str << "\t\tSHARC:              " << (Sharc() ? "true" : "false") << std::endl;
      str << "\t\tTRIFOIL:            " << (TriFoil() ? "true" : "false") << std::endl;
      str << "\t\tTIP:                " << (Tip() ? "true" : "false") << std::endl;
      str << "\t\tCSM:                " << (CSM() ? "true" : "false") << std::endl;
      str << "\t\tSPICE:              " << (Spice() ? "true" : "false") << std::endl;
      str << "\t\tGENERIC:            " << (Generic() ? "true" : "false") << std::endl;
      str << "\t\tS3:                 " << (S3() ? "true" : "false") << std::endl;
      str << "\t\tBAMBINO:            " << (Bambino() ? "true" : "false") << std::endl;
      str << "\t\tRF:                 " << (RF() ? "true" : "false") << std::endl;
      str << "\t\tGRIFFIN:            " << (Griffin() ? "true" : "false") << std::endl;
      str << "\t\tSCEPTAR:            " << (Sceptar() ? "true" : "false") << std::endl;
      str << "\t\tPACES:              " << (Paces() ? "true" : "false") << std::endl;
      str << "\t\tDESCANT:            " << (Descant() ? "true" : "false") << std::endl;
      str << "\t\tZDS:                " << (ZeroDegree() ? "true" : "false") << std::endl;
      str << "\t\tDANTE:              " << (Dante() ? "true" : "false") << std::endl;
      str << "\t\tBGO:                " << (Bgo() ? "true" : "false") << std::endl;
      str << "\t\tEMMA:               " << (Emma() ? "true" : "false") << std::endl;
      str << "\t\tTRIFIC:             " << (Trific() ? "true" : "false") << std::endl;
      str << "\t\tSHARC2:				  " << (Sharc2() ? "true" : "false") << std::endl;
      str << "\t\tRCMP:				     " << (Rcmp() ? "true" : "false") << std::endl;
      str << "\t\tARIES:              " << (Aries() ? "true" : "false") << std::endl;
      str << "\t\tDemand:             " << (Demand() ? "true" : "false") << std::endl;
      str << std::endl;
      std::cout << str.str();
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
   fEmma    = false;
   fTrific  = false;
   fSharc2  = false;

   fGriffin    = false;
   fSceptar    = false;
   fPaces      = false;
   fDante      = false;
   fZeroDegree = false;
   fDescant    = false;
   fBgo        = false;
   fRcmp       = false;
   fAries      = false;
   fDemand     = false;

   fDescantAncillary = false;
   fDescantWall      = false;
}

void TGRSIDetectorInformation::Set()
{
   /// Sets the run info. This figures out what systems are available.
   if(TChannel::GetChannelMap() == nullptr) {
      std::cerr << RED << "TGRSIDetectorInformation::Set(): Failed to get channel map!" << RESET_COLOR << std::endl;
      return;
   }
   for(auto& iter : *(TChannel::GetChannelMap())) {
      std::string channelname = iter.second->GetName();

      // check if we have an old TIG digitizer, in that case sort by trigger ID (instead of time stamp)
      // don't need to check again if we already found one, so include check for fSortByTriggerId being true
      if(!fSortByTriggerId && (iter.second->GetDigitizerType() == EDigitizer::kTIG10 || iter.second->GetDigitizerType() == EDigitizer::kTIG64)) {
         fSortByTriggerId = true;
      }
      //  detector system type.
      //  for more info, see: https://www.triumf.info/wiki/tigwiki/index.php/Detector_Nomenclature
      switch(static_cast<const TGRSIMnemonic*>(iter.second->GetMnemonic())->System()) {
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
      case TGRSIMnemonic::ESystem::kTrific:
         SetTrific();
         break;
      case TGRSIMnemonic::ESystem::kSharc2:
         SetSharc2();
         break;
      case TGRSIMnemonic::ESystem::kRcmp:
         SetRcmp();
         break;
      case TGRSIMnemonic::ESystem::kAries:
         SetAries();
         break;
      case TGRSIMnemonic::ESystem::kDemand:
         SetDemand();
         break;
      case TGRSIMnemonic::ESystem::kGeneric:
         SetGeneric();
         break;
      default:
         std::string system = iter.second->GetMnemonic()->SystemString();
         if(system == "SP") {
            SetSpice();
            SetS3();
         } else if(system == "BA") {
            SetBambino();
         }
      };
   }
}
