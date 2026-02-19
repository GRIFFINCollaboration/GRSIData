#include "TTigress.h"

#include <sstream>
#include <iostream>
#include <iomanip>

#include "TInterpreter.h"
#include "TMnemonic.h"

#include "TGRSIOptions.h"
#include "TSortingDiagnostics.h"

////////////////////////////////////////////////////////////
//
// TTigress
//
// The TTigress class defines the observables and algorithms used
// when analyzing TIGRESS data. It includes detector positions,
// add-back methods, etc.
//
////////////////////////////////////////////////////////////

bool DefaultTigressAddback(const TDetectorHit* one, const TDetectorHit* two)
{
   return ((one->GetDetector() == two->GetDetector()) &&
           (std::fabs(one->GetTime() - two->GetTime()) < TGRSIOptions::AnalysisOptions()->AddbackWindow()));
}

std::function<bool(const TDetectorHit*, const TDetectorHit*)> TTigress::fAddbackCriterion = DefaultTigressAddback;

bool DefaultTigressSuppression(const TDetectorHit* hit, const TDetectorHit* bgoHit)
{
   //return ((hit.GetDetector() == bgoHit.GetDetector() && hit.GetCrystal() == bgoHit.GetCrystal()) &&
   return ((hit->GetDetector() == bgoHit->GetDetector()) &&
           (std::fabs(hit->GetTime() - bgoHit->GetTime()) < TGRSIOptions::AnalysisOptions()->SuppressionWindow()) &&
           (bgoHit->GetEnergy() > TGRSIOptions::AnalysisOptions()->SuppressionEnergy()));
}

std::function<bool(const TDetectorHit*, const TDetectorHit*)> TTigress::fSuppressionCriterion = DefaultTigressSuppression;

double TTigress::fTargetOffset = 0.;
double TTigress::fRadialOffset = 0.;

TTransientBits<uint8_t> TTigress::fGlobalTigressBits(std::underlying_type_t<TTigress::ETigressGlobalBits>(TTigress::ETigressGlobalBits::kSetCoreWave));

TTigress::TTigress()
{
   /// Default ctor. Ignores TObjectStreamer in ROOT < 6
   Clear();
}

TTigress::TTigress(const TTigress& rhs) : TSuppressed(rhs)
{
   /// Copy ctor. Ignores TObjectStreamer in ROOT < 6
   rhs.Copy(*this);
}

void TTigress::Copy(TObject& rhs) const
{
   // Copy function.
   TSuppressed::Copy(rhs);

   // no need to copy hits, this is already taken care of by TDetector::Copy (called by TSuppressed::Copy)
   // not copying addback or suppressed vectors
   for(auto& hit : static_cast<TTigress&>(rhs).fAddbackHits) {
      delete hit;
   }
   for(auto& hit : static_cast<TTigress&>(rhs).fSuppressedHits) {
      delete hit;
   }
   for(auto& hit : static_cast<TTigress&>(rhs).fSuppressedAddbackHits) {
      delete hit;
   }
   static_cast<TTigress&>(rhs).fTigressBits = 0;
   static_cast<TTigress&>(rhs).fAddbackHits.clear();
   static_cast<TTigress&>(rhs).fAddbackFrags.clear();
   static_cast<TTigress&>(rhs).fSuppressedHits.clear();
   static_cast<TTigress&>(rhs).fSuppressedAddbackHits.clear();
   static_cast<TTigress&>(rhs).fSuppressedAddbackFrags.clear();
}

TTigress::~TTigress()
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

void TTigress::Clear(Option_t* opt)
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
}

void TTigress::Print(Option_t*) const
{
   Print(std::cout);
}

void TTigress::Print(std::ostream& out) const
{
   std::ostringstream str;
   str << "Tigress contains " << std::setw(6) << GetMultiplicity() << " hits:" << std::endl;
   for(const auto& hit : Hits()) {
      static_cast<TTigressHit*>(hit)->Print(str);
   }

   if(IsAddbackSet()) {
      str << std::setw(6) << fAddbackHits.size() << " addback hits" << std::endl;
   } else {
      str << std::setw(6) << " "
          << " Addback not set" << std::endl;
   }

   str << std::setw(6) << " "
       << " Cross-talk Set? " << IsCrossTalkSet() << std::endl;
   out << str.str();
}

TTigress& TTigress::operator=(const TTigress& rhs)
{
   rhs.Copy(*this);
   return *this;
}

bool TTigress::IsAddbackSet() const
{
   return TestBitNumber(ETigressBits::kIsAddbackSet);
}

bool TTigress::IsCrossTalkSet() const
{
   return TestBitNumber(ETigressBits::kIsCrossTalkSet);
}

void TTigress::SetAddback(const bool flag) const
{
   SetBitNumber(ETigressBits::kIsAddbackSet, flag);
}

void TTigress::SetCrossTalk(const bool flag) const
{
   SetBitNumber(ETigressBits::kIsCrossTalkSet, flag);
}

TTigressHit* TTigress::GetTigressHit(const int& i)
{
   /// Get Tigress hit indicated by index i.
   /// Throws an out of range exception if the index is out of the range of the hit vector.
   /// Applies cross-talk corrections if they haven't already been applied and are enabled.
   /// Note: This is different from using TDetector::GetHit and casting the result to a `TTigressHit*`
   /// as in that case no cross-talk corrections are applied.
   try {
      if(!IsCrossTalkSet()) {
         FixCrossTalk();
      }
      return static_cast<TTigressHit*>(Hits().at(i));
   } catch(const std::out_of_range& oor) {
      std::cerr << ClassName() << " Hits are out of range: " << oor.what() << std::endl;
      if(!gInterpreter) {
         throw grsi::exit_exception(1);
      }
   }
   return nullptr;
}

Short_t TTigress::GetAddbackMultiplicity()
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

TTigressHit* TTigress::GetAddbackHit(const int& i)
{
   if(i < GetAddbackMultiplicity()) {
      return static_cast<TTigressHit*>(fAddbackHits[i]);
   }
   std::cerr << "Addback hits are out of range" << std::endl;
   throw grsi::exit_exception(1);
   return nullptr;
}

void TTigress::BuildHits()
{
   // remove all hits of segments only
   // remove_if moves all elements to be removed to the end and returns an iterator to the first one to be removed
   auto remove = std::remove_if(Hits().begin(), Hits().end(), [](TDetectorHit* h) -> bool { return !(static_cast<TTigressHit*>(h)->CoreSet()); });
   // using remove_if the elements to be removed are left in an undefined state so we can only log how many we are removing!
   TSortingDiagnostics::Get()->RemovedHits(IsA(), std::distance(remove, Hits().end()), Hits().size());
   Hits().erase(remove, Hits().end());
   for(auto& hit : Hits()) {
      auto* tigressHit = static_cast<TTigressHit*>(hit);
      if(tigressHit->GetNSegments() > 1) {
         tigressHit->SortSegments();
      }

      if(tigressHit->HasWave() && TGRSIOptions::AnalysisOptions()->IsWaveformFitting()) {
         tigressHit->SetWavefit();
      }
   }
   std::sort(Hits().begin(), Hits().end());   // sorting an empty vector is fine, no need to check for that
}

void TTigress::AddFragment(const std::shared_ptr<const TFragment>& frag, TChannel* chan)
{
   /// Builds the TIGRESS Hits directly from the TFragment. Basically, loops through the hits for an event and sets
   /// observables.
   if(frag == nullptr || chan == nullptr) {
      return;
   }
   const TMnemonic* mnemonic = chan->GetMnemonic();
   if(mnemonic == nullptr) {
      std::cerr << "Trying to add fragment to TTigress w/o mnemonic in TChannel!" << std::endl;
      return;
   }

   if(mnemonic->SubSystem() != TMnemonic::EMnemonic::kG) {
      std::cerr << __PRETTY_FUNCTION__ << ": not a TIGRESS detector: '" << static_cast<std::underlying_type_t<TMnemonic::EMnemonic>>(mnemonic->SubSystem()) << "' from " << mnemonic << " = " << mnemonic->GetName() << std::endl;   // NOLINT(cppcoreguidelines-pro-bounds-array-to-pointer-decay)
      return;
   }

   // check whether we have a core (0 or 9) or a segment (any other number)
   if(chan->GetSegmentNumber() == 0 || chan->GetSegmentNumber() == 9) {
      // loop over existing hits to see if this core was already created by a previously found segment
      // of course this means if we have a core in "coincidence" with itself we will overwrite the first hit
      for(Short_t i = 0; i < GetMultiplicity(); ++i) {
         TTigressHit* hit = GetTigressHit(i);
         if((hit->GetDetector() == chan->GetDetectorNumber()) &&
            (hit->GetCrystal() == chan->GetCrystalNumber())) {   // we have a match;

            // B cores will not replace A cores, but they will replace no-core hits created if segments are processed first.
            if(chan->GetMnemonic()->OutputSensor() == TMnemonic::EMnemonic::kB) {
               TChannel* channel = hit->GetChannel();
               if(channel != nullptr && channel->GetMnemonic()->OutputSensor() == TMnemonic::EMnemonic::kA) {
                  return;
               }
            }

            hit->CopyFragment(*frag);
            hit->CoreSet(true);
            if(TestGlobalBit(ETigressGlobalBits::kSetCoreWave)) {
               frag->CopyWave(*hit);
            }
            return;
         }
      }
      // we haven't found this crystal in the existing hits, so we create a new one
      auto* hit = new TTigressHit(*frag);
      hit->CoreSet(true);
      if(TestGlobalBit(ETigressGlobalBits::kSetCoreWave)) {
         frag->CopyWave(*hit);
      }
      AddHit(hit);
      return;
   } else {
      // create a temporary segment hit (with waveform if requested)
      TDetectorHit temp(*frag);
      if(TestGlobalBit(ETigressGlobalBits::kSetSegWave)) {
         frag->CopyWave(temp);
      }
      // check if the crystal this segment belongs to already exists
      for(Short_t i = 0; i < GetMultiplicity(); ++i) {
         TTigressHit* hit = GetTigressHit(i);
         if((hit->GetDetector() == chan->GetDetectorNumber()) &&
            (hit->GetCrystal() == chan->GetCrystalNumber())) {   // we have a match;
            hit->AddSegment(temp);
            return;
         }
      }
      // haven't found matching crystal, so we create a new core hit with a fake address
      auto* corehit = new TTigressHit;
      corehit->SetAddress(frag->GetAddress());   // fake it till you make it
      corehit->AddSegment(temp);
      AddHit(corehit);
      return;
   }

   std::cout << ALERTTEXT << "failed to build!" << RESET_COLOR << std::endl;
   frag->Print();
}

void TTigress::ResetFlags() const
{
   fTigressBits = 0;
}

void TTigress::ResetAddback()
{
   SetAddback(false);
   SetCrossTalk(false);
   for(auto& hit : fAddbackHits) {
      delete hit;
   }
   fAddbackHits.clear();
   fAddbackFrags.clear();
}

UShort_t TTigress::GetNAddbackFrags(const size_t& idx)
{
   // Get the number of addback "fragments" contributing to the total addback hit
   // with index idx.
   if(idx < fAddbackFrags.size()) {
      return fAddbackFrags[idx];
   }
   return 0;
}

void TTigress::SetBitNumber(enum ETigressBits bit, Bool_t set) const
{
   // Used to set the flags that are stored in TTigress.
   fTigressBits.SetBit(bit, set);
}

Double_t TTigress::CTCorrectedEnergy(const TTigressHit* const hit_to_correct, const TTigressHit* const other_hit,
                                     Bool_t time_constraint)
{
   if((hit_to_correct == nullptr) || (other_hit == nullptr)) {
      std::cerr << "One of the hits is invalid in TTigress::CTCorrectedEnergy" << std::endl;
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

void TTigress::FixCrossTalk()
{
   if(!TGRSIOptions::AnalysisOptions()->IsCorrectingCrossTalk()) { return; }

   auto& hitVector = Hits();
   if(hitVector.size() < 2) {
      SetCrossTalk(true);
      return;
   }
   for(auto& hit : hitVector) {
      static_cast<TTigressHit*>(hit)->ClearEnergy();
   }

   for(auto& one : hitVector) {
      for(auto& two : hitVector) {
         one->SetEnergy(CTCorrectedEnergy(static_cast<TTigressHit*>(one), static_cast<TTigressHit*>(two)));
      }
   }
   SetCrossTalk(true);
}

const char* TTigress::GetColorFromNumber(int number)
{
   switch(number) {
   case(0): return "B";
   case(1): return "G";
   case(2): return "R";
   case(3): return "W";
   };
   return "X";
}

bool TTigress::IsSuppressed() const
{
   return TestBitNumber(ETigressBits::kIsSuppressed);
}

bool TTigress::IsSuppressedAddbackSet() const
{
   return TestBitNumber(ETigressBits::kIsSuppressedAddbackSet);
}

TTigressHit* TTigress::GetSuppressedHit(const int& i)
{
   try {
      if(!IsCrossTalkSet()) {
         FixCrossTalk();
      }
      return static_cast<TTigressHit*>(fSuppressedHits.at(i));
   } catch(const std::out_of_range& oor) {
      std::cerr << ClassName() << " Suppressed hits are out of range: " << oor.what() << std::endl;
      if(!gInterpreter) {
         throw grsi::exit_exception(1);
      }
   }
   return nullptr;
}

Short_t TTigress::GetSuppressedMultiplicity(const TBgo* bgo)
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

void TTigress::SetSuppressed(const bool flag) const
{
   SetBitNumber(ETigressBits::kIsSuppressed, flag);
}

void TTigress::ResetSuppressed()
{
   SetSuppressed(false);
   for(auto& hit : fSuppressedHits) {
      delete hit;
   }
   fSuppressedHits.clear();
}

TTigressHit* TTigress::GetSuppressedAddbackHit(const int& i)
{
   try {
      if(!IsCrossTalkSet()) {
         FixCrossTalk();
      }
      return static_cast<TTigressHit*>(fSuppressedAddbackHits.at(i));
   } catch(const std::out_of_range& oor) {
      std::cerr << ClassName() << " Suppressed addback hits are out of range: " << oor.what() << std::endl;
      if(!gInterpreter) {
         throw grsi::exit_exception(1);
      }
   }
   return nullptr;
}

Short_t TTigress::GetSuppressedAddbackMultiplicity(const TBgo* bgo)
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

void TTigress::SetSuppressedAddback(const bool flag) const
{
   SetBitNumber(ETigressBits::kIsSuppressedAddbackSet, flag);
}

void TTigress::ResetSuppressedAddback()
{
   SetSuppressedAddback(false);
   for(auto& hit : fSuppressedAddbackHits) {
      delete hit;
   }
   fSuppressedAddbackHits.clear();
   fSuppressedAddbackFrags.clear();
}

UShort_t TTigress::GetNSuppressedAddbackFrags(const size_t& idx)
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

TVector3 TTigress::GetPosition(const TTigressHit* hit, double dist, bool smear)
{
   return GetPosition(hit->GetDetector(), hit->GetCrystal(), hit->GetFirstSegment(), dist, smear);
}

TVector3 TTigress::GetPosition(int DetNbr, int CryNbr, int SegNbr, double dist, bool smear)
{
   if(!TestGlobalBit(ETigressGlobalBits::kVectorsBuilt)) { BuildVectors(); }

   // 0 - forward position, 1 - backward position
   int position = 0;
   if(dist > 0) {
      if(dist > 140.) { position = 1; }
   } else if(TestGlobalBit(ETigressGlobalBits::kArrayBackPos)) {
      position = 1;
   }

   if(smear && SegNbr == 0) {
      double x = 0.;
      double y = 0.;
      double r = sqrt(gRandom->Uniform(0, 400));
      gRandom->Circle(x, y, r);
      return fPositionVectors[position][DetNbr][CryNbr][SegNbr] + fCloverCross[DetNbr][0] * x + fCloverCross[DetNbr][1] * y;
   }

   return fPositionVectors[position][DetNbr][CryNbr][SegNbr];
}

void TTigress::BuildVectors()
{
   for(int position = 0; position < 2; position++) {
      for(int DetNbr = 0; DetNbr < 17; DetNbr++) {
         for(int CryNbr = 0; CryNbr < 4; CryNbr++) {
            for(int SegNbr = 0; SegNbr < 9; SegNbr++) {
               TVector3 detectorPosition;
               double   xx = 0.;
               double   yy = 0.;
               double   zz = 0.;

               if(position == 1) {   // distance=145.0
                  switch(CryNbr) {
                  case 0:
                     xx = fGeBluePositionBack[DetNbr][SegNbr][0];
                     yy = fGeBluePositionBack[DetNbr][SegNbr][1];
                     zz = fGeBluePositionBack[DetNbr][SegNbr][2];
                     break;
                  case 1:
                     xx = fGeGreenPositionBack[DetNbr][SegNbr][0];
                     yy = fGeGreenPositionBack[DetNbr][SegNbr][1];
                     zz = fGeGreenPositionBack[DetNbr][SegNbr][2];
                     break;
                  case 2:
                     xx = fGeRedPositionBack[DetNbr][SegNbr][0];
                     yy = fGeRedPositionBack[DetNbr][SegNbr][1];
                     zz = fGeRedPositionBack[DetNbr][SegNbr][2];
                     break;
                  case 3:
                     xx = fGeWhitePositionBack[DetNbr][SegNbr][0];
                     yy = fGeWhitePositionBack[DetNbr][SegNbr][1];
                     zz = fGeWhitePositionBack[DetNbr][SegNbr][2];
                     break;
                  };
               } else {
                  switch(CryNbr) {
                  case 0:
                     xx = fGeBluePosition[DetNbr][SegNbr][0];
                     yy = fGeBluePosition[DetNbr][SegNbr][1];
                     zz = fGeBluePosition[DetNbr][SegNbr][2];
                     break;
                  case 1:
                     xx = fGeGreenPosition[DetNbr][SegNbr][0];
                     yy = fGeGreenPosition[DetNbr][SegNbr][1];
                     zz = fGeGreenPosition[DetNbr][SegNbr][2];
                     break;
                  case 2:
                     xx = fGeRedPosition[DetNbr][SegNbr][0];
                     yy = fGeRedPosition[DetNbr][SegNbr][1];
                     zz = fGeRedPosition[DetNbr][SegNbr][2];
                     break;
                  case 3:
                     xx = fGeWhitePosition[DetNbr][SegNbr][0];
                     yy = fGeWhitePosition[DetNbr][SegNbr][1];
                     zz = fGeWhitePosition[DetNbr][SegNbr][2];
                     break;
                  };
               }

               detectorPosition.SetXYZ(xx, yy, zz - fTargetOffset);

               if(fRadialOffset != 0.) {
                  detectorPosition += fCloverRadial[DetNbr].Unit() * fRadialOffset;
               }

               fPositionVectors[position][DetNbr][CryNbr][SegNbr] = detectorPosition;
            }
         }
      }
   }

   for(int DetNbr = 0; DetNbr < 17; DetNbr++) {
      TVector3 a(-fCloverRadial[DetNbr].Y(), fCloverRadial[DetNbr].X(), 0);
      TVector3 b              = fCloverRadial[DetNbr].Cross(a);
      fCloverCross[DetNbr][0] = a.Unit();
      fCloverCross[DetNbr][1] = b.Unit();
   }

   SetGlobalBit(ETigressGlobalBits::kVectorsBuilt, true);
}

std::array<std::array<std::array<std::array<TVector3, 9>, 4>, 17>, 2> TTigress::fPositionVectors;
std::array<std::array<TVector3, 2>, 17>                               TTigress::fCloverCross;

std::array<TVector3, 17> TTigress::fCloverRadial = {TVector3(0., 0., 0.),
                                                    TVector3(0.9239, 0.3827, 1.),
                                                    TVector3(-0.3827, 0.9239, 1.),
                                                    TVector3(-0.9239, -0.3827, 1.),
                                                    TVector3(0.3827, -0.9239, 1.),
                                                    TVector3(0.9239, 0.3827, 0.),
                                                    TVector3(0.3827, 0.9239, 0.),
                                                    TVector3(-0.3827, 0.9239, 0.),
                                                    TVector3(-0.9239, 0.3827, 0.),
                                                    TVector3(-0.9239, -0.3827, 0.),
                                                    TVector3(-0.3827, -0.9239, 0.),
                                                    TVector3(0.3827, -0.9239, 0.),
                                                    TVector3(0.9239, -0.3827, 0.),
                                                    TVector3(0.9239, 0.3827, -1.),
                                                    TVector3(-0.3827, 0.9239, -1.),
                                                    TVector3(-0.9239, -0.3827, -1.),
                                                    TVector3(0.3827, -0.9239, -1.)};

std::array<std::array<std::array<double, 3>, 9>, 17> TTigress::fGeBluePosition = {{{{{0., 0., 0.},
                                                                                     {0., 0., 0.},
                                                                                     {0., 0., 0.},
                                                                                     {0., 0., 0.},
                                                                                     {0., 0., 0.},
                                                                                     {0., 0., 0.},
                                                                                     {0., 0., 0.},
                                                                                     {0., 0., 0.},
                                                                                     {0., 0., 0.}}},
                                                                                   {{{78.05, 61.70, 134.09},
                                                                                     {47.83, 59.64, 119.06},
                                                                                     {63.65, 65.78, 102.12},
                                                                                     {72.14, 44.72, 103.15},
                                                                                     {57.26, 37.61, 118.80},
                                                                                     {72.73, 73.96, 152.77},
                                                                                     {89.31, 81.09, 134.70},
                                                                                     {99.39, 57.11, 134.51},
                                                                                     {82.33, 50.30, 152.93}}},
                                                                                   {{{-61.70, 78.05, 134.09},
                                                                                     {-59.64, 47.83, 119.06},
                                                                                     {-65.78, 63.65, 102.12},
                                                                                     {-44.72, 72.14, 103.15},
                                                                                     {-37.61, 57.26, 118.80},
                                                                                     {-73.96, 72.73, 152.77},
                                                                                     {-81.09, 89.31, 134.70},
                                                                                     {-57.11, 99.39, 134.51},
                                                                                     {-50.30, 82.33, 152.93}}},
                                                                                   {{{-78.05, -61.70, 134.09},
                                                                                     {-47.83, -59.64, 119.06},
                                                                                     {-63.65, -65.78, 102.12},
                                                                                     {-72.14, -44.72, 103.15},
                                                                                     {-57.26, -37.61, 118.80},
                                                                                     {-72.73, -73.96, 152.77},
                                                                                     {-89.31, -81.09, 134.70},
                                                                                     {-99.39, -57.11, 134.51},
                                                                                     {-82.33, -50.30, 152.93}}},
                                                                                   {{{61.70, -78.05, 134.09},
                                                                                     {59.64, -47.83, 119.06},
                                                                                     {65.78, -63.65, 102.12},
                                                                                     {44.72, -72.14, 103.15},
                                                                                     {37.61, -57.26, 118.80},
                                                                                     {73.96, -72.73, 152.77},
                                                                                     {81.09, -89.31, 134.70},
                                                                                     {57.11, -99.39, 134.51},
                                                                                     {50.30, -82.33, 152.93}}},
                                                                                   {{{139.75, 87.25, 27.13},
                                                                                     {107.47, 84.35, 36.80},
                                                                                     {107.64, 84.01, 12.83},
                                                                                     {116.86, 63.25, 13.71},
                                                                                     {116.66, 62.21, 36.42},
                                                                                     {146.69, 104.60, 40.50},
                                                                                     {146.58, 104.81, 14.96},
                                                                                     {156.50, 80.77, 14.73},
                                                                                     {156.44, 80.99, 40.74}}},
                                                                                   {{{37.12, 160.51, 27.13},
                                                                                     {16.35, 135.64, 36.80},
                                                                                     {16.71, 135.51, 12.83},
                                                                                     {37.91, 127.36, 13.71},
                                                                                     {38.50, 126.48, 36.42},
                                                                                     {29.76, 177.69, 40.50},
                                                                                     {29.53, 177.76, 14.96},
                                                                                     {53.55, 167.78, 14.73},
                                                                                     {53.35, 167.89, 40.74}}},
                                                                                   {{{-87.25, 139.75, 27.13},
                                                                                     {-84.35, 107.47, 36.80},
                                                                                     {-84.01, 107.64, 12.83},
                                                                                     {-63.25, 116.86, 13.71},
                                                                                     {-62.21, 116.66, 36.42},
                                                                                     {-104.60, 146.69, 40.50},
                                                                                     {-104.81, 146.58, 14.96},
                                                                                     {-80.77, 156.50, 14.73},
                                                                                     {-80.99, 156.44, 40.74}}},
                                                                                   {{{-160.51, 37.12, 27.13},
                                                                                     {-135.64, 16.35, 36.80},
                                                                                     {-135.51, 16.71, 12.83},
                                                                                     {-127.36, 37.91, 13.71},
                                                                                     {-126.48, 38.50, 36.42},
                                                                                     {-177.69, 29.76, 40.50},
                                                                                     {-177.76, 29.53, 14.96},
                                                                                     {-167.78, 53.55, 14.73},
                                                                                     {-167.89, 53.35, 40.74}}},
                                                                                   {{{-139.75, -87.25, 27.13},
                                                                                     {-107.47, -84.35, 36.80},
                                                                                     {-107.64, -84.01, 12.83},
                                                                                     {-116.86, -63.25, 13.71},
                                                                                     {-116.66, -62.21, 36.42},
                                                                                     {-146.69, -104.60, 40.50},
                                                                                     {-146.58, -104.81, 14.96},
                                                                                     {-156.50, -80.77, 14.73},
                                                                                     {-156.44, -80.99, 40.74}}},
                                                                                   {{{-37.12, -160.51, 27.13},
                                                                                     {-16.35, -135.64, 36.80},
                                                                                     {-16.71, -135.51, 12.83},
                                                                                     {-37.91, -127.36, 13.71},
                                                                                     {-38.50, -126.48, 36.42},
                                                                                     {-29.76, -177.69, 40.50},
                                                                                     {-29.53, -177.76, 14.96},
                                                                                     {-53.55, -167.78, 14.73},
                                                                                     {-53.35, -167.89, 40.74}}},
                                                                                   {{{87.25, -139.75, 27.13},
                                                                                     {84.35, -107.47, 36.80},
                                                                                     {84.01, -107.64, 12.83},
                                                                                     {63.25, -116.86, 13.71},
                                                                                     {62.21, -116.66, 36.42},
                                                                                     {104.60, -146.69, 40.50},
                                                                                     {104.81, -146.58, 14.96},
                                                                                     {80.77, -156.50, 14.73},
                                                                                     {80.99, -156.44, 40.74}}},
                                                                                   {{{160.51, -37.12, 27.13},
                                                                                     {135.64, -16.35, 36.80},
                                                                                     {135.51, -16.71, 12.83},
                                                                                     {127.36, -37.91, 13.71},
                                                                                     {126.48, -38.50, 36.42},
                                                                                     {177.69, -29.76, 40.50},
                                                                                     {177.76, -29.53, 14.96},
                                                                                     {167.78, -53.55, 14.73},
                                                                                     {167.89, -53.35, 40.74}}},
                                                                                   {{{113.50, 76.38, -95.72},
                                                                                     {95.91, 79.56, -67.01},
                                                                                     {80.41, 72.73, -83.98},
                                                                                     {90.05, 52.14, -83.76},
                                                                                     {104.85, 57.32, -67.30},
                                                                                     {125.64, 95.88, -95.49},
                                                                                     {108.85, 89.19, -113.54},
                                                                                     {118.64, 65.08, -113.68},
                                                                                     {135.56, 72.34, -95.31}}},
                                                                                   {{{-76.38, 113.50, -95.72},
                                                                                     {-79.56, 95.91, -67.01},
                                                                                     {-72.73, 80.41, -83.98},
                                                                                     {-52.14, 90.05, -83.76},
                                                                                     {-57.32, 104.85, -67.30},
                                                                                     {-95.88, 125.64, -95.49},
                                                                                     {-89.19, 108.85, -113.54},
                                                                                     {-65.08, 118.64, -113.68},
                                                                                     {-72.34, 135.56, -95.31}}},
                                                                                   {{{-113.50, -76.38, -95.72},
                                                                                     {-95.91, -79.56, -67.01},
                                                                                     {-80.41, -72.73, -83.98},
                                                                                     {-90.05, -52.14, -83.76},
                                                                                     {-104.85, -57.32, -67.30},
                                                                                     {-125.64, -95.88, -95.49},
                                                                                     {-108.85, -89.19, -113.54},
                                                                                     {-118.64, -65.08, -113.68},
                                                                                     {-135.56, -72.34, -95.31}}},
                                                                                   {{{76.38, -113.50, -95.72},
                                                                                     {79.56, -95.91, -67.01},
                                                                                     {72.73, -80.41, -83.98},
                                                                                     {52.14, -90.05, -83.76},
                                                                                     {57.32, -104.85, -67.30},
                                                                                     {95.88, -125.64, -95.49},
                                                                                     {89.19, -108.85, -113.54},
                                                                                     {65.08, -118.64, -113.68},
                                                                                     {72.34, -135.56, -95.31}}}}};

// Assuming this is the 1
std::array<std::array<std::array<double, 3>, 9>, 17> TTigress::fGeGreenPosition = {{{{{0., 0., 0.},
                                                                                      {0., 0., 0.},
                                                                                      {0., 0., 0.},
                                                                                      {0., 0., 0.},
                                                                                      {0., 0., 0.},
                                                                                      {0., 0., 0.},
                                                                                      {0., 0., 0.},
                                                                                      {0., 0., 0.},
                                                                                      {0., 0., 0.}}},
                                                                                    {{{113.50, 76.38, 95.72},
                                                                                      {95.91, 79.56, 67.01},
                                                                                      {104.85, 57.32, 67.30},
                                                                                      {90.05, 52.14, 83.76},
                                                                                      {80.41, 72.73, 83.98},
                                                                                      {125.64, 95.88, 95.49},
                                                                                      {135.56, 72.34, 95.31},
                                                                                      {118.64, 65.08, 113.68},
                                                                                      {108.85, 89.19, 113.54}}},
                                                                                    {{{-76.38, 113.50, 95.72},
                                                                                      {-79.56, 95.91, 67.01},
                                                                                      {-57.32, 104.85, 67.30},
                                                                                      {-52.14, 90.05, 83.76},
                                                                                      {-72.73, 80.41, 83.98},
                                                                                      {-95.88, 125.64, 95.49},
                                                                                      {-72.34, 135.56, 95.31},
                                                                                      {-65.08, 118.64, 113.68},
                                                                                      {-89.19, 108.85, 113.54}}},
                                                                                    {{{-113.50, -76.38, 95.72},
                                                                                      {-95.91, -79.56, 67.01},
                                                                                      {-104.85, -57.32, 67.30},
                                                                                      {-90.05, -52.14, 83.76},
                                                                                      {-80.41, -72.73, 83.98},
                                                                                      {-125.64, -95.88, 95.49},
                                                                                      {-135.56, -72.34, 95.31},
                                                                                      {-118.64, -65.08, 113.68},
                                                                                      {-108.85, -89.19, 113.54}}},
                                                                                    {{{76.38, -113.50, 95.72},
                                                                                      {79.56, -95.91, 67.01},
                                                                                      {57.32, -104.85, 67.30},
                                                                                      {52.14, -90.05, 83.76},
                                                                                      {72.73, -80.41, 83.98},
                                                                                      {95.88, -125.64, 95.49},
                                                                                      {72.34, -135.56, 95.31},
                                                                                      {65.08, -118.64, 113.68},
                                                                                      {89.19, -108.85, 113.54}}},
                                                                                    {{{139.75, 87.25, -27.13},
                                                                                      {107.47, 84.35, -36.80},
                                                                                      {116.66, 62.21, -36.42},
                                                                                      {116.86, 63.25, -13.71},
                                                                                      {107.64, 84.01, -12.83},
                                                                                      {146.69, 104.60, -40.50},
                                                                                      {156.44, 80.99, -40.74},
                                                                                      {156.50, 80.77, -14.73},
                                                                                      {146.58, 104.81, -14.96}}},
                                                                                    {{{37.12, 160.51, -27.13},
                                                                                      {16.35, 135.64, -36.80},
                                                                                      {38.50, 126.48, -36.42},
                                                                                      {37.91, 127.36, -13.71},
                                                                                      {16.71, 135.51, -12.83},
                                                                                      {29.76, 177.69, -40.50},
                                                                                      {53.35, 167.89, -40.74},
                                                                                      {53.55, 167.78, -14.73},
                                                                                      {29.53, 177.76, -14.96}}},
                                                                                    {{{-87.25, 139.75, -27.13},
                                                                                      {-84.35, 107.47, -36.80},
                                                                                      {-62.21, 116.66, -36.42},
                                                                                      {-63.25, 116.86, -13.71},
                                                                                      {-84.01, 107.64, -12.83},
                                                                                      {-104.60, 146.69, -40.50},
                                                                                      {-80.99, 156.44, -40.74},
                                                                                      {-80.77, 156.50, -14.73},
                                                                                      {-104.81, 146.58, -14.96}}},
                                                                                    {{{-160.51, 37.12, -27.13},
                                                                                      {-135.64, 16.35, -36.80},
                                                                                      {-126.48, 38.50, -36.42},
                                                                                      {-127.36, 37.91, -13.71},
                                                                                      {-135.51, 16.71, -12.83},
                                                                                      {-177.69, 29.76, -40.50},
                                                                                      {-167.89, 53.35, -40.74},
                                                                                      {-167.78, 53.55, -14.73},
                                                                                      {-177.76, 29.53, -14.96}}},
                                                                                    {{{-139.75, -87.25, -27.13},
                                                                                      {-107.47, -84.35, -36.80},
                                                                                      {-116.66, -62.21, -36.42},
                                                                                      {-116.86, -63.25, -13.71},
                                                                                      {-107.64, -84.01, -12.83},
                                                                                      {-146.69, -104.60, -40.50},
                                                                                      {-156.44, -80.99, -40.74},
                                                                                      {-156.50, -80.77, -14.73},
                                                                                      {-146.58, -104.81, -14.96}}},
                                                                                    {{{-37.12, -160.51, -27.13},
                                                                                      {-16.35, -135.64, -36.80},
                                                                                      {-38.50, -126.48, -36.42},
                                                                                      {-37.91, -127.36, -13.71},
                                                                                      {-16.71, -135.51, -12.83},
                                                                                      {-29.76, -177.69, -40.50},
                                                                                      {-53.35, -167.89, -40.74},
                                                                                      {-53.55, -167.78, -14.73},
                                                                                      {-29.53, -177.76, -14.96}}},
                                                                                    {{{87.25, -139.75, -27.13},
                                                                                      {84.35, -107.47, -36.80},
                                                                                      {62.21, -116.66, -36.42},
                                                                                      {63.25, -116.86, -13.71},
                                                                                      {84.01, -107.64, -12.83},
                                                                                      {104.60, -146.69, -40.50},
                                                                                      {80.99, -156.44, -40.74},
                                                                                      {80.77, -156.50, -14.73},
                                                                                      {104.81, -146.58, -14.96}}},
                                                                                    {{{160.51, -37.12, -27.13},
                                                                                      {135.64, -16.35, -36.80},
                                                                                      {126.48, -38.50, -36.42},
                                                                                      {127.36, -37.91, -13.71},
                                                                                      {135.51, -16.71, -12.83},
                                                                                      {177.69, -29.76, -40.50},
                                                                                      {167.89, -53.35, -40.74},
                                                                                      {167.78, -53.55, -14.73},
                                                                                      {177.76, -29.53, -14.96}}},
                                                                                    {{{78.05, 61.70, -134.09},
                                                                                      {47.83, 59.64, -119.06},
                                                                                      {57.26, 37.61, -118.80},
                                                                                      {72.14, 44.72, -103.15},
                                                                                      {63.65, 65.78, -102.12},
                                                                                      {72.73, 73.96, -152.77},
                                                                                      {82.33, 50.30, -152.93},
                                                                                      {99.39, 57.11, -134.51},
                                                                                      {89.31, 81.09, -134.70}}},
                                                                                    {{{-61.70, 78.05, -134.09},
                                                                                      {-59.64, 47.83, -119.06},
                                                                                      {-37.61, 57.26, -118.80},
                                                                                      {-44.72, 72.14, -103.15},
                                                                                      {-65.78, 63.65, -102.12},
                                                                                      {-73.96, 72.73, -152.77},
                                                                                      {-50.30, 82.33, -152.93},
                                                                                      {-57.11, 99.39, -134.51},
                                                                                      {-81.09, 89.31, -134.70}}},
                                                                                    {{{-78.05, -61.70, -134.09},
                                                                                      {-47.83, -59.64, -119.06},
                                                                                      {-57.26, -37.61, -118.80},
                                                                                      {-72.14, -44.72, -103.15},
                                                                                      {-63.65, -65.78, -102.12},
                                                                                      {-72.73, -73.96, -152.77},
                                                                                      {-82.33, -50.30, -152.93},
                                                                                      {-99.39, -57.11, -134.51},
                                                                                      {-89.31, -81.09, -134.70}}},
                                                                                    {{{61.70, -78.05, -134.09},
                                                                                      {59.64, -47.83, -119.06},
                                                                                      {37.61, -57.26, -118.80},
                                                                                      {44.72, -72.14, -103.15},
                                                                                      {65.78, -63.65, -102.12},
                                                                                      {73.96, -72.73, -152.77},
                                                                                      {50.30, -82.33, -152.93},
                                                                                      {57.11, -99.39, -134.51},
                                                                                      {81.09, -89.31, -134.70}}}}};

// Assuming this is the 2
std::array<std::array<std::array<double, 3>, 9>, 17> TTigress::fGeRedPosition = {{{{{0., 0., 0.},
                                                                                    {0., 0., 0.},
                                                                                    {0., 0., 0.},
                                                                                    {0., 0., 0.},
                                                                                    {0., 0., 0.},
                                                                                    {0., 0., 0.},
                                                                                    {0., 0., 0.},
                                                                                    {0., 0., 0.},
                                                                                    {0., 0., 0.}}},
                                                                                  {{{134.26, 26.25, 95.72},
                                                                                    {124.08, 11.56, 67.01},
                                                                                    {108.28, 5.43, 83.98},
                                                                                    {100.55, 26.81, 83.76},
                                                                                    {114.67, 33.61, 67.30},
                                                                                    {156.64, 21.05, 95.49},
                                                                                    {140.03, 13.91, 113.54},
                                                                                    {129.91, 37.87, 113.68},
                                                                                    {147.01, 44.70, 95.31}}},
                                                                                  {{{-26.25, 134.26, 95.72},
                                                                                    {-11.56, 124.08, 67.01},
                                                                                    {-5.43, 108.28, 83.98},
                                                                                    {-26.81, 100.55, 83.76},
                                                                                    {-33.61, 114.67, 67.30},
                                                                                    {-21.05, 156.64, 95.49},
                                                                                    {-13.91, 140.03, 113.54},
                                                                                    {-37.87, 129.91, 113.68},
                                                                                    {-44.70, 147.01, 95.31}}},
                                                                                  {{{-134.26, -26.25, 95.72},
                                                                                    {-124.08, -11.56, 67.01},
                                                                                    {-108.28, -5.43, 83.98},
                                                                                    {-100.55, -26.81, 83.76},
                                                                                    {-114.67, -33.61, 67.30},
                                                                                    {-156.64, -21.05, 95.49},
                                                                                    {-140.03, -13.91, 113.54},
                                                                                    {-129.91, -37.87, 113.68},
                                                                                    {-147.01, -44.70, 95.31}}},
                                                                                  {{{26.25, -134.26, 95.72},
                                                                                    {11.56, -124.08, 67.01},
                                                                                    {5.43, -108.28, 83.98},
                                                                                    {26.81, -100.55, 83.76},
                                                                                    {33.61, -114.67, 67.30},
                                                                                    {21.05, -156.64, 95.49},
                                                                                    {13.91, -140.03, 113.54},
                                                                                    {37.87, -129.91, 113.68},
                                                                                    {44.70, -147.01, 95.31}}},
                                                                                  {{{160.51, 37.12, -27.13},
                                                                                    {135.64, 16.35, -36.80},
                                                                                    {135.51, 16.71, -12.83},
                                                                                    {127.36, 37.91, -13.71},
                                                                                    {126.48, 38.50, -36.42},
                                                                                    {177.69, 29.76, -40.50},
                                                                                    {177.76, 29.53, -14.96},
                                                                                    {167.78, 53.55, -14.73},
                                                                                    {167.89, 53.35, -40.74}}},
                                                                                  {{{87.25, 139.75, -27.13},
                                                                                    {84.35, 107.47, -36.80},
                                                                                    {84.01, 107.64, -12.83},
                                                                                    {63.25, 116.86, -13.71},
                                                                                    {62.21, 116.66, -36.42},
                                                                                    {104.60, 146.69, -40.50},
                                                                                    {104.81, 146.58, -14.96},
                                                                                    {80.77, 156.50, -14.73},
                                                                                    {80.99, 156.44, -40.74}}},
                                                                                  {{{-37.12, 160.51, -27.13},
                                                                                    {-16.35, 135.64, -36.80},
                                                                                    {-16.71, 135.51, -12.83},
                                                                                    {-37.91, 127.36, -13.71},
                                                                                    {-38.50, 126.48, -36.42},
                                                                                    {-29.76, 177.69, -40.50},
                                                                                    {-29.53, 177.76, -14.96},
                                                                                    {-53.55, 167.78, -14.73},
                                                                                    {-53.35, 167.89, -40.74}}},
                                                                                  {{{-139.75, 87.25, -27.13},
                                                                                    {-107.47, 84.35, -36.80},
                                                                                    {-107.64, 84.01, -12.83},
                                                                                    {-116.86, 63.25, -13.71},
                                                                                    {-116.66, 62.21, -36.42},
                                                                                    {-146.69, 104.60, -40.50},
                                                                                    {-146.58, 104.81, -14.96},
                                                                                    {-156.50, 80.77, -14.73},
                                                                                    {-156.44, 80.99, -40.74}}},
                                                                                  {{{-160.51, -37.12, -27.13},
                                                                                    {-135.64, -16.35, -36.80},
                                                                                    {-135.51, -16.71, -12.83},
                                                                                    {-127.36, -37.91, -13.71},
                                                                                    {-126.48, -38.50, -36.42},
                                                                                    {-177.69, -29.76, -40.50},
                                                                                    {-177.76, -29.53, -14.96},
                                                                                    {-167.78, -53.55, -14.73},
                                                                                    {-167.89, -53.35, -40.74}}},
                                                                                  {{{-87.25, -139.75, -27.13},
                                                                                    {-84.35, -107.47, -36.80},
                                                                                    {-84.01, -107.64, -12.83},
                                                                                    {-63.25, -116.86, -13.71},
                                                                                    {-62.21, -116.66, -36.42},
                                                                                    {-104.60, -146.69, -40.50},
                                                                                    {-104.81, -146.58, -14.96},
                                                                                    {-80.77, -156.50, -14.73},
                                                                                    {-80.99, -156.44, -40.74}}},
                                                                                  {{{37.12, -160.51, -27.13},
                                                                                    {16.35, -135.64, -36.80},
                                                                                    {16.71, -135.51, -12.83},
                                                                                    {37.91, -127.36, -13.71},
                                                                                    {38.50, -126.48, -36.42},
                                                                                    {29.76, -177.69, -40.50},
                                                                                    {29.53, -177.76, -14.96},
                                                                                    {53.55, -167.78, -14.73},
                                                                                    {53.35, -167.89, -40.74}}},
                                                                                  {{{139.75, -87.25, -27.13},
                                                                                    {107.47, -84.35, -36.80},
                                                                                    {107.64, -84.01, -12.83},
                                                                                    {116.86, -63.25, -13.71},
                                                                                    {116.66, -62.21, -36.42},
                                                                                    {146.69, -104.60, -40.50},
                                                                                    {146.58, -104.81, -14.96},
                                                                                    {156.50, -80.77, -14.73},
                                                                                    {156.44, -80.99, -40.74}}},
                                                                                  {{{98.82, 11.57, -134.09},
                                                                                    {75.99, -8.35, -119.06},
                                                                                    {91.52, -1.51, -102.12},
                                                                                    {82.63, 19.39, -103.15},
                                                                                    {67.08, 13.90, -118.80},
                                                                                    {103.72, -0.87, -152.77},
                                                                                    {120.49, 5.81, -134.70},
                                                                                    {110.66, 29.90, -134.51},
                                                                                    {93.78, 22.65, -152.93}}},
                                                                                  {{{-11.57, 98.82, -134.09},
                                                                                    {8.35, 75.99, -119.06},
                                                                                    {1.51, 91.52, -102.12},
                                                                                    {-19.39, 82.63, -103.15},
                                                                                    {-13.90, 67.08, -118.80},
                                                                                    {0.87, 103.72, -152.77},
                                                                                    {-5.81, 120.49, -134.70},
                                                                                    {-29.90, 110.66, -134.51},
                                                                                    {-22.65, 93.78, -152.93}}},
                                                                                  {{{-98.82, -11.57, -134.09},
                                                                                    {-75.99, 8.35, -119.06},
                                                                                    {-91.52, 1.51, -102.12},
                                                                                    {-82.63, -19.39, -103.15},
                                                                                    {-67.08, -13.90, -118.80},
                                                                                    {-103.72, 0.87, -152.77},
                                                                                    {-120.49, -5.81, -134.70},
                                                                                    {-110.66, -29.90, -134.51},
                                                                                    {-93.78, -22.65, -152.93}}},
                                                                                  {{{11.57, -98.82, -134.09},
                                                                                    {-8.35, -75.99, -119.06},
                                                                                    {-1.51, -91.52, -102.12},
                                                                                    {19.39, -82.63, -103.15},
                                                                                    {13.90, -67.08, -118.80},
                                                                                    {-0.87, -103.72, -152.77},
                                                                                    {5.81, -120.49, -134.70},
                                                                                    {29.90, -110.66, -134.51},
                                                                                    {22.65, -93.78, -152.93}}}}};

// Assuming this is the 3
std::array<std::array<std::array<double, 3>, 9>, 17> TTigress::fGeWhitePosition = {{{{{0., 0., 0.},
                                                                                      {0., 0., 0.},
                                                                                      {0., 0., 0.},
                                                                                      {0., 0., 0.},
                                                                                      {0., 0., 0.},
                                                                                      {0., 0., 0.},
                                                                                      {0., 0., 0.},
                                                                                      {0., 0., 0.},
                                                                                      {0., 0., 0.}}},
                                                                                    {{{98.82, 11.57, 134.09},
                                                                                      {75.99, -8.35, 119.06},
                                                                                      {67.08, 13.90, 118.80},
                                                                                      {82.63, 19.39, 103.15},
                                                                                      {91.52, -1.51, 102.12},
                                                                                      {103.72, -0.87, 152.77},
                                                                                      {93.78, 22.65, 152.93},
                                                                                      {110.66, 29.90, 134.51},
                                                                                      {120.49, 5.81, 134.70}}},
                                                                                    {{{-11.57, 98.82, 134.09},
                                                                                      {8.35, 75.99, 119.06},
                                                                                      {-13.90, 67.08, 118.80},
                                                                                      {-19.39, 82.63, 103.15},
                                                                                      {1.51, 91.52, 102.12},
                                                                                      {0.87, 103.72, 152.77},
                                                                                      {-22.65, 93.78, 152.93},
                                                                                      {-29.90, 110.66, 134.51},
                                                                                      {-5.81, 120.49, 134.70}}},
                                                                                    {{{-98.82, -11.57, 134.09},
                                                                                      {-75.99, 8.35, 119.06},
                                                                                      {-67.08, -13.90, 118.80},
                                                                                      {-82.63, -19.39, 103.15},
                                                                                      {-91.52, 1.51, 102.12},
                                                                                      {-103.72, 0.87, 152.77},
                                                                                      {-93.78, -22.65, 152.93},
                                                                                      {-110.66, -29.90, 134.51},
                                                                                      {-120.49, -5.81, 134.70}}},
                                                                                    {{{11.57, -98.82, 134.09},
                                                                                      {-8.35, -75.99, 119.06},
                                                                                      {13.90, -67.08, 118.80},
                                                                                      {19.39, -82.63, 103.15},
                                                                                      {-1.51, -91.52, 102.12},
                                                                                      {-0.87, -103.72, 152.77},
                                                                                      {22.65, -93.78, 152.93},
                                                                                      {29.90, -110.66, 134.51},
                                                                                      {5.81, -120.49, 134.70}}},
                                                                                    {{{160.51, 37.12, 27.13},
                                                                                      {135.64, 16.35, 36.80},
                                                                                      {126.48, 38.50, 36.42},
                                                                                      {127.36, 37.91, 13.71},
                                                                                      {135.51, 16.71, 12.83},
                                                                                      {177.69, 29.76, 40.50},
                                                                                      {167.89, 53.35, 40.74},
                                                                                      {167.78, 53.55, 14.73},
                                                                                      {177.76, 29.53, 14.96}}},
                                                                                    {{{87.25, 139.75, 27.13},
                                                                                      {84.35, 107.47, 36.80},
                                                                                      {62.21, 116.66, 36.42},
                                                                                      {63.25, 116.86, 13.71},
                                                                                      {84.01, 107.64, 12.83},
                                                                                      {104.60, 146.69, 40.50},
                                                                                      {80.99, 156.44, 40.74},
                                                                                      {80.77, 156.50, 14.73},
                                                                                      {104.81, 146.58, 14.96}}},
                                                                                    {{{-37.12, 160.51, 27.13},
                                                                                      {-16.35, 135.64, 36.80},
                                                                                      {-38.50, 126.48, 36.42},
                                                                                      {-37.91, 127.36, 13.71},
                                                                                      {-16.71, 135.51, 12.83},
                                                                                      {-29.76, 177.69, 40.50},
                                                                                      {-53.35, 167.89, 40.74},
                                                                                      {-53.55, 167.78, 14.73},
                                                                                      {-29.53, 177.76, 14.96}}},
                                                                                    {{{-139.75, 87.25, 27.13},
                                                                                      {-107.47, 84.35, 36.80},
                                                                                      {-116.66, 62.21, 36.42},
                                                                                      {-116.86, 63.25, 13.71},
                                                                                      {-107.64, 84.01, 12.83},
                                                                                      {-146.69, 104.60, 40.50},
                                                                                      {-156.44, 80.99, 40.74},
                                                                                      {-156.50, 80.77, 14.73},
                                                                                      {-146.58, 104.81, 14.96}}},
                                                                                    {{{-160.51, -37.12, 27.13},
                                                                                      {-135.64, -16.35, 36.80},
                                                                                      {-126.48, -38.50, 36.42},
                                                                                      {-127.36, -37.91, 13.71},
                                                                                      {-135.51, -16.71, 12.83},
                                                                                      {-177.69, -29.76, 40.50},
                                                                                      {-167.89, -53.35, 40.74},
                                                                                      {-167.78, -53.55, 14.73},
                                                                                      {-177.76, -29.53, 14.96}}},
                                                                                    {{{-87.25, -139.75, 27.13},
                                                                                      {-84.35, -107.47, 36.80},
                                                                                      {-62.21, -116.66, 36.42},
                                                                                      {-63.25, -116.86, 13.71},
                                                                                      {-84.01, -107.64, 12.83},
                                                                                      {-104.60, -146.69, 40.50},
                                                                                      {-80.99, -156.44, 40.74},
                                                                                      {-80.77, -156.50, 14.73},
                                                                                      {-104.81, -146.58, 14.96}}},
                                                                                    {{{37.12, -160.51, 27.13},
                                                                                      {16.35, -135.64, 36.80},
                                                                                      {38.50, -126.48, 36.42},
                                                                                      {37.91, -127.36, 13.71},
                                                                                      {16.71, -135.51, 12.83},
                                                                                      {29.76, -177.69, 40.50},
                                                                                      {53.35, -167.89, 40.74},
                                                                                      {53.55, -167.78, 14.73},
                                                                                      {29.53, -177.76, 14.96}}},
                                                                                    {{{139.75, -87.25, 27.13},
                                                                                      {107.47, -84.35, 36.80},
                                                                                      {116.66, -62.21, 36.42},
                                                                                      {116.86, -63.25, 13.71},
                                                                                      {107.64, -84.01, 12.83},
                                                                                      {146.69, -104.60, 40.50},
                                                                                      {156.44, -80.99, 40.74},
                                                                                      {156.50, -80.77, 14.73},
                                                                                      {146.58, -104.81, 14.96}}},
                                                                                    {{{134.26, 26.25, -95.72},
                                                                                      {124.08, 11.56, -67.01},
                                                                                      {114.67, 33.61, -67.30},
                                                                                      {100.55, 26.81, -83.76},
                                                                                      {108.28, 5.43, -83.98},
                                                                                      {156.64, 21.05, -95.49},
                                                                                      {147.01, 44.70, -95.31},
                                                                                      {129.91, 37.87, -113.68},
                                                                                      {140.03, 13.91, -113.54}}},
                                                                                    {{{-26.25, 134.26, -95.72},
                                                                                      {-11.56, 124.08, -67.01},
                                                                                      {-33.61, 114.67, -67.30},
                                                                                      {-26.81, 100.55, -83.76},
                                                                                      {-5.43, 108.28, -83.98},
                                                                                      {-21.05, 156.64, -95.49},
                                                                                      {-44.70, 147.01, -95.31},
                                                                                      {-37.87, 129.91, -113.68},
                                                                                      {-13.91, 140.03, -113.54}}},
                                                                                    {{{-134.26, -26.25, -95.72},
                                                                                      {-124.08, -11.56, -67.01},
                                                                                      {-114.67, -33.61, -67.30},
                                                                                      {-100.55, -26.81, -83.76},
                                                                                      {-108.28, -5.43, -83.98},
                                                                                      {-156.64, -21.05, -95.49},
                                                                                      {-147.01, -44.70, -95.31},
                                                                                      {-129.91, -37.87, -113.68},
                                                                                      {-140.03, -13.91, -113.54}}},
                                                                                    {{{26.25, -134.26, -95.72},
                                                                                      {11.56, -124.08, -67.01},
                                                                                      {33.61, -114.67, -67.30},
                                                                                      {26.81, -100.55, -83.76},
                                                                                      {5.43, -108.28, -83.98},
                                                                                      {21.05, -156.64, -95.49},
                                                                                      {44.70, -147.01, -95.31},
                                                                                      {37.87, -129.91, -113.68},
                                                                                      {13.91, -140.03, -113.54}}}}};

std::array<std::array<std::array<double, 3>, 9>, 17> TTigress::fGeBluePositionBack = {{{{{0., 0., 0.},
                                                                                         {0., 0., 0.},
                                                                                         {0., 0., 0.},
                                                                                         {0., 0., 0.},
                                                                                         {0., 0., 0.},
                                                                                         {0., 0., 0.},
                                                                                         {0., 0., 0.},
                                                                                         {0., 0., 0.},
                                                                                         {0., 0., 0.}}},
                                                                                       {{{100.92, 71.17, 158.84},
                                                                                         {70.69, 69.11, 143.80},
                                                                                         {86.51, 75.25, 126.87},
                                                                                         {95.01, 54.19, 127.90},
                                                                                         {80.13, 47.08, 143.55},
                                                                                         {95.59, 83.43, 177.52},
                                                                                         {112.17, 90.56, 159.45},
                                                                                         {122.26, 66.58, 159.26},
                                                                                         {105.20, 59.77, 177.67}}},
                                                                                       {{{-71.17, 100.92, 158.84},
                                                                                         {-69.11, 70.69, 143.80},
                                                                                         {-75.25, 86.51, 126.87},
                                                                                         {-54.19, 95.01, 127.90},
                                                                                         {-47.08, 80.13, 143.55},
                                                                                         {-83.43, 95.59, 177.52},
                                                                                         {-90.56, 112.17, 159.45},
                                                                                         {-66.58, 122.26, 159.26},
                                                                                         {-59.77, 105.20, 177.67}}},
                                                                                       {{{-100.92, -71.17, 158.84},
                                                                                         {-70.69, -69.11, 143.80},
                                                                                         {-86.51, -75.25, 126.87},
                                                                                         {-95.01, -54.19, 127.90},
                                                                                         {-80.13, -47.08, 143.55},
                                                                                         {-95.59, -83.43, 177.52},
                                                                                         {-112.17, -90.56, 159.45},
                                                                                         {-122.26, -66.58, 159.26},
                                                                                         {-105.20, -59.77, 177.67}}},
                                                                                       {{{71.17, -100.92, 158.84},
                                                                                         {69.11, -70.69, 143.80},
                                                                                         {75.25, -86.51, 126.87},
                                                                                         {54.19, -95.01, 127.90},
                                                                                         {47.08, -80.13, 143.55},
                                                                                         {83.43, -95.59, 177.52},
                                                                                         {90.56, -112.17, 159.45},
                                                                                         {66.58, -122.26, 159.26},
                                                                                         {59.77, -105.20, 177.67}}},
                                                                                       {{{172.08, 100.64, 27.13},
                                                                                         {139.81, 97.74, 36.80},
                                                                                         {139.97, 97.40, 12.83},
                                                                                         {149.20, 76.64, 13.71},
                                                                                         {149.00, 75.60, 36.42},
                                                                                         {179.02, 117.99, 40.50},
                                                                                         {178.91, 118.21, 14.96},
                                                                                         {188.84, 94.16, 14.73},
                                                                                         {188.78, 94.39, 40.74}}},
                                                                                       {{{50.52, 192.85, 27.13},
                                                                                         {29.74, 167.97, 36.80},
                                                                                         {30.10, 167.85, 12.83},
                                                                                         {51.31, 159.69, 13.71},
                                                                                         {51.90, 158.82, 36.42},
                                                                                         {43.16, 210.02, 40.50},
                                                                                         {42.93, 210.09, 14.96},
                                                                                         {66.95, 200.11, 14.73},
                                                                                         {66.75, 200.23, 40.74}}},
                                                                                       {{{-100.64, 172.08, 27.13},
                                                                                         {-97.74, 139.81, 36.80},
                                                                                         {-97.40, 139.97, 12.83},
                                                                                         {-76.64, 149.20, 13.71},
                                                                                         {-75.60, 149.00, 36.42},
                                                                                         {-117.99, 179.02, 40.50},
                                                                                         {-118.21, 178.91, 14.96},
                                                                                         {-94.16, 188.84, 14.73},
                                                                                         {-94.39, 188.78, 40.74}}},
                                                                                       {{{-192.85, 50.52, 27.13},
                                                                                         {-167.97, 29.74, 36.80},
                                                                                         {-167.85, 30.10, 12.83},
                                                                                         {-159.69, 51.31, 13.71},
                                                                                         {-158.82, 51.90, 36.42},
                                                                                         {-210.02, 43.16, 40.50},
                                                                                         {-210.09, 42.93, 14.96},
                                                                                         {-200.11, 66.95, 14.73},
                                                                                         {-200.23, 66.75, 40.74}}},
                                                                                       {{{-172.08, -100.64, 27.13},
                                                                                         {-139.81, -97.74, 36.80},
                                                                                         {-139.97, -97.40, 12.83},
                                                                                         {-149.20, -76.64, 13.71},
                                                                                         {-149.00, -75.60, 36.42},
                                                                                         {-179.02, -117.99, 40.50},
                                                                                         {-178.91, -118.21, 14.96},
                                                                                         {-188.84, -94.16, 14.73},
                                                                                         {-188.78, -94.39, 40.74}}},
                                                                                       {{{-50.52, -192.85, 27.13},
                                                                                         {-29.74, -167.97, 36.80},
                                                                                         {-30.10, -167.85, 12.83},
                                                                                         {-51.31, -159.69, 13.71},
                                                                                         {-51.90, -158.82, 36.42},
                                                                                         {-43.16, -210.02, 40.50},
                                                                                         {-42.93, -210.09, 14.96},
                                                                                         {-66.95, -200.11, 14.73},
                                                                                         {-66.75, -200.23, 40.74}}},
                                                                                       {{{100.64, -172.08, 27.13},
                                                                                         {97.74, -139.81, 36.80},
                                                                                         {97.40, -139.97, 12.83},
                                                                                         {76.64, -149.20, 13.71},
                                                                                         {75.60, -149.00, 36.42},
                                                                                         {117.99, -179.02, 40.50},
                                                                                         {118.21, -178.91, 14.96},
                                                                                         {94.16, -188.84, 14.73},
                                                                                         {94.39, -188.78, 40.74}}},
                                                                                       {{{192.85, -50.52, 27.13},
                                                                                         {167.97, -29.74, 36.80},
                                                                                         {167.85, -30.10, 12.83},
                                                                                         {159.69, -51.31, 13.71},
                                                                                         {158.82, -51.90, 36.42},
                                                                                         {210.02, -43.16, 40.50},
                                                                                         {210.09, -42.93, 14.96},
                                                                                         {200.11, -66.95, 14.73},
                                                                                         {200.23, -66.75, 40.74}}},
                                                                                       {{{136.36, 85.85, -120.47},
                                                                                         {118.78, 89.03, -91.76},
                                                                                         {103.27, 82.20, -108.72},
                                                                                         {112.92, 61.61, -108.51},
                                                                                         {127.71, 66.79, -92.04},
                                                                                         {148.51, 105.35, -120.24},
                                                                                         {131.72, 98.66, -138.29},
                                                                                         {141.50, 74.56, -138.43},
                                                                                         {158.43, 81.81, -120.06}}},
                                                                                       {{{-85.85, 136.36, -120.47},
                                                                                         {-89.03, 118.78, -91.76},
                                                                                         {-82.20, 103.27, -108.72},
                                                                                         {-61.61, 112.92, -108.51},
                                                                                         {-66.79, 127.71, -92.04},
                                                                                         {-105.35, 148.51, -120.24},
                                                                                         {-98.66, 131.72, -138.29},
                                                                                         {-74.56, 141.50, -138.43},
                                                                                         {-81.81, 158.43, -120.06}}},
                                                                                       {{{-136.36, -85.85, -120.47},
                                                                                         {-118.78, -89.03, -91.76},
                                                                                         {-103.27, -82.20, -108.72},
                                                                                         {-112.92, -61.61, -108.51},
                                                                                         {-127.71, -66.79, -92.04},
                                                                                         {-148.51, -105.35, -120.24},
                                                                                         {-131.72, -98.66, -138.29},
                                                                                         {-141.50, -74.56, -138.43},
                                                                                         {-158.43, -81.81, -120.06}}},
                                                                                       {{{85.85, -136.36, -120.47},
                                                                                         {89.03, -118.78, -91.76},
                                                                                         {82.20, -103.27, -108.72},
                                                                                         {61.61, -112.92, -108.51},
                                                                                         {66.79, -127.71, -92.04},
                                                                                         {105.35, -148.51, -120.24},
                                                                                         {98.66, -131.72, -138.29},
                                                                                         {74.56, -141.50, -138.43},
                                                                                         {81.81, -158.43, -120.06}}}}};

// Assuming this is the 1
std::array<std::array<std::array<double, 3>, 9>, 17> TTigress::fGeGreenPositionBack = {{{{{0., 0., 0.},
                                                                                          {0., 0., 0.},
                                                                                          {0., 0., 0.},
                                                                                          {0., 0., 0.},
                                                                                          {0., 0., 0.},
                                                                                          {0., 0., 0.},
                                                                                          {0., 0., 0.},
                                                                                          {0., 0., 0.},
                                                                                          {0., 0., 0.}}},
                                                                                        {{{136.36, 85.85, 120.47},
                                                                                          {118.78, 89.03, 91.76},
                                                                                          {127.71, 66.79, 92.04},
                                                                                          {112.92, 61.61, 108.51},
                                                                                          {103.27, 82.20, 108.72},
                                                                                          {148.51, 105.35, 120.24},
                                                                                          {158.43, 81.81, 120.06},
                                                                                          {141.50, 74.56, 138.43},
                                                                                          {131.72, 98.66, 138.29}}},
                                                                                        {{{-85.85, 136.36, 120.47},
                                                                                          {-89.03, 118.78, 91.76},
                                                                                          {-66.79, 127.71, 92.04},
                                                                                          {-61.61, 112.92, 108.51},
                                                                                          {-82.20, 103.27, 108.72},
                                                                                          {-105.35, 148.51, 120.24},
                                                                                          {-81.81, 158.43, 120.06},
                                                                                          {-74.56, 141.50, 138.43},
                                                                                          {-98.66, 131.72, 138.29}}},
                                                                                        {{{-136.36, -85.85, 120.47},
                                                                                          {-118.78, -89.03, 91.76},
                                                                                          {-127.71, -66.79, 92.04},
                                                                                          {-112.92, -61.61, 108.51},
                                                                                          {-103.27, -82.20, 108.72},
                                                                                          {-148.51, -105.35, 120.24},
                                                                                          {-158.43, -81.81, 120.06},
                                                                                          {-141.50, -74.56, 138.43},
                                                                                          {-131.72, -98.66, 138.29}}},
                                                                                        {{{85.85, -136.36, 120.47},
                                                                                          {89.03, -118.78, 91.76},
                                                                                          {66.79, -127.71, 92.04},
                                                                                          {61.61, -112.92, 108.51},
                                                                                          {82.20, -103.27, 108.72},
                                                                                          {105.35, -148.51, 120.24},
                                                                                          {81.81, -158.43, 120.06},
                                                                                          {74.56, -141.50, 138.43},
                                                                                          {98.66, -131.72, 138.29}}},
                                                                                        {{{172.08, 100.64, -27.13},
                                                                                          {139.81, 97.74, -36.80},
                                                                                          {149.00, 75.60, -36.42},
                                                                                          {149.20, 76.64, -13.71},
                                                                                          {139.97, 97.40, -12.83},
                                                                                          {179.02, 117.99, -40.50},
                                                                                          {188.78, 94.39, -40.74},
                                                                                          {188.84, 94.16, -14.73},
                                                                                          {178.91, 118.21, -14.96}}},
                                                                                        {{{50.52, 192.85, -27.13},
                                                                                          {29.74, 167.97, -36.80},
                                                                                          {51.90, 158.82, -36.42},
                                                                                          {51.31, 159.69, -13.71},
                                                                                          {30.10, 167.85, -12.83},
                                                                                          {43.16, 210.02, -40.50},
                                                                                          {66.75, 200.23, -40.74},
                                                                                          {66.95, 200.11, -14.73},
                                                                                          {42.93, 210.09, -14.96}}},
                                                                                        {{{-100.64, 172.08, -27.13},
                                                                                          {-97.74, 139.81, -36.80},
                                                                                          {-75.60, 149.00, -36.42},
                                                                                          {-76.64, 149.20, -13.71},
                                                                                          {-97.40, 139.97, -12.83},
                                                                                          {-117.99, 179.02, -40.50},
                                                                                          {-94.39, 188.78, -40.74},
                                                                                          {-94.16, 188.84, -14.73},
                                                                                          {-118.21, 178.91, -14.96}}},
                                                                                        {{{-192.85, 50.52, -27.13},
                                                                                          {-167.97, 29.74, -36.80},
                                                                                          {-158.82, 51.90, -36.42},
                                                                                          {-159.69, 51.31, -13.71},
                                                                                          {-167.85, 30.10, -12.83},
                                                                                          {-210.02, 43.16, -40.50},
                                                                                          {-200.23, 66.75, -40.74},
                                                                                          {-200.11, 66.95, -14.73},
                                                                                          {-210.09, 42.93, -14.96}}},
                                                                                        {{{-172.08, -100.64, -27.13},
                                                                                          {-139.81, -97.74, -36.80},
                                                                                          {-149.00, -75.60, -36.42},
                                                                                          {-149.20, -76.64, -13.71},
                                                                                          {-139.97, -97.40, -12.83},
                                                                                          {-179.02, -117.99, -40.50},
                                                                                          {-188.78, -94.39, -40.74},
                                                                                          {-188.84, -94.16, -14.73},
                                                                                          {-178.91, -118.21, -14.96}}},
                                                                                        {{{-50.52, -192.85, -27.13},
                                                                                          {-29.74, -167.97, -36.80},
                                                                                          {-51.90, -158.82, -36.42},
                                                                                          {-51.31, -159.69, -13.71},
                                                                                          {-30.10, -167.85, -12.83},
                                                                                          {-43.16, -210.02, -40.50},
                                                                                          {-66.75, -200.23, -40.74},
                                                                                          {-66.95, -200.11, -14.73},
                                                                                          {-42.93, -210.09, -14.96}}},
                                                                                        {{{100.64, -172.08, -27.13},
                                                                                          {97.74, -139.81, -36.80},
                                                                                          {75.60, -149.00, -36.42},
                                                                                          {76.64, -149.20, -13.71},
                                                                                          {97.40, -139.97, -12.83},
                                                                                          {117.99, -179.02, -40.50},
                                                                                          {94.39, -188.78, -40.74},
                                                                                          {94.16, -188.84, -14.73},
                                                                                          {118.21, -178.91, -14.96}}},
                                                                                        {{{192.85, -50.52, -27.13},
                                                                                          {167.97, -29.74, -36.80},
                                                                                          {158.82, -51.90, -36.42},
                                                                                          {159.69, -51.31, -13.71},
                                                                                          {167.85, -30.10, -12.83},
                                                                                          {210.02, -43.16, -40.50},
                                                                                          {200.23, -66.75, -40.74},
                                                                                          {200.11, -66.95, -14.73},
                                                                                          {210.09, -42.93, -14.96}}},
                                                                                        {{{100.92, 71.17, -158.84},
                                                                                          {70.69, 69.11, -143.80},
                                                                                          {80.13, 47.08, -143.55},
                                                                                          {95.01, 54.19, -127.90},
                                                                                          {86.51, 75.25, -126.87},
                                                                                          {95.59, 83.43, -177.52},
                                                                                          {105.20, 59.77, -177.67},
                                                                                          {122.26, 66.58, -159.26},
                                                                                          {112.17, 90.56, -159.45}}},
                                                                                        {{{-71.17, 100.92, -158.84},
                                                                                          {-69.11, 70.69, -143.80},
                                                                                          {-47.08, 80.13, -143.55},
                                                                                          {-54.19, 95.01, -127.90},
                                                                                          {-75.25, 86.51, -126.87},
                                                                                          {-83.43, 95.59, -177.52},
                                                                                          {-59.77, 105.20, -177.67},
                                                                                          {-66.58, 122.26, -159.26},
                                                                                          {-90.56, 112.17, -159.45}}},
                                                                                        {{{-100.92, -71.17, -158.84},
                                                                                          {-70.69, -69.11, -143.80},
                                                                                          {-80.13, -47.08, -143.55},
                                                                                          {-95.01, -54.19, -127.90},
                                                                                          {-86.51, -75.25, -126.87},
                                                                                          {-95.59, -83.43, -177.52},
                                                                                          {-105.20, -59.77, -177.67},
                                                                                          {-122.26, -66.58, -159.26},
                                                                                          {-112.17, -90.56, -159.45}}},
                                                                                        {{{71.17, -100.92, -158.84},
                                                                                          {69.11, -70.69, -143.80},
                                                                                          {47.08, -80.13, -143.55},
                                                                                          {54.19, -95.01, -127.90},
                                                                                          {75.25, -86.51, -126.87},
                                                                                          {83.43, -95.59, -177.52},
                                                                                          {59.77, -105.20, -177.67},
                                                                                          {66.58, -122.26, -159.26},
                                                                                          {90.56, -112.17, -159.45}}}}};

// Assuming this is the 2
std::array<std::array<std::array<double, 3>, 9>, 17> TTigress::fGeRedPositionBack = {{{{{0., 0., 0.},
                                                                                        {0., 0., 0.},
                                                                                        {0., 0., 0.},
                                                                                        {0., 0., 0.},
                                                                                        {0., 0., 0.},
                                                                                        {0., 0., 0.},
                                                                                        {0., 0., 0.},
                                                                                        {0., 0., 0.},
                                                                                        {0., 0., 0.}}},
                                                                                      {{{157.13, 35.72, 120.47},
                                                                                        {146.94, 21.03, 91.76},
                                                                                        {131.15, 14.90, 108.72},
                                                                                        {123.41, 36.28, 108.51},
                                                                                        {137.53, 43.08, 92.04},
                                                                                        {179.50, 30.52, 120.24},
                                                                                        {162.90, 23.38, 138.29},
                                                                                        {152.78, 47.34, 138.43},
                                                                                        {169.87, 54.17, 120.06}}},
                                                                                      {{{-35.72, 157.13, 120.47},
                                                                                        {-21.03, 146.94, 91.76},
                                                                                        {-14.90, 131.15, 108.72},
                                                                                        {-36.28, 123.41, 108.51},
                                                                                        {-43.08, 137.53, 92.04},
                                                                                        {-30.52, 179.50, 120.24},
                                                                                        {-23.38, 162.90, 138.29},
                                                                                        {-47.34, 152.78, 138.43},
                                                                                        {-54.17, 169.87, 120.06}}},
                                                                                      {{{-157.13, -35.72, 120.47},
                                                                                        {-146.94, -21.03, 91.76},
                                                                                        {-131.15, -14.90, 108.72},
                                                                                        {-123.41, -36.28, 108.51},
                                                                                        {-137.53, -43.08, 92.04},
                                                                                        {-179.50, -30.52, 120.24},
                                                                                        {-162.90, -23.38, 138.29},
                                                                                        {-152.78, -47.34, 138.43},
                                                                                        {-169.87, -54.17, 120.06}}},
                                                                                      {{{35.72, -157.13, 120.47},
                                                                                        {21.03, -146.94, 91.76},
                                                                                        {14.90, -131.15, 108.72},
                                                                                        {36.28, -123.41, 108.51},
                                                                                        {43.08, -137.53, 92.04},
                                                                                        {30.52, -179.50, 120.24},
                                                                                        {23.38, -162.90, 138.29},
                                                                                        {47.34, -152.78, 138.43},
                                                                                        {54.17, -169.87, 120.06}}},
                                                                                      {{{192.85, 50.52, -27.13},
                                                                                        {167.97, 29.74, -36.80},
                                                                                        {167.85, 30.10, -12.83},
                                                                                        {159.69, 51.31, -13.71},
                                                                                        {158.82, 51.90, -36.42},
                                                                                        {210.02, 43.16, -40.50},
                                                                                        {210.09, 42.93, -14.96},
                                                                                        {200.11, 66.95, -14.73},
                                                                                        {200.23, 66.75, -40.74}}},
                                                                                      {{{100.64, 172.08, -27.13},
                                                                                        {97.74, 139.81, -36.80},
                                                                                        {97.40, 139.97, -12.83},
                                                                                        {76.64, 149.20, -13.71},
                                                                                        {75.60, 149.00, -36.42},
                                                                                        {117.99, 179.02, -40.50},
                                                                                        {118.21, 178.91, -14.96},
                                                                                        {94.16, 188.84, -14.73},
                                                                                        {94.39, 188.78, -40.74}}},
                                                                                      {{{-50.52, 192.85, -27.13},
                                                                                        {-29.74, 167.97, -36.80},
                                                                                        {-30.10, 167.85, -12.83},
                                                                                        {-51.31, 159.69, -13.71},
                                                                                        {-51.90, 158.82, -36.42},
                                                                                        {-43.16, 210.02, -40.50},
                                                                                        {-42.93, 210.09, -14.96},
                                                                                        {-66.95, 200.11, -14.73},
                                                                                        {-66.75, 200.23, -40.74}}},
                                                                                      {{{-172.08, 100.64, -27.13},
                                                                                        {-139.81, 97.74, -36.80},
                                                                                        {-139.97, 97.40, -12.83},
                                                                                        {-149.20, 76.64, -13.71},
                                                                                        {-149.00, 75.60, -36.42},
                                                                                        {-179.02, 117.99, -40.50},
                                                                                        {-178.91, 118.21, -14.96},
                                                                                        {-188.84, 94.16, -14.73},
                                                                                        {-188.78, 94.39, -40.74}}},
                                                                                      {{{-192.85, -50.52, -27.13},
                                                                                        {-167.97, -29.74, -36.80},
                                                                                        {-167.85, -30.10, -12.83},
                                                                                        {-159.69, -51.31, -13.71},
                                                                                        {-158.82, -51.90, -36.42},
                                                                                        {-210.02, -43.16, -40.50},
                                                                                        {-210.09, -42.93, -14.96},
                                                                                        {-200.11, -66.95, -14.73},
                                                                                        {-200.23, -66.75, -40.74}}},
                                                                                      {{{-100.64, -172.08, -27.13},
                                                                                        {-97.74, -139.81, -36.80},
                                                                                        {-97.40, -139.97, -12.83},
                                                                                        {-76.64, -149.20, -13.71},
                                                                                        {-75.60, -149.00, -36.42},
                                                                                        {-117.99, -179.02, -40.50},
                                                                                        {-118.21, -178.91, -14.96},
                                                                                        {-94.16, -188.84, -14.73},
                                                                                        {-94.39, -188.78, -40.74}}},
                                                                                      {{{50.52, -192.85, -27.13},
                                                                                        {29.74, -167.97, -36.80},
                                                                                        {30.10, -167.85, -12.83},
                                                                                        {51.31, -159.69, -13.71},
                                                                                        {51.90, -158.82, -36.42},
                                                                                        {43.16, -210.02, -40.50},
                                                                                        {42.93, -210.09, -14.96},
                                                                                        {66.95, -200.11, -14.73},
                                                                                        {66.75, -200.23, -40.74}}},
                                                                                      {{{172.08, -100.64, -27.13},
                                                                                        {139.81, -97.74, -36.80},
                                                                                        {139.97, -97.40, -12.83},
                                                                                        {149.20, -76.64, -13.71},
                                                                                        {149.00, -75.60, -36.42},
                                                                                        {179.02, -117.99, -40.50},
                                                                                        {178.91, -118.21, -14.96},
                                                                                        {188.84, -94.16, -14.73},
                                                                                        {188.78, -94.39, -40.74}}},
                                                                                      {{{121.68, 21.04, -158.84},
                                                                                        {98.86, 1.12, -143.80},
                                                                                        {114.39, 7.96, -126.87},
                                                                                        {105.50, 28.86, -127.90},
                                                                                        {89.95, 23.37, -143.55},
                                                                                        {126.59, 8.60, -177.52},
                                                                                        {143.35, 15.28, -159.45},
                                                                                        {133.53, 39.37, -159.26},
                                                                                        {116.65, 32.12, -177.67}}},
                                                                                      {{{-21.04, 121.68, -158.84},
                                                                                        {-1.12, 98.86, -143.80},
                                                                                        {-7.96, 114.39, -126.87},
                                                                                        {-28.86, 105.50, -127.90},
                                                                                        {-23.37, 89.95, -143.55},
                                                                                        {-8.60, 126.59, -177.52},
                                                                                        {-15.28, 143.35, -159.45},
                                                                                        {-39.37, 133.53, -159.26},
                                                                                        {-32.12, 116.65, -177.67}}},
                                                                                      {{{-121.68, -21.04, -158.84},
                                                                                        {-98.86, -1.12, -143.80},
                                                                                        {-114.39, -7.96, -126.87},
                                                                                        {-105.50, -28.86, -127.90},
                                                                                        {-89.95, -23.37, -143.55},
                                                                                        {-126.59, -8.60, -177.52},
                                                                                        {-143.35, -15.28, -159.45},
                                                                                        {-133.53, -39.37, -159.26},
                                                                                        {-116.65, -32.12, -177.67}}},
                                                                                      {{{21.04, -121.68, -158.84},
                                                                                        {1.12, -98.86, -143.80},
                                                                                        {7.96, -114.39, -126.87},
                                                                                        {28.86, -105.50, -127.90},
                                                                                        {23.37, -89.95, -143.55},
                                                                                        {8.60, -126.59, -177.52},
                                                                                        {15.28, -143.35, -159.45},
                                                                                        {39.37, -133.53, -159.26},
                                                                                        {32.12, -116.65, -177.67}}}}};

// Assuming this is the 3
std::array<std::array<std::array<double, 3>, 9>, 17> TTigress::fGeWhitePositionBack = {{{{{0., 0., 0.},
                                                                                          {0., 0., 0.},
                                                                                          {0., 0., 0.},
                                                                                          {0., 0., 0.},
                                                                                          {0., 0., 0.},
                                                                                          {0., 0., 0.},
                                                                                          {0., 0., 0.},
                                                                                          {0., 0., 0.},
                                                                                          {0., 0., 0.}}},
                                                                                        {{{121.68, 21.04, 158.84},
                                                                                          {98.86, 1.12, 143.80},
                                                                                          {89.95, 23.37, 143.55},
                                                                                          {105.50, 28.86, 127.90},
                                                                                          {114.39, 7.96, 126.87},
                                                                                          {126.59, 8.60, 177.52},
                                                                                          {116.65, 32.12, 177.67},
                                                                                          {133.53, 39.37, 159.26},
                                                                                          {143.35, 15.28, 159.45}}},
                                                                                        {{{-21.04, 121.68, 158.84},
                                                                                          {-1.12, 98.86, 143.80},
                                                                                          {-23.37, 89.95, 143.55},
                                                                                          {-28.86, 105.50, 127.90},
                                                                                          {-7.96, 114.39, 126.87},
                                                                                          {-8.60, 126.59, 177.52},
                                                                                          {-32.12, 116.65, 177.67},
                                                                                          {-39.37, 133.53, 159.26},
                                                                                          {-15.28, 143.35, 159.45}}},
                                                                                        {{{-121.68, -21.04, 158.84},
                                                                                          {-98.86, -1.12, 143.80},
                                                                                          {-89.95, -23.37, 143.55},
                                                                                          {-105.50, -28.86, 127.90},
                                                                                          {-114.39, -7.96, 126.87},
                                                                                          {-126.59, -8.60, 177.52},
                                                                                          {-116.65, -32.12, 177.67},
                                                                                          {-133.53, -39.37, 159.26},
                                                                                          {-143.35, -15.28, 159.45}}},
                                                                                        {{{21.04, -121.68, 158.84},
                                                                                          {1.12, -98.86, 143.80},
                                                                                          {23.37, -89.95, 143.55},
                                                                                          {28.86, -105.50, 127.90},
                                                                                          {7.96, -114.39, 126.87},
                                                                                          {8.60, -126.59, 177.52},
                                                                                          {32.12, -116.65, 177.67},
                                                                                          {39.37, -133.53, 159.26},
                                                                                          {15.28, -143.35, 159.45}}},
                                                                                        {{{192.85, 50.52, 27.13},
                                                                                          {167.97, 29.74, 36.80},
                                                                                          {158.82, 51.90, 36.42},
                                                                                          {159.69, 51.31, 13.71},
                                                                                          {167.85, 30.10, 12.83},
                                                                                          {210.02, 43.16, 40.50},
                                                                                          {200.23, 66.75, 40.74},
                                                                                          {200.11, 66.95, 14.73},
                                                                                          {210.09, 42.93, 14.96}}},
                                                                                        {{{100.64, 172.08, 27.13},
                                                                                          {97.74, 139.81, 36.80},
                                                                                          {75.60, 149.00, 36.42},
                                                                                          {76.64, 149.20, 13.71},
                                                                                          {97.40, 139.97, 12.83},
                                                                                          {117.99, 179.02, 40.50},
                                                                                          {94.39, 188.78, 40.74},
                                                                                          {94.16, 188.84, 14.73},
                                                                                          {118.21, 178.91, 14.96}}},
                                                                                        {{{-50.52, 192.85, 27.13},
                                                                                          {-29.74, 167.97, 36.80},
                                                                                          {-51.90, 158.82, 36.42},
                                                                                          {-51.31, 159.69, 13.71},
                                                                                          {-30.10, 167.85, 12.83},
                                                                                          {-43.16, 210.02, 40.50},
                                                                                          {-66.75, 200.23, 40.74},
                                                                                          {-66.95, 200.11, 14.73},
                                                                                          {-42.93, 210.09, 14.96}}},
                                                                                        {{{-172.08, 100.64, 27.13},
                                                                                          {-139.81, 97.74, 36.80},
                                                                                          {-149.00, 75.60, 36.42},
                                                                                          {-149.20, 76.64, 13.71},
                                                                                          {-139.97, 97.40, 12.83},
                                                                                          {-179.02, 117.99, 40.50},
                                                                                          {-188.78, 94.39, 40.74},
                                                                                          {-188.84, 94.16, 14.73},
                                                                                          {-178.91, 118.21, 14.96}}},
                                                                                        {{{-192.85, -50.52, 27.13},
                                                                                          {-167.97, -29.74, 36.80},
                                                                                          {-158.82, -51.90, 36.42},
                                                                                          {-159.69, -51.31, 13.71},
                                                                                          {-167.85, -30.10, 12.83},
                                                                                          {-210.02, -43.16, 40.50},
                                                                                          {-200.23, -66.75, 40.74},
                                                                                          {-200.11, -66.95, 14.73},
                                                                                          {-210.09, -42.93, 14.96}}},
                                                                                        {{{-100.64, -172.08, 27.13},
                                                                                          {-97.74, -139.81, 36.80},
                                                                                          {-75.60, -149.00, 36.42},
                                                                                          {-76.64, -149.20, 13.71},
                                                                                          {-97.40, -139.97, 12.83},
                                                                                          {-117.99, -179.02, 40.50},
                                                                                          {-94.39, -188.78, 40.74},
                                                                                          {-94.16, -188.84, 14.73},
                                                                                          {-118.21, -178.91, 14.96}}},
                                                                                        {{{50.52, -192.85, 27.13},
                                                                                          {29.74, -167.97, 36.80},
                                                                                          {51.90, -158.82, 36.42},
                                                                                          {51.31, -159.69, 13.71},
                                                                                          {30.10, -167.85, 12.83},
                                                                                          {43.16, -210.02, 40.50},
                                                                                          {66.75, -200.23, 40.74},
                                                                                          {66.95, -200.11, 14.73},
                                                                                          {42.93, -210.09, 14.96}}},
                                                                                        {{{172.08, -100.64, 27.13},
                                                                                          {139.81, -97.74, 36.80},
                                                                                          {149.00, -75.60, 36.42},
                                                                                          {149.20, -76.64, 13.71},
                                                                                          {139.97, -97.40, 12.83},
                                                                                          {179.02, -117.99, 40.50},
                                                                                          {188.78, -94.39, 40.74},
                                                                                          {188.84, -94.16, 14.73},
                                                                                          {178.91, -118.21, 14.96}}},
                                                                                        {{{157.13, 35.72, -120.47},
                                                                                          {146.94, 21.03, -91.76},
                                                                                          {137.53, 43.08, -92.04},
                                                                                          {123.41, 36.28, -108.51},
                                                                                          {131.15, 14.90, -108.72},
                                                                                          {179.50, 30.52, -120.24},
                                                                                          {169.87, 54.17, -120.06},
                                                                                          {152.78, 47.34, -138.43},
                                                                                          {162.90, 23.38, -138.29}}},
                                                                                        {{{-35.72, 157.13, -120.47},
                                                                                          {-21.03, 146.94, -91.76},
                                                                                          {-43.08, 137.53, -92.04},
                                                                                          {-36.28, 123.41, -108.51},
                                                                                          {-14.90, 131.15, -108.72},
                                                                                          {-30.52, 179.50, -120.24},
                                                                                          {-54.17, 169.87, -120.06},
                                                                                          {-47.34, 152.78, -138.43},
                                                                                          {-23.38, 162.90, -138.29}}},
                                                                                        {{{-157.13, -35.72, -120.47},
                                                                                          {-146.94, -21.03, -91.76},
                                                                                          {-137.53, -43.08, -92.04},
                                                                                          {-123.41, -36.28, -108.51},
                                                                                          {-131.15, -14.90, -108.72},
                                                                                          {-179.50, -30.52, -120.24},
                                                                                          {-169.87, -54.17, -120.06},
                                                                                          {-152.78, -47.34, -138.43},
                                                                                          {-162.90, -23.38, -138.29}}},
                                                                                        {{{35.72, -157.13, -120.47},
                                                                                          {21.03, -146.94, -91.76},
                                                                                          {43.08, -137.53, -92.04},
                                                                                          {36.28, -123.41, -108.51},
                                                                                          {14.90, -131.15, -108.72},
                                                                                          {30.52, -179.50, -120.24},
                                                                                          {54.17, -169.87, -120.06},
                                                                                          {47.34, -152.78, -138.43},
                                                                                          {23.38, -162.90, -138.29}}}}};
