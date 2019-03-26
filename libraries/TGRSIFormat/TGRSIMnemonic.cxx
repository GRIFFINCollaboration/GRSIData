#include "TGRSIMnemonic.h"

#include <algorithm>

// Detector dependent includes
#include "TGriffin.h"
#include "TSceptar.h"
#include "TTigress.h"
#include "TTip.h"
#include "TTAC.h"
#include "TLaBr.h"
#include "TSharc.h"
#include "TCSM.h"
#include "TTriFoil.h"
#include "TRF.h"
#include "TS3.h"
#include "TPaces.h"
#include "TDescant.h"
#include "TZeroDegree.h"
#include "TSiLi.h"
#include "TGenericDetector.h"
#include "TBgo.h"
#include "TGriffinBgo.h"
#include "TLaBrBgo.h"
#include "TEmma.h"

ClassImp(TGRSIMnemonic)

void TGRSIMnemonic::Clear(Option_t*)
{
	TMnemonic::Clear();
   fSystem = ESystem::kClear;
}

void TGRSIMnemonic::EnumerateSystem()
{
   // Enumerating the fSystemString must come after the total mnemonic has been parsed as the details of other parts of
   // the mnemonic must be known
   if(fSystemString.compare("TI") == 0) {
      fSystem = ESystem::kTigress;
   } else if(fSystemString.compare("SH") == 0) {
      fSystem = ESystem::kSharc;
   } else if(fSystemString.compare("TR") == 0) {
      fSystem = ESystem::kTriFoil;
   } else if(fSystemString.compare("RF") == 0) {
      fSystem = ESystem::kRF;
   } else if(fSystemString.compare("SP") == 0) {
      if(SubSystem() == EMnemonic::kI) {
         fSystem = ESystem::kSiLi;
      } else {
         fSystem = ESystem::kSiLiS3;
      }
   } else if(fSystemString.compare("GD") == 0) {
         fSystem = ESystem::kGeneric;
   } else if(fSystemString.compare("CS") == 0) {
      fSystem = ESystem::kCSM;
   } else if(fSystemString.compare("GR") == 0) {
      if(SubSystem() == EMnemonic::kS) {
			fSystem = ESystem::kGriffinBgo;
		} else {
			fSystem = ESystem::kGriffin;
		}
   } else if(fSystemString.compare("SE") == 0) {
      fSystem = ESystem::kSceptar;
   } else if(fSystemString.compare("PA") == 0) {
      fSystem = ESystem::kPaces;
   } else if(fSystemString.compare("DS") == 0) {
      fSystem = ESystem::kDescant;
   } else if((fSystemString.compare("DA") == 0) || (fSystemString.compare("LB") == 0) ) {
      if(SubSystem() == EMnemonic::kS) {
			fSystem = ESystem::kLaBrBgo;
		} else if(SubSystem() == EMnemonic::kT) {
			fSystem = ESystem::kTAC;
		} else {
			fSystem = ESystem::kLaBr;
		}
   } else if(fSystemString.compare("BA") == 0) {
      fSystem = ESystem::kS3;
   } else if(fSystemString.compare("ZD") == 0) {
      fSystem = ESystem::kZeroDegree;
   } else if(fSystemString.compare("TP") == 0) {
      fSystem = ESystem::kTip;
   } else if(fSystemString.compare("BG") == 0) {
      fSystem = ESystem::kBgo;
   } else if(fSystemString.compare("EM") == 0) {
      fSystem = ESystem::kEmma;
   } else if(fSystemString.compare("ET") == 0) {
      fSystem = ESystem::kEmma;
   } else {
      fSystem = ESystem::kClear;
   }
}

void TGRSIMnemonic::EnumerateDigitizer(TPriorityValue<std::string>& digitizerName, TPriorityValue<EDigitizer>& digitizerType, TPriorityValue<int>& timeStampUnit)
{
	std::string name = digitizerName.Value();
   std::transform(name.begin(), name.end(), name.begin(), ::toupper);
	EDigitizer tmpType = EDigitizer::kDefault;
	int tmpUnit = 1;
   if(name.compare("GRF16") == 0) {
		tmpType = EDigitizer::kGRF16;
		tmpUnit = 10;
   } else if(name.compare("GRF4G") == 0) {
		tmpType = EDigitizer::kGRF4G;
		tmpUnit = 10;
   } else if(name.compare("TIG10") == 0) {
		tmpType = EDigitizer::kTIG10;
		tmpUnit = 10;
   } else if(name.compare("TIG64") == 0) {
		tmpType = EDigitizer::kTIG64;
		tmpUnit = 10;
   } else if(name.compare("CAEN") == 0) {
		tmpType = EDigitizer::kCaen;
		tmpUnit = 2;
   } else if(name.compare("MADC") == 0) {
		tmpType = EDigitizer::kMadc;
		tmpUnit = 10;
   } else if(name.compare("V1190") == 0) {
		tmpType = EDigitizer::kV1190;
		tmpUnit = 10;
   } else {
		std::cout<<"Warning, digitizer type '"<<name<<"' not recognized, options are 'GRF16', 'GRF4G', 'TIG10', 'TIG64', 'CAEN', 'MADC!' && 'V1190'!"<<std::endl;
	}
	digitizerType.Set(tmpType, digitizerName.Priority());
	timeStampUnit.Set(tmpUnit, digitizerName.Priority());
}

void TGRSIMnemonic::Parse(std::string* name)
{
   if((name == nullptr) || name->length() < 9) {
      if((name->length() < 1) && (name->compare(0, 2, "RF") == 0)) {
         SetRFMNEMONIC(name);
      }
      return;
   }
	TMnemonic::Parse(name);
   // Enumerating the fSystemString must come last as the details of other parts of
   // the mnemonic must be known
   EnumerateSystem();

   if(fSystem == ESystem::kSiLi) {
		std::string buf;
      buf.assign(*name, 7, 2);
      fSegment = static_cast<uint16_t>(strtol(buf.c_str(), nullptr, 16));
   }

   return;
}

void TGRSIMnemonic::Print(Option_t*) const
{
   printf("====== GRSIMNEMONIC ======\n");
   printf("fArrayPosition           = %i\n", fArrayPosition);
   printf("fSegment                 = %i\n", fSegment);
   printf("fSystemString            = %s\n", fSystemString.c_str());
   printf("fSubSystemString         = %s\n", fSubSystemString.c_str());
   printf("fArraySubPositionString  = %s\n", fArraySubPositionString.c_str());
   printf("fCollectedChargeString   = %s\n", fCollectedChargeString.c_str());
   printf("fOutputSensorString      = %s\n", fOutputSensorString.c_str());
   printf("===============================\n");
}

TClass* TGRSIMnemonic::GetClassType() const
{
   if(fClassType != nullptr) {
      return fClassType;
   }

   switch(System()) {
		case ESystem::kTigress:       fClassType = TTigress::Class(); break;
		case ESystem::kSharc:         fClassType = TSharc::Class(); break;
		case ESystem::kTriFoil:       fClassType = TTriFoil::Class(); break;
		case ESystem::kRF:            fClassType = TRF::Class(); break;
		case ESystem::kSiLi:          fClassType = TSiLi::Class(); break;
		case ESystem::kS3:            fClassType = TS3::Class(); break;
		case ESystem::kSiLiS3:        fClassType = TS3::Class(); break;
		case ESystem::kCSM:           fClassType = TCSM::Class(); break;
		case ESystem::kGriffin:       fClassType = TGriffin::Class(); break;
		case ESystem::kSceptar:       fClassType = TSceptar::Class(); break;
		case ESystem::kPaces:         fClassType = TPaces::Class(); break;
		case ESystem::kDescant:       fClassType = TDescant::Class(); break;
		case ESystem::kLaBr:          fClassType = TLaBr::Class(); break;
		case ESystem::kTAC:           fClassType = TTAC::Class(); break;
		case ESystem::kZeroDegree:    fClassType = TZeroDegree::Class(); break;
		case ESystem::kTip:           fClassType = TTip::Class(); break;
		case ESystem::kGriffinBgo:    fClassType = TGriffinBgo::Class(); break;
		case ESystem::kLaBrBgo:       fClassType = TLaBrBgo::Class(); break;
		case ESystem::kGeneric:       fClassType = TGenericDetector::Class(); break;
		case ESystem::kEmma:          fClassType = TEmma::Class(); break;
		default:                      fClassType = nullptr;
   };
   return fClassType;
}
