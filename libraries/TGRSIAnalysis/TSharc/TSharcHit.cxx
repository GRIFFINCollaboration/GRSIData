#include "TSharcHit.h"

#include "TClass.h"

#include "TSharc.h"
#include "TChannel.h"

/// \cond CLASSIMP
ClassImp(TSharcHit)
/// \endcond

TSharcHit::TSharcHit()
{
#if ROOT_VERSION_CODE < ROOT_VERSION(6,0,0)
	Class()->IgnoreTObjectStreamer(kTRUE);
#endif
	Clear("ALL");
}

TSharcHit::~TSharcHit() = default;

TSharcHit::TSharcHit(const TSharcHit& rhs) : TDetectorHit()
{
#if ROOT_VERSION_CODE < ROOT_VERSION(6,0,0)
	Class()->IgnoreTObjectStreamer(kTRUE);
#endif
	Clear();
	rhs.Copy(*this);
}

void TSharcHit::Copy(TObject& rhs) const
{
	TDetectorHit::Copy(rhs);
	static_cast<const TDetectorHit&>(fBackHit).Copy(static_cast<TObject&>(static_cast<TSharcHit&>(rhs).fBackHit));
	static_cast<const TDetectorHit&>(fPadHit).Copy(static_cast<TObject&>(static_cast<TSharcHit&>(rhs).fPadHit));
}

void TSharcHit::Clear(Option_t* options)
{
	TDetectorHit::Clear(options); //
	fBackHit.Clear(options);          //
	fPadHit.Clear(options);           //
}

void TSharcHit::Print(Option_t*) const
{
	Print(std::cout);
}

void TSharcHit::Print(std::ostream& out) const
{
	std::ostringstream str;
	str<<DGREEN<<"[D/F/B] = "<<std::setw(2)<<GetDetector()<<"\t/"<<std::setw(2)<<GetFrontStrip()<<"\t/"<<std::setw(2)<<GetBackStrip()<<RESET_COLOR<<std::endl;
	out<<str.str();
}

TVector3 TSharcHit::GetPosition(Double_t) const
{
	// return  fposition; // returned from this -> i.e front...
	// PC BENDER PLEASE LOOK AT THIS.
	//
	// this is fine, in all reality this function should not be used in sharc analysis,
	// the buildhits function now properly sets fPosition in the base class, for finer
	// position tweaks of the target, one should just use the static function in the
	// sharc mother class.   pcb.

	return TSharc::GetPosition(GetDetector(), GetSegment(), GetBack().GetSegment(),
			TSharc::GetXOffset(), TSharc::GetYOffset(), TSharc::GetZOffset());
}

TVector3 TSharcHit::GetPosition() const
{
	return GetPosition(GetDefaultDistance());
}

Double_t TSharcHit::GetTheta(double Xoff, double Yoff, double Zoff)
{
	TVector3 posOff;
	posOff.SetXYZ(Xoff, Yoff, Zoff);
	return (GetPosition() + posOff).Theta();
}

void TSharcHit::SetFront(const TFragment& frag)
{
	frag.Copy(*this);
}

void TSharcHit::SetBack(const TFragment& frag)
{
	frag.Copy(fBackHit);
}

void TSharcHit::SetPad(const TFragment& frag)
{
	frag.Copy(fPadHit);
}
