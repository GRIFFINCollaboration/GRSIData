#include "TTAC.h"
#include <iostream>
#include "TRandom.h"
#include "TMath.h"

/// \cond CLASSIMP
ClassImp(TTAC)
/// \endcond

TTAC::TTAC()
{
	// Default Constructor
#if ROOT_VERSION_CODE < ROOT_VERSION(6,0,0)
	Class()->IgnoreTObjectStreamer(kTRUE);
#endif
	Clear();
}

TTAC::~TTAC()
{
	// Default Destructor
}

TTAC::TTAC(const TTAC& rhs) : TDetector()
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
	printf("%lu fHits\n", fHits.size());
}

void TTAC::AddFragment(const std::shared_ptr<const TFragment>& frag, TChannel*)
{
	TTACHit* hit = new TTACHit(*frag);
	fHits.push_back(hit);
}
