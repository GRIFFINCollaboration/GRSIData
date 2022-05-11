#include "TTip.h"

#include <iostream>

#include "TRandom.h"
#include "TMath.h"
#include "TClass.h"

/// \cond CLASSIMP
ClassImp(TTip)
/// \endcond

TTransientBits<UShort_t> TTip::fgTipBits;
TVector3 TTip::fPositionVectors[128];

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
	if(!GetVectorsBuilt()){
		BuildVectors();
	}

	return fPositionVectors[DetNbr];
}

void TTip::BuildVectors()
{
   // Set the coordinates for the TIP detectors (in mm)
   // Assumes the target position is at (0,0,0)
   fPositionVectors[0].SetXYZ(12.8039,0.0000,91.1047);
   fPositionVectors[1].SetXYZ(0.0000,12.8039,91.1047);
   fPositionVectors[2].SetXYZ(-12.8039,0.0000,91.1047);
   fPositionVectors[3].SetXYZ(0.0000,-12.8039,91.1047);
   fPositionVectors[4].SetXYZ(17.2004,0.0000,54.5526);
   fPositionVectors[5].SetXYZ(8.60019,14.896,54.5526);
   fPositionVectors[6].SetXYZ(-8.60019,14.896,54.5526);
   fPositionVectors[7].SetXYZ(-17.2004,0.0000,54.5526);
   fPositionVectors[8].SetXYZ(-8.60019,-14.896,54.5526);
   fPositionVectors[9].SetXYZ(8.60019,-14.896,54.5526);
   fPositionVectors[10].SetXYZ(31.1534,0.0000,47.972);
   fPositionVectors[11].SetXYZ(26.9796,15.5767,47.972);
   fPositionVectors[12].SetXYZ(15.5767,26.9796,47.972);
   fPositionVectors[13].SetXYZ(0.0000,31.1534,47.972);
   fPositionVectors[14].SetXYZ(-15.5767,26.9796,47.972);
   fPositionVectors[15].SetXYZ(-26.9796,15.5767,47.972);
   fPositionVectors[16].SetXYZ(-31.1534,0.0000,47.972);
   fPositionVectors[17].SetXYZ(-26.9796,-15.5767,47.972);
   fPositionVectors[18].SetXYZ(-15.5767,-26.9796,47.972);
   fPositionVectors[19].SetXYZ(0.0000,-31.1534,47.972);
   fPositionVectors[20].SetXYZ(15.5767,-26.9796,47.972);
   fPositionVectors[21].SetXYZ(26.9796,-15.5767,47.972);
   fPositionVectors[22].SetXYZ(42.8403,0.0000,37.9019);
   fPositionVectors[23].SetXYZ(39.5792,16.3943,37.9019);
   fPositionVectors[24].SetXYZ(30.2926,30.2926,37.9019);
   fPositionVectors[25].SetXYZ(16.3943,39.5792,37.9019);
   fPositionVectors[26].SetXYZ(0.0000,42.8403,37.9019);
   fPositionVectors[27].SetXYZ(-16.3943,39.5792,37.9019);
   fPositionVectors[28].SetXYZ(-30.2926,30.2926,37.9019);
   fPositionVectors[29].SetXYZ(-39.5792,16.3943,37.9019);
   fPositionVectors[30].SetXYZ(-42.8403,0.0000,37.9019);
   fPositionVectors[31].SetXYZ(-39.5792,-16.3943,37.9019);
   fPositionVectors[32].SetXYZ(-30.2926,-30.2926,37.9019);
   fPositionVectors[33].SetXYZ(-16.3943,-39.5792,37.9019);
   fPositionVectors[34].SetXYZ(0.0000,-42.8403,37.9019);
   fPositionVectors[35].SetXYZ(16.3943,-39.5792,37.9019);
   fPositionVectors[36].SetXYZ(30.2926,-30.2926,37.9019);
   fPositionVectors[37].SetXYZ(39.5792,-16.3943,37.9019);
   fPositionVectors[38].SetXYZ(51.411,0.0000,25.0748);
   fPositionVectors[39].SetXYZ(48.8948,15.8869,25.0748);
   fPositionVectors[40].SetXYZ(41.5924,30.2186,25.0748);
   fPositionVectors[41].SetXYZ(30.2186,41.5924,25.0748);
   fPositionVectors[42].SetXYZ(15.8869,48.8948,25.0748);
   fPositionVectors[43].SetXYZ(0.0000,51.411,25.0748);
   fPositionVectors[44].SetXYZ(-15.8869,48.8948,25.0748);
   fPositionVectors[45].SetXYZ(-30.2186,41.5924,25.0748);
   fPositionVectors[46].SetXYZ(-41.5924,30.2186,25.0748);
   fPositionVectors[47].SetXYZ(-48.8948,15.8869,25.0748);
   fPositionVectors[48].SetXYZ(-51.411,0.0000,25.0748);
   fPositionVectors[49].SetXYZ(-48.8948,-15.8869,25.0748);
   fPositionVectors[50].SetXYZ(-41.5924,-30.2186,25.0748);
   fPositionVectors[51].SetXYZ(-30.2186,-41.5924,25.0748);
   fPositionVectors[52].SetXYZ(-15.8869,-48.8948,25.0748);
   fPositionVectors[53].SetXYZ(0.0000,-51.411,25.0748);
   fPositionVectors[54].SetXYZ(15.8869,-48.8948,25.0748);
   fPositionVectors[55].SetXYZ(30.2186,-41.5924,25.0748);
   fPositionVectors[56].SetXYZ(41.5924,-30.2186,25.0748);
   fPositionVectors[57].SetXYZ(48.8948,-15.8869,25.0748);
   fPositionVectors[58].SetXYZ(56.2422,0.0000,10.4239);
   fPositionVectors[59].SetXYZ(52.8504,19.236,10.4239);
   fPositionVectors[60].SetXYZ(43.084,36.1518,10.4239);
   fPositionVectors[61].SetXYZ(28.1211,48.7072,10.4239);
   fPositionVectors[62].SetXYZ(9.76635,55.3877,10.4239);
   fPositionVectors[63].SetXYZ(-9.76635,55.3877,10.4239);
   fPositionVectors[64].SetXYZ(-28.1211,48.7072,10.4239);
   fPositionVectors[65].SetXYZ(-43.084,36.1518,10.4239);
   fPositionVectors[66].SetXYZ(-52.8504,19.236,10.4239);
   fPositionVectors[67].SetXYZ(-56.2422,0.0000,10.4239);
   fPositionVectors[68].SetXYZ(-52.8504,-19.236,10.4239);
   fPositionVectors[69].SetXYZ(-43.084,-36.1518,10.4239);
   fPositionVectors[70].SetXYZ(-28.1211,-48.7072,10.4239);
   fPositionVectors[71].SetXYZ(-9.76635,-55.3877,10.4239);
   fPositionVectors[72].SetXYZ(9.76635,-55.3877,10.4239);
   fPositionVectors[73].SetXYZ(28.1211,-48.7072,10.4239);
   fPositionVectors[74].SetXYZ(43.084,-36.1518,10.4239);
   fPositionVectors[75].SetXYZ(52.8504,-19.236,10.4239);
   fPositionVectors[76].SetXYZ(56.9823,0.0000,-4.98531);
   fPositionVectors[77].SetXYZ(53.5459,19.4891,-4.98531);
   fPositionVectors[78].SetXYZ(43.651,36.6275,-4.98531);
   fPositionVectors[79].SetXYZ(28.4912,49.3482,-4.98531);
   fPositionVectors[80].SetXYZ(9.89488,56.1166,-4.98531);
   fPositionVectors[81].SetXYZ(-9.89488,56.1166,-4.98531);
   fPositionVectors[82].SetXYZ(-28.4912,49.3482,-4.98531);
   fPositionVectors[83].SetXYZ(-43.651,36.6275,-4.98531);
   fPositionVectors[84].SetXYZ(-53.5459,19.4891,-4.98531);
   fPositionVectors[85].SetXYZ(-56.9823,0.0000,-4.98531);
   fPositionVectors[86].SetXYZ(-53.5459,-19.4891,-4.98531);
   fPositionVectors[87].SetXYZ(-43.651,-36.6275,-4.98531);
   fPositionVectors[88].SetXYZ(-28.4912,-49.3482,-4.98531);
   fPositionVectors[89].SetXYZ(-9.89488,-56.1166,-4.98531);
   fPositionVectors[90].SetXYZ(9.89488,-56.1166,-4.98531);
   fPositionVectors[91].SetXYZ(28.4912,-49.3482,-4.98531);
   fPositionVectors[92].SetXYZ(43.651,-36.6275,-4.98531);
   fPositionVectors[93].SetXYZ(53.5459,-19.4891,-4.98531);
   fPositionVectors[94].SetXYZ(53.0722,0.0000,-21.3349);
   fPositionVectors[95].SetXYZ(47.8164,23.0272,-21.3349);
   fPositionVectors[96].SetXYZ(33.09,41.4935,-21.3349);
   fPositionVectors[97].SetXYZ(11.8097,51.7416,-21.3349);
   fPositionVectors[98].SetXYZ(-11.8097,51.7416,-21.3349);
   fPositionVectors[99].SetXYZ(-33.09,41.4935,-21.3349);
   fPositionVectors[100].SetXYZ(-47.8164,23.0272,-21.3349);
   fPositionVectors[101].SetXYZ(-53.0722,0.0000,-21.3349);
   fPositionVectors[102].SetXYZ(-47.8164,-23.0272,-21.3349);
   fPositionVectors[103].SetXYZ(-33.09,-41.4935,-21.3349);
   fPositionVectors[104].SetXYZ(-11.8097,-51.7416,-21.3349);
   fPositionVectors[105].SetXYZ(11.8097,-51.7416,-21.3349);
   fPositionVectors[106].SetXYZ(33.09,-41.4935,-21.3349);
   fPositionVectors[107].SetXYZ(47.8164,-23.0272,-21.3349);
   fPositionVectors[108].SetXYZ(43.6891,0.0000,-36.9202);
   fPositionVectors[109].SetXYZ(37.8359,21.8446,-36.9202);
   fPositionVectors[110].SetXYZ(21.8446,37.8359,-36.9202);
   fPositionVectors[111].SetXYZ(0.0000,43.6891,-36.9202);
   fPositionVectors[112].SetXYZ(-21.8446,37.8359,-36.9202);
   fPositionVectors[113].SetXYZ(-37.8359,21.8446,-36.9202);
   fPositionVectors[114].SetXYZ(-43.6891,0.0000,-36.9202);
   fPositionVectors[115].SetXYZ(-37.8359,-21.8446,-36.9202);
   fPositionVectors[116].SetXYZ(-21.8446,-37.8359,-36.9202);
   fPositionVectors[117].SetXYZ(0.0000,-43.6891,-36.9202);
   fPositionVectors[118].SetXYZ(21.8446,-37.8359,-36.9202);
   fPositionVectors[119].SetXYZ(37.8359,-21.8446,-36.9202);
   fPositionVectors[120].SetXYZ(29.8869,0.0000,-48.771);
   fPositionVectors[121].SetXYZ(21.1332,21.1332,-48.771);
   fPositionVectors[122].SetXYZ(0.0000,29.8869,-48.771);
   fPositionVectors[123].SetXYZ(-21.1332,21.1332,-48.771);
   fPositionVectors[124].SetXYZ(-29.8869,0.0000,-48.771);
   fPositionVectors[125].SetXYZ(-21.1332,-21.1332,-48.771);
   fPositionVectors[126].SetXYZ(0.0000,-29.8869,-48.771);
   fPositionVectors[127].SetXYZ(21.1332,-21.1332,-48.771);

   SetGlobalBit(ETipGlobalBits::kVectorsBuilt, true);

}
