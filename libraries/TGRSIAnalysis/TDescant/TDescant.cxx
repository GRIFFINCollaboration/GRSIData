#include <iostream>
#include <climits>

#include "TRandom.h"
#include "TMath.h"

#include "TDescant.h"
#include "TRunInfo.h"
#include "TGRSIDetectorInformation.h"
#include "TGRSIOptions.h"

bool TDescant::fSetWave = false;

std::array<TVector3, 71> TDescant::fSpherePosition = {
   // Descant positions from James' Thesis
   TVector3(0.0, 0.0, 1.0), TVector3(98.6, 0.0, 490.2), TVector3(30.5, 93.8, 490.2),
   TVector3(-79.8, 57.9, 490.2), TVector3(-79.8, -57.9, 490.2), TVector3(30.5, -93.8, 490.2),
   TVector3(208.7, 0.0, 454.4), TVector3(140.9, 102.4, 468.7), TVector3(64.5, 198.5, 454.4),
   TVector3(-53.8, 165.6, 468.7), TVector3(-168.8, 122.7, 454.4), TVector3(-174.2, 0.0, 468.7),
   TVector3(-168.8, -122.7, 454.4), TVector3(-53.8, -165.6, 468.7), TVector3(64.5, -198.5, 454.4),
   TVector3(140.9, -102.4, 468.7), TVector3(313.1, 0.0, 389.9), TVector3(256.9, 105.8, 415.7),
   TVector3(180.0, 211.6, 415.7), TVector3(96.7, 297.7, 389.9), TVector3(-21.2, 277.0, 415.7),
   TVector3(-145.6, 236.6, 415.7), TVector3(-253.3, 184.0, 389.9), TVector3(-270.0, 65.4, 415.7),
   TVector3(-270.0, -65.4, 415.7), TVector3(-253.3, -184.0, 389.9), TVector3(-145.6, -236.6, 415.7),
   TVector3(-21.2, -277.0, 415.7), TVector3(96.7, -297.7, 389.9), TVector3(180.0, -211.6, 415.7),
   TVector3(256.9, -105.8, 415.7), TVector3(394.3, 0.0, 307.4), TVector3(356.2, 102.4, 335.6),
   TVector3(291.3, 211.6, 346.9), TVector3(207.4, 307.1, 335.6), TVector3(121.9, 375.0, 307.4),
   TVector3(12.7, 370.4, 335.6), TVector3(-111.3, 342.4, 346.9), TVector3(-228.0, 292.2, 335.6),
   TVector3(-319.0, 231.8, 307.4), TVector3(-348.3, 126.5, 335.6), TVector3(-360.1, 0.0, 346.9),
   TVector3(-348.3, -126.5, 335.6), TVector3(-319.0, -231.8, 307.4), TVector3(-228.0, -292.2, 335.6),
   TVector3(-111.3, -342.4, 346.9), TVector3(12.7, -370.4, 335.6), TVector3(121.9, -375.0, 307.4),
   TVector3(207.4, -307.1, 335.6), TVector3(291.3, -211.6, 346.9), TVector3(356.2, -102.4, 335.6),
   TVector3(424.8, 93.8, 246.5), TVector3(377.5, 198.5, 260.9), TVector3(305.4, 297.7, 260.9),
   TVector3(220.4, 375.0, 246.5), TVector3(42.1, 433.0, 246.5), TVector3(-72.1, 420.4, 260.9),
   TVector3(-188.8, 382.5, 260.9), TVector3(-288.6, 325.6, 246.5), TVector3(-398.8, 173.8, 246.5),
   TVector3(-422.1, 61.3, 260.9), TVector3(-422.1, -61.3, 260.9), TVector3(-398.8, -173.8, 246.5),
   TVector3(-288.6, -325.6, 246.5), TVector3(-188.8, -382.5, 260.9), TVector3(-72.1, -420.4, 260.9),
   TVector3(42.1, -433.0, 246.5), TVector3(220.4, -375.0, 246.5), TVector3(305.4, -297.7, 260.9),
   TVector3(377.5, -198.5, 260.9), TVector3(424.8, -93.8, 246.5)};

std::array<TVector3, 9> TDescant::fAncillaryPosition = {
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

std::array<TVector3, 60> TDescant::fWallPosition = {
   // Descant detectors in wall configuration - sorted by detector number
   TVector3(195.966, 0.000, 1596.010), TVector3(-195.966, 0.000, 1596.010), TVector3(195.966, 492.837, 1596.010),
   TVector3(-195.966, 492.837, 1596.010), TVector3(-195.966, -492.837, 1596.010), TVector3(195.966, -492.837, 1596.010),
   TVector3(576.256, 0.000, 1501.200), TVector3(-576.256, 0.000, 1501.200), TVector3(576.256, 492.837, 1501.200),
   TVector3(-576.256, 492.837, 1501.200), TVector3(-576.256, -492.837, 1501.200), TVector3(576.256, -492.837, 1501.200),
   TVector3(208.851, 1050.481, 1700.946), TVector3(-208.851, 1050.481, 1700.946), TVector3(-208.851, -1050.481, 1700.946),
   TVector3(208.851, -1050.481, 1700.946), TVector3(922.311, 0.000, 1317.200), TVector3(-922.311, 0.000, 1317.200),
   TVector3(614.142, 1050.478, 1599.897), TVector3(-614.142, 1050.478, 1599.897), TVector3(-614.142, -1050.478, 1599.897),
   TVector3(614.142, -1050.478, 1599.897), TVector3(922.311, 492.837, 1317.200), TVector3(-922.311, 492.837, 1317.200),
   TVector3(-922.311, -492.837, 1317.200), TVector3(922.311, -492.837, 1317.200), TVector3(982.949, 1050.478, 1403.800),
   TVector3(-982.949, 1050.478, 1403.800), TVector3(-982.949, -1050.478, 1403.800), TVector3(982.949, -1050.478, 1403.800),
   TVector3(1213.570, 0.000, 1054.940), TVector3(-1213.570, 0.000, 1054.940), TVector3(1213.570, 492.837, 1054.940),
   TVector3(-1213.570, 492.837, 1054.940), TVector3(-1213.570, -492.837, 1054.940), TVector3(1213.570, -492.837, 1054.940),
   TVector3(1293.361, 1050.481, 1124.301), TVector3(-1293.361, 1050.481, 1124.301), TVector3(-1293.361, -1050.481, 1124.301),
   TVector3(1293.361, -1050.481, 1124.301), TVector3(1293.361, 1050.481, -1124.301), TVector3(-1293.361, 1050.481, -1124.301),
   TVector3(-1293.361, -1050.481, -1124.301), TVector3(1293.361, -1050.481, -1124.301), TVector3(1213.570, 492.837, -1054.940),
   TVector3(-1213.570, 492.837, -1054.940), TVector3(-1213.570, -492.837, -1054.940), TVector3(1213.570, -492.837, -1054.940),
   TVector3(1213.570, 0.000, -1054.940), TVector3(-1213.570, 0.000, -1054.940), TVector3(982.949, 1050.478, -1403.800),
   TVector3(-982.949, 1050.478, -1403.800), TVector3(-982.949, -1050.478, -1403.800), TVector3(982.949, -1050.478, -1403.800),
   TVector3(922.311, 492.837, -1317.200), TVector3(-922.311, 492.837, -1317.200), TVector3(-922.311, -492.837, -1317.200),
   TVector3(922.311, -492.837, -1317.200), TVector3(922.311, 0.000, -1317.200), TVector3(-922.311, 0.000, -1317.200)};

TDescant::TDescant()
{
   /// Default Constructor
   Clear();
}

void TDescant::Copy(TObject& rhs) const
{
   TDetector::Copy(rhs);
}

TDescant::TDescant(const TDescant& rhs) : TDetector(rhs)
{
   rhs.Copy(*this);
}

void TDescant::Clear(Option_t* opt)
{
   /// Clears all of the hits
   TDetector::Clear(opt);
   fHitFlag = false;
}

TDescant& TDescant::operator=(const TDescant& rhs)
{
   rhs.Copy(*this);
   return *this;
}

void TDescant::Print(Option_t*) const
{
   /// Prints out TDescant members, currently does little.
   Print(std::cout);
}

void TDescant::Print(std::ostream& out) const
{
   std::ostringstream str;
   str << GetMultiplicity() << " hits" << std::endl;
   out << str.str();
}

void TDescant::AddFragment(const std::shared_ptr<const TFragment>& frag, TChannel* chan)
{
   /// Builds the DESCANT Hits directly from the TFragment. Basically, loops through the data for an event and sets
   /// observables.
   /// This is done for both DESCANT and it's suppressors.
   if(frag == nullptr || chan == nullptr) {
      return;
   }

   auto* hit = new TDescantHit(*frag);
   AddHit(hit);
}

TVector3 TDescant::GetPosition(int DetNbr, double dist)
{
   // Gets the position vector for detector DetNbr
   // dist is only used when detectors are in the ancillary positions.
   if(TRunInfo::GetDetectorInformation() != nullptr && static_cast<TGRSIDetectorInformation*>(TRunInfo::GetDetectorInformation())->DescantAncillary()) {
      if(DetNbr > 8) {
         return {0, 0, 1};
      }
      TVector3 temp_pos(fAncillaryPosition[DetNbr]);
      temp_pos.SetMag(dist);
      return temp_pos;
   }

   if(TRunInfo::GetDetectorInformation() != nullptr && static_cast<TGRSIDetectorInformation*>(TRunInfo::GetDetectorInformation())->DescantWall()) {
      if(DetNbr > 60) {
         return {0, 0, 1};
      }
      return fWallPosition[DetNbr];
   }
   if(DetNbr > 70) {
      return {0, 0, 1};
   }
   return fSpherePosition[DetNbr];
}
