#include <iostream>
#include "TLaBr.h"
#include <TRandom.h>
#include <TMath.h>

#include "TGRSIOptions.h"

bool DefaultLaBrSuppression(const TDetectorHit* hit, const TDetectorHit* bgoHit)
{
   return ((hit->GetDetector() == bgoHit->GetDetector()) &&
           (std::fabs(hit->GetTime() - bgoHit->GetTime()) < TGRSIOptions::AnalysisOptions()->SuppressionWindow()) &&
           (bgoHit->GetEnergy() > TGRSIOptions::AnalysisOptions()->SuppressionEnergy()));
}

std::function<bool(const TDetectorHit*, const TDetectorHit*)> TLaBr::fSuppressionCriterion = DefaultLaBrSuppression;

std::array<TVector3, 9> TLaBr::fPosition = {
   // Ancillary detector locations from Evan.
   TVector3(TMath::Sin(TMath::DegToRad() * (0.0)) * TMath::Cos(TMath::DegToRad() * (0.0)),
            TMath::Sin(TMath::DegToRad() * (0.0)) * TMath::Sin(TMath::DegToRad() * (0.0)),
            TMath::Cos(TMath::DegToRad() * (0.0))),
   // Downstream detectors
   TVector3(TMath::Sin(TMath::DegToRad() * (54.73561)) * TMath::Cos(TMath::DegToRad() * (22.5)),
            TMath::Sin(TMath::DegToRad() * (54.73561)) * TMath::Sin(TMath::DegToRad() * (22.5)),
            TMath::Cos(TMath::DegToRad() * (54.73561))),
   TVector3(TMath::Sin(TMath::DegToRad() * (54.73561)) * TMath::Cos(TMath::DegToRad() * (112.5)),
            TMath::Sin(TMath::DegToRad() * (54.73561)) * TMath::Sin(TMath::DegToRad() * (112.5)),
            TMath::Cos(TMath::DegToRad() * (54.73561))),
   TVector3(TMath::Sin(TMath::DegToRad() * (54.73561)) * TMath::Cos(TMath::DegToRad() * (202.5)),
            TMath::Sin(TMath::DegToRad() * (54.73561)) * TMath::Sin(TMath::DegToRad() * (202.5)),
            TMath::Cos(TMath::DegToRad() * (54.73561))),
   TVector3(TMath::Sin(TMath::DegToRad() * (54.73561)) * TMath::Cos(TMath::DegToRad() * (292.5)),
            TMath::Sin(TMath::DegToRad() * (54.73561)) * TMath::Sin(TMath::DegToRad() * (292.5)),
            TMath::Cos(TMath::DegToRad() * (54.73561))),

   // Upstream detectors
   TVector3(TMath::Sin(TMath::DegToRad() * (125.2644)) * TMath::Cos(TMath::DegToRad() * (22.5)),
            TMath::Sin(TMath::DegToRad() * (125.2644)) * TMath::Sin(TMath::DegToRad() * (22.5)),
            TMath::Cos(TMath::DegToRad() * (125.2644))),
   TVector3(TMath::Sin(TMath::DegToRad() * (125.2644)) * TMath::Cos(TMath::DegToRad() * (112.5)),
            TMath::Sin(TMath::DegToRad() * (125.2644)) * TMath::Sin(TMath::DegToRad() * (112.5)),
            TMath::Cos(TMath::DegToRad() * (125.2644))),
   TVector3(TMath::Sin(TMath::DegToRad() * (125.2644)) * TMath::Cos(TMath::DegToRad() * (202.5)),
            TMath::Sin(TMath::DegToRad() * (125.2644)) * TMath::Sin(TMath::DegToRad() * (202.5)),
            TMath::Cos(TMath::DegToRad() * (125.2644))),
   TVector3(TMath::Sin(TMath::DegToRad() * (125.2644)) * TMath::Cos(TMath::DegToRad() * (292.5)),
            TMath::Sin(TMath::DegToRad() * (125.2644)) * TMath::Sin(TMath::DegToRad() * (292.5)),
            TMath::Cos(TMath::DegToRad() * (125.2644)))};

TLaBr::TLaBr()
{
   // Default Constructor
   Clear();
}

TLaBr::~TLaBr()
{
   // Default Destructor
   for(auto& hit : fSuppressedHits) {
      delete hit;
   }
}

TLaBr::TLaBr(const TLaBr& rhs) : TSuppressed(rhs)
{
   // Copy Contructor
   rhs.Copy(*this);
}

void TLaBr::Clear(Option_t* opt)
{
   // Clears all of the hits
   TSuppressed::Clear(opt);
   for(auto& hit : fSuppressedHits) {
      delete hit;
   }
   fSuppressedHits.clear();
   fLaBrBits = 0;
}

void TLaBr::Copy(TObject& rhs) const
{
   // Copies a TLaBr
   TSuppressed::Copy(rhs);

   auto& suppressedHits = static_cast<TLaBr&>(rhs).fSuppressedHits;
   if(suppressedHits.size() > fSuppressedHits.size()) {
      for(size_t i = fSuppressedHits.size(); i < suppressedHits.size(); ++i) {
         delete suppressedHits[i];
      }
      suppressedHits.resize(fSuppressedHits.size());
   } else if(suppressedHits.size() < fSuppressedHits.size()) {
      // right-hand side has less hits, that means there is at least one we can use to determine the type
      // we need to use IsA()->New() to make a new hit of whatever derived type this actually is
      suppressedHits.resize(fSuppressedHits.size(), static_cast<TDetectorHit*>(fSuppressedHits[0]->IsA()->New()));
   }
   // we have now ensured that the size of the two vectors is the same, so we can copy the contents of the hits
   for(size_t i = 0; i < fSuppressedHits.size(); ++i) {
      fSuppressedHits[i]->Copy(*(suppressedHits[i]), true);
   }
   static_cast<TLaBr&>(rhs).fLaBrBits = 0;
}

TLaBr& TLaBr::operator=(const TLaBr& rhs)
{
   rhs.Copy(*this);
   return *this;
}

void TLaBr::Print(Option_t*) const
{
   /// Prints out TLaBr Multiplicity, currently does little.
   Print(std::cout);
}

void TLaBr::Print(std::ostream& out) const
{
   std::ostringstream str;
   str << GetMultiplicity() << " hits" << std::endl;
   out << str.str();
}

bool TLaBr::IsSuppressed() const
{
   return TestBitNumber(ELaBrBits::kIsSuppressed);
}

void TLaBr::SetSuppressed(const bool flag)
{
   SetBitNumber(ELaBrBits::kIsSuppressed, flag);
}

void TLaBr::ResetSuppressed()
{
   SetSuppressed(false);
   for(auto& hit : fSuppressedHits) {
      delete hit;
   }
   fSuppressedHits.clear();
}

Short_t TLaBr::GetSuppressedMultiplicity(const TBgo* bgo)
{
   /// Automatically builds the suppressed hits using the fSuppressionCriterion and returns the number of suppressed hits
   if(NoHits()) {
      return 0;
   }
   // if the suppressed has been reset, clear the suppressed hits
   if(!IsSuppressed()) {
      for(auto& hit : fSuppressedHits) {
         delete hit;
      }
      fSuppressedHits.clear();
   }
   if(fSuppressedHits.empty()) {
      CreateSuppressed(bgo, Hits(), fSuppressedHits);
      SetSuppressed(true);
   }

   return fSuppressedHits.size();
}

TLaBrHit* TLaBr::GetSuppressedHit(const int& i)
{
   try {
      return static_cast<TLaBrHit*>(fSuppressedHits.at(i));
   } catch(const std::out_of_range& oor) {
      std::cerr << ClassName() << " is out of range: " << oor.what() << std::endl;
      throw grsi::exit_exception(1);
   }
   return nullptr;
}

void TLaBr::AddFragment(const std::shared_ptr<const TFragment>& frag, TChannel*)
{
   auto* hit = new TLaBrHit(*frag);   // Building is controlled in the constructor of the hit
   AddHit(hit);
}
