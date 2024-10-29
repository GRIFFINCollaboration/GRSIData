#include <iostream>
#include "TAries.h"
#include <TRandom.h>
#include <TMath.h>

bool TAries::fSetWave = false;

std::array<ROOT::Math::Polar3DVector, 77> TAries::fPaddlePosition = {
   // Aries positions from https://adamgarnsworthy.github.io/griffin-aries-angle-mapper/griffin-aries-angle-mapper.html
	// index 0 is a default value since our detector numbers start at 1!
   ROOT::Math::Polar3DVector(1., 0., 0.),
	ROOT::Math::Polar3DVector(1., 9.4 / 180. * TMath::Pi(),   45.0 / 180. * TMath::Pi()),
	ROOT::Math::Polar3DVector(1., 9.4 / 180. * TMath::Pi(),   135.0 / 180. * TMath::Pi()),
	ROOT::Math::Polar3DVector(1., 9.4 / 180. * TMath::Pi(),   225.0 / 180. * TMath::Pi()),
	ROOT::Math::Polar3DVector(1., 9.4 / 180. * TMath::Pi(),   315.0 / 180. * TMath::Pi()),
	ROOT::Math::Polar3DVector(1., 36.5 / 180. * TMath::Pi(),  51.6 / 180. * TMath::Pi()),
	ROOT::Math::Polar3DVector(1., 36.5 / 180. * TMath::Pi(),  83.4 / 180. * TMath::Pi()),
	ROOT::Math::Polar3DVector(1., 36.5 / 180. * TMath::Pi(),  141.6 / 180. * TMath::Pi()),
	ROOT::Math::Polar3DVector(1., 36.5 / 180. * TMath::Pi(),  173.4 / 180. * TMath::Pi()),
	ROOT::Math::Polar3DVector(1., 36.5 / 180. * TMath::Pi(),  231.6 / 180. * TMath::Pi()),
	ROOT::Math::Polar3DVector(1., 36.5 / 180. * TMath::Pi(),  263.4 / 180. * TMath::Pi()),
	ROOT::Math::Polar3DVector(1., 36.5 / 180. * TMath::Pi(),  321.6 / 180. * TMath::Pi()),
	ROOT::Math::Polar3DVector(1., 36.5 / 180. * TMath::Pi(),  353.4 / 180. * TMath::Pi()),
	ROOT::Math::Polar3DVector(1., 54.7 / 180. * TMath::Pi(),  22.5 / 180. * TMath::Pi()),
	ROOT::Math::Polar3DVector(1., 54.7 / 180. * TMath::Pi(),  112.5 / 180. * TMath::Pi()),
	ROOT::Math::Polar3DVector(1., 54.7 / 180. * TMath::Pi(),  202.5 / 180. * TMath::Pi()),
	ROOT::Math::Polar3DVector(1., 54.7 / 180. * TMath::Pi(),  292.5 / 180. * TMath::Pi()),
	ROOT::Math::Polar3DVector(1., 55.1 / 180. * TMath::Pi(),  56.0 / 180. * TMath::Pi()),
	ROOT::Math::Polar3DVector(1., 55.1 / 180. * TMath::Pi(),  79.0 / 180. * TMath::Pi()),
	ROOT::Math::Polar3DVector(1., 55.1 / 180. * TMath::Pi(),  146.0 / 180. * TMath::Pi()),
	ROOT::Math::Polar3DVector(1., 55.1 / 180. * TMath::Pi(),  169.0 / 180. * TMath::Pi()),
	ROOT::Math::Polar3DVector(1., 55.1 / 180. * TMath::Pi(),  236.0 / 180. * TMath::Pi()),
	ROOT::Math::Polar3DVector(1., 55.1 / 180. * TMath::Pi(),  259.0 / 180. * TMath::Pi()),
	ROOT::Math::Polar3DVector(1., 55.1 / 180. * TMath::Pi(),  326.0 / 180. * TMath::Pi()),
	ROOT::Math::Polar3DVector(1., 55.1 / 180. * TMath::Pi(),  349.0 / 180. * TMath::Pi()),
	ROOT::Math::Polar3DVector(1., 80.6 / 180. * TMath::Pi(),  13.0 / 180. * TMath::Pi()),
	ROOT::Math::Polar3DVector(1., 80.6 / 180. * TMath::Pi(),  32.0 / 180. * TMath::Pi()),
	ROOT::Math::Polar3DVector(1., 80.6 / 180. * TMath::Pi(),  58.0 / 180. * TMath::Pi()),
	ROOT::Math::Polar3DVector(1., 80.6 / 180. * TMath::Pi(),  77.0 / 180. * TMath::Pi()),
	ROOT::Math::Polar3DVector(1., 80.6 / 180. * TMath::Pi(),  103.0 / 180. * TMath::Pi()),
	ROOT::Math::Polar3DVector(1., 80.6 / 180. * TMath::Pi(),  122.0 / 180. * TMath::Pi()),
	ROOT::Math::Polar3DVector(1., 80.6 / 180. * TMath::Pi(),  148.0 / 180. * TMath::Pi()),
	ROOT::Math::Polar3DVector(1., 80.6 / 180. * TMath::Pi(),  167.0 / 180. * TMath::Pi()),
	ROOT::Math::Polar3DVector(1., 80.6 / 180. * TMath::Pi(),  193.0 / 180. * TMath::Pi()),
	ROOT::Math::Polar3DVector(1., 80.6 / 180. * TMath::Pi(),  212.0 / 180. * TMath::Pi()),
	ROOT::Math::Polar3DVector(1., 80.6 / 180. * TMath::Pi(),  238.0 / 180. * TMath::Pi()),
	ROOT::Math::Polar3DVector(1., 80.6 / 180. * TMath::Pi(),  257.0 / 180. * TMath::Pi()),
	ROOT::Math::Polar3DVector(1., 80.6 / 180. * TMath::Pi(),  283.0 / 180. * TMath::Pi()),
	ROOT::Math::Polar3DVector(1., 80.6 / 180. * TMath::Pi(),  302.0 / 180. * TMath::Pi()),
	ROOT::Math::Polar3DVector(1., 80.6 / 180. * TMath::Pi(),  328.0 / 180. * TMath::Pi()),
	ROOT::Math::Polar3DVector(1., 80.6 / 180. * TMath::Pi(),  347.0 / 180. * TMath::Pi()),
	ROOT::Math::Polar3DVector(1., 99.4 / 180. * TMath::Pi(),  13.0 / 180. * TMath::Pi()),
	ROOT::Math::Polar3DVector(1., 99.4 / 180. * TMath::Pi(),  32.0 / 180. * TMath::Pi()),
	ROOT::Math::Polar3DVector(1., 99.4 / 180. * TMath::Pi(),  58.0 / 180. * TMath::Pi()),
	ROOT::Math::Polar3DVector(1., 99.4 / 180. * TMath::Pi(),  77.0 / 180. * TMath::Pi()),
	ROOT::Math::Polar3DVector(1., 99.4 / 180. * TMath::Pi(),  103.0 / 180. * TMath::Pi()),
	ROOT::Math::Polar3DVector(1., 99.4 / 180. * TMath::Pi(),  122.0 / 180. * TMath::Pi()),
	ROOT::Math::Polar3DVector(1., 99.4 / 180. * TMath::Pi(),  148.0 / 180. * TMath::Pi()),
	ROOT::Math::Polar3DVector(1., 99.4 / 180. * TMath::Pi(),  167.0 / 180. * TMath::Pi()),
	ROOT::Math::Polar3DVector(1., 99.4 / 180. * TMath::Pi(),  193.0 / 180. * TMath::Pi()),
	ROOT::Math::Polar3DVector(1., 99.4 / 180. * TMath::Pi(),  212.0 / 180. * TMath::Pi()),
	ROOT::Math::Polar3DVector(1., 99.4 / 180. * TMath::Pi(),  238.0 / 180. * TMath::Pi()),
	ROOT::Math::Polar3DVector(1., 99.4 / 180. * TMath::Pi(),  257.0 / 180. * TMath::Pi()),
	ROOT::Math::Polar3DVector(1., 99.4 / 180. * TMath::Pi(),  283.0 / 180. * TMath::Pi()),
	ROOT::Math::Polar3DVector(1., 99.4 / 180. * TMath::Pi(),  302.0 / 180. * TMath::Pi()),
	ROOT::Math::Polar3DVector(1., 99.4 / 180. * TMath::Pi(),  328.0 / 180. * TMath::Pi()),
	ROOT::Math::Polar3DVector(1., 99.4 / 180. * TMath::Pi(),  347.0 / 180. * TMath::Pi()),
	ROOT::Math::Polar3DVector(1., 124.9 / 180. * TMath::Pi(), 56.0 / 180. * TMath::Pi()),
	ROOT::Math::Polar3DVector(1., 124.9 / 180. * TMath::Pi(), 79.0 / 180. * TMath::Pi()),
	ROOT::Math::Polar3DVector(1., 124.9 / 180. * TMath::Pi(), 146.0 / 180. * TMath::Pi()),
	ROOT::Math::Polar3DVector(1., 124.9 / 180. * TMath::Pi(), 169.0 / 180. * TMath::Pi()),
	ROOT::Math::Polar3DVector(1., 124.9 / 180. * TMath::Pi(), 236.0 / 180. * TMath::Pi()),
	ROOT::Math::Polar3DVector(1., 124.9 / 180. * TMath::Pi(), 259.0 / 180. * TMath::Pi()),
	ROOT::Math::Polar3DVector(1., 124.9 / 180. * TMath::Pi(), 326.0 / 180. * TMath::Pi()),
	ROOT::Math::Polar3DVector(1., 124.9 / 180. * TMath::Pi(), 349.0 / 180. * TMath::Pi()),
	ROOT::Math::Polar3DVector(1., 125.3 / 180. * TMath::Pi(), 22.5 / 180. * TMath::Pi()),
	ROOT::Math::Polar3DVector(1., 125.3 / 180. * TMath::Pi(), 112.5 / 180. * TMath::Pi()),
	ROOT::Math::Polar3DVector(1., 125.3 / 180. * TMath::Pi(), 202.5 / 180. * TMath::Pi()),
	ROOT::Math::Polar3DVector(1., 125.3 / 180. * TMath::Pi(), 292.5 / 180. * TMath::Pi()),
	ROOT::Math::Polar3DVector(1., 143.5 / 180. * TMath::Pi(), 51.6 / 180. * TMath::Pi()),
	ROOT::Math::Polar3DVector(1., 143.5 / 180. * TMath::Pi(), 83.4 / 180. * TMath::Pi()),
	ROOT::Math::Polar3DVector(1., 143.5 / 180. * TMath::Pi(), 141.6 / 180. * TMath::Pi()),
	ROOT::Math::Polar3DVector(1., 143.5 / 180. * TMath::Pi(), 173.4 / 180. * TMath::Pi()),
	ROOT::Math::Polar3DVector(1., 143.5 / 180. * TMath::Pi(), 231.6 / 180. * TMath::Pi()),
	ROOT::Math::Polar3DVector(1., 143.5 / 180. * TMath::Pi(), 263.4 / 180. * TMath::Pi()),
	ROOT::Math::Polar3DVector(1., 143.5 / 180. * TMath::Pi(), 321.6 / 180. * TMath::Pi()),
	ROOT::Math::Polar3DVector(1., 143.5 / 180. * TMath::Pi(), 353.4 / 180. * TMath::Pi())
};

TAries::TAries()
{
   // Default Constructor
   // Class()->AddRule("TAries aries_hits attributes=NotOwner");
   // Class()->AddRule("TAries ariesdata attributes=NotOwner");
   Clear();
}

TAries::TAries(const TAries& rhs) : TDetector(rhs)
{
   // Copy Contructor
   rhs.Copy(*this);
}

TAries& TAries::operator=(const TAries& rhs)
{
   rhs.Copy(*this);
   return *this;
}

void TAries::AddFragment(const std::shared_ptr<const TFragment>& frag, TChannel*)
{
   auto* hit = new TAriesHit(*frag);   // Construction of TAriesHit is handled in the constructor
   AddHit(hit);
}

void TAries::Print(Option_t*) const
{
   /// Prints out TAries Multiplicity, currently does little.
   Print(std::cout);
}

void TAries::Print(std::ostream& out) const
{
   std::ostringstream str;
   str << GetMultiplicity() << " hits" << std::endl;
   out << str.str();
}
