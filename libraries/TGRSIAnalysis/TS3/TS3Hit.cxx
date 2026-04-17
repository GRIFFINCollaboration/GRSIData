#include "TS3Hit.h"

#include "TMath.h"

#include "TS3.h"
#include "TPulseAnalyzer.h"
#include "TGRSIMnemonic.h"
#include "TMnemonic.h"

TS3Hit::TS3Hit()
{
   Clear();
}

TS3Hit::TS3Hit(const TFragment& frag)
{
   frag.Copy(*this);
   if(static_cast<const TGRSIMnemonic*>(GetChannel()->GetMnemonic())->System() == TGRSIMnemonic::ESystem::kSiLiS3) {
      SetIsDownstream(false);
   } else {
      // same as 'if(GetArrayPosition()<2) false, else true'
      SetIsDownstream(GetArrayPosition() >= 2);
   }
}

TS3Hit::TS3Hit(const TS3Hit& rhs) : TDetectorHit(rhs)
{
   Clear();
   rhs.Copy(*this);
}

void TS3Hit::Copy(TObject& rhs) const
{
   TDetectorHit::Copy(rhs);

   static_cast<TS3Hit&>(rhs).fRing         = fRing;
   static_cast<TS3Hit&>(rhs).fSector       = fSector;
   static_cast<TS3Hit&>(rhs).fIsDownstream = fIsDownstream;
   static_cast<TS3Hit&>(rhs).fTimeFit      = fTimeFit;
   static_cast<TS3Hit&>(rhs).fSig2Noise    = fSig2Noise;
	static_cast<TS3Hit&>(rhs).fEDiff        = fEDiff;
	static_cast<TS3Hit&>(rhs).fTDiff        = fTDiff;
   static_cast<TS3Hit&>(rhs).fIsSRIMSet    = fIsSRIMSet;
	static_cast<TS3Hit&>(rhs).fSRIMTable    = fSRIMTable;
}

void TS3Hit::Copy(TObject& rhs, bool waveform) const
{
   Copy(rhs);
   if(waveform) {
      CopyWave(rhs);
   }
}

void TS3Hit::Clear(Option_t* opt)
{
   TDetectorHit::Clear(opt);
   fRing         = -1;
   fSector       = -1;
   fIsDownstream = false;
	fEDiff        = -10000;
	fTDiff        = -10000;
   fIsSRIMSet    = false;
	fSRIMTable    = nullptr;
}

void TS3Hit::SetWavefit(const TFragment& frag)
{
   TPulseAnalyzer pulse(frag);
   if(pulse.IsSet()) {
      fTimeFit   = pulse.fit_newT0();
      fSig2Noise = pulse.get_sig2noise();
   }
}

Bool_t TS3Hit::SectorsDownstream() const
{   //Do not confuse with fIsDownstream which relates to detector position relative to target, NOT orientation.

   if(static_cast<const TGRSIMnemonic*>(GetChannel()->GetMnemonic())->System() == TGRSIMnemonic::ESystem::kSiLiS3) {
      return (GetArrayPosition() == 0);
   }
   return (GetArrayPosition() % 2 == 1);
}

TVector3 TS3Hit::GetPosition(Double_t phioffset, Double_t dist, bool smear) const
{
   return TS3::GetPosition(GetRing(), GetSector(), phioffset, dist, SectorsDownstream(), smear);
}

TVector3 TS3Hit::GetPosition(Double_t phioffset, bool smear) const
{
   return TS3::GetPosition(GetRing(), GetSector(), phioffset, GetDefaultDistance(), SectorsDownstream(), smear);
}

TVector3 TS3Hit::GetPosition(bool smear) const
{
   return TS3::GetPosition(GetRing(), GetSector(), GetDefaultPhiOffset(), GetDefaultDistance(), SectorsDownstream(), smear);
}

void TS3Hit::Print(Option_t*) const
{
   Print(std::cout);
}

void TS3Hit::Print(std::ostream& out) const
{
   std::ostringstream str;
   str << "================" << std::endl;
   str << "not yet written." << std::endl;
   str << "================" << std::endl;
   out << str.str();
}

Double_t TS3Hit::GetDefaultPhiOffset() const
{                           //Phi rotation of connector in setup
   double deg = -90 - 21;   // -22.5 should be bambino chamber rotation -90 rotation of detector in chamber
   if(static_cast<const TGRSIMnemonic*>(GetChannel()->GetMnemonic())->System() == TGRSIMnemonic::ESystem::kSiLiS3) {
      deg = -22.5;
      if(GetChannel()->GetMnemonic()->ArrayPosition() == 2) {
         deg += 90;
      }
   }
   return deg * TMath::Pi() / 180.;
}

Double_t TS3Hit::GetDefaultDistance() const
{   // relative to target (SPICE target not at Z=0)
   double      z   = 0;
   std::string str = GetDistanceStr();
   if(static_cast<const TGRSIMnemonic*>(GetChannel()->GetMnemonic())->System() == TGRSIMnemonic::ESystem::kSiLiS3) {
      if(str.find('D') < str.size()) {
         z = 22.5;
      } else if(str.find('E') < str.size()) {
         z = 28.35;
      } else {
         z = 42.1;
      }
   } else {
      if(str.find('D') < str.size()) {
         z = 20;
      } else if(str.find('E') < str.size()) {
         z = 32;
      } else {
         z = 40;
      }

      if(GetArrayPosition() < 2) {
         z = -z;
      }
   }

   return z;
}

void TS3Hit::SetSRIMTable(TSRIM* srimTable)
{
	fSRIMTable = srimTable;
	SetIsSRIMSet(true);
}

Double_t TS3Hit::GetCorrectedEnergy(double s3DL)
{
  if(!GetIsSRIMSet()){
		std::cerr<<"Error in TS3Hit::GetCorrectedEnergy - Please load SRIMTable"<<std::endl;
		return -1;
	}

	TVector3 vec(0, 0, -1); // S3 is at backward angles w.r.t beam axis. 
												  // z=-1 to get correct positive angles of rings.
	double theta = GetTheta(0, &vec);
	double eCor = GetEnergy() + fSRIMTable->GetEnergyLost(GetEnergy(), s3DL/TMath::Cos(theta));
	
	return eCor;
}
