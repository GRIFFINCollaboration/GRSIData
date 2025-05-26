#include "TDemand.h"
#include "TMnemonic.h"

#include <cstdio>
#include <iostream>
#include <cmath>

#include "TClass.h"
#include "TMath.h"




/// \cond CLASSIMP
ClassImp(TDemand)
/// \endcond



//==========================================================================//
//==========================================================================//
//==========================================================================//
//==========================================================================//
//==========================================================================//
TDemand::TDemand()
{
   Clear();
}

TDemand::TDemand(const TDemand& rhs) : TDetector(rhs)
{
   rhs.Copy(*this);
}

void TDemand::Copy(TObject& rhs) const
{
   TDetector::Copy(rhs);
}

void TDemand::Print(Option_t*) const
{
   /// Prints out TDemand members, currently shows only multiplicity.
   Print(std::cout);
}

void TDemand::Print(std::ostream& out) const
{
   std::ostringstream str;
   str << GetMultiplicity() << " hits" << std::endl;
   out << str.str();
}

TDemand& TDemand::operator=(const TDemand& rhs)
{
   rhs.Copy(*this);
   return *this;
}

void TDemand::AddFragment(const std::shared_ptr<const TFragment>& frag, TChannel*)
{
   auto* hit = new TDemandHit(*frag);
   AddHit(hit);
}