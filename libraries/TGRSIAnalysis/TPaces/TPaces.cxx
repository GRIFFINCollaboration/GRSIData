#include <iostream>
#include "TPaces.h"
#include "TRandom.h"
#include "TMath.h"

////////////////////////////////////////////////////////////
//
// TPaces
//
// The TPaces class defines the observables and algorithms used
// when analyzing GRIFFIN data.
//
////////////////////////////////////////////////////////////

bool TPaces::fSetCoreWave = false;

std::array<ROOT::Math::Polar3DVector, 6> TPaces::fDetectorPosition = {
   ROOT::Math::Polar3DVector(1., 0., 0.),
   ROOT::Math::Polar3DVector(1., 120.178 / 180. * TMath::Pi(), 21. / 180. * TMath::Pi()),
   ROOT::Math::Polar3DVector(1., 120.827 / 180. * TMath::Pi(), 94. / 180. * TMath::Pi()),
   ROOT::Math::Polar3DVector(1., 119.742 / 180. * TMath::Pi(), 166. / 180. * TMath::Pi()),
   ROOT::Math::Polar3DVector(1., 120.299 / 180. * TMath::Pi(), 237. / 180. * TMath::Pi()),
   ROOT::Math::Polar3DVector(1., 120.193 / 180. * TMath::Pi(), 313. / 180. * TMath::Pi())};

TPaces::TPaces()
{
   Clear();
}

TPaces::TPaces(const TPaces& rhs) : TDetector(rhs)
{
   rhs.Copy(*this);
}

void TPaces::Copy(TObject& rhs) const
{
   TDetector::Copy(rhs);
}

void TPaces::Print(Option_t*) const
{
   /// Prints out TPaces members, currently shows only multiplicity.
   Print(std::cout);
}

void TPaces::Print(std::ostream& out) const
{
   std::ostringstream str;
   str << GetMultiplicity() << " hits" << std::endl;
   out << str.str();
}

TPaces& TPaces::operator=(const TPaces& rhs)
{
   rhs.Copy(*this);
   return *this;
}

void TPaces::AddFragment(const std::shared_ptr<const TFragment>& frag, TChannel*)
{
   auto* hit = new TPacesHit(*frag);
   AddHit(hit);
}
