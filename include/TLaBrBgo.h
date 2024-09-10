#ifndef TLABRBGO_H
#define TLABRBGO_H

/** \addtogroup Detectors
 *  @{
 */

#include "Globals.h"
#include "TBgo.h"
#include "TLaBrBgoHit.h"

class TLaBrBgo : public TBgo {
public:
   TLaBrBgo();
   TLaBrBgo(const TLaBrBgo&);
   TLaBrBgo(TLaBrBgo&&) noexcept = default;
   TLaBrBgo& operator=(const TLaBrBgo&);   //!<!
   TLaBrBgo& operator=(TLaBrBgo&&) noexcept = default;
   virtual ~TLaBrBgo()                      = default;

   TLaBrBgoHit* GetLaBrBgoHit(const int& i) const { return static_cast<TLaBrBgoHit*>(GetHit(i)); }

   /// \cond CLASSIMP
   ClassDef(TLaBrBgo, 1)   // LaBrBgo Physics structure // NOLINT(readability-else-after-return)
   /// \endcond
};
/*! @} */
#endif
