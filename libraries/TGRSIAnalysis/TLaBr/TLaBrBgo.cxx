#include "TLaBrBgo.h"

#include <sstream>
#include <iostream>
#include <iomanip>

#include "TRandom.h"
#include "TMath.h"
#include "TInterpreter.h"

#include "TGRSIOptions.h"

////////////////////////////////////////////////////////////
//
// TLaBrBgo
//
// The TLaBrBgo class is just a differently name TBgo.
// This allows us to distinguish between the BGOs for diiferent
// detector classes.
//
////////////////////////////////////////////////////////////

TLaBrBgo::TLaBrBgo()
{
   /// Default ctor.
   TBgo::Clear();
}

TLaBrBgo::TLaBrBgo(const TLaBrBgo& rhs) : TBgo(rhs)
{
   /// Copy ctor.
   rhs.Copy(*this);
}

TLaBrBgo& TLaBrBgo::operator=(const TLaBrBgo& rhs)
{
   rhs.Copy(*this);
   return *this;
}
