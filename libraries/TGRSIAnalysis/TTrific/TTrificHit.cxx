#include "TTrific.h"
#include "TTrificHit.h"
#include "Globals.h"

/// \cond CLASSIMP
ClassImp(TTrificHit)
/// \endcond

TTrificHit::TTrificHit() : TGRSIDetectorHit()
{
#if MAJOR_ROOT_VERSION < 6
	Class()->IgnoreTObjectStreamer(kTRUE);
#endif
	Clear();
}

TTrificHit::TTrificHit(const TTrificHit& rhs) : TGRSIDetectorHit()
{
#if MAJOR_ROOT_VERSION < 6
	Class()->IgnoreTObjectStreamer(kTRUE);
#endif
	Clear();
	rhs.Copy(*this);
}

TTrificHit::~TTrificHit() = default;

void TTrificHit::Copy(TObject& rhs) const
{
	TGRSIDetectorHit::Copy(rhs);
	static_cast<TTrificHit&>(rhs).fFilter = fFilter;
}

void TTrificHit::Clear(Option_t* opt)
{
	TGRSIDetectorHit::Clear(opt); // clears the base (address, position and waveform)
}

void TTrificHit::Print(Option_t*) const
{
	printf("TRIFIC Detector: %i\n", GetDetector());
	printf("TRIFIC Segment:    %i\n", GetSegment());
	printf("TRIFIC Energy:   %lf\n", GetEnergy());
	printf("TRIFIC Hit Time:   %f\n", GetTime());
}

TVector3 TTrificHit::GetPosition() const
//TVector3 TTrificHit::GetPosition()
{
	//calling GetPosition() on a TRIFIC hit should return the x,y,z location of the hit at that grid number

	//TVector3 particle = TTrific::GetPosition()

	//return TTrific::GetPosition(GetDetector());
	return TVector3(1,1,1);
}

