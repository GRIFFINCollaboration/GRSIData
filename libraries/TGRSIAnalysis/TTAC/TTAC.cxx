#include "TTAC.h"
#include <iostream>
#include "TRandom.h"
#include "TMath.h"

TTAC::TTAC()
{
	// Default Constructor
#if ROOT_VERSION_CODE < ROOT_VERSION(6,0,0)
	Class()->IgnoreTObjectStreamer(kTRUE);
#endif
	Clear();
}

TTAC::TTAC(const TTAC& rhs) : TDetector(rhs)
{
	// Copy Contructor
#if ROOT_VERSION_CODE < ROOT_VERSION(6,0,0)
	Class()->IgnoreTObjectStreamer(kTRUE);
#endif
	rhs.Copy(*this);
}

TTAC& TTAC::operator=(const TTAC& rhs)
{
	rhs.Copy(*this);
	return *this;
}

void TTAC::Print(Option_t*) const
{
	// Prints out TTAC Multiplicity, currently does little.
	Print(std::cout);
}

void TTAC::Print(std::ostream& out) const
{
	std::ostringstream str;
	str<<GetMultiplicity()<<" hits"<<std::endl;
	out<<str.str();
}

void TTAC::AddFragment(const std::shared_ptr<const TFragment>& frag, TChannel*)
{
	auto* hit = new TTACHit(*frag);
	AddHit(hit);
}
