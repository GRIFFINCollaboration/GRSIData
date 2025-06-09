#include "TDemand.h"
#include "TMnemonic.h"

#include <cstdio>
#include <iostream>
#include <cmath>

#include "TClass.h"
#include "TMath.h"

/// \cond CLASSIMP
//ClassImp(TDemand)
/// \endcond

//==========================================================================//
//==========================================================================//
//==========================================================================//
//==========================================================================//
//==========================================================================//

//approximate (x,y,z) of the TDemand OGS detectors. Using TIGRESS coordinate system.
const std::array<double, 6> TDemand::fXPositions = {55.5, 43.5, -43.5, -55.5, -43.5, 43.5};
const std::array<double, 6> TDemand::fYPositions = {0, 45.5, 45.5, 0, -45.5, -45.5};
const std::array<double, 6> TDemand::fZPositions = {-93.9, -93.9, -93.9, -93.9, -93.9, -93.9};

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

TVector3 TDemand::GetPosition(const TDemandHit& hit)
{
   return TDemand::GetPosition(hit.GetDetector());
}

TVector3 TDemand::GetPosition(int DetNbr)
{
   if(1 > DetNbr || 6 < DetNbr) {   //if detector number is not 1,2,3,4,5,6, then return a 0 vector
      return TVector3(0, 0, 0);
   }

   else {
      return TVector3(fXPositions[DetNbr - 1], fYPositions[DetNbr - 1], fZPositions[DetNbr - 1]);   //-1 because we index the detectors at 1 but the vector at 0
   }
}
