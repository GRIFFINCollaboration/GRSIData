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
#include "TTrific.h"
#include "TSharc2.h"
#include "TRcmp.h"

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
   if(SystemString().compare("TI") == 0) {
      fSystem = ESystem::kTigress;
   } else if(SystemString().compare("SH") == 0) {
      fSystem = ESystem::kSharc;
   } else if(SystemString().compare("TR") == 0) {
      fSystem = ESystem::kTriFoil;
   } else if(SystemString().compare("RF") == 0) {
      fSystem = ESystem::kRF;
   } else if(SystemString().compare("SP") == 0) {
      if(SubSystem() == EMnemonic::kI) {
         fSystem = ESystem::kSiLi;
      } else {
         fSystem = ESystem::kSiLiS3;
      }
   } else if(SystemString().compare("GD") == 0) {
         fSystem = ESystem::kGeneric;
   } else if(SystemString().compare("CS") == 0) {
      fSystem = ESystem::kCSM;
   } else if(SystemString().compare("GR") == 0) {
      if(SubSystem() == EMnemonic::kS) {
			fSystem = ESystem::kGriffinBgo;
		} else {
			fSystem = ESystem::kGriffin;
		}
   } else if(SystemString().compare("SE") == 0) {
      fSystem = ESystem::kSceptar;
   } else if(SystemString().compare("PA") == 0) {
      fSystem = ESystem::kPaces;
   } else if(SystemString().compare("DS") == 0) {
      fSystem = ESystem::kDescant;
   } else if((SystemString().compare("DA") == 0) || (SystemString().compare("LB") == 0) ) {
      if(SubSystem() == EMnemonic::kS) {
			fSystem = ESystem::kLaBrBgo;
		} else if(SubSystem() == EMnemonic::kT) {
			fSystem = ESystem::kTAC;
		} else {
			fSystem = ESystem::kLaBr;
		}
   } else if(SystemString().compare("BA") == 0) {
      fSystem = ESystem::kS3;
   } else if(SystemString().compare("ZD") == 0) {
      fSystem = ESystem::kZeroDegree;
   } else if(SystemString().compare("TP") == 0) {
      fSystem = ESystem::kTip;
   } else if(SystemString().compare("BG") == 0) {
      fSystem = ESystem::kBgo;
   } else if((SystemString().compare("EM") == 0) || (SystemString().compare("ET") == 0)) {
      if(SubSystem() == EMnemonic::kE) {
        fSystem = ESystem::kEmmaS3;
      }
      else {
        fSystem = ESystem::kEmma;
      }
   } else if(SystemString().compare("TF") == 0) {
      fSystem = ESystem::kTrific;
   }  else if(fSystemString.compare("SZ") == 0) {
      fSystem = ESystem::kSharc2;
   }  else if(fSystemString.compare("RC") == 0) {  
      fSystem = ESystem::kRcmp;
   }  else {
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
		tmpUnit = 50;
   } else if(name.compare("V1190") == 0) {
		tmpType = EDigitizer::kV1190;
		tmpUnit = 50;
   } else if(name.compare("FMC32") == 0) {
		tmpType = EDigitizer::kFMC32;
		tmpUnit = 10;
   } else {
		std::cout<<"Warning, digitizer type '"<<name<<"' not recognized, options are 'GRF16', 'FMC32', 'GRF4G', 'TIG10', 'TIG64', 'CAEN', 'MADC!' && 'V1190'!"<<std::endl;
	}
	digitizerType.Set(tmpType, digitizerName.Priority());
	timeStampUnit.Set(tmpUnit, digitizerName.Priority());
}

void TGRSIMnemonic::Parse(std::string* name)
{
   if((name == nullptr) || name->length() < 9) {
      if((name->length() < 1) && (name->compare(0, 2, "RF") == 0)) {
         SetRFMnemonic(name);
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
      Segment(static_cast<int16_t>(strtol(buf.c_str(), nullptr, 16)));
   }

   return;
}

void TGRSIMnemonic::Print(Option_t*) const
{
	std::ostringstream str;
   str<<"======== GRSIMNEMONIC ========"<<std::endl;
	TMnemonic::Print(str);
   str<<"=============================="<<std::endl;
	std::cout<<str.str();
}

TClass* TGRSIMnemonic::GetClassType() const
{
   if(TMnemonic::GetClassType() != nullptr) {
      return TMnemonic::GetClassType();
   }

   switch(System()) {
		case ESystem::kTigress:       SetClassType(TTigress::Class()); break;
		case ESystem::kSharc:         SetClassType(TSharc::Class()); break;
		case ESystem::kTriFoil:       SetClassType(TTriFoil::Class()); break;
		case ESystem::kRF:            SetClassType(TRF::Class()); break;
		case ESystem::kSiLi:          SetClassType(TSiLi::Class()); break;
		case ESystem::kS3:            SetClassType(TS3::Class()); break;
		case ESystem::kSiLiS3:        SetClassType(TS3::Class()); break;
		case ESystem::kCSM:           SetClassType(TCSM::Class()); break;
		case ESystem::kGriffin:       SetClassType(TGriffin::Class()); break;
		case ESystem::kSceptar:       SetClassType(TSceptar::Class()); break;
		case ESystem::kPaces:         SetClassType(TPaces::Class()); break;
		case ESystem::kDescant:       SetClassType(TDescant::Class()); break;
		case ESystem::kLaBr:          SetClassType(TLaBr::Class()); break;
		case ESystem::kTAC:           SetClassType(TTAC::Class()); break;
		case ESystem::kZeroDegree:    SetClassType(TZeroDegree::Class()); break;
		case ESystem::kTip:           SetClassType(TTip::Class()); break;
		case ESystem::kGriffinBgo:    SetClassType(TGriffinBgo::Class()); break;
		case ESystem::kLaBrBgo:       SetClassType(TLaBrBgo::Class()); break;
		case ESystem::kGeneric:       SetClassType(TGenericDetector::Class()); break;
		case ESystem::kEmma:          SetClassType(TEmma::Class()); break;
		case ESystem::kEmmaS3:        SetClassType(TS3::Class()); break;
		case ESystem::kTrific:        SetClassType(TTrific::Class()); break;
		case ESystem::kSharc2:        SetClassType(TSharc2::Class()); break;
		case ESystem::kRcmp:          SetClassType(TRcmp::Class()); break;
		default:                      SetClassType(nullptr);
   };
   return TMnemonic::GetClassType();
}

double TGRSIMnemonic::GetTime(Long64_t timestamp, Float_t cfd, double energy, const TChannel* channel) const
{
   if(channel == nullptr) {
      Error("GetTime", "No TChannel provided");
		return static_cast<Double_t>((timestamp) + gRandom->Uniform());
   }
	
   switch(static_cast<EDigitizer>(channel->GetDigitizerType())) {
		Double_t dTime;
		case EDigitizer::kGRF16:
		case EDigitizer::kFMC32:
			// we need to zero the lowest 18 bits of the timestamp as those are included in the CFD value
			// TODO: what happens close to the wrap-around of those 18 bits??? This only happens every 2^18 * 10e-8 so 2.5 ms so 400 Hz
			dTime = (timestamp & (~0x3ffff)) * channel->GetTimeStampUnit() + channel->CalibrateCFD((cfd + gRandom->Uniform()) / 1.6); // CFD is in 10/16th of a nanosecond
			return dTime - channel->GetTZero(energy) - channel->GetTimeOffset();
		case EDigitizer::kGRF4G:
			dTime = timestamp*channel->GetTimeStampUnit() + channel->CalibrateCFD((static_cast<Int_t>(cfd) >> 22) + ((static_cast<Int_t>(cfd) & 0x3fffff) + gRandom->Uniform()) / 256.);
			return dTime - channel->GetTZero(energy) - channel->GetTimeOffset();
		case EDigitizer::kTIG10:
			dTime = (timestamp & (~0x7fffff))*channel->GetTimeStampUnit() +	channel->CalibrateCFD((cfd + gRandom->Uniform()) / 1.6); // CFD is in 10/16th of a nanosecond
			//channel->CalibrateCFD((cfd & (~0xf) + gRandom->Uniform()) / 1.6); // PBender suggests this.
			return dTime - channel->GetTZero(energy) - channel->GetTimeOffset();
      case EDigitizer::kCaen:
			//10 bit CFD for 0-2ns => divide by 512
			dTime = timestamp*channel->GetTimeStampUnit() + channel->CalibrateCFD((cfd + gRandom->Uniform()) / 512.);
			return dTime - channel->GetTZero(energy) - channel->GetTimeOffset();
		default:
			dTime = static_cast<Double_t>(((timestamp) + gRandom->Uniform())*channel->GetTimeStampUnit());
			return dTime - channel->GetTZero(energy) - channel->GetTimeOffset();
	}
   return 0.;
}

int TGRSIMnemonic::NumericArraySubPosition() const
{
	/// This function translates the crystal color to an index
	/// B - Blue - 0, G - Green - 1, R - Red - 2, W - White - 3, default - 5
	/// Except for the LaBr BGOs which use A - 0, B - 1, C - 2
	/// and a default of 5
	if(System() == ESystem::kLaBrBgo) {
		switch(ArraySubPosition()) {
			case TMnemonic::EMnemonic::kA:
				return 0;
			case TMnemonic::EMnemonic::kB:
				return 1;
			case TMnemonic::EMnemonic::kC:
				return 2;
			default:
				return 5;
		};
	}

	switch(ArraySubPosition()) {
		case TMnemonic::EMnemonic::kB:
			return 0;
		case TMnemonic::EMnemonic::kG:
			return 1;
		case TMnemonic::EMnemonic::kR:
			return 2;
		case TMnemonic::EMnemonic::kW:
			return 3;
		default:
			return 5;
	};

	// return statement here instead of default case
	// to make sure compiler doesn't warn us about missing return
	return 5;
}


