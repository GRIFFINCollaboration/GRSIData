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
   virtual ~TLaBrBgo();

   TLaBrBgoHit* GetLaBrBgoHit(const int& i) const { return static_cast<TLaBrBgoHit*>(GetHit(i)); }

   TLaBrBgo& operator=(const TLaBrBgo&); //!<!

   /// \cond CLASSIMP
   ClassDef(TLaBrBgo, 1) // LaBrBgo Physics structure
   /// \endcond
};
/*! @} */
#endif
