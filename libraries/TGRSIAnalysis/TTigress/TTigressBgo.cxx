#include "TTigressBgo.h"

#include <sstream>
#include <iostream>
#include <iomanip>

#include "TRandom.h"
#include "TMath.h"
#include "TInterpreter.h"

#include "TGRSIOptions.h"

////////////////////////////////////////////////////////////
//
// TTigressBgo
//
// The TTigressBgo class is just a differently named TBgo.
// This allows us to distinguish between the BGOs for different
// detector classes.
//
////////////////////////////////////////////////////////////

TTigressBgo::TTigressBgo()
{
   /// Default ctor.
   TBgo::Clear();
}

TTigressBgo::TTigressBgo(const TTigressBgo& rhs) : TBgo(rhs)
{
   /// Copy ctor.
   rhs.Copy(*this);
}

TTigressBgo& TTigressBgo::operator=(const TTigressBgo& rhs)
{
   rhs.Copy(*this);
   return *this;
}
