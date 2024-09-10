#include "TTrific.h"
#include "TTrificHit.h"
#include "Globals.h"

TTrificHit::TTrificHit()
{
#if ROOT_VERSION_CODE < ROOT_VERSION(6,0,0)
	Class()->IgnoreTObjectStreamer(kTRUE);
#endif
	Clear();
}

TTrificHit::TTrificHit(const TTrificHit& rhs) : TDetectorHit(rhs)
{
#if ROOT_VERSION_CODE < ROOT_VERSION(6,0,0)
	Class()->IgnoreTObjectStreamer(kTRUE);
#endif
	Clear();
	rhs.Copy(*this);
}

void TTrificHit::Copy(TObject& rhs) const
{
	TDetectorHit::Copy(rhs);
	static_cast<TTrificHit&>(rhs).fFilter = fFilter;
}

void TTrificHit::Copy(TObject& rhs, bool waveform) const
{
	Copy(rhs);
	if(waveform) {
		CopyWave(rhs);
	}
}

void TTrificHit::Clear(Option_t* opt)
{
	TDetectorHit::Clear(opt); // clears the base (address, position and waveform)
}

void TTrificHit::Print(Option_t*) const
{
	Print(std::cout);
}

void TTrificHit::Print(std::ostream& out) const
{
	std::ostringstream str;
	str<<"TRIFIC Detector: "<<GetDetector()<<std::endl;
	str<<"TRIFIC Segment:    "<<GetSegment()<<std::endl;
	str<<"TRIFIC Energy:   "<<GetEnergy()<<std::endl;
	str<<"TRIFIC Hit Time:   "<<GetTime()<<std::endl;
	out<<str.str();
}

TVector3 TTrificHit::GetPosition() const
{
	//calling GetPosition() on a TRIFIC hit will return the position vector to the centre of the grid
	//calling TTrific::GetPosition(det) will give the vector to the position itself.

	return {0,0,TTrific::fTargetToWindowCart+TTrific::fInitialSpacingCart+TTrific::fSpacingCart*GetDetector()};
}

