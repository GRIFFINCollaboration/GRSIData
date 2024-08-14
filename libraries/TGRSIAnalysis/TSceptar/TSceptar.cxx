#include <iostream>
#include "TSceptar.h"
#include <TRandom.h>
#include <TMath.h>

/// \cond CLASSIMP
ClassImp(TSceptar)
/// \endcond

bool TSceptar::fSetWave = false;

TVector3 TSceptar::gPaddlePosition[21] = {
	// Sceptar positions from Evan; Thanks Evan.
	TVector3(0, 0, 1),
	TVector3(14.3025, 4.6472, 22.8096),
	TVector3(0, 15.0386, 22.8096),
	TVector3(-14.3025, 4.6472, 22.8096),
	TVector3(-8.8395, -12.1665, 22.8096),
	TVector3(8.8395, -12.1665, 22.8096),
	TVector3(19.7051, 6.4026, 6.2123),
	TVector3(0, 20.7192, 6.2123),
	TVector3(-19.7051, 6.4026, 6.2123),
	TVector3(-12.1784, -16.7622, 6.2123),
	TVector3(12.1784, -16.7622, 6.2123),
	TVector3(19.7051, 6.4026, -6.2123),
	TVector3(0, 20.7192, -6.2123),
	TVector3(-19.7051, 6.4026, -6.2123),
	TVector3(-12.1784, -16.7622, -6.2123),
	TVector3(12.1784, -16.7622, -6.2123),
	TVector3(14.3025, 4.6472, -22.8096),
	TVector3(0, 15.0386, -22.8096),
	TVector3(-14.3025, 4.6472, -22.8096),
	TVector3(-8.8395, -12.1665, -22.8096),
	TVector3(8.8395, -12.1665, -22.8096)};

TSceptar::TSceptar()
{
	// Default Constructor
#if ROOT_VERSION_CODE < ROOT_VERSION(6,0,0)
	Class()->IgnoreTObjectStreamer(kTRUE);
#endif
	// Class()->AddRule("TSceptar sceptar_hits attributes=NotOwner");
	// Class()->AddRule("TSceptar sceptardata attributes=NotOwner");
	Clear();
}

TSceptar::~TSceptar()
{
	// Default Destructor
}

TSceptar::TSceptar(const TSceptar& rhs) : TDetector()
{
	// Copy Contructor
#if ROOT_VERSION_CODE < ROOT_VERSION(6,0,0)
	Class()->IgnoreTObjectStreamer(kTRUE);
#endif
	rhs.Copy(*this);
}

TSceptar& TSceptar::operator=(const TSceptar& rhs)
{
	rhs.Copy(*this);
	return *this;
}

void TSceptar::AddFragment(const std::shared_ptr<const TFragment>& frag, TChannel*)
{
	TSceptarHit* hit = new TSceptarHit(*frag); // Construction of TSceptarHit is handled in the constructor
	AddHit(hit);
}

void TSceptar::Print(Option_t*) const
{
	/// Prints out TSceptar Multiplicity, currently does little.
	Print(std::cout);
}

void TSceptar::Print(std::ostream& out) const
{
	std::ostringstream str;
	str<<GetMultiplicity()<<" hits"<<std::endl;
	out<<str.str();
}
