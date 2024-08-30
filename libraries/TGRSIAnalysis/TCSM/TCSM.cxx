#include "TCSM.h"
#include "TMath.h"

constexpr bool RecoverHits = true;
constexpr bool SumHits = false;

/// \cond CLASSIMP
ClassImp(TCSM)
/// \endcond

int TCSM::fCfdBuildDiff = 5;

TCSM::TCSM()
   : fAlmostEqualWindow(0.2)
{
#if ROOT_VERSION_CODE < ROOT_VERSION(6,0,0)
   Class()->IgnoreTObjectStreamer(kTRUE);
#endif
}

TCSM::~TCSM() = default;

void TCSM::AddFragment(const std::shared_ptr<const TFragment>& frag, TChannel* chan)
{
   /// This function just stores the fragments and mnemonics in vectors, separated by detector number and type
   /// (horizontal/vertical strip or pad).
   /// The hits themselves are built in the BuildHits function because the way we build them depends on the number of
   /// hits.

   // first index: detector number, second index: 0 = deltaE, 1 = E; third index: 0 = horizontal, 1 = vertical; fourth
   // index: fragments
   int type = -1;
   if(chan->GetMnemonic()->ArraySubPositionString().compare(0, 1, "D") == 0) {
      type = 0;
   } else if(chan->GetMnemonic()->ArraySubPositionString().compare(0, 1, "E") == 0) {
      type = 1;
   }
   int orientation = -1;
   if(chan->GetMnemonic()->CollectedChargeString().compare(0, 1, "N") == 0) {
      // N =  Horizontal Strips. aka "front"
      orientation = 0;
   } else if(chan->GetMnemonic()->CollectedChargeString().compare(0, 1, "P") == 0) {
      // P = Vertical Strips. aka "back"
      orientation = 1;
   }

   if(type < 0 || orientation < 0) {
      return;
   }

   // if this is the first time we got this detector number we make a new vector (of a vector) of fragments
   if(fFragments.find(chan->GetMnemonic()->ArrayPosition()) == fFragments.end()) {
      fFragments[chan->GetMnemonic()->ArrayPosition()].resize(
         2, std::vector<std::vector<std::pair<TFragment, TGRSIMnemonic>>>(2));
   }

   fFragments[chan->GetMnemonic()->ArrayPosition()][type][orientation].push_back(
      std::make_pair(*frag, *static_cast<const TGRSIMnemonic*>(chan->GetMnemonic())));
}

void TCSM::BuildHits()
{
   /// This function takes the fragments that were stored in the successive AddFragment calls and builds hits out of
   /// them
   std::vector<std::vector<TDetectorHit*>> hits(2);

   // first index: detector number, second index: 0 = deltaE, 1 = E; third index: 0 = horizontal, 1 = vertical
   // loop over all found detectors
   for(auto& fFragment : fFragments) {
      // loop over all types (only detectors 1/2 should have both D and E, detectors 3/4 should only have D)
      for(size_t i = 0; i < fFragment.second.size(); ++i) {
         BuildVH(fFragment.second.at(i), hits[i]);
      }
   }
   BuilddEE(hits, Hits());
}

TVector3 TCSM::GetPosition(int detector, char pos, int horizontalstrip, int verticalstrip, double X, double Y, double Z)
{
   // horizontal strips collect N charge!
   // vertical strips collect P charge!
   TVector3 Pos;
   double   SideX    = 68;
   double   SideZ    = -4.8834;
   double   dEX      = 54.9721;
   double   dEZ      = 42.948977;
   double   EX       = 58.062412;
   double   EZ       = 48.09198;
   double   detTheta = 31. * (TMath::Pi() / 180.);
   double   x        = 0.;
   double   y        = 0.;
   double   z        = 0.;

   if(detector == 3 && pos == 'D') {
      // Right Side
      verticalstrip = 15 - verticalstrip;
      x             = SideX;
      z             = SideZ + (50. / 32.) * (2 * verticalstrip + 1);
   } else if(detector == 4 && pos == 'D') {
      // Left Side
      x = -SideX;
      z = SideZ + (50. / 32.) * (2 * verticalstrip + 1);
   } else if(detector == 1 && pos == 'D') {
      // Right dE
      verticalstrip = 15 - verticalstrip;
      x             = dEX - (50. / 32.) * cos(detTheta) * (2 * verticalstrip + 1);
      z             = dEZ + (50. / 32.) * sin(detTheta) * (2 * verticalstrip + 1);
   } else if(detector == 2 && pos == 'D') {
      // Left dE
      x = -dEX + (50. / 32.) * cos(detTheta) * (2 * verticalstrip + 1);
      z = dEZ + (50. / 32.) * sin(detTheta) * (2 * verticalstrip + 1);
   } else if(detector == 1 && pos == 'E') {
      // Right E
      x = EX - (50. / 32.) * cos(detTheta) * (2 * verticalstrip + 1);
      z = EZ + (50. / 32.) * sin(detTheta) * (2 * verticalstrip + 1);
   } else if(detector == 2 && pos == 'E') {
      // Left E
      verticalstrip = 15 - verticalstrip;
      x             = -EX + (50. / 32.) * cos(detTheta) * (2 * verticalstrip + 1);
      z             = EZ + (50. / 32.) * sin(detTheta) * (2 * verticalstrip + 1);
   } else {
		std::cout<<"***Error, unrecognized detector and position combo!***"<<std::endl;
   }

   y = (50. / 32.) * (2 * horizontalstrip + 1) - (50 / 16.) * 8;
   Pos.SetX(x + X);
   Pos.SetY(y + Y);
   Pos.SetZ(z + Z);

   return Pos;
}

void TCSM::BuildVH(std::vector<std::vector<std::pair<TFragment, TGRSIMnemonic>>>& strips, std::vector<TDetectorHit*>& hitVector)
{
   /// Build hits from horizontal (index = 0) and vertical (index = 1) strips into the hitVector
   if(strips[0].empty() && strips[1].empty()) {
      return;
   }
   if(strips[0].size() == 1 && strips[1].empty()) {
      RecoverHit('H', strips[0][0], hitVector);
   } else if(strips[0].empty() && strips[1].size() == 1) {
      RecoverHit('V', strips[1][0], hitVector);
   } else if(strips[0].size() == 1 && strips[1].size() == 1) {
      hitVector.push_back(MakeHit(strips[0][0], strips[1][0]));
   } else if(strips[1].size() == 1 && strips[0].size() == 2) {
      auto he1 = static_cast<int>(strips[0][0].first.GetEnergy());
      auto he2 = static_cast<int>(strips[0][1].first.GetEnergy());
      auto ve1 = static_cast<int>(strips[1][0].first.GetEnergy());
      if(AlmostEqual(ve1, he1 + he2) && SumHits) {
         hitVector.push_back(MakeHit(strips[0], strips[1]));
      } else if(AlmostEqual(ve1, he1)) {
         hitVector.push_back(MakeHit(strips[0][0], strips[1][0]));
         RecoverHit('H', strips[0][1], hitVector);
      } else if(AlmostEqual(ve1, he2)) {
         hitVector.push_back(MakeHit(strips[0][1], strips[1][0]));
         RecoverHit('H', strips[0][0], hitVector);
      }
   } else if(strips[1].size() == 2 && strips[0].size() == 1) {
      auto he1 = static_cast<int>(strips[0][0].first.GetEnergy());
      auto ve1 = static_cast<int>(strips[1][0].first.GetEnergy());
      auto ve2 = static_cast<int>(strips[1][1].first.GetEnergy());
      if(AlmostEqual(ve1 + ve2, he1) && SumHits) {
         hitVector.push_back(MakeHit(strips[0], strips[1]));
      } else if(AlmostEqual(ve1, he1)) {
         hitVector.push_back(MakeHit(strips[0][0], strips[1][0]));
         RecoverHit('V', strips[1][1], hitVector);
      } else if(AlmostEqual(ve2, he1)) {
         hitVector.push_back(MakeHit(strips[0][0], strips[1][1]));
         RecoverHit('V', strips[1][0], hitVector);
      }
   } else if(strips[1].size() == 2 && strips[0].size() == 2) {
      auto he1 = static_cast<int>(strips[0][0].first.GetEnergy());
      auto he2 = static_cast<int>(strips[0][1].first.GetEnergy());
      auto ve1 = static_cast<int>(strips[1][0].first.GetEnergy());
      auto ve2 = static_cast<int>(strips[1][1].first.GetEnergy());
      if((AlmostEqual(ve1, he1) && AlmostEqual(ve2, he2)) || (AlmostEqual(ve1, he2) && AlmostEqual(ve2, he1))) {
         // I can build both 1,1 and 2,2 or 1,2 and 2,1
         if(std::abs(ve1 - he1) + std::abs(ve2 - he2) <= std::abs(ve1 - he2) + std::abs(ve2 - he1)) {
            // 1,1 and 2,2 mimimizes difference
            hitVector.push_back(MakeHit(strips[0][0], strips[1][0]));
            hitVector.push_back(MakeHit(strips[0][1], strips[1][1]));
         } else if(std::abs(ve1 - he1) + std::abs(ve2 - he2) > std::abs(ve1 - he2) + std::abs(ve2 - he1)) {
            // 1,2 and 2,1 mimimizes difference
            hitVector.push_back(MakeHit(strips[0][0], strips[1][1]));
            hitVector.push_back(MakeHit(strips[0][1], strips[1][0]));
         }
      } else if(AlmostEqual(ve1, he1)) {
         hitVector.push_back(MakeHit(strips[0][0], strips[1][0]));
      } else if(AlmostEqual(ve2, he1)) {
         hitVector.push_back(MakeHit(strips[0][1], strips[1][0]));
      } else if(AlmostEqual(ve1, he2)) {
         hitVector.push_back(MakeHit(strips[0][0], strips[1][1]));
      } else if(AlmostEqual(ve2, he2)) {
         hitVector.push_back(MakeHit(strips[0][1], strips[1][1]));
      }
   }
}

TCSMHit* TCSM::MakeHit(std::pair<TFragment, TGRSIMnemonic>& h, std::pair<TFragment, TGRSIMnemonic>& v)
{
   auto* csmHit = new TCSMHit;

   if(h.second.ArrayPosition() != v.second.ArrayPosition()) {
      std::cerr<<"\tSomething is wrong, Horizontal and Vertical detector numbers don't match."<<std::endl;
   }
   if(h.second.ArraySubPositionString().c_str()[0] != v.second.ArraySubPositionString().c_str()[0]) {
      std::cerr<<"\tSomething is wrong, Horizontal and Vertical positions don't match."<<std::endl;
   }

   if(h.second.ArraySubPositionString()[0] == 'D') {
      csmHit->SetDetectorNumber(h.second.ArrayPosition());
      csmHit->SetDHorizontalCharge(h.first.GetCharge());
      csmHit->SetDVerticalCharge(v.first.GetCharge());
      csmHit->SetDHorizontalStrip(h.second.Segment());
      csmHit->SetDVerticalStrip(v.second.Segment());
      csmHit->SetDHorizontalCFD(static_cast<int>(h.first.GetCfd()));
      csmHit->SetDVerticalCFD(static_cast<int>(v.first.GetCfd()));
      csmHit->SetDHorizontalTime(h.first.GetTimeStamp());
      csmHit->SetDVerticalTime(v.first.GetTimeStamp());
      csmHit->SetDHorizontalEnergy(h.first.GetEnergy());
      csmHit->SetDVerticalEnergy(v.first.GetEnergy());
      csmHit->SetDPosition(TCSM::GetPosition(h.second.ArrayPosition(), h.second.ArraySubPositionString()[0],
                                            h.second.Segment(), v.second.Segment()));
   } else if(h.second.ArraySubPositionString().c_str()[0] == 'E') {
      csmHit->SetDetectorNumber(h.second.ArrayPosition());
      csmHit->SetEHorizontalCharge(h.first.GetCharge());
      csmHit->SetEVerticalCharge(v.first.GetCharge());
      csmHit->SetEHorizontalStrip(h.second.Segment());
      csmHit->SetEVerticalStrip(v.second.Segment());
      csmHit->SetEHorizontalCFD(static_cast<int>(h.first.GetCfd()));
      csmHit->SetEVerticalCFD(static_cast<int>(v.first.GetCfd()));
      csmHit->SetEHorizontalTime(h.first.GetTimeStamp());
      csmHit->SetEVerticalTime(v.first.GetTimeStamp());
      csmHit->SetEHorizontalEnergy(h.first.GetEnergy());
      csmHit->SetEVerticalEnergy(v.first.GetEnergy());
      csmHit->SetEPosition(TCSM::GetPosition(h.second.ArrayPosition(), h.second.ArraySubPositionString()[0],
                                            h.second.Segment(), v.second.Segment()));
   }

   return csmHit;
}

TCSMHit* TCSM::MakeHit(std::vector<std::pair<TFragment, TGRSIMnemonic>>& hhV,
                      std::vector<std::pair<TFragment, TGRSIMnemonic>>& vvV)
{
   auto* csmHit = new TCSMHit;

   if(hhV.empty() || vvV.empty()) {
      std::cerr<<"\tSomething is wrong, empty vector in MakeHit"<<std::endl;
   }

   //-------------------- horizontal strips
   int    DetNumH  = hhV[0].second.ArrayPosition();
   char   DetPosH  = hhV[0].second.ArraySubPositionString()[0];
   int    ChargeH  = static_cast<int>(hhV[0].first.GetCharge());
   double EnergyH  = hhV[0].first.GetEnergy();
   int    biggestH = 0;

   // get accumulative charge/energy and find the strip with the highest charge (why not energy?)
   for(size_t i = 1; i < hhV.size(); ++i) {
      if(hhV[i].first.GetCharge() > hhV[biggestH].first.GetCharge()) {
         biggestH = i;
      }

      if(hhV[i].second.ArrayPosition() != DetNumH) {
         std::cerr<<"\tSomething is wrong, Horizontal detector numbers don't match in vector loop."<<std::endl;
      }
      if(hhV[i].second.ArraySubPositionString()[0] != DetPosH) {
         std::cerr<<"\tSomething is wrong, Horizontal detector positions don't match in vector loop."<<std::endl;
      }
      ChargeH += static_cast<int>(hhV[i].first.GetCharge());
      EnergyH += hhV[i].first.GetEnergy();
   }

   int  StripH  = hhV[biggestH].second.Segment();
   auto ConFraH = static_cast<int>(hhV[biggestH].first.GetCfd());
   auto TimeH   = static_cast<double>(hhV[biggestH].first.GetTimeStamp());

   //-------------------- vertical strips
   int    DetNumV  = vvV[0].second.ArrayPosition();
   char   DetPosV  = vvV[0].second.ArraySubPositionString()[0];
   auto   ChargeV  = static_cast<int>(vvV[0].first.GetCharge());
   double EnergyV  = vvV[0].first.GetEnergy();
   int    biggestV = 0;

   // get accumulative charge/energy and find the strip with the highest charge (why not energy?)
   for(size_t i = 1; i < vvV.size(); ++i) {
      if(vvV[i].first.GetCharge() > vvV[biggestV].first.GetCharge()) {
         biggestV = i;
      }

      if(vvV[i].second.ArrayPosition() != DetNumV) {
         std::cerr<<"\tSomething is wrong, Vertical detector numbers don't match in vector loop."<<std::endl;
      }
      if(vvV[i].second.ArraySubPositionString()[0] != DetPosV) {
         std::cerr<<"\tSomething is wrong, Vertical detector positions don't match in vector loop."<<std::endl;
      }
      ChargeV += static_cast<int>(vvV[i].first.GetCharge());
      EnergyV += vvV[i].first.GetEnergy();
   }

   int  StripV  = vvV[biggestV].second.Segment();
   auto ConFraV = static_cast<int>(vvV[biggestV].first.GetCfd());
   auto TimeV   = static_cast<double>(vvV[biggestV].first.GetTimeStamp());

   if(DetNumH != DetNumV) {
      std::cerr<<"\tSomething is wrong, Horizontal and Vertical detector numbers don't match in vector."<<std::endl;
   }
   if(DetPosH != DetPosV) {
      std::cerr<<"\tSomething is wrong, Horizontal and Vertical positions don't match in vector."<<std::endl;
   }

   if(DetPosH == 'D') {
      csmHit->SetDetectorNumber(DetNumH);
      csmHit->SetDHorizontalCharge(static_cast<float>(ChargeH));
      csmHit->SetDVerticalCharge(static_cast<float>(ChargeV));
      csmHit->SetDHorizontalStrip(StripH);
      csmHit->SetDVerticalStrip(StripV);
      csmHit->SetDHorizontalCFD(ConFraH);
      csmHit->SetDVerticalCFD(ConFraV);
      csmHit->SetDHorizontalTime(static_cast<int>(TimeH));
      csmHit->SetDVerticalTime(static_cast<int>(TimeV));
      csmHit->SetDHorizontalEnergy(EnergyH);
      csmHit->SetDVerticalEnergy(EnergyV);
      csmHit->SetDPosition(TCSM::GetPosition(DetNumH, DetPosH, StripH, StripV));
   } else if(DetPosH == 'E') {
      csmHit->SetDetectorNumber(DetNumH);
      csmHit->SetEHorizontalCharge(static_cast<float>(ChargeH));
      csmHit->SetEVerticalCharge(static_cast<float>(ChargeV));
      csmHit->SetEHorizontalStrip(StripH);
      csmHit->SetEVerticalStrip(StripV);
      csmHit->SetEHorizontalCFD(ConFraH);
      csmHit->SetEVerticalCFD(ConFraV);
      csmHit->SetEHorizontalTime(static_cast<int>(TimeH));
      csmHit->SetEVerticalTime(static_cast<int>(TimeV));
      csmHit->SetEHorizontalEnergy(EnergyH);
      csmHit->SetEVerticalEnergy(EnergyV);
      csmHit->SetEPosition(TCSM::GetPosition(DetNumH, DetPosH, StripH, StripV));
   }

   return csmHit;
}

void TCSM::BuilddEE(std::vector<std::vector<TDetectorHit*>>& hitVec, std::vector<TDetectorHit*>& builtHits)
{
   std::vector<TDetectorHit*> d1;
   std::vector<TDetectorHit*> d2;
   std::vector<TDetectorHit*> e1;
   std::vector<TDetectorHit*> e2;

   for(auto& i : hitVec[0]) {
		auto* hit = static_cast<TCSMHit*>(i);
      if(hit->GetDetectorNumber() == 3 || hit->GetDetectorNumber() == 4) { // I am in side detectors
         // I will never have a pair in the side detector, so go ahead and send it through.
         builtHits.push_back(i);
      } else if(hit->GetDetectorNumber() == 1) {
         d1.push_back(i);
      } else if(hit->GetDetectorNumber() == 2) {
         d2.push_back(i);
      } else {
         std::cerr<<"  Caution, in BuilddEE detector number in D vector is out of bounds."<<std::endl;
      }
   }

   for(auto& i : hitVec[1]) {
		auto* hit = static_cast<TCSMHit*>(i);
      if(hit->GetDetectorNumber() == 1) {
         e1.push_back(i);
      } else if(hit->GetDetectorNumber() == 2) {
         e2.push_back(i);
      } else {
         std::cerr<<"  Caution, in BuilddEE detector number in E vector is out of bounds."<<std::endl;
      }
   }

   MakedEE(d1, e1, builtHits);
   MakedEE(d2, e2, builtHits);
}

void TCSM::MakedEE(std::vector<TDetectorHit*>& DHitVec, std::vector<TDetectorHit*>& EHitVec, std::vector<TDetectorHit*>& BuiltHits)
{
   if(DHitVec.empty() && EHitVec.empty()) {
      return;
   }
   if(DHitVec.size() == 1 && EHitVec.empty()) {
      BuiltHits.push_back(DHitVec.at(0));
   } else if(DHitVec.empty() && EHitVec.size() == 1) {
      BuiltHits.push_back(EHitVec.at(0));
   } else if(DHitVec.size() == 1 && EHitVec.size() == 1) {
      BuiltHits.push_back(CombineHits(DHitVec.at(0), EHitVec.at(0)));
   } else if(DHitVec.size() == 2 && EHitVec.empty()) {
      BuiltHits.push_back(DHitVec.at(0));
      BuiltHits.push_back(DHitVec.at(1));
   } else if(DHitVec.empty() && EHitVec.size() == 2) {
      BuiltHits.push_back(EHitVec.at(0));
      BuiltHits.push_back(EHitVec.at(1));
   } else if(DHitVec.size() == 2 && EHitVec.size() == 1) {
      double dt1 = static_cast<TCSMHit*>(DHitVec.at(0))->GetDPosition().Theta();
      double dt2 = static_cast<TCSMHit*>(DHitVec.at(1))->GetDPosition().Theta();
      double et  = static_cast<TCSMHit*>(EHitVec.at(0))->GetEPosition().Theta();

      if(std::abs(dt1 - et) <= std::abs(dt2 - et)) {
         BuiltHits.push_back(CombineHits(DHitVec.at(0), EHitVec.at(0)));
         // BuiltHits.back().Print();
         BuiltHits.push_back(DHitVec.at(1));
         // BuiltHits.back().Print();
      } else {
         BuiltHits.push_back(CombineHits(DHitVec.at(1), EHitVec.at(0)));
         // BuiltHits.back().Print();
         BuiltHits.push_back(DHitVec.at(0));
         // BuiltHits.back().Print();
      }
   } else if(DHitVec.size() == 1 && EHitVec.size() == 2) {
      double dt  = static_cast<TCSMHit*>(DHitVec.at(0))->GetDPosition().Theta();
      double et1 = static_cast<TCSMHit*>(EHitVec.at(0))->GetEPosition().Theta();
      double et2 = static_cast<TCSMHit*>(EHitVec.at(0))->GetEPosition().Theta();

      if(std::abs(dt - et1) <= std::abs(dt - et2)) {
         BuiltHits.push_back(CombineHits(DHitVec.at(0), EHitVec.at(0)));
         // BuiltHits.back().Print();
         BuiltHits.push_back(EHitVec.at(1));
         // BuiltHits.back().Print();
      } else {
         BuiltHits.push_back(CombineHits(DHitVec.at(0), EHitVec.at(1)));
         // BuiltHits.back().Print();
         BuiltHits.push_back(EHitVec.at(0));
         // BuiltHits.back().Print();
      }
   } else if(DHitVec.size() == 2 && EHitVec.size() == 2) {
      double dt1 = static_cast<TCSMHit*>(DHitVec.at(0))->GetDPosition().Theta();
      double dt2 = static_cast<TCSMHit*>(DHitVec.at(1))->GetDPosition().Theta();
      double et1 = static_cast<TCSMHit*>(EHitVec.at(0))->GetEPosition().Theta();
      double et2 = static_cast<TCSMHit*>(EHitVec.at(1))->GetEPosition().Theta();

      if(std::abs(dt1 - et1) + std::abs(dt2 - et2) <= std::abs(dt1 - et2) + std::abs(dt2 - et1)) {
         BuiltHits.push_back(CombineHits(DHitVec.at(0), EHitVec.at(0)));
         // BuiltHits.back().Print();
         BuiltHits.push_back(CombineHits(DHitVec.at(1), EHitVec.at(1)));
         // BuiltHits.back().Print();
      } else {
         BuiltHits.push_back(CombineHits(DHitVec.at(0), EHitVec.at(1)));
         // BuiltHits.back().Print();
         BuiltHits.push_back(CombineHits(DHitVec.at(1), EHitVec.at(0)));
         // BuiltHits.back().Print();
      }
   } else {
      std::cout<<"D Size: "<<DHitVec.size()<<" E Size: "<<EHitVec.size()<<std::endl;
   }
}

void TCSM::OldBuilddEE(std::vector<TDetectorHit*>& DHitVec, std::vector<TDetectorHit*>& EHitVec, std::vector<TDetectorHit*>& BuiltHits)
{
   bool printbit = false;
   if(DHitVec.empty() && EHitVec.empty()) { // Why am I even here?!
      return;
   }

   std::vector<bool> EUsed(EHitVec.size(), false);
   std::vector<bool> DUsed(DHitVec.size(), false);

   for(size_t diter = 0; diter < DHitVec.size(); diter++) {
      if(DUsed.at(diter)) {
         continue;
      }

      for(size_t eiter = 0; eiter < EHitVec.size(); eiter++) {
         if(EUsed.at(eiter)) {
            continue;
         }

         if(static_cast<TCSMHit*>(DHitVec.at(diter))->GetDetectorNumber() ==
            static_cast<TCSMHit*>(EHitVec.at(eiter))->GetDetectorNumber()) { // Hits are in the same stack
            if(AlmostEqual(static_cast<TCSMHit*>(DHitVec.at(diter))->GetDPosition().Theta(),
                           static_cast<TCSMHit*>(EHitVec.at(eiter))->GetEPosition().Theta())) { // Same-ish Theta
               //&& AlmostEqual(DHitVec.at(diter).GetDPosition().Phi(),EHitVec.at(eiter).GetEPosition().Phi())
               //)//Same-ish Phi
               BuiltHits.push_back(CombineHits(DHitVec.at(diter), EHitVec.at(eiter)));
               DUsed.at(diter) = true;
               EUsed.at(eiter) = true;
               break;
            }
         }
      }
   }

   // This loop adds uncorrelated events in the telescope together.  This may be bad, but let's see.
   for(size_t i = 0; i < DHitVec.size(); i++) {
      if(!DUsed.at(i)) {
         for(size_t j = 0; j < EHitVec.size(); j++) {
            if(!EUsed.at(j)) {
               if(static_cast<TCSMHit*>(EHitVec.at(j))->GetDetectorNumber() == static_cast<TCSMHit*>(DHitVec.at(i))->GetDetectorNumber()) {
                  BuiltHits.push_back(CombineHits(DHitVec.at(i), EHitVec.at(j)));
                  DUsed.at(i) = true;
                  EUsed.at(j) = true;
                  break;
               }
            }
         }
      }
   }

   // Send through the stragglers.  This is very permissive, but we trust BuildVH to take care of the riff-raff
   for(size_t i = 0; i < DHitVec.size(); i++) {
      if(!DUsed.at(i)) {
         BuiltHits.push_back(DHitVec.at(i));
      }
   }
   for(size_t j = 0; j < EHitVec.size(); j++) {
      if(!EUsed.at(j)) {
         BuiltHits.push_back(EHitVec.at(j));
      }
   }

   if(printbit) {
      for(auto& BuiltHit : BuiltHits) {
         std::cout<<DGREEN;
         BuiltHit->Print();
         std::cout<<RESET_COLOR<<std::endl;
      }
   }
}

void TCSM::RecoverHit(char orientation, std::pair<TFragment, TGRSIMnemonic>& hit, std::vector<TDetectorHit*>& hits)
{
   if(!RecoverHits) {
      return;
   }

   auto* csmHit = new TCSMHit;

   int  detno = hit.second.ArrayPosition();
   char pos   = hit.second.ArraySubPositionString()[0];

   switch(detno) {
   case 1: return;
   case 2:
      if(pos == 'D' && orientation == 'V') { // Recover 2DN09, channel 1040
         csmHit->SetDetectorNumber(detno);
         csmHit->SetDHorizontalCharge(hit.first.GetCharge());
         csmHit->SetDVerticalCharge(hit.first.GetCharge());
         csmHit->SetDHorizontalStrip(9);
         csmHit->SetDVerticalStrip(hit.second.Segment());
         csmHit->SetDHorizontalCFD(static_cast<int>(hit.first.GetCfd()));
         csmHit->SetDVerticalCFD(static_cast<int>(hit.first.GetCfd()));
         csmHit->SetDHorizontalTime(hit.first.GetTimeStamp());
         csmHit->SetDVerticalTime(hit.first.GetTimeStamp());
         csmHit->SetDHorizontalEnergy(hit.first.GetEnergy());
         csmHit->SetDVerticalEnergy(hit.first.GetEnergy());
         csmHit->SetDPosition(TCSM::GetPosition(detno, pos, 9, hit.second.Segment()));
      }
      break;
   case 3:
      if(pos == 'E') {
         std::cerr<<"3E in RecoverHit"<<std::endl;

         return;
      } else if(orientation == 'H') { // Recover 3DP11, channel 1145
         csmHit->SetDetectorNumber(detno);
         csmHit->SetDHorizontalCharge(hit.first.GetCharge());
         csmHit->SetDVerticalCharge(hit.first.GetCharge());
         csmHit->SetDHorizontalStrip(hit.second.Segment());
         csmHit->SetDVerticalStrip(11);
         csmHit->SetDHorizontalCFD(static_cast<int>(hit.first.GetCfd()));
         csmHit->SetDVerticalCFD(static_cast<int>(hit.first.GetCfd()));
         csmHit->SetDHorizontalTime(hit.first.GetTimeStamp());
         csmHit->SetDVerticalTime(hit.first.GetTimeStamp());
         csmHit->SetDHorizontalEnergy(hit.first.GetEnergy());
         csmHit->SetDVerticalEnergy(hit.first.GetEnergy());
         csmHit->SetDPosition(TCSM::GetPosition(detno, pos, hit.second.Segment(), 11));
      }
      break;
   case 4:
      if(pos == 'E') {
         std::cerr<<"4E in RecoverHit"<<std::endl;
         return;
      } else if(orientation == 'H') { // Recover 4DP15, channel 1181
         csmHit->SetDetectorNumber(detno);
         csmHit->SetDHorizontalCharge(hit.first.GetCharge());
         csmHit->SetDVerticalCharge(hit.first.GetCharge());
         csmHit->SetDHorizontalStrip(hit.second.Segment());
         csmHit->SetDVerticalStrip(15);
         csmHit->SetDHorizontalCFD(static_cast<int>(hit.first.GetCfd()));
         csmHit->SetDVerticalCFD(static_cast<int>(hit.first.GetCfd()));
         csmHit->SetDHorizontalTime(hit.first.GetTimeStamp());
         csmHit->SetDVerticalTime(hit.first.GetTimeStamp());
         csmHit->SetDHorizontalEnergy(hit.first.GetEnergy());
         csmHit->SetDVerticalEnergy(hit.first.GetEnergy());
         csmHit->SetDPosition(TCSM::GetPosition(detno, pos, hit.second.Segment(), 15));
      }
      break;
   default:
      std::cerr<<"Something is wrong.  The detector number in recover hit is out of bounds."<<std::endl;
      return;
   }

   if(!csmHit->IsEmpty()) {
      hits.push_back(csmHit);
   }
}

TCSMHit* TCSM::CombineHits(TDetectorHit* d_hit, TDetectorHit* e_hit)
{
	auto* dHit = static_cast<TCSMHit*>(d_hit);
	auto* eHit = static_cast<TCSMHit*>(e_hit);
   if(dHit->GetDetectorNumber() != eHit->GetDetectorNumber()) {
      std::cerr<<"Something is wrong.  In combine hits, the detector numbers don't match"<<std::endl;
   }

   dHit->SetEHorizontalStrip(eHit->GetEHorizontalStrip());
   dHit->SetEVerticalStrip(eHit->GetEVerticalStrip());

   dHit->SetEHorizontalCharge(eHit->GetEHorizontalCharge());
   dHit->SetEVerticalCharge(eHit->GetEVerticalCharge());

   dHit->SetEHorizontalEnergy(eHit->GetEHorizontalEnergy());
   dHit->SetEVerticalEnergy(eHit->GetEVerticalEnergy());

   dHit->SetEHorizontalCFD(eHit->GetEHorizontalCFD());
   dHit->SetEVerticalCFD(eHit->GetEVerticalCFD());

   dHit->SetEHorizontalTime(static_cast<int>(eHit->GetEHorizontalTime()));
   dHit->SetEVerticalTime(static_cast<int>(eHit->GetEVerticalTime()));

   dHit->SetEPosition(eHit->GetEPosition());

   return dHit;
}

bool TCSM::AlmostEqual(int val1, int val2) const
{
   auto   diff = static_cast<double>(std::abs(val1 - val2));
   double ave  = (val1 + val2) / 2.;
   double frac = diff / ave;
   return frac < fAlmostEqualWindow;
}

bool TCSM::AlmostEqual(double val1, double val2) const
{
   double frac = std::fabs(val1 - val2) / ((val1 + val2) / 2.);
   return frac < fAlmostEqualWindow;
}
