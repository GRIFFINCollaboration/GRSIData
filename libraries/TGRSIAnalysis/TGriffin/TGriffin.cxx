#include "TGriffin.h"

#include <sstream>
#include <iostream>
#include <iomanip>

#include "TMath.h"
#include "TInterpreter.h"
#include "TMnemonic.h"

#include "TGRSIOptions.h"

////////////////////////////////////////////////////////////
//
// TGriffin
//
// The TGriffin class defines the observables and algorithms used
// when analyzing GRIFFIN data. It includes detector positions,
// add-back methods, etc.
//
////////////////////////////////////////////////////////////

bool DefaultGriffinAddback(const TDetectorHit* one, const TDetectorHit* two)
{
   return ((one->GetDetector() == two->GetDetector()) &&
           (std::fabs(one->GetTime() - two->GetTime()) < TGRSIOptions::AnalysisOptions()->AddbackWindow()));
}

std::function<bool(const TDetectorHit*, const TDetectorHit*)> TGriffin::fAddbackCriterion = DefaultGriffinAddback;

bool DefaultGriffinSuppression(const TDetectorHit* hit, const TDetectorHit* bgoHit)
{
   //return ((hit.GetDetector() == bgoHit.GetDetector() && hit.GetCrystal() == bgoHit.GetCrystal()) &&
   return ((hit->GetDetector() == bgoHit->GetDetector()) &&
           (std::fabs(hit->GetTime() - bgoHit->GetTime()) < TGRSIOptions::AnalysisOptions()->SuppressionWindow()) &&
           (bgoHit->GetEnergy() > TGRSIOptions::AnalysisOptions()->SuppressionEnergy()));
}

std::function<bool(const TDetectorHit*, const TDetectorHit*)> TGriffin::fSuppressionCriterion = DefaultGriffinSuppression;

bool TGriffin::fSetCoreWave = false;

// This seems unnecessary, and why 17?;//  they are static members, and need
//  to be defined outside the header
//  17 is to have the detectors go from 1-16
//  plus we can use position zero
//  when the detector winds up back in
//  one of the stands like Alex used in the
//  gps run. pcb.
// Initiallizes the HPGe Clover positions as per the wiki
// <https://www.triumf.info/wiki/tigwiki/index.php/HPGe_Coordinate_Table>
//                                                                             theta                                 phi
//                                                                             theta                                phi
//                                                                             theta
std::array<TVector3, 17> TGriffin::fCloverPosition = {
   TVector3(TMath::Sin(TMath::DegToRad() * (0.0)) * TMath::Cos(TMath::DegToRad() * (0.0)),
            TMath::Sin(TMath::DegToRad() * (0.0)) * TMath::Sin(TMath::DegToRad() * (0.0)),
            TMath::Cos(TMath::DegToRad() * (0.0))),
   // Downstream lampshade
   TVector3(TMath::Sin(TMath::DegToRad() * (45.0)) * TMath::Cos(TMath::DegToRad() * (67.5)),
            TMath::Sin(TMath::DegToRad() * (45.0)) * TMath::Sin(TMath::DegToRad() * (67.5)),
            TMath::Cos(TMath::DegToRad() * (45.0))),
   TVector3(TMath::Sin(TMath::DegToRad() * (45.0)) * TMath::Cos(TMath::DegToRad() * (157.5)),
            TMath::Sin(TMath::DegToRad() * (45.0)) * TMath::Sin(TMath::DegToRad() * (157.5)),
            TMath::Cos(TMath::DegToRad() * (45.0))),
   TVector3(TMath::Sin(TMath::DegToRad() * (45.0)) * TMath::Cos(TMath::DegToRad() * (247.5)),
            TMath::Sin(TMath::DegToRad() * (45.0)) * TMath::Sin(TMath::DegToRad() * (247.5)),
            TMath::Cos(TMath::DegToRad() * (45.0))),
   TVector3(TMath::Sin(TMath::DegToRad() * (45.0)) * TMath::Cos(TMath::DegToRad() * (337.5)),
            TMath::Sin(TMath::DegToRad() * (45.0)) * TMath::Sin(TMath::DegToRad() * (337.5)),
            TMath::Cos(TMath::DegToRad() * (45.0))),
   // Corona
   TVector3(TMath::Sin(TMath::DegToRad() * (90.0)) * TMath::Cos(TMath::DegToRad() * (22.5)),
            TMath::Sin(TMath::DegToRad() * (90.0)) * TMath::Sin(TMath::DegToRad() * (22.5)),
            TMath::Cos(TMath::DegToRad() * (90.0))),
   TVector3(TMath::Sin(TMath::DegToRad() * (90.0)) * TMath::Cos(TMath::DegToRad() * (67.5)),
            TMath::Sin(TMath::DegToRad() * (90.0)) * TMath::Sin(TMath::DegToRad() * (67.5)),
            TMath::Cos(TMath::DegToRad() * (90.0))),
   TVector3(TMath::Sin(TMath::DegToRad() * (90.0)) * TMath::Cos(TMath::DegToRad() * (112.5)),
            TMath::Sin(TMath::DegToRad() * (90.0)) * TMath::Sin(TMath::DegToRad() * (112.5)),
            TMath::Cos(TMath::DegToRad() * (90.0))),
   TVector3(TMath::Sin(TMath::DegToRad() * (90.0)) * TMath::Cos(TMath::DegToRad() * (157.5)),
            TMath::Sin(TMath::DegToRad() * (90.0)) * TMath::Sin(TMath::DegToRad() * (157.5)),
            TMath::Cos(TMath::DegToRad() * (90.0))),
   TVector3(TMath::Sin(TMath::DegToRad() * (90.0)) * TMath::Cos(TMath::DegToRad() * (202.5)),
            TMath::Sin(TMath::DegToRad() * (90.0)) * TMath::Sin(TMath::DegToRad() * (202.5)),
            TMath::Cos(TMath::DegToRad() * (90.0))),
   TVector3(TMath::Sin(TMath::DegToRad() * (90.0)) * TMath::Cos(TMath::DegToRad() * (247.5)),
            TMath::Sin(TMath::DegToRad() * (90.0)) * TMath::Sin(TMath::DegToRad() * (247.5)),
            TMath::Cos(TMath::DegToRad() * (90.0))),
   TVector3(TMath::Sin(TMath::DegToRad() * (90.0)) * TMath::Cos(TMath::DegToRad() * (292.5)),
            TMath::Sin(TMath::DegToRad() * (90.0)) * TMath::Sin(TMath::DegToRad() * (292.5)),
            TMath::Cos(TMath::DegToRad() * (90.0))),
   TVector3(TMath::Sin(TMath::DegToRad() * (90.0)) * TMath::Cos(TMath::DegToRad() * (337.5)),
            TMath::Sin(TMath::DegToRad() * (90.0)) * TMath::Sin(TMath::DegToRad() * (337.5)),
            TMath::Cos(TMath::DegToRad() * (90.0))),
   // Upstream lampshade
   TVector3(TMath::Sin(TMath::DegToRad() * (135.0)) * TMath::Cos(TMath::DegToRad() * (67.5)),
            TMath::Sin(TMath::DegToRad() * (135.0)) * TMath::Sin(TMath::DegToRad() * (67.5)),
            TMath::Cos(TMath::DegToRad() * (135.0))),
   TVector3(TMath::Sin(TMath::DegToRad() * (135.0)) * TMath::Cos(TMath::DegToRad() * (157.5)),
            TMath::Sin(TMath::DegToRad() * (135.0)) * TMath::Sin(TMath::DegToRad() * (157.5)),
            TMath::Cos(TMath::DegToRad() * (135.0))),
   TVector3(TMath::Sin(TMath::DegToRad() * (135.0)) * TMath::Cos(TMath::DegToRad() * (247.5)),
            TMath::Sin(TMath::DegToRad() * (135.0)) * TMath::Sin(TMath::DegToRad() * (247.5)),
            TMath::Cos(TMath::DegToRad() * (135.0))),
   TVector3(TMath::Sin(TMath::DegToRad() * (135.0)) * TMath::Cos(TMath::DegToRad() * (337.5)),
            TMath::Sin(TMath::DegToRad() * (135.0)) * TMath::Sin(TMath::DegToRad() * (337.5)),
            TMath::Cos(TMath::DegToRad() * (135.0)))};

TGriffin::TGriffin()
{
   /// Default ctor. Ignores TObjectStreamer in ROOT < 6
   Clear();
}

TGriffin::TGriffin(const TGriffin& rhs) : TSuppressed(rhs)
{
   /// Copy ctor. Ignores TObjectStreamer in ROOT < 6
   rhs.Copy(*this);
}

void TGriffin::Copy(TObject& rhs) const
{
   // Copy function.
   TSuppressed::Copy(rhs);

   // no need to copy hits, this is already taken care of by TDetector::Copy (called by TSuppressed::Copy)
   // not copying addback or suppressed vectors
   for(auto& hit : static_cast<TGriffin&>(rhs).fAddbackHits) {
      delete hit;
   }
   for(auto& hit : static_cast<TGriffin&>(rhs).fSuppressedHits) {
      delete hit;
   }
   for(auto& hit : static_cast<TGriffin&>(rhs).fSuppressedAddbackHits) {
      delete hit;
   }
   static_cast<TGriffin&>(rhs).fGriffinBits = 0;
   static_cast<TGriffin&>(rhs).fAddbackHits.clear();
   static_cast<TGriffin&>(rhs).fAddbackFrags.clear();
   static_cast<TGriffin&>(rhs).fSuppressedHits.clear();
   static_cast<TGriffin&>(rhs).fSuppressedAddbackHits.clear();
   static_cast<TGriffin&>(rhs).fSuppressedAddbackFrags.clear();

   static_cast<TGriffin&>(rhs).fCycleStart = fCycleStart;
}

TGriffin::~TGriffin()
{
   // Default Destructor
   // no need to delete hits, this is taken care of by the destructor of TDetector
   for(auto& hit : fAddbackHits) {
      delete hit;
   }
   for(auto& hit : fSuppressedHits) {
      delete hit;
   }
   for(auto& hit : fSuppressedAddbackHits) {
      delete hit;
   }
}

void TGriffin::Clear(Option_t* opt)
{
   // Clears the mother, and all of the hits
   ClearStatus();
   TSuppressed::Clear(opt);
   // hits cleared by TDetector::Clear
   for(auto& hit : fAddbackHits) {
      delete hit;
   }
   for(auto& hit : fSuppressedHits) {
      delete hit;
   }
   for(auto& hit : fSuppressedAddbackHits) {
      delete hit;
   }
   fAddbackHits.clear();
   fAddbackFrags.clear();
   fSuppressedHits.clear();
   fSuppressedAddbackHits.clear();
   fSuppressedAddbackFrags.clear();

   fCycleStart = 0;
}

void TGriffin::Print(Option_t*) const
{
   Print(std::cout);
}

void TGriffin::Print(std::ostream& out) const
{
   std::ostringstream str;
   str << "Griffin Contains: " << std::endl;
   str << std::setw(6) << GetMultiplicity() << " hits" << std::endl;
   //if(TString(opt).Contains("all", TString::ECaseCompare::kIgnoreCase)) {
   for(const auto& hit : Hits()) {
      static_cast<TGriffinHit*>(hit)->Print(str);
   }
   //}

   if(IsAddbackSet()) {
      str << std::setw(6) << fAddbackHits.size() << " addback hits" << std::endl;
   } else {
      str << std::setw(6) << " "
          << " Addback not set" << std::endl;
   }

   str << std::setw(6) << " "
       << " Cross-talk Set? " << IsCrossTalkSet() << std::endl;
   str << std::setw(6) << fCycleStart << " cycle start" << std::endl;
   out << str.str();
}

TGriffin& TGriffin::operator=(const TGriffin& rhs)
{
   rhs.Copy(*this);
   return *this;
}

bool TGriffin::IsAddbackSet() const
{
   return TestBitNumber(EGriffinBits::kIsAddbackSet);
}

bool TGriffin::IsCrossTalkSet() const
{
   return TestBitNumber(EGriffinBits::kIsCrossTalkSet);
}

void TGriffin::SetAddback(const bool flag) const
{
   SetBitNumber(EGriffinBits::kIsAddbackSet, flag);
}

void TGriffin::SetCrossTalk(const bool flag) const
{
   SetBitNumber(EGriffinBits::kIsCrossTalkSet, flag);
}

TGriffinHit* TGriffin::GetGriffinHit(const int& i)
{
   /// Get Griffin hit indicated by index i.
   /// Throws an out of range exception if the index is out of the range of the hit vector.
   /// Applies cross-talk corrections if they haven't already been applied and are enabled.
   /// Note: This is different from using TDetector::GetHit and casting the result to a `TGriffinHit*`
   /// as in that case no cross-talk corrections are applied.
   try {
      if(!IsCrossTalkSet()) {
         FixCrossTalk();
      }
      return static_cast<TGriffinHit*>(Hits().at(i));
   } catch(const std::out_of_range& oor) {
      std::cerr << ClassName() << " Hits are out of range: " << oor.what() << std::endl;
      if(!gInterpreter) {
         throw grsi::exit_exception(1);
      }
   }
   return nullptr;
}

Short_t TGriffin::GetAddbackMultiplicity()
{
   // Automatically builds the addback hits using the fAddbackCriterion (if the size of the fAddbackHits vector is zero)
   // and return the number of addback hits.
   if(!IsCrossTalkSet()) {
      // Calculate Cross Talk on each hit
      FixCrossTalk();
   }
   auto& hitVector = Hits();
   if(hitVector.empty()) {
      return 0;
   }
   // if the addback has been reset, clear the addback hits
   if(!IsAddbackSet()) {
      ResetAddback();
   }
   if(fAddbackHits.empty()) {
      CreateAddback(hitVector, fAddbackHits, fAddbackFrags);
      SetAddback(true);
   }

   return fAddbackHits.size();
}

TGriffinHit* TGriffin::GetAddbackHit(const int& i)
{
   if(i < GetAddbackMultiplicity()) {
      return static_cast<TGriffinHit*>(fAddbackHits[i]);
   }
   std::cerr << "Addback hits are out of range" << std::endl;
   throw grsi::exit_exception(1);
   return nullptr;
}

void TGriffin::AddFragment(const std::shared_ptr<const TFragment>& frag, TChannel* chan)
{
   // Builds the GRIFFIN Hits directly from the TFragment. Basically, loops through the hits for an event and sets
   // observables.
   // This is done for both GRIFFIN and it's suppressors.
   if(frag == nullptr || chan == nullptr) {
      return;
   }
   const TMnemonic* mnemonic = chan->GetMnemonic();
   if(mnemonic == nullptr) {
      std::cerr << "Trying to add fragment to TGriffin w/o mnemonic in TChannel!" << std::endl;
      return;
   }

   if(mnemonic->SubSystem() != TMnemonic::EMnemonic::kG) {
      std::cerr << __PRETTY_FUNCTION__ << ": not a GRIFFIN detector: " << static_cast<std::underlying_type_t<TMnemonic::EMnemonic>>(mnemonic->SubSystem()) << std::endl;   // NOLINT(cppcoreguidelines-pro-bounds-array-to-pointer-decay)
      return;
   }

   // only create the hit if we have a HPGe signal (A), not a suppressor (S) or backup HPGe signal (B)
   switch(mnemonic->OutputSensor()) {
   case TMnemonic::EMnemonic::kA: {
      auto* hit = new TGriffinHit(*frag);
      Hits().push_back(hit);
   } break;
   case TMnemonic::EMnemonic::kB:
      break;
   default:
      std::cout << "output sensor " << static_cast<std::underlying_type_t<TMnemonic::EMnemonic>>(mnemonic->OutputSensor()) << ", skipping it" << std::endl;
      break;
   };
}

TVector3 TGriffin::GetPosition(int DetNbr, int CryNbr, double dist)
{
   // Gets the position vector for a crystal specified by CryNbr within Clover DetNbr at a distance of dist mm away.
   // This is calculated to the most likely interaction point within the crystal.
   if(DetNbr > 16) {
      return {0, 0, 1};
   }

   TVector3 temp_pos(fCloverPosition[DetNbr]);

   // Interaction points may eventually be set externally. May make these members of each crystal, or pass from
   // waveforms.
   Double_t cp = 26.0;   // Crystal Center Point  mm.
   Double_t id = 45.0;   // 45.0;  //Crystal interaction depth mm.
   // Set Theta's of the center of each DETECTOR face
   ////Define one Detector position
   TVector3 shift;
   switch(CryNbr) {
   case 0: shift.SetXYZ(-cp, cp, id); break;
   case 1: shift.SetXYZ(cp, cp, id); break;
   case 2: shift.SetXYZ(cp, -cp, id); break;
   case 3: shift.SetXYZ(-cp, -cp, id); break;
   default: shift.SetXYZ(0, 0, 1); break;
   };
   shift.RotateY(temp_pos.Theta());
   shift.RotateZ(temp_pos.Phi());

   temp_pos.SetMag(dist);

   return (temp_pos + shift);
}

TVector3 TGriffin::GetDetectorPosition(int DetNbr)
{
   // Gets the position vector for a Clover DetNbr.
   if(DetNbr > 16) {
      return {0, 0, 1};
   }

   return fCloverPosition[DetNbr];
}

void TGriffin::ResetFlags() const
{
   fGriffinBits = 0;
}

void TGriffin::ResetAddback()
{
   SetAddback(false);
   SetCrossTalk(false);
   for(auto& hit : fAddbackHits) {
      delete hit;
   }
   fAddbackHits.clear();
   fAddbackFrags.clear();
}

UShort_t TGriffin::GetNAddbackFrags(const size_t& idx)
{
   // Get the number of addback "fragments" contributing to the total addback hit
   // with index idx.
   if(idx < fAddbackFrags.size()) {
      return fAddbackFrags[idx];
   }
   return 0;
}

void TGriffin::SetBitNumber(enum EGriffinBits bit, Bool_t set) const
{
   // Used to set the flags that are stored in TGriffin.
   fGriffinBits.SetBit(bit, set);
}

Double_t TGriffin::CTCorrectedEnergy(const TGriffinHit* const hit_to_correct, const TGriffinHit* const other_hit,
                                     Bool_t time_constraint)
{
   if((hit_to_correct == nullptr) || (other_hit == nullptr)) {
      std::cerr << "One of the hits is invalid in TGriffin::CTCorrectedEnergy" << std::endl;
      return 0;
   }

   if(time_constraint) {
      // Figure out if this passes the selected window
      if(TMath::Abs(other_hit->GetTime() - hit_to_correct->GetTime()) >
         TGRSIOptions::AnalysisOptions()->AddbackWindow()) {   // placeholder
         return hit_to_correct->GetEnergy();
      }
   }

   if(hit_to_correct->GetDetector() != other_hit->GetDetector()) {
      return hit_to_correct->GetEnergy();
   }
   static std::array<bool, 256> been_warned  = {false};
   double                       fixed_energy = hit_to_correct->GetEnergy();
   try {
      if(hit_to_correct->GetChannel() != nullptr) {
         fixed_energy -= hit_to_correct->GetChannel()->GetCTCoeff().at(other_hit->GetCrystal()) * other_hit->GetNoCTEnergy();
      }
   } catch(const std::out_of_range& oor) {
      int id = 16 * hit_to_correct->GetDetector() + 4 * hit_to_correct->GetCrystal() + other_hit->GetCrystal();
      if(!been_warned[id]) {
         been_warned[id] = true;
         std::cerr << DRED << "Missing CT correction for Det: " << hit_to_correct->GetDetector()
                   << " Crystals: " << hit_to_correct->GetCrystal() << " " << other_hit->GetCrystal() << " (id " << id << ")" << RESET_COLOR << std::endl;
      }
      return hit_to_correct->GetEnergy();
   }

   return fixed_energy;
}

void TGriffin::FixCrossTalk()
{
   if(!TGRSIOptions::AnalysisOptions()->IsCorrectingCrossTalk()) { return; }

   auto& hitVector = Hits();
   if(hitVector.size() < 2) {
      SetCrossTalk(true);
      return;
   }
   for(auto& hit : hitVector) {
      static_cast<TGriffinHit*>(hit)->ClearEnergy();
   }

   for(auto& one : hitVector) {
      for(auto& two : hitVector) {
         one->SetEnergy(CTCorrectedEnergy(static_cast<TGriffinHit*>(one), static_cast<TGriffinHit*>(two)));
      }
   }
   SetCrossTalk(true);
}

const char* TGriffin::GetColorFromNumber(int number)
{
   switch(number) {
   case(0): return "B";
   case(1): return "G";
   case(2): return "R";
   case(3): return "W";
   };
   return "X";
}

bool TGriffin::IsSuppressed() const
{
   return TestBitNumber(EGriffinBits::kIsSuppressed);
}

bool TGriffin::IsSuppressedAddbackSet() const
{
   return TestBitNumber(EGriffinBits::kIsSuppressedAddbackSet);
}

TGriffinHit* TGriffin::GetSuppressedHit(const int& i)
{
   try {
      if(!IsCrossTalkSet()) {
         FixCrossTalk();
      }
      return static_cast<TGriffinHit*>(fSuppressedHits.at(i));
   } catch(const std::out_of_range& oor) {
      std::cerr << ClassName() << " Suppressed hits are out of range: " << oor.what() << std::endl;
      if(!gInterpreter) {
         throw grsi::exit_exception(1);
      }
   }
   return nullptr;
}

Short_t TGriffin::GetSuppressedMultiplicity(const TBgo* bgo)
{
   /// Automatically builds the suppressed hits using the fSuppressionCriterion and returns the number of suppressed hits
   if(!IsCrossTalkSet()) {
      // Calculate Cross Talk on each hit
      FixCrossTalk();
   }
   auto& hitVector = Hits();
   if(hitVector.empty()) {
      return 0;
   }
   // if the suppressed has been reset, clear the suppressed hits
   if(!IsSuppressed()) {
      ResetSuppressed();
   }
   if(fSuppressedHits.empty()) {
      CreateSuppressed(bgo, hitVector, fSuppressedHits);
      SetSuppressed(true);
   }

   return fSuppressedHits.size();
}

void TGriffin::SetSuppressed(const bool flag) const
{
   SetBitNumber(EGriffinBits::kIsSuppressed, flag);
}

void TGriffin::ResetSuppressed()
{
   SetSuppressed(false);
   for(auto& hit : fSuppressedHits) {
      delete hit;
   }
   fSuppressedHits.clear();
}

TGriffinHit* TGriffin::GetSuppressedAddbackHit(const int& i)
{
   try {
      if(!IsCrossTalkSet()) {
         FixCrossTalk();
      }
      return static_cast<TGriffinHit*>(fSuppressedAddbackHits.at(i));
   } catch(const std::out_of_range& oor) {
      std::cerr << ClassName() << " Suppressed addback hits are out of range: " << oor.what() << std::endl;
      if(!gInterpreter) {
         throw grsi::exit_exception(1);
      }
   }
   return nullptr;
}

Short_t TGriffin::GetSuppressedAddbackMultiplicity(const TBgo* bgo)
{
   /// Automatically builds the suppressed addback hits using the fAddbackCriterion (if the size of the fAddbackHits vector is zero)
   /// and return the number of suppressed addback hits.
   if(!IsCrossTalkSet()) {
      // Calculate Cross Talk on each hit
      FixCrossTalk();
   }
   auto& hitVector = Hits();
   if(hitVector.empty()) {
      return 0;
   }
   // if the addback has been reset, clear the addback hits
   if(!IsSuppressedAddbackSet()) {
      ResetSuppressedAddback();
   }
   if(fSuppressedAddbackHits.empty()) {
      CreateSuppressedAddback(bgo, hitVector, fSuppressedAddbackHits, fSuppressedAddbackFrags);
      SetSuppressedAddback(true);
   }

   return fSuppressedAddbackHits.size();
}

void TGriffin::SetSuppressedAddback(const bool flag) const
{
   SetBitNumber(EGriffinBits::kIsSuppressedAddbackSet, flag);
}

void TGriffin::ResetSuppressedAddback()
{
   SetSuppressedAddback(false);
   for(auto& hit : fSuppressedAddbackHits) {
      delete hit;
   }
   fSuppressedAddbackHits.clear();
   fSuppressedAddbackFrags.clear();
}

UShort_t TGriffin::GetNSuppressedAddbackFrags(const size_t& idx)
{
   try {
      return fSuppressedAddbackFrags.at(idx);
   } catch(const std::out_of_range& oor) {
      std::cerr << ClassName() << " Suppressed addback frags are out of range: " << oor.what() << std::endl;
      if(!gInterpreter) {
         throw grsi::exit_exception(1);
      }
   }
   return 0;
}
