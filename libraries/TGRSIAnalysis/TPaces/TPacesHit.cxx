#include "TPaces.h"
#include "TPacesHit.h"
#include "Globals.h"

TPacesHit::TPacesHit()
{
   Clear();
}

TPacesHit::TPacesHit(const TPacesHit& rhs) : TDetectorHit(rhs)
{
   Clear();
   rhs.Copy(*this);
}

void TPacesHit::Copy(TObject& rhs) const
{
   TDetectorHit::Copy(rhs);
   static_cast<TPacesHit&>(rhs).fFilter = fFilter;
}

void TPacesHit::Copy(TObject& rhs, bool waveform) const
{
	Copy(rhs);
	if(waveform) {
		CopyWave(rhs);
	}
}

bool TPacesHit::InFilter(Int_t)
{
   // check if the desired filter is in wanted filter;
   // return the answer;
   return true;
}

void TPacesHit::Clear(Option_t* opt)
{
   TDetectorHit::Clear(opt); // clears the base (address, position and waveform)
   fFilter = 0;
}

void TPacesHit::Print(Option_t*) const
{
	Print(std::cout);
}

void TPacesHit::Print(std::ostream& out) const
{
	std::ostringstream str;
   str<<"Paces Detector: "<<GetDetector()<<std::endl;
   str<<"Paces Energy:   "<<GetEnergy()<<std::endl;
   str<<"Paces hit time: "<<GetTime()<<std::endl;
	out<<str.str();
}

TVector3 TPacesHit::GetPosition(Double_t) const
{
	auto vec = TPaces::GetPosition(GetDetector());
   return {vec.X(), vec.Y(), vec.Z()};
}

TVector3 TPacesHit::GetPosition() const
{
   return GetPosition(GetDefaultDistance());
}
