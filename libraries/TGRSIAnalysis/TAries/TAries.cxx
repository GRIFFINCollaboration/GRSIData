#include <iostream>
#include "TAries.h"
#include <TRandom.h>
#include <TMath.h>

bool TAries::fSetWave = false;

std::array<TVector3, 21> TAries::fPaddlePosition = {
   // Aries positions from Evan; Thanks Evan.
   TVector3(0, 0, 1),
   TVector3(14.3025, 4.6472, 22.8096),
   TVector3(0, 15.0386, 22.8096),
   TVector3(-14.3025, 4.6472, 22.8096),
   TVector3(-8.8395, -12.1665, 22.8096),
   TVector3(8.8395, -12.1665, 22.8096),
   TVector3(19.7051, 6.4026, 6.2123),
   TVector3(0, 20.7192, 6.2123),
   TVector3(-19.7051, 6.4026, 6.2123),
   TVector3(-12.1784, -16.7622, 6.2123),
   TVector3(12.1784, -16.7622, 6.2123),
   TVector3(19.7051, 6.4026, -6.2123),
   TVector3(0, 20.7192, -6.2123),
   TVector3(-19.7051, 6.4026, -6.2123),
   TVector3(-12.1784, -16.7622, -6.2123),
   TVector3(12.1784, -16.7622, -6.2123),
   TVector3(14.3025, 4.6472, -22.8096),
   TVector3(0, 15.0386, -22.8096),
   TVector3(-14.3025, 4.6472, -22.8096),
   TVector3(-8.8395, -12.1665, -22.8096),
   TVector3(8.8395, -12.1665, -22.8096)};

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
