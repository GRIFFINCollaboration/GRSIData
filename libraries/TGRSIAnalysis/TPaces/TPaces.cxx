#include <iostream>
#include "TPaces.h"
#include "TRandom.h"
#include "TMath.h"

////////////////////////////////////////////////////////////
//
// TPaces
//
// The TPaces class defines the observables and algorithms used
// when analyzing GRIFFIN data.
//
////////////////////////////////////////////////////////////

/// \cond CLASSIMP
ClassImp(TPaces)
/// \endcond

bool TPaces::fSetCoreWave = false;

TPaces::TPaces() : TDetector()
{
#if MAJOR_ROOT_VERSION < 6
	Class()->IgnoreTObjectStreamer(kTRUE);
#endif
	Clear();
}

TPaces::TPaces(const TPaces& rhs) : TDetector()
{
#if MAJOR_ROOT_VERSION < 6
	Class()->IgnoreTObjectStreamer(kTRUE);
#endif
	rhs.Copy(*this);
}

void TPaces::Copy(TObject& rhs) const
{
	TDetector::Copy(rhs);

	static_cast<TPaces&>(rhs).fSetCoreWave = fSetCoreWave;
}

TPaces::~TPaces()
{
	// Default Destructor
}

void TPaces::Print(Option_t*) const
{
	// Prints out TPaces members, currently does nothing.
	printf("%lu fHits\n", fHits.size());
}

TPaces& TPaces::operator=(const TPaces& rhs)
{
	rhs.Copy(*this);
	return *this;
}

void TPaces::AddFragment(const std::shared_ptr<const TFragment>& frag, TChannel*)
{
	TPacesHit* hit = new TPacesHit(*frag);
	fHits.push_back(std::move(hit));
}

TVector3 TPaces::GetPosition(int)
{
	// Gets the position vector for a crystal specified by DetNbr
	// Does not currently contain any positons.
	return TVector3(0, 0, 1);
}
