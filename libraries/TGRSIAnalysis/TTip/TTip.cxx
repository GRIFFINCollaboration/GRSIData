#include "TTip.h"

#include <iostream>

#include "TRandom.h"
#include "TMath.h"
#include "TClass.h"

/// \cond CLASSIMP
ClassImp(TTip)
/// \endcond

// Coordinates for the TIP detectors (in mm)
// Assumes the target position is at (0,0,0)
TVector3 TTip::fPositionVectors[128] = {
   TVector3(12.8039,0.0000,91.1047),
   TVector3(0.0000,12.8039,91.1047),
   TVector3(-12.8039,0.0000,91.1047),
   TVector3(0.0000,-12.8039,91.1047),
   TVector3(17.2004,0.0000,54.5526),
   TVector3(8.60019,14.896,54.5526),
   TVector3(-8.60019,14.896,54.5526),
   TVector3(-17.2004,0.0000,54.5526),
   TVector3(-8.60019,-14.896,54.5526),
   TVector3(8.60019,-14.896,54.5526),
   TVector3(31.1534,0.0000,47.972),
   TVector3(26.9796,15.5767,47.972),
   TVector3(15.5767,26.9796,47.972),
   TVector3(0.0000,31.1534,47.972),
   TVector3(-15.5767,26.9796,47.972),
   TVector3(-26.9796,15.5767,47.972),
   TVector3(-31.1534,0.0000,47.972),
   TVector3(-26.9796,-15.5767,47.972),
   TVector3(-15.5767,-26.9796,47.972),
   TVector3(0.0000,-31.1534,47.972),
   TVector3(15.5767,-26.9796,47.972),
   TVector3(26.9796,-15.5767,47.972),
   TVector3(42.8403,0.0000,37.9019),
   TVector3(39.5792,16.3943,37.9019),
   TVector3(30.2926,30.2926,37.9019),
   TVector3(16.3943,39.5792,37.9019),
   TVector3(0.0000,42.8403,37.9019),
   TVector3(-16.3943,39.5792,37.9019),
   TVector3(-30.2926,30.2926,37.9019),
   TVector3(-39.5792,16.3943,37.9019),
   TVector3(-42.8403,0.0000,37.9019),
   TVector3(-39.5792,-16.3943,37.9019),
   TVector3(-30.2926,-30.2926,37.9019),
   TVector3(-16.3943,-39.5792,37.9019),
   TVector3(0.0000,-42.8403,37.9019),
   TVector3(16.3943,-39.5792,37.9019),
   TVector3(30.2926,-30.2926,37.9019),
   TVector3(39.5792,-16.3943,37.9019),
   TVector3(51.411,0.0000,25.0748),
   TVector3(48.8948,15.8869,25.0748),
   TVector3(41.5924,30.2186,25.0748),
   TVector3(30.2186,41.5924,25.0748),
   TVector3(15.8869,48.8948,25.0748),
   TVector3(0.0000,51.411,25.0748),
   TVector3(-15.8869,48.8948,25.0748),
   TVector3(-30.2186,41.5924,25.0748),
   TVector3(-41.5924,30.2186,25.0748),
   TVector3(-48.8948,15.8869,25.0748),
   TVector3(-51.411,0.0000,25.0748),
   TVector3(-48.8948,-15.8869,25.0748),
   TVector3(-41.5924,-30.2186,25.0748),
   TVector3(-30.2186,-41.5924,25.0748),
   TVector3(-15.8869,-48.8948,25.0748),
   TVector3(0.0000,-51.411,25.0748),
   TVector3(15.8869,-48.8948,25.0748),
   TVector3(30.2186,-41.5924,25.0748),
   TVector3(41.5924,-30.2186,25.0748),
   TVector3(48.8948,-15.8869,25.0748),
   TVector3(56.2422,0.0000,10.4239),
   TVector3(52.8504,19.236,10.4239),
   TVector3(43.084,36.1518,10.4239),
   TVector3(28.1211,48.7072,10.4239),
   TVector3(9.76635,55.3877,10.4239),
   TVector3(-9.76635,55.3877,10.4239),
   TVector3(-28.1211,48.7072,10.4239),
   TVector3(-43.084,36.1518,10.4239),
   TVector3(-52.8504,19.236,10.4239),
   TVector3(-56.2422,0.0000,10.4239),
   TVector3(-52.8504,-19.236,10.4239),
   TVector3(-43.084,-36.1518,10.4239),
   TVector3(-28.1211,-48.7072,10.4239),
   TVector3(-9.76635,-55.3877,10.4239),
   TVector3(9.76635,-55.3877,10.4239),
   TVector3(28.1211,-48.7072,10.4239),
   TVector3(43.084,-36.1518,10.4239),
   TVector3(52.8504,-19.236,10.4239),
   TVector3(56.9823,0.0000,-4.98531),
   TVector3(53.5459,19.4891,-4.98531),
   TVector3(43.651,36.6275,-4.98531),
   TVector3(28.4912,49.3482,-4.98531),
   TVector3(9.89488,56.1166,-4.98531),
   TVector3(-9.89488,56.1166,-4.98531),
   TVector3(-28.4912,49.3482,-4.98531),
   TVector3(-43.651,36.6275,-4.98531),
   TVector3(-53.5459,19.4891,-4.98531),
   TVector3(-56.9823,0.0000,-4.98531),
   TVector3(-53.5459,-19.4891,-4.98531),
   TVector3(-43.651,-36.6275,-4.98531),
   TVector3(-28.4912,-49.3482,-4.98531),
   TVector3(-9.89488,-56.1166,-4.98531),
   TVector3(9.89488,-56.1166,-4.98531),
   TVector3(28.4912,-49.3482,-4.98531),
   TVector3(43.651,-36.6275,-4.98531),
   TVector3(53.5459,-19.4891,-4.98531),
   TVector3(53.0722,0.0000,-21.3349),
   TVector3(47.8164,23.0272,-21.3349),
   TVector3(33.09,41.4935,-21.3349),
   TVector3(11.8097,51.7416,-21.3349),
   TVector3(-11.8097,51.7416,-21.3349),
   TVector3(-33.09,41.4935,-21.3349),
   TVector3(-47.8164,23.0272,-21.3349),
   TVector3(-53.0722,0.0000,-21.3349),
   TVector3(-47.8164,-23.0272,-21.3349),
   TVector3(-33.09,-41.4935,-21.3349),
   TVector3(-11.8097,-51.7416,-21.3349),
   TVector3(11.8097,-51.7416,-21.3349),
   TVector3(33.09,-41.4935,-21.3349),
   TVector3(47.8164,-23.0272,-21.3349),
   TVector3(43.6891,0.0000,-36.9202),
   TVector3(37.8359,21.8446,-36.9202),
   TVector3(21.8446,37.8359,-36.9202),
   TVector3(0.0000,43.6891,-36.9202),
   TVector3(-21.8446,37.8359,-36.9202),
   TVector3(-37.8359,21.8446,-36.9202),
   TVector3(-43.6891,0.0000,-36.9202),
   TVector3(-37.8359,-21.8446,-36.9202),
   TVector3(-21.8446,-37.8359,-36.9202),
   TVector3(0.0000,-43.6891,-36.9202),
   TVector3(21.8446,-37.8359,-36.9202),
   TVector3(37.8359,-21.8446,-36.9202),
   TVector3(29.8869,0.0000,-48.771),
   TVector3(21.1332,21.1332,-48.771),
   TVector3(0.0000,29.8869,-48.771),
   TVector3(-21.1332,21.1332,-48.771),
   TVector3(-29.8869,0.0000,-48.771),
   TVector3(-21.1332,-21.1332,-48.771),
   TVector3(0.0000,-29.8869,-48.771),
   TVector3(21.1332,-21.1332,-48.771)};

TTip::TTip()
{
}

TTip::~TTip()
{
   // Default Destructor
}

TTip::TTip(const TTip& rhs) : TDetector()
{
#if ROOT_VERSION_CODE < ROOT_VERSION(6,0,0)
   Class()->IgnoreTObjectStreamer(kTRUE);
#endif
   rhs.Copy(*this);
}

TTip& TTip::operator=(const TTip& rhs)
{
   rhs.Copy(*this);
   return *this;
}

void TTip::AddFragment(const std::shared_ptr<const TFragment>& frag, TChannel* chan)
{
   if(frag == nullptr || chan == nullptr) {
      return;
   }

   TTipHit* hit = new TTipHit(*frag);
   hit->SetUpNumbering(chan);          // Think about moving this to ctor
   fHits.push_back(hit);
}

void TTip::Print(Option_t*) const
{
   /// Prints out TTip members, currently only prints the multiplicity.
	Print(std::cout);
}

void TTip::Print(std::ostream& out) const
{
	std::ostringstream str;
   str<<fHits.size()<<" fHits"<<std::endl;
	out<<str.str();
}

TVector3 TTip::GetPosition(const TTipHit& hit)
{
	return TTip::GetPosition(hit.GetTipChannel()-1);
}

TVector3 TTip::GetPosition(int DetNbr)
{
   if(DetNbr > 127) {
      return TVector3(0, 0, 1); //handle invalid detector index as in TGriffin
   }
	return fPositionVectors[DetNbr];
}
