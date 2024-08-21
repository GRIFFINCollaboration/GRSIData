#include "TGriffinBgo.h"

#include <sstream>
#include <iostream>
#include <iomanip>

#include "TRandom.h"
#include "TMath.h"
#include "TInterpreter.h"

#include "TGRSIOptions.h"

////////////////////////////////////////////////////////////
//
// TGriffinBgo
//
// The TGriffinBgo class is just a differently named TBgo.
// This allows us to distinguish between the BGOs for different
// detector classes.
//
////////////////////////////////////////////////////////////

/// \cond CLASSIMP
ClassImp(TGriffinBgo)
/// \endcond

TGriffinBgo::TGriffinBgo()
{
	/// Default ctor.
   TBgo::Clear();
}

TGriffinBgo::TGriffinBgo(const TGriffinBgo& rhs) : TBgo(rhs)
{
	/// Copy ctor.
   rhs.Copy(*this);
}

TGriffinBgo::~TGriffinBgo()
{
   // Default Destructor
}

TGriffinBgo& TGriffinBgo::operator=(const TGriffinBgo& rhs)
{
   rhs.Copy(*this);
   return *this;
}
